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

int ay_bevelt_createc3d(double radius, int revert,
			ay_object *o1, ay_object *o2,
			double *n, double *t, ay_nurbpatch_object **bevel);

/* global variables: */

/** standard bevel curves */
static ay_object *ay_bevelt_curves[3] = {0};

/* functions: */

/* ay_bevelt_addbevels:
 *
 */
int
ay_bevelt_addbevels(ay_bparam *bparams, int *caps, ay_object *o,
		    ay_object **dst)
{
 int ay_status = AY_OK;
 int i;
 double *normals = NULL, *tangents = NULL;
 ay_object c = {0};
 ay_object *bevel = NULL, *bevelcurve = NULL;
 ay_object **next = dst, *extrcurve = NULL;
 ay_object *allcaps = NULL, **nextcap = &allcaps;

  if(!bparams || !o || !dst)
    return AY_ENULL;

  for(i = 0; i < 4; i++)
    {
      if(bparams->states[i])
	{
	  ay_object_defaults(&c);
	  ay_trafo_defaults(&c);
	  c.type = AY_IDNCURVE;

	  ay_status = ay_npt_extractnc(o, /*side=*/i,
				       /*param=*/0.0,
				       /*relative=*/AY_FALSE,
				       /*apply_trafo=*/AY_FALSE,
				       /*create_pvn=*/2, &normals,
				   (ay_nurbcurve_object**)(void*)&(c.refine));

	  if(ay_status)
	    goto cleanup;

	  bevel = NULL;
	  ay_status = ay_npt_createnpatchobject(&bevel);
	  if(ay_status)
	    {
	      goto cleanup;
	    }

	  bevelcurve = NULL;

	  if(bparams->types[i] >= 0)
	    {
	      if(bparams->types[i] < 3)
		{
		  if(ay_bevelt_curves[bparams->types[i]] == NULL)
		    {
		      ay_bevelt_createbevelcurve(bparams->types[i]);
		    }
		  bevelcurve = ay_bevelt_curves[bparams->types[i]];
		}
	    }
	  else
	    {
	      bevelcurve = NULL;
	      ay_status = ay_bevelt_findbevelcurve(-bparams->types[i],
						   &bevelcurve);

	      if(ay_status)
		goto cleanup;
	    } /* if */

	  if(i < 2)
	    tangents = &(normals[3]);
	  else
	    tangents =  &(normals[6]);

	  ay_status = ay_bevelt_createc3d(bparams->radii[i], bparams->dirs[i],
					  &c, bevelcurve,
					  normals, tangents,
			     (ay_nurbpatch_object**)(void*)&(bevel->refine));

	  ay_nct_destroy((ay_nurbcurve_object*)c.refine);
	  c.refine = NULL;

	  if(normals)
	    free(normals);
	  normals = NULL;

	  if(ay_status || !bevel->refine)
	    {
	      ay_object_delete(bevel);
	      goto cleanup;
	    }

	  if(bparams->integrate[i])
	    {
	      ay_bevelt_integrate(i, o, bevel);
	      ay_object_delete(bevel);
	    }
	  else
	    {
	      *next = bevel;
	      next = &(bevel->next);
	    }

	  /* create cap from bevel */
	  if(caps[i] && !bparams->integrate[i])
	    {
	      if(!(extrcurve = calloc(1, sizeof(ay_object))))
		{
		  ay_status = AY_EOMEM;
		  goto cleanup;
		}
	      ay_object_defaults(extrcurve);
	      extrcurve->type = AY_IDNCURVE;

	      ay_status = ay_npt_extractnc(bevel, 3, 0.0, AY_FALSE, AY_FALSE,
					   AY_FALSE, NULL,
			 (ay_nurbcurve_object**)(void*)&(extrcurve->refine));

	      if(ay_status)
		goto cleanup;

	      switch(caps[i])
		{
		case 1:
		  ay_status = ay_capt_crttrimcap(extrcurve, nextcap);
		  break;
		case 2:
		  ay_status = ay_capt_crtsimplecap(extrcurve, nextcap);
		  break;
		case 3:
		  ay_status = ay_capt_crtsimplecapint(3, bevel, extrcurve);
		  break;
		case 4:
		  ay_status = ay_capt_crtgordoncap(extrcurve, nextcap);
		  break;
		default:
		  ay_status = AY_ERROR;
		  goto cleanup;
		} /* switch */

	      if(ay_status)
		{
		  ay_object_deletemulti(extrcurve);
		  goto cleanup;
		}

	      if(nextcap)
		nextcap = &((*nextcap)->next);
	    } /* if */
	} /* if */
    } /* for */

  *next = allcaps;

cleanup:

  if(normals)
    free(normals);

 return ay_status;
} /* ay_bevelt_addbevels */


/** ay_bevelt_parsetags:
 * Parse all "BP" tags into a ay_bparam (bevel param) struct.
 *
 * @param[in] tag list of tags to parse
 * @param[in,out] params pointer to bevel param struct
 */
void
ay_bevelt_parsetags(ay_tag *tag, ay_bparam *params)
{
 int where, type, dir, integrate;
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
	      sscanf(tag->val, "%d,%d,%lg,%d,%d", &where, &type,
		     &radius, &dir, &integrate);
	      if(where >= 0 && where < 4)
		{
		  params->states[where] = 1;
		  params->types[where] = type;
		  params->radii[where] = radius;
		  params->dirs[where] = dir;
		  params->integrate[where] = integrate;
		}
	    } /* if */
	} /* if */
      tag = tag->next;
    } /* while */

 return;
} /* ay_bevelt_parsetags */


/* ay_bevelt_create:
 *  create a bevel in <bevel> from a planar closed NURB curve <o>;
 *  direction of curve defines, whether bevel rounds inwards or outwards;
 *  type: 0 - round (quarter circle), 1 - linear, 2 - ridge
 *  radius: radius of the bevel
 *  align: AY_TRUE - curve is not defined in XY plane and needs to be
 *  rotated accordingly first
 */
int
ay_bevelt_create(int type, double radius, int align, ay_object *o,
		 ay_nurbpatch_object **bevel)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *patch = NULL;
 ay_nurbcurve_object *curve = NULL;
 double uknots_round[6] = {0.0, 0.0, 0.0, 1.0, 1.0, 1.0};
 double uknots_linear[4] = {0.0, 0.0, 1.0, 1.0};
 double uknots_ridge[8] = {0.0, 0.0, 0.0, 0.25, 0.75, 1.0, 1.0, 1.0};
 /* vknots are taken from curve! */
 double *uknotv = NULL, *vknotv = NULL, *controlv = NULL, *tccontrolv = NULL;
 double x, y, z, w;
 double tangent[3] = {0}, normal[3] = {0}, zaxis[3] = {0.0, 0.0, -1.0};
 double ww = sqrt(2.0)/2.0, displacex, displacey;
 int i = 0, j = 0, a = 0, b = 0, k = 0;
 double m[16], point[4] = {0};

  if(!o || !bevel)
    return AY_ENULL;

  if(o->type != AY_IDNCURVE)
    return AY_ERROR;

  curve = (ay_nurbcurve_object *)o->refine;

  /* calloc the new patch */
  if(!(patch = calloc(1, sizeof(ay_nurbpatch_object))))
    return AY_EOMEM;
  switch(type)
    {
    case 0:
      {
	if(!(controlv = calloc(4*3*curve->length, sizeof(double))))
	  { free(patch); return AY_EOMEM; }
	if(!(uknotv = calloc(6, sizeof(double))))
	  { free(patch); free(controlv); return AY_EOMEM; }
	memcpy(uknotv, uknots_round, 6*sizeof(double));
	patch->uorder = 3;
	patch->width = 3;
	patch->uknot_type = AY_KTNURB;
	patch->is_rat = AY_TRUE;
      }
      break;
    case 1:
      {
	if(!(controlv = calloc(4*2*curve->length, sizeof(double))))
	  { free(patch); return AY_EOMEM; }
	if(!(uknotv = calloc(4, sizeof(double))))
	  { free(patch); free(controlv); return AY_EOMEM; }
	memcpy(uknotv, uknots_linear, 4*sizeof(double));
	patch->uorder = 2;
	patch->width = 2;
	patch->uknot_type = AY_KTNURB;
	patch->is_rat = curve->is_rat;
      }
      break;
    case 2:
      {
	if(!(controlv = calloc(4*5*curve->length, sizeof(double))))
	  { free(patch); return AY_EOMEM; }
	if(!(uknotv = calloc(8, sizeof(double))))
	  { free(patch); free(controlv); return AY_EOMEM; }
	memcpy(uknotv, uknots_ridge, 8*sizeof(double));
	patch->uorder = 3;
	patch->width = 5;
	patch->uknot_type = AY_KTCUSTOM;
	patch->is_rat = AY_TRUE;
      }
      break;
    default:
      /* XXXX issue proper error message */
      free(patch);
      return AY_ERROR;
    } /* switch */

  if(!(vknotv = calloc(curve->length+curve->order,sizeof(double))))
    { free(patch); free(controlv); free(uknotv); return AY_EOMEM; }
  memcpy(vknotv,curve->knotv,(curve->length+curve->order)*sizeof(double));
  patch->vknotv = vknotv;
  patch->uknotv = uknotv;
  patch->vorder = curve->order;
  patch->vknot_type = curve->knot_type;
  patch->height = curve->length;
  patch->glu_sampling_tolerance = curve->glu_sampling_tolerance;

  /* fill controlv */
  /* first loop */
  if(align)
    {
      if(!(tccontrolv = calloc(curve->length*4, sizeof(double))))
	{
	  free(patch); free(controlv); free(uknotv); free(vknotv);
	  return AY_EOMEM;
	}
      memcpy(tccontrolv, curve->controlv, curve->length*4*sizeof(double));

      /* adjust orientation of curve (next sections only work properly,
	 when curve is planar in XY plane) */
      ay_status = ay_nct_toxy(o);
      if(ay_status)
	{
	  free(patch); free(controlv); free(uknotv); free(vknotv);
	  free(tccontrolv);
	  return ay_status;
	}
      a = 0;
      for(i = 0; i < patch->width; i++)
	{
	  memcpy(&(controlv[a]), curve->controlv,
		 curve->length*4*sizeof(double));
	  a += curve->length*4;
	} /* for */
    }
  else
    {
      /*ay_trafo_creatematrix(o, m);*/

      b = 0;
      for(i = 0; i < patch->width; i++)
	{
	  a = 0;
	  for(j = 0; j < curve->length; j++)
	    {
	      memcpy(&(controlv[b]), &(curve->controlv[a]), 4*sizeof(double));
	      /*ay_trafo_apply3(&(controlv[b]), m);*/
	      controlv[b+2] = 0.0;
	      a += 4;
	      b += 4;
	    } /* for */
	} /* for */
    } /* if */

  b = curve->length*4;
  /* transform second loop */
  if((type == 0) || (type == 3))
    {
      a = 0;
      for(j = 0; j < curve->length; j++)
	{
	  controlv[b] = controlv[a];
	  controlv[b+1] = controlv[a+1];
	  controlv[b+2] = controlv[a+2]+radius;
	  controlv[b+3] = controlv[a+3]*ww;
	  a += 4;
	  b += 4;
	} /* for */
    } /* if */

  if(type == 2)
    {
      /* transform the middle 3 loops */
      for(k = 0; k < 3; k++)
	{
	  if((k == 0) || (k == 2))
	    {
	      ww = 0.8535;
	      if(k == 0)
		{
		  displacex = 0.8535;
		  displacey = 0.3535;
		}
	      else
		{
		  displacex = 0.3535;
		  displacey = 0.8535;
		}
	    }
	  else
	    {
	      ww = 1.1;
	      displacex = 0.5;
	      displacey = 0.5;
	    }

	  for(j = 0; j < curve->length; j++)
	    {
	      /* get displacement direction */
	      ay_npt_gettangentfromcontrol2D(curve->type, curve->length,
					     curve->order-1, 4, controlv, j,
					     tangent);

	      x = controlv[b];
	      y = controlv[b+1];
	      z = controlv[b+2];
	      w = controlv[b+3];

	      AY_V3CROSS(normal, tangent, zaxis)
	      AY_V3SCAL(normal, (radius*(1.0-displacex)))

	      /* create transformation matrix */
	      ay_trafo_identitymatrix(m);
	      ay_trafo_translatematrix(normal[0], normal[1],
				       displacey * radius, m);

	      /* transform point */
	      point[0] = m[0]*x + m[4]*y + m[8]*z + m[12];
	      point[1] = m[1]*x + m[5]*y + m[9]*z + m[13];
	      point[2] = m[2]*x + m[6]*y + m[10]*z + m[14];

	      controlv[b]   = point[0];
	      controlv[b+1] = point[1];
	      controlv[b+2] = point[2];

	      controlv[b+3] = w*ww;

	      b += 4;
	    } /* for */
	} /* for */
    } /* if */

  /* transform last normal loop (before any cap loops) */
  for(j = 0; j < curve->length; j++)
    {
      /* get displacement direction */
      ay_npt_gettangentfromcontrol2D(curve->type, curve->length,
				     curve->order-1, 4, controlv, j, tangent);

      x = controlv[b];
      y = controlv[b+1];
      z = controlv[b+2];
      w = controlv[b+3];

      AY_V3CROSS(normal, tangent, zaxis)
      AY_V3SCAL(normal, radius)

      /* create transformation matrix */
      ay_trafo_identitymatrix(m);
      ay_trafo_translatematrix(normal[0], normal[1], radius, m);

      /* transform point */
      point[0] = m[0]*x + m[4]*y + m[8]*z + m[12]*1.0;
      point[1] = m[1]*x + m[5]*y + m[9]*z + m[13]*1.0;
      point[2] = m[2]*x + m[6]*y + m[10]*z + m[14]*1.0;
      /*point[3] = m[3]*x + m[7]*y + m[11]*z + m[15]*1.0;*/

      controlv[b]   = point[0];
      controlv[b+1] = point[1];
      controlv[b+2] = point[2];
      controlv[b+3] = w;

      b += 4;
    } /* for */

  /* re-process first loop? */
  if(align)
    {
      /* overwrite first loop with original saved curve data (to avoid
         losing precision due to double rotation) */
      memcpy(controlv, tccontrolv, curve->length*4*sizeof(double));
      /* rotate other loops into position */
      ay_trafo_creatematrix(o, m);
      b = curve->length*4;
      for(i = 1; i < patch->width; i++)
	{
	  for(j = 0; j < curve->length; j++)
	    {
	      ay_trafo_apply3(&(controlv[b]), m);
	      b += 4;
	    } /* for */
	} /* for */
    } /* if */

  patch->controlv = controlv;

  *bevel = patch;

  /* clean-up */
  if(tccontrolv)
    free(tccontrolv);

 return ay_status;
} /* ay_bevelt_create */


/* ay_bevelt_createc:
 *  create a bevel in <bevel> from a planar closed NURB curve <o1>;
 *  <o2> defines the cross section of the bevel, it should run from
 *  0,0 to 1,1: bevel rounds inwards, or
 *  0,0 to 0,-1: bevel rounds outwards;
 *  radius: radius of the bevel (-DBL_MAX, DBL_MAX);
 */
int
ay_bevelt_createc(double radius, ay_object *o1, ay_object *o2,
		  ay_nurbpatch_object **bevel)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *curve = NULL;
 ay_nurbcurve_object *offcurve1 = NULL, *offcurve2 = NULL;
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
      if(!(uknotv = calloc(bcurve->length+bcurve->order, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      memcpy(uknotv, bcurve->knotv,
	     (bcurve->length+bcurve->order)*sizeof(double));
    }
  if(curve->knot_type == AY_KTCUSTOM)
    {
      if(!(vknotv = calloc(curve->length+curve->order, sizeof(double))))
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
      if(0)
	{
	  ay_status = ay_nct_offset(o1, 0, -radius*bcurve->controlv[c],
				    &offcurve1);
	  if(ay_status)
	    { goto cleanup; }

	  ay_status = ay_nct_offset(o1, 1, -radius*bcurve->controlv[c],
				    &offcurve2);
	  if(ay_status)
	    { goto cleanup; }

	  b = 0;
	  for(j = 0; j < curve->length; j++)
	    {
	      controlv[a]   =
		(offcurve1->controlv[b]+offcurve2->controlv[b])/2.0;
	      controlv[a+1] =
		(offcurve1->controlv[b+1]+offcurve2->controlv[b+1])/2.0;
	      controlv[a+2] = radius*bcurve->controlv[c+1];
	      controlv[a+3] = curve->controlv[b+3]*bcurve->controlv[c+3];
	      a += stride;
	      b += stride;
	    } /* for */

	  ay_nct_destroy(offcurve1);
	  offcurve1 = NULL;

	  ay_nct_destroy(offcurve2);
	  offcurve2 = NULL;
	}
      else
	{
	  ay_status = ay_nct_offset(o1, 3, -radius,
				    &offcurve1);
	  if(ay_status)
	    { goto cleanup; }

	  b = 0;
	  for(j = 0; j < curve->length; j++)
	    {
	      controlv[a]   = offcurve1->controlv[b];
	      controlv[a+1] = offcurve1->controlv[b+1];
	      controlv[a+2] = offcurve1->controlv[b+2];
	      controlv[a+3] = curve->controlv[b+3]*bcurve->controlv[c+3];
	      a += stride;
	      b += stride;
	    } /* for */

	  ay_nct_destroy(offcurve1);
	  offcurve1 = NULL;

	}
      c += stride;
    } /* for */

  ay_status = ay_npt_create(bcurve->order, curve->order,
			    bcurve->length, curve->length,
			    bcurve->knot_type, curve->knot_type,
			    controlv, uknotv, vknotv,
			    bevel);

  if(ay_status)
    goto cleanup;

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
  if(offcurve1)
    ay_nct_destroy(offcurve1);
  if(offcurve2)
    ay_nct_destroy(offcurve2);

 return ay_status;
} /* ay_bevelt_createc */


/* ay_bevelt_createc3d:
 *  create a 3D bevel in <bevel> from a NURB curve <o1>;
 *  <o2> defines the cross section of the bevel, it should run from
 *   0,0 to 1,1: bevel rounds inwards, or
 *   0,0 to 0,-1: bevel rounds outwards;
 *  radius: radius of the bevel (-DBL_MAX, DBL_MAX);
 */
int
ay_bevelt_createc3d(double radius, int revert, ay_object *o1, ay_object *o2,
		    double *n, double *t, ay_nurbpatch_object **bevel)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *curve = NULL;
 ay_nurbcurve_object *bcurve = NULL;
 double *uknotv = NULL, *vknotv = NULL, *controlv = NULL;
 int stride = 4, i = 0, j = 0, a = 0, b = 0, c = 0;
 double angle, len, v1[3] = {0}, v2[2] = {0}, m[16] = {0};

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
      if(!(uknotv = calloc(bcurve->length+bcurve->order, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      memcpy(uknotv, bcurve->knotv,
	     (bcurve->length+bcurve->order)*sizeof(double));
    }
  if(curve->knot_type == AY_KTCUSTOM)
    {
      if(!(vknotv = calloc(curve->length+curve->order, sizeof(double))))
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
	  memcpy(v1, &(n[j*9]), 3*sizeof(double));
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
					  -t[j*9], -t[j*9+1], -t[j*9+2], m);
		  else
		    ay_trafo_rotatematrix(angle,
					  t[j*9], t[j*9+1], t[j*9+2], m);

		  ay_trafo_apply3(v1, m);
		}
	    }
	  else
	    {
	      memset(v1, 0, 3*sizeof(double));
	    }

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
} /* ay_bevelt_createc3d */


/** ay_bevelt_integrate:
 *  integrate a bevel into a NURBS surface
 *
 * @param[in] side integration place
 * @param[in,out] s NURBS surface object for integration
 * @param[in,out] b bevel object
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

  if(ay_status)
    goto cleanup;

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
      knottype = np->vknot_type;
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
      knottype = np->uknot_type;
      break;
    } /* switch */

  if(ay_status)
    goto cleanup;

  knottype = AY_KTCUSTOM;

  ay_status = ay_npt_concat(sb, 0, order, knottype, 0, 0.0, AY_TRUE, uv,
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

 return ay_status;
} /* ay_bevelt_integrate */


/** ay_bevelt_findbevelcurve:
 * Find a curve object in the current toplevel level named "Bevels"
 * for use as bevel cross section curve.
 *
 * @param[in] index index of bevel curve
 * @param[in,out] c resulting curve
 *
 * @return AY_OK on success, error code otherwise.
 */
int
ay_bevelt_findbevelcurve(int index, ay_object **c)
{
 int ay_status = AY_ERROR;
 int j;
 ay_object *o;

  if(!c)
    return AY_ENULL;

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
	} /* if */
      if(o)
	o = o->next;
    } /* while */

 return ay_status;
} /* ay_bevelt_findbevelcurve */


/** ay_bevelt_createbevelcurve:
 *  Create a standard bevel cross section curve (fills the
 *  curve object into the global array ay_bevelt_curves).
 *
 * @param[in] index type of bevel curve to create:
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
    ay_object_delete(o);

  if(controlv)
    free(controlv);
  if(knotv)
    free(knotv);

 return;
} /* ay_bevelt_createbevelcurve */
