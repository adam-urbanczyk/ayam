/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2002 by Randolf Schultz
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
} /* ay_tcmd_reverttcmd */


/* ay_tcmd_showall:
 *  
 */
void
ay_tcmd_showall(ay_object *o)
{
 ay_object *down;

  if(o->down)
    {
      down = o->down;
      while(down)
	{
	  ay_tcmd_showall(down);
	  down = down->next;
	}
    }

  o->hide = AY_FALSE;

  return;
} /* ay_tcmd_showall */

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

 if(argc > 1)
   {
     if(!strcmp(argv[1], "-all"))
       {
	 o = ay_root;
	 while(o)
	   {
	     ay_tcmd_showall(o);
	     o = o->next;
	   }
	 return TCL_OK;
       }
   }

  while(sel)
    {
      o = sel->object;

      if(o)
	o->hide = AY_FALSE;

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_tcmd_showtcmd */


/* ay_tcmd_hideall:
 *  
 */
void
ay_tcmd_hideall(ay_object *o)
{
 ay_object *down;

  if(o->down)
    {
      down = o->down;
      while(down)
	{
	  ay_tcmd_hideall(down);
	  down = down->next;
	}
    }

  o->hide = AY_TRUE;

  return;
} /* ay_tcmd_hideall */


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

     if(!strcmp(argv[1], "-all"))
       {
	 o = ay_root;
	 while(o)
	   {
	     ay_tcmd_hideall(o);
	     o = o->next;
	   }
	 return TCL_OK;
       }
   } /* if */

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
} /* ay_tcmd_hidetcmd */


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

 return TCL_OK;
} /* ay_tcmd_getvertcmd */


/* ay_tcmd_getbppointtcmd:
 *  
 *  
 */
int
ay_tcmd_getbppointtcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;
 ay_bpatch_object *patch = NULL;
 int index = 0;
 double *p = NULL;
 char fname[] = "getBP";
 Tcl_Obj *to = NULL, *ton = NULL;

  if(argc != 5)
    {
      ay_error(AY_EARGS, fname, "index varx vary varz");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  while(sel)
    {
      src = sel->object;
      if(src->type != AY_IDBPATCH)
	{
	  ay_error(AY_ERROR, fname, "object is not a BPatch");
	  return TCL_OK;
	}
      else
	{
	  patch = (ay_bpatch_object*)src->refine;

	  Tcl_GetInt(interp, argv[1], &index);

	  if(index >= 4 || index < 0)
	    {
	      ay_error(AY_ERROR, fname, "index out of range");
	      return TCL_OK;
	    }

	  switch(index)
	    {
	    case 0:
	      p = patch->p1;
	      break;
	    case 1:
	      p = patch->p2;
	      break;
	    case 2:
	      p = patch->p3;
	      break;
	    case 3:
	      p = patch->p4;
	      break;
	    default:
	      p = patch->p1;
	      break;
	    } /* switch */

	  ton = Tcl_NewStringObj(argv[2],-1);
	  to = Tcl_NewDoubleObj(p[0]);
	  Tcl_ObjSetVar2(interp,ton,NULL,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY | TCL_PARSE_PART1);

	  Tcl_SetStringObj(ton,argv[3],-1);
	  to = Tcl_NewDoubleObj(p[1]);
	  Tcl_ObjSetVar2(interp,ton,NULL,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY | TCL_PARSE_PART1);

	  Tcl_SetStringObj(ton,argv[4],-1);
	  to = Tcl_NewDoubleObj(p[2]);
	  Tcl_ObjSetVar2(interp,ton,NULL,to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY | TCL_PARSE_PART1);
	  
	  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

	} /* if */

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_tcmd_getbppointtcmd */


/* ay_tcmd_setbppointtcmd:
 *  
 *  
 */
int
ay_tcmd_setbppointtcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;
 ay_bpatch_object *patch = NULL;
 double dtemp = 0.0;
 int index = 0;
 double *p = NULL;
 char fname[] = "setBP";

  if(argc != 5)
    {
      ay_error(AY_EARGS, fname, "index x y z");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  while(sel)
    {
      src = sel->object;
      if(src->type != AY_IDBPATCH)
	{
	  ay_error(AY_ERROR, fname, "object is not a BPatch");
	  return TCL_OK;
	}
      else
	{
	  patch = (ay_bpatch_object*)src->refine;

	  Tcl_GetInt(interp, argv[1], &index);

	  if(index >= 4 || index < 0)
	    {
	      ay_error(AY_ERROR, fname, "index out of range");
	      return TCL_OK;
	    }

	  switch(index)
	    {
	    case 0:
	      p = patch->p1;
	      break;
	    case 1:
	      p = patch->p2;
	      break;
	    case 2:
	      p = patch->p3;
	      break;
	    case 3:
	      p = patch->p4;
	      break;
	    default:
	      p = patch->p1;
	      break;
	    } /* switch */

	  Tcl_GetDouble(interp, argv[2], &dtemp);
	  p[0] = dtemp;

	  Tcl_GetDouble(interp, argv[3], &dtemp);
	  p[1] = dtemp;

	  Tcl_GetDouble(interp, argv[4], &dtemp);
	  p[2] = dtemp;

	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_tcmd_setbppointtcmd */
