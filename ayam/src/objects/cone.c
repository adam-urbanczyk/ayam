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

/* cone.c - cone object */

static char *ay_cone_name = "Cone";

int
ay_cone_createcb(int argc, char *argv[], ay_object *o)
{
 ay_cone_object *cone = NULL;
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


int
ay_cone_deletecb(void *c)
{
 ay_cone_object *cone = NULL;

  if(!c)
    return AY_ENULL;    

  cone = (ay_cone_object *)(c);

  free(cone);

 return AY_OK;
} /* ay_cone_deletecb */


int
ay_cone_copycb(void *src, void **dst)
{
 ay_cone_object *cone = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(cone = calloc(1, sizeof(ay_cone_object))))
    return AY_EOMEM; 

  memcpy(cone, src, sizeof(ay_cone_object)); 

  *dst = (void *)cone;

 return AY_OK;
} /* ay_cone_copycb */


int
ay_cone_drawcb(struct Togl *togl, ay_object *o)
{
 ay_cone_object *cone = NULL;
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
  w = (GLdouble)(sqrt(2.0)*0.5);

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


int
ay_cone_shadecb(struct Togl *togl, ay_object *o)
{
 ay_cone_object *cone = NULL;
 GLUquadricObj *qobj = NULL;
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
      if(!(qobj = gluNewQuadric()))
	return AY_EOMEM;

      /* draw */
      gluCylinder(qobj, radius, 0.0, height, 8, 1);

      gluDeleteQuadric(qobj);
      
      if(cone->closed)
	{
	  if(!(qobj = gluNewQuadric()))
	    return AY_EOMEM;
	  gluDisk(qobj, 0.0, radius, 8, 1);
	  gluDeleteQuadric(qobj);
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

       glNormal3d(0.0, -1.0, 0.0);
       glBegin(GL_TRIANGLES);
        glVertex3d(0.0,    0.0, 0.0);
	glVertex3d(radius, 0.0, 0.0);
	glVertex3d(0.0,    0.0, height);
       glEnd();

       glRotated(thetamax, 0.0, 0.0, 1.0);
       glBegin(GL_TRIANGLES);
        glVertex3d(0.0,    0.0, 0.0);
	glVertex3d(radius, 0.0, 0.0);
	glVertex3d(0.0,    0.0, height);
       glEnd();

      glPopMatrix();

      /* draw cap */
      glPushMatrix();

       qobj = NULL;
       if(!(qobj = gluNewQuadric()))
	 return AY_EOMEM;

       glRotated(thetamax-90.0, 0.0, 0.0, 1.0);
       gluPartialDisk(qobj, 0.0, radius, 8, 1, 0.0, thetamax);

       gluDeleteQuadric(qobj);

      glPopMatrix();

    }

 return AY_OK;
} /* ay_cone_shadecb */


/* Tcl -> C! */
int
ay_cone_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1 = "ConeAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_cone_object *cone = NULL;
 int itemp = 0;

  if(!o)
    return AY_ENULL;

  cone = (ay_cone_object *)o->refine;
  
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

 return AY_OK;
} /* ay_cone_setpropcb */


/* C -> Tcl! */
int
ay_cone_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="ConeAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_cone_object *cone = NULL;

  if(!o)
    return AY_ENULL;

  cone = (ay_cone_object *)(o->refine);

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


int
ay_cone_readcb(FILE *fileptr, ay_object *o)
{
 ay_cone_object *cone = NULL;
 int itemp = 0;
 if(!o)
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


int
ay_cone_writecb(FILE *fileptr, ay_object *o)
{
 ay_cone_object *cone = NULL;

  if(!o)
    return AY_ENULL;

  cone = (ay_cone_object *)(o->refine);

  fprintf(fileptr, "%d\n", cone->closed);
  fprintf(fileptr, "%g\n", cone->radius);
  fprintf(fileptr, "%g\n", cone->height);
  fprintf(fileptr, "%g\n", cone->thetamax);

 return AY_OK;
} /* ay_cone_writecb */


int
ay_cone_wribcb(char *file, ay_object *o)
{
 ay_cone_object *cone = NULL;
 RtPoint rect[4];

  if(!o)
   return AY_ENULL;

  cone = (ay_cone_object*)o->refine;

  if(!cone->closed)
    {
      RiCone((RtFloat)cone->height,
	     (RtFloat)cone->radius,
	     (RtFloat)cone->thetamax,
	     RI_NULL);
    }
  else
    {
    if(cone->thetamax == 180.0)
      {
	RiSolidBegin(RI_PRIMITIVE);

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

	RiSolidEnd();
      }
    else if(cone->thetamax == 360.0)
      {
	RiSolidBegin(RI_PRIMITIVE);
	 RiCone((RtFloat)cone->height, (RtFloat)cone->radius,
		(RtFloat)360.0, RI_NULL);
	 RiAttributeBegin();
	  RiReverseOrientation();
	  RiDisk((RtFloat)0.0, (RtFloat)cone->radius, (RtFloat)360.0, RI_NULL);
	 RiAttributeEnd();
	RiSolidEnd();
      }
    else
      {
	RiSolidBegin(RI_PRIMITIVE);

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

	RiSolidEnd();
      }
    }

 return AY_OK;
} /* ay_cone_wribcb */


int
ay_cone_bbccb(ay_object *o, double *bbox, int *flags)
{
 double r = 0.0, h = 0.0;
 ay_cone_object *cone = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  cone = (ay_cone_object *)o->refine; 

  r = cone->radius;
  h = cone->height;

  /* XXXX does not take into account ThetaMax! */

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


int
ay_cone_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_cone_name,
				    ay_cone_createcb,
				    ay_cone_deletecb,
				    ay_cone_copycb,
				    ay_cone_drawcb,
				    NULL, /* no points to edit */
				    ay_cone_shadecb,
				    ay_cone_setpropcb,
				    ay_cone_getpropcb,
				    NULL, /* No Picking! */
				    ay_cone_readcb,
				    ay_cone_writecb,
				    ay_cone_wribcb,
				    ay_cone_bbccb,
				    AY_IDCONE);

 return ay_status;
} /* ay_cone_init */

