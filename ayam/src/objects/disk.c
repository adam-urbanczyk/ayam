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

/* disk.c - disk object */

static char *ay_disk_name = "Disk";

int ay_disk_notifycb(ay_object *o);

/* number of read only points */
#define AY_PDISK 10

/* functions: */

/* ay_disk_createcb:
 *  create callback function of disk object
 */
int
ay_disk_createcb(int argc, char *argv[], ay_object *o)
{
 ay_disk_object *disk;
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


/* ay_disk_deletecb:
 *  delete callback function of disk object
 */
int
ay_disk_deletecb(void *c)
{
 ay_disk_object *disk;

  if(!c)
    return AY_ENULL;

  disk = (ay_disk_object *)(c);

  if(disk->pnts)
    free(disk->pnts);

  free(disk);

 return AY_OK;
} /* ay_disk_deletecb */


/* ay_disk_copycb:
 *  copy callback function of disk object
 */
int
ay_disk_copycb(void *src, void **dst)
{
 ay_disk_object *disk;

  if(!src || !dst)
    return AY_ENULL;

  if(!(disk = malloc(sizeof(ay_disk_object))))
    return AY_EOMEM;

  memcpy(disk, src, sizeof(ay_disk_object));

  disk->pnts = NULL;

  *dst = (void *)disk;

 return AY_OK;
} /* ay_disk_copycb */


/* ay_disk_drawcb:
 *  draw (display in an Ayam view window) callback function of disk object
 */
int
ay_disk_drawcb(struct Togl *togl, ay_object *o)
{
 ay_disk_object *disk;
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


/* ay_disk_shadecb:
 *  shade (display in an Ayam view window) callback function of disk object
 */
int
ay_disk_shadecb(struct Togl *togl, ay_object *o)
{
 ay_disk_object *disk;
 double radius, thetamax;

  if(!o)
    return AY_ENULL;

  disk = (ay_disk_object *)o->refine;

  if(!disk)
    return AY_ENULL;

  radius = disk->radius;
  thetamax = disk->thetamax;

  glTranslated(0.0, 0.0, (GLdouble)disk->height);

  /* check, if disk is simple */
  if(disk->is_simple)
    {
      /* yes, it is */
      gluDisk(ay_gluquadobj, 0.0, radius, 8, 1);
    }
  else
    {
      glRotated(thetamax-90.0, 0.0, 0.0, 1.0);
      gluPartialDisk(ay_gluquadobj, 0.0, radius, 8, 1, 0.0, thetamax);
    }

 return AY_OK;
} /* ay_disk_shadecb */


/* ay_disk_drawhcb:
 *  draw handles (in an Ayam view window) callback function of disk object
 */
int
ay_disk_drawhcb(struct Togl *togl, ay_object *o)
{
 int i;
 double *pnts;
 ay_disk_object *disk;

  if(!o)
    return AY_ENULL;

  disk = (ay_disk_object *) o->refine;

  if(!disk)
    return AY_ENULL;

  if(!disk->pnts)
    {
      if(!(pnts = calloc(AY_PDISK*3, sizeof(double))))
	return AY_EOMEM;

      disk->pnts = pnts;
      (void)ay_disk_notifycb(o);
    }
  else
    {
      pnts = disk->pnts;
    }

  glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
	    (GLfloat)ay_prefs.obb);

  glBegin(GL_POINTS);
   for(i = 0; i < AY_PDISK; i++)
     {
       glVertex3dv((GLdouble *)pnts);
       pnts += 3;
     }
  glEnd();

  glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
	    (GLfloat)ay_prefs.seb);

 return AY_OK;
} /* ay_disk_drawhcb */


/* ay_disk_getpntcb:
 *  get point (editing and selection) callback function of disk object
 */
int
ay_disk_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_disk_object *disk;

  if(!o)
    return AY_ENULL;

  disk = (ay_disk_object *)o->refine;

  if(!disk)
    return AY_ENULL;

  if(!disk->pnts)
    {
      if(!(disk->pnts = calloc(AY_PDISK*3, sizeof(double))))
	return AY_EOMEM;
      (void)ay_disk_notifycb(o);
    }

 return ay_selp_getpnts(mode, o, p, pe, 1, AY_PDISK, 3, disk->pnts);
} /* ay_disk_getpntcb */


/* ay_disk_setpropcb:
 *  set property (from Tcl to C context) callback function of disk object
 */
int
ay_disk_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "DiskAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_disk_object *disk;

  if(!interp || !o)
    return AY_ENULL;

  disk = (ay_disk_object *)o->refine;

  if(!disk)
    return AY_ENULL;

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

  o->modified = AY_TRUE;

  ay_notify_object(o);

  ay_notify_parent();

 return AY_OK;
} /* ay_disk_setpropcb */


/* ay_disk_getpropcb:
 *  get property (from C to Tcl context) callback function of disk object
 */
int
ay_disk_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="DiskAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_disk_object *disk;

  if(!interp || !o)
    return AY_ENULL;

  disk = (ay_disk_object *)(o->refine);

  if(!disk)
    return AY_ENULL;

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


/* ay_disk_readcb:
 *  read (from scene file) callback function of disk object
 */
int
ay_disk_readcb(FILE *fileptr, ay_object *o)
{
 ay_disk_object *disk;

  if(!fileptr || !o)
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


/* ay_disk_writecb:
 *  write (to scene file) callback function of disk object
 */
int
ay_disk_writecb(FILE *fileptr, ay_object *o)
{
 ay_disk_object *disk;

  if(!fileptr || !o)
    return AY_ENULL;

  disk = (ay_disk_object *)(o->refine);

  if(!disk)
    return AY_ENULL;

  fprintf(fileptr, "%g\n", disk->radius);
  fprintf(fileptr, "%g\n", disk->height);
  fprintf(fileptr, "%g\n", disk->thetamax);

 return AY_OK;
} /* ay_disk_writecb */


/* ay_disk_wribcb:
 *  RIB export callback function of disk object
 */
int
ay_disk_wribcb(char *file, ay_object *o)
{
 ay_disk_object *disk;

  if(!o)
   return AY_ENULL;

  disk = (ay_disk_object*)o->refine;

  if(!disk)
    return AY_ENULL;

  RiDisk((RtFloat)disk->height,
	 (RtFloat)disk->radius,
	 (RtFloat)disk->thetamax,
	 NULL);

 return AY_OK;
} /* ay_disk_wribcb */


/* ay_disk_bbccb:
 *  bounding box calculation callback function of disk object
 */
int
ay_disk_bbccb(ay_object *o, double *bbox, int *flags)
{
 double r = 0.0, h = 0.0;
 ay_disk_object *disk;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  disk = (ay_disk_object *)o->refine;

  if(!disk)
    return AY_ENULL;

  if(!disk->is_simple)
    {
      if(!disk->pnts)
	{
	  if(!(disk->pnts = calloc(AY_PDISK*3, sizeof(double))))
	    { return AY_EOMEM; }
	  (void)ay_disk_notifycb(o);
	}

      return ay_bbc_fromarr(disk->pnts, AY_PDISK, 3, bbox);
    }

  r = disk->radius;
  h = disk->height;

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


/* ay_disk_notifycb:
 *  notification callback function of disk object
 */
int
ay_disk_notifycb(ay_object *o)
{
 ay_disk_object *disk;
 double *pnts;
 double radius, w;
 int i, a;
 double thetadiff, angle;

  if(!o)
    return AY_ENULL;

  disk = (ay_disk_object *)o->refine;

  if(!disk)
    return AY_ENULL;

  if(disk->pnts)
    {
      radius = disk->radius;
      w = (sqrt(2.0)*0.5);

      pnts = disk->pnts;
      if(disk->is_simple)
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

	  a = 2;
	  for(i = 0; i <= 9; i++)
	    {
	      pnts[a] = disk->height;
	      a += 3;
	    } /* for */
	  memcpy(&(pnts[27]), &(pnts[3]), 3*sizeof(double));
	}
      else
	{
	  thetadiff = AY_D2R(disk->thetamax/8);
	  angle = 0.0;
	  pnts[2] = disk->height;
	  a = 3;
	  for(i = 0; i < 9; i++)
	    {
	      pnts[a] = cos(angle)*radius;
	      pnts[a+1] = sin(angle)*radius;
	      pnts[a+2] = disk->height;
	      a += 3;
	      angle += thetadiff;
	    } /* for */
	} /* if */
    } /* if */

 return AY_OK;
} /* ay_disk_notifycb */


/* ay_disk_providecb:
 *  provide callback function of disk object
 */
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

  if(!disk)
    return AY_ENULL;

  if(type == AY_IDNPATCH)
    {
      if(disk->thetamax < 0.0)
	{
	  ay_status = ay_nb_CreateNurbsCircleArc(disk->radius,
						 disk->thetamax, 0.0,
						 &height, &vk, &cv);
	}
      else
	{
	  ay_status = ay_nb_CreateNurbsCircleArc(disk->radius,
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
	  controlv[j] = disk->height;
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
} /* ay_disk_providecb */


/* ay_disk_convertcb:
 *  convert callback function of disk object
 */
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
	  ay_object_link(new);
	}
      else
	{
	  ay_object_replace(new, o);
	} /* if */
    } /* if */

 return ay_status;
} /* ay_disk_convertcb */


/* ay_disk_init:
 *  initialize the disk object module
 */
int
ay_disk_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_disk_name,
				    ay_disk_createcb,
				    ay_disk_deletecb,
				    ay_disk_copycb,
				    ay_disk_drawcb,
				    ay_disk_drawhcb,
				    ay_disk_shadecb,
				    ay_disk_setpropcb,
				    ay_disk_getpropcb,
				    ay_disk_getpntcb,
				    ay_disk_readcb,
				    ay_disk_writecb,
				    ay_disk_wribcb,
				    ay_disk_bbccb,
				    AY_IDDISK);

  ay_status += ay_notify_register(ay_disk_notifycb, AY_IDDISK);

  ay_status += ay_convert_register(ay_disk_convertcb, AY_IDDISK);

  ay_status += ay_provide_register(ay_disk_providecb, AY_IDDISK);

 return ay_status;
} /* ay_disk_init */

