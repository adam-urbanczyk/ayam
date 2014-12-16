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

/* cone.c - cone object */

static char *ay_cone_name = "Cone";

int ay_cone_notifycb(ay_object *o);

/* number of read only points */
#define AY_PCONE 11


/* functions: */

/* ay_cone_createcb:
 *  create callback function of cone object
 */
int
ay_cone_createcb(int argc, char *argv[], ay_object *o)
{
 ay_cone_object *cone;
 char fname[] = "crtcone";

  if(!o)
    return AY_ENULL;

  if(!(cone = calloc(1, sizeof(ay_cone_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  cone->closed = AY_TRUE;
  cone->is_simple = AY_TRUE;
  cone->radius = 1.0;
  cone->height = 1.0;
  cone->thetamax = 360.0;

  o->refine = cone;

 return AY_OK;
} /* ay_cone_createcb */


/* ay_cone_deletecb:
 *  delete callback function of cone object
 */
int
ay_cone_deletecb(void *c)
{
 ay_cone_object *cone;

  if(!c)
    return AY_ENULL;

  cone = (ay_cone_object *)(c);

  if(cone->pnts)
    free(cone->pnts);

  free(cone);

 return AY_OK;
} /* ay_cone_deletecb */


/* ay_cone_copycb:
 *  copy callback function of cone object
 */
int
ay_cone_copycb(void *src, void **dst)
{
 ay_cone_object *cone;

  if(!src || !dst)
    return AY_ENULL;

  if(!(cone = malloc(sizeof(ay_cone_object))))
    return AY_EOMEM;

  memcpy(cone, src, sizeof(ay_cone_object));

  cone->pnts = NULL;

  *dst = (void *)cone;

 return AY_OK;
} /* ay_cone_copycb */


/* ay_cone_drawcb:
 *  draw (display in an Ayam view window) callback function of cone object
 */
int
ay_cone_drawcb(struct Togl *togl, ay_object *o)
{
 ay_cone_object *cone;
 double thetamax, height, radius;
 double thetadiff, angle;
 int i;
 double P1[9*2] = {0};
 GLdouble w = 0.0;

  if(!o)
    return AY_ENULL;

  cone = (ay_cone_object *)o->refine;

  if(!cone)
    return AY_ENULL;

  radius = (GLdouble)(cone->radius);
  height = (GLdouble)(cone->height);

  /* check, if cone is simple */
  if(cone->is_simple)
    {
      /* yes, it is */

      w = (GLdouble)(sqrt(2.0)*0.5);
      /* xy-plane-ring */
      glBegin(GL_LINE_LOOP);
      glVertex3d(radius,0.0,0.0);
      glVertex3d(radius*w,-radius*w,0.0);
      glVertex3d(0.0,-radius,0.0);
      glVertex3d(-radius*w,-radius*w,0.0);
      glVertex3d(-radius,0.0,0.0);
      glVertex3d(-radius*w,radius*w,0.0);
      glVertex3d(0.0,radius,0.0);
      glVertex3d(radius*w,radius*w,0.0);
      glEnd();
      /* cone */
      glBegin(GL_LINES);
      glVertex3d(-radius,0.0,0.0);
      glVertex3d(0.0,0.0,height);
      glVertex3d(radius,0.0,0.0);
      glVertex3d(0.0,0.0,height);
      glVertex3d(0.0,-radius,0.0);
      glVertex3d(0.0,0.0,height);
      glVertex3d(0.0,radius,0.0);
      glVertex3d(0.0,0.0,height);
      glEnd();

      if(cone->closed)
	{
	  glBegin(GL_LINES);
	   glVertex3d(radius, 0.0, 0.0);
	   glVertex3d(-radius, 0.0, 0.0);
	   glVertex3d(0.0, radius, 0.0);
	   glVertex3d(0.0, -radius, 0.0);
	  glEnd();
	}

      return AY_OK;
    }

  thetamax = cone->thetamax;
  thetadiff = AY_D2R(thetamax/8);

  angle = 0.0;
  for(i = 0; i <= 8; i++)
    {
      P1[(i*2)] = cos(angle) * radius;
      P1[(i*2)+1] = sin(angle) * radius;
      angle += thetadiff;
    }

  /* draw */
  glBegin(GL_LINE_STRIP);
   for(i = 0; i <= 8; i++)
     {
       glVertex3d(P1[i*2], P1[i*2+1], 0.0);
     }
  glEnd();

  glBegin(GL_LINES);
   for(i = 0; i <= 8; i++)
     {
       glVertex3d(P1[i*2], P1[i*2+1], 0.0);
       glVertex3d(0.0, 0.0, height);
     }
  glEnd();

  if(cone->closed)
    {
      glBegin(GL_LINE_STRIP);
       glVertex3d(radius, 0.0, 0.0);
       glVertex3d(0.0,    0.0, 0.0);
       glVertex3d(0.0,    0.0, height);
      glEnd();
      glRotated(thetamax, 0.0, 0.0, 1.0);
      glBegin(GL_LINES);
       glVertex3d(radius, 0.0, 0.0);
       glVertex3d(0.0,    0.0, 0.0);
      glEnd();
    }

 return AY_OK;
} /* ay_cone_drawcb */


/* ay_cone_shadecb:
 *  shade (display in an Ayam view window) callback function of cone object
 */
int
ay_cone_shadecb(struct Togl *togl, ay_object *o)
{
 ay_cone_object *cone;
 double thetamax, height, radius;
 double thetadiff, angle;
 int i;
 double P1[9*2];

  if(!o)
    return AY_ENULL;

  cone = (ay_cone_object *)o->refine;

  if(!cone)
    return AY_ENULL;

  radius = cone->radius;
  height = cone->height;

  /* check, if cone is simple */
  if(cone->is_simple)
    {
      /* yes, it is */

      /* draw */
      gluCylinder(ay_gluquadobj, radius, 0.0, height, 8, 1);

      if(cone->closed)
	{
	  gluQuadricOrientation(ay_gluquadobj, GLU_INSIDE);
	  gluDisk(ay_gluquadobj, 0.0, radius, 8, 1);
	  gluQuadricOrientation(ay_gluquadobj, GLU_OUTSIDE);
	}

      return AY_OK;
    }

  thetamax = cone->thetamax;

  thetadiff = AY_D2R(thetamax/8);
  angle = 0.0;
  for(i = 0; i <= 8; i++)
    {
      P1[(i*2)] = cos(angle) * radius;
      P1[(i*2)+1] = sin(angle) * radius;
      angle += thetadiff;
    }

  /* draw */
  glBegin(GL_TRIANGLE_FAN);
  glVertex3d(0.0, 0.0, height);
  for(i = 0; i <= 8; i++)
    {
      glNormal3d(P1[i*2], P1[i*2+1], height);
      glVertex3d(P1[i*2], P1[i*2+1], 0.0);
    }
  glEnd();

  if(cone->closed)
    {
      glPushMatrix();

       glNormal3d(0.0, 1.0, 0.0);
       glBegin(GL_TRIANGLES);
        glVertex3d(0.0,    0.0, 0.0);
	glVertex3d(0.0,    0.0, height);
	glVertex3d(radius, 0.0, 0.0);
       glEnd();

       glNormal3d(0.0, -1.0, 0.0);
       glRotated(thetamax, 0.0, 0.0, 1.0);

       glBegin(GL_TRIANGLES);
        glVertex3d(0.0,    0.0, 0.0);
	glVertex3d(radius, 0.0, 0.0);
	glVertex3d(0.0,    0.0, height);
       glEnd();

      glPopMatrix();

      /* draw cap */
      glPushMatrix();

       gluQuadricOrientation(ay_gluquadobj, GLU_INSIDE);

       glRotated(thetamax-90.0, 0.0, 0.0, 1.0);
       gluPartialDisk(ay_gluquadobj, 0.0, radius, 8, 1, 0.0, thetamax);

       gluQuadricOrientation(ay_gluquadobj, GLU_OUTSIDE);

      glPopMatrix();

    }

 return AY_OK;
} /* ay_cone_shadecb */


/* ay_cone_drawhcb:
 *  draw handles (in an Ayam view window) callback function of cone object
 */
int
ay_cone_drawhcb(struct Togl *togl, ay_object *o)
{
 int i = 0, a = 0;
 ay_cone_object *cone;
 double *pnts = NULL;

  if(!o)
    return AY_ENULL;

  cone = (ay_cone_object *) o->refine;

  if(!cone)
    return AY_ENULL;

  if(!cone->pnts)
    {
      if(!(pnts = calloc(AY_PCONE*3, sizeof(double))))
	{
	  return AY_EOMEM;
	}
      cone->pnts = pnts;
      (void)ay_cone_notifycb(o);
    }
  else
    {
      pnts = cone->pnts;
    }

  glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
	    (GLfloat)ay_prefs.obb);

  glBegin(GL_POINTS);
   for(i = 0; i < AY_PCONE; i++)
     {
       glVertex3dv((GLdouble *)&pnts[a]);
       a += 3;
     }
  glEnd();

  glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
	    (GLfloat)ay_prefs.seb);

 return AY_OK;
} /* ay_cone_drawhcb */


/* ay_cone_getpntcb:
 *  get point (editing and selection) callback function of cone object
 */
int
ay_cone_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_cone_object *cone;

  if(!o)
    return AY_ENULL;

  cone = (ay_cone_object *)o->refine;

  if(!cone)
    return AY_ENULL;

  if(!cone->pnts)
    {
      if(!(cone->pnts = calloc(AY_PCONE*3, sizeof(double))))
	return AY_EOMEM;
      (void)ay_cone_notifycb(o);
    }

 return ay_selp_getpnts(mode, o, p, pe, 1, AY_PCONE, 3, cone->pnts);
} /* ay_cone_getpntcb */


/* ay_cone_setpropcb:
 *  set property (from Tcl to C context) callback function of cone object
 */
int
ay_cone_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "ConeAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_cone_object *cone;
 int itemp = 0;

  if(!interp || !o)
    return AY_ENULL;

  cone = (ay_cone_object *)o->refine;

  if(!cone)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &itemp);
  cone->closed = (char)itemp;

  Tcl_SetStringObj(ton,"Radius",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &cone->radius);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &cone->height);

  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &cone->thetamax);

  if(fabs(cone->thetamax) == 360.0)
    {
      cone->is_simple = AY_TRUE;
    }
  else
    {
      cone->is_simple = AY_FALSE;
    }

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  (void)ay_cone_notifycb(o);
  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_cone_setpropcb */


/* ay_cone_getpropcb:
 *  get property (from C to Tcl context) callback function of cone object
 */
int
ay_cone_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="ConeAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_cone_object *cone;

  if(!interp || !o)
    return AY_ENULL;

  cone = (ay_cone_object *)(o->refine);

  if(!cone)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_NewIntObj(cone->closed);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Radius",-1);
  to = Tcl_NewDoubleObj(cone->radius);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_NewDoubleObj(cone->height);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_NewDoubleObj(cone->thetamax);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_cone_getpropcb */


/* ay_cone_readcb:
 *  read (from scene file) callback function of cone object
 */
int
ay_cone_readcb(FILE *fileptr, ay_object *o)
{
 ay_cone_object *cone;
 int itemp = 0;

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(cone = calloc(1, sizeof(ay_cone_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&itemp);
  cone->closed = (char)itemp;
  fscanf(fileptr,"%lg\n",&cone->radius);
  fscanf(fileptr,"%lg\n",&cone->height);
  fscanf(fileptr,"%lg\n",&cone->thetamax);

  if(fabs(cone->thetamax) == 360.0)
    {
      cone->is_simple = AY_TRUE;
    }
  else
    {
      cone->is_simple = AY_FALSE;
    }

  o->refine = cone;

 return AY_OK;
} /* ay_cone_readcb */


/* ay_cone_writecb:
 *  write (to scene file) callback function of cone object
 */
int
ay_cone_writecb(FILE *fileptr, ay_object *o)
{
 ay_cone_object *cone;

  if(!fileptr || !o)
    return AY_ENULL;

  cone = (ay_cone_object *)(o->refine);

  if(!cone)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", (int)cone->closed);
  fprintf(fileptr, "%g\n", cone->radius);
  fprintf(fileptr, "%g\n", cone->height);
  fprintf(fileptr, "%g\n", cone->thetamax);

 return AY_OK;
} /* ay_cone_writecb */


/* ay_cone_wribcb:
 *  RIB export callback function of cone object
 */
int
ay_cone_wribcb(char *file, ay_object *o)
{
 ay_cone_object *cone;
 RtPoint rect[4];

  if(!o)
   return AY_ENULL;

  cone = (ay_cone_object*)o->refine;

  if(!cone)
    return AY_ENULL;

  if(!cone->closed)
    {
      RiCone((RtFloat)cone->height,
	     (RtFloat)cone->radius,
	     (RtFloat)cone->thetamax,
	     RI_NULL);
    }
  else
    {
    if(fabs(cone->thetamax) == 180.0)
      {

	RiCone((RtFloat)cone->height,
	       (RtFloat)cone->radius,
	       (RtFloat)cone->thetamax,
	       RI_NULL);

	rect[0][0] = (RtFloat)(cone->radius);
	rect[0][1] = (RtFloat)0.0;
	rect[0][2] = (RtFloat)0.0;

	rect[1][0] = (RtFloat)(cone->radius);
	rect[1][1] = (RtFloat)0.0;
	rect[1][2] = (RtFloat)0.0;

	rect[2][0] = (RtFloat)0.0;
	rect[2][1] = (RtFloat)0.0;
	rect[2][2] = (RtFloat)cone->height;

	rect[3][0] = (RtFloat)0.0;
	rect[3][1] = (RtFloat)0.0;
	rect[3][2] = (RtFloat)cone->height;

	RiPatch(RI_BILINEAR, RI_P, (RtPointer)rect,RI_NULL);

	RiAttributeBegin();
	 RiReverseOrientation();
	 RiDisk((RtFloat)0.0, (RtFloat)cone->radius,
		(RtFloat)cone->thetamax, RI_NULL);
	RiAttributeEnd();
      }
    else if(fabs(cone->thetamax) == 360.0)
      {
	 RiCone((RtFloat)cone->height, (RtFloat)cone->radius,
		(RtFloat)360.0, RI_NULL);
	 RiAttributeBegin();
	  RiReverseOrientation();
	  RiDisk((RtFloat)0.0, (RtFloat)cone->radius, (RtFloat)360.0, RI_NULL);
	 RiAttributeEnd();
      }
    else
      {
	RiCone((RtFloat)cone->height,
	       (RtFloat)cone->radius,
	       (RtFloat)cone->thetamax,
	       RI_NULL);

	rect[0][0] = (RtFloat)cone->radius;
	rect[0][1] = (RtFloat)0.0;
	rect[0][2] = (RtFloat)0.0;

	rect[1][0] = (RtFloat)0.0;
	rect[1][1] = (RtFloat)0.0;
	rect[1][2] = (RtFloat)0.0;

	rect[2][0] = (RtFloat)0.0;
	rect[2][1] = (RtFloat)0.0;
	rect[2][2] = (RtFloat)cone->height;

	rect[3][0] = (RtFloat)0.0;
	rect[3][1] = (RtFloat)0.0;
	rect[3][2] = (RtFloat)cone->height;

	RiPatch(RI_BILINEAR, RI_P, (RtPointer)rect, RI_NULL);

	RiTransformBegin();
	 RiRotate((RtFloat)cone->thetamax, (RtFloat)0.0, (RtFloat)0.0,
		  (RtFloat)1.0);
	 RiAttributeBegin();
	  RiReverseOrientation();
	  RiPatch(RI_BILINEAR, RI_P, (RtPointer)rect, RI_NULL);
	 RiAttributeEnd();
	 RiTransformEnd();

	 RiAttributeBegin();
	  RiReverseOrientation();
	  RiDisk((RtFloat)0.0, (RtFloat)cone->radius,
		 (RtFloat)cone->thetamax, RI_NULL);
	 RiAttributeEnd();
      }
    }

 return AY_OK;
} /* ay_cone_wribcb */


/* ay_cone_bbccb:
 *  bounding box calculation callback function of cone object
 */
int
ay_cone_bbccb(ay_object *o, double *bbox, int *flags)
{
 double r = 0.0, h = 0.0;
 ay_cone_object *cone;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  cone = (ay_cone_object *)o->refine;

  if(!cone)
    return AY_ENULL;

  if(!cone->is_simple)
    {
      if(!cone->pnts)
	{
	  if(!(cone->pnts = calloc(AY_PCONE*3, sizeof(double))))
	    { return AY_EOMEM; }
	  (void)ay_cone_notifycb(o);
	}

      return ay_bbc_fromarr(cone->pnts, AY_PCONE, 3, bbox);
    }

  r = cone->radius;
  h = cone->height;

  /* P1 */
  bbox[0] = -r; bbox[1] = r; bbox[2] = h;
  /* P2 */
  bbox[3] = -r; bbox[4] = -r; bbox[5] = h;
  /* P3 */
  bbox[6] = r; bbox[7] = -r; bbox[8] = h;
  /* P4 */
  bbox[9] = r; bbox[10] = r; bbox[11] = h;

  /* P5 */
  bbox[12] = -r; bbox[13] = r; bbox[14] = 0.0;
  /* P6 */
  bbox[15] = -r; bbox[16] = -r; bbox[17] = 0.0;
  /* P7 */
  bbox[18] = r; bbox[19] = -r; bbox[20] = 0.0;
  /* P8 */
  bbox[21] = r; bbox[22] = r; bbox[23] = 0.0;

 return AY_OK;
} /* ay_cone_bbccb */


/* ay_cone_notifycb:
 *  notification callback function of cone object
 */
int
ay_cone_notifycb(ay_object *o)
{
 ay_cone_object *cone;
 double *pnts;
 double radius, w;
 int i, a;
 double thetadiff, angle;

  if(!o)
    return AY_ENULL;

  cone = (ay_cone_object *)o->refine;

  if(!cone)
    return AY_ENULL;

  if(cone->pnts)
    {
      radius = cone->radius;

      w = (sqrt(2.0)*0.5);

      pnts = cone->pnts;
      if(cone->is_simple)
	{
	  pnts[3] = radius;
	  pnts[4] = 0.0;

	  pnts[6] = radius*w;
	  pnts[7] = -radius*w;

	  pnts[9] = 0.0;
	  pnts[10] = -radius;

	  pnts[12] = -radius*w;
	  pnts[13] = -radius*w;

	  pnts[15] = -radius;
	  pnts[16] = 0.0;

	  pnts[18] = -radius*w;
	  pnts[19] = radius*w;

	  pnts[21] = 0.0;
	  pnts[22] = radius;

	  pnts[24] = radius*w;
	  pnts[25] = radius*w;

	  memcpy(&(pnts[27]),&(pnts[3]),3*sizeof(double));

	  pnts[32] = cone->height;
	}
      else
	{
	  thetadiff = AY_D2R(cone->thetamax/8);
	  angle = 0.0;
	  a = 3;
	  for(i = 0; i <= 8; i++)
	    {
	      pnts[a] = cos(angle)*radius;
	      pnts[a+1] = sin(angle)*radius;

	      a += 3;
	      angle += thetadiff;
	    } /* for */
	  pnts[32] = cone->height;
	} /* if */
    } /* if */

 return AY_OK;
} /* ay_cone_notifycb */


/* ay_cone_providecb:
 *  provide callback function of cone object
 */
int
ay_cone_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 int stride = 4, i, j, height;
 double *cv = NULL, *vk = NULL, *controlv = NULL;
 ay_cone_object *cone = NULL;
 ay_disk_object disk = {0};
 ay_bpatch_object bpatch = {{0}};
 ay_object *new = NULL, d = {0}, **n = NULL;
 ay_nurbpatch_object *np = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNPATCH)
	return AY_OK;
      else
	return AY_ERROR;
    }

  cone = (ay_cone_object *) o->refine;

  if(!cone)
    return AY_ENULL;

  if(type == AY_IDNPATCH)
    {
      if(cone->thetamax < 0.0)
	{
	  ay_status = ay_nb_CreateNurbsCircleArc(cone->radius,
						 cone->thetamax, 0.0,
						 &height, &vk, &cv);
	}
      else
	{
	  ay_status = ay_nb_CreateNurbsCircleArc(cone->radius,
						 0.0, cone->thetamax,
						 &height, &vk, &cv);
	} /* if */

      if(ay_status)
	return ay_status;

      if(!(controlv = calloc(1, height*2*stride*sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      memcpy(controlv, cv, height*stride*sizeof(double));

      j = height*stride+2;
      for(i = 0; i < height; i++)
	{
	  controlv[j] = cone->height;
	  controlv[j+1] = controlv[j+1-(height*stride)];
	  j += stride;
	}

      ay_status = ay_npt_create(2, 3, 2, height, AY_KTBEZIER, AY_KTCUSTOM,
				controlv, NULL, vk, &np);

      if(ay_status)
	goto cleanup;

      if((ay_status = ay_npt_createnpatchobject(&new)))
	goto cleanup;

      ay_trafo_copy(o, new);
      new->down = ay_endlevel;

      new->refine = np;

      if(cone->closed)
	{
	  /* create caps */
	  n = &(new->next);
	  ay_object_defaults(&d);
	  ay_trafo_copy(o, &d);
	  d.type = AY_IDDISK;
	  d.refine = &disk;
	  disk.radius = cone->radius;
	  disk.height = 0.0;
	  disk.thetamax = cone->thetamax;
	  ay_provide_object(&d, AY_IDNPATCH, n);
	  if(*n)
	    {
	      n = &((*n)->next);
	    } /* if */

	  if(fabs(cone->thetamax) != 360.0)
	    {
	      ay_object_defaults(&d);
	      ay_trafo_copy(o, &d);
	      d.type = AY_IDBPATCH;
	      d.refine = &bpatch;
	      memcpy(bpatch.p1, &(controlv[0]), 3*sizeof(double));
	      bpatch.p4[2] = cone->height;
	      ay_provide_object(&d, AY_IDNPATCH, n);
	      if(*n)
		{
		  n = &((*n)->next);
		  memcpy(bpatch.p1, &(controlv[height*stride-stride]),
			 3*sizeof(double));
		  ay_provide_object(&d, AY_IDNPATCH, n);
		} /* if */
	    } /* if */
	} /* if */

      /* copy some tags */
      ay_tag_copyselected(o, new, ay_prefs.converttags,
			  ay_prefs.converttagslen);

      /* return result */
      *result = new;

      /* prevent cleanup code from doing something harmful */
      vk = NULL; controlv = NULL; np = NULL; new = NULL;
    } /* if */

cleanup:

  if(cv)
    free(cv);

  if(vk)
    free(vk);

  if(controlv)
    free(controlv);

  if(np)
    free(np);

  if(new)
    {
      (void)ay_object_deletemulti(new, AY_FALSE);
    }

 return ay_status;
} /* ay_cone_providecb */


/* ay_cone_convertcb:
 *  convert callback function of cone object
 */
int
ay_cone_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_object *new = NULL, *t;
 ay_cone_object *cone;

  if(!o)
    return AY_ENULL;

  cone = (ay_cone_object *) o->refine;

  if(!cone)
    return AY_ENULL;

  /* first, create new object(s) */

  if(cone->closed)
    {
      if(!(new = calloc(1, sizeof(ay_object))))
	{ return AY_EOMEM; }

      ay_object_defaults(new);
      new->type = AY_IDLEVEL;
      new->parent = AY_TRUE;
      new->inherit_trafos = AY_TRUE;

      if(!(new->refine = calloc(1, sizeof(ay_level_object))))
	{ free(new); return AY_EOMEM; }

      ((ay_level_object *)(new->refine))->type = AY_LTLEVEL;

      ay_status = ay_cone_providecb(o, AY_IDNPATCH, &new->down);
      if(ay_status)
	{ free(new->refine); free(new); return ay_status; }

      t = new->down;
      while(t->next)
	{
	  t = t->next;
	}
      t->next = ay_endlevel;
    }
  else
    {
      ay_status = ay_cone_providecb(o, AY_IDNPATCH, &new);
    }


  /* second, link new object(s), or replace old object with it/them */

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
} /* ay_cone_convertcb */


/* ay_cone_init:
 *  initialize the cone object module
 */
int
ay_cone_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_cone_name,
				    ay_cone_createcb,
				    ay_cone_deletecb,
				    ay_cone_copycb,
				    ay_cone_drawcb,
				    ay_cone_drawhcb,
				    ay_cone_shadecb,
				    ay_cone_setpropcb,
				    ay_cone_getpropcb,
				    ay_cone_getpntcb,
				    ay_cone_readcb,
				    ay_cone_writecb,
				    ay_cone_wribcb,
				    ay_cone_bbccb,
				    AY_IDCONE);

  ay_status += ay_notify_register(ay_cone_notifycb, AY_IDCONE);

  ay_status += ay_convert_register(ay_cone_convertcb, AY_IDCONE);

  ay_status += ay_provide_register(ay_cone_providecb, AY_IDCONE);

 return ay_status;
} /* ay_cone_init */

