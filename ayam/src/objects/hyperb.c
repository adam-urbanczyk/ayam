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

/* hyperb.c - hyperboloid object */

static char *ay_hyperb_name = "Hyperboloid";

int
ay_hyperb_createcb(int argc, char *argv[], ay_object *o)
{
 ay_hyperboloid_object *h = NULL;
 char fname[] = "crthyperboloid";

  if(!o)
    return AY_ENULL;

  if(!(h = calloc(1, sizeof(ay_hyperboloid_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  h->closed = AY_TRUE;
  h->p1[0] = 0.0; h->p1[1] = 1.0; h->p1[2] = -0.5;
  h->p2[0] = 1.0; h->p2[1] = 0.0; h->p2[2] = 0.5;
  h->thetamax = 360.0;

  o->refine = h;

 return AY_OK;
} /* ay_hyperb_createcb */


int
ay_hyperb_deletecb(void *c)
{
 ay_hyperboloid_object *h = NULL;

  if(!c)
    return AY_ENULL;    

  h = (ay_hyperboloid_object *)(c);

  free(h);

 return AY_OK;
} /* ay_hyperb_deletecb */


int
ay_hyperb_copycb(void *src, void **dst)
{
 ay_hyperboloid_object *h = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(h = calloc(1, sizeof(ay_hyperboloid_object))))
    return AY_EOMEM; 

  memcpy(h, src, sizeof(ay_hyperboloid_object)); 

  *dst = (void *)h;

 return AY_OK;
} /* ay_hyperb_copycb */


int
ay_hyperb_drawcb(struct Togl *togl, ay_object *o)
{
 ay_hyperboloid_object *h = NULL;
 double rmi, rma, ami, ama;
 int i;
 double P1[9*2], P2[9*2];
 double thetadiff, angle;

  if(!o)
    return AY_ENULL;

  h = (ay_hyperboloid_object *)o->refine;

  if(!h)
    return AY_ENULL;

  rmi = sqrt((h->p1[0]*h->p1[0])+(h->p1[1]*h->p1[1]));
  ami = acos(h->p1[0]/rmi);
  if (h->p1[1] < 0.0)
    ami = -ami;

  rma = sqrt((h->p2[0]*h->p2[0])+(h->p2[1]*h->p2[1]));
  ama = acos(h->p2[0]/rma);
  if (h->p2[1] < 0.0)
    ama = -ama;

  thetadiff = AY_D2R(h->thetamax/8);

  angle = ami;
  for(i = 0; i <= 8; i++)
    {
      P1[(i*2)] = cos(angle) * rmi;
      P1[(i*2)+1] = sin(angle) * rmi;
      angle += thetadiff;
    }

  angle = ama;
  for(i = 0; i <= 8; i++)
    {
      P2[(i*2)] = cos(angle) * rma;
      P2[(i*2)+1] = sin(angle) * rma;
      angle += thetadiff;
    }

  /* draw */
  glBegin(GL_LINE_STRIP);
   for(i = 0; i <= 8; i++)
     {
       glVertex3d(P1[i*2], P1[i*2+1], h->p1[2]);
     }
  glEnd();

  glBegin(GL_LINE_STRIP);
   for(i = 0; i <= 8; i++)
     {
       glVertex3d(P2[i*2], P2[i*2+1], h->p2[2]);
     }
  glEnd();

  glBegin(GL_LINES);
   for(i = 0; i <= 8; i++)
     {
       glVertex3d(P1[i*2], P1[i*2+1], h->p1[2]);
       glVertex3d(P2[i*2], P2[i*2+1], h->p2[2]);
     }
  glEnd();

  if(h->closed)
    {
      if(fabs(h->thetamax) == 360.0)
	{
	  glBegin(GL_LINES);
	   glVertex3d(P1[0*2], P1[0*2+1], h->p1[2]);
	   glVertex3d(P1[4*2], P1[4*2+1], h->p1[2]);
	   glVertex3d(P1[2*2], P1[2*2+1], h->p1[2]);
	   glVertex3d(P1[6*2], P1[6*2+1], h->p1[2]);
	  glEnd();

	  glBegin(GL_LINES);
	   glVertex3d(P2[0*2], P2[0*2+1], h->p2[2]);
	   glVertex3d(P2[4*2], P2[4*2+1], h->p2[2]);
	   glVertex3d(P2[2*2], P2[2*2+1], h->p2[2]);
	   glVertex3d(P2[6*2], P2[6*2+1], h->p2[2]);
	  glEnd();
	}
      else
	{
	  glBegin(GL_LINE_STRIP);
	   glVertex3dv(h->p1);
	   glVertex3d(0.0, 0.0, h->p1[2]);
	   glVertex3d(0.0, 0.0, h->p2[2]);
	   glVertex3dv(h->p2);
	  glEnd();
	  glRotated(h->thetamax, 0.0, 0.0, 1.0);
	  glBegin(GL_LINES);
	   glVertex3dv(h->p1);
	   glVertex3d(0.0, 0.0, h->p1[2]);
	   glVertex3d(0.0, 0.0, h->p2[2]);
	   glVertex3dv(h->p2);
	  glEnd();
	}
    }


 return AY_OK;
} /* ay_hyperb_drawcb */


int
ay_hyperb_shadecb(struct Togl *togl, ay_object *o)
{
 ay_hyperboloid_object *h = NULL;
 GLUquadricObj *qobj = NULL;
 double rmi, rma, ami, ama;
 int i;
 double P1[9*2], P2[9*2];
 double thetadiff, angle;

  if(!o)
    return AY_ENULL;

  h = (ay_hyperboloid_object *)o->refine;

  if(!h)
    return AY_ENULL;

  
  rmi = sqrt((h->p1[0]*h->p1[0])+(h->p1[1]*h->p1[1]));
  ami = acos(h->p1[0]/rmi);
  if (h->p1[1] < 0.0)
    ami = -ami;

  rma = sqrt((h->p2[0]*h->p2[0])+(h->p2[1]*h->p2[1]));
  ama = acos(h->p2[0]/rma);
  if (h->p2[1] < 0.0)
    ama = -ama;

  thetadiff = AY_D2R(h->thetamax/8);

  angle = ami;
  for(i = 0; i <= 8; i++)
    {
      P1[(i*2)] = cos(angle) * rmi;
      P1[(i*2)+1] = sin(angle) * rmi;
      angle += thetadiff;
    }

  angle = ama;
  for(i = 0; i <= 8; i++)
    {
      P2[(i*2)] = cos(angle) * rma;
      P2[(i*2)+1] = sin(angle) * rma;
      angle += thetadiff;
    }

  /* draw */

  /* XXXX this is just a gross approximation of the hyperboloids shape */
  glBegin(GL_QUAD_STRIP);
   for(i = 0; i <= 8; i++)
     {
       glNormal3d(-P1[i*2], -P1[i*2+1], 0.0);
       glVertex3d(P1[i*2], P1[i*2+1], h->p1[2]);
       glNormal3d(-P2[i*2], -P2[i*2+1], 0.0);
       glVertex3d(P2[i*2], P2[i*2+1], h->p2[2]);
     }
  glEnd();

  if(h->closed)
    {
      /* caps */
      qobj = NULL;
      if(!(qobj = gluNewQuadric()))
	return AY_EOMEM;
      glPushMatrix();
       glRotated(AY_R2D(ami),0.0,0.0,1.0);
       glTranslated(0.0, 0.0, h->p1[2]);
       if(fabs(h->thetamax) == 360.0)
	 {
	   gluDisk(qobj, 0.0, rmi, 8, 1);
	 }
       else
	 {
	   glRotated(h->thetamax-90.0, 0.0, 0.0, 1.0);
	   gluPartialDisk(qobj, 0.0, rmi, 8, 1, 0.0, h->thetamax);
	 }
      glPopMatrix();
      gluDeleteQuadric(qobj);

      qobj = NULL;
      if(!(qobj = gluNewQuadric()))
	return AY_EOMEM;
      glPushMatrix();
       glRotated(AY_R2D(ama),0.0,0.0,1.0);
       glTranslated(0.0, 0.0, h->p2[2]);
       if(fabs(h->thetamax) == 360.0)
	 {
	   gluDisk(qobj, 0.0, rma, 8, 1);
	 }
       else
	 {
	   glRotated(h->thetamax-90.0, 0.0, 0.0, 1.0);
	   gluPartialDisk(qobj, 0.0, rma, 8, 1, 0.0, h->thetamax);
	 }
      glPopMatrix();
      gluDeleteQuadric(qobj);

      /* other patches */
      glBegin(GL_QUADS);
       glNormal3d(0.0, 1.0, 0.0);
       glVertex3dv(h->p1);
       glVertex3d(0.0,0.0,h->p1[2]);
       glVertex3d(0.0,0.0,h->p2[2]);
       glVertex3dv(h->p2);
      glEnd();
      glRotated(h->thetamax,0.0,0.0,1.0);
      glBegin(GL_QUADS);
       glVertex3dv(h->p1);
       glVertex3d(0.0,0.0,h->p1[2]);
       glVertex3d(0.0,0.0,h->p2[2]);
       glVertex3dv(h->p2);
      glEnd();

    }

 return AY_OK;
} /* ay_hyperb_shadecb */


/* Tcl -> C! */
int
ay_hyperb_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1 = "HyperbAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_hyperboloid_object *hyperb = NULL;
 int itemp;

  if(!o)
    return AY_ENULL;

  hyperb = (ay_hyperboloid_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &itemp);
  hyperb->closed = (char)itemp;

  Tcl_SetStringObj(ton,"P1_X",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &hyperb->p1[0]);

  Tcl_SetStringObj(ton,"P1_Y",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &hyperb->p1[1]);

  Tcl_SetStringObj(ton,"P1_Z",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &hyperb->p1[2]);


  Tcl_SetStringObj(ton,"P2_X",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &hyperb->p2[0]);

  Tcl_SetStringObj(ton,"P2_Y",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &hyperb->p2[1]);

  Tcl_SetStringObj(ton,"P2_Z",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &hyperb->p2[2]);


  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &hyperb->thetamax);


  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_hyperb_setpropcb */


/* C -> Tcl! */
int
ay_hyperb_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="HyperbAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_hyperboloid_object *hyperb = NULL;

  if(!o)
    return AY_ENULL;

  hyperb = (ay_hyperboloid_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_NewIntObj(hyperb->closed);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton,"P1_X",-1);
  to = Tcl_NewDoubleObj(hyperb->p1[0]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P1_Y",-1);
  to = Tcl_NewDoubleObj(hyperb->p1[1]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P1_Z",-1);
  to = Tcl_NewDoubleObj(hyperb->p1[2]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton,"P2_X",-1);
  to = Tcl_NewDoubleObj(hyperb->p2[0]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P2_Y",-1);
  to = Tcl_NewDoubleObj(hyperb->p2[1]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P2_Z",-1);
  to = Tcl_NewDoubleObj(hyperb->p2[2]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_NewDoubleObj(hyperb->thetamax);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_hyperb_getpropcb */


int
ay_hyperb_readcb(FILE *fileptr, ay_object *o)
{
 ay_hyperboloid_object *hyperb = NULL;
 int itemp;
  if(!o)
   return AY_ENULL;

  if(!(hyperb = calloc(1, sizeof(ay_hyperboloid_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&itemp);
  hyperb->closed = (char)itemp;

  fscanf(fileptr,"%lg %lg %lg\n", &hyperb->p1[0], &hyperb->p1[1],
	 &hyperb->p1[2]);
  fscanf(fileptr,"%lg %lg %lg\n", &hyperb->p2[0], &hyperb->p2[1],
	 &hyperb->p2[2]);
  fscanf(fileptr,"%lg\n",&hyperb->thetamax);

  o->refine = hyperb;

 return AY_OK;
} /* ay_hyperb_readcb */


int
ay_hyperb_writecb(FILE *fileptr, ay_object *o)
{
 ay_hyperboloid_object *hyperb = NULL;

  if(!o)
    return AY_ENULL;

  hyperb = (ay_hyperboloid_object *)(o->refine);

  fprintf(fileptr, "%d\n", hyperb->closed);
  fprintf(fileptr, "%g %g %g\n", hyperb->p1[0], hyperb->p1[1], hyperb->p1[2]);
  fprintf(fileptr, "%g %g %g\n", hyperb->p2[0], hyperb->p2[1], hyperb->p2[2]);
  fprintf(fileptr, "%g\n", hyperb->thetamax);

 return AY_OK;
} /* ay_hyperb_writecb */


int
ay_hyperb_wribcb(char *file, ay_object *o)
{
 ay_hyperboloid_object *hyperb = NULL;
 RtPoint p1, p2;
 RtPoint patch[4];
 RtFloat angle = (RtFloat)0.0, radius = (RtFloat)0.0;

  if(!o)
   return AY_ENULL;

  hyperb = (ay_hyperboloid_object*)o->refine;

  p1[0] = (RtFloat)hyperb->p1[0];
  p1[1] = (RtFloat)hyperb->p1[1];
  p1[2] = (RtFloat)hyperb->p1[2];
  p2[0] = (RtFloat)hyperb->p2[0];
  p2[1] = (RtFloat)hyperb->p2[1];
  p2[2] = (RtFloat)hyperb->p2[2];

  if(!hyperb->closed)
    {
      RiHyperboloid(p1, p2, (RtFloat)hyperb->thetamax, NULL);
    }
  else
    {
      RiSolidBegin(RI_PRIMITIVE);
       RiHyperboloid(p1, p2, (RtFloat)hyperb->thetamax, NULL);
       if(p1[2] == p2[2])
	 {
	   RiSolidEnd();
	   return AY_OK;
	 }

       radius = (RtFloat)sqrt(p1[0]*p1[0]+p1[1]*p1[1]);
       angle = (RtFloat)(180.0/AY_PI * acos(p1[0]/radius));
       if(p1[1] < 0.0)
	 angle = -angle;

       RiTransformBegin();
        RiRotate(angle, (RtFloat)0.0, (RtFloat)0.0, (RtFloat)1.0);
	RiDisk(p1[2], radius, (RtFloat)hyperb->thetamax, RI_NULL);
       RiTransformEnd();     

       radius = (RtFloat)sqrt(p2[0]*p2[0] + p2[1]*p2[1]);
       angle = (RtFloat)(180.0/AY_PI * acos(p2[0]/radius));
       if (p2[1] < 0.0)
	 angle = -angle;

       RiAttributeBegin();
        RiRotate(angle, (RtFloat)0.0, (RtFloat)0.0, (RtFloat)1.0);
	RiReverseOrientation();
	RiDisk(p2[2], radius, (RtFloat)hyperb->thetamax, RI_NULL);
	RiAttributeEnd();

	if(fabs(hyperb->thetamax) != 360.0)
	  {
	    patch[0][0] = p2[0]; patch[0][1] = p2[1]; patch[0][2] = p2[2];
	    patch[1][0] = (RtFloat)0.0; patch[1][1] = (RtFloat)0.0;
	    patch[1][2] = p2[2];
	    patch[2][0] = p1[0]; patch[2][1] = p1[1]; patch[2][2] = p1[2];
	    patch[3][0] = (RtFloat)0.0; patch[3][1] = (RtFloat)0.0;
	    patch[3][2] = p1[2];

	    RiPatch(RI_BILINEAR, RI_P, (RtPointer)patch, RI_NULL);
	    RiAttributeBegin();
	     RiReverseOrientation();
	     RiRotate((RtFloat)hyperb->thetamax, (RtFloat)0.0, (RtFloat)0.0,
		      (RtFloat)1.0);
	     RiPatch(RI_BILINEAR, RI_P, (RtPointer)patch, RI_NULL);
	    RiAttributeEnd();
	}
      RiSolidEnd();
    }


 return AY_OK;
} /* ay_hyperb_wribcb */


int
ay_hyperb_bbccb(ay_object *o, double *bbox, int *flags)
{
 double r = 0.0, rmi = 0.0, rma = 0.0, zmi = 0.0, zma = 0.0;
 ay_hyperboloid_object *h = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  h = (ay_hyperboloid_object *)o->refine;

  rmi = sqrt((h->p1[0]*h->p1[0])+(h->p1[2]*h->p1[2]));
  rma = sqrt((h->p2[0]*h->p2[0])+(h->p2[2]*h->p2[2]));
  zmi = h->p1[1];
  zma = h->p2[1];

  r = rmi>rma?rmi:rma;

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
} /* ay_hyperb_bbccb */


int
ay_hyperb_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_hyperb_name,
				    ay_hyperb_createcb,
				    ay_hyperb_deletecb,
				    ay_hyperb_copycb,
				    ay_hyperb_drawcb,
				    NULL, /* no points to edit */
				    ay_hyperb_shadecb,
				    ay_hyperb_setpropcb,
				    ay_hyperb_getpropcb,
				    NULL, /* No Picking! */
				    ay_hyperb_readcb,
				    ay_hyperb_writecb,
				    ay_hyperb_wribcb,
				    ay_hyperb_bbccb,
				    AY_IDHYPERBOLOID);

 return ay_status;
} /* ay_hyperb_init */

