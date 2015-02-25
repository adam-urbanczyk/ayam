/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2005 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* ncurve.c - ncurve object */

static char *ay_ncurve_name = "NCurve";

static Tcl_Obj *arrobj = NULL;
static Tcl_Obj *typeobj = NULL;
static Tcl_Obj *lengthobj = NULL;
static Tcl_Obj *orderobj = NULL;
static Tcl_Obj *creatempobj = NULL;
static Tcl_Obj *knottypeobj = NULL;
static Tcl_Obj *knotsobj = NULL;
static Tcl_Obj *tolobj = NULL;
static Tcl_Obj *dmobj = NULL;
static Tcl_Obj *knotsmodobj = NULL;
static Tcl_Obj *isratobj = NULL;
static Tcl_Obj *isdegenobj = NULL;


/* prototypes of functions local to this module: */

void ay_ncurve_cacheflt(ay_nurbcurve_object *ncurve);

int ay_ncurve_drawstess(ay_view_object *view, ay_nurbcurve_object *ncurve);

int ay_ncurve_drawglu(ay_view_object *view, ay_nurbcurve_object *ncurve);

int ay_ncurve_drawch(ay_nurbcurve_object *ncurve);


/* functions: */

/* ay_ncurve_createcb:
 *  create callback function of ncurve object
 */
int
ay_ncurve_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 int tcl_status = TCL_OK;
 char fname[] = "crtncurve";
 char option_handled = AY_FALSE;
 int center = AY_FALSE, createmp = -1;
 int stride = 4, order = 4, length = 4, kt = AY_KTNURB, optnum = 0;
 int i = 2, j = 0;
 int acvlen = 0, akvlen = 0;
 char **acv = NULL, **akv = NULL;
 double *cv = NULL, *kv = NULL, *tmp = NULL;
 double dx = 0.25, dy = 0.0, dz = 0.0;
 double s[3] = {0};

  if(!o)
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
		case 'r':
		  /* -createmp */
		  tcl_status = Tcl_GetBoolean(ay_interp, argv[i+1], &createmp);
		  option_handled = AY_TRUE;
		  break;
		default:
		  break;
		} /* switch */
	      break;
	    case 'k':
	      switch(argv[i][2])
		{
		case 'n':
		  /* -kn */
		  if(kv)
		    {
		      free(kv);
		      kv = NULL;
		    }
		  tcl_status = ay_tcmd_convdlist(argv[i+1], &akvlen, &kv);
		  option_handled = AY_TRUE;
		  break;
		case 't':
		  /* -kt */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &kt);
		  option_handled = AY_TRUE;
		  break;
		case 'v':
		  /* -kv */
		  if(Tcl_SplitList(ay_interp, argv[i+1], &akvlen, &akv) ==
		     TCL_OK)
		    {
		      if(kv)
			{
			  free(kv);
			}
		      if(!(kv = calloc(akvlen, sizeof(double))))
			{
			  Tcl_Free((char *) akv);
			  ay_status = AY_EOMEM;
			  goto cleanup;
			}
		      for(j = 0; j < akvlen; j++)
			{
			  tcl_status = Tcl_GetDouble(ay_interp,
						     akv[j], &kv[j]);
			  if(tcl_status != TCL_OK)
			    {
			      break;
			    }
			} /* for */
		      Tcl_Free((char *) akv);
		    }
		  option_handled = AY_TRUE;
		  break;
		default:
		  break;
		} /* switch */
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

  if(length < order)
    {
      order = length;
    }

  if(cv)
    {
      /* check length of user provided control point array */
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
	}
      /*
	in case we got less control points than we
	originally needed, extrapolate the last point(s)
	using dx,dy,dz;
	this provides a way for easy specification of
	a curve with an arbitrary starting point:
	just provide one control point, we take care
	of the rest here...
      */
      if(acvlen/stride < length)
	{
	  if(acvlen < stride)
	    j = 1;
	  else
	    j = acvlen/stride;
	  for(i = j; i < length; i++)
	    {
	      cv[i*4]   = cv[(i-1)*stride]   + dx;
	      cv[i*4+1] = cv[(i-1)*stride+1] + dy;
	      cv[i*4+2] = cv[(i-1)*stride+2] + dz;
	      cv[i*4+3] = cv[(i-1)*stride+3];
	    }
	}
      /* check and correct the weights */
      for(i = 0; i < length; i++)
	{
	  if(fabs(cv[i*4+3]) < AY_EPSILON)
	    cv[i*4+3] = 1.0;
	}
    } /* if */

  if(kv)
    {
      if(ay_knots_check(length, order, akvlen, kv))
	{
	  /* knot check failed,
	     discard user delivered knots and
	     switch back knot type to AY_KTNURB */
	  free(kv);
	  kv = NULL;
	  if(kt == AY_KTCUSTOM)
	    {
	      kt = AY_KTNURB;
	    }
	}
      else
	{
	  /* knot check ok,
	     since the user delivered own knots he probably wants the
	     knot type set to AY_KTCUSTOM in any case */
	  kt = AY_KTCUSTOM;
	}
    }

  if(kt < 0 || kt > 5 || (kt == AY_KTCUSTOM && !kv))
    {
      kt = AY_KTNURB;
    }

  if((ay_status = ay_nct_create(order, length, kt, cv, kv,
				(ay_nurbcurve_object**)(void*)&(o->refine))))
    {
      ay_error(ay_status, fname, NULL);
      ay_status = AY_ERROR;
      goto cleanup;
    }

  if(!cv)
    {
      cv = ((ay_nurbcurve_object*)(o->refine))->controlv;

      if(center)
	{
	  if(dx > 0.0)
	    s[0] = -(((length-1)*dx)/2.0);
	  if(dy > 0.0)
	    s[1] = -(((length-1)*dy)/2.0);
	  if(dz > 0.0)
	    s[2] = -(((length-1)*dz)/2.0);
	}

      for(i = 0; i < (length); i++)
	{
	  cv[i*4]   = s[0]+(double)i*dx;
	  cv[i*4+1] = s[1]+(double)i*dy;
	  cv[i*4+2] = s[2]+(double)i*dz;
	  cv[i*4+3] = 1.0;
	}
    }

  ay_nct_settype((ay_nurbcurve_object*)(o->refine));

  if(createmp>-1)
    {
      ((ay_nurbcurve_object*)(o->refine))->createmp = createmp;
    }
  else
    {
      ((ay_nurbcurve_object*)(o->refine))->createmp = AY_TRUE;
    }

  /* prevent cleanup code from doing something harmful */
  cv = NULL;
  kv = NULL;

cleanup:

  if(cv)
    free(cv);

  if(kv)
    free(kv);

  if(ay_status == AY_EOMEM)
    {
      ay_error(AY_EOMEM, fname, NULL);
      ay_status = AY_ERROR;
    }

 return ay_status;
} /* ay_ncurve_createcb */


/* ay_ncurve_deletecb:
 *  delete callback function of ncurve object
 */
int
ay_ncurve_deletecb(void *c)
{
 ay_nurbcurve_object *ncurve;

  if(!c)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)(c);

  /* free knots */
  if(ncurve->knotv)
    free(ncurve->knotv);

  /* free controlv */
  if(ncurve->controlv)
    free(ncurve->controlv);

  /* free multipe points */
  ay_nct_clearmp(ncurve);

  /* free gluNurbsRenderer */
  if(ncurve->no)
    gluDeleteNurbsRenderer(ncurve->no);

  if(ncurve->fltcv)
    free(ncurve->fltcv);

  /* free (simple) tesselation */
  if(ncurve->tessv)
    free(ncurve->tessv);

  free(ncurve);

 return AY_OK;
} /* ay_ncurve_deletecb */


/* ay_ncurve_copycb:
 *  copy callback function of ncurve object
 */
int
ay_ncurve_copycb(void *src, void **dst)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *ncurve, *ncurvesrc;
 int knot_count;

  if(!src || !dst)
    return AY_ENULL;

  ncurvesrc = (ay_nurbcurve_object *)src;

  if(!(ncurve = malloc(sizeof(ay_nurbcurve_object))))
    return AY_EOMEM;

  memcpy(ncurve, src, sizeof(ay_nurbcurve_object));

  ncurve->no = NULL;
  ncurve->fltcv = NULL;
  ncurve->knotv = NULL;
  ncurve->controlv = NULL;
  ncurve->tessv = NULL;
  ncurve->tesslen = 0;

  /* copy knots */
  knot_count = ncurve->order + ncurve->length;
  if(!(ncurve->knotv = malloc(knot_count * sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }
  memcpy(ncurve->knotv, ncurvesrc->knotv, knot_count * sizeof(double));

  /* copy controlv */
  if(!(ncurve->controlv = malloc(4 * ncurve->length * sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }
  memcpy(ncurve->controlv, ncurvesrc->controlv,
	 4 * ncurve->length * sizeof(double));

  /* copy mpoints */
  ncurve->mpoints = NULL;
  if(ncurvesrc->mpoints)
    {
      ay_nct_recreatemp(ncurve);
    }

  /* return result */
  *dst = (void *)ncurve;

  /* prevent cleanup code from doing something harmful */
  ncurve = NULL;

cleanup:

  if(ncurve)
    {
      if(ncurve->controlv)
	free(ncurve->controlv);
      if(ncurve->knotv)
	free(ncurve->knotv);
      if(ncurve->tessv)
	free(ncurve->tessv);
      free(ncurve);
    }

 return ay_status;
} /* ay_ncurve_copycb */


/* ay_ncurve_drawstess:
 *  internal helper function
 *  draw the curve using STESS
 */
int
ay_ncurve_drawstess(ay_view_object *view, ay_nurbcurve_object *ncurve)
{
 int ay_status = AY_OK;
 int a = 0, i, tesslen, tstride;
 double *tessv;
 int qf = ay_prefs.stess_qf;

  if(ncurve->order == 2)
    {
      tstride = 4;
      tessv = ncurve->controlv;
      tesslen = ncurve->length;
    }
  else
    {
      if((ncurve->glu_sampling_tolerance > 0.0) && !view->action_state)
	{
	  qf = ay_stess_GetQF(ncurve->glu_sampling_tolerance);
	}

      if(ncurve->tessqf != qf)
	{
	  if(ncurve->tessv)
	    {
	      free(ncurve->tessv);
	      ncurve->tessv = NULL;
	      ncurve->tesslen = 0;
	    }
	}

      if(!ncurve->tessv)
	{
	  ay_status = ay_stess_CurvePoints3D(ncurve->length, ncurve->order-1,
		         ncurve->knotv, ncurve->controlv, ncurve->is_rat, qf,
					   &ncurve->tesslen, &ncurve->tessv);

	  if(ay_status)
	    {
	      return ay_status;
	    }
	}
      tstride = 3;
      tessv = ncurve->tessv;
      tesslen = ncurve->tesslen;
      ncurve->tessqf = qf;
    } /* if */

  if(tesslen)
    {
      glBegin(GL_LINE_STRIP);
      for(i = 0; i < tesslen; i++)
	{
	  glVertex3dv(&(tessv[a]));
	  a += tstride;
	}
      glEnd();
    } /* if */

 return AY_OK;
} /* ay_ncurve_drawstess */


/* ay_ncurve_cacheflt
 *  internal helper function
 *  cache knots and control vertices as floats (for GLU)
 */
void
ay_ncurve_cacheflt(ay_nurbcurve_object *ncurve)
{
 int knot_count, i, a, b;
 float *flt;
 double w;

  knot_count = ncurve->order+ncurve->length;

  if(ncurve->fltcv)
    free(ncurve->fltcv);

  if((ncurve->fltcv = malloc((knot_count+ncurve->length*(ncurve->is_rat?4:3)) *
			     sizeof(GLfloat))) == NULL)
    return;

  flt = ncurve->fltcv;

  for(i = 0; i < knot_count; i++)
    {
      flt[i] = (GLfloat)ncurve->knotv[i];
    }
  a = knot_count;
  b = 0;
  if(ncurve->is_rat)
    {
      for(i = 0; i < ncurve->length; i++)
	{
	  w = ncurve->controlv[b+3];
	  flt[a] = (GLfloat)(ncurve->controlv[b]*w);
	  a++; b++;
	  flt[a] = (GLfloat)(ncurve->controlv[b]*w);
	  a++; b++;
	  flt[a] = (GLfloat)(ncurve->controlv[b]*w);
	  a++; b++;
	  flt[a] = (GLfloat)ncurve->controlv[b];
	  a++; b++;
	} /* for */
    }
  else
    {
      for(i = 0; i < ncurve->length; i++)
	{
	  flt[a] = (GLfloat)ncurve->controlv[b];
	  a++; b++;
	  flt[a] = (GLfloat)ncurve->controlv[b];
	  a++; b++;
	  flt[a] = (GLfloat)ncurve->controlv[b];
	  a++; b+=2;
	} /* for */
    } /* if */

 return;
} /* ay_ncurve_cacheflt */


/* ay_ncurve_drawglu:
 *  internal helper function
 *  draw the curve using GLU
 */
int
ay_ncurve_drawglu(ay_view_object *view, ay_nurbcurve_object *ncurve)
{
 int knot_count;
 GLdouble sampling_tolerance = ay_prefs.glu_sampling_tolerance;

 if((ncurve->glu_sampling_tolerance > 0.0) && !view->action_state)
    sampling_tolerance = ncurve->glu_sampling_tolerance;

  knot_count = ncurve->length + ncurve->order;

  if(!ncurve->fltcv)
    {
      ay_ncurve_cacheflt(ncurve);
    }

  if(!ncurve->fltcv)
    {
      return AY_ERROR;
    }

#ifndef AYWITHAQUA
  if(!ncurve->no)
  {
#endif /* !AYWITHAQUA */
      ncurve->no = gluNewNurbsRenderer();
      if(ncurve->no == NULL)
	{
	  return AY_EOMEM;
	}
#ifndef AYWITHAQUA
      } /* if */
#endif /* !AYWITHAQUA */

  gluNurbsCallback(ncurve->no, GLU_ERROR, AYGLUCBTYPE ay_error_glucb);
  gluBeginCurve(ncurve->no);

#ifdef AYIRIXBUG
   gluNurbsProperty(ncurve->no, GLU_NURBS_MODE_EXT, GLU_NURBS_RENDERER_EXT);
#endif /* AYIRIXBUG */

   gluNurbsProperty(ncurve->no, GLU_SAMPLING_TOLERANCE,
		    (GLfloat)sampling_tolerance);

   gluNurbsProperty(ncurve->no, GLU_CULLING, GL_TRUE);

   gluNurbsCurve(ncurve->no, (GLint)knot_count, (GLfloat*)ncurve->fltcv,
		 (GLint)(ncurve->is_rat?4:3),
		 (GLfloat*)&(ncurve->fltcv[knot_count]),
		 (GLint)ncurve->order,
		 (ncurve->is_rat?GL_MAP1_VERTEX_4:GL_MAP1_VERTEX_3));

  gluEndCurve(ncurve->no);

#ifdef AYWITHAQUA
  gluDeleteNurbsRenderer(ncurve->no);
  ncurve->no = NULL;
#endif /* AYWITHAQUA */

 return AY_OK;
} /* ay_ncurve_drawglu */


/* ay_ncurve_drawch:
 *  internal helper function
 *  draw the control hull of the curve
 */
int
ay_ncurve_drawch(ay_nurbcurve_object *ncurve)
{
 int a, i;
 double w;

  a = 0;
  glBegin(GL_LINE_STRIP);
   if(ncurve->is_rat && ay_prefs.rationalpoints)
     {
       for(i = 0; i < ncurve->length; i++)
	 {
	   w = ncurve->controlv[a+3];
	   glVertex3d((GLdouble)(ncurve->controlv[a]*w),
		      (GLdouble)(ncurve->controlv[a+1]*w),
		      (GLdouble)(ncurve->controlv[a+2]*w));
	   a += 4;
	 }
     }
   else
     {
       for(i = 0; i < ncurve->length; i++)
	 {
	   glVertex3dv((GLdouble *)&(ncurve->controlv[a]));
	   a += 4;
	 }
     }
  glEnd();

 return AY_OK;
} /* ay_ncurve_drawch */


/* ay_ncurve_drawcb:
 *  draw (display in an Ayam view window) callback function of ncurve object
 */
int
ay_ncurve_drawcb(struct Togl *togl, ay_object *o)
{
 ay_nurbcurve_object *ncurve;
 int display_mode = ay_prefs.nc_display_mode;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);

  if(!o)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)o->refine;

  if(!ncurve)
    return AY_ENULL;

  /* object local mode override */
  if((ncurve->display_mode != 0) && !view->action_state)
    {
      display_mode = ncurve->display_mode-1;
    }

  switch(display_mode)
    {
    case 0: /* ControlHull */
      ay_ncurve_drawch(ncurve);
      break;
    case 1: /* CurveAndHull (GLU) */
      ay_ncurve_drawch(ncurve);
      ay_ncurve_drawglu(view, ncurve);
      break;
    case 2: /* Curve (GLU) */
      ay_ncurve_drawglu(view, ncurve);
      break;
    case 3: /* CurveAndHull (STESS) */
      ay_ncurve_drawch(ncurve);
      ay_ncurve_drawstess(view, ncurve);
      break;
    case 4: /* Curve (STESS) */
      ay_ncurve_drawstess(view, ncurve);
      break;
    } /* switch */

 return AY_OK;
} /* ay_ncurve_drawcb */


/* ay_ncurve_shadecb:
 *  shade (display in an Ayam view window) callback function of ncurve object
 */
int
ay_ncurve_shadecb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_ncurve_shadecb */


/* ay_ncurve_drawacb:
 *  draw annotations (in an Ayam view window) callback function of ncurve object
 */
int
ay_ncurve_drawacb(struct Togl *togl, ay_object *o)
{
 ay_nurbcurve_object *ncurve;
 double *ver = NULL;

  if(!o)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)o->refine;

  if(!ncurve)
    return AY_ENULL;

  ver = ncurve->controlv;

  ay_draw_arrow(togl, &(ver[ncurve->length*4-8]),
		&(ver[ncurve->length*4-4]));

 return AY_OK;
} /* ay_ncurve_drawacb */


/* ay_ncurve_drawhcb:
 *  draw handles (in an Ayam view window) callback function of ncurve object
 */
int
ay_ncurve_drawhcb(struct Togl *togl, ay_object *o)
{
 int i;
 double w, *pnts;
 double point_size = ay_prefs.handle_size;
 ay_mpoint *mp;
 ay_nurbcurve_object *ncurve;

  if(!o)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *) o->refine;

  if(!ncurve)
    return AY_ENULL;

  pnts = ncurve->controlv;

  /* draw normal points */
  glBegin(GL_POINTS);
   if(ncurve->is_rat && ay_prefs.rationalpoints)
     {
       for(i = 0; i < ncurve->length; i++)
	 {
	   w = pnts[3];
	   glVertex3d((GLdouble)(pnts[0]*w),
		      (GLdouble)(pnts[1]*w),
		      (GLdouble)(pnts[2]*w));
	   pnts += 4;
	 }
     }
   else
     {
       for(i = 0; i < ncurve->length; i++)
	 {
	   glVertex3dv((GLdouble *)pnts);
	   pnts += 4;
	 }
     }
  glEnd();

  /* draw multiple points */
  if(ncurve->mpoints)
    {
      glPointSize((GLfloat)(point_size*1.25));
      glBegin(GL_POINTS);
       mp = ncurve->mpoints;
       while(mp)
	 {
	   if(ncurve->is_rat && ay_prefs.rationalpoints)
	     {
	       pnts = mp->points[0];
	       w = pnts[3];
	       glVertex3d((GLdouble)pnts[0]*w,
			  (GLdouble)pnts[1]*w,
			  (GLdouble)pnts[2]*w);
	     }
	   else
	     {
	       glVertex3dv((GLdouble *)(mp->points[0]));
	     }
	   mp = mp->next;
	 }
      glEnd();
      glPointSize((GLfloat)point_size);
    }

 return AY_OK;
} /* ay_ncurve_drawhcb */


/* ay_ncurve_getpntcb:
 *  get point (editing and selection) callback function of ncurve object
 */
int
ay_ncurve_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbcurve_object *ncurve = NULL;
 ay_point *pnt = NULL, **lastpnt = NULL;
 ay_mpoint *mp = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **ctmp;
 double *control = NULL, *c, h[3];
 int i = 0, j = 0, a = 0, found = AY_FALSE;
 unsigned int *itmp, peindex = 0;

  if(!o || ((mode != 3) && (!p || !pe)))
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)(o->refine);

  if(!ncurve)
    return AY_ENULL;

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(pe)
    pe->rational = AY_TRUE;

  switch(mode)
    {
    case 0:
      /* select all points */
      if(!(pe->coords = malloc(ncurve->length * sizeof(double*))))
	return AY_EOMEM;
      if(!(pe->indices = malloc(ncurve->length * sizeof(unsigned int))))
	return AY_EOMEM;

      for(i = 0; i < ncurve->length; i++)
	{
	  pe->coords[i] = &(ncurve->controlv[a]);
	  pe->indices[i] = i;
	  a += 4;
	}

      pe->num = ncurve->length;
      break;
    case 1:
      /* selection based on a single point */
      control = ncurve->controlv;
      for(i = 0; i < ncurve->length; i++)
	{
	  if(ncurve->is_rat && ay_prefs.rationalpoints)
	    {
	      dist = AY_VLEN((p[0] - (control[j]*control[j+3])),
			     (p[1] - (control[j+1]*control[j+3])),
			     (p[2] - (control[j+2]*control[j+3])));
	    }
	  else
	    {
	      dist = AY_VLEN((p[0] - control[j]),
			     (p[1] - control[j+1]),
			     (p[2] - control[j+2]));
	    }

	  if(dist < min_dist)
	    {
	      pecoord = &(control[j]);
	      peindex = i;
	      min_dist = dist;
	    }

	  j += 4;
	} /* for */

      if(!pecoord)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      if(ncurve->mpoints)
	{
	  mp = ncurve->mpoints;
	  while(mp && !found)
	    {
	      found = AY_FALSE;
	      for(i = 0; i < mp->multiplicity; i++)
		{
		  if(mp->points[i] == pecoord)
		    {
		      found = AY_TRUE;
		      pe->num = mp->multiplicity;
		      if(!(pe->coords = calloc(mp->multiplicity,
					       sizeof(double*))))
			return AY_EOMEM;
		      memcpy(pe->coords, mp->points,
			     mp->multiplicity * sizeof(double *));

		      if(!(pe->indices = calloc(mp->multiplicity,
					       sizeof(unsigned int))))
			return AY_EOMEM;
		      memcpy(pe->indices, mp->indices,
			     mp->multiplicity * sizeof(unsigned int));

		    } /* if */
		} /* for */

	      mp = mp->next;
	    } /* while */
	} /* if */

      if(!found)
	{
	  if(!(pe->coords = calloc(1, sizeof(double *))))
	    return AY_EOMEM;

	  if(!(pe->indices = calloc(1, sizeof(unsigned int))))
	    return AY_EOMEM;

	  pe->coords[0] = pecoord;
	  pe->indices[0] = peindex;
	  pe->num = 1;
	} /* if */
      break;
    case 2:
      /* selection based on planes */
      control = ncurve->controlv;
      j = 0;
      a = 0;
      if(ncurve->is_rat && ay_prefs.rationalpoints)
	{
	  c = h;
	}
      for(i = 0; i < ncurve->length; i++)
	{
	  if(ncurve->is_rat && ay_prefs.rationalpoints)
	    {
	      h[0] = control[j]*control[j+3];
	      h[1] = control[j+1]*control[j+3];
	      h[2] = control[j+2]*control[j+3];
	    }
	  else
	    {
	      c = &(control[j]);
	    }
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

	  j += 4;
	} /* for */

      pe->num = a;
      break;
    case 3:
      /* rebuild from o->selp */
      pnt = o->selp;
      lastpnt = &o->selp;
      while(pnt)
	{
	  if(pnt->index < (unsigned int)ncurve->length)
	    {
	      pnt->point = &(ncurve->controlv[pnt->index*4]);
	      pnt->rational = AY_TRUE;
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
    } /* switch */

 return AY_OK;
} /* ay_ncurve_getpntcb */


/* ay_ncurve_setpropcb:
 *  set property (from Tcl to C context) callback function of ncurve object
 */
int
ay_ncurve_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK, tcl_status = TCL_OK;
 char fname[] = "ncurve_setpropcb";
 Tcl_Obj *to = NULL;
 ay_nurbcurve_object *ncurve = NULL;
 ay_mpoint *mp = NULL;
 int new_order, new_length, new_knot_type, new_type;
 int order_modified = AY_FALSE, knots_modified = AY_FALSE;
 double *nknotv = NULL, *ncontrolv = NULL;
 int updateKnots = AY_FALSE, updateMPs = AY_TRUE;
 int knotc, i, stride = 4;
 Tcl_Obj **knotv;

  if(!interp || !o)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)o->refine;

  if(!ncurve)
    return AY_ENULL;

  /* get new values from Tcl */
  to = Tcl_ObjGetVar2(interp, arrobj, lengthobj,
		      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_length);

  to = Tcl_ObjGetVar2(interp, arrobj, orderobj,
		      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_order);

  to = Tcl_ObjGetVar2(interp, arrobj, knottypeobj,
		      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_knot_type);

  to = Tcl_ObjGetVar2(interp, arrobj, typeobj,
		      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &new_type);

  to = Tcl_ObjGetVar2(interp, arrobj, creatempobj,
		      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(ncurve->createmp));

  to = Tcl_ObjGetVar2(interp, arrobj, tolobj,
		      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(ncurve->glu_sampling_tolerance));

  to = Tcl_ObjGetVar2(interp, arrobj, dmobj,
		      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(ncurve->display_mode));

  to = Tcl_ObjGetVar2(interp, arrobj, knotsmodobj,
		      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &knots_modified);


  /* apply changed values to curve */

  /* resize curve */
  if(new_length != ncurve->length && (new_length > 1))
    {
      if(o->selp)
	{
	  ay_selp_clear(o);
	}

      ay_status = ay_nct_resize(ncurve, new_length);

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Could not resize curve!");
	}
      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    }

  /* apply new order */
  if((ncurve->order != new_order) && (new_order > 1))
    {
      /* special case for periodic curves */
      if(ncurve->type == AY_CTPERIODIC)
	{
	  if(new_length == ncurve->length)
	    {
	      if(new_order < ncurve->order)
		{
		  ncurve->length -= (ncurve->order-new_order);
		}
	      else
		{
		  new_length = ncurve->length + (new_order - ncurve->order);
		  if(!(ncontrolv = calloc(stride*new_length, sizeof(double))))
		    return AY_EOMEM;
		  memcpy(ncontrolv, ncurve->controlv,
			 ncurve->length*stride*sizeof(double));
		  free(ncurve->controlv);
		  ncurve->controlv = ncontrolv;
		  ncurve->length = new_length;
		}
	    }
	}

      ncurve->order = new_order;
      updateKnots = AY_TRUE;
      order_modified = AY_TRUE;
      o->modified = AY_TRUE;
    }

  /* change knot type */
  if((new_knot_type != ncurve->knot_type) || (updateKnots))
    {
      ncurve->knot_type = new_knot_type;
      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    }

  /* plausibility checks */
  if(ncurve->knot_type == AY_KTBEZIER)
    {
      if(ncurve->order != ncurve->length)
	{
	  ay_error(AY_EWARN, fname, "Changing order to match length!");
	  ncurve->order = ncurve->length;
	  order_modified = AY_TRUE;
	}
    }

  if(ncurve->length < ncurve->order)
    {
      ay_error(AY_EWARN, fname, "Lowering order to match length!");
      ncurve->order = ncurve->length;
    }

  /* create new knot vectors */
  if((updateKnots) && (ncurve->knot_type != AY_KTCUSTOM))
    ay_status = ay_knots_createnc(ncurve);

  /* decompose knot-list (create custom knot sequence) */
  if((ncurve->knot_type == AY_KTCUSTOM) && knots_modified)
    {
      to = Tcl_ObjGetVar2(interp, arrobj, knotsobj,
			  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      tcl_status = Tcl_ListObjGetElements(interp, to, &knotc, &knotv);
      if(tcl_status == TCL_OK)
	{
	  if(!(nknotv = malloc(knotc * sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return AY_ERROR;
	    }

	  for(i = 0; i < knotc; i++)
	    {
	      tcl_status = Tcl_GetDoubleFromObj(interp, knotv[i], &nknotv[i]);

	      if(tcl_status != TCL_OK)
		break;
	    } /* for */
	}
      if((tcl_status == TCL_OK) &&
	 !(ay_status = ay_knots_check(new_length,new_order,knotc,nknotv)))
	{
	  /* the knots are ok */
	  free(ncurve->knotv);
	  ncurve->knotv = nknotv;
	  /*
	  if(ncurve->knotv[0] != 0.0 || ncurve->knotv[
	      ncurve->length+ncurve->order-1] != 1.0)
	    {
	      ay_status = ay_knots_rescale(ncurve->length+ncurve->order,
	      ncurve->knotv);
	    }
	  */
	}
      else
	{
	  /* the knots are wrong */
	  /* first, tell the user what went wrong */
	  ay_error(AY_EOUTPUT, fname, "Checking new knots...");
	  ay_knots_printerr(fname, ay_status);

	  /* get rid of user supplied knots */
	  free(nknotv);

	  /* create new knots */
	  ay_error(AY_EWARN, fname, "Falling back to knot type NURB!");
	  ncurve->knot_type = AY_KTNURB;

	  ay_status = ay_knots_createnc(ncurve);

	  if(ay_status)
	    ay_error(AY_ERROR, fname, "Error creating new knots!");

	} /* if */

      /* XXXX compare old and new knots before setting this flag */
      o->modified = AY_TRUE;
    } /* if */

  /* close curve? */
  if(((new_type != AY_CTOPEN) && (new_type != ncurve->type)) ||
     order_modified)
    {
      /* close it */
      if(o->selp)
	{
	  ay_selp_clear(o);
	}
      o->modified = AY_TRUE;
      ncurve->type = new_type;
      ay_status = ay_nct_close(ncurve);
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Could not close curve!");
	}
      else
	{
	  if(ncurve->type == AY_CTPERIODIC)
	    {
	      if((ncurve->knot_type == AY_KTNURB) ||
		 (ncurve->knot_type == AY_KTBEZIER))
		{
		  ay_error(AY_EWARN, fname, "Changing knot type to B-Spline!");
		  ncurve->knot_type = AY_KTBSPLINE;
		  ay_status = ay_knots_createnc(ncurve);

		  if(ay_status)
		    ay_error(ay_status, fname, "Error creating new knots!");
		} /* if */
	    }
	  if(ncurve->type == AY_CTCLOSED)
	    {
	      if(ncurve->knot_type == AY_KTBSPLINE)
		{
		  ay_error(AY_EWARN, fname, "Changing knot type to NURB!");
		  ncurve->knot_type = AY_KTNURB;
		  ay_status = ay_knots_createnc(ncurve);

		  if(ay_status)
		    ay_error(ay_status, fname, "Error creating new knots!");
		} /* if */
	    } /* if */
	} /* if */
    } /* if */

  /* open curve? */
  if((new_type == AY_CTOPEN) && (ncurve->type != AY_CTOPEN))
    {
      /* open it */
      ncurve->type = AY_CTOPEN;
      /* we assume user wants the end cvs to be exploded;
	 thus, we clear all mpoints, user may re-create them easily
	 pressing apply another time (should CreateMP be enabled) */
      while(ncurve->mpoints)
	{
	  mp = ncurve->mpoints->next;
	  if(ncurve->mpoints->points)
	    free(ncurve->mpoints->points);
	  free(ncurve->mpoints);
	  ncurve->mpoints = mp;
	} /* while */
      updateMPs = AY_FALSE;
    } /* if */

  /*  if(ncurve->mpoints)*/
  if(updateMPs)
    {
      ay_nct_recreatemp(ncurve);
    }

  /* re-create stess tesselation */
  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_ncurve_setpropcb */


/* ay_ncurve_getpropcb:
 *  get property (from C to Tcl context) callback function of ncurve object
 */
int
ay_ncurve_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 Tcl_Obj *to = NULL;
 ay_nurbcurve_object *ncurve = NULL;
 int i;

  if(!interp || !o)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)(o->refine);

  if(!ncurve)
    return AY_ENULL;

  if(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'i')
    {
      to = Tcl_NewIntObj(ay_nct_isdegen(ncurve));
      Tcl_ObjSetVar2(interp, arrobj, isdegenobj, to,
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      return AY_OK;
    }

  to = Tcl_NewIntObj(ncurve->length);
  Tcl_ObjSetVar2(interp, arrobj, lengthobj, to,
		 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(ncurve->order);
  Tcl_ObjSetVar2(interp, arrobj, orderobj, to,
		 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(ncurve->type);
  Tcl_ObjSetVar2(interp, arrobj, typeobj, to,
		 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(ncurve->createmp);
  Tcl_ObjSetVar2(interp, arrobj, creatempobj, to,
		 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(ncurve->knot_type);
  Tcl_ObjSetVar2(interp, arrobj, knottypeobj, to,
		 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  to = Tcl_NewStringObj("",-1);
  Tcl_ObjSetVar2(interp, arrobj, knotsobj, to,
		 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  for(i = 0; i < ncurve->length+ncurve->order; i++)
    {
      to = Tcl_NewDoubleObj((ncurve->knotv)[i]);

      Tcl_ObjSetVar2(interp, arrobj, knotsobj, to,
		     TCL_APPEND_VALUE | TCL_LIST_ELEMENT |
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    }

  to = Tcl_NewDoubleObj(ncurve->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp, arrobj, tolobj, to,
		 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(ncurve->display_mode);
  Tcl_ObjSetVar2(interp, arrobj, dmobj, to,
		 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  if(ncurve->is_rat)
    to = Tcl_NewStringObj("yes", -1);
  else
    to = Tcl_NewStringObj("no", -1);
  Tcl_ObjSetVar2(interp, arrobj, isratobj, to,
		 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  to = Tcl_NewIntObj(0);
  Tcl_ObjSetVar2(interp, arrobj, knotsmodobj, to,
		 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

 return AY_OK;
} /* ay_ncurve_getpropcb */


/* ay_ncurve_readcb:
 *  read (from scene file) callback function of ncurve object
 */
int
ay_ncurve_readcb(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *ncurve = NULL;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(ncurve = calloc(1, sizeof(ay_nurbcurve_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&ncurve->length);
  fscanf(fileptr,"%d\n",&ncurve->order);
  fscanf(fileptr,"%d\n",&ncurve->knot_type);

  if(ncurve->knot_type == AY_KTCUSTOM)
    {
      if(!(ncurve->knotv =
	   calloc((ncurve->length + ncurve->order), sizeof(double))))
	{ free(ncurve); return AY_EOMEM; }

      for(i = 0; i<(ncurve->length + ncurve->order); i++)
	fscanf(fileptr,"%lg\n",&(ncurve->knotv[i]));
    }

  if(!(ncurve->controlv = calloc(ncurve->length*4, sizeof(double))))
    {if(ncurve->knotv){free(ncurve->knotv);} free(ncurve); return AY_EOMEM;}

  a = 0;
  for(i = 0; i < ncurve->length; i++)
    {
      fscanf(fileptr,"%lg %lg %lg %lg\n",&(ncurve->controlv[a]),
	     &(ncurve->controlv[a+1]),
	     &(ncurve->controlv[a+2]),
	     &(ncurve->controlv[a+3]));
      a += 4;
    }

  fscanf(fileptr,"%d\n",&(ncurve->type));
  fscanf(fileptr,"%lg\n",&(ncurve->glu_sampling_tolerance));
  fscanf(fileptr,"%d\n",&(ncurve->display_mode));

  if(ay_read_version >= 1)
    {
      fscanf(fileptr,"%d\n",&(ncurve->createmp));
    }
  else
    {
      ncurve->createmp = AY_TRUE;
    }

  ay_nct_recreatemp(ncurve);

  if(ay_read_version < 8)
    if(ncurve->type == AY_CTCLOSED)
      ncurve->type = AY_CTPERIODIC;

  ncurve->is_rat = ay_nct_israt(ncurve);

  /* Prior to 1.19 Ayam used pre-multiplied rational coordinates... */
  if(ncurve->is_rat && (ay_read_version < 14))
    {
      a = 0;
      for(i = 0; i < ncurve->length; i++)
	{
	  ncurve->controlv[a+1] /= ncurve->controlv[a+3];
	  ncurve->controlv[a+2] /= ncurve->controlv[a+3];
	  ncurve->controlv[a+3] /= ncurve->controlv[a+3];
	  a += 4;
	}
    }

  if(ncurve->knot_type != AY_KTCUSTOM)
    {
      ay_status = ay_knots_createnc(ncurve);
      if(ay_status)
	{ free(ncurve->controlv); free(ncurve); return ay_status; }
    }

  o->refine = ncurve;

 return AY_OK;
} /* ay_ncurve_readcb */


/* ay_ncurve_writecb:
 *  write (to scene file) callback function of ncurve object
 */
int
ay_ncurve_writecb(FILE *fileptr, ay_object *o)
{
 ay_nurbcurve_object *ncurve = NULL;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)(o->refine);

  if(!ncurve)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", ncurve->length);
  fprintf(fileptr, "%d\n", ncurve->order);
  fprintf(fileptr, "%d\n", ncurve->knot_type);

  if(ncurve->knot_type == AY_KTCUSTOM)
    {
      for(i=0;i<(ncurve->length+ncurve->order);i++)
	fprintf(fileptr,"%g\n",ncurve->knotv[i]);
    }

  a = 0;
  for(i=0;i<ncurve->length;i++)
    {
      fprintf(fileptr,"%g %g %g %g\n", ncurve->controlv[a],
	      ncurve->controlv[a+1],
	      ncurve->controlv[a+2],
	      ncurve->controlv[a+3]);
      a += 4;
    }

  fprintf(fileptr, "%d\n", ncurve->type);
  fprintf(fileptr, "%g\n", ncurve->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", ncurve->display_mode);
  fprintf(fileptr, "%d\n", ncurve->createmp);

 return AY_OK;
} /* ay_ncurve_writecb */


/* ay_ncurve_wribcb:
 *  RIB export callback function of ncurve object
 */
int
ay_ncurve_wribcb(char *file, ay_object *o)
{

  if(!o)
   return AY_ENULL;


 return AY_OK;
} /* ay_ncurve_wribcb */


/* ay_ncurve_bbccb:
 *  bounding box calculation callback function of ncurve object
 */
int
ay_ncurve_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_nurbcurve_object *ncurve = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)o->refine;

  if(!ncurve)
    return AY_ENULL;

 return ay_bbc_fromarr(ncurve->controlv, ncurve->length, 4, bbox);
} /* ay_ncurve_bbccb */


/* ay_ncurve_convertcb:
 *  convert callback function of ncurve object
 */
int
ay_ncurve_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 int i, a, b;
 ay_nurbcurve_object *nc = NULL;
 ay_icurve_object *ic = NULL;
 ay_object *new = NULL;

  if(!o)
    { return AY_ENULL; }

  nc = (ay_nurbcurve_object *) o->refine;

  if(!nc)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  ay_object_defaults(new);
  new->type = AY_IDICURVE;
  ay_trafo_copy(o, new);

  if(!(ic = calloc(1, sizeof(ay_icurve_object))))
    { free(new); return AY_EOMEM; }

  new->refine = ic;

  ic->length = nc->length;
  ic->type = nc->type>0?AY_TRUE:AY_FALSE;
  ic->glu_sampling_tolerance = nc->glu_sampling_tolerance;
  ic->display_mode = nc->display_mode;
  ic->sdlen = 1.0/8.0;
  ic->edlen = 1.0/8.0;
  ic->order = nc->order; /* XXXX ? */

  if(!(ic->controlv = calloc(1, ic->length*3*sizeof(double))))
    { free(new); free(ic); return AY_EOMEM; }

  a = 0; b = 0;
  for(i = 0; i < ic->length; i++)
    {
      memcpy(&(ic->controlv[a]), &(nc->controlv[b]), 3*sizeof(double));
      a += 3;
      b += 4;
    }

  ay_notify_object(new);

  if(!in_place)
    {
      ay_object_link(new);
    }
  else
    {
      ay_status = ay_object_replace(new, o);
    }

 return ay_status;
} /* ay_ncurve_convertcb */


/* ay_ncurve_notifycb:
 *  notification callback function of ncurve object
 */
int
ay_ncurve_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *ncurve = NULL;

  if(!o)
    return AY_ENULL;

  ncurve = (ay_nurbcurve_object *)(o->refine);

  if(!ncurve)
    return AY_ENULL;

  if(ncurve->knot_type > AY_KTCUSTOM)
    {
      ay_status = ay_knots_createnc(ncurve);
      if(ay_status)
	return ay_status;
    }

  if(o->modified)
    {
      ay_nct_settype(ncurve);
    }

  /* manage the GLU NURBS renderer */
  if(ncurve->no)
    gluDeleteNurbsRenderer(ncurve->no);
  ncurve->no = NULL;

  /* manage cached float data */
  if(ncurve->fltcv)
    {
      free(ncurve->fltcv);
      ncurve->fltcv = NULL;
    }

  /* manage the cached tesselation */
  if(ncurve->tessv)
    {
      free(ncurve->tessv);
      ncurve->tessv = NULL;
      ncurve->tesslen = 0;
    }

 return AY_OK;
} /* ay_ncurve_notifycb */


/* ay_ncurve_init:
 *  initialize the ncurve object module
 */
int
ay_ncurve_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_ncurve_name,
				    ay_ncurve_createcb,
				    ay_ncurve_deletecb,
				    ay_ncurve_copycb,
				    ay_ncurve_drawcb,
				    ay_ncurve_drawhcb,
				    NULL, /* no shading */
				    ay_ncurve_setpropcb,
				    ay_ncurve_getpropcb,
				    ay_ncurve_getpntcb,
				    ay_ncurve_readcb,
				    ay_ncurve_writecb,
				    NULL, /* no RIB export */
				    ay_ncurve_bbccb,
				    AY_IDNCURVE);

  ay_status += ay_draw_registerdacb(ay_ncurve_drawacb, AY_IDNCURVE);

  ay_status += ay_convert_register(ay_ncurve_convertcb, AY_IDNCURVE);

  ay_status += ay_notify_register(ay_ncurve_notifycb, AY_IDNCURVE);

  /* ncurve objects may not be associated with materials */
  ay_matt_nomaterial(AY_IDNCURVE);

  arrobj = Tcl_NewStringObj("NCurveAttrData",-1);
  typeobj = Tcl_NewStringObj("Type",-1);
  lengthobj = Tcl_NewStringObj("Length",-1);
  orderobj = Tcl_NewStringObj("Order",-1);
  creatempobj = Tcl_NewStringObj("CreateMP",-1);
  knottypeobj = Tcl_NewStringObj("Knot-Type",-1);
  knotsobj = Tcl_NewStringObj("Knots",-1);
  tolobj = Tcl_NewStringObj("Tolerance",-1);
  dmobj = Tcl_NewStringObj("DisplayMode",-1);
  knotsmodobj = Tcl_NewStringObj("Knots-Modified",-1);
  isratobj = Tcl_NewStringObj("IsRat",-1);
  isdegenobj = Tcl_NewStringObj("IsDegen",-1);

 return ay_status;
} /* ay_ncurve_init */

