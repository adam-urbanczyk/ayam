/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2015 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

/** \file luainterp.c Ayam Lua interpreter plugin */

/* Ayam includes: */
#include "ayam.h"

/* Lua includes: */
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


/* type definitions local to this module: */

typedef struct luainterp_cscript_t
{
  size_t size;
  char *data;
} luainterp_cscript;


/* prototypes of functions local to this module: */

/** error reporting wrapper function */
void luainterp_error();

/** helper function to convert a Tcl_Obj to a Lua val */
int luainterp_pushobj(Tcl_Obj *to);

/** Lua function to wrap a Tcl command */
int luainterp_wraptclcmd(lua_State *L);

/** Lua function to wrap the Tcl eval command */
int luainterp_wrapevalcmd(lua_State *L);

/** variable trace procedure to transport a Tcl variable to Lua */
char *luainterp_vartraceproc(ClientData clientData, Tcl_Interp *interp,
			     char *name1, char *name2, int flags);

/** Lua function to link a Tcl var to a Lua var */
int luainterp_tclvar(lua_State *L);

/** Lua function to set a Tcl var from Lua */
int luainterp_tclset(lua_State *L);

/** Tcl command to evaluate a Lua script */
int luainterp_evaltcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/** Ayam Script object evaluation callback */
int luainterp_evalcb(Tcl_Interp *interp, char *script, int compile,
		     Tcl_Obj **cscript);


/* global variables: */

/** major Ayam version we are compiled with */
char luainterp_version_ma[] = AY_VERSIONSTR;
/** minor Ayam version we are compiled with */
char luainterp_version_mi[] = AY_VERSIONSTRMI;

/** language id */
char *luainterp_langid = NULL;

static lua_State *L = NULL;


/** A bunch of pre-defined global functions,
    that wrap Ayam Tcl commands (e.g. crtOb()),
    evaluate Tcl scripts (tcleval()),
    manage variable connections (tclvar()), and
    set Tcl variables from Lua (tclset()). */

static const struct luaL_Reg luainterplib [] = {
      {"crtOb", luainterp_wraptclcmd},
      {"delOb", luainterp_wraptclcmd},
      {"cutOb", luainterp_wraptclcmd},
      {"copOb", luainterp_wraptclcmd},
      {"pasOb", luainterp_wraptclcmd},
      {"pasmovOb", luainterp_wraptclcmd},
      {"repOb", luainterp_wraptclcmd},
      {"clearClip", luainterp_wraptclcmd},
      {"convOb", luainterp_wraptclcmd},
      {"notifyOb", luainterp_wraptclcmd},
      {"goTop", luainterp_wraptclcmd},
      {"goUp", luainterp_wraptclcmd},
      {"goDown", luainterp_wraptclcmd},
      {"getLevel", luainterp_wraptclcmd},
      {"resolveIn", luainterp_wraptclcmd},
      {"nameOb", luainterp_wraptclcmd},
      {"showOb", luainterp_wraptclcmd},
      {"hideOb", luainterp_wraptclcmd},

      {"mergePo", luainterp_wraptclcmd},
      {"optiPo", luainterp_wraptclcmd},
      {"splitPo", luainterp_wraptclcmd},
      {"genfnPo", luainterp_wraptclcmd},
      {"gensnPo", luainterp_wraptclcmd},

      {"setProp", luainterp_wraptclcmd},
      {"getProp", luainterp_wraptclcmd},
      {"setTrafo", luainterp_wraptclcmd},
      {"getTrafo", luainterp_wraptclcmd},
      {"setAttr", luainterp_wraptclcmd},
      {"getAttr", luainterp_wraptclcmd},
      {"setMat", luainterp_wraptclcmd},
      {"getMat", luainterp_wraptclcmd},
      {"shaderSet", luainterp_wraptclcmd},
      {"shaderGet", luainterp_wraptclcmd},
      {"selOb", luainterp_wraptclcmd},
      {"getSel", luainterp_wraptclcmd},
      {"sL", luainterp_wraptclcmd},
      {"setTags", luainterp_wraptclcmd},
      {"addTag", luainterp_wraptclcmd},
      {"getTags", luainterp_wraptclcmd},
      {"getTag", luainterp_wraptclcmd},
      {"hasTag", luainterp_wraptclcmd},
      {"delTags", luainterp_wraptclcmd},

      {"getPnt", luainterp_wraptclcmd},
      {"setPnt", luainterp_wraptclcmd},
      {"delegTrafo", luainterp_wraptclcmd},
      {"movOb", luainterp_wraptclcmd},
      {"movPnts", luainterp_wraptclcmd},
      {"scalOb", luainterp_wraptclcmd},
      {"scalPnts", luainterp_wraptclcmd},
      {"rotOb", luainterp_wraptclcmd},
      {"rotPnts", luainterp_wraptclcmd},
      {"selPnts", luainterp_wraptclcmd},
      {"invPnts", luainterp_wraptclcmd},
      {"applyTrafo", luainterp_wraptclcmd},
      {"centerPnts", luainterp_wraptclcmd},

      {"revertC", luainterp_wraptclcmd},

      {"refineC", luainterp_wraptclcmd},
      {"refineknNC", luainterp_wraptclcmd},
      {"coarsenNC", luainterp_wraptclcmd},
      {"clampNC", luainterp_wraptclcmd},
      {"unclampNC", luainterp_wraptclcmd},
      {"elevateNC", luainterp_wraptclcmd},
      {"insknNC", luainterp_wraptclcmd},
      {"remknNC", luainterp_wraptclcmd},
      /* collMP explMP ? */
      {"splitNC", luainterp_wraptclcmd},
      {"concatNC", luainterp_wraptclcmd},
      {"crtNCircle", luainterp_wraptclcmd},
      {"crtNRect", luainterp_wraptclcmd},
      {"crtTrimRect", luainterp_wraptclcmd},
      {"crtClosedBS", luainterp_wraptclcmd},
      {"rescaleknNC", luainterp_wraptclcmd},
      /* curvPlot ? */
      {"shiftC", luainterp_wraptclcmd},
      {"toXYNC", luainterp_wraptclcmd},
      {"toXYC", luainterp_wraptclcmd},
      {"makeCompNC", luainterp_wraptclcmd},
      {"centerNC", luainterp_wraptclcmd},
      {"trimNC", luainterp_wraptclcmd},
      {"estlenNC", luainterp_wraptclcmd},
      {"reparamNC", luainterp_wraptclcmd},
      {"extendNC", luainterp_wraptclcmd},

      {"crtNSphere", luainterp_wraptclcmd},
      {"crtNSphere2", luainterp_wraptclcmd},
      {"breakNP", luainterp_wraptclcmd},
      {"buildNP", luainterp_wraptclcmd},
      /* topoly? */
      {"elevateuNP", luainterp_wraptclcmd},
      {"elevatevNP", luainterp_wraptclcmd},
      {"refineuNP", luainterp_wraptclcmd},
      {"refinevNP", luainterp_wraptclcmd},
      {"swapuvS", luainterp_wraptclcmd},
      {"revertuS", luainterp_wraptclcmd},
      {"revertvS", luainterp_wraptclcmd},
      {"closeuNP", luainterp_wraptclcmd},
      {"closevNP", luainterp_wraptclcmd},
      {"clampuNP", luainterp_wraptclcmd},
      {"clampvNP", luainterp_wraptclcmd},
      {"unclampuNP", luainterp_wraptclcmd},
      {"unclampvNP", luainterp_wraptclcmd},
      {"rescaleknNP", luainterp_wraptclcmd},
      {"insknuNP", luainterp_wraptclcmd},
      {"insknvNP", luainterp_wraptclcmd},
      {"splituNP", luainterp_wraptclcmd},
      {"splitvNP", luainterp_wraptclcmd},
      {"remknuNP", luainterp_wraptclcmd},
      {"remknvNP", luainterp_wraptclcmd},
      {"extrNP", luainterp_wraptclcmd},
      {"concatS", luainterp_wraptclcmd},
      {"tobasisPM", luainterp_wraptclcmd},

      {"tcleval", luainterp_wrapevalcmd},
      {"tclset", luainterp_tclset},
      {"tclvar", luainterp_tclvar},
      {NULL, NULL}  /* sentinel */
}; /* luainterplib */

/** current Tcl interpreter */
static Tcl_Interp *luainterp_interp;

/** \name Tcl_Obj types (for data conversion in luainterp_pushobj()) */
/*@{*/
static Tcl_ObjType *luainterp_BooleanType = NULL;
static Tcl_ObjType *luainterp_ByteArrayType = NULL;
static Tcl_ObjType *luainterp_DoubleType = NULL;
static Tcl_ObjType *luainterp_IntType = NULL;
static Tcl_ObjType *luainterp_ListType = NULL;
static Tcl_ObjType *luainterp_StringType = NULL;
static Tcl_ObjType *luainterp_WideIntType = NULL;
/*@}*/

/** current error line */
static unsigned int luainterp_errorline;

/* functions: */

/* luainterp_error:
 *  error reporting wrapper function
 */
void
luainterp_error()
{
 char fname[] = "luainterp_error";
 char buf[128] = "";
 char *bracket;
 const char *errmsg;

  errmsg = lua_tostring(L, -1);

  if(errmsg)
    {
      if((bracket = strchr(errmsg, ']')))
	{
	  sscanf(bracket, "]:%u:", &luainterp_errorline);
	  sprintf(buf, "Lua line %u", luainterp_errorline);
	  bracket += 2;
	  bracket = strchr(bracket, ':');
	  bracket += 2;
	  if(bracket)
	    ay_error(AY_ERROR, buf, bracket);
	  else
	    ay_error(AY_ERROR, buf, errmsg);
	}
      else
	{
	  ay_error(AY_ERROR, fname, errmsg);
	}
    }
  else
    {
      ay_error(AY_ERROR, fname, "No error message found!");

      luainterp_errorline = 0;
    }

 return;
} /* luainterp_error */


/* luainterp_unload:
 *  unused, there is no way to shut down the plugin
 */
void
luainterp_unload()
{
  lua_close(L);
 return;
} /* luainterp_unload */


/* luainterp_pushobj:
 *  helper function to convert a Tcl_Obj to a Lua value;
 *  the converted value is pushed on the stack
 */
int
luainterp_pushobj(Tcl_Obj *to)
{
 char *sval;
 int i, objc;
 Tcl_Obj **objv;
 lua_Number num;

  if(to->typePtr == luainterp_IntType)
    {
      num = (double)to->internalRep.longValue;
      lua_pushnumber(L, num);
      return AY_OK;
    }
  else if (to->typePtr == luainterp_DoubleType)
    {
      num = to->internalRep.doubleValue;
      lua_pushnumber(L, num);
      return AY_OK;
    }
  else if (to->typePtr == luainterp_BooleanType)
    {
      lua_pushboolean(L, (int)to->internalRep.longValue);
      return AY_OK;
    }
  /*
  else if (to->typePtr == tclByteArrayTypePtr) {
    str = (char *) Tcl_GetByteArrayFromObj(to, &len);
    ...
  }
  */
  else if (to->typePtr == luainterp_ListType)
    {
      Tcl_ListObjGetElements(NULL, to, &objc, &objv);
      if(objc)
	{
	  lua_newtable (L);

	  /* now convert/add the array elements */
	  for(i = 0; i < objc; i++)
	    {
	      if(!luainterp_pushobj(objv[i]))
		{
		  lua_rawseti(L, -2, i);
		}
	      else
		{
		  return AY_ERROR;
		}
	    } /* for */
	  return AY_OK;
	} /* if */
    }
  else
    {
      if((sval = Tcl_GetString(to)) != NULL)
	{
	  lua_pushstring(L, sval);
	  return AY_OK;
	}
    } /* if */

 return AY_ERROR;
} /* luainterp_pushobj */


/* luainterp_wrapevalcmd:
 *  Lua function to wrap the Tcl eval command
 *  we can not use luainterp_wraptclcmd() because the Tcl_CmdInfo proc
 *  of "eval" points to Tcl_CommandObjV which we can not use with plain
 *  argc/argv-args; furthermore, we need to transfer the result of the
 *  evaluation back to Lua
 */
int
luainterp_wrapevalcmd(lua_State *L)
{
 int tcl_status = TCL_OK;
 int nargs;
 char *resstr;
 Tcl_Obj *resobj;
 const char *script;

  nargs = lua_gettop(L);

  if(nargs < 1)
    {
      return luaL_error(L, "insufficient arguments, need script");
    }

  if(!lua_isstring(L, -1))
    {
      return luaL_error(L, "script must be a string");
    }
  script = lua_tostring( L, -1);

  tcl_status = Tcl_EvalEx(luainterp_interp, script, -1, TCL_EVAL_GLOBAL);
  switch(tcl_status)
    {
    case TCL_ERROR:
      return luaL_error(L, "Tcl script failed");
      break;
    default:
      /* caters for TCL_RETURN, TCL_OK, TCL_BREAK, and TCL_CONTINUE */
      resobj = Tcl_GetObjResult(luainterp_interp);
      resstr = Tcl_GetString(resobj);
      if(resstr && (resstr[0] != '\0'))
	{
	  if(!luainterp_pushobj(resobj))
	    {
	      return 1;
	    }
	  else
	    {
	       return luaL_error(L, "failed to convert script result");
	    }
	} /* if */
      break;
    } /* switch */

 return 0;
} /* luainterp_wrapevalcmd */


/* luainterp_convtableds:
 *  helper function to convert a Lua table (array) to a Tcl dynamic string
 */
int
luainterp_convtableds(lua_State *L, int i, char **result)
{
 int ay_status = AY_OK;
 int k = 1;
 char *valstr;
 Tcl_DString ds;
 const char *tstr;

  /* convert table (array) to Tcl list */
  Tcl_DStringInit(&ds);

  lua_rawgeti(L, i, k);

  while(1)
    {
      switch(lua_type(L, -1))
	{
	case LUA_TNUMBER:
	case LUA_TBOOLEAN:
	case LUA_TSTRING:
	  tstr = lua_tostring(L, -1);
	  Tcl_DStringAppend(&ds, tstr, -1);
	  Tcl_DStringAppend(&ds, " ", 1);
	  break;
	case LUA_TTABLE:
	  Tcl_DStringAppend(&ds, "{", 1);
	  ay_status = luainterp_convtableds(L, -1, &valstr);
	  if(ay_status)
	    goto cleanup;
	  Tcl_DStringAppend(&ds, valstr, -1);
	  free(valstr);
	  Tcl_DStringAppend(&ds, "}", 1);
	  Tcl_DStringAppend(&ds, " ", 1);
	  break;
	default:
	  break;
	} /* switch */

      /* get next array elem */
      lua_pop(L, 1);
      k++;
      lua_rawgeti(L, i, k);
      if(lua_isnil(L, -1))
	break;
    } /* while */

  if(!(*result = calloc(Tcl_DStringLength(&ds)+1, sizeof(char))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }
  strcpy(*result, Tcl_DStringValue(&ds));

cleanup:

  Tcl_DStringFree(&ds);

  lua_pop(L, 1);

 return ay_status;
} /* luainterp_convtableds */


/* luainterp_convtableobj:
 *  helper function to convert a Lua table (array) to a Tcl list object
 */
int
luainterp_convtableobj(lua_State *L, int i, Tcl_Obj **result)
{
 int ay_status = AY_OK;
 int k = 1;
 Tcl_Obj *to;
 const char *val;

  /* convert table (array) to Tcl list obj */

  *result = Tcl_NewListObj(0, NULL);

  lua_rawgeti(L, i, k);

  while(1)
    {
      to = NULL;
      switch(lua_type(L, -1))
	{
	case LUA_TBOOLEAN:
	  to = Tcl_NewBooleanObj(lua_toboolean(L, -1));
	case LUA_TNUMBER:
	  to = Tcl_NewDoubleObj((double) lua_tonumber(L, -1));
	case LUA_TSTRING:
	  val = lua_tostring(L, -1);
	  to = Tcl_NewStringObj(val, -1);
	  break;
	case LUA_TTABLE:
	  ay_status = luainterp_convtableobj(L, -1, &to);
	  if(ay_status)
	    goto cleanup;
	  break;
	default:
	  break;
	} /* switch */

      if(to)
	{
	  Tcl_ListObjAppendElement(NULL, *result, to);
	}

      /* get next array elem */
      lua_pop(L, 1);
      k++;
      lua_rawgeti(L, i, k);
      if(lua_isnil(L, -1))
	break;
    } /* while */

cleanup:

  lua_pop(L, 1);

 return ay_status;
} /* luainterp_convtableobj */


/* luainterp_wraptclcmd:
 *  Lua function to wrap a Tcl command in a generic way
 */
int
luainterp_wraptclcmd(lua_State *L)
{
 int ay_status = AY_OK;
 char **argv = NULL;
 ClientData clientData = {0};
 Tcl_CmdInfo cmdinfo = {0};
 lua_Debug ar;
 int nargs, argi = 1;
 int i = 1;
 const char *val;

  /* get number of arguments */
  nargs = lua_gettop(L);

  /* construct argv array */
  if(!(argv = calloc(nargs+2, sizeof(char*))))
    return luaL_error(L, "out of memory");

  for(i = 1; i <= nargs; i++)
    {
      switch(lua_type(L, i))
	{
	case LUA_TNUMBER:
	case LUA_TBOOLEAN:
	case LUA_TSTRING:
	  val = lua_tostring(L, i);

	  if(!(argv[argi] = calloc(strlen(val)+1, sizeof(char))))
	    {
	      ay_status = AY_EOMEM;
	      goto cleanup;
	    }
	  strcpy(argv[argi], val);

	  argi++;
	  break;
	case LUA_TTABLE:
	  ay_status = luainterp_convtableds(L, i, &(argv[argi]));
	  if(ay_status)
	    goto cleanup;
	  argi++;
	  break;
	default:
	  break;
	} /* switch */
    } /* for */

  /* get function name */
  lua_getstack (L, 0, &ar);
  lua_getinfo(L, "n", &ar);

  if(!(argv[0] = calloc(strlen(ar.name)+1, sizeof(char))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }
  strcpy(argv[0], ar.name);

  /* call into Tcl */
  if(Tcl_GetCommandInfo(luainterp_interp, argv[0], &cmdinfo))
    {
      if(!cmdinfo.isNativeObjectProc)
	{
	  cmdinfo.proc(clientData, luainterp_interp, argi, argv);
	}
      else
	{
	  /* internal error, should never happen, no cleanup */
	  return luaL_error(L, "unsupported command type");
	}
    }
  else
    {
      /* internal error, should never happen, no cleanup */
      return luaL_error(L, "command not found");
    } /* if */

cleanup:

  for(i = 0; i <= nargs; i++)
    {
      if(argv[i])
	free(argv[i]);
    }

  free(argv);

  if(ay_status == AY_EOMEM)
    {
      return luaL_error(L, "out of memory");
    }

 return 0;
} /* luainterp_wraptclcmd */


/* luainterp_vartraceproc:
 *  variable trace procedure,
 *  transport Tcl variable to Lua context
 */
char *
luainterp_vartraceproc(ClientData clientData, Tcl_Interp *interp,
		       char *name1, char *name2, int flags)
{
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;

  toa = Tcl_NewStringObj(name1, -1);
  if(name2)
    {
      ton = Tcl_NewStringObj(name2, -1);
    }

  to = Tcl_ObjGetVar2(interp, toa, ton, flags);

  if(luainterp_pushobj(to))
    {
      /* XXXX report error? */
      goto cleanup;
    }

  if(name2)
    {
      /* Tcl array => Lua table */
      lua_getglobal(L, name1);
      if(lua_isnil(L, -1) || !lua_istable(L, -1))
	{
	  /* table does not exist (yet) => create it */
	  lua_pop(L, 1);
	  lua_newtable(L);
	}
      /* swap value and table on the stack */
      lua_insert(L, -2);

      lua_setfield(L, -2, name2);
    } /* if */

  lua_setglobal(L, name1);

cleanup:

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  if(ton)
    {
      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
    }

 return NULL;
} /* luainterp_vartraceproc */


/* luainterp_tclvar:
 *  Lua function to establish an automatic connection
 *  between a Tcl variable and a Lua variable
 */
int
luainterp_tclvar(lua_State *L)
{
 ClientData clientData = luainterp_vartraceproc;
 int nargs, i = 0;
 const char *vname;

  nargs = lua_gettop(L);

  if(nargs < 1)
    {
      return luaL_error(L, "insufficient arguments, need varname");
    }

  for(i = 1; i <= nargs; i++)
    {
      if(!lua_isstring(L, i))
	{
	  return luaL_error(L, "varname must be a string");
	}
      vname = lua_tostring(L, i);

      /* if there is not already a write trace... */
      if(!Tcl_VarTraceInfo(luainterp_interp, vname, TCL_TRACE_WRITES,
			   luainterp_vartraceproc, NULL))
	{
	  /* ...establish a write trace */
	  if(!TCL_OK == Tcl_TraceVar(luainterp_interp,
				     vname,
				     TCL_TRACE_WRITES,
				     luainterp_vartraceproc,
				     clientData))
	    {
	       return luaL_error(L, "failed to establish write trace");
	    }
	} /* if */
    } /* for */

 return 0;
} /* luainterp_tclvar */


/* luainterp_tclset:
 *  Lua function to efficiently set Tcl vars from Lua
 */
int
luainterp_tclset(lua_State *L)
{
 int ay_status = AY_OK;
 int nargs, i = 0;
 const char *vname, *val;
 Tcl_Obj *ton = NULL, *to = NULL;

  nargs = lua_gettop(L);

  if(nargs < 2)
    {
      return luaL_error(L, "insufficient arguments, need varname value");
    }

  for(i = 1; i+1 <= nargs; i += 2)
    {
      if(!lua_isstring(L, i))
	{
	  return luaL_error(L, "varname must be a string");
	}
      vname = lua_tostring(L, i);

      ton = Tcl_NewStringObj(vname, -1);

      /* get variable value */
      to = NULL;
      switch(lua_type(L, i+1))
	{
	case LUA_TBOOLEAN:
	  to = Tcl_NewBooleanObj(lua_toboolean(L, i+1));
	case LUA_TNUMBER:
	  to = Tcl_NewDoubleObj((double) lua_tonumber(L, i+1));
	case LUA_TSTRING:
	  val = lua_tostring(L, i+1);
	  to = Tcl_NewStringObj(val, -1);
	  break;
	case LUA_TTABLE:
	  ay_status = luainterp_convtableobj(L, i+1, &to);
	  if(ay_status)
	    {
	      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
	      goto cleanup;
	    }
	  break;
	default:
	  break;
	} /* switch */

      /* set the variable */
      if(to)
	{
	  Tcl_ObjSetVar2(luainterp_interp, ton, NULL, to,
			 TCL_LEAVE_ERR_MSG);
	}

      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
    } /* for */

cleanup:

 return 0;
} /* luainterp_tclset */


/* luainterp_evaltcmd:
 *  Tcl command to evaluate a Lua script
 */
int
luainterp_evaltcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int lua_status = 0;
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

  if(luainterp_interp)
    {
      ay_error(AY_ERROR, argv[0], "concurrent/recursive evaluation");
      return TCL_OK;
    }

  luainterp_interp = interp;

  /* evaluate */
  if((argv[1][0] == '-') && (argv[1][1] == 'f'))
    {
      /* -file */

      if(argc < 3)
	{
	  ay_error(AY_EARGS, argv[0], "(script | -file filename)");
	  luainterp_interp = NULL;
	  return TCL_OK;
	}

      channel = Tcl_OpenFileChannel(interp, argv[2], "r", 0);
      if(channel)
	{
	  obj = Tcl_NewObj();
	  Tcl_ReadChars(channel, obj, -1, 0);
	  Tcl_Close(interp, channel);

	  bytes = Tcl_GetByteArrayFromObj(obj, &length);
	  if(length > 0)
	    {
	      lua_status = luaL_loadstring(L, (const char *)bytes);
	      if(lua_status)
		{
		  luainterp_error();
		  goto cleanup;
		}
	    }
	  else
	    goto cleanup;
	}
      else
	{
	  ay_error(AY_EOPENFILE, argv[0], argv[2]);
	  goto cleanup;
	}
    }
  else
    {
      /* interpret first argument as Lua script */
      lua_status = luaL_loadstring(L, argv[1]);
      if(lua_status)
	{
	  luainterp_error();
	  goto cleanup;
	}
    }

  lua_status = lua_pcall(L, 0, 0, 0);
  if(lua_status)
    {
      luainterp_error();
    }

cleanup:

  if(obj)
    {
      Tcl_IncrRefCount(obj);Tcl_DecrRefCount(obj);
    }

  luainterp_interp = NULL;

 return TCL_OK;
} /* luainterp_evaltcmd */


/*
lua_Writer
*/
int
luainterp_savecompiledchunk(lua_State *L, const void* p,
			    size_t sz, void* ud)
{
 void *tmpdata;
 luainterp_cscript *cscript;

  if(L == NULL || p == NULL || ud == NULL)
    return 1;

  cscript = (luainterp_cscript*)ud;

  if(!(tmpdata = realloc(cscript->data, sz+cscript->size)))
    {
      return 1;
    }
  else
    {
      cscript->data = tmpdata;
      memcpy(&(cscript->data[cscript->size]), p, sz);
      cscript->size += sz;
    }

 return 0;
} /* luainterp_savecompiledchunk */


/*
lua_Reader
*/
const char *
luainterp_readcompiledchunk(lua_State *L,
			    void *data,
			    size_t *size)
{
 luainterp_cscript  *cscript;

  cscript = (luainterp_cscript*)data;

  if(cscript->size > 0)
    {
      *size = cscript->size;
      /* remember that we passed along all data for
	 the next call...  */
      cscript->size = 0;
      return cscript->data;
    }

 return NULL;
} /* luainterp_readcompiledchunk */


/* luainterp_evalcb:
 *  Ayam Script object evaluation callback
 */
int
luainterp_evalcb(Tcl_Interp *interp, char *script, int compile,
		 Tcl_Obj **cscriptobj)
{
 int lua_status = 0;
 char fname[] = "luainterp_evalcb";
 luainterp_cscript cscript = {0};

  /* compile or execute? */
  if(compile)
    {
      /* compile... */
      if(*cscriptobj)
	{
	  /* there is already an outdated version of the compiled script
	     => manage associated memory */
	  Tcl_DecrRefCount(*cscriptobj);
	  *cscriptobj = NULL;
	}

      if(!script)
	return TCL_OK;

      lua_status = luaL_loadstring(L, script);
      if(lua_status)
	{
	  /* compilation failed */
	  luainterp_error();
	  interp->errorLine = (int)luainterp_errorline;
	  return TCL_ERROR;
	}

      lua_status = lua_dump(L, &luainterp_savecompiledchunk, &cscript);
      if(lua_status)
	{
	  luainterp_error();
	  interp->errorLine = (int)luainterp_errorline;
	  return TCL_ERROR;
	}

      if(cscript.size > 0)
	{
	  *cscriptobj = Tcl_NewByteArrayObj((unsigned char*)cscript.data,
					    cscript.size);

	  free(cscript.data);

	  Tcl_IncrRefCount(*cscriptobj);
	}
      else
	{
	  ay_error(AY_ERROR, fname, NULL);
	  return TCL_ERROR;
	}
    }
  else
    {
      /* execute... */
      if(*cscriptobj)
	{
	  luainterp_interp = interp;
	  cscript.data = (char*)Tcl_GetByteArrayFromObj(*cscriptobj,
						 (int*)&(cscript.size));

	  lua_status = lua_load(L, &luainterp_readcompiledchunk, &cscript,
				"script", "b");

	  if(lua_status)
	    {
	      luainterp_interp = NULL;
	      luainterp_error();
	      interp->errorLine = (int)luainterp_errorline;
	      return TCL_ERROR;
	    }

	  lua_status = lua_pcall(L, 0, 0, 0);
	  if(lua_status)
	    {
	      luainterp_interp = NULL;
	      luainterp_error();
	      interp->errorLine = (int)luainterp_errorline;
	      return TCL_ERROR;
	    }

	  luainterp_interp = NULL;
	} /* if */
    } /* if */

 return TCL_OK;
} /* luainterp_evalcb */


/* Luainterp_Init:
 *  initialize Lua interpreter plugin
 *  note: this function _must_ be capitalized exactly this way
 *  regardless of the filename of the shared object (see: man n load)!
 */
#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int
Luainterp_Init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;
 char fname[] = "luainterp_init";
 int i = 0;

  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }

  /* first, check versions */
  if(ay_checkversion(fname, luainterp_version_ma, luainterp_version_mi))
    {
      return TCL_ERROR;
    }

  /* Create a Lua runtime. */
  L = luaL_newstate();
  luaopen_base(L);
  luaopen_bit32(L);
  luaopen_debug(L);
  luaopen_string(L);
  luaopen_math(L);
  luaopen_table(L);

  /* Create Tcl commands. */
  Tcl_CreateCommand(interp, "luaEval", luainterp_evaltcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* Register Lua language. */
  ay_status = ay_tcmd_registerlang("Lua", &luainterp_langid);
  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Failed to register language.");
      return TCL_OK;
    }

  ay_status = ay_table_additem(&ay_sevalcbt, (ay_voidfp)luainterp_evalcb,
			       (int)luainterp_langid);
  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Failed to register evaluation callback.");
      return TCL_OK;
    }

  /* Register all Lua functions. */
  while(1)
    {
      if(luainterplib[i].name != NULL)
	{
	  lua_register (L, luainterplib[i].name,  luainterplib[i].func);
	  i++;
	}
      else
	break;
    }

  luainterp_interp = NULL;

  /* Get object type struct pointers. */
  luainterp_BooleanType = Tcl_GetObjType("boolean");
  luainterp_ByteArrayType = Tcl_GetObjType("bytearray");
  luainterp_DoubleType = Tcl_GetObjType("double");
  luainterp_IntType = Tcl_GetObjType("int");
  luainterp_ListType = Tcl_GetObjType("list");
  luainterp_StringType = Tcl_GetObjType("string");
  luainterp_WideIntType = Tcl_GetObjType("wideInt");

  ay_error(AY_EOUTPUT, fname, "Plugin 'luainterp' successfully loaded.");

 return TCL_OK;
} /* Luainterp_Init */
