/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2006 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

/* aysdr.c - Plugin to scan shaders compiled with sdrc (Pixie)
   using libsdr */

/* includes: */

#include "tcl.h"
#include "errcode.h"
#include "sdr.h"
#include <string.h>

/* prototypes: */

int aysdr_scansdrsarg(TSdrParameter *param, Tcl_DString *ds);

int aysdr_scansdrtcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

extern void ay_error(int code, char *where, char *what);

#ifdef WIN32
  __declspec( dllexport ) int Aysdr_Init(Tcl_Interp *interp);
#else
  int Aysdr_Init(Tcl_Interp *interp);
#endif

extern Tcl_Interp *ay_plugin_interp;
Tcl_Interp *ay_plugin_interp;


/* functions: */

/* aysdr_scanslosarg:
 *  helper for aysdr_scansdrtcmd
 *  scan a single shader argument
 */
int
aysdr_scansdrsarg(TSdrParameter *param, Tcl_DString *ds)
{
 int ay_status = AY_OK;
 char buffer[255];
 double deffltval;
 char *defstrval;
 int j;

  switch(param->type)
    {
    case TYPE_POINT:
    case TYPE_COLOR:
    case TYPE_VECTOR:
    case TYPE_NORMAL:
      if((param->defaultValue).vector)
	{
	  Tcl_DStringAppend(ds, "{ ", -1);
	  deffltval = (double)((param->defaultValue).vector[0]);
	  sprintf(buffer, "%g ", deffltval);
	  Tcl_DStringAppend(ds, buffer, -1);
	  deffltval = (double)((param->defaultValue).vector[1]);
	  sprintf(buffer, "%g ", deffltval);
	  Tcl_DStringAppend(ds, buffer, -1);
	  deffltval = (double)((param->defaultValue).vector[2]);
	  sprintf(buffer, "%g ", deffltval);
	  Tcl_DStringAppend(ds, buffer, -1);
	  Tcl_DStringAppend(ds, "} ", -1);
	}
      else
	{
	  Tcl_DStringAppend(ds, "{ 0 0 0 } ", -1);
	}
      break;
    case TYPE_MATRIX:
      if((param->defaultValue).matrix)
	{
	  Tcl_DStringAppend(ds, "{ ", -1);
	  for(j = 0; j < 16; j++)
	    {
	      deffltval = (double)((param->defaultValue).matrix[j]);
	      sprintf(buffer, "%g ", deffltval);
	      Tcl_DStringAppend(ds, buffer, -1);
	    } /* for */
	  Tcl_DStringAppend(ds, "} ", -1);
	}
      else
	{
	  Tcl_DStringAppend(ds, "{ 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 } ", -1);
	}
      break;
    case TYPE_FLOAT:
      deffltval = (double)((param->defaultValue).scalar);
      sprintf(buffer, "%g ", deffltval);
      Tcl_DStringAppend(ds, buffer, -1);
      break;
    case TYPE_STRING:
      if((param->defaultValue).string)
	{
	  defstrval = (param->defaultValue).string;
	  Tcl_DStringAppend(ds, defstrval, -1);
	  Tcl_DStringAppend(ds, " ", -1);
	}
      else
	{
	  Tcl_DStringAppend(ds, " \"\"", -1);
	}
      break;
    default:
      break;
    } /* switch */

 return ay_status;
} /* aysdr_scansdrsarg */


/* aysdr_scansdrtcmd:
 *  scan a shader compiled with sdrc using libsdr
 */
int
aysdr_scansdrtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 char fname[] = "shaderScanSDR";
 /* int i = 0, j = 0, numargs = 0; */
 TSdrShader *shader = NULL;
 TSdrParameter *param = NULL;
 char buffer[255];
 int arraylen;
 ESdrShaderType stype;
 Tcl_DString ds, dsp;
 char vname[] = "ayprefs(Shaders)", vnamewin[] = "ay(PixieShaders)";
 char command[] = "aysdr_rewritepath";
 char *c = NULL;

  if(argc < 3)
    {
      ay_error(AY_EARGS, fname, "shaderpath varname");
      return TCL_OK;
    }

  Tcl_DStringInit(&dsp);

#ifdef WIN32
  /* change '...;C:/bla...' to '...;//C/bla...' */
  Tcl_Eval(interp, command);
  Tcl_DStringAppend(&dsp, Tcl_GetVar(interp, vnamewin,
				     TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG), -1);
#else
  Tcl_DStringAppend(&dsp, Tcl_GetVar(interp, vname,
				     TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG), -1);
#endif

#ifdef WIN32
  /* change all ; to : in shader search path */
  c = strchr(Tcl_DStringValue(&dsp), ';');
  while(c)
    {
      *c = ':';
      c = strchr(c, ';');
    }
#endif

  shader = sdrGet(argv[1], Tcl_DStringValue(&dsp));
  Tcl_DStringFree(&dsp);
  if(!shader)
    {
      ay_error(AY_ERROR, fname, "sdrGet failed for:");
      ay_error(AY_ERROR, fname, argv[1]);
      return TCL_OK;
    }

  Tcl_DStringInit(&ds);

  /* get name of shader */
  Tcl_DStringAppend(&ds, argv[1], -1);

  /* get type of shader */
  stype = shader->type;
  switch(stype)
    {
    case SHADER_SURFACE:
      Tcl_DStringAppend(&ds, " surface ", -1);
     break;
   case SHADER_DISPLACEMENT:
      Tcl_DStringAppend(&ds, " displacement ", -1);
     break;
   case SHADER_LIGHT:
      Tcl_DStringAppend(&ds, " light ", -1);
     break;
   case SHADER_VOLUME:
     Tcl_DStringAppend(&ds, " volume ", -1);
     break;
   case SHADER_IMAGER:
     Tcl_DStringAppend(&ds, " imager ", -1);
     break;
     /* the shader type "transformation" is unknown to Pixie-1.1.2 */
     /*
     case SLO_TYPE_TRANSFORMATION:
      Tcl_DStringAppend(&ds, " transformation ", -1);
     break;
     */
   default:
     break;
   }

  /* get arguments of shader */

  Tcl_DStringAppend(&ds, "{ ", -1);
  param = shader->parameters;
  while(param)
    {
      /* XXXX temporarily discard array arguments   */
      if(param->numItems == 1)
	{

      Tcl_DStringAppend(&ds, "{ ", -1);
      Tcl_DStringAppend(&ds, param->name, -1);
      Tcl_DStringAppend(&ds, " ", -1 );

      switch(param->type)
	{
	case TYPE_POINT:
	  Tcl_DStringAppend(&ds, "point ", -1);
	  break;
	case TYPE_COLOR:
	  Tcl_DStringAppend(&ds, "color ", -1);
	  break;
	case TYPE_VECTOR:
	  Tcl_DStringAppend(&ds, "vector ", -1);
	  break;
	case TYPE_NORMAL:
	  Tcl_DStringAppend(&ds, "normal ", -1);
	  break;
	case TYPE_MATRIX:
	  Tcl_DStringAppend(&ds, "matrix ", -1);
	  break;
	case TYPE_FLOAT:
	  Tcl_DStringAppend(&ds, "float ", -1);
	  break;
	case TYPE_STRING:
	  Tcl_DStringAppend(&ds, "string ", -1);
	  break;
	default:
	  Tcl_DStringAppend(&ds, "unknown ", -1);
	  break;
	}

      arraylen = param->numItems;
      sprintf(buffer, "%d ", arraylen);
      Tcl_DStringAppend(&ds, buffer, -1);

      if(arraylen > 1)
	{
	  /* XXXX TBD */
          #if 0
	  Tcl_DStringAppend(&ds, "{ ", -1);

	  for(j = 0; j < arraylen; j++)
	    {
	      element = NULL;
	      element = Sdr_GetArrayArgElement(param, j);
	      if(!element)
		{
		  ay_error(AY_ERROR, fname, "Could not get array element:");
		  ay_error(AY_ERROR, fname, param->name);
		  Tcl_DStringFree(&ds);
		  return TCL_OK;
		} /* if */
	      aysdr_scansdrsarg(element, &ds);
	    } /* for */

	  aysdr_scansdrsarg(param, &ds);
	  Tcl_DStringAppend(&ds, "} ", -1);
          #endif /* 0 */
	}
      else
	{
	  aysdr_scansdrsarg(param, &ds);
	} /* if */

      Tcl_DStringAppend(&ds, "} ", -1);

	}
      else
	{
	  ay_error(AY_EWARN,fname,"Skipping array argument!");
	  /*	  ay_error(AY_EWARN,fname,symbol->svd_name);*/
	} /* if */
      /* XXXX temporarily discard array arguments */
      param = param->next;
    } /* while */
  Tcl_DStringAppend(&ds, "} ", -1);

  sdrDelete(shader);

  Tcl_SetVar(interp, argv[2], Tcl_DStringValue(&ds), TCL_LEAVE_ERR_MSG);

  Tcl_DStringFree(&ds);

 return TCL_OK;
} /* aysdr_scansdrtcmd */


/* note: this function _must_ be capitalized exactly this way
 * regardless of the filename of the shared object (see: man n load)!
 */
#ifdef WIN32
__declspec( dllexport ) int
Aysdr_Init(Tcl_Interp *interp)
#else
int
Aysdr_Init(Tcl_Interp *interp)
#endif
{
 char fname[] = "aysdr_init";
 char vname[] = "ay(sext)", vval[] = ".sdr";

  ay_plugin_interp = interp;
  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }

  Tcl_SetVar(interp, vname, vval, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_CreateCommand(interp, "shaderScan", aysdr_scansdrtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* source aysdr.tcl, it contains Tcl-code for path rewriting */
  if((Tcl_EvalFile(interp, "aysdr.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname,
		  "Error while sourcing \\\"aysdr.tcl\\\"!");
       return TCL_OK;
     }

  ay_error(AY_EOUTPUT, fname,
	   "Plugin 'aysdr' loaded.");
  ay_error(AY_EOUTPUT, fname,
	   "Ayam will now scan for .sdr-shaders only!");

 return TCL_OK;
} /* Aysdr_Init */
