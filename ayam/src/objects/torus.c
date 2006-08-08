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
    } /* for */

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

	} /* for */
      glEnd();
      angle += AY_D2R(thetadiff);
    } /* for */

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
	  glNormal3d(0.0, -1.0, 0.0);
	  glBegin(GL_TRIANGLE_FAN);
	   glVertex3d(mar, 0.0, 0.0);
	   for(i = 0; i <= 8; i++)
	     {
	       glVertex3d(mar+P1[i*2], 0.0, P1[i*2+1]);
	     }
	  glEnd();

	  glNormal3d(0.0, 1.0, 0.0);
	  glRotated(thetamax,0.0,0.0,1.0);

	  glBegin(GL_TRIANGLE_FAN);
	   glVertex3d(mar, 0.0, 0.0);
	   for(i = 8; i >= 0; i--)
	     {
	       glVertex3d(mar+P1[i*2], 0.0, P1[i*2+1]);
	     }
	  glEnd();

	} /* if */
    } /* if */


 return AY_OK;
} /* ay_torus_shadecb */


/* Tcl -> C! */
int
ay_torus_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
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

  o->modified = AY_TRUE;
  ay_status = ay_notify_parent();

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
      if(!ay_current_primlevel)
	{
	  RiSolidBegin(RI_PRIMITIVE);
	}

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
      if(!ay_current_primlevel)
	{
	  RiSolidEnd();
	}
    } /* if */

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
ay_torus_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 int i, j, k, height, stride = 4;
 double *cv = NULL, *cv2 = NULL, *kn = NULL;
 double majorrad, minorrad, phimax, phimin, thetamax;
 double xaxis[3] = {1.0, 0.0, 0.0}, yaxis[3] = {0.0, 1.0, 0.0};
 double zaxis[3] = {0.0, 0.0, 1.0}, quat[4] = {0}, m[16] = {0};
 ay_torus_object *torus = NULL;
 ay_disk_object disk = {0};
 ay_nurbpatch_object *np = NULL;
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

  torus = (ay_torus_object *) o->refine;

  if(type == AY_IDNPATCH)
    {

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

      majorrad = torus->majorrad;
      minorrad = torus->minorrad;
      phimin = torus->phimin;
      phimax = torus->phimax;
      thetamax = torus->thetamax;

      ay_status = ay_nb_CreateNurbsCircleArc(minorrad,
					     phimin, phimax,
					     &height, &kn, &cv);

      if(ay_status)
	goto cleanup;

      if(!(newc = calloc(1, sizeof(ay_object))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      newc->type = AY_IDNCURVE;
      ay_status = ay_object_defaults(newc);
      newc->movx += majorrad;
      ay_status = ay_nct_create(3, height, AY_KTCUSTOM, cv, kn,
				(ay_nurbcurve_object **)&(newc->refine));

      ay_status = ay_npt_revolve(newc, -thetamax, 0, 0,
				 (ay_nurbpatch_object **)&(new->refine));


      ay_quat_axistoquat(xaxis, -AY_D2R(90.0), quat);
      new->rotx += 90.0;
      ay_quat_add(new->quat, quat, new->quat);
      ay_trafo_add(o, new);

      kn = NULL;
      cv = NULL;

      /* create caps */
      if(torus->closed)
	{
	  n = &(new->next);

	  if(fabs(thetamax) != 360.0)
	    {
	      ay_object_defaults(&d);
	      d.type = AY_IDDISK;
	      d.refine = &disk;
	      disk.radius = minorrad;
	      disk.thetamax = phimax-phimin;
	      ay_provide_object(&d, AY_IDNPATCH, n);
	      if(*n)
		{
		  ay_quat_axistoquat(yaxis, AY_D2R(phimin), quat);
		  (*n)->roty -= phimin;
		  ay_quat_add((*n)->quat, quat, (*n)->quat);
		  ay_quat_axistoquat(xaxis, -AY_D2R(90.0), quat);
		  (*n)->rotx += 90.0;
		  ay_quat_add((*n)->quat, quat, (*n)->quat);
		  (*n)->movx += majorrad;
		  /* copy object before completing trafos, so that
		     we may transform the copy further */
		  ay_status = ay_object_copy(*n, &((*n)->next));
		  ay_trafo_add(o, *n);
		  n = &((*n)->next);
		  if(*n)
		    {
		      np = (ay_nurbpatch_object *)(*n)->refine;
		      cv = np->controlv;
		      ay_trafo_creatematrix(*n, m);
		      j = 0;
		      for(i = 0; i < np->width*np->height; i++)
			{
			  ay_trafo_apply4(&(cv[j]), m);
			  j += stride;
			}
		      ay_trafo_defaults(*n);
		      ay_quat_axistoquat(zaxis, -AY_D2R(thetamax), quat);
		      (*n)->rotz += thetamax;
		      ay_quat_add((*n)->quat, quat, (*n)->quat);

		      ay_trafo_add(o, *n);
		      n = &((*n)->next);

		      np = NULL;
		      cv = NULL;
		    } /* if */
		}  /* if */
	    } /* if */

	  if(fabs(phimax-phimin) < 360.0)
	    {
	      np = (ay_nurbpatch_object *)new->refine;
	      if(!(cv2 = calloc(2*np->height*stride, sizeof(double))))
		{
		  ay_status = AY_EOMEM;
		  np = NULL;
		  goto cleanup;
		}
	      cv = np->controlv;
	      /* copy from torus */
	      memcpy(cv2, cv, np->height*stride*sizeof(double));
	      cv = NULL;
	      if(torus->thetamax < 0.0)
		{
		  ay_status = ay_nb_CreateNurbsCircleArc(majorrad,
							 0, thetamax,
							 &height, &kn, &cv);
		}
	      else
		{
		  ay_status = ay_nb_CreateNurbsCircleArc(majorrad,
							 thetamax, 0,
							 &height, &kn, &cv);
		}
	      if(ay_status)
		goto cleanup;
	      j = height*stride; k = 0;
	      for(i = 0; i < height; i++)
		{
		  /* copy x */
		  cv2[j] = cv[k];
		  /* copy y to z (== rotate around x by 90 degrees) */
		  cv2[j+2] = cv[k+1];
		  /* copy weight */
		  cv2[j+3] = cv[k+3];
		  j += stride; k += stride;
		}

	      if(!(newp = calloc(1, sizeof(ay_object))))
		{
		  ay_status = AY_EOMEM;
		  goto cleanup;
		}

	      ay_object_defaults(newp);
	      newp->type = AY_IDNPATCH;
	      newp->inherit_trafos = AY_FALSE;
	      newp->parent = AY_TRUE;
	      newp->hide_children = AY_TRUE;

	      if((ay_status = ay_object_crtendlevel(&(newp->down))))
		goto cleanup;

	      ay_trafo_copy(new, newp);

	      if((ay_status = ay_npt_create(2, 3, 2, height,
				     AY_KTNURB, AY_KTCUSTOM,
				     cv2, NULL, kn,
				     (ay_nurbpatch_object**)&(newp->refine))))
		goto cleanup;

	      kn = NULL; cv2 = NULL;

	      *n = newp;
	      n = &((*n)->next);

	      if((ay_status = ay_object_copy(newp, n)))
		goto cleanup;

	      newp = NULL;

	      if(*n)
		{
		  np = (ay_nurbpatch_object *)((*n)->refine);
		  cv2 = np->controlv;
		  np = (ay_nurbpatch_object *)new->refine;
		  cv = np->controlv;
		  j = 0; k = (np->width*np->height-np->height)*stride;
		  memcpy(&(cv2[j]), &(cv[k]), np->height*stride*
			 sizeof(double));
		} /* if */

	      np = NULL;
	      cv = NULL;
	      cv2 = NULL;
	    } /* if */
	} /* if */

      /* copy eventually present TP tags */
      ay_npt_copytptag(o, new);

      /* return result */
      *result = new;
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
      if(new->down)
	ay_object_delete(o->down);
      free(new);
    }

  if(newc)
    {
      ay_object_delete(newc);
    }

  if(newp)
    {
      ay_object_delete(newp);
    }

 return ay_status;
} /* ay_torus_providecb */


int
ay_torus_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_object *new = NULL, *t;
 ay_torus_object *torus = NULL;

  if(!o)
    return AY_ENULL;

  torus = (ay_torus_object *) o->refine;

  /* first, create new object(s) */

  if(torus->closed)
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

      ay_status = ay_torus_providecb(o, AY_IDNPATCH, &new->down);
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
      ay_status = ay_torus_providecb(o, AY_IDNPATCH, &new);
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
} /* ay_torus_convertcb */


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

  ay_status = ay_convert_register(ay_torus_convertcb, AY_IDTORUS);

  ay_status = ay_provide_register(ay_torus_providecb, AY_IDTORUS);

 return ay_status;
} /* ay_torus_init */

