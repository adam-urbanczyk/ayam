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
 *
 */

#include "ayam.h"
#include "meta.h"

static char versionstring[] = "$VER: " PROGNAME VERSION "";

/* metaobj.c - metaobj custom object */

static char *metaobj_name = "MetaObj";
static unsigned int metaobj_id;
static char *metacomp_name = "MetaComp";
static unsigned int metacomp_id;

int Metacomp_Init (Tcl_Interp * interp);
int metaobj_notifycb (ay_object * o);


int
metaobj_createcb (int argc, char *argv[], ay_object * o)
{
  int ay_status = AY_OK;
  world *w = NULL;
  char fname[] = "crtball";
  ay_object *first_child = NULL;


  if (!(w = calloc (1, sizeof (world))))
    {
      ay_error (AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  w->maxpoly = mt_MAXPOLY;


  if (!
      (w->vertex =
       (double *) calloc (1, sizeof (double) * 3 * 3 * (w->maxpoly + 20))))
    {
      if (w);
      free (w);

      ay_error (AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  if (!
      (w->nvertex =
       (double *) calloc (1, sizeof (double) * 3 * 3 * (w->maxpoly + 20))))
    {
      if (w);
      free (w);

      ay_error (AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }


  w->aktcubes = mt_MAXCUBE;	/* erstmal maximale anzahl */

  if (!
      (w->mgrid =
       (char *) calloc (1, sizeof (char) * mt_MAXCUBE * mt_MAXCUBE * mt_MAXCUBE)))
    {
      if (w->Vertex3d);
      free (w->Vertex3d);

      if (w);
      free (w);

      ay_error (AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }


  mt_initcubestack (w);

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
  w->cid = &metacomp_id;

  mt_calceffect (w, SHADE);

  return AY_OK;
} /* metaobj_createcb */


int
metaobj_deletecb (void *c)
{
  world *w;

  if (!c)
    return AY_ENULL;

  w = (world *) (c);

  if (w->vertex)
    free (w->vertex);

  if (w->nvertex)
    free (w->nvertex);


  if (w->mgrid);
  free (w->mgrid);

  mt_freecubestack (w);

  if (w)
    free (w);


  return AY_OK;
} /* metaobj_deletecb */


int
metaobj_copycb (void *src, void **dst)
{
  world *w = NULL;
  world *wsrc = NULL;

  if (!src || !dst)
    return AY_ENULL;

  wsrc = (world *) src;

  if (!(w = calloc (1, sizeof (world))))
    return AY_EOMEM;

  memcpy (w, src, sizeof (world));

  if (!
      (w->vertex =
       (double *) calloc (1, sizeof (double) * 3 * 3 * (w->maxpoly + 20))))
    {
      if (w);
      free (w);

      return AY_ERROR;
    }

  memcpy (w->vertex, wsrc->vertex,
	  sizeof (double) * 3 * 3 * (w->maxpoly + 20));

  if (!
      (w->nvertex =
       (double *) calloc (1, sizeof (double) * 3 * 3 * (w->maxpoly + 20))))
    {
      if (w);
      free (w);

      return AY_ERROR;
    }


  memcpy (w->nvertex, wsrc->nvertex,
	  sizeof (double) * 3 * 3 * (w->maxpoly + 20));


  if (!
      (w->mgrid =
       (char *) calloc (1,
			sizeof (char) * w->aktcubes * w->aktcubes *
			w->aktcubes)))
    {
      free (w->Vertex3d);
      free (w);
      return AY_EOMEM;
    }


  mt_initcubestack (w);

  *dst = (void *) w;

  return AY_OK;
} /* metaobj_copycb */


int
metaobj_drawcb (struct Togl *togl, ay_object * o)
{
  world *w;
  double *vptr, x, y, z, x1, y1, z1, x2, y2, z2;
  int i;

  w = (world *) o->refine;

  vptr = w->vertex;

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

      glVertex3f (x, y, z);
      glVertex3f (x1, y1, z1);

      glVertex3f (x1, y1, z1);
      glVertex3f (x2, y2, z2);

      glVertex3f (x, y, z);
      glVertex3f (x2, y2, z2);
    }

  glEnd ();


  return AY_OK;

} /* metaobj_drawcb */


int
metaobj_shadecb (struct Togl *togl, ay_object * o)
{
  world *w;
  double *vptr, *nptr;
  int i;

  w = (world *) o->refine;

  vptr = w->vertex;
  nptr = w->nvertex;

  glBegin (GL_TRIANGLES);

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

    }

  glEnd ();

  return AY_OK;
} /* metaobj_shadecb */


/* Tcl -> C! */
int
metaobj_setpropcb (Tcl_Interp * interp, int argc, char *argv[], ay_object * o)
{
  char *n1 = "MetaObjAttrData";
  Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
  world *w = NULL;


  if (!o)
    return AY_ENULL;

  w = (world *) o->refine;

  toa = Tcl_NewStringObj (n1, -1);
  ton = Tcl_NewStringObj (n1, -1);


  Tcl_SetStringObj (ton, "Gridwith", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj (interp, to, &w->aktcubes);

  Tcl_SetStringObj (ton, "Isolevel", -1);
  to = Tcl_ObjGetVar2 (interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj (interp, to, &w->isolevel);


  Tcl_IncrRefCount (toa);
  Tcl_DecrRefCount (toa);
  Tcl_IncrRefCount (ton);
  Tcl_DecrRefCount (ton);


  if (w->aktcubes < 5)
    w->aktcubes = 5;


  w->edgelength = w->unisize / (double) w->aktcubes;


  if (w->mgrid)
    free (w->mgrid);

  if (!
      (w->mgrid =
       (char *) calloc (1,
			sizeof (char) * w->aktcubes * w->aktcubes *
			w->aktcubes)))
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
  world *w = NULL;

  if (!o)
    return AY_ENULL;

  w = (world *) (o->refine);

  toa = Tcl_NewStringObj (n1, -1);

  ton = Tcl_NewStringObj (n1, -1);


  Tcl_SetStringObj (ton, "Gridwith", -1);
  to = Tcl_NewIntObj (w->aktcubes);
  Tcl_ObjSetVar2 (interp, toa, ton, to, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj (ton, "Isolevel", -1);
  to = Tcl_NewDoubleObj (w->isolevel);
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
  world *w;

  if (!o)
    return AY_ENULL;

  if (!(w = calloc (1, sizeof (world))))
    return AY_ERROR;

  fscanf (fileptr, "%d\n", &w->aktcubes);
  fscanf (fileptr, "%lg\n", &w->isolevel);

  w->maxpoly = 10000;

  if (!
      (w->vertex =
       (double *) calloc (1, sizeof (double) * 3 * 3 * (10000 + 20))))
    {
      if (w);
      free (w);
      return AY_ERROR;
    }

  if (!
      (w->nvertex =
       (double *) calloc (1, sizeof (double) * 3 * 3 * (10000 + 20))))
    {
      if (w);
      free (w);
      return AY_ERROR;
    }

  if (!
      (w->mgrid =
       (char *) calloc (1,
			sizeof (char) * w->aktcubes * w->aktcubes *
			w->aktcubes)))
    {
      free (w->vertex);
      free (w->nvertex);
      free (w);
      return AY_EOMEM;
    }

  w->unisize = 4L;
  w->edgelength = w->unisize / (double) w->aktcubes;

  mt_initcubestack (w);

  o->parent = AY_TRUE;
  o->refine = w;


  return AY_OK;
} /* metaobj_readcb */


int
metaobj_writecb (FILE * fileptr, ay_object * o)
{
  world *w;

  if (!o)
    return AY_ENULL;

  w = (world *) (o->refine);

  fprintf (fileptr, "%d\n", w->aktcubes);
  fprintf (fileptr, "%g\n", w->isolevel);

  return AY_OK;
} /* metaobj_writecb */



int
metaobj_wribcb (char *file, ay_object * o)
{
  world *w;
  RtPoint points[3];
  RtPoint normals[3];
  double *vptr, *nptr;
  int i, j;

  if (!o)
    return AY_ENULL;

  w = (world *) o->refine;

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
	}


      RiPolygon (3, "P", (RtPointer) points, "N", (RtPointer) normals,
		 RI_NULL);

    }


  RiSolidEnd ();

  return AY_OK;
} /* metaobj_wribcb */


int
metaobj_bbccb (ay_object * o, double *bbox, int *flags)
{

  return AY_OK;
} /* metaobj_bbccb */


int
metaobj_notifycb (ay_object * o)
{
  GLdouble m[16];
  double eu[3];
  blob *b;
  ay_object *down;
  double p[3] = { 0 };
  world *w;

  glMatrixMode (GL_MODELVIEW);

  down = o->down;

  while (down->next != NULL)
    {
	 if(down->type == metacomp_id)
	 {
	      b = (blob *) down->refine;

	      glPushMatrix ();

	      glLoadIdentity ();
	      glTranslated (down->movx, down->movy, down->movz);
	      glGetDoublev (GL_MODELVIEW_MATRIX, m);

	      glLoadIdentity ();

	      glTranslated (down->movx, down->movy, down->movz);
	      ay_quat_toeuler(down->quat, eu);
	      glRotated(eu[2]*180/AY_PI,1,0,0);
	      glRotated(eu[1]*180/AY_PI,0,1,0);
	      glRotated(eu[0]*180/AY_PI,0,0,1);
      
     	 glTranslated (-down->movx, -down->movy, -down->movz);
     	 glGetDoublev (GL_MODELVIEW_MATRIX, b->rm);

     	 glPopMatrix ();

     	 p[0] = m[0] * b->p.x + m[4] * b->p.y + m[8] * b->p.z + m[12] * 1.0;
    		 p[1] = m[1] * b->p.x + m[5] * b->p.y + m[9] * b->p.z + m[13] * 1.0;
   		 p[2] = m[2] * b->p.x + m[6] * b->p.y + m[10] * b->p.z + m[14] * 1.0;

	      b->cp.x = p[0];
	      b->cp.y = p[1];
	      b->cp.z = p[2];

	      b->scalex = 1 / (down->scalx < 0.00001 ? 0.00001 : down->scalx);
	      b->scaley = 1 / (down->scaly < 0.00001 ? 0.00001 : down->scaly);
	      b->scalez = 1 / (down->scalz < 0.00001 ? 0.00001 : down->scalz);

     }

	 down = down->next;
    }

  w = (world *) o->refine;

  w->currentnumpoly = 0;
  w->o = o->down;

  mt_calceffect (w, SHADE);

  return AY_OK;

} /* metaobj_notifycb */


/* note: this function _must_ be capitalized exactly this way
 * regardless of filename (see: man n load)!
 */
int
Metaobj_Init (Tcl_Interp * interp)
{
  int ay_status = AY_OK;
  char fname[] = "metaobj_init";
  char success_cmd[] =
    "puts stdout \"CustomObject \\\"Metaobj\\\" successfully loaded.\"\n";


  ay_status = ay_otype_register (metaobj_name, metaobj_createcb, metaobj_deletecb, metaobj_copycb, metaobj_drawcb, NULL,	/* no points to edit */
				 metaobj_shadecb, metaobj_setpropcb, metaobj_getpropcb, NULL,	/* No Picking! */
				 metaobj_readcb,
				 metaobj_writecb,
				 metaobj_wribcb, metaobj_bbccb, &metaobj_id);

  ay_status = ay_notify_register (metaobj_notifycb, metaobj_id);

/*
  ay_status = ay_convert_register(ay_revolve_convertcb, AY_IDREVOLVE);
*/

  if (ay_status)
    {
      ay_error (AY_ERROR, fname, "Error registering custom object!");
      return TCL_OK;
    }



  /* source metaobj.tcl, it contains a Tcl-code to build
     the metaobj-Attributes Property GUI */
  if ((Tcl_EvalFile (interp, "metaobj.tcl")) != TCL_OK)
    {
      ay_error (AY_ERROR, fname, "Error while sourcing \\\"metaobj.tcl\\\"!");
      return TCL_OK;
    }

  Tcl_Eval (interp, success_cmd);

  /* first Metacomp init */
  ay_status = Metacomp_Init (interp);
  if (ay_status)
    {
      ay_error (AY_ERROR, fname, "Error registering Metacomp object!");
      return TCL_OK;
    }

  return TCL_OK;
} /* metaobj_Init */


/*
 * Metacomp part 
 */

int
metacomp_createcb (int argc, char *argv[], ay_object * o)
{
  blob *b = NULL;
  char fname[] = "crtccomp";

  if (!(b = calloc (1, sizeof (blob))))
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
  blob *b;

  if (!c)
    return AY_ENULL;

  b = (blob *) (c);

  if (b)
    free (b);

  return AY_OK;
} /* metacomp_deletecb */


int
metacomp_copycb (void *src, void **dst)
{
  blob *b = NULL;

  if (!src || !dst)
    return AY_ENULL;

  if (!(b = calloc (1, sizeof (blob))))
    return AY_EOMEM;

  memcpy (b, src, sizeof (blob));

  *dst = (void *) b;

  return AY_OK;
} /* metacomp_copycb */


int
metacomp_drawcb (struct Togl *togl, ay_object * o)
{
  blob *b;

  b = (blob *) o->refine;

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
  char *n1 = "MetaCompAttrData";
  Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
  blob *b = NULL;

  if (!o)
    return AY_ENULL;

  b = (blob *) o->refine;

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

  Tcl_IncrRefCount (toa);
  Tcl_DecrRefCount (toa);
  Tcl_IncrRefCount (ton);
  Tcl_DecrRefCount (ton);


  ay_notify_parent ();

  return AY_OK;
} /* metacomp_setpropcb */


/* C -> Tcl! */
int
metacomp_getpropcb (Tcl_Interp * interp, int argc, char *argv[],
		    ay_object * o)
{
  char *n1 = "MetaCompAttrData";
  Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
  blob *b = NULL;

  if (!o)
    return AY_ENULL;

  b = (blob *) (o->refine);

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

  Tcl_IncrRefCount (toa);
  Tcl_DecrRefCount (toa);
  Tcl_IncrRefCount (ton);
  Tcl_DecrRefCount (ton);

  return AY_OK;
} /* metacomp_getpropcb */

int
metacomp_getpntcb (ay_object * o, double *p)
{
  /*
     double min_distance = ay_prefs.pick_epsilon, distance = 0.0, point[3];
     blob *b;



     b = (blob *)o->refine;

     if(ay_point_edit_coords) free(ay_point_edit_coords);
     ay_point_edit_coords = NULL;


     distance = AY_VLEN((objX - b->p.x),
     (objY - b->p.y),
     (objZ - b->p.z));

     if(distance < min_distance)
     {
     ay_point_edit_coords_homogenous = AY_FALSE;
     if(!(ay_point_edit_coords = calloc(1,sizeof(double *))));
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
  blob *b;

  if (!o)
    return AY_ENULL;

  if (!(b = calloc (1, sizeof (blob))))
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
  fscanf (fileptr, "%d\n", &b->ex);
  fscanf (fileptr, "%d\n", &b->ey);
  fscanf (fileptr, "%d\n", &b->ez);

  o->refine = b;

  return AY_OK;
} /* metacomp_readcb */

int
metacomp_writecb (FILE * fileptr, ay_object * o)
{
  blob *b;

  if (!o)
    return AY_ENULL;

  b = (blob *) (o->refine);

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

  ay_status = ay_otype_register (metacomp_name, metacomp_createcb, metacomp_deletecb, metacomp_copycb, metacomp_drawcb, NULL,	/* no points to edit */
				 NULL,	/* metacomp_shadecb, */
				 metacomp_setpropcb, metacomp_getpropcb, metacomp_getpntcb, metacomp_readcb, metacomp_writecb, NULL,	/*metacomp_wribcb, */
				 NULL,	/* metacomp_bbccb, */
				 &metacomp_id);

  if (ay_status)
    {
      ay_error (AY_ERROR, fname, "Error registering custom object!");
      return TCL_OK;
    }



  /* source metacomp.tcl, it contains a Tcl-code to build
     the metaobj-Attributes Property GUI */
  if ((Tcl_EvalFile (interp, "metacomp.tcl")) != TCL_OK)
    {
      ay_error (AY_ERROR, fname,
		"Error while sourcing \\\"metacomp.tcl\\\"!");
      return TCL_OK;
    }

  Tcl_Eval (interp, success_cmd);



  return TCL_OK;
} /* metacomp_Init */
