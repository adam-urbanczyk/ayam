/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2004 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* tess.c NURBS (and PolyMesh) tesselation tools */

/* types local to this module */

typedef struct ay_tess_listobject_s
{
  struct ay_tess_listobject_s *next;
  void *data;
} ay_tess_listobject;

typedef struct ay_tess_tri_s {
  struct ay_tess_tri_s *next;

  double p1[3];
  double p2[3];
  double p3[3];

  double n1[3];
  double n2[3];
  double n3[3];

} ay_tess_tri;

typedef struct ay_tess_object_s {
  GLenum type;

  int count;
  int startup;

  double *p1;
  double *p2;
  double *p3;
  double *p4;

  double *n1;
  double *n2;
  double *n3;
  double *n4;

  double **nextpd;
  double **nextnd;

  struct ay_tess_tri_s *tris;
} ay_tess_object;


/* prototypes of functions local to this module */

int ay_tess_checktri(double *p1, double *p2, double *p3);

void ay_tess_begindata(GLenum type, void *userData);

void ay_tess_vertexdata(GLfloat *vertex, void *userData);

void ay_tess_normaldata(GLfloat *normal, void *userData);

void ay_tess_enddata(void *userData);

void ay_tess_combinedata(GLdouble c[3], void *d[4], GLfloat w[4], void **out,
			 void *userData);

void ay_tess_managecombined(void *userData);

int ay_tess_tristopomesh(ay_tess_tri *tris, ay_object **result);

/* functions */

/* ay_tess_checktri:
 *  check triangle built from p1,p2,p3 for degeneracy;
 *  returns AY_FALSE if triangle is degenerated,
 *  otherwise returns AY_TRUE
 */
int
ay_tess_checktri(double *p1, double *p2, double *p3)
{

  /* check p1 vs. p2 */
  if((fabs(p1[0] - p2[0]) < AY_EPSILON) &&
     (fabs(p1[1] - p2[1]) < AY_EPSILON) &&
     (fabs(p1[2] - p2[2]) < AY_EPSILON))
      return(AY_FALSE);

  /* check p2 vs. p3 */
  if((fabs(p2[0] - p3[0]) < AY_EPSILON) &&
     (fabs(p2[1] - p3[1]) < AY_EPSILON) &&
     (fabs(p2[2] - p3[2]) < AY_EPSILON))
      return(AY_FALSE);

  /* check p3 vs. p1 */
  if((fabs(p3[0] - p1[0]) < AY_EPSILON) &&
     (fabs(p3[1] - p1[1]) < AY_EPSILON) &&
     (fabs(p3[2] - p1[2]) < AY_EPSILON))
      return(AY_FALSE);

 return AY_TRUE;
} /* ay_tess_checktri */


/* ay_tess_begindata:
 *  tesselation callback
 *
 */
void
ay_tess_begindata(GLenum type, void *userData)
{
 ay_tess_object *to;

  to = (ay_tess_object *)userData;

  to->type = type;

  switch(type)
    {
    case GL_TRIANGLE_FAN:
      to->count = 3;
      to->startup = 3;
      break;
    case GL_TRIANGLE_STRIP:
      to->count = 3;
      to->startup = 3;
      break;
    case GL_TRIANGLES:
      to->count = 3;
      to->startup = 3;
      break;
    case GL_QUAD_STRIP:
      to->count = 4;
      to->startup = 4;
      break;
    } /* switch */

 return;
} /* ay_tess_begindata */


/* ay_tess_vertexdata:
 *  tesselation callback
 *
 */
void
ay_tess_vertexdata(GLfloat *vertex, void *userData)
{
 ay_tess_object *to;
 double *t = NULL;
 double *nextpd;
 ay_tess_tri *tri = NULL;

  to = (ay_tess_object *)userData;

  to->count--;

  if(to->startup > 0)
    {
      to->startup--;
    }

  nextpd = *to->nextpd;

  nextpd[0] = (double)vertex[0];
  nextpd[1] = (double)vertex[1];
  nextpd[2] = (double)vertex[2];

  if((*(to->nextpd)) == to->p1)
    {
      to->nextpd = &(to->p2);
    }
  else
  if((*(to->nextpd)) == to->p2)
    {
      to->nextpd = &(to->p3);
    }
  else
  if((*(to->nextpd)) == to->p3)
    {
      to->nextpd = &(to->p4);
    }
  else
  if((*(to->nextpd)) == to->p4)
    {
      to->nextpd = &(to->p1);
    }

  if(to->count == 0)
    {
      switch(to->type)
	{
	case GL_TRIANGLE_FAN:
	  to->count = 1;
	  /*printf("Fan\n");*/
	  if(to->startup == 0)
	    {
	      /* create new triangle */
	      if(ay_tess_checktri(to->p1, to->p2, to->p3))
		{
		  if(!(tri = calloc(1, sizeof(ay_tess_tri))))
		    return;

		  tri->next = to->tris;
		  to->tris = tri;

		  memcpy(tri->p1, to->p1, 3*sizeof(double));
		  memcpy(tri->p2, to->p2, 3*sizeof(double));
		  memcpy(tri->p3, to->p3, 3*sizeof(double));

		  memcpy(tri->n1, to->n1, 3*sizeof(double));
		  memcpy(tri->n2, to->n2, 3*sizeof(double));
		  memcpy(tri->n3, to->n3, 3*sizeof(double));
		} /* if */

	      /* shift vertex/normal data */
	      t = to->p2;
	      to->p2 = to->p3;
	      to->p3 = t;

	      t = to->n2;
	      to->n2 = to->n3;
	      to->n3 = t;

	      to->nextpd = &(to->p3);
	      to->nextnd = &(to->n3);
	    } /* if */
	  break;
	case GL_TRIANGLE_STRIP:
	  to->count = 1;
	  /*printf("Strip\n");*/
	  if(to->startup == 0)
	    {
	      /* create new triangle */
	      if(ay_tess_checktri(to->p1, to->p2, to->p3))
		{
		  if(!(tri = calloc(1, sizeof(ay_tess_tri))))
		    return;
		  tri->next = to->tris;
		  to->tris = tri;

		  memcpy(tri->p1, to->p1, 3*sizeof(double));
		  memcpy(tri->p2, to->p2, 3*sizeof(double));
		  memcpy(tri->p3, to->p3, 3*sizeof(double));

		  memcpy(tri->n1, to->n1, 3*sizeof(double));
		  memcpy(tri->n2, to->n2, 3*sizeof(double));
		  memcpy(tri->n3, to->n3, 3*sizeof(double));
		} /* if */

	      /* shift vertex/normal data */
	      t = to->p1;
	      to->p1 = to->p2;
	      to->p2 = to->p3;
	      to->p3 = to->p4;
	      to->p4 = t;

	      t = to->n1;
	      to->n1 = to->n2;
	      to->n2 = to->n3;
	      to->n3 = to->n4;
	      to->n4 = t;

	      to->nextpd = &(to->p3);
	      to->nextnd = &(to->n3);
	    } /* if */

	  break;
	case GL_TRIANGLES:
	  to->count = 3;
	  /*printf("Tri\n");*/
	  if(to->startup == 0)
	    {
	      /* create new triangle */
	      if(ay_tess_checktri(to->p1, to->p2, to->p3))
		{
		  if(!(tri = calloc(1, sizeof(ay_tess_tri))))
		    return;
		  tri->next = to->tris;
		  to->tris = tri;

		  memcpy(tri->p1, to->p1, 3*sizeof(double));
		  memcpy(tri->p2, to->p2, 3*sizeof(double));
		  memcpy(tri->p3, to->p3, 3*sizeof(double));

		  memcpy(tri->n1, to->n1, 3*sizeof(double));
		  memcpy(tri->n2, to->n2, 3*sizeof(double));
		  memcpy(tri->n3, to->n3, 3*sizeof(double));
		} /* if */

	      to->nextpd = &(to->p1);
	      to->nextnd = &(to->n1);
	    } /* if */
	  break;
	case GL_QUAD_STRIP:
	  to->count = 2;
	  /*printf("QuadStrip\n");*/
	  if(to->startup == 0)
	    {
	      /* create two new triangles */
	      if(ay_tess_checktri(to->p1, to->p2, to->p3))
		{
		  if(!(tri = calloc(1, sizeof(ay_tess_tri))))
		    return;
		  tri->next = to->tris;
		  to->tris = tri;

		  memcpy(tri->p1, to->p1, 3*sizeof(double));
		  memcpy(tri->p2, to->p2, 3*sizeof(double));
		  memcpy(tri->p3, to->p3, 3*sizeof(double));

		  memcpy(tri->n1, to->n1, 3*sizeof(double));
		  memcpy(tri->n2, to->n2, 3*sizeof(double));
		  memcpy(tri->n3, to->n3, 3*sizeof(double));
		} /* if */

	      if(ay_tess_checktri(to->p2, to->p4, to->p3))
		{
		  if(!(tri = calloc(1, sizeof(ay_tess_tri))))
		    return;
		  tri->next = to->tris;
		  to->tris = tri;

		  memcpy(tri->p1, to->p2, 3*sizeof(double));
		  memcpy(tri->p2, to->p4, 3*sizeof(double));
		  memcpy(tri->p3, to->p3, 3*sizeof(double));

		  memcpy(tri->n1, to->n2, 3*sizeof(double));
		  memcpy(tri->n2, to->n4, 3*sizeof(double));
		  memcpy(tri->n3, to->n3, 3*sizeof(double));
		} /* if */

	      /* shift vertex/normal data */
	      t = to->p1;
	      to->p1 = to->p3;
	      to->p3 = t;
	      t = to->p2;
	      to->p2 = to->p4;
	      to->p4 = t;

	      t = to->n1;
	      to->n1 = to->n3;
	      to->n3 = t;
	      t = to->n2;
	      to->n2 = to->n4;
	      to->n4 = t;

	      to->nextpd = &(to->p3);
	      to->nextnd = &(to->n3);
	    } /* if */
	  break;
	} /* switch */
    } /* if */

 return;
} /* ay_tess_vertexdata */


/* ay_tess_normaldata:
 *  tesselation callback
 *
 */
void
ay_tess_normaldata(GLfloat *normal, void *userData)
{
 ay_tess_object *to;
 double *nextnd;

  to = (ay_tess_object *)userData;

  nextnd = *to->nextnd;

  nextnd[0] = (double)normal[0];
  nextnd[1] = (double)normal[1];
  nextnd[2] = (double)normal[2];

  if((*(to->nextnd)) == to->n1)
    {
      to->nextnd = &(to->n2);
    }
  else
  if((*(to->nextnd)) == to->n2)
    {
      to->nextnd = &(to->n3);
    }
  else
  if((*(to->nextnd)) == to->n3)
    {
      to->nextnd = &(to->n4);
    }
  else
  if((*(to->nextnd)) == to->n4)
    {
      to->nextnd = &(to->n1);
    }

 return;
} /* ay_tess_normaldata */


/* ay_tess_enddata:
 *  tesselation callback
 *
 */
void
ay_tess_enddata(void *userData)
{
 ay_tess_object *to;

  to = (ay_tess_object *)userData;

  to->nextpd = &(to->p1);
  to->nextnd = &(to->n1);

 return;
} /* ay_tess_enddata */


/* ay_tess_combinedata:
 *  tesselation callback
 *
 */
void
ay_tess_combinedata(GLdouble c[3], void *d[4], GLfloat w[4], void **out,
		    void *userData)
{
 GLdouble *nv = NULL;
 ay_tess_object *to;

  to = (ay_tess_object *)userData;

  if(!(nv = (GLdouble *) malloc(sizeof(GLdouble)*3)))
    return;

  nv[0] = c[0];
  nv[1] = c[1];
  nv[2] = c[2];

  /* remember pointer to free it later */
  ay_tess_managecombined((void*)nv);

  *out = nv;

 return;
} /* ay_tess_combinedata */


/* ay_tess_managecombined:
 *  manage data created by the combinedata tesselation callback
 *  if <data> is NULL, all data remembered is cleared
 */
void
ay_tess_managecombined(void *data)
{
 static ay_tess_listobject *list = NULL;
 ay_tess_listobject *new = NULL;

  if(data)
    { /* add new pointer to the list */
      if(!(new = calloc(1, sizeof(ay_tess_listobject))))
	return;

      new->data = data;
      new->next = list;
      list = new;
    }
  else
    { /* data == NULL => free the list */
      while(list)
	{
	  new = list;
	  list = new->next;
	  free(new->data);
	  free(new);
	} /* while */
    } /* if */

 return;
} /* ay_tess_managecombined */


/* ay_tess_tristopomesh:
 *  convert a bunch of tesselated triangles to a PolyMesh object
 */
int
ay_tess_tristopomesh(ay_tess_tri *tris, ay_object **result)
{
 int ay_status = AY_OK;
 ay_object *new;
 ay_pomesh_object *po = NULL;
 ay_tess_tri *tri;
 unsigned int numtris = 0, i;
 

  if(!tris || !result)
    return AY_ENULL;

  /* create new object (the PolyMesh) */
  if(!(new = calloc(1, sizeof(ay_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  ay_object_defaults(new);
  new->type = AY_IDPOMESH;
  if(!(new->refine = calloc(1, sizeof(ay_pomesh_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }
  po = (ay_pomesh_object*)new->refine;

  
  /* first, count the triangles */
  tri = tris;
  while(tri)
    {
      numtris++;
      tri = tri->next;
    }

  po->npolys = numtris;

  /* XXXX what happens, when numtris/numcontrolv gets too big for
     an unsigned int? */

  /* create index structures */
  if(!(po->nloops = calloc(numtris, sizeof(unsigned int))))
    {
      return AY_EOMEM;
    }
  for(i = 0; i < numtris; i++)
    {
      /* each polygon has just one loop */
      po->nloops[i] = 1;
    } /* for */

  if(!(po->nverts = calloc(numtris, sizeof(unsigned int))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }
  for(i = 0; i < numtris; i++)
    {
      /* each loop has just three vertices (is a triangle) */
      po->nverts[i] = 3;
    } /* for */

  if(!(po->verts = calloc(numtris*3, sizeof(unsigned int))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }
  for(i = 0; i < numtris*3; i++)
    {
      /* vertex indices are simply ordered (user may remove multiply used
	 vertices using PolyMesh optimization later) */
      po->verts[i] = i;
    } /* for */

  po->ncontrols = numtris*3;
  po->has_normals = AY_TRUE;

  /* now copy all the vertices and normals */
  if(!(po->controlv = calloc(numtris*3*6, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  i = 0;
  tri = tris;
  while(tri)
    {
      memcpy(&(po->controlv[i]), tri->p1, 3*sizeof(double));
      memcpy(&(po->controlv[i+3]), tri->n1, 3*sizeof(double));
      memcpy(&(po->controlv[i+6]), tri->p2, 3*sizeof(double));
      memcpy(&(po->controlv[i+9]), tri->n2, 3*sizeof(double));
      memcpy(&(po->controlv[i+12]), tri->p3, 3*sizeof(double));
      memcpy(&(po->controlv[i+15]), tri->n3, 3*sizeof(double));

      i += 18;

      tri = tri->next;
    } /* while */

  *result = new;

  new = NULL;
  po = NULL;

cleanup:
  if(new)
    free(new);

  if(po)
    {
      if(po->nloops)
	free(po->nloops);

      if(po->nverts)
	free(po->nverts);

      if(po->verts)
	free(po->verts);

      if(po->controlv)
	free(po->controlv);

      free(po);
    }

 return ay_status;
} /* ay_tess_tristopomesh */


/* ay_tess_npatch:
 *  tesselate the NURBS patch object o into a PolyMesh object
 *  using the GLU (V1.3+) tesselation facility
 *  smethod - sampling method:
 *   1: GLU_OBJECT_PARAMETRIC_ERROR
 *   2: GLU_OBJECT_PATH_LENGTH
 *   3: GLU_DOMAIN_DISTANCE
 *  smparam - sampling method parameter
 */
int
ay_tess_npatch(ay_object *o, int smethod, double sparam,
	       ay_object **pm)
{
#ifndef GLU_VERSION_1_3
 char fname[] = "ay_tess_npatch";
 ay_error(AY_ERROR, fname, "This function is just available on GLU V1.3+ !");
 return AY_ERROR;
#else
 int ay_status = AY_OK;
 ay_object *new = NULL;
 ay_nurbpatch_object *npatch = NULL;
 int uorder = 0, vorder = 0, width = 0, height = 0;
 unsigned int uknot_count = 0, vknot_count = 0, i = 0, a = 0;
 GLdouble sampling_tolerance = ay_prefs.glu_sampling_tolerance;
 GLfloat *uknots = NULL, *vknots = NULL, *controls = NULL;
 ay_object *trim = NULL, *loop = NULL, *nc = NULL;
 ay_tess_object to = {0};
 double p1[3], p2[3], p3[3], p4[3], n1[3], n2[3], n3[3], n4[4];
 ay_tess_tri *t1 = NULL, *t2;

  if(!o || !pm)
   return AY_ENULL;

  if(o->type != AY_IDNPATCH)
    return AY_ERROR;

  npatch = (ay_nurbpatch_object *)(o->refine);

  /* properly initialize tesselation object */
  to.p1 = p1;
  to.p2 = p2;
  to.p3 = p3;
  to.p4 = p4;
  to.n1 = n1;
  to.n2 = n2;
  to.n3 = n3;
  to.n4 = n4;

  to.nextpd = &(to.p1);
  to.nextnd = &(to.n1);

  /* convert npatch data from double to float */

  uorder = npatch->uorder;
  vorder = npatch->vorder;
  width = npatch->width;
  height = npatch->height;

  if(npatch->glu_sampling_tolerance > 0.0)
    sampling_tolerance = npatch->glu_sampling_tolerance;

  uknot_count = width + uorder;
  vknot_count = height + vorder;

  if(!(uknots = calloc(uknot_count, sizeof(GLfloat))))
    { free(new->refine); free(new); return AY_EOMEM; }
  if((vknots = calloc(vknot_count, sizeof(GLfloat))) == NULL)
    { free(new->refine); free(new); free(uknots); return AY_EOMEM; }
  if((controls = calloc(width*height*4, sizeof(GLfloat))) == NULL)
    {
      free(new->refine); free(new); free(uknots); free(vknots);
      return AY_EOMEM;
    }

  a=0;
  for(i = 0; i < uknot_count; i++)
    {
      uknots[a] = (GLfloat)npatch->uknotv[a];
      a++;
    }
  a=0;
  for(i = 0; i < vknot_count; i++)
    {
      vknots[a] = (GLfloat)npatch->vknotv[a];
      a++;
    }
  a=0;
  for(i = 0; i < (unsigned int)width*height*4; i++)
    {
      controls[a] = (GLfloat)npatch->controlv[a];
      a++;
    }

  if(npatch->no)
    {
      gluDeleteNurbsRenderer(npatch->no);
      npatch->no = NULL;
    }

  npatch->no = gluNewNurbsRenderer();
  if(npatch->no == NULL)
    {
      free(new->refine); free(new); free(uknots); free(vknots); free(controls);
      return AY_EOMEM;
    }

  /* register error handling callback */
#if defined(WIN32) && !defined(AYUSESUPERGLU)
  gluNurbsCallback(npatch->no, GLU_ERROR, (GLUnurbsErrorProc)ay_error_glucb);
#else
  gluNurbsCallback(npatch->no, GLU_ERROR, ay_error_glucb);
#endif

  /* set properties */
  gluNurbsProperty(npatch->no, GLU_NURBS_MODE, GLU_NURBS_TESSELLATOR);

  gluNurbsProperty(npatch->no, GLU_DISPLAY_MODE, GLU_FILL);

  if(smethod == 1)
    {
      gluNurbsProperty(npatch->no, GLU_SAMPLING_METHOD,
		       GLU_OBJECT_PARAMETRIC_ERROR);
      gluNurbsProperty(npatch->no, GLU_PARAMETRIC_TOLERANCE,
		       (GLfloat)sparam);
    }

  if(smethod == 2)
    {
      gluNurbsProperty(npatch->no, GLU_SAMPLING_METHOD,
		       GLU_OBJECT_PATH_LENGTH);
      gluNurbsProperty(npatch->no, GLU_SAMPLING_TOLERANCE,
		       (GLfloat)sparam);
    }

  if(smethod == 3)
    {
      gluNurbsProperty(npatch->no, GLU_SAMPLING_METHOD,
		       GLU_DOMAIN_DISTANCE);
      gluNurbsProperty(npatch->no, GLU_U_STEP,
		       (GLfloat)sparam);
      gluNurbsProperty(npatch->no, GLU_V_STEP,
		       (GLfloat)sparam);
    }

  /*
  gluNurbsProperty(npatch->no, GLU_AUTO_LOAD_MATRIX, GL_FALSE);
  */

  /* register callbacks to get tesselated data back from GLU */
  gluNurbsCallbackData(npatch->no, (void *)(&to));
  gluNurbsCallback(npatch->no, GLU_NURBS_BEGIN_DATA, ay_tess_begindata);
  gluNurbsCallback(npatch->no, GLU_NURBS_VERTEX_DATA, ay_tess_vertexdata);
  gluNurbsCallback(npatch->no, GLU_NURBS_NORMAL_DATA, ay_tess_normaldata);
  gluNurbsCallback(npatch->no, GLU_NURBS_END_DATA, ay_tess_enddata);

  /* tesselate the patch */
  gluBeginSurface(npatch->no);

  gluNurbsSurface(npatch->no, (GLint)uknot_count, uknots,
		  (GLint)vknot_count, vknots,
		  (GLint)height*4, (GLint)4, controls,
		  (GLint)npatch->uorder, (GLint)npatch->vorder,
		  GL_MAP2_VERTEX_4);

  /* draw trimcurves */
  if(o->down && o->down->next)
    {
      trim = o->down;

      while(trim->next)
	{
	  switch(trim->type)
	    {
	    case AY_IDNCURVE:
	      gluBeginTrim(npatch->no);
	       ay_status = ay_npt_drawtrimcurve(NULL, trim, npatch->no);
	      gluEndTrim(npatch->no);
	      break;
	    case AY_IDLEVEL:
	      /* XXXX check, whether level is of type trimloop? */
	      loop = trim->down;
	      if(loop && loop->next)
		{
		  gluBeginTrim(npatch->no);
		  while(loop->next)
		    {
		      if(loop->type == AY_IDNCURVE)
			{
			  ay_status = ay_npt_drawtrimcurve(NULL, loop,
							   npatch->no);
			}
		      else
			{
			  nc = NULL;
			  ay_status = ay_provide_object(loop, AY_IDNCURVE,
							&nc);
			  if(nc)
			    {
			      ay_status = ay_npt_drawtrimcurve(NULL, nc,
							       npatch->no);
			      ay_object_delete(nc);
			    } /* if */
			} /* if */
		      loop = loop->next;
		    } /* while */
		  gluEndTrim(npatch->no);
		} /* if */
	      break;
	    default:
	      nc = NULL;
	      ay_status = ay_provide_object(trim, AY_IDNCURVE, &nc);
	      if(nc)
		{
		  gluBeginTrim(npatch->no);
		   ay_status = ay_npt_drawtrimcurve(NULL, nc, npatch->no);
		  gluEndTrim(npatch->no);
		  ay_object_delete(nc);
		}
	      break;
	    } /* switch */
	  trim = trim->next;
	} /* while */
    } /* if */

  gluEndSurface(npatch->no);

  gluDeleteNurbsRenderer(npatch->no);
  npatch->no = NULL;

  /* the tess_object should now contain lots of triangles;
     copy them to the PolyMesh object */

  ay_status = ay_tess_tristopomesh(to.tris, &new);

  /* immediately optimize the polymesh (remove multiply used vertices) */
  ay_status = ay_pomesht_optimizecoords((ay_pomesh_object*)new->refine,
					AY_FALSE);

  /* return result */
  *pm = new;

  /* clean up */
  free(uknots);
  free(vknots);
  free(controls);

  t1 = to.tris;
  while(t1)
    {
      t2 = t1;
      t1 = t1->next;
      free(t2);
    } /* while */
  to.tris = NULL;

 return AY_OK;
#endif
} /* ay_tess_npatch */


/* ay_tess_npatchtcmd:
 *
 *
 */
int
ay_tess_npatchtcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status;
 char fname[] = "tess_npatch";
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL, *new = NULL;
 double sparam = ay_prefs.sparam;
 int smethod = ay_prefs.smethod+1;

  if(argc > 1)
    {
      Tcl_GetInt(interp, argv[1], &smethod);

      if(smethod == 1)
	sparam = 0.5;

      if(smethod == 2)
	sparam = 50.0;

      if(argc > 2)
	{
	  Tcl_GetDouble(interp, argv[2], &sparam);
	}
    }

  while(sel)
    {
      o = sel->object;

      if(o->type == AY_IDNPATCH)
	{
	  new = NULL;
	  ay_status = ay_tess_npatch(o, smethod, sparam, &new);
	  if(!ay_status)
	    {
	      ay_object_link(new);
	    }
	  else
	    {
	      ay_error(AY_ERROR, fname, "Could not convert object!");
	      return TCL_OK;
	    }
	}
      else
	{
	  ay_error(AY_ERROR, fname, "object is not a NPatch");
	}

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_tess_npatchtcmd */


/* ay_tess_setautonormal:
 *
 */
void
ay_tess_setautonormal(double *v1, double *v2, double *v3)
{
 double t1[3], t2[3];
 GLdouble n[3];

  AY_V3SUB(t1, v1, v2)
  AY_V3SUB(t2, v1, v3)
  AY_V3CROSS(n, t1, t2)

  /* the next fragment should be unneeded, because we enable
     GL_NORMALIZE in ay_toglcb_create */

  /*
  AY_V3NORM(n)

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
   glLoadIdentity();
  */
   glNormal3dv(n);
  /*
  glPopMatrix();
  */

 return;
} /* ay_tess_setautonormal */


/* ay_tess_pomeshf:
 *
 */
int
ay_tess_pomeshf(ay_pomesh_object *pomesh, unsigned int f,
		int optimize,
		ay_pomesh_object **trpomesh)
{
#ifndef GLU_VERSION_1_2
 char fname[] = "ay_tess_pomeshf";
 ay_error(AY_ERROR, fname, "This function is just available on GLU V1.2+ !");
 return AY_ERROR;
#else
 int ay_status = AY_OK;
 unsigned int i = 0, j = 0, k = 0, m = 0, n = 0;
 unsigned int a;
 int stride = 0;
 GLUtesselator *tess = NULL;
 ay_tess_object to = {0};
 double p1[3], p2[3], p3[3], p4[3], n1[3], n2[3], n3[3], n4[4];
 ay_tess_tri *t1 = NULL, *t2;
 ay_object *new = NULL;

  if(pomesh->has_normals)
    stride = 6;
  else
    stride = 3;

  if(!(tess = gluNewTess()))
    return AY_EOMEM;

  /* properly initialize tesselation object */
  to.p1 = p1;
  to.p2 = p2;
  to.p3 = p3;
  to.p4 = p4;
  to.n1 = n1;
  to.n2 = n2;
  to.n3 = n3;
  to.n4 = n4;

  to.nextpd = &(to.p1);
  to.nextnd = &(to.n1);

#if defined(WIN32) && !defined(AYUSESUPERGLU)
  gluTessCallback(tess, GLU_TESS_ERROR,
		  (GLUtessErrorProc)ay_error_glucb);
  gluTessCallback(tess, GLU_TESS_BEGIN_DATA,
		  (GLUtessBeginProc)ay_tess_begindata);
#else
  gluTessCallback(tess, GLU_TESS_ERROR, ay_error_glucb);
  gluTessCallback(tess, GLU_TESS_BEGIN_DATA, ay_tess_begindata);
#endif

  if(!pomesh->has_normals)
    {
#if defined(WIN32) && !defined(AYUSESUPERGLU)
      gluTessCallback(tess, GLU_TESS_VERTEX_DATA,
		      (GLUtessVertexProc)ay_tess_vertexdata);
#else
      gluTessCallback(tess, GLU_TESS_VERTEX_DATA, ay_tess_vertexdata);
#endif
    }
  else
    {
#if defined(WIN32) && !defined(AYUSESUPERGLU)
      gluTessCallback(tess, GLU_TESS_VERTEX_DATA,
		      (GLUtessVertexProc)ay_tess_normaldata);
#else
      gluTessCallback(tess, GLU_TESS_VERTEX_DATA, ay_tess_normaldata);
#endif
    } /* if */

#if defined(WIN32) && !defined(AYUSESUPERGLU)
  gluTessCallback(tess, GLU_TESS_END_DATA,
		  (GLUtessEndProc)ay_tess_enddata);
  gluTessCallback(tess, GLU_TESS_COMBINE_DATA,
		  (GLUtessCombineProc)ay_tess_combinedata);
#else
  gluTessCallback(tess, GLU_TESS_END_DATA, ay_tess_enddata);
  gluTessCallback(tess, GLU_TESS_COMBINE_DATA, ay_tess_combinedata);
#endif

  gluTessBeginPolygon(tess, (GLvoid*)(&to));
  if(!pomesh->has_normals)
    {
      ay_tess_setautonormal(&pomesh->controlv[pomesh->verts[n]*stride],
			    &pomesh->controlv[pomesh->verts[n+1]*stride],
			    &pomesh->controlv[pomesh->verts[n+2]*stride]);
    }

  for(j = 0; j < pomesh->nloops[f]; j++)
    {
      gluTessBeginContour(tess);
      for(k = 0; k < pomesh->nverts[m]; k++)
	{
	  a = pomesh->verts[n++];
	  gluTessVertex(tess,
			(GLdouble*)(&(pomesh->controlv[a*stride])),
			(GLdouble*)(&(pomesh->controlv[a*stride])));
	} /* for */
      gluTessEndContour(tess);
      m++;
    } /* for */
  gluTessEndPolygon(tess);
  gluDeleteTess(tess);

  /* free combined vertices */
  ay_tess_managecombined(NULL);

  /* the tess_object should now contain lots of triangles;
     copy them to the PolyMesh object */
  ay_status = ay_tess_tristopomesh(to.tris, &new);

  /* immediately optimize the polymesh (remove multiply used vertices) */
  if(optimize)
    ay_status = ay_pomesht_optimizecoords((ay_pomesh_object*)new->refine,
					  AY_FALSE);

  /* return result */
  *trpomesh = new->refine;

  /* clean up */
  free(new);

  /* free triangles */
  t1 = to.tris;
  while(t1)
    {
      t2 = t1;
      t1 = t1->next;
      free(t2);
    } /* while */
  to.tris = NULL;

 return AY_OK;
#endif
} /* ay_tess_pomeshf */


/* ay_tess_pomesh:
 *
 */
int
ay_tess_pomesh(ay_pomesh_object *pomesh, int optimize)
{
#ifndef GLU_VERSION_1_2
 char fname[] = "ay_tess_pomeshf";
 ay_error(AY_ERROR, fname, "This function is just available on GLU V1.2+ !");
 return AY_ERROR;
#else
 int ay_status = AY_OK;
 unsigned int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0;
 ay_object *to = NULL;
 ay_list_object *li = NULL, **nextli = NULL, *lihead = NULL;

  if(!pomesh)
    return AY_ENULL;

 return AY_OK;
#endif
} /* ay_tess_pomesh */
