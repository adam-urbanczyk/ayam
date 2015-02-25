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

/* sweep.c - sweep object */

static char *ay_sweep_name = "Sweep";

int ay_sweep_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_sweep_createcb:
 *  create callback function of sweep object
 */
int
ay_sweep_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtsweep";
 ay_sweep_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_sweep_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->rotate = AY_TRUE;
  new->sections = 0;

  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_sweep_createcb */


/* ay_sweep_deletecb:
 *  delete callback function of sweep object
 */
int
ay_sweep_deletecb(void *c)
{
 ay_sweep_object *sweep = NULL;

  if(!c)
    return AY_ENULL;

  sweep = (ay_sweep_object *)(c);

  if(sweep->npatch)
    (void)ay_object_delete(sweep->npatch);

  if(sweep->caps_and_bevels)
    (void)ay_object_deletemulti(sweep->caps_and_bevels, AY_FALSE);

  free(sweep);

 return AY_OK;
} /* ay_sweep_deletecb */


/* ay_sweep_copycb:
 *  copy callback function of sweep object
 */
int
ay_sweep_copycb(void *src, void **dst)
{
 ay_sweep_object *sweep = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(sweep = malloc(sizeof(ay_sweep_object))))
    return AY_EOMEM;

  memcpy(sweep, src, sizeof(ay_sweep_object));

  sweep->npatch = NULL;
  sweep->caps_and_bevels = NULL;

  *dst = (void *)sweep;

 return AY_OK;
} /* ay_sweep_copycb */


/* ay_sweep_drawcb:
 *  draw (display in an Ayam view window) callback function of sweep object
 */
int
ay_sweep_drawcb(struct Togl *togl, ay_object *o)
{
 ay_sweep_object *sweep = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  sweep = (ay_sweep_object *)o->refine;

  if(!sweep)
    return AY_ENULL;

  if(sweep->npatch)
    ay_draw_object(togl, sweep->npatch, AY_TRUE);

  if(sweep->caps_and_bevels)
    {
      b = sweep->caps_and_bevels;
      while(b)
	{
	  ay_draw_object(togl, b, AY_TRUE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_sweep_drawcb */


/* ay_sweep_shadecb:
 *  shade (display in an Ayam view window) callback function of sweep object
 */
int
ay_sweep_shadecb(struct Togl *togl, ay_object *o)
{
 ay_sweep_object *sweep = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  sweep = (ay_sweep_object *)o->refine;

  if(!sweep)
    return AY_ENULL;

  if(sweep->npatch)
    ay_shade_object(togl, sweep->npatch, AY_FALSE);

  if(sweep->caps_and_bevels)
    {
      b = sweep->caps_and_bevels;
      while(b)
	{
	  ay_shade_object(togl, b, AY_FALSE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_sweep_shadecb */


/* ay_sweep_drawhcb:
 *  draw handles (in an Ayam view window) callback function of sweep object
 */
int
ay_sweep_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_sweep_object *sweep;

  if(!o)
    return AY_ENULL;

  sweep = (ay_sweep_object *) o->refine;

  if(!sweep)
    return AY_ENULL;

  if(sweep->npatch)
    {
      ay_npt_drawrohandles((ay_nurbpatch_object *)sweep->npatch);
    }

 return AY_OK;
} /* ay_sweep_drawhcb */


/* ay_sweep_getpntcb:
 *  get point (editing and selection) callback function of sweep object
 */
int
ay_sweep_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbpatch_object *patch = NULL;
 ay_sweep_object *sweep = NULL;

  if(!o)
    return AY_ENULL;

  sweep = (ay_sweep_object *)o->refine;

  if(!sweep)
    return AY_ENULL;

  if(sweep->npatch)
    {
      patch = (ay_nurbpatch_object *)sweep->npatch->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, patch->width*patch->height, 4,
			     patch->controlv);
    }

 return AY_ERROR;
} /* ay_sweep_getpntcb */


/* ay_sweep_setpropcb:
 *  set property (from Tcl to C context) callback function of sweep object
 */
int
ay_sweep_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "SweepAttrData";
 /* char fname[] = "sweep_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_sweep_object *sweep = NULL;

  if(!interp || !o)
    return AY_ENULL;

  sweep = (ay_sweep_object *)o->refine;

  if(!sweep)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Rotate",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(sweep->rotate));

  Tcl_SetStringObj(ton,"Interpolate",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(sweep->interpolate));

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(sweep->close));

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(sweep->sections));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(sweep->display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(sweep->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_notify_object(o);

  o->modified = AY_TRUE;
  ay_notify_parent();

 return AY_OK;
} /* ay_sweep_setpropcb */


/* ay_sweep_getpropcb:
 *  get property (from C to Tcl context) callback function of sweep object
 */
int
ay_sweep_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="SweepAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_sweep_object *sweep = NULL;

  if(!interp || !o)
    return AY_ENULL;

  sweep = (ay_sweep_object *)(o->refine);

  if(!sweep)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Rotate",-1);
  to = Tcl_NewIntObj(sweep->rotate);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Interpolate",-1);
  to = Tcl_NewIntObj(sweep->interpolate);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_NewIntObj(sweep->close);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_NewIntObj(sweep->sections);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(sweep->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(sweep->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getnpinfo(interp, n1, sweep->npatch);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_sweep_getpropcb */


/* ay_sweep_readcb:
 *  read (from scene file) callback function of sweep object
 */
int
ay_sweep_readcb(FILE *fileptr, ay_object *o)
{
 ay_sweep_object *sweep = NULL;
 int caps[4] = {0};

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(sweep = calloc(1, sizeof(ay_sweep_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&sweep->rotate);
  fscanf(fileptr,"%d\n",&sweep->interpolate);
  fscanf(fileptr,"%d\n",&sweep->sections);
  fscanf(fileptr,"%d\n",&caps[0]);
  fscanf(fileptr,"%d\n",&caps[1]);
  fscanf(fileptr,"%d\n",&sweep->display_mode);
  fscanf(fileptr,"%lg\n",&sweep->glu_sampling_tolerance);

  if(ay_read_version > 3)
    {
      /* since 1.4 */
      fscanf(fileptr,"%d\n",&sweep->close);
    }

  if(ay_read_version < 16)
    {
      /* before Ayam 1.21 */
      ay_capt_createtags(o, caps);
    }

  o->refine = sweep;

 return AY_OK;
} /* ay_sweep_readcb */


/* ay_sweep_writecb:
 *  write (to scene file) callback function of sweep object
 */
int
ay_sweep_writecb(FILE *fileptr, ay_object *o)
{
 ay_sweep_object *sweep = NULL;
 ay_cparam cparams = {0};
 int caps[2] = {0};

  if(!fileptr || !o)
    return AY_ENULL;

  sweep = (ay_sweep_object *)(o->refine);

  if(!sweep)
    return AY_ENULL;

  if(o->tags)
    {
      /* for backwards compatibility wrt. caps */
      ay_capt_parsetags(o->tags, &cparams);
      if(cparams.states[0])
	caps[0] = cparams.types[0]+1;
      if(cparams.states[1])
	caps[1] = cparams.types[1]+1;
    }

  fprintf(fileptr, "%d\n", sweep->rotate);
  fprintf(fileptr, "%d\n", sweep->interpolate);
  fprintf(fileptr, "%d\n", sweep->sections);
  fprintf(fileptr, "%d\n", caps[0]);
  fprintf(fileptr, "%d\n", caps[1]);
  fprintf(fileptr, "%d\n", sweep->display_mode);
  fprintf(fileptr, "%g\n", sweep->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", sweep->close);

 return AY_OK;
} /* ay_sweep_writecb */


/* ay_sweep_wribcb:
 *  RIB export callback function of sweep object
 */
int
ay_sweep_wribcb(char *file, ay_object *o)
{
 ay_sweep_object *sweep;
 ay_object *b;

  if(!o)
   return AY_ENULL;

  sweep = (ay_sweep_object*)o->refine;

  if(!sweep)
    return AY_ENULL;

  if(sweep->npatch)
    ay_wrib_toolobject(file, sweep->npatch, o);

  if(sweep->caps_and_bevels)
    {
      b = sweep->caps_and_bevels;
      while(b)
	{
	  ay_wrib_toolobject(file, b, o);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_sweep_wribcb */


/* ay_sweep_bbccb:
 *  bounding box calculation callback function of sweep object
 */
int
ay_sweep_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_sweep_object *sweep = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  sweep = (ay_sweep_object *)o->refine;

  if(!sweep)
    return AY_ENULL;

  if(sweep->npatch)
    {
      ay_bbc_get(sweep->npatch, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return AY_OK;
} /* ay_sweep_bbccb */


/* ay_sweep_notifycb:
 *  notification callback function of sweep object
 */
int
ay_sweep_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_sweep_object *sweep = NULL;
 ay_object *curve1 = NULL, *curve2 = NULL, *curve3 = NULL;
 ay_object *pobject1 = NULL, *pobject2 = NULL, *pobject3 = NULL;
 ay_object *npatch = NULL, *bevel = NULL;
 ay_object **nextcb;
 ay_nurbpatch_object *np = NULL;
 int is_provided[3] = {0}, mode = 0;
 ay_bparam bparams = {0};
 ay_cparam cparams = {0};
 double tolerance;

  if(!o)
    return AY_ENULL;

  sweep = (ay_sweep_object *)(o->refine);

  if(!sweep)
    return AY_ENULL;

  mode = sweep->display_mode;
  tolerance = sweep->glu_sampling_tolerance;

  /* remove old objects */
  if(sweep->npatch)
    (void)ay_object_delete(sweep->npatch);
  sweep->npatch = NULL;

  if(sweep->caps_and_bevels)
    {
      (void)ay_object_deletemulti(sweep->caps_and_bevels, AY_FALSE);
      sweep->caps_and_bevels = NULL;
    }

  nextcb = &(sweep->caps_and_bevels);

  /* get curves to sweep */
  if(!o->down)
    goto cleanup;

  curve1 = o->down;
  if(curve1->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(curve1, AY_IDNCURVE, &pobject1);
      if(!pobject1)
	{
	  goto cleanup;
	}
      else
	{
	  curve1 = pobject1;
	  is_provided[0] = AY_TRUE;
	} /* if */
    } /* if */

  if(!o->down->next)
    goto cleanup;
  curve2 = o->down->next;
  if(curve2->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(curve2, AY_IDNCURVE, &pobject2);
      if(!pobject2)
	{
	  goto cleanup;
	}
      else
	{
	  curve2 = pobject2;
	  is_provided[1] = AY_TRUE;
	} /* if */
    } /* if */

  if(o->down->next->next)
    {
      curve3 = o->down->next->next;
      if(curve3->type != AY_IDNCURVE)
	{
	  (void)ay_provide_object(curve3, AY_IDNCURVE, &pobject3);
	  if(pobject3)
	    {
	      curve3 = pobject3;
	      is_provided[2] = AY_TRUE;
	    }
	  else
	    {
	      curve3 = NULL;
	    } /* if */
	} /* if */
    } /* if */

  /* sweep */
  if((ay_status = ay_npt_createnpatchobject(&npatch)))
    {
      goto cleanup;
    }

  if(sweep->close < 2)
    {
      /* open or simple closed sweep */
      ay_status = ay_npt_sweep(curve1, curve2, curve3,
			       sweep->sections, sweep->rotate, sweep->close,
			     (ay_nurbpatch_object **)(void*)&(npatch->refine));
    }
  else
    {
      /* periodic sweep */
      ay_status = ay_npt_sweepperiodic(curve1, curve2, curve3,
				       sweep->sections, sweep->rotate,
			    (ay_nurbpatch_object **)(void*)&(npatch->refine));
    }

  if(ay_status)
    {
      goto cleanup;
    }

  /* interpolate the swept surface */
  if(sweep->interpolate)
    {
      np = (ay_nurbpatch_object *)npatch->refine;
      ay_status = ay_ipt_interpolateu(np, np->uorder, AY_KTCHORDAL);
      if(ay_status)
	{
	  goto cleanup;
	}
    }

  sweep->npatch = npatch;

  /* prevent cleanup code from doing something harmful */
  npatch = NULL;

  /* get bevel and cap parameters */
  if(o->tags)
    {
      ay_bevelt_parsetags(o->tags, &bparams);
      ay_capt_parsetags(o->tags, &cparams);
    }

  /* create/add caps */
  if(cparams.has_caps)
    {
      ay_status = ay_capt_addcaps(&cparams, &bparams, sweep->npatch, nextcb);
      if(ay_status)
	goto cleanup;

      while(*nextcb)
	nextcb = &((*nextcb)->next);
    }

  /* create/add bevels */
  if(bparams.has_bevels)
    {
      ay_status = ay_bevelt_addbevels(&bparams, &cparams, sweep->npatch,
				      nextcb);
      if(ay_status)
	goto cleanup;
    }

  /* copy sampling tolerance/mode over to new objects */
  ((ay_nurbpatch_object *)sweep->npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)sweep->npatch->refine)->display_mode =
    mode;

  /* copy sampling tolerance/mode attributes to caps and bevels */
  if(sweep->caps_and_bevels)
    {
      bevel = sweep->caps_and_bevels;
      while(bevel)
	{
	  ((ay_nurbpatch_object *)
	   (bevel->refine))->glu_sampling_tolerance = tolerance;
	  ((ay_nurbpatch_object *)
	   (bevel->refine))->display_mode = mode;
	  bevel = bevel->next;
	}
    }

cleanup:
  /* remove provided objects */
  if(is_provided[0])
    {
      (void)ay_object_deletemulti(pobject1, AY_FALSE);
    }

  if(is_provided[1])
    {
      (void)ay_object_deletemulti(pobject2, AY_FALSE);
    }

  if(is_provided[2])
    {
      (void)ay_object_deletemulti(pobject3, AY_FALSE);
    }

  /* remove patch */
  if(npatch)
    {
      (void)ay_object_delete(npatch);
      sweep->npatch = NULL;
    }

  /* recover selected points */
  if(o->selp)
    {
      if(sweep->npatch)
	ay_sweep_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

 return ay_status;
} /* ay_sweep_notifycb */


/* ay_sweep_providecb:
 *  provide callback function of sweep object
 */
int
ay_sweep_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 ay_sweep_object *s = NULL;

  if(!o)
    return AY_ENULL;

  s = (ay_sweep_object *) o->refine;

  if(!s)
    return AY_ENULL;

 return ay_provide_nptoolobj(o, type, s->npatch, s->caps_and_bevels, result);
} /* ay_sweep_providecb */


/* ay_sweep_convertcb:
 *  convert callback function of sweep object
 */
int
ay_sweep_convertcb(ay_object *o, int in_place)
{
 ay_sweep_object *s = NULL;

  if(!o)
    return AY_ENULL;

  s = (ay_sweep_object *) o->refine;

  if(!s)
    return AY_ENULL;

 return ay_convert_nptoolobj(o, s->npatch, s->caps_and_bevels, in_place);
} /* ay_sweep_convertcb */


/* ay_sweep_init:
 *  initialize the sweep object module
 */
int
ay_sweep_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_sweep_name,
				    ay_sweep_createcb,
				    ay_sweep_deletecb,
				    ay_sweep_copycb,
				    ay_sweep_drawcb,
				    ay_sweep_drawhcb,
				    ay_sweep_shadecb,
				    ay_sweep_setpropcb,
				    ay_sweep_getpropcb,
				    ay_sweep_getpntcb,
				    ay_sweep_readcb,
				    ay_sweep_writecb,
				    ay_sweep_wribcb,
				    ay_sweep_bbccb,
				    AY_IDSWEEP);


  ay_status += ay_notify_register(ay_sweep_notifycb, AY_IDSWEEP);

  ay_status += ay_convert_register(ay_sweep_convertcb, AY_IDSWEEP);

  ay_status += ay_provide_register(ay_sweep_providecb, AY_IDSWEEP);

 return ay_status;
} /* ay_sweep_init */

