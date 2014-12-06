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

/* tcmd.c - various simple Tcl commands and support functions */

/* prototypes of functions local to this module: */

int ay_tcmd_setallpoints(Tcl_Interp *interp, char *fname, char *vn,
			 int from_world);

int ay_tcmd_getallpoints(Tcl_Interp *interp, char *fname, char *vn,
			 int apply_trafo);


/* functions: */

/** ay_tcmd_convdlist:
 *  convert a Tcl list of doubles to a C array of doubles
 *
 *  \returns TCL_OK upon successful completion, TCL_ERROR else.
 */
int
ay_tcmd_convdlist(char *vname, int *dllen, double **dl)
{
 int tcl_status = TCL_OK;
 Tcl_Obj *vnamePtr = NULL, *listPtr = NULL, **elemvPtr = NULL;
 int i;

  if(!vname || !dllen || !dl)
    {
      return TCL_ERROR;
    }

  vnamePtr = Tcl_NewStringObj(vname, -1);
  if(!vnamePtr)
    {
      return TCL_ERROR;
    }

  listPtr = Tcl_ObjGetVar2(ay_interp, vnamePtr, NULL, TCL_LEAVE_ERR_MSG |
			   TCL_PARSE_PART1);

  if(!listPtr)
    {
      return TCL_ERROR;
    }

  tcl_status = Tcl_ListObjGetElements(ay_interp, listPtr, dllen, &elemvPtr);

  if((tcl_status != TCL_OK) || !elemvPtr)
    {
      return TCL_ERROR;
    }

  if(!(*dl = malloc(*dllen*sizeof(double))))
    {
      return TCL_ERROR;
    }

  for(i = 0; i < *dllen; i++)
    {
      tcl_status = Tcl_GetDoubleFromObj(ay_interp, elemvPtr[i], &((*dl)[i]));
      if(tcl_status != TCL_OK)
	{
	  free(*dl);
	  *dl = NULL;
	  return TCL_ERROR;
	}
    }

 return TCL_OK;
} /* ay_tcmd_convdlist */


/* ay_tcmd_reverttcmd:
 *  revert selected curves
 *  Implements the \a revertC scripting interface command.
 *  See also the corresponding section in the \ayd{screvertc}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_tcmd_reverttcmd(ClientData clientData, Tcl_Interp *interp,
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
	  acurve = (ay_acurve_object*)o->refine;

	  ay_status = ay_act_revert(acurve);
	  if(ay_status)
	    {
	      ay_error(ay_status, argv[0], "Could not revert ACurve!");
	    }
	  else
	    {
	      ay_notify_object(o);
	      o->modified = AY_TRUE;
	    }
	  break;
	case AY_IDICURVE:
	  icurve = (ay_icurve_object*)o->refine;

	  ay_status = ay_ict_revert(icurve);
	  if(ay_status)
	    {
	      ay_error(ay_status, argv[0], "Could not revert ICurve!");
	    }
	  else
	    {
	      ay_notify_object(o);
	      o->modified = AY_TRUE;
	    }
	  break;
	case AY_IDNCURVE:
	  ncurve = (ay_nurbcurve_object*)o->refine;

	  ay_status = ay_nct_revert(ncurve);
	  if(ay_status)
	    {
	      ay_error(ay_status, argv[0], "Could not revert NCurve!");
	    }
	  else
	    {
	      ay_notify_object(o);
	      o->modified = AY_TRUE;
	    }
	  break;
	default:
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	  break;
	} /* switch */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_tcmd_reverttcmd */


/** ay_tcmd_showhideall:
 *  _recursively_ set the hidden flag of all children of object
 *  \a o and \a o to \a val (showing/hiding the objects)
 *  if \a val is -1, the hide state of the objects is toggled
 */
void
ay_tcmd_showhideall(ay_object *o, int val)
{
 ay_object *down;

  if(!o)
   return;

  if(o->down && o->down->next)
    {
      down = o->down;
      while(down->next)
	{
	  ay_tcmd_showhideall(down, val);
	  down = down->next;
	}
    }

  if(val == -1)
    o->hide = !o->hide;
  else
    o->hide = val;

 return;
} /* ay_tcmd_showhideall */


/** ay_tcmd_showtcmd:
 *  show/hide selected (or all) objects
 *  Implements the \a showOb scripting interface command.
 *  Implements the \a hideOb scripting interface command.
 *  See also the corresponding section in the \ayd{schideob}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_tcmd_showtcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
  /*
 int ay_status = AY_OK;
  */
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 int i = 1, all = AY_FALSE, val = AY_TRUE;

  if(argv[0][0] == 's')
    val = AY_FALSE;

  if(argc > 1)
    {
      while(i < argc)
	{
	  /* -toggle */
	  if(argv[i][0] == '-' && argv[i][1] == 't')
	    {
	      val = -1;
	    }
	  /* -all */
	  if(argv[i][0] == '-' && argv[i][1] == 'a')
	    {
	      all = AY_TRUE;
	    }
	  i++;
	} /* while */
    } /* if */

  if(all)
    {
      o = ay_root->next;
      while(o->next)
	{
	  ay_tcmd_showhideall(o, val);
	  o = o->next;
	}
    }
  else
    {
      while(sel)
	{
	  o = sel->object;

	  if(val == -1)
	    o->hide = !o->hide;
	  else
	    o->hide = val;

	  sel = sel->next;
	} /* while */
    } /* if */

 return TCL_OK;
} /* ay_tcmd_hidetcmd */


/* ay_tcmd_getvertcmd:
 *  get all version information from the current OpenGL/GLU and Ayam
 */
int
ay_tcmd_getvertcmd(ClientData clientData, Tcl_Interp *interp,
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
 *  get bilinear patch coordinate address from index
 *
 */
int
ay_tcmd_getbppntfromindex(ay_bpatch_object *patch, int index,
			  double **p)
{
 char fname[] = "getbppntfromindex";

  if(!patch || !p)
    return AY_ENULL;

  if(index >= 4 || index < 0)
    {
      ay_error(AY_ERANGE, fname, "[0, 3]");
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


/* ay_tcmd_getallpoints:
 *  helper for ay_tcmd_getpointtcmd() below
 *  get all points of selected objects
 */
int
ay_tcmd_getallpoints(Tcl_Interp *interp, char *fname, char *vn,
		     int apply_trafo)
{
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_pointedit pe = {0};
 unsigned int i = 0;
 double p[4] = {0};
 double pm[16], m[16];
 int flags = TCL_APPEND_VALUE | TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG |
   TCL_PARSE_PART1;
 Tcl_Obj *to = NULL, *ton = NULL;

  ton = Tcl_NewStringObj(vn, -1);

  if(apply_trafo == 2)
    {
      ay_trafo_identitymatrix(pm);
      if(ay_currentlevel->object != ay_root)
	{
	  ay_trafo_getparent(ay_currentlevel->next, pm);
	}
    }

  while(sel)
    {
      o = sel->object;

      if(apply_trafo == 1)
	{
	  memcpy(m, pm, 16*sizeof(double));
	  ay_trafo_getall(NULL, o, m);
	} /* if */

      ay_pact_getpoint(0, o, p, &pe);

      for(i = 0; i < pe.num; i++)
	{
	  memcpy(p, pe.coords[i], 3*sizeof(double));
	  if(apply_trafo)
	    ay_trafo_apply3(p, m);

	  to = Tcl_NewDoubleObj(p[0]);
	  Tcl_ObjSetVar2(interp, ton, NULL, to, flags);
	  to = Tcl_NewDoubleObj(p[1]);
	  Tcl_ObjSetVar2(interp, ton, NULL, to, flags);
	  to = Tcl_NewDoubleObj(p[2]);
	  Tcl_ObjSetVar2(interp, ton, NULL, to, flags);

	  if(pe.rational)
	    {
	      to = Tcl_NewDoubleObj(pe.coords[i][3]);
	      Tcl_ObjSetVar2(interp, ton, NULL, to, flags);
	    } /* if */
	} /* for */

      ay_pact_clearpointedit(&pe);

      sel = sel->next;
    } /* while */

  ay_notify_parent();

  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return TCL_OK;
} /* ay_tcmd_getallpoints */


/* ay_tcmd_getpointtcmd:
 *  get points of selected objects
 *  Implements the \a getPnt scripting interface command.
 *  See also the corresponding section in the \ayd{scgetpnt}.
 *
 *  \returns TCL_OK in any case.
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
 int rational = AY_FALSE, apply_trafo = AY_FALSE;
 int to_world = AY_FALSE, eval = AY_FALSE, vn = AY_FALSE;
 int handled = AY_FALSE, freepo = AY_FALSE;
 double *p = NULL, *tp = NULL, tmp[4] = {0}, utmp[4] = {0};
 double m[16], u = 0.0, v = 0.0;
 char fargs[] = "[-trafo|-world|-eval] (index | indexu indexv | u | u v (varx vary varz [varw] | -vn varname)|-all varname)";
 Tcl_Obj *to = NULL, *ton = NULL;
 int lflags = TCL_LEAVE_ERR_MSG | TCL_APPEND_VALUE | TCL_LIST_ELEMENT |
   TCL_PARSE_PART1;
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

  while((j < 4) && (j < argc))
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
      /* provided for backwards compatibility... */
      if(argv[j][0] == '-' && argv[j][1] == 'p')
	{
	  /* -param */
	  eval = AY_TRUE;
	  i++;
	  argc2--;
	}
      j++;
    } /* while */

  if(argv[i][0] == '-' && argv[i][1] == 'a')
    {
      /* -all */
      i = 0;
      if(to_world)
	i = 2;
      else
	if(apply_trafo)
	  i = 1;
      return ay_tcmd_getallpoints(interp, argv[0], argv[argc-1], i);
    }

  if(argv[argc-2][0] == '-' && argv[argc-2][1] == 'v')
    {
      /* -vn */
      if(argc2 < 2)
	{
	  ay_error(AY_EARGS, argv[0], fargs);
	  goto cleanup;
	}
      vn = AY_TRUE;
    }

  j = 0;
  while(sel)
    {
      o = sel->object;
      p = NULL;
      rational = AY_FALSE;
      freepo = AY_FALSE;
      switch(o->type)
	{
	case AY_IDNCURVE:
	  if(!vn && (argc2+eval < 6))
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
	      rational = AY_TRUE;
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
		  (void)ay_error_reportdrange(argv[0], "\"u\"",
					      nc->knotv[nc->order-1],
					      nc->knotv[nc->length]);
		  goto cleanup;
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
	      rational = AY_FALSE;
	    } /* if */
	  j = i+1;
	  break;
	case AY_IDACURVE:
	  if(!vn && (argc2 < 5))
	    {
	      ay_error(AY_EARGS, argv[0], fargs);
	      return TCL_OK;
	    }
	  if(eval)
	    goto eval_provided;
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_act_getpntfromindex((ay_acurve_object*)(o->refine),
				 indexu, &p);
	  rational = AY_FALSE;
	  j = i+1;
	  break;
	case AY_IDICURVE:
	  if(!vn && (argc2 < 5))
	    {
	      ay_error(AY_EARGS, argv[0], fargs);
	      return TCL_OK;
	    }
	  if(eval)
	    goto eval_provided;
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_ict_getpntfromindex((ay_icurve_object*)(o->refine),
				 indexu, &p);
	  rational = AY_FALSE;
	  j = i+1;
	  break;
	case AY_IDNPATCH:
	  if(!vn && (argc2+eval < 7))
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
	      rational = AY_TRUE;
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
		  (void)ay_error_reportdrange(argv[0], "\"u\"",
					      np->uknotv[np->uorder-1],
					      np->uknotv[np->width]);
		  goto cleanup;
		}

	      if((v < np->vknotv[np->vorder-1]) ||
		 (v > np->vknotv[np->height]))
		{
		  (void)ay_error_reportdrange(argv[0], "\"v\"",
					      np->vknotv[np->vorder-1],
					      np->vknotv[np->height]);
		  goto cleanup;
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
	      rational = AY_FALSE;
	    } /* if */
	  j = i+2;
	  break;
	case AY_IDIPATCH:
	  if(!vn && (argc2 < 6))
	    {
	      ay_error(AY_EARGS, argv[0], fargs);
	      return TCL_OK;
	    }
	  if(eval)
	    goto eval_provided;
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  tcl_status = Tcl_GetInt(interp, argv[i+1], &indexv);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_ipt_getpntfromindex((ay_ipatch_object*)(o->refine),
				 indexu, indexv, &p);
	  rational = AY_FALSE;
	  j = i+2;
	  break;
	case AY_IDBPATCH:
	  if(!vn && (argc2 < 5))
	    {
	      ay_error(AY_EARGS, argv[0], fargs);
	      return TCL_OK;
	    }
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_tcmd_getbppntfromindex((ay_bpatch_object*)(o->refine),
				    indexu, &p);
	  rational = AY_FALSE;
	  j = i+1;
	  break;
	case AY_IDPAMESH:
	  if(!vn && (argc2 < 7))
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
	  rational = AY_TRUE;
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
		  ay_status = ay_tcmd_getuint(argv[i], &selp->index);
		  if(ay_status)
		    {
		      o->selp = old_selp;
		      free(selp);
		      selp = NULL;
		      goto cleanup;
		    }
		  ay_status = cb(3, o, NULL, NULL);
		  if(ay_status || (!o->selp))
		    {
		      o->selp = old_selp;
		      free(selp);
		      selp = NULL;
		      ay_error(AY_ERROR, argv[0], "getpntcb failed");
		      goto cleanup;
		    }
		  p = selp->point;
		  rational = selp->rational;
		  free(selp);
		  o->selp = old_selp;
		  handled = AY_TRUE;
		  j = i+1;
		} /* if */
	    } /* if */

	  if(!handled)
	    {
eval_provided:
	      if((!vn && argc2 < 7) || (vn && argc2 < 5))
		{
		  po = NULL;
		  ay_provide_object(o, AY_IDNCURVE, &po);
		  if(po)
		    {
		      freepo = AY_TRUE;
		      if(!vn && (argc2+eval < 6))
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
			  rational = AY_TRUE;
			}
		      else
			{
			  tcl_status = Tcl_GetDouble(interp, argv[i], &u);
			  AY_CHTCLERRGOT(tcl_status, argv[0], interp);
			  p = utmp;
			  nc = (ay_nurbcurve_object *)(po->refine);
			  ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
					     nc->knotv, nc->controlv, u, p);
			  rational = AY_FALSE;
			} /* if */
		      j = i+1;
		      handled = AY_TRUE;
		    } /* if */
		}
	      else
		{
		  po = NULL;
		  ay_provide_object(o, AY_IDNPATCH, &po);
		  if(po)
		    {
		      freepo = AY_TRUE;
		      if(!vn && (argc2+eval < 7))
			{
			  ay_error(AY_EARGS, argv[0], fargs);
			  goto cleanup;
			}
		      if(!eval)
			{
			  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
			  AY_CHTCLERRGOT(tcl_status, argv[0], interp);
			  tcl_status = Tcl_GetInt(interp, argv[i+1], &indexv);
			  AY_CHTCLERRGOT(tcl_status, argv[0], interp);
			  ay_npt_getpntfromindex((ay_nurbpatch_object*)
						 (po->refine),
						 indexu, indexv, &p);
			  rational = AY_TRUE;
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
			  rational = AY_FALSE;
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
		  ay_trafo_identitymatrix(m);
		  ay_trafo_getall(ay_currentlevel, o, m);
		}
	      else
		{
		  ay_trafo_creatematrix(o, m);
		}
	      memcpy(tmp, p, 3*sizeof(double));
	      ay_trafo_apply3(tmp, m);
	      if(rational)
		{
		  tmp[3] = p[3];
		}

	      tp = tmp;
	    }
	  else
	    { /* No! */
	      tp = p;
	    }

	  if(vn)
	    {
	      /* -vn */
	      ton = Tcl_NewStringObj(argv[argc-1], -1);
	      to = Tcl_NewDoubleObj(tp[0]);
	      Tcl_ObjSetVar2(interp, ton, NULL, to, lflags);
	      to = Tcl_NewDoubleObj(tp[1]);
	      Tcl_ObjSetVar2(interp, ton, NULL, to, lflags);
	      to = Tcl_NewDoubleObj(tp[2]);
	      Tcl_ObjSetVar2(interp, ton, NULL, to, lflags);
	      if(rational)
		{
		  to = Tcl_NewDoubleObj(tp[3]);
		  Tcl_ObjSetVar2(interp, ton, NULL, to, lflags);
		}
	    }
	  else
	    {
	      ton = Tcl_NewStringObj(argv[j], -1);
	      to = Tcl_NewDoubleObj(tp[0]);
	      Tcl_ObjSetVar2(interp, ton, NULL, to, TCL_LEAVE_ERR_MSG |
			     TCL_PARSE_PART1);

	      Tcl_SetStringObj(ton, argv[j+1], -1);
	      to = Tcl_NewDoubleObj(tp[1]);
	      Tcl_ObjSetVar2(interp, ton, NULL, to, TCL_LEAVE_ERR_MSG |
			     TCL_PARSE_PART1);

	      Tcl_SetStringObj(ton, argv[j+2], -1);
	      to = Tcl_NewDoubleObj(tp[2]);
	      Tcl_ObjSetVar2(interp, ton, NULL, to, TCL_LEAVE_ERR_MSG |
			     TCL_PARSE_PART1);
	      if(rational)
		{
		  Tcl_SetStringObj(ton, argv[j+3], -1);
		  to = Tcl_NewDoubleObj(tp[3]);
		  Tcl_ObjSetVar2(interp, ton, NULL, to, TCL_LEAVE_ERR_MSG |
				 TCL_PARSE_PART1);
		}
	    } /* if */
	  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
	} /* if */

      if(freepo)
	{
	  if(po)
	    (void)ay_object_deletemulti(po, AY_FALSE);
	} /* if */

      sel = sel->next;
    } /* while */

cleanup:

  if(freepo)
    {
      if(po)
	(void)ay_object_deletemulti(po, AY_FALSE);
    } /* if */

 return TCL_OK;
} /* ay_tcmd_getpointtcmd */


/* ay_tcmd_setallpoints:
 *  helper for ay_tcmd_setpointtcmd() below
 *  set all points of selected objects
 */
int
ay_tcmd_setallpoints(Tcl_Interp *interp, char *fname, char *vn,
		     int from_world)
{
 int tcl_status = TCL_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_pointedit pe = {0};
 int vlen = 0;
 unsigned int i = 0, j = 0;
 double *p = NULL, *v = NULL;
 double pm[16], m[16], mi[16];

  tcl_status = ay_tcmd_convdlist(vn, &vlen, &v);
  AY_CHTCLERRRET(tcl_status, fname, interp);

  if(vlen < 2)
    {
      if(v)
	free(v);
      ay_error(AY_ERROR, fname, "Not enough coordinates provided.");
      return TCL_OK;
    }

  if(from_world)
    {
      ay_trafo_identitymatrix(pm);
      if(ay_currentlevel->object != ay_root)
	{
	  ay_trafo_getparent(ay_currentlevel->next, pm);
	}
    }

  while(sel)
    {
      o = sel->object;

      if(from_world)
	{
	  memcpy(m, pm, 16*sizeof(double));
	  ay_trafo_getall(NULL, o, m);
	  ay_trafo_invmatrix(m, mi);
	} /* if */

      p = m;
      ay_pact_getpoint(0, o, p, &pe);
      if(pe.num && !pe.readonly)
	{
	  if(!pe.rational)
	    {
	      for(i = 0; i < pe.num; i++)
		{
		  p = pe.coords[i];
		  memcpy(p, &(v[j]), 3*sizeof(double));
		  if(from_world)
		    ay_trafo_apply3(p, mi);
		  j += 3;
		  if(j > (unsigned int)vlen)
		    break;
		}
	    }
	  else
	    {
	      /* rational */
	      for(i = 0; i < pe.num; i++)
		{
		  p = pe.coords[i];
		  memcpy(p, &(v[j]), 4*sizeof(double));
		  if(from_world)
		    ay_trafo_apply3(p, mi);
		  j += 4;
		  if(j > (unsigned int)vlen)
		    break;
		}
	    } /* if */

	  ay_notify_object(o);
	  o->modified = AY_TRUE;
	} /* if */

      ay_pact_clearpointedit(&pe);

      sel = sel->next;
    } /* while */

  ay_notify_parent();

  if(v)
    {
      free(v);
    }

 return TCL_OK;
} /* ay_tcmd_setallpoints */


/* ay_tcmd_setpointtcmd:
 *  set points of selected objects
 *  Implements the \a setPnt scripting interface command.
 *  See also the corresponding section in the \ayd{scsetpnt}.
 *
 *  \returns TCL_OK in any case.
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
 int remargc = argc, indexu = 0, indexv = 0, i = 1, rational = AY_FALSE;
 int from_world = AY_FALSE, clear_selp = AY_FALSE, handled = AY_FALSE;
 int from_var = AY_FALSE, vlen = 0;
 double *p = NULL, *v = NULL;
 ay_voidfp *arr = NULL;
 ay_getpntcb *cb = NULL;
 double pm[16], m[16], mi[16];
 char args[] =
   "[-world] (index [indexv] (x y z [w] | -vn varname) | -all varname)";

  if(argc <= 1)
    {
      ay_error(AY_EARGS, argv[0], args);
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(argv[1][0] == '-' && argv[1][1] == 'w')
    {
      /* -world */
      from_world = AY_TRUE;
      i++;
      remargc--;
    }

  if(argv[i][0] == '-' && argv[i][1] == 'a')
    {
      /* -all */
      if(argc < i+1)
	{
	  ay_error(AY_EARGS, argv[0], args);
	  return TCL_OK;
	}
      return ay_tcmd_setallpoints(interp, argv[0], argv[argc-1], from_world);
    }
  else
    {
      /* check for -vn argument */
      if(remargc < 3)
	{
	  ay_error(AY_EARGS, argv[0], args);
	  return TCL_OK;
	}

      if((argc-2 > 1) && (argv[argc-2][0] == '-' && argv[argc-2][1] == 'v'))
	{
	  from_var = AY_TRUE;
	  tcl_status = ay_tcmd_convdlist(argv[argc-1], &vlen, &v);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	}
    }

  if(from_world)
    {
      ay_trafo_identitymatrix(pm);
      if(ay_currentlevel->object != ay_root)
	{
	  ay_trafo_getparent(ay_currentlevel->next, pm);
	}
    }

  while(sel)
    {
      o = sel->object;
      p = NULL;
      rational = AY_FALSE;
      clear_selp = AY_FALSE;

      switch(o->type)
	{
	case AY_IDNCURVE:
	  remargc--;
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_nct_getpntfromindex((ay_nurbcurve_object*)(o->refine),
				 indexu, &p);
	  rational = AY_TRUE;
	  i++;
	  break;
	case AY_IDACURVE:
	  remargc--;
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_act_getpntfromindex((ay_acurve_object*)(o->refine),
				 indexu, &p);
	  i++;
	  break;
	case AY_IDICURVE:
	  remargc--;
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_ict_getpntfromindex((ay_icurve_object*)(o->refine),
				 indexu, &p);
	  i++;
	  break;
	case AY_IDNPATCH:
	  remargc -= 2;
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  tcl_status = Tcl_GetInt(interp, argv[i+1], &indexv);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_npt_getpntfromindex((ay_nurbpatch_object*)(o->refine),
				 indexu, indexv, &p);
	  rational = AY_TRUE;
	  i += 2;
	  break;
	case AY_IDIPATCH:
	  remargc -= 2;
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  tcl_status = Tcl_GetInt(interp, argv[i+1], &indexv);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_ipt_getpntfromindex((ay_ipatch_object*)(o->refine),
				 indexu, indexv, &p);
	  i += 2;
	  break;
	case AY_IDBPATCH:
	  remargc--;
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_tcmd_getbppntfromindex((ay_bpatch_object*)(o->refine),
				    indexu, &p);
	  i++;
	  break;
	case AY_IDPAMESH:
	  remargc -= 2;
	  tcl_status = Tcl_GetInt(interp, argv[i], &indexu);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  tcl_status = Tcl_GetInt(interp, argv[i+1], &indexv);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	  ay_pmt_getpntfromindex((ay_pamesh_object*)(o->refine),
				 indexu, indexv, &p);
	  rational = AY_TRUE;
	  i += 2;
	  break;
	default:
	  handled = AY_FALSE;
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
	      rational = selp->rational;
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
	  if(!from_var)
	    {
	      if(remargc < 4)
		{
		  ay_error(AY_EARGS, argv[0], args);
		  goto cleanup;
		}
	      tcl_status = Tcl_GetDouble(interp, argv[i], &dtemp);
	      AY_CHTCLERRGOT(tcl_status, argv[0], interp);
	      p[0] = dtemp;

	      tcl_status = Tcl_GetDouble(interp, argv[i+1], &dtemp);
	      AY_CHTCLERRGOT(tcl_status, argv[0], interp);
	      p[1] = dtemp;

	      tcl_status = Tcl_GetDouble(interp, argv[i+2], &dtemp);
	      AY_CHTCLERRGOT(tcl_status, argv[0], interp);
	      p[2] = dtemp;

	      if(rational)
		{
		  if(remargc > 4)
		    {
		      tcl_status = Tcl_GetDouble(interp, argv[i+3],
						 &dtemp);
		      AY_CHTCLERRGOT(tcl_status, argv[0], interp);
		      p[3] = dtemp;
		    }
		  else
		    {
		      p[3] = 1.0;
		    }
		} /* if */
	    }
	  else
	    {
	      /* from_var */
	      if(!rational)
		{
		  memset(p, 0, 3*sizeof(double));
		  memcpy(p, v, (vlen<3?vlen:3)*sizeof(double));
		}
	      else
		{
		  memset(p, 0, 3*sizeof(double));
		  p[3] = 1.0;
		  memcpy(p, v, (vlen<4?vlen:4)*sizeof(double));
		}
	    } /* if */

	  if(from_world)
	    {
	      memcpy(m, pm, 16*sizeof(double));
	      ay_trafo_getall(NULL, o, m);
	      ay_trafo_invmatrix(m, mi);

	      ay_trafo_apply3(p, mi);
	    } /* if */

	  ay_notify_object(o);
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

  ay_notify_parent();

cleanup:

  if(clear_selp)
    {
      free(selp);
      o->selp = old_selp;
    }

  if(v)
    {
      free(v);
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
 *  execute command(s) for one of the selected objects;
 *  this command modifies the selection before executing
 *  another command of the Ayam scripting interface
 *
 *  Implements the \a withOb scripting interface command.
 *  See also the corresponding section in the \ayd{scwithob}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_tcmd_withobtcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_list_object *oldsel = ay_selection, *l = NULL;
 int i = 0, index = 0, commandindex = 3;
 char args[] = "index [do] command";

  if(!oldsel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(argc < 3)
    {
      ay_error(AY_EARGS, argv[0], args);
      return TCL_OK;
    }

  if(strcmp(argv[2], "do"))
    commandindex--;

  if(commandindex >= argc)
    {
      ay_error(AY_EARGS, argv[0], args);
      return TCL_OK;
    }

  tcl_status = Tcl_GetInt(interp, argv[1], &index);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  if(index < 0)
    {
      ay_error(AY_ERROR, argv[0], "Index must be positive.");
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
	  ay_status = ay_sel_add(l->object, AY_FALSE);
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
      ay_error(AY_ERROR, argv[0], "Object not found in selection.");
    }

 return TCL_OK;
} /* ay_tcmd_withobtcmd */


/* ay_tcmd_getstring:
 *  get string from Tcl variable
 */
int
ay_tcmd_getstring(Tcl_Interp *interp, Tcl_Obj *arr, Tcl_Obj *var, char **result)
{
 Tcl_Obj *to;
 char *str;
 int len = 0;

  if(!interp)
    interp = ay_interp;

  if(!arr || !var || !result)
   return AY_ENULL;

  to = Tcl_ObjGetVar2(interp, arr, var, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(to)
    {
      str = Tcl_GetStringFromObj(to, &len);
      if(str)
	{	  
	  if(*result)
	    {
	      if(!strcmp(*result, str))
		{
		  return AY_OK;
		}
	      free(*result);
	      *result = NULL;
	    }
	  if(!(*result = malloc((len+1)*sizeof(char))))
	    {
	      return AY_EOMEM;
	    }
	  memcpy(*result, str, (len+1)*sizeof(char));
	} /* if str */
    } /* if to */

 return AY_OK;
} /* ay_tcmd_getstring */


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
      ay_error(AY_ERROR, fname, "Expected unsigned integer value but got:");
      ay_error(AY_ERROR, fname, str);
      return AY_ERROR;
    }

  if(errno != 0)
    {
#ifdef EINVAL
      if(errno == EINVAL)
	{
	  ay_error(AY_ERROR, fname, "Conversion failed.");
	  return AY_ERROR;
	}
#endif
      if(errno == ERANGE && ret == ULONG_MAX)
	{
	  ay_error(AY_ERROR, fname, "Conversion overflow.");
	  return AY_ERROR;
	}

      if(ret == 0)
	{
	  ay_error(AY_ERROR, fname, "Conversion failed.");
	  return AY_ERROR;
	}
    } /* if */

  if(p && (*p != '\0'))
    {
      ay_error(AY_EWARN, fname, "Ignoring trailing characters.");
    }

  *uint = (unsigned int)ret;

 return AY_OK;
} /* ay_tcmd_getuint */


/* ay_tcmd_registerlang:
 *  register a new language
 */
int
ay_tcmd_registerlang(char *name, char **result)
{
 int new_item = 0;
 Tcl_HashEntry *entry = NULL;
 static char *langcounter = (char *) 0;
 char fname[] = "registerlang";

  /* check, if language is already registered */
  if((entry = Tcl_FindHashEntry(&ay_languagesht, name)))
    {
      ay_error(AY_ERROR, fname, "Language is already registered.");
      return AY_ERROR;
    }

  langcounter++;

  entry = Tcl_CreateHashEntry(&ay_languagesht, name, &new_item);
  Tcl_SetHashValue(entry, langcounter);

  *result = langcounter;

 return AY_OK;
} /* ay_tcmd_registerlang */


/* ay_tcmd_menustatetcmd:
 *  set action state from menu state
 *  Implements the \a menuState scripting interface command.
 *  \returns TCL_OK in any case.
 */
int
ay_tcmd_menustatetcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 ay_object *o = ay_root->down;
 ay_view_object *view = NULL;
 int offset = 1;

  if(argc > 1)
    offset = -1;

  while(o)
    {
      if(o->type == AY_IDVIEW)
	{
	  view = (ay_view_object*)o->refine;
	  view->action_state += offset;
	}
      o = o->next;
    } /* while */

 return TCL_OK;
} /* ay_tcmd_menustatetcmd */
