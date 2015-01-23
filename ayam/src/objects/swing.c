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

/* swing.c - swing object */

static char *ay_swing_name = "Swing";

int ay_swing_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_swing_createcb:
 *  create callback function of swing object
 */
int
ay_swing_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtswing";
 ay_swing_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_swing_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_swing_createcb */


/* ay_swing_deletecb:
 *  delete callback function of swing object
 */
int
ay_swing_deletecb(void *c)
{
 ay_swing_object *swing = NULL;

  if(!c)
    return AY_ENULL;

  swing = (ay_swing_object *)(c);

  if(swing->npatch)
    (void)ay_object_delete(swing->npatch);

  if(swing->caps_and_bevels)
    (void)ay_object_deletemulti(swing->caps_and_bevels, AY_FALSE);

  free(swing);

 return AY_OK;
} /* ay_swing_deletecb */


/* ay_swing_copycb:
 *  copy callback function of swing object
 */
int
ay_swing_copycb(void *src, void **dst)
{
 ay_swing_object *swing = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(swing = malloc(sizeof(ay_swing_object))))
    return AY_EOMEM;

  memcpy(swing, src, sizeof(ay_swing_object));

  swing->npatch = NULL;
  swing->caps_and_bevels = NULL;

  *dst = (void *)swing;

 return AY_OK;
} /* ay_swing_copycb */


/* ay_swing_drawcb:
 *  draw (display in an Ayam view window) callback function of swing object
 */
int
ay_swing_drawcb(struct Togl *togl, ay_object *o)
{
 ay_swing_object *swing = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  swing = (ay_swing_object *)o->refine;

  if(!swing)
    return AY_ENULL;

  if(swing->npatch)
    ay_draw_object(togl, swing->npatch, AY_TRUE);

  if(swing->caps_and_bevels)
    {
      b = swing->caps_and_bevels;
      while(b)
	{
	  ay_draw_object(togl, b, AY_TRUE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_swing_drawcb */


/* ay_swing_shadecb:
 *  shade (display in an Ayam view window) callback function of swing object
 */
int
ay_swing_shadecb(struct Togl *togl, ay_object *o)
{
 ay_swing_object *swing = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  swing = (ay_swing_object *)o->refine;

  if(!swing)
    return AY_ENULL;

  if(swing->npatch)
    ay_shade_object(togl, swing->npatch, AY_FALSE);

  if(swing->caps_and_bevels)
    {
      b = swing->caps_and_bevels;
      while(b)
	{
	  ay_shade_object(togl, b, AY_FALSE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_swing_shadecb */


/* ay_swing_drawhcb:
 *  draw handles (in an Ayam view window) callback function of swing object
 */
int
ay_swing_drawhcb(struct Togl *togl, ay_object *o)
{
 int i;
 double *pnts;
 ay_swing_object *swing;
 ay_nurbpatch_object *patch;

  if(!o)
    return AY_ENULL;

  swing = (ay_swing_object *) o->refine;

  if(!swing)
    return AY_ENULL;

  if(swing->npatch)
    {
      patch = (ay_nurbpatch_object *)swing->npatch->refine;
      pnts = patch->controlv;

      glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
		(GLfloat)ay_prefs.obb);

      glBegin(GL_POINTS);
      for(i = 0; i < patch->width*patch->height; i++)
	{
	  glVertex3dv((GLdouble *)pnts);
	  pnts += 4;
	}
      glEnd();

      glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
		(GLfloat)ay_prefs.seb);
    }

 return AY_OK;
} /* ay_swing_drawhcb */


/* ay_swing_getpntcb:
 *  get point (editing and selection) callback function of swing object
 */
int
ay_swing_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbpatch_object *patch = NULL;
 ay_swing_object *swing = NULL;

  if(!o)
    return AY_ENULL;

  swing = (ay_swing_object *)o->refine;

  if(!swing)
    return AY_ENULL;

  if(swing->npatch)
    {
      patch = (ay_nurbpatch_object *)swing->npatch->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, patch->width*patch->height, 4,
			     patch->controlv);
    }

 return AY_ERROR;
} /* ay_swing_getpntcb */


/* ay_swing_setpropcb:
 *  set property (from Tcl to C context) callback function of swing object
 */
int
ay_swing_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "SwingAttrData";
 /* char fname[] = "swing_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_swing_object *swing = NULL;

  if(!interp || !o)
    return AY_ENULL;

  swing = (ay_swing_object *)o->refine;

  if(!swing)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(swing->display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(swing->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_swing_setpropcb */


/* ay_swing_getpropcb:
 *  get property (from C to Tcl context) callback function of swing object
 */
int
ay_swing_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="SwingAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_swing_object *swing = NULL;

  if(!interp || !o)
    return AY_ENULL;

  swing = (ay_swing_object *)(o->refine);

  if(!swing)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(swing->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(swing->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getnpinfo(interp, n1, swing->npatch);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_swing_getpropcb */


/* ay_swing_readcb:
 *  read (from scene file) callback function of swing object
 */
int
ay_swing_readcb(FILE *fileptr, ay_object *o)
{
 ay_swing_object *swing = NULL;
 int caps[4] = {0};

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(swing = calloc(1, sizeof(ay_swing_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&caps[0]);
  fscanf(fileptr,"%d\n",&caps[1]);
  fscanf(fileptr,"%d\n",&caps[2]);
  fscanf(fileptr,"%d\n",&caps[3]);
  fscanf(fileptr,"%d\n",&swing->display_mode);
  fscanf(fileptr,"%lg\n",&swing->glu_sampling_tolerance);

  if(ay_read_version < 16)
    {
      /* before Ayam 1.21 */
      ay_capt_createtags(o, caps);
    }

  o->refine = swing;

 return AY_OK;
} /* ay_swing_readcb */


/* ay_swing_writecb:
 *  write (to scene file) callback function of swing object
 */
int
ay_swing_writecb(FILE *fileptr, ay_object *o)
{
 ay_swing_object *swing = NULL;
 ay_cparam cparams = {0};
 int caps[4] = {0};

  if(!fileptr || !o)
    return AY_ENULL;

  swing = (ay_swing_object *)(o->refine);

  if(!swing)
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

  fprintf(fileptr, "%d\n", caps[0]);
  fprintf(fileptr, "%d\n", caps[1]);
  fprintf(fileptr, "%d\n", caps[2]);
  fprintf(fileptr, "%d\n", caps[3]);
  fprintf(fileptr, "%d\n", swing->display_mode);
  fprintf(fileptr, "%g\n", swing->glu_sampling_tolerance);

 return AY_OK;
} /* ay_swing_writecb */


/* ay_swing_wribcb:
 *  RIB export callback function of swing object
 */
int
ay_swing_wribcb(char *file, ay_object *o)
{
 ay_swing_object *swing = NULL;
 ay_object *b;

  if(!o)
   return AY_ENULL;

  swing = (ay_swing_object*)o->refine;

  if(!swing)
    return AY_ENULL;

  if(swing->npatch)
    ay_wrib_toolobject(file, swing->npatch, o);

  if(swing->caps_and_bevels)
    {
      b = swing->caps_and_bevels;
      while(b)
	{
	  ay_wrib_toolobject(file, b, o);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_swing_wribcb */


/* ay_swing_bbccb:
 *  bounding box calculation callback function of swing object
 */
int
ay_swing_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_swing_object *swing = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  swing = (ay_swing_object *)o->refine;

  if(!swing)
    return AY_ENULL;

  if(swing->npatch)
    {
      ay_bbc_get(swing->npatch, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return AY_OK;
} /* ay_swing_bbccb */


/* ay_swing_notifycb:
 *  notification callback function of swing object
 */
int
ay_swing_notifycb(ay_object *o)
{
 int ay_status = AY_OK, phase = 0;
 char fname[] = "swing_notify";
 ay_object *cs = NULL, *tr = NULL, *pobject = NULL, *newo = NULL;
 ay_object *bevel, **nextcb;
 ay_swing_object *swing = NULL;
 ay_bparam bparams = {0};
 ay_cparam cparams = {0};
 int provided_cs = AY_FALSE, provided_tr = AY_FALSE, mode = 0;
 double tolerance;

  if(!o)
    return AY_ENULL;

  swing = (ay_swing_object *)(o->refine);

  if(!swing)
    return AY_ENULL;

  mode = swing->display_mode;
  tolerance = swing->glu_sampling_tolerance;

  nextcb = &(swing->caps_and_bevels);

  /* remove old objects */
  if(swing->npatch)
    (void)ay_object_delete(swing->npatch);
  swing->npatch = NULL;

  if(swing->caps_and_bevels)
    {
      (void)ay_object_deletemulti(swing->caps_and_bevels, AY_FALSE);
      swing->caps_and_bevels = NULL;
    }

  /* get curve to swing */
  if(!o->down || !o->down->next)
    goto cleanup;
  cs = o->down;
  if(cs->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(cs, AY_IDNCURVE, &pobject);
      if(!pobject)
	{
	  goto cleanup;
	}
      else
	{
	  cs = pobject;
	  provided_cs = AY_TRUE;
	} /* if */
    } /* if */

  tr = o->down->next;
  if(tr->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(tr, AY_IDNCURVE, &pobject);
      if(!pobject)
	{
	  goto cleanup;
	}
      else
	{
	  tr = pobject;
	  provided_tr = AY_TRUE;
	} /* if */
    } /* if */

  /* swing */
  phase = 1;
  ay_status = ay_npt_createnpatchobject(&newo);

  if(ay_status)
    goto cleanup;

  ay_status = ay_npt_swing(cs, tr,
			  (ay_nurbpatch_object **)(void*)&(newo->refine));

  if(ay_status)
    {
      free(newo);
      goto cleanup;
    }

  swing->npatch = newo;

  /* create bevels&caps */
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
      ay_status = ay_capt_addcaps(&cparams, &bparams, swing->npatch, nextcb);
      if(ay_status)
	goto cleanup;

      while(*nextcb)
	nextcb = &((*nextcb)->next);
    }

  /* create/add bevels */
  if(bparams.has_bevels)
    {
      ay_status = ay_bevelt_addbevels(&bparams, &cparams, swing->npatch,
				      nextcb);
      if(ay_status)
	goto cleanup;
    }

  /* copy sampling tolerance/mode over to new objects */
  ((ay_nurbpatch_object *)swing->npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)swing->npatch->refine)->display_mode =
    mode;

  if(swing->caps_and_bevels)
    {
      bevel = swing->caps_and_bevels;
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
  if(provided_cs && cs)
    {
      (void)ay_object_deletemulti(cs, AY_FALSE);
    }
  if(provided_tr && tr)
    {
      (void)ay_object_deletemulti(tr, AY_FALSE);
    }

  /* recover selected points */
  if(o->selp)
    {
      if(swing->npatch)
	ay_swing_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

  if(ay_status)
    {
      switch(phase)
	{
	case 1:
	  ay_error(AY_ERROR, fname, "Swing failed.");
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
} /* ay_swing_notifycb */


/* ay_swing_providecb:
 *  provide callback function of swing object
 */
int
ay_swing_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 ay_swing_object *s = NULL;

  if(!o)
    return AY_ENULL;

  s = (ay_swing_object *) o->refine;

  if(!s)
    return AY_ENULL;

 return ay_provide_nptoolobj(o, type, s->npatch, s->caps_and_bevels, result);
} /* ay_swing_providecb */


/* ay_swing_convertcb:
 *  convert callback function of swing object
 */
int
ay_swing_convertcb(ay_object *o, int in_place)
{
 ay_swing_object *s = NULL;

  if(!o)
    return AY_ENULL;

  s = (ay_swing_object *) o->refine;

  if(!s)
    return AY_ENULL;

 return ay_convert_nptoolobj(o, s->npatch, s->caps_and_bevels, in_place);
} /* ay_swing_convertcb */


/* ay_swing_init:
 *  initialize the swing object module
 */
int
ay_swing_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_swing_name,
				    ay_swing_createcb,
				    ay_swing_deletecb,
				    ay_swing_copycb,
				    ay_swing_drawcb,
				    ay_swing_drawhcb,
				    ay_swing_shadecb,
				    ay_swing_setpropcb,
				    ay_swing_getpropcb,
				    ay_swing_getpntcb,
				    ay_swing_readcb,
				    ay_swing_writecb,
				    ay_swing_wribcb,
				    ay_swing_bbccb,
				    AY_IDSWING);


  ay_status += ay_notify_register(ay_swing_notifycb, AY_IDSWING);

  ay_status += ay_convert_register(ay_swing_convertcb, AY_IDSWING);

  ay_status += ay_provide_register(ay_swing_providecb, AY_IDSWING);

 return ay_status;
} /* ay_swing_init */

