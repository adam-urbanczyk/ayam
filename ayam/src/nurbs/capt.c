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

/** \file capt.c \brief cap creation tools */

/** ay_capt_crtsimplecap:
 *  create a simple cap surface from a single NURBS curve
 *
 * @param[in,out] c NURBS curve object
 * @param[in,out] cap new NURBS patch object
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_capt_crtsimplecap(ay_object *c, ay_object **cap)
{
 int ay_status = AY_OK;
 ay_object *npatch = NULL;
 ay_nurbcurve_object *nc = NULL;
 ay_nurbpatch_object *np = NULL;
 double knotv[4] = {0.0,0.0,1.0,1.0};
 int a, i = 0, stride = 4;
 double m[4] = {0};

  if(!c || !cap)
    return AY_ENULL;

  if(c->type != AY_IDNCURVE)
    { ay_status = AY_ERROR; goto cleanup; }

  nc = (ay_nurbcurve_object *)(c->refine);

  ay_status = ay_npt_createnpatchobject(&npatch);
  if(ay_status)
    { goto cleanup; }

  ay_trafo_copy(c, npatch);

  /* calloc the new patch */
  if(!(np = calloc(1, sizeof(ay_nurbpatch_object))))
    { ay_status = AY_EOMEM; goto cleanup; }
  npatch->refine = np;

  np->width = 2;
  np->height = nc->length;
  np->uorder = 2;
  np->vorder = nc->order;

  np->uknot_type = AY_KTNURB;
  np->vknot_type = nc->knot_type;

  if(!(np->vknotv = malloc((nc->length+nc->order) * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(np->uknotv = malloc(4 * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(np->controlv = malloc(np->width*np->height*stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  memcpy(np->uknotv, knotv, 4*sizeof(double));
  memcpy(np->vknotv, nc->knotv, (nc->length+nc->order)*sizeof(double));

  memcpy(np->controlv, nc->controlv, nc->length*stride*sizeof(double));

  ay_status = ay_npt_extractmiddlepoint(0, nc->controlv, nc->length,
					stride, NULL, m);

  if(ay_status)
    goto cleanup;

  a = nc->length*stride;
  for(i = 0; i < nc->length; i++)
    {
      memcpy(&(np->controlv[a]), m, 3*sizeof(double));
      np->controlv[a+3] = 1.0;/*nc->controlv[b+3];*/
      a += stride;
    }

  np->is_rat = nc->is_rat;

  ay_npt_setuvtypes(np);

  /* return result */
  *cap = npatch;

  /* prevent cleanup code from doing something harmful */
  npatch = NULL;

cleanup:

  if(npatch)
    {
      if(npatch->refine)
	ay_npt_destroy(npatch->refine);

      ay_object_delete(npatch);
    }

 return ay_status;
} /* ay_capt_crtsimplecap */


/** ay_capt_crtsimplecapint:
 *  create a simple cap surface from a single NURBS curve
 *  and integrate it into the NURBS surface
 *
 * @param[in] side integration place
 * @param[in,out] s NURBS surface object for integration
 * @param[in,out] c NURBS curve object
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_capt_crtsimplecapint(int side, ay_object *s, ay_object *c)
{
 int ay_status = AY_OK;
 ay_object *cc = NULL, *cap = NULL, *o = NULL, *oldnext;
 ay_nurbpatch_object *np = NULL;
 char *uv = NULL, uvs[][4] = {"Vu","vu","Uu","uu"};
 int knottype = AY_KTCUSTOM, order = 0;

  if(!s || !c)
    return AY_ENULL;

  if(!s->type == AY_IDNPATCH)
    return AY_ERROR;

  np = (ay_nurbpatch_object*)s->refine;

  ay_status = ay_capt_crtsimplecap(c, &cap);

  if(ay_status)
    goto cleanup;

  uv = uvs[side];

  oldnext = s->next;
  cc = s;
  s->next = cap;

  switch(side)
    {
    case 0:
    case 1:
      if(np->vorder != 2)
	{
	  ay_status = ay_npt_elevateu((ay_nurbpatch_object*)cap->refine,
				      np->vorder-2);
	}
      order = np->vorder;
      break;
    case 2:
    case 3:
      if(np->uorder != 2)
	{
	  ay_status = ay_npt_elevateu((ay_nurbpatch_object*)cap->refine,
				      np->uorder-2);
	}
      order = np->uorder;
      break;
    } /* switch */

  if(ay_status)
    goto cleanup;

#if 0
  switch(knottype)
    {
    AY_KTBEZIER:
      break;
    AY_KTBSPLINE:
      break;
    AY_KTNURB:
      break;

    default:
      break;
    }
#endif

  knottype = AY_KTCUSTOM;

  ay_status = ay_npt_concat(cc, 0, order, knottype, 0, 0.0, AY_TRUE, uv,
			    &o);

  if(ay_status)
    goto cleanup;

  /* correct orientation of concatenated surface */
  if(side < 2)
    ay_npt_swapuv(o->refine);

  if(side == 0)
    ay_npt_revertv(o->refine);

  if(side == 2)
    ay_npt_revertu(o->refine);


  /* replace old patch with new */
  ay_npt_destroy(s->refine);
  s->refine = o->refine;
  o->refine = NULL;
  ay_object_delete(o);

  s->next = oldnext;

  /* copy transformations/tags? */

cleanup:

  if(cap)
    ay_object_delete(cap);

 return ay_status;
} /* ay_capt_crtsimplecapint */


/** ay_capt_crttrimcap:
 *  create a cap surface from planar NURBS curves; the curve objects
 *  will be transformed and moved to the new NURBS patch as trim curves
 *
 * @param[in,out] c NURBS curve object(s); multiple objects may
 *  be provided as list of connected objects; the first curve
 *  must be the outmost curve, all following curves define
 *  holes; the curve orientations will be detected and adjusted
 *  properly
 * @param[in,out] cap new NURBS patch object
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_capt_crttrimcap(ay_object *c, ay_object **cap)
{
 int ay_status = AY_OK;
 ay_object *npatch = NULL;
 ay_nurbcurve_object *nc = NULL;
 ay_nurbpatch_object *np = NULL;
 double knotv[4] = {0.0,0.0,1.0,1.0};
 double minx = 0.0, miny = 0.0, maxx = 0.0, maxy = 0.0, angle = 0.0;
 double firstmovx = 0.0, firstmovy = 0.0, trimmx = 0.0, trimmy = 0.0;
 int first = AY_TRUE, i = 0, stride = 4;
 double z;

  if(!c || !cap)
    return AY_ENULL;

  while(c)
    {
      if(c->type != AY_IDNCURVE)
	{ ay_status = AY_ERROR; goto cleanup; }

      ay_status = ay_nct_toxy(c);
      if(ay_status)
	{ ay_status = AY_ERROR; goto cleanup; }

      nc = (ay_nurbcurve_object *)(c->refine);

      if(first)
	{
	  firstmovx = c->movx;
	  firstmovy = c->movy;
	  ay_status = ay_npt_createnpatchobject(&npatch);
	  if(ay_status)
	    { goto cleanup; }

	  npatch->down = c;

	  ay_trafo_copy(c, npatch);
	  npatch->scalx = 1.0;
	  npatch->scaly = 1.0;
	  npatch->scalz = 1.0;

	  /* calloc the new patch */
	  if(!(np = calloc(1, sizeof(ay_nurbpatch_object))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  npatch->refine = np;
	  if(!(np->vknotv = malloc(4 * sizeof(double))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  if(!(np->uknotv = malloc(4 * sizeof(double))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  if(!(np->controlv = calloc(4*4, sizeof(double))))
	    { ay_status = AY_EOMEM; goto cleanup; }

	  np->width = 2;
	  np->height = 2;
	  np->uorder = 2;
	  np->vorder = 2;
	  memcpy(np->uknotv, knotv, 4*sizeof(double));
	  memcpy(np->vknotv, knotv, 4*sizeof(double));

	  i = 0;
	  minx = DBL_MAX; maxx = -DBL_MAX;
	  miny = DBL_MAX; maxy = -DBL_MAX;

	  stride = 4;
	  while(i < nc->length*stride)
	    {
	      if(nc->controlv[i] > maxx)
		maxx = nc->controlv[i];
	      if(nc->controlv[i] < minx)
		minx = nc->controlv[i];
	      if(nc->controlv[i+1] > maxy)
		maxy = nc->controlv[i+1];
	      if(nc->controlv[i+1] < miny)
		miny = nc->controlv[i+1];

	      i += stride;
	    } /* while */

	  /* clear curve trafo */
	  ay_trafo_defaults(c);

	  np->controlv[0] = minx;
	  np->controlv[1] = miny;

	  np->controlv[4] = minx;
	  np->controlv[5] = maxy;

	  np->controlv[8] = maxx;
	  np->controlv[9] = miny;

	  np->controlv[12] = maxx;
	  np->controlv[13] = maxy;

	  z = nc->controlv[2];

	  for(i = 2; i <= 15; i += 4)
	    {
	      np->controlv[i] = z;
	      np->controlv[i+1] = 1.0;
	    } /* for */
	}
      else
	{
	  /* clear trafo */
	  ay_nct_applytrafo(c);
	} /* if */

      ay_nct_getorientation(nc, 4, &angle);

      if(first)
	{
	  if(angle < 0.0)
	    {
	      ay_nct_revert(nc);
	    } /* if */

	  c->scalx /= fabs(maxx-minx);
	  c->scaly /= fabs(maxy-miny);
	  c->movx = -(minx + (fabs(maxx-minx)/2.0))*c->scalx;
	  c->movy = -(miny + (fabs(maxy-miny)/2.0))*c->scaly;

	  c->movx += 0.5;
	  c->movy += 0.5;
	}
      else
	{
	  if(angle > 0.0)
	    {
	      ay_nct_revert(nc);
	    } /* if */

	  trimmx = c->movx-firstmovx;
	  trimmy = c->movy-firstmovy;

	  c->scalx /= fabs(maxx-minx);
	  c->scaly /= fabs(maxy-miny);
	  c->movx = -(minx + (fabs(maxx-minx)/2.0))*c->scalx;
	  c->movy = -(miny + (fabs(maxy-miny)/2.0))*c->scaly;

	  c->movx += 0.5+(trimmx/fabs(maxx-minx));
	  c->movy += 0.5+(trimmy/fabs(maxy-miny));
	} /* if */

      if(!c->next)
	{
	  c->next = ay_endlevel;
	  c = NULL;
	}
      else
	{
	  c = c->next;
	} /* if */

      first = AY_FALSE;
    } /* while */

  /* return result */
  *cap = npatch;

  /* prevent cleanup code from doing something harmful */
  npatch = NULL;

cleanup:

  if(npatch)
    {
      if(npatch->refine)
	ay_npt_destroy(npatch->refine);

      /* prevent curves from being deleted here */
      npatch->down = NULL;

      ay_object_delete(npatch);
    }

 return ay_status;
} /* ay_capt_crttrimcap */


/** ay_capt_crtgordoncap:
 *  create a cap surface from a single non-planar NURBS curve
 *  by splitting the outline into four pieces, arranging the
 *  pieces properly and building a Gordon surface from them
 *  (thanks to "the reverse" for inspiration);
 *  the curve object will be modified (split)!
 *
 *  XXXX allow parameterisation of split points and desired surface orders
 *
 * @param[in,out] c NURBS curve object
 * @param[in,out] cap new NURBS patch object
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_capt_crtgordoncap(ay_object *c, ay_object **cap)
{
 int ay_status = AY_OK;
 ay_object *c1 = NULL, *c2 = NULL, *c3 = NULL, *c4 = NULL, *new = NULL;
 ay_nurbcurve_object *curve = NULL;
 int i = 0, numhknots = 5;
 double hknots[5] = {0.5, 0.495, 0.496, 0.504, 0.53333};
 /*double u12, u14, u34;*/

  c1 = c;

  if(c1->type != AY_IDNCURVE)
    return AY_ERROR;

  ay_status = ay_npt_createnpatchobject(&new);
  if(ay_status)
    goto cleanup;

  curve = (ay_nurbcurve_object*)c1->refine;

  /* split curve in half */
  ay_nct_clamp(curve, 0);
  ay_status = ay_knots_rescaletorange(curve->length+curve->order, curve->knotv,
				      0.0, 1.0);
  i = 0;
  ay_status = AY_OK;
  while((!c3 || ay_status) && (i < numhknots))
    {
      ay_status = ay_nct_split(c1, hknots[i], &c3);
      i++;
    }
  if(!c3 || ay_status)
    goto cleanup;

  /* split first half in first and second quarter */
  ay_nct_clamp(curve, 1);
  ay_status = ay_knots_rescaletorange(curve->length+curve->order, curve->knotv,
				      0.0, 1.0);
  i = 0;
  ay_status = AY_OK;
  while((!c2 || ay_status) && (i < numhknots))
    {
      ay_status = ay_nct_split(c1, hknots[i], &c2);
      i++;
    }
  if(!c2 || ay_status)
    goto cleanup;

  /* split second half in third and fourth quarter */
  curve = (ay_nurbcurve_object*)c3->refine;
  ay_nct_clamp(curve, 2);
  ay_status = ay_knots_rescaletorange(curve->length+curve->order, curve->knotv,
				      0.0, 1.0);
  i = 0;
  ay_status = AY_OK;
  while((!c4 || ay_status) && (i < numhknots))
    {
      ay_status = ay_nct_split(c3, hknots[i], &c4);
      i++;
    }
  if(!c4 || ay_status)
    goto cleanup;

  /* arrange quarters */
  ay_status = ay_nct_revert((ay_nurbcurve_object*)c3->refine);
  if(ay_status)
    goto cleanup;

  ay_status = ay_nct_revert((ay_nurbcurve_object*)c4->refine);
  if(ay_status)
    goto cleanup;

  c1->next = c3;
  c4->next = c2;

  /* create Gordon surface */
  ay_status = ay_npt_gordon(c1, c4, NULL,
			    curve->order, curve->order,
			    (ay_nurbpatch_object**)(void*)&(new->refine));

  if(ay_status)
    goto cleanup;

  /* return result */
  *cap = new;

  /* prevent cleanup code from doing something harmful */
  new = NULL;

  /* clean up */
cleanup:

  if(new)
    ay_object_delete(new);

  /* c1 deleted outside... */
  if(c2)
    ay_object_delete(c2);
  if(c3)
    ay_object_delete(c3);
  if(c4)
    ay_object_delete(c4);

 return ay_status;
} /* ay_capt_crtgordoncap */


/** ay_capt_addcaps:
 * Create cap surfaces for a NURBS surface.
 *
 * @param[in] caps designates which caps to create (and their types)
 * @param[in] bparams bevel parameters, if a bevel is to be created
 *  at the corresponding boundary, we do not create the cap here but
 *  later when creating the bevel
 * @param[in] o NURBS patch object
 * @param[in,out] cap list of new NURBS patch objects (the caps)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_capt_addcaps(int *caps, ay_bparam *bparams, ay_object *o, ay_object **dst)
{
 int ay_status = AY_OK;
 int i, side;
 double param = 0.0;
 ay_object *extrcurve = NULL;
 ay_object *cap = NULL, **nextcap = dst;
 ay_nurbpatch_object *np = NULL;

  if(!caps || !bparams || !o || !dst)
    return AY_ENULL;

  if(o->type != AY_IDNPATCH)
   return AY_ERROR;

  np = (ay_nurbpatch_object*)o->refine;

  for(i = 0; i < 4; i++)
    {
      if(caps[i] && !bparams->states[i])
	{
	  cap = NULL;

	  if(!(extrcurve = calloc(1, sizeof(ay_object))))
	    {
	      ay_status = AY_EOMEM;
	      goto cleanup;
	    }
	  ay_object_defaults(extrcurve);
	  extrcurve->type = AY_IDNCURVE;
	  param = 0.0;
	  switch(i)
	    {
	    case 0:
	      if(ay_knots_isclamped(/*side=*/1, np->vorder, np->vknotv,
				    np->vorder+np->height, AY_EPSILON))
		side = 0;
	      else
		side = 4;
	      break;
	    case 1:
	      if(ay_knots_isclamped(/*side=*/2, np->vorder, np->vknotv,
				    np->vorder+np->height, AY_EPSILON))
		side = 1;
	      else
		{
		  side = 4;
		  param = 1.0;
		}
	      break;
	    case 2:
	      if(ay_knots_isclamped(/*side=*/1, np->uorder, np->uknotv,
				    np->uorder+np->width, AY_EPSILON))
		side = 2;
	      else
		side = 5;
	      break;
	    case 3:
	      if(ay_knots_isclamped(/*side=*/2, np->uorder, np->uknotv,
				    np->uorder+np->width, AY_EPSILON))
		side = 3;
	      else
		{
		  side = 5;
		  param = 1.0;
		}
	      break;
	    default:
	      break;
	    } /* switch */

	  ay_status = ay_npt_extractnc(o, side, param,
				       /*relative=*/AY_TRUE,
				       /*apply_trafo=*/AY_FALSE,
				       /*extractnt=*/AY_FALSE, NULL,
			  (ay_nurbcurve_object**)(void*)&(extrcurve->refine));

	  if(ay_status)
	    {
	      free(extrcurve);
	      goto cleanup;
	    }

	  if(ay_nct_isdegen((ay_nurbcurve_object*)(void*)extrcurve->refine))
	    {
	      ay_object_delete(extrcurve);
	      continue;
	    }

	  switch(caps[i])
	    {
	    case 1:
	      ay_status = ay_capt_crttrimcap(extrcurve, &cap);
	      break;
	    case 2:
	      ay_status = ay_capt_crtsimplecap(extrcurve, &cap);
	      if(!ay_status && (i > 1))
		ay_status = ay_npt_swapuv(cap->refine);
	      break;
	    case 3:
	      ay_status = ay_capt_crtsimplecapint(i, o, extrcurve);
	      break;
	    case 4:
	      ay_status = ay_capt_crtgordoncap(extrcurve, &cap);
	      break;
	    default:
	      ay_status = AY_ERROR;
	    } /* switch */

	  if(caps[i] != 1)
	    ay_object_delete(extrcurve);

	  if(ay_status)
	    goto cleanup;

	  if(cap)
	    {
	      *nextcap = cap;
	      nextcap = &(cap->next);
	    }
	} /* if */
    } /* for */

cleanup:

 return ay_status;
} /* ay_capt_addcaps */
