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

/* birail2.c - birail2 object */

static char *ay_birail2_name = "Birail2";

int ay_birail2_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_birail2_createcb:
 *  create callback function of birail2 object
 */
int
ay_birail2_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtbirail2";
 ay_birail2_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_birail2_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->sections = 0;

  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_birail2_createcb */


/* ay_birail2_deletecb:
 *  delete callback function of birail2 object
 */
int
ay_birail2_deletecb(void *c)
{
 ay_birail2_object *birail2 = NULL;

  if(!c)
    return AY_ENULL;

  birail2 = (ay_birail2_object *)(c);

  if(birail2->npatch)
    (void)ay_object_delete(birail2->npatch);

  if(birail2->caps_and_bevels)
    (void)ay_object_deletemulti(birail2->caps_and_bevels, AY_FALSE);

  free(birail2);

 return AY_OK;
} /* ay_birail2_deletecb */


/* ay_birail2_copycb:
 *  copy callback function of birail2 object
 */
int
ay_birail2_copycb(void *src, void **dst)
{
 ay_birail2_object *birail2 = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(birail2 = malloc(sizeof(ay_birail2_object))))
    return AY_EOMEM;

  memcpy(birail2, src, sizeof(ay_birail2_object));

  birail2->npatch = NULL;
  birail2->caps_and_bevels = NULL;

  *dst = (void *)birail2;

 return AY_OK;
} /* ay_birail2_copycb */


/* ay_birail2_drawcb:
 *  draw (display in an Ayam view window) callback function of birail2 object
 */
int
ay_birail2_drawcb(struct Togl *togl, ay_object *o)
{
 ay_birail2_object *birail2;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  birail2 = (ay_birail2_object *)o->refine;

  if(!birail2)
    return AY_ENULL;

  if(birail2->npatch)
    ay_draw_object(togl, birail2->npatch, AY_TRUE);

  if(birail2->caps_and_bevels)
    {
      b = birail2->caps_and_bevels;
      while(b)
	{
	  ay_draw_object(togl, b, AY_TRUE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_birail2_drawcb */


/* ay_birail2_shadecb:
 *  shade (display in an Ayam view window) callback function of birail2 object
 */
int
ay_birail2_shadecb(struct Togl *togl, ay_object *o)
{
 ay_birail2_object *birail2;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  birail2 = (ay_birail2_object *)o->refine;

  if(!birail2)
    return AY_ENULL;

  if(birail2->npatch)
    ay_shade_object(togl, birail2->npatch, AY_FALSE);

  if(birail2->caps_and_bevels)
    {
      b = birail2->caps_and_bevels;
      while(b)
	{
	  ay_shade_object(togl, b, AY_FALSE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_birail2_shadecb */


/* ay_birail2_drawhcb:
 *  draw handles (in an Ayam view window) callback function of birail2 object
 */
int
ay_birail2_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_birail2_object *birail2;

  if(!o)
    return AY_ENULL;

  birail2 = (ay_birail2_object *) o->refine;

  if(!birail2)
    return AY_ENULL;

  if(birail2->npatch)
    {
      ay_npt_drawrohandles((ay_nurbpatch_object *)birail2->npatch);
    }

 return AY_OK;
} /* ay_birail2_drawhcb */


/* ay_birail2_getpntcb:
 *  get point (editing and selection) callback function of birail2 object
 */
int
ay_birail2_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbpatch_object *patch = NULL;
 ay_birail2_object *birail2 = NULL;

  if(!o)
    return AY_ENULL;

  birail2 = (ay_birail2_object *)o->refine;

  if(!birail2)
    return AY_ENULL;

  if(birail2->npatch)
    {
      patch = (ay_nurbpatch_object *)birail2->npatch->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, patch->width*patch->height, 4,
			     patch->controlv);
    }

 return AY_ERROR;
} /* ay_birail2_getpntcb */


/* ay_birail2_setpropcb:
 *  set property (from Tcl to C context) callback function of birail2 object
 */
int
ay_birail2_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "Birail2AttrData";
 /* char fname[] = "birail2_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_birail2_object *birail2 = NULL;

  if(!interp || !o)
    return AY_ENULL;

  birail2 = (ay_birail2_object *)o->refine;

  if(!birail2)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Close",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail2->close));

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail2->sections));

  Tcl_SetStringObj(ton,"InterpolCtrl",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail2->interpolctrl));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail2->display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(birail2->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_birail2_setpropcb */


/* ay_birail2_getpropcb:
 *  get property (from C to Tcl context) callback function of birail2 object
 */
int
ay_birail2_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="Birail2AttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_birail2_object *birail2 = NULL;

  if(!interp || !o)
    return AY_ENULL;

  birail2 = (ay_birail2_object *)(o->refine);

  if(!birail2)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Close",-1);
  to = Tcl_NewIntObj(birail2->close);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_NewIntObj(birail2->sections);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"InterpolCtrl",-1);
  to = Tcl_NewIntObj(birail2->interpolctrl);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(birail2->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(birail2->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getnpinfo(interp, n1, birail2->npatch);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_birail2_getpropcb */


/* ay_birail2_readcb:
 *  read (from scene file) callback function of birail2 object
 */
int
ay_birail2_readcb(FILE *fileptr, ay_object *o)
{
 ay_birail2_object *birail2 = NULL;
 int caps[4] = {0};

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(birail2 = calloc(1, sizeof(ay_birail2_object))))
    { return AY_EOMEM; }

  fscanf(fileptr, "%d\n", &birail2->close);
  fscanf(fileptr, "%d\n", &birail2->sections);
  fscanf(fileptr, "%d\n", &caps[0]);
  fscanf(fileptr, "%d\n", &caps[1]);
  fscanf(fileptr, "%d\n", &birail2->display_mode);
  fscanf(fileptr, "%lg\n", &birail2->glu_sampling_tolerance);

  if(ay_read_version >= 9)
    {
      /* since 1.10 */
      fscanf(fileptr, "%d\n", &birail2->interpolctrl);
    }

  if(ay_read_version < 16)
    {
      /* before Ayam 1.21 */
      ay_capt_createtags(o, caps);
    }

  o->refine = birail2;

 return AY_OK;
} /* ay_birail2_readcb */


/* ay_birail2_writecb:
 *  write (to scene file) callback function of birail2 object
 */
int
ay_birail2_writecb(FILE *fileptr, ay_object *o)
{
 ay_birail2_object *birail2;
 ay_cparam cparams = {0};
 int caps[2] = {0};

  if(!fileptr || !o)
    return AY_ENULL;

  birail2 = (ay_birail2_object *)(o->refine);

  if(!birail2)
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

  fprintf(fileptr, "%d\n", birail2->close);
  fprintf(fileptr, "%d\n", birail2->sections);
  fprintf(fileptr, "%d\n", caps[0]);
  fprintf(fileptr, "%d\n", caps[1]);
  fprintf(fileptr, "%d\n", birail2->display_mode);
  fprintf(fileptr, "%g\n", birail2->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", birail2->interpolctrl);

 return AY_OK;
} /* ay_birail2_writecb */


/* ay_birail2_wribcb:
 *  RIB export callback function of birail2 object
 */
int
ay_birail2_wribcb(char *file, ay_object *o)
{
 ay_birail2_object *birail2;
 ay_object *b;

  if(!o)
   return AY_ENULL;

  birail2 = (ay_birail2_object*)o->refine;

  if(!birail2)
    return AY_ENULL;

  if(birail2->npatch)
    ay_wrib_toolobject(file, birail2->npatch, o);

  if(birail2->caps_and_bevels)
    {
      b = birail2->caps_and_bevels;
      while(b)
	{
	  ay_wrib_toolobject(file, b, o);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_birail2_wribcb */


/* ay_birail2_bbccb:
 *  bounding box calculation callback function of birail2 object
 */
int
ay_birail2_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_birail2_object *birail2 = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  birail2 = (ay_birail2_object *)o->refine;

  if(!birail2)
    return AY_ENULL;

  if(birail2->npatch)
    {
      ay_bbc_get(birail2->npatch, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return AY_OK;
} /* ay_birail2_bbccb */


/* ay_birail2_notifycb:
 *  notification callback function of birail2 object
 */
int
ay_birail2_notifycb(ay_object *o)
{
 ay_birail2_object *birail2 = NULL;
 ay_object *curve1 = NULL, *curve2 = NULL, *pobject1 = NULL, *pobject2 = NULL;
 ay_object *curve3 = NULL, *curve4 = NULL, *pobject3 = NULL, *pobject4 = NULL;
 ay_object *curve5 = NULL, *pobject5 = NULL;
 ay_object *npatch = NULL, **nextcb;
 ay_object *bevel = NULL;
 ay_bparam bparams = {0};
 ay_cparam cparams = {0};
 int ay_status = AY_OK;
 int is_provided[5] = {0};
 int mode = 0;
 double tolerance;

  if(!o)
    return AY_ENULL;

  birail2 = (ay_birail2_object *)(o->refine);

  if(!birail2)
    return AY_ENULL;

  mode = birail2->display_mode;
  tolerance = birail2->glu_sampling_tolerance;

  nextcb = &(birail2->caps_and_bevels);

  /* remove old objects */
  if(birail2->npatch)
    (void)ay_object_delete(birail2->npatch);
  birail2->npatch = NULL;

  if(birail2->caps_and_bevels)
    {
      (void)ay_object_deletemulti(birail2->caps_and_bevels, AY_FALSE);
      birail2->caps_and_bevels = NULL;
    }

  /* get curves to birail2 */
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

  if(!o->down->next->next->next)
    goto cleanup;

  curve4 = o->down->next->next->next;
  if(curve4->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(curve4, AY_IDNCURVE, &pobject4);
      if(!pobject4)
	{
	  goto cleanup;
	}
      else
	{
	  curve4 = pobject4;
	  is_provided[3] = AY_TRUE;
	} /* if */
    } /* if */

  if(o->down->next->next->next->next)
    {
      curve5 = o->down->next->next->next->next;
      if(curve5->type != AY_IDNCURVE)
	{
	  (void)ay_provide_object(curve5, AY_IDNCURVE, &pobject5);
	  if(pobject5)
	    {
	      curve5 = pobject5;
	      is_provided[4] = AY_TRUE;
	    }
	  else
	    {
	      curve5 = NULL;
	    } /* if */
	} /* if */
    } /* if */

  /* do the birail */
  ay_status = ay_npt_createnpatchobject(&npatch);
  if(ay_status)
    {
      goto cleanup;
    }

  ay_status = ay_npt_birail2(curve1, curve2, curve3, curve4, curve5,
			     birail2->sections, AY_FALSE/*birail2->close*/,
			     birail2->interpolctrl,
			     (ay_nurbpatch_object **)(void*)&(npatch->refine));

  if(ay_status)
    goto cleanup;

  birail2->npatch = npatch;

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
      ay_status = ay_capt_addcaps(&cparams, &bparams, birail2->npatch, nextcb);
      if(ay_status)
	goto cleanup;

      while(*nextcb)
	nextcb = &((*nextcb)->next);
    }

  /* create/add bevels */
  if(bparams.has_bevels)
    {
      ay_status = ay_bevelt_addbevels(&bparams, &cparams, birail2->npatch,
				      nextcb);
      if(ay_status)
	goto cleanup;
    }

  /* copy sampling tolerance/mode attributes over to birail */
  ((ay_nurbpatch_object *)birail2->npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)birail2->npatch->refine)->display_mode =
    mode;

  /* copy sampling tolerance/mode attributes to caps and bevels */
  if(birail2->caps_and_bevels)
    {
      bevel = birail2->caps_and_bevels;
      while(bevel)
	{
	  ((ay_nurbpatch_object *)
	   (bevel->refine))->glu_sampling_tolerance = tolerance;
	  ((ay_nurbpatch_object *)
	   (bevel->refine))->display_mode = mode;
	  bevel = bevel->next;
	}
    }

  /* remove provided objects */
cleanup:
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

  if(is_provided[3])
    {
      (void)ay_object_deletemulti(pobject4, AY_FALSE);
    }

  if(is_provided[4])
    {
      (void)ay_object_deletemulti(pobject5, AY_FALSE);
    }

  if(npatch)
    {
      (void)ay_object_delete(npatch);
    }

  /* recover selected points */
  if(o->selp)
    {
      if(birail2->npatch)
	ay_birail2_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

 return ay_status;
} /* ay_birail2_notifycb */


/* ay_birail2_providecb:
 *  provide callback function of birail2 object
 */
int
ay_birail2_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 ay_birail2_object *b = NULL;

  if(!o)
    return AY_ENULL;

  b = (ay_birail2_object *) o->refine;

  if(!b)
    return AY_ENULL;

 return ay_provide_nptoolobj(o, type, b->npatch, b->caps_and_bevels, result);
} /* ay_birail2_providecb */


/* ay_birail2_convertcb:
 *  convert callback function of birail2 object
 */
int
ay_birail2_convertcb(ay_object *o, int in_place)
{
 ay_birail2_object *b = NULL;

  if(!o)
    return AY_ENULL;

  b = (ay_birail2_object *) o->refine;

  if(!b)
    return AY_ENULL;

 return ay_convert_nptoolobj(o, b->npatch, b->caps_and_bevels, in_place);
} /* ay_birail2_convertcb */


/* ay_birail2_init:
 *  initialize the birail2 object module
 */
int
ay_birail2_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_birail2_name,
				    ay_birail2_createcb,
				    ay_birail2_deletecb,
				    ay_birail2_copycb,
				    ay_birail2_drawcb,
				    ay_birail2_drawhcb,
				    ay_birail2_shadecb,
				    ay_birail2_setpropcb,
				    ay_birail2_getpropcb,
				    ay_birail2_getpntcb,
				    ay_birail2_readcb,
				    ay_birail2_writecb,
				    ay_birail2_wribcb,
				    ay_birail2_bbccb,
				    AY_IDBIRAIL2);


  ay_status += ay_notify_register(ay_birail2_notifycb, AY_IDBIRAIL2);

  ay_status += ay_convert_register(ay_birail2_convertcb, AY_IDBIRAIL2);

  ay_status += ay_provide_register(ay_birail2_providecb, AY_IDBIRAIL2);

 return ay_status;
} /* ay_birail2_init */

