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

/* riproc.c - riproc (RIB-Procedural) object */

static char *ay_riproc_name = "RiProc";

int
ay_riproc_createcb(int argc, char *argv[], ay_object *o)
{
 ay_riproc_object *riproc = NULL;
 char fname[] = "crtriproc";

  if(!o)
    return AY_ENULL;

  if(!(riproc = calloc(1, sizeof(ay_riproc_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  riproc->minx = -1.0;
  riproc->maxx = 1.0;
  riproc->miny = -1.0;
  riproc->maxy = 1.0;
  riproc->minz = -1.0;
  riproc->maxz = 1.0;

  o->refine = riproc;

 return AY_OK;
} /* ay_riproc_createcb */


int
ay_riproc_deletecb(void *c)
{
 ay_riproc_object *riproc = NULL;

  if(!c)
    return AY_ENULL;

  riproc = (ay_riproc_object *)(c);

  if(riproc->file)
    free(riproc->file);

  if(riproc->data)
    free(riproc->data);

  free(riproc);

 return AY_OK;
} /* ay_riproc_deletecb */


int
ay_riproc_copycb(void *src, void **dst)
{
 ay_riproc_object *srcr = NULL, *riproc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  srcr = (ay_riproc_object*) src;

  if(!(riproc = calloc(1, sizeof(ay_riproc_object))))
    return AY_EOMEM;

  memcpy(riproc, src, sizeof(ay_riproc_object));
  riproc->file = NULL;
  riproc->data = NULL;

  /* copy file */
  if(srcr->file)
    {
      if(!(riproc->file = calloc(strlen(srcr->file)+1, sizeof(char))))
	{
	  free(riproc);
	  return AY_EOMEM;
	}

      strcpy(riproc->file, srcr->file);
    }

  /* copy data */
  if(srcr->data)
    {
      if(!(riproc->data = calloc(strlen(srcr->data)+1, sizeof(char))))
	{
	  if(riproc->file)
	    free(riproc->file);
	  free(riproc);
	  return AY_EOMEM;
	}

      strcpy(riproc->data, srcr->data);
    }

  *dst = (void *)riproc;

 return AY_OK;
} /* ay_riproc_copycb */


int
ay_riproc_drawcb(struct Togl *togl, ay_object *o)
{
 ay_riproc_object *riproc = NULL;

  if(!o)
    return AY_ENULL;

  riproc = (ay_riproc_object *)o->refine;

  if(!riproc)
    return AY_ENULL;

  /* draw */
  glBegin(GL_LINE_STRIP);
   glVertex3d((GLdouble)riproc->maxx, (GLdouble)riproc->maxy,
	      (GLdouble)riproc->maxz);
   glVertex3d((GLdouble)riproc->maxx,(GLdouble)riproc->miny,
	      (GLdouble)riproc->maxz);
   glVertex3d((GLdouble)riproc->minx,(GLdouble)riproc->miny,
	      (GLdouble)riproc->maxz);
   glVertex3d((GLdouble)riproc->minx, (GLdouble)riproc->maxy,
	      (GLdouble)riproc->maxz);
   glVertex3d((GLdouble)riproc->maxx, (GLdouble)riproc->maxy,
	      (GLdouble)riproc->maxz);
   glVertex3d((GLdouble)riproc->maxx, (GLdouble)riproc->maxy,
	      (GLdouble)riproc->minz);
   glVertex3d((GLdouble)riproc->maxx,(GLdouble)riproc->miny,
	      (GLdouble)riproc->minz);
   glVertex3d((GLdouble)riproc->minx,(GLdouble)riproc->miny,
	      (GLdouble)riproc->minz);
   glVertex3d((GLdouble)riproc->minx, (GLdouble)riproc->maxy,
	      (GLdouble)riproc->minz);
   glVertex3d((GLdouble)riproc->maxx, (GLdouble)riproc->maxy,
	      (GLdouble)riproc->minz);
  glEnd();

  glBegin(GL_LINES);
   glVertex3d((GLdouble)riproc->maxx,(GLdouble)riproc->miny,
	      (GLdouble)riproc->maxz);
   glVertex3d((GLdouble)riproc->maxx,(GLdouble)riproc->miny,
	      (GLdouble)riproc->minz);
   glVertex3d((GLdouble)riproc->minx,(GLdouble)riproc->miny,
	      (GLdouble)riproc->maxz);
   glVertex3d((GLdouble)riproc->minx,(GLdouble)riproc->miny,
	      (GLdouble)riproc->minz);
   glVertex3d((GLdouble)riproc->minx, (GLdouble)riproc->maxy,
	      (GLdouble)riproc->maxz);
   glVertex3d((GLdouble)riproc->minx, (GLdouble)riproc->maxy,
	      (GLdouble)riproc->minz);
  glEnd();

 return AY_OK;
} /* ay_riproc_drawcb */


int
ay_riproc_shadecb(struct Togl *togl, ay_object *o)
{
 ay_riproc_object *riproc = NULL;

  if(!o)
    return AY_ENULL;

  riproc = (ay_riproc_object *)o->refine;

  if(!riproc)
    return AY_ENULL;

 return AY_OK;
} /* ay_riproc_shadecb */


/* Tcl -> C! */
int
ay_riproc_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "riproc_setprop";
 char *n1 = "RiProcAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_riproc_object *riproc = NULL;
 char *result = NULL;

  if(!o)
    return AY_ENULL;

  riproc = (ay_riproc_object *)o->refine;

  if(riproc->file)
    {
      free(riproc->file);
      riproc->file = NULL;
    }

  toa = Tcl_NewStringObj(n1,-1);

  /* get file */
  result = Tcl_GetVar2(interp, n1, "File", TCL_LEAVE_ERR_MSG |
		       TCL_GLOBAL_ONLY);
  if(!(riproc->file = calloc(strlen(result)+1, sizeof(char))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }
  strcpy(riproc->file, result);

  /* get data */
  result = Tcl_GetVar2(interp, n1, "Data", TCL_LEAVE_ERR_MSG |
		       TCL_GLOBAL_ONLY);
  if(!(riproc->data = calloc(strlen(result)+1, sizeof(char))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }
  strcpy(riproc->data, result);

  /* get bounding box */
  ton = Tcl_NewStringObj("MinX",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &riproc->minx);

  ton = Tcl_NewStringObj("MaxX",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &riproc->maxx);

  ton = Tcl_NewStringObj("MinY",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &riproc->miny);

  ton = Tcl_NewStringObj("MaxY",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &riproc->maxy);

  ton = Tcl_NewStringObj("MinZ",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &riproc->minz);

  ton = Tcl_NewStringObj("MaxZ",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &riproc->maxz);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  o->modified = AY_TRUE;
  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_riproc_setpropcb */


/* C -> Tcl! */
int
ay_riproc_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="RiProcAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_riproc_object *riproc = NULL;

  if(!o)
    return AY_ENULL;

  riproc = (ay_riproc_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  Tcl_SetVar2(interp,n1,"File","",TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(riproc->file)
    Tcl_SetVar2(interp,n1,"File",riproc->file,TCL_LEAVE_ERR_MSG |
		TCL_GLOBAL_ONLY);


  Tcl_SetVar2(interp,n1,"Data","",TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(riproc->data)
    Tcl_SetVar2(interp,n1,"Data",riproc->data,TCL_LEAVE_ERR_MSG |
		TCL_GLOBAL_ONLY);

  ton = Tcl_NewStringObj("MinX",-1);
  to = Tcl_NewDoubleObj(riproc->minx);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  ton = Tcl_NewStringObj("MaxX",-1);
  to = Tcl_NewDoubleObj(riproc->maxx);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  ton = Tcl_NewStringObj("MinY",-1);
  to = Tcl_NewDoubleObj(riproc->miny);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  ton = Tcl_NewStringObj("MaxY",-1);
  to = Tcl_NewDoubleObj(riproc->maxy);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  ton = Tcl_NewStringObj("MinZ",-1);
  to = Tcl_NewDoubleObj(riproc->minz);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  ton = Tcl_NewStringObj("MaxZ",-1);
  to = Tcl_NewDoubleObj(riproc->maxz);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_riproc_getpropcb */


int
ay_riproc_readcb(FILE *fileptr, ay_object *o)
{
 ay_riproc_object *riproc = NULL;
 int read = 0;

 if(!o)
   return AY_ENULL;

  if(!(riproc = calloc(1, sizeof(ay_riproc_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%lg\n",&riproc->minx);
  fscanf(fileptr,"%lg\n",&riproc->maxx);
  fscanf(fileptr,"%lg\n",&riproc->miny);
  fscanf(fileptr,"%lg\n",&riproc->maxy);
  fscanf(fileptr,"%lg\n",&riproc->minz);
  fscanf(fileptr,"%lg",&riproc->maxz);
  read = fgetc(fileptr);
  if(read == '\r')
    fgetc(fileptr);
  ay_read_string(fileptr, &(riproc->file));

  ay_read_string(fileptr, &(riproc->data));

  o->refine = riproc;

 return AY_OK;
} /* ay_riproc_readcb */


int
ay_riproc_writecb(FILE *fileptr, ay_object *o)
{
 ay_riproc_object *riproc = NULL;

  if(!o)
    return AY_ENULL;

  riproc = (ay_riproc_object *)(o->refine);

  fprintf(fileptr, "%g\n", riproc->minx);
  fprintf(fileptr, "%g\n", riproc->maxx);
  fprintf(fileptr, "%g\n", riproc->miny);
  fprintf(fileptr, "%g\n", riproc->maxy);
  fprintf(fileptr, "%g\n", riproc->minz);
  fprintf(fileptr, "%g\n", riproc->maxz);

  fprintf(fileptr, "%s\n", riproc->file);
  fprintf(fileptr, "%s\n", riproc->data);

 return AY_OK;
} /* ay_riproc_writecb */

RtVoid
ay_free(RtPointer data)
{
  free(data);
}

int
ay_riproc_wribcb(char *file, ay_object *o)
{
 char fname[] = "riproc_wribcb";
 ay_riproc_object *riproc = NULL;
 RtBound bound;
 RtString *data = NULL;

  if(!o)
    return AY_ENULL;

  riproc = (ay_riproc_object *)(o->refine);

  if(!riproc)
    return AY_ENULL;

  bound[0] = (RtFloat)riproc->minx;
  bound[1] = (RtFloat)riproc->maxx;
  bound[2] = (RtFloat)riproc->miny;
  bound[3] = (RtFloat)riproc->maxy;
  bound[4] = (RtFloat)riproc->minz;
  bound[5] = (RtFloat)riproc->maxz;

  switch(riproc->type)
    {
    case AY_PRTDREADA:
      if(!(data = calloc(1, sizeof(RtString))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return TCL_OK;
	}
      *data = riproc->file;

      RiProcedural((RtPointer)data, bound, RiProcDelayedReadArchive, ay_free);
      break;
    case AY_PRTRUNPROG:
      if(!(data = calloc(2, sizeof(RtString))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return TCL_OK;
	}
      data[0] = riproc->file;
      data[1] = riproc->data;

      RiProcedural((RtPointer)data, bound, RiProcRunProgram, ay_free);
      break;
    case AY_PRTDYNLOAD:
      if(!(data = calloc(2, sizeof(RtString))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return TCL_OK;
	}
      data[0] = riproc->file;
      data[1] = riproc->data;

      RiProcedural((RtPointer)data, bound, RiProcDynamicLoad, ay_free);
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_riproc_wribcb */


int
ay_riproc_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_riproc_object *riproc = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  riproc = (ay_riproc_object *)o->refine;

  /* P1 */
  bbox[0] = riproc->minx; bbox[1] = riproc->maxy; bbox[2] = riproc->maxz;
  /* P2 */
  bbox[3] = riproc->minx; bbox[4] = riproc->maxy; bbox[5] = riproc->minz;
  /* P3 */
  bbox[6] = riproc->maxx; bbox[7] = riproc->maxy; bbox[8] = riproc->minz;
  /* P4 */
  bbox[9] = riproc->maxx; bbox[10] = riproc->maxy; bbox[11] = riproc->maxz;

  /* P5 */
  bbox[12] = riproc->minx; bbox[13] = riproc->miny; bbox[14] = riproc->maxz;
  /* P6 */
  bbox[15] = riproc->minx; bbox[16] = riproc->miny; bbox[17] = riproc->minz;
  /* P7 */
  bbox[18] = riproc->maxx; bbox[19] = riproc->miny; bbox[20] = riproc->minz;
  /* P8 */
  bbox[21] = riproc->maxx; bbox[22] = riproc->miny; bbox[23] = riproc->maxz;

 return AY_OK;
} /* ay_riproc_bbccb */


int
ay_riproc_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_riproc_name,
				    ay_riproc_createcb,
				    ay_riproc_deletecb,
				    ay_riproc_copycb,
				    ay_riproc_drawcb,
				    NULL, /* no points to edit */
				    ay_riproc_shadecb,
				    ay_riproc_setpropcb,
				    ay_riproc_getpropcb,
				    NULL, /* No Picking! */
				    ay_riproc_readcb,
				    ay_riproc_writecb,
				    ay_riproc_wribcb,
				    ay_riproc_bbccb,
				    AY_IDRIPROC);

 return ay_status;
} /* ay_riproc_init */

