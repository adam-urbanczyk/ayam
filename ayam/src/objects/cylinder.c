/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2005 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* cylinder.c - cylinder object */

static char *ay_cylinder_name = "Cylinder";

int ay_cylinder_notifycb(ay_object *o);

/* number of read only points */
#define AY_PCYLINDER 30


/* functions: */

/* ay_cylinder_createcb:
 *  create callback function of cylinder object
 */
int
ay_cylinder_createcb(int argc, char *argv[], ay_object *o)
{
 ay_cylinder_object *cylinder;
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


/* ay_cylinder_deletecb:
 *  delete callback function of cylinder object
 */
int
ay_cylinder_deletecb(void *c)
{
 ay_cylinder_object *cylinder;

  if(!c)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)(c);

  if(cylinder->pnts)
    free(cylinder->pnts);

  free(cylinder);

 return AY_OK;
} /* ay_cylinder_deletecb */


/* ay_cylinder_copycb:
 *  copy callback function of cylinder object
 */
int
ay_cylinder_copycb(void *src, void **dst)
{
 ay_cylinder_object *cylinder;

  if(!src || !dst)
    return AY_ENULL;

  if(!(cylinder = malloc(sizeof(ay_cylinder_object))))
    return AY_EOMEM;

  memcpy(cylinder, src, sizeof(ay_cylinder_object));

  cylinder->pnts = NULL;

  *dst = (void *)cylinder;

 return AY_OK;
} /* ay_cylinder_copycb */


/* ay_cylinder_drawcb:
 *  draw (display in an Ayam view window) callback function of cylinder object
 */
int
ay_cylinder_drawcb(struct Togl *togl, ay_object *o)
{
 ay_cylinder_object *cylinder;
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


/* ay_cylinder_shadecb:
 *  shade (display in an Ayam view window) callback function of cylinder object
 */
int
ay_cylinder_shadecb(struct Togl *togl, ay_object *o)
{
 ay_cylinder_object *cylinder;
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

      glTranslated(0.0, 0.0, cylinder->zmin);
      /* draw */
      gluCylinder(ay_gluquadobj, radius, radius,
		  cylinder->zmax - cylinder->zmin, 8, 1);

      if(cylinder->closed)
	{
	  gluQuadricOrientation(ay_gluquadobj, GLU_INSIDE);
	  gluDisk(ay_gluquadobj, 0.0, radius, 8, 1);
	  gluQuadricOrientation(ay_gluquadobj, GLU_OUTSIDE);

	  glTranslated(0.0, 0.0, -cylinder->zmin+cylinder->zmax);

	  gluDisk(ay_gluquadobj, 0.0, radius, 8, 1);
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

       gluQuadricOrientation(ay_gluquadobj, GLU_INSIDE);

       glTranslated(0.0,0.0,zmin);
       glRotated(thetamax-90.0, 0.0, 0.0, 1.0);

       gluPartialDisk(ay_gluquadobj, 0.0, radius, 8, 1, 0.0, thetamax);

       gluQuadricOrientation(ay_gluquadobj, GLU_OUTSIDE);

      glPopMatrix();

      glPushMatrix();

       glTranslated(0.0, 0.0, zmax);
       glRotated(thetamax-90.0, 0.0, 0.0, 1.0);

       gluPartialDisk(ay_gluquadobj, 0.0, radius, 8, 1, 0.0, thetamax);

      glPopMatrix();
    } /* if */

 return AY_OK;
} /* ay_cylinder_shadecb */


/* ay_cylinder_drawhcb:
 *  draw handles (in an Ayam view window) callback function of cylinder object
 */
int
ay_cylinder_drawhcb(struct Togl *togl, ay_object *o)
{
 int i;
 double *pnts;
 ay_cylinder_object *cylinder;

  if(!o)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)o->refine;

  if(!cylinder)
    return AY_ENULL;

  if(!cylinder->pnts)
    {
      if(!(pnts = calloc(AY_PCYLINDER*3, sizeof(double))))
	{
	  return AY_EOMEM;
	}
      cylinder->pnts = pnts;
      (void)ay_cylinder_notifycb(o);
    }
  else
    {
      pnts = cylinder->pnts;
    }

  glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
	    (GLfloat)ay_prefs.obb);

  glBegin(GL_POINTS);
   for(i = 0; i < AY_PCYLINDER; i++)
     {
       glVertex3dv((GLdouble *)pnts);
       pnts += 3;
     }
  glEnd();

  glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
	    (GLfloat)ay_prefs.seb);

 return AY_OK;
} /* ay_cylinder_drawhcb */


/* ay_cylinder_getpntcb:
 *  get point (editing and selection) callback function of cylinder object
 */
int
ay_cylinder_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_cylinder_object *cylinder;

  if(!o)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)o->refine;

  if(!cylinder)
    return AY_ENULL;

  if(!cylinder->pnts)
    {
      if(!(cylinder->pnts = calloc(AY_PCYLINDER*3, sizeof(double))))
	return AY_EOMEM;
      (void)ay_cylinder_notifycb(o);
    }

 return ay_selp_getpnts(mode, o, p, pe, 1, AY_PCYLINDER, 3, cylinder->pnts);
} /* ay_cylinder_getpntcb */


/* ay_cylinder_setpropcb:
 *  set property (from Tcl to C context) callback function of cylinder object
 */
int
ay_cylinder_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "CylinderAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_cylinder_object *cylinder;
 int itemp = 0;

  if(!interp || !o)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)o->refine;

  if(!cylinder)
    return AY_ENULL;

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

  (void)ay_cylinder_notifycb(o);
  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_cylinder_setpropcb */


/* ay_cylinder_getpropcb:
 *  get property (from C to Tcl context) callback function of cylinder object
 */
int
ay_cylinder_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="CylinderAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_cylinder_object *cylinder;

  if(!interp || !o)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)(o->refine);

  if(!cylinder)
    return AY_ENULL;

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


/* ay_cylinder_readcb:
 *  read (from scene file) callback function of cylinder object
 */
int
ay_cylinder_readcb(FILE *fileptr, ay_object *o)
{
 ay_cylinder_object *cylinder;
 int itemp = 0;

  if(!fileptr || !o)
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


/* ay_cylinder_writecb:
 *  write (to scene file) callback function of cylinder object
 */
int
ay_cylinder_writecb(FILE *fileptr, ay_object *o)
{
 ay_cylinder_object *cylinder;

  if(!fileptr || !o)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)(o->refine);

  if(!cylinder)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", (int)cylinder->closed);
  fprintf(fileptr, "%g\n", cylinder->radius);
  fprintf(fileptr, "%g\n", cylinder->zmin);
  fprintf(fileptr, "%g\n", cylinder->zmax);
  fprintf(fileptr, "%g\n", cylinder->thetamax);

 return AY_OK;
} /* ay_cylinder_writecb */


/* ay_cylinder_wribcb:
 *  RIB export callback function of cylinder object
 */
int
ay_cylinder_wribcb(char *file, ay_object *o)
{
 ay_cylinder_object *cylinder;
 RtPoint rect[4];

  if(!o)
   return AY_ENULL;

  cylinder = (ay_cylinder_object*)o->refine;

  if(!cylinder)
    return AY_ENULL;

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

      }
    else if(fabs(cylinder->thetamax) == 360.0)
      {
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

      }
    else
      {
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

      } /* if */
  } /* if */

 return AY_OK;
} /* ay_cylinder_wribcb */


/* ay_cylinder_bbccb:
 *  bounding box calculation callback function of cylinder object
 */
int
ay_cylinder_bbccb(ay_object *o, double *bbox, int *flags)
{
 double r = 0.0, zmi = 0.0, zma = 0.0;
 ay_cylinder_object *cylinder;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)o->refine;

  if(!cylinder)
    return AY_ENULL;

  if(!cylinder->is_simple)
    {
      if(!cylinder->pnts)
	{
	  if(!(cylinder->pnts = calloc(AY_PCYLINDER*3, sizeof(double))))
	    { return AY_EOMEM; }
	  (void)ay_cylinder_notifycb(o);
	}

      return ay_bbc_fromarr(cylinder->pnts, AY_PCYLINDER, 3, bbox);
    }

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


/* ay_cylinder_notifycb:
 *  notification callback function of cylinder object
 */
int
ay_cylinder_notifycb(ay_object *o)
{
 ay_cylinder_object *cylinder;
 double *pnts;
 double radius, w;
 int i, a;
 double thetadiff, angle, hh;

  if(!o)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)o->refine;

  if(!cylinder)
    return AY_ENULL;

  if(cylinder->pnts)
    {
      radius = cylinder->radius;
      w = (sqrt(2.0)*0.5);

      pnts = cylinder->pnts;

      hh = cylinder->zmin + ((cylinder->zmax - cylinder->zmin) / 2.0);

      pnts[2] = cylinder->zmin;
      pnts[5] = hh;
      pnts[8] = cylinder->zmax;

      if(cylinder->is_simple)
	{
	  /* lower ring */
	  pnts[9] = radius;
	  pnts[10] = 0.0;

	  pnts[12] = radius*w;
	  pnts[13] = -radius*w;

	  pnts[15] = 0.0;
	  pnts[16] = -radius;

	  pnts[18] = -radius*w;
	  pnts[19] = -radius*w;

	  pnts[21] = -radius;
	  pnts[22] = 0.0;

	  pnts[24] = -radius*w;
	  pnts[25] = radius*w;

	  pnts[27] = 0.0;
	  pnts[28] = radius;

	  pnts[30] = radius*w;
	  pnts[31] = radius*w;

	  memcpy(&(pnts[33]),&(pnts[9]),3*sizeof(double));
	}
      else
	{
	  thetadiff = AY_D2R(cylinder->thetamax/8);
	  angle = 0.0;
	  a = 9;
	  for(i = 0; i <= 8; i++)
	    {
	      pnts[a] = cos(angle)*radius;
	      pnts[a+1] = sin(angle)*radius;

	      a += 3;
	      angle += thetadiff;
	    } /* for */
	} /* if */

      /* middle ring */
      memcpy(&(pnts[36]),&(pnts[9]),9*3*sizeof(double));

      /* upper ring */
      memcpy(&(pnts[63]),&(pnts[9]),9*3*sizeof(double));

      /* set heights */
      /* lower ring */
      a = 11;
      for(i = 0; i <= 8; i++)
	{
	  pnts[a] = cylinder->zmin;
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
	  pnts[a] = cylinder->zmax;
	  a += 3;
	} /* for */
    } /* if */

 return AY_OK;
} /* ay_cylinder_notifycb */


/* ay_cylinder_providecb:
 *  provide callback function of cylinder object
 */
int
ay_cylinder_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 int stride = 4, i, j, height;
 double *cv = NULL, *vk = NULL, *controlv = NULL;
 ay_cylinder_object *cylinder = NULL;
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

  cylinder = (ay_cylinder_object *) o->refine;

  if(!cylinder)
    return AY_ENULL;

  if(type == AY_IDNPATCH)
    {
      if(cylinder->thetamax < 0.0)
	{
	  ay_status = ay_nb_CreateNurbsCircleArc(cylinder->radius,
						 cylinder->thetamax, 0.0,
						 &height, &vk, &cv);
	}
      else
	{
	  ay_status = ay_nb_CreateNurbsCircleArc(cylinder->radius,
						 0.0, cylinder->thetamax,
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
      j = 2;
      for(i = 0; i < height; i++)
	{
	  controlv[j] = cylinder->zmin;
	  j += stride;
	}
      memcpy(&(controlv[j-2]), cv, height*stride*sizeof(double));
      for(i = 0; i < height; i++)
	{
	  controlv[j] = cylinder->zmax;
	  j += stride;
	}

      if((ay_status = ay_npt_create(2, 3, 2, height, AY_KTBEZIER, AY_KTCUSTOM,
				    controlv, NULL, vk, &np)))
	goto cleanup;

      if((ay_status = ay_npt_createnpatchobject(&new)))
	goto cleanup;

      ay_trafo_copy(o, new);
      new->down = ay_endlevel;
      new->refine = np;

      if(cylinder->closed)
	{
	  /* create caps */
	  n = &(new->next);
	  ay_object_defaults(&d);
	  ay_trafo_copy(o, &d);
	  d.type = AY_IDDISK;
	  d.refine = &disk;
	  disk.radius = cylinder->radius;
	  disk.height = cylinder->zmin;
	  disk.thetamax = cylinder->thetamax;
	  ay_provide_object(&d, AY_IDNPATCH, n);
	  if(*n)
	    {
	      n = &((*n)->next);
	      disk.height = cylinder->zmax;
	      ay_provide_object(&d, AY_IDNPATCH, n);
	      if(*n)
		{
		  n = &((*n)->next);
		} /* if */
	    } /* if */

	  if(fabs(cylinder->thetamax) != 360.0)
	    {
	      ay_object_defaults(&d);
	      ay_trafo_copy(o, &d);
	      d.type = AY_IDBPATCH;
	      d.refine = &bpatch;
	      memcpy(bpatch.p1, &(controlv[0]), 3*sizeof(double));
	      memcpy(bpatch.p2, &(controlv[height*stride]), 3*sizeof(double));
	      bpatch.p3[2] = cylinder->zmax;
	      bpatch.p4[2] = cylinder->zmin;
	      ay_provide_object(&d, AY_IDNPATCH, n);
	      if(*n)
		{
		  n = &((*n)->next);
		  memcpy(bpatch.p1, &(controlv[height*stride-stride]),
			 3*sizeof(double));
		  memcpy(bpatch.p2, &(controlv[height*2*stride-stride]),
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
} /* ay_cylinder_providecb */


/* ay_cylinder_convertcb:
 *  convert callback function of cylinder object
 */
int
ay_cylinder_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_object *new = NULL, *t;
 ay_cylinder_object *cylinder = NULL;

  if(!o)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *) o->refine;

  if(!cylinder)
    return AY_ENULL;

  /* first, create new object(s) */

  if(cylinder->closed)
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

      ay_status = ay_cylinder_providecb(o, AY_IDNPATCH, &new->down);
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
      ay_status = ay_cylinder_providecb(o, AY_IDNPATCH, &new);
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
} /* ay_cylinder_convertcb */


/* ay_cylinder_init:
 *  initialize the cylinder object module
 */
int
ay_cylinder_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_cylinder_name,
				    ay_cylinder_createcb,
				    ay_cylinder_deletecb,
				    ay_cylinder_copycb,
				    ay_cylinder_drawcb,
				    ay_cylinder_drawhcb,
				    ay_cylinder_shadecb,
				    ay_cylinder_setpropcb,
				    ay_cylinder_getpropcb,
				    ay_cylinder_getpntcb,
				    ay_cylinder_readcb,
				    ay_cylinder_writecb,
				    ay_cylinder_wribcb,
				    ay_cylinder_bbccb,
				    AY_IDCYLINDER);

  ay_status += ay_notify_register(ay_cylinder_notifycb, AY_IDCYLINDER);

  ay_status += ay_convert_register(ay_cylinder_convertcb, AY_IDCYLINDER);

  ay_status += ay_provide_register(ay_cylinder_providecb, AY_IDCYLINDER);

 return ay_status;
} /* ay_cylinder_init */

