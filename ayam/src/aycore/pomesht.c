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

/* pomesht.c -  PolyMesh object tools */

typedef struct list_object_s
{
  struct list_object_s *next;
  void *data;
} list_object;

/* functions local to this module */
void ay_pomesht_tcbBegin(GLenum prim);

void ay_pomesht_tcbVertex(void *data);

void ay_pomesht_tcbEnd(void);

void ay_pomesht_tcbCombine(GLdouble c[3], void *d[4], GLfloat w[4],
			   void **out);


void ay_pomesht_ManageCombined(void *data);

void ay_pomesht_setautonormal(double *v1, double *v2, double *v3);


/* tesselation callbacks needed by GLU */
void
ay_pomesht_tcbBegin(GLenum prim)
{
  glBegin (prim);
} /* ay_pomesht_tcbBegin */


void
ay_pomesht_tcbVertex(void *data)

{
  glVertex3dv((GLdouble *)data);
} /* ay_pomesht_tcbVertex */


void
ay_pomesht_tcbEnd(void)
{
  glEnd();
} /* ay_pomesht_tcbEnd */


void
ay_pomesht_tcbCombine(GLdouble c[3], void *d[4], GLfloat w[4], void **out)
{
 GLdouble *nv = NULL;

 if(!(nv = (GLdouble *) malloc(sizeof(GLdouble)*3)))
   return;

  nv[0] = c[0];
  nv[1] = c[1];
  nv[2] = c[2];

  /* remember pointer to free it later */
  ay_pomesht_ManageCombined((void*)nv);

  *out = nv; 
} /* ay_pomesht_tcbCombine */


void
ay_pomesht_ManageCombined(void *data)
{
  static list_object *list = NULL;
  list_object *new = NULL;

  if(data)
    { /* add new pointer to the list */
      if(!(new = calloc(1, sizeof(list_object))))
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
} /* ay_pomesht_ManageCombined */


/* ay_pomesht_setautonormal:
 *
 */
void
ay_pomesht_setautonormal(double *v1, double *v2, double *v3)
{
 double t1[3], t2[3];
 GLdouble n[3];

  AY_V3SUB(t1, v1, v2)
  AY_V3SUB(t2, v1, v3)
  AY_V3CROSS(n, t1, t2)
  glNormal3dv(n);

 return;
} /* ay_pomesht_setautonormal */


/* ay_pomesht_tesselate:
 *
 */
int
ay_pomesht_tesselate(ay_pomesh_object *pomesh)
{
 int i = 0, j = 0, k = 0, l = 0;
 unsigned int a;
 GLUtesselator *tess = NULL;

  for(i = 0; i < pomesh->npolys; i++)
    {
      if(!(tess = gluNewTess()))
	return AY_EOMEM;

#ifdef WIN32
      gluTessCallback(tess, GLU_TESS_ERROR, (GLUtessErrorProc)ay_error_glucb);
#else
      gluTessCallback(tess, GLU_TESS_ERROR, ay_error_glucb);
#endif
      gluTessCallback(tess, GLU_TESS_BEGIN, ay_pomesht_tcbBegin);
      gluTessCallback(tess, GLU_TESS_VERTEX, ay_pomesht_tcbVertex);
      gluTessCallback(tess, GLU_TESS_END, ay_pomesht_tcbEnd);
      gluTessCallback(tess, GLU_TESS_COMBINE, ay_pomesht_tcbCombine);

      /* GLU 1.2 only: */
      /*gluTessBeginPolygon(tess, NULL);*/
      gluBeginPolygon(tess);

       ay_pomesht_setautonormal(&pomesh->controlv[pomesh->verts[l]*3],
				&pomesh->controlv[pomesh->verts[l+1]*3],
				&pomesh->controlv[pomesh->verts[l+2]*3]);

       for(j = 0; j < pomesh->nloops[i]; j++)
	 {
	   /*gluTessBeginContour(tess);*/
	    for(k = 0; k < pomesh->nverts[j]; k++)
	      {
		a = pomesh->verts[l++];
		gluTessVertex(tess, (GLdouble*)(&(pomesh->controlv[a * 3])),
			      (GLdouble*)(&(pomesh->controlv[a * 3])));
	      } /* for */
	    /*gluTessEndContour(tess);*/
	    gluNextContour(tess, GLU_INTERIOR);
	 } /* for */

       /*      gluTessEndPolygon(tess);*/
      gluEndPolygon(tess);
      gluDeleteTess(tess);
      /* free combined vertices */
      ay_pomesht_ManageCombined(NULL);
    } /* for */

 return AY_OK;
} /* ay_pomesht_tesselate */
