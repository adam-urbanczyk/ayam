/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2004 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

/* tti.h - truetype font parsing helpers */

/* original version of this file provided by Frank (copper) Pagels */

#ifndef TTI_H_
#define TTI_H_ 1

enum {
  AY_TTI_OK,
  AY_TTI_NOMEM,
  AY_TTI_NOTFOUND,
  AY_TTI_BADFONT
};

typedef struct ay_tti_point_s
{
  double x;
  double y;
} ay_tti_point;

typedef struct ay_tti_outline_s
{
  int filled;
  int numpoints;
  ay_tti_point *points;
} ay_tti_outline;

typedef struct ay_tti_letter_s
{
  double xoffset;
  double yoffset;
  int numoutlines;
  ay_tti_outline *outlines;
} ay_tti_letter;


int ay_tti_getcurves(char *ttfname, int letter, ay_tti_letter *cur);

int ay_tti_outlinetoncurve(ay_tti_outline *outline, ay_object **result);


#endif /* TTI_H_ */
