/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2001 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/** \file tmp.c \brief temporary file name management */

/** ay_tmp_gettcmd:
 *  Create a temporary filename.
 *  Implements the \a tmpGet scripting interface command.
 *  See also the corresponding section in the \ayd{sctmpget}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_tmp_gettcmd(ClientData clientData, Tcl_Interp *interp,
	       int argc, char *argv[])
{
 char *tmpname = NULL;
 Tcl_Obj *to = NULL, *ton = NULL;
 Tcl_DString ds;

  if(argc < 3)
    {
      ay_error(AY_EARGS, argv[0], "tmpdir varname [extension]");
      return TCL_OK;
    }

#ifndef AYNOTEMPNAM
  if(!(tmpname = tempnam(argv[1], "ayam")))
    {
      ay_error(AY_ERROR, argv[0], "cannot create tempname");
      return TCL_OK;
    }
#else
  if(!(tmpname = tmpnam(argv[1])))
    {
      ay_error(AY_ERROR, argv[0], "cannot create tempname");
      return TCL_OK;
    }
#endif

  ton = Tcl_NewStringObj(argv[2], -1);

  if(argc > 3)
    {
      Tcl_DStringInit(&ds);
      Tcl_DStringAppend(&ds, tmpname, -1);
      Tcl_DStringAppend(&ds, argv[3], -1);
      to = Tcl_NewStringObj(Tcl_DStringValue(&ds), -1);
      Tcl_DStringFree(&ds);
    }
  else
    {
      to = Tcl_NewStringObj(tmpname, -1);
    }

  Tcl_ObjSetVar2(interp, ton, NULL, to, TCL_LEAVE_ERR_MSG);

  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  free(tmpname);

 return TCL_OK;
} /* ay_tmp_gettcmd */
