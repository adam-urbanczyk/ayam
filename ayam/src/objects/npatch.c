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

/* npatch.c - npatch object */

static char *ay_npatch_name = "NPatch";

/* prototypes of functions local to this module: */

void ay_npatch_cacheflt(ay_nurbpatch_object *npatch);

int ay_npatch_drawstess(ay_view_object *view, ay_object *o);

int ay_npatch_drawglu(ay_view_object *view, ay_object *o);

int ay_npatch_drawch(ay_nurbpatch_object *npatch);

int ay_npatch_shadestess(ay_view_object *view, ay_object *o);

int ay_npatch_shadeglu(ay_view_object *view, ay_object *o);

int ay_npatch_shadech(ay_nurbpatch_object *npatch);

/* Export trim curves to RIB. */
void ay_npatch_wribtrimcurve(ay_object *o,
			     RtFloat *min, RtFloat *max, RtFloat *knot,
			     RtFloat *u, RtFloat *v, RtFloat *w,
			     RtInt *n, RtInt *order,
			     int *a, int *b, int *c);

int ay_npatch_wribtrimcurves(ay_object *o);

/* functions: */

/* ay_npatch_createcb:
 *  create callback function of npatch object
 */
int
ay_npatch_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 int tcl_status = TCL_OK;
 char fname[] = "crtnpatch";
 char option_handled = AY_FALSE;
 int center = AY_FALSE, createmp = -1;
 int stride = 4, uorder = 4, vorder = 4, width = 4, height = 4;
 int ukt = AY_KTNURB, vkt = AY_KTNURB, optnum = 0, i = 2, j = 0, k = 0;
 int acvlen = 0, aukvlen = 0, avkvlen = 0;
 char **acv = NULL, **akv = NULL;
 double *cv = NULL, *ukv = NULL, *vkv = NULL;
 double udx = 0.25, udy = 0.0, udz = 0.0;
 double vdx = 0.0, vdy = 0.25, vdz = 0.0;
 double ext = 0.0, s[3] = {0};
 ay_nurbpatch_object *npatch = NULL;

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
	    case 'w':
	      /* -width */
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &width);
	      option_handled = AY_TRUE;
	      break;
	    case 'h':
	      /* -height */
	      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &height);
	      option_handled = AY_TRUE;
	      break;
	    case 'u':
	      switch(argv[i][2])
		{
		case 'd':
		  switch(argv[i][3])
		    {
		    case 'x':
		      /* -udx */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &udx);
		      option_handled = AY_TRUE;
		      break;
		    case 'y':
		      /* -udy */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &udy);
		      option_handled = AY_TRUE;
		      break;
		    case 'z':
		      /* -udz */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &udz);
		      option_handled = AY_TRUE;
		      break;
		    default:
		      break;
		    } /* switch */
		  break;
		case 'o':
		  /* -uorder */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &uorder);
		  option_handled = AY_TRUE;
		  break;
		case 'k':
		  switch(argv[i][3])
		    {
		    case 'n':
		      /* -uknotv */
		      if(Tcl_SplitList(ay_interp, argv[i+1], &aukvlen, &akv) ==
			 TCL_OK)
			{
			  if(ukv)
			    {
			      free(ukv);
			    }
			  if(!(ukv = calloc(aukvlen, sizeof(double))))
			    {
			      Tcl_Free((char *) akv);
			      ay_status = AY_EOMEM;
			      goto cleanup;
			    }
			  for(j = 0; j < aukvlen; j++)
			    {
			      tcl_status = Tcl_GetDouble(ay_interp,
							 akv[j], &ukv[j]);
			      if(tcl_status != TCL_OK)
				{
				  break;
				}
			    } /* for */
			  Tcl_Free((char *) akv);
			}
		      option_handled = AY_TRUE;
		      break;
		    case 't':
		      /* -uktype */
		      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &ukt);
		      option_handled = AY_TRUE;
		      break;
		    default:
		      break;
		    } /* switch */
		  break;
		case 'n':
		  /* -un */
		  if(ukv)
		    {
		      free(ukv);
		      ukv = NULL;
		    }
		  tcl_status = ay_tcmd_convdlist(argv[i+1], &aukvlen, &ukv);
		  option_handled = AY_TRUE;
		  break;
		default:
		  break;
		} /* switch */
	      break;
	    case 'v':
	      switch(argv[i][2])
		{
		case 'd':
		  switch(argv[i][3])
		    {
		    case 'x':
		      /* -vdx */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &vdx);
		      option_handled = AY_TRUE;
		      break;
		    case 'y':
		      /* -vdy */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &vdy);
		      option_handled = AY_TRUE;
		      break;
		    case 'z':
		      /* -vdz */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &vdz);
		      option_handled = AY_TRUE;
		      break;
		    default:
		      break;
		    } /* switch */
		  break;
		case 'o':
		  /* -vorder */
		  tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &vorder);
		  option_handled = AY_TRUE;
		  break;
		case 'k':
		  switch(argv[i][3])
		    {
		    case 'n':
		      /* -vknotv */
		      if(Tcl_SplitList(ay_interp, argv[i+1], &avkvlen, &akv) ==
			 TCL_OK)
			{
			  if(vkv)
			    {
			      free(vkv);
			    }
			  if(!(vkv = calloc(avkvlen, sizeof(double))))
			    {
			      Tcl_Free((char *) akv);
			      ay_status = AY_EOMEM;
			      goto cleanup;
			    }
			  for(j = 0; j < avkvlen; j++)
			    {
			      tcl_status = Tcl_GetDouble(ay_interp,
							 akv[j], &vkv[j]);
			      if(tcl_status != TCL_OK)
				{
				  break;
				}
			    } /* for */
			  Tcl_Free((char *) akv);
			}
		      option_handled = AY_TRUE;
		      break;
		    case 't':
		      /* -vktype */
		      tcl_status = Tcl_GetInt(ay_interp, argv[i+1], &vkt);
		      option_handled = AY_TRUE;
		      break;
		    default:
		      break;
		    } /* switch */
		  break;
		case 'n':
		  /* -vn */
		  if(vkv)
		    {
		      free(vkv);
		      vkv = NULL;
		    }
		  tcl_status = ay_tcmd_convdlist(argv[i+1], &avkvlen, &vkv);
		  option_handled = AY_TRUE;
		  break;
		default:
		  break;
		} /* switch */
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
		      if(!(cv = calloc(acvlen, sizeof(double))))
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

  if(uorder <= 0)
    {
      uorder = 4;
    }

  if(vorder <= 0)
    {
      vorder = 4;
    }

  if(width <= 1)
    {
      width = 4;
    }

  if(width < uorder)
    {
      uorder = width;
    }

  if(height <= 1)
    {
      height = 4;
    }

  if(height < vorder)
    {
      vorder = height;
    }

  if(cv)
    {
      /* check length of user provided control point array */
      if(acvlen/stride < width*height)
	{
	  if(acvlen > 0)
	    s[0] = cv[0];
	  if(acvlen > 1)
	    s[1] = cv[1];
	  if(acvlen > 2)
	    s[2] = cv[2];

	  free(cv);
	  cv = NULL;
	  center = AY_FALSE;
	}
    } /* if */

  if(ukv)
    {
      if(ay_knots_check(width, uorder, aukvlen, ukv))
	{
	  /* knot check failed,
	     discard user delivered knots and
	     switch back knot type to AY_KTNURB */
	  free(ukv);
	  ukv = NULL;
	  if(ukt == AY_KTCUSTOM)
	    {
	      ukt = AY_KTNURB;
	    }
	}
      else
	{
	  /* knot check ok,
	     since the user delivered own knots he probably wants the
	     knot type set to AY_KTCUSTOM in any case */
	  ukt = AY_KTCUSTOM;
	}
    }

  if(vkv)
    {
      if(ay_knots_check(height, vorder, avkvlen, vkv))
	{
	  /* knot check failed,
	     discard user delivered knots and
	     switch back knot type to AY_KTNURB */
	  free(vkv);
	  vkv = NULL;
	  if(vkt == AY_KTCUSTOM)
	    {
	      vkt = AY_KTNURB;
	    }
	}
      else
	{
	  /* knot check ok,
	     since the user delivered own knots he probably wants the
	     knot type set to AY_KTCUSTOM in any case */
	  vkt = AY_KTCUSTOM;
	}
    }

  if(ukt < 0 || ukt > 5 || (ukt == AY_KTCUSTOM && !ukv))
    {
      ukt = AY_KTNURB;
    }

  if(vkt < 0 || vkt > 5 || (vkt == AY_KTCUSTOM && !vkv))
    {
      vkt = AY_KTNURB;
    }

  if((ay_status = ay_npt_create(uorder, vorder, width, height, ukt, vkt,
				cv, ukv, vkv,
				&npatch)))
    {
      ay_error(ay_status, fname, NULL);
      ay_status = AY_ERROR;
      goto cleanup;
    }

  o->refine = npatch;
  /* allow children (trim curves) */
  o->parent = AY_TRUE;
  /* but hide them under normal circumstances */
  o->hide_children = AY_TRUE;

  if(!cv)
    {
      cv = npatch->controlv;

      if(center)
	{
	  if(fabs(udx) > AY_EPSILON)
	    ext = (width-1)*udx;
	  if(fabs(vdx) > AY_EPSILON)
	    ext += (height-1)*vdx;
	  s[0] = -(ext/2.0);
	  ext = 0.0;
	  if(fabs(udy) > AY_EPSILON)
	    ext = (width-1)*udy;
	  if(fabs(vdy) > AY_EPSILON)
	    ext += (height-1)*vdy;
	  s[1] = -(ext/2.0);
	  ext = 0.0;
	  if(fabs(udz) > AY_EPSILON)
	    ext = (width-1)*udz;
	  if(fabs(vdz) > AY_EPSILON)
	    ext += (height-1)*vdz;
	  s[2] = -(ext/2.0);
	}

      k = 0;
      for(i = 0; i < width; i++)
	{
	  for(j = 0; j < height; j++)
	    {
	      cv[k]   = s[0] + (double)j*vdx;
	      cv[k+1] = s[1] + (double)j*vdy;
	      cv[k+2] = s[2] + (double)j*vdz;
	      cv[k+3] = 1.0;
	      k += stride;
	    }
	  s[0] += udx;
	  s[1] += udy;
	  s[2] += udz;
	}
    } /* if */

  ay_npt_setuvtypes(npatch, 0);

  if(createmp>-1)
    {
      npatch->createmp = createmp;
    }
  else
    {
      npatch->createmp = AY_TRUE;
    }

  ay_object_placemark(o);

  /* prevent cleanup code from doing something harmful */
  cv = NULL;
  ukv = NULL;
  vkv = NULL;

cleanup:

  if(cv)
    free(cv);

  if(ukv)
    free(ukv);

  if(vkv)
    free(vkv);

  if(ay_status == AY_EOMEM)
    {
      ay_error(AY_EOMEM, fname, NULL);
      ay_status = AY_ERROR;
    }

 return ay_status;
} /* ay_npatch_createcb */


/* ay_npatch_deletecb:
 *  delete callback function of npatch object
 */
int
ay_npatch_deletecb(void *c)
{
 ay_nurbpatch_object *npatch = NULL;

  if(!c)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)(c);

  /* free knots */
  if(npatch->uknotv)
    free(npatch->uknotv);
  if(npatch->vknotv)
    free(npatch->vknotv);

  /* free controlv */
  if(npatch->controlv)
    free(npatch->controlv);

  /* free multiple points */
  ay_npt_clearmp(npatch);

  if(npatch->caps_and_bevels)
    {
      (void)ay_object_deletemulti(npatch->caps_and_bevels, AY_FALSE);
      npatch->caps_and_bevels = NULL;
    }

  /* free gluNurbsRenderer */
  if(npatch->no)
    gluDeleteNurbsRenderer(npatch->no);

  if(npatch->fltcv)
    free(npatch->fltcv);

  /* free tesselation */
  if(npatch->stess)
    ay_stess_destroy(npatch);

  free(npatch);

 return AY_OK;
} /* ay_npatch_deletecb */


/* ay_npatch_copycb:
 *  copy callback function of npatch object
 */
int
ay_npatch_copycb(void *src, void **dst)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *npatch = NULL, *npatchsrc = NULL;
 int kl;

  if(!src || !dst)
    return AY_ENULL;

  npatchsrc = (ay_nurbpatch_object *)src;

  if(!(npatch = malloc(sizeof(ay_nurbpatch_object))))
    return AY_EOMEM;

  memcpy(npatch, src, sizeof(ay_nurbpatch_object));

  npatch->no = NULL;
  npatch->fltcv = NULL;
  npatch->stess = NULL;

  /* copy knots */
  kl = npatch->uorder + npatch->width;
  if(!(npatch->uknotv = malloc(kl * sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }
  memcpy(npatch->uknotv, npatchsrc->uknotv, kl * sizeof(double));

  kl = npatch->vorder + npatch->height;
  if(!(npatch->vknotv = malloc(kl * sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }
  memcpy(npatch->vknotv, npatchsrc->vknotv, kl * sizeof(double));

  /* copy controlv */
  if(!(npatch->controlv = malloc(4 * npatch->width * npatch->height *
				 sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }
  memcpy(npatch->controlv, npatchsrc->controlv,
	 4 * npatch->width * npatch->height * sizeof(double));

  /* copy mpoints */
  npatch->mpoints = NULL;
  if(npatchsrc->mpoints)
    {
      ay_npt_recreatemp(npatch);
    }

  npatch->caps_and_bevels = NULL;

  /* return result */
  *dst = (void *)npatch;

  /* prevent cleanup code from doing something harmful */
  npatch = NULL;

cleanup:

  if(npatch)
    {
      if(npatch->controlv)
	free(npatch->controlv);
      if(npatch->uknotv)
	free(npatch->uknotv);
      if(npatch->vknotv)
	free(npatch->vknotv);
      free(npatch);
    }

 return ay_status;
} /* ay_npatch_copycb */


/* ay_npatch_drawstess:
 *  internal helper function
 *  draw the patch using STESS
 */
int
ay_npatch_drawstess(ay_view_object *view, ay_object *o)
{
 int ay_status = AY_OK;
 /*char fname[] = "npatch_drawstesscb";*/
 int a, i, j, tessw, tessh;
 double *tessv = NULL;
 int qf = ay_prefs.stess_qf;
 ay_nurbpatch_object *npatch = (ay_nurbpatch_object *)o->refine;

  if((npatch->glu_sampling_tolerance != 0.0) && !view->action_state)
    {
      qf = ay_stess_GetQF(npatch->glu_sampling_tolerance);
    }

  if(npatch->tessqf != qf)
    {
      if(npatch->stess)
	{
	  ay_stess_destroy(npatch);
	}
    } /* if */

  if(!npatch->stess || (npatch->stess && npatch->stess->tessw == -1))
    {
      ay_status = ay_stess_TessNP(o, qf);
      if(ay_status)
	return ay_status;
    }

  if(npatch->stess && npatch->stess->tessv)
    {
      tessv = npatch->stess->tessv;
      tessw = npatch->stess->tessw;
      tessh = npatch->stess->tessh;

      a = 0;
      for(i = 0; i < tessw; i++)
	{
	  glBegin(GL_LINE_STRIP);
	  for(j = 0; j < tessh; j++)
	    {
	      glVertex3dv(&(tessv[a]));
	      a += 6;
	    } /* for */
	  glEnd();
	} /* for */

      for(j = 0; j < tessh; j++)
	{
	  a = j * 6;
	  glBegin(GL_LINE_STRIP);
	  for(i = 0; i < tessw; i++)
	    {
	      glVertex3dv(&(tessv[a]));
	      a += (tessh*6);
	    } /* for */
	  glEnd();
	} /* for */
    }
  else
    {
      if(npatch->stess)
	{
	  ay_stess_DrawTrimmedSurface(npatch->stess);
	}
    } /* if */

 return AY_OK;
} /* ay_npatch_drawstess */


/* ay_npatch_cacheflt
 *  internal helper function
 *  cache knots and control vertices as floats (for GLU)
 */
void
ay_npatch_cacheflt(ay_nurbpatch_object *npatch)
{
 int uorder, vorder, width, height;
 int uknot_count, vknot_count, i, a, b;
 float *flt;
 double w;

  uorder = npatch->uorder;
  vorder = npatch->vorder;
  width = npatch->width;
  height = npatch->height;

  uknot_count = width + uorder;
  vknot_count = height + vorder;

  if(npatch->fltcv)
    free(npatch->fltcv);

  if(!(npatch->fltcv = malloc((width*height*(npatch->is_rat?4:3)+
			      uknot_count+vknot_count)*
			      sizeof(float))))
    return;

  flt = npatch->fltcv;
  for(i = 0; i < uknot_count; i++)
    {
      flt[i] = (GLfloat)npatch->uknotv[i];
    }
  a = uknot_count;
  for(i = 0; i < vknot_count; i++)
    {
      flt[a] = (GLfloat)npatch->vknotv[i];
      a++;
    }
  a = 0;
  b = uknot_count+vknot_count;
  if(npatch->is_rat)
    {
      for(i = 0; i < width*height; i++)
	{
	  w = npatch->controlv[a+3];
	  flt[b] = (GLfloat)(npatch->controlv[a]*w);
	  a++; b++;
	  flt[b] = (GLfloat)(npatch->controlv[a]*w);
	  a++; b++;
	  flt[b] = (GLfloat)(npatch->controlv[a]*w);
	  a++; b++;
	  flt[b] = (GLfloat)npatch->controlv[a];
	  a++; b++;
	}
    }
  else
    {
      for(i = 0; i < width*height; i++)
	{
	  flt[b] = (GLfloat)npatch->controlv[a];
	  a++; b++;
	  flt[b] = (GLfloat)npatch->controlv[a];
	  a++; b++;
	  flt[b] = (GLfloat)npatch->controlv[a];
	  a+=2; b++;
	}
    }

 return;
} /* ay_npatch_cacheflt */


/* ay_npatch_drawglu:
 *  internal helper function
 *  draw the patch using GLU
 */
int
ay_npatch_drawglu(ay_view_object *view, ay_object *o)
{
 int ay_status = AY_OK;
 int uorder, vorder, width, height, uknot_count, vknot_count;
 int display_mode = ay_prefs.np_display_mode;
 GLdouble sampling_tolerance = ay_prefs.glu_sampling_tolerance;
 ay_nurbpatch_object *npatch = (ay_nurbpatch_object *)o->refine;

  uorder = npatch->uorder;
  vorder = npatch->vorder;
  width = npatch->width;
  height = npatch->height;

  uknot_count = width + uorder;
  vknot_count = height + vorder;

  if(!npatch->fltcv)
    {
      ay_npatch_cacheflt(npatch);
    }

  if(!npatch->fltcv)
    {
      return AY_ERROR;
    }

  if((npatch->glu_sampling_tolerance > 0.0) && !view->action_state)
    sampling_tolerance = npatch->glu_sampling_tolerance;

  if((npatch->display_mode != 0) && !view->action_state)
    display_mode = npatch->display_mode-1;

#ifdef AYWITHAQUA
  glPushAttrib((GLbitfield) GL_POLYGON_BIT);
#endif /* AYWITHAQUA */

#ifndef AYWITHAQUA
  if(!npatch->no)
    {
#endif /* !AYWITHAQUA */
      npatch->no = gluNewNurbsRenderer();
      if(npatch->no == NULL)
	{
	  return AY_EOMEM;
	}
#ifndef AYWITHAQUA
    }
#endif /* !AYWITHAQUA */


  gluNurbsCallback(npatch->no, GLU_ERROR, AYGLUCBTYPE ay_error_glucb);

  gluBeginSurface(npatch->no);

  gluNurbsProperty(npatch->no, GLU_SAMPLING_TOLERANCE,
		   (GLfloat)sampling_tolerance);


  if(display_mode == 2)
    gluNurbsProperty(npatch->no, GLU_DISPLAY_MODE, GLU_OUTLINE_PATCH);
  else
    gluNurbsProperty(npatch->no, GLU_DISPLAY_MODE, GLU_OUTLINE_POLYGON);


  gluNurbsProperty(npatch->no, GLU_CULLING, GL_TRUE);

  gluNurbsSurface(npatch->no, (GLint)uknot_count, (GLfloat*)npatch->fltcv,
		  (GLint)vknot_count, (GLfloat*)&(npatch->fltcv[uknot_count]),
		  (GLint)height*(npatch->is_rat?4:3),
		  (GLint)(npatch->is_rat?4:3),
		  (GLfloat*)&(npatch->fltcv[uknot_count+vknot_count]),
		  (GLint)npatch->uorder, (GLint)npatch->vorder,
		  (npatch->is_rat?GL_MAP2_VERTEX_4:GL_MAP2_VERTEX_3));

  /* draw trimcurves */
  if(o->down && o->down->next)
    {
      ay_status = ay_npt_drawtrimcurves(o, 0);
    } /* if */

  gluEndSurface(npatch->no);

#ifdef AYWITHAQUA
  gluDeleteNurbsRenderer(npatch->no);
  npatch->no = NULL;

  glPopAttrib();
#endif /* AYWITHAQUA */

 return AY_OK;
} /* ay_npatch_drawglu */


/* ay_npatch_drawch:
 *  internal helper function
 *  draw the control hull of the patch
 */
int
ay_npatch_drawch(ay_nurbpatch_object *npatch)
{
 double *dcv;
 float *fcv;
 int i, j, a, width, height;

  width = npatch->width;
  height = npatch->height;

  if(npatch->is_rat)
    {
      if(!npatch->fltcv)
	{
	  ay_npatch_cacheflt(npatch);
	}

      if(!npatch->fltcv)
	{
	  return AY_ERROR;
	}

      fcv = &(npatch->fltcv[width+height+npatch->uorder+npatch->vorder]);
      a = 0;

      if(ay_prefs.rationalpoints)
	{
	  for(i = 0; i < width; i++)
	    {
	      glBegin(GL_LINE_STRIP);
	       for(j = 0; j < height; j++)
		 {
		   glVertex3f((GLfloat)fcv[a]*fcv[a+3],
			      (GLfloat)fcv[a+1]*fcv[a+3],
			      (GLfloat)fcv[a+2]*fcv[a+3]);
		   a += 4;
		 }
	      glEnd();
	    }

	  for(j = 0; j < height; j++)
	    {
	      a = j * 4;
	      glBegin(GL_LINE_STRIP);
	       for(i = 0; i < width; i++)
		 {
		   glVertex3f((GLfloat)fcv[a]*fcv[a+3],
			      (GLfloat)fcv[a+1]*fcv[a+3],
			      (GLfloat)fcv[a+2]*fcv[a+3]);
		   a += (4 * height);
		 }
	      glEnd();
	    }
	}
      else
	{
	  for(i = 0; i < width; i++)
	    {
	      glBegin(GL_LINE_STRIP);
	       for(j = 0; j < height; j++)
		 {
		   glVertex3fv((GLfloat *)&fcv[a]);
		   a += 4;
		 }	     
	      glEnd();
	    }

	  for(j = 0; j < height; j++)
	    {
	      a = j * 4;
	      glBegin(GL_LINE_STRIP);
	       for(i = 0; i < width; i++)
		 {
		   glVertex3fv((GLfloat *)&fcv[a]);
		   a += (4 * height);
		 }	     
	      glEnd();
	    }
	}
    }
  else
    {
      dcv = npatch->controlv;
      a = 0;
      for(i = 0; i < width; i++)
	{
	  glBegin(GL_LINE_STRIP);
	  for(j = 0; j < height; j++)
	    {
	      glVertex3dv((GLdouble *)&dcv[a]);
	      a += 4;
	    }
	  glEnd();
	}

      for(j = 0; j < height; j++)
	{
	  a = j * 4;

	  glBegin(GL_LINE_STRIP);
	  for(i = 0; i < width; i++)
	    {
	      glVertex3dv((GLdouble *)&dcv[a]);
	      a += (4 * height);
	    }
	  glEnd();
	}
    }

 return AY_OK;
} /* ay_npatch_drawchcb */


/* ay_npatch_drawcb:
 *  draw (display in an Ayam view window) callback function of npatch object
 */
int
ay_npatch_drawcb(struct Togl *togl, ay_object *o)
{
 int display_mode = ay_prefs.np_display_mode;
 ay_nurbpatch_object *npatch;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_object *b;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_ENULL;

  if((npatch->display_mode != 0) && !view->action_state)
    {
      display_mode = npatch->display_mode-1;
    }

  switch(display_mode)
    {
    case 0: /* ControlHull */
      ay_npatch_drawch(npatch);
      break;
    case 1: /* OutlinePolygon (GLU) */
      ay_npatch_drawglu(view, o);
      break;
    case 2: /* OutlinePatch (GLU) */
      ay_npatch_drawglu(view, o);
      break;
    case 3: /* OutlinePatch (STESS) */
      ay_npatch_drawstess(view, o);
      break;
    default:
      break;
    } /* switch */

  b = npatch->caps_and_bevels;
  while(b)
    {
      ay_draw_object(togl, b, AY_TRUE);
      b = b->next;
    }

 return AY_OK;
} /* ay_npatch_drawcb */


/* ay_npatch_shadech:
 *  internal helper function
 *  shade the patch control polygon
 */
int
ay_npatch_shadech(ay_nurbpatch_object *npatch)
{
 int a, b, c, d, i, j;
 float *fcv;

  if(npatch->stess && npatch->stess->tessw != -1)
    ay_stess_destroy(npatch);

  if(!npatch->stess)
    {
      if(!(npatch->stess = calloc(1, sizeof(ay_stess))))
	return AY_EOMEM;
      if(!(npatch->stess->tessv = calloc(npatch->width*npatch->height*3,
					 sizeof(double))))
	{free(npatch->stess); npatch->stess = NULL; return AY_EOMEM;}

      ay_npt_getcvnormals(npatch, npatch->stess->tessv);
      npatch->stess->tessw = -1;
    } /* if */

  if(npatch->is_rat)
    {
      if(!npatch->fltcv)
	{
	  ay_npatch_cacheflt(npatch);
	}

      if(!npatch->fltcv)
	{
	  return AY_ERROR;
	}

      fcv = &(npatch->fltcv[npatch->width+npatch->height+
			    npatch->uorder+npatch->vorder]);
      for(i = 0; i < npatch->width-1; i++)
	{
	  a = i*npatch->height*4;
	  b = a+(npatch->height*4);
	  c = i*npatch->height*3;
	  d = c+(npatch->height*3);
	  glBegin(GL_QUAD_STRIP);
	  for(j = 0; j < npatch->height; j++)
	    {
	      glNormal3dv(&(npatch->stess->tessv[c]));
	      glVertex3fv(&(fcv[a]));
	      glNormal3dv(&(npatch->stess->tessv[d]));
	      glVertex3fv(&(fcv[b]));
	      a += 4;
	      b += 4;
	      c += 3;
	      d += 3;
	    } /* for */
	  glEnd();
	} /* for */
    }
  else
    {
      for(i = 0; i < npatch->width-1; i++)
	{
	  a = i*npatch->height*4;
	  b = a+(npatch->height*4);
	  c = i*npatch->height*3;
	  d = c+(npatch->height*3);
	  glBegin(GL_QUAD_STRIP);
	  for(j = 0; j < npatch->height; j++)
	    {
	      glNormal3dv(&(npatch->stess->tessv[c]));
	      glVertex3dv(&(npatch->controlv[a]));
	      glNormal3dv(&(npatch->stess->tessv[d]));
	      glVertex3dv(&(npatch->controlv[b]));
	      a += 4;
	      b += 4;
	      c += 3;
	      d += 3;
	    } /* for */
	  glEnd();
	} /* for */
    }
 return AY_OK;
} /* ay_npatch_shadech */


/* ay_npatch_shadestess:
 *  internal helper function
 *  shade the patch using STESS
 */
int
ay_npatch_shadestess(ay_view_object *view, ay_object *o)
{
 int ay_status = AY_OK;
 int qf = ay_prefs.stess_qf;
 int a, b, i, j, tessw, tessh;
 double *tessv = NULL;
 ay_nurbpatch_object *npatch = (ay_nurbpatch_object *)o->refine;

  if((npatch->glu_sampling_tolerance != 0.0) && !view->action_state)
    {
      qf = ay_stess_GetQF(npatch->glu_sampling_tolerance);
    }

  if(npatch->tessqf != qf)
    {
      if(npatch->stess)
	{
	  ay_stess_destroy(npatch);
	}
    } /* if */

  if(!npatch->stess || (npatch->stess && npatch->stess->tessw == -1))
    {
      ay_status = ay_stess_TessNP(o, qf);
    }

  if(npatch->stess && npatch->stess->tessv)
    {
      tessv = npatch->stess->tessv;
      tessw = npatch->stess->tessw;
      tessh = npatch->stess->tessh;

      a = 0;
      b = tessh*6;
      for(i = 0; i < tessw-1; i++)
	{
	  glBegin(GL_QUAD_STRIP);
	  for(j = 0; j < tessh; j++)
	    {
	      glNormal3dv(&(tessv[a+3]));
	      glVertex3dv(&(tessv[a]));
	      glNormal3dv(&(tessv[b+3]));
	      glVertex3dv(&(tessv[b]));
	      a += 6;
	      b += 6;
	    } /* for */
	  glEnd();
	} /* for */
    }
  else
    {
      if(npatch->stess)
	{
	  (void)ay_stess_ShadeTrimmedSurface(npatch->stess);
	}
    }

 return AY_OK;
} /* ay_npatch_shadestess */


/* ay_npatch_shadeglu:
 *  internal helper function
 *  shade the patch using GLU
 */
int
ay_npatch_shadeglu(ay_view_object *view, ay_object *o)
{
 int ay_status = AY_OK;
 int uorder, vorder, width, height, uknot_count, vknot_count;
 GLdouble sampling_tolerance = ay_prefs.glu_sampling_tolerance;
 ay_nurbpatch_object *npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch->fltcv)
    {
      ay_npatch_cacheflt(npatch);
    }

  if(!npatch->fltcv)
    {
      return AY_ERROR;
    }

  uorder = npatch->uorder;
  vorder = npatch->vorder;
  width = npatch->width;
  height = npatch->height;

  uknot_count = width + uorder;
  vknot_count = height + vorder;

  if((npatch->glu_sampling_tolerance > 0.0) && !view->action_state)
    sampling_tolerance = npatch->glu_sampling_tolerance;

#ifndef AYWITHAQUA
  if(!npatch->no)
    {
#endif /* !AYWITHAQUA */
      npatch->no = gluNewNurbsRenderer();
      if(npatch->no == NULL)
	{
	  return AY_EOMEM;
	}
#ifndef AYWITHAQUA
    } /* if */
#endif /* !AYWITHAQUA */

  gluNurbsCallback(npatch->no, GLU_ERROR, AYGLUCBTYPE ay_error_glucb);

  gluBeginSurface(npatch->no);

  gluNurbsProperty(npatch->no, GLU_SAMPLING_TOLERANCE,
		   (GLfloat)sampling_tolerance);

  gluNurbsProperty(npatch->no, GLU_DISPLAY_MODE, GLU_FILL);

  /*gluNurbsProperty(npatch->no, GLU_AUTO_LOAD_MATRIX, GL_FALSE);*/
  gluNurbsProperty(npatch->no, GLU_CULLING, GL_TRUE);

  gluNurbsSurface(npatch->no, (GLint)uknot_count, (GLfloat*)npatch->fltcv,
		  (GLint)vknot_count, (GLfloat*)&(npatch->fltcv[uknot_count]),
		  (GLint)height*(npatch->is_rat?4:3),
		  (GLint)(npatch->is_rat?4:3),
		  (GLfloat*)&(npatch->fltcv[uknot_count+vknot_count]),
		  (GLint)npatch->uorder, (GLint)npatch->vorder,
		  (npatch->is_rat?GL_MAP2_VERTEX_4:GL_MAP2_VERTEX_3));

  /* draw trimcurves */
  if(o->down && o->down->next)
    {
      ay_status = ay_npt_drawtrimcurves(o, 0);
    } /* if */

  gluEndSurface(npatch->no);

#ifdef AYWITHAQUA
  gluDeleteNurbsRenderer(npatch->no);
  npatch->no = NULL;
#endif /* AYWITHAQUA */

 return AY_OK;
} /* ay_npatch_shadeglu */


/* ay_npatch_shadecb:
 *  shade (display in an Ayam view window) callback function of npatch object
 */
int
ay_npatch_shadecb(struct Togl *togl, ay_object *o)
{
 int display_mode = ay_prefs.np_display_mode;
 ay_nurbpatch_object *npatch;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 ay_object *b;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_ENULL;

  if((npatch->display_mode != 0) && !view->action_state)
    {
      display_mode = npatch->display_mode-1;
    }

  switch(display_mode)
    {
    case 0: /* ControlHull */
      ay_npatch_shadech(npatch);
      break;
    case 1: /* OutlinePolygon (GLU) */
      ay_npatch_shadeglu(view, o);
      break;
    case 2: /* OutlinePatch (GLU) */
      ay_npatch_shadeglu(view, o);
      break;
    case 3: /* OutlinePatch (STESS) */
      ay_npatch_shadestess(view, o);
      break;
    default:
      break;
    } /* switch */

  b = npatch->caps_and_bevels;
  while(b)
    {
      ay_shade_object(togl, b, AY_FALSE);
      b = b->next;
    }

 return AY_OK;
} /* ay_npatch_shadecb */


/* ay_npatch_drawacb:
 *  draw annotations (in an Ayam view window) callback function of npatch object
 */
int
ay_npatch_drawacb(struct Togl *togl, ay_object *o)
{
 ay_nurbpatch_object *npatch = NULL;
 int width = 0, height = 0;
 double *cv = NULL;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_ENULL;

  width = npatch->width;
  height = npatch->height;

  cv = npatch->controlv;

  /* draw arrow */
  ay_draw_arrow(togl, &(cv[width*height*4-8]), &(cv[width*height*4-4]));

 return AY_OK;
} /* ay_npatch_drawacb */


/* ay_npatch_drawhcb:
 *  draw handles (in an Ayam view window) callback function of npatch object
 */
int
ay_npatch_drawhcb(struct Togl *togl, ay_object *o)
{
 int i;
 double *pnts, w;
 double point_size = ay_prefs.handle_size;
 ay_mpoint *mp;
 ay_nurbpatch_object *npatch;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_ENULL;

  pnts = npatch->controlv;

  /* draw normal points */
  glBegin(GL_POINTS);
   if(npatch->is_rat && ay_prefs.rationalpoints)
     {
       for(i = 0; i < (npatch->width * npatch->height); i++)
	 {
	   glVertex3d((GLdouble)pnts[0]*pnts[3],
		      (GLdouble)pnts[1]*pnts[3],
		      (GLdouble)pnts[2]*pnts[3]);
	   pnts += 4;
	 }
     }
   else
     {
       for(i = 0; i < (npatch->width * npatch->height); i++)
	 {
	   glVertex3dv((GLdouble *)pnts);
	   pnts += 4;
	 }
     }
  glEnd();

  /* draw multiple points */
  if(npatch->mpoints)
    {
      glPointSize((GLfloat)(point_size*1.25));
      glBegin(GL_POINTS);
       if(npatch->is_rat && ay_prefs.rationalpoints)
	 {
	   mp = npatch->mpoints;
	   while(mp)
	     {
	       pnts = mp->points[0];
	       w = pnts[3];
	       glVertex3d((GLdouble)pnts[0]*w,
			  (GLdouble)pnts[1]*w,
			  (GLdouble)pnts[2]*w);
	       mp = mp->next;
	     }
	 }
       else
	 {
	   mp = npatch->mpoints;
	   while(mp)
	     {
	       glVertex3dv((GLdouble *)(mp->points[0]));
	       mp = mp->next;
	     }
	 }
      glEnd();
      glPointSize((GLfloat)point_size);
    }

 return AY_OK;
} /* ay_npatch_drawhcb */


/* ay_npatch_getpntcb:
 *  get point (editing and selection) callback function of npatch object
 */
int
ay_npatch_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbpatch_object *npatch = NULL;
 ay_point *pnt = NULL, **lastpnt = NULL;
 ay_mpoint *mp = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **ctmp;
 double *control = NULL, *c, h[3];
 int i = 0, j = 0, a = 0, found = AY_FALSE;
 unsigned int *itmp, peindex = 0;

  if(!o || ((mode != 3) && (!p || !pe)))
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)(o->refine);

  if(!npatch)
    return AY_ENULL;

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(pe)
    pe->rational = AY_TRUE;

  switch(mode)
    {
    case 0:
      /* select all points */
      if(!(pe->coords = malloc(npatch->width * npatch->height *
					 sizeof(double*))))
	return AY_EOMEM;
      if(!(pe->indices = malloc(npatch->width * npatch->height *
					 sizeof(unsigned int))))
	return AY_EOMEM;

      for(i = 0; i < (npatch->width*npatch->height); i++)
	{
	  pe->coords[i] = &(npatch->controlv[a]);
	  pe->indices[i] = i;
	  a += 4;
	}

      pe->num = npatch->width * npatch->height;
      break;
    case 1:
      /* selection based on a single point */
      control = npatch->controlv;
      for(i = 0; i < (npatch->width * npatch->height); i++)
	{
	  if(npatch->is_rat && ay_prefs.rationalpoints)
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
	}

      if(!pecoord)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      if(npatch->mpoints)
	{
	  mp = npatch->mpoints;
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
	  if(!(pe->coords = calloc(1, sizeof(double*))))
	    return AY_EOMEM;

	  if(!(pe->indices = calloc(1, sizeof(unsigned int))))
	    return AY_EOMEM;

	  pe->coords[0] = pecoord;
	  pe->indices[0] = peindex;
	  pe->num = 1;
	}
      break;
    case 2:
      /* selection based on planes */
      control = npatch->controlv;
      j = 0;
      a = 0;
      if(npatch->is_rat && ay_prefs.rationalpoints)
	{
	  c = h;
	}
      for(i = 0; i < npatch->width * npatch->height; i++)
	{
	  if(npatch->is_rat && ay_prefs.rationalpoints)
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

	      if(!(itmp = realloc(pe->indices,
				  (a+1)*sizeof(unsigned int))))
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
	  if(pnt->index < (unsigned int)(npatch->width*npatch->height))
	    {
	      pnt->point = &(npatch->controlv[pnt->index*4]);
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
    } /* if */

 return AY_OK;
} /* ay_npatch_getpntcb */


/* ay_npatch_setpropcb:
 *  set property (from Tcl to C context) callback function of npatch object
 */
int
ay_npatch_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK, tcl_status = TCL_OK;
 char *n1 = "NPatchAttrData";
 char fname[] = "npatch_setpropcb";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_object *b;
 ay_nurbpatch_object *npatch = NULL;
 int new_uorder, new_width, new_uknot_type, uknots_modified = 0;
 int new_vorder, new_height, new_vknot_type, vknots_modified = 0;
 double *nknotv = NULL, *olduknotv = NULL, *oldvknotv = NULL;
 int update = AY_FALSE, updateKnots = AY_FALSE, updateMPs = AY_TRUE;
 int knotc, i;
 char **knotv;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  /* get new values from Tcl */
  Tcl_SetStringObj(ton,"Width",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_width);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_height);

  Tcl_SetStringObj(ton,"Order_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_uorder);

  Tcl_SetStringObj(ton,"Order_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_vorder);

  Tcl_SetStringObj(ton,"Knot-Type_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_uknot_type);

  Tcl_SetStringObj(ton,"Knot-Type_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_vknot_type);

  Tcl_SetStringObj(ton,"CreateMP",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(npatch->createmp));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(npatch->glu_sampling_tolerance));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(npatch->display_mode));

  Tcl_SetStringObj(ton, "Knots_U-Modified", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &uknots_modified);

  Tcl_SetStringObj(ton, "Knots_V-Modified", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &vknots_modified);

  Tcl_SetStringObj(ton,"BevelsChanged",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &i);
  if(i)
    {
      update = AY_TRUE;

      to = Tcl_NewIntObj(0);
      Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    }

  Tcl_SetStringObj(ton,"CapsChanged",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &i);
  if(i)
    {
      update = AY_TRUE;

      to = Tcl_NewIntObj(0);
      Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    }

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  /* apply changed values to patch */

  /* resize patch */
  if(new_width != npatch->width && (new_width > 1))
    {
      if(o->selp)
	{
	  ay_selp_clear(o);
	}

      ay_status = ay_npt_resizew(npatch, new_width);

      if(ay_status)
       ay_error(AY_ERROR,fname,"Could not resize patch!");

      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    } /* if */

  if(new_height != npatch->height && (new_height > 1))
    {
      if(o->selp)
	{
	  ay_selp_clear(o);
	}

      ay_status = ay_npt_resizeh(npatch, new_height);

      if(ay_status)
       ay_error(AY_ERROR,fname,"Could not resize patch!");

      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    } /* if */

  /* apply new order */
  if((npatch->uorder != new_uorder) && (new_uorder > 1))
    {
      npatch->uorder = new_uorder;
      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    }

  if((npatch->vorder != new_vorder) && (new_vorder > 1))
    {
      npatch->vorder = new_vorder;
      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    }

  /* change knot type */
  if((new_uknot_type != npatch->uknot_type) || (updateKnots))
    {
      npatch->uknot_type = new_uknot_type;
      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    }

  if((new_vknot_type != npatch->vknot_type) || (updateKnots))
    {
      npatch->vknot_type = new_vknot_type;
      updateKnots = AY_TRUE;
      o->modified = AY_TRUE;
    }

  /* plausibility checks */
  if(npatch->uknot_type == AY_KTBEZIER)
    {
      if(npatch->uorder != npatch->width)
	{
	  ay_error(AY_EWARN,fname, "Changing uorder to match width!");
	  npatch->uorder = npatch->width;
	}
    }

  if(npatch->vknot_type == AY_KTBEZIER)
    {
      if(npatch->vorder != npatch->height)
	{
	  ay_error(AY_EWARN, fname, "Changing vorder to match height!");
	  npatch->vorder = npatch->height;
	}
    }

  if(npatch->width < npatch->uorder)
    {
      ay_error(AY_EWARN, fname, "Lowering uorder to match width!");
      npatch->uorder = npatch->width;
    }

  if(npatch->height < npatch->vorder)
    {
      ay_error(AY_EWARN, fname, "Lowering vorder to match height!");
      npatch->vorder = npatch->height;
    }

  if(updateKnots)
    {
      if(npatch->uknot_type == AY_KTCUSTOM)
	{
	  olduknotv = npatch->uknotv;
	  npatch->uknotv = NULL;
	}
      if(npatch->vknot_type == AY_KTCUSTOM)
	{
	  oldvknotv = npatch->vknotv;
	  npatch->vknotv = NULL;
	}

      ay_status = ay_knots_createnp(npatch);

      if(olduknotv)
	{
	  if(npatch->uknotv)
	    free(npatch->uknotv);
	  npatch->uknotv = olduknotv;
	}
      if(oldvknotv)
	{
	  if(npatch->vknotv)
	    free(npatch->vknotv);
	  npatch->vknotv = oldvknotv;
	}
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Error creating new knots!");
	  return AY_ERROR;
	}
    } /* if */

  /* decompose uknot-list (create custom knot sequence) */
  if((npatch->uknot_type == AY_KTCUSTOM) && uknots_modified)
    {
      Tcl_SplitList(interp, Tcl_GetVar2(interp, n1, "Knots_U",
					TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY),
		    &knotc, &knotv);

      if(!(nknotv = malloc(knotc * sizeof(double))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  Tcl_Free((char *) knotv);
	  return AY_ERROR;
	}
      tcl_status = TCL_OK;
      for(i = 0; i < knotc; i++)
	{
	  tcl_status = Tcl_GetDouble(interp, knotv[i], &nknotv[i]);
	  if(tcl_status != TCL_OK)
	    break;
	} /* for */

      if((tcl_status == TCL_OK) &&
	 !(ay_status = ay_knots_check(new_width, new_uorder, knotc, nknotv)))
	{
	  /* the knots are ok */
	  free(npatch->uknotv);
	  npatch->uknotv = nknotv;
	}
      else
	{
	  /* the knots are wrong */
	  /* tell the user what went wrong */
	  ay_error(AY_EOUTPUT, fname, "Checking new knots for U...");
	  ay_knots_printerr(fname, ay_status);

	  /* get rid of user supplied knots */
	  free(nknotv);

	  /* create new knots */
	  ay_error(AY_EWARN,fname, "Falling back to knot type NURB!");
	  npatch->uknot_type = AY_KTNURB;

	  ay_status = ay_knots_createnp(npatch);

	  if(ay_status)
	    ay_error(AY_ERROR, fname, "Error creating new knots!");
	} /* if */

      /* XXXX compare old and new knots before setting this flag */
      o->modified = AY_TRUE;

      Tcl_Free((char *) knotv);
    } /* if */

  /* decompose vknot-list (create custom knot sequence) */
  if((npatch->vknot_type == AY_KTCUSTOM) && vknots_modified)
    {
      Tcl_SplitList(interp,Tcl_GetVar2(interp, n1, "Knots_V",
				       TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY),
		    &knotc, &knotv);

      if(!(nknotv = malloc(knotc * sizeof(double))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  Tcl_Free((char *) knotv);
	  return AY_ERROR;
	}

      tcl_status = TCL_OK;
      for(i = 0; i < knotc; i++)
	{
	  tcl_status = Tcl_GetDouble(interp, knotv[i], &nknotv[i]);
	  if(tcl_status != TCL_OK)
	    break;
	} /* for */

      if((tcl_status == TCL_OK) &&
	 !(ay_status = ay_knots_check(new_height, new_vorder, knotc, nknotv)))
	{
	  /* the knots are ok */
	  free(npatch->vknotv);
	  npatch->vknotv = nknotv;
	}
      else
	{
	  /* the knots are wrong */
	  /* tell the user what went wrong */
	  ay_error(AY_EOUTPUT, fname, "Checking new knots for V...");
	  ay_knots_printerr(fname, ay_status);

	  /* get rid of user supplied knots */
	  free(nknotv);

	  /* create new knots */
	  ay_error(AY_EWARN,fname, "Falling back to knot type NURB!");
	  npatch->vknot_type = AY_KTNURB;

	  ay_status = ay_knots_createnp(npatch);

	  if(ay_status)
	    ay_error(AY_ERROR, fname, "Error creating new knots!");
	} /* if */

      /* XXXX compare old and new knots before setting this flag */
      o->modified = AY_TRUE;

      Tcl_Free((char *) knotv);
    } /* if */

  if(updateMPs)
    {
      ay_npt_recreatemp(npatch);
    }

  if(update || o->modified)
    {
      (void)ay_notify_object(o);

      o->modified = AY_TRUE;
    }
  else
    {
      b = npatch->caps_and_bevels;
      while(b)
	{
	  ((ay_nurbpatch_object *) (b->refine))->glu_sampling_tolerance =
	    npatch->glu_sampling_tolerance;
	  ((ay_nurbpatch_object *) (b->refine))->display_mode =
	    npatch->display_mode;
	  b = b->next;
	}
    }

  (void)ay_notify_parent();

 return AY_OK;
} /* ay_npatch_setpropcb */


/* ay_npatch_getpropcb:
 *  get property (from C to Tcl context) callback function of npatch object
 */
int
ay_npatch_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="NPatchAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_nurbpatch_object *npatch = NULL;
 int i;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)(o->refine);

  if(!npatch)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Width",-1);
  to = Tcl_NewIntObj(npatch->width);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_NewIntObj(npatch->height);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order_U",-1);
  to = Tcl_NewIntObj(npatch->uorder);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order_V",-1);
  to = Tcl_NewIntObj(npatch->vorder);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Knot-Type_U",-1);
  to = Tcl_NewIntObj(npatch->uknot_type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Knot-Type_V",-1);
  to = Tcl_NewIntObj(npatch->vknot_type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetVar2(interp,n1,"Knots_U","", TCL_LEAVE_ERR_MSG |
	      TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"Knots_U",-1);
  for(i=0; i<npatch->width+npatch->uorder; i++)
    {

      to = Tcl_NewDoubleObj((npatch->uknotv)[i]);

      Tcl_ObjSetVar2(interp,toa,ton,to,TCL_APPEND_VALUE |
		     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG |
		     TCL_GLOBAL_ONLY);
    }

  Tcl_SetVar2(interp,n1,"Knots_V","", TCL_LEAVE_ERR_MSG |
	      TCL_GLOBAL_ONLY);
  Tcl_SetStringObj(ton,"Knots_V",-1);
  for(i=0; i<npatch->height+npatch->vorder; i++)
    {

      to = Tcl_NewDoubleObj((npatch->vknotv)[i]);

      Tcl_ObjSetVar2(interp,toa,ton,to,TCL_APPEND_VALUE |
		     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG |
		     TCL_GLOBAL_ONLY);
    }

  Tcl_SetStringObj(ton,"CreateMP",-1);
  to = Tcl_NewIntObj(npatch->createmp);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(npatch->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(npatch->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"IsRat", -1);
  if(npatch->is_rat)
    to = Tcl_NewStringObj("yes", -1);
  else
    to = Tcl_NewStringObj("no", -1);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Knots_U-Modified",-1);
  to = Tcl_NewIntObj(0);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Knots_V-Modified",-1);
  to = Tcl_NewIntObj(0);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_npatch_getpropcb */


/* ay_npatch_readcb:
 *  read (from scene file) callback function of npatch object
 */
int
ay_npatch_readcb(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *npatch = NULL;
 int i, a;

 if(!fileptr || !o)
   return AY_ENULL;

  if(!(npatch = calloc(1, sizeof(ay_nurbpatch_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&npatch->width);
  fscanf(fileptr,"%d\n",&npatch->height);
  fscanf(fileptr,"%d\n",&npatch->uorder);
  fscanf(fileptr,"%d\n",&npatch->vorder);
  fscanf(fileptr,"%d\n",&npatch->uknot_type);
  fscanf(fileptr,"%d\n",&npatch->vknot_type);

  ay_status = ay_knots_createnp(npatch);
  if(ay_status)
    { free(npatch); return ay_status; }

  if(npatch->uknot_type == AY_KTCUSTOM)
    {
      if(npatch->uknotv)
	free(npatch->uknotv);
      npatch->uknotv = NULL;
      if(!(npatch->uknotv =
	   calloc((npatch->width + npatch->uorder), sizeof(double))))
	{ ay_npt_destroy(npatch); return AY_EOMEM; }

      for(i = 0; i < (npatch->width + npatch->uorder); i++)
	fscanf(fileptr,"%lg\n",&(npatch->uknotv[i]));
    }

  if(npatch->vknot_type == AY_KTCUSTOM)
    {
      if(npatch->vknotv)
	free(npatch->vknotv);
      npatch->vknotv = NULL;

      if(!(npatch->vknotv =
	   calloc((npatch->height + npatch->vorder), sizeof(double))))
	{ ay_npt_destroy(npatch); return AY_EOMEM; }

      for(i = 0; i < (npatch->height + npatch->vorder); i++)
	fscanf(fileptr,"%lg\n",&(npatch->vknotv[i]));
    }

  if(!(npatch->controlv = calloc(npatch->width*npatch->height*4,
				 sizeof(double))))
    { ay_npt_destroy(npatch); return AY_EOMEM; }

  a = 0;
  for(i = 0; i < (npatch->width*npatch->height); i++)
    {
      fscanf(fileptr,"%lg %lg %lg %lg\n", &(npatch->controlv[a]),
	     &(npatch->controlv[a+1]),
	     &(npatch->controlv[a+2]),
	     &(npatch->controlv[a+3]));
      a += 4;
    }

  fscanf(fileptr,"%lg\n",&(npatch->glu_sampling_tolerance));
  fscanf(fileptr,"%d\n",&(npatch->display_mode));

  if(ay_read_version >= 9)
    {
      fscanf(fileptr,"%d\n",&(npatch->createmp));
    }
  else
    {
      npatch->createmp = AY_TRUE;
    }

  ay_npt_recreatemp(npatch);

  npatch->is_rat = ay_npt_israt(npatch);

  /* Prior to 1.19 Ayam used pre-multiplied rational coordinates... */
  if(npatch->is_rat && (ay_read_version < 14))
    {
      a = 0;
      for(i = 0; i < npatch->width*npatch->height; i++)
	{
	  npatch->controlv[a+1] /= npatch->controlv[a+3];
	  npatch->controlv[a+2] /= npatch->controlv[a+3];
	  npatch->controlv[a+3] /= npatch->controlv[a+3];
	  a += 4;
	}
    }

  o->refine = npatch;

 return AY_OK;
} /* ay_npatch_readcb */


/* ay_npatch_writecb:
 *  write (to scene file) callback function of npatch object
 */
int
ay_npatch_writecb(FILE *fileptr, ay_object *o)
{
 ay_nurbpatch_object *npatch = NULL;
 int i, a;

  if(!fileptr || !o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)(o->refine);

  if(!npatch)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", npatch->width);
  fprintf(fileptr, "%d\n", npatch->height);
  fprintf(fileptr, "%d\n", npatch->uorder);
  fprintf(fileptr, "%d\n", npatch->vorder);
  fprintf(fileptr, "%d\n", npatch->uknot_type);
  fprintf(fileptr, "%d\n", npatch->vknot_type);

  if(npatch->uknot_type == AY_KTCUSTOM)
    {
      for(i=0;i<(npatch->width+npatch->uorder);i++)
	fprintf(fileptr,"%g\n",npatch->uknotv[i]);
    }

  if(npatch->vknot_type == AY_KTCUSTOM)
    {
      for(i=0;i<(npatch->height+npatch->vorder);i++)
	fprintf(fileptr,"%g\n",npatch->vknotv[i]);
    }

  a = 0;
  for(i=0;i<npatch->width*npatch->height;i++)
    {
      fprintf(fileptr,"%g %g %g %g\n", npatch->controlv[a],
	      npatch->controlv[a+1],
	      npatch->controlv[a+2],
	      npatch->controlv[a+3]);
      a+=4;
    }

  fprintf(fileptr, "%g\n", npatch->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", npatch->display_mode);

  fprintf(fileptr, "%d\n", npatch->createmp);

 return AY_OK;
} /* ay_npatch_writecb */


/* ay_npatch_wribtrimcurves
 *  internal helper function
 *  for ay_npatch_wribtrimcurves() below
 */
void
ay_npatch_wribtrimcurve(ay_object *o,
			RtFloat *min, RtFloat *max, RtFloat *knot,
			RtFloat *u, RtFloat *v, RtFloat *w,
			RtInt *n, RtInt *order,
			int *a, int *b, int *c)
{
 ay_nurbcurve_object *curve = NULL;
 RtFloat x, y, z, w2;
 double m[16];
 int k;

  curve = (ay_nurbcurve_object*)o->refine;

  /* fill order[], n[], min[], max[] */
  order[*a] = (RtInt)curve->order;
  n[*a] = (RtInt)curve->length;
  min[*a] = (RtFloat)((curve->knotv)[curve->order - 1]);
  max[*a] = (RtFloat)((curve->knotv)[curve->length]);
  *a = *a+1;

  /* get curves transformation-matrix */
  ay_trafo_creatematrix(o, m);

  /* copy & revert control (fill u[] v[] w[]) */
  for(k = 0; k < curve->length ; k++)
    {
      x = (RtFloat)((curve->controlv)[k*4]);
      y = (RtFloat)((curve->controlv)[(k*4)+1]);
      z = (RtFloat)((curve->controlv)[(k*4)+2]);
      w2 = (RtFloat)((curve->controlv)[(k*4)+3]);

      /* apply transformation & multiply in the weight */
      u[*b] = (RtFloat)((m[0]*x + m[4]*y + m[8]*z + m[12])*w2);
      v[*b] = (RtFloat)((m[1]*x + m[5]*y + m[9]*z + m[13])*w2);
      w[*b] = (RtFloat)w2;

      *b = *b+1;
    } /* for */

  /* copy knots (fill knot[]) */
  for(k = 0; k < curve->length + curve->order; k++)
    {
      knot[*c] = (RtFloat)curve->knotv[k];
      *c = *c+1;
    }

 return;
} /* ay_npatch_wribtrimcurve */


/* ay_npatch_wribtrimcurves
 *  internal helper function
 *  export trim curves to RIB
 */
int
ay_npatch_wribtrimcurves(ay_object *o)
{
 int ay_status = AY_OK;
 int a, b, c, totalcurves, totalcontrol, totalknots;
 RtInt nloops = 0, *ncurves = NULL, *order = NULL, *n = NULL;
 RtFloat *knot = NULL, *min = NULL, *max = NULL;
 RtFloat *u = NULL, *v = NULL, *w = NULL;
 ay_object *trim = NULL, *loop = NULL, *p = NULL, *nc = NULL;
 ay_nurbcurve_object *curve = NULL;
 int valid_loop;

  if(!o)
    return AY_OK;

  if(o->type != AY_IDNPATCH)
    return AY_ERROR;

  /* parse trimcurves */
  /* count loops */
  trim = o->down;

  while(trim->next)
    {
      switch(trim->type)
	{
	case AY_IDNCURVE:
	  nloops++;
	  break;
	case AY_IDLEVEL:
	  if(trim->down && trim->down->next)
	    {
	      valid_loop = AY_FALSE;
	      loop = trim->down;
	      while(loop)
		{
		  if(loop->type == AY_IDNCURVE)
		    {
		      valid_loop = AY_TRUE;
		      break;
		    }
		  else
		    {
		      nc = NULL;
		      ay_status = ay_provide_object(loop, AY_IDNCURVE, &nc);
		      if(nc)
			{
			  valid_loop = AY_TRUE;
			  (void)ay_object_deletemulti(nc, AY_FALSE);
			  break;
			}
		    }
		  loop = loop->next;
		} /* while */
	      if(valid_loop)
		nloops++;
	    } /* if */
	  break;
	default:
	  p = NULL;
	  ay_status = ay_provide_object(trim, AY_IDNCURVE, &p);
	  nc = p;
	  while(nc)
	    {
	      nloops++;
	      nc = nc->next;
	    }
	  (void)ay_object_deletemulti(p, AY_FALSE);
	  break;
	} /* switch */
      trim = trim->next;
    } /* while */

  if(nloops == 0)
    return AY_OK;

  /* count curves per loop */
  if(!(ncurves = malloc(nloops * sizeof(RtInt))))
    return AY_EOMEM;

  trim = o->down;

  totalcurves = 0;
  totalcontrol = 0;
  totalknots = 0;
  nloops = 0;
  while(trim->next)
    {
      switch(trim->type)
	{
	case AY_IDNCURVE:
	  valid_loop = AY_TRUE;
	  totalcurves++;
	  curve = (ay_nurbcurve_object *)(trim->refine);
	  totalcontrol += curve->length;
	  totalknots += curve->length;
	  totalknots += curve->order;
	  ncurves[nloops] = 1;
	  nloops++;
	  break;
	case AY_IDLEVEL:
	  valid_loop = AY_FALSE;
	  ncurves[nloops] = 0;
	  loop = trim->down;
	  while(loop && loop->next)
	    {
	      nc = NULL;
	      if(loop->type == AY_IDNCURVE)
		{
		  valid_loop = AY_TRUE;
		  curve = (ay_nurbcurve_object *)(loop->refine);
		  totalcurves++;
		  totalcontrol += curve->length;
		  totalknots += curve->length;
		  totalknots += curve->order;
		  ncurves[nloops] += 1;
		}
	      else
		{
		  p = NULL;
		  ay_status = ay_provide_object(loop, AY_IDNCURVE, &p);
		  if(p)
		    valid_loop = AY_TRUE;
		  nc = p;
		  while(nc)
		    {
		      curve = (ay_nurbcurve_object *)(nc->refine);
		      totalcurves++;
		      totalcontrol += curve->length;
		      totalknots += curve->length;
		      totalknots += curve->order;
		      ncurves[nloops] += 1;
		      nc = nc->next;
		    }
		  (void)ay_object_deletemulti(p, AY_FALSE);
		} /* if */

	      loop = loop->next;
	    } /* while */
	  if(valid_loop)
	    nloops++;
	  break;
	default:
	  p = NULL;
	  ay_status = ay_provide_object(trim, AY_IDNCURVE, &p);
	  nc = p;
	  while(nc)
	    {
	      totalcurves++;
	      curve = (ay_nurbcurve_object *)(nc->refine);
	      totalcontrol += curve->length;
	      totalknots += curve->length;
	      totalknots += curve->order;
	      ncurves[nloops] = 1;
	      nloops++;
	      nc = nc->next;
	    }
	  (void)ay_object_deletemulti(p, AY_FALSE);
	  break;
	} /* switch */
      trim = trim->next;
    } /* while */

  if(!(order = malloc(totalcurves*sizeof(RtInt))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(n = malloc(totalcurves*sizeof(RtInt))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(knot = malloc(totalknots*sizeof(RtFloat))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(min = malloc(totalcurves*sizeof(RtFloat))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(max = malloc(totalcurves*sizeof(RtFloat))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(u = malloc(totalcontrol*sizeof(RtFloat))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(v = malloc(totalcontrol*sizeof(RtFloat))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(w = malloc(totalcontrol*sizeof(RtFloat))))
    { ay_status = AY_EOMEM; goto cleanup; }

  trim = o->down;
  a = 0;
  b = 0;
  c = 0;
  /* compile arguments */
  while(trim->next)
    {
      switch(trim->type)
	{
	case AY_IDNCURVE:
	  ay_npatch_wribtrimcurve(trim, min, max, knot,	u, v, w, n, order,
				  &a, &b, &c);
	  break;
	case AY_IDLEVEL:
	  if(trim->down && trim->down->next)
	    {
	      loop = trim->down;

	      while(loop && loop->next)
		{
		  nc = NULL;

		  if(loop->type == AY_IDNCURVE)
		    {
		      ay_npatch_wribtrimcurve(loop, min, max, knot,
					      u, v, w, n, order,
					      &a, &b, &c);
		    }
		  else
		    {
		      p = NULL;
		      ay_status = ay_provide_object(loop, AY_IDNCURVE, &p);
		      nc = p;
		      while(nc)
			{
			  ay_npatch_wribtrimcurve(nc, min, max, knot,
					      u, v, w, n, order,
					      &a, &b, &c);
			  nc = nc->next;
			}
		      (void)ay_object_deletemulti(p, AY_FALSE);
		    } /* if */

		  loop = loop->next;
		} /* while */
	    } /* if */
	  break;
	default:
	  p = NULL;
	  ay_status = ay_provide_object(trim, AY_IDNCURVE, &p);
	  nc = p;
	  while(nc)
	    {
	      ay_npatch_wribtrimcurve(nc, min, max, knot,
				      u, v, w, n, order,
				      &a, &b, &c);
	      nc = nc->next;
	    }
	  (void)ay_object_deletemulti(p, AY_FALSE);
	  break;
	} /* switch */

      trim = trim->next;
    } /* while */

  /* Finally, write the TrimCurves */
  RiTrimCurve(nloops, ncurves, order, knot, min, max, n, u, v, w);

  /* clean up the mess */
cleanup:

  if(ncurves)
    free(ncurves);
  if(order)
    free(order);
  if(knot)
    free(knot);
  if(min)
    free(min);
  if(max)
    free(max);
  if(n)
    free(n);
  if(u)
    free(u);
  if(v)
    free(v);
  if(w)
    free(w);

 return ay_status;
} /* ay_npatch_wribtrimcurves */


/* ay_npatch_wribcb:
 *  RIB export callback function of npatch object
 */
int
ay_npatch_wribcb(char *file, ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *patch = NULL;
 ay_object *cb;
 double umind, umaxd, vmind, vmaxd, w;
 RtInt nu, nv, uorder, vorder;
 RtFloat *uknots = NULL, *vknots = NULL, umin, umax, vmin, vmax;
 RtFloat *controls = NULL;
 RtToken *tokens = NULL;
 RtPointer *parms = NULL;
 int i = 0, j = 0, a = 0, b = 0, n = 0, pvc = 0;

  if(!o)
    return AY_OK;

  if(o->down && o->down->next)
    {
      ay_status = ay_npatch_wribtrimcurves(o);
    }

  patch = (ay_nurbpatch_object*)(o->refine);

  if(!patch)
    return AY_ENULL;

  nu = (RtInt)patch->width;
  uorder = (RtInt)patch->uorder;
  nv = (RtInt)patch->height;
  vorder = (RtInt)patch->vorder;

  if((uknots = calloc(nu+uorder, sizeof(RtFloat))) == NULL)
    {ay_status = AY_EOMEM; goto cleanup;}
  if((vknots = calloc(nv+vorder, sizeof(RtFloat))) == NULL)
    {ay_status = AY_EOMEM; goto cleanup;}
  if((controls = calloc(nu*nv*(patch->is_rat?4:3), sizeof(RtFloat))) == NULL)
    {ay_status = AY_EOMEM; goto cleanup;}

  a = 0;
  for(i = 0; i < nu+uorder; i++)
    {
      uknots[a] = (RtFloat)patch->uknotv[a];
      a++;
    }
  a = 0;
  for(i = 0; i < nv+vorder; i++)
    {
      vknots[a] = (RtFloat)patch->vknotv[a];
      a++;
    }
  a = 0;
  /* RenderMan expects u-major order! */
  if(patch->is_rat)
    {
      for(i = 0; i < nv; i++)
	{
	  b = i*4;
	  for(j = 0; j < nu; j++)
	    {
	      w = patch->controlv[b+3];
	      controls[a] = (RtFloat)(patch->controlv[b]*w);
	      a++;
	      controls[a] = (RtFloat)(patch->controlv[b+1]*w);
	      a++;
	      controls[a] = (RtFloat)(patch->controlv[b+2]*w);
	      a++;
	      controls[a] = (RtFloat)patch->controlv[b+3];
	      a++;

	      b += (nv*4);
	    } /* for */
	} /* for */
    }
  else
    {
      for(i = 0; i < nv; i++)
	{
	  b = i*4;
	  for(j = 0; j < nu; j++)
	    {
	      controls[a] = (RtFloat)patch->controlv[b];
	      a++;
	      controls[a] = (RtFloat)patch->controlv[b+1];
	      a++;
	      controls[a] = (RtFloat)patch->controlv[b+2];
	      a++;

	      b += (nv*4);
	    } /* for */
	} /* for */
    } /* if */

  ay_knots_getuminmax(o, patch->uorder, patch->uorder + patch->width,
		      patch->uknotv, &umind, &umaxd);
  umin = (RtFloat)umind;
  umax = (RtFloat)umaxd;
  ay_knots_getvminmax(o, patch->vorder, patch->vorder + patch->height,
		      patch->vknotv, &vmind, &vmaxd);
  vmin = (RtFloat)vmind;
  vmax = (RtFloat)vmaxd;

  /* Do we have any primitive variables? */
  if(!(pvc = ay_pv_count(o)))
    {
      /* No */
      RiNuPatch(nu, uorder, uknots,
		/*(RtFloat)uknots[uorder-1], (RtFloat)uknots[nu],*/
		umin, umax,
		nv, vorder, vknots,
		/*(RtFloat)vknots[vorder-1], (RtFloat)vknots[nv],*/
		vmin, vmax,
		patch->is_rat?"Pw":"P", controls, NULL);
    }
  else
    {
      /* Yes, we have primitive variables. */
      if(!(tokens = calloc(pvc+1, sizeof(RtToken))))
	{ay_status = AY_EOMEM; goto cleanup;}

      if(!(parms = calloc(pvc+1, sizeof(RtPointer))))
	{free(tokens); ay_status = AY_EOMEM; goto cleanup;}

      if(patch->is_rat)
	tokens[0] = "Pw";
      else
	tokens[0] = "P";
      parms[0] = (RtPointer)controls;

      n = 1;
      ay_status = ay_pv_filltokpar(o, AY_TRUE, 1, &n, tokens, parms);

      if(!ay_status)
	{
	  RiNuPatchV(nu, uorder, uknots,
		     /*(RtFloat)uknots[uorder-1], (RtFloat)uknots[nu],*/
		     umin, umax,
		     nv, vorder, vknots,
		     /*(RtFloat)vknots[vorder-1], (RtFloat)vknots[nv],*/
		     vmin, vmax,
		     (RtInt)n, tokens, parms);
	}

      for(i = 1; i < n; i++)
	{
	  if(tokens[i])
	    free(tokens[i]);
	  if(parms[i])
	    free(parms[i]);
	}

      free(tokens);
      free(parms);
    } /* if */

  cb = patch->caps_and_bevels;
  while(cb)
    {
      ay_wrib_object(file, cb);
      cb = cb->next;
    }

cleanup:

  if(uknots)
    free(uknots);
  if(vknots)
    free(vknots);
  if(controls)
    free(controls);

 return ay_status;
} /* ay_npatch_wribcb */


/* ay_npatch_bbccb:
 *  bounding box calculation callback function of npatch object
 */
int
ay_npatch_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_nurbpatch_object *npatch = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_ENULL;

  /* exclusive bounding box, discard children/trim curves bbox */
  *flags = 1;

 return ay_bbc_fromarr(npatch->controlv, npatch->width*npatch->height,
		       4, bbox);
} /* ay_npatch_bbccb */


/* ay_npatch_providecb:
 *  provide callback function of npatch object
 */
int
ay_npatch_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 /*char fname[] = "npatch_providecb";*/
 ay_object *cb = NULL, **next = NULL;
 ay_nurbpatch_object *npatch = NULL;
 ay_tag *tag = NULL;
 int use_tc = AY_FALSE, use_vc = AY_FALSE, use_vn = AY_FALSE;
 int smethod = ay_prefs.smethod;
 double sparamu = ay_prefs.sparamu, sparamv = ay_prefs.sparamv;
 int refine_trim = 0;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)(o->refine);

  if(!npatch)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDPOMESH)
	return AY_OK;
      else
	return AY_ERROR;
    }

  if(type == AY_IDPOMESH)
    {
      /* infer parameters from (eventually present) TP tag */
      tag = o->tags;
      while(tag)
	{
	  if(tag->type == ay_tp_tagtype)
	    {
	      if(tag->val)
		sscanf(tag->val,"%d,%lg,%lg,%d", &smethod,
		       &sparamu, &sparamv, &refine_trim);
	    }
	  tag = tag->next;
	} /* while */

      /* tesselate */
      ay_status = ay_tess_npatch(o, smethod, sparamu, sparamv,
				 use_tc, NULL, use_vc, NULL, use_vn, NULL,
				 refine_trim,
				 result);

      if(*result)
	{
	  /* copy transformation attributes */
	  ay_trafo_copy(o, *result);

	  /* process caps and bevels */
	  next = &((*result)->next);
	  cb = npatch->caps_and_bevels;
	  while(cb)
	    {
	      ay_status = ay_npatch_providecb(cb, type, next);
	      if(ay_status)
		return ay_status;
	      if(!*next)
		return AY_ENULL;
	      next = &((*next)->next);
	      cb = cb->next;
	    } /* while */
	} /* if */
    } /* if */

 return ay_status;
} /* ay_npatch_providecb */


/* ay_npatch_convertcb:
 *  convert callback function of npatch object
 */
int
ay_npatch_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 /*char fname[] = "npatch_convertcb";*/
 ay_object *new = NULL, *c = NULL;
 ay_nurbpatch_object *npatch = NULL;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)(o->refine);

  if(!npatch)
    return AY_ENULL;

  if(npatch->caps_and_bevels)
    {
      if(!(new = calloc(1, sizeof(ay_object))))
	{ return AY_EOMEM; }

      ay_object_defaults(new);
      new->type = AY_IDLEVEL;
      new->parent = AY_TRUE;
      new->inherit_trafos = AY_TRUE;
      ay_trafo_copy(o, new);

      if(!(new->refine = calloc(1, sizeof(ay_level_object))))
	{ free(new); return AY_EOMEM; }

      ((ay_level_object *)(new->refine))->type = AY_LTLEVEL;

      ay_status = ay_npatch_providecb(o, AY_IDPOMESH, &(new->down));

      /* terminate the level */
      c = new->down;
      while(c->next)
	c = c->next;
      c->next = ay_endlevel;
    }
  else
    {
      ay_status = ay_npatch_providecb(o, AY_IDPOMESH, &new);
    }

  if(new)
    {
      if(!in_place)
	{
	  ay_trafo_copy(o, new);
	  ay_object_link(new);
	}
      else
	{
	  ay_status = ay_object_replace(new, o);
	}
    }

 return ay_status;
} /* ay_npatch_convertcb */


/* ay_npatch_setnttag:
 *
 */
void
ay_npatch_setnttag(ay_object *o, double *normal)
{
 ay_tag *tag;
 ay_btval *btval;
 double *nval;

  tag = o->tags;
  while(tag)
    {
      if(tag->type == ay_nt_tagtype)
	break;
      tag = tag->next;
    }

  if(!tag)
    {
      /* create and link new tag */
      if(!(tag = calloc(1, sizeof(ay_tag))))
	return;

      if(!(btval = calloc(1, sizeof(ay_btval))))
	{free(tag); return;}

      if(!(nval = calloc(3, sizeof(double))))
	{free(tag); free(btval); return;}

      memcpy(nval, normal, 3*sizeof(double));
      btval->payload = nval;
      btval->size = 3*sizeof(double);

      tag->type = ay_nt_tagtype;
      tag->is_intern = AY_TRUE;
      tag->is_binary = AY_TRUE;
      tag->val = btval;

      tag->next = o->tags;
      o->tags = tag;
    }
  else
    {
      /* just update */
      btval = (ay_btval*)tag->val;
      memcpy(btval->payload, normal, 3*sizeof(double));
    }

  return;
} /* ay_npatch_setnttag */


/* ay_npatch_notifycb:
 *  notification callback function of npatch object
 */
int
ay_npatch_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *npatch = NULL;
 ay_object *bevel = NULL, **nextcb;
 ay_bparam bparams = {0};
 ay_cparam cparams = {0};
 int display_mode = ay_prefs.np_display_mode, mode;
 int i, qf = ay_prefs.stess_qf;
 double normal[3], tolerance;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)(o->refine);

  if(!npatch)
    return AY_ENULL;

  if(npatch->caps_and_bevels)
    {
      (void)ay_object_deletemulti(npatch->caps_and_bevels, AY_FALSE);
      npatch->caps_and_bevels = NULL;
    }

  if((npatch->uknot_type > AY_KTCUSTOM) ||
     (npatch->vknot_type > AY_KTCUSTOM))
    {
      ay_status = ay_knots_createnp(npatch);
      if(ay_status)
	return ay_status;
    }

  if(o->modified)
    {
      ay_npt_setuvtypes(npatch, 0);
      npatch->is_planar = (char)ay_npt_isplanar(npatch, normal);
      if(npatch->is_planar)
	{
	  ay_npatch_setnttag(o, normal);
	}
    }

  /* get bevel and caps parameters */
  if(o->tags)
    {
      ay_bevelt_parsetags(o->tags, &bparams);
      ay_capt_parsetags(o->tags, &cparams);

      /* silently avoid bevel & cap integration... */
      for(i = 0; i < 4; i++)
	{
	  bparams.integrate[i] = AY_FALSE;
	  /* ...but still allow integration of a cap into a bevel surface */
	  if(!bparams.states[i])
	    {
	      cparams.integrate[i] = AY_FALSE;
	    }
	}
    }

  nextcb = &(npatch->caps_and_bevels);

  /* create/add caps */
  if(cparams.has_caps)
    {
      ay_status = ay_capt_addcaps(&cparams, &bparams, o, nextcb);
      if(ay_status)
	goto cleanup;

      while(*nextcb)
	nextcb = &((*nextcb)->next);
    }

  /* create/add bevels */
  if(bparams.has_bevels)
    {
      ay_status = ay_bevelt_addbevels(&bparams, &cparams, o, nextcb);
      if(ay_status)
	goto cleanup;
    }

  /* copy display modes over to the caps&bevels */
  if(npatch->caps_and_bevels)
    {
      mode = npatch->display_mode;
      tolerance = npatch->glu_sampling_tolerance;

      bevel = npatch->caps_and_bevels;
      while(bevel)
	{
	  ((ay_nurbpatch_object *)
	   (bevel->refine))->glu_sampling_tolerance = tolerance;
	  ((ay_nurbpatch_object *)
	   (bevel->refine))->display_mode = mode;
	  bevel = bevel->next;
	}
    }

  /* manage the GLU NURBS renderer */
  if(npatch->no)
    gluDeleteNurbsRenderer(npatch->no);
  npatch->no = NULL;

  /* manage cached float data */
  if(npatch->fltcv)
    {
      free(npatch->fltcv);
      npatch->fltcv = NULL;
    }

  /* manage the cached tesselation */
  if(npatch->stess)
    {
      ay_stess_destroy(npatch);
    }

  if(npatch->display_mode != 0)
    {
      display_mode = npatch->display_mode-1;
    }

  switch(display_mode)
    {
    case 0:
      break;
    case 1:
    case 2:
      npatch->no = gluNewNurbsRenderer();
      break;
    case 3:
    case 4:
      if(npatch->glu_sampling_tolerance != 0.0)
	{
	  qf = ay_stess_GetQF(npatch->glu_sampling_tolerance);
	}

      npatch->tessqf = qf;
      break;
    default:
      break;
    } /* switch */

cleanup:

 return ay_status;
} /* ay_npatch_notifycb */


/* ay_npatch_init:
 *  initialize the npatch object module
 */
int
ay_npatch_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_npatch_name,
				    ay_npatch_createcb,
				    ay_npatch_deletecb,
				    ay_npatch_copycb,
				    ay_npatch_drawcb,
				    ay_npatch_drawhcb,
				    ay_npatch_shadecb,
				    ay_npatch_setpropcb,
				    ay_npatch_getpropcb,
				    ay_npatch_getpntcb,
				    ay_npatch_readcb,
				    ay_npatch_writecb,
				    ay_npatch_wribcb,
				    ay_npatch_bbccb,
				    AY_IDNPATCH);

  ay_status += ay_draw_registerdacb(ay_npatch_drawacb, AY_IDNPATCH);

  ay_status += ay_provide_register(ay_npatch_providecb, AY_IDNPATCH);

  ay_status += ay_convert_register(ay_npatch_convertcb, AY_IDNPATCH);

  ay_status += ay_notify_register(ay_npatch_notifycb, AY_IDNPATCH);

 return ay_status;
} /* ay_npatch_init */

