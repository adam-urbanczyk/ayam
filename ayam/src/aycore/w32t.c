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

/* w32t.c - tools for the Win32 platform */

/* ay_w32t_w32killtcmd:
 *  a kill command for the Win32 platform
 *  code taken from the #C-Wiki:
 *  http://twistedmatrix.com/users/jh.twistd/c/moin.cgi/FrontPage
 */
int
ay_w32t_w32killtcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "w32kill";
 HANDLE proc; 

  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "process_id");
      return TCL_OK;
    }

  /* Open process to be killed. */
  proc = OpenProcess(PROCESS_ALL_ACCESS,TRUE,(unsigned long)atol(argv[1])); 
  
  /* Kill process. */ 
  TerminateProcess(proc, 0); 
  
  /* Close process. */ 
  CloseHandle(proc); 

 return TCL_OK;
} /* ay_w32t_w32killtcmd */


#ifdef AYW32PLUGIN
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
 extern Tcl_Interp *ay_plugin_interp;

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

  Tcl_Eval(ay_plugin_interp, Tcl_DStringValue(&ds));

  Tcl_DStringFree(&ds);

 return;
} /* ay_error */
#endif
