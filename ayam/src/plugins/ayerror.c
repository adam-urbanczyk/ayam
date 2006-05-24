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

/* ayerror.c - support for plugin error reporting on Win32 */

/* includes: */
#include "tcl.h"


/* prototypes: */
void ay_error(int code, char *where, char *what);

void ay_error_init(Tcl_Interp *interp);


/* global variables: */
static Tcl_Interp *ay_error_interp = NULL;


/* functions: */

/* ay_error:
 *  this function is used by the shader parsing plugins on Win32 only,
 *  it calls the original ay_error via its Tcl interface because the
 *  original ay_error is not reachable directly from Win32 plugins
 */
void
ay_error(int code, char *where, char *what)
{
 Tcl_DString ds;
 char ayerrcmdstr[] = "ayError ", codestr[64];

  Tcl_DStringInit(&ds);

  Tcl_DStringAppend(&ds, ayerrcmdstr, -1);

  sprintf(codestr, "%d ", code);

  Tcl_DStringAppend(&ds, codestr, -1);

  if(where)
    {
      Tcl_DStringAppend(&ds, where, -1);

      if(what)
	{
	  Tcl_DStringAppend(&ds, " \"", -1);

	  Tcl_DStringAppend(&ds, what, -1);

	  Tcl_DStringAppend(&ds, "\"", -1);
	}
    }

  Tcl_Eval(ay_error_interp, Tcl_DStringValue(&ds));

  Tcl_DStringFree(&ds);

 return;
} /* ay_error */


/* ay_error_init:
 */
void
ay_error_init(Tcl_Interp *interp)
{
   ay_error_interp = interp;
  return;
} /* ay_error_init */
