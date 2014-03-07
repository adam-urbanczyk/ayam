/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2001 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* acurve.c - acurve (approximating curve) object */

static char *ay_acurve_name = "ACurve";

/* functions: */

/* ay_acurve_createcb:
 *  create callback function of acurve object
 */
int
ay_acurve_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 int tcl_status = TCL_OK;
 char fname[] = "crtacurve";
 char option_handled = AY_FALSE;
 int center = AY_FALSE, type = 0, sym = 0;
 int stride = 3, order = 3, length = 4, alength = 0, optnum = 0, i = 2, j = 0;
 int acvlen = 0;
 char **acv = NULL;
 double *cv = NULL, *tmp = NULL;
 double dx = 0.25, dy = 0.0, dz = 0.0;
 double s[3] = {0};
 ay_acurve_object *acurve = NULL;

  if(!argv || !o)
    return AY_ENULL;

  /* parse args */
  while(i < argc)
    {
      if(i+1 >= argc)
	{
	  ay_error(AY_EOPT, fname, argv[i]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

      tcl_status = TCL_OK;
      option_handled = AY_FALSE;
      optnum = i;
      if(argv[i] && argv[i][0] != '\0')
	{
	  switch(argv[i][1])
	    {
	    case 'a':
	      /* -alength */
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &alength);
	      option_handled = AY_TRUE;
	      break;
	    case 'd':
	      switch(argv[i][2])
		{
		case 'x':
		  /* -dx */
		  tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &dx);
		  option_handled = AY_TRUE;
		  break;
		case 'y':
		  /* -dy */
		  tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &dy);
		  option_handled = AY_TRUE;
		  break;
		case 'z':
		  /* -dz */
		  tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &dz);
		  option_handled = AY_TRUE;
		  break;
		default:
		  break;
		} /* switch */
	      break;
	    case 'l':
	      /* -length */
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &length);
	      option_handled = AY_TRUE;
	      break;
	    case 'o':
	      /* -order */
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &order);
	      option_handled = AY_TRUE;
	      break;
	    case 'c':
	      switch(argv[i][2])
		{
		case 'n':
		  /* -cn */
		  if(cv)
		    {
		      free(cv);
		      cv = NULL;
		    }
		  tcl_status = ay_tcmd_convdlist(argv[i+1], &acvlen, &cv);
		  option_handled = AY_TRUE;
		  break;
		case 'v':
		  /* -cv */
		  if(Tcl_SplitList(ay_interp, argv[i+1], &acvlen, &acv) ==
		     TCL_OK)
		    {
		      if(cv)
			{
			  free(cv);
			}
		      if(!(cv = calloc(acvlen<stride?stride:acvlen,
				       sizeof(double))))
			{
			  Tcl_Free((char *) acv);
			  ay_status = AY_EOMEM;
			  goto cleanup;
			}
		      for(j = 0; j < acvlen; j++)
			{
			  tcl_status = Tcl_GetDouble(ay_interp,
						     acv[j], &cv[j]);
			  if(tcl_status != TCL_OK)
			    {
			      break;
			    }

			} /* for */
		      Tcl_Free((char *) acv);
		    }
		  option_handled = AY_TRUE;
		  break;
		case 'e':
		  /* -center */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &center);
		  option_handled = AY_TRUE;
		  break;
		default:
		  break;
		} /* switch */
	      break;
	    case 's':
	      /* -symmetric */
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &sym);
	      option_handled = AY_TRUE;
	      break;
	    case 't':
	      /* -type */
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &type);
	      option_handled = AY_TRUE;
	      break;
	    default:
	      break;
	    } /* switch */

	  if(option_handled && (tcl_status != TCL_OK))
	    {
	      ay_error(AY_EOPT, fname, argv[i]);
	      ay_status = AY_ERROR;
	      goto cleanup;
	    }

	  i += 2;
	}
      else
	{
	  i++;
	} /* if */

      if(!option_handled)
	{
	  ay_error(AY_EUOPT, fname, argv[optnum]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

    } /* while */


  /* check length and order */
  if(length < 4)
    {
      length = 4;
    }

  if((alength == 0) || (alength >= length))
    {
      alength = length-1;
    }

  if(order > alength)
    {
      order = alength;
    }

  if(type != 0 && type != 1)
    {
      type = 0;
    }

  if(sym != 0 && sym != 1)
    {
      sym = 0;
    }

  /* if the user specified control points ... */
  if(cv)
    {
      /* ... check length of user provided control point array */
      if(acvlen/stride != length)
	{
	  /* create proper cv, copy what we have got so far */
	  if(!(tmp = calloc(stride*length, sizeof(double))))
	    {
	      ay_status = AY_EOMEM;
	      goto cleanup;
	    }
	  if(acvlen/stride < length)
	    {
	      memcpy(tmp, cv, acvlen*sizeof(double));
	    }
	  else
	    {
	      memcpy(tmp, cv, stride*length*sizeof(double));
	    }
	  free(cv);
	  cv = tmp;
	  /*
	    in case we got less control points than we
	    originally needed, extrapolate the last point
	    using dx,dy,dz;
	    this provides a way for easy specification of
	    an arbitrary starting point (just provide one
	    control point, we take care of the rest here...)
	  */
	  if(acvlen/stride < length)
	    {
	      if(acvlen < stride)
		j = 1;
	      else
		j = acvlen/stride;
	      for(i = j; i < (length); i++)
		{
		  cv[i*stride]   = cv[(i-1)*stride]   + dx;
		  cv[i*stride+1] = cv[(i-1)*stride+1] + dy;
		  cv[i*stride+2] = cv[(i-1)*stride+2] + dz;
		}
	    }
	} /* if */
    }
  else
    {
      /* user did not specify any control points */
      if(!(cv = calloc(length*stride, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      if(center)
	{
	  if(dx > 0.0)
	    s[0] = -(((length-1)*dx)/2.0);
	  if(dy > 0.0)
	    s[1] = -(((length-1)*dy)/2.0);
	  if(dz > 0.0)
	    s[2] = -(((length-1)*dz)/2.0);
	}

      for(i = 0; i < length; i++)
	{
	  cv[i*stride]   = s[0] + (double)i*dx;
	  cv[i*stride+1] = s[1] + (double)i*dy;
	  cv[i*stride+2] = s[2] + (double)i*dz;
	}
    } /* if */

  /* create the acurve object */
  if(!(acurve = calloc(1, sizeof(ay_acurve_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  acurve->order = order;
  acurve->closed = type;
  acurve->symmetric = sym;
  acurve->length = length;
  acurve->alength = alength;
  acurve->controlv = cv;

  o->refine = acurve;

  ay_notify_object(o);

  /* prevent cleanup code from doing something harmful */
  cv = NULL;

cleanup:

  if(cv)
    free(cv);

  if(ay_status == AY_EOMEM)
    {
      ay_error(AY_EOMEM, fname, NULL);
      ay_status = AY_ERROR;
    }

 return ay_status;
} /* ay_acurve_createcb */


/* ay_acurve_deletecb:
 *  delete callback function of acurve object
 */
int
ay_acurve_deletecb(void *c)
{
 ay_acurve_object *acurve = NULL;

  if(!c)
    return AY_ENULL;

  acurve = (ay_acurve_object *)(c);

  /* free controlv */
  if(acurve->controlv)
    free(acurve->controlv);

  if(acurve->ncurve)
    (void)ay_object_delete(acurve->ncurve);

  free(acurve);

 return AY_OK;
} /* ay_acurve_deletecb */


/* ay_acurve_copycb:
 *  copy callback function of acurve object
 */
int
ay_acurve_copycb(void *src, void **dst)
{
 ay_acurve_object *acurve = NULL, *acurvesrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  acurvesrc = (ay_acurve_object *)src;

  if(!(acurve = malloc(sizeof(ay_acurve_object))))
    return AY_EOMEM;

  memcpy(acurve, src, sizeof(ay_acurve_object));

  /* copy controlv */
  if(!(acurve->controlv = malloc(3 * acurve->length * sizeof(double))))
    {
      free(acurve);
      return AY_EOMEM;
    }
  memcpy(acurve->controlv, acurvesrc->controlv,
	 3 * acurve->length * sizeof(double));

  acurve->ncurve = NULL;

  *dst = (void *)acurve;

 return AY_OK;
} /* ay_acurve_copycb */


/* ay_acurve_drawcb:
 *  draw (display in an Ayam view window) callback function of acurve object
 */
int
ay_acurve_drawcb(struct Togl *togl, ay_object *o)
{
 ay_acurve_object *acurve;
 ay_nurbcurve_object *ncurve = NULL;
 int display_mode = ay_prefs.nc_display_mode;
 int i, a, drawch = AY_FALSE;

  if(!o)
    return AY_ENULL;

  acurve = (ay_acurve_object *)o->refine;

  if(!acurve)
    return AY_ENULL;

  if(acurve->display_mode != 0)
    {
      display_mode = acurve->display_mode-1;
    }

  switch(display_mode)
    {
    case 0: /* ControlHull */
      drawch = AY_TRUE;
      break;
    case 1: /* CurveAndHull (GLU) */
      drawch = AY_TRUE;
      if(acurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)acurve->ncurve->refine;
	  ncurve->display_mode = 3;
	  ay_draw_object(togl, acurve->ncurve, AY_TRUE);
	}
      break;
    case 2: /* Curve (GLU) */
      if(acurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)acurve->ncurve->refine;
	  ncurve->display_mode = 3;
	  ay_draw_object(togl, acurve->ncurve, AY_TRUE);
	}
      break;
    case 3: /* CurveAndHull (STESS) */
      drawch = AY_TRUE;
      if(acurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)acurve->ncurve->refine;
	  ncurve->display_mode = 5;
	  ay_draw_object(togl, acurve->ncurve, AY_TRUE);
	}
      break;
    case 4: /* Curve (STESS) */
      if(acurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)acurve->ncurve->refine;
	  ncurve->display_mode = 5;
	  ay_draw_object(togl, acurve->ncurve, AY_TRUE);
	}
      break;
    } /* switch */

  if(drawch)
    {
      a = 0;
      glBegin(GL_LINE_STRIP);
      for(i = 0; i < acurve->length; i++)
	{
	  glVertex3dv((GLdouble *)&(acurve->controlv[a]));
	  a += 3;
	}
      glEnd();
    } /* if */

 return AY_OK;
} /* ay_acurve_drawcb */


/* ay_acurve_shadecb:
 *  shade (display in an Ayam view window) callback function of acurve object
 */
int
ay_acurve_shadecb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_acurve_shadecb */


/* ay_acurve_drawacb:
 *  draw annotations (in an Ayam view window) callback function of acurve object
 */
int
ay_acurve_drawacb(struct Togl *togl, ay_object *o)
{
 ay_acurve_object *acurve;
 double *ver;

  if(!o)
    return AY_ENULL;

  acurve = (ay_acurve_object *)o->refine;

  if(!acurve)
    return AY_ENULL;

  ver = acurve->controlv;

  /* draw arrow */
  ay_draw_arrow(togl, &(ver[acurve->length*3-6]), &(ver[acurve->length*3-3]));

 return AY_OK;
} /* ay_acurve_drawacb */


/* ay_acurve_drawhcb:
 *  draw handles (in an Ayam view window) callback function of acurve object
 */
int
ay_acurve_drawhcb(struct Togl *togl, ay_object *o)
{
 int i;
 double *pnts;
 ay_acurve_object *acurve;

  if(!o)
    return AY_ENULL;

  acurve = (ay_acurve_object *)o->refine;

  if(!acurve)
    return AY_ENULL;

  pnts = acurve->controlv;

  /* draw points */
  glBegin(GL_POINTS);
   for(i = 0; i < acurve->length; i++)
     {
       glVertex3dv((GLdouble *)pnts);
       pnts += 3;
     }
  glEnd();

 return AY_OK;
} /* ay_acurve_drawhcb */


/* ay_acurve_getpntcb:
 *  get point (editing and selection) callback function of acurve object
 */
int
ay_acurve_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_acurve_object *acurve = NULL;
 ay_point *pnt = NULL, **lastpnt = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **ctmp;
 double *control = NULL, *c = NULL;
 int i = 0, j = 0, a = 0;
 const int stride = 3;
 unsigned int *itmp, peindex = 0;

  if(!o || ((mode != 3) && (!p || !pe)))
    return AY_ENULL;

  acurve = (ay_acurve_object *)(o->refine);

  if(!acurve)
    return AY_ENULL;

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(pe)
    pe->rational = AY_FALSE;

  switch(mode)
    {
    case 0:
      /* select all points */
      if(!(pe->coords = calloc(acurve->length, sizeof(double*))))
	return AY_EOMEM;
      if(!(pe->indices = calloc(acurve->length, sizeof(unsigned int))))
	return AY_EOMEM;

      for(i = 0; i < acurve->length; i++)
	{
	  pe->coords[i] = &(acurve->controlv[a]);
	  pe->indices[i] = i;
	  a += stride;
	}

      pe->num = acurve->length;
      break;
    case 1:
      /* selection based on a single point */

      control = acurve->controlv;
      for(i = 0; i < acurve->length; i++)
	{
	  dist = AY_VLEN((p[0] - control[j]),
			 (p[1] - control[j+1]),
			 (p[2] - control[j+2]));

	  if(dist < min_dist)
	    {
	      pecoord = &(control[j]);
	      peindex = i;
	      min_dist = dist;
	    }

	  j += stride;
	} /* for */

      if(!pecoord)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      if(!(pe->coords = calloc(1, sizeof(double*))))
	return AY_EOMEM;

      if(!(pe->indices = calloc(1, sizeof(unsigned int))))
	return AY_EOMEM;

      pe->coords[0] = pecoord;
      pe->indices[0] = peindex;
      pe->num = 1;
      break;
    case 2:
      /* selection based on planes */
      control = acurve->controlv;
      j = 0;
      a = 0;
      for(i = 0; i < acurve->length; i++)
	{
	  c = &(control[j]);

	  /* test point c against the four planes in p */
	  if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
	     ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
	     ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
	     ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
	    {
	      if(!(ctmp = realloc(pe->coords, (a+1)*sizeof(double *))))
		return AY_EOMEM;
	      pe->coords = ctmp;
	      if(!(itmp = realloc(pe->indices, (a+1)*sizeof(unsigned int))))
		return AY_EOMEM;
	      pe->indices = itmp;

	      pe->coords[a] = &(control[j]);
	      pe->indices[a] = i;
	      a++;
	    } /* if */

	  j += stride;
	} /* for */

      pe->num = a;

      break;
    case 3:
      /* rebuild from o->selp */
      pnt = o->selp;
      lastpnt = &o->selp;
      while(pnt)
	{
	  if(pnt->index < (unsigned int)acurve->length)
	    {
	      pnt->point = &(acurve->controlv[pnt->index*stride]);
	      pnt->rational = AY_FALSE;
	      lastpnt = &(pnt->next);
	      pnt = pnt->next;
	    }
	  else
	    {
	      *lastpnt = pnt->next;
	      free(pnt);
	      pnt = *lastpnt;
	    }
	} /* while */
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_acurve_getpntcb */


/* ay_acurve_setpropcb:
 *  set property (from Tcl to C context) callback function of acurve object
 */
int
ay_acurve_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "ACurveAttrData";
 char fname[] = "acurve_setpropcb";
 int new_length, new_alength, new_order;
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_acurve_object *acurve = NULL;

  if(!interp || !o)
    return AY_ENULL;

  acurve = (ay_acurve_object *)o->refine;

  if(!acurve)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Length",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_length);

  Tcl_SetStringObj(ton,"ALength",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_alength);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(acurve->closed));

  Tcl_SetStringObj(ton,"Symmetric",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(acurve->symmetric));

  Tcl_SetStringObj(ton,"Order",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_order);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(acurve->glu_sampling_tolerance));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(acurve->display_mode));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  if(new_length != acurve->length)
    {
      if(new_length > 2)
	{
	  ay_status = ay_act_resize(acurve, new_length);
	}
      else
	{
	  ay_error(AY_ERROR, fname, "Length must be > 2!");
	}
    }

  if(new_order != acurve->order)
    {
      if(new_order > 1)
	acurve->order = new_order;
      else
	ay_error(AY_ERROR, fname, "Order must be > 1!");
    }

  /* check (and correct?) approximation length */
  if(new_alength > 1)
    {
      acurve->alength = new_alength;
    }
  else
    {
      ay_error(AY_EWARN, fname, "ALength must be > 1!");
    }

  if(!acurve->closed)
    {
      if(acurve->alength > acurve->length)
	{
	  ay_error(AY_EWARN, fname, "Lowering ALength to match Length!");
	  acurve->alength = acurve->length;
	}
    }
  else
    {
      if(acurve->alength > acurve->length+acurve->order-1)
	{
	  ay_error(AY_EWARN, fname, "Lowering ALength to match Length!");
	  acurve->alength = acurve->length;
	}
    }

  /* check (and correct?) order */
  if(acurve->alength < acurve->order)
    {
      ay_error(AY_EWARN, fname, "Lowering Order to match ALength!");
      acurve->order = acurve->alength;
    }

  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return ay_status;
} /* ay_acurve_setpropcb */


/* ay_acurve_getpropcb:
 *  get property (from C to Tcl context) callback function of acurve object
 */
int
ay_acurve_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="ACurveAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_acurve_object *acurve = NULL;

  if(!interp || !o)
    return AY_ENULL;

  acurve = (ay_acurve_object *)(o->refine);

  if(!acurve)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Length",-1);
  to = Tcl_NewIntObj(acurve->length);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"ALength",-1);
  to = Tcl_NewIntObj(acurve->alength);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_NewIntObj(acurve->closed);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Symmetric",-1);
  to = Tcl_NewIntObj(acurve->symmetric);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order",-1);
  to = Tcl_NewIntObj(acurve->order);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(acurve->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(acurve->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getncinfo(interp, n1, acurve->ncurve);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_acurve_getpropcb */


/* ay_acurve_readcb:
 *  read (from scene file) callback function of acurve object
 */
int
ay_acurve_readcb(FILE *fileptr, ay_object *o)
{
 ay_acurve_object *acurve = NULL;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(acurve = calloc(1, sizeof(ay_acurve_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&acurve->length);
  fscanf(fileptr,"%d\n",&acurve->alength);
  fscanf(fileptr,"%d\n",&acurve->closed);
  fscanf(fileptr,"%d\n",&acurve->symmetric);
  fscanf(fileptr,"%d\n",&acurve->order);

  fscanf(fileptr,"%lg\n",&acurve->glu_sampling_tolerance);
  fscanf(fileptr,"%d\n",&acurve->display_mode);

  if(!(acurve->controlv = calloc(acurve->length*3, sizeof(double))))
    { free(acurve); return AY_EOMEM; }

  a = 0;
  for(i = 0; i < acurve->length; i++)
    {
      fscanf(fileptr,"%lg %lg %lg\n",
	     &(acurve->controlv[a]),
	     &(acurve->controlv[a+1]),
	     &(acurve->controlv[a+2]));
      a += 3;
    }

  o->refine = acurve;

 return AY_OK;
} /* ay_acurve_readcb */


/* ay_acurve_writecb:
 *  write (to scene file) callback function of acurve object
 */
int
ay_acurve_writecb(FILE *fileptr, ay_object *o)
{
 ay_acurve_object *acurve = NULL;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  acurve = (ay_acurve_object *)(o->refine);

  if(!acurve)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", acurve->length);
  fprintf(fileptr, "%d\n", acurve->alength);
  fprintf(fileptr, "%d\n", acurve->closed);
  fprintf(fileptr, "%d\n", acurve->symmetric);
  fprintf(fileptr, "%d\n", acurve->order);

  fprintf(fileptr, "%g\n", acurve->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", acurve->display_mode);

  a = 0;
  for(i = 0; i < acurve->length; i++)
    {
      fprintf(fileptr,"%g %g %g\n",
	      acurve->controlv[a],
	      acurve->controlv[a+1],
	      acurve->controlv[a+2]);
      a += 3;
    }

 return AY_OK;
} /* ay_acurve_writecb */


/* ay_acurve_wribcb:
 *  RIB export callback function of acurve object
 */
int
ay_acurve_wribcb(char *file, ay_object *o)
{
  /*
 ay_acurve_object *acurve = NULL;

  if(!o)
   return AY_ENULL;

  acurve = (ay_acurve_object*)o->refine;
  */
 return AY_OK;
} /* ay_acurve_wribcb */


/* ay_acurve_bbccb:
 *  bounding box calculation callback function of acurve object
 */
int
ay_acurve_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_acurve_object *acurve = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  acurve = (ay_acurve_object *)o->refine;

  if(!acurve)
    return AY_ENULL;

 return ay_bbc_fromarr(acurve->controlv, acurve->length, 3, bbox);
} /* ay_acurve_bbccb */


/* ay_acurve_notifycb:
 *  notification callback function of acurve object
 */
int
ay_acurve_notifycb(ay_object *o)
{
 ay_acurve_object *acurve = NULL;
 ay_nurbcurve_object *nc = NULL;
 ay_object *ncurve = NULL;
 int ay_status = AY_OK, dlen = 0, aclen = 0, i, j;
 double *knotv = NULL, *controlv = NULL;
 double *controlvr = NULL, *knotv2 = NULL, *controlv2 = NULL;
 double t[3] = {0}, *p1, *p2;

  if(!o)
    return AY_ENULL;

  acurve = (ay_acurve_object *)(o->refine);

  if(!acurve)
    return AY_ENULL;

  (void)ay_object_delete(acurve->ncurve);
  acurve->ncurve = NULL;

  /* create new approximating curve */
  if(!(ncurve = calloc(1, sizeof(ay_object))))
    {
      return AY_EOMEM;
    }

  ay_object_defaults(ncurve);
  ncurve->type = AY_IDNCURVE;
  if(!acurve->closed)
    {
      aclen = acurve->alength;
      ay_status = ay_act_leastSquares(acurve->controlv,
				      acurve->length,
				      aclen,
				      acurve->order-1,
				      &knotv, &controlv);
    }
  else
    {
      aclen = acurve->alength + acurve->order - 1;
      ay_status = ay_act_leastSquaresClosed(acurve->controlv,
					    acurve->length,
					    aclen,
					    acurve->order-1,
					    &knotv, &controlv);
    }

  if(ay_status)
    {
      goto cleanup;
    }

  if(acurve->symmetric && !acurve->closed)
    {
      /* create reverted data point vector */
      dlen = acurve->length;
      if(!(controlvr = calloc(dlen*3, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      memcpy(controlvr, acurve->controlv, dlen*3*sizeof(double));

      for(i = 0; i < dlen/2; i++)
	{
	  memcpy(t, &(controlvr[i*3]), 3*sizeof(double));
	  memcpy(&(controlvr[i*3]), &(controlvr[(dlen-i-1)*3]),
		 3*sizeof(double));
	  memcpy(&(controlvr[(dlen-i-1)*3]), t, 3*sizeof(double));
	} /* for */

      /* approximate the reverted vector */
      ay_status = ay_act_leastSquares(controlvr,
				      acurve->length,
				      aclen,
				      acurve->order-1,
				      &knotv2, &controlv2);

      /* merge forward and backward approximation results */
      if(!ay_status)
	{
	  for(i = 1; i < aclen-1; i++)
	    {
	      p1 = &(controlv[i*4]);
	      p2 = &(controlv2[(aclen-i-1)*4]);
	      if(!AY_V3COMP(p1, p2))
		{
		  AY_V3SUB(t, p2, p1);
		  AY_V3SCAL(t, 0.5);
		  AY_V3ADD(p1, p1, t);
		}
	    }

	  /* create new symmetric knot vector */
	  j = (aclen+acurve->order)/2-1;
	  if(((aclen+acurve->order)%2) != 0)
	    {
	      /* odd number of knots */
	      for(i = (aclen+acurve->order)/2+1; i < aclen; i++)
		{
		  knotv[i] = 0.5+(0.5-knotv[j]);
		  j--;
		}
	      knotv[(aclen+acurve->order)/2] = 0.5;
	    }
	  else
	    {
	      /* even number of knots */
	      for(i = (aclen+acurve->order)/2; i < aclen; i++)
		{
		  knotv[i] = 0.5+(0.5-knotv[j]);
		  j--;
		}
	    }
	} /* if */

      free(controlvr);

      if(knotv2)
	free(knotv2);

      if(controlv2)
        free(controlv2);

      if(ay_status)
	{
	  goto cleanup;
	}
    } /* if(symmetric */

  ay_status = ay_nct_create(acurve->order, aclen, AY_KTCUSTOM,
			    controlv, knotv,
			   (ay_nurbcurve_object **)(void*)&(ncurve->refine));

  if(ay_status)
    {
      goto cleanup;
    }

  nc = (ay_nurbcurve_object *)ncurve->refine;
  nc->display_mode = acurve->display_mode;
  nc->glu_sampling_tolerance = acurve->glu_sampling_tolerance;

  acurve->ncurve = ncurve;

  if(acurve->closed && acurve->symmetric)
    {
      for(i = 0; i < nc->length-(nc->order); i++)
	{
	  ay_status = ay_nct_shiftarr(1, 4, nc->length, nc->controlv);
	  if(ay_status)
	    goto cleanup;
	}
    }

  /* prevent cleanup code from doing something harmful */
  ncurve = NULL;

cleanup:

  if(ncurve)
    {
      (void)ay_object_delete(ncurve);
    }

 return ay_status;
} /* ay_acurve_notifycb */


/* ay_acurve_convertcb:
 *  convert callback function of acurve object
 */
int
ay_acurve_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_acurve_object *ac = NULL;
 ay_object *new = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  ac = (ay_acurve_object *)o->refine;

  if(!ac)
    return AY_ENULL;

  if(ac->ncurve)
    {
      ay_status = ay_object_copy(ac->ncurve, &new);

      if(new)
	{
	  nc = (ay_nurbcurve_object *)(new->refine);

	  /* reset display mode and sampling tolerance
	     of new curve to "global"? */
	  if(!in_place && ay_prefs.conv_reset_display)
	    {
	      nc->display_mode = 0;
	      nc->glu_sampling_tolerance = 0.0;
	    }

	  if(ac->closed)
	    {
	      nc->createmp = AY_TRUE;
	      ay_nct_recreatemp(nc);
	    }

	  ay_trafo_copy(o, new);

	  if(!in_place)
	    {
	      ay_object_link(new);
	    }
	  else
	    {
	      ay_status = ay_object_replace(new, o);
	    } /* if */
	} /* if */
    } /* if */

 return ay_status;
} /* ay_acurve_convertcb */


/* ay_acurve_providecb:
 *  provide callback function of acurve object
 */
int
ay_acurve_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 ay_acurve_object *ac = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNCURVE)
	return AY_OK;
      else
	return AY_ERROR;
    }

  ac = (ay_acurve_object *)o->refine;

  if(!ac)
    return AY_ENULL;

  if(type == AY_IDNCURVE)
    {
      if(ac->ncurve)
	{
	  ay_status = ay_object_copy(ac->ncurve, result);
	  if(*result)
	    {
	      ay_trafo_copy(o, *result);
	    } /* if */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
    } /* if */

 return ay_status;
} /* ay_acurve_providecb */


/* ay_acurve_init:
 *  initialize the acurve object module
 */
int
ay_acurve_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_acurve_name,
				    ay_acurve_createcb,
				    ay_acurve_deletecb,
				    ay_acurve_copycb,
				    ay_acurve_drawcb,
				    ay_acurve_drawhcb,
				    NULL, /* no shading */
				    ay_acurve_setpropcb,
				    ay_acurve_getpropcb,
				    ay_acurve_getpntcb,
				    ay_acurve_readcb,
				    ay_acurve_writecb,
				    NULL, /* no RIB export */
				    ay_acurve_bbccb,
				    AY_IDACURVE);

  ay_status += ay_draw_registerdacb(ay_acurve_drawacb, AY_IDACURVE);

  ay_status += ay_notify_register(ay_acurve_notifycb, AY_IDACURVE);

  ay_status += ay_convert_register(ay_acurve_convertcb, AY_IDACURVE);

  ay_status += ay_provide_register(ay_acurve_providecb, AY_IDACURVE);

  /* acurves may not be associated with materials */
  ay_matt_nomaterial(AY_IDACURVE);

 return ay_status;
} /* ay_acurve_init */

