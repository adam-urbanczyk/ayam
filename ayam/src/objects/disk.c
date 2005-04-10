/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2005 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* disk.c - disk object */

static char *ay_disk_name = "Disk";

int
ay_disk_createcb(int argc, char *argv[], ay_object *o)
{
 ay_disk_object *disk = NULL;
 char fname[] = "crtdisk";

  if(!o)
    return AY_ENULL;

  if(!(disk = calloc(1, sizeof(ay_disk_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  disk->is_simple = AY_TRUE;
  disk->radius = 1.0;
  disk->height = 0.0;
  disk->thetamax = 360.0;

  o->refine = disk;

 return AY_OK;
} /* ay_disk_createcb */


int
ay_disk_deletecb(void *c)
{
 ay_disk_object *disk = NULL;

  if(!c)
    return AY_ENULL;

  disk = (ay_disk_object *)(c);

  free(disk);

 return AY_OK;
} /* ay_disk_deletecb */


int
ay_disk_copycb(void *src, void **dst)
{
 ay_disk_object *disk = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(disk = calloc(1, sizeof(ay_disk_object))))
    return AY_EOMEM;

  memcpy(disk, src, sizeof(ay_disk_object));

  *dst = (void *)disk;

 return AY_OK;
} /* ay_disk_copycb */


int
ay_disk_drawcb(struct Togl *togl, ay_object *o)
{
 ay_disk_object *disk = NULL;
 GLdouble radius = 0.0;
 GLdouble w = 0.0;
 GLdouble height = 0.0;
 int j;
 double thetadiff, angle;

  if(!o)
    return AY_ENULL;

  disk = (ay_disk_object *)o->refine;

  if(!disk)
    return AY_ENULL;

  radius = disk->radius;
  height = disk->height;

  /* check, if disk is simple */
  if(disk->is_simple)
    {
      /* yes, it is */

      w = (GLdouble)(sqrt(2.0)*0.5);

      /* draw */
      /* xy-plane-ring */
      glBegin(GL_LINE_LOOP);
       glVertex3d(radius,0.0,height);
       glVertex3d(radius*w,-radius*w,height);
       glVertex3d(0.0,-radius,height);
       glVertex3d(-radius*w,-radius*w,height);
       glVertex3d(-radius,0.0,height);
       glVertex3d(-radius*w,radius*w,height);
       glVertex3d(0.0,radius,height);
       glVertex3d(radius*w,radius*w,height);
      glEnd();
      /* cross */
      glBegin(GL_LINES);
       glVertex3d(-radius,0.0,height);
       glVertex3d(radius,0.0,height);
       glVertex3d(0.0,-radius,height);
       glVertex3d(0.0,radius,height);
      glEnd();

      return AY_OK;
    }


  thetadiff = AY_D2R(disk->thetamax/8);

  angle = 0.0;
  glBegin(GL_LINE_LOOP);
   glVertex3d(0.0, 0.0, height);
   for(j = 0; j <= 8; j++)
     {
       glVertex3d(cos(angle)*radius, sin(angle)*radius, height);
       angle += thetadiff;
     }
  glEnd();

 return AY_OK;
} /* ay_disk_drawcb */


int
ay_disk_shadecb(struct Togl *togl, ay_object *o)
{
 ay_disk_object *disk = NULL;
 GLUquadricObj *qobj = NULL;
 double radius, thetamax;

  if(!o)
    return AY_ENULL;

  disk = (ay_disk_object *)o->refine;

  if(!disk)
    return AY_ENULL;

  radius = disk->radius;
  thetamax = disk->thetamax;

  if(!(qobj = gluNewQuadric()))
    return AY_EOMEM;

  glTranslated(0.0, 0.0, (GLdouble)disk->height);

  /* check, if disk is simple */
  if(disk->is_simple)
    {
      /* yes, it is */
      gluDisk(qobj, 0.0, radius, 8, 1);
    }
  else
    {
      glRotated(thetamax-90.0, 0.0, 0.0, 1.0);
      gluPartialDisk(qobj, 0.0, radius, 8, 1, 0.0, thetamax);
    }

  gluDeleteQuadric(qobj);

 return AY_OK;
} /* ay_disk_shadecb */


/* Tcl -> C! */
int
ay_disk_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1 = "DiskAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_disk_object *disk = NULL;

  if(!o)
    return AY_ENULL;

  disk = (ay_disk_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Radius",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &disk->radius);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &disk->height);

  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &disk->thetamax);

  if(fabs(disk->thetamax) == 360.0)
    {
      disk->is_simple = AY_TRUE;
    }
  else
    {
      disk->is_simple = AY_FALSE;
    }

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_disk_setpropcb */


/* C -> Tcl! */
int
ay_disk_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="DiskAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_disk_object *disk = NULL;

  if(!o)
    return AY_ENULL;

  disk = (ay_disk_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Radius",-1);
  to = Tcl_NewDoubleObj(disk->radius);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_NewDoubleObj(disk->height);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"ThetaMax",-1);
  to = Tcl_NewDoubleObj(disk->thetamax);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_disk_getpropcb */


int
ay_disk_readcb(FILE *fileptr, ay_object *o)
{
 ay_disk_object *disk = NULL;

  if(!o)
   return AY_ENULL;

  if(!(disk = calloc(1, sizeof(ay_disk_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%lg\n",&disk->radius);
  fscanf(fileptr,"%lg\n",&disk->height);
  fscanf(fileptr,"%lg\n",&disk->thetamax);

  if(fabs(disk->thetamax) == 360.0)
    {
      disk->is_simple = AY_TRUE;
    }
  else
    {
      disk->is_simple = AY_FALSE;
    }

  o->refine = disk;

 return AY_OK;
} /* ay_disk_readcb */


int
ay_disk_writecb(FILE *fileptr, ay_object *o)
{
 ay_disk_object *disk = NULL;

  if(!o)
    return AY_ENULL;

  disk = (ay_disk_object *)(o->refine);

  fprintf(fileptr, "%g\n", disk->radius);
  fprintf(fileptr, "%g\n", disk->height);
  fprintf(fileptr, "%g\n", disk->thetamax);

 return AY_OK;
} /* ay_disk_writecb */


int
ay_disk_wribcb(char *file, ay_object *o)
{
 ay_disk_object *disk = NULL;

  if(!o)
   return AY_ENULL;

  disk = (ay_disk_object*)o->refine;

  RiDisk((RtFloat)disk->height,
	 (RtFloat)disk->radius,
	 (RtFloat)disk->thetamax,
	 NULL);

 return AY_OK;
} /* ay_disk_wribcb */


int
ay_disk_bbccb(ay_object *o, double *bbox, int *flags)
{
 double r = 0.0, h = 0.0;
 ay_disk_object *disk = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  disk = (ay_disk_object *)o->refine;

  r = disk->radius;
  h = disk->height;

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
  bbox[12] = -r; bbox[13] = r; bbox[14] = h;
  /* P6 */
  bbox[15] = -r; bbox[16] = -r; bbox[17] = h;
  /* P7 */
  bbox[18] = r; bbox[19] = -r; bbox[20] = h;
  /* P8 */
  bbox[21] = r; bbox[22] = r; bbox[23] = h;

 return AY_OK;
} /* ay_disk_bbccb */


int
ay_disk_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 int stride = 4, i, j, height;
 double *cv = NULL, *vk = NULL, *controlv = NULL;
 ay_disk_object *disk = NULL;
 ay_object *new = NULL;
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

  disk = (ay_disk_object *) o->refine;

  if(type == AY_IDNPATCH)
    {
      if(disk->thetamax < 0.0)
	{
	  ay_status = ay_nb_CreateNurbsCircle(disk->radius,
					      disk->thetamax, 0.0,
					      &height, &vk, &cv);
	}
      else
	{
	  ay_status = ay_nb_CreateNurbsCircle(disk->radius,
					      0.0, disk->thetamax,
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
      j = (height*stride)+3;
      for(i = 0; i < height; i++)
	{
	  controlv[j] = 1.0;
	  j += stride;
	}
      j = 2;
      for(i = 0; i < height*2; i++)
	{
	  controlv[j] = disk->height * controlv[j+1];
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
      if(new->down)
	ay_object_delete(o->down);
      free(new);
    }

 return ay_status;
} /* ay_disk_providecb */


int
ay_disk_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_object *new = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new object(s) */

  ay_status = ay_disk_providecb(o, AY_IDNPATCH, &new);


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
} /* ay_disk_convertcb */


int
ay_disk_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_disk_name,
				    ay_disk_createcb,
				    ay_disk_deletecb,
				    ay_disk_copycb,
				    ay_disk_drawcb,
				    NULL, /* no points to edit */
				    ay_disk_shadecb,
				    ay_disk_setpropcb,
				    ay_disk_getpropcb,
				    NULL, /* No Picking! */
				    ay_disk_readcb,
				    ay_disk_writecb,
				    ay_disk_wribcb,
				    ay_disk_bbccb,
				    AY_IDDISK);

  ay_status = ay_convert_register(ay_disk_convertcb, AY_IDDISK);

  ay_status = ay_provide_register(ay_disk_providecb, AY_IDDISK);

 return ay_status;
} /* ay_disk_init */

