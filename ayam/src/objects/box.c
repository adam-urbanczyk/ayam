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

/* box.c - box object */

static char *ay_box_name = "Box";

int ay_box_notifycb(ay_object *o);

/* functions: */

/* ay_box_createcb:
 *  create callback function of box object
 */
int
ay_box_createcb(int argc, char *argv[], ay_object *o)
{
 ay_box_object *box;
 char fname[] = "crtbox";

  if(!o)
    return AY_ENULL;

  if(!(box = calloc(1, sizeof(ay_box_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  box->width  = 1.0;
  box->length = 1.0;
  box->height = 1.0;

  o->refine = box;

 return AY_OK;
} /* ay_box_createcb */


/* ay_box_deletecb:
 *  delete callback function of box object
 */
int
ay_box_deletecb(void *c)
{
 ay_box_object *box;

  if(!c)
    return AY_ENULL;

  box = (ay_box_object *)(c);

  if(box->pnts)
    free(box->pnts);

  free(box);

 return AY_OK;
} /* ay_box_deletecb */


/* ay_box_copycb:
 *  copy callback function of box object
 */
int
ay_box_copycb(void *src, void **dst)
{
 ay_box_object *box;

  if(!src || !dst)
    return AY_ENULL;

  if(!(box = malloc(sizeof(ay_box_object))))
    return AY_EOMEM;

  memcpy(box, src, sizeof(ay_box_object));

  box->pnts = NULL;

  *dst = (void *)box;

 return AY_OK;
} /* ay_box_copycb */


/* ay_box_drawcb:
 *  draw (display in an Ayam view window) callback function of box object
 */
int
ay_box_drawcb(struct Togl *togl, ay_object *o)
{
 ay_box_object *box;
 GLdouble wh, hh, lh;

  if(!o)
    return AY_ENULL;

  box = (ay_box_object *)o->refine;

  if(!box)
    return AY_ENULL;

  wh = (GLdouble)(box->width  * 0.5);
  lh = (GLdouble)(box->length * 0.5);
  hh = (GLdouble)(box->height * 0.5);

  /* draw */
  glBegin(GL_LINE_STRIP);
   glVertex3d( wh, hh, lh);
   glVertex3d( wh,-hh, lh);
   glVertex3d(-wh,-hh, lh);
   glVertex3d(-wh, hh, lh);
   glVertex3d( wh, hh, lh);
   glVertex3d( wh, hh,-lh);
   glVertex3d( wh,-hh,-lh);
   glVertex3d(-wh,-hh,-lh);
   glVertex3d(-wh, hh,-lh);
   glVertex3d( wh, hh,-lh);
  glEnd();

  glBegin(GL_LINES);
   glVertex3d( wh,-hh, lh);
   glVertex3d( wh,-hh,-lh);
   glVertex3d(-wh,-hh, lh);
   glVertex3d(-wh,-hh,-lh);
   glVertex3d(-wh, hh, lh);
   glVertex3d(-wh, hh,-lh);
  glEnd();

 return AY_OK;
} /* ay_box_drawcb */


/* ay_box_shadecb:
 *  shade (display in an Ayam view window) callback function of box object
 */
int
ay_box_shadecb(struct Togl *togl, ay_object *o)
{
 ay_box_object *box;
 GLdouble wh, hh, lh;

  if(!o)
    return AY_ENULL;

  box = (ay_box_object *)o->refine;

  if(!box)
    return AY_ENULL;

  wh = (GLdouble)(box->width  * 0.5);
  lh = (GLdouble)(box->length * 0.5);
  hh = (GLdouble)(box->height * 0.5);

  glBegin(GL_QUADS);
   /* front */
   glNormal3d((GLdouble)0.0,(GLdouble)0.0,(GLdouble)1.0);
   glVertex3d(-wh, hh, lh);
   glVertex3d(-wh,-hh, lh);
   glVertex3d( wh,-hh, lh);
   glVertex3d( wh, hh, lh);
   /* top */
   glNormal3d((GLdouble)0.0,(GLdouble)1.0,(GLdouble)0.0);
   glVertex3d( wh, hh, lh);
   glVertex3d( wh, hh,-lh);
   glVertex3d(-wh, hh,-lh);
   glVertex3d(-wh, hh, lh);
   /* right */
   glNormal3d((GLdouble)1.0,(GLdouble)0.0,(GLdouble)0.0);
   glVertex3d( wh, hh, lh);
   glVertex3d( wh,-hh, lh);
   glVertex3d( wh,-hh,-lh);
   glVertex3d( wh, hh,-lh);
   /* back */
   glNormal3d((GLdouble)0.0,(GLdouble)0.0,(GLdouble)-1.0);
   glVertex3d( wh, hh,-lh);
   glVertex3d( wh,-hh,-lh);
   glVertex3d(-wh,-hh,-lh);
   glVertex3d(-wh, hh,-lh);
   /* bottom */
   glNormal3d((GLdouble)0.0,(GLdouble)-1.0,(GLdouble)0.0);
   glVertex3d( wh,-hh, lh);
   glVertex3d(-wh,-hh, lh);
   glVertex3d(-wh,-hh,-lh);
   glVertex3d( wh,-hh,-lh);
   /* left */
   glNormal3d((GLdouble)-1.0,(GLdouble)0.0,(GLdouble)0.0);
   glVertex3d(-wh, hh, lh);
   glVertex3d(-wh, hh,-lh);
   glVertex3d(-wh,-hh,-lh);
   glVertex3d(-wh,-hh, lh);

  glEnd();

 return AY_OK;
} /* ay_box_shadecb */


/* ay_box_drawhcb:
 *  draw handles (in an Ayam view window) callback function of box object
 */
int
ay_box_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_box_object *box;
 double *pnts;
 int i;

  if(!o)
    return AY_ENULL;

  box = (ay_box_object *)o->refine;

  if(!box)
    return AY_ENULL;

  if(!box->pnts)
    {
      if(!(pnts = calloc(8*3, sizeof(double))))
	{
	  return AY_EOMEM;
	}
      box->pnts = pnts;
      (void)ay_box_notifycb(o);
    }
  else
    {
      pnts = box->pnts;
    }

  glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
	    (GLfloat)ay_prefs.obb);

  glBegin(GL_POINTS);
   for(i = 0; i < 8; i++)
     {
       glVertex3dv((GLdouble *)pnts);
       pnts += 3;
     }
  glEnd();

  glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
	    (GLfloat)ay_prefs.seb);

 return AY_OK;
} /* ay_box_drawhcb */


/* ay_box_getpntcb:
 *  get point (editing and selection) callback function of box object
 */
int
ay_box_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_box_object *box;

  if(!o)
    return AY_ENULL;

  box = (ay_box_object *)o->refine;

  if(!box)
    return AY_ENULL;

  if(!box->pnts)
    {
      if(!(box->pnts = calloc(8*3, sizeof(double))))
	return AY_EOMEM;
      (void)ay_box_notifycb(o);
    }

 return ay_selp_getpnts(mode, o, p, pe, 1, 8, 3, box->pnts);
} /* ay_box_getpntcb */


/* ay_box_setpropcb:
 *  set property (from Tcl to C context) callback function of box object
 */
int
ay_box_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "BoxAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_box_object *box;

  if(!interp || !o)
    return AY_ENULL;

  box = (ay_box_object *)o->refine;

  if(!box)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("Width",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &box->width);

  Tcl_SetStringObj(ton,"Length",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &box->length);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &box->height);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  (void)ay_box_notifycb(o);
  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_box_setpropcb */


/* ay_box_getpropcb:
 *  get property (from C to Tcl context) callback function of box object
 */
int
ay_box_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="BoxAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_box_object *box;

  if(!interp || !o)
    return AY_ENULL;

  box = (ay_box_object *)(o->refine);

  if(!box)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("Width",-1);
  to = Tcl_NewDoubleObj(box->width);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Length",-1);
  to = Tcl_NewDoubleObj(box->length);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_NewDoubleObj(box->height);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_box_getpropcb */


/* ay_box_readcb:
 *  read (from scene file) callback function of box object
 */
int
ay_box_readcb(FILE *fileptr, ay_object *o)
{
 ay_box_object *box;

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(box = calloc(1, sizeof(ay_box_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%lg\n",&box->width);
  fscanf(fileptr,"%lg\n",&box->length);
  fscanf(fileptr,"%lg\n",&box->height);

  o->refine = box;

 return AY_OK;
} /* ay_box_readcb */


/* ay_box_writecb:
 *  write (to scene file) callback function of box object
 */
int
ay_box_writecb(FILE *fileptr, ay_object *o)
{
 ay_box_object *box;

  if(!fileptr || !o)
    return AY_ENULL;

  box = (ay_box_object *)(o->refine);

  if(!box)
    return AY_ENULL;

  fprintf(fileptr, "%g\n", box->width);
  fprintf(fileptr, "%g\n", box->length);
  fprintf(fileptr, "%g\n", box->height);

 return AY_OK;
} /* ay_box_writecb */


/* ay_box_wribcb:
 *  RIB export callback function of box object
 */
int
ay_box_wribcb(char *file, ay_object *o)
{
 ay_box_object *box;
 RtPoint rect[4];

  if(!o)
   return AY_ENULL;

  box = (ay_box_object*)o->refine;

  if(!box)
    return AY_ENULL;

   RiTransformBegin();
    RiScale((RtFloat)box->width, (RtFloat)box->height, (RtFloat)box->length);

    rect[0][0] = (RtFloat)(0.5);
    rect[0][1] = (RtFloat)(0.5);
    rect[0][2] = (RtFloat)(0.5);

    rect[1][0] = (RtFloat)(0.5);
    rect[1][1] = (RtFloat)(0.5);
    rect[1][2] = (RtFloat)(-0.5);

    rect[2][0] = (RtFloat)(0.5);
    rect[2][1] = (RtFloat)(-0.5);
    rect[2][2] = (RtFloat)(0.5);

    rect[3][0] = (RtFloat)(0.5);
    rect[3][1] = (RtFloat)(-0.5);
    rect[3][2] = (RtFloat)(-0.5);

    RiPatch(RI_BILINEAR, RI_P, (RtPointer)rect, RI_NULL);
    RiRotate((RtFloat)90.0, (RtFloat)0.0, (RtFloat)0.0, (RtFloat)1.0);
    RiPatch(RI_BILINEAR, RI_P, (RtPointer)rect, RI_NULL);
    RiRotate((RtFloat)90.0, (RtFloat)0.0, (RtFloat)0.0, (RtFloat)1.0);
    RiPatch(RI_BILINEAR, RI_P, (RtPointer)rect, RI_NULL);
    RiRotate((RtFloat)90.0, (RtFloat)0.0, (RtFloat)0.0, (RtFloat)1.0);
    RiPatch(RI_BILINEAR, RI_P, (RtPointer)rect, RI_NULL);
    RiRotate((RtFloat)90.0, (RtFloat)0.0, (RtFloat)1.0, (RtFloat)0.0);
    RiPatch(RI_BILINEAR, RI_P, (RtPointer)rect, RI_NULL);
    RiRotate((RtFloat)180.0, (RtFloat)0.0, (RtFloat)1.0, (RtFloat)0.0);
    RiPatch(RI_BILINEAR, RI_P, (RtPointer)rect, RI_NULL);

   RiTransformEnd();

 return AY_OK;
} /* ay_box_wribcb */


/* ay_box_bbccb:
 *  bounding box calculation callback function of box object
 */
int
ay_box_bbccb(ay_object *o, double *bbox, int *flags)
{
 double wh = 0.0, lh = 0.0, hh = 0.0;
 ay_box_object *box;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  box = (ay_box_object *)o->refine;

  if(!box)
    return AY_ENULL;

  wh = box->width  * 0.5;
  lh = box->length * 0.5;
  hh = box->height * 0.5;

  /* P1 */
  bbox[0] = -wh; bbox[1] = hh; bbox[2] = lh;
  /* P2 */
  bbox[3] = -wh; bbox[4] = hh; bbox[5] = -lh;
  /* P3 */
  bbox[6] = wh; bbox[7] = hh; bbox[8] = -lh;
  /* P4 */
  bbox[9] = wh; bbox[10] = hh; bbox[11] = lh;

  /* P5 */
  bbox[12] = -wh; bbox[13] = -hh; bbox[14] = lh;
  /* P6 */
  bbox[15] = -wh; bbox[16] = -hh; bbox[17] = -lh;
  /* P7 */
  bbox[18] = wh; bbox[19] = -hh; bbox[20] = -lh;
  /* P8 */
  bbox[21] = wh; bbox[22] = -hh; bbox[23] = lh;

 return AY_OK;
} /* ay_box_bbccb */


/* ay_box_notifycb:
 *  notification callback function of box object
 */
int
ay_box_notifycb(ay_object *o)
{
 ay_box_object *box;
 double wh, hh, lh;
 double *pnts;
 int i = 0, a = 0;

  if(!o)
    return AY_ENULL;

  box = (ay_box_object *)o->refine;

  if(!box)
    return AY_ENULL;

  if(box->pnts)
    {
      pnts = box->pnts;
      wh = (box->width  * 0.5);
      lh = (box->length * 0.5);
      hh = (box->height * 0.5);
      a = 0;
      for(i = 0; i < 8; i++)
	{
	  if(i%4<2)
	    pnts[a] = -wh;
	  else
	    pnts[a] = wh;
	  if(i%2<1)
	    pnts[a+1] = -hh;
	  else
	    pnts[a+1] = hh;
	  if(i<4)
	    pnts[a+2] = -lh;
	  else
	    pnts[a+2] = lh;
	  a += 3;
	} /* for */
    } /* if */

 return AY_OK;
} /* ay_box_notifycb */


/* ay_box_providecb:
 *  provide callback function of box object
 */
int
ay_box_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 int stride = 4, i, j;
 double *controlv = NULL;
 double wh = 0.0, lh = 0.0, hh = 0.0;
 ay_box_object *box = NULL;
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

  box = (ay_box_object *) o->refine;

  if(!box)
    return AY_ENULL;

  wh = box->width  * 0.5;
  lh = box->length * 0.5;
  hh = box->height * 0.5;

  if(type == AY_IDNPATCH)
    {
      if(!(controlv = calloc(1, 5*2*stride*sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      /* set x */
      controlv[0] = wh;
      controlv[4] = -wh;
      controlv[8] = -wh;
      controlv[12] = wh;
      controlv[16] = wh;
      controlv[20] = wh;
      controlv[24] = -wh;
      controlv[28] = -wh;
      controlv[32] = wh;
      controlv[36] = wh;

      /* set y */
      j = 1;
      for(i = 0; i < 5; i++)
	{
	  controlv[j] = hh;
	  controlv[j+5*stride] = -hh;
	  j += stride;
	}

      /* set z */
      controlv[2] = lh;
      controlv[6] = lh;
      controlv[10] = -lh;
      controlv[14] = -lh;
      controlv[18] = lh;
      controlv[22] = lh;
      controlv[26] = lh;
      controlv[30] = -lh;
      controlv[34] = -lh;
      controlv[38] = lh;

      /* set w eights */
      j = 3;
      for(i = 0; i < 10; i++)
	{
	  controlv[j] = 1.0;
	  j += stride;
	}

      ay_status = ay_npt_create(2, 2, 2, 5, AY_KTBEZIER, AY_KTNURB,
				controlv, NULL, NULL, &np);

      if(ay_status)
	goto cleanup;

      if((ay_status = ay_npt_createnpatchobject(&new)))
	goto cleanup;

      new->down = ay_endlevel;
      ay_trafo_copy(o, new);
      new->refine = np;

      n = &(new->next);

      /* create upper cap */
      ay_object_defaults(&d);
      d.type = AY_IDBPATCH;
      d.refine = &bpatch;
      memcpy(bpatch.p1, &(controlv[0*stride]), 3*sizeof(double));
      memcpy(bpatch.p2, &(controlv[1*stride]), 3*sizeof(double));
      memcpy(bpatch.p3, &(controlv[2*stride]), 3*sizeof(double));
      memcpy(bpatch.p4, &(controlv[3*stride]), 3*sizeof(double));
      ay_provide_object(&d, AY_IDNPATCH, n);
      if(*n)
	{
	  ay_trafo_copy(o, *n);
	  n = &((*n)->next);
	}
      /* create lower cap */
      memcpy(bpatch.p1, &(controlv[6*stride]), 3*sizeof(double));
      memcpy(bpatch.p2, &(controlv[7*stride]), 3*sizeof(double));
      memcpy(bpatch.p3, &(controlv[8*stride]), 3*sizeof(double));
      memcpy(bpatch.p4, &(controlv[9*stride]), 3*sizeof(double));
      ay_provide_object(&d, AY_IDNPATCH, n);
      if(*n)
	{
	  ay_trafo_copy(o, *n);
	  /*n = &((*n)->next);*/
	}

      /* copy some tags */
      ay_tag_copyselected(o, new, ay_prefs.converttags,
			  ay_prefs.converttagslen);

      /* return result */
      *result = new;

      controlv = NULL; np = NULL; new = NULL;
    } /* if */


cleanup:

  if(controlv)
    free(controlv);

  if(np)
    free(np);

  if(new)
    {
      (void)ay_object_deletemulti(new, AY_FALSE);
    }

 return ay_status;
} /* ay_box_providecb */


/* ay_box_convertcb:
 *  convert callback function of box object
 */
int
ay_box_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_object *new = NULL, *t;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */

  if(!(new = calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  ay_object_defaults(new);
  new->type = AY_IDLEVEL;
  new->parent = AY_TRUE;
  new->inherit_trafos = AY_TRUE;

  if(!(new->refine = calloc(1, sizeof(ay_level_object))))
    { free(new); return AY_EOMEM; }

  ((ay_level_object *)(new->refine))->type = AY_LTLEVEL;

  ay_status = ay_box_providecb(o, AY_IDNPATCH, &(new->down));
  if(ay_status)
    { free(new->refine); free(new); return ay_status; }

  t = new->down;
  while(t && t->next)
    {
      t = t->next;
    }
  if(t)
    t->next = ay_endlevel;

  /* second, link new objects, or replace old object with them */

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
} /* ay_box_convertcb */


/* ay_box_init:
 *  initialize the box object module
 */
int
ay_box_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_box_name,
				    ay_box_createcb,
				    ay_box_deletecb,
				    ay_box_copycb,
				    ay_box_drawcb,
				    ay_box_drawhcb,
				    ay_box_shadecb,
				    ay_box_setpropcb,
				    ay_box_getpropcb,
				    ay_box_getpntcb,
				    ay_box_readcb,
				    ay_box_writecb,
				    ay_box_wribcb,
				    ay_box_bbccb,
				    AY_IDBOX);

  ay_status += ay_notify_register(ay_box_notifycb, AY_IDBOX);

  ay_status += ay_convert_register(ay_box_convertcb, AY_IDBOX);

  ay_status += ay_provide_register(ay_box_providecb, AY_IDBOX);

 return ay_status;
} /* ay_box_init */

