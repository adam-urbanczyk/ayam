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
    ay_object_delete(offnp->npatch);

  free(offnp);

 return AY_OK;
} /* ay_offnp_deletecb */


/* ay_offnp_copycb:
 *  copy callback function of offnp object
 */
int
ay_offnp_copycb(void *src, void **dst)
{
 ay_offnp_object *offnp = NULL, *offnpsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  offnpsrc = (ay_offnp_object *)src;

  if(!(offnp = calloc(1, sizeof(ay_offnp_object))))
    return AY_EOMEM;

  memcpy(offnp, src, sizeof(ay_offnp_object));

  /* copy npatch */
  if(offnpsrc->npatch)
    {
      ay_object_copy(offnpsrc->npatch, &(offnp->npatch));
    }

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

  if(!o)
    return AY_ENULL;

  offnp = (ay_offnp_object *)o->refine;

  if(!offnp)
    return AY_ENULL;

  if(offnp->npatch)
    ay_draw_object(togl, offnp->npatch, AY_TRUE);

 return AY_OK;
} /* ay_offnp_drawcb */


/* ay_offnp_shadecb:
 *  shade (display in an Ayam view window) callback function of offnp object
 */
int
ay_offnp_shadecb(struct Togl *togl, ay_object *o)
{
 ay_offnp_object *offnp = NULL;

  if(!o)
    return AY_ENULL;

  offnp = (ay_offnp_object *)o->refine;

  if(!offnp)
    return AY_ENULL;

  if(offnp->npatch)
    ay_shade_object(togl, offnp->npatch, AY_FALSE);

 return AY_OK;
} /* ay_offnp_shadecb */


/* ay_offnp_drawhcb:
 *  draw handles (in an Ayam view window) callback function of offnp object
 */
int
ay_offnp_drawhcb(struct Togl *togl, ay_object *o)
{
 int i = 0, a = 0;
 ay_object *c = NULL;
 ay_offnp_object *offnp = NULL;
 ay_nurbpatch_object *patch = NULL;
 double *pnts = NULL;
 double point_size = ay_prefs.handle_size;
 double *p1, *p2;
 double m[16];

  if(!o)
    return AY_ENULL;

  offnp = (ay_offnp_object *) o->refine;

  if(offnp->npatch)
    {
      /* get NURBS patch and its last control points */
      patch = (ay_nurbpatch_object *)offnp->npatch->refine;
      p1 = &(patch->controlv[patch->width*patch->height*4-8]);
      p2 = p1+4;

      /* draw points */
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

      /* draw arrow */
      glPushMatrix();
       c = offnp->npatch;
       glTranslated((GLdouble)c->movx, (GLdouble)c->movy, (GLdouble)c->movz);
       ay_quat_torotmatrix(c->quat, m);
       glMultMatrixd((GLdouble*)m);
       glScaled((GLdouble)c->scalx, (GLdouble)c->scaly, (GLdouble)c->scalz);

       ay_draw_arrow(togl, p1, p2);
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

  if(!o)
    return AY_ENULL;

  offnp = (ay_offnp_object *)o->refine;

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

  ay_notify_object(o);

  o->modified = AY_TRUE;

  ay_notify_parent();

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

  if(!o)
    return AY_ENULL;

  offnp = (ay_offnp_object *)(o->refine);

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

 if(!o)
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

  if(!o)
    return AY_ENULL;

  offnp = (ay_offnp_object *)(o->refine);

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

  if(!o)
   return AY_ENULL;

  offnp = (ay_offnp_object*)o->refine;

  if(offnp->npatch)
    ay_wrib_object(file, offnp->npatch);

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
 int mode, provided = AY_FALSE;
 double tolerance;

  if(!o)
    return AY_ENULL;

  offnp = (ay_offnp_object *)(o->refine);

  mode = offnp->display_mode;
  tolerance = offnp->glu_sampling_tolerance;

  /* remove old objects */
  if(offnp->npatch)
    ay_object_delete(offnp->npatch);
  offnp->npatch = NULL;

  /* get npatch to offset */
  if(!o->down)
    return AY_OK;
  if(!o->down->next)
    return AY_OK;

  down = o->down;
  if(down->type != AY_IDNPATCH)
    {
      ay_status = ay_provide_object(down, AY_IDNPATCH, &npatch);
      if(!npatch)
	{
	  return AY_ERROR;
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
  ay_npt_createnpatchobject(&newo);

  /* create the offset */
  ay_status = ay_npt_offset(npatch, offnp->mode, offnp->offset,
			    (ay_nurbpatch_object **)(void*)&(newo->refine));

  if(ay_status || !newo->refine)
    goto cleanup;

  /* link new object to offnp object */
  offnp->npatch = newo;

  /* copy transformation attributes over to offnp object
     (the offset curve is always at the same place as
     the surface) */
  ay_trafo_copy(npatch, o);

  /* copy trim curve(s) */
  if(npatch->down && npatch->down->next)
    {
      ay_object_copymulti(npatch->down, &(newo->down));
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

cleanup:

  if(newo)
    ay_object_delete(newo);

  /* remove provided object(s) */
  if(provided)
    {
      ay_object_deletemulti(npatch);
    }

  /* recover selected points */
  if(o->selp)
    {
      ay_offnp_getpntcb(3, o, NULL, NULL);
    }

 return ay_status;
} /* ay_offnp_notifycb */


/* ay_offnp_providecb:
 *  provide callback function of offnp object
 */
int
ay_offnp_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "offnp_providecb";
 ay_offnp_object *s = NULL;
 ay_object *new = NULL, **t = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNPATCH)
	return AY_OK;
      else
	return AY_ERROR;
    } /* if */

  s = (ay_offnp_object *) o->refine;

  if(type == AY_IDNPATCH)
    {
      t = &(new);

      if(!s->npatch)
	return AY_ERROR;

      /* copy offnp */
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
} /* ay_offnp_providecb */


/* ay_offnp_convertcb:
 *  convert callback function of offnp object
 */
int
ay_offnp_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_offnp_object *r = NULL;
 ay_object *new = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */
  r = (ay_offnp_object *) o->refine;

  if(r->npatch)
    {
      ay_status = ay_object_copy(r->npatch, &new);
    } /* if */

  /* second, link new object, or replace old object with it */
  if(new)
    {
      /* reset display mode and sampling tolerance
	 of new patch to "global"? */
      if(!in_place && ay_prefs.conv_reset_display)
	{
	  ay_npt_resetdisplay(new);
	}

      ay_trafo_copy(o, new);

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


  ay_status = ay_notify_register(ay_offnp_notifycb, AY_IDOFFNP);

  ay_status = ay_convert_register(ay_offnp_convertcb, AY_IDOFFNP);

  ay_status = ay_provide_register(ay_offnp_providecb, AY_IDOFFNP);

 return ay_status;
} /* ay_offnp_init */
