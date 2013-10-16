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

/* pomesht.c - PolyMesh object tools */


/* types local to this module */

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

void ay_pomesht_tcbCombineN(GLdouble c[3], void *d[4], GLfloat w[4],
			    void **out);

void ay_pomesht_ManageCombined(void *data);

int ay_pomesht_inithash(ay_pomesht_hash *hash);

void ay_pomesht_destroyhash(ay_pomesht_hash *hash);

int ay_pomesht_addvertextohash(ay_pomesht_hash *phash, int ignore_normals,
			       double *point);


/* functions */

 /* ay_pomesht_destroy:
  *  free all memory associated with the polymesh object <pomesh>
  *  _and_ pomesh itself in a tolerant fashion
  */
int
ay_pomesht_destroy(ay_pomesh_object *pomesh)
{

  if(!pomesh)
    return AY_ENULL;

  if(pomesh->nloops)
    free(pomesh->nloops);
  if(pomesh->nverts)
    free(pomesh->nverts);
  if(pomesh->verts)
    free(pomesh->verts);
  if(pomesh->controlv)
    free(pomesh->controlv);
  if(pomesh->face_normals)
    free(pomesh->face_normals);
  free(pomesh);

 return AY_OK;
} /* ay_pomesht_destroy */


/* tesselation callbacks needed by GLU */
void
ay_pomesht_tcbBegin(GLenum prim)
{
  glBegin(prim);
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
ay_pomesht_tcbCombineN(GLdouble c[3], void *d[4], GLfloat w[4], void **out)
{
 GLdouble *nv = NULL;

  if(!(nv = (GLdouble *) malloc(sizeof(GLdouble)*6)))
    return;

  nv[0] = c[0];
  nv[1] = c[1];
  nv[2] = c[2];

  nv[3] = w[0]*((double*)d[0])[3] + w[1]*((double*)d[1])[3] +
    w[2]*((double*)d[2])[3] + w[3]*((double*)d[3])[3];

  nv[4] = w[0]*((double*)d[0])[4] + w[1]*((double*)d[1])[4] +
    w[2]*((double*)d[2])[4] + w[3]*((double*)d[3])[4];

  nv[5] = w[0]*((double*)d[0])[5] + w[1]*((double*)d[1])[5] +
    w[2]*((double*)d[2])[5] + w[3]*((double*)d[3])[5];

  /* remember pointer to free it later */
  ay_pomesht_ManageCombined((void*)nv);

  *out = nv;
} /* ay_pomesht_tcbCombineN */


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


/* ay_pomesht_tesselate:
 *  tesselate PolyMesh <pomesh> into triangles and draw them
 *  immediately using OpenGL
 */
int
ay_pomesht_tesselate(ay_pomesh_object *pomesh)
{
 int ay_status = AY_OK;
 unsigned int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0;
 unsigned int a;
 int stride = 0;
 GLUtesselator *tess = NULL;
 double *fn = NULL;

  if(pomesh->has_normals)
    {
      stride = 6;
    }
  else
    {
      stride = 3;
      if(pomesh->face_normals)
	{
	  fn = pomesh->face_normals;
	}
      else
	{
	  /* generate and cache face normals */
	  if((ay_status = ay_pomesht_genfacenormals(pomesh, &fn)))
	    return ay_status;

	  pomesh->face_normals = fn;
	}
    }

  for(i = 0; i < pomesh->npolys; i++)
    {
      /* is this polygon a simple triangle or quad? */
      if((pomesh->nloops[i] == 1) && (pomesh->nverts[m] < 5))
	{
	  switch(pomesh->nverts[m])
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
	      /* is triangle */
	      glBegin(GL_TRIANGLES);
	       a = pomesh->verts[n++];
	       if(pomesh->has_normals)
		 {
		   glNormal3dv((GLdouble*)(&(pomesh->controlv[a*stride+3])));
		   glVertex3dv((GLdouble*)(&(pomesh->controlv[a*stride])));
		   a = pomesh->verts[n++];
		   glNormal3dv((GLdouble*)(&(pomesh->controlv[a*stride+3])));
		   glVertex3dv((GLdouble*)(&(pomesh->controlv[a*stride])));
		   a = pomesh->verts[n++];
		   glNormal3dv((GLdouble*)(&(pomesh->controlv[a*stride+3])));
		   glVertex3dv((GLdouble*)(&(pomesh->controlv[a*stride])));
		 }
	       else
		 {
		   glNormal3dv(&(fn[i*3]));
		   glVertex3dv((GLdouble*)(&(pomesh->controlv[a*stride])));
		   a = pomesh->verts[n++];
		   glVertex3dv((GLdouble*)(&(pomesh->controlv[a*stride])));
		   a = pomesh->verts[n++];
		   glVertex3dv((GLdouble*)(&(pomesh->controlv[a*stride])));
		 }
	      glEnd();
	      break;
	    case 4:
	      /* is quad */
	      glBegin(GL_QUADS);
	       a = pomesh->verts[n++];
	       if(pomesh->has_normals)
		 {
		   glNormal3dv((GLdouble*)(&(pomesh->controlv[a*stride+3])));
		   glVertex3dv((GLdouble*)(&(pomesh->controlv[a*stride])));
		   a = pomesh->verts[n++];
		   glNormal3dv((GLdouble*)(&(pomesh->controlv[a*stride+3])));
		   glVertex3dv((GLdouble*)(&(pomesh->controlv[a*stride])));
		   a = pomesh->verts[n++];
		   glNormal3dv((GLdouble*)(&(pomesh->controlv[a*stride+3])));
		   glVertex3dv((GLdouble*)(&(pomesh->controlv[a*stride])));
		   a = pomesh->verts[n++];
		   glNormal3dv((GLdouble*)(&(pomesh->controlv[a*stride+3])));
		   glVertex3dv((GLdouble*)(&(pomesh->controlv[a*stride])));
		 }
	       else
		 {
		   glNormal3dv(&(fn[i*3]));
		   glVertex3dv((GLdouble*)(&(pomesh->controlv[a*stride])));
		   a = pomesh->verts[n++];
		   glVertex3dv((GLdouble*)(&(pomesh->controlv[a*stride])));
		   a = pomesh->verts[n++];
		   glVertex3dv((GLdouble*)(&(pomesh->controlv[a*stride])));
		   a = pomesh->verts[n++];
		   glVertex3dv((GLdouble*)(&(pomesh->controlv[a*stride])));
		 }
	      glEnd();
	      break;
	    default:
	      break;
	    } /* switch */
	  m++;
	  l++;
	}
      else
	{
	  /* general polygon */
	  if(!(tess = gluNewTess()))
	    return AY_EOMEM;

	  gluTessCallback(tess, GLU_TESS_ERROR,
			  AYGLUCBTYPE ay_error_glucb);
	  gluTessCallback(tess, GLU_TESS_BEGIN,
			  AYGLUCBTYPE ay_pomesht_tcbBegin);

	  if(!pomesh->has_normals)
	    {
	      gluTessCallback(tess, GLU_TESS_VERTEX,
			      AYGLUCBTYPE ay_pomesht_tcbVertex);
	      gluTessCallback(tess, GLU_TESS_COMBINE,
			      AYGLUCBTYPE ay_pomesht_tcbCombine);
	    }
	  else
	    {
	      gluTessCallback(tess, GLU_TESS_VERTEX,
			      AYGLUCBTYPE ay_pomesht_tcbVertexN);
	      gluTessCallback(tess, GLU_TESS_COMBINE,
			      AYGLUCBTYPE ay_pomesht_tcbCombineN);
	    } /* if */

	  gluTessCallback(tess, GLU_TESS_END,
			  AYGLUCBTYPE ay_pomesht_tcbEnd);

	  /* GLU 1.2 only: */
	  /*gluTessBeginPolygon(tess, NULL);*/
	  gluBeginPolygon(tess);
	   if(!pomesh->has_normals)
	     {
	       glNormal3dv(&(fn[i*3]));
	     }

	   for(j = 0; j < pomesh->nloops[l]; j++)
	     {
	       /*gluTessBeginContour(tess);*/
	       for(k = 0; k < pomesh->nverts[m]; k++)
		 {
		   a = pomesh->verts[n++];
		   gluTessVertex(tess,
				 (GLdouble*)(&(pomesh->controlv[a*stride])),
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
	} /* if */
    } /* for */

 return AY_OK;
} /* ay_pomesht_tesselate */


/* ay_pomesht_merge:
 *  merge all PolyMesh objects to be found in list into a single
 *  new PolyMesh and return this new object in result
 */
int
ay_pomesht_merge(int merge_pv_tags, ay_list_object *list, ay_object **result)
{
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
 double tm[16], rtm[16];
 int have_trafo = AY_FALSE, have_pv_tags = AY_TRUE;
 int have_rotation = AY_FALSE;
 ay_tag *tag1 = NULL, *tag2 = NULL, *mtag = NULL;
 char *ct;

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
			   "Found meshes with and without vertex normals!");
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
      ay_error(AY_ERROR, fname, "Need atleast two PolyMesh objects to merge!");
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
  ay_object_defaults(no);

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
      free(no); free(npm->nloops); free(npm); return AY_EOMEM;
    }
  if(!(npm->verts = calloc(total_verts, sizeof(unsigned int))))
    {
      free(no); free(npm->nloops); free(npm->nverts); free(npm);
      return AY_EOMEM;
    }
  if(!(npm->controlv = calloc(stride * total_controls, sizeof(double))))
    {
      free(no); free(npm->nloops); free(npm->nverts);
      free(npm->verts); free(npm); return AY_EOMEM;
    }

  /* now we fill the new object with the values from all meshes to merge */
  lo = list;
  while(lo)
    {
      o = lo->object;
      if(o->type == AY_IDPOMESH)
	{
	  if(AY_ISTRAFO(o))
	    {
	      have_trafo = AY_TRUE;
	    }
	  else
	    {
	      have_trafo = AY_FALSE;
	    }

	  if((fabs(o->quat[0]) > AY_EPSILON) ||
	     (fabs(o->quat[1]) > AY_EPSILON) ||
	     (fabs(o->quat[2]) > AY_EPSILON) ||
	     (fabs(1.0 - o->quat[3]) > AY_EPSILON))
	    {
	      have_rotation = AY_TRUE;
	    }
	  else
	    {
	      have_rotation = AY_FALSE;
	    }

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
	  if(have_trafo || have_rotation)
	    {
	      k = 0;
	      ay_trafo_creatematrix(o, tm);
	      if(has_normals && have_rotation)
		ay_quat_torotmatrix(o->quat, rtm);

	      for(j = 0; j < pm->ncontrols; j++)
		{
		  memcpy(&(npm->controlv[nextcontrols + k]),
			 &(pm->controlv[k]),
			 stride * sizeof(double));

		  ay_trafo_apply3(&(npm->controlv[nextcontrols + k]), tm);

		  if(has_normals && have_rotation)
		    ay_trafo_apply3(&(npm->controlv[nextcontrols + k + 3]),
				    rtm);

		  k += stride;
		} /* for */
	    }
	  else
	    {
	      memcpy(&(npm->controlv[nextcontrols]), pm->controlv,
		     stride * pm->ncontrols * sizeof(double));
	    }

	  nextloops    += pm->npolys;
	  nextnverts   += pmloops;
	  nextverts    += pmverts;
	  nextcontrols += (stride * pm->ncontrols);
	  oldpmncontrols += pm->ncontrols;

	  if(merge_pv_tags && have_pv_tags)
	    {
	      /* first object? */
	      if(lo == list)
		{
		  /* yes, just copy the tags */
		  have_pv_tags = AY_FALSE;
		  if(o->tags)
		    {
		      tag1 = o->tags;
		      while(tag1)
			{
			  if(tag1->type == ay_pv_tagtype)
			    {
			      have_pv_tags = AY_TRUE;
			      break;
			    }
			  tag1 = tag1->next;
			} /* while */
		      (void)ay_tags_copyall(o, no);
		    } /* if */
		}
	      else
		{
		  /* not first object, merge tags */
		  tag1 = no->tags;
		  while(tag1)
		    {
		      /* only work on PV tags with varying/vertex detail */
		      if(tag1->type == ay_pv_tagtype &&
			 ay_pv_getdetail(tag1, NULL) >= 2)
			{
			  /* find matching PV tag to merge it in */
			  tag2 = o->tags;
			  while(tag2)
			    {
			      mtag = NULL;
			      if((tag2->type == ay_pv_tagtype) &&
				 ay_pv_cmpndt(tag1, tag2))
				(void)ay_pv_merge(tag1, tag2, &mtag);
			      if(mtag)
				{
				  /* swap value strings tag1<>mtag */
				  ct = tag1->val;
				  tag1->val = mtag->val;
				  mtag->val = ct;
				  ay_tags_free(mtag);
				} /* if */
			      tag2 = tag2->next;
			    } /* while */
			} /* if */
		      tag1 = tag1->next;
		    } /* while */
		} /* if */
	    } /* if */
	} /* if */

      lo = lo->next;
    } /* while */

  *result = no;

 return AY_OK;
} /* ay_pomesht_merge */


/** ay_pomesht_mergetcmd:
 *  merge all selected PolyMesh objects into a new one
 *  and link this new PolyMesh to the scene
 *  Implements the \a mergePo scripting interface command.
 *  See also the corresponding section in the \ayd{scmergepo}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_pomesht_mergetcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 int i = 1, mergepvtags = AY_FALSE;
 ay_object *no = NULL;

  if(!ay_selection)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(i+1 < argc)
    {
      if(!strcmp(argv[i], "-p"))
	{
	  sscanf(argv[i+1], "%d", &mergepvtags);
	}
      i += 2;
    }

  ay_status = ay_pomesht_merge(mergepvtags, ay_selection, &no);

  if(ay_status)
    { /* emit error message */
      ay_error(AY_ERROR, argv[0], "Merge operation failed!");
    }
  else
    { /* link the new PolyMesh to the scene */
      ay_object_link(no);
    }

 return TCL_OK;
} /* ay_pomesht_mergetcmd */


/* ay_pomesht_inithash:
 *  helper for ay_pomesht_optimizecoords() below
 *  initialize the hash table
 */
int
ay_pomesht_inithash(ay_pomesht_hash *hash)
{

  hash->table = (ay_pomesht_htentry **)calloc(1,
				    sizeof(ay_pomesht_htentry *)*hash->T);
  if(hash->table)
    return AY_OK;
  else
    return AY_EOMEM;
} /* ay_pomesht_inithash */


/* ay_pomesht_inithash:
 *  helper for ay_pomesht_optimizecoords() below
 *  destroy the hash table
 */
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


/* ay_pomesht_inithash:
 *  helper for ay_pomesht_optimizecoords() below
 *  add point <point> to the hash table <phash>
 *  (if it is not present already)
 */
int
ay_pomesht_addvertextohash(ay_pomesht_hash *phash, int ignore_normals,
			   double *point)
{
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
          if(ignore_normals)
	    {
	      if(AYVCOMP(chain->x, chain->y, chain->z, x, y, z))
		{
		  phash->index = chain->index;
		  phash->found = AY_TRUE;
		  break;
		} /* if */
	    }
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

      if(!(new = (ay_pomesht_htentry *)calloc(1, sizeof(ay_pomesht_htentry))))
	{
	  return AY_EOMEM;
	}

      new->index = phash->index;
      new->x = x;
      new->y = y;
      new->z = z;

      /* use normals? */
      if(!ignore_normals)
	{
	  new->nx = point[3];
	  new->ny = point[4];
	  new->nz = point[5];
	}

      if(entry)
	new->next = entry;

      phash->table[index] = new;
    } /* if */

 return AY_OK;
} /* ay_pomesht_addvertextohash */


int
ay_pomesht_optimizecoords(ay_pomesh_object *pomesh, int ignore_normals)
{
 int ay_status = AY_OK;
 ay_pomesh_object *new = NULL;
 ay_pomesht_hash *phash;
 unsigned int i, total_loops = 0, total_verts = 0;
 unsigned int p, dp, t;
 int stride;
 double *tmp = NULL;

  /* calc total verts */
  for(i = 0; i < pomesh->npolys; i++)
    {
      total_loops += pomesh->nloops[i];
    } /* for */

  for(i = 0; i < total_loops; i++)
    {
      total_verts += pomesh->nverts[i];
    } /* for */

  /* should we optimize at all? */
  if(pomesh->npolys <= 1)
     return AY_OK;

  if(!(new = (ay_pomesh_object *)calloc(1, sizeof(ay_pomesh_object))))
    return AY_EOMEM;

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
    { free(new->verts); free(new); return AY_EOMEM; }

  phash = (ay_pomesht_hash *)calloc(1, sizeof(ay_pomesht_hash));
  phash->T = total_verts/5;		/* hashtablesize */
  phash->c = 1024;

  if(ay_pomesht_inithash(phash) == AY_EOMEM)
    { free(new->verts); free(new->controlv); free(new); return AY_EOMEM; }

  dp = 0;

  /* if the user requested to honour normals but we have no normals
     we have to set ignore_normals to true */
  if(!ignore_normals && !pomesh->has_normals)
    ignore_normals = AY_TRUE;


  for(i = 0; i < total_verts; i++)
    {
      p = pomesh->verts[i] * stride;

      phash->found = AY_FALSE;
      phash->index = dp;

      ay_status = ay_pomesht_addvertextohash(phash, ignore_normals,
					     &(pomesh->controlv[p]));

      if(ay_status)
	{
	  /* XXXX early exit, potential memory leak! */
	  return ay_status;
	}

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
	  memcpy(&(new->controlv[t]), &(pomesh->controlv[p]),
		 stride * sizeof(double));
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
  if(!(tmp = realloc(pomesh->controlv, new->ncontrols * stride *
		     sizeof(double))))
    {
      ay_status = AY_EOMEM;
    }
  else
    {
      pomesh->controlv = tmp;
    }
  free(new);

 return ay_status;
} /* ay_pomesht_optimizecoords */


/** ay_pomesht_optimizetcmd:
 *  optimizes all selected PolyMesh objects
 *  Implements the \a optiPo scripting interface command.
 *  See also the corresponding section in the \ayd{scoptipo}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_pomesht_optimizetcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[])
{
 int ay_status = AY_OK;
 int i = 1, optimize_coords = 1, ignore_normals = 1, optimize_faces = 0;
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;

  while(i+1 < argc)
    {
      if(!strcmp(argv[i], "-c"))
	{
	  sscanf(argv[i+1], "%d", &optimize_coords);
	}
      else
      if(!strcmp(argv[i], "-n"))
	{
	  sscanf(argv[i+1], "%d", &ignore_normals);
	}
      else
      if(!strcmp(argv[i], "-f"))
	{
	  sscanf(argv[i+1], "%d", &optimize_faces);
	}

      i += 2;
  } /* while */

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;

      if(o->type == AY_IDPOMESH)
	{
	  ay_status = AY_OK;
	  if(optimize_coords)
	    {
	      ay_status = ay_pomesht_optimizecoords(o->refine, ignore_normals);
	    }
	  if(ay_status)
	    { /* emit error message */
	      ay_error(AY_ERROR, argv[0], "Optimize operation failed!");
	    }
	  else
	    {
	      /* update pointers to controlv */
	      ay_selp_clear(o);
	    } /* if */
	}
      else
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	} /* if */

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_pomesht_optimizetcmd */


/* ay_pomesht_tosdmesh:
 *  convert PolyMesh object <pomesh> to a SDMesh object, return result
 *  in <sdmesh>
 *  Note: faces with more than one loop (holes) are ignored, if these are
 *  encountered, not all control points in the resulting SDMesh object
 *  may end up being used
 */
int
ay_pomesht_tosdmesh(ay_pomesh_object *pomesh, ay_sdmesh_object **sdmesh)
{
 int ay_status = AY_OK;
 double *ncontrolv = NULL;
 unsigned int *nverts = NULL, *verts = NULL, *t = NULL;
 unsigned int i, j, k, l = 0, m = 0, n = 0, totalverts = 0;
 ay_sdmesh_object *nsdmesh = NULL;

  if(!pomesh || !sdmesh)
    return AY_ENULL;

  if(!(nsdmesh = calloc(1, sizeof(ay_sdmesh_object))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* copy control points */
  if(!(ncontrolv = calloc(pomesh->ncontrols, 3*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  nsdmesh->controlv = ncontrolv;
  nsdmesh->ncontrols = pomesh->ncontrols;

  if(!pomesh->has_normals)
    {
      memcpy(ncontrolv, pomesh->controlv, pomesh->ncontrols*3*sizeof(double));
    }
  else
    {
      for(i = 0; i < pomesh->ncontrols; i++)
	{
	  memcpy(&(ncontrolv[i*3]), &(pomesh->controlv[i*6]),
		 3*sizeof(double));
	} /* for */
    } /* if */

  /* copy faces */
  for(i = 0; i < pomesh->npolys; i++)
    {
      if(pomesh->nloops[i] < 2)
	{
	  t = NULL;
	  if(!(t = realloc(nverts, (nsdmesh->nfaces+1)*sizeof(unsigned int))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  nverts = t;
	  nverts[nsdmesh->nfaces] = pomesh->nverts[m];

	  t = NULL;
	  if(!(t = realloc(verts, (totalverts+pomesh->nverts[m])*
			   sizeof(unsigned int))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  verts = t;
	  for(k = 0; k < pomesh->nverts[m]; k++)
	    {
	      verts[totalverts+k] = pomesh->verts[n];
	      n++;
	    } /* for */

	  totalverts += pomesh->nverts[m];
	  m++;
	  nsdmesh->nfaces++;
	}
      else
	{
	  /* just advance pointers m/n correctly (while ignoring
	     faces with holes) */
	  for(j = 0; j < pomesh->nloops[l]; j++)
	    {
	      for(k = 0; k < pomesh->nverts[m]; k++)
		{
		  n++;
		} /* for */
	      m++;
	    } /* for */
	} /* if */
      l++;
    } /* for */

  nsdmesh->nverts = nverts;
  nsdmesh->verts = verts;

  /* return result */
  *sdmesh = nsdmesh;
  nsdmesh = NULL;

cleanup:

 if(nsdmesh)
   {
     free(nsdmesh);
     if(ncontrolv)
       free(ncontrolv);
     if(nverts)
       free(nverts);
     if(verts)
       free(verts);
   } /* if */

 return ay_status;
} /* ay_pomesht_tosdmesh */


/* ay_pomesht_splitface:
 *  split face <f> off of polymesh <pomesh> and add it to the
 *  polymesh <target>
 *  <pomesh> remains unchanged
 */
int
ay_pomesht_splitface(ay_pomesh_object *pomesh, unsigned int f,
		     ay_pomesh_object *target)
{
 int ay_status = AY_OK;
 int stride;
 unsigned int i, j, k, l, m, n, *tmp;
 unsigned int oldtotalloops = 0, oldtotalverts = 0;
 double *dtmp;

  if(!pomesh || !target)
    return AY_ENULL;

  if(f > pomesh->npolys)
    return AY_ERROR;

  if(pomesh->has_normals)
    stride = 6;
  else
    stride = 3;

  target->has_normals = pomesh->has_normals;

  /* fast forward m to face to split off, also keep l for later use
     as loop index (points to the loops of the face to split off) */
  l = 0;
  m = 0;
  for(i = 0; i < f; i++)
    {
      for(j = 0; j < pomesh->nloops[i]; j++)
	{
	  m += pomesh->nverts[l];
	  l++;
	}
    }

  /* count number of old loops and vertices (in target) */
  n = 0;
  for(i = 0; i < target->npolys; i++)
    {
      oldtotalloops += target->nloops[i];
      for(j = 0; j < target->nloops[i]; j++)
	{
	  oldtotalverts += target->nverts[n];
	  n++;
	}
    }

  /* increase targets number of faces counter */
  target->npolys++;

  /* for the new face in target, set the number of loops */
  tmp = NULL;
  if(!(tmp = realloc(target->nloops, target->npolys*sizeof(unsigned int))))
    {
      return AY_EOMEM;
    }
  target->nloops = tmp;
  target->nloops[target->npolys-1] = pomesh->nloops[f];

  /* for all new loops in target, set the number of vertices */
  tmp = NULL;
  if(!(tmp = realloc(target->nverts, (oldtotalloops + pomesh->nloops[f]) *
		     sizeof(unsigned int))))
    {
      return AY_EOMEM;
    }
  target->nverts = tmp;
  for(k = oldtotalloops; k < oldtotalloops + pomesh->nloops[f]; k++)
    {
      /* l is the loop index (in pomesh) */
      target->nverts[k] = pomesh->nverts[l];

      /* also create new vertice info and copy control points */
      tmp = NULL;
      if(!(tmp = realloc(target->verts, (oldtotalverts + pomesh->nverts[l]) *
			 sizeof(unsigned int))))
	{
	  return AY_EOMEM;
	}
      target->verts = tmp;
      dtmp = NULL;
      if(!(dtmp = realloc(target->controlv, (target->ncontrols +
					     pomesh->nverts[l]) * stride *
			  sizeof(double))))
	{
	  return AY_EOMEM;
	}
      target->controlv = dtmp;
      for(n = 0; n < pomesh->nverts[l]; n++)
	{
	  target->verts[oldtotalverts + n] = target->ncontrols + n;
	  memcpy(&(target->controlv[(target->ncontrols + n)*stride]),
		 &(pomesh->controlv[(pomesh->verts[m + n])*stride]),
		 stride*sizeof(double));
	} /* for */
      target->ncontrols += pomesh->nverts[l];

      /* increase all index variables used */
      m += pomesh->nverts[l];
      oldtotalverts += pomesh->nverts[l];
      l++;
    } /* for */

 return ay_status;
} /* ay_pomesht_splitface */


/* ay_pomesht_split:
 *  split polymesh <pomesh> into two, based on selected points in <pnts>
 *  returns resulting new polymesh in <result>
 */
int
ay_pomesht_split(ay_pomesh_object *pomesh, ay_point *pnts,
		 ay_pomesh_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "pomesht_split";
 ay_point *pnt = NULL;
 unsigned int i, j, k, m, n;
 int stride = 3;
 int foundpnt, splitoffthisface;
 double *v0, *v1;
 ay_pomesh_object *pomesh0 = NULL, *pomesh1 = NULL;

  if(!pomesh || !pnts || !result)
    return AY_ENULL;

  if(!(pomesh0 = calloc(1, sizeof(ay_pomesh_object))))
    {
      return AY_EOMEM;
    }
  if(!(pomesh1 = calloc(1, sizeof(ay_pomesh_object))))
    {
      free(pomesh0); return AY_EOMEM;
    }

  if(pomesh->has_normals)
    stride = 6;

  m = 0;
  n = 0;
  for(i = 0; i < pomesh->npolys; i++)
    {
      /* for every face, we start with the assumption it is to be split off */
      splitoffthisface = AY_TRUE;

      for(j = 0; j < pomesh->nloops[i]; j++)
	{
	  /* if we still believe we split off this face we continue
	     checking its vertices, otherwise just increase m */
	  if(splitoffthisface)
	    {
	      for(k = 0; k < pomesh->nverts[n]; k++)
		{
		  /* if we still believe we split off this face we continue
		     checking its vertices, otherwise just increase m */
		  if(splitoffthisface)
		    {
		      foundpnt = AY_FALSE;
		      pnt = pnts;
		      while(pnt)
			{
			  v0 = &(pomesh->controlv[pomesh->verts[m]*stride]);
			  v1 = pnt->point;
			  if(AY_V3COMP(v0,v1))
			    {
			      foundpnt = AY_TRUE;
			      break;
			    }
			  pnt = pnt->next;
			} /* while */
		      if(!foundpnt)
			{
			  /* a face vertex was _not_ found in the list of
			     selected points => do not split off this face */
			  splitoffthisface = AY_FALSE;
			  /* increase m as if we continued looping */
			  m += (pomesh->nverts[n]-k);
			  /* now we may break the inner for() and return
			     to the pnt comparison only for the next face
			     (when splitoffthisface is AY_TRUE again) */
			  break;
			} /* if */
		    } /* if */
		  m++;
		} /* for */
	    }
	  else
	    {
	      m += pomesh->nverts[n];
	    } /* if */
	  n++;
	} /* for */

      /* if all vertices of this face were to be found in <pnts>... */
      if(splitoffthisface == AY_TRUE)
	{
	  /* ...split off this face (<i>) */
	  ay_status = ay_pomesht_splitface(pomesh, i, pomesh1);
	}
      else
	{
	  ay_status = ay_pomesht_splitface(pomesh, i, pomesh0);
	} /* if */

    } /* for */

  /* check and return result */
  if(pomesh1->npolys == 0)
    {
      /* oops, no faces were split off */
      ay_pomesht_destroy(pomesh0);
      ay_pomesht_destroy(pomesh1);
      ay_error(AY_ERROR, fname, "No faces were split off, check the point selection!");
      return AY_ERROR;
    }

  if(pomesh1->npolys == pomesh->npolys)
    {
      /* oops, all faces from the original polymesh are in pomesh1
	 => do nothing */
      ay_pomesht_destroy(pomesh0);
      ay_pomesht_destroy(pomesh1);
      ay_error(AY_ERROR, fname, "All faces would be split off, check the point selection!");
      return AY_ERROR;
    } /* if */

  *result = pomesh1;

  /* copy arrays from pomesh0 to original pomesh */
  pomesh->npolys = pomesh0->npolys;
  free(pomesh->nloops);
  pomesh->nloops = pomesh0->nloops;
  free(pomesh->nverts);
  pomesh->nverts = pomesh0->nverts;
  free(pomesh->verts);
  pomesh->verts = pomesh0->verts;
  free(pomesh->controlv);
  pomesh->controlv = pomesh0->controlv;
  pomesh->ncontrols = pomesh0->ncontrols;
  free(pomesh0);

 return ay_status;
} /* ay_pomesht_split */


/** ay_pomesht_splittcmd:
 *  split selected polymesh objects into two, based on their
 *  selected points (split off selected faces)
 *  Implements the \a splitPo scripting interface command.
 *  See also the corresponding section in the \ayd{scsplitpo}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_pomesht_splittcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 /*int ay_status = AY_OK;*/
 ay_object *o = NULL, *newo;
 ay_list_object *sel = ay_selection;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;

      if(o->type == AY_IDPOMESH)
	{
	  if(o->selp)
	    {
	      if(!(newo = calloc(1, sizeof(ay_object))))
		{
		  ay_error(AY_EOMEM, argv[0], NULL);
		  return TCL_OK;
		}

	      ay_pomesht_split((ay_pomesh_object*)o->refine, o->selp,
			       (ay_pomesh_object**)(void*)&(newo->refine));

	      if(newo->refine)
		{
		  o->modified = AY_TRUE;
		  /* update pointers to controlv */
		  ay_selp_clear(o);
		  /* finishing touches for new object */
		  ay_object_defaults(newo);
		  ay_trafo_copy(o, newo);
		  newo->type = AY_IDPOMESH;
		  /* link new object to scene */
		  ay_object_link(newo);
		}
	      else
		{
		  ay_error(AY_ERROR, argv[0], "Split operation failed!");
		  free(newo);
		} /* if */
	    }
	  else
	    {
	      ay_error(AY_ERROR, argv[0], "No point selection.");
	    } /* if */
	}
      else
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_pomesht_splittcmd */


/** ay_pomesht_genfacenormals:
 *  Generate face normals for an arbitrary PolyMesh using Newell's method
 *  which is more robust than a simple cross product.
 *
 *  The generated normal vectors will be normalized.
 *
 * \param[in] po PoMesh object to generate the normals for
 * \param[in,out] result where to store the normals
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_pomesht_genfacenormals(ay_pomesh_object *po, double **result)
{
 unsigned int i, j, k, l = 0, m = 0, n = 0;
 double *fn = NULL, *normals = NULL;
 double *v1, *v2, len;
 int stride = 3;

  if(!po || !result)
    return AY_ENULL;

  if(po->npolys == 0)
    return AY_ERROR;

  if(!(normals = calloc(3*po->npolys, sizeof(double))))
    return AY_EOMEM;
  fn = normals;

  if(po->has_normals)
    stride = 6;

  for(i = 0; i < po->npolys; i++)
    {
      if(po->nloops[l] > 0 && po->nverts[m] > 2)
	{
	  for(k = 0; k < po->nverts[m]-1; k++)
	    {
	      v1 = &(po->controlv[po->verts[n+k]*stride]);
	      v2 = &(po->controlv[po->verts[n+k+1]*stride]);

	      fn[0] += (v1[1] - v2[1]) * (v1[2] + v2[2]);
	      fn[1] += (v1[2] - v2[2]) * (v1[0] + v2[0]);
	      fn[2] += (v1[0] - v2[0]) * (v1[1] + v2[1]);
	    }

	  v1 = &(po->controlv[po->verts[n+k]*stride]);
	  v2 = &(po->controlv[po->verts[n]*stride]);
	  fn[0] += (v1[1] - v2[1]) * (v1[2] + v2[2]);
	  fn[1] += (v1[2] - v2[2]) * (v1[0] + v2[0]);
	  fn[2] += (v1[0] - v2[0]) * (v1[1] + v2[1]);

	  /* normalize */
	  len = AY_V3LEN(fn);
	  if(len > AY_EPSILON)
	    AY_V3SCAL(fn, 1.0/len);
	} /* if */

      /* advance the indices for next poly */
      for(j = 0; j < po->nloops[l]; j++)
	{
	  for(k = 0; k < po->nverts[m]; k++)
	    {
	      n++;
	    }
	  m++;
	}
      fn += 3;
      l++;
    } /* for */

  /* return result */
  *result = normals;

 return AY_OK;
} /* ay_pomesht_genfacenormals */


/** ay_pomesht_gensmoothnormals:
 *  Generate smooth vertex normals for an arbitrary PolyMesh using weighted
 *  mean face normals. Weighting is done via vertex-centroid distance,
 *  which takes both, face area and face shape, into account.
 *  Inner loop (hole) vertices will get the respective face normals.
 *
 *  If the \a result parameter is NULL, the generated normals will be
 *  stored in the PoMesh object. Already existing vertex normals will
 *  be destroyed.
 *  Otherwise, the array returned via \a result will contain the vertex
 *  coordinates and generated normals in the same layout as normally
 *  used by the PoMesh object. The PoMesh itself will not be changed.
 *
 *  The generated normal vectors will be normalized.
 *
 * \param[in,out] po PoMesh object to generate the normals for
 * \param[in,out] result where to store the normals, may be NULL
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_pomesht_gensmoothnormals(ay_pomesh_object *po, double **result)
{
 int ay_status = AY_OK;
 unsigned int a, b, i, j, k, l = 0, m = 0, n = 0;
 double *newcv = NULL, *centroids = NULL, cd[3], cw;
 double len, *fn = NULL, *normal;

  if(!po)
    return AY_ENULL;

  if(po->npolys == 0)
    return AY_ERROR;

  if(!po->has_normals || result)
    {
      if(!(newcv = calloc(po->ncontrols*6, sizeof(double))))
	{ay_status = AY_EOMEM; goto cleanup;}
      a = 0;
      b = 0;
      for(i = 0; i < po->ncontrols; i++)
	{
	  memcpy(&(newcv[a]), &(po->controlv[b]), 3*sizeof(double));
	  a += 6;
	  b += 3;
	}
    }
  else
    {
      /* arrange to overwrite existing vertex normals */
      newcv = po->controlv;
    }

  fn = po->face_normals;
  if(!fn)
    {
      if((ay_status = ay_pomesht_genfacenormals(po, &fn)))
	goto cleanup;
    }

  if(!(centroids = calloc(po->npolys*3, sizeof(double))))
    {ay_status = AY_EOMEM; goto cleanup;}

  for(i = 0; i < po->npolys; i++)
    {
      if(po->nloops[l] > 0)
	{
	  for(k = 0; k < po->nverts[m]; k++)
	    {
	      a = po->verts[n++]*6;
	      centroids[i*3]   += (newcv[a]   / po->nverts[m]);
	      centroids[i*3+1] += (newcv[a+1] / po->nverts[m]);
	      centroids[i*3+2] += (newcv[a+2] / po->nverts[m]);
	    }
	  m++;
	  for(j = 1; j < po->nloops[l]; j++)
	    {
	      n += po->nverts[m];
	      m++;
	    }
	} /* if */
      l++;
    } /* for */

  l = 0; m = 0; n = 0;
  for(i = 0; i < po->npolys; i++)
    {
      if(po->nloops[l] > 0)
	{
	  normal = &(fn[i*3]);
	  /* calc/update weighted normal of outer loops vertices */
	  for(k = 0; k < po->nverts[m]; k++)
	    {
	      a = po->verts[n++]*6;

	      cd[0] = newcv[a]   - centroids[i*3];
	      cd[1] = newcv[a+1] - centroids[i*3+1];
	      cd[2] = newcv[a+2] - centroids[i*3+2];

	      len = AY_V3LEN(cd);
	      if(len > AY_EPSILON)
		cw = 1.0/(len*len);
	      else
		cw = 0.0;

	      newcv[a+3] += normal[0]*cw;
	      newcv[a+4] += normal[1]*cw;
	      newcv[a+5] += normal[2]*cw;
	    } /* for */
	  m++;
	  /* the vertices of the inner loops just get the
	     face normal */
	  for(j = 1; j < po->nloops[l]; j++)
	    {
	      for(k = 0; k < po->nverts[m]; k++)
		{
		  a = po->verts[n++]*6+3;
		  memcpy(&(newcv[a]), normal, 3*sizeof(double));
		}
	      m++;
	    }
	} /* if */
      l++;
    } /* for */

  /* normalize */
  a = 3;
  for(i = 0; i < po->ncontrols; i++)
    {
      normal = &(newcv[a]);
      len = AY_V3LEN(normal);
      if(fabs(len) > AY_EPSILON)
	AY_V3SCAL(normal, 1.0/len);
      a += 6;
    }

  /* return result */
  if(result)
    {
      *result = newcv;
    }
  else
    {
      if(po->controlv != newcv)
	{
	  free(po->controlv);
	  po->controlv = newcv;
	}
      po->has_normals = AY_TRUE;
    }

  newcv = NULL;

cleanup:

  if(newcv)
    free(newcv);

  if(fn)
    free(fn);

  if(centroids)
    free(centroids);

 return AY_OK;
} /* ay_pomesht_gensmoothnormals */


/** ay_pomesht_gennormtcmd:
 * Generate normals for all selected PoMesh objects.
 * Implements the \a genfnPo scripting interface command.
 * Also implements the \a gensnPo scripting interface command.
 * See also the corresponding section in the \ayd{scgenfnpo}.
 * See also the corresponding section in the \ayd{scgensnpo}.
 *
 * \returns TCL_OK in any case.
 */
int
ay_pomesht_gennormtcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 ay_pomesh_object *pomesh;
 double *fn = NULL;
 int smooth = AY_FALSE;
 char *nname = ay_prefs.normalname;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(!strcmp(argv[0], "gensnPo"))
    smooth = AY_TRUE;

  while(sel)
    {
      o = sel->object;

      if(o->type == AY_IDPOMESH)
	{
	  pomesh = (ay_pomesh_object *)o->refine;
	  if(smooth)
	    {
	      if((ay_status = ay_pomesht_gensmoothnormals(pomesh, NULL)))
		{
		  ay_error(ay_status, argv[0], NULL);
		  return TCL_OK;
		}
	    }
	  else
	    {
	      fn = pomesh->face_normals;
	      if(!fn)
		{
		  if((ay_status = ay_pomesht_genfacenormals(pomesh, &fn)))
		    {
		      ay_error(ay_status, argv[0], NULL);
		      return TCL_OK;
		    }
		}

	      ay_pv_add(o, nname, "uniform", "n", pomesh->npolys, 3, fn);

	      if(!pomesh->face_normals)
		free(fn);
	    } /* if */
	}
      else
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_pomesht_gennormtcmd */

