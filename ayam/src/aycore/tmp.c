/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2001 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* tmp.c - temporary files */

/* ay_tmp_gettcmd:
 *  create a temporary filename
 */
int
ay_tmp_gettcmd(ClientData clientData, Tcl_Interp *interp,
	       int argc, char *argv[])
{
 char fname[] = "tmpGet";
 char *tmpname = NULL;
 Tcl_Obj *to = NULL, *ton = NULL;

  if(argc < 3)
    {
      ay_error(AY_EARGS, fname, "tmpdir varname");
      return TCL_OK;
    }

#ifndef AYNOTEMPNAM
  if(!(tmpname = tempnam(argv[1], "ayam")))
    {
      ay_error(AY_ERROR, fname, "cannot create tempname");
       return TCL_OK;
    }
#else
  if(!(tmpname = tmpnam(argv[1])))
    {
      ay_error(AY_ERROR, fname, "cannot create tempname");
      return TCL_OK;
    }
#endif

  ton = Tcl_NewStringObj(argv[2],-1);
  to = Tcl_NewStringObj(tmpname,-1);
  
  Tcl_ObjSetVar2(interp,ton,NULL,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  free(tmpname);

 return TCL_OK;
} /* ay_tmp_gettcmd */
