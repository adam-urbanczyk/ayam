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


/* metaobj.c:
 * Meta Object by Frank Pagels 2001
 * www.coplabs.org
 */

#include "ayam.h"
#include "meta.h"

/* metaobj.c - metaobj custom object */

static char *metaobj_name = "MetaObj";
static unsigned int metaobj_id;
static char *metacomp_name = "MetaComp";
static unsigned int metacomp_id;

#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int Metacomp_Init (Tcl_Interp *interp);
int metaobj_notifycb (ay_object *o);

#define META_VERSION 3

int
metaobj_createcb (int argc, char *argv[], ay_object * o)
{
 int ay_status = AY_OK;
 meta_world *w = NULL;
 char fname[] = "crtball";
 ay_object *first_child = NULL;

  if (!(w = calloc (1, sizeof (meta_world))))
    {
      ay_error (AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  w->maxpoly = META_MAXPOLY;

  if (!
      (w->vertex =
       (double *) calloc (1, sizeof (double) * 3 * 3 * (w->maxpoly + 20))))
    {
      if (w)
	free(w);

      ay_error (AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  if (!
      (w->nvertex =
       (double *) calloc (1, sizeof (double) * 3 * 3 * (w->maxpoly + 20))))
    {
      if (w)
	free(w);

      ay_error (AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  w->aktcubes = META_MAXCUBE;	/* erstmal maximale anzahl */

  if (!
      (w->mgrid =
       (short *) calloc (1, sizeof (short) * META_MAXCUBE * META_MAXCUBE *
			 META_MAXCUBE)))
    {
      if (w->Vertex3d)
	free(w->Vertex3d);

      if (w)
	free(w);

      ay_error (AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }


#if META_USEVERTEXARRAY

  w->tablesize = 40000;

  if (!
      (w->vindex =
       (GLuint *) calloc (1, sizeof (GLuint) * ((w->tablesize-1) +
						(w->tablesize/10-1) +
						(w->tablesize/100-1)))))
    {
      if (w->Vertex3d)
	free(w->Vertex3d);

      if (w)
	free(w);

      if (w->mgrid)
	 free(w->mgrid);

      ay_error (AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

   (w->vhash =
       (int *) calloc (1, sizeof (int) * ((w->tablesize-1) +
					  (w->tablesize/10-1) +
					  (w->tablesize/100-1))));

#endif


  meta_initcubestack (w);

  w->lastmark = 0;

  w->isolevel = 0.6;
  w->unisize = 4L;

  o->parent = AY_TRUE;
  o->refine = w;

  w->edgelength = w->unisize / (double) w->aktcubes;

  /* Init first Metacomp */

  ay_status = ay_object_create (metacomp_id, &first_child);

  o->down = first_child;

  ay_status = ay_object_crtendlevel (&(first_child->next));

  w->currentnumpoly = 0;
  w->o = o->down;

  w->version = META_VERSION;

  w->adapt = 0;
  w->flatness = 0.9;
  w->epsilon = 0.001;
  w->step = 0.001;

  meta_calceffect (w);

 return AY_OK;
} /* metaobj_createcb */


int
metaobj_deletecb (void *c)
{
  meta_world *w;

  if (!c)
    return AY_ENULL;

  w = (meta_world *) (c);

  if (w->vertex)
    free(w->vertex);

  if (w->nvertex)
    free(w->nvertex);


  if (w->mgrid)
  free(w->mgrid);

  meta_freecubestack (w);

  if (w)
    free(w);

#if META_USEVERTEXARRAY
  if ( w->vindex)
    free(w->vindex);
  if (w->vhash)
    free(w->vhash);
#endif

 return AY_OK;
} /* metaobj_deletecb */


int
metaobj_copycb (void *src, void **dst)
{
 meta_world *w = NULL;
 meta_world *wsrc = NULL;

  if (!src || !dst)
    return AY_ENULL;

  wsrc = (meta_world *) src;

  if (!(w = calloc (1, sizeof (meta_world))))
    return AY_EOMEM;

  memcpy (w, src, sizeof (meta_world));

  if (!
      (w->vertex =
       (double *) calloc (1, sizeof (double) * 3 * 3 * (w->maxpoly + 20))))
    {
      if (w)
	free(w);

      return AY_ERROR;
    }

  memcpy (w->vertex, wsrc->vertex,
	  sizeof (double) * 3 * 3 * (w->maxpoly + 20));

  if (!
      (w->nvertex =
       (double *) calloc (1, sizeof (double) * 3 * 3 * (w->maxpoly + 20))))
    {
      if (w)
	free(w);

      return AY_ERROR;
    }


  memcpy (w->nvertex, wsrc->nvertex,
	  sizeof (double) * 3 * 3 * (w->maxpoly + 20));


  if (!
      (w->mgrid =
       (short *) calloc (1,
			sizeof (short) * w->aktcubes * w->aktcubes *
			w->aktcubes)))
    {
      free(w->Vertex3d);
      free(w);
      return AY_EOMEM;
    }


#if META_USEVERTEXARRAY

  if (!
      (w->vindex =
       (GLuint *) calloc (1, sizeof (GLuint) * ((w->tablesize-1) +
						(w->tablesize/10-1) +
						(w->tablesize/100-1)))))
    {
      if (w->Vertex3d)
	free(w->Vertex3d);

      if (w)
	free(w);

      if (w->mgrid)
	 free(w->mgrid);

      ay_error (AY_EOMEM, NULL, NULL);
      return AY_ERROR;
    }

   (w->vhash =
       (int *) calloc (1, sizeof (int) * ((w->tablesize-1) +
					  (w->tablesize/10-1) +
					  (w->tablesize/100-1))));

#endif

  meta_initcubestack (w);

  *dst = (void *) w;

 return AY_OK;
} /* metaobj_copycb */


int
metaobj_drawcb (struct Togl *togl, ay_object * o)
{
 meta_world *w;

#if !META_USEVERTEXARRAY
  double x, y, z, x1, y1, z1, x2, y2, z2;
  int i;
#endif

  double *vptr;

  w = (meta_world *) o->refine;

  vptr = w->vertex;

#if META_USEVERTEXARRAY

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_DOUBLE, 0, w->vertex);

  glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

  glDrawElements(GL_TRIANGLES, w->currentnumpoly*3, GL_UNSIGNED_INT, w->vindex);

#endif

  if (w->showworld)
    {
      double u;
      u = w->unisize/2;

      glBegin (GL_LINE_STRIP);
       glVertex3d (-u, +u, +u);
       glVertex3d (+u, +u, +u);
       glVertex3d (+u, +u, -u);
       glVertex3d (-u, +u, -u);
       glVertex3d (-u, +u, +u);
       glVertex3d (-u, -u, +u);
       glVertex3d (+u, -u, +u);
       glVertex3d (+u, -u, -u);
       glVertex3d (-u, -u, -u);
       glVertex3d (-u, -u, +u);
      glEnd();

      glBegin (GL_LINES);
       glVertex3d (+u, +u, +u);
       glVertex3d (+u, -u, +u);
       glVertex3d (+u, +u, -u);
       glVertex3d (+u, -u, -u);
       glVertex3d (-u, +u, -u);
       glVertex3d (-u, -u, -u);
      glEnd();

      glBegin (GL_POINTS);
       glVertex3d (+u, +u, +u);
       glVertex3d (+u, +u, -u);
       glVertex3d (+u, -u, +u);
       glVertex3d (+u, -u, -u);
       glVertex3d (-u, +u, +u);
       glVertex3d (-u, +u, -u);
       glVertex3d (-u, -u, +u);
       glVertex3d (-u, -u, -u);
      glEnd();
    } /* if */


#if !META_USEVERTEXARRAY

  glBegin (GL_LINES);

  for (i = 0; i < w->currentnumpoly; i++)
    {
      x = *vptr++;
      y = *vptr++;
      z = *vptr++;
      x1 = *vptr++;
      y1 = *vptr++;
      z1 = *vptr++;
      x2 = *vptr++;
      y2 = *vptr++;
      z2 = *vptr++;

      glVertex3d (x, y, z);
      glVertex3d (x1, y1, z1);

      glVertex3d (x1, y1, z1);
      glVertex3d (x2, y2, z2);

      glVertex3d (x, y, z);
      glVertex3d (x2, y2, z2);
    }

  glEnd ();

#endif


 return AY_OK;
} /* metaobj_drawcb */


int
metaobj_shadecb (struct Togl *togl, ay_object *o)
{
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 meta_world *w;
 double *vptr, *nptr, n[3];
 int i;

  w = (meta_world *) o->refine;

  vptr = w->vertex;
  nptr = w->nvertex;

#if META_USEVERTEXARRAY

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glVertexPointer(3, GL_DOUBLE, 0, w->vertex);
  glNormalPointer(GL_DOUBLE, 0, w->nvertex);

  glDrawElements(GL_TRIANGLES, w->currentnumpoly*3, GL_UNSIGNED_INT,
		 w->vindex);

#endif

#if !META_USEVERTEXARRAY

  glBegin(GL_TRIANGLES);
  if(view->altdispcb)
    {
      for(i = 0; i < w->currentnumpoly; i++)
	{
	  memcpy(n, nptr, 3*sizeof(double));
	  n[0] *= -1.0;
	  n[1] *= -1.0;
	  n[2] *= -1.0;
	  glNormal3dv ((GLdouble *) &n);
	  nptr += 6;
	  glVertex3dv ((GLdouble *) vptr);
	  vptr += 6;

	  memcpy(n, nptr, 3*sizeof(double));
	  n[0] *= -1.0;
	  n[1] *= -1.0;
	  n[2] *= -1.0;
	  glNormal3dv ((GLdouble *) &n);
	  nptr -= 3;
	  glVertex3dv ((GLdouble *) vptr);
	  vptr -= 3;

	  memcpy(n, nptr, 3*sizeof(double));
	  n[0] *= -1.0;
	  n[1] *= -1.0;
	  n[2] *= -1.0;
	  glNormal3dv ((GLdouble *) &n);
	  nptr += 6;
	  glVertex3dv ((GLdouble *) vptr);
	  vptr += 6;

	} /* for */
    }
  else
    {
      for (i = 0; i < w->currentnumpoly; i++)
	{

	  glNormal3dv ((GLdouble *) nptr);
	  nptr += 3;
	  glVertex3dv ((GLdouble *) vptr);
	  vptr += 3;
	  glNormal3dv ((GLdouble *) nptr);
	  nptr += 3;
	  glVertex3dv ((GLdouble *) vptr);
	  vptr += 3;
	  glNormal3dv ((GLdouble *) nptr);
	  nptr += 3;
	  glVertex3dv ((GLdouble *) vptr);
	  vptr += 3;

	} /* for */
    } /* if */

  glEnd();
#endif

 return AY_OK;
} /* metaobj_shadecb */


/* Tcl -> C! */
int
metaobj_setpropcb (Tcl_Interp * interp, int argc, char *argv[], ay_object * o)
{
 char *n1 = "MetaObjAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 meta_world *w = NULL;

  if (!o)
    return AY_ENULL;

  w = (meta_world *) o->refine;

  toa = Tcl_NewStringObj (n1, -1);
  ton = Tcl_NewStringObj (n1, -1);

  Tcl_SetStringObj (ton, "NumSamples", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj (interp, to, &w->aktcubes);

  Tcl_SetStringObj (ton, "IsoLevel", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj (interp, to, &w->isolevel);

  Tcl_SetStringObj (ton, "ShowWorld", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj (interp, to, &w->showworld);

  Tcl_SetStringObj (ton, "Adaptive", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj (interp, to, &w->adapt);

  Tcl_SetStringObj (ton, "Flatness", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj (interp, to, &w->flatness);

  Tcl_SetStringObj (ton, "Epsilon", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj (interp, to, &w->epsilon);

  Tcl_SetStringObj (ton, "StepSize", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj (interp, to, &w->step);

  Tcl_IncrRefCount (toa);
  Tcl_DecrRefCount (toa);
  Tcl_IncrRefCount (ton);
  Tcl_DecrRefCount (ton);

  if (w->aktcubes < 5)
    {
      w->aktcubes = 5;
    }

  w->edgelength = w->unisize / (double) w->aktcubes;

  if (w->mgrid)
    free(w->mgrid);

  if (!(w->mgrid = (short *) calloc (1, sizeof (short) *
				     w->aktcubes * w->aktcubes * w->aktcubes)))
    return AY_EOMEM;

  metaobj_notifycb (o);

 return AY_OK;
} /* metaobj_setpropcb */


/* C -> Tcl! */
int
metaobj_getpropcb (Tcl_Interp * interp, int argc, char *argv[], ay_object * o)
{
 char *n1 = "MetaObjAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 meta_world *w = NULL;

  if (!o)
    return AY_ENULL;

  w = (meta_world *) (o->refine);

  toa = Tcl_NewStringObj (n1, -1);

  ton = Tcl_NewStringObj (n1, -1);

  Tcl_SetStringObj (ton, "NumSamples", -1);
  to = Tcl_NewIntObj (w->aktcubes);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "IsoLevel", -1);
  to = Tcl_NewDoubleObj (w->isolevel);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "ShowWorld", -1);
  to = Tcl_NewIntObj (w->showworld);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "Adaptive", -1);
  to = Tcl_NewIntObj (w->adapt);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "Flatness", -1);
  to = Tcl_NewDoubleObj (w->flatness);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  if (w->flatness > 0.99f)
    {
      w->flatness = 0.99;
    }

  Tcl_SetStringObj (ton, "Epsilon", -1);
  to = Tcl_NewDoubleObj (w->epsilon);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "StepSize", -1);
  to = Tcl_NewDoubleObj (w->step);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount (toa);
  Tcl_DecrRefCount (toa);
  Tcl_IncrRefCount (ton);
  Tcl_DecrRefCount (ton);

 return AY_OK;
} /* metaobj_getpropcb */


int
metaobj_readcb (FILE * fileptr, ay_object * o)
{
 meta_world *w;

  if (!o)
    return AY_ENULL;

  if (!(w = calloc (1, sizeof (meta_world))))
    return AY_ERROR;

  w->version = 1;

  fscanf (fileptr, "%d\n", &w->aktcubes);
  fscanf (fileptr, "%lg\n", &w->isolevel);

  w->adapt = 0;
  w->flatness = 0.9;
  w->epsilon = 0.001;
  w->step = 0.001;

  if (ay_read_version >= 3)
    {
      fscanf (fileptr, "%d\n", &w->version);

      if (w->version >= 3)
	{
	  fscanf (fileptr, "%d\n", &w->adapt);
	  fscanf (fileptr, "%lg\n", &w->flatness);
	  fscanf (fileptr, "%lg\n", &w->epsilon);
	  fscanf (fileptr, "%lg\n", &w->step);
	}
    }

  w->maxpoly = 10000;

  if (!
      (w->vertex =
       (double *) calloc (1, sizeof (double) * 3 * 3 * (10000 + 20))))
    {
      if (w)
	free(w);
      return AY_ERROR;
    }

  if (!
      (w->nvertex =
       (double *) calloc (1, sizeof (double) * 3 * 3 * (10000 + 20))))
    {
      if (w)
	free(w);
      return AY_ERROR;
    }

  if (!
      (w->mgrid =
       (short *) calloc (1,
			sizeof (short) * w->aktcubes * w->aktcubes *
			w->aktcubes)))
    {
      free(w->vertex);
      free(w->nvertex);
      free(w);
      return AY_EOMEM;
    }

#if META_USEVERTEXARRAY

  w->tablesize = 40000;

  if (!
      (w->vindex =
       (GLuint *) calloc (1, sizeof (GLuint) * ((w->tablesize-1) +
						(w->tablesize/10-1) +
						(w->tablesize/100-1)))))
    {
      if (w->Vertex3d)
	free(w->Vertex3d);

      if (w)
	free(w);

      if (w->mgrid)
	free(w->mgrid);

      ay_error (AY_EOMEM, NULL, NULL);
      return AY_ERROR;
    }

   (w->vhash =
       (int *) calloc (1, sizeof (int) * ((w->tablesize-1) +
					  (w->tablesize/10-1) +
					  (w->tablesize/100-1))));

#endif

  w->unisize = 4L;
  w->edgelength = w->unisize / (double) w->aktcubes;

  meta_initcubestack (w);

  o->parent = AY_TRUE;
  o->refine = w;

 return AY_OK;
} /* metaobj_readcb */


int
metaobj_writecb (FILE * fileptr, ay_object * o)
{
 meta_world *w;

  if (!o)
    return AY_ENULL;

  w = (meta_world *) (o->refine);

  fprintf (fileptr, "%d\n", w->aktcubes);
  fprintf (fileptr, "%g\n", w->isolevel);
  fprintf (fileptr, "%d\n", w->version);

  fprintf (fileptr, "%d\n", w->adapt);
  fprintf (fileptr, "%g\n", w->flatness);
  fprintf (fileptr, "%g\n", w->epsilon);
  fprintf (fileptr, "%g\n", w->step);

 return AY_OK;
} /* metaobj_writecb */


int
metaobj_wribcb (char *file, ay_object * o)
{
 meta_world *w;
 RtPoint points[3];
 RtPoint normals[3];
 double *vptr, *nptr;
 int i, j;

  if (!o)
    return AY_ENULL;

  w = (meta_world *) o->refine;

  vptr = w->vertex;
  nptr = w->nvertex;

  RiSolidBegin (RI_PRIMITIVE);

  for (i = 0; i < w->currentnumpoly; i++)
    {
      for (j = 0; j < 3; j++)
	{
	  normals[j][0] = (RtFloat) * nptr++;
	  normals[j][1] = (RtFloat) * nptr++;
	  normals[j][2] = (RtFloat) * nptr++;
	  points[j][0] = (RtFloat) * vptr++;
	  points[j][1] = (RtFloat) * vptr++;
	  points[j][2] = (RtFloat) * vptr++;
	} /* for */

      RiPolygon (3, "P", (RtPointer) points, "N", (RtPointer) normals,
		 RI_NULL);

    } /* for */

  RiSolidEnd ();

 return AY_OK;
} /* metaobj_wribcb */


int
metaobj_bbccb (ay_object *o, double *bbox, int *flags)
{
 meta_world *w;
 double xmin, xmax, ymin, ymax, zmin, zmax;
 double *controlv = NULL;
 int i, a, stride = 3;

  if(!o || !bbox)
    {
      return AY_ENULL;
    }

  w = (meta_world *) o->refine;

  controlv = w->vertex;
  if(!controlv)
    {
      return AY_ERROR;
    }

  xmin = controlv[0];
  xmax = xmin;
  ymin = controlv[1];
  ymax = ymin;
  zmin = controlv[2];
  zmax = zmin;

  a = 0;
  for(i = 0; i < w->currentnumpoly; i++)
    {
      if(controlv[a] < xmin)
	xmin = controlv[a];
      if(controlv[a] > xmax)
	xmax = controlv[a];

      if(controlv[a+1] < ymin)
	ymin = controlv[a+1];
      if(controlv[a+1] > ymax)
	ymax = controlv[a+1];

      if(controlv[a+2] < zmin)
	zmin = controlv[a+2];
      if(controlv[a+2] > zmax)
	zmax = controlv[a+2];

      a += stride;
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
} /* metaobj_bbccb */


int
metaobj_notifycb (ay_object *o)
{
 meta_world *w;
 meta_blob *b;
 ay_object *down;
 char *adapt;
 char vname[] = "ay";
 char vname1[] = "action";
 double euler[3] = {0};

  down = o->down;

  while (down->next != NULL)
    {
      if (down->type == metacomp_id)
	{
	  b = (meta_blob *) down->refine;

	  ay_trafo_identitymatrix(b->rm);
	  ay_trafo_translatematrix(down->movx, down->movy, down->movz, b->rm);
	  ay_quat_toeuler(down->quat, euler);
	  ay_trafo_rotatematrix(AY_R2D(euler[2]), 1, 0, 0, b->rm);
	  ay_trafo_rotatematrix(AY_R2D(euler[1]), 0, 1, 0, b->rm);
	  ay_trafo_rotatematrix(AY_R2D(euler[0]), 0, 0, 1, b->rm);
	  ay_trafo_translatematrix(-down->movx, -down->movy, -down->movz,
				   b->rm);
	  b->cp.x = b->p.x + down->movx;
	  b->cp.y = b->p.y + down->movy;
	  b->cp.z = b->p.z + down->movz;

	  b->scalex = 1 / (down->scalx < 0.00001 ? 0.00001 : down->scalx);
	  b->scaley = 1 / (down->scaly < 0.00001 ? 0.00001 : down->scaly);
	  b->scalez = 1 / (down->scalz < 0.00001 ? 0.00001 : down->scalz);
	} /* if */

      down = down->next;
    } /* while */

  w = (meta_world *) o->refine;

  w->currentnumpoly = 0;
  w->o = o->down;

  adapt = Tcl_GetVar2(ay_interp, vname, vname1, TCL_GLOBAL_ONLY);

  w->adaptflag = 0;

  if (w->adapt)
    {
      if (w->adapt == 1)
	w->adaptflag = 1;
      else
	{
	  if (*adapt == '0')
	    {
	      w->adaptflag = 1;
	    }
	  else
	    {
	      w->adaptflag = 0;
	    }
	}
    }

  meta_calceffect (w);

 return AY_OK;
} /* metaobj_notifycb */


int
metaobj_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 ay_object *new = NULL;
 ay_pomesh_object *po = NULL;
 meta_world *mw = NULL;
 int i,p,ii,j;
 int ay_status = AY_OK;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDPOMESH)
	return AY_OK;
      else
	return AY_ERROR;
    }

  mw = (meta_world *) o->refine;

  if (type == AY_IDPOMESH)
     {

       if (!(new = calloc(1, sizeof(ay_object))))
	 return AY_EOMEM;

       new->type = AY_IDPOMESH;
       ay_object_defaults (new);

       p = mw->currentnumpoly;

       if (!(po = (ay_pomesh_object *) calloc(1, sizeof(ay_pomesh_object))))
	 return AY_EOMEM;

       po->npolys = p;
       po->nloops = (unsigned int *)calloc(1,sizeof(unsigned int)*p);
       po->nverts = (unsigned int *)calloc(1,sizeof(unsigned int)*p);
       po->verts = (unsigned int *)calloc(1,sizeof(unsigned int)*p*3);
       po->ncontrols = p*3;
       po->controlv = (double *)calloc(1,po->ncontrols*6*sizeof(double));
       po->has_normals = AY_TRUE;

       if (!(po->nloops && po->verts && po->controlv))
	 {
	   if (po->nloops)
	     free(po->nloops);

	   if (po->nverts)
	     free(po->nverts);

	   if (po->controlv)
	     free(po->controlv);

	   return AY_EOMEM;
	 }

       for (i = 0; i<p; i++)
	 {
	   po->nloops[i] = 1;
	   po->nverts[i] = 3;
	   ii = i*3;
	   po->verts[ii] = ii;
	   po->verts[ii+1] = ii+1;
	   po->verts[ii+2] = ii+2;
	   j = ii*6;
	   ii *= 3;
	   po->controlv[j] = mw->vertex[ii];
	   po->controlv[j+1] = mw->vertex[ii+1];
	   po->controlv[j+2] = mw->vertex[ii+2];

	   po->controlv[j+3] = mw->nvertex[ii];
	   po->controlv[j+4] = mw->nvertex[ii+1];
	   po->controlv[j+5] = mw->nvertex[ii+2];

	   po->controlv[j+6] = mw->vertex[ii+3];
	   po->controlv[j+7] = mw->vertex[ii+4];
	   po->controlv[j+8] = mw->vertex[ii+5];

	   po->controlv[j+9] = mw->nvertex[ii+3];
	   po->controlv[j+10] = mw->nvertex[ii+4];
	   po->controlv[j+11] = mw->nvertex[ii+5];

	   po->controlv[j+12] = mw->vertex[ii+6];
	   po->controlv[j+13] = mw->vertex[ii+7];
	   po->controlv[j+14] = mw->vertex[ii+8];

	   po->controlv[j+15] = mw->nvertex[ii+6];
	   po->controlv[j+16] = mw->nvertex[ii+7];
	   po->controlv[j+17] = mw->nvertex[ii+8];
	 } /* for */

       new->refine = po;
       ay_trafo_copy(o, new);

       *result = new;
     } /* if */

 return ay_status;
} /* metaobj_providecb */


int
metaobj_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_object *new = NULL;

  if (!o)
    return AY_ENULL;

  ay_status = metaobj_providecb (o, AY_IDPOMESH, &new);

  if (new)
    {
      if (!in_place)
	{
	  ay_status = ay_object_link (new);
	}
      else
	{
	  ay_status = ay_object_replace (new, o);
	}
    }

 return ay_status;
} /* metaobj_convertcb */


/* note: this function _must_ be capitalized exactly this way
 * regardless of filename (see: man n load)!
 */
#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int
Metaobj_Init (Tcl_Interp * interp)
{
 int ay_status = AY_OK;
 char fname[] = "metaobj_init";
 char success_cmd[] =
   "puts stdout \"CustomObject \\\"MetaObj\\\" successfully loaded.\"\n";

#ifdef WIN32
  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }
#endif /* WIN32 */

  ay_status = ay_otype_register (metaobj_name,
				 metaobj_createcb,
				 metaobj_deletecb,
				 metaobj_copycb,
				 metaobj_drawcb,
				 NULL,	/* no points to edit */
				 metaobj_shadecb,
				 metaobj_setpropcb,
				 metaobj_getpropcb,
				 NULL,	/* No Picking! */
				 metaobj_readcb,
				 metaobj_writecb,
				 metaobj_wribcb,
				 metaobj_bbccb,
				 &metaobj_id);

  if (ay_status)
    {
      ay_error (AY_ERROR, fname, "Error registering custom object!");
      return TCL_OK;
    }

  ay_status = ay_notify_register (metaobj_notifycb, metaobj_id);

  ay_status = ay_convert_register(metaobj_convertcb, metaobj_id);

  ay_status = ay_provide_register(metaobj_providecb, metaobj_id);

  /* first Metacomp init */
  Metacomp_Init (interp);

  /* source metaobj.tcl, it contains the Tcl-code to build
     the metaobj-Attributes Property GUI */
  if ((Tcl_EvalFile (interp, "metaobj.tcl")) != TCL_OK)
    {
      ay_error (AY_ERROR, fname, "Error while sourcing \\\"metaobj.tcl\\\"!");
      return TCL_OK;
    }

  Tcl_Eval (interp, success_cmd);

 return TCL_OK;
} /* Metaobj_Init */


/**************************************************************************
 * Metacomp part
 *************************************************************************/

int
metacomp_createcb (int argc, char *argv[], ay_object * o)
{
 meta_blob *b = NULL;
 char fname[] = "crtccomp";

  if (!(b = calloc (1, sizeof (meta_blob))))
    {
      ay_error (AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  b->r = 0.4;

  b->Ri = 0.1;
  b->Ro = 0.4;
  b->a = -.444444;
  b->b = 1.888889;
  b->c = -2.444444;
  b->s = 1;
  b->formula = 0;
  b->rot = 0;
  b->scalex = 1;
  b->scaley = 1;
  b->scalez = 1;
  b->ex = 8;
  b->ey = 8;
  b->ez = 8;
  b->rm[0] = 1; /* init rotmatrix */
  b->rm[5] = 1;
  b->rm[10] = 1;
  b->rm[15] = 1;

  o->refine = b;

 return AY_OK;
} /* metacomp_createcb */


int
metacomp_deletecb (void *c)
{
 meta_blob *b;

  if (!c)
    {
      return AY_ENULL;
    }

  b = (meta_blob *) (c);

  if (b->expression)
    {
      Tcl_DecrRefCount (b->expression);
    }

  if (b)
    {
      free(b);
    }

 return AY_OK;
} /* metacomp_deletecb */


int
metacomp_copycb (void *src, void **dst)
{
 meta_blob *b = NULL;

  if (!src || !dst)
    {
      return AY_ENULL;
    }

  if (!(b = calloc (1, sizeof (meta_blob))))
    {
      return AY_EOMEM;
    }

  memcpy (b, src, sizeof (meta_blob));

  if (b->expression)
    {
      Tcl_IncrRefCount (b->expression);
    }

  *dst = (void *) b;

 return AY_OK;
} /* metacomp_copycb */


int
metacomp_drawcb (struct Togl *togl, ay_object * o)
{
 meta_blob *b;

  b = (meta_blob *) o->refine;

  glBegin (GL_POINTS);

    glVertex3d (b->p.x, b->p.y, b->p.z);

  glEnd ();

 return AY_OK;
} /* metacomp_drawcb */


/* Tcl -> C! */
int
metacomp_setpropcb (Tcl_Interp * interp, int argc, char *argv[],
		    ay_object * o)
{
 int ay_status = AY_OK;
 char *n1 = "MetaCompAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 meta_blob *b = NULL;

  if (!o)
    {
      return AY_ENULL;
    }

  b = (meta_blob *) o->refine;

  toa = Tcl_NewStringObj (n1, -1);
  ton = Tcl_NewStringObj (n1, -1);


  Tcl_SetStringObj (ton, "Formula", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj (interp, to, &b->formula);

  Tcl_SetStringObj (ton, "Radius", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj (interp, to, &b->r);

  Tcl_SetStringObj (ton, "Negative", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj (interp, to, &b->negativ);

  Tcl_SetStringObj (ton, "Rotate", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj (interp, to, &b->rot);

  Tcl_SetStringObj (ton, "Ri", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj (interp, to, &b->Ri);

  Tcl_SetStringObj (ton, "Ro", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj (interp, to, &b->Ro);

  Tcl_SetStringObj (ton, "EnergyCoeffA", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj (interp, to, &b->a);

  Tcl_SetStringObj (ton, "EnergyCoeffB", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj (interp, to, &b->b);

  Tcl_SetStringObj (ton, "EnergyCoeffC", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj (interp, to, &b->c);

/*
  Tcl_SetStringObj (ton, "Strenght", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj (interp, to, &b->s);
*/

  Tcl_SetStringObj (ton, "EdgeX", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj (interp, to, &b->ex);

  Tcl_SetStringObj (ton, "EdgeY", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj (interp, to, &b->ey);

  Tcl_SetStringObj (ton, "EdgeZ", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj (interp, to, &b->ez);

  Tcl_SetStringObj (ton, "Expression", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  if (b->expression)
    {
      Tcl_DecrRefCount(b->expression);
      b->expression = NULL;
    }

  b->expression = to;
  if (to)
    {
      Tcl_IncrRefCount(b->expression);
    }

  Tcl_IncrRefCount (toa);
  Tcl_DecrRefCount (toa);
  Tcl_IncrRefCount (ton);
  Tcl_DecrRefCount (ton);

  o->modified = AY_TRUE;

  ay_status = ay_notify_parent();

 return AY_OK;
} /* metacomp_setpropcb */


/* C -> Tcl! */
int
metacomp_getpropcb (Tcl_Interp * interp, int argc, char *argv[],
		    ay_object * o)
{
 char *n1 = "MetaCompAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 meta_blob *b = NULL;

  if (!o)
    return AY_ENULL;

  b = (meta_blob *) (o->refine);

  toa = Tcl_NewStringObj (n1, -1);

  ton = Tcl_NewStringObj (n1, -1);


  Tcl_SetStringObj (ton, "Radius", -1);
  to = Tcl_NewDoubleObj (b->r);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "Negative", -1);
  to = Tcl_NewIntObj (b->negativ);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "Rotate", -1);
  to = Tcl_NewIntObj (b->rot);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "Ri", -1);
  to = Tcl_NewDoubleObj (b->Ri);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "Ro", -1);
  to = Tcl_NewDoubleObj (b->Ro);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "EnergyCoeffA", -1);
  to = Tcl_NewDoubleObj (b->a);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "EnergyCoeffB", -1);
  to = Tcl_NewDoubleObj (b->b);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "EnergyCoeffC", -1);
  to = Tcl_NewDoubleObj (b->c);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

/*
  Tcl_SetStringObj (ton, "Strenght", -1);
  to = Tcl_NewDoubleObj (b->s);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
*/

  Tcl_SetStringObj (ton, "EdgeX", -1);
  to = Tcl_NewIntObj (b->ex);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "EdgeY", -1);
  to = Tcl_NewIntObj (b->ey);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "EdgeZ", -1);
  to = Tcl_NewIntObj (b->ez);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "Formula", -1);
  to = Tcl_NewIntObj (b->formula);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  if (b->expression)
    {
      Tcl_SetStringObj (ton, "Expression", -1);
      Tcl_ObjSetVar2 (interp, toa, ton,
		      b->expression, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    }

  Tcl_IncrRefCount (toa);
  Tcl_DecrRefCount (toa);
  Tcl_IncrRefCount (ton);
  Tcl_DecrRefCount (ton);

 return AY_OK;
} /* metacomp_getpropcb */


int
metacomp_getpntcb (int mode, ay_object * o, double *p)
{
  /*
     double min_distance = ay_prefs.pick_epsilon, distance = 0.0, point[3];
     meta_blob *b;



     b = (meta_blob *)o->refine;

     if (ay_point_edit_coords) free(ay_point_edit_coords);
     ay_point_edit_coords = NULL;


     distance = AY_VLEN((objX - b->p.x),
     (objY - b->p.y),
     (objZ - b->p.z));

     if (distance < min_distance)
     {
     ay_point_edit_coords_homogenous = AY_FALSE;
     if (!(ay_point_edit_coords = calloc(1,sizeof(double *))));
     return AY_OUTOFMEM;

     point[0] = b->p.x;
     point[1] = b->p.y;
     point[2] = b->p.z;

     ay_point_edit_coords_number = 1;
     ay_point_edit_coords[0] = &point;
     }

   */
 return AY_OK;
} /* metacomp_getpntcb */


int
metacomp_readcb (FILE * fileptr, ay_object * o)
{
 meta_blob *b;
 int read;
 char *expr = NULL;

  if (!o)
    return AY_ENULL;

  if (!(b = calloc (1, sizeof (meta_blob))))
    return AY_ERROR;

  fscanf (fileptr, "%lg\n", &b->r);
  fscanf (fileptr, "%lg\n", &b->p.x);
  fscanf (fileptr, "%lg\n", &b->p.y);
  fscanf (fileptr, "%lg\n", &b->p.z);
  fscanf (fileptr, "%d\n", &b->negativ);
  fscanf (fileptr, "%lg\n", &b->Ri);
  fscanf (fileptr, "%lg\n", &b->Ro);
  fscanf (fileptr, "%lg\n", &b->a);
  fscanf (fileptr, "%lg\n", &b->b);
  fscanf (fileptr, "%lg\n", &b->c);
  fscanf (fileptr, "%lg\n", &b->s);
  fscanf (fileptr, "%d\n", &b->formula);
  fscanf (fileptr, "%d\n", &b->rot);

  if (ay_read_version >= 2)
    {
      fscanf (fileptr, "%d\n", &b->ex);
      fscanf (fileptr, "%d\n", &b->ey);
      fscanf (fileptr, "%d", &b->ez);
      read = fgetc(fileptr);
      ay_read_string(fileptr, &expr);
      if (expr && strlen(expr))
	{
	  b->expression = Tcl_NewStringObj(expr,-1);
	  Tcl_IncrRefCount(b->expression);
	  free(expr);
	}
    }

  o->refine = b;

 return AY_OK;
} /* metacomp_readcb */


int
metacomp_writecb (FILE * fileptr, ay_object * o)
{
 meta_blob *b;

  if (!o)
    return AY_ENULL;

  b = (meta_blob *) (o->refine);

  fprintf (fileptr, "%g\n", b->r);
  fprintf (fileptr, "%g\n", b->p.x);
  fprintf (fileptr, "%g\n", b->p.y);
  fprintf (fileptr, "%g\n", b->p.z);
  fprintf (fileptr, "%d\n", b->negativ);
  fprintf (fileptr, "%g\n", b->Ri);
  fprintf (fileptr, "%g\n", b->Ro);
  fprintf (fileptr, "%g\n", b->a);
  fprintf (fileptr, "%g\n", b->b);
  fprintf (fileptr, "%g\n", b->c);
  fprintf (fileptr, "%g\n", b->s);
  fprintf (fileptr, "%d\n", b->formula);
  fprintf (fileptr, "%d\n", b->rot);
  fprintf (fileptr, "%d\n", b->ex);
  fprintf (fileptr, "%d\n", b->ey);
  fprintf (fileptr, "%d\n", b->ez);

  if (b->expression)
    fprintf (fileptr, "%s\n", Tcl_GetStringFromObj(b->expression, NULL));
  else
    fprintf (fileptr, "\n");

 return AY_OK;
} /* metacomp_writecb */

/* note: this function _must_ be capitalized exactly this way
 * regardless of filename (see: man n load)!
 */
int
Metacomp_Init (Tcl_Interp * interp)
{
 int ay_status = AY_OK;
 char fname[] = "metacomp_init";
 char success_cmd[] =
   "puts stdout \"Customobject \\\"MetaComp\\\" successfully loaded.\"\n";

  ay_status = ay_otype_register (metacomp_name,
				 metacomp_createcb,
				 metacomp_deletecb,
				 metacomp_copycb,
				 NULL,
				 metacomp_drawcb,
				 NULL,	/* metacomp_shadecb, */
				 metacomp_setpropcb,
				 metacomp_getpropcb,
				 metacomp_getpntcb,
				 metacomp_readcb,
				 metacomp_writecb,
				 NULL,	/* metacomp_wribcb, */
				 NULL,	/* metacomp_bbccb, */
				 &metacomp_id);

  if (ay_status)
    {
      ay_error (AY_ERROR, fname, "Error registering custom object!");
      return TCL_OK;
    }

  metautils_init(metacomp_id);

  /* source metacomp.tcl, it contains the Tcl-code to build
     the metaobj-Attributes Property GUI */
  if ((Tcl_EvalFile (interp, "metacomp.tcl")) != TCL_OK)
    {
      ay_error (AY_ERROR, fname,
		"Error while sourcing \\\"metacomp.tcl\\\"!");
      return TCL_OK;
    }

  Tcl_Eval (interp, success_cmd);

 return TCL_OK;
} /* Metacomp_Init */
