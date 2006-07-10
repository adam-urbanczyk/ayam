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

/* revolve.c - revolve object */

static char *ay_revolve_name = "Revolve";

int
ay_revolve_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtrevolve";
 ay_revolve_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_revolve_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->thetamax = 360.0;

  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_revolve_createcb */


int
ay_revolve_deletecb(void *c)
{
 ay_revolve_object *revolve = NULL;

  if(!c)
    return AY_ENULL;    

  revolve = (ay_revolve_object *)(c);

  if(revolve->npatch)
    ay_object_delete(revolve->npatch);

  if(revolve->upper_cap)
    ay_object_delete(revolve->upper_cap);

  if(revolve->lower_cap)
    ay_object_delete(revolve->lower_cap);

  if(revolve->start_cap)
    ay_object_delete(revolve->start_cap);

  if(revolve->end_cap)
    ay_object_delete(revolve->end_cap);

  free(revolve);

 return AY_OK;
} /* ay_revolve_deletecb */


int
ay_revolve_copycb(void *src, void **dst)
{
 ay_revolve_object *revolve = NULL, *revolvesrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  revolvesrc = (ay_revolve_object *)src;

  if(!(revolve = calloc(1, sizeof(ay_revolve_object))))
    return AY_EOMEM; 

  memcpy(revolve, src, sizeof(ay_revolve_object)); 


  /* copy npatch */
  ay_object_copy(revolvesrc->npatch, &(revolve->npatch));

  /* copy upper cap */
  ay_object_copy(revolvesrc->upper_cap, &(revolve->upper_cap));

  /* copy lower cap */
  ay_object_copy(revolvesrc->lower_cap, &(revolve->lower_cap));

  /* copy start cap */
  ay_object_copy(revolvesrc->start_cap, &(revolve->start_cap));

  /* copy end cap */
  ay_object_copy(revolvesrc->end_cap, &(revolve->end_cap));


  *dst = (void *)revolve;

 return AY_OK;
} /* ay_revolve_copycb */


int
ay_revolve_drawcb(struct Togl *togl, ay_object *o)
{
 ay_revolve_object *revolve = NULL;

  if(!o)
    return AY_ENULL;

  revolve = (ay_revolve_object *)o->refine;

  if(!revolve)
    return AY_ENULL;

  if(revolve->npatch)
    ay_draw_object(togl, revolve->npatch, AY_TRUE);

  if(revolve->upper_cap)
    ay_draw_object(togl, revolve->upper_cap, AY_TRUE);

  if(revolve->lower_cap)
    ay_draw_object(togl, revolve->lower_cap, AY_TRUE);

  if(revolve->start_cap)
    ay_draw_object(togl, revolve->start_cap, AY_TRUE);

  if(revolve->end_cap)
    ay_draw_object(togl, revolve->end_cap, AY_TRUE);

 return AY_OK;
} /* ay_revolve_drawcb */


int
ay_revolve_shadecb(struct Togl *togl, ay_object *o)
{
 ay_revolve_object *revolve = NULL;

  if(!o)
    return AY_ENULL;

  revolve = (ay_revolve_object *)o->refine;

  if(!revolve)
    return AY_ENULL;

  if(revolve->npatch)
    ay_shade_object(togl, revolve->npatch, AY_FALSE);

  if(revolve->upper_cap)
    ay_shade_object(togl, revolve->upper_cap, AY_FALSE);

  if(revolve->lower_cap)
    ay_shade_object(togl, revolve->lower_cap, AY_FALSE);

  if(revolve->start_cap)
    ay_shade_object(togl, revolve->start_cap, AY_FALSE);

  if(revolve->end_cap)
    ay_shade_object(togl, revolve->end_cap, AY_FALSE);

 return AY_OK;
} /* ay_revolve_shadecb */


int
ay_revolve_drawhcb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_revolve_drawhcb */


int
ay_revolve_getpntcb(int mode, ay_object *o, double *p)
{

 return AY_OK;
} /* ay_revolve_getpntcb */


/* Tcl -> C! */
int
ay_revolve_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "RevolveAttrData";
 /* char fname[] = "revolve_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_revolve_object *revolve = NULL;


  if(!o)
    return AY_ENULL;

  revolve = (ay_revolve_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(revolve->thetamax));

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(revolve->sections));

  Tcl_SetStringObj(ton,"Order",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(revolve->order));

  Tcl_SetStringObj(ton,"UpperCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(revolve->has_upper_cap));

  Tcl_SetStringObj(ton,"LowerCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(revolve->has_lower_cap));

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(revolve->has_start_cap));

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(revolve->has_end_cap));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(revolve->glu_display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(revolve->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_force(o);

  o->modified = AY_TRUE;
  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_revolve_setpropcb */


/* C -> Tcl! */
int
ay_revolve_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="RevolveAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_revolve_object *revolve = NULL;

  if(!o)
    return AY_ENULL;

  revolve = (ay_revolve_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_NewDoubleObj(revolve->thetamax);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_NewIntObj(revolve->sections);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order",-1);
  to = Tcl_NewIntObj(revolve->order);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"UpperCap",-1);
  to = Tcl_NewIntObj(revolve->has_upper_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"LowerCap",-1);
  to = Tcl_NewIntObj(revolve->has_lower_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_NewIntObj(revolve->has_start_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_NewIntObj(revolve->has_end_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(revolve->glu_display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(revolve->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_revolve_getpropcb */


int
ay_revolve_readcb(FILE *fileptr, ay_object *o)
{
 ay_revolve_object *revolve = NULL;

 if(!o)
   return AY_ENULL;

  if(!(revolve = calloc(1, sizeof(ay_revolve_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%lg\n",&revolve->thetamax);
  fscanf(fileptr,"%d\n",&revolve->has_upper_cap);
  fscanf(fileptr,"%d\n",&revolve->has_lower_cap);
  fscanf(fileptr,"%d\n",&revolve->has_start_cap);
  fscanf(fileptr,"%d\n",&revolve->has_end_cap);
  fscanf(fileptr,"%d\n",&revolve->glu_display_mode);
  fscanf(fileptr,"%lg\n",&revolve->glu_sampling_tolerance);
  if(ay_read_version >= 7)
    {
      fscanf(fileptr,"%d\n",&revolve->sections);
      fscanf(fileptr,"%d\n",&revolve->order);
    }

  o->refine = revolve;

 return AY_OK;
} /* ay_revolve_readcb */


int
ay_revolve_writecb(FILE *fileptr, ay_object *o)
{
 ay_revolve_object *revolve = NULL;

  if(!o)
    return AY_ENULL;

  revolve = (ay_revolve_object *)(o->refine);

  fprintf(fileptr, "%g\n", revolve->thetamax);
  fprintf(fileptr, "%d\n", revolve->has_upper_cap);
  fprintf(fileptr, "%d\n", revolve->has_lower_cap);
  fprintf(fileptr, "%d\n", revolve->has_start_cap);
  fprintf(fileptr, "%d\n", revolve->has_end_cap);
  fprintf(fileptr, "%d\n", revolve->glu_display_mode);
  fprintf(fileptr, "%g\n", revolve->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", revolve->sections);
  fprintf(fileptr, "%d\n", revolve->order);

 return AY_OK;
} /* ay_revolve_writecb */


int
ay_revolve_wribcb(char *file, ay_object *o)
{
 ay_revolve_object *revolve = NULL;

  if(!o)
   return AY_ENULL;

  revolve = (ay_revolve_object*)o->refine;

  if(revolve->npatch)
    ay_wrib_object(file, revolve->npatch);

  if(revolve->upper_cap)
    ay_wrib_object(file, revolve->upper_cap);

  if(revolve->lower_cap)
    ay_wrib_object(file, revolve->lower_cap);

  if(revolve->start_cap)
    ay_wrib_object(file, revolve->start_cap);

  if(revolve->end_cap)
    ay_wrib_object(file, revolve->end_cap);

 return AY_OK;
} /* ay_revolve_wribcb */


int
ay_revolve_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_revolve_object *revolve = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  revolve = (ay_revolve_object *)o->refine; 

  if(revolve->npatch)
    {
      ay_bbc_get(revolve->npatch, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return AY_OK;
} /* ay_revolve_bbccb */


int
ay_revolve_crtcap(ay_revolve_object *revolve, ay_object *curve,
		  double u, int upper,
		  ay_object **o)
{
 int ay_status = AY_OK;
 int mode = 0, i = 0;
 double P1[4] = {0}, P2[4] = {0}, *controlv = NULL;
 double tolerance;
 double m[16];
 ay_object *cap = NULL, *trim = NULL, *tloop = NULL;
 ay_nurbcurve_object *nc = NULL;

  nc = (ay_nurbcurve_object *)curve->refine;

  mode = revolve->glu_display_mode;
  tolerance = revolve->glu_sampling_tolerance;

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
  ((ay_nurbpatch_object *)cap->refine)->glu_display_mode = mode;

  /* create trimcurve */
  if(!(trim = calloc(1, sizeof(ay_object))))
    {return AY_EOMEM;}
  ay_object_defaults(trim);
  trim->type = AY_IDNCURVE;

  if(fabs(revolve->thetamax) != 360.0)
    {
      ay_nct_crtncirclearc(1.0, revolve->thetamax,
			   (ay_nurbcurve_object **) (&(trim->refine)));
    }
  else
    {
      ay_nct_crtncircle(1.0, (ay_nurbcurve_object **) (&(trim->refine)));
    }

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

  if(fabs(revolve->thetamax) != 360.0)
    {
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

    } /* if */

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
} /* ay_revolve_crtcap */


int
ay_revolve_crtside(ay_revolve_object *revolve, ay_object *curve, double th,
		  ay_object **o)
{
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

  mode = revolve->glu_display_mode;
  tolerance = revolve->glu_sampling_tolerance;

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
  ((ay_nurbpatch_object *)cap->refine)->glu_display_mode = mode;

  if(th != 0.0)
    {
      ay_quat_axistoquat(yaxis, AY_D2R(th), quat);
      ay_quat_add(quat, cap->quat, cap->quat);
    }

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

      ay_nct_getorientation((ay_nurbcurve_object *)
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

 return AY_OK;
} /* ay_revolve_crtside */


int
ay_revolve_notifycb(ay_object *o)
{
 ay_revolve_object *revolve = NULL;
 ay_object *curve = NULL, *pobject = NULL, *npatch = NULL;
 ay_nurbcurve_object *nc = NULL;
 int ay_status = AY_OK;
 int got_object = AY_FALSE, mode = 0;
 double tolerance;

  if(!o)
    return AY_ENULL;    

  revolve = (ay_revolve_object *)(o->refine);

  mode = revolve->glu_display_mode;
  tolerance = revolve->glu_sampling_tolerance;

  /* remove old objects */
  if(revolve->npatch)
    ay_object_delete(revolve->npatch);
  revolve->npatch = NULL;

  if(revolve->upper_cap)
    ay_object_delete(revolve->upper_cap);
  revolve->upper_cap = NULL;

  if(revolve->lower_cap)
    ay_object_delete(revolve->lower_cap);
  revolve->lower_cap = NULL;

  if(revolve->start_cap)
    ay_object_delete(revolve->start_cap);
  revolve->start_cap = NULL;

  if(revolve->end_cap)
    ay_object_delete(revolve->end_cap);
  revolve->end_cap = NULL;

  /* get curve to revolve */
  if(!o->down)
    return AY_OK;
  curve = o->down;
  if(curve->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(curve, AY_IDNCURVE, &pobject);
      if(!pobject)
	{
	  return AY_OK;
	}
      else
	{
	  curve = pobject;
	  got_object = AY_TRUE;
	} /* if */
    } /* if */

  /* revolve */
  if(!(npatch = calloc(1, sizeof(ay_object))))
    {
      return AY_ERROR;
    }

  ay_object_defaults(npatch);
  npatch->type = AY_IDNPATCH;

  ay_status = ay_npt_revolve(curve, revolve->thetamax, revolve->sections,
			     revolve->order,
			     (ay_nurbpatch_object **)(&(npatch->refine)));

  if(ay_status)
    return ay_status;
  
  revolve->npatch = npatch;

  ((ay_nurbpatch_object *)npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)npatch->refine)->glu_display_mode =
    mode;

  nc = (ay_nurbcurve_object *)curve->refine;

  /* create caps */
  if(revolve->has_upper_cap)
    {
      ay_status = ay_revolve_crtcap(revolve, curve,
				    nc->knotv[nc->length], AY_TRUE,
				    &(revolve->upper_cap));
  
    } /* if */

  if(revolve->has_lower_cap)
    {
      ay_status = ay_revolve_crtcap(revolve, curve,
				    nc->knotv[nc->order - 1], AY_FALSE,
				    &(revolve->lower_cap));
    } /* if */

  if(revolve->has_start_cap)
    {
      ay_status = ay_revolve_crtside(revolve, curve, 0.0,
				     &(revolve->start_cap));
      if(revolve->start_cap)
	revolve->start_cap->scalz *= -1.0;
  
    } /* if */

  if(revolve->has_end_cap)
    {
      ay_status = ay_revolve_crtside(revolve, curve, revolve->thetamax,
				    &(revolve->end_cap));
  
    } /* if */

  /* remove provided object */
  if(got_object)
    {
      ay_object_delete(pobject);
    }

 return AY_OK;
} /* ay_revolve_notifycb */


int
ay_revolve_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "revolve_providecb";
 ay_revolve_object *r = NULL;
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

  r = (ay_revolve_object *) o->refine;

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
	  ay_trafo_copy(o, *t);

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
	  ay_trafo_add(o, *t);
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
	  ay_trafo_add(o, *t);
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
	  ay_trafo_add(o, *t);
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
	  ay_trafo_add(o, *t);
	  t = &((*t)->next);
	  p = p->next;
	} /* while */

      /* copy eventually present TP tags */
      ay_npt_copytptag(o, new);

      *result = new;
    } /* if */

 return ay_status;
} /* ay_revolve_providecb */

int
ay_revolve_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_revolve_object *r = NULL;
 ay_object *new = NULL, **next = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */

  r = (ay_revolve_object *) o->refine;

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

	}

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
} /* ay_revolve_convertcb */


int
ay_revolve_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_revolve_name,
				    ay_revolve_createcb,
				    ay_revolve_deletecb,
				    ay_revolve_copycb,
				    ay_revolve_drawcb,
				    NULL, /* no handles */
				    ay_revolve_shadecb,
				    ay_revolve_setpropcb,
				    ay_revolve_getpropcb,
				    ay_revolve_getpntcb,
				    ay_revolve_readcb,
				    ay_revolve_writecb,
				    ay_revolve_wribcb,
				    ay_revolve_bbccb,
				    AY_IDREVOLVE);

  
  ay_status = ay_notify_register(ay_revolve_notifycb, AY_IDREVOLVE);

  ay_status = ay_convert_register(ay_revolve_convertcb, AY_IDREVOLVE);

  ay_status = ay_provide_register(ay_revolve_providecb, AY_IDREVOLVE);

 return ay_status;
} /* ay_revolve_init */

