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

void ay_pomesht_tcbVertexN(void *data);

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
ay_pomesht_tcbVertexN(void *data)

{
  glNormal3dv(((GLdouble *)data)+3);
  glVertex3dv((GLdouble *)data);
} /* ay_pomesht_tcbVertexN */


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
} /* ay_pomesht_setautonormal */


/* ay_pomesht_tesselate:
 *
 */
int
ay_pomesht_tesselate(ay_pomesh_object *pomesh)
{
 int i = 0, j = 0, k = 0, l = 0, stride = 0;
 unsigned int a;
 GLUtesselator *tess = NULL;

  if(pomesh->has_normals)
    stride = 6;
  else
    stride = 3;

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
      if(!pomesh->has_normals)
	{
	  gluTessCallback(tess, GLU_TESS_VERTEX, ay_pomesht_tcbVertex);
	}
      else
	{
	  gluTessCallback(tess, GLU_TESS_VERTEX, ay_pomesht_tcbVertexN);
	}
      gluTessCallback(tess, GLU_TESS_END, ay_pomesht_tcbEnd);
      gluTessCallback(tess, GLU_TESS_COMBINE, ay_pomesht_tcbCombine);

      /* GLU 1.2 only: */
      /*gluTessBeginPolygon(tess, NULL);*/
      gluBeginPolygon(tess);
      if(!pomesh->has_normals)
	{
	  ay_pomesht_setautonormal(&pomesh->controlv[pomesh->verts[l]*stride],
				  &pomesh->controlv[pomesh->verts[l+1]*stride],
				 &pomesh->controlv[pomesh->verts[l+2]*stride]);
	}

       for(j = 0; j < pomesh->nloops[i]; j++)
	 {
	   /*gluTessBeginContour(tess);*/
	    for(k = 0; k < pomesh->nverts[j]; k++)
	      {
		a = pomesh->verts[l++];
		gluTessVertex(tess, (GLdouble*)(&(pomesh->controlv[a*stride])),
			      (GLdouble*)(&(pomesh->controlv[a*stride])));
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


/* ay_pomesht_merge:
 *
 */
int
ay_pomesht_merge(ay_list_object *list, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "mergePo";
 ay_list_object *lo = list;
 ay_object *o = NULL, *no = NULL;
 ay_pomesh_object *pm = NULL, *npm = NULL;
 int i = 0, j = 0, k = 0, stride = 0;
 unsigned int total_polys = 0, total_loops = 0, total_verts = 0;
 unsigned int pmloops = 0, pmverts = 0;
 unsigned int nextloops = 0, nextnverts = 0, nextverts = 0,
   nextcontrols = 0;
 int has_normals = -1;

  while(lo)
    {
      o = lo->object;
      if(o->type == AY_IDPOMESH)
	{
	  i++;
	  pm = (ay_pomesh_object*)o->refine;

	  if(has_normals != -1)
	    {
	      if(pm->has_normals != has_normals)
		{
		  ay_error(AY_ERROR, fname,
			   "found meshes with and without vertex normals");
		  return TCL_OK;
		} /* if */
	    } /* if */

	  total_polys += pm->npolys;

	  pmloops = 0;
	  for(j = 0; j < pm->npolys; j++)
	    {
	      pmloops += pm->nloops[j];
	      total_loops += pm->nloops[j];
	    }

	  pmverts = 0;
	  for(j = 0; j < pmloops; j++)
	    {
	      pmverts += pm->nverts[j];
	      total_verts += pm->nverts[j];
	    }


	  has_normals = pm->has_normals;
	} /* if */
      lo = lo->next;
    } /* while */

  if(i < 2)
    {
      ay_error(AY_ERROR, fname, "found too few meshes");
      return AY_ERROR;
    }

  if(has_normals)
    stride = 6;
  else
    stride = 3;

  /* create a new PolyMesh object */
  if(!(no = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_EOMEM;
    }
  ay_status = ay_object_defaults(no);

  no->type = AY_IDPOMESH;

  if(!(npm = calloc(1, sizeof(ay_pomesh_object))))
    {
      free(no);
      ay_error(AY_EOMEM, fname, NULL);
      return AY_EOMEM;
    }

  npm->npolys = total_polys;
  npm->has_normals = has_normals;
  npm->ncontrols = total_verts;

  no->refine = (void*)npm;

  /* allocate memory for new merged PolyMesh */
  if(!(npm->nloops = calloc(total_polys, sizeof(unsigned int))))
    {
      free(no); free(npm); return AY_EOMEM;
    }
  if(!(npm->nverts = calloc(total_loops, sizeof(unsigned int))))
    {
      free(no); free(npm); free(npm->nloops); return AY_EOMEM;
    }
  if(!(npm->verts = calloc(total_verts, sizeof(unsigned int))))
    {
      free(no); free(npm); free(npm->nloops); free(npm->nverts);
      return AY_EOMEM;
    }
  if(!(npm->controlv = calloc(stride * total_verts, sizeof(double))))
    {
      free(no); free(npm);  free(npm->nloops); free(npm->nverts);
      free(npm->verts); return AY_EOMEM;
    }

  /* now we fill the new object with the values from all meshes to merge */
  lo = list;
  while(lo)
    {
      o = lo->object;
      if(o->type == AY_IDPOMESH)
	{
	  i++;
	  pm = (ay_pomesh_object*)o->refine;

	  memcpy(&(npm->nloops[nextloops]), pm->nloops,
		 pm->npolys * sizeof(unsigned int));

	  pmloops = 0;
	  for(j = 0; j < pm->npolys; j++)
	    {
	      pmloops += pm->nloops[j];
	    } /* for */

	  memcpy(&(npm->nverts[nextnverts]), pm->nverts,
		 pmloops * sizeof(unsigned int));

	  pmverts = 0;
	  for(j = 0; j < pmloops; j++)
	    {
	      pmverts += pm->nverts[j];
	    } /* for */

	  k = 0;
	  for(j = nextverts; j < nextverts+pmverts; j++)
	    {
	      npm->verts[j] = pm->verts[k]+nextverts;
	      k++;
	    } /* for */

	  memcpy(&(npm->controlv[nextcontrols]), pm->controlv,
		 stride * pmverts * sizeof(double));

	  nextloops    += pm->npolys;
	  nextnverts   += pmloops;
	  nextverts    += pmverts;
	  nextcontrols += (stride * pmverts);
	} /* if */
      lo = lo->next;
    } /* while */

  *result = no;

 return AY_OK;
} /* ay_pomesht_merge */

/*
 *
 */
int
ay_pomesht_mergetcmd(ClientData clientData, Tcl_Interp * interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "mergePo";
 ay_object *no = NULL;

  if(!ay_selection)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  ay_status = ay_pomesht_merge(ay_selection, &no);

  if(ay_status)
    { /* emit error message */
      ay_error(AY_ERROR, fname, "could not merge objects");
    }
  else
    { /* link the new PolyMesh to the scene */
      ay_status = ay_object_link(no);
    }

 return TCL_OK;
} /* ay_pomesht_mergetcmd */

