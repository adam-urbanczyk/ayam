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

/* meta.h:
**  Definitions for the Meta Object
**  2001 Frank Pagels
*/

#ifndef META_H
#define META_H 1

#include <stdio.h>
#include <time.h>
#include "ayam.h"

extern unsigned int metacomp_id;

#define META_USEVERTEXARRAY 0

#define META_ABS(a) (((a) < 0 ) ? -(a) : (a))
#define META_SIGN(a) ((a) < 0 ? -1.0 : 1.0)

#define META_MAXCUBE 80
#define META_MAXPOLY 10000

/* coefficients of influence equation */
#define META_A -.444444
#define META_B 1.888889
#define META_C -2.444444

/* some macros to keep things cleaner */
#define META_SQ(val) ((val) * (val))
#define META_CUB(val) ((val) * (val) * (val))
#define META_DIST(x1, y1, z1, x2, y2, z2) (tmp->scalex*META_SQ(x1 - x2) + tmp->scaley*META_SQ(y1 - y2) + tmp->scalez*META_SQ(z1-z2))
#define META_DIST2(x1, y1, z1, x2, y2, z2) (META_SQ(x1 - x2) + META_SQ(y1 - y2) + META_SQ(z1-z2))

#define META_OK  0
#define META_ERROR  1

#define META_BALL	0
/*#define META2	1*/
/*#define META3	2*/
#define META_TORUS	1
#define META_CUBE	2
#define META_HEART	3
#define META_CUSTOM	4

typedef struct meta_xy_s
{
  double x, y;
}
meta_xy;

typedef struct meta_meta_xyz_s
{
  double x, y, z;
}
meta_xyz;

typedef struct meta_intxyz_s
{
  int x, y, z;
}
meta_intxyz;

typedef struct meta_vertex_s
{
  meta_xyz p;		/* position */
  meta_xyz n;		/* normal */
  double d;		/* density */
}
meta_vertex;

typedef struct meta_blob_s
{
  meta_xyz p;			/* center of the blob */
  meta_xyz cp;			/* coordinates for calculating the blob */

  double r;			/* Squared Radius */

  double Ri;			/* Torus stuff */
  double Ro;

  double a;			/* Energy Coeffizients */
  double b;
  double c;
  double s;			/* Strenght */

  int negativ;
  int rot;
  int formula;
  int ex;				/* Edgesharp for Cubes */
  int ey;
  int ez;
  
  double scalex;
  double scaley;
  double scalez;

  Tcl_Obj *expression; /* compiled expression for custom components */

  GLdouble rm[16];		/* rotation matrix */
  GLdouble tm[16];		/* translation matrix */

}
meta_blob;


typedef struct meta_triangle_s
{
  meta_xyz p[3];
}
meta_triangle;

typedef struct meta_gridcell_s
{
  meta_xyz p[8];
  double val[8];
  meta_intxyz pos;
  meta_xyz n[8];
}
meta_gridcell;

typedef struct meta_grid_s
{
  meta_xyz p;
  double val;
}
meta_grid;

typedef struct meta_world_s
{
  char *mgrid;
  char lastmark;

  meta_grid *grid;
  meta_gridcell cell;
  int aktcubes;			/* current number of cubes */
  double edgelength;
  double isolevel;
  int id;
  ay_object *o;
  double *Vertex3d;

  double *vertex;
  double *nvertex;
  GLuint *vindex;
  int	tablesize;
  int	*vhash;
  int	h1,h2;
  int	actindex;
  int 	indexnum;
    
  int currentnumpoly;
  int maxpoly;
  meta_gridcell *stack;
  int stackpos;
  int maxstack;
  int edgecode;
  double unisize;
  unsigned int *cid;

  int showworld;
  int version;

  int adaptflag;
  
  int adapt;
  double flatness;
  double epsilon;
  double step;

  double scale;

}
meta_world;


void meta_initgrid (meta_world * w);
int meta_calceffect (meta_world * w);
double meta_calcall (double x1, double y1, double z1, meta_world * w);
int meta_polygonise (meta_world * w, meta_gridcell * grid, double isolevel);
void meta_getnormal (meta_world * w, meta_xyz * point, meta_xyz * normal);
void meta_movedown (meta_gridcell * cube, meta_world * w);
void meta_moveup (meta_gridcell * cube, meta_world * w);
void meta_moveleft (meta_gridcell * cube, meta_world * w);
void meta_moveright (meta_gridcell * cube, meta_world * w);
void meta_movefront (meta_gridcell * cube, meta_world * w);
void meta_moveback (meta_gridcell * cube, meta_world * w);
int meta_initcubestack (meta_world * w);
int meta_freecubestack (meta_world * w);


#endif
