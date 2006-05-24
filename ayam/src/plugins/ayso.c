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

/* ayso.c - Plugin to scan shaders compiled with shaderdc (RenderDotC)
   using libsoargs */

/* includes: */
#include "tcl.h"
#include "errcode.h"
#include "so.h"
#include <string.h>

/* prototypes: */
int ayso_scansosarg(SO_VISSYMDEF *symbol, Tcl_DString *ds);

int ayso_scansotcmd(ClientData clientData, Tcl_Interp *interp,
			    int argc, char *argv[]);

extern void ay_error(int code, char *where, char *what);

#ifdef WIN32
__declspec( dllexport ) int Ayso_Init(Tcl_Interp *interp);
#else
int Ayso_Init(Tcl_Interp *interp);
#endif

extern Tcl_Interp *ay_plugin_interp;
Tcl_Interp *ay_plugin_interp;


/* functions: */

/* ayso_scansosarg:
 *  helper for ayso_scansotcmd
 *  scan a single shader argument
 */
int
ayso_scansosarg(SO_VISSYMDEF *symbol, Tcl_DString *ds)
{
 int ay_status = AY_OK;
 char buffer[255];
 double deffltval;
 char *defstrval;
 int j;

  switch(symbol->svd_type)
    {
    case SO_TYPE_POINT:
    case SO_TYPE_COLOR:
    case SO_TYPE_VECTOR:
    case SO_TYPE_NORMAL:
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
    case SO_TYPE_MATRIX:
      Tcl_DStringAppend(ds, "{ ", -1);
      for(j = 0; j < 16; j++)
	{
	  deffltval = (double)((symbol->svd_default).matrixval[j]);
	  sprintf(buffer, "%g ", deffltval);
	  Tcl_DStringAppend(ds, buffer, -1);
	} /* for */
      Tcl_DStringAppend(ds, "} ", -1);
      break;
    case SO_TYPE_SCALAR:
      deffltval = (double)(*(symbol->svd_default).scalarval);
      sprintf(buffer, "%g ", deffltval);
      Tcl_DStringAppend(ds, buffer, -1);
      break;
    case SO_TYPE_STRING:
      defstrval = (symbol->svd_default).stringval;
      Tcl_DStringAppend(ds, defstrval, -1);
      Tcl_DStringAppend(ds, " ", -1);
      break;
    default:
      break;
    } /* switch */

 return ay_status;
} /* ayso_scansosarg */


/* ayso_scansotcmd:
 *  scan a shader compiled with shaderdc using libsoargs
 */
int
ayso_scansotcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 char fname[] = "shaderScanSO";
 int i = 0, j = 0, numargs = 0;
 SO_VISSYMDEF *symbol = NULL, *element = NULL;
 SO_TYPE type;
 char buffer[255];
 int arraylen;
 Tcl_DString ds;
 char vname[] = "ayprefs(Shaders)";
 char *c = NULL;

  if(argc < 3)
    {
      ay_error(AY_EARGS, fname, "shaderpath varname");
      return TCL_OK;
    }

  /* change all ; to , in shader search path */
  Tcl_DStringInit(&ds);
  Tcl_DStringAppend(&ds,
	     Tcl_GetVar(ay_plugin_interp, vname,
			TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG), -1);
  c = strchr(Tcl_DStringValue(&ds), ';');
  while(c)
    {
      *c = ':';
      c = strchr(c, ';');
    }
  So_SetPath(Tcl_DStringValue(&ds));
  Tcl_DStringFree(&ds);

  if((So_SetShader(argv[1])) == -1)
    {
      ay_error(AY_ERROR, fname, "So_SetShader failed for:");
      ay_error(AY_ERROR, fname, argv[1]);
      return TCL_OK;
    }

  Tcl_DStringInit(&ds);

  /* get name of shader */
  Tcl_DStringAppend(&ds, argv[1]/*So_GetName()*/, -1);

  /* get type of shader */
  type = So_GetType();
  switch (type)
    {
    case SO_TYPE_SURFACE:
      Tcl_DStringAppend(&ds, " surface ", -1);
     break;
   case SO_TYPE_DISPLACEMENT:
      Tcl_DStringAppend(&ds, " displacement ", -1);
     break;
   case SO_TYPE_LIGHT:
      Tcl_DStringAppend(&ds, " light ", -1);
     break;
   case SO_TYPE_VOLUME:
     Tcl_DStringAppend(&ds, " volume ", -1);
     break;
   case SO_TYPE_IMAGER:
     Tcl_DStringAppend(&ds, " imager ", -1);
     break;
   case SO_TYPE_TRANSFORMATION:
     Tcl_DStringAppend(&ds, " transformation ", -1);
     break;
   default:
     break;
   }

  /* get arguments of shader */
  numargs = So_GetNArgs();
  Tcl_DStringAppend(&ds, "{ ", -1);
  for(i = 1; i <= numargs; i++)
    {

      symbol = NULL;
      symbol = So_GetArgById(i);

      if(!symbol)
	{
	  ay_error(AY_ERROR, fname, "Cannot get symbol from shader:");
	  ay_error(AY_ERROR, fname, argv[1]);

	  So_EndShader();
	  Tcl_DStringFree(&ds);
	  return TCL_OK;
	}

      /* XXXX temporarily discard array arguments   */
      if(symbol->svd_arraylen < 1)
	{

	  Tcl_DStringAppend(&ds, "{ ", -1);
	  Tcl_DStringAppend(&ds, symbol->svd_name, -1);
	  Tcl_DStringAppend(&ds, " ", -1 );

	  switch(symbol->svd_type)
	    {
	    case SO_TYPE_POINT:
	      Tcl_DStringAppend(&ds, "point ", -1);
	      break;
	    case SO_TYPE_COLOR:
	      Tcl_DStringAppend(&ds, "color ", -1);
	      break;
	    case SO_TYPE_VECTOR:
	      Tcl_DStringAppend(&ds, "vector ", -1);
	      break;
	    case SO_TYPE_NORMAL:
	      Tcl_DStringAppend(&ds, "normal ", -1);
	      break;
	    case SO_TYPE_MATRIX:
	      Tcl_DStringAppend(&ds, "matrix ", -1);
	      break;
	    case SO_TYPE_SCALAR:
	      Tcl_DStringAppend(&ds, "float ", -1);
	      break;
	    case SO_TYPE_STRING:
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
	      element = So_GetArrayArgElement(symbol, j);
	      if(!element)
		{
		  ay_error(AY_ERROR, fname, "Could not get array element:");
		  ay_error(AY_ERROR, fname, symbol->svd_name);
		  Tcl_DStringFree(&ds);
		  return TCL_OK;
		} /* if */
	      ayso_scansosarg(element, &ds);
	    } /* for */

	  Tcl_DStringAppend(&ds, "} ", -1);
	  }
	  else
	    {
	      ayso_scansosarg(symbol, &ds);
	    } /* if */
	  Tcl_DStringAppend(&ds, "} ", -1);

	}
      else
	{
	  ay_error(AY_EWARN, fname, "Skipping array argument!");
	  /*	  ay_error(AY_EWARN,fname,symbol->svd_name);*/
	} /* if */
      /* XXXX temporarily discard array arguments */
    } /* for */
  Tcl_DStringAppend(&ds, "} ", -1);

  So_EndShader();

  Tcl_SetVar(interp, argv[2], Tcl_DStringValue(&ds), TCL_LEAVE_ERR_MSG);

  Tcl_DStringFree(&ds);

 return TCL_OK;
} /* ayso_scansotcmd */


/* note: this function _must_ be capitalized exactly this way
 * regardless of the filename of the shared object (see: man n load)!
 */
#ifdef WIN32
__declspec( dllexport ) int
Ayso_Init(Tcl_Interp *interp)
#else
int
Ayso_Init(Tcl_Interp *interp)
#endif
{
 char fname[] = "ayso_init";
#ifdef WIN32
 char vname[] = "ay(sext)", vval[] = ".dll";
#else
 /* XXXX HPUX - dylib, AIX? */
 char vname[] = "ay(sext)", vval[] = ".so";
#endif


  ay_plugin_interp = interp;
  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }

  Tcl_SetVar(interp, vname, vval, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_CreateCommand(interp, "shaderScan", ayso_scansotcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  ay_error(AY_EOUTPUT, fname,
	   "Plugin 'ayso' successfully loaded.");
#ifdef WIN32
  ay_error(AY_EOUTPUT, fname,
	   "Ayam will now scan for .dll-shaders only!");
#else
  ay_error(AY_EOUTPUT, fname,
	   "Ayam will now scan for .so-shaders only!");
#endif

 return TCL_OK;
} /* Ayso_Init */
