/* 
 * wrapMain.c --
 *
 *	This file contains a generic main program for Wrap-based applications.
 *	It can be used as-is for many applications, just by supplying a
 *	different appInitProc procedure for each specific application.
 *	Or, it can be used as a template for creating new main programs
 *	for Tk applications.
 *
 * Copyright (c) 1990-1994 The Regents of the University of California.
 * Copyright (c) 1994-1996 Sun Microsystems, Inc.
 * Copyright (c) 1999      Jan Nijtmans (adapted for Wrap)
 * Copyright (c) 1999      Randolf Schultz (adapted for The Mops)
 * Copyright (c) 2001      Randolf Schultz (adapted for Ayam)
 *                         changes marked XXXX
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
 * Ayam wrapMain.c
 */

#ifdef WIN32
#include <windows.h>
#endif

#define reserved188 tcl_MainLoop

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <wrap.h>
#include <tk.h>
#ifdef NO_STDLIB_H
#   include "../compat/stdlib.h"
#else
#   include <stdlib.h>
#endif

/*
 * Declarations for various library procedures and variables (don't want
 * to include tclPort.h here, because people might copy this
 * file out of the Tcl source directory to make their own modified versions).
 */

#ifdef USE_TCL_STUBS
#include "tclInt.h"
#else
EXTERN char *		TclSetPreInitScript _ANSI_ARGS_((CONST char *string));
extern TclStubs tclStubs;
#define tclStubsPtr &tclStubs
#endif

/*
 * Make sure that Tcl_MainLoop always references the stub pointer, not
 * the real function.
 */
#undef Tcl_MainLoop
#define Tcl_MainLoop ((void (*) _ANSI_ARGS_((void))) ((tclStubsPtr)->tcl_MainLoop))

/*
 * Global variables used by the main program:
 */


static Tcl_Interp *interp;	/* Interpreter for this application. */

/*
 * Forward declarations for procedures defined later in this file.
 */

static void		DisplayWarning _ANSI_ARGS_((Tcl_Interp *interp,
			    CONST char *warning, CONST char *title));


/*
 *----------------------------------------------------------------------
 *
 * Wrap_Main --
 *
 *	Main program for all standalone applications.
 *
 * Results:
 *	None. This procedure never returns (it exits the process when
 *	it's done.
 *
 * Side effects:
 *	This procedure initializes the Tcl world and then starts
 *	interpreting commands;  almost anything could happen, depending
 *	on the script being interpreted.
 *
 *----------------------------------------------------------------------
 */

static char initScript[] = "\n\
load {} Wrap\n\
::wrap::source tclInit.tcl\n\
";

static char defaultScript[] = "\n\
set argv0 [file tail [info nameofexecutable]]\n\
if {![string compare $tcl_platform(platform) windows]} {\n\
    set argv0 [string tolower [file rootname $argv0]]\n\
}\n\
append argv0 .tcl\n\
if {[catch {::wrap::source $argv0} msg]} {\n\
    set argv0 [lindex $argv 0]\n\
    set argv [lrange $argv 1 end]\n\
    if {([incr argc -1] < 0) || !([file readable $argv0])} {\n\
	error $msg\n\
    } else {\n\
	source $argv0\n\
    }\n\
}\n\
";
/* XXXX */
int ay_InitStandalone(Tcl_Interp *interp);

void
Wrap_Main(int argc, char **argv, Tcl_AppInitProc *appInitProc,
	  Tcl_Interp *interp)
#if 0
     /* XXXX K&R->ANSI */
    int argc;				/* Number of arguments. */
    char **argv;			/* Array of argument strings. */
    Tcl_AppInitProc *appInitProc;	/* Application-specific initialization
					 * procedure to call after most
					 * initialization but before starting
					 * to execute commands. */
    Tcl_Interp *interp;
#endif
{
    char *args;
    char buf[20];
    int code = TCL_OK;
    size_t length;
    char *version;
    Tcl_Channel inChannel, outChannel;

    version = Tcl_InitStubs(interp, "8.2", 0);
    if (version == NULL) {
	DisplayWarning(interp, "incompatible Tcl version", "Error");
	return;
    }

    TclSetPreInitScript(initScript);
    Tcl_FindExecutable(argv[0]);
    Tcl_StaticPackage(NULL, "Wrap", Wrap_Init, NULL);

#ifdef TCL_MEM_DEBUG
    Tcl_InitMemory(interp);
#endif

    /*
     * Make command-line arguments available in the Tcl variables "argc"
     * and "argv".
     */

    args = Tcl_Merge(argc-1, argv+1);
    Tcl_SetVar(interp, "argv", args, TCL_GLOBAL_ONLY);
    ckfree(args);
    sprintf(buf, "%d", argc-1);
    Tcl_SetVar(interp, "argc", buf, TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "argv0", argv[0], TCL_GLOBAL_ONLY);

    /*
     * Set the "tcl_interactive" variable.
     */

    Tcl_SetVar(interp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

    /*
     * Invoke application-specific initialization.
     */

    if ((*appInitProc)(interp) != TCL_OK) {
	DisplayWarning(interp, Tcl_GetStringResult(interp),
		"Application initialization failed");
    }

    /* XXXX */
    ay_InitStandalone(interp);


    /*
     * Invoke the script specified on the command line, if any.
     */

    if (Tcl_Eval(interp, defaultScript) != TCL_OK) {
	Tcl_AddErrorInfo(interp, "");
	DisplayWarning(interp, Tcl_GetVar(interp, "errorInfo",
		TCL_GLOBAL_ONLY), "Error in script");
    }

    outChannel = Tcl_GetStdChannel(TCL_STDOUT);
    if (outChannel) {
	Tcl_Flush(outChannel);
    }
    Tcl_ResetResult(interp);

    /*
     * If the Tcl_MainLoop function exists (only available with the
     * plus-patches), use it. This allows other applications to
     * override the event loop.
     */

    if (Tcl_MainLoop != NULL) {
	Tcl_MainLoop();
    } else {
	/*
	 * If Tk is loaded, attempt to start Tk's event loop.
	 */

	TkStubs *ptr = NULL;
	Tcl_PkgPresentEx(interp, "Tk", version, 0, (ClientData *) &ptr);
	if (ptr != NULL) {
	    ptr->tk_MainLoop();
	}
    }
    sprintf(buf, "exit %d", code);
    Tcl_Eval(interp, buf);
}


static void DisplayWarning(interp, msg, title)
    Tcl_Interp *interp;
    CONST char *msg;
    CONST char *title;
{
/* XXXX __WIN32__ => WIN32 */
#ifdef WIN32
    static int notty = -1;
    if (notty == -1) {
	HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
	notty = (handle == INVALID_HANDLE_VALUE) || (handle == 0) 
		|| (GetFileType(handle) != FILE_TYPE_CHAR);
    }
    if (notty) {
	MessageBox(NULL, msg, title, MB_OK | MB_ICONEXCLAMATION
		| MB_SYSTEMMODAL | MB_SETFOREGROUND | MB_TOPMOST);
    } else
#endif
    {
	Tcl_Channel errChannel = Tcl_GetChannel(interp, "stderr", NULL);
	if (errChannel != (Tcl_Channel) NULL) {
	    Tcl_Write(errChannel, (char *) msg, -1);
	    Tcl_Write(errChannel, "\n", 1);
	}
    }
}
