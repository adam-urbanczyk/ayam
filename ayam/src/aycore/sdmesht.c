/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2002 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* sdmesht.c - SubdivisionMesh object tools */

typedef struct ay_sdmesht_listobject_s
{
  struct ay_sdmesht_listobject_s *next;
  void *data;
} ay_sdmesht_listobject;

/* prototypes of functions local to this module */
void ay_sdmesht_tcbBegin(GLenum prim);

void ay_sdmesht_tcbVertex(void *data);

void ay_sdmesht_tcbEnd(void);

void ay_sdmesht_tcbCombine(GLdouble c[3], void *d[4], GLfloat w[4],
			   void **out);

void ay_sdmesht_ManageCombined(void *data);

int ay_sdmesht_genfacenormals(ay_sdmesh_object *sd, double **result);


/* functions */

/* tesselation callbacks needed by GLU */
void
ay_sdmesht_tcbBegin(GLenum prim)
{
  glBegin(prim);
} /* ay_sdmesht_tcbBegin */


void
ay_sdmesht_tcbVertex(void *data)
{
  glVertex3dv((GLdouble *)data);
} /* ay_sdmesht_tcbVertex */


void
ay_sdmesht_tcbEnd(void)
{
  glEnd();
} /* ay_sdmesht_tcbEnd */


void
ay_sdmesht_tcbCombine(GLdouble c[3], void *d[4], GLfloat w[4], void **out)
{
 GLdouble *nv = NULL;

  if(!(nv = (GLdouble *) malloc(sizeof(GLdouble)*3)))
    return;

  nv[0] = c[0];
  nv[1] = c[1];
  nv[2] = c[2];

  /* remember pointer to free it later */
  ay_sdmesht_ManageCombined((void*)nv);

  *out = nv;

 return;
} /* ay_sdmesht_tcbCombine */


void
ay_sdmesht_ManageCombined(void *data)
{
 static ay_sdmesht_listobject *list = NULL;
 ay_sdmesht_listobject *new = NULL;

  if(data)
    { /* add new pointer to the list */
      if(!(new = calloc(1, sizeof(ay_sdmesht_listobject))))
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
} /* ay_sdmesht_ManageCombined */


/* ay_sdmesht_tesselate:
 *
 */
int
ay_sdmesht_tesselate(ay_sdmesh_object *sdmesh)
{
 int ay_status = AY_OK;
 unsigned int i = 0, k = 0, m = 0, n = 0;
 unsigned int a;
 GLUtesselator *tess = NULL;
 double *fn = NULL;

  if(sdmesh->face_normals)
    {
      fn = sdmesh->face_normals;
    }
  else
    {
      /* generate and cache face normals */
      if((ay_status = ay_sdmesht_genfacenormals(sdmesh, &fn)))
	return ay_status;

      sdmesh->face_normals = fn;
    }

  for(i = 0; i < sdmesh->nfaces; i++)
    {
      switch(sdmesh->nverts[m])
	{
	case 0:
	  break;
	case 1:
	  n++;
	  break;
	case 2:
	  n += 2;
	  break;
	case 3:
	  glBegin(GL_TRIANGLES);
	   glNormal3dv(&(fn[i*3]));
	   a = sdmesh->verts[n++];
	   glVertex3dv((GLdouble*)(&(sdmesh->controlv[a*3])));
	   a = sdmesh->verts[n++];
	   glVertex3dv((GLdouble*)(&(sdmesh->controlv[a*3])));
	   a = sdmesh->verts[n++];
	   glVertex3dv((GLdouble*)(&(sdmesh->controlv[a*3])));
	  glEnd();
	  break;
	case 4:
	  glBegin(GL_QUADS);
	   glNormal3dv(&(fn[i*3]));
	   a = sdmesh->verts[n++];
	   glVertex3dv((GLdouble*)(&(sdmesh->controlv[a*3])));
	   a = sdmesh->verts[n++];
	   glVertex3dv((GLdouble*)(&(sdmesh->controlv[a*3])));
	   a = sdmesh->verts[n++];
	   glVertex3dv((GLdouble*)(&(sdmesh->controlv[a*3])));
	   a = sdmesh->verts[n++];
	   glVertex3dv((GLdouble*)(&(sdmesh->controlv[a*3])));
	  glEnd();
	  break;
	default:
	  /* No, we need to tesselate this face. */
	  if(!(tess = gluNewTess()))
	    return AY_EOMEM;

	  gluTessCallback(tess, GLU_TESS_ERROR,
			  AYGLUCBTYPE ay_error_glucb);
	  gluTessCallback(tess, GLU_TESS_BEGIN,
			  AYGLUCBTYPE ay_sdmesht_tcbBegin);
	  gluTessCallback(tess, GLU_TESS_VERTEX,
			  AYGLUCBTYPE ay_sdmesht_tcbVertex);
	  gluTessCallback(tess, GLU_TESS_END,
			  AYGLUCBTYPE ay_sdmesht_tcbEnd);
	  gluTessCallback(tess, GLU_TESS_COMBINE,
			  AYGLUCBTYPE ay_sdmesht_tcbCombine);

	  /* GLU 1.2 only: */
	  /*gluTessBeginPolygon(tess, NULL);*/
	  gluBeginPolygon(tess);
	   glNormal3dv(&(fn[i*3]));

	   for(k = 0; k < sdmesh->nverts[m]; k++)
	     {
	       a = sdmesh->verts[n++];
	       gluTessVertex(tess,
			     (GLdouble*)(&(sdmesh->controlv[a * 3])),
			     (GLdouble*)(&(sdmesh->controlv[a * 3])));
	     }

	   /* GLU 1.2 only: */
	   /*gluTessEndPolygon(tess);*/
	  gluEndPolygon(tess);
	  gluDeleteTess(tess);

	  /* free combined vertices */
	  ay_sdmesht_ManageCombined(NULL);
	  break;
	} /* switch */
      m++;
    } /* for */

 return AY_OK;
} /* ay_sdmesht_tesselate */


/* ay_sdmesht_topolymesh:
 *  convert SDMesh object <sdmesh> to a PolyMesh object, return result
 *  in <pomesh>
 */
int
ay_sdmesht_topolymesh(ay_sdmesh_object *sdmesh, ay_pomesh_object **pomesh)
{
 int ay_status = AY_OK;
 double *ncontrolv = NULL;
 unsigned int *nverts = NULL, *verts = NULL, *nloops = NULL;
 unsigned int i, totalverts = 0;
 ay_pomesh_object *npomesh = NULL;

  if(!sdmesh || !pomesh)
    return AY_ENULL;

  if(!(npomesh = calloc(1, sizeof(ay_pomesh_object))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* copy control points */
  if(!(ncontrolv = malloc(sdmesh->ncontrols * 3 * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  npomesh->controlv = ncontrolv;
  npomesh->ncontrols = sdmesh->ncontrols;

  memcpy(ncontrolv, sdmesh->controlv, sdmesh->ncontrols * 3 * sizeof(double));

  /* copy faces */
  if(!(nloops = malloc(sdmesh->nfaces * sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(nverts = malloc(sdmesh->nfaces * sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }

  for(i = 0; i < sdmesh->nfaces; i++)
    {
      nloops[i] = 1;
      nverts[i] = sdmesh->nverts[i];
      totalverts += sdmesh->nverts[i];
    }

  if(!(verts = malloc(totalverts * sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }

  for(i = 0; i < totalverts; i++)
    {
      verts[i] = sdmesh->verts[i];
    }

  npomesh->npolys = sdmesh->nfaces;
  npomesh->nloops = nloops;
  npomesh->nverts = nverts;
  npomesh->verts = verts;

  /* return result */
  *pomesh = npomesh;
  npomesh = NULL;

cleanup:

 if(npomesh)
   {
     free(npomesh);
     if(ncontrolv)
       free(ncontrolv);
     if(nloops)
       free(nloops);
     if(nverts)
       free(nverts);
     if(verts)
       free(verts);
   } /* if */

 return ay_status;
} /* ay_sdmesht_topolymesh */


/** ay_sdmesht_genfacenormals:
 *  Generate face normals for SDMesh.
 */
int
ay_sdmesht_genfacenormals(ay_sdmesh_object *sd, double **result)
{
 int ay_status = AY_OK;
 ay_pomesh_object po = {0};
 unsigned int i;

  if(!(po.nloops = malloc(sd->nfaces * sizeof(unsigned int))))
    return AY_EOMEM;

  for(i = 0; i < sd->nfaces; i++)
    po.nloops[i] = 1;

  po.npolys = sd->nfaces;
  po.nverts = sd->nverts;
  po.verts = sd->verts;
  po.controlv = sd->controlv;

  ay_status = ay_pomesht_genfacenormals(&po, result);

  if(po.nloops)
    free(po.nloops);

 return ay_status;
} /* ay_sdmesht_genfacenormals */
