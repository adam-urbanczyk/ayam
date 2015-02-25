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

/* skin.c - skin object */

static char *ay_skin_name = "Skin";

int ay_skin_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_skin_createcb:
 *  create callback function of skin object
 */
int
ay_skin_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtskin";
 ay_skin_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_skin_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->uknot_type = AY_KTNURB;

  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_skin_createcb */


/* ay_skin_deletecb:
 *  delete callback function of skin object
 */
int
ay_skin_deletecb(void *c)
{
 ay_skin_object *skin = NULL;

  if(!c)
    return AY_ENULL;

  skin = (ay_skin_object *)(c);

  if(skin->npatch)
    (void)ay_object_delete(skin->npatch);

  if(skin->caps_and_bevels)
    (void)ay_object_deletemulti(skin->caps_and_bevels, AY_FALSE);

  free(skin);

 return AY_OK;
} /* ay_skin_deletecb */


/* ay_skin_copycb:
 *  copy callback function of skin object
 */
int
ay_skin_copycb(void *src, void **dst)
{
 ay_skin_object *skin = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(skin = malloc(sizeof(ay_skin_object))))
    return AY_EOMEM;

  memcpy(skin, src, sizeof(ay_skin_object));

  skin->npatch = NULL;
  skin->caps_and_bevels = NULL;

  *dst = (void *)skin;

 return AY_OK;
} /* ay_skin_copycb */


/* ay_skin_drawcb:
 *  draw (display in an Ayam view window) callback function of skin object
 */
int
ay_skin_drawcb(struct Togl *togl, ay_object *o)
{
 ay_skin_object *skin;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  skin = (ay_skin_object *)o->refine;

  if(!skin)
    return AY_ENULL;

  if(skin->npatch)
    ay_draw_object(togl, skin->npatch, AY_TRUE);

  if(skin->caps_and_bevels)
    {
      b = skin->caps_and_bevels;
      while(b)
	{
	  ay_draw_object(togl, b, AY_TRUE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_skin_drawcb */


/* ay_skin_shadecb:
 *  shade (display in an Ayam view window) callback function of skin object
 */
int
ay_skin_shadecb(struct Togl *togl, ay_object *o)
{
 ay_skin_object *skin;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  skin = (ay_skin_object *)o->refine;

  if(!skin)
    return AY_ENULL;

  if(skin->npatch)
    ay_shade_object(togl, skin->npatch, AY_FALSE);

  if(skin->caps_and_bevels)
    {
      b = skin->caps_and_bevels;
      while(b)
	{
	  ay_shade_object(togl, b, AY_FALSE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_skin_shadecb */


/* ay_skin_drawhcb:
 *  draw handles (in an Ayam view window) callback function of skin object
 */
int
ay_skin_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_skin_object *skin;

  if(!o)
    return AY_ENULL;

  skin = (ay_skin_object *) o->refine;

  if(!skin)
    return AY_ENULL;

  if(skin->npatch)
    {
      ay_npt_drawrohandles((ay_nurbpatch_object *)skin->npatch);
    }

 return AY_OK;
} /* ay_skin_drawhcb */


/* ay_skin_getpntcb:
 *  get point (editing and selection) callback function of skin object
 */
int
ay_skin_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbpatch_object *patch = NULL;
 ay_skin_object *skin = NULL;

  if(!o)
    return AY_ENULL;

  skin = (ay_skin_object *)o->refine;

  if(!skin)
    return AY_ENULL;

  if(skin->npatch)
    {
      patch = (ay_nurbpatch_object *)skin->npatch->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, patch->width*patch->height, 4,
			     patch->controlv);
    }

 return AY_ERROR;
} /* ay_skin_getpntcb */


/* ay_skin_setpropcb:
 *  set property (from Tcl to C context) callback function of skin object
 */
int
ay_skin_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "SkinAttrData";
 /* char fname[] = "skin_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_skin_object *skin = NULL;

  if(!interp || !o)
    return AY_ENULL;

  skin = (ay_skin_object *)o->refine;

  if(!skin)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Interpolate",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(skin->interpolate));

  Tcl_SetStringObj(ton,"Order_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(skin->uorder));

  Tcl_SetStringObj(ton,"Knot-Type_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(skin->uknot_type));


  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(skin->display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(skin->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_skin_setpropcb */


/* ay_skin_getpropcb:
 *  get property (from C to Tcl context) callback function of skin object
 */
int
ay_skin_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="SkinAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_skin_object *skin = NULL;

  if(!interp || !o)
    return AY_ENULL;

  skin = (ay_skin_object *)(o->refine);

  if(!skin)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Interpolate",-1);
  to = Tcl_NewIntObj(skin->interpolate);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order_U",-1);
  to = Tcl_NewIntObj(skin->uorder);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Knot-Type_U",-1);
  to = Tcl_NewIntObj(skin->uknot_type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(skin->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(skin->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getnpinfo(interp, n1, skin->npatch);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_skin_getpropcb */


/* ay_skin_readcb:
 *  read (from scene file) callback function of skin object
 */
int
ay_skin_readcb(FILE *fileptr, ay_object *o)
{
 ay_skin_object *skin = NULL;
 int caps[4] = {0};

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(skin = calloc(1, sizeof(ay_skin_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&skin->interpolate);
  fscanf(fileptr,"%d\n",&skin->uorder);
  fscanf(fileptr,"%d\n",&skin->uknot_type);
  fscanf(fileptr,"%d\n",&caps[0]);
  fscanf(fileptr,"%d\n",&caps[1]);
  fscanf(fileptr,"%d\n",&skin->display_mode);
  fscanf(fileptr,"%lg\n",&skin->glu_sampling_tolerance);

  if(ay_read_version < 16)
    {
      /* before Ayam 1.21 */
      ay_capt_createtags(o, caps);
    }

  o->refine = skin;

 return AY_OK;
} /* ay_skin_readcb */


/* ay_skin_writecb:
 *  write (to scene file) callback function of skin object
 */
int
ay_skin_writecb(FILE *fileptr, ay_object *o)
{
 ay_skin_object *skin = NULL;
 ay_cparam cparams = {0};
 int caps[2] = {0};

  if(!fileptr || !o)
    return AY_ENULL;

  skin = (ay_skin_object *)(o->refine);

  if(!skin)
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

  fprintf(fileptr, "%d\n", skin->interpolate);
  fprintf(fileptr, "%d\n", skin->uorder);
  fprintf(fileptr, "%d\n", skin->uknot_type);
  fprintf(fileptr, "%d\n", caps[0]);
  fprintf(fileptr, "%d\n", caps[1]);
  fprintf(fileptr, "%d\n", skin->display_mode);
  fprintf(fileptr, "%g\n", skin->glu_sampling_tolerance);

 return AY_OK;
} /* ay_skin_writecb */


/* ay_skin_wribcb:
 *  RIB export callback function of skin object
 */
int
ay_skin_wribcb(char *file, ay_object *o)
{
 ay_skin_object *skin;
 ay_object *b;

  if(!o)
   return AY_ENULL;

  skin = (ay_skin_object*)o->refine;

  if(!skin)
    return AY_ENULL;

  if(skin->npatch)
    ay_wrib_object(file, skin->npatch);

  if(skin->caps_and_bevels)
    {
      b = skin->caps_and_bevels;
      while(b)
	{
	  ay_wrib_object(file, b);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_skin_wribcb */


/* ay_skin_bbccb:
 *  bounding box calculation callback function of skin object
 */
int
ay_skin_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_skin_object *skin = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  skin = (ay_skin_object *)o->refine;

  if(!skin)
    return AY_ENULL;

  if(skin->npatch)
    {
      ay_bbc_get(skin->npatch, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return AY_OK;
} /* ay_skin_bbccb */


/* ay_skin_notifycb:
 *  notification callback function of skin object
 */
int
ay_skin_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *curve = NULL;
 ay_skin_object *skin = NULL;
 ay_object *down = NULL, *c = NULL, *last = NULL, *all_curves = NULL;
 ay_object *newo = NULL, **nextcb = NULL;
 ay_object *bevel = NULL;
 ay_bparam bparams = {0};
 ay_cparam cparams = {0};
 double m[16] = {0}, tolerance;
 int mode = 0, count = 0, i, a;

  if(!o)
    return AY_ENULL;

  skin = (ay_skin_object *)(o->refine);

  if(!skin)
    return AY_ENULL;

  mode = skin->display_mode;
  tolerance = skin->glu_sampling_tolerance;

  nextcb = &(skin->caps_and_bevels);

  /* remove old objects */
  if(skin->npatch)
    (void)ay_object_delete(skin->npatch);
  skin->npatch = NULL;

  if(skin->caps_and_bevels)
    {
      (void)ay_object_deletemulti(skin->caps_and_bevels, AY_FALSE);
      skin->caps_and_bevels = NULL;
    }

  /* get curves to skin */
  if(!o->down)
    goto cleanup;
  down = o->down;

  while(down->next)
    {
      c = NULL;

      if(down->type == AY_IDNCURVE)
	{
	  ay_object_copy(down, &c);
	}
      else
	{
	  ay_provide_object(down, AY_IDNCURVE, &c);
	} /* if */

      if(c)
	{
	  /* link curve to list */
	  if(last)
	    last->next = c;
	  else
	    all_curves = c;

	  /*if(skin->use_multiple_provided_objects)*/
	  while(c->next)
	    {
	      count++;
	      c = c->next;
	    }

	  last = c;

	  count++;
	} /* if */

      down = down->next;
    } /* while */

  /* No curves to skin? */
  if(count <= 1)
    {
      goto cleanup;
    }

  /* do the skin */
  ay_status = ay_npt_createnpatchobject(&newo);
  if(ay_status)
    {
      goto cleanup;
    }

  c = all_curves;
  while(c)
    {
      /* apply transformation attributes to control points */
      if(AY_ISTRAFO(c))
	{
	  ay_trafo_creatematrix(c, m);
	  curve = (ay_nurbcurve_object *)c->refine;
	  a = 0;
	  for(i = 0; i < curve->length; i++)
	    {
	      ay_trafo_apply3(&(curve->controlv[a]), m);
	      a += 4;
	    }
	} /* if */

      c = c->next;
    } /* while */

  ay_status = ay_npt_skinu(all_curves, skin->uorder, skin->uknot_type,
			   skin->interpolate,
			   (ay_nurbpatch_object **)(void*)&(newo->refine));

  if(ay_status)
    {
      free(newo);
      goto cleanup;
    }

  skin->npatch = newo;

  /* get bevel and cap parameters */
  if(o->tags)
    {
      ay_bevelt_parsetags(o->tags, &bparams);
      ay_capt_parsetags(o->tags, &cparams);
    }

  /* create/add caps */
  if(cparams.has_caps)
    {
      ay_status = ay_capt_addcaps(&cparams, &bparams, skin->npatch, nextcb);
      if(ay_status)
	goto cleanup;

      while(*nextcb)
	nextcb = &((*nextcb)->next);
    }

  /* create/add bevels */
  if(bparams.has_bevels)
    {
      ay_status = ay_bevelt_addbevels(&bparams, &cparams, skin->npatch, nextcb);
      if(ay_status)
	goto cleanup;
    }

  /* copy sampling tolerance/mode over to new objects */
  ((ay_nurbpatch_object *)skin->npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)skin->npatch->refine)->display_mode =
    mode;

  if(skin->caps_and_bevels)
    {
      bevel = skin->caps_and_bevels;
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
  /* remove temporary curves */
  (void)ay_object_deletemulti(all_curves, AY_FALSE);

  /* recover selected points */
  if(o->selp)
    {
      if(skin->npatch)
	ay_skin_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

 return ay_status;
} /* ay_skin_notifycb */


/* ay_skin_providecb:
 *  provide callback function of skin object
 */
int
ay_skin_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 ay_skin_object *s = NULL;

  if(!o)
    return AY_ENULL;

  s = (ay_skin_object *) o->refine;

  if(!s)
    return AY_ENULL;

 return ay_provide_nptoolobj(o, type, s->npatch, s->caps_and_bevels, result);
} /* ay_skin_providecb */


/* ay_skin_convertcb:
 *  convert callback function of skin object
 */
int
ay_skin_convertcb(ay_object *o, int in_place)
{
 ay_skin_object *s = NULL;

  if(!o)
    return AY_ENULL;

  s = (ay_skin_object *) o->refine;

  if(!s)
    return AY_ENULL;

 return ay_convert_nptoolobj(o, s->npatch, s->caps_and_bevels, in_place);
} /* ay_skin_convertcb */


/* ay_skin_init:
 *  initialize the skin object module
 */
int
ay_skin_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_skin_name,
				    ay_skin_createcb,
				    ay_skin_deletecb,
				    ay_skin_copycb,
				    ay_skin_drawcb,
				    ay_skin_drawhcb,
				    ay_skin_shadecb,
				    ay_skin_setpropcb,
				    ay_skin_getpropcb,
				    ay_skin_getpntcb,
				    ay_skin_readcb,
				    ay_skin_writecb,
				    ay_skin_wribcb,
				    ay_skin_bbccb,
				    AY_IDSKIN);


  ay_status += ay_notify_register(ay_skin_notifycb, AY_IDSKIN);

  ay_status += ay_convert_register(ay_skin_convertcb, AY_IDSKIN);

  ay_status += ay_provide_register(ay_skin_providecb, AY_IDSKIN);

 return ay_status;
} /* ay_skin_init */

