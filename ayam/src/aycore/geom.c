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

/* geom.c - miscellaneous small geometric algorithms */


int
ay_geom_intersectlines3D(double *p1, double *t1,
			 double *p2, double *t2,
			 double *p)
{
 double v[3] = {0}, px[3] = {0};
 double pd[3] = {0};
 double vw = 0.0, vw2 = 0.0;

  AY_V3CROSS(px, t1, t2)
  AY_V3CROSS(v, px, t1)
  vw = AY_V3DOT(v, t2);

  if((vw*vw) < 1e-7)
    return 0;

  AY_V3SUB(pd, p1, p2)
  vw2 = AY_V3DOT(v, pd);
  p[0] = p2[0]+(vw2/vw)*t2[0];
  p[1] = p2[1]+(vw2/vw)*t2[1];
  p[2] = p2[2]+(vw2/vw)*t2[2];

 return 1;
} /* ay_geom_intersectlines3D */


void
ay_geom_calcnfrom3(double *p1, double *p2, double *p3, double *n)
{
 double v1[3] = {0}, v2[3] = {0}, len = 0.0;

  AY_V3SUB(v2, p1, p2)
  AY_V3SUB(v1, p3, p2)
  AY_V3CROSS(n, v1, v2)
  len = AY_V3LEN(n);
  AY_V3SCAL(n, 1.0/len)

 return;
} /* ay_geom_calcnfrom3 */
