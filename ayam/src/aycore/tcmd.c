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
  char *glver = NULL, *glven = NULL, *glren = NULL, *gluver = NULL,
    *gluext = NULL, *glext = NULL;
  char arr[] = "ay";

  glver = (char *)glGetString(GL_VERSION);
  if(glver)
    Tcl_SetVar2(interp, arr, "gl_ver", glver, TCL_GLOBAL_ONLY |
		TCL_LEAVE_ERR_MSG);

  glven = (char *)glGetString(GL_VENDOR);
  if(glven)
    Tcl_SetVar2(interp, arr, "gl_ven", glven, TCL_GLOBAL_ONLY |
		TCL_LEAVE_ERR_MSG);

  glren = (char *)glGetString(GL_RENDERER);
  if(glren)
    Tcl_SetVar2(interp, arr, "gl_ren", glren, TCL_GLOBAL_ONLY |
		TCL_LEAVE_ERR_MSG);

  glext = (char *)glGetString(GL_EXTENSIONS);
  if(glext)
    Tcl_SetVar2(interp, arr, "gl_ext", glext, TCL_GLOBAL_ONLY |
		TCL_LEAVE_ERR_MSG);

  gluver = (char *)gluGetString(GLU_VERSION);
  if(gluver)
    Tcl_SetVar2(interp, arr, "glu_ver", gluver, TCL_GLOBAL_ONLY |
		TCL_LEAVE_ERR_MSG);

  gluext = (char *)gluGetString(GLU_EXTENSIONS);
  if(gluext)
    Tcl_SetVar2(interp, arr, "glu_ext", gluext, TCL_GLOBAL_ONLY |
		TCL_LEAVE_ERR_MSG);

 return TCL_OK;
} /* ay_tcmd_getvertcmd */

/* ay_tcmd_getbppntfromindex:
 *  
 *  
 */
int
ay_tcmd_getbppntfromindex(ay_bpatch_object *patch, int index,
			  double **p)
{
 char fname[] = "ay_tcmd_getbppntfromindex";

  if(index >= 4 || index < 0)
    {
      ay_error(AY_ERROR, fname, "index out of range");
      return TCL_OK;
    }

  switch(index)
    {
    case 0:
      *p = patch->p1;
      break;
    case 1:
      *p = patch->p2;
      break;
    case 2:
      *p = patch->p3;
      break;
    case 3:
      *p = patch->p4;
      break;
    default:
      *p = patch->p1;
      break;
    } /* switch */

 return AY_OK;
} /* ay_tcmd_getbppntfromindex */


/* ay_tcmd_getpointtcmd:
 *  
 *  
 */
int
ay_tcmd_getpointtcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;
 int indexu = 0, indexv = 0, i = 1, j = 0, argc2 = argc;
 int homogenous = AY_FALSE, trafo = AY_FALSE;
 double *p = NULL, *tp = NULL, tmp[4] = {0};
 double m[16];
 char fname[] = "getPnt";
 Tcl_Obj *to = NULL, *ton = NULL;

  if(argc <= 1)
    {
      ay_error(AY_EARGS, fname,
	       "\\[-trafo\\] (index | indexu indexv) varx vary varz \\[varw\\]");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  if(!strcmp(argv[1], "-trafo"))
    {
      trafo = AY_TRUE;
      i++;
      argc2--;
    }
  
  while(sel)
    {
      src = sel->object;
      p = NULL;
      homogenous = AY_FALSE;
      switch(src->type)
	{
	case AY_IDNCURVE:
	  if(argc2 < 6)
	    {
	      ay_error(AY_EARGS, fname, "\\[-trafo\\] index varx vary varz varw");
	      return TCL_OK;
	    }
	  Tcl_GetInt(interp, argv[i], &indexu);
	  ay_nct_getpntfromindex((ay_nurbcurve_object*)(src->refine),
				 indexu, &p);
	  homogenous = AY_TRUE;
	  j = i+1;
	  break;
	case AY_IDICURVE:
	  if(argc2 < 5)
	    {
	      ay_error(AY_EARGS, fname, "\\[-trafo\\] index varx vary varz");
	      return TCL_OK;
	    }
	  Tcl_GetInt(interp, argv[i], &indexu);
	  ay_ict_getpntfromindex((ay_icurve_object*)(src->refine),
				 indexu, &p);
	  homogenous = AY_FALSE;
	  j = i+1;
	  break;
	case AY_IDNPATCH:
	  if(argc2 < 7)
	    {
	      ay_error(AY_EARGS, fname,
		       "\\[-trafo\\] indexu indexv varx vary varz varw");
	      return TCL_OK;
	    }
	  Tcl_GetInt(interp, argv[i], &indexu);
	  Tcl_GetInt(interp, argv[i+1], &indexv);
	  ay_npt_getpntfromindex((ay_nurbpatch_object*)(src->refine),
				 indexu, indexv, &p);
	  homogenous = AY_TRUE;
	  j = i+2;
	  break;
	case AY_IDBPATCH:
	  if(argc2 < 5)
	    {
	      ay_error(AY_EARGS, fname, "\\[-trafo\\] index varx vary varz");
	      return TCL_OK;
	    }
	  Tcl_GetInt(interp, argv[i], &indexu);
	  ay_tcmd_getbppntfromindex((ay_bpatch_object*)(src->refine),
				    indexu, &p);
	  homogenous = AY_FALSE;
	  j = i+1;
	  break;
	case AY_IDPAMESH:
	  if(argc2 < 7)
	    {
	      ay_error(AY_EARGS, fname,
		       "\\[-trafo\\] indexu indexv varx vary varz varw");
	      return TCL_OK;
	    }
	  Tcl_GetInt(interp, argv[i], &indexu);
	  Tcl_GetInt(interp, argv[i+1], &indexv);
	  ay_pmt_getpntfromindex((ay_pamesh_object*)(src->refine),
				 indexu, indexv, &p);
	  homogenous = AY_TRUE;
	  j = i+2;
	  break;

	default:
	  ay_error(AY_EWARN, fname,
		   "don't know how to get point from this object");
	  break;
	} /* switch */

      if(p)
	{
	  /* apply trafos? */
	  if(trafo)
	    { /* Yes! */
	      ay_trafo_creatematrix(src, m);

	      if(homogenous)
		{
		  memcpy(tmp, p, 4*sizeof(double));
		  ay_trafo_apply4(tmp, m);
		}
	      else
		{
		  memcpy(tmp, p, 3*sizeof(double));
		  ay_trafo_apply3(tmp, m);
		}

	      tp = tmp;
	    }
	  else
	    { /* No! */
	      tp = p;
	    }
	    
	  ton = Tcl_NewStringObj(argv[j], -1);
	  to = Tcl_NewDoubleObj(tp[0]);
	  Tcl_ObjSetVar2(interp, ton, NULL, to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY | TCL_PARSE_PART1);

	  Tcl_SetStringObj(ton, argv[j+1], -1);
	  to = Tcl_NewDoubleObj(tp[1]);
	  Tcl_ObjSetVar2(interp, ton, NULL, to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY | TCL_PARSE_PART1);

	  Tcl_SetStringObj(ton, argv[j+2], -1);
	  to = Tcl_NewDoubleObj(tp[2]);
	  Tcl_ObjSetVar2(interp, ton, NULL, to, TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY | TCL_PARSE_PART1);
	  if(homogenous)
	    {
	      Tcl_SetStringObj(ton, argv[j+3], -1);
	      to = Tcl_NewDoubleObj(tp[3]);
	      Tcl_ObjSetVar2(interp, ton, NULL, to, TCL_LEAVE_ERR_MSG |
			     TCL_GLOBAL_ONLY | TCL_PARSE_PART1);
	    }

	  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
	} /* if */

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_tcmd_getpointtcmd */


/* ay_tcmd_setpointtcmd:
 *  
 *  
 */
int
ay_tcmd_setpointtcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;
 double dtemp = 0.0;
 int indexu = 0, indexv = 0, i = 0, homogenous = AY_FALSE;
 double *p = NULL;
 char fname[] = "setPnt";

  if(argc <= 1)
    {
      ay_error(AY_EARGS, fname, "(index | indexu indexv) x y z \\[w\\]");
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
      p = NULL;
      homogenous = AY_FALSE;
      switch(src->type)
	{
	case AY_IDNCURVE:
	  if(argc < 6)
	    {
	      ay_error(AY_EARGS, fname, "index x y z w");
	      return TCL_OK;
	    }
	  Tcl_GetInt(interp, argv[1], &indexu);
	  ay_nct_getpntfromindex((ay_nurbcurve_object*)(src->refine),
				 indexu, &p);
	  homogenous = AY_TRUE;
	  i = 2;
	  break;
	case AY_IDICURVE:
	  if(argc < 5)
	    {
	      ay_error(AY_EARGS, fname, "index x y z");
	      return TCL_OK;
	    }
	  Tcl_GetInt(interp, argv[1], &indexu);
	  ay_ict_getpntfromindex((ay_icurve_object*)(src->refine),
				 indexu, &p);
	  homogenous = AY_FALSE;
	  i = 2;
	  break;
	case AY_IDNPATCH:
	  if(argc < 7)
	    {
	      ay_error(AY_EARGS, fname, "indexu indexv x y z w");
	      return TCL_OK;
	    }
	  Tcl_GetInt(interp, argv[1], &indexu);
	  Tcl_GetInt(interp, argv[2], &indexv);
	  ay_npt_getpntfromindex((ay_nurbpatch_object*)(src->refine),
				 indexu, indexv, &p);
	  homogenous = AY_TRUE;
	  i = 3;
	  break;
	case AY_IDBPATCH:
	  if(argc < 5)
	    {
	      ay_error(AY_EARGS, fname, "index x y z");
	      return TCL_OK;
	    }
	  Tcl_GetInt(interp, argv[1], &indexu);
	  ay_tcmd_getbppntfromindex((ay_bpatch_object*)(src->refine),
				    indexu, &p);
	  homogenous = AY_FALSE;
	  i = 2;
	  break;
	case AY_IDPAMESH:
	  if(argc < 6)
	    {
	      ay_error(AY_EARGS, fname, "indexu indexv x y z w");
	      return TCL_OK;
	    }
	  Tcl_GetInt(interp, argv[1], &indexu);
	  Tcl_GetInt(interp, argv[2], &indexv);
	  ay_pmt_getpntfromindex((ay_pamesh_object*)(src->refine),
				 indexu, indexv, &p);
	  homogenous = AY_TRUE;
	  i = 3;
	  break;
	default:
	  ay_error(AY_EWARN, fname,
		   "don't know how to set point of this object");
	  break;
	} /* switch */

      if(p)
	{
	  Tcl_GetDouble(interp, argv[i], &dtemp);
	  p[0] = dtemp;

	  Tcl_GetDouble(interp, argv[i+1], &dtemp);
	  p[1] = dtemp;

	  Tcl_GetDouble(interp, argv[i+2], &dtemp);
	  p[2] = dtemp;

	  if(homogenous)
	    {
	      Tcl_GetDouble(interp, argv[i+3], &dtemp);
	      p[3] = dtemp;
	    } /* if */
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_tcmd_setpointtcmd */


/* ay_tcmd_getglerrortcmd:
 *  
 *  
 */
int
ay_tcmd_getglerrortcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{
 char fname[] = "getGLErr";

  ay_error(AY_EOUTPUT, fname, (char *)gluErrorString(glGetError()));

 return TCL_OK;
} /* ay_tcmd_getglerrortcmd */
