/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2003 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

#include <slx.h>

/* ayslx.c - Plug-In to scan shaders compiled with aqsl (Aqsis)
   using libslxargs  */

/* global variables */
char ayslx_version_ma[] = AY_VERSIONSTR;
char ayslx_version_mi[] = AY_VERSIONSTRMI;

/* prototypes of functions local to this module */
int ayslx_scanslxsarg(SLX_VISSYMDEF *symbol, Tcl_DString *ds);

int ayslx_scanslxtcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

#ifdef WIN32
extern Tcl_Interp *ay_plugin_interp;
Tcl_Interp *ay_plugin_interp;
__declspec( dllexport ) int Ayslx_Init(Tcl_Interp *interp);
#else
int Ayslx_Init(Tcl_Interp *interp);
#endif
/* functions: */

/* ayslx_scanslxsarg:
 *  helper for ayslx_scanslxtcmd
 *  scan a single shader argument
 */
int
ayslx_scanslxsarg(SLX_VISSYMDEF *symbol, Tcl_DString *ds)
{
 int ay_status = AY_OK;
 char buffer[255];
 double deffltval;
 char *defstrval;
 int i, j;

  switch(symbol->svd_type)
    {
    case SLX_TYPE_POINT:
    case SLX_TYPE_COLOR:
    case SLX_TYPE_VECTOR:
    case SLX_TYPE_NORMAL:
      Tcl_DStringAppend(ds, "{ ", -1);
      deffltval = (double)((symbol->svd_default).pointval->xval);
      sprintf(buffer, "%g ", deffltval);
      Tcl_DStringAppend(ds, buffer, -1);
      deffltval = (double)((symbol->svd_default).pointval->yval);
      sprintf(buffer, "%g ", deffltval);
      Tcl_DStringAppend(ds, buffer, -1);
      deffltval = (double)((symbol->svd_default).pointval->zval);
      sprintf(buffer, "%g ", deffltval);
      Tcl_DStringAppend(ds, buffer, -1);
      Tcl_DStringAppend(ds, "} ", -1);
      break;
    case SLX_TYPE_MATRIX:
      Tcl_DStringAppend(ds, "{ ", -1);
#ifndef AYOLDSLX
      for(i = 0; i < 4; i++)
	{
	  for(j = 0; j < 4; j++)
	    {
	      deffltval = (double)((symbol->svd_default).matrixval->val[i][j]);
	      sprintf(buffer, "%g ", deffltval);
	      Tcl_DStringAppend(ds, buffer, -1);
	    } /* for */
	} /* for */
#else
      for(j = 0; j < 16; j++)
	{
	  deffltval = (double)((symbol->svd_default).matrixval[j]);
	  sprintf(buffer, "%g ", deffltval);
	  Tcl_DStringAppend(ds, buffer, -1);
	} /* for */
#endif
      Tcl_DStringAppend(ds, "} ", -1);
      break;
    case SLX_TYPE_SCALAR:
      deffltval = (double)(*(symbol->svd_default).scalarval);
      sprintf(buffer, "%g ", deffltval);
      Tcl_DStringAppend(ds, buffer, -1);
      break;
    case SLX_TYPE_STRING:
      defstrval = *(symbol->svd_default).stringval;
      Tcl_DStringAppend(ds, defstrval, -1);
      Tcl_DStringAppend(ds, " ", -1);
      break;
    default:
      break;
    } /* switch */

 return ay_status;
} /* ayslx_scanslxsarg */


/* ayslx_scanslxtcmd:
 *  scan a shader compiled with aqsl with libslxargs
 */
int
ayslx_scanslxtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 char fname[] = "shaderScanSLX";
 int i = 0, j = 0, numargs = 0;
 SLX_VISSYMDEF *symbol = NULL, *element = NULL;
 SLX_TYPE type;
 char buffer[255];
 int arraylen;
 Tcl_DString ds;
 char vname[] = "ayprefs(Shaders)";

  if(argc < 3)
    {
      ay_error(AY_EARGS, fname, "shaderpath varname");
      return TCL_OK;
    }

#ifdef WIN32
  SLX_SetPath(Tcl_GetVar(ay_plugin_interp, vname,
			 TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG));
#else
  SLX_SetPath(Tcl_GetVar(ay_interp, vname,
			 TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG));
#endif

  if((SLX_SetShader(argv[1])) == -1)
    {
      ay_error(AY_ERROR, fname, "SLX_SetShader failed for:");
      ay_error(AY_ERROR, fname, argv[1]);
      return TCL_OK;
    }

  Tcl_DStringInit(&ds);

  /* get name of shader */
  Tcl_DStringAppend(&ds, argv[1]/*SLX_GetName()*/, -1);

  /* get type of shader */
  type = SLX_GetType();
  switch (type)
    {
    case SLX_TYPE_SURFACE:
      Tcl_DStringAppend(&ds, " surface ", -1);
     break;
   case SLX_TYPE_DISPLACEMENT:
      Tcl_DStringAppend(&ds, " displacement ", -1);
     break;
   case SLX_TYPE_LIGHT:
      Tcl_DStringAppend(&ds, " light ", -1);
     break;
   case SLX_TYPE_VOLUME:
     Tcl_DStringAppend(&ds, " volume ", -1);
     break;
   case SLX_TYPE_IMAGER:
     Tcl_DStringAppend(&ds, " imager ", -1);
     break;
   case SLX_TYPE_TRANSFORMATION:
     Tcl_DStringAppend(&ds, " transformation ", -1);
     break;
   default:
     break;
   }

  /* get arguments of shader */
  numargs = SLX_GetNArgs();
  Tcl_DStringAppend(&ds, "{ ", -1);
  for(i = 0; i < numargs; i++)
    {

      symbol = NULL;
      symbol = SLX_GetArgById(i);

      if(!symbol)
	{
	  ay_error(AY_ERROR, fname, "Cannot get symbol from shader:");
	  ay_error(AY_ERROR, fname, argv[1]);
	  /*
	  SLX_EndShader();
	  Tcl_DStringFree(&ds);
	  return TCL_OK;
	  */
	}

      /* XXXX temporarily discard array arguments   */
      if(symbol->svd_arraylen < 1)
	{

      Tcl_DStringAppend(&ds, "{ ", -1);
      Tcl_DStringAppend(&ds, symbol->svd_name, -1);
      Tcl_DStringAppend(&ds, " ", -1 );

      switch(symbol->svd_type)
	{
	case SLX_TYPE_POINT:
	  Tcl_DStringAppend(&ds, "point ", -1);
	  break;
	case SLX_TYPE_COLOR:
	  Tcl_DStringAppend(&ds, "color ", -1);
	  break;
	case SLX_TYPE_VECTOR:
	  Tcl_DStringAppend(&ds, "vector ", -1);
	  break;
	case SLX_TYPE_NORMAL:
	  Tcl_DStringAppend(&ds, "normal ", -1);
	  break;
	case SLX_TYPE_MATRIX:
	  Tcl_DStringAppend(&ds, "matrix ", -1);
	  break;
	case SLX_TYPE_SCALAR:
	  Tcl_DStringAppend(&ds, "float ", -1);
	  break;
	case SLX_TYPE_STRING:
	  Tcl_DStringAppend(&ds, "string ", -1);
	  break;
	default:
	  Tcl_DStringAppend(&ds, "unknown ", -1);
	  break;
	}

      arraylen = symbol->svd_arraylen;
      sprintf(buffer, "%d ", arraylen);
      Tcl_DStringAppend(&ds, buffer, -1);

      if(arraylen > 0)
	{
	  Tcl_DStringAppend(&ds, "{ ", -1);
	  for(j = 0; j < arraylen; j++)
	    {
	      element = NULL;
	      element = SLX_GetArrayArgElement(symbol, j);
	      if(!element)
		{
		  ay_error(AY_ERROR, fname, "Could not get array element:");
		  ay_error(AY_ERROR, fname, symbol->svd_name);
		  Tcl_DStringFree(&ds);
		  return TCL_OK;
		} /* if */
	      ayslx_scanslxsarg(element, &ds);
	    } /* for */

	  Tcl_DStringAppend(&ds, "} ", -1);
	}
      else
	{
	  ayslx_scanslxsarg(symbol, &ds);
	} /* if */
      Tcl_DStringAppend(&ds, "} ", -1);

	} 
      else
	{
	  ay_error(AY_EWARN,fname,"Skipping array argument!");
	  /*	  ay_error(AY_EWARN,fname,symbol->svd_name);*/
	} /* if */
      /* XXXX temporarily discard array arguments */
    } /* for */
  Tcl_DStringAppend(&ds, "} ", -1);


  SLX_EndShader();

  Tcl_SetVar(interp, argv[2], Tcl_DStringValue(&ds), TCL_LEAVE_ERR_MSG);

  Tcl_DStringFree(&ds);

 return TCL_OK;
} /* ayslx_scanslxtcmd */


/* note: this function _must_ be capitalized exactly this way
 * regardless of the filename of the shared object (see: man n load)!
 */
#ifdef WIN32
__declspec( dllexport ) int
Ayslx_Init(Tcl_Interp *interp)
#else
int
Ayslx_Init(Tcl_Interp *interp)
#endif
{
 char fname[] = "ayslx_init";
 char vname[] = "ay(sext)", vval[] = ".slx";

#ifdef WIN32
  ay_plugin_interp = interp;
  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }
#else
  /* first, check versions */
  if(strcmp(ay_version_ma, ayslx_version_ma))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "It is unsafe to continue! Bailing out...");
      return TCL_OK;
    }

  if(strcmp(ay_version_mi, ayslx_version_mi))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "However, it is probably safe to continue...");
    }
#endif

  Tcl_SetVar(interp, vname, vval, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_CreateCommand(interp, "shaderScan", ayslx_scanslxtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  ay_error(AY_EOUTPUT, fname,
	   "Plug-In 'ayslx' loaded.");
  ay_error(AY_EOUTPUT, fname,
	   "Ayam will now scan for .slx-shaders only!");

 return TCL_OK;
} /* Ayslx_Init */
