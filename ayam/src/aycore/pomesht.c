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

typedef struct ay_pomesht_listobject_s
{
  struct ay_pomesht_listobject_s *next;
  void *data;
} ay_pomesht_listobject;

typedef struct ay_pomesht_htentry_s
{
  struct ay_pomesht_htentry_s *next;
  unsigned int index;
  double x, y, z, nx, ny, nz;
} ay_pomesht_htentry;

typedef struct ay_pomesh_hash_s
{
  int found;
  unsigned int index;
  int c;
  int T; /* tablesize */
  ay_pomesht_htentry **table;    
} ay_pomesht_hash;

#define AYVCOMP(x1,y1,z1,x2,y2,z2) ((fabs(x1-x2)<=AY_EPSILON) && \
           (fabs(y1-y2)<=AY_EPSILON)&&(fabs(z1-z2)<= AY_EPSILON)) 

/* prototypes of functions local to this module */
void ay_pomesht_tcbBegin(GLenum prim);

void ay_pomesht_tcbVertex(void *data);

void ay_pomesht_tcbVertexN(void *data);

void ay_pomesht_tcbEnd(void);

void ay_pomesht_tcbCombine(GLdouble c[3], void *d[4], GLfloat w[4],
			   void **out);


void ay_pomesht_ManageCombined(void *data);

void ay_pomesht_setautonormal(double *v1, double *v2, double *v3);

int ay_pomesht_inithash(ay_pomesht_hash *hash);

void ay_pomesht_destroyhash(ay_pomesht_hash *hash);

int ay_pomesht_addvertexhash(ay_pomesht_hash *phash, int ign, double *point);


/* functions */

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
 static ay_pomesht_listobject *list = NULL;
 ay_pomesht_listobject *new = NULL;

  if(data)
    { /* add new pointer to the list */
      if(!(new = calloc(1, sizeof(ay_pomesht_listobject))))
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
 unsigned int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0;
 unsigned int a;
 int stride = 0;
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
      gluTessCallback(tess, GLU_TESS_BEGIN,
		      (GLUtessBeginProc)ay_pomesht_tcbBegin);
#else
      gluTessCallback(tess, GLU_TESS_ERROR, ay_error_glucb);
      gluTessCallback(tess, GLU_TESS_BEGIN, ay_pomesht_tcbBegin);
#endif
      if(!pomesh->has_normals)
	{
#ifdef WIN32
	  gluTessCallback(tess, GLU_TESS_VERTEX,
			  (GLUtessVertexProc)ay_pomesht_tcbVertex);
#else
	  gluTessCallback(tess, GLU_TESS_VERTEX, ay_pomesht_tcbVertex);
#endif
	}
      else
	{
#ifdef WIN32
	  gluTessCallback(tess, GLU_TESS_VERTEX,
			  (GLUtessVertexProc)ay_pomesht_tcbVertexN);
#else
	  gluTessCallback(tess, GLU_TESS_VERTEX, ay_pomesht_tcbVertexN);
#endif
	}
#ifdef WIN32
      gluTessCallback(tess, GLU_TESS_END,
		      (GLUtessEndProc)ay_pomesht_tcbEnd);
      gluTessCallback(tess, GLU_TESS_COMBINE,
		      (GLUtessCombineProc)ay_pomesht_tcbCombine);
#else
      gluTessCallback(tess, GLU_TESS_END, ay_pomesht_tcbEnd);
      gluTessCallback(tess, GLU_TESS_COMBINE, ay_pomesht_tcbCombine);
#endif
      /* GLU 1.2 only: */
      /*gluTessBeginPolygon(tess, NULL);*/
      gluBeginPolygon(tess);
      if(!pomesh->has_normals)
	{
	  ay_pomesht_setautonormal(&pomesh->controlv[pomesh->verts[l]*stride],
				  &pomesh->controlv[pomesh->verts[l+1]*stride],
				 &pomesh->controlv[pomesh->verts[l+2]*stride]);
	}

       for(j = 0; j < pomesh->nloops[l]; j++)
	 {
	   /*gluTessBeginContour(tess);*/
	    for(k = 0; k < pomesh->nverts[m]; k++)
	      {
		a = pomesh->verts[n++];
		gluTessVertex(tess, (GLdouble*)(&(pomesh->controlv[a*stride])),
			      (GLdouble*)(&(pomesh->controlv[a*stride])));
	      } /* for */
	    /*gluTessEndContour(tess);*/
	    gluNextContour(tess, GLU_INTERIOR);
	    m++;
	 } /* for */
       /*      gluTessEndPolygon(tess);*/
      gluEndPolygon(tess);
      gluDeleteTess(tess);
      /* free combined vertices */
      ay_pomesht_ManageCombined(NULL);
      l++;
    } /* for */

 return AY_OK;
} /* ay_pomesht_tesselate */


/* ay_pomesht_merge:
 *  merge all PolyMesh objects to be found in list into a single
 *  new PolyMesh and return this new object in result
 */
int
ay_pomesht_merge(ay_list_object *list, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "mergePo";
 ay_list_object *lo = list;
 ay_object *o = NULL, *no = NULL;
 ay_pomesh_object *pm = NULL, *npm = NULL;
 unsigned int i = 0, j = 0, k = 0;
 unsigned int total_polys = 0, total_loops = 0, total_verts = 0,
   total_controls = 0;
 unsigned int pmloops = 0, pmverts = 0;
 unsigned int nextloops = 0, nextnverts = 0, nextverts = 0,
   nextcontrols = 0, oldpmncontrols = 0;
 int has_normals = -1, stride = 0;
 double dummy[3] = {0};

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
		  return AY_ERROR;
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

	  total_controls += pm->ncontrols;
	  
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
  npm->ncontrols = total_controls;

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
  if(!(npm->controlv = calloc(stride * total_controls, sizeof(double))))
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

	  /* the vertex indices need to be adapted to the new
	     (bigger) control point database */
	  k = 0;
	  for(j = nextverts; j < nextverts+pmverts; j++)
	    {
	      npm->verts[j] = pm->verts[k]+oldpmncontrols;
	      k++;
	    } /* for */

	  /* if the object has non-default transformation attributes,
	     we also need to transform the control points */
	  if((fabs(o->movx) > AY_EPSILON) ||
	     (fabs(o->movy) > AY_EPSILON) ||
	     (fabs(o->movz) > AY_EPSILON) ||
	     (fabs(1.0 - o->scalx) > AY_EPSILON) ||
	     (fabs(1.0 - o->scaly) > AY_EPSILON) ||
	     (fabs(1.0 - o->scalz) > AY_EPSILON) ||
	     (fabs(o->quat[0]) > AY_EPSILON) ||
	     (fabs(o->quat[1]) > AY_EPSILON) ||
	     (fabs(o->quat[2]) > AY_EPSILON) ||
	     (fabs(1.0 - o->quat[3]) > AY_EPSILON))
	    {
	      k = 0;
	      ay_trafo_apply(o, dummy, 3, AY_FALSE);
	      for(j = 0; j < pmverts; j++)
		{
		  memcpy(&(npm->controlv[nextcontrols + k]),
			 &(pm->controlv[k]),
			 stride * sizeof(double));
		  ay_trafo_apply(o, &(npm->controlv[nextcontrols + k]), 3,
				 AY_TRUE);
		  k += stride;
		} /* for */
	    } else {	  
	      memcpy(&(npm->controlv[nextcontrols]), pm->controlv,
		     stride * pmverts * sizeof(double));
	    }

	  nextloops    += pm->npolys;
	  nextnverts   += pmloops;
	  nextverts    += pmverts;
	  nextcontrols += (stride * pm->ncontrols);
	  oldpmncontrols += pm->ncontrols;
	} /* if */
      lo = lo->next;
    } /* while */

  *result = no;

 return AY_OK;
} /* ay_pomesht_merge */


/* ay_pomesht_mergetcmd:
 *  merge all selected PolyMesh objects into a new one
 *  and link this new PolyMesh to the scene
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


int
ay_pomesht_inithash(ay_pomesht_hash *hash)
{

  hash->table = (ay_pomesht_htentry **)calloc(1, sizeof(ay_pomesht_htentry *) *
					    hash->T);
  if(hash->table)
    return AY_OK;
  else
    return AY_EOMEM;
} /* ay_pomesht_inithash */

void
ay_pomesht_destroyhash(ay_pomesht_hash *hash)
{
 int i;
 ay_pomesht_htentry *entry;
 ay_pomesht_htentry *tmp;
	
  if(hash)
    {
      for(i = 0; i < hash->T; i++)
	{
	  entry = hash->table[i];
	  tmp = entry;

	  while(tmp)
	    {
	      tmp = entry->next;
	      free(entry);
	      entry = tmp;
	    } /* while */
	} /* for */
   
      if(hash->table)
	free(hash->table);

      free(hash);
    } /* if */

 return;
} /* ay_pomesht_destroyhash */


int
ay_pomesht_addvertexhash(ay_pomesht_hash *phash, int ign, double *point) 
{
 int ay_status = AY_OK;
 unsigned int index;
 ay_pomesht_htentry *entry;
 ay_pomesht_htentry *chain;
 double x, y, z;
  
  x = point[0];
  y = point[1];
  z = point[2];
  
  index = (unsigned int)((3 * fabs(x) + 5 * fabs(y) + 7 * fabs(z)) *
			 phash->c + 0.5f) % phash->T;

  entry = phash->table[index];

  phash->found = AY_FALSE;

  if(entry)
    {
      chain = entry;

      while(chain)
	{
	  /* ignore normals? */
          if(ign)
	    {
	      if(AYVCOMP(chain->x, chain->y, chain->z, x, y, z))
		{
		  phash->index = chain->index;
		  phash->found = AY_TRUE;
		  break;
		}/* if */
	    } /* if */
	  else
	    {
	      if(AYVCOMP(chain->x, chain->y, chain->z, x, y, z) &&
     AYVCOMP(chain->nx, chain->ny, chain->nz, point[3], point[4], point[5]))
	        {
		  phash->index = chain->index;
		  phash->found = AY_TRUE;
		  break;
		} /* if */
	    } /* if */

	  chain = chain->next;
	} /* while */
    } /* if */

  /* add new entry? */
  if(!phash->found)
    {
      ay_pomesht_htentry *new;
	
      new = (ay_pomesht_htentry *)calloc(1, sizeof(ay_pomesht_htentry));
	
      new->index = phash->index;
      new->x = x;
      new->y = y;
      new->z = z;

      /* use normals? */
      if(!ign)
	{
	  new->nx = point[3];
	  new->ny = point[4];
	  new->nz = point[5];
	}

      if(entry)
	new->next = entry;
	
      phash->table[index] = new;
	  
    } /* if */
   
 return ay_status;
} /* ay_pomesht_addvertexhash */

int
ay_pomesht_optimizecoords(ay_pomesh_object *pomesh, int ignore_normals)
{
 int ay_status = AY_OK;
 ay_pomesh_object *new = NULL;
 ay_pomesht_hash *phash;
 unsigned int i, total_loops = 0, total_verts = 0;
 unsigned int p, dp, t;
 int stride;
  
  if(!(new = (ay_pomesh_object *) calloc(1, sizeof(ay_pomesh_object))))
    return AY_EOMEM;
  
  /* calc total verts */
  for(i = 0; i < pomesh->npolys; i++)
    {
      total_loops += pomesh->nloops[i];
    } /* for */

  for(i = 0; i < total_loops; i++)
    {
      total_verts += pomesh->nverts[i];
    } /* for */
    
  if(!(new->verts = (unsigned int *)calloc(1, sizeof(unsigned int) *
					   total_verts)))
    { free(new); return AY_EOMEM; }
  new->ncontrols = 0;

  if(pomesh->has_normals)
    stride = 6;
  else
    stride = 3;

  if(!(new->controlv = (double *)calloc(1, pomesh->ncontrols * stride *
					sizeof(double))))
    { free(new); free(new->verts); return AY_EOMEM; }
  
  phash = (ay_pomesht_hash *)calloc(1, sizeof(ay_pomesht_hash));
  phash->T = total_verts/5;		/* hashtablesize */   
  phash->c = 1024;

  if(ay_pomesht_inithash(phash) == AY_EOMEM)
    { free(new); free(new->verts); free(new->controlv); return AY_EOMEM; }
  
  dp = 0;

  /* if user requested to honour normals but we have no normals
     we have to set ignore_normals to true */
  if(!ignore_normals && !pomesh->has_normals)
    ignore_normals = AY_TRUE;
   
  
  for(i = 0; i < total_verts; i++)
    {
      p = pomesh->verts[i] * stride;
	  
      phash->found = AY_FALSE;
      phash->index = dp;

      ay_pomesht_addvertexhash(phash, ignore_normals, &pomesh->controlv[p]);
	  
      if(phash->found)
	{
	  new->verts[i] = phash->index;
	}
      else
	{
	  new->verts[i] = dp;
	  t = dp * stride;
	  dp++;
	  new->ncontrols++;
	  new->controlv[t]   = pomesh->controlv[p];	
	  new->controlv[t+1] = pomesh->controlv[p+1];	
	  new->controlv[t+2] = pomesh->controlv[p+2];

	  if(stride == 6)
	    {
	      new->controlv[t+3] = pomesh->controlv[p+3];	
	      new->controlv[t+4] = pomesh->controlv[p+4];	
	      new->controlv[t+5] = pomesh->controlv[p+5];
	    } /* if */
	} /* if */
    } /* for */
	
  ay_pomesht_destroyhash(phash);

  if(pomesh->verts)
    free(pomesh->verts);
  
  if(pomesh->controlv)
    free(pomesh->controlv);
   
  pomesh->verts = new->verts;
  pomesh->controlv = new->controlv;
  pomesh->ncontrols = new->ncontrols; 
  realloc(pomesh->controlv, new->ncontrols * stride * sizeof(double));
  free(new);

  return ay_status;
} /* ay_pomesht_optimizecoords */


/* ay_pomesht_optimizetcmd:
 *  optimizes all selected PolyMesh objects
 */
int
ay_pomesht_optimizetcmd(ClientData clientData, Tcl_Interp * interp,
			int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "optiPo";
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = ay_selection->object;

      if(o && o->type == AY_IDPOMESH)
	{
	  ay_status = AY_OK;
	  ay_status = ay_pomesht_optimizecoords(o->refine, AY_TRUE);

	  if(ay_status)
	    { /* emit error message */
	      ay_error(AY_ERROR, fname, "could not optimize object");
	    }
	} /* if */

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_pomesht_optimizetcmd */
