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
  if(h->p1[1] < 0.0)
    ami = -ami;

  rma = sqrt((h->p2[0]*h->p2[0])+(h->p2[1]*h->p2[1]));
  ama = acos(h->p2[0]/rma);
  if(h->p2[1] < 0.0)
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
   for(i = 8; i >= 0; i--)
     {
       glNormal3d(P1[i*2], P1[i*2+1], 0.0);
       glVertex3d(P1[i*2], P1[i*2+1], h->p1[2]);
       glNormal3d(P2[i*2], P2[i*2+1], 0.0);
       glVertex3d(P2[i*2], P2[i*2+1], h->p2[2]);
     }
  glEnd();

  if(h->closed)
    {
      /* caps */
      qobj = NULL;
      if(!(qobj = gluNewQuadric()))
	return AY_EOMEM;
      gluQuadricOrientation(qobj, GLU_INSIDE);
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
 int ay_status = AY_OK;
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

  o->modified = AY_TRUE;
  ay_status = ay_notify_parent();

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
      if(!ay_current_primlevel)
	{
	  RiSolidBegin(RI_PRIMITIVE);
	}

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
       if(!ay_current_primlevel)
	 {
	   RiSolidEnd();
	 }
    } /* if */


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
ay_hyperboloid_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 int stride = 4, i, j, height;
 double rmin = 0.0, rmax = 0.0, ths = 0.0, the = 0.0;
 double *cv = NULL, *vk = NULL, *controlv = NULL;
 double zaxis[3]={0.0,0.0,1.0};
 double quat[4];
 ay_hyperboloid_object *hyperboloid = NULL, *h = NULL;
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

  hyperboloid = (ay_hyperboloid_object *) o->refine;
  h = hyperboloid;

  if(type == AY_IDNPATCH)
    {
      if(fabs((h->p1[0]*h->p1[0])+(h->p1[1]*h->p1[1]))>AY_EPSILON)
	rmin = sqrt((h->p1[0]*h->p1[0])+(h->p1[1]*h->p1[1]));
      ths = AY_R2D(atan(h->p1[1]/h->p1[0]));
      the = ths + hyperboloid->thetamax;

      if(hyperboloid->thetamax < 0.0)
	{
	  ay_status = ay_nb_CreateNurbsCircleArc(rmin, the, ths,
						 &height, &vk, &cv);
	}
      else
	{
	  ay_status = ay_nb_CreateNurbsCircleArc(rmin, ths, the,
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
	  controlv[j] = h->p1[2] * controlv[j+1];
	  j += stride;
	}
      free(cv);
      cv = NULL;
      if(fabs((h->p2[0]*h->p2[0])+(h->p2[1]*h->p2[1]))>AY_EPSILON)
	rmax = sqrt((h->p2[0]*h->p2[0])+(h->p2[1]*h->p2[1]));
      ths = AY_R2D(atan(h->p2[1]/h->p2[0]));
      the = ths + hyperboloid->thetamax;

      if(hyperboloid->thetamax < 0.0)
	{
	  ay_status = ay_nb_CreateNurbsCircleArc(rmax, the, ths,
						 &height, &vk, &cv);
	}
      else
	{
	  ay_status = ay_nb_CreateNurbsCircleArc(rmax, ths, the,
						 &height, &vk, &cv);
	} /* if */

      if(ay_status)
	return ay_status;

      memcpy(&(controlv[height*stride]), cv, height*stride*sizeof(double));
      j = height*stride+2;
      for(i = 0; i < height; i++)
	{
	  controlv[j] = h->p2[2] * controlv[j+1];
	  j += stride;
	}


      ay_status = ay_npt_create(2, 3, 2, height, AY_KTBEZIER, AY_KTCUSTOM,
				controlv, NULL, vk, &np);

      if(ay_status)
	goto cleanup;

      if(!(new = calloc(1, sizeof(ay_object))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      ay_object_defaults(new);
      new->type = AY_IDNPATCH;
      new->inherit_trafos = AY_FALSE;
      new->parent = AY_TRUE;
      new->hide_children = AY_TRUE;

      ay_status = ay_object_crtendlevel(&(new->down));
      if(ay_status)
	goto cleanup;

      ay_trafo_copy(o, new);
      new->refine = np;

      if(hyperboloid->closed)
	{
	  /* create caps */
	  n = &(new->next);
	  ay_object_defaults(&d);
	  d.type = AY_IDDISK;
	  d.refine = &disk;
	  disk.radius = rmin;
	  disk.height = h->p1[2];
	  disk.thetamax = hyperboloid->thetamax;
	  ths = -AY_R2D(atan(h->p1[1]/h->p1[0]));
	  d.rotz = ths;
	  ay_quat_axistoquat(zaxis, AY_D2R(ths), quat);
	  ay_quat_add(quat, d.quat, d.quat);
	  if(fabs(rmin) > AY_EPSILON)
	    ay_provide_object(&d, AY_IDNPATCH, n);
	  if(*n)
	    {
	      ay_trafo_add(o, *n);
	      n = &((*n)->next);
	    }
	  disk.radius = rmax;
	  disk.height = h->p2[2];
	  ths = -AY_R2D(atan(h->p2[1]/h->p2[0]));
	  d.rotz = ths;
	  d.quat[0] = 0.0;
	  d.quat[1] = 0.0;
	  d.quat[2] = 0.0;
	  d.quat[3] = 1.0;
	  ay_quat_axistoquat(zaxis, AY_D2R(ths), quat);
	  ay_quat_add(quat, d.quat, d.quat);
	  if(fabs(rmax) > AY_EPSILON)
	    ay_provide_object(&d, AY_IDNPATCH, n);
	  if(*n)
	    {
	      ay_trafo_add(o, *n);
	      n = &((*n)->next);
	    } /* if */

	  if(fabs(hyperboloid->thetamax) != 360.0)
	    {
	      ay_object_defaults(&d);
	      d.type = AY_IDBPATCH;
	      d.refine = &bpatch;
	      memcpy(bpatch.p1, &(controlv[0]), 3*sizeof(double));
	      memcpy(bpatch.p2, &(controlv[height*stride]), 3*sizeof(double));
	      bpatch.p3[2] = h->p2[2];
	      bpatch.p4[2] = h->p1[2];
	      ay_provide_object(&d, AY_IDNPATCH, n);
	      if(*n)
		{
		  ay_trafo_copy(o, *n);
		  n = &((*n)->next);
		  memcpy(bpatch.p1, &(controlv[height*stride-stride]),
			 3*sizeof(double));
		  memcpy(bpatch.p2, &(controlv[height*2*stride-stride]),
			 3*sizeof(double));
		  ay_provide_object(&d, AY_IDNPATCH, n);
		  if(*n)
		    {
		      ay_trafo_copy(o, *n);
		    } /* if */
		} /* if */
	    } /* if */
	} /* if */

      /* copy eventually present TP tags */
      ay_npt_copytptag(o, new);

      /* return result */
      *result = new;

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
      ay_object_deletemulti(new);
    }

 return ay_status;
} /* ay_hyperboloid_providecb */


int
ay_hyperboloid_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_object *new = NULL, *t;
 ay_hyperboloid_object *hyperboloid = NULL;

  if(!o)
    return AY_ENULL;

  hyperboloid = (ay_hyperboloid_object *) o->refine;

  /* first, create new object(s) */

  if(hyperboloid->closed)
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

      ay_status = ay_hyperboloid_providecb(o, AY_IDNPATCH, &new->down);
      if(ay_status)
	{ free(new->refine); free(new); return ay_status; }

      t = new->down;
      while(t->next)
	{
	  t = t->next;
	}
      ay_status = ay_object_crtendlevel(&(t->next));
    }
  else
    {
      ay_status = ay_hyperboloid_providecb(o, AY_IDNPATCH, &new);
    }


  /* second, link new object(s), or replace old object with it/them */

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
} /* ay_hyperboloid_convertcb */


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

  ay_status = ay_convert_register(ay_hyperboloid_convertcb, AY_IDHYPERBOLOID);

  ay_status = ay_provide_register(ay_hyperboloid_providecb, AY_IDHYPERBOLOID);

 return ay_status;
} /* ay_hyperb_init */

