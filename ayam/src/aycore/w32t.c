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
