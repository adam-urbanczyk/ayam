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

/* torus.c - torus object */

static char *ay_torus_name = "Torus";

int
ay_torus_createcb(int argc, char *argv[], ay_object *o)
{
 ay_torus_object *torus = NULL;
 char fname[] = "crttorus";

  if(!o)
    return AY_ENULL;

  if(!(torus = calloc(1, sizeof(ay_torus_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  torus->closed = AY_TRUE;
  torus->majorrad = 0.75;
  torus->minorrad = 0.25;
  torus->phimin = 0.0;
  torus->phimax = 360.0;
  torus->thetamax = 360.0;

  o->refine = torus;

 return AY_OK;
} /* ay_torus_createcb */


int
ay_torus_deletecb(void *c)
{
 ay_torus_object *torus = NULL;

  if(!c)
    return AY_ENULL;    

  torus = (ay_torus_object *)(c);

  free(torus);

 return AY_OK;
} /* ay_torus_deletecb */


int
ay_torus_copycb(void *src, void **dst)
{
 ay_torus_object *torus = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(torus = calloc(1, sizeof(ay_torus_object))))
    return AY_EOMEM; 

  memcpy(torus, src, sizeof(ay_torus_object)); 

  *dst = (void *)torus;

 return AY_OK;
} /* ay_torus_copycb */


int
ay_torus_drawcb(struct Togl *togl, ay_object *o)
{
 ay_torus_object *torus = NULL;
 double phi, mar, mir,thetamax, radius;
 double phidiff, thetadiff, angle;
 int i, j;
 double P1[18] = {0}, R[9];

  if(!o)
    return AY_ENULL;

  torus = (ay_torus_object *)o->refine;

  if(!torus)
    return AY_ENULL;

  mar = torus->majorrad;
  mir = torus->minorrad;
  thetamax = torus->thetamax;
  thetadiff = (thetamax/8);

  phi = torus->phimax - torus->phimin;
  phidiff = AY_D2R(phi/8);
  
  radius = torus->minorrad;
  angle = AY_D2R(torus->phimin);
  for(i = 0; i <= 8; i++)
    {
      P1[i*2] = cos(angle)*radius;
      P1[i*2+1] = sin(angle)*radius;
      R[i] = sqrt(radius*radius-(sin(angle)*radius)*(sin(angle)*radius));
      angle += phidiff;
    }

  /* draw */
  glPushMatrix();
  for(i = 0; i <= 8; i++)
    {
      glBegin(GL_LINE_STRIP);
      for(j = 0; j <= 8; j++)
	{
	  glVertex3d(mar+P1[j*2], 0.0, P1[j*2+1]);
	}
      glEnd();
      glRotated(thetadiff, 0.0, 0.0, 1.0);
    }
  glPopMatrix();

  /* XY-plane rings */
  for(i = 0; i <= 8; i++)
    {

      glBegin(GL_LINE_STRIP);
      angle = 0.0;
      for(j = 0; j <= 8; j++)
	{
	  glVertex3d((mar+P1[i*2])*cos(angle), (mar+P1[i*2])*sin(angle),
		     P1[i*2+1]);
	  angle += AY_D2R(thetadiff);
	}
      glEnd();
    }

  if(torus->closed)
    {
      if(fabs(torus->thetamax) != 360.0)
	{
	  /* ring at 0,0 */
	  for(i = 0; i <= 8; i++)
	    {

	      glBegin(GL_LINE_STRIP);
	      angle = 0.0;
	      for(j = 0; j <= 8; j++)
		{
		  glVertex3d(mar*cos(angle), mar*sin(angle), 0.0);
		  angle += AY_D2R(thetadiff);
		}
	      glEnd();
	    }

	  /* caps */
	  glBegin(GL_LINE_STRIP);
	   glVertex3d(mar+P1[0], 0.0, P1[1]);
	   glVertex3d(mar, 0.0, 0.0);
	   glVertex3d(mar+P1[8*2], 0.0, P1[8*2+1]);
	  glEnd();

	  glRotated(thetamax,0.0,0.0,1.0);

	  glBegin(GL_LINE_STRIP);
	   glVertex3d(mar+P1[0], 0.0, P1[1]);
	   glVertex3d(mar, 0.0, 0.0);
	   glVertex3d(mar+P1[8*2], 0.0, P1[8*2+1]);
	  glEnd();
	}
    }

 return AY_OK;
} /* ay_torus_drawcb */


int
ay_torus_shadecb(struct Togl *togl, ay_object *o)
{
 ay_torus_object *torus = NULL;
 double phi, mar, mir, thetamax, radius;
 double phidiff, thetadiff, angle;
 int i, j;
 double P1[18] = {0}, P2[18] = {0};

  if(!o)
    return AY_ENULL;

  torus = (ay_torus_object *)o->refine;

  if(!torus)
    return AY_ENULL;

  mar = torus->majorrad;
  mir = torus->minorrad;
  thetamax = torus->thetamax;
  thetadiff = (thetamax/8);

  phi = torus->phimax - torus->phimin;
  phidiff = AY_D2R(phi/8);
  
  radius = torus->minorrad;
  angle = AY_D2R(torus->phimin);
  for(i = 0; i <= 8; i++)
    {
      P1[i*2] = cos(angle)*radius;
      P1[i*2+1] = sin(angle)*radius;
      P2[i*2] = cos(angle+AY_HALFPI)*radius;
      P2[i*2+1] = sin(angle+AY_HALFPI)*radius;
      angle += phidiff;
    }

  /* draw */
  angle = 0.0;
  for(i = 0; i < 8; i++)
    {
      glBegin(GL_QUAD_STRIP);
      for(j = 0; j <= 8; j++)
	{
	  glNormal3d((P1[j*2])*cos(angle), (P1[j*2])*sin(angle),
		     P1[j*2+1]);

	  glVertex3d((mar+P1[j*2])*cos(angle), (mar+P1[j*2])*sin(angle),
		     P1[j*2+1]);

	  glNormal3d((P1[j*2])*cos(angle+AY_D2R(thetadiff)),
		     (P1[j*2])*sin(angle+AY_D2R(thetadiff)),
		     P1[j*2+1]);

	  glVertex3d((mar+P1[j*2])*cos(angle+AY_D2R(thetadiff)),
		     (mar+P1[j*2])*sin(angle+AY_D2R(thetadiff)),
		     P1[j*2+1]);

	}
      glEnd();
      angle += AY_D2R(thetadiff);
    }

  if(torus->closed)
    {
      if(fabs(torus->thetamax) != 360.0)
	{
	  /* rings at 0,0 */

	  phidiff = -AY_HALFPI;

	  angle = 0.0;
	  glBegin(GL_QUAD_STRIP);
	  for(j = 0; j <= 8; j++)
	    {
	      glNormal3d((P2[0])*cos(angle),
			 (P2[0])*sin(angle),
			 P2[0+1]);

	      glVertex3d(mar*cos(angle), mar*sin(angle), 0.0);

	      glVertex3d((mar+P1[0])*cos(angle),
			 (mar+P1[0])*sin(angle),
			 P1[0+1]);

	      angle += AY_D2R(thetadiff);
	    }
	  glEnd();

	  angle = 0.0;
	  glBegin(GL_QUAD_STRIP);
	  for(j = 0; j <= 8; j++)
	    {
	      glNormal3d((P2[8*2])*cos(angle),
			 (P2[8*2])*sin(angle),
			 P2[8*2+1]);

	      glVertex3d(mar*cos(angle), mar*sin(angle), 0.0);

	      glVertex3d((mar+P1[8*2])*cos(angle),
			 (mar+P1[8*2])*sin(angle),
			 P1[8*2+1]);

	      angle += AY_D2R(thetadiff);
	    }
	  glEnd();
	  
	  /* caps */
	  glNormal3d(0.0,-1.0,0.0);
	  glBegin(GL_TRIANGLE_FAN);
	   glVertex3d(mar, 0.0, 0.0);
	   for(i = 0; i <= 8; i++)
	     {
	       glVertex3d(mar+P1[i*2], 0.0, P1[i*2+1]);
	     }
	  glEnd();

	  glRotated(thetamax,0.0,0.0,1.0);

	  glBegin(GL_TRIANGLE_FAN);
	   glVertex3d(mar, 0.0, 0.0);
	   for(i = 0; i <= 8; i++)
	     {
	       glVertex3d(mar+P1[i*2], 0.0, P1[i*2+1]);
	     }
	  glEnd();

	}
    }


 return AY_OK;
} /* ay_torus_shadecb */


/* Tcl -> C! */
int
ay_torus_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1 = "TorusAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_torus_object *torus = NULL;
 int itemp = 0;

  if(!o)
    return AY_ENULL;

  torus = (ay_torus_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &itemp);
  torus->closed = (char)itemp;

  Tcl_SetStringObj(ton,"MajorRad",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &torus->majorrad);

  Tcl_SetStringObj(ton,"MinorRad",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &torus->minorrad);

  Tcl_SetStringObj(ton,"PhiMin",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &torus->phimin);

  Tcl_SetStringObj(ton,"PhiMax",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &torus->phimax);

  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &torus->thetamax);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_torus_setpropcb */


/* C -> Tcl! */
int
ay_torus_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="TorusAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_torus_object *torus = NULL;

  if(!o)
    return AY_ENULL;

  torus = (ay_torus_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_NewIntObj(torus->closed);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"MajorRad",-1);
  to = Tcl_NewDoubleObj(torus->majorrad);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"MinorRad",-1);
  to = Tcl_NewDoubleObj(torus->minorrad);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"PhiMin",-1);
  to = Tcl_NewDoubleObj(torus->phimin);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"PhiMax",-1);
  to = Tcl_NewDoubleObj(torus->phimax);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_NewDoubleObj(torus->thetamax);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_torus_getpropcb */


int
ay_torus_readcb(FILE *fileptr, ay_object *o)
{
 ay_torus_object *torus = NULL;
 int itemp = 0;
 if(!o)
   return AY_ENULL;

  if(!(torus = calloc(1, sizeof(ay_torus_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&itemp);
  torus->closed = (char)itemp;
  fscanf(fileptr,"%lg\n",&torus->majorrad);
  fscanf(fileptr,"%lg\n",&torus->minorrad);
  fscanf(fileptr,"%lg\n",&torus->phimin);
  fscanf(fileptr,"%lg\n",&torus->phimax);
  fscanf(fileptr,"%lg\n",&torus->thetamax);

  o->refine = torus;

 return AY_OK;
} /* ay_torus_readcb */


int
ay_torus_writecb(FILE *fileptr, ay_object *o)
{
 ay_torus_object *torus = NULL;

  if(!o)
    return AY_ENULL;

  torus = (ay_torus_object *)(o->refine);

  fprintf(fileptr, "%d\n", torus->closed);
  fprintf(fileptr, "%g\n", torus->majorrad);
  fprintf(fileptr, "%g\n", torus->minorrad);
  fprintf(fileptr, "%g\n", torus->phimin);
  fprintf(fileptr, "%g\n", torus->phimax);
  fprintf(fileptr, "%g\n", torus->thetamax);

 return AY_OK;
} /* ay_torus_writecb */


/* code taken from Affine */
int
ay_torus_wribcb(char *file, ay_object *o)
{
 ay_torus_object *torus = NULL;
 RtPoint p1, p2;
 double phidiff;

  if(!o)
   return AY_ENULL;

  torus = (ay_torus_object*)o->refine;

  if(!torus->closed)
    {
      RiTorus((RtFloat)torus->majorrad, (RtFloat)torus->minorrad,
	      (RtFloat)torus->phimin, (RtFloat)torus->phimax,
	      (RtFloat)torus->thetamax,
	      NULL);
    }
  else
    {
      RiSolidBegin(RI_PRIMITIVE);

      RiTorus((RtFloat)torus->majorrad, (RtFloat)torus->minorrad,
	      (RtFloat)torus->phimin, (RtFloat)torus->phimax,
	      (RtFloat)torus->thetamax,
	      NULL);

      phidiff = torus->phimax - torus->phimin;
      if(fabs(phidiff) < 360.0)
	{
	  
	  p1[0] = (RtFloat)torus->majorrad;
	  p1[1] = (RtFloat)0.0;
	  p1[2] = (RtFloat)0.0;

	  p2[0] = (RtFloat)(cos(AY_D2R(torus->phimin))*torus->minorrad);
	  p2[1] = (RtFloat)0.0;
	  p2[2] = (RtFloat)(sin(AY_D2R(torus->phimin))*torus->minorrad);
	  p2[0] += (RtFloat)torus->majorrad;
	  RiHyperboloid(p1, p2, (RtFloat)torus->thetamax, RI_NULL);

	  p2[0] = (RtFloat)(cos(AY_D2R(torus->phimax))*torus->minorrad);
	  p2[1] = (RtFloat)0.0;
	  p2[2] = (RtFloat)(sin(AY_D2R(torus->phimax))*torus->minorrad);
	  p2[0] += (RtFloat)torus->majorrad;
	  RiAttributeBegin();
	   RiReverseOrientation();
	   RiHyperboloid(p1, p2, (RtFloat)torus->thetamax, RI_NULL);
	  RiAttributeEnd();
	}

      /* caps */
      if(fabs(torus->thetamax) != 360.0)
	{
	  RiTransformBegin();
	   RiTranslate((RtFloat)torus->majorrad, (RtFloat)0.0, (RtFloat)0.0);
	   RiRotate((RtFloat)90.0, (RtFloat)1.0, (RtFloat)0.0, (RtFloat)0.0);
	   RiRotate((RtFloat)torus->phimin, (RtFloat)0.0, (RtFloat)0.0,
		    (RtFloat)1.0);

	   RiDisk((RtFloat)0.0, (RtFloat)torus->minorrad,
		  (RtFloat)(torus->phimax - torus->phimin), RI_NULL);
	  RiTransformEnd();
	  RiAttributeBegin();
	   RiRotate((RtFloat)torus->thetamax, (RtFloat)0.0, (RtFloat)0.0,
		    (RtFloat)1.0);
	   RiTranslate((RtFloat)torus->majorrad, (RtFloat)0.0, (RtFloat)0.0);
	   RiRotate((RtFloat)90.0, (RtFloat)1.0, (RtFloat)0.0, (RtFloat)0.0);
	   RiRotate((RtFloat)torus->phimin, (RtFloat)0.0, (RtFloat)0.0,
		    (RtFloat)1.0);
	   RiReverseOrientation();
	   RiDisk((RtFloat)0.0, (RtFloat)torus->minorrad,
		  (RtFloat)(torus->phimax - torus->phimin), RI_NULL);
	  RiAttributeEnd();

	}

      RiSolidEnd();
    }

 return AY_OK;
} /* ay_torus_wribcb */


int
ay_torus_bbccb(ay_object *o, double *bbox, int *flags)
{
 double r = 0.0, zmi = 0.0, zma = 0.0;
 ay_torus_object *t = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  t = (ay_torus_object *)o->refine; 

  r = t->majorrad+t->minorrad;
  zmi = -(t->minorrad);
  zma = t->minorrad;

  /* XXXX does not take into account ThetaMax! */

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
} /* ay_torus_bbccb */


int
ay_torus_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_torus_name,
				    ay_torus_createcb,
				    ay_torus_deletecb,
				    ay_torus_copycb,
				    ay_torus_drawcb,
				    NULL, /* no points to edit */
				    ay_torus_shadecb,
				    ay_torus_setpropcb,
				    ay_torus_getpropcb,
				    NULL, /* No Picking! */
				    ay_torus_readcb,
				    ay_torus_writecb,
				    ay_torus_wribcb,
				    ay_torus_bbccb,
				    AY_IDTORUS);

 return ay_status;
} /* ay_torus_init */

