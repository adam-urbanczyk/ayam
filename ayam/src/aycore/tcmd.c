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

/* tcmd.c - various simple tcl commands */

/* ay_tcmd_reverttcmd:
 *  revert selected curves
 */
int
ay_tcmd_reverttcmd(ClientData clientData, Tcl_Interp * interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "revert";
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_icurve_object *icurve = NULL;
 ay_nurbcurve_object *ncurve = NULL;

  while(sel)
    {
      o = sel->object;

      switch(o->type)
	{
	case AY_IDICURVE:

	  if(o->selp)
	    {
	      ay_selp_clear(o);
	    }

	  icurve = (ay_icurve_object*)o->refine;

	  ay_status = ay_ict_revert(icurve);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, "Could not revert ICurve!");
	    }
	  
	  break;
	case AY_IDNCURVE:

	  if(o->selp)
	    {
	      ay_selp_clear(o);
	    }

	  ncurve = (ay_nurbcurve_object*)o->refine;

	  ay_status = ay_nct_revert(ncurve);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, "Could not revert NCurve!");
	    }
	  break;
	default:
	  break;
	} /* switch */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
}


/* ay_tcmd_showtcmd:
 *  
 */
int
ay_tcmd_showtcmd(ClientData clientData, Tcl_Interp * interp,
		  int argc, char *argv[])
{
  /*
 int ay_status = AY_OK;
 char fname[] = "show";
  */
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;

  while(sel)
    {
      o = sel->object;

      if(o)
	o->hide = AY_FALSE;

      sel = sel->next;
    } /* while */

 return TCL_OK;
}

/* ay_tcmd_hidetcmd:
 *  
 */
int
ay_tcmd_hidetcmd(ClientData clientData, Tcl_Interp * interp,
		  int argc, char *argv[])
{
  /*  
 int ay_status = AY_OK;
 char fname[] = "hide";
  */
 int toggle = AY_FALSE;

 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;

 if(argc>1)
   {
     if(!strcmp(argv[1],"-toggle"))
       {
	 toggle = AY_TRUE;
       }
   }

  while(sel)
    {
      o = sel->object;

      if(o)
	{
	  if(toggle)
	    {
	      if(o->hide)
		{
		  o->hide = AY_FALSE;
		}
	      else
		{
		  o->hide = AY_TRUE;
		}
	    }
	  else
	    {
	      o->hide = AY_TRUE;
	    }
	}
      sel = sel->next;
    } /* while */

 return TCL_OK;
}

/* ay_tcmd_getvertcmd:
 *  get all version information from the current OpenGL/GLU and Ayam
 */
int
ay_tcmd_getvertcmd(ClientData clientData, Tcl_Interp * interp,
		   int argc, char *argv[])
{
  char *glver, *glven, *glren, *gluver, *gluext, *glext;
  char arr[] = "ay";

  glver = (char *)glGetString(GL_VERSION);
  Tcl_SetVar2(interp, arr, "gl_ver", glver, TCL_GLOBAL_ONLY |
	      TCL_LEAVE_ERR_MSG);

  glven = (char *)glGetString(GL_VENDOR);
  Tcl_SetVar2(interp, arr, "gl_ven", glven, TCL_GLOBAL_ONLY |
	     TCL_LEAVE_ERR_MSG);

  glren = (char *)glGetString(GL_RENDERER);
  Tcl_SetVar2(interp, arr, "gl_ren", glren, TCL_GLOBAL_ONLY |
	     TCL_LEAVE_ERR_MSG);

  glext = (char *)glGetString(GL_EXTENSIONS);
  Tcl_SetVar2(interp, arr, "gl_ext", glext, TCL_GLOBAL_ONLY |
	     TCL_LEAVE_ERR_MSG);

  gluver = (char *)gluGetString(GLU_VERSION);
  Tcl_SetVar2(interp, arr, "glu_ver", gluver, TCL_GLOBAL_ONLY |
	     TCL_LEAVE_ERR_MSG);

  gluext = (char *)gluGetString(GLU_EXTENSIONS);
  Tcl_SetVar2(interp, arr, "glu_ext", gluext, TCL_GLOBAL_ONLY |
	     TCL_LEAVE_ERR_MSG);

  Tcl_SetVar2(interp, arr, "ay_version", AYVERSIONSTR, TCL_GLOBAL_ONLY |
	     TCL_LEAVE_ERR_MSG);

 return TCL_OK;
} /* ay_tcmd_getvertcmd */
