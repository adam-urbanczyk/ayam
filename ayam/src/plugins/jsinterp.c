/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2007 by Randolf Schultz
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

/** Tcl command to evaluate a JS script */
int jsinterp_evaltcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** JS function to wrap the crtOb command */
int jsinterp_wrapcrtobcmd(JSContext *cx, JSObject *obj,
			  uintN argc, jsval *argv,
			  jsval *rval);

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

/** JS function to link a Tcl var to a JS var */
int jsinterp_tclvar(JSContext *cx, JSObject *obj,
		    uintN argc, jsval *argv,
		    jsval *rval);


/* global variables: */

char jsinterp_version_ma[] = AY_VERSIONSTR;
char jsinterp_version_mi[] = AY_VERSIONSTRMI;

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

/** A bunch of pre-defined global functions (that wrap Ayam Tcl commands). */
static JSFunctionSpec jsinterp_global_functions[] = {
  {"tcleval", jsinterp_wrapevalcmd, 0, 0, 0},
  {"crtOb", jsinterp_wrapcrtobcmd, 0, 0, 0},
  {"delOb", jsinterp_wraptcmdargs, 0, 0, 0},
  {"cutOb", jsinterp_wraptcmdargs, 0, 0, 0},
  {"copOb", jsinterp_wraptcmdargs, 0, 0, 0},
  {"pasOb", jsinterp_wraptcmdargs, 0, 0, 0},
  {"pasmovOb", jsinterp_wraptcmdargs, 0, 0, 0},
  {"tclvar", jsinterp_tclvar, 0, 0, 0},
  {0}
};

/** Tcl interpreter */
static Tcl_Interp *jsinterp_interp;

/** Tcl_Obj types */
static Tcl_ObjType *jsinterp_BooleanType = NULL;
static Tcl_ObjType *jsinterp_ByteArrayType = NULL;
static Tcl_ObjType *jsinterp_DoubleType = NULL;
static Tcl_ObjType *jsinterp_IntType = NULL;
static Tcl_ObjType *jsinterp_ListType = NULL;
static Tcl_ObjType *jsinterp_StringType = NULL;
static Tcl_ObjType *jsinterp_WideIntType = NULL;


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

  sprintf(buf, "JS line %u", (unsigned int) report->lineno);
  ay_error(AY_ERROR, buf, message);

 return;
} /* jsinterp_error */


void jsinterp_unload()
{

    /* Cleanup. */
    JS_DestroyContext(jsinterp_cx);
    JS_DestroyRuntime(jsinterp_rt);
    JS_ShutDown();

 return;
}

/* jsinterp_convargs:
 *  helper to convert function/command arguments from
 *  JS objects to C/Tcl char*s
 */
int
jsinterp_convargs(JSContext *cx, uintN argc, jsval *argv,
		  char ***sargv)
{
 char *c, **nargv = NULL;
 uintN i;
 JSString *jss;

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
	  nargv[i+1] = JS_GetStringBytes(jss);

	  /* convert JS Array to Tcl list */
	  if(JSVAL_IS_OBJECT(argv[i]))
	    {
	      if(JS_IsArrayObject(cx, JSVAL_TO_OBJECT(argv[i])))
		{
		  c = nargv[i+1];
		  while(*c != '\0')
		    {
		      if(*c == ',')
			*c = ' ';
		      c++;
		    }
		} /* if */
	    } /* if */
	}
      else
	{
	  free(nargv);
	  return AY_ERROR;
	}
    } /* for */

  /* return result */
  *sargv = nargv;

 return AY_OK;
} /* jsinterp_convargs */


/* jsinterp_objtoval:
 *  helper function to convert a Tcl_Obj to a jsval
 */
int
jsinterp_objtoval(Tcl_Obj *to, jsval *v)
{
 char *sval;
 int ival;
 /* double dval;*/
 JSString *jss;
 JSObject *arr;
 jsint i;
 int objc;
 Tcl_Obj **objv;
 jsval elemv, arrv;

  if(to->typePtr == jsinterp_IntType)
    {
      if(JS_NewNumberValue(jsinterp_cx,
			   (double)to->internalRep.longValue, v))
	{
	  return AY_OK;
	}
    }
  else if (to->typePtr == jsinterp_DoubleType)
    {
      if(JS_NewNumberValue(jsinterp_cx,
			   (double)to->internalRep.doubleValue, v))
	{
	  return AY_OK;
	}
    }
    else if (to->typePtr == jsinterp_BooleanType)
      {
	if(Tcl_GetIntFromObj(NULL, to, &ival) != TCL_ERROR)
	  {
	    if(JS_NewNumberValue(jsinterp_cx, (double)ival, v))
	      {
		return AY_OK;
	      }
	  }
      }
  /*
  else if (to->typePtr == tclByteArrayTypePtr) {
    str = (char *) Tcl_GetByteArrayFromObj(to, &len);
    ...
  }
  */
  else if (to->typePtr == jsinterp_ListType)
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
	  if((jss = JS_NewStringCopyZ(jsinterp_cx, NULL)))
	    {
	      *v = STRING_TO_JSVAL(jss);
	      return AY_OK;
	    }
	}
    }
  else
    {
      if((sval = Tcl_GetString(to)) != NULL)
	{
	  if((jss = JS_NewStringCopyZ(jsinterp_cx, sval)))
	    {
	      *v = STRING_TO_JSVAL(jss);
	      return AY_OK;
	    }
	}
    }

#if 0
  if(Tcl_GetIntFromObj(NULL, to, &ival) != TCL_ERROR)
    {
      if(JS_NewNumberValue(jsinterp_cx, (double)ival, v))
	{
	  return AY_OK;
	}
    }
  else
    {
      if(Tcl_GetDoubleFromObj(NULL, to, &dval) != TCL_ERROR)
	{
	  if(JS_NewNumberValue(jsinterp_cx, dval, v))
	    {
	      return AY_OK;
	    }
	}
      else
	{
	  if((sval = Tcl_GetString(to)) != NULL)
	    {
	      if((jss = JS_NewStringCopyZ(jsinterp_cx, sval)))
		{
		  *v = STRING_TO_JSVAL(jss);
		  return AY_OK;
		}
	    }
	  /*
	  else
	    {

	    }*/ /* if */
	} /* if */
    } /* if */
#endif
 return AY_ERROR;
} /* jsinterp_objtoval */


/* jsinterp_wrapcrtobcmd:
 *  JS function to wrap the crtOb command
 */
int
jsinterp_wrapcrtobcmd(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
		      jsval *rval)
{
 int ay_status = AY_OK;
 ClientData clientData = {0};
 char **sargv;

  ay_status = jsinterp_convargs(cx, argc, argv, &sargv);

  if(ay_status)
    {
      JS_ReportError(cx, "argument conversion failed");
      return JS_FALSE;
    }

  ay_object_createtcmd(clientData, jsinterp_interp, argc+1, sargv);

  free(sargv);

  *rval = JSVAL_VOID; /* return undefined */

 return JS_TRUE;
} /* jsinterp_wrapcrtobcmd */


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
 int tcl_status = TCL_OK;
 char *script, *resstr;
 Tcl_Obj *resobj;

  if(argc < 1)
    {
      JS_ReportError(cx, "insufficient arguments, need script");
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
	  break;
	case TCL_RETURN:
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
		}
	    }
	  break;
	default:
	  /* caters for TCL_OK, TCL_BREAK, and TCL_CONTINUE */
	  break;
	} /* switch */
    }
  else
    {
      JS_ReportError(cx, "bad script");
    }

  *rval = JSVAL_VOID; /* return undefined */

 return JS_TRUE;
} /* jsinterp_wrapevalcmd */


/* jsinterp_wraptcmdargs:
 *  JS function to wrap a Tcl command that takes arguments
 */
int
jsinterp_wraptcmdargs(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
		      jsval *rval)
{
 int ay_status = AY_OK;
 ClientData clientData = {0};
 Tcl_CmdInfo cmdinfo = {0};
 char **sargv;

  ay_status = jsinterp_convargs(cx, argc, argv, &sargv);

  if(ay_status)
    {
      JS_ReportError(cx, "argument conversion failed");
      return JS_FALSE;
    }

  if(Tcl_GetCommandInfo(jsinterp_interp, sargv[0], &cmdinfo))
    {
      cmdinfo.proc(clientData, jsinterp_interp, argc+1, sargv);
    }
  else
    {
      /* command not found! */
      JS_ReportError(cx, "command not found");
      return JS_FALSE;
    }

  free(sargv);

  *rval = JSVAL_VOID; /* return undefined */

 return JS_TRUE;
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
      cmdinfo.proc(clientData, jsinterp_interp, 1, sargv);
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

  JS_AddRoot(jsinterp_cx, &newjsval);

  if(jsinterp_objtoval(to, newjsval))
    {
      /* XXXX report error? */
      goto cleanup;
    }

  if(name2)
    {
      /* array */
      if(!(objval = calloc(1, sizeof(jsval))))
	{
	  goto cleanup;
	}

      JS_AddRoot(jsinterp_cx, objval);

      if(JS_LookupProperty(jsinterp_cx, jsinterp_global, name1, objval))
	{
	  if(!JSVAL_IS_VOID(*objval))
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

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
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
 ClientData clientData = {0};
 uintN i = 0;
 char *vname;

  for(i = 0; i < argc; i++)
    {
      if(JSVAL_IS_STRING(argv[i]))
	{
	  vname = JS_GetStringBytes(JSVAL_TO_STRING(argv[i]));

	  /* establish a write trace */
	  if(!TCL_OK == Tcl_TraceVar(jsinterp_interp,
				     vname,
				     TCL_TRACE_WRITES,
				     jsinterp_vartraceproc, clientData))
	    {
	      JS_ReportError(cx, "failed to establish trace");
	      printf("BLEA!\n");
	    }
	}
    }

  *rval = JSVAL_VOID; /* return undefined */

 return JS_TRUE;
} /* jsinterp_tclvar */


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
  if(argv[1][0] == '-' && argv[1][1] == 'f')
    {
      if(argc < 3)
	{
	  ay_error(AY_EARGS, argv[0], "(script | -file filename)");
	  jsinterp_interp = NULL;
	  return TCL_OK;
	}
      /* -file */
      obj = Tcl_NewObj();
      channel = Tcl_OpenFileChannel(interp, argv[2], "r", 0);
      if(channel)
	{
	  Tcl_ReadChars(channel, obj, -1, 0);
	  Tcl_Close(interp, channel);
	}
      bytes = Tcl_GetByteArrayFromObj(obj, &length);
      ok = JS_EvaluateScript(jsinterp_cx, jsinterp_global,
			     (char*)bytes, length,
			     argv[2], 0, &rval);
      Tcl_IncrRefCount(obj);Tcl_DecrRefCount(obj);
    }
  else
    {
      /* interpret argument */
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
 char fname[] = "jsinterp_init";

  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }

  /* first, check versions */
  if(strcmp(ay_version_ma, jsinterp_version_ma))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "It is unsafe to continue! Bailing out...");
      return TCL_OK;
    }

  if(strcmp(ay_version_mi, jsinterp_version_mi))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "However, it is probably safe to continue...");
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

  jsinterp_interp = NULL;

  /* Get object type struct pointers. */
  jsinterp_BooleanType = Tcl_GetObjType("boolean");
  jsinterp_ByteArrayType = Tcl_GetObjType("bytearray");
  jsinterp_DoubleType = Tcl_GetObjType("double");
  jsinterp_IntType = Tcl_GetObjType("int");
  jsinterp_ListType = Tcl_GetObjType("list");
  jsinterp_StringType = Tcl_GetObjType("string");
  jsinterp_WideIntType = Tcl_GetObjType("wideInt");

  ay_error(AY_EOUTPUT, fname, "Plugin 'jsinterp' successfully loaded.");

 return TCL_OK;
} /* Jsinterp_Init */
