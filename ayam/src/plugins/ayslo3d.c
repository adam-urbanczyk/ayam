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

/* ayslo3d.c - Plugin to scan shaders compiled with shaderdl (3Delight)
   using lib3delight  */

/* includes: */
#include "tcl.h"
#include "errcode.h"
#include "slo.h"


/* prototypes: */
int ayslo3d_scanslo3dsarg(SLO_VISSYMDEF *symbol, Tcl_DString *ds);

int ayslo3d_scanslo3dtcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);

extern void ay_error(int code, char *where, char *what);

extern void ay_error_init(Tcl_Interp *interp);

#ifdef WIN32
__declspec( dllexport ) int Ayslo_Init(Tcl_Interp *interp);
#else
int Ayslo_Init(Tcl_Interp *interp);
#endif


/* functions: */

/* ayslo3d_scanslo3dsarg:
 *  helper for ayslo3d_scanslo3dtcmd
 *  scan a single shader argument
 */
int
ayslo3d_scanslo3dsarg(SLO_VISSYMDEF *symbol, Tcl_DString *ds)
{
 int ay_status = AY_OK;
 char buffer[255];
 double deffltval;
 const char *defstrval;
 int i, j;

  switch(symbol->svd_type)
    {
    case SLO_TYPE_POINT:
    case SLO_TYPE_COLOR:
    case SLO_TYPE_VECTOR:
    case SLO_TYPE_NORMAL:
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
    case SLO_TYPE_MATRIX:
      Tcl_DStringAppend(ds, "{ ", -1);
      for(i = 0; i < 4; i++)
	{
	  for(j = 0; j < 4; j++)
	    {
#ifdef AYOLDSLO3D
	      deffltval = (double)((*((symbol->svd_default).matrixval))[i][j]);
#else
	      deffltval = (double)(((symbol->svd_default).matrixval)[i*4+j]);
#endif /* AYOLDSLO3D */
	      sprintf(buffer, "%g ", deffltval);
	      Tcl_DStringAppend(ds, buffer, -1);
	    } /* for */
	} /* for */
      Tcl_DStringAppend(ds, "} ", -1);
      break;
    case SLO_TYPE_SCALAR:
      deffltval = (double)(*(symbol->svd_default).scalarval);
      sprintf(buffer, "%g ", deffltval);
      Tcl_DStringAppend(ds, buffer, -1);
      break;
    case SLO_TYPE_STRING:
      defstrval = (symbol->svd_default).stringval;
      if(defstrval)
	{
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
} /* ayslo3d_scanslo3dsarg */


/* ayslo3d_scanslo3dtcmd:
 *  scan a shader compiled with shaderdl using lib3delight
 */
int
ayslo3d_scanslo3dtcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 char fname[] = "shaderScanSLO3D";
 int i = 0, j = 0, numargs = 0;
 SLO_VISSYMDEF *symbol = NULL, *element = NULL;
 SLO_TYPE type;
 char buffer[255];
 int arraylen;
 Tcl_DString ds;
 char vname[] = "ayprefs(Shaders)";

  if(argc < 3)
    {
      ay_error(AY_EARGS, fname, "shaderpath varname");
      return TCL_OK;
    }

  Slo_SetPath(Tcl_GetVar(interp, vname,
			 TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG));

  if((Slo_SetShader(argv[1])) == -1)
    {
      ay_error(AY_ERROR, fname, "Slo_SetShader failed for:");
      ay_error(AY_ERROR, fname, argv[1]);
      return TCL_OK;
    }

  Tcl_DStringInit(&ds);

  /* get name of shader */
  Tcl_DStringAppend(&ds, argv[1]/*SLO_GetName()*/, -1);

  /* get type of shader */
  type = Slo_GetType();
  switch (type)
    {
    case SLO_TYPE_SURFACE:
      Tcl_DStringAppend(&ds, " surface ", -1);
     break;
   case SLO_TYPE_DISPLACEMENT:
      Tcl_DStringAppend(&ds, " displacement ", -1);
     break;
   case SLO_TYPE_LIGHT:
      Tcl_DStringAppend(&ds, " light ", -1);
     break;
   case SLO_TYPE_VOLUME:
     Tcl_DStringAppend(&ds, " volume ", -1);
     break;
   case SLO_TYPE_IMAGER:
     Tcl_DStringAppend(&ds, " imager ", -1);
     break;
   case SLO_TYPE_TRANSFORMATION:
     Tcl_DStringAppend(&ds, " transformation ", -1);
     break;
   default:
     ay_error(AY_ERROR, fname, "skipping shader of unknown type");

     Slo_EndShader();

     Tcl_DStringFree(&ds);

     return TCL_OK;
     break;
   }

  /* get arguments of shader */
  numargs = Slo_GetNArgs();
  Tcl_DStringAppend(&ds, "{ ", -1);
  for(i = 1; i <= numargs; i++)
    {

      symbol = NULL;
      symbol = Slo_GetArgById(i);

      if(!symbol)
	{
	  ay_error(AY_ERROR, fname, "Cannot get symbol from shader:");
	  ay_error(AY_ERROR, fname, argv[1]);

	  Slo_EndShader();
	  Tcl_DStringFree(&ds);
	  return TCL_OK;
	}

      /* XXXX temporarily discard array arguments   */
      if(symbol->svd_arraylen < 2)
	{

      Tcl_DStringAppend(&ds, "{ ", -1);
      Tcl_DStringAppend(&ds, symbol->svd_name, -1);
      Tcl_DStringAppend(&ds, " ", -1 );

      switch(symbol->svd_type)
	{
	case SLO_TYPE_POINT:
	  Tcl_DStringAppend(&ds, "point ", -1);
	  break;
	case SLO_TYPE_COLOR:
	  Tcl_DStringAppend(&ds, "color ", -1);
	  break;
	case SLO_TYPE_VECTOR:
	  Tcl_DStringAppend(&ds, "vector ", -1);
	  break;
	case SLO_TYPE_NORMAL:
	  Tcl_DStringAppend(&ds, "normal ", -1);
	  break;
	case SLO_TYPE_MATRIX:
	  Tcl_DStringAppend(&ds, "matrix ", -1);
	  break;
	case SLO_TYPE_SCALAR:
	  Tcl_DStringAppend(&ds, "float ", -1);
	  break;
	case SLO_TYPE_STRING:
	  Tcl_DStringAppend(&ds, "string ", -1);
	  break;
	default:
	  Tcl_DStringAppend(&ds, "unknown ", -1);
	  break;
	}

      arraylen = symbol->svd_arraylen;
      sprintf(buffer, "%d ", arraylen);
      Tcl_DStringAppend(&ds, buffer, -1);

      if(arraylen > 1)
	{
	  Tcl_DStringAppend(&ds, "{ ", -1);
	  for(j = 0; j < arraylen; j++)
	    {
	      element = NULL;
              #if 0
	      element = Slo_GetArrayArgElement(symbol, j);
	      #endif
	      if(!element)
		{
		  ay_error(AY_ERROR, fname, "Could not get array element:");
		  ay_error(AY_ERROR, fname, (char*)symbol->svd_name);
		  Tcl_DStringFree(&ds);
		  return TCL_OK;
		} /* if */
	      ayslo3d_scanslo3dsarg(element, &ds);
	    } /* for */

	  Tcl_DStringAppend(&ds, "} ", -1);
	}
      else
	{
	  ayslo3d_scanslo3dsarg(symbol, &ds);
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


  Slo_EndShader();

  Tcl_SetVar(interp, argv[2], Tcl_DStringValue(&ds), TCL_LEAVE_ERR_MSG);

  Tcl_DStringFree(&ds);

 return TCL_OK;
} /* ayslo3d_scanslo3dtcmd */


/* note: this function _must_ be capitalized _and named_ exactly this way
 * regardless of the filename of the shared object (see: man n load)!
 */
#ifdef WIN32
__declspec( dllexport ) int
Ayslo_Init(Tcl_Interp *interp)
#else
int
Ayslo_Init(Tcl_Interp *interp)
#endif
{
 char fname[] = "ayslo3d_init";
 char vname[] = "ay(sext)", vval[] = ".sdl";

  ay_error_init(interp);

  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }

  Tcl_SetVar(interp, vname, vval, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_CreateCommand(interp, "shaderScan", ayslo3d_scanslo3dtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  ay_error(AY_EOUTPUT, fname,
	   "Plugin 'ayslo3d' loaded.");
  ay_error(AY_EOUTPUT, fname,
	   "Ayam will now scan for .sdl-shaders only!");

 return TCL_OK;
} /* Ayslo_Init */
