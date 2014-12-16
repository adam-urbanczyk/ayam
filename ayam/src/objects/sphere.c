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

/* sphere.c - sphere object */

static char *ay_sphere_name = "Sphere";

int ay_sphere_notifycb(ay_object *o);

/* number of read only points */
#define AY_PSPHERE 30


/* functions: */

/* ay_sphere_createcb:
 *  create callback function of sphere object
 */
int
ay_sphere_createcb(int argc, char *argv[], ay_object *o)
{
 ay_sphere_object *sphere;
 char fname[] = "crtsphere";

  if(!o)
    return AY_ENULL;

  if(!(sphere = calloc(1, sizeof(ay_sphere_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  sphere->closed = AY_TRUE;
  sphere->is_simple = AY_TRUE;
  sphere->radius = 1.0;
  sphere->zmin = -1.0;
  sphere->zmax = 1.0;
  sphere->thetamax = 360.0;

  o->refine = sphere;

 return AY_OK;
} /* ay_sphere_createcb */


/* ay_sphere_deletecb:
 *  delete callback function of sphere object
 */
int
ay_sphere_deletecb(void *c)
{
 ay_sphere_object *sphere;

  if(!c)
    return AY_ENULL;

  sphere = (ay_sphere_object *)(c);

  if(sphere->pnts)
    free(sphere->pnts);

  free(sphere);

 return AY_OK;
} /* ay_sphere_deletecb */


/* ay_sphere_copycb:
 *  copy callback function of sphere object
 */
int
ay_sphere_copycb(void *src, void **dst)
{
 ay_sphere_object *sphere;

  if(!src || !dst)
    return AY_ENULL;

  if(!(sphere = malloc(sizeof(ay_sphere_object))))
    return AY_EOMEM;

  memcpy(sphere, src, sizeof(ay_sphere_object));

  sphere->pnts = NULL;

  *dst = (void *)sphere;

 return AY_OK;
} /* ay_sphere_copycb */


/* ay_sphere_drawcb:
 *  draw (display in an Ayam view window) callback function of sphere object
 */
int
ay_sphere_drawcb(struct Togl *togl, ay_object *o)
{
 ay_sphere_object *sphere;
 double phimax, phimin, phi, rmax, rmin, thetamax, zmin, zmax, radius;
 double phidiff, thetadiff, angle;
 int i, j, k;
 double P1[15] = {0}, P2[135] = {0}, R[5];
 GLdouble w = 0.0;

  if(!o)
    return AY_ENULL;

  sphere = (ay_sphere_object *)o->refine;

  if(!sphere)
    return AY_ENULL;

  radius = sphere->radius;

  /* check, if sphere is simple */
  if(sphere->is_simple)
    {
      /* yes, it is */

      w = (GLdouble)(sqrt(2.0)*0.5);

      /* draw */
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
      /* xz-plane-ring */
      glBegin(GL_LINE_LOOP);
      glVertex3d(radius,0.0,0.0);
      glVertex3d(radius*w,0.0,-radius*w);
      glVertex3d(0.0,0.0,-radius);
      glVertex3d(-radius*w,0.0,-radius*w);
      glVertex3d(-radius,0.0,0.0);
      glVertex3d(-radius*w,0.0,radius*w);
      glVertex3d(0.0,0.0,radius);
      glVertex3d(radius*w,0.0,radius*w);
      glEnd();
      /* yz-plane-ring */
      glBegin(GL_LINE_LOOP);
      glVertex3d(0.0,radius,0.0);
      glVertex3d(0.0,radius*w,-radius*w);
      glVertex3d(0.0,0.0,-radius);
      glVertex3d(0.0,-radius*w,-radius*w);
      glVertex3d(0.0,-radius,0.0);
      glVertex3d(0.0,-radius*w,radius*w);
      glVertex3d(0.0,0.0,radius);
      glVertex3d(0.0,radius*w,radius*w);
      glEnd();

      return AY_OK;
    }

  zmin = sphere->zmin;
  zmax = sphere->zmax;
  thetamax = sphere->thetamax;

  if(fabs(zmax) < radius)
    {
      rmax = sqrt(radius*radius-zmax*zmax);
      phimax = asin(zmax/radius);
    }
  else
    {
      rmax = 0.0;
      phimax = AY_HALFPI;
    }

  if(fabs(zmin) < radius)
    {
      rmin = sqrt(radius*radius-zmin*zmin);
      phimin = asin(zmin/radius);
    }
  else
    {
      rmin = 0.0;
      phimin = -AY_HALFPI;
    }

  phi = fabs(phimax - phimin);

  phidiff = phi/4;
  thetadiff = AY_D2R(thetamax/8);

  angle = phimin<phimax?phimin:phimax;
  for(j = 0; j <= 4; j++)
    {
      P1[j*3] = cos(angle)*radius;
      P1[j*3+2] = sin(angle)*radius;
      R[j] = sqrt(radius*radius-(sin(angle)*radius)*(sin(angle)*radius));
      angle += phidiff;
    }

  angle = 0;
  for(i = 0; i <= 8; i++)
    {
      memcpy(&(P2[i*3*5]), P1, 5*3*sizeof(double));
      k = i*3*5;

      for(j = 0; j < 5; j++)
	{
	  P2[k+(j*3)] *= cos(angle);
	  P2[k+(j*3)+1] = sin(angle) * R[j];
	}
      angle += thetadiff;
    }

  /* draw */
  for(i = 0; i < 9; i++)
    {
      k = i*3*5;
      glBegin(GL_LINE_STRIP);
      for(j = 0; j < 5; j++)
	{
	  glVertex3dv(&(P2[k+(3*j)]));
	}
      glEnd();
    }

  for(j = 0; j < 5; j++)
    {
      k = j*3;
      glBegin(GL_LINE_STRIP);
      for(i = 0; i < 9; i++)
	{
	  glVertex3dv(&(P2[k+(i*3*5)]));
	}
      glEnd();
    }

  if(sphere->closed)
    {
      if(fabs(thetamax) != 360.0)
	{
	  glBegin(GL_LINE_STRIP);
	   glVertex3d(rmin, 0.0, zmin);
	   glVertex3d(0.0,  0.0, zmin);
	   glVertex3d(0.0,  0.0, zmax);
	   glVertex3d(rmax, 0.0, zmax);
	  glEnd();
	  glRotated(thetamax, 0.0, 0.0, 1.0);
	  glBegin(GL_LINES);
	   glVertex3d(rmin, 0.0, zmin);
	   glVertex3d(0.0,  0.0, zmin);
	   glVertex3d(rmax, 0.0, zmax);
	   glVertex3d(0.0,  0.0, zmax);
	  glEnd();
	}
    }

 return AY_OK;
} /* ay_sphere_drawcb */


/* ay_sphere_shadecb:
 *  shade (display in an Ayam view window) callback function of sphere object
 */
int
ay_sphere_shadecb(struct Togl *togl, ay_object *o)
{
 ay_sphere_object *sphere;
 double phimax, phimin, phi, rmax, rmin, thetamax, zmin, zmax, radius;
 double phidiff, thetadiff, angle;
 int i, j, k;
 double P1[15] = {0}, P2[135] = {0}, R[5];

  if(!o)
    return AY_ENULL;

  sphere = (ay_sphere_object *)o->refine;

  if(!sphere)
    return AY_ENULL;

  radius = sphere->radius;

  /* check, if sphere is simple */
  if(sphere->is_simple)
    {
      /* yes, it is */

      /* draw */
      gluSphere(ay_gluquadobj, radius, 8, 4);

      return AY_OK;
    }

  zmin = sphere->zmin;
  zmax = sphere->zmax;
  thetamax = sphere->thetamax;

  if(fabs(zmax) < radius)
    {
      rmax = sqrt(radius*radius-zmax*zmax);
      phimax = asin(zmax/radius);
    }
  else
    {
      rmax = 0.0;
      phimax = AY_HALFPI;
    }

  if(fabs(zmin) < radius)
    {
      rmin = sqrt(radius*radius-zmin*zmin);
      phimin = asin(zmin/radius);
    }
  else
    {
      rmin = 0.0;
      phimin = -AY_HALFPI;
    }

  phi = fabs(phimax - phimin);
  phidiff = phi/4;
  thetadiff = thetamax/8;

  angle = phimin<phimax?phimin:phimax;
  for(j = 0; j <= 4; j++)
    {
      P1[j*3] = cos(angle)*radius;
      P1[j*3+2] = sin(angle)*radius;
      R[j] = sqrt(radius*radius-(sin(angle)*radius)*(sin(angle)*radius));
      angle += phidiff;
    }

  angle = 0;
  for(i = 0; i <= 8; i++)
    {
      memcpy(&(P2[i*3*5]), P1, 5*3*sizeof(double));
      k = i*3*5;

      for(j = 0; j <= 4; j++)
	{
	  P2[k+(j*3)] *= cos(angle);
	  P2[k+(j*3)+1] = sin(angle) * R[j];
	}

      angle += AY_D2R(thetadiff);
    }

  /* draw */
  for(i = 0; i < 8; i++)
    {
      k = i*3*5;
      glBegin(GL_QUAD_STRIP);
      for(j = 0; j < 5; j++)
	{
	  glNormal3dv(&(P2[k+(3*j)]));
	  glVertex3dv(&(P2[k+(3*j)]));
	  glNormal3dv(&(P2[k+(3*5)+(3*j)]));
	  glVertex3dv(&(P2[k+(3*5)+(3*j)]));
	}
      glEnd();
    }

  if(sphere->closed)
    {
      if(fabs(thetamax) != 360.0)
	{
	  glPushMatrix();
	  glNormal3d(0.0,-1.0,0.0);
	  glBegin(GL_TRIANGLE_FAN);
	   glVertex3d(0.0,  0.0, zmin<zmax?zmin:zmax);
	   for(i = 0; i < 5; i++)
	     {
	       glVertex3dv(&(P1[i*3]));
	     }
	  glEnd();
	  glBegin(GL_TRIANGLES);
	   glVertex3d(0.0,  0.0, zmin<zmax?zmin:zmax);
	   glVertex3d(zmin<zmax?rmax:rmin, 0.0, zmin>zmax?zmin:zmax);
	   glVertex3d(0.0,  0.0, zmin>zmax?zmin:zmax);
	  glEnd();

	  glRotated(thetamax, 0.0, 0.0, 1.0);

	  glBegin(GL_TRIANGLE_FAN);
	   glVertex3d(0.0,  0.0, zmin<zmax?zmin:zmax);
	   for(i = 0; i < 5; i++)
	     {
	       glVertex3dv(&(P1[i*3]));
	     }
	  glEnd();
	  glBegin(GL_TRIANGLES);
	   glVertex3d(0.0,  0.0, zmin<zmax?zmin:zmax);
	   glVertex3d(zmin<zmax?rmax:rmin, 0.0, zmin>zmax?zmin:zmax);
	   glVertex3d(0.0,  0.0, zmin>zmax?zmin:zmax);
	  glEnd();

	  glPopMatrix();
	}

      /* draw caps */
      if(rmin != 0.0)
	{
	  glPushMatrix();

	  glTranslated(0.0,0.0,zmin);
	  if(fabs(thetamax) != 360.0)
	    {
	      glRotated(thetamax-90.0, 0.0, 0.0, 1.0);
	      gluPartialDisk(ay_gluquadobj, 0.0, rmin, 8, 1, 0.0, thetamax);
	    }
	  else
	    {
	      gluDisk(ay_gluquadobj, 0.0, rmin, 8, 1);
	    }

	  glPopMatrix();
	}

      if(rmax != 0.0)
	{
	  glPushMatrix();

	  glTranslated(0.0, 0.0, zmax);
	  if(fabs(thetamax) != 360.0)
	    {
	      glRotated(thetamax-90.0, 0.0, 0.0, 1.0);
	      gluPartialDisk(ay_gluquadobj, 0.0, rmax, 8, 1, 0.0, thetamax);
	    }
	  else
	    {
	      gluDisk(ay_gluquadobj, 0.0, rmax, 8, 1);
	    }

	  glPopMatrix();
	}
    } /* if */

 return AY_OK;
} /* ay_sphere_shadecb */


/* ay_sphere_drawhcb:
 *  draw handles (in an Ayam view window) callback function of sphere object
 */
int
ay_sphere_drawhcb(struct Togl *togl, ay_object *o)
{
 int i;
 double *pnts;
 ay_sphere_object *sphere;

  if(!o)
    return AY_ENULL;

  sphere = (ay_sphere_object *) o->refine;

  if(!sphere)
    return AY_ENULL;

  if(!sphere->pnts)
    {
      if(!(pnts = calloc(AY_PSPHERE*3, sizeof(double))))
	{ return AY_EOMEM; }
      sphere->pnts = pnts;
      (void)ay_sphere_notifycb(o);
    }
  else
    {
      pnts = sphere->pnts;
    }

  glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
	    (GLfloat)ay_prefs.obb);

  glBegin(GL_POINTS);
   for(i = 0; i < AY_PSPHERE; i++)
     {
       glVertex3dv((GLdouble *)pnts);
       pnts += 3;
     }
  glEnd();

  glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
	    (GLfloat)ay_prefs.seb);

 return AY_OK;
} /* ay_sphere_drawhcb */


/* ay_sphere_getpntcb:
 *  get point (editing and selection) callback function of sphere object
 */
int
ay_sphere_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_sphere_object *sphere;

  if(!o)
    return AY_ENULL;

  sphere = (ay_sphere_object *)o->refine;

  if(!sphere)
    return AY_ENULL;

  if(!sphere->pnts)
    {
      if(!(sphere->pnts = calloc(AY_PSPHERE*3, sizeof(double))))
	return AY_EOMEM;
      (void)ay_sphere_notifycb(o);
    }

 return ay_selp_getpnts(mode, o, p, pe, 1, AY_PSPHERE, 3, sphere->pnts);
} /* ay_sphere_getpntcb */


/* ay_sphere_setpropcb:
 *  set property (from Tcl to C context) callback function of sphere object
 */
int
ay_sphere_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "SphereAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_sphere_object *sphere;
 int itemp = 0;

  if(!o)
    return AY_ENULL;

  sphere = (ay_sphere_object *)o->refine;

  if(!sphere)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &itemp);
  sphere->closed = (char)itemp;

  Tcl_SetStringObj(ton,"Radius",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &sphere->radius);

  Tcl_SetStringObj(ton,"ZMin",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &sphere->zmin);

  Tcl_SetStringObj(ton,"ZMax",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &sphere->zmax);

  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &sphere->thetamax);

  if((fabs(sphere->zmin) == sphere->radius) &&
     (fabs(sphere->zmax) == sphere->radius) &&
     (fabs(sphere->thetamax) == 360.0))
    {
      sphere->is_simple = AY_TRUE;
    }
  else
    {
      sphere->is_simple = AY_FALSE;
    }

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  (void)ay_sphere_notifycb(o);
  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_sphere_setpropcb */


/* ay_sphere_getpropcb:
 *  get property (from C to Tcl context) callback function of sphere object
 */
int
ay_sphere_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="SphereAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_sphere_object *sphere;

  if(!o)
    return AY_ENULL;

  sphere = (ay_sphere_object *)(o->refine);

  if(!sphere)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_NewIntObj(sphere->closed);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Radius",-1);
  to = Tcl_NewDoubleObj(sphere->radius);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"ZMin",-1);
  to = Tcl_NewDoubleObj(sphere->zmin);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"ZMax",-1);
  to = Tcl_NewDoubleObj(sphere->zmax);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_NewDoubleObj(sphere->thetamax);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_sphere_getpropcb */


/* ay_sphere_readcb:
 *  read (from scene file) callback function of sphere object
 */
int
ay_sphere_readcb(FILE *fileptr, ay_object *o)
{
 ay_sphere_object *sphere;
 int itemp = 0;

  if(!o)
    return AY_ENULL;

  if(!(sphere = calloc(1, sizeof(ay_sphere_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&itemp);
  sphere->closed = (char)itemp;
  fscanf(fileptr,"%lg\n",&sphere->radius);
  fscanf(fileptr,"%lg\n",&sphere->zmin);
  fscanf(fileptr,"%lg\n",&sphere->zmax);
  fscanf(fileptr,"%lg\n",&sphere->thetamax);

  if((fabs(sphere->zmin) == sphere->radius) &&
     (fabs(sphere->zmax) == sphere->radius) &&
     (fabs(sphere->thetamax) == 360.0))
    {
      sphere->is_simple = AY_TRUE;
    }
  else
    {
      sphere->is_simple = AY_FALSE;
    }

  o->refine = sphere;

 return AY_OK;
} /* ay_sphere_readcb */


/* ay_sphere_writecb:
 *  write (to scene file) callback function of sphere object
 */
int
ay_sphere_writecb(FILE *fileptr, ay_object *o)
{
 ay_sphere_object *sphere;

  if(!o)
    return AY_ENULL;

  sphere = (ay_sphere_object *)(o->refine);

  if(!sphere)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", (int)sphere->closed);
  fprintf(fileptr, "%g\n", sphere->radius);
  fprintf(fileptr, "%g\n", sphere->zmin);
  fprintf(fileptr, "%g\n", sphere->zmax);
  fprintf(fileptr, "%g\n", sphere->thetamax);

 return AY_OK;
} /* ay_sphere_writecb */


/* ay_sphere_wribcb:
 *  RIB export callback function of sphere object
 *  code taken from Affine
 */
int
ay_sphere_wribcb(char *file, ay_object *o)
{
 ay_sphere_object *sphere;
 RtPoint P1[16];
 RtPoint P2[16];
 RtFloat rmin, rmax;
 RtFloat xmid, zmid;
 RtFloat angle, circle;
 RtFloat phimax, phimin, phidiff;
 RtFloat radius, zmin, zmax, thetamax;

#define X 0
#define Y 1
#define Z 2
#define UNITCIRCLE (0.5522847)

  if(!o)
   return AY_ENULL;

  sphere = (ay_sphere_object*)o->refine;

  if(!sphere)
    return AY_ENULL;

  if(!sphere->closed)
  {
    RiSphere((RtFloat)sphere->radius,
	     (RtFloat)sphere->zmin,
	     (RtFloat)sphere->zmax,
	     (RtFloat)sphere->thetamax,
	     NULL);
  }
  else
  {
    radius = (RtFloat)sphere->radius;
    zmin = (RtFloat)sphere->zmin;
    zmax = (RtFloat)sphere->zmax;
    thetamax = (RtFloat)sphere->thetamax;


    if(radius == 0.0)
      return AY_OK;

    RiSphere(radius, zmin, zmax, thetamax, NULL);

    /* Top if needed. */
    if(fabs(zmax) < radius)
      {
	/* Refer to [UPST90]. */
	rmax = (RtFloat)(sqrt(radius*radius-zmax*zmax));
	RiDisk(zmax, rmax, thetamax, NULL);

	phimax = (RtFloat)(asin(zmax/radius));
      }
    else
      {
	rmax = (RtFloat)0.0;
	phimax = (RtFloat)(AY_HALFPI);
      }

    /* Bottom if needed. */
    if(fabs(zmin) < radius)
      {
	RiAttributeBegin();
	 RiReverseOrientation();
	 /* Refer to [UPST90]. */
	 rmin = (RtFloat)(sqrt(radius*radius-zmin*zmin));
	 RiDisk(zmin, rmin, thetamax, NULL);
	RiAttributeEnd();

	phimin = (RtFloat)(asin(zmin/radius));
      }
    else
      {
	rmin = (RtFloat)0.0;
	phimin = (RtFloat)(-AY_HALFPI);
      }

    /* Don't add patches for wedge shape if they are not needed. */
    if(fabs(thetamax) != 360.0)
      {
	phidiff = phimax - phimin;
	zmid = (RtFloat)(sin(phimin + phidiff/2.0));
	xmid = (RtFloat)(sqrt(radius*radius-zmid*zmid));

	/* The Y coordinates need to be zero, so just clear everything. */
	memset(P1, 0, 16*sizeof(RtPoint));
	memset(P2, 0, 16*sizeof(RtPoint));

	/* Calculate the patch from bottom to middle. */
	P1[0][Z] = P1[1][Z] = P1[2][Z] = P1[3][Z] = zmin; /* height */
	P1[0][X] = (RtFloat)0.0;
	P1[3][X] = (RtFloat)rmin; /* radius */
	P1[1][X] = (RtFloat)(P1[3][X]/3.0);
	P1[2][X] = (RtFloat)(2.0*P1[1][X]);

	P1[12][Z] = P1[13][Z] = P1[14][Z] = P1[15][Z] = zmid; /* middle's
								 height */
	P1[12][X] = (RtFloat)0.0;
	P1[15][X] = xmid;
	P1[13][X] = (RtFloat)(P1[15][X]/3.0);
	P1[14][X] = (RtFloat)(2.0*P1[13][X]);

	angle = (RtFloat)(phimin + AY_HALFPI);
	circle = (RtFloat)((UNITCIRCLE/AY_PI)*radius*phidiff);

	P1[4][Z] = P1[5][Z] = P1[6][Z] = P1[7][Z] =
	  (RtFloat)(zmin + circle*sin(angle));
	P1[4][X] = (RtFloat)0.0;
	P1[7][X] = (RtFloat)(rmin + circle*cos(angle));
	P1[5][X] = (RtFloat)(P1[7][X]/3.0);
	P1[6][X] = (RtFloat)(2.0*P1[5][X]);

	angle = (RtFloat)(phimin + phidiff/2.0 - AY_HALFPI);
	P1[8][Z]  = P1[9][Z] = P1[10][Z] = P1[11][Z] = (RtFloat)
	  (zmid + circle*sin(angle));
	P1[8][X]  = (RtFloat)0.0;
	P1[11][X] = (RtFloat)(xmid + circle*cos(angle));
	P1[9][X]  = (RtFloat)(P1[11][X]/3.0);
	P1[10][X] = (RtFloat)(2.0*P1[9][X]);


	/* Calculate the patch from equator to top. */
	P2[0][Z] = P2[1][Z] = P2[2][Z] = P2[3][Z] = zmid; /* middle's height */
	P2[0][X] = (RtFloat)0.0;
	P2[3][X] = xmid;
	P2[1][X] = (RtFloat)(P2[3][X]/3.0);
	P2[2][X] = (RtFloat)(2.0*P2[1][X]);

	P2[12][Z] = P2[13][Z] = P2[14][Z] = P2[15][Z] = zmax; /* height */
	P2[12][X] = (RtFloat)0.0;
	P2[15][X] = rmax; /* Radius at zmax. */
	P2[13][X] = (RtFloat)(P2[15][X]/3.0);
	P2[14][X] = (RtFloat)(2.0*P2[13][X]);

	angle = (RtFloat)(phimin + phidiff/2.0 + AY_HALFPI);
	circle = (RtFloat)((UNITCIRCLE/AY_PI)*radius*phidiff);

	P2[4][Z] = P2[5][Z] = P2[6][Z] = P2[7][Z] =
	  (RtFloat)(zmid + circle*sin(angle));
	P2[4][X] = (RtFloat)0.0;
	P2[7][X] = (RtFloat)(xmid + circle*cos(angle));
	P2[5][X] = (RtFloat)(P2[7][X]/3.0);
	P2[6][X] = (RtFloat)(2.0*P2[5][X]);

	angle = (RtFloat)(phimin + phidiff - AY_HALFPI);
	P2[8][Z]  = P2[9][Z] = P2[10][Z] = P2[11][Z] = (RtFloat)(zmax +
							   circle*sin(angle));
	P2[8][X]  = (RtFloat)0.0;
	P2[11][X] = (RtFloat)(rmax + circle*cos(angle));
	P2[9][X]  = (RtFloat)(P2[11][X]/3.0);
	P2[10][X] = (RtFloat)(2.0*P2[9][X]);

	RiPatch(RI_BICUBIC, RI_P, (RtPointer)P1, NULL);
	RiPatch(RI_BICUBIC, RI_P, (RtPointer)P2, NULL);

	RiAttributeBegin();
	 RiRotate(thetamax, (RtFloat)0.0, (RtFloat)0.0, (RtFloat)1.0);
	 RiReverseOrientation();
	 RiPatch(RI_BICUBIC, RI_P, (RtPointer)P1, NULL);
	 RiPatch(RI_BICUBIC, RI_P, (RtPointer)P2, NULL);
	RiAttributeEnd();
      } /* if */

  } /* if */

#undef X
#undef Y
#undef Z
#undef UNITCIRCLE

 return AY_OK;
} /* ay_sphere_wribcb */


/* ay_sphere_bbccb:
 *  bounding box calculation callback function of sphere object
 */
int
ay_sphere_bbccb(ay_object *o, double *bbox, int *flags)
{
 double r = 0.0, zmi = 0.0, zma = 0.0;
 ay_sphere_object *sphere;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  sphere = (ay_sphere_object *)o->refine;

  if(!sphere)
    return AY_ENULL;

  if(!sphere->is_simple)
    {
      if(!sphere->pnts)
	{
	  if(!(sphere->pnts = calloc(AY_PSPHERE*3, sizeof(double))))
	    { return AY_EOMEM; }
	  (void)ay_sphere_notifycb(o);
	}

      return ay_bbc_fromarr(sphere->pnts, AY_PSPHERE, 3, bbox);
    }

  r = sphere->radius;
  zmi = sphere->zmin;
  zma = sphere->zmax;

  /* P1 */
  bbox[0] = -r; bbox[1] = r; bbox[2] = zma;
  /* P2 */
  bbox[3] = -r; bbox[4] = -r; bbox[5] = zma;
  /* P3 */
  bbox[6] = r; bbox[7] = -r; bbox[8] = zma;
  /* P4 */
  bbox[9] = r; bbox[10] = r; bbox[11] = zma;

  /* P5 */
  bbox[12] = -r; bbox[13] = r; bbox[14] = zmi;
  /* P6 */
  bbox[15] = -r; bbox[16] = -r; bbox[17] = zmi;
  /* P7 */
  bbox[18] = r; bbox[19] = -r; bbox[20] = zmi;
  /* P8 */
  bbox[21] = r; bbox[22] = r; bbox[23] = zmi;

 return AY_OK;
} /* ay_sphere_bbccb */


/* ay_sphere_notifycb:
 *  notification callback function of sphere object
 */
int
ay_sphere_notifycb(ay_object *o)
{
 ay_sphere_object *sphere;
 double *pnts;
 double radius, w, zmin, zmax, rmin, rmax;
 int i, a;
 double thetadiff, angle, hh;

  if(!o)
    return AY_ENULL;

  sphere = (ay_sphere_object *)o->refine;

  if(!sphere)
    return AY_ENULL;

  if(sphere->pnts)
    {
      radius = sphere->radius;
      w = (sqrt(2.0)*0.5);
      pnts = sphere->pnts;

      if(fabs(sphere->zmin) < sphere->radius)
	zmin = sphere->zmin;
      else
	if(sphere->zmin < 0)
	  zmin = -sphere->radius;
	else
	  zmin = sphere->radius;

      if(fabs(sphere->zmax) < sphere->radius)
	zmax = sphere->zmax;
      else
	if(sphere->zmax < 0)
	  zmax = -sphere->radius;
	else
	  zmax = sphere->radius;

      hh = zmin + ((zmax - zmin) / 2.0);

      memset(&(pnts[0]), 0, AY_PSPHERE*3*sizeof(double));

      pnts[2] = zmin;
      pnts[5] = hh;
      pnts[8] = zmax;

      if(sphere->is_simple)
	{
	  /* lower ring */
	  memset(&(pnts[9]), 0, 9*3*sizeof(double));

	  /* middle ring */
	  pnts[36] = radius;
	  pnts[37] = 0.0;

	  pnts[39] = radius*w;
	  pnts[40] = -radius*w;

	  pnts[42] = 0.0;
	  pnts[43] = -radius;

	  pnts[45] = -radius*w;
	  pnts[46] = -radius*w;

	  pnts[48] = -radius;
	  pnts[49] = 0.0;

	  pnts[51] = -radius*w;
	  pnts[52] = radius*w;

	  pnts[54] = 0.0;
	  pnts[55] = radius;

	  pnts[57] = radius*w;
	  pnts[58] = radius*w;

	  memcpy(&(pnts[60]),&(pnts[36]),3*sizeof(double));

	  /* upper ring */
 	  memset(&(pnts[63]), 0, 9*3*sizeof(double));
	}
      else
	{
	  if(fabs(sphere->zmax) < radius)
	    {
	      rmax = sqrt(radius*radius-sphere->zmax*sphere->zmax);
	    }
	  else
	    {
	      rmax = 0.0;
	    }

	  if(fabs(sphere->zmin) < radius)
	    {
	      rmin = sqrt(radius*radius-sphere->zmin*sphere->zmin);
	    }
	  else
	    {
	      rmin = 0.0;
	    }

	  thetadiff = AY_D2R(sphere->thetamax/8);

	  /* lower ring */
	  angle = 0.0;
	  a = 9;
	  for(i = 0; i <= 8; i++)
	    {
	      pnts[a] = cos(angle)*rmin;
	      pnts[a+1] = sin(angle)*rmin;

	      a += 3;
	      angle += thetadiff;
	    } /* for */

	  /* middle ring */
	  angle = 0.0;
	  for(i = 0; i <= 8; i++)
	    {
	      pnts[a] = cos(angle)*radius;
	      pnts[a+1] = sin(angle)*radius;

	      a += 3;
	      angle += thetadiff;
	    } /* for */

	  /* upper ring */
	  angle = 0.0;
	  for(i = 0; i <= 8; i++)
	    {
	      pnts[a] = cos(angle)*rmax;
	      pnts[a+1] = sin(angle)*rmax;

	      a += 3;
	      angle += thetadiff;
	    } /* for */
	} /* if */

      /* set heights */

      /* lower ring */
      a = 11;
      for(i = 0; i <= 8; i++)
	{
	  pnts[a] = zmin;
	  a += 3;
	} /* for */
      /* middle ring */
      for(i = 0; i <= 8; i++)
	{
	  pnts[a] = hh;
	  a += 3;
	} /* for */
      /* upper ring */
      for(i = 0; i <= 8; i++)
	{
	  pnts[a] = zmax;
	  a += 3;
	} /* for */
    } /* if */

 return AY_OK;
} /* ay_sphere_notifycb */


/* ay_sphere_providecb:
 *  provide callback function of sphere object
 */
int
ay_sphere_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 int i, j, k, height, stride = 4;
 double *cv = NULL, *cv2 = NULL, *kn = NULL;
 double xaxis[3] = {1.0, 0.0, 0.0}, quat[4] = {0};
 double phimax, phimin, rmax, rmin, thetamax, zmin, zmax, radius;
 ay_nurbpatch_object *np;
 ay_sphere_object *sphere;
 ay_disk_object disk = {0};
 ay_object *new = NULL, *newc = NULL, *newp = NULL, d = {0}, **n = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNPATCH)
	return AY_OK;
      else
	return AY_ERROR;
    }

  sphere = (ay_sphere_object *) o->refine;

  if(!sphere)
    return AY_ENULL;

  if(type == AY_IDNPATCH)
    {
      if((ay_status = ay_npt_createnpatchobject(&new)))
	goto cleanup;
      new->down = ay_endlevel;

      radius = sphere->radius;
      if(sphere->is_simple)
	{
	  if((ay_status = ay_npt_crtnsphere(radius,
			      (ay_nurbpatch_object **)(void*)&(new->refine))))
	    goto cleanup;
	  ay_trafo_copy(o, new);
	}
      else
	{
	  zmin = sphere->zmin;
	  zmax = sphere->zmax;
	  thetamax = sphere->thetamax;
	  if(fabs(zmax) < radius)
	    {
	      rmax = sqrt(radius*radius-zmax*zmax);
	      phimax = asin(zmax/radius);
	    }
	  else
	    {
	      rmax = 0.0;
	      phimax = AY_HALFPI;
	    }

	  if(fabs(zmin) < radius)
	    {
	      rmin = sqrt(radius*radius-zmin*zmin);
	      phimin = asin(zmin/radius);
	    }
	  else
	    {
	      rmin = 0.0;
	      phimin = -AY_HALFPI;
	    }

	  if((ay_status = ay_nb_CreateNurbsCircleArc(sphere->radius,
					      AY_R2D(phimin), AY_R2D(phimax),
						     &height, &kn, &cv)))
	    goto cleanup;

	  if(!(newc = calloc(1, sizeof(ay_object))))
	    {
	      ay_status = AY_EOMEM;
	      goto cleanup;
	    }
	  newc->type = AY_IDNCURVE;
	  ay_object_defaults(newc);

	  if((ay_status = ay_nct_create(3, height, AY_KTCUSTOM, cv, kn,
			     (ay_nurbcurve_object **)(void*)&(newc->refine))))
	    goto cleanup;

	  if((ay_status = ay_npt_revolve(newc, thetamax, 0, 0,
			      (ay_nurbpatch_object **)(void*)&(new->refine))))
	    goto cleanup;

	  ay_quat_axistoquat(xaxis, AY_D2R(90.0), quat);
	  new->rotx += 90.0;
	  ay_quat_add(new->quat, quat, new->quat);
	  new->scaly *= -1.0;
	  ay_npt_applytrafo(new);
	  ay_trafo_copy(o, new);

	  kn = NULL;
	  cv = NULL;

	  /* create caps */
	  if(sphere->closed)
	    {
	      n = &(new->next);

	      /* lower cap */
	      if(fabs(zmin) < radius)
		{
		  ay_object_defaults(&d);
		  ay_trafo_copy(o, &d);
		  d.type = AY_IDDISK;
		  d.refine = &disk;
		  disk.radius = rmin;
		  disk.height = sphere->zmin;
		  disk.thetamax = sphere->thetamax;
		  if((ay_status = ay_provide_object(&d, AY_IDNPATCH, n)))
		    goto cleanup;
		  if(*n)
		    {
		      n = &((*n)->next);
		    }
		} /* if */

	      /* upper cap */
	      if(fabs(zmax) < radius)
		{
		  ay_object_defaults(&d);
		  ay_trafo_copy(o, &d);
		  d.type = AY_IDDISK;
		  d.refine = &disk;
		  disk.radius = rmax;
		  disk.height = sphere->zmax;
		  disk.thetamax = sphere->thetamax;
		  if((ay_status = ay_provide_object(&d, AY_IDNPATCH, n)))
		    goto cleanup;
		  if(*n)
		    {
		      n = &((*n)->next);
		    }
		} /* if */

	      if(fabs(thetamax) != 360.0)
		{
		  /* side cap (at theta=0.0) */
		  if(!(cv2 = calloc(2*height*stride, sizeof(double))))
		    {
		      ay_status = AY_EOMEM;
		      goto cleanup;
		    }
		  np = (ay_nurbpatch_object *)new->refine;
		  cv = np->controlv;

		  j = 0; k = (np->height-1)*stride;
		  for(i = 0; i < height; i++)
		    {
		      memcpy(&(cv2[j]), &(cv[k]), stride*sizeof(double));
		      j += stride; k += np->height*stride;
		    }

		  j = height*stride; k = 0;
		  for(i = 0; i < height; i++)
		    {
		      /*X = 0.0*/
		      /*Y = 0.0*/
		      /*Z*/
		      cv2[j+2] = cv2[k+2];
		      /*W*/
		      cv2[j+3] = 1.0;
		      j += stride; k += stride;
		    }

		  if(!(kn = calloc(height+3, sizeof(double))))
		    {
		      ay_status = AY_EOMEM;
		      goto cleanup;
		    }
		  memcpy(kn, np->uknotv, (height+3)*sizeof(double));

		  if((ay_status = ay_npt_createnpatchobject(&newp)))
		    goto cleanup;
		  ay_trafo_copy(o, newp);
		  newp->down = ay_endlevel;

		  if((ay_status = ay_npt_create(2, 3, 2, height,
						AY_KTNURB, AY_KTCUSTOM,
						cv2, NULL, kn,
			      (ay_nurbpatch_object**)(void*)&(newp->refine))))
		    goto cleanup;

		  kn = NULL;
		  *n = newp;
		  n = &((*n)->next);

		  /* side cap (at theta=ThetaMax) */
		  if((ay_status = ay_object_copy(newp, n)))
		    goto cleanup;

		  np = (ay_nurbpatch_object *)((*n)->refine);
		  cv2 = np->controlv;
		  np = (ay_nurbpatch_object *)new->refine;

		  j = 0; k = 0;
		  for(i = 0; i < height; i++)
		    {
		      memcpy(&(cv2[j]), &(cv[k]), stride*sizeof(double));
		      j += stride; k += (np->height*stride);
		    }

		  /* prevent cleanup code from doing something harmful */
		  cv = NULL;
		  cv2 = NULL;
		} /* if */
	    } /* if */
	} /* if */

      /* copy some tags */
      (void)ay_tag_copyselected(o, new, ay_prefs.converttags,
				ay_prefs.converttagslen);

      /* return result */
      *result = new;

      /* prevent cleanup code from doing something harmful */
      new = NULL;
    } /* if */

cleanup:

  if(cv)
    free(cv);

  if(cv2)
    free(cv2);

  if(kn)
    free(kn);

  if(new)
    {
      (void)ay_object_deletemulti(new, AY_FALSE);
    }

  if(newc)
    {
      (void)ay_object_delete(newc);
    }

 return ay_status;
} /* ay_sphere_providecb */


/* ay_sphere_convertcb:
 *  convert callback function of sphere object
 */
int
ay_sphere_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_object *new = NULL, *t;
 ay_sphere_object *sphere;

  if(!o)
    return AY_ENULL;

  sphere = (ay_sphere_object *) o->refine;

  if(!sphere)
    return AY_ENULL;

  /* first, create new object(s) */

  if(!sphere->is_simple)
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

      ay_status = ay_sphere_providecb(o, AY_IDNPATCH, &new->down);
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
      ay_status = ay_sphere_providecb(o, AY_IDNPATCH, &new);
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
} /* ay_sphere_convertcb */


/* ay_sphere_init:
 *  initialize the sphere object module
 */
int
ay_sphere_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_sphere_name,
				    ay_sphere_createcb,
				    ay_sphere_deletecb,
				    ay_sphere_copycb,
				    ay_sphere_drawcb,
				    ay_sphere_drawhcb,
				    ay_sphere_shadecb,
				    ay_sphere_setpropcb,
				    ay_sphere_getpropcb,
				    ay_sphere_getpntcb,
				    ay_sphere_readcb,
				    ay_sphere_writecb,
				    ay_sphere_wribcb,
				    ay_sphere_bbccb,
				    AY_IDSPHERE);

  ay_status += ay_notify_register(ay_sphere_notifycb, AY_IDSPHERE);

  ay_status += ay_convert_register(ay_sphere_convertcb, AY_IDSPHERE);

  ay_status += ay_provide_register(ay_sphere_providecb, AY_IDSPHERE);

 return ay_status;
} /* ay_sphere_init */

