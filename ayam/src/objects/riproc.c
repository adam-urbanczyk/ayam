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

/* riproc.c - riproc (RIB-Procedural) object */

static char *ay_riproc_name = "RiProc";

/* functions: */

/* ay_riproc_createcb:
 *  create callback function of riproc object
 */
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
  o->parent = AY_TRUE;
  o->hide_children = AY_TRUE;

 return AY_OK;
} /* ay_riproc_createcb */


/* ay_riproc_deletecb:
 *  delete callback function of riproc object
 */
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


/* ay_riproc_copycb:
 *  copy callback function of riproc object
 */
int
ay_riproc_copycb(void *src, void **dst)
{
 ay_riproc_object *srcr = NULL, *riproc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  srcr = (ay_riproc_object*) src;

  if(!(riproc = malloc(sizeof(ay_riproc_object))))
    return AY_EOMEM;

  memcpy(riproc, src, sizeof(ay_riproc_object));
  riproc->file = NULL;
  riproc->data = NULL;

  /* copy file */
  if(srcr->file)
    {
      if(!(riproc->file = malloc((strlen(srcr->file)+1) * sizeof(char))))
	{
	  free(riproc);
	  return AY_EOMEM;
	}

      strcpy(riproc->file, srcr->file);
    }

  /* copy data */
  if(srcr->data)
    {
      if(!(riproc->data = malloc((strlen(srcr->data)+1) * sizeof(char))))
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


/* ay_riproc_drawcb:
 *  draw (display in an Ayam view window) callback function of riproc object
 */
int
ay_riproc_drawcb(struct Togl *togl, ay_object *o)
{
 ay_riproc_object *riproc = NULL;
 ay_object *d;

  if(!o)
    return AY_ENULL;

  riproc = (ay_riproc_object *)o->refine;

  if(!riproc)
    return AY_ENULL;

  glEnable(GL_LINE_STIPPLE);
  glLineStipple((GLint)3, (GLushort)0x5555);

  if(o->down && o->down->next)
    {
      d = o->down;
      while(d->next)
	{
	  ay_draw_object(togl, d, AY_FALSE);
	  d = d->next;
	}
    }
  else
    {
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
    }

  glDisable(GL_LINE_STIPPLE);

 return AY_OK;
} /* ay_riproc_drawcb */


/* ay_riproc_shadecb:
 *  shade (display in an Ayam view window) callback function of riproc object
 */
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


/* ay_riproc_setpropcb:
 *  set property (from Tcl to C context) callback function of riproc object
 */
int
ay_riproc_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char fname[] = "riproc_setprop";
 char *arr = "RiProcAttrData";
 Tcl_Obj *to = NULL;
 ay_riproc_object *riproc = NULL;
 const char *stemp = NULL;

  if(!interp || !o)
    return AY_ENULL;

  riproc = (ay_riproc_object *)o->refine;

  if(!riproc)
    return AY_ENULL;

  if(riproc->file)
    {
      free(riproc->file);
      riproc->file = NULL;
    }

  /* get file */
  stemp = Tcl_GetVar2(interp, arr, "File", TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY);
  if(stemp)
    {
      if(!(riproc->file = calloc(strlen(stemp)+1, sizeof(char))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return AY_ERROR;
	}
      strcpy(riproc->file, stemp);
    }

  /* get data */
  stemp = Tcl_GetVar2(interp, arr, "Data", TCL_LEAVE_ERR_MSG |
		      TCL_GLOBAL_ONLY);
  if(stemp)
    {
      if(!(riproc->data = calloc(strlen(stemp)+1, sizeof(char))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return AY_ERROR;
	}
      strcpy(riproc->data, stemp);
    }

  /* get bounding box */
  to = Tcl_GetVar2Ex(interp, arr, "MinX",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(riproc->minx));
  to = Tcl_GetVar2Ex(interp, arr, "MaxX",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(riproc->maxx));
  to = Tcl_GetVar2Ex(interp, arr, "MinY",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(riproc->miny));
  to = Tcl_GetVar2Ex(interp, arr, "MaxY",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(riproc->maxy));
  to = Tcl_GetVar2Ex(interp, arr, "MinZ",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(riproc->minz));
  to = Tcl_GetVar2Ex(interp, arr, "MaxZ",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(riproc->maxz));

  o->modified = AY_TRUE;
  ay_notify_parent();

 return AY_OK;
} /* ay_riproc_setpropcb */


/* ay_riproc_getpropcb:
 *  get property (from C to Tcl context) callback function of riproc object
 */
int
ay_riproc_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *arr = "RiProcAttrData";
 ay_riproc_object *riproc = NULL;

  if(!interp || !o)
    return AY_ENULL;

  riproc = (ay_riproc_object *)(o->refine);

  if(!riproc)
    return AY_ENULL;

  if(riproc->file)
    {
      Tcl_SetVar2(interp, arr, "File", riproc->file,
		  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    }
  else
    {
      Tcl_SetVar2(interp, arr, "File", "",
		  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    }

  if(riproc->data)
    {
      Tcl_SetVar2(interp, arr, "Data", riproc->data,
		  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    }
  else
    {
      Tcl_SetVar2(interp, arr, "Data", "",
		  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    }

  Tcl_SetVar2Ex(interp, arr, "MinX",
		Tcl_NewDoubleObj(riproc->minx),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "MaxX",
		Tcl_NewDoubleObj(riproc->maxx),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "MinY",
		Tcl_NewDoubleObj(riproc->miny),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "MaxY",
		Tcl_NewDoubleObj(riproc->maxy),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "MinZ",
		Tcl_NewDoubleObj(riproc->minz),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "MaxZ",
		Tcl_NewDoubleObj(riproc->maxz),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

 return AY_OK;
} /* ay_riproc_getpropcb */


/* ay_riproc_readcb:
 *  read (from scene file) callback function of riproc object
 */
int
ay_riproc_readcb(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_riproc_object *riproc = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(riproc = calloc(1, sizeof(ay_riproc_object))))
    { return AY_EOMEM; }

  fscanf(fileptr, "%lg\n", &riproc->minx);
  fscanf(fileptr, "%lg\n", &riproc->maxx);
  fscanf(fileptr, "%lg\n", &riproc->miny);
  fscanf(fileptr, "%lg\n", &riproc->maxy);
  fscanf(fileptr, "%lg\n", &riproc->minz);
  fscanf(fileptr, "%lg", &riproc->maxz);
  (void)fgetc(fileptr);

  ay_status = ay_read_string(fileptr, &(riproc->file));
  if(ay_status)
    {
      free(riproc);
      return ay_status;
    }
  ay_status = ay_read_string(fileptr, &(riproc->data));
  if(ay_status)
    {
      if(riproc->file)
	free(riproc->file);
      free(riproc);
      return ay_status;
    }
  o->refine = riproc;

 return AY_OK;
} /* ay_riproc_readcb */


/* ay_riproc_writecb:
 *  write (to scene file) callback function of riproc object
 */
int
ay_riproc_writecb(FILE *fileptr, ay_object *o)
{
 ay_riproc_object *riproc = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  riproc = (ay_riproc_object *)(o->refine);

  if(!riproc)
    return AY_ENULL;

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


/* ay_free:
 * helper for ay_riproc_wribcb) below.
 *
 */
RtVoid
ay_free(RtPointer data)
{
  if(data)
    free(data);
}


/* ay_riproc_wribcb:
 *  RIB export callback function of riproc object
 */
int
ay_riproc_wribcb(char *file, ay_object *o)
{
 ay_riproc_object *riproc = NULL;
 RtBound bound;
 RtString data[2] = {0};

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
      data[0] = riproc->file;
      RiProcedural((RtPointer)data, bound, RiProcDelayedReadArchive, ay_free);
      break;
    case AY_PRTRUNPROG:
      data[0] = riproc->file;
      data[1] = riproc->data;
      RiProcedural((RtPointer)data, bound, RiProcRunProgram, ay_free);
      break;
    case AY_PRTDYNLOAD:
      data[0] = riproc->file;
      data[1] = riproc->data;
      RiProcedural((RtPointer)data, bound, RiProcDynamicLoad, ay_free);
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_riproc_wribcb */


/* ay_riproc_bbccb:
 *  bounding box calculation callback function of riproc object
 */
int
ay_riproc_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_riproc_object *riproc = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  riproc = (ay_riproc_object *)o->refine;

  if(!riproc)
    return AY_ENULL;

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


/* ay_riproc_init:
 *  initialize the riproc object module
 */
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

