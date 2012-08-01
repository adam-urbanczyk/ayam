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

/* bevel.c - bevel object */

static char *ay_bevel_name = "Bevel";

int ay_bevel_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_bevel_createcb:
 *  create callback function of bevel object
 */
int
ay_bevel_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "crtbevel";
 ay_bevel_object *new = NULL;
 ay_tag bt = {0};
 char tagname[] = "BP", tagval[] = "0,0,0.1,0";

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_bevel_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  o->parent = AY_TRUE;
  o->refine = new;

  bt.type = ay_bp_tagtype;
  bt.name = tagname;
  bt.val = tagval;

  /* copy the tag over to the object */
  bt.next = o->tags;
  ay_status = ay_tags_copy(&bt, &(o->tags));

 return ay_status;
} /* ay_bevel_createcb */


/* ay_bevel_deletecb:
 *  delete callback function of bevel object
 */
int
ay_bevel_deletecb(void *c)
{
 ay_bevel_object *bevel = NULL;

  if(!c)
    return AY_ENULL;

  bevel = (ay_bevel_object *)(c);

  if(bevel->npatch)
    ay_object_delete(bevel->npatch);

  if(bevel->caps)
    {
      ay_object_deletemulti(bevel->caps);
      bevel->caps = NULL;
    }

  free(bevel);

 return AY_OK;
} /* ay_bevel_deletecb */


/* ay_bevel_copycb:
 *  copy callback function of bevel object
 */
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

  if(bevelsrc->npatch)
    ay_object_copymulti(bevelsrc->npatch, &(bevel->npatch));

  bevel->caps = NULL;

  if(bevelsrc->caps)
    ay_object_copymulti(bevelsrc->caps, &(bevel->caps));

  *dst = (void *)bevel;

 return AY_OK;
} /* ay_bevel_copycb */


/* ay_bevel_drawcb:
 *  draw (display in an Ayam view window) callback function of bevel object
 */
int
ay_bevel_drawcb(struct Togl *togl, ay_object *o)
{
 ay_bevel_object *bevel = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  bevel = (ay_bevel_object *)o->refine;

  if(!bevel)
    return AY_ENULL;

  if(bevel->npatch)
    ay_draw_object(togl, bevel->npatch, AY_TRUE);

  b = bevel->caps;
  while(b)
    {
      ay_draw_object(togl, b, AY_TRUE);
      b = b->next;
    }

 return AY_OK;
} /* ay_bevel_drawcb */


/* ay_bevel_shadecb:
 *  shade (display in an Ayam view window) callback function of bevel object
 */
int
ay_bevel_shadecb(struct Togl *togl, ay_object *o)
{
 ay_bevel_object *bevel = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  bevel = (ay_bevel_object *)o->refine;

  if(!bevel)
    return AY_ENULL;

  if(bevel->npatch)
    ay_shade_object(togl, bevel->npatch, AY_FALSE);

  b = bevel->caps;
  while(b)
    {
      ay_shade_object(togl, b, AY_FALSE);
      b = b->next;
    }

 return AY_OK;
} /* ay_bevel_shadecb */


/* ay_bevel_drawhcb:
 *  draw handles (in an Ayam view window) callback function of bevel object
 */
int
ay_bevel_drawhcb(struct Togl *togl, ay_object *o)
{
 int i = 0, a = 0;
 ay_bevel_object *bevel = NULL;
 double *pnts = NULL;
 double point_size = ay_prefs.handle_size;
 ay_nurbpatch_object *patch = NULL;

  if(!o)
    return AY_ENULL;

  bevel = (ay_bevel_object *) o->refine;

  if(bevel->npatch)
    {
      patch = (ay_nurbpatch_object *)bevel->npatch->refine;
      pnts = patch->controlv;
      glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
		(GLfloat)ay_prefs.obb);

      glPointSize((GLfloat)point_size);

      glBegin(GL_POINTS);
      for(i = 0; i < patch->width*patch->height; i++)
	{
	  glVertex3dv((GLdouble *)&pnts[a]);
	  a += 4;
	}
      glEnd();

      glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
		(GLfloat)ay_prefs.seb);
    }

 return AY_OK;
} /* ay_bevel_drawhcb */


/* ay_bevel_getpntcb:
 *  get point (editing and selection) callback function of bevel object
 */
int
ay_bevel_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbpatch_object *patch = NULL;
 ay_bevel_object *bevel = NULL;

  if(!o)
    return AY_ENULL;

  bevel = (ay_bevel_object *)o->refine;

  if(bevel->npatch)
    {
      patch = (ay_nurbpatch_object *)bevel->npatch->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, patch->width*patch->height, 4,
			     patch->controlv);
    }

 return AY_ERROR;
} /* ay_bevel_getpntcb */


/* ay_bevel_setpropcb:
 *  set property (from Tcl to C context) callback function of bevel object
 */
int
ay_bevel_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
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
  Tcl_GetIntFromObj(interp,to, &(bevel->display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(bevel->glu_sampling_tolerance));

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(bevel->has_start_cap));

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(bevel->has_end_cap));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_notify_object(o);

  o->modified = AY_TRUE;
  ay_notify_parent();

 return AY_OK;
} /* ay_bevel_setpropcb */


/* ay_bevel_getpropcb:
 *  get property (from C to Tcl context) callback function of bevel object
 */
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
  to = Tcl_NewIntObj(bevel->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(bevel->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_NewIntObj(bevel->has_start_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_NewIntObj(bevel->has_end_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_bevel_getpropcb */


/* ay_bevel_readcb:
 *  read (from scene file) callback function of bevel object
 */
int
ay_bevel_readcb(FILE *fileptr, ay_object *o)
{
 ay_bevel_object *bevel = NULL;

 if(!o)
   return AY_ENULL;

  if(!(bevel = calloc(1, sizeof(ay_bevel_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&bevel->display_mode);
  fscanf(fileptr,"%lg\n",&bevel->glu_sampling_tolerance);

  if(ay_read_version >= 16)
    {
      /* Since Ayam 1.21 */
      fscanf(fileptr,"%d\n",&bevel->has_start_cap);
      fscanf(fileptr,"%d\n",&bevel->has_end_cap);
    }

  o->refine = bevel;

 return AY_OK;
} /* ay_bevel_readcb */


/* ay_bevel_writecb:
 *  write (to scene file) callback function of bevel object
 */
int
ay_bevel_writecb(FILE *fileptr, ay_object *o)
{
 ay_bevel_object *bevel = NULL;

  if(!o)
    return AY_ENULL;

  bevel = (ay_bevel_object *)(o->refine);

  fprintf(fileptr, "%d\n", bevel->display_mode);
  fprintf(fileptr, "%g\n", bevel->glu_sampling_tolerance);

  fprintf(fileptr, "%d\n", bevel->has_start_cap);
  fprintf(fileptr, "%d\n", bevel->has_end_cap);

 return AY_OK;
} /* ay_bevel_writecb */


/* ay_bevel_wribcb:
 *  RIB export callback function of bevel object
 */
int
ay_bevel_wribcb(char *file, ay_object *o)
{
 ay_bevel_object *bevel;
 ay_object *b;

  if(!o)
   return AY_ENULL;

  bevel = (ay_bevel_object*)o->refine;

  if(bevel->npatch)
    ay_wrib_toolobject(file, bevel->npatch, o);

  b = bevel->caps;
  while(b)
    {
      ay_wrib_object(file, b);
      b = b->next;
    }

 return AY_OK;
} /* ay_bevel_wribcb */


/* ay_bevel_bbccb:
 *  bounding box calculation callback function of bevel object
 */
int
ay_bevel_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_bevel_object *bevel = NULL;

  if(!o || !bbox || !flags)
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


/* ay_bevel_notifycb:
 *  notification callback function of bevel object
 */
int
ay_bevel_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 int mode = 0;
 ay_bevel_object *bevel = NULL;
 ay_object *npatch = NULL, *curve = NULL, *bcurve = NULL, *t = NULL;
 ay_object *pobject1 = NULL, *pobject2 = NULL, **nextcb;
 ay_bparam bparams = {0};
 int align = AY_FALSE, has_b = AY_FALSE;
 int b_type, b_sense;
 int caps[4] = {0};
 double b_radius, tolerance;

  if(!o)
    return AY_ENULL;

  bevel = (ay_bevel_object *)(o->refine);

  mode = bevel->display_mode;
  tolerance = bevel->glu_sampling_tolerance;

  nextcb = &(bevel->caps);

  /* remove old objects */
  if(bevel->npatch)
    ay_object_delete(bevel->npatch);
  bevel->npatch = NULL;

  if(bevel->caps)
    {
      ay_object_deletemulti(bevel->caps);
      bevel->caps = NULL;
    }

  /* get curve to bevel */
  if(!o->down)
    return AY_OK;

  curve = o->down;
  if(curve->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(curve, AY_IDNCURVE, &pobject1);
      if(!pobject1)
	{
	  return AY_OK;
	}
      else
	{
	  curve = pobject1;
	} /* if */
    }
  else
    {
      ay_status = ay_object_copy(curve, &t);
      if(!t)
	goto cleanup;

      curve = t;
    } /* if */

  if(!curve)
    {
      ay_status = AY_OK;
      goto cleanup;
    }

  /* see if there is a bevel curve */
  if(o->down->next)
    {
      if(o->down->next->type == AY_IDNCURVE)
	{
	  bcurve = o->down->next;
	}
      else
	{
	  ay_status = ay_provide_object(o->down->next, AY_IDNCURVE, &pobject2);
	  /*XXXX report error?*/
	  bcurve = pobject2;
	}
    }

  /* check curves rotation attributes; only allow align operation,
     if curve is not rotated, because otherwise the transformations
     would not be treated correctly in ay_npt_bevel() */
  if((curve->quat[0] == 0.0) && (curve->quat[1] == 0.0) &&
     (curve->quat[2] == 0.0) && (curve->quat[3] == 1.0))
    {
      align = AY_TRUE;
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
      ay_nct_revert((ay_nurbcurve_object*)curve->refine);
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

  if(!bcurve && (b_type < 0))
    {
      ay_bevelt_findbevelcurve(-b_type, &bcurve);
    }

  if(!bcurve)
    {
      ay_status = ay_bevelt_create(b_type, b_radius, align, curve,
			   (ay_nurbpatch_object**)(void*)&(npatch->refine));
    }
  else
    {
      ay_status = ay_bevelt_createc(b_radius, curve, bcurve,
			   (ay_nurbpatch_object**)(void*)&(npatch->refine));
    }
  if(ay_status)
    goto cleanup;

  /* create/add caps */
  caps[2] = bevel->has_start_cap;
  caps[3] = bevel->has_end_cap;

  ay_status = ay_capt_addcaps(caps, &bparams, npatch, nextcb);
  if(ay_status)
    goto cleanup;

  /* copy sampling tolerance/mode over to new objects */
  ((ay_nurbpatch_object *)npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)npatch->refine)->display_mode =
    mode;

  /* copy transformations, if align was not used */
  if(!align)
    {
      ay_trafo_copy(curve, npatch);
    }

  bevel->npatch = npatch;
  npatch = NULL;

cleanup:
  /* remove provided objects */
  if(pobject1)
    {
      ay_object_deletemulti(pobject1);
    }
  if(pobject2)
    {
      ay_object_deletemulti(pobject2);
    }

  if(t)
    {
      ay_object_delete(t);
    }

  if(npatch)
    {
      ay_object_delete(npatch);
    }

  /* recover selected points */
  if(o->selp)
    {
      ay_bevel_getpntcb(3, o, NULL, NULL);
    }

 return ay_status;
} /* ay_bevel_notifycb */


/* ay_bevel_providecb:
 *  provide callback function of bevel object
 */
int
ay_bevel_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 ay_bevel_object *b = NULL;

  if(!o)
    return AY_ENULL;

  b = (ay_bevel_object *) o->refine;

  return ay_provide_nptoolobj(o, type, b->npatch, b->caps, result);
} /* ay_bevel_providecb */


/* ay_bevel_convertcb:
 *  convert callback function of bevel object
 */
int
ay_bevel_convertcb(ay_object *o, int in_place)
{
 ay_bevel_object *b = NULL;

  if(!o)
    return AY_ENULL;

  b = (ay_bevel_object *) o->refine;

 return ay_convert_nptoolobj(o, b->npatch, b->caps, in_place);
} /* ay_bevel_convertcb */


/* ay_bevel_init:
 *  initialize the bevel object module
 */
int
ay_bevel_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_bevel_name,
				    ay_bevel_createcb,
				    ay_bevel_deletecb,
				    ay_bevel_copycb,
				    ay_bevel_drawcb,
				    ay_bevel_drawhcb,
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

