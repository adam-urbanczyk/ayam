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

/* revolve.c - revolve object */

static char *ay_revolve_name = "Revolve";

int ay_revolve_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_revolve_createcb:
 *  create callback function of revolve object
 */
int
ay_revolve_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtrevolve";
 ay_revolve_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_revolve_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->thetamax = 360.0;

  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_revolve_createcb */


/* ay_revolve_deletecb:
 *  delete callback function of revolve object
 */
int
ay_revolve_deletecb(void *c)
{
 ay_revolve_object *revolve = NULL;

  if(!c)
    return AY_ENULL;

  revolve = (ay_revolve_object *)(c);

  if(revolve->npatch)
    (void)ay_object_delete(revolve->npatch);

  if(revolve->caps_and_bevels)
    (void)ay_object_deletemulti(revolve->caps_and_bevels, AY_FALSE);

  free(revolve);

 return AY_OK;
} /* ay_revolve_deletecb */


/* ay_revolve_copycb:
 *  copy callback function of revolve object
 */
int
ay_revolve_copycb(void *src, void **dst)
{
 ay_revolve_object *revolve = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(revolve = malloc(sizeof(ay_revolve_object))))
    return AY_EOMEM;

  memcpy(revolve, src, sizeof(ay_revolve_object));

  revolve->npatch = NULL;
  revolve->caps_and_bevels = NULL;

  *dst = (void *)revolve;

 return AY_OK;
} /* ay_revolve_copycb */


/* ay_revolve_drawcb:
 *  draw (display in an Ayam view window) callback function of revolve object
 */
int
ay_revolve_drawcb(struct Togl *togl, ay_object *o)
{
 ay_revolve_object *revolve = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  revolve = (ay_revolve_object *)o->refine;

  if(!revolve)
    return AY_ENULL;

  if(revolve->npatch)
    ay_draw_object(togl, revolve->npatch, AY_TRUE);

  if(revolve->caps_and_bevels)
    {
      b = revolve->caps_and_bevels;
      while(b)
	{
	  ay_draw_object(togl, b, AY_TRUE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_revolve_drawcb */


/* ay_revolve_shadecb:
 *  shade (display in an Ayam view window) callback function of revolve object
 */
int
ay_revolve_shadecb(struct Togl *togl, ay_object *o)
{
 ay_revolve_object *revolve = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  revolve = (ay_revolve_object *)o->refine;

  if(!revolve)
    return AY_ENULL;

  if(revolve->npatch)
    ay_shade_object(togl, revolve->npatch, AY_FALSE);

  if(revolve->caps_and_bevels)
    {
      b = revolve->caps_and_bevels;
      while(b)
	{
	  ay_shade_object(togl, b, AY_FALSE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_revolve_shadecb */


/* ay_revolve_drawhcb:
 *  draw handles (in an Ayam view window) callback function of revolve object
 */
int
ay_revolve_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_revolve_object *revolve;

  if(!o)
    return AY_ENULL;

  revolve = (ay_revolve_object *) o->refine;

  if(!revolve)
    return AY_ENULL;

  if(revolve->npatch)
    {
      ay_npt_drawrohandles((ay_nurbpatch_object *)revolve->npatch);
    }

 return AY_OK;
} /* ay_revolve_drawhcb */


/* ay_revolve_getpntcb:
 *  get point (editing and selection) callback function of revolve object
 */
int
ay_revolve_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbpatch_object *patch = NULL;
 ay_revolve_object *revolve = NULL;

  if(!o)
    return AY_ENULL;

  revolve = (ay_revolve_object *)o->refine;

  if(!revolve)
    return AY_ENULL;

  if(revolve->npatch)
    {
      patch = (ay_nurbpatch_object *)revolve->npatch->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, patch->width*patch->height, 4,
			     patch->controlv);
    }

 return AY_ERROR;
} /* ay_revolve_getpntcb */


/* ay_revolve_setpropcb:
 *  set property (from Tcl to C context) callback function of revolve object
 */
int
ay_revolve_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "RevolveAttrData";
 /* char fname[] = "revolve_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_revolve_object *revolve = NULL;

  if(!interp || !o)
    return AY_ENULL;

  revolve = (ay_revolve_object *)o->refine;

  if(!revolve)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(revolve->thetamax));

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(revolve->sections));

  Tcl_SetStringObj(ton,"Order",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(revolve->order));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(revolve->display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(revolve->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_revolve_setpropcb */


/* ay_revolve_getpropcb:
 *  get property (from C to Tcl context) callback function of revolve object
 */
int
ay_revolve_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="RevolveAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_revolve_object *revolve = NULL;

  if(!interp || !o)
    return AY_ENULL;

  revolve = (ay_revolve_object *)(o->refine);

  if(!revolve)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_NewDoubleObj(revolve->thetamax);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_NewIntObj(revolve->sections);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order",-1);
  to = Tcl_NewIntObj(revolve->order);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(revolve->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(revolve->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getnpinfo(interp, n1, revolve->npatch);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_revolve_getpropcb */


/* ay_revolve_readcb:
 *  read (from scene file) callback function of revolve object
 */
int
ay_revolve_readcb(FILE *fileptr, ay_object *o)
{
 ay_revolve_object *revolve = NULL;
 int caps[4] = {0};

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(revolve = calloc(1, sizeof(ay_revolve_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%lg\n",&revolve->thetamax);
  fscanf(fileptr,"%d\n",&caps[0]);
  fscanf(fileptr,"%d\n",&caps[1]);
  fscanf(fileptr,"%d\n",&caps[2]);
  fscanf(fileptr,"%d\n",&caps[3]);
  fscanf(fileptr,"%d\n",&revolve->display_mode);
  fscanf(fileptr,"%lg\n",&revolve->glu_sampling_tolerance);

  if(ay_read_version >= 7)
    {
      /* since 1.8 */
      fscanf(fileptr,"%d\n",&revolve->sections);
      fscanf(fileptr,"%d\n",&revolve->order);
    }

  if(ay_read_version < 16)
    {
      /* before Ayam 1.21 */
      ay_capt_createtags(o, caps);
    }

  o->refine = revolve;

 return AY_OK;
} /* ay_revolve_readcb */


/* ay_revolve_writecb:
 *  write (to scene file) callback function of revolve object
 */
int
ay_revolve_writecb(FILE *fileptr, ay_object *o)
{
 ay_revolve_object *revolve = NULL;
 ay_cparam cparams = {0};
 int caps[4] = {0};

  if(!fileptr || !o)
    return AY_ENULL;

  revolve = (ay_revolve_object *)(o->refine);

  if(!revolve)
    return AY_ENULL;

  if(o->tags)
    {
      /* for backwards compatibility wrt. caps */
      ay_capt_parsetags(o->tags, &cparams);
      if(cparams.states[0])
	caps[0] = cparams.types[0]+1;
      if(cparams.states[1])
	caps[1] = cparams.types[1]+1;
      if(cparams.states[2])
	caps[2] = cparams.types[2]+1;
      if(cparams.states[3])
	caps[3] = cparams.types[3]+1;
    }

  fprintf(fileptr, "%g\n", revolve->thetamax);
  fprintf(fileptr, "%d\n", caps[0]);
  fprintf(fileptr, "%d\n", caps[1]);
  fprintf(fileptr, "%d\n", caps[2]);
  fprintf(fileptr, "%d\n", caps[3]);
  fprintf(fileptr, "%d\n", revolve->display_mode);
  fprintf(fileptr, "%g\n", revolve->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", revolve->sections);
  fprintf(fileptr, "%d\n", revolve->order);

 return AY_OK;
} /* ay_revolve_writecb */


/* ay_revolve_wribcb:
 *  RIB export callback function of revolve object
 */
int
ay_revolve_wribcb(char *file, ay_object *o)
{
 ay_revolve_object *revolve = NULL;
 ay_object *b;

  if(!o)
   return AY_ENULL;

  revolve = (ay_revolve_object*)o->refine;

  if(!revolve)
    return AY_ENULL;

  if(revolve->npatch)
    ay_wrib_toolobject(file, revolve->npatch, o);

  if(revolve->caps_and_bevels)
    {
      b = revolve->caps_and_bevels;
      while(b)
	{
	  ay_wrib_object(file, b);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_revolve_wribcb */


/* ay_revolve_bbccb:
 *  bounding box calculation callback function of revolve object
 */
int
ay_revolve_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_revolve_object *revolve = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  revolve = (ay_revolve_object *)o->refine;

  if(!revolve)
    return AY_ENULL;

  if(revolve->npatch)
    {
      ay_bbc_get(revolve->npatch, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return AY_OK;
} /* ay_revolve_bbccb */


/* ay_revolve_notifycb:
 *  notification callback function of revolve object
 */
int
ay_revolve_notifycb(ay_object *o)
{
 int ay_status = AY_OK, phase = 0;
 char fname[] = "revolve_notify";
 ay_object *curve = NULL, *pobject = NULL, **nextcb = NULL, *newo = NULL;
 ay_revolve_object *revolve = NULL;
 int is_provided = AY_FALSE, mode = 0;
 double tolerance;
 ay_object *bevel = NULL;
 ay_bparam bparams = {0};
 ay_cparam cparams = {0};

  if(!o)
    return AY_ENULL;

  revolve = (ay_revolve_object *)(o->refine);

  if(!revolve)
    return AY_ENULL;

  mode = revolve->display_mode;
  tolerance = revolve->glu_sampling_tolerance;

  nextcb = &(revolve->caps_and_bevels);

  /* remove old objects */
  if(revolve->npatch)
    (void)ay_object_delete(revolve->npatch);
  revolve->npatch = NULL;

  if(revolve->caps_and_bevels)
    {
      (void)ay_object_deletemulti(revolve->caps_and_bevels, AY_FALSE);
      revolve->caps_and_bevels = NULL;
    }

  /* get curve to revolve */
  if(!o->down || !o->down->next)
    goto cleanup;

  curve = o->down;
  if(curve->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(curve, AY_IDNCURVE, &pobject);
      if(!pobject)
	{
	  ay_status = AY_ERROR;
	  goto cleanup;
	}
      else
	{
	  curve = pobject;
	  is_provided = AY_TRUE;
	} /* if */
    } /* if */

  /* revolve */
  phase = 1;
  ay_status = ay_npt_createnpatchobject(&newo);

  if(ay_status)
    goto cleanup;

  ay_status = ay_npt_revolve(curve, revolve->thetamax, revolve->sections,
			     revolve->order,
			  (ay_nurbpatch_object **)(void*)&(newo->refine));

  if(ay_status)
    goto cleanup;

  revolve->npatch = newo;

  /* create bevels/caps */
  phase = 2;

  /* get bevel and cap parameters */
  if(o->tags)
    {
      ay_bevelt_parsetags(o->tags, &bparams);
      ay_capt_parsetags(o->tags, &cparams);
    }

  /* create/add caps */
  if(cparams.has_caps)
    {
      ay_status = ay_capt_addcaps(&cparams, &bparams, revolve->npatch, nextcb);
      if(ay_status)
	goto cleanup;

      while(*nextcb)
	nextcb = &((*nextcb)->next);
    }

  /* create/add bevels */
  if(bparams.has_bevels)
    {
      ay_status = ay_bevelt_addbevels(&bparams, &cparams, revolve->npatch,
				      nextcb);
      if(ay_status)
	goto cleanup;
    }

  /* copy sampling tolerance/mode over to new objects */
  ((ay_nurbpatch_object *)revolve->npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)revolve->npatch->refine)->display_mode =
    mode;

  if(revolve->caps_and_bevels)
    {
      bevel = revolve->caps_and_bevels;
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
  /* remove provided object(s) */
  if(is_provided)
    {
      (void)ay_object_deletemulti(pobject, AY_FALSE);
    }

  /* recover selected points */
  if(o->selp)
    {
      if(revolve->npatch)
	ay_revolve_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

  if(ay_status)
    {
      switch(phase)
	{
	case 1:
	  ay_error(AY_ERROR, fname, "Revolve failed.");
	  break;
	case 2:
	  ay_error(AY_EWARN, fname, "Bevel/Cap creation failed.");
	  ay_status = AY_OK;
	  break;
	default:
	  break;
	}
    }

 return ay_status;
} /* ay_revolve_notifycb */


/* ay_revolve_providecb:
 *  provide callback function of revolve object
 */
int
ay_revolve_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 ay_revolve_object *r = NULL;

  if(!o)
    return AY_ENULL;

  r = (ay_revolve_object *) o->refine;

  if(!r)
    return AY_ENULL;

 return ay_provide_nptoolobj(o, type, r->npatch, r->caps_and_bevels, result);
} /* ay_revolve_providecb */


/* ay_revolve_convertcb:
 *  convert callback function of revolve object
 */
int
ay_revolve_convertcb(ay_object *o, int in_place)
{
 ay_revolve_object *r = NULL;

  if(!o)
    return AY_ENULL;

  r = (ay_revolve_object *) o->refine;

  if(!r)
    return AY_ENULL;

 return ay_convert_nptoolobj(o, r->npatch, r->caps_and_bevels, in_place);
} /* ay_revolve_convertcb */


/* ay_revolve_init:
 *  initialize the revolve object module
 */
int
ay_revolve_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_revolve_name,
				    ay_revolve_createcb,
				    ay_revolve_deletecb,
				    ay_revolve_copycb,
				    ay_revolve_drawcb,
				    ay_revolve_drawhcb,
				    ay_revolve_shadecb,
				    ay_revolve_setpropcb,
				    ay_revolve_getpropcb,
				    ay_revolve_getpntcb,
				    ay_revolve_readcb,
				    ay_revolve_writecb,
				    ay_revolve_wribcb,
				    ay_revolve_bbccb,
				    AY_IDREVOLVE);


  ay_status += ay_notify_register(ay_revolve_notifycb, AY_IDREVOLVE);

  ay_status += ay_convert_register(ay_revolve_convertcb, AY_IDREVOLVE);

  ay_status += ay_provide_register(ay_revolve_providecb, AY_IDREVOLVE);

 return ay_status;
} /* ay_revolve_init */

