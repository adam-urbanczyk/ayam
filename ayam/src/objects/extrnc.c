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

/* extrnc.c - ExtrNC (extract NURBS curve) object */

static char *ay_extrnc_name = "ExtrNC";

/* functions: */
int
ay_extrnc_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtextrnc";
 ay_extrnc_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_extrnc_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  o->hide_children = AY_TRUE;
  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_extrnc_createcb */


int
ay_extrnc_deletecb(void *c)
{
 ay_extrnc_object *extrnc = NULL;

  if(!c)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)(c);

  if(extrnc->ncurve)
    ay_object_delete(extrnc->ncurve);

  free(extrnc);

 return AY_OK;
} /* ay_extrnc_deletecb */


int
ay_extrnc_copycb(void *src, void **dst)
{
 ay_extrnc_object *extrnc = NULL, *extrncsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  extrncsrc = (ay_extrnc_object *)src;

  if(!(extrnc = calloc(1, sizeof(ay_extrnc_object))))
    return AY_EOMEM;

  memcpy(extrnc, src, sizeof(ay_extrnc_object));


  /* copy ncurve */
  ay_object_copy(extrncsrc->ncurve, &(extrnc->ncurve));


  *dst = (void *)extrnc;

 return AY_OK;
} /* ay_extrnc_copycb */


int
ay_extrnc_drawcb(struct Togl *togl, ay_object *o)
{
 ay_extrnc_object *extrnc = NULL;

  if(!o)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)o->refine;

  if(!extrnc)
    return AY_ENULL;

  if(extrnc->ncurve)
    ay_draw_object(togl, extrnc->ncurve, AY_TRUE);

 return AY_OK;
} /* ay_extrnc_drawcb */


int
ay_extrnc_shadecb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_extrnc_shadecb */


int
ay_extrnc_drawhcb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_extrnc_drawhcb */


int
ay_extrnc_getpntcb(int mode, ay_object *o, double *p)
{

 return AY_OK;
} /* ay_extrnc_getpntcb */


/* Tcl -> C! */
int
ay_extrnc_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "ExtrNCAttrData";
 /* char fname[] = "extrnc_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_extrnc_object *extrnc = NULL;

  if(!o)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Side",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(extrnc->side));

  Tcl_SetStringObj(ton,"Parameter",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(extrnc->parameter));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(extrnc->glu_display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(extrnc->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_force(o);

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_extrnc_setpropcb */


/* C -> Tcl! */
int
ay_extrnc_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="ExtrNCAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_extrnc_object *extrnc = NULL;

  if(!o)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Side",-1);
  to = Tcl_NewIntObj(extrnc->side);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Parameter",-1);
  to = Tcl_NewDoubleObj(extrnc->parameter);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(extrnc->glu_display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(extrnc->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_extrnc_getpropcb */


int
ay_extrnc_readcb(FILE *fileptr, ay_object *o)
{
 ay_extrnc_object *extrnc = NULL;

 if(!o)
   return AY_ENULL;

  if(!(extrnc = calloc(1, sizeof(ay_extrnc_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&extrnc->side);
  fscanf(fileptr,"%lg\n",&extrnc->parameter);
  fscanf(fileptr,"%d\n",&extrnc->glu_display_mode);
  fscanf(fileptr,"%lg\n",&extrnc->glu_sampling_tolerance);

  o->refine = extrnc;

 return AY_OK;
} /* ay_extrnc_readcb */


int
ay_extrnc_writecb(FILE *fileptr, ay_object *o)
{
 ay_extrnc_object *extrnc = NULL;

  if(!o)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)(o->refine);

  fprintf(fileptr, "%d\n", extrnc->side);
  fprintf(fileptr, "%g\n", extrnc->parameter);
  fprintf(fileptr, "%d\n", extrnc->glu_display_mode);
  fprintf(fileptr, "%g\n", extrnc->glu_sampling_tolerance);

 return AY_OK;
} /* ay_extrnc_writecb */


int
ay_extrnc_wribcb(char *file, ay_object *o)
{

 return AY_OK;
} /* ay_extrnc_wribcb */


int
ay_extrnc_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_extrnc_object *extrnc = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)o->refine;

  if(extrnc->ncurve)
    {
      ay_bbc_get(extrnc->ncurve, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    } /* if */

 return AY_OK;
} /* ay_extrnc_bbccb */


int
ay_extrnc_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_extrnc_object *extrnc = NULL;
 ay_object *npatch = NULL, *pobject = NULL;
 ay_object *ncurve = NULL;
 int mode, provided = AY_FALSE;
 double tolerance;

  if(!o)
    return AY_ENULL;

  extrnc = (ay_extrnc_object *)(o->refine);

  mode = extrnc->glu_display_mode;
  tolerance = extrnc->glu_sampling_tolerance;

  /* remove old objects */
  if(extrnc->ncurve)
    ay_object_delete(extrnc->ncurve);
  extrnc->ncurve = NULL;

  /* get patch to extract the ncurve from */
  if(!o->down)
    return AY_OK;
  npatch = o->down;
  if(npatch->type != AY_IDNPATCH)
    {
      ay_status = ay_provide_object(npatch, AY_IDNPATCH, &pobject);
      if(!pobject)
	{
	  return AY_OK;
	}
      else
	{
	  npatch = pobject;
	  provided = AY_TRUE;
	} /* if */
    } /* if */

  /* extract the curve */
  if(!(ncurve = calloc(1, sizeof(ay_object))))
    {
      return AY_ERROR;
    }

  ay_object_defaults(ncurve);
  ncurve->type = AY_IDNCURVE;

  ay_status = ay_npt_extractnc(npatch, extrnc->side, extrnc->parameter,
			       AY_FALSE,
			       (ay_nurbcurve_object **)(&(ncurve->refine)));

  if(ay_status || !ncurve->refine)
    return ay_status;

  extrnc->ncurve = ncurve;

  /* copy transformation attributes over to new object */
  ay_trafo_copy(npatch, ncurve);

  /* copy sampling tolerance/mode over to new object */
  ((ay_nurbcurve_object *)ncurve->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbcurve_object *)ncurve->refine)->display_mode =
    mode;

  /* remove provided object(s) */
  if(provided)
    {
      ay_object_deletemulti(pobject);
    }

 return AY_OK;
} /* ay_extrnc_notifycb */


int
ay_extrnc_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "extrnc_providecb";
 ay_extrnc_object *s = NULL;
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

  s = (ay_extrnc_object *) o->refine;

  if(type == AY_IDNCURVE)
    {
      t = &(new);

      /* copy extrnc */
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
} /* ay_extrnc_providecb */


int
ay_extrnc_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_extrnc_object *r = NULL;
 ay_object *new = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */
  r = (ay_extrnc_object *) o->refine;

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
} /* ay_extrnc_convertcb */


int
ay_extrnc_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_extrnc_name,
				    ay_extrnc_createcb,
				    ay_extrnc_deletecb,
				    ay_extrnc_copycb,
				    ay_extrnc_drawcb,
				    NULL, /* no handles */
				    ay_extrnc_shadecb,
				    ay_extrnc_setpropcb,
				    ay_extrnc_getpropcb,
				    ay_extrnc_getpntcb,
				    ay_extrnc_readcb,
				    ay_extrnc_writecb,
				    ay_extrnc_wribcb,
				    ay_extrnc_bbccb,
				    AY_IDEXTRNC);


  ay_status = ay_notify_register(ay_extrnc_notifycb, AY_IDEXTRNC);

  ay_status = ay_convert_register(ay_extrnc_convertcb, AY_IDEXTRNC);

  ay_status = ay_provide_register(ay_extrnc_providecb, AY_IDEXTRNC);

 return ay_status;
} /* ay_extrnc_init */

