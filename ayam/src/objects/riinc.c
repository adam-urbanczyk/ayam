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

/* riinc.c - riinc (RIB-Include) object */

static char *ay_riinc_name = "RiInc";

int
ay_riinc_createcb(int argc, char *argv[], ay_object *o)
{
 ay_riinc_object *riinc = NULL;
 char fname[] = "crtriinc";

  if(!o)
    return AY_ENULL;

  if(!(riinc = calloc(1, sizeof(ay_riinc_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  riinc->width  = 1.0;
  riinc->length = 1.0;
  riinc->height = 1.0;

  o->refine = riinc;

 return AY_OK;
} /* ay_riinc_createcb */


int
ay_riinc_deletecb(void *c)
{
 ay_riinc_object *riinc = NULL;

  if(!c)
    return AY_ENULL;    

  riinc = (ay_riinc_object *)(c);

  if(riinc->file)
    free(riinc->file);

  free(riinc);

 return AY_OK;
} /* ay_riinc_deletecb */


int
ay_riinc_copycb(void *src, void **dst)
{
 ay_riinc_object *srcr = NULL, *riinc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  srcr = (ay_riinc_object*) src;

  if(!(riinc = calloc(1, sizeof(ay_riinc_object))))
    return AY_EOMEM; 

  memcpy(riinc, src, sizeof(ay_riinc_object)); 
  riinc->file = NULL;

  /* copy file name */
  if(srcr->file)
    {
      if(!(riinc->file = calloc(strlen(srcr->file)+1, sizeof(char))))
	{
	  free(riinc);
	  return AY_EOMEM;
	}

      strcpy(riinc->file, srcr->file);
    }

  *dst = (void *)riinc;

 return AY_OK;
} /* ay_riinc_copycb */


int
ay_riinc_drawcb(struct Togl *togl, ay_object *o)
{
 ay_riinc_object *riinc = NULL;
 GLdouble wh, hh, lh;

  if(!o)
    return AY_ENULL;

  riinc = (ay_riinc_object *)o->refine;

  if(!riinc)
    return AY_ENULL;

  wh = (GLdouble)(riinc->width  * 0.5);
  lh = (GLdouble)(riinc->length * 0.5);
  hh = (GLdouble)(riinc->height * 0.5);

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
} /* ay_riinc_drawcb */


int
ay_riinc_shadecb(struct Togl *togl, ay_object *o)
{
 ay_riinc_object *riinc = NULL;

  if(!o)
    return AY_ENULL;

  riinc = (ay_riinc_object *)o->refine;

  if(!riinc)
    return AY_ENULL;

 return AY_OK;
} /* ay_riinc_shadecb */


/* Tcl -> C! */
int
ay_riinc_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char fname[] = "riinc_setprop";
 char *n1 = "RiIncAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_riinc_object *riinc = NULL;
 char *result = NULL;

  if(!o)
    return AY_ENULL;

  riinc = (ay_riinc_object *)o->refine;

  if(riinc->file)
    {
      free(riinc->file);
      riinc->file = NULL;
    }

  toa = Tcl_NewStringObj(n1,-1);

  /* get filename */
  result = Tcl_GetVar2(interp, n1, "File", TCL_LEAVE_ERR_MSG |
		       TCL_GLOBAL_ONLY);
  if(!(riinc->file = calloc(strlen(result)+1, sizeof(char))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  strcpy(riinc->file, result);


  ton = Tcl_NewStringObj("Width",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &riinc->width);

  Tcl_SetStringObj(ton,"Length",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &riinc->length);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &riinc->height);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_riinc_setpropcb */


/* C -> Tcl! */
int
ay_riinc_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="RiIncAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_riinc_object *riinc = NULL;

  if(!o)
    return AY_ENULL;

  riinc = (ay_riinc_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  Tcl_SetVar2(interp,n1,"File","",TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(riinc->file)
    Tcl_SetVar2(interp,n1,"File",riinc->file,TCL_LEAVE_ERR_MSG |
		TCL_GLOBAL_ONLY);

  ton = Tcl_NewStringObj("Width",-1);
  to = Tcl_NewDoubleObj(riinc->width);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Length",-1);
  to = Tcl_NewDoubleObj(riinc->length);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_NewDoubleObj(riinc->height);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_riinc_getpropcb */


int
ay_riinc_readcb(FILE *fileptr, ay_object *o)
{
 ay_riinc_object *riinc = NULL;
 int read = 0;

 if(!o)
   return AY_ENULL;

  if(!(riinc = calloc(1, sizeof(ay_riinc_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%lg\n",&riinc->width);
  fscanf(fileptr,"%lg\n",&riinc->length);
  fscanf(fileptr,"%lg",&riinc->height);
  read = fgetc(fileptr);
  if(read == '\r')
    fgetc(fileptr);
  ay_read_string(fileptr, &(riinc->file));

  o->refine = riinc;

 return AY_OK;
} /* ay_riinc_readcb */


int
ay_riinc_writecb(FILE *fileptr, ay_object *o)
{
 ay_riinc_object *riinc = NULL;

  if(!o)
    return AY_ENULL;

  riinc = (ay_riinc_object *)(o->refine);

  fprintf(fileptr, "%g\n", riinc->width);
  fprintf(fileptr, "%g\n", riinc->length);
  fprintf(fileptr, "%g\n", riinc->height);
  fprintf(fileptr, "%s\n", riinc->file);

 return AY_OK;
} /* ay_riinc_writecb */


int
ay_riinc_wribcb(char *file, ay_object *o)
{
 ay_riinc_object *riinc = NULL;

  if(!o)
    return AY_ENULL;

  riinc = (ay_riinc_object *)(o->refine);

  if(!riinc)
    return AY_ENULL;

  if(riinc->file)
    RiReadArchive(riinc->file,(RtVoid*)RI_NULL,RI_NULL);

 return AY_OK;
} /* ay_riinc_wribcb */


int
ay_riinc_bbccb(ay_object *o, double *bbox, int *flags)
{
 double wh = 0.0, lh = 0.0, hh = 0.0;
 ay_riinc_object *riinc = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  riinc = (ay_riinc_object *)o->refine; 

  wh = riinc->width  * 0.5;
  lh = riinc->length * 0.5;
  hh = riinc->height * 0.5;

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
} /* ay_riinc_bbccb */


int
ay_riinc_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_riinc_name,
				    ay_riinc_createcb,
				    ay_riinc_deletecb,
				    ay_riinc_copycb,
				    ay_riinc_drawcb,
				    NULL, /* no points to edit */
				    ay_riinc_shadecb,
				    ay_riinc_setpropcb,
				    ay_riinc_getpropcb,
				    NULL, /* No Picking! */
				    ay_riinc_readcb,
				    ay_riinc_writecb,
				    ay_riinc_wribcb,
				    ay_riinc_bbccb,
				    AY_IDRIINC);

 return ay_status;
} /* ay_riinc_init */

