/*
**
** routines for adaptive triangulation
**
*/

#include <math.h>
#include "meta.h"

#define STEP_0 0.001
#define MAX_REC 25
#define MAX_REC1 8
#define META_FLAT 1
#define META_NOFLAT 0
#define META_DOT_TOL 0.90

#define midpoint(e, v0, v1, m, w) \
   m.p.x = (v0->p.x + v1->p.x)/2; \
   m.p.y = (v0->p.y + v1->p.y)/2; \
   m.p.z = (v0->p.z + v1->p.z)/2; \
   if (e == META_NOFLAT) \
     m = meta_project_s(w->step,m,meta_calcall(m.p.x,m.p.y,m.p.z,w),MAX_REC,w);

/* prototypes of functions local to this module */

meta_vertex meta_project_s(double s, meta_vertex m, double v, int k,
			   meta_world *w);

void meta_midpoint(int e, meta_vertex *v0, meta_vertex *v1, meta_vertex *m,
		   meta_world *w);

int meta_edgecode(meta_xyz *n1, meta_xyz *n2, meta_world *w);

void meta_tri_adapt(meta_vertex *v0, meta_vertex *v1, meta_vertex *v2,
		    int k, meta_world *w);

void meta_intersect(meta_gridcell *cube, int k0, int k1, meta_vertex *i,
		    meta_world *w);


void meta_quad(meta_gridcell *cube, int k0, int k1, int k2, int k3,
	       meta_world *w);


void meta_tri(meta_gridcell *cube, int k0, int k1, int k2, int k3,
	      meta_world *w);

void meta_simplex(meta_gridcell *cube, int k0, int k1, int k2, int k3,
		  meta_world *w);

void meta_boxscan(meta_world *w, meta_gridcell *cube);


/* functions */

meta_vertex
meta_project_s(double s, meta_vertex m, double v, int k, meta_world *w)
{
 double v1;

  meta_getnormal(w,&m.p,&m.n);

  m.p.x = m.p.x - m.n.x * META_SIGN(v-w->isolevel) * s;
  m.p.y = m.p.y - m.n.y * META_SIGN(v-w->isolevel) * s;
  m.p.z = m.p.z - m.n.z * META_SIGN(v-w->isolevel) * s;

  v1 = meta_calcall(m.p.x, m.p.y, m.p.z, w);

  if((META_ABS(v1-w->isolevel) < w->epsilon)||(--k == 0))
    {
      return m;
    }

  if((v-w->isolevel)*(v1-w->isolevel) < 0)
    {
      s /= 2;
    }

 return meta_project_s(s, m, v1, k, w);
} /* meta_project_s */


void
meta_midpoint(int e, meta_vertex *v0, meta_vertex *v1, meta_vertex *m,
	      meta_world *w)
{
  meta_vertex mm;
  double va;

   mm.p.x = (v0->p.x + v1->p.x)/2;
   mm.p.y = (v0->p.y + v1->p.y)/2;
   mm.p.z = (v0->p.z + v1->p.z)/2;

   if(e == META_NOFLAT)
     {
       va = meta_calcall(mm.p.x, mm.p.y, mm.p.z, w);

       if((META_ABS(va-w->isolevel) < w->epsilon))
	 {
	   meta_getnormal(w, &mm.p, &mm.n);
	   *m = mm;

	 }
       else
	 {
	   *m = meta_project_s(w->step, mm, va, MAX_REC, w);
	 }
     }
   else
     {
       meta_getnormal(w,&mm.p, &mm.n);
       *m = mm;
     }

 return;
} /* meta_midpoint */


int
meta_edgecode(meta_xyz *n1, meta_xyz *n2, meta_world *w)
{
  if ((n1->x*n2->x + n1->y*n2->y + n1->z*n2->z) < w->flatness)
    return META_NOFLAT;
  else
    return META_FLAT;
} /* meta_edgecode */


void
meta_tri_adapt(meta_vertex *v0, meta_vertex *v1, meta_vertex *v2,
	       int k, meta_world *w)
{
 int e1, e2, e3;
 meta_vertex m1, m2, m3;
 double *vptr, *nptr;

  k--;

  e1 = meta_edgecode(&v0->n,&v1->n,w);
  e2 = meta_edgecode(&v1->n,&v2->n,w);
  e3 = meta_edgecode(&v2->n,&v0->n,w);

  if((e1 && e2 && e3)||(k == 0))
     {
       if (w->currentnumpoly >= (w->maxpoly))
	 {
	   w->vertex = realloc(w->vertex,
			 sizeof(double) * 3 * 3 * (w->maxpoly + 10000 + 20));
	   w->nvertex = realloc(w->nvertex,
			 sizeof(double) * 3 * 3 * (w->maxpoly + 10000 + 20));
	   w->maxpoly += 10000;
	 }

       vptr = &w->vertex[w->currentnumpoly * 9];
       nptr = &w->nvertex[w->currentnumpoly * 9];

       *nptr++ = v0->n.x;
       *nptr++ = v0->n.y;
       *nptr++ = v0->n.z;

       *nptr++ = v1->n.x;
       *nptr++ = v1->n.y;
       *nptr++ = v1->n.z;

       *nptr++ = v2->n.x;
       *nptr++ = v2->n.y;
       *nptr++ = v2->n.z;

       *vptr++ = v0->p.x;
       *vptr++ = v0->p.y;
       *vptr++ = v0->p.z;

       *vptr++ = v1->p.x;
       *vptr++ = v1->p.y;
       *vptr++ = v1->p.z;

       *vptr++ = v2->p.x;
       *vptr++ = v2->p.y;
       *vptr++ = v2->p.z;

       w->currentnumpoly++;
     }
   else
     {
       meta_midpoint(e1, v0, v1, &m1, w);
       meta_midpoint(e2, v1, v2, &m2, w);
       meta_midpoint(e3, v2, v0, &m3, w);

       meta_tri_adapt(v0, &m1, &m3, k, w);
       meta_tri_adapt(v1, &m2, &m1, k, w);
       meta_tri_adapt(v2, &m3, &m2, k, w);
       meta_tri_adapt(&m1, &m2, &m3, k, w);
     }

 return;
} /* meta_tri_adapt */


void
meta_intersect(meta_gridcell *cube, int k0, int k1, meta_vertex *i,
	       meta_world *w)
{
 double t, tt;
 meta_vertex m;

  t = (w->isolevel - cube->val[k0])/(cube->val[k1]- cube->val[k0]);
  tt = 1-t;
  m.p.x = (cube->p[k0].x * tt) + (cube->p[k1].x * t);
  m.p.y = (cube->p[k0].y * tt) + (cube->p[k1].y * t);
  m.p.z = (cube->p[k0].z * tt) + (cube->p[k1].z * t);

  *i = meta_project_s(w->step, m, meta_calcall(m.p.x, m.p.y, m.p.z, w),
		      MAX_REC, w);

  meta_getnormal(w, &i->p, &i->n);

 return;
} /* meta_intersect */


void
meta_quad(meta_gridcell *cube, int k0, int k1, int k2, int k3, meta_world *w)
{
 meta_vertex i0, i1, i2, i3;

  meta_intersect(cube,k0,k2,&i0,w);
  meta_intersect(cube,k0,k3,&i1,w);
  meta_intersect(cube,k1,k3,&i2,w);
  meta_intersect(cube,k1,k2,&i3,w);

  meta_tri_adapt(&i0,&i1,&i2,MAX_REC1,w);
  meta_tri_adapt(&i0,&i2,&i3,MAX_REC1,w);

 return;
} /* meta_quad */

void
meta_tri(meta_gridcell *cube, int k0, int k1, int k2, int k3, meta_world *w)
{
 meta_vertex i0, i1, i2;

  meta_intersect(cube,k0,k1,&i0,w);
  meta_intersect(cube,k0,k2,&i1,w);
  meta_intersect(cube,k0,k3,&i2,w);

  meta_tri_adapt(&i0,&i1,&i2,MAX_REC1,w);

 return;
} /* meta_tri */

void
meta_simplex(meta_gridcell *cube, int k0, int k1, int k2, int k3,
	     meta_world *w)
{
 double v0, v1, v2, v3;

  v0 = cube->val[k0]-w->isolevel;
  v1 = cube->val[k1]-w->isolevel;
  v2 = cube->val[k2]-w->isolevel;
  v3 = cube->val[k3]-w->isolevel;

  if(v0 < 0)
    {
      if(v1 < 0)
	{
	  if(v2 < 0)
	    {
	      if(v3 > 0)
		meta_tri(cube,k3,k2,k1,k0,w);
	    }
	  else
	    {
	      if(v3 < 0)
		meta_tri(cube,k2,k0,k1,k3,w);
	      else
		meta_quad(cube,k2,k3,k0,k1,w);
	    } /* if */
	}
      else
	{
	  if(v2 < 0)
	    {
	      if(v3 < 0)
		meta_tri(cube,k1,k3,k2,k0,w);
	      else
		meta_quad(cube,k1,k3,k2,k0,w);
	    }
	  else
	    {
	      if(v3 < 0)
		meta_quad(cube,k1,k2,k0,k3,w);
	      else
		meta_tri(cube,k0,k3,k2,k1,w);
	    } /* if */
	} /* if */
    }
  else
    {
      if(v1 < 0)
	{
	  if(v2 < 0)
	    {
	      if(v3 < 0)
		meta_tri(cube,k0,k1,k2,k3,w);
	      else
		meta_quad(cube,k0,k3,k1,k2,w);
	    }
	  else
	    {
	      if(v3 < 0)
		meta_quad(cube,k0,k2,k3,k1,w);
	      else
		meta_tri(cube,k1,k3,k0,k2,w);
	    } /* if */
	}
      else
	{
	  if(v2 < 0)
	    {
	      if(v3 < 0)
		meta_quad(cube,k0,k1,k2,k3,w);
	      else
		meta_tri(cube,k2,k3,k1,k0,w);
	    }
	  else
	    {
	      if(v3 < 0)
		meta_tri(cube,k3,k0,k1,k2,w);
	    } /* if */
	} /* if */
    } /* if */

 return;
} /* meta_simplex */

void
meta_boxscan(meta_world *w, meta_gridcell *cube)
{

   meta_simplex(cube,3,2,6,5,w);
   meta_simplex(cube,3,1,2,5,w);
   meta_simplex(cube,3,6,7,5,w);
   meta_simplex(cube,3,7,4,5,w);
   meta_simplex(cube,3,0,1,5,w);
   meta_simplex(cube,3,4,0,5,w);

  return;
} /* meta_boxscan */

