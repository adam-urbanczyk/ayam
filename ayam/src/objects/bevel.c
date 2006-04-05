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

/* bevel.c - bevel object */

static char *ay_bevel_name = "Bevel";

int
ay_bevel_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtbevel";
 ay_bevel_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_bevel_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_bevel_createcb */


int
ay_bevel_deletecb(void *c)
{
 ay_bevel_object *bevel = NULL;

  if(!c)
    return AY_ENULL;    

  bevel = (ay_bevel_object *)(c);

  if(bevel->npatch)
    ay_object_delete(bevel->npatch);

  free(bevel);

 return AY_OK;
} /* ay_bevel_deletecb */


int
ay_bevel_copycb(void *src, void **dst)
{
 ay_bevel_object *bevel = NULL, *bevelsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  bevelsrc = (ay_bevel_object *)src;

  if(!(bevel = calloc(1, sizeof(ay_bevel_object))))
    return AY_EOMEM; 

  memcpy(bevel, src, sizeof(ay_bevel_object)); 

  bevel->npatch = NULL;

  *dst = (void *)bevel;

 return AY_OK;
} /* ay_bevel_copycb */


int
ay_bevel_drawcb(struct Togl *togl, ay_object *o)
{
 ay_bevel_object *bevel = NULL;

  if(!o)
    return AY_ENULL;

  bevel = (ay_bevel_object *)o->refine;

  if(!bevel)
    return AY_ENULL;

  if(bevel->npatch)
    ay_draw_object(togl, bevel->npatch, AY_TRUE);

 return AY_OK;
} /* ay_bevel_drawcb */


int
ay_bevel_shadecb(struct Togl *togl, ay_object *o)
{
 ay_bevel_object *bevel = NULL;

  if(!o)
    return AY_ENULL;

  bevel = (ay_bevel_object *)o->refine;

  if(!bevel)
    return AY_ENULL;

  if(bevel->npatch)
    ay_shade_object(togl, bevel->npatch, AY_FALSE);

 return AY_OK;
} /* ay_bevel_shadecb */


int
ay_bevel_drawhcb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_bevel_drawhcb */


int
ay_bevel_getpntcb(int mode, ay_object *o, double *p)
{

 return AY_OK;
} /* ay_bevel_getpntcb */


/* Tcl -> C! */
int
ay_bevel_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "BevelAttrData";
 /* char fname[] = "bevel_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_bevel_object *bevel = NULL;

  if(!o)
    return AY_ENULL;

  bevel = (ay_bevel_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(bevel->glu_display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(bevel->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_force(o);

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_bevel_setpropcb */


/* C -> Tcl! */
int
ay_bevel_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="BevelAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_bevel_object *bevel = NULL;

  if(!o)
    return AY_ENULL;

  bevel = (ay_bevel_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(bevel->glu_display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(bevel->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_bevel_getpropcb */


int
ay_bevel_readcb(FILE *fileptr, ay_object *o)
{
 ay_bevel_object *bevel = NULL;

 if(!o)
   return AY_ENULL;

  if(!(bevel = calloc(1, sizeof(ay_bevel_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&bevel->glu_display_mode);
  fscanf(fileptr,"%lg\n",&bevel->glu_sampling_tolerance);

  o->refine = bevel;

 return AY_OK;
} /* ay_bevel_readcb */


int
ay_bevel_writecb(FILE *fileptr, ay_object *o)
{
 ay_bevel_object *bevel = NULL;

  if(!o)
    return AY_ENULL;

  bevel = (ay_bevel_object *)(o->refine);

  fprintf(fileptr, "%d\n", bevel->glu_display_mode);
  fprintf(fileptr, "%g\n", bevel->glu_sampling_tolerance);

 return AY_OK;
} /* ay_bevel_writecb */


int
ay_bevel_wribcb(char *file, ay_object *o)
{
 ay_bevel_object *bevel;

  if(!o)
   return AY_ENULL;

  bevel = (ay_bevel_object*)o->refine;

  if(bevel->npatch)
    ay_wrib_object(file, bevel->npatch);

 return AY_OK;
} /* ay_bevel_wribcb */


int
ay_bevel_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_bevel_object *bevel = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  bevel = (ay_bevel_object *)o->refine; 

  if(bevel->npatch)
    {
      ay_bbc_get(bevel->npatch, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return AY_OK;
} /* ay_bevel_bbccb */


int
ay_bevel_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 int c1provided = AY_FALSE, mode = 0;
 ay_bevel_object *bevel = NULL;
 ay_object *npatch = NULL, *curve1, *rcurve = NULL, *pobject1 = NULL;
 int has_b = AY_FALSE;
 int b_type, b_sense;
 double b_radius, tolerance;

  if(!o)
    return AY_ENULL;    

  bevel = (ay_bevel_object *)(o->refine);

  mode = bevel->glu_display_mode;
  tolerance = bevel->glu_sampling_tolerance;

  /* remove old objects */
  if(bevel->npatch)
    ay_object_delete(bevel->npatch);
  bevel->npatch = NULL;

  /* get curves to bevel */
  if(!o->down)
    return AY_OK;

  curve1 = o->down;
  if(curve1->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(curve1, AY_IDNCURVE, &pobject1);
      if(!pobject1)
	{
	  return AY_OK;
	}
      else
	{
	  curve1 = pobject1;
	  c1provided = AY_TRUE;
	} /* if */
    } /* if */

  if(!curve1)
    {
      ay_status = AY_OK;
      goto cleanup;
    }

  /* get bevel parameters */
  ay_npt_getbeveltags(o, 0, &has_b, &b_type, &b_radius, &b_sense);
  if(!has_b)
    {
      ay_status = AY_ERROR;
      goto cleanup;
    }

  if(b_sense)
    {
      if(!c1provided)
	{
	  ay_status = ay_object_copy(curve1, &rcurve);
	  if(!rcurve)
	    goto cleanup;

	  curve1 = rcurve;
	}
      ay_nct_revert((ay_nurbcurve_object*)curve1->refine);
    } /* if */

  /* create bevel */
  if(!(npatch = calloc(1, sizeof(ay_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  ay_object_defaults(npatch);
  npatch->type = AY_IDNPATCH;
  npatch->parent = AY_TRUE;
  npatch->inherit_trafos = AY_FALSE;
  ay_status = ay_npt_bevel(b_type, b_radius, AY_TRUE, curve1,
			   (ay_nurbpatch_object**)&(npatch->refine));

  if(ay_status)
    goto cleanup;

  /* copy sampling tolerance/mode over to new objects */
  ((ay_nurbpatch_object *)npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)npatch->refine)->glu_display_mode =
    mode;

  bevel->npatch = npatch;
  npatch = NULL;


cleanup:
  /* remove provided object */
  if(c1provided && pobject1)
    {
      ay_object_delete(pobject1);
    }

  if(npatch)
    {
      ay_object_delete(npatch);
    }

 return ay_status;
} /* ay_bevel_notifycb */


int
ay_bevel_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "bevel_providecb";
 ay_bevel_object *s = NULL;
 ay_object *new = NULL, **t = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNPATCH)
	return AY_OK;
      else
	return AY_ERROR;
    }

  s = (ay_bevel_object *) o->refine;

  if(type == AY_IDNPATCH)
    {
      t = &(new);

      /* copy bevel */
      ay_status = ay_object_copy(s->npatch, t);
      if(ay_status)
	{
	  ay_error(ay_status, fname, NULL);
	  return AY_ERROR;
	}
      ay_trafo_copy(o, *t);
      t = &((*t)->next);

      *result = new;
    } /* if */

 return ay_status;
} /* ay_bevel_providecb */


int
ay_bevel_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_bevel_object *r = NULL;
 ay_object *new = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */
  r = (ay_bevel_object *) o->refine;

  if(r->npatch)
    {
      ay_status = ay_object_copy(r->npatch, &new);
      ay_trafo_copy(o, new);
      new->hide_children = AY_TRUE;
      new->parent = AY_TRUE;
      ay_object_crtendlevel(&(new->down));
    } /* if */

  /* second, link new objects, or replace old objects with them */
  if(new)
    {
      if(!in_place)
	{
	  ay_status = ay_object_link(new);
	}
      else
	{
	  ay_object_replace(new, o);
	} /* if */
    } /* if */

 return ay_status;
} /* ay_bevel_convertcb */


int
ay_bevel_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_bevel_name,
				    ay_bevel_createcb,
				    ay_bevel_deletecb,
				    ay_bevel_copycb,
				    ay_bevel_drawcb,
				    NULL, /* no handles */
				    ay_bevel_shadecb,
				    ay_bevel_setpropcb,
				    ay_bevel_getpropcb,
				    ay_bevel_getpntcb,
				    ay_bevel_readcb,
				    ay_bevel_writecb,
				    ay_bevel_wribcb,
				    ay_bevel_bbccb,
				    AY_IDBEVEL);

  
  ay_status = ay_notify_register(ay_bevel_notifycb, AY_IDBEVEL);

  ay_status = ay_convert_register(ay_bevel_convertcb, AY_IDBEVEL);

  ay_status = ay_provide_register(ay_bevel_providecb, AY_IDBEVEL);

 return ay_status;
} /* ay_bevel_init */

