/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2010 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

/** \file jsinterp.c Ayam JavaScript interpreter plugin based on
    Mozilla SpiderMonkey */

/* Ayam includes: */
#include "ayam.h"

/* SpiderMonkey includes: */
#include "jsapi.h"


/* prototypes of functions local to this module: */

/** error reporting wrapper function */
void jsinterp_error(JSContext *cx, const char *message, JSErrorReport *report);

/** helper to convert function/command arguments */
int jsinterp_convargs(JSContext *cx, uintN argc, jsval *argv,
		      char ***sargv);

/** helper function to convert a Tcl_Obj to a jsval */
int jsinterp_objtoval(Tcl_Obj *to, jsval *v);

/** JS function to wrap the eval command */
int jsinterp_wrapevalcmd(JSContext *cx, JSObject *obj,
			 uintN argc, jsval *argv,
			 jsval *rval);

/** JS function to wrap a command with args */
int jsinterp_wraptcmdargs(JSContext *cx, JSObject *obj,
			  uintN argc, jsval *argv,
			  jsval *rval);

/** JS function to wrap a command without args */
int jsinterp_wraptcmd(JSContext *cx, JSObject *obj,
		      uintN argc, jsval *argv,
		      jsval *rval);

/** variable trace procedure to transport a Tcl variable to JavaScript */
char *jsinterp_vartraceproc(ClientData clientData, Tcl_Interp *interp,
			    char *name1, char *name2, int flags);

/** JS function to link a Tcl var to a JS var */
int jsinterp_tclvar(JSContext *cx, JSObject *obj,
		    uintN argc, jsval *argv,
		    jsval *rval);

/** helper function to convert a jsval to a Tcl_Obj */
void jsinterp_jsvaltoobj(jsval jv, Tcl_Obj **to);

/** JS function to set a Tcl var from JS */
int jsinterp_tclset(JSContext *cx, JSObject *obj,
		    uintN argc, jsval *argv,
		    jsval *rval);

/** Tcl command to evaluate a JS script */
int jsinterp_evaltcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** Ayam Script object evaluation callback */
int jsinterp_evalcb(Tcl_Interp *interp, char *script, int compile,
		    Tcl_Obj **cscript);


/* global variables: */

/** major Ayam version we are compiled with */
char jsinterp_version_ma[] = AY_VERSIONSTR;
/** minor Ayam version we are compiled with */
char jsinterp_version_mi[] = AY_VERSIONSTRMI;

/** language id */
char *jsinterp_langtype = NULL;

/** JS runtime */
static JSRuntime *jsinterp_rt;
/** JS context */
static JSContext *jsinterp_cx;
/** JS global object */
static JSObject *jsinterp_global;

/** The class of the global object. */
static JSClass jsinterp_global_class = {
    "global", JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

/** A bunch of pre-defined global functions,
    that wrap Ayam Tcl commands (e.g. crtOb()),
    evaluate Tcl scripts (tcleval()),
    manage variable connections (tclvar()), and
    set Tcl variables from JS (tclset()). */
static JSFunctionSpec jsinterp_global_functions[] = {
  {"crtOb", jsinterp_wraptcmdargs, 0, 0, 0},
  {"delOb", jsinterp_wraptcmd, 0, 0, 0},
  {"cutOb", jsinterp_wraptcmd, 0, 0, 0},
  {"copOb", jsinterp_wraptcmd, 0, 0, 0},
  {"pasOb", jsinterp_wraptcmd, 0, 0, 0},
  {"pasmovOb", jsinterp_wraptcmd, 0, 0, 0},
  {"repOb", jsinterp_wraptcmd, 0, 0, 0},
  {"clearClip", jsinterp_wraptcmd, 0, 0, 0},
  {"convOb", jsinterp_wraptcmd, 0, 0, 0},
  {"notifyOb", jsinterp_wraptcmdargs, 0, 0, 0},
  {"goTop", jsinterp_wraptcmd, 0, 0, 0},
  {"goUp", jsinterp_wraptcmd, 0, 0, 0},
  {"goDown", jsinterp_wraptcmd, 0, 0, 0},
  {"getLevel", jsinterp_wraptcmdargs, 0, 0, 0},
  {"resolveIn", jsinterp_wraptcmd, 0, 0, 0},
  {"nameOb", jsinterp_wraptcmdargs, 0, 0, 0},
  {"showOb", jsinterp_wraptcmdargs, 0, 0, 0},
  {"hideOb", jsinterp_wraptcmdargs, 0, 0, 0},

  {"mergePo", jsinterp_wraptcmd, 0, 0, 0},
  {"optiPo", jsinterp_wraptcmd, 0, 0, 0},
  {"splitPo", jsinterp_wraptcmd, 0, 0, 0},
  {"genfnPo", jsinterp_wraptcmd, 0, 0, 0},
  {"gensnPo", jsinterp_wraptcmd, 0, 0, 0},

  {"setProp", jsinterp_wraptcmd, 0, 0, 0},
  {"getProp", jsinterp_wraptcmd, 0, 0, 0},
  {"setTrafo", jsinterp_wraptcmd, 0, 0, 0},
  {"getTrafo", jsinterp_wraptcmd, 0, 0, 0},
  {"setAttr", jsinterp_wraptcmd, 0, 0, 0},
  {"getAttr", jsinterp_wraptcmd, 0, 0, 0},
  {"setMat", jsinterp_wraptcmd, 0, 0, 0},
  {"getMat", jsinterp_wraptcmd, 0, 0, 0},
  {"shaderSet", jsinterp_wraptcmd, 0, 0, 0},
  {"shaderGet", jsinterp_wraptcmd, 0, 0, 0},
  {"selOb", jsinterp_wraptcmd, 0, 0, 0},
  {"getSel", jsinterp_wraptcmdargs, 0, 0, 0},
  {"sL", jsinterp_wraptcmd, 0, 0, 0},
  {"setTags", jsinterp_wraptcmdargs, 0, 0, 0},
  {"addTag", jsinterp_wraptcmdargs, 0, 0, 0},
  {"getTags", jsinterp_wraptcmdargs, 0, 0, 0},
  {"getTag", jsinterp_wraptcmdargs, 0, 0, 0},
  {"hasTag", jsinterp_wraptcmdargs, 0, 0, 0},
  {"delTags", jsinterp_wraptcmdargs, 0, 0, 0},

  {"getPnt", jsinterp_wraptcmdargs, 0, 0, 0},
  {"setPnt", jsinterp_wraptcmdargs, 0, 0, 0},
  {"delegTrafo", jsinterp_wraptcmd, 0, 0, 0},
  {"movOb", jsinterp_wraptcmdargs, 0, 0, 0},
  {"movPnts", jsinterp_wraptcmdargs, 0, 0, 0},
  {"scalOb", jsinterp_wraptcmdargs, 0, 0, 0},
  {"scalPnts", jsinterp_wraptcmdargs, 0, 0, 0},
  {"rotOb", jsinterp_wraptcmdargs, 0, 0, 0},
  {"rotPnts", jsinterp_wraptcmdargs, 0, 0, 0},
  {"selPnts", jsinterp_wraptcmdargs, 0, 0, 0},
  {"invPnts", jsinterp_wraptcmd, 0, 0, 0},
  {"applyTrafo", jsinterp_wraptcmdargs, 0, 0, 0},
  {"centerPnts", jsinterp_wraptcmd, 0, 0, 0},

  {"revertC", jsinterp_wraptcmd, 0, 0, 0},

  {"refineC", jsinterp_wraptcmd, 0, 0, 0},
  {"refineknNC", jsinterp_wraptcmd, 0, 0, 0},
  {"coarsenNC", jsinterp_wraptcmd, 0, 0, 0},
  {"clampNC", jsinterp_wraptcmd, 0, 0, 0},
  {"unclampNC", jsinterp_wraptcmdargs, 0, 0, 0},
  {"elevateNC", jsinterp_wraptcmdargs, 0, 0, 0},
  {"insknNC", jsinterp_wraptcmdargs, 0, 0, 0},
  {"remknNC", jsinterp_wraptcmdargs, 0, 0, 0},
  /* collMP explMP ? */
  {"splitNC", jsinterp_wraptcmdargs, 0, 0, 0},
  {"concatNC", jsinterp_wraptcmdargs, 0, 0, 0},
  {"crtNCircle", jsinterp_wraptcmdargs, 0, 0, 0},
  {"crtNRect", jsinterp_wraptcmdargs, 0, 0, 0},
  {"crtTrimRect", jsinterp_wraptcmdargs, 0, 0, 0},
  {"crtClosedBS", jsinterp_wraptcmdargs, 0, 0, 0},
  {"rescaleknNC", jsinterp_wraptcmdargs, 0, 0, 0},
  /* curvPlot ? */
  {"shiftC", jsinterp_wraptcmdargs, 0, 0, 0},
  {"toXYNC", jsinterp_wraptcmdargs, 0, 0, 0},
  {"toXYC", jsinterp_wraptcmdargs, 0, 0, 0},
  {"makeCompNC", jsinterp_wraptcmdargs, 0, 0, 0},
  {"centerNC", jsinterp_wraptcmdargs, 0, 0, 0},
  {"trimNC", jsinterp_wraptcmdargs, 0, 0, 0},
  {"estlenNC", jsinterp_wraptcmdargs, 0, 0, 0},
  {"reparamNC", jsinterp_wraptcmdargs, 0, 0, 0},
  {"extendNC", jsinterp_wraptcmdargs, 0, 0, 0},

  {"crtNSphere", jsinterp_wraptcmd, 0, 0, 0},
  {"crtNSphere2", jsinterp_wraptcmd, 0, 0, 0},
  {"breakNP", jsinterp_wraptcmd, 0, 0, 0},
  {"buildNP", jsinterp_wraptcmd, 0, 0, 0},
  /* topoly? */
  {"elevateuNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"elevatevNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"refineuNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"refinevNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"swapuvS", jsinterp_wraptcmd, 0, 0, 0},
  {"revertuS", jsinterp_wraptcmd, 0, 0, 0},
  {"revertvS", jsinterp_wraptcmd, 0, 0, 0},
  {"closeuNP", jsinterp_wraptcmd, 0, 0, 0},
  {"closevNP", jsinterp_wraptcmd, 0, 0, 0},
  {"clampuNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"clampvNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"unclampuNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"unclampvNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"rescaleknNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"insknuNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"insknvNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"splituNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"splitvNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"remknuNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"remknvNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"extrNP", jsinterp_wraptcmdargs, 0, 0, 0},
  {"concatS", jsinterp_wraptcmdargs, 0, 0, 0},
  {"tobasisPM", jsinterp_wraptcmdargs, 0, 0, 0},

  {"tcleval", jsinterp_wrapevalcmd, 0, 0, 0},
  {"tclvar", jsinterp_tclvar, 0, 0, 0},
  {"tclset", jsinterp_tclset, 0, 0, 0},
  {0}
};

/** current Tcl interpreter */
static Tcl_Interp *jsinterp_interp;

/** \name Tcl_Obj types (for data conversion in jsinterp_objtoval()) */
/*@{*/
static Tcl_ObjType *jsinterp_BooleanType = NULL;
static Tcl_ObjType *jsinterp_ByteArrayType = NULL;
static Tcl_ObjType *jsinterp_DoubleType = NULL;
static Tcl_ObjType *jsinterp_IntType = NULL;
static Tcl_ObjType *jsinterp_LongType = NULL;
static Tcl_ObjType *jsinterp_ListType = NULL;
static Tcl_ObjType *jsinterp_StringType = NULL;
static Tcl_ObjType *jsinterp_WideIntType = NULL;
/*@}*/

/** current error line */
static unsigned int jsinterp_errorline;


/* functions: */

/* jsinterp_error:
 *  error reporting wrapper function
 */
void
jsinterp_error(JSContext *cx, const char *message, JSErrorReport *report)
{
 char fname[] = "jsinterp_error";
 char buf[128] = "";

  if(!cx || !message || !report)
    {
      ay_error(AY_ENULL, fname, NULL);
    }
  else
    {
      sprintf(buf, "JS line %u", (unsigned int) report->lineno);
      ay_error(AY_ERROR, buf, message);

      jsinterp_errorline = (unsigned int) report->lineno;
    }

 return;
} /* jsinterp_error */


/* jsinterp_unload:
 *  unused, there is no way to shut down the plugin
 */
void
jsinterp_unload()
{

    /* Cleanup. */
    JS_DestroyContext(jsinterp_cx);
    JS_DestroyRuntime(jsinterp_rt);
    JS_ShutDown();

 return;
} /* jsinterp_unload */


/* jsinterp_convargs:
 *  helper to convert function/command arguments from
 *  JS objects to C/Tcl char*s
 */
int
jsinterp_convargs(JSContext *cx, uintN argc, jsval *argv,
		  char ***sargv)
{
 int ay_status = AY_OK;
 char *c, **nargv = NULL;
 uintN i;
 JSString *jss;
 jsval jssval;

  if(!(nargv = calloc(argc+1, sizeof(char*))))
    return AY_EOMEM;

  /* convert function/command name */
  nargv[0] = JS_GetStringBytes(JS_GetFunctionId(
				      JS_ValueToFunction(cx, argv[-2])));

  /* convert args */
  for(i = 0; i < argc; i++)
    {
      jss = JS_ValueToString(cx, argv[i]);

      if(jss)
	{
	  jssval = STRING_TO_JSVAL(jss);
	  JS_AddRoot(jsinterp_cx, &jssval);

	  c = JS_GetStringBytes(jss);

	  if(c && (c[0] != '\0'))
	    {
	      if(!(nargv[i+1] = calloc(strlen(c)+1, sizeof(char))))
		{
		  JS_RemoveRoot(jsinterp_cx, &jssval);
		  ay_status = AY_EOMEM;
		  goto cleanup;
		}

	      strcpy(nargv[i+1], c);

	      /* convert JS Array syntax (a,b,c) to Tcl list syntax (a b c) */
	      if(JSVAL_IS_OBJECT(argv[i]))
		{
		  if(JS_IsArrayObject(cx, JSVAL_TO_OBJECT(argv[i])))
		    {
		      c = nargv[i+1];
		      while(*c != '\0')
			{
			  if(*c == ',')
			    {
			      *c = ' ';
			    }
			  c++;
			}
		    } /* if */
		} /* if */
	    }
	  else
	    {
	      /* could not get string? */
	      JS_RemoveRoot(jsinterp_cx, &jssval);
	      ay_status = AY_ERROR;
	      goto cleanup;
	    } /* if */
	  JS_RemoveRoot(jsinterp_cx, &jssval);
	}
      else
	{
	  ay_status = AY_ERROR;
	  goto cleanup;
	} /* if */
    } /* for */

cleanup:

  if(ay_status == AY_OK)
    {
      /* return result */
      *sargv = nargv;
    }
  else
    {
      /* clean up partial conversion */
      if(nargv)
	{
	  for(; i > 0; i--)
	    {
	      if(nargv[i])
		free(nargv[i]);
	    }

	  free(nargv);
	} /* if */
    } /* if */

 return ay_status;
} /* jsinterp_convargs */


/* jsinterp_objtoval:
 *  helper function to convert a Tcl_Obj to a jsval
 *  XXXX add support for unicode strings
 */
int
jsinterp_objtoval(Tcl_Obj *to, jsval *v)
{
 char *sval;
 int ival;
 JSString *jss;
 JSObject *arr;
 jsint i;
 int objc;
 Tcl_Obj **objv;
 jsval elemv, arrv;

  if((to->typePtr == jsinterp_IntType) || (to->typePtr == jsinterp_LongType))
    {
      if(JS_NewNumberValue(jsinterp_cx, (double)to->internalRep.longValue, v))
	{
	  return AY_OK;
	}
    }
    else if(to->typePtr == jsinterp_WideIntType)
    {
      if(JS_NewNumberValue(jsinterp_cx, (double)to->internalRep.wideValue, v))
	{
	  return AY_OK;
	}
    }
    else if(to->typePtr == jsinterp_DoubleType)
    {
      if(JS_NewNumberValue(jsinterp_cx, (double)to->internalRep.doubleValue, v))
	{
	  return AY_OK;
	}
    }
    else if(to->typePtr == jsinterp_BooleanType)
    {
      if(Tcl_GetIntFromObj(NULL, to, &ival) != TCL_ERROR)
	{
	  if(ival == 0)
	    *v = JSVAL_FALSE;
	  else
	    *v = JSVAL_TRUE;
	  return AY_OK;
	}
    }
    else if(to->typePtr == jsinterp_ListType)
    {
      Tcl_ListObjGetElements(NULL, to, &objc, &objv);
      if(objc)
	{
	  if((arr = JS_NewArrayObject(jsinterp_cx, objc, NULL)))
	    {
	      /* root the Array object so that it is GC safe */
	      arrv = OBJECT_TO_JSVAL(arr);
	      JS_AddRoot(jsinterp_cx, &arrv);

	      /* now convert/add the array elements */
	      for(i = 0; i < (jsint)objc; i++)
		{
		  if(!jsinterp_objtoval(objv[i], &elemv))
		    {
		      if(!JS_SetElement(jsinterp_cx, arr, i, &elemv))
			{
			  return AY_ERROR;
			}
		    }
		  else
		    {
		      return AY_ERROR;
		    }
		} /* for */

	      /* unroot the Array object to avoid memory leak */
	      JS_RemoveRoot(jsinterp_cx, &arrv);

	      /* for the outermost call, this effectively roots the
		array again (sigh) so that it is GC safe */
	      *v = arrv;
	      return AY_OK;
	    } /* if */
	}
      else
	{
	  /* create empty string */
	  /* XXXX rather create empty array object? */
	  if((jss = JS_NewStringCopyZ(jsinterp_cx, NULL)))
	    {
	      *v = STRING_TO_JSVAL(jss);
	      return AY_OK;
	    }
	}
    } else {
      /* XXXX TODO
       * for unicode, rather use Tcl_GetStringFromObj() with lengthPtr and then
       * JSString * JS_NewStringCopyN(JSContext *cx, const char *s, size_t n);
       */
      if((sval = Tcl_GetString(to)) != NULL)
	{
	  if((jss = JS_NewStringCopyZ(jsinterp_cx, sval)))
	    {
	      *v = STRING_TO_JSVAL(jss);
	      return AY_OK;
	    }
	}
    } /* switch objtype */
  /*
  else if (to->typePtr == tclByteArrayTypePtr) {
    str = (char *) Tcl_GetByteArrayFromObj(to, &len);
    ...
  }
  */

 return AY_ERROR;
} /* jsinterp_objtoval */


/* jsinterp_wrapevalcmd:
 *  JS function to wrap the eval command
 *  we can not use jsinterp_wraptcmdargs() because the Tcl_CmdInfo proc
 *  of "eval" points to Tcl_CommandObjV which we can not use with plain
 *  argc/argv-args; furthermore, we need to transfer the result of the
 *  evaluation back to JS
 */
int
jsinterp_wrapevalcmd(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
		     jsval *rval)
{
 int tcl_status = TCL_OK, retval = JS_TRUE;
 char *script, *resstr;
 Tcl_Obj *resobj;

  if(argc < 1)
    {
      JS_ReportError(cx, "insufficient arguments, need script");
      return JS_FALSE;
    }

  if(JSVAL_IS_STRING(argv[0]))
    {
      script = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
      tcl_status = Tcl_EvalEx(jsinterp_interp, script, -1, TCL_EVAL_GLOBAL);
      switch(tcl_status)
	{
	case TCL_ERROR:
	  JS_ReportError(cx, "Tcl script failed");
	  /* XXXX add line info? */
	  retval = JS_FALSE;
	  break;
	default:
	  /* caters for TCL_RETURN, TCL_OK, TCL_BREAK, and TCL_CONTINUE */
	  resobj = Tcl_GetObjResult(jsinterp_interp);
	  resstr = Tcl_GetString(resobj);
	  if(resstr && (resstr[0] != '\0'))
	    {
	      if(!jsinterp_objtoval(resobj, rval))
		{
		  return JS_TRUE;
		}
	      else
		{
		  JS_ReportError(cx, "failed to convert script result");
		  retval = JS_FALSE;
		}
	    }
	  break;
	} /* switch */
    }
  else
    {
      JS_ReportError(cx, "bad script");
      retval = JS_FALSE;
    }

  *rval = JSVAL_VOID; /* return undefined */

 return retval;
} /* jsinterp_wrapevalcmd */


/* jsinterp_wraptcmdargs:
 *  JS function to wrap a Tcl command that takes arguments
 */
int
jsinterp_wraptcmdargs(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
		      jsval *rval)
{
 int ay_status = AY_OK, js_status = JS_TRUE;
 ClientData clientData = {0};
 Tcl_CmdInfo cmdinfo = {0};
 char **sargv;
 uintN i;

  ay_status = jsinterp_convargs(cx, argc, argv, &sargv);

  if(ay_status)
    {
      JS_ReportError(cx, "argument conversion failed");
      return JS_FALSE;
    }

  if(Tcl_GetCommandInfo(jsinterp_interp, sargv[0], &cmdinfo))
    {
      if(!cmdinfo.isNativeObjectProc)
	{
	  cmdinfo.proc(clientData, jsinterp_interp, argc+1, sargv);
	}
      else
	{
	  JS_ReportError(cx, "unsupported command type");
	  js_status = JS_FALSE;
	}
    }
  else
    {
      /* command not found! */
      JS_ReportError(cx, "command not found");
      js_status = JS_FALSE;
    }

  if(sargv)
    {
      for(i = 1; i < argc+1; i++)
	{
	  if(sargv[i])
	    free(sargv[i]);
	}
      free(sargv);
    }

  *rval = JSVAL_VOID; /* return undefined */

 return js_status;
} /* jsinterp_wraptcmdargs */


/* jsinterp_wraptcmd:
 *  JS function to wrap a simple Tcl command (no arguments)
 */
int
jsinterp_wraptcmd(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
		  jsval *rval)
{
 char *sargv[2] = {0};
 ClientData clientData = {0};
 Tcl_CmdInfo cmdinfo = {0};

  sargv[0] = JS_GetStringBytes(JS_GetFunctionId(
				JS_ValueToFunction(cx, argv[-2])));

  if(Tcl_GetCommandInfo(jsinterp_interp, sargv[0], &cmdinfo))
    {
      if(!cmdinfo.isNativeObjectProc)
	{
	  cmdinfo.proc(clientData, jsinterp_interp, 1, sargv);
	}
      else
	{
	  JS_ReportError(cx, "unsupported command type");
	  return JS_FALSE;
	}
    }
  else
    {
      /* command not found! */
      JS_ReportError(cx, "command not found");
      return JS_FALSE;
    }

  /* free something? */

  *rval = JSVAL_VOID; /* return undefined */

 return JS_TRUE;
} /* jsinterp_wraptcmd */


/* jsinterp_vartraceproc:
 *  variable trace procedure,
 *  transport Tcl variable to JavaScript context
 */
char *
jsinterp_vartraceproc(ClientData clientData, Tcl_Interp *interp,
		      char *name1, char *name2, int flags)
{
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 JSObject *obj = NULL;
 jsval *newjsval, *objval;
 int create_object = AY_TRUE;

  if(!(newjsval = calloc(1, sizeof(jsval))))
    {
      goto cleanup;
    }

  toa = Tcl_NewStringObj(name1, -1);
  if(name2)
    {
      ton = Tcl_NewStringObj(name2, -1);
    }

  to = Tcl_ObjGetVar2(interp, toa, ton, flags);

  JS_AddRoot(jsinterp_cx, newjsval);

  if(jsinterp_objtoval(to, newjsval))
    {
      /* XXXX report error? */
      JS_RemoveRoot(jsinterp_cx, newjsval);
      goto cleanup;
    }

  if(name2)
    {
      /* array */
      if(!(objval = calloc(1, sizeof(jsval))))
	{
	  JS_RemoveRoot(jsinterp_cx, newjsval);
	  goto cleanup;
	}

      JS_AddRoot(jsinterp_cx, objval);

      if(JS_LookupProperty(jsinterp_cx, jsinterp_global, name1, objval))
	{
	  if(!JSVAL_IS_NULL(*objval) && !JSVAL_IS_VOID(*objval))
	    {
	      if(JSVAL_IS_OBJECT(*objval))
		{
		  obj = JSVAL_TO_OBJECT(*objval);
		  create_object = AY_FALSE;
		}
	    }
	} /* if */

      if(create_object)
	{
	  obj = JS_NewObject(jsinterp_cx, NULL, NULL, NULL);
	  *objval = OBJECT_TO_JSVAL(obj);
	  JS_SetProperty(jsinterp_cx, jsinterp_global, name1, objval);
	}

      JS_SetProperty(jsinterp_cx, obj, name2, newjsval);

      JS_RemoveRoot(jsinterp_cx, objval);

      if(!create_object)
	{
	  free(objval);
	}
    }
  else
    {
      /* scalar */
      JS_SetProperty(jsinterp_cx, jsinterp_global, name1, newjsval);
    }

  JS_RemoveRoot(jsinterp_cx, newjsval);
  newjsval = NULL;

cleanup:

  if(newjsval)
    free(newjsval);
  if(toa)
    {
      Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
    }
  if(ton)
    {
      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
    }

 return NULL;
} /* jsinterp_vartraceproc */


/* jsinterp_tclvar:
 *  JS function to establish an automatic connection
 *  between a Tcl var and a JS var
 */
int
jsinterp_tclvar(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
		jsval *rval)
{
 ClientData clientData = jsinterp_vartraceproc;
 uintN i = 0;
 char *vname;

  if(argc < 1)
    {
      JS_ReportError(cx, "insufficient arguments, need varname");
      return JS_FALSE;
    }

  for(i = 0; i < argc; i++)
    {
      if(JSVAL_IS_STRING(argv[i]))
	{
	  vname = JS_GetStringBytes(JSVAL_TO_STRING(argv[i]));

	  /* if there is not already a write trace... */
	  if(!Tcl_VarTraceInfo(jsinterp_interp, vname, TCL_TRACE_WRITES,
			       jsinterp_vartraceproc, NULL))
	    {
	      /* ...establish a write trace */
	      if(!TCL_OK == Tcl_TraceVar(jsinterp_interp,
					 vname,
					 TCL_TRACE_WRITES,
					 jsinterp_vartraceproc,
					 clientData))
		{
		  JS_ReportError(cx, "failed to establish trace");
		  return JS_FALSE;
		}
	    }
	}
    }

  *rval = JSVAL_VOID; /* return undefined */

 return JS_TRUE;
} /* jsinterp_tclvar */


/* jsinterp_jsvaltoobj:
 *  helper for jsinterp_tclset() below
 *  convert jsval to Tcl_Obj
 *  XXXX add support for unicode strings
 */
void
jsinterp_jsvaltoobj(jsval jv, Tcl_Obj **to)
{
 int ival;
 jsuint i, arrlen;
 jsdouble *dval;
 jsval arrelem, bval;
 JSString *jss;
 JSObject *jso;
 JSClass *jsc;
 Tcl_Obj *lto;

  if(JSVAL_IS_BOOLEAN(jv))
    {
      ival = JSVAL_TO_BOOLEAN(jv);
      *to = Tcl_NewBooleanObj(ival);
      return;
    }
  if(JSVAL_IS_INT(jv))
    {
      /*JS_ValueToECMAInt32(jsinterp_cx, jv, &ival);*/
      ival = JSVAL_TO_INT(jv);
      *to = Tcl_NewIntObj(ival);
      return;
    }
  if(JSVAL_IS_DOUBLE(jv))
    {
      dval = JSVAL_TO_DOUBLE(jv);
      *to = Tcl_NewDoubleObj((double)*dval);
      return;
    }
  if(JSVAL_IS_STRING(jv))
    {
      jss = JSVAL_TO_STRING(jv);
      *to = Tcl_NewStringObj(JS_GetStringBytes(jss), -1);
      return;
    }
  if(!JSVAL_IS_NULL(jv) && JSVAL_IS_OBJECT(jv))
    {
      jso = JSVAL_TO_OBJECT(jv);
      if(JS_IsArrayObject(jsinterp_cx, jso))
	{
	  if(JS_GetArrayLength(jsinterp_cx, jso, &arrlen))
	    {
	      *to = Tcl_NewListObj(0, NULL);
	      for(i = 0; i < arrlen; i++)
		{
		  if(JS_GetElement(jsinterp_cx, jso, i, &arrelem))
		    {
		      lto = NULL;
		      jsinterp_jsvaltoobj(arrelem, &lto);
		      if(lto)
			{
			  Tcl_ListObjAppendElement(NULL, *to, lto);
			}
		      else
			{
			  /* could not convert element! */
			  /* XXXX add proper error handling */
			  return;
			}
		    } /* if */
		} /* for */
	    } /* if */
	}
      else
	{
	  /* assume non-thread-safe build */
	  jsc = JS_GetClass(jso);
	  if(jsc && !strcmp(jsc->name, "Boolean"))
	    {
	      if(!JS_CallFunctionName(jsinterp_cx, jso, "valueOf",
				      0, NULL, &bval))
		return;
	      return jsinterp_jsvaltoobj(bval, to);
	    }
	} /* if is array or Boolean */
    } /* if */

 return;
} /* jsinterp_jsvaltoobj */


/* jsinterp_tclset:
 *  JS function to efficiently set Tcl vars from JS
 */
int
jsinterp_tclset(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
		jsval *rval)
{
 uintN i = 0;
 char *vname;
 Tcl_Obj *ton = NULL, *to = NULL;

  if(argc < 2)
    {
      JS_ReportError(cx, "insufficient arguments, need varname and value");
      return JS_FALSE;
    }

  for(i = 0; (i+1) < argc; i += 2)
    {
      if(JSVAL_IS_STRING(argv[i]))
	{
	  /* get variable name */
	  vname = JS_GetStringBytes(JSVAL_TO_STRING(argv[i]));
	  ton = Tcl_NewStringObj(vname, -1);

	  /* get variable value */
	  to = NULL;
	  jsinterp_jsvaltoobj(argv[i+1], &to);

	  /* set the variable */
	  if(to)
	    {
	      Tcl_ObjSetVar2(jsinterp_interp, ton, NULL, to,
			     TCL_LEAVE_ERR_MSG);
	    }
	  else
	    {
	      JS_ReportError(cx, "could not convert the value");
	      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
	      return JS_FALSE;
	    }

	  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
	} /* if */
    } /* for */

  *rval = JSVAL_VOID; /* return undefined */

 return JS_TRUE;
} /* jsinterp_tclset */


/* jsinterp_evaltcmd:
 *  Tcl command to evaluate a JS script
 */
int
jsinterp_evaltcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 jsval rval;
 JSBool ok = JS_TRUE;
 Tcl_Channel channel;
 Tcl_Obj *obj = NULL;
 unsigned char *bytes;
 int length;

  /* check args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "(script | -file filename)");
      return TCL_OK;
    }

  if(jsinterp_interp)
    {
      ay_error(AY_ERROR, argv[0], "concurrent/recursive evaluation");
      return TCL_OK;
    }

  jsinterp_interp = interp;

  /* evaluate */
  if((argv[1][0] == '-') && (argv[1][1] == 'f'))
    {
      /* -file */

      if(argc < 3)
	{
	  ay_error(AY_EARGS, argv[0], "(script | -file filename)");
	  jsinterp_interp = NULL;
	  return TCL_OK;
	}

      channel = Tcl_OpenFileChannel(interp, argv[2], "r", 0);
      if(channel)
	{
	  obj = Tcl_NewObj();
	  Tcl_ReadChars(channel, obj, -1, 0);
	  Tcl_Close(interp, channel);

	  bytes = Tcl_GetByteArrayFromObj(obj, &length);
	  ok = JS_EvaluateScript(jsinterp_cx, jsinterp_global,
				 (char*)bytes, length,
				 argv[2], 0, &rval);
	  Tcl_IncrRefCount(obj);Tcl_DecrRefCount(obj);
	}
    }
  else
    {
      /* interpret first argument as JS script */
      ok = JS_EvaluateScript(jsinterp_cx, jsinterp_global,
			     argv[1], strlen(argv[1]),
			     "argv[1]", 0, &rval);
      JS_ClearPendingException(jsinterp_cx);
    }

  if(!ok)
    {
      ay_error(AY_ERROR, argv[0], "Evaluation failed!");
    }

  jsinterp_interp = NULL;

 return TCL_OK;
} /* jsinterp_evaltcmd */


/* jsinterp_evalcb:
 *  Ayam Script object evaluation callback
 */
int
jsinterp_evalcb(Tcl_Interp *interp, char *script, int compile,
		Tcl_Obj **cscript)
{
 JSScript *binscript = NULL;
 JSObject *scriptObj = NULL;
 jsval result;
 char fname[] = "jsinterp_evalcb";

  /* compile or execute? */
  if(compile)
    {
      /* compile... */

      if(*cscript)
	{
	  /* there is already an outdated version of the compiled script
	     => manage associated memory */
	  JS_RemoveRoot(jsinterp_cx,
			(jsval*)Tcl_GetByteArrayFromObj(*cscript, NULL));

	  Tcl_DecrRefCount(*cscript);
	  *cscript = NULL;
	}

      if(!script)
	return TCL_OK;

      binscript = JS_CompileScript(jsinterp_cx, jsinterp_global,
				   script, strlen(script),
				   NULL, 1);
      if(binscript)
	{
	  scriptObj = JS_NewScriptObject(jsinterp_cx, binscript);
	  result = OBJECT_TO_JSVAL(scriptObj);
	  *cscript = Tcl_NewByteArrayObj((unsigned char*)&result,
					 sizeof(jsval));

	  /* Tcl_NewByteArrayObj() stored a _copy_ of the jsval, so that
	     we must protect the internal representation of the Tcl_Obj
	     from GC (not &result) */
	  JS_AddRoot(jsinterp_cx,
		     (jsval*)Tcl_GetByteArrayFromObj(*cscript, NULL));

	  Tcl_IncrRefCount(*cscript);
	}
      else
	{
	  /* compilation failed */
	  ay_error(AY_ERROR, fname, "compilation failed");
	  return TCL_ERROR;
	}
    }
  else
    {
      /* execute... */
      jsinterp_interp = interp;
      scriptObj =
	JSVAL_TO_OBJECT(*(jsval*)Tcl_GetByteArrayFromObj(*cscript, NULL));
      binscript = JS_GetPrivate(jsinterp_cx, scriptObj);
      if(!JS_ExecuteScript(jsinterp_cx, jsinterp_global, binscript,
			   &result))
	{
	  interp->errorLine = (int)jsinterp_errorline;
	  /* Starting with Tcl 8.6 we will need this instead: */
	  /*
	  Tcl_SetErrorLine(interp, (int)jsinterp_errorline);
	  */
	  jsinterp_interp = NULL;
	  return TCL_ERROR;
	}
      jsinterp_interp = NULL;
    } /* if */

  JS_MaybeGC(jsinterp_cx);
  /*JS_GC(jsinterp_cx);*/

 return TCL_OK;
} /* jsinterp_evalcb */


/* Jsinterp_Init:
 *  initialize JavaScript interpreter plugin
 *  note: this function _must_ be capitalized exactly this way
 *  regardless of the filename of the shared object (see: man n load)!
 */
#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int
Jsinterp_Init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;
 char fname[] = "jsinterp_init";

  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }

  /* first, check versions */
  if(ay_checkversion(fname, jsinterp_version_ma, jsinterp_version_mi))
    {
      return TCL_ERROR;
    }

  /* Create a JS runtime. */
  jsinterp_rt = JS_NewRuntime(8L * 1024L * 1024L);
  if(jsinterp_rt == NULL)
    {
      ay_error(AY_ERROR, fname, "Failed to create runtime.");
      return TCL_OK;
    }

  /* Create a context. */
  jsinterp_cx = JS_NewContext(jsinterp_rt, 8192);
  if(jsinterp_cx == NULL)
    {
      ay_error(AY_ERROR, fname, "Failed to create context.");
      return TCL_OK;
    }
  JS_SetOptions(jsinterp_cx, JSOPTION_VAROBJFIX);
  /*  JS_SetVersion(jsinterp_cx, JSVERSION_LATEST);*/
  JS_SetVersion(jsinterp_cx, JSVERSION_DEFAULT);
  JS_SetErrorReporter(jsinterp_cx, jsinterp_error);

  /* Create the global object. */
  jsinterp_global = JS_NewObject(jsinterp_cx, &jsinterp_global_class,
				 NULL, NULL);
  if(jsinterp_global == NULL)
    {
      ay_error(AY_ERROR, fname, "Failed to create global object.");
      return TCL_OK;
    }

  /* Populate the global object with the standard globals,
     like Object and Array. */
  if(!JS_InitStandardClasses(jsinterp_cx, jsinterp_global))
    {
      ay_error(AY_ERROR, fname, "Failed to init global object.");
      return TCL_OK;
    }

  /* Define new global JS functions that wrap Tcl/Ayam functionality. */
  if (!JS_DefineFunctions(jsinterp_cx, jsinterp_global,
			  jsinterp_global_functions))
    {
      ay_error(AY_ERROR, fname, "Failed to define global functions.");
      return TCL_OK;
    }

  /* Create Tcl commands. */
  Tcl_CreateCommand(interp, "jsEval", jsinterp_evaltcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* Register JavaScript language. */
  ay_status = ay_tcmd_registerlang("JavaScript", &jsinterp_langtype);
  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Failed to register language.");
      return TCL_OK;
    }

  ay_status = ay_table_additem(&ay_sevalcbt, (ay_voidfp)jsinterp_evalcb,
			       (int)jsinterp_langtype);
  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Failed to register evaluation callback.");
      return TCL_OK;
    }

  jsinterp_interp = NULL;

  /* Get object type struct pointers. */
  jsinterp_BooleanType = Tcl_GetObjType("boolean");
  jsinterp_ByteArrayType = Tcl_GetObjType("bytearray");
  jsinterp_DoubleType = Tcl_GetObjType("double");
  jsinterp_IntType = Tcl_GetObjType("int");
  jsinterp_LongType = Tcl_GetObjType("long");
  jsinterp_ListType = Tcl_GetObjType("list");
  jsinterp_StringType = Tcl_GetObjType("string");
  jsinterp_WideIntType = Tcl_GetObjType("wideInt");

  ay_error(AY_EOUTPUT, fname, "Plugin 'jsinterp' successfully loaded.");

 return TCL_OK;
} /* Jsinterp_Init */
