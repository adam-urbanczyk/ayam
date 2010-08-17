/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2002 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
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
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_acurve_object *acurve = NULL;
 ay_icurve_object *icurve = NULL;
 ay_nurbcurve_object *ncurve = NULL;

  while(sel)
    {
      o = sel->object;

      switch(o->type)
	{
	case AY_IDACURVE:

	  if(o->selp)
	    {
	      ay_selp_clear(o);
	    }

	  acurve = (ay_acurve_object*)o->refine;

	  ay_status = ay_act_revert(acurve);
	  if(ay_status)
	    {
	      ay_error(ay_status, argv[0], "Could not revert ACurve!");
	    }

	  ay_notify_force(o);

	  o->modified = AY_TRUE;

	  break;
	case AY_IDICURVE:

	  if(o->selp)
	    {
	      ay_selp_clear(o);
	    }

	  icurve = (ay_icurve_object*)o->refine;

	  ay_status = ay_ict_revert(icurve);
	  if(ay_status)
	    {
	      ay_error(ay_status, argv[0], "Could not revert ICurve!");
	    }

	  ay_notify_force(o);

	  o->modified = AY_TRUE;

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
	      ay_error(ay_status, argv[0], "Could not revert NCurve!");
	    }

	  o->modified = AY_TRUE;

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
 *  _recursively_ set the hidden flag of all children of object
 *  <o> and <o> to false (show the objects)
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
 *  _recursively_ set the hidden flag of all children of object
 *  <o> and <o> to true (hide the objects)
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
 char fname[] = "tcmd_getbppntfromindex";

  if(index >= 4 || index < 0)
    {
      ay_error(AY_ERROR, fname, "index out of range");
      return AY_ERROR;
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
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_point *old_selp = NULL, *selp = NULL;
 ay_nurbcurve_object *nc = NULL;
 ay_nurbpatch_object *np = NULL;
 ay_object *o = NULL, *po = NULL;
 int indexu = 0, indexv = 0, i = 1, j = 1, argc2 = argc;
 int homogenous = AY_FALSE, apply_trafo = AY_FALSE;
 int to_world = AY_FALSE, eval = AY_FALSE;
 int handled = AY_FALSE, freepo = AY_FALSE;
 int clear_selp = AY_FALSE;
 double *p = NULL, *tp = NULL, tmp[4] = {0}, utmp[4] = {0};
 double m[16], u = 0.0, v = 0.0;
 char fargs[] = "[-trafo|-world|-eval] (index | indexu indexv | u | u v) varx vary varz [varw]";
 Tcl_Obj *to = NULL, *ton = NULL;
 ay_voidfp *arr = NULL;
 ay_getpntcb *cb = NULL;

  if(argc <= 1)
    {
      ay_error(AY_EARGS, argv[0], fargs);
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while((j < 3) && (j < argc))
    {
      if(argv[j][0] == '-' && argv[j][1] == 't')
	{
	  /* -trafo */
	  apply_trafo = AY_TRUE;
	  i++;
	  argc2--;
	}
      if(argv[j][0] == '-' && argv[j][1] == 'w')
	{
	  /* -world */
	  apply_trafo = AY_TRUE;
	  to_world = AY_TRUE;
	  i++;
	  argc2--;
	}
      if(argv[j][0] == '-' && argv[j][1] == 'e')
	{
	  /* -eval */
	  eval = AY_TRUE;
	  i++;
	  argc2--;
	}
      if(argv[j][0] == '-' && argv[j][1] == 'p')
	{
	  /* -param */
	  eval = AY_TRUE;
	  i++;
	  argc2--;
	}
      j++;
    } /* while */

  j = 0;
  while(sel)
    {
      o = sel->object;
      p = NULL;
      homogenous = AY_FALSE;
      freepo = AY_FALSE;
      switch(o->type)
	{
	case AY_IDNCURVE:
	  if(argc2 < 6)
	    {
	      ay_error(AY_EARGS, argv[0], fargs);
	      return TCL_OK;
	    }
	  if(!eval)
	    {
	      tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	      ay_nct_getpntfromindex((ay_nurbcurve_object*)(o->refine),
				     indexu, &p);
	      homogenous = AY_TRUE;
	    }
	  else
	    {
	      tcl_status = Tcl_GetDouble(interp, argv[i], &u);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	      p = utmp;
	      nc = (ay_nurbcurve_object *)(o->refine);

	      /* check parameter */
	      if((u < nc->knotv[nc->order-1]) ||
		 (u > nc->knotv[nc->length]))
		{
		  ay_error(AY_ERROR, argv[0], "Parameter out of range.");
		  return TCL_OK;
		}

	      /* evaluate the curve */
	      if(nc->is_rat)
		{
		  ay_status = ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
						 nc->knotv, nc->controlv,
						 u, p);
		}
	      else
		{
		  ay_status = ay_nb_CurvePoint3D(nc->length-1, nc->order-1,
						 nc->knotv, nc->controlv,
						 u, p);
		}

	      if(ay_status)
		{
		  ay_error(AY_ERROR, argv[0], "Evaluation failed.");
		  return TCL_OK;
		}
	      homogenous = AY_FALSE;
	    } /* if */
	  j = i+1;
	  break;
	case AY_IDACURVE:
	  if(argc2 < 5)
	    {
	      ay_error(AY_EARGS, argv[0], fargs);
	      return TCL_OK;
	    }
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_act_getpntfromindex((ay_acurve_object*)(o->refine),
				 indexu, &p);
	  homogenous = AY_FALSE;
	  j = i+1;
	  break;
	case AY_IDICURVE:
	  if(argc2 < 5)
	    {
	      ay_error(AY_EARGS, argv[0], fargs);
	      return TCL_OK;
	    }
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_ict_getpntfromindex((ay_icurve_object*)(o->refine),
				 indexu, &p);
	  homogenous = AY_FALSE;
	  j = i+1;
	  break;
	case AY_IDNPATCH:
	  if(argc2 < 7)
	    {
	      ay_error(AY_EARGS, argv[0], fargs);
	      return TCL_OK;
	    }
	  if(!eval)
	    {
	      tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &indexv);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	      ay_npt_getpntfromindex((ay_nurbpatch_object*)(o->refine),
				     indexu, indexv, &p);
	      homogenous = AY_TRUE;
	    }
	  else
	    {
	      tcl_status = Tcl_GetDouble(interp, argv[i], &u);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	      tcl_status = Tcl_GetDouble(interp, argv[i+1], &v);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	      p = utmp;
	      np = (ay_nurbpatch_object *)(o->refine);

	      /* check parameters */
	      if((u < np->uknotv[np->uorder-1]) ||
		 (u > np->uknotv[np->width]))
		{
		  ay_error(AY_ERROR, argv[0], "Parameter u out of range.");
		  return TCL_OK;
		}

	      if((v < np->vknotv[np->vorder-1]) ||
		 (v > np->vknotv[np->height]))
		{
		  ay_error(AY_ERROR, argv[0], "Parameter v out of range.");
		  return TCL_OK;
		}

	      /* evaluate the patch */
	      if(np->is_rat)
		{
		  ay_status = ay_nb_SurfacePoint4D(np->width-1, np->height-1,
						   np->uorder-1, np->vorder-1,
						   np->uknotv, np->vknotv,
						   np->controlv, u, v, p);
		}
	      else
		{
		  ay_status = ay_nb_SurfacePoint3D(np->width-1, np->height-1,
						   np->uorder-1, np->vorder-1,
						   np->uknotv, np->vknotv,
						   np->controlv, u, v, p);
		}

	      if(ay_status)
		{
		  ay_error(AY_ERROR, argv[0], "Evaluation failed.");
		  return TCL_OK;
		}
	      homogenous = AY_FALSE;
	    } /* if */
	  j = i+2;
	  break;
	case AY_IDBPATCH:
	  if(argc2 < 5)
	    {
	      ay_error(AY_EARGS, argv[0], fargs);
	      return TCL_OK;
	    }
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_tcmd_getbppntfromindex((ay_bpatch_object*)(o->refine),
				    indexu, &p);
	  homogenous = AY_FALSE;
	  j = i+1;
	  break;
	case AY_IDPAMESH:
	  if(argc2 < 7)
	    {
	      ay_error(AY_EARGS, argv[0], fargs);
	      return TCL_OK;
	    }
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  tcl_status = Tcl_GetInt(interp, argv[i+1], &indexv);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_pmt_getpntfromindex((ay_pamesh_object*)(o->refine),
				 indexu, indexv, &p);
	  homogenous = AY_TRUE;
	  j = i+2;
	  break;

	default:
	  handled = AY_FALSE;

	  if(!eval)
	    {
	      arr = ay_getpntcbt.arr;
	      cb = (ay_getpntcb *)(arr[o->type]);
	      if(cb)
		{
		  if(!(selp = calloc(1, sizeof(ay_point))))
		    {
		      ay_error(AY_EOMEM, argv[0], NULL);
		      goto cleanup;
		    }
		  old_selp = o->selp;
		  o->selp = selp;
		  clear_selp = AY_TRUE;
		  ay_status = ay_tcmd_getuint(argv[i], &selp->index);
		  if(ay_status)
		    {
		      goto cleanup;
		    }
		  selp->index = (unsigned int)indexu;
		  ay_status = cb(3, o, NULL, NULL);
		  if(ay_status || (!o->selp))
		    {
		      ay_error(AY_ERROR, argv[0], "getpntcb failed");
		      goto cleanup;
		    }
		  p = selp->point;
		  homogenous = selp->homogenous;
		  free(selp);
		  o->selp = old_selp;
		  clear_selp = AY_FALSE;
		  handled = AY_TRUE;
		  j = i+1;
		}
	    }

	  if(!handled)
	    {
	      if(argc2 < 7)
		{
		  po = NULL;
		  ay_provide_object(o, AY_IDNCURVE, &po);
		  if(po)
		    {
		      freepo = AY_TRUE;
		      if(argc2 < 6)
			{
			  ay_error(AY_EARGS, argv[0], fargs);
			  goto cleanup;
			}
		      if(!eval)
			{
			  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
			  AY_CHTCLERRGOT(tcl_status, argv[0], interp);
			  ay_nct_getpntfromindex((ay_nurbcurve_object*)
						 (po->refine),
						 indexu, &p);
			  homogenous = AY_TRUE;
			}
		      else
			{
			  tcl_status = Tcl_GetDouble(interp, argv[i], &u);
			  p = utmp;
			  nc = (ay_nurbcurve_object *)(po->refine);
			  ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
					     nc->knotv, nc->controlv, u, p);
			  homogenous = AY_FALSE;
			} /* if */
		      j = i+1;
		      handled = AY_TRUE;
		    } /* if */
		} /* if */

	      if(argc2 == 7)
		{
		  po = NULL;
		  ay_provide_object(o, AY_IDNPATCH, &po);
		  if(po)
		    {
		      freepo = AY_TRUE;
		      if(!eval)
			{
			  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
			  AY_CHTCLERRGOT(tcl_status, argv[0], interp);
			  tcl_status = Tcl_GetInt(interp, argv[i+1], &indexv);
			  AY_CHTCLERRGOT(tcl_status, argv[0], interp);
			  ay_npt_getpntfromindex((ay_nurbpatch_object*)
						 (po->refine),
						 indexu, indexv, &p);
			  homogenous = AY_TRUE;
			}
		      else
			{
			  tcl_status = Tcl_GetDouble(interp, argv[i], &u);
			  AY_CHTCLERRGOT(tcl_status, argv[0], interp);
			  tcl_status = Tcl_GetDouble(interp, argv[i+1], &v);
			  AY_CHTCLERRGOT(tcl_status, argv[0], interp);
			  p = utmp;
			  np = (ay_nurbpatch_object *)(po->refine);
			  ay_nb_SurfacePoint4D(np->width-1, np->height-1,
					       np->uorder-1, np->vorder-1,
					       np->uknotv, np->vknotv,
					       np->controlv, u, v, p);
			  homogenous = AY_FALSE;
			} /* if */
		      j = i+2;
		      handled = AY_TRUE;
		    } /* if */
		} /* if */
	    } /* if */
	  if(!handled)
	    {
	      ay_error(AY_EWARN, argv[0],
		       "do not know how to get point from this object");
	    }
	  break;
	} /* switch */

      if(p)
	{
	  /* apply trafos? */
	  if(apply_trafo)
	    { /* Yes! */
	      if(to_world)
		{
		  glMatrixMode(GL_MODELVIEW);
		  glPushMatrix();
		   glLoadIdentity();
		   if(ay_currentlevel->object != ay_root)
		     {
		       ay_trafo_getall(ay_currentlevel->next);
		     }
		   glTranslated(o->movx, o->movy, o->movz);
		   ay_quat_torotmatrix(o->quat, m);
		   glMultMatrixd((GLdouble*)m);
		   glScaled(o->scalx, o->scaly, o->scalz);
		   glGetDoublev(GL_MODELVIEW_MATRIX, m);
		  glPopMatrix();
		}
	      else
		{
		  ay_trafo_creatematrix(o, m);
		}

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

      if(freepo)
	{
	  if(po)
	    ay_object_deletemulti(po);
	} /* if */

      sel = sel->next;
    } /* while */

cleanup:

  if(freepo)
    {
      if(po)
	ay_object_deletemulti(po);
    } /* if */

  if(clear_selp)
    {
      o->selp = old_selp;
    }

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
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_point *old_selp = NULL, *selp = NULL;
 double dtemp = 0.0;
 int indexu = 0, indexv = 0, i = 1, homogenous = AY_FALSE;
 int from_world = AY_FALSE, clear_selp = AY_FALSE, handled = AY_FALSE;
 double *p = NULL;
 ay_voidfp *arr = NULL;
 ay_getpntcb *cb = NULL;
 double m[16], mi[16];

  if(argc <= 1)
    {
      ay_error(AY_EARGS, argv[0],
	       "[-world] (index | indexu indexv) x y z [w]");
      return TCL_OK;
    }

  if(argv[1][0] == '-' && argv[1][1] == 'w')
    {
      /* -world */
      from_world = AY_TRUE;
      i++;
      argc--;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(from_world)
    {
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      if(ay_currentlevel->object != ay_root)
	ay_trafo_getall(ay_currentlevel->next);
    }

  while(sel)
    {
      o = sel->object;
      p = NULL;
      homogenous = AY_FALSE;
      clear_selp = AY_FALSE;
      handled = AY_FALSE;
      switch(o->type)
	{
	case AY_IDNCURVE:
	  if(argc < 5)
	    {
	      ay_error(AY_EARGS, argv[0], "index x y z [w]");
	      return TCL_OK;
	    }
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_nct_getpntfromindex((ay_nurbcurve_object*)(o->refine),
				 indexu, &p);
	  homogenous = AY_TRUE;
	  i++;
	  break;
	case AY_IDACURVE:
	  if(argc < 5)
	    {
	      ay_error(AY_EARGS, argv[0], "index x y z");
	      return TCL_OK;
	    }
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_act_getpntfromindex((ay_acurve_object*)(o->refine),
				 indexu, &p);
	  i++;
	  break;
	case AY_IDICURVE:
	  if(argc < 5)
	    {
	      ay_error(AY_EARGS, argv[0], "index x y z");
	      return TCL_OK;
	    }
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_ict_getpntfromindex((ay_icurve_object*)(o->refine),
				 indexu, &p);
	  i++;
	  break;
	case AY_IDNPATCH:
	  if(argc < 6)
	    {
	      ay_error(AY_EARGS, argv[0], "indexu indexv x y z [w]");
	      return TCL_OK;
	    }
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  tcl_status = Tcl_GetInt(interp, argv[i+1], &indexv);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_npt_getpntfromindex((ay_nurbpatch_object*)(o->refine),
				 indexu, indexv, &p);
	  homogenous = AY_TRUE;
	  i += 2;
	  break;
	case AY_IDBPATCH:
	  if(argc < 5)
	    {
	      ay_error(AY_EARGS, argv[0], "index x y z");
	      return TCL_OK;
	    }
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_tcmd_getbppntfromindex((ay_bpatch_object*)(o->refine),
				    indexu, &p);
	  i++;
	  break;
	case AY_IDPAMESH:
	  if(argc < 5)
	    {
	      ay_error(AY_EARGS, argv[0], "indexu indexv x y z [w]");
	      return TCL_OK;
	    }
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  tcl_status = Tcl_GetInt(interp, argv[i+1], &indexv);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_pmt_getpntfromindex((ay_pamesh_object*)(o->refine),
				 indexu, indexv, &p);
	  homogenous = AY_TRUE;
	  i += 2;
	  break;
	default:
	  if(argc < 4)
	    {
	      ay_error(AY_EARGS, argv[0], "index x y z [w]");
	      return TCL_OK;
	    }
	  handled = AY_FALSE;
	  arr = ay_getpntcbt.arr;
	  cb = (ay_getpntcb *)(arr[o->type]);
	  if(cb)
	    {
	      if(!(selp = calloc(1, sizeof(ay_point))))
		{
		  ay_error(AY_EOMEM, argv[0], NULL);
		  return TCL_OK;
		}
	      old_selp = o->selp;
	      o->selp = selp;
	      clear_selp = AY_TRUE;
	      ay_status = ay_tcmd_getuint(argv[i], &selp->index);
	      if(ay_status)
		{
		  goto cleanup;
		}
	      ay_status = cb(3, o, NULL, NULL);
	      if(ay_status || (!o->selp))
		{
		  ay_error(AY_ERROR, argv[0], "getpntcb failed");
		  goto cleanup;
		}
	      if(o->selp->readonly)
		{
		  ay_error(AY_ERROR, argv[0], "points are readonly");
		  goto cleanup;
		}
	      p = selp->point;
	      homogenous = selp->homogenous;
	      handled = AY_TRUE;
	      i++;
	    }
	  if(!handled)
	    {
	      ay_error(AY_EWARN, argv[0],
		       "do not know how to set point of this object");
	    }
	  break;
	} /* switch */

      if(p)
	{
	  tcl_status = Tcl_GetDouble(interp, argv[i], &dtemp);
	  AY_CHTCLERRGOT(tcl_status, argv[0], interp);
	  p[0] = dtemp;

	  tcl_status = Tcl_GetDouble(interp, argv[i+1], &dtemp);
	  AY_CHTCLERRGOT(tcl_status, argv[0], interp);
	  p[1] = dtemp;

	  tcl_status = Tcl_GetDouble(interp, argv[i+2], &dtemp);
	  AY_CHTCLERRGOT(tcl_status, argv[0], interp);
	  p[2] = dtemp;

	  if(homogenous)
	    {
	      if(argc > i+3)
		{
		  tcl_status = Tcl_GetDouble(interp, argv[i+3], &dtemp);
		  AY_CHTCLERRGOT(tcl_status, argv[0], interp);
		  p[3] = dtemp;
		}
	      else
		{
		  p[3] = 1.0;
		}
	    } /* if */

	  if(from_world)
	    {
	      glPushMatrix();
	        glTranslated(o->movx, o->movy, o->movz);
		ay_quat_torotmatrix(o->quat, m);
		glMultMatrixd(m);
		glScaled(o->scalx, o->scaly, o->scalz);

		glGetDoublev(GL_MODELVIEW_MATRIX, m);
		ay_trafo_invmatrix4(m, mi);
	       glPopMatrix();

	       if(homogenous)
		 {
		   ay_trafo_apply4(p, mi);
		 }
	       else
		 {
		   ay_trafo_apply3(p, mi);
		 }
	    } /* if */

	  ay_notify_force(o);
	  o->modified = AY_TRUE;

	  if(clear_selp)
	    {
	      free(selp);
	      o->selp = old_selp;
	      clear_selp = AY_FALSE;
	    }
	} /* if */

      sel = sel->next;
    } /* while */

  if(from_world)
    {
      glPopMatrix();
    }

  ay_notify_parent();

cleanup:

  if(clear_selp)
    {
      free(selp);
      o->selp = old_selp;
    }

 return TCL_OK;
} /* ay_tcmd_setpointtcmd */


#ifdef AYENABLEWAIT
#include <sys/types.h>
#include <sys/wait.h>
/* ay_tcmd_waitpidtcmd:
 *  this command waits for a spawned process in order to avoid zombies
 *  (code taken from tclUnixPipe.c/Tcl_WaitPid())
 */
int
ay_tcmd_waitpidtcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int pid;
 pid_t real_pid;
 int result;

  if(argc <= 1)
    {
      ay_error(AY_EARGS, argv[0], "pid");
      return TCL_OK;
    }

  Tcl_GetInt(interp, argv[1], &pid);

  real_pid = (pid_t) pid;
  while(1)
    {
      result = (int) waitpid(real_pid, NULL, WUNTRACED);
      if((result != -1) || (errno != EINTR))
	{
	  return TCL_OK;
	}
    } /* while */

 return TCL_OK;
} /* ay_tcmd_waitpidtcmd */

#endif /* AYENABLEWAIT */


#ifdef AYENABLEFEXIT
/* ay_tcmd_fastexittcmd:
 *  this command exits the application (without trying to clean up properly);
 *  it seems that this is the only way to quit Ayam on Mac OS X (atleast
 *  using Tcl/Tk8.2.2...)
 */
int
ay_tcmd_fastexittcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
  exit(0);
} /* ay_tcmd_fastexittcmd */
#endif /* AYENABLEFEXIT */


/* ay_tcmd_withobtcmd:
 *  this command modifies the selection before executing
 *  another command of the Ayam scripting interface
 */
int
ay_tcmd_withobtcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_list_object *oldsel = ay_selection, *l = NULL;
 int i = 0, index = 0, commandindex = 3;

  if(!oldsel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(argc < 3)
    {
      ay_error(AY_EARGS, argv[0], "index [do] command");
      return TCL_OK;
    }

  if(strcmp(argv[2], "do"))
    commandindex--;

  if(commandindex >= argc)
    {
      ay_error(AY_EARGS, argv[0], "index [do] command");
      return TCL_OK;
    }

  tcl_status = Tcl_GetInt(interp, argv[1], &index);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  if(index < 0)
    {
      ay_error(AY_ERROR, argv[0], "index must be positive");
      return TCL_OK;
    }

  l = oldsel;
  while(l)
    {
      if(i == index)
	{
	  /* found the object with right index =>
	     fake a single object selection */
	  ay_selection = NULL;
	  ay_status = ay_sel_add(l->object);
	  if(!ay_status)
	    {
	      /* execute the command */
	      if(argv[commandindex])
		{
		  Tcl_Eval(interp, argv[commandindex]);
		}
	    }
	  /* restore original selection */
	  ay_sel_free(AY_FALSE);
	  ay_selection = oldsel;
	  /* remember, that we found the object */
	  index = -1;
	  /* break the while loop */
	  break;
	} /* if */
      i++;
      l = l->next;
    } /* while */

  /* if index is here not -1, we did not find the object */
  if(index >= 0)
    {
      ay_error(AY_ERROR, argv[0], "object not found in selection");
    }

 return TCL_OK;
} /* ay_tcmd_withobtcmd */


/* ay_tcmd_getuint:
 *  convert string to unsigned int
 */
int
ay_tcmd_getuint(char *str, unsigned int *uint)
{
 unsigned long int ret;
 char fname[] = "getuint", *p;

  if(!str || !uint)
    return AY_ENULL;

  errno = 0;

  ret = strtoul(str, &p, 0);

  if(p == str)
    {
      ay_error(AY_ERROR, fname, "expected unsigned integer value but got:");
      ay_error(AY_ERROR, fname, str);
      return AY_ERROR;
    }

  if(errno != 0)
    {
#ifdef EINVAL
      if(errno == EINVAL)
	{
	  ay_error(AY_ERROR, fname, "conversion failed");
	  return AY_ERROR;
	}
#endif
      if(errno == ERANGE && ret == ULONG_MAX)
	{
	  ay_error(AY_ERROR, fname, "conversion overflow");
	  return AY_ERROR;
	}

      if(ret == 0)
	{
	  ay_error(AY_ERROR, fname, "conversion failed");
	  return AY_ERROR;
	}
    } /* if */

  if(p && (*p != '\0'))
    {
      ay_error(AY_EWARN, fname, "ignoring trailing characters");
    }

  *uint = (unsigned int)ret;

 return AY_OK;
} /* ay_tcmd_getuint */
