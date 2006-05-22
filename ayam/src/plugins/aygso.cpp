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

/* aygso.cpp - Plugin to scan Gelato shaders */


#include "tcl.h"

#include "gsoargs.h"

/* global variables */

#ifndef AYWIN32PLUGIN
char ayglo_version_ma[] = AY_VERSIONSTR;
char ayglo_version_mi[] = AY_VERSIONSTRMI;
#endif // AYWIN32PLUGIN

/* prototypes of functions local to this module */
int aygso_scangsosarg(const Gelato::GsoArgs::Parameter *p, Tcl_DString *ds);

int aygso_scangsotcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

extern "C" {

#ifdef AYWIN32PLUGIN

#include "errcode.h"

extern Tcl_Interp *ay_plugin_interp;
Tcl_Interp *ay_plugin_interp;
extern void ay_error(int code, char *where, char *what);
__declspec( dllexport ) int Aygso_Init(Tcl_Interp *interp);
#else
int Aygso_Init(Tcl_Interp *interp);
#endif // AYWIN32PLUGIN

} // extern "C"

/* functions: */

/* aygso_scangsosarg:
 *  helper for aygso_scangsotcmd
 *  scan a single shader argument
 */
int
aygso_scangsosarg(const Gelato::GsoArgs::Parameter *p, Tcl_DString *ds)
{
 int ay_status = 0;
 char buffer[255];
 int j, arraylen;

#if 0
  arraylen = p->arraylen;
  sprintf(buffer, "%d ", arraylen);
  Tcl_DStringAppend(&ds, buffer, -1);

  if(arraylen > 0)
    {
      Tcl_DStringAppend(&ds, "{ ", -1);
      for(j = 0; j < arraylen; j++)
	{

	} /* for */

      Tcl_DStringAppend(&ds, "} ", -1);
    }
  else
    {
#endif

	switch(p->type.basetype)
    {
    case Gelato::PT_POINT:
    case Gelato::PT_COLOR:
    case Gelato::PT_VECTOR:
    case Gelato::PT_NORMAL:
      if(p->valid)
	{
	  Tcl_DStringAppend(ds, "{ ", -1);
	  sprintf(buffer, "%g %g %g ", (double)(p->fdefault[0]),
		  (double)(p->fdefault[1]), (double)(p->fdefault[2]));
	  Tcl_DStringAppend(ds, buffer, -1);
	  Tcl_DStringAppend(ds, "} ", -1);
	}
      else
	{
	  Tcl_DStringAppend(ds, "{ 0.0 0.0 0.0 } ", -1);
	}
      break;
    case Gelato::PT_MATRIX:
      if(p->valid)
	{
	  Tcl_DStringAppend(ds, "{ ", -1);
	  for(j = 0; j < 16; j++)
	    {
	      sprintf(buffer, "%g ", (double)(p->fdefault[j]));
	      Tcl_DStringAppend(ds, buffer, -1);
	    } /* for */
	}
      else
	{
	  Tcl_DStringAppend(ds, "{ 0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0 }", -1);
	}
      Tcl_DStringAppend(ds, "} ", -1);
      break;
    case Gelato::PT_FLOAT:
    case Gelato::PT_HALF:
    case Gelato::PT_DOUBLE:
      sprintf(buffer, "%g ", (double)(p->fdefault[0]));
      Tcl_DStringAppend(ds, buffer, -1);
      break;
    case Gelato::PT_STRING:
      if(p->valid)
	{
	  Tcl_DStringAppend(ds, p->sdefault[0], -1);
	  Tcl_DStringAppend(ds, " ", -1);
	}
      else
	{
	  Tcl_DStringAppend(ds, "\"\" ", -1);
	}
      break;
    default:
      break;
    } /* switch */

#if 0
    }
#endif

 return ay_status;
} /* aygso_scangsosarg */


/* aygso_scangsotcmd:
 *  scan a Gelato shader using libgsoargs
 */
int
aygso_scangsotcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 char fname[] = "shaderScanGSO";
 Gelato::GsoArgs argparser;
 const Gelato::GsoArgs::Parameter *p = NULL;
 Tcl_DString ds;
 bool result;
 char vname[] = "ayprefs(Shaders)";
 int i = 0, numargs = 0;

  if(argc < 3)
    {
      ay_error(AY_EARGS, fname, "shaderpath varname");
      return TCL_OK;
    }

  Tcl_DStringInit(&ds);
  Tcl_DStringAppend(&ds, argv[1], -1);
  Tcl_DStringAppend(&ds, ".gso", -1);

#ifdef AYWIN32PLUGIN
  result = argparser.open(argv[1], Tcl_GetVar(ay_plugin_interp, vname,
				    TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG));
#else
  result = argparser.open(argv[1], Tcl_GetVar(ay_interp, vname,
			            TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG));
#endif // AYWIN32PLUGIN

  Tcl_DStringFree(&ds);

  if(result == false)
    {
      ay_error(AY_ERROR, fname, "Could not open shader:");
      ay_error(AY_ERROR, fname, argv[1]);
      return TCL_OK;
    }

  Tcl_DStringInit(&ds);

  /* get name of shader */
  Tcl_DStringAppend(&ds, argv[1]/*argparser.shadername()*/, -1);

  Tcl_DStringAppend(&ds, " ", -1);
  Tcl_DStringAppend(&ds, argparser.shadertype(), -1);
  Tcl_DStringAppend(&ds, " ", -1);


  /* get arguments of shader */
  numargs = argparser.nargs();

  Tcl_DStringAppend(&ds, "{ ", -1);

  for(i = 0; i < numargs; i++)
    {

      p = NULL;
      p = argparser.getarg(i);

      if(!p)
	{
	  ay_error(AY_ERROR, fname, "Cannot get parameter from shader:");
	  ay_error(AY_ERROR, fname, argv[1]);
	  /*
	  Tcl_DStringFree(&ds);
	  return TCL_OK;
	  */
	}

      /* XXXX temporarily discard array arguments */
      if(!p->isoutput)
	{
	  if(!p->type.isarray)
	    {
	      Tcl_DStringAppend(&ds, "{ ", -1);
	      Tcl_DStringAppend(&ds, p->name, -1);
	      Tcl_DStringAppend(&ds, " ", -1 );

	      switch(p->type.basetype)
		{
		case Gelato::PT_POINT:
		  Tcl_DStringAppend(&ds, "point ", -1);
		  break;
		case Gelato::PT_COLOR:
		  Tcl_DStringAppend(&ds, "color ", -1);
		  break;
		case Gelato::PT_VECTOR:
		  Tcl_DStringAppend(&ds, "vector ", -1);
		  break;
		case Gelato::PT_NORMAL:
		  Tcl_DStringAppend(&ds, "normal ", -1);
		  break;
		case Gelato::PT_MATRIX:
		  Tcl_DStringAppend(&ds, "matrix ", -1);
		  break;
		case Gelato::PT_FLOAT:
		case Gelato::PT_HALF:
		case Gelato::PT_DOUBLE:
		  Tcl_DStringAppend(&ds, "float ", -1);
		  break;
		case Gelato::PT_STRING:
		  Tcl_DStringAppend(&ds, "string ", -1);
		  break;
		default:
		  Tcl_DStringAppend(&ds, "unknown ", -1);
		  break;
		} /* switch */

	      aygso_scangsosarg(p, &ds);
	      Tcl_DStringAppend(&ds, "} ", -1);
	    }
	  else
	    {
	      /* XXXX temporarily discard array arguments */
	      ay_error(AY_EWARN,fname,"Skipping array argument!");
	      /*	  ay_error(AY_EWARN,fname,symbol->svd_name);*/
	    } /* if */

	} /* if */

    } /* for */

  Tcl_DStringAppend(&ds, "} ", -1);

  Tcl_SetVar(interp, argv[2], Tcl_DStringValue(&ds), TCL_LEAVE_ERR_MSG);

  Tcl_DStringFree(&ds);

 return TCL_OK;
} /* aygso_scangsotcmd */


extern "C" {

/* note: this function _must_ be capitalized exactly this way
 * regardless of the filename of the shared object (see: man n load)!
 */
#ifdef AYWIN32PLUGIN
__declspec( dllexport ) int
Aygso_Init(Tcl_Interp *interp)
#else
int
Aygso_Init(Tcl_Interp *interp)
#endif // AYWIN32PLUGIN
{
 char fname[] = "aygso_init";
 char vname[] = "ay(sext)", vval[] = ".gso";

#ifdef AYWIN32PLUGIN
  ay_plugin_interp = interp;
  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }
#else
  /* first, check versions */
  if(strcmp(ay_version_ma, aygso_version_ma))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "It is unsafe to continue! Bailing out...");
      return TCL_OK;
    }

  if(strcmp(ay_version_mi, aygso_version_mi))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "However, it is probably safe to continue...");
    }
#endif // AYWIN32PLUGIN

  Tcl_SetVar(interp, vname, vval, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_CreateCommand(interp, "shaderScan", aygso_scangsotcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  ay_error(AY_EOUTPUT, fname,
	   "Plug-In 'aygso' loaded.");
  ay_error(AY_EOUTPUT, fname,
	   "Ayam will now scan for .gso-shaders only!");

 return TCL_OK;
} /* Aygso_Init */

} // extern "C"
