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

/* icurve.c - icurve (interpolating curve) object */

static char *ay_icurve_name = "ICurve";

/* functions: */

/* ay_icurve_createcb:
 *  create callback function of icurve object
 */
int
ay_icurve_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 int tcl_status = TCL_OK;
 char fname[] = "crticurve";
 char option_handled = AY_FALSE;
 int center = AY_FALSE, type = AY_FALSE, derivs = AY_FALSE;
 int stride = 3, order = 4, length = 4, kt = 0, optnum = 0, i = 2, j = 0;
 int acvlen = 0, derlen = 0;
 char **acv = NULL, **der = NULL;
 double *cv = NULL, *tmp = NULL;
 double dx = 0.25, dy = 0.0, dz = 0.0;
 double s[3] = {0}, sder[3] = {0}, eder[3] = {0};
 double sdlen = 0.125, edlen = 0.125;
 ay_icurve_object *icurve = NULL;

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
	    case 'd':
	      switch(argv[i][2])
		{
		case 'e':
		  /* -derivs */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &derivs);
		  option_handled = AY_TRUE;
		  break;
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
	    case 'k':
	      /* -kt */
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &kt);
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
		      Tcl_Free((char *)acv);
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
	      if(argv[i][2] == 'd')
		{
		  switch(argv[i][3])
		    {
		    case 'e':
		      /* -sderiv */
		      derlen = 0;
		      der = NULL;
		      if(Tcl_SplitList(ay_interp, argv[i+1], &derlen, &der) ==
			 TCL_OK)
			{
			  memset(sder, 0, 3*sizeof(double));
			  for(j = 0; j < derlen; j++)
			    {
			      if(j < 3)
				{
				  tcl_status = Tcl_GetDouble(ay_interp,
							     der[j], &sder[j]);
				  if(tcl_status != TCL_OK)
				    {
				      break;
				    }
				}
			      else
				{
				  break;
				}
			    } /* for */
			  Tcl_Free((char *)der);
			} /* if */
		      option_handled = AY_TRUE;
		      break;
		    case 'l':
		      /* -sdlen */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &sdlen);
		      option_handled = AY_TRUE;
		      break;
		    default:
		      break;
		    } /* switch */
		} /* if */
	      break;
	    case 'e':
	      if(argv[i][2] == 'd')
		{
		  switch(argv[i][3])
		    {
		    case 'e':
		      /* -ederiv */
		      derlen = 0;
		      der = NULL;
		      if(Tcl_SplitList(ay_interp, argv[i+1], &derlen, &der) ==
			 TCL_OK)
			{
			  memset(eder, 0, 3*sizeof(double));
			  for(j = 0; j < derlen; j++)
			    {
			      if(j < 3)
				{
				  tcl_status = Tcl_GetDouble(ay_interp,
							     der[j], &eder[j]);
				  if(tcl_status != TCL_OK)
				    {
				      break;
				    }
				}
			      else
				{
				  break;
				}
			    } /* for */
			  Tcl_Free((char *)der);
			} /* if */
		      option_handled = AY_TRUE;
		      break;
		    case 'l':
		      /* -edlen */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &edlen);
		      option_handled = AY_TRUE;
		      break;
		    default:
		      break;
		    } /* switch */
		} /* if */
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

  /* check args */
  if(length <= 1)
    {
      length = 4;
    }

  if(order < 3)
    {
      order = 4;
    }

  if(fabs(sdlen) < AY_EPSILON)
    {
      sdlen = 0.125;
    }

  if(fabs(edlen) < AY_EPSILON)
    {
      edlen = 0.125;
    }

  if(kt != 0 && kt != 1)
    {
      kt = 0;
    }

  if(type != 0 && type != 1)
    {
      type = 0;
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
	}
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
    }

  /* check end derivatives */
  if(derivs)
    {

    }

  /* create the icurve object */
  if(!(icurve = calloc(1, sizeof(ay_icurve_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  icurve->type = type;
  icurve->length = length;
  icurve->order = order;
  icurve->param_type = kt;
  icurve->sdlen = sdlen;
  icurve->edlen = edlen;
  icurve->derivs = derivs;

  /*memcpy(icurve->sderiv, sder, 3*sizeof(double));*/

  icurve->sderiv[0] = cv[0] - sder[0];
  icurve->sderiv[1] = cv[1] - sder[1];
  icurve->sderiv[2] = cv[2] - sder[2];

  /*memcpy(icurve->ederiv, eder, 3*sizeof(double));*/

  icurve->ederiv[0] = cv[(length-1)*3]   - eder[0];
  icurve->ederiv[1] = cv[(length-1)*3+1] - eder[1];
  icurve->ederiv[2] = cv[(length-1)*3+2] - eder[2];

  icurve->controlv = cv;

  o->refine = icurve;

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
} /* ay_icurve_createcb */


/* ay_icurve_deletecb:
 *  delete callback function of icurve object
 */
int
ay_icurve_deletecb(void *c)
{
 ay_icurve_object *icurve = NULL;

  if(!c)
    return AY_ENULL;

  icurve = (ay_icurve_object *)(c);

  /* free controlv */
  if(icurve->controlv)
    free(icurve->controlv);

  /* free cached ncurve */
  if(icurve->ncurve)
    (void)ay_object_delete(icurve->ncurve);

  free(icurve);

 return AY_OK;
} /* ay_icurve_deletecb */


/* ay_icurve_copycb:
 *  copy callback function of icurve object
 */
int
ay_icurve_copycb(void *src, void **dst)
{
 ay_icurve_object *icurve = NULL, *icurvesrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  icurvesrc = (ay_icurve_object *)src;

  if(!(icurve = malloc(sizeof(ay_icurve_object))))
    return AY_EOMEM;

  memcpy(icurve, src, sizeof(ay_icurve_object));

  /* copy controlv */
  if(!(icurve->controlv = malloc(3 * icurve->length * sizeof(double))))
    {
      free(icurve);
      return AY_EOMEM;
    }
  memcpy(icurve->controlv, icurvesrc->controlv,
	 3 * icurve->length * sizeof(double));

  icurve->ncurve = NULL;

  *dst = (void *)icurve;

 return AY_OK;
} /* ay_icurve_copycb */


/* ay_icurve_drawcb:
 *  draw (display in an Ayam view window) callback function of icurve object
 */
int
ay_icurve_drawcb(struct Togl *togl, ay_object *o)
{
 ay_icurve_object *icurve = NULL;
 ay_nurbcurve_object *ncurve = NULL;
 int display_mode = ay_prefs.nc_display_mode;
 int length = 0, i = 0, a = 0, drawch = AY_FALSE;

  if(!o)
    return AY_ENULL;

  icurve = (ay_icurve_object *)o->refine;

  if(!icurve)
    return AY_ENULL;

  if(icurve->display_mode != 0)
    {
      display_mode = icurve->display_mode-1;
    }

  switch(display_mode)
    {
    case 0: /* ControlHull */
      drawch = AY_TRUE;
      break;
    case 1: /* CurveAndHull (GLU) */
      drawch = AY_TRUE;
      if(icurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)icurve->ncurve->refine;
	  ncurve->display_mode = 3;
	  ay_draw_object(togl, icurve->ncurve, AY_TRUE);
	}
      break;
    case 2: /* Curve (GLU) */
      if(icurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)icurve->ncurve->refine;
	  ncurve->display_mode = 3;
	  ay_draw_object(togl, icurve->ncurve, AY_TRUE);
	}
      break;
    case 3: /* CurveAndHull (STESS) */
      drawch = AY_TRUE;
      if(icurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)icurve->ncurve->refine;
	  ncurve->display_mode = 5;
	  ay_draw_object(togl, icurve->ncurve, AY_TRUE);
	}
      break;
    case 4: /* Curve (STESS) */
      if(icurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)icurve->ncurve->refine;
	  ncurve->display_mode = 5;
	  ay_draw_object(togl, icurve->ncurve, AY_TRUE);
	}
      break;
    } /* switch */

  if(drawch)
    {
      length = icurve->length;
      a = 0;
      glBegin(GL_LINE_STRIP);
       for(i = 0; i < length; i++)
	 {
	   glVertex3dv((GLdouble *)&(icurve->controlv[a]));
	   a += 3;
	 }
      glEnd();
    } /* if */

  /* draw derivatives */
  if(icurve->derivs)
    {
      glBegin(GL_LINES);
       if(icurve->type)
	 glVertex3dv((GLdouble *)&(icurve->controlv[0]));
       else
	 glVertex3dv((GLdouble *)&(icurve->controlv[icurve->length*3-3]));
       glVertex3dv((GLdouble *)&(icurve->ederiv[0]));

       glVertex3dv((GLdouble *)&(icurve->controlv[0]));
       glVertex3dv((GLdouble *)&(icurve->sderiv[0]));
      glEnd();
    }

 return AY_OK;
} /* ay_icurve_drawcb */


/* ay_icurve_shadecb:
 *  shade (display in an Ayam view window) callback function of icurve object
 */
int
ay_icurve_shadecb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_icurve_shadecb */


/* ay_icurve_drawacb:
 *  draw annotations (in an Ayam view window) callback function of icurve object
 */
int
ay_icurve_drawacb(struct Togl *togl, ay_object *o)
{
 ay_icurve_object *icurve = NULL;
 GLdouble *ver = NULL;

  icurve = (ay_icurve_object *) o->refine;

  if(!icurve)
    return AY_ENULL;

  ver = icurve->controlv;

  /* draw arrow */
  if(icurve->type)
    ay_draw_arrow(togl, &(ver[icurve->length*3-3]), &(ver[0]));
  else
    ay_draw_arrow(togl, &(ver[icurve->length*3-6]),
		  &(ver[icurve->length*3-3]));

  /* draw deriv arrows? */
 return AY_OK;
} /* ay_icurve_drawacb */


/* ay_icurve_drawhcb:
 *  draw handles (in an Ayam view window) callback function of icurve object
 */
int
ay_icurve_drawhcb(struct Togl *togl, ay_object *o)
{
 int i;
 double *pnts;
 ay_icurve_object *icurve;

  icurve = (ay_icurve_object *) o->refine;

  if(!icurve)
    return AY_ENULL;

  pnts = icurve->controlv;

  glBegin(GL_POINTS);
   /* draw control points */
   for(i = 0; i < icurve->length; i++)
     {
       glVertex3dv((GLdouble *)pnts);
       pnts += 3;
     }

   /* draw deriv endpoints */
   if(icurve->derivs)
     {
       glVertex3dv((GLdouble *)&icurve->sderiv);
       glVertex3dv((GLdouble *)&icurve->ederiv);
     }
  glEnd();

 return AY_OK;
} /* ay_icurve_drawhcb */


/* ay_icurve_getpntcb:
 *  get point (editing and selection) callback function of icurve object
 */
int
ay_icurve_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_icurve_object *icurve = NULL;
 ay_point *pnt = NULL, **lastpnt = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **ctmp;
 double *control = NULL, *c = NULL;
 int i = 0, j = 0, a = 0;
 const int stride = 3;
 unsigned int *itmp, peindex = 0;

  if(!o || ((mode != 3) && (!p || !pe)))
    return AY_ENULL;

  icurve = (ay_icurve_object *)(o->refine);

  if(!icurve)
    return AY_ENULL;

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(pe)
    pe->rational = AY_FALSE;

  switch(mode)
    {
    case 0:
      /* select all points */
      if(!icurve->derivs)
	{
	  if(!(pe->coords = calloc(icurve->length, sizeof(double*))))
	    return AY_EOMEM;
	  if(!(pe->indices = calloc(icurve->length, sizeof(unsigned int))))
	    return AY_EOMEM;
	}
      else
	{
	  if(!(pe->coords = calloc(icurve->length+2, sizeof(double*))))
	    return AY_EOMEM;
	  if(!(pe->indices = calloc(icurve->length+2, sizeof(unsigned int))))
	    return AY_EOMEM;
	}

      for(i = 0; i < icurve->length; i++)
	{
	  pe->coords[i] = &(icurve->controlv[a]);
	  pe->indices[i] = i;
	  a += stride;
	}

      if(icurve->derivs)
	{
	  pe->coords[icurve->length] = icurve->sderiv;
	  pe->indices[icurve->length] = icurve->length;
	  pe->coords[icurve->length+1] = icurve->ederiv;
	  pe->indices[icurve->length+1] = icurve->length+1;
	}

      pe->num = icurve->length;
      if(icurve->derivs)
	{
	  pe->num += 2;
	}
      break;
    case 1:
      /* selection based on a single point */
      control = icurve->controlv;
      for(i = 0; i < icurve->length; i++)
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

      if(icurve->derivs)
	{
	  dist = AY_VLEN((p[0] - icurve->sderiv[0]),
			 (p[1] - icurve->sderiv[1]),
			 (p[2] - icurve->sderiv[2]));

	  if(dist < min_dist)
	    {
	      pecoord = icurve->sderiv;
	      peindex = icurve->length;
	      min_dist = dist;
	    }

	  dist = AY_VLEN((p[0] - icurve->ederiv[0]),
			 (p[1] - icurve->ederiv[1]),
			 (p[2] - icurve->ederiv[2]));

	  if(dist < min_dist)
	    {
	      pecoord = icurve->ederiv;
	      peindex = icurve->length+1;
	      /*min_dist = dist;*/
	    }
	}

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
      control = icurve->controlv;
      j = 0;
      a = 0;
      for(i = 0; i < icurve->length; i++)
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

      if(icurve->derivs)
	{
	  c = icurve->sderiv;

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

	      pe->coords[a] = icurve->sderiv;
	      pe->indices[a] = icurve->length;
	      a++;
	    } /* if */

	  c = icurve->ederiv;

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

	      pe->coords[a] = icurve->ederiv;
	      pe->indices[a] = icurve->length+1;
	      a++;
	    } /* if */
	} /* if */

      pe->num = a;

      break;
    case 3:
      /* rebuild from o->selp */
      pnt = o->selp;
      lastpnt = &(o->selp);
      while(pnt)
	{
	  if(pnt->index < (unsigned int)icurve->length)
	    {
	      pnt->point = &(icurve->controlv[pnt->index*stride]);
	      pnt->rational = AY_FALSE;
	      lastpnt = &(pnt->next);
	      pnt = pnt->next;
	    }
	  else
	    {
	      if((pnt->index < (unsigned int)(icurve->length+2)) &&
		 icurve->derivs)
		{
		  if(pnt->index == (unsigned int)icurve->length)
		    pnt->point = icurve->sderiv;
		  else
		    pnt->point = icurve->ederiv;
		}
	      else
		{
		  *lastpnt = pnt->next;
		  free(pnt);
		  pnt = *lastpnt;
		} /* if */
	    } /* if */
	} /* while */
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_icurve_getpntcb */


/* ay_icurve_setpropcb:
 *  set property (from Tcl to C context) callback function of icurve object
 */
int
ay_icurve_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "ICurveAttrData";
 char fname[] = "icurve_setpropcb";
 int a, old_deriv, new_length, new_order;
 double *cv;
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_icurve_object *icurve = NULL;

  if(!interp || !o)
    return AY_ENULL;

  icurve = (ay_icurve_object *)o->refine;

  if(!icurve)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(icurve->type));

  Tcl_SetStringObj(ton,"Length",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_length);

  old_deriv = icurve->derivs;
  Tcl_SetStringObj(ton,"Derivatives",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(icurve->derivs));

  Tcl_SetStringObj(ton,"Order",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_order);

  Tcl_SetStringObj(ton,"SDLen",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(icurve->sdlen));

  Tcl_SetStringObj(ton,"EDLen",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(icurve->edlen));

  Tcl_SetStringObj(ton,"ParamType",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(icurve->param_type));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(icurve->glu_sampling_tolerance));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(icurve->display_mode));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  if(new_length != icurve->length)
    {
      if(new_length > 2)
	{
	  ay_status = ay_ict_resize(icurve, new_length);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR,fname,"Resize failed!");
	    }
	}
      else
	{
	  ay_error(AY_ERROR,fname,"Length must be > 2!");
	}
    }

  if(new_order != icurve->order)
    {
      if(new_order > 2)
	icurve->order = new_order;
      else
	ay_error(AY_ERROR,fname,"Order must be > 2!");
    }

  /* when switching from auto to manual, create manual
     derivatives similar to the automatic ones */
  if((old_deriv == 0) && (icurve->derivs == 1))
    {
      cv = icurve->controlv;
      if(!icurve->type)
	{
	  icurve->sderiv[0] = cv[0]+(cv[3]-cv[0])*icurve->sdlen;
	  icurve->sderiv[1] = cv[1]+(cv[4]-cv[1])*icurve->sdlen;
	  icurve->sderiv[2] = cv[2]+(cv[5]-cv[2])*icurve->sdlen;

	  a = (icurve->length-2)*3;
	  icurve->ederiv[0] = cv[a+3]+(cv[a]-cv[a+3])*icurve->edlen;
	  icurve->ederiv[1] = cv[a+4]+(cv[a+1]-cv[a+4])*icurve->edlen;
	  icurve->ederiv[2] = cv[a+5]+(cv[a+2]-cv[a+5])*icurve->edlen;
	}
      else
	{
	  a = (icurve->length-1)*3;
	  icurve->sderiv[0] = cv[0]+(cv[3]-cv[a])*icurve->sdlen;
	  icurve->sderiv[1] = cv[1]+(cv[4]-cv[a+1])*icurve->sdlen;
	  icurve->sderiv[2] = cv[2]+(cv[5]-cv[a+2])*icurve->sdlen;

	  icurve->ederiv[0] = cv[0]-(cv[3]-cv[a])*icurve->edlen;
	  icurve->ederiv[1] = cv[1]-(cv[4]-cv[a+1])*icurve->edlen;
	  icurve->ederiv[2] = cv[2]-(cv[5]-cv[a+2])*icurve->edlen;
	}
    } /* if */

  o->modified = AY_TRUE;

  ay_notify_object(o);

  ay_notify_parent();

 return AY_OK;
} /* ay_icurve_setpropcb */


/* ay_icurve_getpropcb:
 *  get property (from C to Tcl context) callback function of icurve object
 */
int
ay_icurve_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1 = "ICurveAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_icurve_object *icurve = NULL;

  if(!interp || !o)
    return AY_ENULL;

  icurve = (ay_icurve_object *)(o->refine);

  if(!icurve)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_NewIntObj(icurve->type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Length",-1);
  to = Tcl_NewIntObj(icurve->length);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Derivatives",-1);
  to = Tcl_NewIntObj(icurve->derivs);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order",-1);
  to = Tcl_NewIntObj(icurve->order);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"SDLen",-1);
  to = Tcl_NewDoubleObj(icurve->sdlen);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"EDLen",-1);
  to = Tcl_NewDoubleObj(icurve->edlen);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"ParamType",-1);
  to = Tcl_NewIntObj(icurve->param_type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(icurve->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(icurve->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  ay_prop_getncinfo(interp, n1, icurve->ncurve);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_icurve_getpropcb */


/* ay_icurve_readcb:
 *  read (from scene file) callback function of icurve object
 */
int
ay_icurve_readcb(FILE *fileptr, ay_object *o)
{
 ay_icurve_object *icurve = NULL;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(icurve = calloc(1, sizeof(ay_icurve_object))))
    { return AY_EOMEM; }

  fscanf(fileptr, "%d\n", &icurve->length);
  fscanf(fileptr, "%d\n", &icurve->type); /* was: Closed */
  fscanf(fileptr, "%d\n", &a); /* was: IMode */
  fscanf(fileptr, "%d\n", &icurve->order);
  fscanf(fileptr, "%lg\n", &icurve->sdlen); /* was: IParam */
  icurve->edlen = icurve->sdlen;
  fscanf(fileptr, "%lg\n", &icurve->glu_sampling_tolerance);
  fscanf(fileptr, "%d\n", &icurve->display_mode);

  if(!(icurve->controlv = calloc(icurve->length*3, sizeof(double))))
    { free(icurve); return AY_EOMEM; }

  a = 0;
  for(i = 0; i < icurve->length; i++)
    {
      fscanf(fileptr, "%lg %lg %lg\n", &(icurve->controlv[a]),
	     &(icurve->controlv[a+1]),
	     &(icurve->controlv[a+2]));
      a += 3;
    }

  if(ay_read_version > 13)
    {
      /* Since 1.19 */
      fscanf(fileptr, "%lg\n", &icurve->edlen);
      fscanf(fileptr, "%d\n", &icurve->param_type);
      fscanf(fileptr, "%d\n", &icurve->derivs);

      fscanf(fileptr, "%lg %lg %lg\n", &(icurve->sderiv[0]),
	     &(icurve->sderiv[1]),
	     &(icurve->sderiv[2]));
      fscanf(fileptr, "%lg %lg %lg\n", &(icurve->ederiv[0]),
	     &(icurve->ederiv[1]),
	     &(icurve->ederiv[2]));
    }

  o->refine = icurve;

 return AY_OK;
} /* ay_icurve_readcb */


/* ay_icurve_writecb:
 *  write (to scene file) callback function of icurve object
 */
int
ay_icurve_writecb(FILE *fileptr, ay_object *o)
{
 ay_icurve_object *icurve = NULL;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  icurve = (ay_icurve_object *)(o->refine);

  if(!icurve)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", icurve->length);
  fprintf(fileptr, "%d\n", icurve->type); /* was: Closed */
  fprintf(fileptr, "%d\n", 0); /* was: IMode */
  fprintf(fileptr, "%d\n", icurve->order);
  fprintf(fileptr, "%g\n", icurve->sdlen); /* was: IParam */
  fprintf(fileptr, "%g\n", icurve->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", icurve->display_mode);

  a = 0;
  for(i = 0; i < icurve->length; i++)
    {
      fprintf(fileptr,"%g %g %g\n", icurve->controlv[a],
	      icurve->controlv[a+1],
	      icurve->controlv[a+2]);
      a += 3;
    }

  fprintf(fileptr, "%g\n", icurve->edlen);
  fprintf(fileptr, "%d\n", icurve->param_type);
  fprintf(fileptr, "%d\n", icurve->derivs);
  fprintf(fileptr,"%g %g %g\n", icurve->sderiv[0],
	  icurve->sderiv[1],
	  icurve->sderiv[2]);
  fprintf(fileptr,"%g %g %g\n", icurve->ederiv[0],
	  icurve->ederiv[1],
	  icurve->ederiv[2]);

 return AY_OK;
} /* ay_icurve_writecb */


/* ay_icurve_wribcb:
 *  RIB export callback function of icurve object
 */
int
ay_icurve_wribcb(char *file, ay_object *o)
{
  /*
  ay_icurve_object *icurve = NULL;

  if(!o)
   return AY_ENULL;

  icurve = (ay_icurve_object*)o->refine;
  */

 return AY_OK;
} /* ay_icurve_wribcb */


/* ay_icurve_bbccb:
 *  bounding box calculation callback function of icurve object
 */
int
ay_icurve_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_icurve_object *icurve = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  icurve = (ay_icurve_object *)o->refine;

  if(!icurve)
    return AY_ENULL;

 return ay_bbc_fromarr(icurve->controlv, icurve->length, 3, bbox);
} /* ay_icurve_bbccb */


/* ay_icurve_notifycb:
 *  notification callback function of icurve object
 */
int
ay_icurve_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 int param_type = AY_KTCHORDAL;
 ay_icurve_object *icurve = NULL;
 ay_nurbcurve_object *nc = NULL;
 ay_object *ncurve = NULL;
 int i, a, b;

  if(!o)
    return AY_ENULL;

  icurve = (ay_icurve_object *)(o->refine);

  if(!icurve)
    return AY_ENULL;

  /*
   * Do we have a child?
   * This may be only the case, if we just loaded a Mops scene file.
   * => Copy the control points over from the child, then delete it!
   */
  if(o->down)
    {
      /* yes, copy it's controlv and delete it */
      if(o->down->type == AY_IDNCURVE)
	{
	  nc = (ay_nurbcurve_object *)o->down->refine;
	  icurve->length = nc->length;
	  if(icurve->controlv)
	    free(icurve->controlv);
	  icurve->controlv = NULL;
	  if(!(icurve->controlv = calloc(icurve->length*3, sizeof(double))))
	    return AY_EOMEM;
	  a = 0; b = 0;
	  for(i = 0; i < nc->length; i++)
	    {
	      memcpy(&(icurve->controlv[a]), &(nc->controlv[b]),
		     3*sizeof(double));

	      a += 3;
	      b += 4;
	    } /* for */

	  /* delete child */
	  (void)ay_object_deletemulti(o->down, AY_FALSE);
	  o->down = NULL;
	} /* if */
    } /* if */

  /* create new interpolated curve */
  (void)ay_object_delete(icurve->ncurve);
  icurve->ncurve = NULL;

  if(!(ncurve = calloc(1, sizeof(ay_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  ay_object_defaults(ncurve);
  ncurve->type = AY_IDNCURVE;

  if(icurve->param_type == 1)
    param_type = AY_KTCENTRI;

  if(icurve->param_type == 2)
    param_type = AY_KTUNIFORM;

  if(icurve->order != 4)
    {
      if(icurve->type)
	{
	  ay_status = ay_ict_interpolateG3DClosed(icurve->order,
						  icurve->length,
					     icurve->sdlen, icurve->edlen,
					     icurve->derivs, param_type,
					     icurve->controlv,
					     icurve->sderiv, icurve->ederiv,
			  (ay_nurbcurve_object **)(void*)&(ncurve->refine));
	}
      else
	{
	  ay_status = ay_ict_interpolateG3D(icurve->order, icurve->length,
   					    icurve->sdlen, icurve->edlen,
					    icurve->derivs, param_type,
					    icurve->controlv,
					    icurve->sderiv, icurve->ederiv,
			  (ay_nurbcurve_object **)(void*)&(ncurve->refine));
	}
    }
  else
    {
      if(icurve->type)
	{
	  ay_status = ay_ict_interpolateC2CClosed(icurve->length,
       					    icurve->sdlen, icurve->edlen,
					    param_type, icurve->derivs,
					    icurve->sderiv, icurve->ederiv,
					    icurve->controlv,
			  (ay_nurbcurve_object **)(void*)&(ncurve->refine));
	}
      else
	{
	  ay_status = ay_ict_interpolateC2C(icurve->length,
       					    icurve->sdlen, icurve->edlen,
					    param_type, icurve->derivs,
					    icurve->sderiv, icurve->ederiv,
					    icurve->controlv,
		          (ay_nurbcurve_object **)(void*)&(ncurve->refine));
	}
    }

  if(ay_status)
    {
      goto cleanup;
    }

  nc = (ay_nurbcurve_object *)ncurve->refine;
  nc->display_mode = icurve->display_mode;
  nc->glu_sampling_tolerance = icurve->glu_sampling_tolerance;

  icurve->ncurve = ncurve;

  /* prevent cleanup code from doing something harmful */
  ncurve = NULL;

cleanup:

  if(ncurve)
    {
      (void)ay_object_delete(ncurve);
    }

 return ay_status;
} /* ay_icurve_notifycb */


/* ay_icurve_convertcb:
 *  convert callback function of icurve object
 */
int
ay_icurve_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_icurve_object *ic = NULL;
 ay_object *new = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  ic = (ay_icurve_object *) o->refine;

  if(!ic)
    return AY_ENULL;

  if(ic->ncurve)
    {
      ay_status = ay_object_copy(ic->ncurve, &new);

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

	  if(ic->type == 1)
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
} /* ay_icurve_convertcb */


/* ay_icurve_providecb:
 *  provide callback function of icurve object
 */
int
ay_icurve_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 ay_icurve_object *ic = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNCURVE)
	return AY_OK;
      else
	return AY_ERROR;
    }

  ic = (ay_icurve_object *) o->refine;

  if(!ic)
    return AY_ENULL;

  if(type == AY_IDNCURVE)
    {
      if(ic->ncurve)
	{
	  ay_status = ay_object_copy(ic->ncurve, result);
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
} /* ay_icurve_providecb */


/* ay_icurve_init:
 *  initialize the icurve object module
 */
int
ay_icurve_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_icurve_name,
				    ay_icurve_createcb,
				    ay_icurve_deletecb,
				    ay_icurve_copycb,
				    ay_icurve_drawcb,
				    ay_icurve_drawhcb,
				    NULL, /* no shading */
				    ay_icurve_setpropcb,
				    ay_icurve_getpropcb,
				    ay_icurve_getpntcb,
				    ay_icurve_readcb,
				    ay_icurve_writecb,
				    NULL, /* no RIB export */
				    ay_icurve_bbccb,
				    AY_IDICURVE);

  ay_status += ay_draw_registerdacb(ay_icurve_drawacb, AY_IDICURVE);

  ay_status += ay_notify_register(ay_icurve_notifycb, AY_IDICURVE);

  ay_status += ay_convert_register(ay_icurve_convertcb, AY_IDICURVE);

  ay_status += ay_provide_register(ay_icurve_providecb, AY_IDICURVE);

  /* icurve objects may not be associated with materials */
  ay_matt_nomaterial(AY_IDICURVE);

 return ay_status;
} /* ay_icurve_init */

