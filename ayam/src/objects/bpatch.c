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

/* bpatch.c - bilinear patch object */

static char *ay_bpatch_name = "BPatch";

int
ay_bpatch_createcb(int argc, char *argv[], ay_object *o)
{
 ay_bpatch_object *bpatch = NULL;
 char fname[] = "crtbpatch";

  if(!(bpatch = calloc(1, sizeof(ay_bpatch_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  bpatch->p1[0]  = -0.5;
  bpatch->p1[1]  = -0.5;

  bpatch->p2[0]  = 0.5;
  bpatch->p2[1]  = -0.5;

  bpatch->p3[0]  = 0.5;
  bpatch->p3[1]  = 0.5;

  bpatch->p4[0]  = -0.5;
  bpatch->p4[1]  = 0.5;

  o->refine = bpatch;

 return AY_OK;
} /* ay_bpatch_createcb */


int
ay_bpatch_deletecb(void *c)
{
 ay_bpatch_object *bpatch = NULL;

  if(!c)
    return AY_ENULL; 

  bpatch = (ay_bpatch_object *)(c);
   
  free(bpatch);

 return AY_OK;
} /* ay_bpatch_deletecb */


int
ay_bpatch_copycb(void *src, void **dst)
{
 ay_bpatch_object *bpatch = NULL;

  if(!(bpatch = calloc(1, sizeof(ay_bpatch_object))))
    return AY_EOMEM; 

  memcpy(bpatch, src, sizeof(ay_bpatch_object)); 

  *dst = bpatch;

 return AY_OK;
} /* ay_bpatch_copycb */


int
ay_bpatch_drawcb(struct Togl *togl, ay_object *o)
{
 ay_bpatch_object *bpatch = NULL;
 double h1[9*3], h2[9*3], w1[9*3], w2[9*3], d[3], len = 0.0;
 int i;

  if(!o)
    return AY_ENULL;

  bpatch = (ay_bpatch_object *)o->refine;

  if(!bpatch)
    return AY_ENULL;

  /* draw */
  /*
  glBegin(GL_LINE_LOOP);
   glVertex3dv((GLdouble *)(bpatch->p1));
   glVertex3dv((GLdouble *)(bpatch->p2));
   glVertex3dv((GLdouble *)(bpatch->p3));
   glVertex3dv((GLdouble *)(bpatch->p4));
  glEnd();
  */

  /* interpolate */
  memcpy(h1,bpatch->p1,3*sizeof(double));
  for(i = 1; i <= 8; i++)
    {
      memcpy(&(h1[i*3]),bpatch->p1,3*sizeof(double));
      d[0] =  bpatch->p4[0]-bpatch->p1[0];
      d[1] =  bpatch->p4[1]-bpatch->p1[1];
      d[2] =  bpatch->p4[2]-bpatch->p1[2];
      len = AY_V3LEN(d);
      if(len > 0.0)
	{
	  AY_V3SCAL(d, i/8.0);
	}
      h1[i*3]   += d[0];
      h1[i*3+1] += d[1];
      h1[i*3+2] += d[2];
    }
  memcpy(h2,bpatch->p2,3*sizeof(double));
  for(i = 1; i <= 8; i++)
    {
      memcpy(&(h2[i*3]),bpatch->p2,3*sizeof(double));
      d[0] =  bpatch->p3[0]-bpatch->p2[0];
      d[1] =  bpatch->p3[1]-bpatch->p2[1];
      d[2] =  bpatch->p3[2]-bpatch->p2[2];
      len = AY_V3LEN(d);
      if(len > 0.0)
	{
	  AY_V3SCAL(d, i/8.0);
	}
      h2[i*3]   += d[0];
      h2[i*3+1] += d[1];
      h2[i*3+2] += d[2];
    }

  memcpy(w1,bpatch->p1,3*sizeof(double));
  for(i = 1; i <= 8; i++)
    {
      memcpy(&(w1[i*3]),bpatch->p1,3*sizeof(double));
      d[0] =  bpatch->p2[0]-bpatch->p1[0];
      d[1] =  bpatch->p2[1]-bpatch->p1[1];
      d[2] =  bpatch->p2[2]-bpatch->p1[2];
      len = AY_V3LEN(d);
      if(len > 0.0)
	{
	  AY_V3SCAL(d, i/8.0);
	}
      w1[i*3]   += d[0];
      w1[i*3+1] += d[1];
      w1[i*3+2] += d[2];
    }
  memcpy(w2,bpatch->p4,3*sizeof(double));
  for(i = 1; i <= 8; i++)
    {
      memcpy(&(w2[i*3]),bpatch->p4,3*sizeof(double));
      d[0] =  bpatch->p3[0]-bpatch->p4[0];
      d[1] =  bpatch->p3[1]-bpatch->p4[1];
      d[2] =  bpatch->p3[2]-bpatch->p4[2];
      len = AY_V3LEN(d);
      if(len > 0.0)
	{
	  AY_V3SCAL(d, i/8.0);
	}
      w2[i*3]   += d[0];
      w2[i*3+1] += d[1];
      w2[i*3+2] += d[2];
    }

  glBegin(GL_LINES);
  for(i = 0; i <= 8; i++)
    {
      glVertex3dv((GLdouble*)(&(h1[i*3])));
      glVertex3dv((GLdouble*)(&(h2[i*3])));
      glVertex3dv((GLdouble*)(&(w1[i*3])));
      glVertex3dv((GLdouble*)(&(w2[i*3])));
    }
  glEnd();

 return AY_OK;
} /* ay_bpatch_drawcb */


int
ay_bpatch_drawpntcb(struct Togl *togl, ay_object *o)
{
 ay_bpatch_object *bpatch = NULL;

  if(!o)
    return AY_ENULL;

  bpatch = (ay_bpatch_object *)o->refine;

  if(!bpatch)
    return AY_ENULL;

  /* draw */
  glBegin(GL_POINTS);
   glVertex3dv((GLdouble *)(bpatch->p1));
   glVertex3dv((GLdouble *)(bpatch->p2));
   glVertex3dv((GLdouble *)(bpatch->p3));
   glVertex3dv((GLdouble *)(bpatch->p4));
  glEnd();

 return AY_OK;
} /* ay_bpatch_drawpntcb */


int
ay_bpatch_shadecb(struct Togl *togl, ay_object *o)
{
 ay_bpatch_object *bpatch = NULL;
 double n[3] = {0};

  if(!o)
    return AY_ENULL;

  bpatch = (ay_bpatch_object *)o->refine;

  if(!bpatch)
    return AY_ENULL;

  glBegin(GL_QUADS);
   /*  XXXX should we cache the normals? */
   ay_geom_calcnfrom3(bpatch->p4, bpatch->p1, bpatch->p2, n);
   glNormal3dv((GLdouble *)n);
   glVertex3dv((GLdouble *)(bpatch->p1));
   ay_geom_calcnfrom3(bpatch->p1, bpatch->p2, bpatch->p3, n);
   glNormal3dv((GLdouble *)n);
   glVertex3dv((GLdouble *)(bpatch->p2));
   ay_geom_calcnfrom3(bpatch->p2, bpatch->p3, bpatch->p4, n);
   glNormal3dv((GLdouble *)n);
   glVertex3dv((GLdouble *)(bpatch->p3));
   ay_geom_calcnfrom3(bpatch->p3, bpatch->p4, bpatch->p1, n);
   glNormal3dv((GLdouble *)n);
   glVertex3dv((GLdouble *)(bpatch->p4));
  glEnd();

 return AY_OK;
} /* ay_bpatch_shadecb */


int
ay_bpatch_getpntcb(ay_object *o, double *p)
{
 ay_bpatch_object *bpatch = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoords = NULL;

  if(!o || !p)
    return AY_ENULL;

  bpatch = (ay_bpatch_object *)(o->refine);

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(ay_point_edit_coords)
    free(ay_point_edit_coords);

  ay_point_edit_coords = NULL;

  /* select all points? */
  if((p[0] == DBL_MIN) && (p[1] == DBL_MIN) && (p[2] == DBL_MIN))
    { /* yes */
      if(!(ay_point_edit_coords = calloc(4, sizeof(double*))))
	return AY_EOMEM;

      ay_point_edit_coords[0] = bpatch->p1;
      ay_point_edit_coords[1] = bpatch->p2;
      ay_point_edit_coords[2] = bpatch->p3;
      ay_point_edit_coords[3] = bpatch->p4;

      ay_point_edit_coords_homogenous = AY_FALSE;
      ay_point_edit_coords_number = 4;
    }
  else
    { /* no */
      dist = AY_VLEN((p[0] - bpatch->p1[0]),
		     (p[1] - bpatch->p1[1]),
		     (p[2] - bpatch->p1[2]));

      if(dist < min_dist)
	{
	  pecoords = bpatch->p1;
	  min_dist = dist;
	}

      dist = AY_VLEN((p[0] - bpatch->p2[0]),
		     (p[1] - bpatch->p2[1]),
		     (p[2] - bpatch->p2[2]));

      if(dist < min_dist)
	{
	  pecoords = bpatch->p2;
	  min_dist = dist;
	}

      dist = AY_VLEN((p[0] - bpatch->p3[0]),
		     (p[1] - bpatch->p3[1]),
		     (p[2] - bpatch->p3[2]));

      if(dist < min_dist)
	{
	  pecoords = bpatch->p3;
	  min_dist = dist;
	}

      dist = AY_VLEN((p[0] - bpatch->p4[0]),
		     (p[1] - bpatch->p4[1]),
		     (p[2] - bpatch->p4[2]));

      if(dist < min_dist)
	{
	  pecoords = bpatch->p4;
	  min_dist = dist;
	}

      if(!pecoords)
	return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

      if(!(ay_point_edit_coords = calloc(1, sizeof(double*))))
	return AY_EOMEM;

      ay_point_edit_coords[0] = pecoords;
      ay_point_edit_coords_homogenous = AY_FALSE;
      ay_point_edit_coords_number = 1;
    }

 return AY_OK;
} /* ay_bpatch_getpntcb */


/* Tcl -> C! */
int
ay_bpatch_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1 = "BPatchAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_bpatch_object *bpatch = NULL;

  if(!o)
    return AY_ENULL;

  bpatch = (ay_bpatch_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("P1_X",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p1[0]);

  Tcl_SetStringObj(ton,"P1_Y",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p1[1]);

  Tcl_SetStringObj(ton,"P1_Z",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p1[2]);


  Tcl_SetStringObj(ton,"P2_X",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p2[0]);

  Tcl_SetStringObj(ton,"P2_Y",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p2[1]);

  Tcl_SetStringObj(ton,"P2_Z",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p2[2]);


  Tcl_SetStringObj(ton,"P3_X",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p3[0]);

  Tcl_SetStringObj(ton,"P3_Y",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p3[1]);

  Tcl_SetStringObj(ton,"P3_Z",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p3[2]);


  Tcl_SetStringObj(ton,"P4_X",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p4[0]);

  Tcl_SetStringObj(ton,"P4_Y",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p4[1]);

  Tcl_SetStringObj(ton,"P4_Z",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &bpatch->p4[2]);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_bpatch_setpropcb */


/* C -> Tcl! */
int
ay_bpatch_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="BPatchAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_bpatch_object *bpatch = NULL;

  if(!o)
    return AY_ENULL;

  bpatch = (ay_bpatch_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("P1_X",-1);
  to = Tcl_NewDoubleObj(bpatch->p1[0]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P1_Y",-1);
  to = Tcl_NewDoubleObj(bpatch->p1[1]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P1_Z",-1);
  to = Tcl_NewDoubleObj(bpatch->p1[2]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton,"P2_X",-1);
  to = Tcl_NewDoubleObj(bpatch->p2[0]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P2_Y",-1);
  to = Tcl_NewDoubleObj(bpatch->p2[1]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P2_Z",-1);
  to = Tcl_NewDoubleObj(bpatch->p2[2]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton,"P3_X",-1);
  to = Tcl_NewDoubleObj(bpatch->p3[0]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P3_Y",-1);
  to = Tcl_NewDoubleObj(bpatch->p3[1]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P3_Z",-1);
  to = Tcl_NewDoubleObj(bpatch->p3[2]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton,"P4_X",-1);
  to = Tcl_NewDoubleObj(bpatch->p4[0]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P4_Y",-1);
  to = Tcl_NewDoubleObj(bpatch->p4[1]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"P4_Z",-1);
  to = Tcl_NewDoubleObj(bpatch->p4[2]);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_bpatch_getpropcb */


int
ay_bpatch_readcb(FILE *fileptr, ay_object *o)
{
 ay_bpatch_object *bp = NULL;

  if(!o)
   return AY_ENULL;

  if(!(bp = calloc(1, sizeof(ay_bpatch_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%lg %lg %lg\n", &bp->p1[0], &bp->p1[1], &bp->p1[2]);
  fscanf(fileptr,"%lg %lg %lg\n", &bp->p2[0], &bp->p2[1], &bp->p2[2]);
  fscanf(fileptr,"%lg %lg %lg\n", &bp->p3[0], &bp->p3[1], &bp->p3[2]);
  fscanf(fileptr,"%lg %lg %lg\n", &bp->p4[0], &bp->p4[1], &bp->p4[2]);

  o->refine = bp;

 return AY_OK;
} /* ay_bpatch_readcb */


int
ay_bpatch_writecb(FILE *fileptr, ay_object *o)
{
 ay_bpatch_object *bpatch = NULL;

  if(!o)
    return AY_ENULL;

  bpatch = (ay_bpatch_object *)(o->refine);

  fprintf(fileptr, "%g %g %g\n", bpatch->p1[0], bpatch->p1[1], bpatch->p1[2]);
  fprintf(fileptr, "%g %g %g\n", bpatch->p2[0], bpatch->p2[1], bpatch->p2[2]);
  fprintf(fileptr, "%g %g %g\n", bpatch->p3[0], bpatch->p3[1], bpatch->p3[2]);
  fprintf(fileptr, "%g %g %g\n", bpatch->p4[0], bpatch->p4[1], bpatch->p4[2]);

 return AY_OK;
} /* ay_bpatch_writecb */


int
ay_bpatch_wribcb(char *file, ay_object *o)
{
 ay_bpatch_object *bpatch = NULL;
 RtPoint rect[4];

  if(!o)
   return AY_ENULL;

  bpatch = (ay_bpatch_object*)o->refine;

  /* XXXX order correct for RIB? */
  rect[0][0] = (RtFloat)(bpatch->p1[0]);
  rect[0][1] = (RtFloat)(bpatch->p1[1]);
  rect[0][2] = (RtFloat)(bpatch->p1[2]);

  rect[1][0] = (RtFloat)(bpatch->p2[0]);
  rect[1][1] = (RtFloat)(bpatch->p2[1]);
  rect[1][2] = (RtFloat)(bpatch->p2[2]);

  rect[2][0] = (RtFloat)(bpatch->p4[0]);
  rect[2][1] = (RtFloat)(bpatch->p4[1]);
  rect[2][2] = (RtFloat)(bpatch->p4[2]);

  rect[3][0] = (RtFloat)(bpatch->p3[0]);
  rect[3][1] = (RtFloat)(bpatch->p3[1]);
  rect[3][2] = (RtFloat)(bpatch->p3[2]);

  RiPatch(RI_BILINEAR, RI_P, (RtPointer)rect, RI_NULL);

 return AY_OK;
} /* ay_bpatch_wribcb */


int
ay_bpatch_bbccb(ay_object *o, double *bbox, int *flags)
{
 double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX;
 double ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;
 double cv[12] = {0};
 int i = 0, a = 0;
 ay_bpatch_object *b = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  b = (ay_bpatch_object *)o->refine; 

  memcpy(&(cv[0]), b->p1, 3*sizeof(double));
  memcpy(&(cv[3]), b->p2, 3*sizeof(double));
  memcpy(&(cv[6]), b->p3, 3*sizeof(double));
  memcpy(&(cv[9]), b->p4, 3*sizeof(double));

  for(i = 0; i < 4; i++)
    {
      if(cv[a] < xmin)
	xmin = cv[a];
      if(cv[a] > xmax)
	xmax = cv[a];

      if(cv[a+1] < ymin)
	ymin = cv[a+1];
      if(cv[a+1] > ymax)
	ymax = cv[a+1];

      if(cv[a+2] < zmin)
	zmin = cv[a+2];
      if(cv[a+2] > zmax)
	zmax = cv[a+2];

      a += 3;
    } /* for */

  /* P1 */
  bbox[0] = xmin; bbox[1] = ymax; bbox[2] = zmax;
  /* P2 */
  bbox[3] = xmin; bbox[4] = ymax; bbox[5] = zmin;
  /* P3 */
  bbox[6] = xmax; bbox[7] = ymax; bbox[8] = zmin;
  /* P4 */
  bbox[9] = xmax; bbox[10] = ymax; bbox[11] = zmax;

  /* P5 */
  bbox[12] = xmin; bbox[13] = ymin; bbox[14] = zmax;
  /* P6 */
  bbox[15] = xmin; bbox[16] = ymin; bbox[17] = zmin;
  /* P7 */
  bbox[18] = xmax; bbox[19] = ymin; bbox[20] = zmin;
  /* P8 */
  bbox[21] = xmax; bbox[22] = ymin; bbox[23] = zmax;

 return AY_OK;
} /* ay_bpatch_bbccb */


int
ay_bpatch_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

 ay_status = ay_otype_registercore(ay_bpatch_name,
				   ay_bpatch_createcb,
				   ay_bpatch_deletecb,
				   ay_bpatch_copycb,
				   ay_bpatch_drawcb,
				   ay_bpatch_drawpntcb,
				   ay_bpatch_shadecb,
				   ay_bpatch_setpropcb,
				   ay_bpatch_getpropcb,
				   ay_bpatch_getpntcb,
				   ay_bpatch_readcb,
				   ay_bpatch_writecb,
				   ay_bpatch_wribcb,
				   ay_bpatch_bbccb,
				   AY_IDBPATCH);

  Tcl_SetVar(interp,"propertyList","BPatchAttr", TCL_APPEND_VALUE |
	     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


 return ay_status;
} /* ay_bpatch_init */
