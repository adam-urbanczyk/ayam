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

/* capt.c NURBS cap creation tools */


/* ay_capt_createfromcurve:
 *  create a cap surface from planar NURBS curves;
 *  the curve objects will be transformed and moved to
 *  the new NURBS patch as trim curves
 *
 * @param[in,out] c NURBS curve objects; multiple objects may
 *  be provided as list of connected objects; the first curve
 *  must be the outmost curve, all following curves define
 *  holes
 * @param[in,out] cap new NURBS patch object
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_capt_createfromcurve(ay_object *c, ay_object **cap)
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
	  if(!(np->vknotv = calloc(4, sizeof(double))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  if(!(np->uknotv = calloc(4, sizeof(double))))
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

	  /* clear trafo */
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
	  ay_trafo_defaults(c);
	} /* if */

      ay_nct_getorientation(nc, &angle);

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
} /* ay_capt_createfromcurve */


/* ay_capt_createfromnpcurve:
 *  create a cap surface from a single non-planar NURBS curve
 *  by splitting the outline into four pieces, arranging the
 *  pieces and building a Gordon surface from them (thanks to
 *  "the reverse" for inspiration);
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
ay_capt_createfromnpcurve(ay_object *c, ay_object **cap)
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
} /* ay_capt_createfromnpcurve */
