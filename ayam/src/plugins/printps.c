/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2014 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

#include "gl2ps.h"

/* printps.c - print PostScript plugin */

/* prototypes of functions local to this module: */

#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int Printps_Init(Tcl_Interp *interp);

int printps_convnptcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[]);

/* functions: */

/* printps_printtcmd:
 *  Tcl command to print to PostScript
 */
int
printps_printtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 struct Togl *togl = NULL;
 ay_view_object *view = NULL;
 ay_object *o = NULL;
 int i = 1;
 FILE *fp;
 int state = GL2PS_OVERFLOW, buffsize = 0;

  /* parse args */
  if(argc > 2)
    {
      while(i+1 < argc)
	{
	  if(!strcmp(argv[i], "-r"))
	    {
	      /*
	      mode = 0;
	      sscanf(argv[i+1], "%lg", &rmin);
	      sscanf(argv[i+2], "%lg", &rmax);
	      */
	    }
	  if(!strcmp(argv[i], "-d"))
	    {
	      /*
	      mode = 1;
	      sscanf(argv[i+1], "%lg", &mindist);
	      */
	    }
	  i += 2;
	}
    }

  o = ay_root->down;
  while(o && o->next)
    {
      if(o->type == AY_IDVIEW)
	{
	  i--;
	  if(i == 0)
	    {
	      view = (ay_view_object*)o->refine;
	      togl = view->togl;
	      break;
	    }
	}
      o = o->next;
    }

  if(!togl)
    {
      ay_error(AY_ERROR, "No view found!", NULL);
      return TCL_OK;
    }

    fp = fopen("out.eps", "wb");
    while(state == GL2PS_OVERFLOW)
      {
	buffsize += 1024*1024;
	gl2psBeginPage("test", "gl2psTestSimple", NULL, GL2PS_EPS,
		       GL2PS_SIMPLE_SORT,
		       GL2PS_DRAW_BACKGROUND | GL2PS_USE_CURRENT_VIEWPORT,
		       GL_RGBA, 0, NULL, 0, 0, 0, buffsize, fp, "out.eps");
	gl2psPointSize(4);
	gl2psLineWidth(0.8);
	ay_toglcb_display(togl);

	state = gl2psEndPage();
      }
    fclose(fp);

 return TCL_OK;
} /* printps_printtcmd */


/* Printps_Init:
 * initializes the printps module/plugin by registering a new
 * Tcl command
 */
#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int
Printps_Init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;
 char fname[] = "printps_init";

#ifdef WIN32
  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }
#endif /* WIN32 */

  Tcl_CreateCommand(interp, "printPS",
		    (Tcl_CmdProc*) printps_printtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

#if 0
  /* source printps.tcl, it contains Tcl-code */
  if((Tcl_EvalFile(interp, "printps.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname,
		  "Error while sourcing \"printps.tcl\"!");
       return TCL_OK;
     }
#endif

  ay_error(AY_EOUTPUT, fname,
	   "Plugin \"PrintPS\" successfully loaded.");

 return TCL_OK;
} /* Printps_Init */

