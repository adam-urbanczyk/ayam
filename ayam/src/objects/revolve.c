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

/* revolve.c - revolve object */

static char *ay_revolve_name = "Revolve";

int ay_revolve_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_revolve_createcb:
 *  create callback function of revolve object
 */
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


/* ay_revolve_deletecb:
 *  delete callback function of revolve object
 */
int
ay_revolve_deletecb(void *c)
{
 ay_revolve_object *revolve = NULL;

  if(!c)
    return AY_ENULL;

  revolve = (ay_revolve_object *)(c);

  if(revolve->npatch)
    (void)ay_object_delete(revolve->npatch);

  if(revolve->upper_cap)
    (void)ay_object_delete(revolve->upper_cap);

  if(revolve->lower_cap)
    (void)ay_object_delete(revolve->lower_cap);

  if(revolve->start_cap)
    (void)ay_object_delete(revolve->start_cap);

  if(revolve->end_cap)
    (void)ay_object_delete(revolve->end_cap);

  free(revolve);

 return AY_OK;
} /* ay_revolve_deletecb */


/* ay_revolve_copycb:
 *  copy callback function of revolve object
 */
int
ay_revolve_copycb(void *src, void **dst)
{
 ay_revolve_object *revolve = NULL, *revolvesrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  revolvesrc = (ay_revolve_object *)src;

  if(!(revolve = malloc(sizeof(ay_revolve_object))))
    return AY_EOMEM;

  memcpy(revolve, src, sizeof(ay_revolve_object));

  revolve->npatch = NULL;
  revolve->upper_cap = NULL;
  revolve->lower_cap = NULL;
  revolve->start_cap = NULL;
  revolve->end_cap = NULL;

  *dst = (void *)revolve;

 return AY_OK;
} /* ay_revolve_copycb */


/* ay_revolve_drawcb:
 *  draw (display in an Ayam view window) callback function of revolve object
 */
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


/* ay_revolve_shadecb:
 *  shade (display in an Ayam view window) callback function of revolve object
 */
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


/* ay_revolve_drawhcb:
 *  draw handles (in an Ayam view window) callback function of revolve object
 */
int
ay_revolve_drawhcb(struct Togl *togl, ay_object *o)
{
 int i;
 double *pnts;
 ay_revolve_object *revolve;
 ay_nurbpatch_object *patch;

  if(!o)
    return AY_ENULL;

  revolve = (ay_revolve_object *) o->refine;

  if(revolve->npatch)
    {
      patch = (ay_nurbpatch_object *)revolve->npatch->refine;
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
} /* ay_revolve_drawhcb */


/* ay_revolve_getpntcb:
 *  get point (editing and selection) callback function of revolve object
 */
int
ay_revolve_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbpatch_object *patch = NULL;
 ay_revolve_object *revolve = NULL;

  if(!o)
    return AY_ENULL;

  revolve = (ay_revolve_object *)o->refine;

  if(revolve->npatch)
    {
      patch = (ay_nurbpatch_object *)revolve->npatch->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, patch->width*patch->height, 4,
			     patch->controlv);
    }

 return AY_ERROR;
} /* ay_revolve_getpntcb */


/* ay_revolve_setpropcb:
 *  set property (from Tcl to C context) callback function of revolve object
 */
int
ay_revolve_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
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
  Tcl_GetIntFromObj(interp,to, &(revolve->display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(revolve->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_notify_object(o);

  o->modified = AY_TRUE;

  ay_notify_parent();

 return AY_OK;
} /* ay_revolve_setpropcb */


/* ay_revolve_getpropcb:
 *  get property (from C to Tcl context) callback function of revolve object
 */
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
  to = Tcl_NewIntObj(revolve->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(revolve->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getnpinfo(interp, n1, revolve->npatch);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_revolve_getpropcb */


/* ay_revolve_readcb:
 *  read (from scene file) callback function of revolve object
 */
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
  fscanf(fileptr,"%d\n",&revolve->display_mode);
  fscanf(fileptr,"%lg\n",&revolve->glu_sampling_tolerance);

  if(ay_read_version >= 7)
    {
      /* since 1.8 */
      fscanf(fileptr,"%d\n",&revolve->sections);
      fscanf(fileptr,"%d\n",&revolve->order);
    }

  o->refine = revolve;

 return AY_OK;
} /* ay_revolve_readcb */


/* ay_revolve_writecb:
 *  write (to scene file) callback function of revolve object
 */
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
  fprintf(fileptr, "%d\n", revolve->display_mode);
  fprintf(fileptr, "%g\n", revolve->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", revolve->sections);
  fprintf(fileptr, "%d\n", revolve->order);

 return AY_OK;
} /* ay_revolve_writecb */


/* ay_revolve_wribcb:
 *  RIB export callback function of revolve object
 */
int
ay_revolve_wribcb(char *file, ay_object *o)
{
 ay_revolve_object *revolve = NULL;

  if(!o)
   return AY_ENULL;

  revolve = (ay_revolve_object*)o->refine;

  if(revolve->npatch)
    ay_wrib_toolobject(file, revolve->npatch, o);

  if(revolve->upper_cap)
    ay_wrib_toolobject(file, revolve->upper_cap, o);

  if(revolve->lower_cap)
    ay_wrib_toolobject(file, revolve->lower_cap, o);

  if(revolve->start_cap)
    ay_wrib_toolobject(file, revolve->start_cap, o);

  if(revolve->end_cap)
    ay_wrib_toolobject(file, revolve->end_cap, o);

 return AY_OK;
} /* ay_revolve_wribcb */


/* ay_revolve_bbccb:
 *  bounding box calculation callback function of revolve object
 */
int
ay_revolve_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_revolve_object *revolve = NULL;

  if(!o || !bbox || !flags)
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


/* ay_revolve_crtcap:
 *  create top or bottom revolve cap
 */
int
ay_revolve_crtcap(ay_revolve_object *revolve, ay_object *curve,
		  double u, int upper,
		  ay_object **o)
{
 int ay_status = AY_OK;
 int mode = 0, i = 0, order = 0;
 double P1[4] = {0}, P2[4] = {0}, m[16];
 double tolerance;
 double *controlv = NULL;
 ay_object *cap = NULL, *trim = NULL, *tloop = NULL;
 ay_nurbcurve_object *nc = NULL;

  nc = (ay_nurbcurve_object *)curve->refine;

  mode = revolve->display_mode;
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
    {ay_status = AY_EOMEM; goto cleanup;}

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
      for(i = 1; i <= 15; i += 4)
	controlv[i] = -P2[1];
      cap->scaly *= -1.0;
    }
  else
    {
      for(i = 1; i <= 15; i += 4)
	controlv[i] = P2[1];
    }

  ay_status = ay_npt_create(2, 2, 2, 2,
			    AY_KTBEZIER, AY_KTBEZIER,
			    controlv, NULL, NULL,
			    (ay_nurbpatch_object **)(void*)&(cap->refine));

  if(ay_status || !cap->refine)
    {
      goto cleanup;
    }

  ((ay_nurbpatch_object *)cap->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)cap->refine)->display_mode = mode;
  controlv = NULL;

  /* create trimcurve */
  if(!(trim = calloc(1, sizeof(ay_object))))
    {ay_status = AY_EOMEM; goto cleanup;}
  ay_object_defaults(trim);
  trim->type = AY_IDNCURVE;

  trim->scalx = 0.5;
  trim->scaly = 0.5;
  trim->movx = 0.5;
  trim->movy = 0.5;

  cap->down = trim;
  if(revolve->sections == 0)
    {
      if(fabs(revolve->thetamax) != 360.0)
	{
	  ay_status = ay_nct_crtncirclearc(1.0, revolve->thetamax,
			   (ay_nurbcurve_object **)(void*)&(trim->refine));
	}
      else
	{
	  ay_status = ay_nct_crtncircle(1.0,
			   (ay_nurbcurve_object **)(void*)&(trim->refine));
	}
    }
  else
    {
      if(revolve->order <= 1)
	order = 3;
      else
	order = revolve->order;
      ay_status = ay_nct_crtcircbsp(revolve->sections, 1.0,
				    revolve->thetamax, order,
			      (ay_nurbcurve_object **)(void*)&(trim->refine));
    }

  if(ay_status || !trim->refine)
    {
      goto cleanup;
    }

  ((ay_nurbcurve_object *)(trim->refine))->glu_sampling_tolerance =
    nc->glu_sampling_tolerance;
  ((ay_nurbcurve_object *)(trim->refine))->display_mode =
    nc->display_mode;

  trim->next = ay_endlevel;

  if(fabs(revolve->thetamax) != 360.0)
    {
      if(!(tloop = calloc(1, sizeof(ay_object))))
	{ay_status = AY_EOMEM; goto cleanup;}
      ay_object_defaults(tloop);
      tloop->type = AY_IDLEVEL;

      tloop->down = cap->down;
      cap->down = tloop;

      if(!(tloop->refine = calloc(1, sizeof(ay_level_object))))
	{ay_status = AY_EOMEM; goto cleanup;}

      /* create another trimcurve */
      if(!(trim = calloc(1, sizeof(ay_object))))
	{ay_status = AY_EOMEM; goto cleanup;}
      ay_object_defaults(trim);
      trim->type = AY_IDNCURVE;

      trim->scalx = 0.5;
      trim->scaly = 0.5;
      trim->movx = 0.5;
      trim->movy = 0.5;

      trim->next = tloop->down->next;
      tloop->down->next = trim;

      if(!(controlv = calloc(3*4, sizeof(double))))
	{ay_status = AY_EOMEM; goto cleanup;}

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
			     (ay_nurbcurve_object **)(void*)&(trim->refine));

      if(ay_status || !trim->refine)
	{
	  goto cleanup;
	}

      ((ay_nurbcurve_object *)(trim->refine))->glu_sampling_tolerance =
	nc->glu_sampling_tolerance;
      ((ay_nurbcurve_object *)(trim->refine))->display_mode =
	nc->display_mode;
      controlv = NULL;

      /* properly terminate level */
      tloop = cap->down;
      if(tloop)
	{
	  while(tloop->next)
	    {
	      tloop = tloop->next;
	    }
	  tloop->next = ay_endlevel;
	}


      if(revolve->sections != 0 && revolve->thetamax < 0)
	{
	  ay_nct_revert((ay_nurbcurve_object *)trim->refine);
	  ay_nct_revert((ay_nurbcurve_object *)tloop->down->refine);
	}

    }
  else
    {
      if(revolve->sections == 0 || revolve->thetamax < 0)
	ay_nct_revert((ay_nurbcurve_object *)trim->refine);
    } /* if */

  *o = cap;

  /* prevent cleanup code from doing something harmful */
  cap = NULL;

cleanup:

  if(cap)
    {
      (void)ay_object_deletemulti(cap);
    }

  if(controlv)
    {
      free(controlv);
    }

 return ay_status;
} /* ay_revolve_crtcap */


/* ay_revolve_crtside:
 *  create start or end revolve cap
 */
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
 ay_object *cap = NULL, *trim = NULL, *tloop = NULL, *rc = NULL;
 ay_nurbcurve_object *nc = NULL, *tc = NULL;
 int closed, revert = AY_FALSE;

  nc = (ay_nurbcurve_object *)curve->refine;

  if(revolve->sections != 0)
   {
     ay_status = ay_object_copy(revolve->npatch, &rc);
     if(!ay_status && rc)
       {
	 ay_npt_clampv((ay_nurbpatch_object *)(rc->refine), 0);
	 ay_npt_extractnc(rc, 0, 0.0, AY_FALSE, AY_FALSE, AY_FALSE, NULL, &nc);
	 (void)ay_object_delete(rc);
	 if(!(curve = calloc(1, sizeof(ay_object))))
	   {
	     ay_status = AY_EOMEM;
	     goto cleanup;
	   }
	 ay_object_defaults(curve);
	 curve->type = AY_IDNCURVE;
	 curve->refine = nc;
       }
     else
       {
	 return ay_status;
       }
   }

  ccv = nc->controlv;

  closed = ay_nct_isclosed(nc);

  mode = revolve->display_mode;
  tolerance = revolve->glu_sampling_tolerance;

  /* get curves transformation-matrix */
  ay_trafo_creatematrix(curve, m);

  ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
		     nc->knotv, nc->controlv,
		     nc->knotv[nc->order - 1], P1);

  /* apply transform */
  AY_APTRAN3(PS,P1,m);

  ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
		     nc->knotv, nc->controlv,
		     nc->knotv[nc->length], P1);

  /* apply transform */
  AY_APTRAN3(PE,P1,m);

  /* sanity check; cannot create caps for flat revolutions... */
  if(!closed && (fabs(PE[1]-PS[1]) < AY_EPSILON))
    { ay_status = AY_ERROR; goto cleanup; }

  /* if the curve points downwards, we need to revert the trim */
  if(PS[1] > PE[1])
    {
      revert = AY_TRUE;
    }

  /* create NURBS patch */
  if(!(cap = calloc(1, sizeof(ay_object))))
    { ay_status = AY_EOMEM; goto cleanup; }
  ay_object_defaults(cap);
  cap->type = AY_IDNPATCH;
  cap->hide_children = AY_TRUE;
  cap->parent = AY_TRUE;
  cap->inherit_trafos = AY_FALSE;

  if(!(controlv = calloc(4*4, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

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
			    (ay_nurbpatch_object **)(void*)&(cap->refine));
  if(ay_status)
    { goto cleanup; }

  ((ay_nurbpatch_object *)cap->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)cap->refine)->display_mode = mode;

  /* prevent cleanup code from doing something harmful */
  controlv = NULL;

  if(th != 0.0)
    {
      ay_quat_axistoquat(yaxis, AY_D2R(th), quat);
      ay_quat_add(quat, cap->quat, cap->quat);
    }

  /* create trimcurve(s) */
  ay_status = ay_object_copy(curve, &trim);
  if(ay_status)
    { goto cleanup; }

  cap->down = trim;

  tc = (ay_nurbcurve_object *)trim->refine;
  ccv = tc->controlv;
  for(i = 0; i < tc->length*4; i+=4)
    {
      memcpy(P1, &(ccv[i]), 4*sizeof(double));
      AY_APTRAN3(P2, P1, m)
      memcpy(&(ccv[i]), P2, 3*sizeof(double));
    }

  ay_trafo_defaults(trim);

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
      if(revert)
	{
	  ay_nct_revert((ay_nurbcurve_object *)(trim->refine));
	}
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
					trim->refine, 4, 1, 0, &angle);

      if(angle < 0.0)
	{
	  ay_nct_revert(trim->refine);
	}
    } /* if !closed */

  trim->next = ay_endlevel;

  if(!closed)
    {
      if(!(tloop = calloc(1, sizeof(ay_object))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      ay_object_defaults(tloop);
      tloop->type = AY_IDLEVEL;

      tloop->down = cap->down;
      cap->down = tloop;

      if(!(tloop->refine = calloc(1, sizeof(ay_level_object))))
	{ay_status = AY_EOMEM; goto cleanup;}

      /* create another trimcurve */
      if(!(trim = calloc(1, sizeof(ay_object))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      ay_object_defaults(trim);
      trim->type = AY_IDNCURVE;

      trim->next = tloop->down->next;
      tloop->down->next = trim;

      if(!(controlv = calloc(4*4, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

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
			     (ay_nurbcurve_object **)(void*)&(trim->refine));

      if(ay_status)
	{ goto cleanup; }

      ((ay_nurbcurve_object *)(trim->refine))->glu_sampling_tolerance =
	nc->glu_sampling_tolerance;
      ((ay_nurbcurve_object *)(trim->refine))->display_mode =
	nc->display_mode;
      /* prevent cleanup code from doing something harmful */
      controlv = NULL;

      if(revert)
	{
	  ay_nct_revert((ay_nurbcurve_object *)(trim->refine));
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

      /* properly terminate level */
      tloop = cap->down;
      if(tloop)
	{
	  while(tloop->next)
	    {
	      tloop = tloop->next;
	    }
	  tloop->next = ay_endlevel;
	}
    } /* if !closed */

  *o = cap;

  /* prevent cleanup code from doing something harmful */
  cap = NULL;

cleanup:

  if(cap)
    {
      (void)ay_object_deletemulti(cap);
    }

  if(controlv)
    {
      free(controlv);
    }

  if(revolve->sections != 0)
   {
     if(curve)
       (void)ay_object_delete(curve);
   }

 return ay_status;
} /* ay_revolve_crtside */


/* ay_revolve_notifycb:
 *  notification callback function of revolve object
 */
int
ay_revolve_notifycb(ay_object *o)
{
 int ay_status = AY_OK, phase = 0;
 ay_revolve_object *revolve = NULL;
 char fname[] = "revolve_notify";
 ay_object *curve = NULL, *pobject = NULL, *npatch = NULL;
 ay_nurbcurve_object *nc = NULL;
 int is_provided = AY_FALSE, mode = 0;
 double tolerance;

  if(!o)
    return AY_ENULL;

  revolve = (ay_revolve_object *)(o->refine);

  mode = revolve->display_mode;
  tolerance = revolve->glu_sampling_tolerance;

  /* remove old objects */
  if(revolve->npatch)
    (void)ay_object_delete(revolve->npatch);
  revolve->npatch = NULL;

  if(revolve->upper_cap)
    (void)ay_object_delete(revolve->upper_cap);
  revolve->upper_cap = NULL;

  if(revolve->lower_cap)
    (void)ay_object_delete(revolve->lower_cap);
  revolve->lower_cap = NULL;

  if(revolve->start_cap)
    (void)ay_object_delete(revolve->start_cap);
  revolve->start_cap = NULL;

  if(revolve->end_cap)
    (void)ay_object_delete(revolve->end_cap);
  revolve->end_cap = NULL;

  /* get curve to revolve */
  if(!o->down || !o->down->next)
    goto cleanup;

  curve = o->down;
  if(curve->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(curve, AY_IDNCURVE, &pobject);
      if(!pobject)
	{
	  ay_status = AY_ERROR;
	  goto cleanup;
	}
      else
	{
	  curve = pobject;
	  is_provided = AY_TRUE;
	} /* if */
    } /* if */

  /* revolve */
  phase = 1;
  ay_status = ay_npt_createnpatchobject(&npatch);

  if(ay_status)
    goto cleanup;

  ay_status = ay_npt_revolve(curve, revolve->thetamax, revolve->sections,
			     revolve->order,
			  (ay_nurbpatch_object **)(void*)&(npatch->refine));

  if(ay_status)
    goto cleanup;

  revolve->npatch = npatch;

  ((ay_nurbpatch_object *)npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)npatch->refine)->display_mode =
    mode;

  nc = (ay_nurbcurve_object *)curve->refine;

  /* create caps */
  phase = 2;
  if(revolve->has_upper_cap)
    {
      ay_status = ay_revolve_crtcap(revolve, curve,
				    nc->knotv[nc->length], AY_TRUE,
				    &(revolve->upper_cap));
      if(ay_status)
	goto cleanup;
    } /* if */

  if(revolve->has_lower_cap)
    {
      ay_status = ay_revolve_crtcap(revolve, curve,
				    nc->knotv[nc->order - 1], AY_FALSE,
				    &(revolve->lower_cap));
      if(ay_status)
	goto cleanup;
    } /* if */

  if(revolve->has_start_cap)
    {
      ay_status = ay_revolve_crtside(revolve, curve, 0.0,
				     &(revolve->start_cap));
      if(ay_status)
	goto cleanup;

      if(revolve->start_cap)
	revolve->start_cap->scalz *= -1.0;
    } /* if */

  if(revolve->has_end_cap)
    {
      ay_status = ay_revolve_crtside(revolve, curve, revolve->thetamax,
				    &(revolve->end_cap));
      if(ay_status)
	goto cleanup;
    } /* if */

cleanup:
  /* remove provided object(s) */
  if(is_provided)
    {
      (void)ay_object_deletemulti(pobject);
    }

  /* recover selected points */
  if(o->selp)
    {
      if(revolve->npatch)
	ay_revolve_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

  if(ay_status)
    {
      switch(phase)
	{
	case 1:
	  ay_error(AY_ERROR, fname, "Revolve failed.");
	  break;
	case 2:
	  ay_error(AY_EWARN, fname, "Cap creation failed.");
	  ay_status = AY_OK;
	  break;
	default:
	  break;
	}
    }

 return ay_status;
} /* ay_revolve_notifycb */


/* ay_revolve_providecb:
 *  provide callback function of revolve object
 */
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

      if(!r->npatch)
	return AY_ERROR;

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

	  ay_npt_applytrafo(*t);
	  ay_trafo_copy(o, *t);

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

	  ay_npt_applytrafo(*t);
	  ay_trafo_copy(o, *t);

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

	  ay_npt_applytrafo(*t);
	  ay_trafo_copy(o, *t);

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

	  ay_npt_applytrafo(*t);
	  ay_trafo_copy(o, *t);

	  t = &((*t)->next);
	  p = p->next;
	} /* while */

      /* copy eventually present TP tags */
      ay_npt_copytptag(o, new);

      *result = new;
    } /* if */

 return ay_status;
} /* ay_revolve_providecb */


/* ay_revolve_convertcb:
 *  convert callback function of revolve object
 */
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
	      /* reset display mode and sampling tolerance
		 of new patch to "global"? */
	      if(!in_place && ay_prefs.conv_reset_display)
		{
		  ay_npt_resetdisplay(*next);
		}

	      (*next)->hide_children = AY_TRUE;
	      (*next)->parent = AY_TRUE;
	      (*next)->down = ay_endlevel;
	      next = &((*next)->next);
	    }
	} /* if */

      if(r->upper_cap)
	{
	  ay_status = ay_object_copy(r->upper_cap, next);
	  if(*next)
	    {
	      /* reset display mode and sampling tolerance
		 of new patch to "global"? */
	      if(!in_place && ay_prefs.conv_reset_display)
		{
		  ay_npt_resetdisplay(*next);
		}

	      next = &((*next)->next);
	    }
	}

      if(r->lower_cap)
	{
	  ay_status = ay_object_copy(r->lower_cap, next);
	  if(*next)
	    {
	      /* reset display mode and sampling tolerance
		 of new patch to "global"? */
	      if(!in_place && ay_prefs.conv_reset_display)
		{
		  ay_npt_resetdisplay(*next);
		}

	      next = &((*next)->next);
	    }
	}

      if(r->start_cap)
	{
	  ay_status = ay_object_copy(r->start_cap, next);
	  if(*next)
	    {
	      /* reset display mode and sampling tolerance
		 of new patch to "global"? */
	      if(!in_place && ay_prefs.conv_reset_display)
		{
		  ay_npt_resetdisplay(*next);
		}

	      next = &((*next)->next);
	    }
	}

      if(r->end_cap)
	{
	  ay_status = ay_object_copy(r->end_cap, next);
	  if(*next)
	    {
	      /* reset display mode and sampling tolerance
		 of new patch to "global"? */
	      if(!in_place && ay_prefs.conv_reset_display)
		{
		  ay_npt_resetdisplay(*next);
		}

	      next = &((*next)->next);
	    }
	}

      /* copy eventually present TP tags */
      ay_npt_copytptag(o, new->down);
      *next = ay_endlevel;
    }
  else
    {
      if(r->npatch)
	{
	  ay_status = ay_object_copy(r->npatch, &new);
	  if(new)
	    {
	      /* reset display mode and sampling tolerance
		 of new patch to "global"? */
	      if(!in_place && ay_prefs.conv_reset_display)
		{
		  ay_npt_resetdisplay(new);
		}

	      ay_trafo_copy(o, new);
	      new->hide_children = AY_TRUE;
	      new->parent = AY_TRUE;
	      new->down = ay_endlevel;

	      /* copy eventually present TP tags */
	      ay_npt_copytptag(o, new);
	    }
	} /* if */
    } /* if */

  /* second, link new objects, or replace old objects with them */

  if(new)
    {
      if(!in_place)
	{
	  ay_object_link(new);
	}
      else
	{
	  ay_object_replace(new, o);
	} /* if */
    } /* if */

 return ay_status;
} /* ay_revolve_convertcb */


/* ay_revolve_init:
 *  initialize the revolve object module
 */
int
ay_revolve_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_revolve_name,
				    ay_revolve_createcb,
				    ay_revolve_deletecb,
				    ay_revolve_copycb,
				    ay_revolve_drawcb,
				    ay_revolve_drawhcb,
				    ay_revolve_shadecb,
				    ay_revolve_setpropcb,
				    ay_revolve_getpropcb,
				    ay_revolve_getpntcb,
				    ay_revolve_readcb,
				    ay_revolve_writecb,
				    ay_revolve_wribcb,
				    ay_revolve_bbccb,
				    AY_IDREVOLVE);


  ay_status += ay_notify_register(ay_revolve_notifycb, AY_IDREVOLVE);

  ay_status += ay_convert_register(ay_revolve_convertcb, AY_IDREVOLVE);

  ay_status += ay_provide_register(ay_revolve_providecb, AY_IDREVOLVE);

 return ay_status;
} /* ay_revolve_init */

