/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2001 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* swing.c - swing object */

typedef struct ay_swing_object_s
{

  int has_upper_cap;
  ay_object *upper_cap;
  int has_lower_cap;
  ay_object *lower_cap;
  int has_start_cap;
  ay_object *start_cap;
  int has_end_cap;
  ay_object *end_cap;

  /* cache NURBS patch representation */
  ay_object *npatch;
  double glu_sampling_tolerance;
  int display_mode;
} ay_swing_object;

static char *ay_swing_name = "Swing";

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


int
ay_swing_deletecb(void *c)
{
 ay_swing_object *swing = NULL;

  if(!c)
    return AY_ENULL;

  swing = (ay_swing_object *)(c);

  if(swing->npatch)
    ay_object_delete(swing->npatch);

  if(swing->upper_cap)
    ay_object_delete(swing->upper_cap);

  if(swing->lower_cap)
    ay_object_delete(swing->lower_cap);

  if(swing->start_cap)
    ay_object_delete(swing->start_cap);

  if(swing->end_cap)
    ay_object_delete(swing->end_cap);

  free(swing);

 return AY_OK;
} /* ay_swing_deletecb */


int
ay_swing_copycb(void *src, void **dst)
{
 ay_swing_object *swing = NULL, *swingsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  swingsrc = (ay_swing_object *)src;

  if(!(swing = calloc(1, sizeof(ay_swing_object))))
    return AY_EOMEM;

  memcpy(swing, src, sizeof(ay_swing_object));


  /* copy npatch */
  ay_object_copy(swingsrc->npatch, &(swing->npatch));

  /* copy upper cap */
  ay_object_copy(swingsrc->upper_cap, &(swing->upper_cap));

  /* copy lower cap */
  ay_object_copy(swingsrc->lower_cap, &(swing->lower_cap));

  /* copy start cap */
  ay_object_copy(swingsrc->start_cap, &(swing->start_cap));

  /* copy end cap */
  ay_object_copy(swingsrc->end_cap, &(swing->end_cap));


  *dst = (void *)swing;

 return AY_OK;
} /* ay_swing_copycb */


int
ay_swing_drawcb(struct Togl *togl, ay_object *o)
{
 ay_swing_object *swing = NULL;

  if(!o)
    return AY_ENULL;

  swing = (ay_swing_object *)o->refine;

  if(!swing)
    return AY_ENULL;

  if(swing->npatch)
    ay_draw_object(togl, swing->npatch, AY_TRUE);

  if(swing->upper_cap)
    ay_draw_object(togl, swing->upper_cap, AY_TRUE);

  if(swing->lower_cap)
    ay_draw_object(togl, swing->lower_cap, AY_TRUE);

  if(swing->start_cap)
    ay_draw_object(togl, swing->start_cap, AY_TRUE);

  if(swing->end_cap)
    ay_draw_object(togl, swing->end_cap, AY_TRUE);

 return AY_OK;
} /* ay_swing_drawcb */


int
ay_swing_shadecb(struct Togl *togl, ay_object *o)
{
 ay_swing_object *swing = NULL;

  if(!o)
    return AY_ENULL;

  swing = (ay_swing_object *)o->refine;

  if(!swing)
    return AY_ENULL;

  if(swing->npatch)
    ay_shade_object(togl, swing->npatch, AY_FALSE);

  if(swing->upper_cap)
    ay_shade_object(togl, swing->upper_cap, AY_FALSE);

  if(swing->lower_cap)
    ay_shade_object(togl, swing->lower_cap, AY_FALSE);

  if(swing->start_cap)
    ay_shade_object(togl, swing->start_cap, AY_FALSE);

  if(swing->end_cap)
    ay_shade_object(togl, swing->end_cap, AY_FALSE);

 return AY_OK;
} /* ay_swing_shadecb */


int
ay_swing_drawhcb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_swing_drawhcb */


int
ay_swing_getpntcb(int mode, ay_object *o, double *p)
{

 return AY_OK;
} /* ay_swing_getpntcb */


/* Tcl -> C! */
int
ay_swing_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "SwingAttrData";
 /* char fname[] = "swing_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_swing_object *swing = NULL;


  if(!o)
    return AY_ENULL;

  swing = (ay_swing_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"UpperCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(swing->has_upper_cap));

  Tcl_SetStringObj(ton,"LowerCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(swing->has_lower_cap));

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(swing->has_start_cap));

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(swing->has_end_cap));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(swing->display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(swing->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_force(o);

  o->modified = AY_TRUE;
  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_swing_setpropcb */


/* C -> Tcl! */
int
ay_swing_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="SwingAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_swing_object *swing = NULL;

  if(!o)
    return AY_ENULL;

  swing = (ay_swing_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"UpperCap",-1);
  to = Tcl_NewIntObj(swing->has_upper_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"LowerCap",-1);
  to = Tcl_NewIntObj(swing->has_lower_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_NewIntObj(swing->has_start_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_NewIntObj(swing->has_end_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

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


int
ay_swing_readcb(FILE *fileptr, ay_object *o)
{
 ay_swing_object *swing = NULL;

 if(!o)
   return AY_ENULL;

  if(!(swing = calloc(1, sizeof(ay_swing_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&swing->has_upper_cap);
  fscanf(fileptr,"%d\n",&swing->has_lower_cap);
  fscanf(fileptr,"%d\n",&swing->has_start_cap);
  fscanf(fileptr,"%d\n",&swing->has_end_cap);
  fscanf(fileptr,"%d\n",&swing->display_mode);
  fscanf(fileptr,"%lg\n",&swing->glu_sampling_tolerance);

  o->refine = swing;

 return AY_OK;
} /* ay_swing_readcb */


int
ay_swing_writecb(FILE *fileptr, ay_object *o)
{
 ay_swing_object *swing = NULL;

  if(!o)
    return AY_ENULL;

  swing = (ay_swing_object *)(o->refine);

  fprintf(fileptr, "%d\n", swing->has_upper_cap);
  fprintf(fileptr, "%d\n", swing->has_lower_cap);
  fprintf(fileptr, "%d\n", swing->has_start_cap);
  fprintf(fileptr, "%d\n", swing->has_end_cap);
  fprintf(fileptr, "%d\n", swing->display_mode);
  fprintf(fileptr, "%g\n", swing->glu_sampling_tolerance);

 return AY_OK;
} /* ay_swing_writecb */


int
ay_swing_wribcb(char *file, ay_object *o)
{
 ay_swing_object *swing = NULL;

  if(!o)
   return AY_ENULL;

  swing = (ay_swing_object*)o->refine;

  if(swing->npatch)
    ay_wrib_object(file, swing->npatch);

  if(swing->upper_cap)
    ay_wrib_object(file, swing->upper_cap);

  if(swing->lower_cap)
    ay_wrib_object(file, swing->lower_cap);

  if(swing->start_cap)
    ay_wrib_object(file, swing->start_cap);

  if(swing->end_cap)
    ay_wrib_object(file, swing->end_cap);

 return AY_OK;
} /* ay_swing_wribcb */


int
ay_swing_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_swing_object *swing = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  swing = (ay_swing_object *)o->refine;

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


int
ay_swing_crtcap(ay_swing_object *swing, ay_object *curve,
		  double u, int upper,
		  ay_object **o)
{
 int ay_status = AY_OK;
 int mode = 0, i = 0;
 double P1[4] = {0}, P2[4] = {0}, *controlv = NULL;
 double thetamax, tolerance;
 double m[16];
 ay_object *cap = NULL, *trim = NULL, *tloop = NULL;
 ay_nurbcurve_object *nc = NULL;

  nc = (ay_nurbcurve_object *)curve->refine;

  mode = swing->display_mode;
  tolerance = swing->glu_sampling_tolerance;

  /* get curves transformation-matrix */
  ay_trafo_creatematrix(curve, m);

  ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
		     nc->knotv, nc->controlv,
		     u, P1);

  /* apply transform */
  AY_APTRAN3(P2,P1,m)

  if(!(cap = calloc(1, sizeof(ay_object))))
    {return AY_EOMEM;}
  ay_object_defaults(cap);
  cap->type = AY_IDNPATCH;
  cap->hide_children = AY_TRUE;
  cap->parent = AY_TRUE;
  cap->inherit_trafos = AY_FALSE;

  if(!(controlv = calloc(4*4, sizeof(double))))
    {free(cap); return AY_EOMEM;}

  controlv[0] = -P2[0];
  controlv[2] = -P2[0];
  controlv[3] = 1.0;

  controlv[4] = -P2[0];
  controlv[6] = P2[0];
  controlv[7] = 1.0;

  controlv[8] = P2[0];
  controlv[10] = -P2[0];
  controlv[11] = 1.0;

  controlv[12] = P2[0];
  controlv[14] = P2[0];
  controlv[15] = 1.0;

  if(upper)
    {
      for(i=1;i<=15;i+=4)
	controlv[i] = -P2[1];
      cap->scaly *= -1.0;
    }
  else
    {
      for(i=1;i<=15;i+=4)
	controlv[i] = P2[1];
    }

  ay_status = ay_npt_create(2, 2, 2, 2,
			    AY_KTBEZIER, AY_KTBEZIER,
			    controlv, NULL, NULL,
			    (ay_nurbpatch_object **)(&(cap->refine)));

  ((ay_nurbpatch_object *)cap->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)cap->refine)->display_mode = mode;

  /* create trimcurve */
  if(!(trim = calloc(1, sizeof(ay_object))))
    {return AY_EOMEM;}
  ay_object_defaults(trim);
  trim->type = AY_IDNCURVE;

  /* calc thetamax! */

  /**/
  ay_nct_crtncirclearc(1.0, thetamax,
		       (ay_nurbcurve_object **) (&(trim->refine)));


  if(trim->refine)
    {
      ((ay_nurbcurve_object *)(trim->refine))->glu_sampling_tolerance =
	nc->glu_sampling_tolerance;
      ((ay_nurbcurve_object *)(trim->refine))->display_mode =
	nc->display_mode;
    }

  trim->scalx = 0.5;
  trim->scaly = 0.5;
  trim->movx = 0.5;
  trim->movy = 0.5;

  ay_nct_revert((ay_nurbcurve_object *)trim->refine);

  cap->down = trim;

  ay_status = ay_object_crtendlevel(&(trim->next));

  if(!(tloop = calloc(1, sizeof(ay_object))))
    {return AY_EOMEM;}
  ay_object_defaults(tloop);
  tloop->type = AY_IDLEVEL;

  if(!(tloop->refine = calloc(1, sizeof(ay_level_object))))
    {return AY_EOMEM;}

  tloop->down = cap->down;
  cap->down = tloop;

  /* create another trimcurve */
  if(!(trim = calloc(1, sizeof(ay_object))))
    {return AY_EOMEM;}
  ay_object_defaults(trim);
  trim->type = AY_IDNCURVE;
  if(!(controlv = calloc(3*4, sizeof(double))))
    {return AY_EOMEM;}

  nc = tloop->down->refine;

  ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
		     nc->knotv, nc->controlv,
		     nc->knotv[nc->length], P1);

  memcpy(controlv, P1, 3*sizeof(double));

  controlv[3] = 1.0;
  controlv[7] = 1.0;
  controlv[11] = 1.0;

  ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
		     nc->knotv, nc->controlv,
		     nc->knotv[nc->order-1], P1);

  memcpy(&(controlv[8]), P1, 3*sizeof(double));

  ay_status = ay_nct_create(2, 3, AY_KTNURB,
			    controlv, NULL,
			    (ay_nurbcurve_object **)(&(trim->refine)));

  if(trim->refine)
    {
      ((ay_nurbcurve_object *)(trim->refine))->glu_sampling_tolerance =
	nc->glu_sampling_tolerance;
      ((ay_nurbcurve_object *)(trim->refine))->display_mode =
	nc->display_mode;
    }

  trim->scalx = 0.5;
  trim->scaly = 0.5;
  trim->movx = 0.5;
  trim->movy = 0.5;

  ay_nct_revert((ay_nurbcurve_object *)trim->refine);

  trim->next = tloop->down->next;
  tloop->down->next = trim;

  trim = tloop->down;
  ay_nct_revert((ay_nurbcurve_object *)trim->refine);

  /* properly terminate level */
  tloop = cap->down;
  if(tloop)
    {
      while(tloop->next)
	{
	  tloop = tloop->next;
	}
      ay_status = ay_object_crtendlevel(&(tloop->next));
    }
  else
    {
      ay_status = ay_object_crtendlevel(&(trim->down));
    }

  *o = cap;

 return AY_OK;
} /* ay_swing_crtcap */


int
ay_swing_crtside(ay_swing_object *swing, ay_object *curve,
		 ay_object **o)
{
#if 0
 int ay_status = AY_OK;
 int mode = 0, i = 0;
 double P1[4] = {0}, P2[4] = {0}, PS[4] = {0}, PE[4] = {0}, *controlv = NULL;
 double *ccv = NULL, tolerance, minx, maxx, miny, maxy;
 double yaxis[3] = {0.0,1.0,0.0}, quat[4] = {0}, angle = 0.0;
 double m[16];
 ay_object *cap = NULL, *trim = NULL, *tloop = NULL;
 ay_nurbcurve_object *nc = NULL, *tc = NULL;
 int closed;

  nc = (ay_nurbcurve_object *)curve->refine;
  ccv = nc->controlv;

  closed = ay_nct_isclosed(nc);

  mode = swing->display_mode;
  tolerance = swing->glu_sampling_tolerance;

  /* get curves transformation-matrix */
  ay_trafo_creatematrix(curve, m);

  ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
		     nc->knotv, nc->controlv,
		     nc->knotv[nc->order - 1], P1);

  /* apply transform */
  AY_APTRAN3(PS,P1,m)

  ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
		     nc->knotv, nc->controlv,
		     nc->knotv[nc->length], P1);

  /* apply transform */
  AY_APTRAN3(PE,P1,m)

  /* create NURBS patch */
  if(!(cap = calloc(1, sizeof(ay_object))))
    {return AY_EOMEM;}
  ay_object_defaults(cap);
  cap->type = AY_IDNPATCH;
  cap->hide_children = AY_TRUE;
  cap->parent = AY_TRUE;
  cap->inherit_trafos = AY_FALSE;

  if(!(controlv = calloc(4*4, sizeof(double))))
    {free(cap); return AY_EOMEM;}

  /* calc minx, maxx, miny, maxy from curve */
  minx = miny = DBL_MAX;
  maxx = maxy = DBL_MIN;
  for(i = 0; i < nc->length*4; i+=4)
    {
      memcpy(P1, &(ccv[i]), 4*sizeof(double));
      AY_APTRAN3(P2, P1, m)
      if(P2[0] < minx)
	minx = P2[0];
      if(P2[0] > maxx)
	maxx = P2[0];
      if(P2[1] < miny)
	miny = P2[1];
      if(P2[1] > maxy)
	maxy = P2[1];
    }

  /* fill controlv */
  if(!closed)
    {
      /* P1 {0, PS.y, 0} */
      controlv[1] = miny/*PS[1]*/;
      controlv[3] = 1.0;
      /* P2 {0, PE.y, 0} */
      controlv[5] = maxy/*PE[1]*/;
      controlv[7] = 1.0;
      /* P3 {maxx, PS.y, 0} */
      controlv[8] = maxx;
      controlv[9] = miny/*PS[1]*/;
      controlv[11] = 1.0;
      /* P4 {maxx, PE.y, 0} */
      controlv[12] = maxx;
      controlv[13] = maxy/*PE[1]*/;
      controlv[15] = 1.0;
    }
  else
    {
      /* P1 {minx, miny, 0} */
      controlv[0] = minx;
      controlv[1] = miny;
      controlv[3] = 1.0;
      /* P2 {0, PE.y, 0} */
      controlv[4] = minx;
      controlv[5] = maxy;
      controlv[7] = 1.0;
      /* P3 {maxx, PS.y, 0} */
      controlv[8] = maxx;
      controlv[9] = miny;
      controlv[11] = 1.0;
      /* P4 {maxx, PE.y, 0} */
      controlv[12] = maxx;
      controlv[13] = maxy;
      controlv[15] = 1.0;
    }

  ay_status = ay_npt_create(2, 2, 2, 2,
			    AY_KTBEZIER, AY_KTBEZIER,
			    controlv, NULL, NULL,
			    (ay_nurbpatch_object **)(&(cap->refine)));

  ((ay_nurbpatch_object *)cap->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)cap->refine)->display_mode = mode;
  /*
  if(th != 0.0)
    {
      ay_quat_axistoquat(yaxis, AY_D2R(th), quat);
      ay_quat_add(quat, cap->quat, cap->quat);
    }
  */
  /* create trimcurve(s) */
  ay_object_copy(curve, &trim);
  tc = (ay_nurbcurve_object *)trim->refine;
  ccv = tc->controlv;
  for(i = 0; i < tc->length*4; i+=4)
    {
      memcpy(P1, &(ccv[i]), 4*sizeof(double));
      AY_APTRAN4(P2, P1, m)
      memcpy(&(ccv[i]), P2, 4*sizeof(double));
    }

  ay_trafo_defaults(trim);

  cap->down = trim;
  if(!closed)
    {
      trim->scalx /= maxx;
      trim->scaly /= (maxy-miny);

      if(miny < 0.0)
	{
	  trim->movy = fabs(miny)*trim->scaly;
	}
      else
	{
	  trim->movy = -fabs(miny)*trim->scaly;
	}
      /* XXXX add reversion of curve, if maxy is at end of curve ?
      if(???)
      ay_nct_revert((ay_nurbcurve_object *)(trim->refine));
      */
    }
  else
    {
      trim->scalx /= (maxx-minx);
      trim->scaly /= (maxy-miny);
      trim->movx = -(minx + (fabs(maxx-minx)/2.0))*trim->scalx;
      trim->movy = -(miny + (fabs(maxy-miny)/2.0))*trim->scaly;

      trim->movx += 0.5;
      trim->movy += 0.5;

      ay_status = ay_nct_getorientation((ay_nurbcurve_object *)
					trim->refine, &angle);

      if(angle<0.0)
	ay_nct_revert(trim->refine);

    } /* if !closed */

  ay_status = ay_object_crtendlevel(&(trim->next));

  if(!closed)
    {
      if(!(tloop = calloc(1, sizeof(ay_object))))
	{return AY_EOMEM;}
      ay_object_defaults(tloop);
      tloop->type = AY_IDLEVEL;

      if(!(tloop->refine = calloc(1, sizeof(ay_level_object))))
	return AY_EOMEM;

      tloop->down = cap->down;
      cap->down = tloop;

      /* create another trimcurve */
      if(!(trim = calloc(1, sizeof(ay_object))))
	{return AY_EOMEM;}
      ay_object_defaults(trim);
      trim->type = AY_IDNCURVE;
      if(!(controlv = calloc(4*4, sizeof(double))))
	{return AY_EOMEM;}

      nc = tloop->down->refine;

      /* P1 {PS.x, PS.y, 0} */
      memcpy(controlv, PS, 2*sizeof(double));
      controlv[3] = 1.0;

      /* P2 {0, PS.y, 0} */
      controlv[5] = PS[1];
      controlv[7] = 1.0;

      /* P3 {0, PE.y, 0} */
      controlv[9] = PE[1];
      controlv[11] = 1.0;

      /* P4 {PE.x, PE.y, 0} */
      memcpy(&(controlv[12]), PE, 2*sizeof(double));
      controlv[15] = 1.0;

      ay_status = ay_nct_create(2, 4, AY_KTNURB,
				controlv, NULL,
				(ay_nurbcurve_object **)(&(trim->refine)));

      if(trim->refine)
	{
	  ((ay_nurbcurve_object *)(trim->refine))->glu_sampling_tolerance =
	    nc->glu_sampling_tolerance;
	  ((ay_nurbcurve_object *)(trim->refine))->display_mode =
	    nc->display_mode;
	}

      trim->scalx /= maxx;
      trim->scaly /= (maxy-miny);
      if(miny < 0.0)
	{
	  trim->movy = fabs(miny)*trim->scaly;
	}
      else
	{
	  trim->movy = -fabs(miny)*trim->scaly;
	}

      ay_nct_revert((ay_nurbcurve_object *)(trim->refine));

      trim->next = tloop->down->next;
      tloop->down->next = trim;

      trim = tloop->down;

      /* properly terminate level */
      tloop = cap->down;
      if(tloop)
	{
	  while(tloop->next)
	    {
	      tloop = tloop->next;
	    }
	  ay_status = ay_object_crtendlevel(&(tloop->next));
	}
      else
	{
	  ay_status = ay_object_crtendlevel(&(trim->down));
	}
    } /* if !closed */

  *o = cap;
#endif
 return AY_OK;
} /* ay_swing_crtside */


int
ay_swing_notifycb(ay_object *o)
{
 ay_swing_object *swing = NULL;
 ay_object *cs = NULL, *tr = NULL, *pobject = NULL, *npatch = NULL;
 ay_nurbcurve_object *nc = NULL;
 int ay_status = AY_OK;
 int provided_cs = AY_FALSE, provided_tr = AY_FALSE, mode = 0;
 double tolerance;

  if(!o)
    return AY_ENULL;

  swing = (ay_swing_object *)(o->refine);

  mode = swing->display_mode;
  tolerance = swing->glu_sampling_tolerance;

  /* remove old objects */
  if(swing->npatch)
    ay_object_delete(swing->npatch);
  swing->npatch = NULL;

  if(swing->upper_cap)
    ay_object_delete(swing->upper_cap);
  swing->upper_cap = NULL;

  if(swing->lower_cap)
    ay_object_delete(swing->lower_cap);
  swing->lower_cap = NULL;

  if(swing->start_cap)
    ay_object_delete(swing->start_cap);
  swing->start_cap = NULL;

  if(swing->end_cap)
    ay_object_delete(swing->end_cap);
  swing->end_cap = NULL;

  /* get curve to swing */
  if(!o->down || !o->down->next)
    return AY_OK;
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
  if(!(npatch = calloc(1, sizeof(ay_object))))
    {
      ay_status =  AY_EOMEM;
      goto cleanup;
    }

  ay_object_defaults(npatch);
  npatch->type = AY_IDNPATCH;

  ay_status = ay_npt_swing(cs, tr,
			   (ay_nurbpatch_object **)(&(npatch->refine)));

  if(ay_status)
    return ay_status;

  swing->npatch = npatch;

  ((ay_nurbpatch_object *)npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)npatch->refine)->display_mode =
    mode;

  nc = (ay_nurbcurve_object *)cs->refine;

  /* create caps */
  if(swing->has_upper_cap)
    {
      ay_status = ay_swing_crtcap(swing, cs,
				    nc->knotv[nc->length], AY_TRUE,
				    &(swing->upper_cap));

    } /* if */

  if(swing->has_lower_cap)
    {
      ay_status = ay_swing_crtcap(swing, cs,
				    nc->knotv[nc->order - 1], AY_FALSE,
				    &(swing->lower_cap));
    } /* if */

  if(swing->has_start_cap)
    {
      ay_status = ay_swing_crtside(swing, cs, &(swing->start_cap));
      if(swing->start_cap)
	swing->start_cap->scalz *= -1.0;

    } /* if */

  if(swing->has_end_cap)
    {
      ay_status = ay_swing_crtside(swing, cs, &(swing->end_cap));

    } /* if */

  /* remove provided object */
cleanup:
  if(provided_cs && cs)
    {
      ay_object_delete(cs);
    }
  if(provided_tr && tr)
    {
      ay_object_delete(tr);
    }

 return ay_status;
} /* ay_swing_notifycb */


int
ay_swing_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "swing_providecb";
 ay_swing_object *r = NULL;
 ay_object *new = NULL, **t = NULL, *p = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNPATCH)
	return AY_OK;
      else
	return AY_ERROR;
    }

  r = (ay_swing_object *) o->refine;

  if(type == AY_IDNPATCH)
    {
      t = &(new);

      /* copy revolution */
      p = r->npatch;
      while(p)
	{
	  ay_status = ay_object_copy(p, t);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, NULL);
	      return AY_ERROR;
	    }

	  t = &((*t)->next);
	  p = p->next;
	} /* while */

      /* copy caps */
      p = r->upper_cap;
      while(p)
	{
	  ay_status = ay_object_copy(p, t);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, NULL);
	      return AY_ERROR;
	    }
	  t = &((*t)->next);
	  p = p->next;
	} /* while */

      p = r->lower_cap;
      while(p)
	{
	  ay_status = ay_object_copy(p, t);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, NULL);
	      return AY_ERROR;
	    }
	  t = &((*t)->next);
	  p = p->next;
	} /* while */

      p = r->start_cap;
      while(p)
	{
	  ay_status = ay_object_copy(p, t);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, NULL);
	      return AY_ERROR;
	    }
	  t = &((*t)->next);
	  p = p->next;
	} /* while */

      p = r->end_cap;
      while(p)
	{
	  ay_status = ay_object_copy(p, t);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, NULL);
	      return AY_ERROR;
	    }
	  t = &((*t)->next);
	  p = p->next;
	} /* while */

      /* copy eventually present TP tags */
      ay_npt_copytptag(o, new);

      *result = new;
    } /* if */

 return ay_status;
} /* ay_swing_providecb */

int
ay_swing_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_swing_object *r = NULL;
 ay_object *new = NULL, **next = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */

  r = (ay_swing_object *) o->refine;

  if((r->upper_cap) || (r->lower_cap) || (r->start_cap) || (r->end_cap))
    {
      if(!(new = calloc(1, sizeof(ay_object))))
	{ return AY_EOMEM; }

      ay_object_defaults(new);
      new->type = AY_IDLEVEL;
      new->parent = AY_TRUE;
      new->inherit_trafos = AY_TRUE;
      ay_trafo_copy(o, new);

      if(!(new->refine = calloc(1, sizeof(ay_level_object))))
	{ free(new); return AY_EOMEM; }

      ((ay_level_object *)(new->refine))->type = AY_LTLEVEL;

      next = &(new->down);

      if(r->npatch)
	{
	  ay_status = ay_object_copy(r->npatch, next);
	  if(*next)
	    {
	      (*next)->hide_children = AY_TRUE;
	      (*next)->parent = AY_TRUE;
	      ay_object_crtendlevel(&(*next)->down);
	      next = &((*next)->next);
	    }
	} /* if */

      if(r->upper_cap)
	{
	  ay_status = ay_object_copy(r->upper_cap, next);
	  if(*next)
	    next = &((*next)->next);
	}

      if(r->lower_cap)
	{
	  ay_status = ay_object_copy(r->lower_cap, next);
	  if(*next)
	    next = &((*next)->next);
	}

      if(r->start_cap)
	{
	  ay_status = ay_object_copy(r->start_cap, next);
	  if(*next)
	    next = &((*next)->next);
	}

      if(r->end_cap)
	{
	  ay_status = ay_object_copy(r->end_cap, next);
	  if(*next)
	    next = &((*next)->next);
	}

      /* copy eventually present TP tags */
      ay_npt_copytptag(o, new->down);

      ay_object_crtendlevel(next);
    }
  else
    {
      if(r->npatch)
	{
	  ay_status = ay_object_copy(r->npatch, &new);
	  ay_trafo_copy(o, new);
	  new->hide_children = AY_TRUE;
	  new->parent = AY_TRUE;
	  ay_object_crtendlevel(&(new->down));

	  /* copy eventually present TP tags */
	  ay_npt_copytptag(o, new);
	} /* if */
    } /* if */

  /* second, link new objects, or replace old objects with them */

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
} /* ay_swing_convertcb */


int
ay_swing_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_swing_name,
				    ay_swing_createcb,
				    ay_swing_deletecb,
				    ay_swing_copycb,
				    ay_swing_drawcb,
				    NULL, /* no handles */
				    ay_swing_shadecb,
				    ay_swing_setpropcb,
				    ay_swing_getpropcb,
				    ay_swing_getpntcb,
				    ay_swing_readcb,
				    ay_swing_writecb,
				    ay_swing_wribcb,
				    ay_swing_bbccb,
				    AY_IDSWING);


  ay_status = ay_notify_register(ay_swing_notifycb, AY_IDSWING);

  ay_status = ay_convert_register(ay_swing_convertcb, AY_IDSWING);

  ay_status = ay_provide_register(ay_swing_providecb, AY_IDSWING);

 return ay_status;
} /* ay_swing_init */

