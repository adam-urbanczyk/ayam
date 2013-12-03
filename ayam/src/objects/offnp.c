/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2010 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* offnp.c - OffsetNP (offset NURBS patch) object */

static char *ay_offnp_name = "OffsetNP";

int ay_offnp_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_offnp_createcb:
 *  create callback function of offnp object
 */
int
ay_offnp_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtoffnp";
 ay_offnp_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_offnp_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  o->hide_children = AY_TRUE;
  o->parent = AY_TRUE;
  o->refine = new;

  new->offset = 0.1;

 return AY_OK;
} /* ay_offnp_createcb */


/* ay_offnp_deletecb:
 *  delete callback function of offnp object
 */
int
ay_offnp_deletecb(void *c)
{
 ay_offnp_object *offnp = NULL;

  if(!c)
    return AY_ENULL;

  offnp = (ay_offnp_object *)(c);

  if(offnp->npatch)
    (void)ay_object_delete(offnp->npatch);

  if(offnp->caps_and_bevels)
    (void)ay_object_deletemulti(offnp->caps_and_bevels, AY_FALSE);

  free(offnp);

 return AY_OK;
} /* ay_offnp_deletecb */


/* ay_offnp_copycb:
 *  copy callback function of offnp object
 */
int
ay_offnp_copycb(void *src, void **dst)
{
 ay_offnp_object *offnp = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(offnp = malloc(sizeof(ay_offnp_object))))
    return AY_EOMEM;

  memcpy(offnp, src, sizeof(ay_offnp_object));

  offnp->npatch = NULL;
  offnp->caps_and_bevels = NULL;

  *dst = (void *)offnp;

 return AY_OK;
} /* ay_offnp_copycb */


/* ay_offnp_drawcb:
 *  draw (display in an Ayam view window) callback function of offnp object
 */
int
ay_offnp_drawcb(struct Togl *togl, ay_object *o)
{
 ay_offnp_object *offnp = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  offnp = (ay_offnp_object *)o->refine;

  if(!offnp)
    return AY_ENULL;

  if(offnp->npatch)
    ay_draw_object(togl, offnp->npatch, AY_TRUE);

  if(offnp->caps_and_bevels)
    {
      b = offnp->caps_and_bevels;
      while(b)
	{
	  ay_draw_object(togl, b, AY_TRUE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_offnp_drawcb */


/* ay_offnp_shadecb:
 *  shade (display in an Ayam view window) callback function of offnp object
 */
int
ay_offnp_shadecb(struct Togl *togl, ay_object *o)
{
 ay_offnp_object *offnp = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  offnp = (ay_offnp_object *)o->refine;

  if(!offnp)
    return AY_ENULL;

  if(offnp->npatch)
    ay_shade_object(togl, offnp->npatch, AY_FALSE);

  if(offnp->caps_and_bevels)
    {
      b = offnp->caps_and_bevels;
      while(b)
	{
	  ay_shade_object(togl, b, AY_FALSE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_offnp_shadecb */


/* ay_offnp_drawacb:
 *  draw annotations (in an Ayam view window) callback function of offnp object
 */
int
ay_offnp_drawacb(struct Togl *togl, ay_object *o)
{
 ay_object *c = NULL;
 ay_offnp_object *offnp = NULL;
 ay_nurbpatch_object *np = NULL;
 double m[16];

  if(!o)
    return AY_ENULL;

  offnp = (ay_offnp_object *)o->refine;

  if(!offnp)
    return AY_ENULL;

  if(offnp->npatch)
    {
      /* get NURBS patch */
      np = (ay_nurbpatch_object *)offnp->npatch->refine;

      /* draw arrow */
      glPushMatrix();
       c = offnp->npatch;
       glTranslated((GLdouble)c->movx, (GLdouble)c->movy,
		    (GLdouble)c->movz);
       ay_quat_torotmatrix(c->quat, m);
       glMultMatrixd((GLdouble*)m);
       glScaled((GLdouble)c->scalx, (GLdouble)c->scaly,
		(GLdouble)c->scalz);

       ay_draw_arrow(togl, &(np->controlv[np->width*np->height*4-8]),
		     &(np->controlv[np->width*np->height*4-4]));
      glPopMatrix();
    }

 return AY_OK;
} /* ay_offnp_drawacb */


/* ay_offnp_drawhcb:
 *  draw handles (in an Ayam view window) callback function of offnp object
 */
int
ay_offnp_drawhcb(struct Togl *togl, ay_object *o)
{
 int i;
 double *pnts, m[16];
 ay_object *c;
 ay_offnp_object *offnp;
 ay_nurbpatch_object *np;

  if(!o)
    return AY_ENULL;

  offnp = (ay_offnp_object *) o->refine;

  if(!offnp)
    return AY_ENULL;

  if(offnp->npatch)
    {
      /* get NURBS patch */
      np = (ay_nurbpatch_object *)offnp->npatch->refine;
      pnts = np->controlv;

      glPushMatrix();
       c = offnp->npatch;
       glTranslated((GLdouble)c->movx, (GLdouble)c->movy,
		    (GLdouble)c->movz);
       ay_quat_torotmatrix(c->quat, m);
       glMultMatrixd((GLdouble*)m);
       glScaled((GLdouble)c->scalx, (GLdouble)c->scaly,
		(GLdouble)c->scalz);

       /* draw read only points */
       glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
		 (GLfloat)ay_prefs.obb);

       glBegin(GL_POINTS);
        for(i = 0; i < np->width*np->height; i++)
	  {
	    glVertex3dv((GLdouble *)pnts);
	    pnts += 4;
	  }
	glEnd();

	glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
		  (GLfloat)ay_prefs.seb);
      glPopMatrix();
    }

 return AY_OK;
} /* ay_offnp_drawhcb */


/* ay_offnp_getpntcb:
 *  get point (editing and selection) callback function of offnp object
 */
int
ay_offnp_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbpatch_object *patch = NULL;
 ay_offnp_object *offnp = NULL;

  if(!o)
    return AY_ENULL;

  offnp = (ay_offnp_object *)o->refine;

  if(!offnp)
    return AY_ENULL;

  if(offnp->npatch)
    {
      patch = (ay_nurbpatch_object *)offnp->npatch->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, patch->width*patch->height, 4,
			     patch->controlv);
    }

 return AY_ERROR;
} /* ay_offnp_getpntcb */


/* ay_offnp_setpropcb:
 *  set property (from Tcl to C context) callback function of offnp object
 */
int
ay_offnp_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "OffsetNPAttrData";
 /* char fname[] = "offnp_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_offnp_object *offnp = NULL;

  if(!interp || !o)
    return AY_ENULL;

  offnp = (ay_offnp_object *)o->refine;

  if(!offnp)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Mode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(offnp->mode));

  Tcl_SetStringObj(ton,"Offset",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(offnp->offset));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(offnp->display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(offnp->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_offnp_setpropcb */


/* ay_offnp_getpropcb:
 *  get property (from C to Tcl context) callback function of offnp object
 */
int
ay_offnp_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="OffsetNPAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_offnp_object *offnp = NULL;

  if(!interp || !o)
    return AY_ENULL;

  offnp = (ay_offnp_object *)(o->refine);

  if(!offnp)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Mode",-1);
  to = Tcl_NewIntObj(offnp->mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Offset",-1);
  to = Tcl_NewDoubleObj(offnp->offset);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(offnp->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(offnp->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getnpinfo(interp, n1, offnp->npatch);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_offnp_getpropcb */


/* ay_offnp_readcb:
 *  read (from scene file) callback function of offnp object
 */
int
ay_offnp_readcb(FILE *fileptr, ay_object *o)
{
 ay_offnp_object *offnp = NULL;

 if(!fileptr || !o)
   return AY_ENULL;

  if(!(offnp = calloc(1, sizeof(ay_offnp_object))))
    { return AY_EOMEM; }

  fscanf(fileptr, "%d\n", &offnp->mode);
  fscanf(fileptr, "%lg\n", &offnp->offset);
  fscanf(fileptr, "%d\n", &offnp->display_mode);
  fscanf(fileptr, "%lg\n", &offnp->glu_sampling_tolerance);

  o->refine = offnp;

 return AY_OK;
} /* ay_offnp_readcb */


/* ay_offnp_writecb:
 *  write (to scene file) callback function of offnp object
 */
int
ay_offnp_writecb(FILE *fileptr, ay_object *o)
{
 ay_offnp_object *offnp = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  offnp = (ay_offnp_object *)(o->refine);

  if(!offnp)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", offnp->mode);
  fprintf(fileptr, "%g\n", offnp->offset);
  fprintf(fileptr, "%d\n", offnp->display_mode);
  fprintf(fileptr, "%g\n", offnp->glu_sampling_tolerance);

 return AY_OK;
} /* ay_offnp_writecb */


/* ay_offnp_wribcb:
 *  RIB export callback function of offnp object
 */
int
ay_offnp_wribcb(char *file, ay_object *o)
{
 ay_offnp_object *offnp = NULL;
 ay_object *b;

  if(!o)
   return AY_ENULL;

  offnp = (ay_offnp_object*)o->refine;

  if(!offnp)
    return AY_ENULL;

  if(offnp->npatch)
    ay_wrib_toolobject(file, offnp->npatch, o);

  b = offnp->caps_and_bevels;
  while(b)
    {
      ay_wrib_object(file, b);
      b = b->next;
    }

 return AY_OK;
} /* ay_offnp_wribcb */


/* ay_offnp_bbccb:
 *  bounding box calculation callback function of offnp object
 */
int
ay_offnp_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_offnp_object *offnp = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  offnp = (ay_offnp_object *)o->refine;

  if(!offnp)
    return AY_ENULL;

  if(offnp->npatch)
    {
      ay_bbc_get(offnp->npatch, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    } /* if */

 return AY_OK;
} /* ay_offnp_bbccb */


/* ay_offnp_notifycb:
 *  notification callback function of offnp object
 */
int
ay_offnp_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_offnp_object *offnp = NULL;
 ay_object *down = NULL, *npatch = NULL, *newo = NULL;
 ay_object *bevel = NULL, **nextcb;
 ay_bparam bparams = {0};
 ay_cparam cparams = {0};
 int mode, provided = AY_FALSE;
 double tolerance;

  if(!o)
    return AY_ENULL;

  offnp = (ay_offnp_object *)(o->refine);

  if(!offnp)
    return AY_ENULL;

  mode = offnp->display_mode;
  tolerance = offnp->glu_sampling_tolerance;

  nextcb = &(offnp->caps_and_bevels);

  /* remove old objects */
  if(offnp->npatch)
    (void)ay_object_delete(offnp->npatch);
  offnp->npatch = NULL;

  if(offnp->caps_and_bevels)
    {
      (void)ay_object_deletemulti(offnp->caps_and_bevels, AY_FALSE);
      offnp->caps_and_bevels = NULL;
    }

  /* get npatch to offset */
  if(!o->down)
    goto cleanup;

  down = o->down;
  if(down->type != AY_IDNPATCH)
    {
      ay_status = ay_provide_object(down, AY_IDNPATCH, &npatch);
      if(!npatch)
	{
	  goto cleanup;
	}
      else
	{
	  provided = AY_TRUE;
	} /* if */
    }
  else
    {
      npatch = down;
    } /* if */

  /* create new object */
  ay_status = ay_npt_createnpatchobject(&newo);
  if(ay_status)
    {
      goto cleanup;
    }

  /* create the offset surface */
  ay_status = ay_npt_offset(npatch, offnp->mode, offnp->offset,
			    (ay_nurbpatch_object **)(void*)&(newo->refine));
  if(ay_status || !newo->refine)
    {
      goto cleanup;
    }

  /* link new object to offnp object */
  offnp->npatch = newo;

  /* copy transformation attributes over to offnp object
     (the offset curve is always at the same place as
     the surface) */
  ay_trafo_copy(npatch, o);

  /* copy trim curve(s) */
  if(npatch->down && npatch->down->next)
    {
      ay_status = ay_object_copymulti(npatch->down, &(newo->down));
      if(ay_status)
	{
	  (void)ay_object_deletemulti(newo->down, AY_FALSE);
	  newo->down = ay_endlevel;
	}
    }
  else
    {
      newo->down = ay_endlevel;
    }

  /* copy sampling tolerance/mode over to new object */
  ((ay_nurbpatch_object *)newo->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)newo->refine)->display_mode =
    mode;

  /* prevent cleanup code from doing something harmful */
  newo = NULL;

  /* get bevel an caps parameters */
  if(o->tags)
    {
      ay_bevelt_parsetags(o->tags, &bparams);
      ay_capt_parsetags(o->tags, &cparams);
    }

  /* create/add caps */
  if(cparams.has_caps)
    {
      ay_status = ay_capt_addcaps(&cparams, &bparams, offnp->npatch, nextcb);
      if(ay_status)
	goto cleanup;

      while(*nextcb)
	nextcb = &((*nextcb)->next);
    }

  /* create/add bevels */
  if(bparams.has_bevels)
    {
      bparams.dirs[2] = !bparams.dirs[2];

      ay_status = ay_bevelt_addbevels(&bparams, &cparams, offnp->npatch,
				      nextcb);
      if(ay_status)
	goto cleanup;
    }

  if(offnp->caps_and_bevels)
    {
      bevel = offnp->caps_and_bevels;
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

  if(newo)
    (void)ay_object_delete(newo);

  /* remove provided object(s) */
  if(provided)
    {
      (void)ay_object_deletemulti(npatch, AY_FALSE);
    }

  /* recover selected points */
  if(o->selp)
    {
      if(offnp->npatch)
	ay_offnp_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

 return ay_status;
} /* ay_offnp_notifycb */


/* ay_offnp_providecb:
 *  provide callback function of offnp object
 */
int
ay_offnp_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 ay_offnp_object *b = NULL;

  if(!o)
    return AY_ENULL;

  b = (ay_offnp_object *) o->refine;

  if(!b)
    return AY_ENULL;

 return ay_provide_nptoolobj(o, type, b->npatch, b->caps_and_bevels, result);
} /* ay_offnp_providecb */


/* ay_offnp_convertcb:
 *  convert callback function of offnp object
 */
int
ay_offnp_convertcb(ay_object *o, int in_place)
{
 ay_offnp_object *b = NULL;

  if(!o)
    return AY_ENULL;

  b = (ay_offnp_object *) o->refine;

  if(!b)
    return AY_ENULL;

 return ay_convert_nptoolobj(o, b->npatch, b->caps_and_bevels, in_place);
} /* ay_offnp_convertcb */


/* ay_offnp_init:
 *  initialize the offnp object module
 */
int
ay_offnp_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_offnp_name,
				    ay_offnp_createcb,
				    ay_offnp_deletecb,
				    ay_offnp_copycb,
				    ay_offnp_drawcb,
				    ay_offnp_drawhcb,
				    ay_offnp_shadecb,
				    ay_offnp_setpropcb,
				    ay_offnp_getpropcb,
				    ay_offnp_getpntcb,
				    ay_offnp_readcb,
				    ay_offnp_writecb,
				    ay_offnp_wribcb,
				    ay_offnp_bbccb,
				    AY_IDOFFNP);

  ay_status += ay_draw_registerdacb(ay_offnp_drawacb, AY_IDOFFNP);

  ay_status += ay_notify_register(ay_offnp_notifycb, AY_IDOFFNP);

  ay_status += ay_convert_register(ay_offnp_convertcb, AY_IDOFFNP);

  ay_status += ay_provide_register(ay_offnp_providecb, AY_IDOFFNP);

 return ay_status;
} /* ay_offnp_init */
