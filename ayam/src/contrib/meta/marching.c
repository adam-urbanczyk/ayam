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

/*
** The Marchingcube Algo from Paul Bourke
** found at: http://astronomy.swin.edu.au/pbourke/modelling/polygonise/
** modified by Frank Pagels 2001
*/

#include <math.h>
#include "meta.h"
#include "marching.h"


/*
   Linearly interpolate the position where an isosurface cuts
   an edge between two vertices, each with their own scalar value
*/
void
VertexInterp (double isolevel, meta_xyz * p1, meta_xyz * p2, double valp1, double valp2,
	      meta_xyz * d)
{
  double mu;
  /* XYZ p; */

  if (META_ABS (isolevel - valp1) < 0.00001)
    *d = *p1;			/* return(p1); */
  if (META_ABS (isolevel - valp2) < 0.00001)
    *d = *p2;			/* return(p2); */
  if (META_ABS (valp1 - valp2) < 0.00001)
    *d = *p1;			/* return(p1); */
  mu = (isolevel - valp1) / (valp2 - valp1);
  d->x = p1->x + mu * (p2->x - p1->x);
  d->y = p1->y + mu * (p2->y - p1->y);
  d->z = p1->z + mu * (p2->z - p1->z);

}

/* int Polygonise(GRIDCELL grid,double isolevel,TRIANGLE *triangles) */
int
meta_polygonise (meta_world * w, meta_gridcell * grid, double isolevel)
{
  int i, j, ti;
  int cubeindex, edgeindex;
  meta_xyz vertlist[12];
  meta_xyz normlist[12];
  double *vptr, *nptr;


  /*
     Determine the index into the edge table which
     tells us which vertices are inside of the surface
   */
  cubeindex = 0;

  if (grid->val[0] < isolevel)
    cubeindex |= 1;
  if (grid->val[1] < isolevel)
    cubeindex |= 2;
  if (grid->val[2] < isolevel)
    cubeindex |= 4;
  if (grid->val[3] < isolevel)
    cubeindex |= 8;
  if (grid->val[4] < isolevel)
    cubeindex |= 16;
  if (grid->val[5] < isolevel)
    cubeindex |= 32;
  if (grid->val[6] < isolevel)
    cubeindex |= 64;
  if (grid->val[7] < isolevel)
    cubeindex |= 128;


  w->edgecode = edgeTable[cubeindex];
  edgeindex = edgeTable[cubeindex];

  /* Cube is entirely in/out of the surface */
  if (edgeindex == 0)
    return (0);

  /* Find the vertices where the surface intersects the cube */
  if (edgeindex & 1)
    {
      VertexInterp (isolevel, &grid->p[0], &grid->p[1], grid->val[0],
		    grid->val[1], &vertlist[0]);
      meta_getnormal (w, &vertlist[0], &normlist[0]);
    }
  if (edgeindex & 2)
    {
      VertexInterp (isolevel, &grid->p[1], &grid->p[2], grid->val[1],
		    grid->val[2], &vertlist[1]);
      meta_getnormal (w, &vertlist[1], &normlist[1]);
    }
  if (edgeindex & 4)
    {
      VertexInterp (isolevel, &grid->p[2], &grid->p[3], grid->val[2],
		    grid->val[3], &vertlist[2]);
      meta_getnormal (w, &vertlist[2], &normlist[2]);
    }
  if (edgeindex & 8)
    {
      VertexInterp (isolevel, &grid->p[3], &grid->p[0], grid->val[3],
		    grid->val[0], &vertlist[3]);
      meta_getnormal (w, &vertlist[3], &normlist[3]);
    }
  if (edgeindex & 16)
    {
      VertexInterp (isolevel, &grid->p[4], &grid->p[5], grid->val[4],
		    grid->val[5], &vertlist[4]);
      meta_getnormal (w, &vertlist[4], &normlist[4]);
    }
  if (edgeindex & 32)
    {
      VertexInterp (isolevel, &grid->p[5], &grid->p[6], grid->val[5],
		    grid->val[6], &vertlist[5]);
      meta_getnormal (w, &vertlist[5], &normlist[5]);
    }
  if (edgeindex & 64)
    {
      VertexInterp (isolevel, &grid->p[6], &grid->p[7], grid->val[6],
		    grid->val[7], &vertlist[6]);
      meta_getnormal (w, &vertlist[6], &normlist[6]);
    }
  if (edgeindex & 128)
    {
      VertexInterp (isolevel, &grid->p[7], &grid->p[4], grid->val[7],
		    grid->val[4], &vertlist[7]);
      meta_getnormal (w, &vertlist[7], &normlist[7]);
    }
  if (edgeindex & 256)
    {
      VertexInterp (isolevel, &grid->p[0], &grid->p[4], grid->val[0],
		    grid->val[4], &vertlist[8]);
      meta_getnormal (w, &vertlist[8], &normlist[8]);
    }
  if (edgeindex & 512)
    {
      VertexInterp (isolevel, &grid->p[1], &grid->p[5], grid->val[1],
		    grid->val[5], &vertlist[9]);
      meta_getnormal (w, &vertlist[9], &normlist[9]);
    }
  if (edgeindex & 1024)
    {
      VertexInterp (isolevel, &grid->p[2], &grid->p[6], grid->val[2],
		    grid->val[6], &vertlist[10]);
      meta_getnormal (w, &vertlist[10], &normlist[10]);
    }
  if (edgeindex & 2048)
    {
      VertexInterp (isolevel, &grid->p[3], &grid->p[7], grid->val[3],
		    grid->val[7], &vertlist[11]);
      meta_getnormal (w, &vertlist[11], &normlist[11]);
    }
  /* Create the triangle */

  /*
     vptr = &w->Vertex3d[w->currentnumpoly*9];
     nptr = &w->Vertex3d[w->currentnumpoly*9+w->maxpoly*9];
   */

  vptr = &w->vertex[w->currentnumpoly * 9];
  nptr = &w->nvertex[w->currentnumpoly * 9];

  for (i = 0; triTable[cubeindex][i] != -1; i += 3)
    {

      for (j = 0; j < 3; ++j)
	{
	  ti = triTable[cubeindex][i + j];

	  *nptr++ = normlist[ti].x;
	  *nptr++ = normlist[ti].y;
	  *nptr++ = normlist[ti].z;

	  *vptr++ = vertlist[ti].x;
	  *vptr++ = vertlist[ti].y;
	  *vptr++ = vertlist[ti].z;
	}

      w->currentnumpoly++;


    }

  return (cubeindex);
}
