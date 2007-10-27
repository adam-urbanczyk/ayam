/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2004 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* offnc.c - OffsetNC (offset NURBS curve) object */

static char *ay_offnc_name = "OffsetNC";

/* functions: */
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

 return AY_OK;
} /* ay_offnc_createcb */


int
ay_offnc_deletecb(void *c)
{
 ay_offnc_object *offnc = NULL;

  if(!c)
    return AY_ENULL;

  offnc = (ay_offnc_object *)(c);

  if(offnc->ncurve)
    ay_object_delete(offnc->ncurve);

  free(offnc);

 return AY_OK;
} /* ay_offnc_deletecb */


int
ay_offnc_copycb(void *src, void **dst)
{
 ay_offnc_object *offnc = NULL, *offncsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  offncsrc = (ay_offnc_object *)src;

  if(!(offnc = calloc(1, sizeof(ay_offnc_object))))
    return AY_EOMEM;

  memcpy(offnc, src, sizeof(ay_offnc_object));

  /* copy ncurve */
  if(offncsrc->ncurve)
    {
      ay_object_copy(offncsrc->ncurve, &(offnc->ncurve));
    }

  *dst = (void *)offnc;

 return AY_OK;
} /* ay_offnc_copycb */


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


int
ay_offnc_shadecb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_offnc_shadecb */


int
ay_offnc_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_offnc_object *offnc = NULL;
 ay_nurbcurve_object *nc = NULL;
 double *p1, *p2;

  if(!o)
    return AY_ENULL;

  offnc = (ay_offnc_object *)o->refine;

  if(offnc && offnc->ncurve)
    {
      /* get NURBS curve and its last control points */
      nc = (ay_nurbcurve_object *)offnc->ncurve->refine;
      p1 = &(nc->controlv[nc->length*4-8]);
      p2 = p1+4;
      /* draw arrow */
      ay_draw_arrow(togl, p1, p2);
    }

 return AY_OK;
} /* ay_offnc_drawhcb */


int
ay_offnc_getpntcb(int mode, ay_object *o, double *p)
{

 return AY_OK;
} /* ay_offnc_getpntcb */


/* Tcl -> C! */
int
ay_offnc_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "OffNCAttrData";
 /* char fname[] = "offnc_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_offnc_object *offnc = NULL;

  if(!o)
    return AY_ENULL;

  offnc = (ay_offnc_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Revert",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(offnc->revert));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(offnc->display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(offnc->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_force(o);

  o->modified = AY_TRUE;
  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_offnc_setpropcb */


/* C -> Tcl! */
int
ay_offnc_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="OffNCAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_offnc_object *offnc = NULL;

  if(!o)
    return AY_ENULL;

  offnc = (ay_offnc_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Revert",-1);
  to = Tcl_NewIntObj(offnc->revert);
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


int
ay_offnc_readcb(FILE *fileptr, ay_object *o)
{
 ay_offnc_object *offnc = NULL;

 if(!o)
   return AY_ENULL;

  if(!(offnc = calloc(1, sizeof(ay_offnc_object))))
    { return AY_EOMEM; }

  fscanf(fileptr, "%d\n", &offnc->revert);
  fscanf(fileptr, "%d\n", &offnc->display_mode);
  fscanf(fileptr, "%lg\n", &offnc->glu_sampling_tolerance);

  o->refine = offnc;

 return AY_OK;
} /* ay_offnc_readcb */


int
ay_offnc_writecb(FILE *fileptr, ay_object *o)
{
 ay_offnc_object *offnc = NULL;

  if(!o)
    return AY_ENULL;

  offnc = (ay_offnc_object *)(o->refine);

  fprintf(fileptr, "%d\n", offnc->revert);
  fprintf(fileptr, "%d\n", offnc->display_mode);
  fprintf(fileptr, "%g\n", offnc->glu_sampling_tolerance);

 return AY_OK;
} /* ay_offnc_writecb */


int
ay_offnc_wribcb(char *file, ay_object *o)
{

 return AY_OK;
} /* ay_offnc_wribcb */


int
ay_offnc_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_offnc_object *offnc = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  offnc = (ay_offnc_object *)o->refine;

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


int
ay_offnc_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_offnc_object *offnc = NULL;
 ay_object *down = NULL, *pobject = NULL;
 ay_object *ncurve = NULL, *newo = NULL;
 int mode, provided = AY_FALSE;
 double tolerance;

  if(!o)
    return AY_ENULL;

  offnc = (ay_offnc_object *)(o->refine);

  pnum = offnc->pnum - 1;
  mode = offnc->display_mode;
  tolerance = offnc->glu_sampling_tolerance;

  /* remove old objects */
  if(offnc->ncurve)
    ay_object_delete(offnc->ncurve);
  offnc->ncurve = NULL;

  /* get ncurve to offset */
  if(!o->down)
    return AY_OK;
  if(!o->down->next)
    return AY_OK;

  down = o->down;
  if(down->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(down, AY_IDNCURVE, &ncurve);
      if(!pobject)
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
  ay_status = ay_nct_offset(ncurve,
			    (ay_nurbcurve_object **)(&(newo->refine)));

  if(ay_status || !newo->refine)
    goto cleanup;

  if(offnc->revert)
    {
      ay_status = ay_nct_revert((ay_nurbcurve_object *)(newo->refine));
    }

  /* link new object to offnc object */
  offnc->ncurve = newo;

  /* copy transformation attributes over to new object */
  ay_trafo_copy(npatch, newo);

  /* copy sampling tolerance/mode over to new object */
  ((ay_nurbcurve_object *)newo->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbcurve_object *)newo->refine)->display_mode =
    mode;

  /* prevent cleanup code from doing something harmful */
  newo = NULL;

cleanup:

  if(newo)
    ay_object_delete(newo);

  /* remove provided object(s) */
  if(provided)
    {
      ay_object_deletemulti(ncurve);
    }

 return ay_status;
} /* ay_offnc_notifycb */


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

  if(type == AY_IDNCURVE)
    {
      t = &(new);

      /* copy offnc */
      ay_status = ay_object_copy(s->ncurve, t);
      if(ay_status)
	{
	  ay_error(ay_status, fname, NULL);
	  return AY_ERROR;
	}
      ay_trafo_add(o, *t);
      t = &((*t)->next);

      *result = new;
    } /* if */

 return ay_status;
} /* ay_offnc_providecb */


int
ay_offnc_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_offnc_object *r = NULL;
 ay_object *new = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */
  r = (ay_offnc_object *) o->refine;

  if(r->ncurve)
    {
      ay_status = ay_object_copy(r->ncurve, &new);
      ay_trafo_add(o, new);
    } /* if */

  /* second, link new object, or replace old object with it */
  if(new)
    {
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
} /* ay_offnc_convertcb */


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


  ay_status = ay_notify_register(ay_offnc_notifycb, AY_IDOFFNC);

  ay_status = ay_convert_register(ay_offnc_convertcb, AY_IDOFFNC);

  ay_status = ay_provide_register(ay_offnc_providecb, AY_IDOFFNC);

  ay_matt_nomaterial(AY_IDOFFNC);

 return ay_status;
} /* ay_offnc_init */

