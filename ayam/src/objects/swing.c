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
    ay_object_delete(swing->npatch);

  if(swing->caps_and_bevels)
    {
      ay_object_deletemulti(swing->caps_and_bevels);
      swing->caps_and_bevels = NULL;
    }

  free(swing);

 return AY_OK;
} /* ay_swing_deletecb */


/* ay_swing_copycb:
 *  copy callback function of swing object
 */
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

  /* copy caps and bevels */
  swing->caps_and_bevels = NULL;

  if(swingsrc->caps_and_bevels)
    ay_object_copymulti(swingsrc->caps_and_bevels,
			&(swing->caps_and_bevels));

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
 int i = 0, a = 0;
 ay_swing_object *swing = NULL;
 double *pnts = NULL;
 double point_size = ay_prefs.handle_size;
 ay_nurbpatch_object *patch = NULL;

  if(!o)
    return AY_ENULL;

  swing = (ay_swing_object *) o->refine;

  if(swing->npatch)
    {
      patch = (ay_nurbpatch_object *)swing->npatch->refine;
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

  ay_notify_object(o);

  o->modified = AY_TRUE;

  ay_notify_parent();

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


/* ay_swing_readcb:
 *  read (from scene file) callback function of swing object
 */
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


/* ay_swing_writecb:
 *  write (to scene file) callback function of swing object
 */
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


/* ay_swing_crtcap:
 *  create top or bottom swing cap
 */
int
ay_swing_crtcap(ay_swing_object *swing, int upper,
		ay_object **o)
{
 int ay_status = AY_OK;
 int mode = 0, i = 0;
 double P1[4] = {0}, *cv = NULL, *controlv = NULL;
 double tolerance, minx, maxx, minz, maxz, y;
 ay_object *curve = NULL, *cap = NULL, *trim = NULL, *tloop = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!swing || !o)
    return AY_ENULL;

  mode = swing->display_mode;
  tolerance = swing->glu_sampling_tolerance;

  if(!(curve = calloc(1, sizeof(ay_object))))
    return AY_EOMEM;

  ay_status = ay_npt_extractnc(swing->npatch, upper?0:1, 0.0, AY_FALSE,
			       AY_TRUE, AY_FALSE, NULL, &nc);

  if(ay_status || !nc)
    { ay_status = AY_ERROR; goto cleanup; }

  ay_object_defaults(curve);
  curve->type = AY_IDNCURVE;
  curve->refine = nc;

  /* calc minx, maxx, minz, maxz from curve */
  cv = nc->controlv;
  minx = minz = DBL_MAX;
  maxx = maxz = DBL_MIN;
  for(i = 0; i < nc->length*4; i += 4)
    {
      if(cv[i] < minx)
	minx = cv[i];
      if(cv[i] > maxx)
	maxx = cv[i];
      if(cv[i+2] < minz)
	minz = cv[i+2];
      if(cv[i+2] > maxz)
	maxz = cv[i+2];
    }
  y = cv[1];

  /* create a NURBS patch object */
  if(!(cap = calloc(1, sizeof(ay_object))))
    { ay_status = AY_EOMEM; goto cleanup; }
  ay_object_defaults(cap);
  cap->type = AY_IDNPATCH;
  cap->hide_children = AY_TRUE;
  cap->parent = AY_TRUE;
  cap->inherit_trafos = AY_FALSE;

  if(!(controlv = calloc(4*4, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  controlv[0] = minx;
  controlv[1] = y;
  controlv[2] = minz;
  controlv[3] = 1.0;

  controlv[4] = minx;
  controlv[5] = y;
  controlv[6] = maxz;
  controlv[7] = 1.0;

  controlv[8] = maxx;
  controlv[9] = y;
  controlv[10] = minz;
  controlv[11] = 1.0;

  controlv[12] = maxx;
  controlv[13] = y;
  controlv[14] = maxz;
  controlv[15] = 1.0;

  ay_status = ay_npt_create(2, 2, 2, 2,
			    AY_KTBEZIER, AY_KTBEZIER,
			    controlv, NULL, NULL,
			    (ay_nurbpatch_object **)(void*)&(cap->refine));

  ((ay_nurbpatch_object *)cap->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)cap->refine)->display_mode = mode;

  /* rescale curve and rotate it to the XY plane
     to serve as proper trim (loop component) */
  for(i = 0; i < nc->length*4; i += 4)
    {
      cv[i] -= minx;
      cv[i] /= (maxx-minx);
      cv[i+1] = cv[i+2] - minz;
      cv[i+1] /= (maxz-minz);
      cv[i+2] = 0.0;
    }

  curve->next = ay_endlevel;
  cap->down = curve;

  if(ay_nct_isclosed(nc) == AY_FALSE)
    {
      /* create trim-loop level */
      if(!(tloop = calloc(1, sizeof(ay_object))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      ay_object_defaults(tloop);
      tloop->type = AY_IDLEVEL;

      if(!(tloop->refine = calloc(1, sizeof(ay_level_object))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      tloop->down = cap->down;
      cap->down = tloop;

      /* create another trimcurve to close the loop */
      if(!(trim = calloc(1, sizeof(ay_object))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      ay_object_defaults(trim);
      trim->type = AY_IDNCURVE;

      if(!(controlv = calloc(3*4, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      /* P1 == trajectory start */
      ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
			 nc->knotv, nc->controlv,
			 nc->knotv[nc->length], P1);
      memcpy(controlv, P1, 3*sizeof(double));
      controlv[3] = 1.0;

      /* P2 == origin */
      controlv[4] = (-minx)/(maxx-minx);
      controlv[5] = (-minz)/(maxz-minz);
      controlv[7] = 1.0;

      /* P3 == trajectory end */
      ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
			 nc->knotv, nc->controlv,
			 nc->knotv[nc->order-1], P1);
      memcpy(&(controlv[8]), P1, 3*sizeof(double));
      controlv[11] = 1.0;

      ay_status = ay_nct_create(2, 3, AY_KTNURB,
				controlv, NULL,
			      (ay_nurbcurve_object **)(void*)&(trim->refine));

      if(trim->refine)
	{
	  ((ay_nurbcurve_object *)(trim->refine))->glu_sampling_tolerance =
	    nc->glu_sampling_tolerance;
	  ((ay_nurbcurve_object *)(trim->refine))->display_mode =
	    nc->display_mode;
	}

      /* fix orientation of trims */
      ay_nct_revert((ay_nurbcurve_object *)trim->refine);

      trim->next = tloop->down->next;
      tloop->down->next = trim;

      trim = tloop->down;
      ay_nct_revert((ay_nurbcurve_object *)trim->refine);


      /* properly terminate trim-loop level */
      tloop = cap->down;
      if(tloop)
	{
	  while(tloop->next)
	    {
	      tloop = tloop->next;
	    }
	  tloop->next = ay_endlevel;
	}
      else
	{
	  trim->down = ay_endlevel;
	}

    } /* if */

  /* return result */
  *o = cap;

  /* prevent cleanup code from doing something harmful */
  cap = NULL;
  curve = NULL;
  trim = NULL;
  tloop = NULL;

 cleanup:

  if(cap)
    {
      cap->down = NULL;
      ay_object_delete(cap);
    }

  if(curve)
    {
      ay_object_delete(curve);
    }

  if(tloop)
    {
      tloop->down = NULL;
      ay_object_deletemulti(tloop);
    }

  if(trim)
    {
      ay_object_delete(trim);
    }

 return ay_status;
} /* ay_swing_crtcap */


/* ay_swing_crtside:
 *  create start or end swing cap
 */
int
ay_swing_crtside(ay_swing_object *swing, ay_object *cso, ay_object *tro,
		 int start, ay_object **o)
{
 int ay_status = AY_OK;
 int stride = 4, mode = 0, i = 0;
 double P1[4] = {0}, P2[4] = {0}, *cv = NULL, *controlv = NULL;
 double tolerance, minx, maxx, minz, maxz, miny, maxy;
 ay_object *curve = NULL, *cap = NULL, *trim = NULL, *tloop = NULL;
 ay_nurbcurve_object *cs = NULL, *tr = NULL, *nc = NULL;
 double *p = NULL, angle, scale, m[16];

  if(!swing || !cso || !tro || !o)
    return AY_ENULL;

  if((cso->type != AY_IDNCURVE) || (tro->type != AY_IDNCURVE))
    return AY_ERROR;

  cs = (ay_nurbcurve_object *)(cso->refine);
  tr = (ay_nurbcurve_object *)(tro->refine);

  mode = swing->display_mode;
  tolerance = swing->glu_sampling_tolerance;

  if(ay_nct_isclosed(cs) == AY_TRUE)
    {
      if(!(curve = calloc(1, sizeof(ay_object))))
	return AY_EOMEM;

      ay_status = ay_npt_extractnc(swing->npatch, start?2:3, 0.0, AY_FALSE,
				   AY_TRUE, AY_FALSE, NULL,
				   &nc);

      if(!nc)
	{ ay_status = AY_ERROR; goto cleanup; }

      ay_object_defaults(curve);
      curve->type = AY_IDNCURVE;
      curve->refine = nc;

      ay_status = ay_capt_createfromcurve(curve, o);
      return ay_status;
    }

  /* calculate angle and scale factor */
  if(start)
    p = tr->controlv;
  else
    p = &(tr->controlv[(tr->length-1)*stride]);

  if(fabs(p[2]) > AY_EPSILON)
    {
      if(p[2] > 0.0)
	{
	  angle = atan(p[0]/p[2]);
	}
      else
	{
	  if(fabs(p[0]) < AY_EPSILON)
	    angle = AY_D2R(180.0);
	  else
	    angle = AY_D2R(180.0)+atan(p[0]/p[2]);
	}
    }
  else
    {
      if(p[0] > 0.0)
	angle = AY_D2R(90.0);
      else
	angle = AY_D2R(270.0);
    } /* if */

  /* calculate scale factor */
  scale = AY_V3LEN(p);

  /* copy cross-section to serve as trim curve */
  ay_object_copy(cso, &curve);
  if(!curve)
    return AY_ERROR;

  ay_nct_applytrafo(curve);

  nc = (ay_nurbcurve_object *)curve->refine;

  /* calc minx, maxx, minz, maxz from cross section */
  cv = nc->controlv;
  minx = miny = minz = DBL_MAX;
  maxx = maxy = maxz = DBL_MIN;
  for(i = 0; i < nc->length*4; i += 4)
    {
      if(cv[i] < minx)
	minx = cv[i];
      if(cv[i] > maxx)
	maxx = cv[i];
      if(cv[i+1] < miny)
	miny = cv[i+1];
      if(cv[i+1] > maxy)
	maxy = cv[i+1];
      if(cv[i+2] < minz)
	minz = cv[i+2];
      if(cv[i+2] > maxz)
	maxz = cv[i+2];
    }

  /* create a NURBS patch object */
  if(!(cap = calloc(1, sizeof(ay_object))))
    { ay_status = AY_EOMEM; goto cleanup; }
  ay_object_defaults(cap);
  cap->type = AY_IDNPATCH;
  cap->hide_children = AY_TRUE;
  cap->parent = AY_TRUE;
  cap->inherit_trafos = AY_FALSE;

  if(!(controlv = calloc(4*4, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* place patch in YZ plane */
  controlv[0] = 0.0;
  controlv[1] = miny;
  controlv[2] = 0.0;
  controlv[3] = 1.0;

  controlv[4] = 0.0;
  controlv[5] = maxy;
  controlv[6] = 0.0;
  controlv[7] = 1.0;

  controlv[8] = 0.0;
  controlv[9] = miny;
  controlv[10] = maxz;
  controlv[11] = 1.0;

  controlv[12] = 0.0;
  controlv[13] = maxy;
  controlv[14] = maxz;
  controlv[15] = 1.0;

  ay_status = ay_npt_create(2, 2, 2, 2,
			    AY_KTBEZIER, AY_KTBEZIER,
			    controlv, NULL, NULL,
			    (ay_nurbpatch_object **)(void*)&(cap->refine));

  if(ay_status)
    {
      goto cleanup;
    }

  controlv = NULL;

  ((ay_nurbpatch_object *)cap->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)cap->refine)->display_mode = mode;

  /* rotate and scale patch */

  /* set up transformation matrix */
  ay_trafo_identitymatrix(m);
  ay_trafo_scalematrix(scale, 1.0, scale, m);
  ay_trafo_rotatematrix(AY_R2D(angle), 0.0, 1.0, 0.0, m);
  /* apply transformation matrix */
  for(i = 0; i < 4; i++)
    {
      ay_trafo_apply3(&(controlv[i*4]), m);
    } /* for */

  /* rescale cross section curve and rotate it to the XY plane
     to serve as proper trim (loop component) */
  for(i = 0; i < nc->length*4; i += 4)
    {
      cv[i] = cv[i+2];
      cv[i] /= maxz;
      cv[i+1] -= miny;
      cv[i+1] /= (maxy-miny);
      cv[i+2] = 0.0;
    }

  curve->next = ay_endlevel;
  cap->down = curve;

  /* create another trimcurve to close the loop */
  if(!(trim = calloc(1, sizeof(ay_object))))
    { ay_status = AY_EOMEM; goto cleanup; }
  ay_object_defaults(trim);
  trim->type = AY_IDNCURVE;

  if(!(controlv = calloc(4*4, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* get cross-section end points */
  ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
		     nc->knotv, nc->controlv,
		     nc->knotv[nc->order-1], P1);

  ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
		     nc->knotv, nc->controlv,
		     nc->knotv[nc->length], P2);

  /* sanity check; cannot create caps for flat swings... */
  if(fabs(P2[1]-P1[1]) < AY_EPSILON)
    { ay_status = AY_ERROR; goto cleanup; }

  /* P1 == (lower) cross section start */
  if(P1[1] < P2[1])
    memcpy(controlv, P1, 3*sizeof(double));
  else
    memcpy(controlv, P2, 3*sizeof(double));
  controlv[3] = 1.0;

  /* P2 == origin */
  controlv[4] = 0.0;
  controlv[5] = miny;
  controlv[6] = 0.0;
  controlv[7] = 1.0;

  /* P3 == origin */
  controlv[8] = 0.0;
  controlv[9] = maxy;
  controlv[10] = 0.0;
  controlv[11] = 1.0;

  /* P4 == (upper) cross section end */
  if(P1[1] < P2[1])
    memcpy(&(controlv[12]), P2, 3*sizeof(double));
  else
    memcpy(&(controlv[12]), P1, 3*sizeof(double));
  controlv[15] = 1.0;

  ay_status = ay_nct_create(2, 4, AY_KTNURB,
			    controlv, NULL,
			    (ay_nurbcurve_object **)(void*)&(trim->refine));
  if(ay_status)
    goto cleanup;

  ((ay_nurbcurve_object *)(trim->refine))->glu_sampling_tolerance =
    cs->glu_sampling_tolerance;
  ((ay_nurbcurve_object *)(trim->refine))->display_mode =
    cs->display_mode;

  controlv = NULL;

  /* fix orientation of trim */
  ay_nct_revert((ay_nurbcurve_object *)trim->refine);

  /* create trim-loop level */
  if(!(tloop = calloc(1, sizeof(ay_object))))
    { ay_status = AY_EOMEM; goto cleanup; }
  ay_object_defaults(tloop);
  tloop->type = AY_IDLEVEL;

  if(!(tloop->refine = calloc(1, sizeof(ay_level_object))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* link level */
  tloop->down = cap->down;
  cap->down = tloop;

  trim->next = tloop->down->next;
  tloop->down->next = trim;

  trim = tloop->down;

  /* fix orientation of trim */
  if(P1[1] > P2[1])
    {
      ay_nct_revert((ay_nurbcurve_object *)trim->refine);
    }

  /* properly terminate trim-loop level */
  tloop = cap->down;
  if(tloop)
    {
      while(tloop->next)
	{
	  tloop = tloop->next;
	}
      tloop->next = ay_endlevel;
    }
  else
    {
      trim->down = ay_endlevel;
    }

  /* return result */
  *o = cap;

  /* prevent cleanup code from doing something harmful */
  cap = NULL;
  curve = NULL;
  trim = NULL;
  tloop = NULL;

cleanup:

  if(controlv)
    free(controlv);

  if(cap)
    {
      cap->down = NULL;
      ay_object_delete(cap);
    }

  if(curve)
    {
      ay_object_delete(curve);
    }

  if(tloop)
    {
      tloop->down = NULL;
      ay_object_deletemulti(tloop);
    }

  if(trim)
    {
      ay_object_delete(trim);
    }

 return ay_status;
} /* ay_swing_crtside */


/* ay_swing_notifycb:
 *  notification callback function of swing object
 */
int
ay_swing_notifycb(ay_object *o)
{
 int ay_status = AY_OK, phase = 0;
 char fname[] = "swing_notify";
 ay_swing_object *swing = NULL;
 ay_object *cs = NULL, *tr = NULL, *pobject = NULL, *npatch = NULL;
 ay_object **nextcb;
 int provided_cs = AY_FALSE, provided_tr = AY_FALSE, mode = 0;
 double tolerance;

  if(!o)
    return AY_ENULL;

  swing = (ay_swing_object *)(o->refine);

  mode = swing->display_mode;
  tolerance = swing->glu_sampling_tolerance;

  nextcb = &(swing->caps_and_bevels);

  /* remove old objects */
  if(swing->npatch)
    ay_object_delete(swing->npatch);
  swing->npatch = NULL;
 
  if(swing->caps_and_bevels)
    {
      ay_object_deletemulti(swing->caps_and_bevels);
      swing->caps_and_bevels = NULL;
    }

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
  phase = 1;
  if(!(npatch = calloc(1, sizeof(ay_object))))
    {
      ay_status =  AY_EOMEM;
      goto cleanup;
    }

  ay_object_defaults(npatch);
  npatch->type = AY_IDNPATCH;

  ay_status = ay_npt_swing(cs, tr,
			  (ay_nurbpatch_object **)(void*)&(npatch->refine));

  if(ay_status)
    return ay_status;

  swing->npatch = npatch;

  ((ay_nurbpatch_object *)npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)npatch->refine)->display_mode =
    mode;

  /* create caps */
  phase = 2;
  if(swing->has_upper_cap)
    {
      ay_status = ay_swing_crtcap(swing, AY_TRUE, nextcb);
      if(ay_status)
	goto cleanup;
      nextcb = &((*nextcb)->next);
    } /* if */

  if(swing->has_lower_cap)
    {
      ay_status = ay_swing_crtcap(swing, AY_FALSE, nextcb);
      if(ay_status)
	goto cleanup;
      nextcb = &((*nextcb)->next);
    } /* if */

  if(swing->has_start_cap)
    {
      ay_status = ay_swing_crtside(swing, cs, tr, AY_TRUE, nextcb);
      if(ay_status)
	goto cleanup;
      /*
      if(swing->start_cap)
	swing->start_cap->scalz *= -1.0;
      */
      nextcb = &((*nextcb)->next);
    } /* if */

  if(swing->has_end_cap)
    {
      ay_status = ay_swing_crtside(swing, cs, tr, AY_FALSE, nextcb);
      if(ay_status)
	goto cleanup;
    } /* if */

cleanup:
  /* remove provided object(s) */
  if(provided_cs && cs)
    {
      ay_object_deletemulti(cs);
    }
  if(provided_tr && tr)
    {
      ay_object_deletemulti(tr);
    }

  /* recover selected points */
  if(o->selp)
    {
      ay_swing_getpntcb(3, o, NULL, NULL);
    }

  if(ay_status)
    {
      switch(phase)
	{
	case 1:
	  ay_error(AY_ERROR, fname, "Swing failed.");
	  break;
	case 2:
	  ay_error(AY_EWARN, fname, "Cap creation failed.");
	  ay_status = AY_OK;
	  break;
	default:
	  break;
	}
    }

 return AY_OK;
} /* ay_swing_notifycb */


/* ay_swing_providecb:
 *  provide callback function of swing object
 */
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

      if(!r->npatch)
	return AY_ERROR;

      /* copy swung surface */
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

      /* copy caps and bevels */
      p = r->caps_and_bevels;
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
} /* ay_swing_providecb */


/* ay_swing_convertcb:
 *  convert callback function of swing object
 */
int
ay_swing_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_swing_object *r = NULL;
 ay_object *new = NULL, **next = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */

  r = (ay_swing_object *) o->refine;

  if(r->caps_and_bevels)
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

	  b = r->caps_and_bevels;
	  while(b)
	    {
	      ay_status = ay_object_copy(b, next);
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
	      b = b->next;
	    } /* while */
	} /* if */

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
	    } /* if */
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


  ay_status = ay_notify_register(ay_swing_notifycb, AY_IDSWING);

  ay_status = ay_convert_register(ay_swing_convertcb, AY_IDSWING);

  ay_status = ay_provide_register(ay_swing_providecb, AY_IDSWING);

 return ay_status;
} /* ay_swing_init */

