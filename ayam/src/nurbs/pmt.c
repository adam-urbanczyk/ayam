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

/* pmt.c PatchMesh tools */


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
 double *cv = NULL;
 ay_object *o = NULL;
 int w, h, i, a, b;
 int ay_status = AY_OK;

  if(!(o = calloc(1, sizeof(ay_object))))
    return AY_EOMEM;
  ay_object_defaults(o);
  o->type = AY_IDNPATCH;

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
			    (ay_nurbpatch_object **)&(o->refine));
  *result = o;

 return AY_OK;
} /* ay_pmt_bilinearcltonpatch */


/* ay_pmt_bicubiccltonpatch:
 *   create a number of NURBS patches from a closed bicubic PatchMesh
 */
int
ay_pmt_bicubiccltonpatch(ay_pamesh_object *pamesh, ay_object **result)
{
 double *cv = NULL, *ncv = NULL;
 int evwinwidth, evwinheight, ktu, ktv, uorder, vorder, wrapu = 0, wrapv = 0;
 int i = 0, j = 0, winu, winv, k, a, b, nw, nh;
 ay_object *o = NULL, **nexto = NULL;
 char fname[] = "ay_pmt_bicubiccltonpatch";
 int ay_status = AY_OK;

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
      for(i = 0; i<wrapu; i++)
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
	  o = NULL;
	  if(!(o = calloc(1, sizeof(ay_object))))
	    return AY_EOMEM;
	  ay_object_defaults(o);
	  o->type = AY_IDNPATCH;

	  cv = NULL;
	  if(!(cv = calloc(evwinwidth*evwinheight*4, sizeof(double))))
	    {
	      free(o);
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
				    (ay_nurbpatch_object **)&(o->refine));

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

	}/* for */

    }/* for */

  if(ncv)
    free(ncv);

 return AY_OK;
} /* ay_pmt_bicubiccltonpatch */


/* ay_pmt_tonpatch:
 *   create a number of NURBS patches from a PatchMesh
 *   o this function will not work properly or crash if the PatchMesh
 *     fails the validity check using ay_pmt_valid() below!
 */
int
ay_pmt_tonpatch(ay_pamesh_object *pamesh, ay_object **result)
{
 double *cv = NULL;
 int evwinwidth, evwinheight, ktu, ktv, uorder, vorder;
 int i = 0, j = 0, winu, winv, k, a, b;
 ay_object *o = NULL, **nexto = NULL;
 char fname[] = "ay_pmt_tonpatch";
 int ay_status = AY_OK;

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
      if(!(o = calloc(1, sizeof(ay_object))))
	return AY_EOMEM;
      ay_object_defaults(o);
      o->type = AY_IDNPATCH;

      if(!(cv = calloc(evwinwidth*evwinheight*4, sizeof(double))))
      {
	free(o);
	return AY_EOMEM;
      }
      memcpy(cv, pamesh->controlv, evwinwidth*evwinheight*4*sizeof(double));
      ay_status = ay_npt_create(uorder, vorder, evwinwidth, evwinheight,
				ktu, ktv, cv, NULL, NULL,
				(ay_nurbpatch_object **)&(o->refine));
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
	      o = NULL;
	      if(!(o = calloc(1, sizeof(ay_object))))
		return AY_EOMEM;
	      ay_object_defaults(o);
	      o->type = AY_IDNPATCH;

	      cv = NULL;
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
					(ay_nurbpatch_object **)&(o->refine));

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


	    }/* for */
	}/* for */

    } /* if */

 return AY_OK;
} /* ay_pmt_tonpatch */


/* ay_pmt_valid:
 *   check PatchMesh pamesh for validity
 *   returns AY_OK (0) if mesh is valid
 *   else:
 *   1: too few control points (need atleast 4 by 4)
 *   2: stepsize too small
 *   3: u-basistype width mismatch
 *   4: v-basistype height mismatch
 *
 */
int
ay_pmt_valid(ay_pamesh_object *pamesh, int *detail)
{
 int stepu = 0, stepv = 0;

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
	      /**detail =*/
	      return 3;
	    }
	}
      else
	{ /* non periodic patch */
	  if(fabs(fmod((double)pamesh->width-4, stepu)) > AY_EPSILON)
	    {
	      /**detail =*/
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
	      /**detail =*/
	      return 4;
	    }
	}
      else
	{ /* non periodic patch */
	  if(fabs(fmod((double)pamesh->height-4, stepv)) > AY_EPSILON)
	    {
	      /**detail =*/
	      return 4;
	    }
	}
    } /* if */

  return AY_OK;
} /* ay_pmt_valid */


/* ay_pmt_getpntfromindex:
 *
 *
 */
int
ay_pmt_getpntfromindex(ay_pamesh_object *patch, int indexu, int indexv,
		       double **p)
{
 int stride = 4;
 char fname[] = "ay_pmt_getpntfromindex";

  if(indexu >= patch->width || indexu < 0)
    {
      ay_error(AY_ERROR, fname, "index u out of range");
      return TCL_OK;
    }

  if(indexv >= patch->height || indexv < 0)
    {
      ay_error(AY_ERROR, fname, "index v out of range");
      return TCL_OK;
    }

  *p = &(patch->controlv[(indexu*patch->width+indexv)*stride]);

 return TCL_OK;
} /* ay_pmt_getpntfromindex */


/* ay_pmt_swapuv:
 *
 *
 */
int
ay_pmt_swapuv(ay_pamesh_object *pm)
{
 int ay_status = AY_OK;
 int stride = 4, i1 = 0, i2 = 0, i, j;
 double *dt, *ncontrolv = NULL;

  if(!pm)
    return AY_ENULL;

  if(!(ncontrolv = calloc(pm->width*pm->height*stride, sizeof(double))))
    return AY_EOMEM;

  for(i = 0; i < pm->width; i++)
    {
      i2 = i*stride;
      for(j = 0; j < pm->height; j++)
	{
	  memcpy(&(ncontrolv[i2]), &(pm->controlv[i1]),
		 stride*sizeof(double));

	  i1 += stride;
	  i2 += (pm->width*stride);
	} /* for */
    } /* for */

  free(pm->controlv);
  pm->controlv = ncontrolv;

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
 *
 *
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

/* ay_pmt_revertv:
 *  
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
