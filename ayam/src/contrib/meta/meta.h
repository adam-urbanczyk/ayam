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

#define PROGNAME "Meta Object Modul"
#define VERSION	 "0.9"


#define ABS(a) (((a) < 0 ) ? -(a) : (a))

#define MAXCUBE 80
#define MAXPOLY 10000

/* coefficients of influence equation */
#define A -.444444
#define B 1.888889
#define C -2.444444

/* some macros to keep things cleaner */
#define SQ(val) ((val) * (val))
#define CUBE(val) ((val) * (val) * (val))
#define DIST(x1, y1, z1, x2, y2, z2) (tmp->scalex*SQ(x1 - x2) + tmp->scaley*SQ(y1 - y2) + tmp->scalez*SQ(z1-z2))

#define META_OK  0
#define META_ERROR  1

#define SHADE	0
#define WIRE	1

#define META1	0
#define META2	1
#define META3	2
#define TORUS	3
#define HEART	4

typedef struct xy
{
  double x, y;
}
XY;

typedef struct xyz
{
  double x, y, z;
}
XYZ;

typedef struct intxyz
{
  int x, y, z;
}
INTXYZ;


typedef struct BLOB
{
  XYZ p;			/* center of the blob */
  XYZ cp;			/* coordinates for calculating the blob */

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

  double scalex;
  double scaley;
  double scalez;

  GLdouble rm[16];		/* rotation matrix */
  GLdouble tm[16];		/* translation matrix */

}
blob;


typedef struct triangle
{
  XYZ p[3];
}
TRIANGLE;

typedef struct gridcell
{
  XYZ p[8];
  double val[8];
  INTXYZ pos;
}
GRIDCELL;

typedef struct grid
{
  XYZ p;
  double val;
}
GRID;

typedef struct WORLD
{
  char *mgrid;
  char lastmark;

  GRID *grid;
  GRIDCELL cell;
  int aktcubes;			/* current number of cubes */
  double edgelength;
  double isolevel;
  int id;
  ay_object *o;
  double *Vertex3d;

  double *vertex;
  double *nvertex;

  int currentnumpoly;
  int maxpoly;
  GRIDCELL *stack;
  int stackpos;
  int maxstack;
  int edgecode;
  double unisize;

}
world;


void initgrid (world * w);
int calcballs (world * w, int mode);
double calcall (double x1, double y1, double z1, world * w);
int Polygonise (world * w, GRIDCELL * grid, double isolevel, int mode);
void getnormal (world * w, XYZ * point, XYZ * normal);
void movedown (GRIDCELL * cube, world * w);
void moveup (GRIDCELL * cube, world * w);
void moveleft (GRIDCELL * cube, world * w);
void moveright (GRIDCELL * cube, world * w);
void movefront (GRIDCELL * cube, world * w);
void moveback (GRIDCELL * cube, world * w);
int initcubestack (world * w);
int freecubestack (world * w);


#endif
