/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2012 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/** \file bevelt.c \brief bevel creation tools */

/* prototypes of functions local to this module: */

int ay_bevelt_integrate(int side, ay_object *s, ay_object *b);

void ay_bevelt_createbevelcurve(int index);

/* global variables: */

/** standard bevel curves */
static ay_object *ay_bevelt_curves[3] = {0};

/* functions: */

/** ay_bevelt_addbevels:
 * Add bevel surfaces to the sides of a NURBS surface, also
 * creates the caps on the bevels.
 *
 * \param[in] bparams bevel creation parameters
 * \param[in] cparams cap creation parameters
 * \param[in,out] o NURBS surface object (may be modified
 *  if the integrate parameter of a bevel is set)
 * \param[in,out] dst resulting bevel and cap NURBS surface objects
 *  (may stay empty if all bevels are integrated into the surface)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_bevelt_addbevels(ay_bparam *bparams, ay_cparam *cparams, ay_object *o,
		    ay_object **dst)
{
 int ay_status = AY_OK;
 int i, is_planar, is_roundtocap, do_integrate;
 int winding = 0, side = 0, revert = AY_FALSE;
 int capintknottype = AY_KTCUSTOM;
 double param = 0.0, *mp = NULL, *normals = NULL, *tangents = NULL;
 ay_object curve = {0}, *alignedcurve = NULL;
 ay_object *bevel = NULL, *bevelcurve = NULL;
 ay_object **next = dst, *extrcurve = NULL;
 ay_object *allcaps = NULL, **nextcap = &allcaps;
 ay_nurbpatch_object *np = NULL;

  if(!bparams || !o || !dst)
    return AY_ENULL;

  if(o->type != AY_IDNPATCH)
    return AY_ERROR;

  np = (ay_nurbpatch_object*)o->refine;

  for(i = 0; i < 4; i++)
    {
      if(bparams->states[i])
	{
	  is_planar = AY_TRUE;
	  is_roundtocap = AY_FALSE;
	  do_integrate = AY_FALSE;
	  ay_object_defaults(&curve);
	  ay_trafo_defaults(&curve);
	  curve.type = AY_IDNCURVE;
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

	  normals = NULL;
	  ay_status = ay_npt_extractnc(o, side, param,
				       /*relative=*/AY_TRUE,
				       /*apply_trafo=*/AY_FALSE,
				       /*extractnt=*/2, &normals,
			       (ay_nurbcurve_object**)(void*)&(curve.refine));

	  if(ay_status || !curve.refine)
	    goto cleanup;

	  if(ay_nct_isdegen((ay_nurbcurve_object*)(void*)curve.refine))
	    {
	      ay_nct_destroy(curve.refine);
	      if(normals)
		free(normals);
	      continue;
	    }

	  winding = ay_nct_getwinding(curve.refine, normals, 9);

	  bevel = NULL;
	  ay_status = ay_npt_createnpatchobject(&bevel);
	  if(ay_status || !bevel)
	    goto cleanup;

	  bevelcurve = NULL;
	  if(bparams->types[i] < 3)
	    {
	      ay_status = ay_bevelt_findbevelcurve(-bparams->types[i],
						   &bevelcurve);

	      if(ay_status || !bevelcurve)
		goto cleanup;

	      alignedcurve = NULL;
	      if(bparams->force3d[i])
		is_planar = AY_FALSE;
	      else
		ay_nct_isplanar(&curve, &alignedcurve, &is_planar);

	    }
	  else
	    {
	      is_planar = AY_FALSE;
	      is_roundtocap = AY_TRUE;
	    }

	  if(bparams->integrate[i])
	    {
	      do_integrate = AY_TRUE;
	    }

	  if(do_integrate && is_roundtocap)
	    {
	      if(bparams->force3d[i])
		{
		  do_integrate = AY_FALSE;
		}
	      else
		{
		  ay_nct_isplanar(&curve, NULL, &do_integrate);
		  if(do_integrate)
		    do_integrate = AY_FALSE;
		  else
		    do_integrate = AY_TRUE;
		}
	    }

	  if(is_planar)
	    {
	      /* 2D bevel */
	      switch(i)
		{
		case 0:
		  if(winding > 0)
		    {
		      revert = !bparams->dirs[i];
		      bparams->radii[i] = -bparams->radii[i];
		    }
		  else
		    {
		      revert = bparams->dirs[i];
		      /*bparams->radii[i] = bparams->radii[i];*/
		    }
		  break;
		case 1:
		  if(winding > 0)
		    {
		      revert = bparams->dirs[i];
		      /*bparams->radii[i] = bparams->radii[i];*/
		    }
		  else
		    {
		      revert = !bparams->dirs[i];
		      bparams->radii[i] = -bparams->radii[i];
		    }
		  break;
		case 2:
		  if(winding > 0)
		    {
		      revert = bparams->dirs[i];
		      /*bparams->radii[i] = bparams->radii[i];*/
		    }
		  else
		    {
		      revert = !bparams->dirs[i];
		      bparams->radii[i] = -bparams->radii[i];
		    }
		  break;
		case 3:
		  if(winding > 0)
		    {
		      revert = !bparams->dirs[i];
		      bparams->radii[i] = -bparams->radii[i];
		    }
		  else
		    {
		      revert = bparams->dirs[i];
		      /*bparams->radii[i] = bparams->radii[i];*/
		    }
		  break;
		default:
		  revert = AY_FALSE;
		  break;
		} /* switch */

	      if(revert)
		{
		  ay_nct_revert((ay_nurbcurve_object *)alignedcurve->refine);
		}

	      ay_status = ay_bevelt_createc(bparams->radii[i],
					    alignedcurve, bevelcurve,
			     (ay_nurbpatch_object**)(void*)&(bevel->refine));

	      /* rotate bevel created from aligned curve
		 back to the original curve orientation */
	      if(!ay_status && bevel->refine)
		{
		  ay_trafo_copy(alignedcurve, bevel);
		  ay_npt_applytrafo(bevel);
		}
	    }
	  else
	    {
	      /* 3D bevel */

	      /* pick the correct tangents */
	      if(is_roundtocap)
		{
		  if(i < 2)
		    tangents = &(normals[6]);
		  else
		    tangents = &(normals[3]);
		}
	      else
		{
		  if(i < 2)
		    tangents = &(normals[3]);
		  else
		    tangents = &(normals[6]);
		}

	      switch(i)
		{
		case 0:
		  if(winding > 0)
		    {
		      bparams->dirs[i] = !bparams->dirs[i];
		      bparams->radii[i] = -bparams->radii[i];
		    }
		  /*
		  else
		    {
		      bparams->dirs[i] = bparams->dirs[i];
		      bparams->radii[i] = bparams->radii[i];
		    }
		  */
		  break;
		case 1:
		  if(winding > 0)
		    {
		      /*bparams->dirs[i] = bparams->dirs[i];*/
		      bparams->radii[i] = -bparams->radii[i];
		    }
		  else
		    {
		      bparams->dirs[i] = !bparams->dirs[i];
		      /*bparams->radii[i] = bparams->radii[i];*/
		    }
		  break;
		case 2:
		  if(winding > 0)
		    {
		    /*bparams->dirs[i] = bparams->dirs[i];*/
		      bparams->radii[i] = -bparams->radii[i];
		    }
		  else
		    {
		      bparams->dirs[i] = !bparams->dirs[i];
		      /*bparams->radii[i] = bparams->radii[i];*/
		    }
		  break;
		case 3:
		  if(winding > 0)
		    {
		      bparams->dirs[i] = !bparams->dirs[i];
		      bparams->radii[i] = -bparams->radii[i];
		    }
		  /*
		  else
		    {
		      bparams->dirs[i] = bparams->dirs[i];
		      bparams->radii[i] = bparams->radii[i];
		    }
		  */
		  break;
		default:
		  break;
		} /* switch */

	      if(is_roundtocap)
		{
		  ay_status = ay_bevelt_createroundtocap(bparams->radii[i],
							 bparams->dirs[i],
							 &curve, tangents, 9,
			     (ay_nurbpatch_object**)(void*)&(bevel->refine));
		}
	      else
		{
		  ay_status = ay_bevelt_createc3d(bparams->radii[i],
						  bparams->dirs[i],
						  &curve, bevelcurve,
						  normals, 9, tangents, 9,
			     (ay_nurbpatch_object**)(void*)&(bevel->refine));
		}
	    } /* if 2D or 3D */

	  ay_nct_destroy((ay_nurbcurve_object*)curve.refine);
	  curve.refine = NULL;

	  if(alignedcurve)
	    (void)ay_object_delete(alignedcurve);
	  alignedcurve = NULL;

	  if(normals)
	    free(normals);
	  normals = NULL;

	  if(ay_status || !bevel->refine)
	    {
	      (void)ay_object_delete(bevel);
	      goto cleanup;
	    }

	  if(do_integrate)
	    {
	      ay_status = ay_bevelt_integrate(i, o, bevel);

	      (void)ay_object_delete(bevel);

	      if(ay_status)
		goto cleanup;
	    }
	  else
	    {
	      if(!is_roundtocap || !cparams->states[i])
		{
		  *next = bevel;
		  next = &(bevel->next);
		}
	    }

	  /* create cap from bevel */
	  if(cparams->states[i])
	    {
	      if(!(extrcurve = calloc(1, sizeof(ay_object))))
		{
		  ay_status = AY_EOMEM;
		  goto cleanup;
		}
	      ay_object_defaults(extrcurve);
	      extrcurve->type = AY_IDNCURVE;
	      if(bparams->integrate[i])
		{
		  ay_status = ay_npt_extractnc(o, side, param,
					       AY_FALSE, AY_FALSE,
					       AY_FALSE, NULL,
			 (ay_nurbcurve_object**)(void*)&(extrcurve->refine));
		}
	      else
		{
		  ay_status = ay_npt_extractnc(bevel, 3, 0.0,
					       AY_FALSE, AY_FALSE,
					       AY_FALSE, NULL,
			 (ay_nurbcurve_object**)(void*)&(extrcurve->refine));
		}
	      if(ay_status)
		goto cleanup;

	      if(ay_nct_isdegen(
			    (ay_nurbcurve_object*)(void*)extrcurve->refine))
		{
		  (void)ay_object_delete(extrcurve);
		  continue;
		}

	      switch(cparams->types[i])
		{
		case 0:
		  ay_status = ay_capt_crttrimcap(extrcurve, nextcap);
		  break;
		case 1:
		  ay_status = ay_capt_crtgordoncap(extrcurve, nextcap);
		  break;
		case 2:
		  if(cparams->use_mp[i])
		    mp = &(cparams->mp[i*3]);
		  else
		    mp = NULL;
		  ay_status = ay_capt_crtsimplecap(extrcurve, 0,
					     cparams->frac[i], mp, nextcap);
		  if(!ay_status)
		    ay_status = ay_npt_swapuv((*nextcap)->refine);
		  break;
		case 3:
		  if(cparams->use_mp[i])
		    mp = &(cparams->mp[i*3]);
		  else
		    mp = NULL;
		  ay_status = ay_capt_crtsimplecap(extrcurve, 1,
					     cparams->frac[i], mp, nextcap);
		  if(!ay_status)
		    ay_status = ay_npt_swapuv((*nextcap)->refine);
		  break;
		default:
		  ay_status = AY_ERROR;
		} /* switch */

	      if(cparams->types[i] != 0)
		{
		  (void) ay_object_delete(extrcurve);
		}

	      if(ay_status)
		goto cleanup;

	      /* in round to cap mode on 3D curves, create the bevel via
		 concatenation of surface and cap (but only if we have
		 a cap) */
	      if(is_roundtocap && !do_integrate && *nextcap)
		{
		  (void) ay_object_delete(bevel);
		  bevel = NULL;

		  ay_status = ay_bevelt_createconcat(i, o, *nextcap, &bevel);
		  if(ay_status)
		    goto cleanup;

		  if(bparams->integrate[i])
		    {
		      ay_status = ay_bevelt_integrate(i, o, bevel);

		      (void) ay_object_delete(bevel);

		      if(ay_status)
			goto cleanup;
		    }
		  else
		    {
		      *next = bevel;
		      next = &(bevel->next);
		    }
		}
	      else
		{
		  if(is_roundtocap && !do_integrate)
		    {
		      *next = bevel;
		      next = &(bevel->next);
		    }
		}

	      if(cparams->integrate[i])
		{
		  if(is_roundtocap)
		    {
		      capintknottype = AY_KTNURB;
		    }

		  if(bparams->integrate[i])
		    {
		      ay_status = ay_capt_integrate(*nextcap, i,
						    capintknottype, o);
		    }
		  else
		    {
		      ay_status = ay_capt_integrate(*nextcap, 3,
						    capintknottype, bevel);
		    }

		  (void) ay_object_delete(*nextcap);
		  *nextcap = NULL;

		  if(ay_status)
		    goto cleanup;
		}
	      else
		{
		  if(nextcap)
		    nextcap = &((*nextcap)->next);
		}
	    } /* if cap on this side */
	} /* if bevel on this side */
    } /* for all sides */

  *next = allcaps;

cleanup:

  if(normals)
    free(normals);

 return ay_status;
} /* ay_bevelt_addbevels */


/** ay_bevelt_parsetags:
 * Parse all "BP" tags into a ay_bparam (bevel parameter) struct.
 *
 * \param[in] tag list of tags to parse
 * \param[in,out] params pointer to bevel parameter struct
 */
void
ay_bevelt_parsetags(ay_tag *tag, ay_bparam *params)
{
 int where, type, dir, integrate, force3d;
 double radius;

  if(!params)
    return;

  while(tag)
    {
      if(tag->type == ay_bp_tagtype)
	{
	  if(tag->val)
	    {
	      params->has_bevels = AY_TRUE;
	      where = -1;
	      type = 0;
	      radius = 0.1;
	      dir = 0;
	      integrate = 0;
	      force3d = 0;
	      sscanf(tag->val, "%d,%d,%lg,%d,%d,%d", &where, &type,
		     &radius, &dir, &integrate, &force3d);
	      if(where >= 0 && where < 4)
		{
		  params->states[where] = 1;
		  params->types[where] = type;
		  params->radii[where] = radius;
		  params->dirs[where] = dir;
		  params->integrate[where] = integrate;
		  params->force3d[where] = force3d;
		}
	    } /* if */
	} /* if */
      tag = tag->next;
    } /* while */

 return;
} /* ay_bevelt_parsetags */


/** ay_bevelt_createconcat:
 * Create a (3D) bevel by the concatenation fillet creation algorithm.
 *
 * \param[in] side boundary on which to create the bevel
 * \param[in] o surface to be beveled
 * \param[in] c cap surface
 * \param[in,out] bevel resulting bevel object
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_bevelt_createconcat(int side, ay_object *o, ay_object *c, ay_object **bevel)
{
 int ay_status = AY_OK;
 char *uv = NULL, uvs[][4] = {"Vu","vu","Uu","uu"};

  if(!o || !c || !bevel)
    return AY_ENULL;

  uv = uvs[side];
  ay_status = ay_npt_fillgap(o, c, -0.33, uv, bevel);

 return ay_status;
} /* ay_bevelt_createconcat */


/** ay_bevelt_createc:
 * Create a (2D) bevel.
 *
 * \param[in] radius width/height of the bevel (may be negative)
 * \param[in] o1 curve on which the bevel is constructed (usually
 *  a border extracted from a surface), should be planar and defined
 *  in the XY plane
 * \param[in] o2 bevel cross section curve (should be planar in XY
 *  and run from 0,0 to 1,1)
 * \param[in,out] bevel resulting bevel object
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_bevelt_createc(double radius, ay_object *o1, ay_object *o2,
		  ay_nurbpatch_object **bevel)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *curve = NULL;
 ay_nurbcurve_object *offcurve = NULL;
 ay_nurbcurve_object *bcurve = NULL;
 double *uknotv = NULL, *vknotv = NULL, *controlv = NULL, *tccontrolv = NULL;
 int stride = 4, i = 0, j = 0, a = 0, b = 0, c = 0;

  if(!o1 || !o2 || !bevel)
    return AY_ENULL;

  if(o1->type != AY_IDNCURVE)
    return AY_ERROR;

  if(o2->type != AY_IDNCURVE)
    return AY_ERROR;

  curve = (ay_nurbcurve_object *)o1->refine;

  bcurve = (ay_nurbcurve_object *)o2->refine;

  if(!(controlv = calloc(bcurve->length*curve->length*stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* copy custom knots */
  if(bcurve->knot_type == AY_KTCUSTOM)
    {
      if(!(uknotv = malloc((bcurve->length+bcurve->order) * sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      memcpy(uknotv, bcurve->knotv,
	     (bcurve->length+bcurve->order)*sizeof(double));
    }
  if(curve->knot_type == AY_KTCUSTOM)
    {
      if(!(vknotv = malloc((curve->length+curve->order) * sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      memcpy(vknotv, curve->knotv,
	     (curve->length+curve->order)*sizeof(double));
    }

  /* fill controlv */
  /* first loop */
  memcpy(controlv, curve->controlv, curve->length*stride*sizeof(double));

  /* other loops */
  a = curve->length*stride;
  c = stride;
  for(i = 1; i < bcurve->length; i++)
    {
      ay_status = ay_nct_offset(o1, 1, -radius*bcurve->controlv[c],
				&offcurve);
      if(ay_status)
	{ goto cleanup; }

      b = 0;
      for(j = 0; j < curve->length; j++)
	{
	  controlv[a]   = offcurve->controlv[b];
	  controlv[a+1] = offcurve->controlv[b+1];
	  controlv[a+2] = curve->controlv[b+2]+radius*bcurve->controlv[c+1];
	  controlv[a+3] = curve->controlv[b+3]*bcurve->controlv[c+3];
	  a += stride;
	  b += stride;
	} /* for */

      ay_nct_destroy(offcurve);
      offcurve = NULL;

      c += stride;
    } /* for */

  ay_status = ay_npt_create(bcurve->order, curve->order,
			    bcurve->length, curve->length,
			    bcurve->knot_type, curve->knot_type,
			    controlv, uknotv, vknotv,
			    bevel);

  if(ay_status)
    goto cleanup;

  if(curve->type == AY_CTCLOSED)
    {
      (void)ay_npt_closev(*bevel, /*mode=*/2);
    }

  if(curve->type == AY_CTPERIODIC)
    {
      (void)ay_npt_closev(*bevel, /*mode=*/5);
    }

  /* prevent cleanup code from doing something harmful */
  controlv = NULL;
  uknotv = NULL;
  vknotv = NULL;

cleanup:

  /* clean-up */
  if(controlv)
    free(controlv);
  if(uknotv)
    free(uknotv);
  if(vknotv)
    free(vknotv);
  if(tccontrolv)
    free(tccontrolv);
  if(offcurve)
    ay_nct_destroy(offcurve);

 return ay_status;
} /* ay_bevelt_createc */


/** ay_bevelt_createc3d:
 * Create a 3D bevel.
 *
 * \param[in] radius width/height of the bevel (may be negative)
 * \param[in] revert direction of bevel (0 - inwards, 1 - outwards)
 * \param[in] o1 curve on which the bevel is constructed (usually
 *  a border extracted from a surface)
 * \param[in] o2 bevel cross section curve (should be planar in XY
 *  and run from 0,0 to 1,1)
 * \param[in] n array of normals
 * \param[in] nstride stride in normals array
 * \param[in] t array of tangents, may be NULL
 * \param[in] tstride stride in tangents array
 * \param[in,out] bevel resulting bevel object
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_bevelt_createc3d(double radius, int revert, ay_object *o1, ay_object *o2,
		    double *n, int nstride,
		    double *t, int tstride,
		    ay_nurbpatch_object **bevel)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *curve = NULL;
 ay_nurbcurve_object *bcurve = NULL;
 double *uknotv = NULL, *vknotv = NULL, *controlv = NULL;
 int stride = 4, i = 0, j = 0, a = 0, b = 0, c = 0;
 double angle, len, v1[3] = {0}, v2[2] = {0}, m[16] = {0};
 double *tt = NULL;

  if(!o1 || !o2 || !n || !bevel)
    return AY_ENULL;

  if(o1->type != AY_IDNCURVE)
    return AY_ERROR;

  if(o2->type != AY_IDNCURVE)
    return AY_ERROR;

  curve = (ay_nurbcurve_object *)o1->refine;

  bcurve = (ay_nurbcurve_object *)o2->refine;

  if(!t)
    {
      ay_status = ay_nct_getcvtangents(curve, &tt);
      if(ay_status)
	goto cleanup;
      t = tt;
      tstride = 3;
    }

  if(!(controlv = calloc(bcurve->length*curve->length*stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* copy custom knots */
  if(bcurve->knot_type == AY_KTCUSTOM)
    {
      if(!(uknotv = malloc((bcurve->length+bcurve->order) * sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      memcpy(uknotv, bcurve->knotv,
	     (bcurve->length+bcurve->order)*sizeof(double));
    }
  if(curve->knot_type == AY_KTCUSTOM)
    {
      if(!(vknotv = malloc((curve->length+curve->order) * sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      memcpy(vknotv, curve->knotv,
	     (curve->length+curve->order)*sizeof(double));
    }

  /* fill controlv */
  a = 0;
  c = 0;
  for(i = 0; i < bcurve->length; i++)
    {
      b = 0;
      for(j = 0; j < curve->length; j++)
	{
	  memcpy(v1, &(n[j*nstride]), 3*sizeof(double));
	  /* normalize normal vector */
	  len = AY_V3LEN(v1);
	  AY_V3SCAL(v1, 1.0/len);
	  if(fabs(bcurve->controlv[c]) > AY_EPSILON ||
	     fabs(bcurve->controlv[c+1]) > AY_EPSILON)
	    {
	      /* scale normal */
	      len = sqrt(bcurve->controlv[c]*bcurve->controlv[c] +
			 bcurve->controlv[c+1]*bcurve->controlv[c+1]);
	      AY_V3SCAL(v1, len);
	      AY_V3SCAL(v1, radius);

	      /* rotate normal (around tangent) */
	      v2[0] = bcurve->controlv[c];
	      v2[1] = bcurve->controlv[c+1];
	      if((fabs(v2[0]) > AY_EPSILON) || (fabs(v2[1]) > AY_EPSILON))
		{
		  angle = AY_R2D(acos(v2[0]/AY_V2LEN(v2)));
		  if(v2[1] < 0.0)
		    angle = 360.0-angle;

		  ay_trafo_identitymatrix(m);

		  if(!revert)
		    ay_trafo_rotatematrix(angle,
                       -t[j*tstride], -t[j*tstride+1], -t[j*tstride+2], m);
		  else
		    ay_trafo_rotatematrix(angle,
			t[j*tstride], t[j*tstride+1], t[j*tstride+2], m);

		  ay_trafo_apply3(v1, m);
		}
	    }
	  else
	    {
	      memset(v1, 0, 3*sizeof(double));
	    }
	  /* offset along scaled/rotated normal vector */
	  controlv[a]   = curve->controlv[b]   + v1[0];
	  controlv[a+1] = curve->controlv[b+1] + v1[1];
	  controlv[a+2] = curve->controlv[b+2] + v1[2];
	  controlv[a+3] = curve->controlv[b+3]*bcurve->controlv[c+3];
	  a += stride;
	  b += stride;
	} /* for */
      c += stride;
    } /* for */

  ay_status = ay_npt_create(bcurve->order, curve->order,
			    bcurve->length, curve->length,
			    bcurve->knot_type, curve->knot_type,
			    controlv, uknotv, vknotv,
			    bevel);

  if(ay_status)
    goto cleanup;

  if(curve->type == AY_CTCLOSED)
    {
      (void)ay_npt_closev(*bevel, /*mode=*/2);
    }

  if(curve->type == AY_CTPERIODIC)
    {
      (void)ay_npt_closev(*bevel, /*mode=*/5);
    }

  /* prevent cleanup code from doing something harmful */
  controlv = NULL;
  uknotv = NULL;
  vknotv = NULL;

cleanup:

  /* clean-up */
  if(controlv)
    free(controlv);
  if(uknotv)
    free(uknotv);
  if(vknotv)
    free(vknotv);
  if(tt)
    free(tt);

 return ay_status;
} /* ay_bevelt_createc3d */


/** ay_bevelt_createroundtocap:
 * Create a 3D bevel that rounds to a cap.
 *
 * \param[in] radius width/height of the bevel (may be negative)
 * \param[in] revert direction of bevel (0 - inwards, 1 - outwards)
 * \param[in] o1 curve on which the bevel is constructed (usually
 *  a border extracted from a surface)
 * \param[in] t array of tangents
 * \param[in] tstride stride in tangents array
 * \param[in,out] bevel resulting bevel object
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_bevelt_createroundtocap(double radius, int revert,
			   ay_object *o1, double *t, int tstride,
			   ay_nurbpatch_object **bevel)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *curve = NULL;
 double *uknotv = NULL, *vknotv = NULL, *controlv = NULL;
 int stride = 4, i = 0, j = 0, a = 0, b = 0;
 double len, v1[3] = {0}, v2[3] = {0}, *p;
 double minmax[6];

  if(!o1 || !t || !bevel)
    return AY_ENULL;

  if(o1->type != AY_IDNCURVE)
    return AY_ERROR;

  curve = (ay_nurbcurve_object *)o1->refine;

  if(!(controlv = calloc(3*curve->length*stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(curve->knot_type == AY_KTCUSTOM)
    {
      if(!(vknotv = malloc((curve->length+curve->order) * sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      memcpy(vknotv, curve->knotv,
	     (curve->length+curve->order)*sizeof(double));
    }

  memcpy(controlv, curve->controlv, curve->length*stride*sizeof(double));

  a = curve->length*stride;
  b = 0;
  for(j = 0; j < curve->length; j++)
    {
      memcpy(v1, &(t[j*tstride]), 3*sizeof(double));

      /* normalize and (possibly) flip tangent vector */
      len = AY_V3LEN(v1);
      if(len > AY_EPSILON)
	{
	  if(revert)
	    {
	      AY_V3SCAL(v1, -1.0/len);
	    }
	  else
	    {
	      AY_V3SCAL(v1, 1.0/len);
	    }
	}
      else
	{
	  memset(v1, 0, 3*sizeof(double));
	}
      /* offset along scaled tangent vector */
      controlv[a]   = curve->controlv[b]   - v1[0]*radius;
      controlv[a+1] = curve->controlv[b+1] - v1[1]*radius;
      controlv[a+2] = curve->controlv[b+2] - v1[2]*radius;
      controlv[a+3] = curve->controlv[b+3];
      a += stride;
      b += stride;
    } /* for */

  /* calculate middle of offset curve */
  a = curve->length*stride;
  p = &(controlv[a]);
  minmax[0] = DBL_MAX;
  minmax[1] = -DBL_MAX;
  minmax[2] = DBL_MAX;
  minmax[3] = -DBL_MAX;
  minmax[4] = DBL_MAX;
  minmax[5] = -DBL_MAX;

  for(i = 0; i < curve->length; i++)
    {
      if(p[0] < minmax[0])
	minmax[0] = p[0];
      if(p[0] > minmax[1])
	minmax[1] = p[0];

      if(p[1] < minmax[2])
	minmax[2] = p[1];
      if(p[1] > minmax[3])
	minmax[3] = p[1];

      if(p[2] < minmax[4])
	minmax[4] = p[2];
      if(p[2] > minmax[5])
	minmax[5] = p[2];

      p += stride;
    } /* for */

  v1[0] = minmax[0]+((minmax[1]-minmax[0])*0.5);
  v1[1] = minmax[2]+((minmax[3]-minmax[2])*0.5);
  v1[2] = minmax[4]+((minmax[5]-minmax[4])*0.5);

  /* offset a second time, in the direction to the middle point */
  a = curve->length*stride;
  b = a+curve->length*stride;
  p = &(controlv[a]);
  for(j = 0; j < curve->length; j++)
    {
      AY_V3SUB(v2, v1, p);

      /* normalize offset vector */
      len = AY_V3LEN(v2);
      if(len > AY_EPSILON)
	{
	  AY_V3SCAL(v2, 1.0/len);
	}
      else
	{
	  memset(v2, 0, 3*sizeof(double));
	}

      controlv[b]   = controlv[a]   - v2[0]*radius;
      controlv[b+1] = controlv[a+1] - v2[1]*radius;
      controlv[b+2] = controlv[a+2] - v2[2]*radius;
      controlv[b+3] = controlv[a+3];
      p += stride;
      a += stride;
      b += stride;
    } /* for */

  ay_status = ay_npt_create(3, curve->order,
			    3, curve->length,
			    AY_KTNURB, curve->knot_type,
			    controlv, uknotv, vknotv,
			    bevel);

  if(ay_status)
    goto cleanup;

  if(curve->type == AY_CTCLOSED)
    {
      (void)ay_npt_closev(*bevel, /*mode=*/2);
    }

  if(curve->type == AY_CTPERIODIC)
    {
      (void)ay_npt_closev(*bevel, /*mode=*/5);
    }

  /* prevent cleanup code from doing something harmful */
  controlv = NULL;
  uknotv = NULL;
  vknotv = NULL;

cleanup:

  /* clean-up */
  if(controlv)
    free(controlv);
  if(uknotv)
    free(uknotv);
  if(vknotv)
    free(vknotv);

 return ay_status;
} /* ay_bevelt_createroundtocap */


/** ay_bevelt_createroundtonormal:
 * Create a 3D bevel that starts in the direction of the tangents
 * and rounds to the mean normal.
 *
 * \param[in] radius width/height of the bevel (may be negative)
 * \param[in] revert direction of bevel (0 - inwards, 1 - outwards)
 * \param[in] o1 curve on which the bevel is constructed (usually
 *  a border extracted from a surface)
 * \param[in] nt array of normals
 * \param[in] nstride stride in normals array
 * \param[in] ta array of tangents, may be NULL
 * \param[in] tstride stride in tangents array
 * \param[in] mn mean normal, may be NULL
 * \param[in,out] bevel resulting bevel object
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_bevelt_createroundtonormal(double radius, int revert, ay_object *o1,
			      double *nt, int nstride, double *ta, int tstride,
			      double *mn, ay_nurbpatch_object **bevel)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *curve = NULL;
 double *uknotv = NULL, *vknotv = NULL, *controlv = NULL;
 double *ct = NULL, *tt = NULL, *c, *t, *n, m[16];
 double len, v1[3] = {0}, *p;
 int stride = 4, i = 0, j = 0, a = 0, b = 0;

  if(!o1 || !nt || !bevel)
    return AY_ENULL;

  if(o1->type != AY_IDNCURVE)
    return AY_ERROR;

  curve = (ay_nurbcurve_object *)o1->refine;

  if(!(controlv = calloc(3*curve->length*stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(curve->knot_type == AY_KTCUSTOM)
    {
      if(!(vknotv = malloc((curve->length+curve->order) * sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      memcpy(vknotv, curve->knotv,
	     (curve->length+curve->order)*sizeof(double));
    }

  if(!ta)
    {
      ay_status = ay_nct_getcvtangents(curve, &ct);
      if(ay_status || !ct)
	{ ay_status = AY_ERROR; goto cleanup; }

      if(!(tt = malloc(curve->length*3*sizeof(double))))
	{ free(ct); ay_status = AY_EOMEM; goto cleanup; }

      c = ct;
      t = tt;
      n = nt;

      for(i = 0; i < curve->length; i++)
	{
	  len = AY_V3LEN(c);
	  if(len > AY_EPSILON && fabs(len-1.0) > AY_EPSILON)
	    {
	      AY_V3SCAL(c, 1.0/len);
	    }

	  /* project c onto plane spanned by real surface tangents (nt) */
	  ay_trafo_identitymatrix(m);
	  m[0] = 1.0-n[0]*n[0];
	  m[1] = (-n[1])*n[0];
	  m[2] = (-n[2])*n[0];
	  m[4] = (-n[0])*n[1];
	  m[5] = 1.0-n[1]*n[1];
	  m[6] = (-n[2])*n[1];
	  m[8] = (-n[0])*n[2];
	  m[9] = (-n[1])*n[2];
	  m[10] = 1.0-n[2]*n[2];
	  ay_trafo_apply3(c, m);

	  len = AY_V3LEN(c);
	  if(len > AY_EPSILON && fabs(len-1.0) > AY_EPSILON)
	    {
	      AY_V3SCAL(c, 1.0/len);
	    }

	  /* rotate c to a mostly perpendicular direction
	     wrt. the unprojected c while keeping it in the
	     tangent plane of the underlying surface point */
	  ay_trafo_identitymatrix(m);
	  ay_trafo_rotatematrix(90.0, n[0], n[1], n[2], m);
	  ay_trafo_apply3(c, m);

	  /* store result */
	  memcpy(t, c, 3*sizeof(double));

	  c += 3;
	  t += 3;
	  n += nstride;
	} /* for */

      ta = tt;
      tstride = 3;
      free(ct);
    } /* if !t */

  /* first cross section is a copy of the original curve */
  memcpy(controlv, curve->controlv, curve->length*stride*sizeof(double));

  /* offset in the direction of the surface tangent */
  a = curve->length*stride;
  b = 0;
  for(j = 0; j < curve->length; j++)
    {
      memcpy(v1, &(ta[j*tstride]), 3*sizeof(double));

      /* normalize and (possibly) flip tangent vector */
      len = AY_V3LEN(v1);
      if(len > AY_EPSILON)
	{
	  if(revert)
	    {
	      AY_V3SCAL(v1, -1.0/len);
	    }
	  else
	    {
	      AY_V3SCAL(v1, 1.0/len);
	    }
	}
      else
	{
	  memset(v1, 0, 3*sizeof(double));
	}

      /* offset along scaled tangent vector */
      controlv[a]   = curve->controlv[b]   - v1[0]*radius;
      controlv[a+1] = curve->controlv[b+1] - v1[1]*radius;
      controlv[a+2] = curve->controlv[b+2] - v1[2]*radius;
      controlv[a+3] = curve->controlv[b+3];
      a += stride;
      b += stride;
    } /* for */

  /* offset a second time, in the direction of the mean normal */
  if(mn)
    {
      memcpy(v1, mn, 3*sizeof(double));
    }
  else
    {
      if(curve->type == AY_CTPERIODIC)
	ay_status = ay_geom_extractmeannormal(curve->controlv,
					      curve->length-(curve->order/2), 4,
					      NULL, v1);
      else
	ay_status = ay_geom_extractmeannormal(curve->controlv, curve->length, 4,
					      NULL, v1);
      if(ay_status)
	goto cleanup;
    }

  a = curve->length*stride;
  b = a+curve->length*stride;
  p = &(controlv[a]);
  for(j = 0; j < curve->length; j++)
    {
      controlv[b]   = controlv[a]   - v1[0]*radius;
      controlv[b+1] = controlv[a+1] - v1[1]*radius;
      controlv[b+2] = controlv[a+2] - v1[2]*radius;
      controlv[b+3] = controlv[a+3];
      p += stride;
      a += stride;
      b += stride;
    } /* for */

  ay_status = ay_npt_create(3, curve->order,
			    3, curve->length,
			    AY_KTNURB, curve->knot_type,
			    controlv, uknotv, vknotv,
			    bevel);

  if(ay_status)
    goto cleanup;

  if(curve->type == AY_CTCLOSED)
    {
      (void)ay_npt_closev(*bevel, /*mode=*/2);
    }

  if(curve->type == AY_CTPERIODIC)
    {
      (void)ay_npt_closev(*bevel, /*mode=*/5);
    }

  /* prevent cleanup code from doing something harmful */
  controlv = NULL;
  uknotv = NULL;
  vknotv = NULL;

cleanup:

  /* clean-up */
  if(controlv)
    free(controlv);
  if(uknotv)
    free(uknotv);
  if(vknotv)
    free(vknotv);
  if(tt)
    free(tt);

 return ay_status;
} /* ay_bevelt_createroundtonormal */


/** ay_bevelt_integrate:
 * Integrate a bevel into a NURBS surface.
 *
 * \param[in] side integration place
 * \param[in,out] s NURBS surface object for integration
 * \param[in,out] b bevel object
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_bevelt_integrate(int side, ay_object *s, ay_object *b)
{
 int ay_status = AY_OK;
 ay_object *sb = NULL, *o = NULL, *oldnext;
 ay_nurbpatch_object *np = NULL, *bevel = NULL;
 char *uv = NULL, uvs[][4] = {"Vu","vu","Uu","uu"};
 int knottype = AY_KTCUSTOM, order = 0;

  if(!s || !b)
    return AY_ENULL;

  if((s->type != AY_IDNPATCH) || (b->type != AY_IDNPATCH))
    return AY_ERROR;

  np = (ay_nurbpatch_object*)s->refine;
  bevel = (ay_nurbpatch_object*)b->refine;

  uv = uvs[side];

  oldnext = s->next;
  sb = s;
  s->next = b;

  switch(side)
    {
    case 0:
    case 1:
      if(np->vorder > bevel->uorder)
	{
	  ay_status = ay_npt_elevateu(bevel, np->vorder-bevel->uorder);
	  if(ay_status)
	    goto cleanup;
	}
      order = np->vorder;
      break;
    case 2:
    case 3:
      if(np->uorder > bevel->uorder)
	{
	  ay_status = ay_npt_elevateu(bevel, np->uorder-bevel->uorder);
	  if(ay_status)
	    goto cleanup;
	}
      order = np->uorder;
      break;
    } /* switch */

  knottype = AY_KTCUSTOM;

  ay_status = ay_npt_concat(sb, /*type=*/0, order, knottype,
			    /*fillet_type=*/1, /*ftlen=*/0.0,
			    /*compatible=*/AY_TRUE, uv,
			    &o);

  if(ay_status)
    goto cleanup;

  /* correct orientation of concatenated surface */
  if(side < 2)
    (void)ay_npt_swapuv(o->refine);

  if(side == 0)
    (void)ay_npt_revertv(o->refine);

  if(side == 2)
    (void)ay_npt_revertu(o->refine);

  /* replace old patch with new */
  ay_npt_destroy(s->refine);
  s->refine = o->refine;
  o->refine = NULL;
  (void)ay_object_delete(o);

  s->next = oldnext;

  /* copy transformations/tags? */

cleanup:

 return ay_status;
} /* ay_bevelt_integrate */


/** ay_bevelt_findbevelcurve:
 * Find a curve object in the current toplevel level named "Bevels"
 * for use as bevel cross section curve. If the index provided is
 * <= 0, the matching standard bevel curve will be returned instead
 * of a curve from the "Bevels" level.
 *
 * \param[in] index index of bevel curve
 * \param[in,out] c resulting curve
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_bevelt_findbevelcurve(int index, ay_object **c)
{
 int ay_status = AY_ERROR;
 int j;
 ay_object *o;

  if(!c)
    return AY_ENULL;

  if(index <= 0)
    {
      if(ay_bevelt_curves[abs(index)] == NULL)
	{
	  ay_bevelt_createbevelcurve(abs(index));
	}
      *c = ay_bevelt_curves[abs(index)];
      if(!*c)
	ay_status = AY_ERROR;
      else
	ay_status = AY_OK;
    }
  else
    {
      o = ay_root;

      while(o)
	{
	  if(o->type == AY_IDLEVEL && o->name && !strcmp(o->name, "Bevels"))
	    {
	      o = o->down;
	      j = 1;
	      while(o)
		{
		  if(index == j)
		    {
		      *c = o;
		      return AY_OK;
		    }
		  j++;
		  o = o->next;
		} /* while */
	    } /* if level is "Bevels" */
	  if(o)
	    o = o->next;
	} /* while */
    } /* if index */

 return ay_status;
} /* ay_bevelt_findbevelcurve */


/** ay_bevelt_createbevelcurve:
 * Create a standard bevel cross section curve (fills the
 * curve object into the global array \a ay_bevelt_curves).
 *
 * \param[in] index type of bevel curve to create:
 *  0 - round, 1 - linear, 2 - ridge
 */
void
ay_bevelt_createbevelcurve(int index)
{
 int ay_status = AY_OK;
 ay_object *o = NULL;
 ay_nurbcurve_object *nc = NULL;
 int stride = 4;
 double *knotv = NULL, *controlv = NULL;
 double ridgeknots[8] = {0.0, 0.0, 0.0, 0.25, 0.75, 1.0, 1.0, 1.0};

  if(ay_bevelt_curves[index])
    {
      /* curve already exists? */
      return;
    }

  if(!(o = calloc(1, sizeof(ay_object))))
    return;
  ay_object_defaults(o);
  o->type = AY_IDNCURVE;

  switch(index)
    {
    case 0:
      /* round (quarter circle) */
      if(!(controlv = calloc(3*stride, sizeof(double))))
	goto cleanup;
      controlv[3] = 1.0;
      controlv[5] = 1.0;
      controlv[7] = sqrt(2.0)/2.0;
      controlv[8] = 1.0;
      controlv[9] = 1.0;
      controlv[11] = 1.0;
      ay_status = ay_nct_create(3, 3, AY_KTNURB, controlv, NULL, &nc);
      if(ay_status)
	goto cleanup;
      break;
    case 1:
      /* linear */
      if(!(controlv = calloc(2*stride, sizeof(double))))
	goto cleanup;
      controlv[3] = 1.0;
      controlv[4] = 1.0;
      controlv[5] = 1.0;
      controlv[7] = 1.0;
      ay_status = ay_nct_create(2, 2, AY_KTNURB, controlv, NULL, &nc);
      if(ay_status)
	goto cleanup;
      break;
    case 2:
      /* ridge */
      if(!(controlv = calloc(5*stride, sizeof(double))))
	goto cleanup;
      controlv[3] = 1.0;
      controlv[4] = 1.0-0.8535;
      controlv[5] = 0.3535;
      controlv[7] = 0.8535;
      controlv[8] = 0.5;
      controlv[9] = 0.5;
      controlv[11] = 1.1;
      controlv[12] = 1.0-0.3535;
      controlv[13] = 0.8535;
      controlv[15] = 0.8535;
      controlv[16] = 1.0;
      controlv[17] = 1.0;
      controlv[19] = 1.0;

      if(!(knotv = calloc(5+3, sizeof(double))))
	goto cleanup;
      memcpy(knotv, ridgeknots, (5+3)*sizeof(double));
      ay_status = ay_nct_create(3, 5, AY_KTCUSTOM, controlv, knotv, &nc);
      if(ay_status)
	goto cleanup;
      break;
    default:
      break;
    } /* switch */

  if(!nc)
    goto cleanup;

  o->refine = nc;

  ay_bevelt_curves[index] = o;

  /* prevent cleanup code from doing something harmful */
  o = NULL;
  controlv = NULL;
  knotv = NULL;

cleanup:

  if(o)
    (void)ay_object_delete(o);

  if(controlv)
    free(controlv);
  if(knotv)
    free(knotv);

 return;
} /* ay_bevelt_createbevelcurve */
