/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2004 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* birail1.c - birail1 object */

static char *ay_birail1_name = "Birail1";

int ay_birail1_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_birail1_createcb:
 *  create callback function of birail1 object
 */
int
ay_birail1_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtbirail1";
 ay_birail1_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_birail1_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->sections = 0;

  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_birail1_createcb */


/* ay_birail1_deletecb:
 *  delete callback function of birail1 object
 */
int
ay_birail1_deletecb(void *c)
{
 ay_birail1_object *birail1 = NULL;

  if(!c)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)(c);

  if(birail1->npatch)
    (void)ay_object_delete(birail1->npatch);

  if(birail1->caps_and_bevels)
    (void)ay_object_deletemulti(birail1->caps_and_bevels, AY_FALSE);

  free(birail1);

 return AY_OK;
} /* ay_birail1_deletecb */


/* ay_birail1_copycb:
 *  copy callback function of birail1 object
 */
int
ay_birail1_copycb(void *src, void **dst)
{
 ay_birail1_object *birail1 = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(birail1 = malloc(sizeof(ay_birail1_object))))
    return AY_EOMEM;

  memcpy(birail1, src, sizeof(ay_birail1_object));

  birail1->npatch = NULL;
  birail1->caps_and_bevels = NULL;

  *dst = (void *)birail1;

 return AY_OK;
} /* ay_birail1_copycb */


/* ay_birail1_drawcb:
 *  draw (display in an Ayam view window) callback function of birail1 object
 */
int
ay_birail1_drawcb(struct Togl *togl, ay_object *o)
{
 ay_birail1_object *birail1;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)o->refine;

  if(!birail1)
    return AY_ENULL;

  if(birail1->npatch)
    ay_draw_object(togl, birail1->npatch, AY_TRUE);

  if(birail1->caps_and_bevels)
    {
      b = birail1->caps_and_bevels;
      while(b)
	{
	  ay_draw_object(togl, b, AY_TRUE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_birail1_drawcb */


/* ay_birail1_shadecb:
 *  shade (display in an Ayam view window) callback function of birail1 object
 */
int
ay_birail1_shadecb(struct Togl *togl, ay_object *o)
{
 ay_birail1_object *birail1;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)o->refine;

  if(!birail1)
    return AY_ENULL;

  if(birail1->npatch)
    ay_shade_object(togl, birail1->npatch, AY_FALSE);

  if(birail1->caps_and_bevels)
    {
      b = birail1->caps_and_bevels;
      while(b)
	{
	  ay_shade_object(togl, b, AY_FALSE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_birail1_shadecb */


/* ay_birail1_drawhcb:
 *  draw handles (in an Ayam view window) callback function of birail1 object
 */
int
ay_birail1_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_birail1_object *birail1;

  if(!o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *) o->refine;

  if(!birail1)
    return AY_ENULL;

  if(birail1->npatch)
    {
      ay_npt_drawrohandles((ay_nurbpatch_object *)birail1->npatch);
    }

 return AY_OK;
} /* ay_birail1_drawhcb */


/* ay_birail1_getpntcb:
 *  get point (editing and selection) callback function of birail1 object
 */
int
ay_birail1_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbpatch_object *patch = NULL;
 ay_birail1_object *birail1 = NULL;

  if(!o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)o->refine;

  if(!birail1)
    return AY_ENULL;

  if(birail1->npatch)
    {
      patch = (ay_nurbpatch_object *)birail1->npatch->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, patch->width*patch->height, 4,
			     patch->controlv);
    }

 return AY_ERROR;
} /* ay_birail1_getpntcb */


/* ay_birail1_setpropcb:
 *  set property (from Tcl to C context) callback function of birail1 object
 */
int
ay_birail1_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "Birail1AttrData";
 /* char fname[] = "birail1_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_birail1_object *birail1 = NULL;

  if(!interp || !o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)o->refine;

  if(!birail1)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail1->type));

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail1->sections));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail1->display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(birail1->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_birail1_setpropcb */


/* ay_birail1_getpropcb:
 *  get property (from C to Tcl context) callback function of birail1 object
 */
int
ay_birail1_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="Birail1AttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_birail1_object *birail1 = NULL;

  if(!interp || !o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)(o->refine);

  if(!birail1)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_NewIntObj(birail1->type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_NewIntObj(birail1->sections);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(birail1->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(birail1->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getnpinfo(interp, n1, birail1->npatch);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_birail1_getpropcb */


/* ay_birail1_readcb:
 *  read (from scene file) callback function of birail1 object
 */
int
ay_birail1_readcb(FILE *fileptr, ay_object *o)
{
 ay_birail1_object *birail1 = NULL;
 int caps[4] = {0};

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(birail1 = calloc(1, sizeof(ay_birail1_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&birail1->type);
  fscanf(fileptr,"%d\n",&birail1->sections);
  fscanf(fileptr,"%d\n",&caps[0]);
  fscanf(fileptr,"%d\n",&caps[1]);
  fscanf(fileptr,"%d\n",&birail1->display_mode);
  fscanf(fileptr,"%lg\n",&birail1->glu_sampling_tolerance);

  if(ay_read_version < 16)
    {
      /* before Ayam 1.21 */
      ay_capt_createtags(o, caps);
    }

  o->refine = birail1;

 return AY_OK;
} /* ay_birail1_readcb */


/* ay_birail1_writecb:
 *  write (to scene file) callback function of birail1 object
 */
int
ay_birail1_writecb(FILE *fileptr, ay_object *o)
{
 ay_birail1_object *birail1 = NULL;
 ay_cparam cparams = {0};
 int caps[2] = {0};

  if(!fileptr || !o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)(o->refine);

  if(!birail1)
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

  fprintf(fileptr, "%d\n", birail1->type);
  fprintf(fileptr, "%d\n", birail1->sections);
  fprintf(fileptr, "%d\n", caps[0]);
  fprintf(fileptr, "%d\n", caps[1]);
  fprintf(fileptr, "%d\n", birail1->display_mode);
  fprintf(fileptr, "%g\n", birail1->glu_sampling_tolerance);

 return AY_OK;
} /* ay_birail1_writecb */


/* ay_birail1_wribcb:
 *  RIB export callback function of birail1 object
 */
int
ay_birail1_wribcb(char *file, ay_object *o)
{
 ay_birail1_object *birail1;
 ay_object *b;

  if(!o)
   return AY_ENULL;

  birail1 = (ay_birail1_object*)o->refine;

  if(!birail1)
    return AY_ENULL;

  if(birail1->npatch)
    ay_wrib_toolobject(file, birail1->npatch, o);

  if(birail1->caps_and_bevels)
    {
      b = birail1->caps_and_bevels;
      while(b)
	{
	  ay_wrib_toolobject(file, b, o);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_birail1_wribcb */


/* ay_birail1_bbccb:
 *  bounding box calculation callback function of birail1 object
 */
int
ay_birail1_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_birail1_object *birail1 = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)o->refine;

  if(!birail1)
    return AY_ENULL;

  if(birail1->npatch)
    {
      ay_bbc_get(birail1->npatch, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return AY_OK;
} /* ay_birail1_bbccb */


/* ay_birail1_notifycb:
 *  notification callback function of birail1 object
 */
int
ay_birail1_notifycb(ay_object *o)
{
 ay_birail1_object *birail1 = NULL;
 ay_object *curve1 = NULL, *curve2 = NULL, *pobject1 = NULL, *pobject2 = NULL;
 ay_object *curve3 = NULL, *pobject3 = NULL;
 ay_object *npatch = NULL, **nextcb;
 ay_object *bevel = NULL;
 ay_bparam bparams = {0};
 ay_cparam cparams = {0};
 int ay_status = AY_OK;
 int is_provided[3] = {0}, mode = 0;
 double tolerance;

  if(!o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)(o->refine);

  if(!birail1)
    return AY_ENULL;

  mode = birail1->display_mode;
  tolerance = birail1->glu_sampling_tolerance;

  nextcb = &(birail1->caps_and_bevels);

  /* remove old objects */
  if(birail1->npatch)
    (void)ay_object_delete(birail1->npatch);
  birail1->npatch = NULL;

  if(birail1->caps_and_bevels)
    {
      (void)ay_object_deletemulti(birail1->caps_and_bevels, AY_FALSE);
      birail1->caps_and_bevels = NULL;
    }

  /* get curves to birail1 */
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

  if(!o->down->next->next)
    goto cleanup;

  curve3 = o->down->next->next;
  if(curve3->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(curve3, AY_IDNCURVE, &pobject3);
      if(!pobject3)
	{
	  goto cleanup;
	}
      else
	{
	  curve3 = pobject3;
	  is_provided[2] = AY_TRUE;
	} /* if */
    } /* if */

  /* do the birail */
  ay_status = ay_npt_createnpatchobject(&npatch);
  if(ay_status)
    goto cleanup;

  if(birail1->type < 2)
    {
      ay_status = ay_npt_birail1(curve1, curve2, curve3,
			   birail1->sections, birail1->type,
			   (ay_nurbpatch_object **)(void*)&(npatch->refine));
    }
  else
    {
      ay_status = ay_npt_birail1periodic(curve1, curve2, curve3,
					 birail1->sections,
			   (ay_nurbpatch_object **)(void*)&(npatch->refine));
    }

  if(ay_status)
    goto cleanup;

  birail1->npatch = npatch;

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
      ay_status = ay_capt_addcaps(&cparams, &bparams, birail1->npatch, nextcb);
      if(ay_status)
	goto cleanup;

      while(*nextcb)
	nextcb = &((*nextcb)->next);
    }

  /* create/add bevels */
  if(bparams.has_bevels)
    {
      ay_status = ay_bevelt_addbevels(&bparams, &cparams, birail1->npatch,
				      nextcb);
      if(ay_status)
	goto cleanup;
    }

  /* copy sampling tolerance/mode attributes over to birail */
  ((ay_nurbpatch_object *)birail1->npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)birail1->npatch->refine)->display_mode =
    mode;

  /* copy sampling tolerance/mode attributes to caps and bevels */
  if(birail1->caps_and_bevels)
    {
      bevel = birail1->caps_and_bevels;
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

  if(npatch)
    {
      (void)ay_object_delete(npatch);
    }

  /* recover selected points */
  if(o->selp)
    {
      if(birail1->npatch)
	ay_birail1_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

 return ay_status;
} /* ay_birail1_notifycb */


/* ay_birail1_providecb:
 *  provide callback function of birail1 object
 */
int
ay_birail1_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 ay_birail1_object *b = NULL;

  if(!o)
    return AY_ENULL;

  b = (ay_birail1_object *) o->refine;

  if(!b)
    return AY_ENULL;

 return ay_provide_nptoolobj(o, type, b->npatch, b->caps_and_bevels, result);
} /* ay_birail1_providecb */


/* ay_birail1_convertcb:
 *  convert callback function of birail1 object
 */
int
ay_birail1_convertcb(ay_object *o, int in_place)
{
 ay_birail1_object *b = NULL;

  if(!o)
    return AY_ENULL;

  b = (ay_birail1_object *) o->refine;

  if(!b)
    return AY_ENULL;

 return ay_convert_nptoolobj(o, b->npatch, b->caps_and_bevels, in_place);
} /* ay_birail1_convertcb */


/* ay_birail1_init:
 *  initialize the birail1 object module
 */
int
ay_birail1_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_birail1_name,
				    ay_birail1_createcb,
				    ay_birail1_deletecb,
				    ay_birail1_copycb,
				    ay_birail1_drawcb,
				    ay_birail1_drawhcb,
				    ay_birail1_shadecb,
				    ay_birail1_setpropcb,
				    ay_birail1_getpropcb,
				    ay_birail1_getpntcb,
				    ay_birail1_readcb,
				    ay_birail1_writecb,
				    ay_birail1_wribcb,
				    ay_birail1_bbccb,
				    AY_IDBIRAIL1);


  ay_status += ay_notify_register(ay_birail1_notifycb, AY_IDBIRAIL1);

  ay_status += ay_convert_register(ay_birail1_convertcb, AY_IDBIRAIL1);

  ay_status += ay_provide_register(ay_birail1_providecb, AY_IDBIRAIL1);

 return ay_status;
} /* ay_birail1_init */

