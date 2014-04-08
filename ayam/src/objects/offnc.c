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

/* offnc.c - OffsetNC (offset NURBS curve) object */

static char *ay_offnc_name = "OffsetNC";

int ay_offnc_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_offnc_createcb:
 *  create callback function of offnc object
 */
int
ay_offnc_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtoffnc";
 ay_offnc_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_offnc_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  o->hide_children = AY_TRUE;
  o->parent = AY_TRUE;
  o->refine = new;

  new->offset = 0.1;

 return AY_OK;
} /* ay_offnc_createcb */


/* ay_offnc_deletecb:
 *  delete callback function of offnc object
 */
int
ay_offnc_deletecb(void *c)
{
 ay_offnc_object *offnc = NULL;

  if(!c)
    return AY_ENULL;

  offnc = (ay_offnc_object *)(c);

  if(offnc->ncurve)
    (void)ay_object_delete(offnc->ncurve);

  free(offnc);

 return AY_OK;
} /* ay_offnc_deletecb */


/* ay_offnc_copycb:
 *  copy callback function of offnc object
 */
int
ay_offnc_copycb(void *src, void **dst)
{
 ay_offnc_object *offnc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(offnc = malloc(sizeof(ay_offnc_object))))
    return AY_EOMEM;

  memcpy(offnc, src, sizeof(ay_offnc_object));

  offnc->ncurve = NULL;

  *dst = (void *)offnc;

 return AY_OK;
} /* ay_offnc_copycb */


/* ay_offnc_drawcb:
 *  draw (display in an Ayam view window) callback function of offnc object
 */
int
ay_offnc_drawcb(struct Togl *togl, ay_object *o)
{
 ay_offnc_object *offnc = NULL;

  if(!o)
    return AY_ENULL;

  offnc = (ay_offnc_object *)o->refine;

  if(!offnc)
    return AY_ENULL;

  if(offnc->ncurve)
    ay_draw_object(togl, offnc->ncurve, AY_TRUE);

 return AY_OK;
} /* ay_offnc_drawcb */


/* ay_offnc_shadecb:
 *  shade (display in an Ayam view window) callback function of offnc object
 */
int
ay_offnc_shadecb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_offnc_shadecb */


/* ay_offnc_drawacb:
 *  draw annotations (in an Ayam view window) callback function of offnc object
 */
int
ay_offnc_drawacb(struct Togl *togl, ay_object *o)
{
 ay_offnc_object *offnc = NULL;
 ay_nurbcurve_object *nc = NULL;
 ay_object *c = NULL;
 double m[16];

  if(!o)
    return AY_ENULL;

  offnc = (ay_offnc_object *)o->refine;

  if(!offnc)
    return AY_ENULL;

  if(offnc->ncurve)
    {
      /* get NURBS curve */
      nc = (ay_nurbcurve_object *)offnc->ncurve->refine;

      /* draw arrow */
      glPushMatrix();
       c = offnc->ncurve;
       glTranslated((GLdouble)c->movx, (GLdouble)c->movy,
		    (GLdouble)c->movz);
       ay_quat_torotmatrix(c->quat, m);
       glMultMatrixd((GLdouble*)m);
       glScaled((GLdouble)c->scalx, (GLdouble)c->scaly,
		(GLdouble)c->scalz);

       ay_draw_arrow(togl, &(nc->controlv[nc->length*4-8]),
		     &(nc->controlv[nc->length*4-4]));
      glPopMatrix();
    }

 return AY_OK;
} /* ay_offnc_drawacb */


/* ay_offnc_drawhcb:
 *  draw handles (in an Ayam view window) callback function of offnc object
 */
int
ay_offnc_drawhcb(struct Togl *togl, ay_object *o)
{
 int i;
 double m[16], *pnts;
 ay_object *c;
 ay_offnc_object *offnc;
 ay_nurbcurve_object *nc;

  if(!o)
    return AY_ENULL;

  offnc = (ay_offnc_object *)o->refine;

  if(!offnc)
    return AY_ENULL;

  if(offnc->ncurve)
    {
      /* get NURBS curve */
      nc = (ay_nurbcurve_object *)offnc->ncurve->refine;
      pnts = nc->controlv;

      glPushMatrix();
       c = offnc->ncurve;
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
        for(i = 0; i < nc->length; i++)
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
} /* ay_offnc_drawhcb */


/* ay_offnc_getpntcb:
 *  get point (editing and selection) callback function of offnc object
 */
int
ay_offnc_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbcurve_object *curve = NULL;
 ay_offnc_object *offnc = NULL;

  if(!o)
    return AY_ENULL;

  offnc = (ay_offnc_object *)o->refine;

  if(!offnc)
    return AY_ENULL;

  if(offnc->ncurve)
    {
      curve = (ay_nurbcurve_object *)offnc->ncurve->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, curve->length, 4,
			     curve->controlv);
    }

 return AY_ERROR;
} /* ay_offnc_getpntcb */


/* ay_offnc_setpropcb:
 *  set property (from Tcl to C context) callback function of offnc object
 */
int
ay_offnc_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "OffsetNCAttrData";
 /* char fname[] = "offnc_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_offnc_object *offnc = NULL;

  if(!interp || !o)
    return AY_ENULL;

  offnc = (ay_offnc_object *)o->refine;

  if(!offnc)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Mode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(offnc->mode));

  Tcl_SetStringObj(ton,"Revert",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(offnc->revert));

  Tcl_SetStringObj(ton,"Offset",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(offnc->offset));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(offnc->display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(offnc->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_offnc_setpropcb */


/* ay_offnc_getpropcb:
 *  get property (from C to Tcl context) callback function of offnc object
 */
int
ay_offnc_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="OffsetNCAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_offnc_object *offnc = NULL;

  if(!interp || !o)
    return AY_ENULL;

  offnc = (ay_offnc_object *)(o->refine);

  if(!offnc)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Mode",-1);
  to = Tcl_NewIntObj(offnc->mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Revert",-1);
  to = Tcl_NewIntObj(offnc->revert);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Offset",-1);
  to = Tcl_NewDoubleObj(offnc->offset);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(offnc->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(offnc->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getncinfo(interp, n1, offnc->ncurve);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_offnc_getpropcb */


/* ay_offnc_readcb:
 *  read (from scene file) callback function of offnc object
 */
int
ay_offnc_readcb(FILE *fileptr, ay_object *o)
{
 ay_offnc_object *offnc = NULL;

 if(!fileptr || !o)
   return AY_ENULL;

  if(!(offnc = calloc(1, sizeof(ay_offnc_object))))
    { return AY_EOMEM; }

  fscanf(fileptr, "%d\n", &offnc->mode);
  fscanf(fileptr, "%d\n", &offnc->revert);
  fscanf(fileptr, "%lg\n", &offnc->offset);
  fscanf(fileptr, "%d\n", &offnc->display_mode);
  fscanf(fileptr, "%lg\n", &offnc->glu_sampling_tolerance);

  o->refine = offnc;

 return AY_OK;
} /* ay_offnc_readcb */


/* ay_offnc_writecb:
 *  write (to scene file) callback function of offnc object
 */
int
ay_offnc_writecb(FILE *fileptr, ay_object *o)
{
 ay_offnc_object *offnc = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  offnc = (ay_offnc_object *)(o->refine);

  if(!offnc)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", offnc->mode);
  fprintf(fileptr, "%d\n", offnc->revert);
  fprintf(fileptr, "%g\n", offnc->offset);
  fprintf(fileptr, "%d\n", offnc->display_mode);
  fprintf(fileptr, "%g\n", offnc->glu_sampling_tolerance);

 return AY_OK;
} /* ay_offnc_writecb */


/* ay_offnc_wribcb:
 *  RIB export callback function of offnc object
 */
int
ay_offnc_wribcb(char *file, ay_object *o)
{

 return AY_OK;
} /* ay_offnc_wribcb */


/* ay_offnc_bbccb:
 *  bounding box calculation callback function of offnc object
 */
int
ay_offnc_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_offnc_object *offnc = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  offnc = (ay_offnc_object *)o->refine;

  if(!offnc)
    return AY_ENULL;

  if(offnc->ncurve)
    {
      ay_bbc_get(offnc->ncurve, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    } /* if */

 return AY_OK;
} /* ay_offnc_bbccb */


/* ay_offnc_notifycb:
 *  notification callback function of offnc object
 */
int
ay_offnc_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_offnc_object *offnc = NULL;
 ay_object *down = NULL, *ncurve = NULL, *newo = NULL;
 int mode, provided = AY_FALSE;
 double tolerance;

  if(!o)
    return AY_ENULL;

  offnc = (ay_offnc_object *)(o->refine);

  if(!offnc)
    return AY_ENULL;

  mode = offnc->display_mode;
  tolerance = offnc->glu_sampling_tolerance;

  /* remove old objects */
  if(offnc->ncurve)
    (void)ay_object_delete(offnc->ncurve);
  offnc->ncurve = NULL;

  /* get ncurve to offset */
  if(!o->down)
    goto cleanup;

  down = o->down;
  if(down->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(down, AY_IDNCURVE, &ncurve);
      if(!ncurve)
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
      ncurve = down;
    } /* if */

  /* create new object */
  if(!(newo = calloc(1, sizeof(ay_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  ay_object_defaults(newo);
  newo->type = AY_IDNCURVE;

  /* create the offset */
  ay_status = ay_nct_offset(ncurve, offnc->mode, offnc->offset,
			    (ay_nurbcurve_object **)(void*)&(newo->refine));

  if(ay_status || !newo->refine)
    goto cleanup;

  if(((ay_nurbcurve_object *)(ncurve->refine))->type != AY_CTOPEN)
    {
      ((ay_nurbcurve_object *)(newo->refine))->type =
	((ay_nurbcurve_object *)(ncurve->refine))->type;
      ay_status = ay_nct_close((ay_nurbcurve_object *)(newo->refine));
    }

  if(offnc->revert)
    {
      ay_status = ay_nct_revert((ay_nurbcurve_object *)(newo->refine));
    }

  /* link new object to offnc object */
  offnc->ncurve = newo;

  /* copy transformation attributes over to offnp object
     (the offset curve is always at the same place as
     the surface) */
  ay_trafo_copy(ncurve, o);

  /* copy sampling tolerance/mode over to new object */
  ((ay_nurbcurve_object *)newo->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbcurve_object *)newo->refine)->display_mode =
    mode;

  /* prevent cleanup code from doing something harmful */
  newo = NULL;

cleanup:

  if(newo)
    (void)ay_object_delete(newo);

  /* remove provided object(s) */
  if(provided)
    {
      (void)ay_object_deletemulti(ncurve, AY_FALSE);
    }

  /* recover selected points */
  if(o->selp)
    {
      if(offnc->ncurve)
	ay_offnc_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

 return ay_status;
} /* ay_offnc_notifycb */


/* ay_offnc_providecb:
 *  provide callback function of offnc object
 */
int
ay_offnc_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "offnc_providecb";
 ay_offnc_object *s = NULL;
 ay_object *new = NULL, **t = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNCURVE)
	return AY_OK;
      else
	return AY_ERROR;
    } /* if */

  s = (ay_offnc_object *) o->refine;

  if(!s)
    return AY_ENULL;

  if(type == AY_IDNCURVE)
    {
      t = &(new);

      if(!s->ncurve)
	return AY_ERROR;

      /* copy offnc */
      ay_status = ay_object_copy(s->ncurve, t);
      if(ay_status)
	{
	  ay_error(ay_status, fname, NULL);
	  return AY_ERROR;
	}

      ay_trafo_copy(o, *t);

      /*t = &((*t)->next);*/

      *result = new;
    } /* if */

 return ay_status;
} /* ay_offnc_providecb */


/* ay_offnc_convertcb:
 *  convert callback function of offnc object
 */
int
ay_offnc_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_offnc_object *r = NULL;
 ay_object *new = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */
  r = (ay_offnc_object *) o->refine;

  if(!r)
    return AY_ENULL;

  if(r->ncurve)
    {
      ay_status = ay_object_copy(r->ncurve, &new);

      /* second, link new object, or replace old object with it */
      if(new)
	{
	  /* reset display mode and sampling tolerance
	     of new curve to "global"? */
	  if(!in_place && ay_prefs.conv_reset_display)
	    {
	      nc = (ay_nurbcurve_object *)(new->refine);
	      nc->display_mode = 0;
	      nc->glu_sampling_tolerance = 0.0;
	    }

	  ay_trafo_copy(o, new);

	  if(!in_place)
	    {
	      ay_object_link(new);
	    }
	  else
	    {
	      ay_object_replace(new, o);
	    } /* if */
	} /* if */
    } /* if */

 return ay_status;
} /* ay_offnc_convertcb */


/* ay_offnc_init:
 *  initialize the offnc object module
 */
int
ay_offnc_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_offnc_name,
				    ay_offnc_createcb,
				    ay_offnc_deletecb,
				    ay_offnc_copycb,
				    ay_offnc_drawcb,
				    ay_offnc_drawhcb,
				    NULL/*ay_offnc_shadecb*/,
				    ay_offnc_setpropcb,
				    ay_offnc_getpropcb,
				    ay_offnc_getpntcb,
				    ay_offnc_readcb,
				    ay_offnc_writecb,
				    ay_offnc_wribcb,
				    ay_offnc_bbccb,
				    AY_IDOFFNC);

  ay_status += ay_draw_registerdacb(ay_offnc_drawacb, AY_IDOFFNC);

  ay_status += ay_notify_register(ay_offnc_notifycb, AY_IDOFFNC);

  ay_status += ay_convert_register(ay_offnc_convertcb, AY_IDOFFNC);

  ay_status += ay_provide_register(ay_offnc_providecb, AY_IDOFFNC);

  /* offnc objects may not be associated with materials */
  ay_matt_nomaterial(AY_IDOFFNC);

 return ay_status;
} /* ay_offnc_init */
