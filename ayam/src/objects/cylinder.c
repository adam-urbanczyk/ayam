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

/* cylinder.c - cylinder object */

static char *ay_cylinder_name = "Cylinder";

int
ay_cylinder_createcb(int argc, char *argv[], ay_object *o)
{
 ay_cylinder_object *cylinder = NULL;
 char fname[] = "crtcylinder";

  if(!o)
    return AY_ENULL;

  if(!(cylinder = calloc(1, sizeof(ay_cylinder_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  cylinder->closed = AY_TRUE;
  cylinder->is_simple = AY_TRUE;
  cylinder->radius = 1.0;
  cylinder->zmin = -1.0;
  cylinder->zmax = 1.0;
  cylinder->thetamax = 360.0;

  o->refine = cylinder;

 return AY_OK;
} /* ay_cylinder_createcb */


int
ay_cylinder_deletecb(void *c)
{
 ay_cylinder_object *cylinder = NULL;

  if(!c)
    return AY_ENULL;    

  cylinder = (ay_cylinder_object *)(c);

  free(cylinder);

 return AY_OK;
} /* ay_cylinder_deletecb */


int
ay_cylinder_copycb(void *src, void **dst)
{
 ay_cylinder_object *cylinder = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(cylinder = calloc(1, sizeof(ay_cylinder_object))))
    return AY_EOMEM; 

  memcpy(cylinder, src, sizeof(ay_cylinder_object)); 

  *dst = (void *)cylinder;

 return AY_OK;
} /* ay_cylinder_copycb */


int
ay_cylinder_drawcb(struct Togl *togl, ay_object *o)
{
 ay_cylinder_object *cylinder = NULL;
 double thetamax, zmin, zmax, radius;
 double thetadiff, angle;
 int i;
 double P1[9*2] = {0};
 GLdouble w = 0.0;

  if(!o)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)o->refine;

  if(!cylinder)
    return AY_ENULL;

  radius = cylinder->radius;
  zmin = cylinder->zmin;
  zmax = cylinder->zmax;

  /* check, if cylinder is simple */
  if(cylinder->is_simple)
    {
      /* yes, it is */

      w = (GLdouble)(sqrt(2.0)*0.5);

      /* xy-plane-rings */
      glBegin(GL_LINE_LOOP);
       glVertex3d(radius,0.0,zmin);
       glVertex3d(radius*w,-radius*w,zmin);
       glVertex3d(0.0,-radius,zmin);
       glVertex3d(-radius*w,-radius*w,zmin);
       glVertex3d(-radius,0.0,zmin);
       glVertex3d(-radius*w,radius*w,zmin);
       glVertex3d(0.0,radius,zmin);
       glVertex3d(radius*w,radius*w,zmin);
      glEnd();
      glBegin(GL_LINE_LOOP);
       glVertex3d(radius,0.0,zmax);
       glVertex3d(radius*w,-radius*w,zmax);
       glVertex3d(0.0,-radius,zmax);
       glVertex3d(-radius*w,-radius*w,zmax);
       glVertex3d(-radius,0.0,zmax);
       glVertex3d(-radius*w,radius*w,zmax);
       glVertex3d(0.0,radius,zmax);
       glVertex3d(radius*w,radius*w,zmax);
      glEnd();
      /* wall */
      glBegin(GL_LINES);
       glVertex3d(radius,0.0,zmin);
       glVertex3d(radius,0.0,zmax);
       glVertex3d(0.0,-radius,zmin);
       glVertex3d(0.0,-radius,zmax);
       glVertex3d(-radius,0.0,zmin);
       glVertex3d(-radius,0.0,zmax);
       glVertex3d(0.0,radius,zmin);
       glVertex3d(0.0,radius,zmax);
      glEnd();

      if(cylinder->closed)
	{
	  glBegin(GL_LINES);
	   glVertex3d(radius, 0.0, zmin);
	   glVertex3d(-radius, 0.0, zmin);
	   glVertex3d(0.0, radius, zmin);
	   glVertex3d(0.0, -radius, zmin);
	   glVertex3d(radius, 0.0, zmax);
	   glVertex3d(-radius, 0.0, zmax);
	   glVertex3d(0.0, radius, zmax);
	   glVertex3d(0.0, -radius, zmax);
	  glEnd();
	}

      return AY_OK;
    }

  thetamax = cylinder->thetamax;
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
       glVertex3d(P1[i*2], P1[i*2+1], zmin);
     }
  glEnd();

  glBegin(GL_LINE_STRIP);
   for(i = 0; i <= 8; i++)
     {
       glVertex3d(P1[i*2], P1[i*2+1], zmax);
     }
  glEnd();

  glBegin(GL_LINES);
   for(i = 0; i <= 8; i++)
     {
       glVertex3d(P1[i*2], P1[i*2+1], zmin);
       glVertex3d(P1[i*2], P1[i*2+1], zmax);
     }
  glEnd();

  if(cylinder->closed)
    {
      if(fabs(thetamax) != 360.0)
	{
	  glBegin(GL_LINE_STRIP);
	   glVertex3d(radius, 0.0, zmin);
	   glVertex3d(0.0,    0.0, zmin);
	   glVertex3d(0.0,    0.0, zmax);
	   glVertex3d(radius, 0.0, zmax);
	  glEnd();
	  glRotated(thetamax, 0.0, 0.0, 1.0);
	  glBegin(GL_LINES);
	   glVertex3d(radius, 0.0, zmin);
	   glVertex3d(0.0,    0.0, zmin);
	   glVertex3d(radius, 0.0, zmax);
	   glVertex3d(0.0,    0.0, zmax);
	  glEnd();
	}
    }

 return AY_OK;
} /* ay_cylinder_drawcb */


int
ay_cylinder_shadecb(struct Togl *togl, ay_object *o)
{
 ay_cylinder_object *cylinder = NULL;
 GLUquadricObj *qobj = NULL;
 double thetamax, zmin, zmax, radius;
 double thetadiff, angle;
 int i;
 double P1[9*2] = {0};

  if(!o)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)o->refine;

  if(!cylinder)
    return AY_ENULL;

  radius = cylinder->radius;

  /* check, if cylinder is simple */
  if(cylinder->is_simple)
    {
      /* yes, it is */

      if(!(qobj = gluNewQuadric()))
	return AY_EOMEM;
      glTranslated(0.0, 0.0, cylinder->zmin);
      /* draw */
      gluCylinder(qobj, radius, radius, cylinder->zmax - cylinder->zmin, 8, 1);

      gluDeleteQuadric(qobj);
      
      if(cylinder->closed)
	{
	  if(!(qobj = gluNewQuadric()))
	    return AY_EOMEM;
	  gluQuadricOrientation(qobj, GLU_INSIDE);
	  gluDisk(qobj, 0.0, radius, 8, 1);
	  gluDeleteQuadric(qobj);

	  glTranslated(0.0, 0.0, -cylinder->zmin+cylinder->zmax);

	  if(!(qobj = gluNewQuadric()))
	    return AY_EOMEM;
	  gluDisk(qobj, 0.0, radius, 8, 1);
	  gluDeleteQuadric(qobj);
	}
      
      return AY_OK;
    }

  zmin = cylinder->zmin;
  zmax = cylinder->zmax;
  thetamax = cylinder->thetamax;

  thetadiff = thetamax/8;
  angle = 0;
  for(i = 0; i <= 8; i++)
    {
      P1[(i*2)] = cos(angle) * radius;
      P1[(i*2)+1] = sin(angle) * radius;
      angle += AY_D2R(thetadiff);
    }

  /* draw */
  glBegin(GL_QUAD_STRIP);
  for(i = 0; i <= 8; i++)
    {
      glNormal3d(-P1[i*2], -P1[i*2+1], 0.0);
      glVertex3d(P1[i*2], P1[i*2+1], zmin);
      glVertex3d(P1[i*2], P1[i*2+1], zmax);
    }
  glEnd();

  if(cylinder->closed)
    {
      glPushMatrix();

       glNormal3d(0.0, 1.0, 0.0);
       glBegin(GL_QUADS);
        glVertex3d(0.0,    0.0, zmin);
	glVertex3d(0.0,    0.0, zmax);
	glVertex3d(radius, 0.0, zmax);
	glVertex3d(radius, 0.0, zmin);
       glEnd();

       glNormal3d(0.0, -1.0, 0.0);
       glRotated(thetamax, 0.0, 0.0, 1.0);

       glBegin(GL_QUADS);
        glVertex3d(0.0,    0.0, zmin);
	glVertex3d(radius, 0.0, zmin);
	glVertex3d(radius, 0.0, zmax);
	glVertex3d(0.0,    0.0, zmax);
       glEnd();

      glPopMatrix();

      /* draw caps */
      glPushMatrix();

       qobj = NULL;
       if(!(qobj = gluNewQuadric()))
	 return AY_EOMEM;
       gluQuadricOrientation(qobj, GLU_INSIDE);

       glTranslated(0.0,0.0,zmin);
       glRotated(thetamax-90.0, 0.0, 0.0, 1.0);

       gluPartialDisk(qobj, 0.0, radius, 8, 1, 0.0, thetamax);

       gluDeleteQuadric(qobj);

      glPopMatrix();
	
      glPushMatrix();

       qobj = NULL;
       if(!(qobj = gluNewQuadric()))
	 return AY_EOMEM;

       glTranslated(0.0, 0.0, zmax);
       glRotated(thetamax-90.0, 0.0, 0.0, 1.0);

       gluPartialDisk(qobj, 0.0, radius, 8, 1, 0.0, thetamax);

       gluDeleteQuadric(qobj);

      glPopMatrix();

    }

 return AY_OK;
} /* ay_cylinder_shadecb */


/* Tcl -> C! */
int
ay_cylinder_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1 = "CylinderAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_cylinder_object *cylinder = NULL;
 int itemp = 0;

  if(!o)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &itemp);
  cylinder->closed = (char)itemp;

  Tcl_SetStringObj(ton,"Radius",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &cylinder->radius);

  Tcl_SetStringObj(ton,"ZMin",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &cylinder->zmin);

  Tcl_SetStringObj(ton,"ZMax",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &cylinder->zmax);

  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &cylinder->thetamax);

  if(fabs(cylinder->thetamax) == 360.0)
    {
      cylinder->is_simple = AY_TRUE;
    }
  else
    {
      cylinder->is_simple = AY_FALSE;
    }

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_cylinder_setpropcb */


/* C -> Tcl! */
int
ay_cylinder_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="CylinderAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_cylinder_object *cylinder = NULL;

  if(!o)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_NewIntObj(cylinder->closed);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Radius",-1);
  to = Tcl_NewDoubleObj(cylinder->radius);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"ZMin",-1);
  to = Tcl_NewDoubleObj(cylinder->zmin);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"ZMax",-1);
  to = Tcl_NewDoubleObj(cylinder->zmax);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_NewDoubleObj(cylinder->thetamax);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_cylinder_getpropcb */


int
ay_cylinder_readcb(FILE *fileptr, ay_object *o)
{
 ay_cylinder_object *cylinder = NULL;
 int itemp = 0;
 if(!o)
   return AY_ENULL;

  if(!(cylinder = calloc(1, sizeof(ay_cylinder_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&itemp);
  cylinder->closed = (char)itemp;
  fscanf(fileptr,"%lg\n",&cylinder->radius);
  fscanf(fileptr,"%lg\n",&cylinder->zmin);
  fscanf(fileptr,"%lg\n",&cylinder->zmax);
  fscanf(fileptr,"%lg\n",&cylinder->thetamax);

  if(fabs(cylinder->thetamax) == 360.0)
    {
      cylinder->is_simple = AY_TRUE;
    }
  else
    {
      cylinder->is_simple = AY_FALSE;
    }

  o->refine = cylinder;

 return AY_OK;
} /* ay_cylinder_readcb */


int
ay_cylinder_writecb(FILE *fileptr, ay_object *o)
{
 ay_cylinder_object *cylinder = NULL;

  if(!o)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)(o->refine);

  fprintf(fileptr, "%d\n", cylinder->closed);
  fprintf(fileptr, "%g\n", cylinder->radius);
  fprintf(fileptr, "%g\n", cylinder->zmin);
  fprintf(fileptr, "%g\n", cylinder->zmax);
  fprintf(fileptr, "%g\n", cylinder->thetamax);

 return AY_OK;
} /* ay_cylinder_writecb */


int
ay_cylinder_wribcb(char *file, ay_object *o)
{
 ay_cylinder_object *cylinder = NULL;
 RtPoint rect[4];

  if(!o)
   return AY_ENULL;

  cylinder = (ay_cylinder_object*)o->refine;


  if(!cylinder->closed)
  {
    RiCylinder((RtFloat)cylinder->radius,
	     (RtFloat)cylinder->zmin,
	     (RtFloat)cylinder->zmax,
	     (RtFloat)cylinder->thetamax,
	     NULL);
  }
  else
  {
    if(fabs(cylinder->thetamax) == 180.0)
      {
	if(!ay_current_primlevel)
	  {
	    RiSolidBegin(RI_PRIMITIVE);
	  }

	RiCylinder((RtFloat)cylinder->radius,
		   (RtFloat)cylinder->zmin,
		   (RtFloat)cylinder->zmax,
		   (RtFloat)cylinder->thetamax,
		   RI_NULL);

	rect[0][0] = (RtFloat)cylinder->radius;
	rect[0][1] = (RtFloat)0.0;
	rect[0][2] = (RtFloat)cylinder->zmin;

	rect[1][0] = (RtFloat)(-cylinder->radius);
	rect[1][1] = (RtFloat)0.0;
	rect[1][2] = (RtFloat)cylinder->zmin;

	rect[2][0] = (RtFloat)(cylinder->radius);
	rect[2][1] = (RtFloat)0.0;
	rect[2][2] = (RtFloat)cylinder->zmax;

	rect[3][0] = (RtFloat)(-cylinder->radius);
	rect[3][1] = (RtFloat)0.0;
	rect[3][2] = (RtFloat)cylinder->zmax;

	RiPatch(RI_BILINEAR, RI_P, (RtPointer)rect, RI_NULL);

	RiAttributeBegin();
 	 RiReverseOrientation();
	 RiDisk((RtFloat)cylinder->zmin, (RtFloat)cylinder->radius,
		(RtFloat)cylinder->thetamax, RI_NULL);
	RiAttributeEnd();

	RiDisk((RtFloat)cylinder->zmax, (RtFloat)cylinder->radius,
	       (RtFloat)cylinder->thetamax, RI_NULL);

	if(!ay_current_primlevel)
	  {
	    RiSolidEnd();
	  }
      }
    else if(fabs(cylinder->thetamax) == 360.0)
      {
	if(!ay_current_primlevel)
	  {
	    RiSolidBegin(RI_PRIMITIVE);
	  }

	RiCylinder((RtFloat)cylinder->radius,
		   (RtFloat)cylinder->zmin,
		   (RtFloat)cylinder->zmax,
		   (RtFloat)cylinder->thetamax,
		   RI_NULL);

	RiAttributeBegin();
 	 RiReverseOrientation();
	 RiDisk((RtFloat)cylinder->zmin, (RtFloat)cylinder->radius,
		(RtFloat)cylinder->thetamax, RI_NULL);
	RiAttributeEnd(); 

	RiDisk((RtFloat)cylinder->zmax, (RtFloat)cylinder->radius,
	       (RtFloat)cylinder->thetamax, RI_NULL);

	if(!ay_current_primlevel)
	  {
	    RiSolidEnd();
	  }
      }
    else
      {
	if(!ay_current_primlevel)
	  {
	    RiSolidBegin(RI_PRIMITIVE);
	  }

	RiCylinder((RtFloat)cylinder->radius,
		   (RtFloat)cylinder->zmin,
		   (RtFloat)cylinder->zmax,
		   (RtFloat)cylinder->thetamax,
		   RI_NULL);

	rect[0][0] = (RtFloat)cylinder->radius;
	rect[0][1] = (RtFloat)0.0;
	rect[0][2] = (RtFloat)cylinder->zmin;

	rect[1][0] = (RtFloat)0.0;
	rect[1][1] = (RtFloat)0.0;
	rect[1][2] = (RtFloat)cylinder->zmin;

	rect[2][0] = (RtFloat)(cylinder->radius);
	rect[2][1] = (RtFloat)0.0;
	rect[2][2] = (RtFloat)cylinder->zmax;

	rect[3][0] = (RtFloat)0.0;
	rect[3][1] = (RtFloat)0.0;
	rect[3][2] = (RtFloat)cylinder->zmax;


	RiPatch(RI_BILINEAR, RI_P, (RtPointer)rect, RI_NULL);

	RiTransformBegin();
	RiRotate((RtFloat)cylinder->thetamax, (RtFloat)0.0, (RtFloat)0.0,
		 (RtFloat)1.0);
	RiAttributeBegin();
	 RiReverseOrientation();
	 RiPatch(RI_BILINEAR, RI_P, (RtPointer)rect, RI_NULL);
	RiAttributeEnd();
	RiTransformEnd();

	RiAttributeBegin();
	 RiReverseOrientation();
	RiDisk((RtFloat)cylinder->zmin, (RtFloat)cylinder->radius,
	       (RtFloat)cylinder->thetamax, RI_NULL);
	RiAttributeEnd();

	RiDisk((RtFloat)cylinder->zmax, (RtFloat)cylinder->radius,
	       (RtFloat)cylinder->thetamax, RI_NULL);

	if(!ay_current_primlevel)
	  {
	    RiSolidEnd();
	  }
      } /* if */
  } /* if */

 return AY_OK;
} /* ay_cylinder_wribcb */


int
ay_cylinder_bbccb(ay_object *o, double *bbox, int *flags)
{
 double r = 0.0, zmi = 0.0, zma = 0.0;
 ay_cylinder_object *cylinder = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)o->refine; 

  r = cylinder->radius;
  zmi = cylinder->zmin;
  zma = cylinder->zmax;

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
} /* ay_cylinder_bbccb */


int
ay_cylinder_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_cylinder_name,
				    ay_cylinder_createcb,
				    ay_cylinder_deletecb,
				    ay_cylinder_copycb,
				    ay_cylinder_drawcb,
				    NULL, /* no points to edit */
				    ay_cylinder_shadecb,
				    ay_cylinder_setpropcb,
				    ay_cylinder_getpropcb,
				    NULL, /* No Picking! */
				    ay_cylinder_readcb,
				    ay_cylinder_writecb,
				    ay_cylinder_wribcb,
				    ay_cylinder_bbccb,
				    AY_IDCYLINDER);

 return ay_status;
} /* ay_cylinder_init */

