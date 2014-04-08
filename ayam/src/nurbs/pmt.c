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

/** \file pmt.c \brief PatchMesh tools */


/* prototypes of functions local to this module: */

int ay_pmt_bilinearcltonpatch(ay_pamesh_object *pamesh, ay_object **result);

int ay_pmt_bicubiccltonpatch(ay_pamesh_object *pamesh, ay_object **result);


/* functions: */

/* ay_pmt_bilinearcltonpatch:
 *   create a NURBS patch from a closed bilinear PatchMesh
 */
int
ay_pmt_bilinearcltonpatch(ay_pamesh_object *pamesh, ay_object **result)
{
 int ay_status = AY_OK;
 double *cv = NULL;
 ay_object *o = NULL;
 int w, h, i, a, b;

  if(!pamesh || !result)
   return AY_ENULL;

  ay_status = ay_npt_createnpatchobject(&o);

  if(ay_status || !o)
    return AY_ERROR;

  if(pamesh->close_u)
    w = pamesh->width+1;
  else
    w = pamesh->width;

  if(pamesh->close_v)
    h = pamesh->height+1;
  else
    h = pamesh->height;

  if(!(cv = calloc(w*h*4, sizeof(double))))
    {
      free(o);
      return AY_EOMEM;
    }

  a = 0; b = 0;
  for(i = 0; i < pamesh->width; i++)
    {
      memcpy(&(cv[a]), &(pamesh->controlv[b]),
	     pamesh->height*4*sizeof(double));

      a += pamesh->height*4;
      if(pamesh->close_v)
	{
	  memcpy(&(cv[a]), &(pamesh->controlv[b]), 4*sizeof(double));
	  a += 4;
	}
      b += pamesh->height*4;
    } /* for */

  if(pamesh->close_u)
    {
      b = 0;
      if(pamesh->close_v)
	{
	  memcpy(&(cv[a]), &(pamesh->controlv[b]),
		 pamesh->height*4*sizeof(double));
	}
      else
	{
	  memcpy(&(cv[a]), &(pamesh->controlv[b]),
		 h*4*sizeof(double));
	}
    }

  ay_status = ay_npt_create(2, 2, w, h, AY_KTNURB, AY_KTNURB, cv, NULL, NULL,
			    (ay_nurbpatch_object **)(void*)&(o->refine));

  if(ay_status)
    {
      free(o);
      return ay_status;
    }

  *result = o;

 return AY_OK;
} /* ay_pmt_bilinearcltonpatch */


/* ay_pmt_bicubiccltonpatch:
 *   create a number of NURBS patches from a closed bicubic PatchMesh
 */
int
ay_pmt_bicubiccltonpatch(ay_pamesh_object *pamesh, ay_object **result)
{
 int ay_status = AY_OK;
 double *cv = NULL, *ncv = NULL;
 int evwinwidth, evwinheight, ktu, ktv, uorder, vorder, wrapu = 0, wrapv = 0;
 int i = 0, j = 0, winu, winv, k, a, b, nw, nh;
 ay_object *o = NULL, **nexto = NULL;
 char fname[] = "pmt_bicubiccltonpatch";

  if(!pamesh || !result)
   return AY_ENULL;

  uorder = 4;
  vorder = 4;
  switch(pamesh->btype_u)
    {
    case AY_BTBEZIER:
      ktu = AY_KTBEZIER;
      evwinwidth = 4;
      wrapu = 1;
      break;
    case AY_BTBSPLINE:
      ktu = AY_KTBSPLINE;
      evwinwidth = pamesh->width;
      wrapu = 3;
      break;
    default:
      ktu = AY_KTBSPLINE;
      evwinwidth = 4;
      wrapu = 3;
      break;
    }

  switch(pamesh->btype_v)
    {
    case AY_BTBEZIER:
      ktv = AY_KTBEZIER;
      evwinheight = 4;
      wrapv = 1;
      break;
    case AY_BTBSPLINE:
      ktv = AY_KTBSPLINE;
      evwinheight = pamesh->height;
      wrapv = 3;
      break;
    default:
      ktv = AY_KTBSPLINE;
      evwinheight = 4;
      wrapv = 3;
      break;
    }

  /* create new control vector with coordinates unwrapped */
  nw = pamesh->width + (pamesh->close_u?wrapu:0);
  nh = pamesh->height + (pamesh->close_v?wrapv:0);

  if(!(ncv = calloc(nw*nh*4, sizeof(double))))
    return AY_EOMEM;
  cv = pamesh->controlv;

  a = 0;
  b = 0;
  /* fill ncv */
  for(i = 0; i < pamesh->width; i++)
    {
      memcpy(&(ncv[a]), &(cv[b]), 4*pamesh->height*sizeof(double));
      a += pamesh->height*4;
      /* copy last, wrapped points in line */
      if(pamesh->close_v)
	{
	  memcpy(&(ncv[a]), &(cv[b]),
		 4*wrapv*sizeof(double));
	  a += 4*wrapv;
	} /* if */
      b += pamesh->height*4;

    } /* for */
  b = 0;
  /* copy last (wrapped) lines */
  if(pamesh->close_u)
    {
      for(i = 0; i < wrapu; i++)
	{
	  memcpy(&(ncv[a]), &(cv[b]), 4*pamesh->height*sizeof(double));
	  a += pamesh->height*4;
	  /* copy last, wrapped points in line */
	  if(pamesh->close_v)
	    {
	      memcpy(&(ncv[a]), &(cv[b]),
		     4*wrapv*sizeof(double));
	      a += 4*wrapv;
	    }

	  b += pamesh->height*4;
	} /* for */
    } /* if */

  /* now continue with ncv, as if the patch were not closed */
  if(evwinwidth != pamesh->width)
    {
      winu = (nw/(evwinwidth-1));
    }
  else
    {
      winu = 1;
      if(pamesh->close_u)
	evwinwidth = nw;
    }
  if(evwinheight != pamesh->height)
    {
      winv = (nh/(evwinheight-1));
    }
  else
    {
      winv = 1;
      if(pamesh->close_v)
	evwinheight = nh;
    }

  if(winu == 0)
    {
      free(ncv);
      ay_error(AY_ERROR, fname,
	       "Not enough control points in u direction.");
      return AY_ERROR;
    }
  if(winv == 0)
    {
      free(ncv);
      ay_error(AY_ERROR, fname,
	       "Not enough control points in v direction.");
      return AY_ERROR;
    }

  for(i = 0; i < winu; i++)
    {
      for(j = 0; j < winv; j++)
	{
	  /* create new object */
	  ay_status = ay_npt_createnpatchobject(&o);

	  if(ay_status || !o)
	    {
	      free(ncv);
	      return AY_ERROR;
	    }

	  if(!(cv = calloc(evwinwidth*evwinheight*4, sizeof(double))))
	    {
	      free(ncv); free(o);
	      return AY_EOMEM;
	    }

	  /* place evaluation window and copy control points */
	  a = 0;
	  for(k = 0; k < evwinwidth; k++)
	    {
	      b = ((3*i*nh)+(j*3)+(k*nh))*4;

	      /* because row/column order is identical for
		 PatchMeshes and NURBS Patches,
		 we may copy a whole line of control points
		 at once */
	      memcpy(&(cv[a]), &(ncv[b]),
		     4*evwinheight*sizeof(double));

	      a += (evwinheight*4);
	    } /* for */

	  ay_status = ay_npt_create(uorder, vorder,
				    evwinwidth, evwinheight,
				    ktu, ktv, cv, NULL, NULL,
				 (ay_nurbpatch_object **)(void*)&(o->refine));

	  if(ay_status)
	    {
	      free(o); free(cv);
	      goto cleanup;
	    }

	  /* link new object */
	  if(nexto)
	    {
	      *nexto = o;
	    }
	  else
	    {
	      *result = o;
	    }
	  nexto = &(o->next);
	} /* for */
    } /* for */

cleanup:

  if(ncv)
    free(ncv);

 return ay_status;
} /* ay_pmt_bicubiccltonpatch */


/* ay_pmt_tonpatch:
 *   create a number of NURBS patches from a PatchMesh
 *   o this function will not work properly or crash if the PatchMesh
 *     fails the validity check using ay_pmt_valid() below!
 */
int
ay_pmt_tonpatch(ay_pamesh_object *pamesh, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "pmt_tonpatch";
 double *cv = NULL;
 int evwinwidth, evwinheight, ktu, ktv, uorder, vorder;
 int i = 0, j = 0, winu, winv, k, a, b;
 ay_object *o = NULL, **nexto = NULL;

  if(!pamesh || !result)
    return AY_ENULL;

  if(pamesh->type == AY_PTBILINEAR)
    {
      if((pamesh->close_u) || (pamesh->close_v))
	{
	  ay_status = ay_pmt_bilinearcltonpatch(pamesh, result);
	  return ay_status;
	}

      ktu = AY_KTNURB;
      uorder = 2;
      evwinwidth = pamesh->width;
      ktv = AY_KTNURB;
      vorder = 2;
      evwinheight = pamesh->height;
    }
  else
    {
      if((pamesh->close_u) || (pamesh->close_v))
	{
	  ay_status = ay_pmt_bicubiccltonpatch(pamesh, result);
	  return ay_status;
	}

      uorder = 4;
      vorder = 4;
      switch(pamesh->btype_u)
	{
	case AY_BTBEZIER:
	  ktu = AY_KTBEZIER;
	  evwinwidth = 4;
	  break;
	case AY_BTBSPLINE:
	  ktu = AY_KTBSPLINE;
	  evwinwidth = pamesh->width;
      break;
	default:
	  ktu = AY_KTBSPLINE;
	  evwinwidth = 4;
	  break;
	}

      switch(pamesh->btype_v)
	{
	case AY_BTBEZIER:
	  ktv = AY_KTBEZIER;
	  evwinheight = 4;
	  break;
	case AY_BTBSPLINE:
	  ktv = AY_KTBSPLINE;
	  evwinheight = pamesh->height;
	  break;
	default:
	  ktv = AY_KTBSPLINE;
	  evwinheight = 4;
	  break;
	}

    } /* if */

  if((evwinwidth == pamesh->width) && (evwinheight == pamesh->height))
    {
      ay_status = ay_npt_createnpatchobject(&o);

      if(ay_status || !o)
	return AY_ERROR;

      if(!(cv = malloc(evwinwidth*evwinheight*4*sizeof(double))))
      {
	free(o);
	return AY_EOMEM;
      }
      memcpy(cv, pamesh->controlv, evwinwidth*evwinheight*4*sizeof(double));
      ay_status = ay_npt_create(uorder, vorder, evwinwidth, evwinheight,
				ktu, ktv, cv, NULL, NULL,
				(ay_nurbpatch_object **)(void*)&(o->refine));
      *result = o;
    }
  else
    {
      winu = (pamesh->width/(evwinwidth-1));
      winv = (pamesh->height/(evwinheight-1));

      if(winu == 0)
	{
	  ay_error(AY_ERROR, fname,
		   "Not enough control points in u direction.");
	  return AY_ERROR;
	}
      if(winv == 0)
	{
	  ay_error(AY_ERROR, fname,
		   "Not enough control points in v direction.");
	  return AY_ERROR;
	}

      for(i = 0; i < winu; i++)
	{
	  for(j = 0; j < winv; j++)
	    {
	      /* create new object */
	      ay_status = ay_npt_createnpatchobject(&o);

	      if(ay_status || !o)
		return AY_ERROR;

	      if(!(cv = calloc(evwinwidth*evwinheight*4, sizeof(double))))
		{
		  free(o);
		  return AY_EOMEM;
		}

	      /* place evaluation window and copy control points */
	      a = 0;
	      for(k = 0; k < evwinwidth; k++)
		{
		  b = ((3*i*pamesh->height)+(j*3)+(k*pamesh->height))*4;

		  /* because row/column order is identical for
		     PatchMeshes and NURBS Patches,
		     we may copy a whole line of control points
		     at once */
		  memcpy(&(cv[a]), &(pamesh->controlv[b]),
			     4*evwinheight*sizeof(double));

		  a += (evwinheight*4);
		} /* for */

	      ay_status = ay_npt_create(uorder, vorder,
					evwinwidth, evwinheight,
					ktu, ktv, cv, NULL, NULL,
				 (ay_nurbpatch_object **)(void*)&(o->refine));

	      /* link new object */
	      if(nexto)
		{
		  *nexto = o;
		}
	      else
		{
		  *result = o;
		}
	      nexto = &(o->next);
	    } /* for */
	} /* for */
    } /* if */

 return AY_OK;
} /* ay_pmt_tonpatch */


/** ay_pmt_valid:
 *  check patch mesh for validity
 *
 * \param[in] patch patch mesh object to process
 *
 * \returns AY_OK (0) if mesh is valid
 *   else:
 *  -1: NULL pointer delivered
 *   1: too few control points (need atleast 4 by 4)
 *   2: stepsize too small
 *   3: u-basistype width mismatch
 *   4: v-basistype height mismatch
 *
 */
int
ay_pmt_valid(ay_pamesh_object *pamesh)
{
 int stepu = 0, stepv = 0;

  if(!pamesh)
    return -1;

  if(pamesh->type == AY_PTBILINEAR)
    { /* bilinear patch mesh */
      /* nothing to check */
      return AY_OK;
    }
  else
    { /* bicubic patch mesh */

      if((pamesh->width < 4) || (pamesh->height < 4))
	{
	  return 1;
	}

      switch(pamesh->btype_u)
	{
	case AY_BTBEZIER:
	  stepu = 3;
	  break;
	case AY_BTBSPLINE:
	  stepu = 1;
	  break;
	case AY_BTCATMULLROM:
	  stepu = 1;
	  break;
	case AY_BTHERMITE:
	  stepu = 2;
	  break;
	case AY_BTCUSTOM:
	  stepu = pamesh->ustep;
	  break;
	default:
	  break;
	} /* switch */

      if(stepu <= 0)
	{
	  return 2;
	}

      if(pamesh->close_u)
	{ /* periodic patch */
	  if(fabs(fmod((double)pamesh->width-4+(4-stepu), stepu)) >
	     AY_EPSILON)
	    {
	      return 3;
	    }
	}
      else
	{ /* non periodic patch */
	  if(fabs(fmod((double)pamesh->width-4, stepu)) > AY_EPSILON)
	    {
	      return 3;
	    }
	}

      switch(pamesh->btype_v)
	{
	case AY_BTBEZIER:
	  stepv = 3;
	  break;
	case AY_BTBSPLINE:
	  stepv = 1;
	  break;
	case AY_BTCATMULLROM:
	  stepv = 1;
	  break;
	case AY_BTHERMITE:
	  stepv = 2;
	  break;
	case AY_BTCUSTOM:
	  stepv = pamesh->vstep;
	  break;
	default:
	  break;
	} /* switch */

      if(stepv <= 0)
	{
	  return 2;
	}

      if(pamesh->close_v)
	{ /* periodic patch */
	  if(fabs(fmod((double)pamesh->height-4+(4-stepv), stepv)) >
	     AY_EPSILON)
	    {
	      return 4;
	    }
	}
      else
	{ /* non periodic patch */
	  if(fabs(fmod((double)pamesh->height-4, stepv)) > AY_EPSILON)
	    {
	      return 4;
	    }
	}
    } /* if */

 return AY_OK;
} /* ay_pmt_valid */


/* ay_pmt_getpntfromindex:
 *  get address of a single control point from its indices
 *  (performing bounds checking)
 *
 * \param[in] patch patch mesh object to process
 * \param[in] indexu index of desired control point in U dimension (width)
 * \param[in] indexv index of desired control point in V dimension (height)
 * \param[in,out] p pointer to pointer where to store the resulting address
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_pmt_getpntfromindex(ay_pamesh_object *patch, int indexu, int indexv,
		       double **p)
{
 int stride = 4;
 char fname[] = "pmt_getpntfromindex";

  if(!patch || !p)
    return AY_ENULL;

  if(indexu >= patch->width || indexu < 0)
    return ay_error_reportirange(fname, "\"indexu\"", 0, patch->width-1);

  if(indexv >= patch->height || indexv < 0)
    return ay_error_reportirange(fname, "\"indexv\"", 0, patch->height-1);

  *p = &(patch->controlv[(indexu*patch->height+indexv)*stride]);

 return AY_OK;
} /* ay_pmt_getpntfromindex */


/** ay_pmt_swapuv:
 * swap U and V dimensions of a patch mesh
 *
 * \param[in,out] pm patch mesh object to process
 *
 * \returns AY_OK on success, error code otherwise.
 *
 */
int
ay_pmt_swapuv(ay_pamesh_object *pm)
{
 int ay_status = AY_OK;
 int i;
 double *dt;

  if(!pm)
    return AY_ENULL;

  ay_status = ay_npt_swaparray(&(pm->controlv), 4, pm->width, pm->height);

  if(ay_status)
    return ay_status;

  i = pm->width;
  pm->width = pm->height;
  pm->height = i;

  i = pm->btype_u;
  pm->btype_u = pm->btype_v;
  pm->btype_v = i;

  i = pm->ustep;
  pm->ustep = pm->vstep;
  pm->vstep = i;

  dt = pm->ubasis;
  pm->ubasis = pm->vbasis;
  pm->vbasis = dt;

 return ay_status;
} /* ay_pmt_swapuv */


/* ay_pmt_revertu:
 * revert patch mesh along U (width)
 *
 * \param[in,out] pm patch mesh object to revert
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_pmt_revertu(ay_pamesh_object *pm)
{
 int ay_status = AY_OK;
 int i, j, ii, jj, stride = 4;
 double t[4];

  for(i = 0; i < pm->height; i++)
    {
      for(j = 0; j < pm->width/2; j++)
	{
	  ii = (j*pm->height+i)*stride;
	  jj = ((pm->width-1-j)*pm->height+i)*stride;
	  memcpy(t, &(pm->controlv[ii]), stride*sizeof(double));
	  memcpy(&(pm->controlv[ii]), &(pm->controlv[jj]),
		 stride*sizeof(double));
	  memcpy(&(pm->controlv[jj]), t, stride*sizeof(double));
	}
    }

 return ay_status;
} /* ay_pmt_revertu */


/** ay_pmt_revertv:
 * revert patch mesh along V (height)
 *
 * \param[in,out] pm patch mesh object to revert
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_pmt_revertv(ay_pamesh_object *pm)
{
 int ay_status = AY_OK;
 int i, j, ii, jj, stride = 4;
 double t[4];

  for(i = 0; i < pm->width; i++)
    {
      ii = i*pm->height*stride;
      jj = ii + ((pm->height-1)*stride);
      for(j = 0; j < pm->height/2; j++)
	{
	  memcpy(t, &(pm->controlv[ii]), stride*sizeof(double));
	  memcpy(&(pm->controlv[ii]), &(pm->controlv[jj]),
		 stride*sizeof(double));
	  memcpy(&(pm->controlv[jj]), t, stride*sizeof(double));
	  ii += stride;
	  jj -= stride;
	}
    }

 return ay_status;
} /* ay_pmt_revertv */


/* ay_pmt_israt:
 *  check whether any control point of patch mesh <pm>
 *  uses a weight value (!= 1.0)
 */
int
ay_pmt_israt(ay_pamesh_object *pm)
{
 int i, j;
 double *p;

  if(!pm)
    return AY_FALSE;

  p = &(pm->controlv[3]);
  for(i = 0; i < pm->width; i++)
    {
      for(j = 0; j < pm->height; j++)
	{
	  if((fabs(*p) < (1.0-AY_EPSILON)) || (fabs(*p) > (1.0+AY_EPSILON)))
	    return AY_TRUE;
	  p += 4;
	} /* for */
    } /* for */

 return AY_FALSE;
} /* ay_pmt_israt */


/* ay_pmt_tobezier:
 */
int
ay_pmt_tobezier(ay_pamesh_object *pm)
{
 int ay_status;
 int convertu = AY_FALSE, convertv = AY_FALSE, i, j, k, l, i1, i2;
 int w, h, nw, nh;
 double *cv, *newcv, *expcv, *n, *p, *p1, *p2, *p3, *p4;
 double *n1, *n2, *n3, *n4;
 double m[16], tm[16], mu[16], mv[16], mut[16];
 double mbi[16];
 double mb[16] = {-1, 3, -3, 1,  3, -6, 3, 0,  -3, 3, 0, 0,  1, 0, 0, 0};
 double mh[16] = {2, -3, 0, 1,  1, -2, 1, 0,  -2, 3, 0, 0,  1, -1, 0, 0};
 double mc[16] = {-0.5, 1, -0.5, 1,  1.5, -2.5, 0, 1,  -1.5, 2, 0.5, 0,
		  0.5, -0.5, 0, 0};

#if 0
RtBasis RiBezierBasis = { { -1.0,  3.0, -3.0,  1.0 },
                          {  3.0, -6.0,  3.0,  0.0 },
                          { -3.0,  3.0,  0.0,  0.0 },
                          {  1.0,  0.0,  0.0,  0.0 }
                         };


/* Define One Sixth of */
#define Sx(a) ((a)/6)
RtBasis RiBSplineBasis = { { Sx(-1.0), Sx( 3.0), Sx(-3.0), Sx( 1.0) },
                           { Sx( 3.0), Sx(-6.0), Sx( 3.0), Sx( 0.0) },
                           { Sx(-3.0), Sx( 0.0), Sx( 3.0), Sx( 0.0) },
                           { Sx( 1.0), Sx( 4.0), Sx( 1.0), Sx( 0.0) }
                         };

/* Define One Half of */
#define Hf(a) ((a)/2)
RtBasis RiCatmullRomBasis = { { Hf(-1.0), Hf( 3.0), Hf(-3.0), Hf( 1.0) },
                              { Hf( 2.0), Hf(-5.0), Hf( 4.0), Hf(-1.0) },
                              { Hf(-1.0), Hf( 0.0), Hf( 1.0), Hf( 0.0) },
                              { Hf( 0.0), Hf( 2.0), Hf( 0.0), Hf( 0.0) }
                             };

RtBasis RiHermiteBasis = { {  2.0,  1.0, -2.0,  1.0 },
                           { -3.0, -2.0,  3.0, -1.0 },
                           {  0.0,  1.0,  0.0,  0.0 },
                           {  1.0,  0.0,  0.0,  0.0 }
                          };

RtBasis RiPowerBasis = { { 1.0, 0.0, 0.0, 0.0 },
                         { 0.0, 1.0, 0.0, 0.0 },
                         { 0.0, 0.0, 1.0, 0.0 },
                         { 0.0, 0.0, 0.0, 1.0 }
                        };
#endif

  if(!pm)
    return AY_FALSE;

  if(pm->btype_u != AY_BTBEZIER)
    {
      convertu = AY_TRUE;
    }

  if(pm->btype_v != AY_BTBEZIER)
    {
      convertv = AY_TRUE;
    }

  if(!convertu && !convertv)
    return AY_OK;

  /* invert target basis matrix */
  ay_status = ay_trafo_invgenmatrix4(mb, mbi);

  if(ay_status)
    return AY_ERROR;

  /* create conversion matrices */
  if(convertu)
    {
      memcpy(mu, mbi, 16*sizeof(double));
      switch(pm->btype_u)
	{
	case AY_BTHERMITE:
	  ay_trafo_multmatrix4(mu, mh);
	  break;
	case AY_BTCATMULLROM:
	  ay_trafo_multmatrix4(mu, mc);
	  break;
	case AY_BTCUSTOM:
	  ay_trafo_multmatrix4(mu, pm->ubasis);
	  break;
	}

      /* transpose u conversion matrix */
      i1 = 0;
      for(i = 0; i < 4; i++)
	{
	  i2 = i;
	  for(j = 0; j < 4; j++)
	    {
	      mut[i2] = mu[i1];

	      i1++;
	      i2 += 4;
	    } /* for */
	} /* for */
    } /* if convertu */

  if(convertv)
    {
      memcpy(mv, mbi, 16*sizeof(double));
      switch(pm->btype_v)
	{
	case AY_BTHERMITE:
	  ay_trafo_multmatrix4(mv, mh);
	  break;
	case AY_BTCATMULLROM:
	  ay_trafo_multmatrix4(mv, mc);
	  break;
	case AY_BTCUSTOM:
	  ay_trafo_multmatrix4(mv, pm->vbasis);
	  break;
	}
    } /* if convertv */

  w = pm->width;
  h = pm->height;

  nw = w;
  if(pm->close_u)
    nw += 3;

  nh = h;
  if(pm->close_v)
    nh += 3;

  cv = pm->controlv;
  if(!(newcv = malloc(pm->width*pm->height*4*sizeof(double))))
    return AY_EOMEM;

  /* copy weights */
  i1 = 3;
  for(i = 0; i < pm->width*pm->height; i++)
    {
      newcv[i1] = cv[i1];
      i1 += 4;
    }

  /* unwrap */
  if(nw != w || nh != h)
    {
      if(!(expcv = malloc(nw*nh*4*sizeof(double))))
	{ free(newcv); return AY_EOMEM; }

      for(i = 0; i < pm->width; i++)
	{
	  memcpy(&(expcv[i*nh*4]), &(cv[i*h*4]), 4*h*sizeof(double));

	  if(pm->close_v)
	    {
	      memcpy(&(expcv[i*nh*4+h*4]), &(cv[i*h*4]),
		     (nh-h)*4*sizeof(double));
	    }
	}

      if(pm->close_u)
	{
	  j = 0;
	  for(i = w; i < nw; i++)
	    {
	      memcpy(&(expcv[i*nh*4]), &(expcv[j*nh*4]), 4*nh*sizeof(double));
	      j++;
	    }
	}

      p = expcv;
    }
  else
    {
      p = pm->controlv;
    }

  n = newcv;
  i1 = pm->width/3;
  if(0&&pm->close_u)
    i1++;

  i2 = pm->height/3;
  if(0&&pm->close_v)
    i2++;

  /* convert control points */
  for(i = 0; i < i1; i++)
    {
      p1 = &(p[i*3*nh*4]);
      p2 = p1+(nh*4);
      p3 = p2+(nh*4);
      p4 = p3+(nh*4);

      n1 = &(n[i*3*h*4]);
      n2 = n1+(h*4);
      n3 = n2+(h*4);
      n4 = n3+(h*4);

      for(j = 0; j < i2; j++)
	{
	  /* for each control point component (x,y,z) */
	  for(k = 0; k < 3; k++)
	    {
	      /* get coordinates into a 4x4 matrix */
	      for(l = 0; l < 4; l++)
		{
		  m[l]    = *(p1+(l*4)+k);
		  m[l+4]  = *(p2+(l*4)+k);
		  m[l+8]  = *(p3+(l*4)+k);
		  m[l+12] = *(p4+(l*4)+k);
		}

	      /* apply conversion matrices */
	      if(convertv)
		{
		  memcpy(tm, mv, 16*sizeof(double));
		  ay_trafo_multmatrix4(tm, m);
		  memcpy(m, tm, 16*sizeof(double));
		}

	      if(convertu)
		{
		  ay_trafo_multmatrix4(m, mut);
		}

	      /* copy converted coordinates to newcv */
	      for(l = 0; l < 4; l++)
		{
		  *(n1+(l*4)+k) = m[l];
		  *(n2+(l*4)+k) = m[l+4];
		  *(n3+(l*4)+k) = m[l+8];
		  *(n4+(l*4)+k) = m[l+12];
		}
	    } /* for each component */

	  /* move conversion window 3 points downwards (3*4 = 12) */
	  p1 += 12;
	  p2 += 12;
	  p3 += 12;
	  p4 += 12;

	  n1 += 12;
	  n2 += 12;
	  n3 += 12;
	  n4 += 12;
	} /* for */
    } /* for */

  if(nw != w || nh != h)
    {
      free(expcv);
    }

  free(pm->controlv);
  pm->controlv = newcv;
  pm->btype_u = AY_BTBEZIER;
  pm->btype_v = AY_BTBEZIER;
  pm->ustep = 3;
  pm->vstep = 3;

 return AY_OK;
} /* ay_pmt_tobezier */


/** ay_pmt_tobeztcmd:
 *
 *  \returns TCL_OK in any case.
 */
int
ay_pmt_tobeztcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
 int ay_status;
 ay_list_object *sel = ay_selection;
 ay_pamesh_object *pm = NULL;

  while(sel)
    {
      if(sel->object->type == AY_IDPAMESH)
	{
	  pm = (ay_pamesh_object*)sel->object->refine;
	  ay_status = ay_pmt_tobezier(pm);
	  sel->object->modified = AY_TRUE;
	}
      if(sel->object->modified)
	{
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);
	  (void)ay_notify_object(sel->object);
	}

      sel = sel->next;
    } /* while */

  (void)ay_notify_parent();

 return TCL_OK;
} /* ay_pmt_tobeztcmd */
