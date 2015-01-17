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

/** ay_capt_addcaps:
 * Create cap surfaces for a NURBS surface.
 *
 * \param[in] cparams designates which caps to create (and their types)
 * \param[in] bparams bevel parameters, if a bevel is to be created
 *  at the corresponding boundary, we do not create the cap here but
 *  later when creating the bevel
 * \param[in,out] o NURBS patch object (may be modified
 *  if the integrate parameter of a cap is set)
 * \param[in,out] dst list of new NURBS patch objects,
 *  (may stay empty if all caps are integrated into the surface)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_capt_addcaps(ay_cparam *cparams, ay_bparam *bparams,
		ay_object *o, ay_object **dst)
{
 int ay_status = AY_OK;
 int i, side;
 double param = 0.0, *mp;
 ay_object *extrcurve = NULL;
 ay_object *cap = NULL, **nextcap = dst;
 ay_nurbpatch_object *np = NULL;

  if(!cparams || !bparams || !o || !dst)
    return AY_ENULL;

  for(i = 0; i < 4; i++)
    {
      if(o->type != AY_IDNPATCH)
	return AY_ERROR;

      np = (ay_nurbpatch_object*)o->refine;

      if(cparams->states[i] && !bparams->states[i])
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
	      (void)ay_object_delete(extrcurve);
	      continue;
	    }

	  switch(cparams->types[i])
	    {
	    case 0:
	      ay_status = ay_capt_crttrimcap(extrcurve, &cap);
	      break;
	    case 1:
	      ay_status = ay_capt_crtgordoncap(extrcurve, &cap);
	      break;
	    case 2:
	      if(cparams->use_mp[i])
		mp = &(cparams->mp[i*3]);
	      else
		mp = NULL;
	      ay_status = ay_capt_crtsimplecap(extrcurve, 0, cparams->frac[i],
					       mp, &cap);
	      if(!ay_status && (i > 1))
		ay_status = ay_npt_swapuv(cap->refine);
	      break;
	    case 3:
	      if(cparams->use_mp[i])
		mp = &(cparams->mp[i*3]);
	      else
		mp = NULL;
	      ay_status = ay_capt_crtsimplecap(extrcurve, 1, cparams->frac[i],
					       mp, &cap);
	      if(!ay_status && (i > 1))
		ay_status = ay_npt_swapuv(cap->refine);
	      break;
	    default:
	      ay_status = AY_ERROR;
	    } /* switch */

	  if(cparams->types[i] != 0)
	    (void)ay_object_delete(extrcurve);

	  if(ay_status)
	    goto cleanup;

	  if(cparams->integrate[i])
	    {
	      ay_status = ay_capt_integrate(cap, i, AY_KTCUSTOM, o);
	      (void)ay_object_delete(cap);
	      cap = NULL;
	      if(ay_status)
		goto cleanup;
	    }

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


/** ay_capt_crtsimplecap:
 * Create a simple cap surface from a single NURBS curve.
 *
 * \param[in] c NURBS curve object
 * \param[in] mode 0 - 2D, 1 - 3D quadric, 2 - 3D cubic
 * \param[in] frac fraction parameter for 3D mode
 * \param[in] mp middle point [double*3], may be NULL
 * \param[in,out] cap new NURBS patch object
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_capt_crtsimplecap(ay_object *c, int mode, double frac, double *mp,
		     ay_object **cap)
{
 int ay_status = AY_OK;
 ay_object *npatch = NULL;
 ay_nurbcurve_object *nc = NULL, *rnc = NULL;
 ay_nurbpatch_object *np = NULL;
 double knotv0[4] = {0.0,0.0,1.0,1.0};
 double knotv1[6] = {0.0,0.0,0.0,1.0,1.0,1.0};
 double knotv2[8] = {0.0,0.0,0.0,0.0,1.0,1.0,1.0,1.0};
 int a, b = 0, i = 0, stride = 4, shiftlen, numuniq;
 double r = 0.0, m[4] = {0}, n[3] = {0}, z[3] = {0.0,0.0,1.0};
 double *p, *q1, *q2, angle, anglem, len, rm[16], rotaxis[3], *circcv = NULL;
 ay_object *rc = NULL;

  if(!c || !cap)
    return AY_ENULL;

  if(c->type != AY_IDNCURVE)
    { ay_status = AY_ERROR; goto cleanup; }

  nc = (ay_nurbcurve_object *)(c->refine);

  if(nc->length < 3)
    { ay_status = AY_ERROR; goto cleanup; }

  ay_status = ay_npt_createnpatchobject(&npatch);
  if(ay_status)
    { goto cleanup; }

  ay_trafo_copy(c, npatch);

  /* calloc the new patch */
  if(!(np = calloc(1, sizeof(ay_nurbpatch_object))))
    { ay_status = AY_EOMEM; goto cleanup; }
  npatch->refine = np;

  if(mode)
    {
      if(mode == 1)
	{
	  np->width = 3;
	  np->uorder = 3;
	}
      else
	{
	  np->width = 4;
	  np->uorder = 4;
	}
    }
  else
    {
      np->width = 2;
      np->uorder = 2;
    }
  np->height = nc->length;
  np->vorder = nc->order;

  np->uknot_type = AY_KTNURB;
  np->vknot_type = nc->knot_type;

  if(!(np->vknotv = malloc((nc->length+nc->order) * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(np->uknotv = malloc((np->width+np->uorder) * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(np->controlv = malloc(np->width*np->height*stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  memcpy(np->vknotv, nc->knotv, (nc->length+nc->order)*sizeof(double));
  if(mode)
    {
      if(mode == 1)
	memcpy(np->uknotv, knotv1, (np->width+np->uorder)*sizeof(double));
      else
	memcpy(np->uknotv, knotv2, (np->width+np->uorder)*sizeof(double));
    }
  else
    {
      memcpy(np->uknotv, knotv0, (np->width+np->uorder)*sizeof(double));
    }

  memcpy(np->controlv, nc->controlv, nc->length*stride*sizeof(double));

  if(mp)
    {
      memcpy(m, mp, 3*sizeof(double));
    }
  else
    {
      if(mode == 0)
	ay_status = ay_geom_extractmiddlepoint(/*mode=*/0, nc->controlv,
					       nc->length, stride, NULL, m);
      else
	ay_status = ay_geom_extractmiddlepoint(/*mode=*/1, nc->controlv,
					       nc->length, stride, &circcv, m);

      if(ay_status)
	goto cleanup;
    }

  if(mode)
    {
      /* 3D mode */
      if(!(circcv = malloc(nc->length * stride * sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      a = 0;
      numuniq = 0;
      for(i = 0; i < nc->length; i++)
	{
	  p = &(nc->controlv[a]);
	  if(!AY_V3COMP(m,p))
	    {
	      n[0] = p[0]-m[0];
	      n[1] = p[1]-m[1];
	      n[2] = p[2]-m[2];
	      len = AY_V3LEN(n);
	      r += len;
	      b++;
	    }
	  a += stride;
	}

      r /= b;
      r *= frac;

      /* create middle curve */
      if(nc->type == AY_CTPERIODIC)
	{
	  /* periodic curve */
	  ay_status = ay_nct_crtcircbspcv(/*sections=*/nc->length-(nc->order-1),
					 /*radius=*/r, /*arc=*/360.0, nc->order,
					 &circcv);
	  if(ay_status)
	    goto cleanup;
	}
      else
	{
	  /* simple closed curve */

	  numuniq = 0;
	  q1 = nc->controlv;
	  q2 = q1+stride;
	  for(i = 0; i < nc->length-1; i++)
	    {
	      if(!AY_V3COMP(q1,q2))
		numuniq++;
	      q1 = q2;
	      q2 += stride;
	    }

	  ay_trafo_identitymatrix(rm);
	  p = circcv;
	  p[0] = r;
	  p[1] = 0.0;
	  p[2] = 0.0;
	  p[3] = 1.0;
	  p = &(circcv[stride]);
	  angle = 360.0/numuniq;
	  q1 = &(nc->controlv[stride]);
	  q1 = nc->controlv;
	  q2 = q1+stride;
	  for(i = 1; i < nc->length-1; i++)
	    {
	      p[0] = r;
	      p[1] = 0.0;
	      p[2] = 0.0;
	      p[3] = 1.0;
	      if(!AY_V3COMP(q1,q2))
		{
		  ay_trafo_rotatematrix(angle, 0.0, 0.0, 1.0, rm);
		}
	      ay_trafo_apply3(p, rm);
	      p += stride;
	      q1 = q2;
	      q2 += stride;
	    } /* for */
	  p[0] = r;
	  p[1] = 0.0;
	  p[2] = 0.0;
	  p[3] = 1.0;
	} /* if */

      /* rotate middle curve in its plane */
      (void)ay_object_copy(c, &rc);
      if(!rc)
	goto cleanup;

      (void)ay_nct_toxy(/*allow_flip=*/AY_FALSE, rc);

      rnc = (ay_nurbcurve_object *)(rc->refine);

      if(fabs(rnc->controlv[0]) > AY_EPSILON || fabs(circcv[0]) > AY_EPSILON)
	{
	  r = sqrt(rnc->controlv[0]*rnc->controlv[0] +
		   rnc->controlv[1]*rnc->controlv[1]);
	  if(r > AY_EPSILON)
	    {
	      angle = AY_R2D(acos(rnc->controlv[0]/r));
	      if(rnc->controlv[1] < 0.0)
		angle = 360.0 - angle;
	    }
	  else
	    angle = 0;

	  r = sqrt(circcv[0]*circcv[0]+circcv[1]*circcv[1]);
	  if(r > AY_EPSILON)
	    {
	      anglem = AY_R2D(acos(circcv[0]/r));
	      if(circcv[1] < 0.0)
		anglem = 360.0 - anglem;
	    }
	  else
	    anglem = 0;

	  ay_trafo_identitymatrix(rm);
	  /*printf("angle:%lg\n",angle-anglem);*/
	  ay_trafo_rotatematrix(angle-anglem, 0, 0, 1, rm);

	  a = 0;
	  for(i = 0; i < nc->length; i++)
	    {
	      ay_trafo_apply3(&(circcv[a]), rm);
	      a += stride;
	    }
	} /* if */

      (void)ay_object_delete(rc);

      /* place and orient middle curve wrt. middle point and boundary*/
      ay_trafo_identitymatrix(rm);

      ay_trafo_translatematrix(m[0], m[1], m[2], rm);

      if(nc->type == AY_CTPERIODIC)
	ay_status = ay_geom_extractmeannormal(nc->controlv,
					    (nc->length-(nc->order-2)), stride,
					    m, n);
      else
	ay_status = ay_geom_extractmeannormal(nc->controlv,
					      nc->length-1, stride,
					      m, n);

      if(ay_status)
	goto cleanup;

      angle = AY_R2D(acos(AY_V3DOT(n, z)));

      if((fabs(angle) > AY_EPSILON) /*&& (fabs(angle - 180.0) > AY_EPSILON)*/)
	{
	  AY_V3CROSS(rotaxis, n, z);
	  r = AY_V3LEN(rotaxis);
	  AY_V3SCAL(rotaxis, (1.0/r));
	  ay_trafo_rotatematrix(-angle, rotaxis[0], rotaxis[1], rotaxis[2], rm);
	}

      a = 0;
      for(i = 0; i < nc->length; i++)
	{
	  ay_trafo_apply3(&(circcv[a]), rm);
	  a += stride;
	}

      /* shift middle curve */
      if(0)
	{
	  shiftlen = nc->length;
	  if(nc->type == AY_CTPERIODIC)
	    shiftlen -= (nc->order-1);
	  if(nc->type == AY_CTCLOSED)
	    shiftlen--;

	  ay_nct_shifttominmeandist(shiftlen, 4, nc->controlv, circcv);

	  if(nc->type == AY_CTPERIODIC)
	    {
	      memcpy(&(circcv[(nc->length-nc->order+1)*stride]), circcv,
		     (nc->order-1)*stride*sizeof(double));
	    }
	  else
	    {
	      memcpy(&(circcv[(nc->length-1)*stride]), circcv,
		     stride*sizeof(double));
	    }
	}
      else
	{
	  ay_nct_rotatetominmeandist(nc->length, 4, nc->controlv, circcv);
	}

      memcpy(&(np->controlv[nc->length*mode*stride]), circcv,
	     nc->length*stride*sizeof(double));

      if(mode == 2)
	ay_interpol_1DA4D(0.5, nc->length, np->controlv,
			  &(np->controlv[nc->length*2*stride]),
			  &(np->controlv[nc->length*stride]));

      /* set middle point */
      if(mode == 1)
	a = (nc->length*2)*stride;
      else
	a = (nc->length*3)*stride;

      for(i = 0; i < nc->length; i++)
	{
	  memcpy(&(np->controlv[a]), m, 3*sizeof(double));
	  np->controlv[a+3] = 1.0;/*nc->controlv[b+3];*/
	  a += stride;
	}
    }
  else
    {
      /* 2D mode */
      a = nc->length*stride;
      for(i = 0; i < nc->length; i++)
	{
	  memcpy(&(np->controlv[a]), m, 3*sizeof(double));
	  np->controlv[a+3] = 1.0;/*nc->controlv[b+3];*/
	  a += stride;
	}
    } /* if mode */

  np->is_rat = nc->is_rat;

  ay_npt_setuvtypes(np, 0);

  /* return result */
  *cap = npatch;

  /* prevent cleanup code from doing something harmful */
  npatch = NULL;

cleanup:

  if(circcv)
    free(circcv);

  if(npatch)
    {
      if(npatch->refine)
	ay_npt_destroy(npatch->refine);

      (void)ay_object_delete(npatch);
    }

 return ay_status;
} /* ay_capt_crtsimplecap */


/** ay_capt_integrate:
 * Integrate a cap into the progenitor NURBS surface by
 * surface concatenation.
 *
 * \param[in,out] c cap object
 * \param[in] side integration place in surface \a s
 * \param[in] knottype knot type for concatenation
 * \param[in,out] s NURBS surface object for integration
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_capt_integrate(ay_object *c, int side, int knottype, ay_object *s)
{
 int ay_status = AY_OK;
 ay_object *cc = NULL, *o = NULL, *oldnext;
 ay_nurbpatch_object *npc = NULL;
 ay_nurbpatch_object *nps = NULL;
 char *uv = NULL, uvs[][4] = {"Vu","vu","Uv","uv"};
 int order = 0;

  if(!c || !s)
    return AY_ENULL;

  if(c->type != AY_IDNPATCH || s->type != AY_IDNPATCH)
    return AY_ERROR;

  nps = (ay_nurbpatch_object*)s->refine;
  npc = (ay_nurbpatch_object*)c->refine;

  uv = uvs[side];

  oldnext = s->next;
  cc = s;
  s->next = c;

  switch(side)
    {
    case 0:
    case 1:
      if(nps->vorder > npc->uorder)
	{
	  ay_status = ay_npt_elevateu(npc, nps->vorder-npc->uorder);
	}
      order = nps->vorder;
      break;
    case 2:
    case 3:
      if(nps->uorder > npc->uorder)
	{
	  ay_status = ay_npt_elevateu(npc, nps->uorder-npc->uorder);
	}
      order = nps->uorder;
      break;
    } /* switch */

  if(ay_status)
    goto cleanup;

  if(knottype == AY_KTNURB)
    ay_status = ay_npt_concat(cc, /*type=*/0, order, knottype,
			      /*-1*/0, /*-0.2*/0, AY_TRUE, uv,
			      &o);
  else
    ay_status = ay_npt_concat(cc, /*type=*/0, order, knottype,
			      0, 0.0, AY_TRUE, uv,
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
  (void)ay_object_delete(o);

  s->next = oldnext;

  /* copy transformations/tags? */

cleanup:

 return ay_status;
} /* ay_capt_integrate */


/** ay_capt_crttrimcap:
 * Create a cap surface from planar NURBS curves; the curve objects
 * will be transformed and moved to the new NURBS patch as trim curves.
 *
 * \param[in,out] c NURBS curve object(s); multiple objects may
 *  be provided as list of connected objects; the first curve
 *  must be the outmost curve, all following curves define
 *  holes; the curve orientations will be detected and adjusted
 *  properly
 * \param[in,out] cap new NURBS patch object
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

      if(!ay_nct_isclosed(c->refine))
	{ ay_status = AY_ERROR; goto cleanup; }

      ay_status = ay_nct_toxy(/*allow_flip=*/AY_FALSE, c);
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

	  np->is_planar = AY_TRUE;

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

      ay_nct_getorientation(nc, 4, 1, 0, &angle);

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

      (void)ay_object_delete(npatch);
    }

 return ay_status;
} /* ay_capt_crttrimcap */


/** ay_capt_crtgordoncap:
 * Create a cap surface from a single non-planar NURBS curve
 * by splitting the outline into four pieces, arranging the
 * pieces properly and building a Gordon surface from them
 * (thanks to "the reverse" for inspiration);
 * the curve object will be modified (split)!
 *
 *  XXXX allow parameterisation of split points and desired surface orders
 *
 * \param[in,out] c NURBS curve object
 * \param[in,out] cap new NURBS patch object
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
 double u, hknots[5] = {0.5, 0.495, 0.496, 0.504, 0.5111};

  c1 = c;

  if(c1->type != AY_IDNCURVE)
    return AY_ERROR;

  curve = (ay_nurbcurve_object*)c1->refine;

  if(curve->length < 3)
    return AY_ERROR;

  ay_status = ay_npt_createnpatchobject(&new);
  if(ay_status)
    goto cleanup;

  /* split curve in half */
  ay_status = ay_nct_clamp(curve, 0);
  if(ay_status)
    goto cleanup;

  ay_status = ay_knots_rescaletorange(curve->length+curve->order, curve->knotv,
				      0.0, 1.0);
  if(ay_status)
    goto cleanup;

  i = 0;
  while((!c3 || ay_status) && (i < numhknots))
    {
      u = hknots[i];
      ay_knots_match((ay_nurbcurve_object*)c1->refine, AY_EPSILON, &u);
      ay_status = ay_nct_split(c1, u, &c3);
      i++;
    }
  if(!c3 || ay_status)
    goto cleanup;


  /* split first half in first and second quarter */
  ay_status = ay_nct_clamp(curve, 1);
  if(ay_status)
    goto cleanup;
  ay_status = ay_knots_rescaletorange(curve->length+curve->order, curve->knotv,
				      0.0, 1.0);
  if(ay_status)
    goto cleanup;
  i = 0;
  ay_status = AY_OK;
  while((!c2 || ay_status) && (i < numhknots))
    {
      u = hknots[i];
      ay_knots_match((ay_nurbcurve_object*)c1->refine, AY_EPSILON, &u);
      ay_status = ay_nct_split(c1, u, &c2);
      i++;
    }
  if(!c2 || ay_status)
    goto cleanup;


  /* split second half in third and fourth quarter */
  curve = (ay_nurbcurve_object*)c3->refine;
  ay_status = ay_nct_clamp(curve, 2);

  if(ay_status)
    goto cleanup;
  ay_status = ay_knots_rescaletorange(curve->length+curve->order, curve->knotv,
				      0.0, 1.0);
  if(ay_status)
    goto cleanup;

  i = 0;
  ay_status = AY_OK;
  while((!c4 || ay_status) && (i < numhknots))
    {
      u = hknots[i];
      ay_knots_match((ay_nurbcurve_object*)c3->refine, AY_EPSILON, &u);
      ay_status = ay_nct_split(c3, u, &c4);
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
    (void)ay_object_delete(new);

  /* c1 deleted outside... */
  if(c2)
    (void)ay_object_delete(c2);
  if(c3)
    (void)ay_object_delete(c3);
  if(c4)
    (void)ay_object_delete(c4);

 return ay_status;
} /* ay_capt_crtgordoncap */


/** ay_capt_parsetags:
 * Parse all "CP" tags into a ay_cparam (cap parameter) struct.
 *
 * \param[in] tag list of tags to parse
 * \param[in,out] params pointer to cap parameter struct
 */
void
ay_capt_parsetags(ay_tag *tag, ay_cparam *params)
{
 int where, type, integrate;
 double frac;
 double v[3] = {0};

  if(!params)
    return;

  while(tag)
    {
      if(tag->type == ay_cp_tagtype)
	{
	  if(tag->val)
	    {
	      params->has_caps = AY_TRUE;
	      where = -1;
	      type = 0;
	      frac = 0.5;
	      integrate = 0;
	      sscanf(tag->val, "%d,%d,%d,%lg", &where, &type, &integrate,
		     &frac);
	      if(where >= 0 && where < 4)
		{
		  params->states[where] = 1;
		  params->types[where] = type;
		  params->integrate[where] = integrate;
		  params->frac[where] = frac;
		}
	    } /* if val */
	} /* if */
      if(tag->type == ay_mp_tagtype)
	{
	  if(tag->val)
	    {
	      where = -1;
	      sscanf(tag->val, "%lg,%lg,%lg,%d",
		     &(v[0]), &(v[1]), &(v[2]), &where);
	      if(where > 0 && where < 4)
		{
		  params->use_mp[where] = 1;
		  memcpy(&(params->mp[where*3]), v, 3*sizeof(double));
		}
	    }
	} /* if */
      tag = tag->next;
    } /* while */

 return;
} /* ay_capt_parsetags */


/** ay_capt_createtags:
 * Create all "CP" tags from old (pre 1.21) style cap type array.
 * No attempt is made to check for already existing tags! The only
 * safe usage scenario is therefore to call this with objects just
 * read from a pre 1.21 scene file.
 *
 * \param[in,out] o object to add the tags to
 * \param[in] caps cap types
 */
void
ay_capt_createtags(ay_object *o, int *caps)
{
 ay_tag *new = NULL;
 int i, l;
 char buf[3*TCL_INTEGER_SPACE+TCL_DOUBLE_SPACE+4];

  if(!o || !caps)
    return;

  for(i = 0; i < 4; i++)
    {
      if(caps[i])
	{
	  l = sprintf(buf, "%d,%d,%d,%g", i, caps[i]-1, 0, 0.5);

	  if(!(new = calloc(1, sizeof(ay_tag))))
	    return;
	  if(!(new->name = calloc(3, sizeof(char))))
	    { free(new); return; }
	  strcpy(new->name, ay_cp_tagname);
	  new->type = ay_cp_tagtype;
	  if(!(new->val = calloc(l+1, sizeof(char))))
	    { free(new->name); free(new); return; }
	  strcpy(new->val,buf);
	  new->next = o->tags;
	  o->tags = new;
	} /* if */
    } /* for */

 return;
} /* ay_capt_createtags */
