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

/* box.c - box object */

static char *ay_box_name = "Box";

int
ay_box_createcb(int argc, char *argv[], ay_object *o)
{
 ay_box_object *box = NULL;
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


int
ay_box_deletecb(void *c)
{
 ay_box_object *box = NULL;

  if(!c)
    return AY_ENULL;    

  box = (ay_box_object *)(c);

  free(box);

 return AY_OK;
} /* ay_box_deletecb */


int
ay_box_copycb(void *src, void **dst)
{
 ay_box_object *box = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(box = calloc(1, sizeof(ay_box_object))))
    return AY_EOMEM; 

  memcpy(box, src, sizeof(ay_box_object)); 

  *dst = (void *)box;

 return AY_OK;
} /* ay_box_copycb */


int
ay_box_drawcb(struct Togl *togl, ay_object *o)
{
 ay_box_object *box = NULL;
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


int
ay_box_shadecb(struct Togl *togl, ay_object *o)
{
 ay_box_object *box = NULL;
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
   // front
   glNormal3d((GLdouble)0.0,(GLdouble)0.0,(GLdouble)1.0);
   glVertex3d(-wh,-hh, lh);
   glVertex3d( wh,-hh, lh);
   glVertex3d( wh, hh, lh);
   glVertex3d(-wh, hh, lh);

   // top
   glNormal3d((GLdouble)0.0,(GLdouble)1.0,(GLdouble)0.0);
   glVertex3d( wh, hh,-lh);
   glVertex3d(-wh, hh,-lh);
   glVertex3d(-wh, hh, lh);
   glVertex3d( wh, hh, lh);
   // right
   glNormal3d((GLdouble)1.0,(GLdouble)0.0,(GLdouble)0.0);
   glVertex3d( wh,-hh, lh);
   glVertex3d( wh,-hh,-lh);
   glVertex3d( wh, hh,-lh);
   glVertex3d( wh, hh, lh);
   // back
   glNormal3d((GLdouble)0.0,(GLdouble)0.0,(GLdouble)-1.0);
   glVertex3d( wh,-hh,-lh);
   glVertex3d(-wh,-hh,-lh);
   glVertex3d(-wh, hh,-lh);
   glVertex3d( wh, hh,-lh);
   // bottom
   glNormal3d((GLdouble)0.0,(GLdouble)-1.0,(GLdouble)0.0);
   glVertex3d(-wh,-hh, lh);
   glVertex3d(-wh,-hh,-lh);
   glVertex3d( wh,-hh,-lh);
   glVertex3d( wh,-hh, lh);
   // left
   glNormal3d((GLdouble)-1.0,(GLdouble)0.0,(GLdouble)0.0);
   glVertex3d(-wh, hh,-lh);
   glVertex3d(-wh,-hh,-lh);
   glVertex3d(-wh,-hh, lh);
   glVertex3d(-wh, hh, lh);
  glEnd();

 return AY_OK;
} /* ay_box_shadecb */


/* Tcl -> C! */
int
ay_box_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1 = "BoxAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_box_object *box = NULL;

  if(!o)
    return AY_ENULL;

  box = (ay_box_object *)o->refine;
  
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

 return AY_OK;
} /* ay_box_setpropcb */


/* C -> Tcl! */
int
ay_box_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="BoxAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_box_object *box = NULL;

  if(!o)
    return AY_ENULL;

  box = (ay_box_object *)(o->refine);

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


int
ay_box_readcb(FILE *fileptr, ay_object *o)
{
 ay_box_object *box = NULL;

 if(!o)
   return AY_ENULL;

  if(!(box = calloc(1, sizeof(ay_box_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%lg\n",&box->width);
  fscanf(fileptr,"%lg\n",&box->length);
  fscanf(fileptr,"%lg\n",&box->height);

  o->refine = box;

 return AY_OK;
} /* ay_box_readcb */


int
ay_box_writecb(FILE *fileptr, ay_object *o)
{
 ay_box_object *box = NULL;

  if(!o)
    return AY_ENULL;

  box = (ay_box_object *)(o->refine);

  fprintf(fileptr, "%g\n", box->width);
  fprintf(fileptr, "%g\n", box->length);
  fprintf(fileptr, "%g\n", box->height);

 return AY_OK;
} /* ay_box_writecb */


int
ay_box_wribcb(char *file, ay_object *o)
{
 ay_box_object *box = NULL;
 RtPoint rect[4];

  if(!o)
   return AY_ENULL;

  box = (ay_box_object*)o->refine;

  if(!ay_current_primlevel)
    {
      RiSolidBegin(RI_PRIMITIVE);
    }

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
  if(!ay_current_primlevel)
    {
      RiSolidEnd();
    }

 return AY_OK;
} /* ay_box_wribcb */


int
ay_box_bbccb(ay_object *o, double *bbox, int *flags)
{
 double wh = 0.0, lh = 0.0, hh = 0.0;
 ay_box_object *box = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  box = (ay_box_object *)o->refine; 

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


int
ay_box_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_box_name,
				    ay_box_createcb,
				    ay_box_deletecb,
				    ay_box_copycb,
				    ay_box_drawcb,
				    NULL, /* no points to edit */
				    ay_box_shadecb,
				    ay_box_setpropcb,
				    ay_box_getpropcb,
				    NULL, /* No Picking! */
				    ay_box_readcb,
				    ay_box_writecb,
				    ay_box_wribcb,
				    ay_box_bbccb,
				    AY_IDBOX);

 return ay_status;
} /* ay_box_init */

