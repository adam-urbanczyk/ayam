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

char printps_version_ma[] = AY_VERSIONSTR;
char printps_version_mi[] = AY_VERSIONSTRMI;

/* prototypes of functions local to this module: */

#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int Printps_Init(Tcl_Interp *interp);

int printps_parseformat(char *filename);

int printps_printtcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[]);

/* functions: */

/* printps_parseformat:
 *  parse extension of filename into output format type
 */
int
printps_parseformat(char *filename)
{
 int format = GL2PS_EPS;
 char *ext;

  if(!filename)
    goto cleanup;

  ext = strrchr(filename, '.');
  if(!ext)
    goto cleanup;

  switch(ext[1])
    {
    case 'p':
      if(ext[2] == 'd')
	format = GL2PS_PDF;
      else
	format = GL2PS_PS;
      break;
    case 's':
      format = GL2PS_SVG;
      break;
    case 't':
      format = GL2PS_TEX;
      break;
    default:
      break;
    }

cleanup:
 return format;
} /* printps_parseformat */


/* printps_printtcmd:
 *  Tcl command to print to PostScript
 */
int
printps_printtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 int tcl_status = TCL_OK;
 struct Togl *togl = NULL;
 ay_view_object *view = NULL;
 ay_object *o = NULL;
 int i = 1;
 char *title = NULL;
 char *producer = "Ayam";
 char *filename = "out.eps";
 FILE *fp;
 int state = GL2PS_OVERFLOW, buffsize = 0;
 int format = GL2PS_EPS;

  /* parse args */
  if(argc > 2)
    {
      while(i+1 < argc)
	{
	  if(!strcmp(argv[i], "-v"))
	    {
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &i);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	    }
	  if(!strcmp(argv[i], "-f"))
	    {
	      filename = argv[i+1];
	      format = printps_parseformat(filename);
	    }
	  if(!strcmp(argv[i], "-p"))
	    {
	      producer = argv[i+1];
	    }
	  if(!strcmp(argv[i], "-t"))
	    {
	      title = argv[i+1];
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
	      if(!title)
		title = o->name;
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

    fp = fopen(filename, "wb");
    if(!fp)
      {
	ay_error(AY_EOPENFILE, argv[0], filename);
	return TCL_OK;
      }

    Togl_MakeCurrent(togl);

    while(state == GL2PS_OVERFLOW)
      {
	buffsize += 1024*1024;
	gl2psBeginPage(title, producer, NULL, format,
		       GL2PS_SIMPLE_SORT,
		       GL2PS_DRAW_BACKGROUND | GL2PS_USE_CURRENT_VIEWPORT,
		       GL_RGBA, 0, NULL, 0, 0, 0, buffsize, fp, filename);

	gl2psPointSize(4);
	gl2psLineWidth(0.8);

	ay_toglcb_display(togl);

	state = gl2psEndPage();
      }

    if(ay_currentview)
      Togl_MakeCurrent(ay_currentview->togl);

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

  if(ay_checkversion(fname, printps_version_ma, printps_version_mi))
    {
      return TCL_ERROR;
    }

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

