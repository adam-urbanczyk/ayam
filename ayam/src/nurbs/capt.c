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

/* capt.c NURBS cap creation tools */


/* ay_capt_createfromcurve:
 *  create a cap from a single NURBS curve c
 *  the curve object c will be used as trim curve
 *  so copy it before calling this function,
 *  if c already lives in the scene hierarchy!
 */
int
ay_capt_createfromcurve(ay_object *c, ay_object **ca)
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

  if(!c || !ca)
    return AY_ENULL;

  while(c)
    {

      if(!c->type == AY_IDNCURVE)
	return AY_ERROR;

      ay_status = ay_nct_toxy(c);
      if(ay_status)
	return AY_ERROR;

      nc = (ay_nurbcurve_object *)(c->refine);

      if(first)
	{
	  firstmovx = c->movx;
	  firstmovy = c->movy;

	  if(!(npatch = calloc(1, sizeof(ay_object))))
	    return AY_EOMEM;
	  ay_object_defaults(npatch);
	  npatch->type = AY_IDNPATCH;
	  npatch->parent = AY_TRUE;
	  npatch->inherit_trafos = AY_FALSE;
	  npatch->hide_children = AY_TRUE;
	  npatch->down = c;

	  ay_trafo_copy(c, npatch);
	  npatch->scalx = 1.0;
	  npatch->scaly = 1.0;
	  npatch->scalz = 1.0;

	  /* calloc the new patch */
	  if(!(np = calloc(1, sizeof(ay_nurbpatch_object))))
	    return AY_EOMEM;
	  npatch->refine = np;
	  if(!(np->vknotv = calloc(4, sizeof(double))))
	    { free(np); return AY_EOMEM; }
	  if(!(np->uknotv = calloc(4, sizeof(double))))
	    { free(np); free(np->vknotv); return AY_EOMEM; }
	  if(!(np->controlv = calloc(4*4, sizeof(double))))
	    { free(np); free(np->vknotv); free(np->uknotv); return AY_EOMEM; }

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
	  ay_object_crtendlevel(&(c->next));
	  c = NULL;
	}
      else
	{
	  c = c->next;
	} /* if */

      first = AY_FALSE;
    } /* while */

  *ca = npatch;

 return ay_status;
} /* ay_capt_createfromcurve */

