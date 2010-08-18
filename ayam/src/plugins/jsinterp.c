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

void jsinterp_error(JSContext *cx, const char *message, JSErrorReport *report);

int jsinterp_evaltcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

int jsinterp_wrapcrtobcmd(JSContext *cx, JSObject *obj, uintN argc,
			  jsval *argv, jsval *rval);

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
static JSFunctionSpec jsinterp_globalfunctions[] = {
  {"crtOb", jsinterp_wrapcrtobcmd, 0, 0, 0},
  {0}
};

/** Tcl interpreter */
static Tcl_Interp *jsinterp_interp;


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
 char **nargv = NULL;
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
	}
      else
	{
	  return AY_ERROR;
	}
    } /* for */

  /* return result */
  *sargv = nargv;

 return AY_OK;
} /* jsinterp_convargs */

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

  *rval = JSVAL_VOID; /* return undefined */

 return JS_TRUE;
} /* jsinterp_wrapcrtobcmd */


/* jsinterp_evaltcmd:
 *  Tcl command to evaluate a JS script
 */
int
jsinterp_evaltcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 jsval rval;
 JSBool ok = JS_TRUE;

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
      /* -file */
      /* XXXX todo: read and eval .js file */
    }
  else
    {
      /* interpret argument */
      ok = JS_EvaluateScript(jsinterp_cx, jsinterp_global,
			     argv[1], strlen(argv[1]),
			     "argv[1]", 0, &rval);
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
      ay_error(AY_ERROR, fname, "Failed to init.");
      return TCL_OK;
    }

  if (!JS_DefineFunctions(jsinterp_cx, jsinterp_global,
			  jsinterp_globalfunctions))
    {
      ay_error(AY_ERROR, fname, "Failed to define functions.");
      return TCL_OK;
    }

  /* create commands */

  Tcl_CreateCommand(interp, "jsEval", jsinterp_evaltcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  jsinterp_interp = NULL;

  ay_error(AY_EOUTPUT, fname, "Plugin 'jsinterp' successfully loaded.");

 return TCL_OK;
} /* Jsinterp_Init */
