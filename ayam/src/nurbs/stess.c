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

#include "ayam.h"

/* stess.c simple NURB tesselators */

/* definition of types local to this module: */

/* a tesselated point */
typedef struct ay_stess_uvp_s {
  struct ay_stess_uvp_s *next;
  int type;    /* 0 - original point, 1 - trimloop point */
  int dir;     /* direction of associated trimcurve, 1 - ccw, 0 - cw */
  double u, v; /* associated parametric values of this point */
  double C[4]; /* coordinates of this point */
} ay_stess_uvp;

/* a list of lists (lines) of tesselated points */
typedef struct ay_stess_uvpl_s {
  struct stess_uvpl_s *next; /* next list (line) of points */
  struct stess_uvp_s *uvp;   /* a list (line) of points */
} ay_stess_uvpl;


/* local preprocessor definitions: */
#define AY_STESSEPSILON 0.000001


/* prototypes of functions local to this module: */


/* functions: */

/* ay_stess_2DCurvePoints4D:
 *   calculate all points of a 2D curve (TrimCurve)
 */
int
ay_stess_2DCurvePoints4D(int n, int p, double *U, double *Pw, int num,
			 int *Clen, double **C)
{
 int span, j, k, l, m;
 double *N = NULL, Cw[3], *Ct = NULL, u, ud, umin, umax;

  if(!(N = calloc(p+1, sizeof(double))))
    return AY_EOMEM;

  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p+n;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  *Clen = (n*num);

  if(!(Ct = calloc(*Clen*2, sizeof(double))))
    {
      free(N);
      return AY_EOMEM;
    }

  ud = (umax-umin)/((*Clen)-1);
  u = umin;

  for(l = 0; l < (*Clen); l++)
    {
      span = ay_nb_FindSpan(n, p, u, U);

      ay_nb_BasisFuns(span, u, p, U, N);

      memset(Cw, 0, 3*sizeof(double));

      for(j = 0; j <= p; j++)
	{
	  k = (span-p+j)*4;
	  Cw[0] = Cw[0] + N[j]*Pw[k];
	  Cw[1] = Cw[1] + N[j]*Pw[k+1];
	  Cw[2] = Cw[2] + N[j]*Pw[k+3];
	}

	m = l*2;
	Ct[m]   = Cw[0]/Cw[2];
	Ct[m+1] = Cw[1]/Cw[2];

      u += ud;
    } /* for */

  *C = Ct;

  free(N);

 return AY_OK;
} /* ay_stess_2DCurvePoints4D */


/* ay_stess_2DCurvePoints3D:
 *   calculate all points of a 2D curve (TrimCurve)
 */
int
ay_stess_2DCurvePoints3D(int n, int p, double *U, double *P, int num,
			 int *Clen, double **C)
{
 int span, j, k, l, m;
 double *N = NULL, *Ct = NULL, u, ud, umin, umax;

  if(!(N = calloc(p+1, sizeof(double))))
    return AY_EOMEM;

  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p+n;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  *Clen = n*num;

  if(!(Ct = calloc(*Clen*2, sizeof(double))))
    return AY_EOMEM;

  ud = (umax-umin)/((*Clen)-1);
  u = umin;

  for(l = 0; l < (*Clen); l++)
    {
      span = ay_nb_FindSpan(n, p, u, U);

      ay_nb_BasisFuns(span, u, p, U, N);

      m = l*2;
      for(j = 0; j <= p; j++)
	{
	  k = (span-p+j)*3;
	  Ct[m]   = Ct[m]   + N[j]*P[k];
	  Ct[m+1] = Ct[m+1] + N[j]*P[k+1];
	}

      u += ud;
    }

  *C = Ct;

  free(N);

 return AY_OK;
} /* ay_stess_2DCurvePoints3D */


/* ay_stess_CurvePoints4D:
 *   calculate all points of a 3D curve with weights
 */
int
ay_stess_CurvePoints4D(int n, int p, double *U, double *Pw, int num,
		       int *Clen, double **C)
{
 int span, j, k, l, m;
 double *N = NULL, Cw[4], *Ct = NULL, u, ud, umin, umax;

  if(!(N = calloc(p+1, sizeof(double))))
    return AY_EOMEM;

  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p+n;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  *Clen = (n*num);

  if(!(Ct = calloc(*Clen*3, sizeof(double))))
    return AY_EOMEM;

  ud = (umax-umin)/((*Clen)-1);
  u = umin;

  for(l = 0; l < (*Clen); l++)
    {
      span = ay_nb_FindSpan(n, p, u, U);

      ay_nb_BasisFuns(span, u, p, U, N);

      memset(Cw, 0, 4*sizeof(double));

      for(j = 0; j <= p; j++)
	{
	  k = (span-p+j)*4;
	  Cw[0] = Cw[0] + N[j]*Pw[k];
	  Cw[1] = Cw[1] + N[j]*Pw[k+1];
	  Cw[2] = Cw[2] + N[j]*Pw[k+2];
	  Cw[3] = Cw[3] + N[j]*Pw[k+3];
	}

	m = l*3;
	Ct[m]   = Cw[0]/Cw[3];
	Ct[m+1] = Cw[1]/Cw[3];
	Ct[m+2] = Cw[2]/Cw[3];

      u += ud;
    }

  *C = Ct;

  free(N);

 return AY_OK;
} /* ay_stess_CurvePoints4D */


/* ay_stess_CurvePoints3D:
 *   calculate all points of a 3D curve
 */
int
ay_stess_CurvePoints3D(int n, int p, double *U, double *P, int num,
		       int *Clen, double **C)
{
 int span, j, k, l, m;
 double *N = NULL, *Ct = NULL, u, ud, umin, umax;

  if(!(N = calloc(p+1, sizeof(double))))
    return AY_EOMEM;

  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p+n;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  *Clen = n*num;

  if(!(Ct = calloc(*Clen*3, sizeof(double))))
    return AY_EOMEM;

  ud = (umax-umin)/((*Clen)-1);
  u = umin;

  for(l = 0; l < (*Clen); l++)
    {
      span = ay_nb_FindSpan(n, p, u, U);

      ay_nb_BasisFuns(span, u, p, U, N);

      m = l*3;
      for(j = 0; j <= p; j++)
	{
	  k = (span-p+j)*3;
	  Ct[m]   = Ct[m]   + N[j]*P[k];
	  Ct[m+1] = Ct[m+1] + N[j]*P[k+1];
	  Ct[m+2] = Ct[m+2] + N[j]*P[k+2];
	}

      u += ud;
    }

  *C = Ct;

  free(N);

 return AY_OK;
} /* ay_stess_CurvePoints3D */


/* ay_stess_SurfacePoints4D:
 *   calculate all points of a surface with weights
 */
int
ay_stess_SurfacePoints4D(int n, int m, int p, int q, double *U, double *V,
			 double *Pw, int num, int *Cn, int *Cm, double **C)
{
 int spanu = 0, spanv = 0, indu = 0, indv = 0, l = 0, k = 0, i = 0, j = 0;
 int a, b;
 double u, v, ud, vd, umin, umax, vmin, vmax, *Nu = NULL, *Nv = NULL;
 double Cw[4] = {0}, *Ct = NULL, temp[4] = {0};

  if(!(Nu = calloc(p+1, sizeof(double))))
    return AY_EOMEM;
  if(!(Nv = calloc(q+1, sizeof(double))))
    return AY_EOMEM;

  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p+n;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  *Cn = n*num;
  ud = (umax-umin)/((*Cn)-1);
  u = umin;

  j = 0;
  while(V[j] == V[j+1])
    {
      j++;
    }
  vmin = V[j];

  j = q+m;
  while(V[j] == V[j-1])
    {
      j--;
    }
  vmax = V[j];

  *Cm = m*num;
  vd = (umax-umin)/((*Cm)-1);

  if(!(Ct = calloc((*Cn)*(*Cm)*3, sizeof(double))))
    return AY_EOMEM;

  for(a = 0; a < (*Cn); a++)
    {
      v = vmin;
      for(b = 0; b < (*Cm); b++)
	{

	  spanu = ay_nb_FindSpan(n, p, u, U);
	  ay_nb_BasisFuns(spanu, u, p, U, Nu);
	  spanv = ay_nb_FindSpan(m, q, v, V);
	  ay_nb_BasisFuns(spanv, v, q, V, Nv);

	  indu = spanu - p;
	  for(l =0 ; l <= q; l++)
	    {
	      memset(temp, 0, 4*sizeof(double));
	      indv = spanv - q + l;

	      for(k = 0; k <= p; k++)
		{
		  /* was: temp = temp + Nu[k]*Pw[indu+k][indv]; */
		  i = (((indu+k)*(m+1))+indv)*4;

		  temp[0] += Nu[k]*Pw[i];
		  temp[1] += Nu[k]*Pw[i+1];
		  temp[2] += Nu[k]*Pw[i+2];
		  temp[3] += Nu[k]*Pw[i+3];
		} /* for */
	      /* was: Cw = Cw + Nv[l]*temp */
	      Cw[0] += Nv[l]*temp[0];
	      Cw[1] += Nv[l]*temp[1];
	      Cw[2] += Nv[l]*temp[2];
	      Cw[3] += Nv[l]*temp[3];
	    } /* for */

	  j = (a*(*Cn)+b)*3;
	  Ct[j]   = Cw[0]/Cw[3];
	  Ct[j+1] = Cw[1]/Cw[3];
	  Ct[j+2] = Cw[2]/Cw[3];

	  v += vd;
	} /* for */

      u += ud;
    } /* for */

  *C = Ct;

  free(Nu);
  free(Nv);

 return AY_OK;
} /* ay_stess_SurfacePoints4D */


/* ay_stess_SurfacePoints3D:
 *   calculate all points of a surface
 */
int
ay_stess_SurfacePoints3D(int n, int m, int p, int q, double *U, double *V,
			 double *P, int num, int *Cn, int *Cm, double **C)
{
 int spanu = 0, spanv = 0, indu = 0, indv = 0, l = 0, k = 0, i = 0, j = 0;
 int a, b;
 double u, v, ud, vd, umin, umax, vmin, vmax, *Nu = NULL, *Nv = NULL;
 double temp[3] = {0}, *Ct = NULL;

  if(!(Nu = calloc(p+1, sizeof(double))))
    return AY_EOMEM;
  if(!(Nv = calloc(q+1, sizeof(double))))
    return AY_EOMEM;

  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p+n;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  *Cn = n*num;
  ud = (umax-umin)/((*Cn)-1);
  u = umin;

  j = 0;
  while(V[j] == V[j+1])
    {
      j++;
    }
  vmin = V[j];

  j = q+m;
  while(V[j] == V[j-1])
    {
      j--;
    }
  vmax = V[j];

  *Cm = m*num;
  vd = (umax-umin)/((*Cm)-1);

  if(!(Ct = calloc((*Cn)*(*Cm)*3, sizeof(double))))
    return AY_EOMEM;

  for(a = 0; a < (*Cn); a++)
    {
      v = vmin;
      for(b = 0; b < (*Cm); b++)
	{

	  spanu = ay_nb_FindSpan(n, p, u, U);
	  ay_nb_BasisFuns(spanu, u, p, U, Nu);
	  spanv = ay_nb_FindSpan(m, q, v, V);
	  ay_nb_BasisFuns(spanv, v, q, V, Nv);

	  indu = spanu - p;
	  j = (a*(*Cn)+b)*3;
	  for(l = 0; l <= q; l++)
	    {
	      memset(temp, 0, 3*sizeof(double));
	      indv = spanv - q + l;

	      for(k = 0; k <= p; k++)
		{
		  /* was: temp = temp + Nu[k]*P[indu+k][indv]; */
		  i = (((indu+k)*(m+1))+indv)*3;

		  temp[0] += Nu[k]*P[i];
		  temp[1] += Nu[k]*P[i+1];
		  temp[2] += Nu[k]*P[i+2];
		} /* for */
	      /* was: Cw = Cw + Nv[l]*temp */
	      Ct[j]   += Nv[l]*temp[0];
	      Ct[j+1] += Nv[l]*temp[1];
	      Ct[j+2] += Nv[l]*temp[2];
	    } /* for */

	  v += vd;
	} /* for */

      u += ud;
    } /* for */

  *C = Ct;

  free(Nu);
  free(Nv);

 return AY_OK;
} /* ay_stess_SurfacePoints3D */


/* ay_stess_IntersectLines2D:
 *  Code taken from the c.g.algorithms FAQ, which in turn points to:
 *  Graphics Gems III pp. 199-202 "Faster Line Segment Intersection"
 *
 *  Input:   p1[2],p2[2]; p3[2],p4[2] - 2 2D line segments
 *  Returns: 0 - no intersection in the given segments
 *           1 - segments intersect in point ip
 */
int
ay_stess_IntersectLines2D(double *p1, double *p2, double *p3, double *p4,
			  double *ip)
{
 double r, s, den;

  den = ((p2[0]-p1[0])*(p4[1]-p3[1])-(p2[1]-p1[1])*(p4[0]-p3[0]));

  r = ((p1[1]-p3[1])*(p4[0]-p3[0])-(p1[0]-p3[0])*(p4[1]-p3[1]))/den;

  if((r < 0.0) || (r > 1.0))
    {
      return 0; /* XXXX early exit! */
    }

  s = ((p1[1]-p3[1])*(p2[0]-p1[0])-(p1[0]-p3[0])*(p2[1]-p1[1]))/den;

  if((s < 0.0) || (s > 1.0))
    {
      return 0; /* XXXX early exit! */
    }

  ip[0] = p1[0]+r*(p2[0]-p1[0]);
  ip[1] = p1[1]+r*(p2[1]-p1[1]);

 return 1;
} /* ay_stess_IntersectLines2D */


/* ay_stess_TessTrimCurves:
 *  tesselate all trim curves of object <o>
 */
int
ay_st_TessTrimCurves(ay_object *o, int num, int *nt, double ***tt,
		     int **tl, int **td)
{
 int ay_status = AY_OK;
 double *dtmp = NULL, angle, **tts, p1[4], p2[4];
 int i, j, numtrims = 0, *tls, *tds, got_object = 0;
 ay_object *d = NULL, *dd = NULL;
 ay_nurbcurve_object *c = NULL;
 GLfloat m[4][4];
 GLdouble mm[16];

  /* count trimloops */
  d = o->down;
  while(d->next)
    {
      if((d->type == AY_IDNCURVE) ||
	 (ay_provide_object(d, AY_IDNCURVE, NULL) == AY_OK) ||
	 ((d->type == AY_IDLEVEL) && (d->down) && (d->down->next)))
	numtrims++;
      d = d->next;
    } /* while */

  if(numtrims == 0)
    return AY_OK;

  /* tesselate trimloops and get their orientation */
  if(!(tts = calloc(numtrims, sizeof(double *))))
    return AY_EOMEM;
  if(!(tls = calloc(numtrims, sizeof(int))))
    { free(tts); return AY_EOMEM; }
  if(!(tds = calloc(numtrims, sizeof(int))))
    { free(tts); free(tls); return AY_EOMEM; }

  i = 0;
  d = o->down;
  while(d)
    {
      if(d->type == AY_IDNCURVE)
	{
	  c = (ay_nurbcurve_object *)d->object;
	  glMatrixMode(GL_MODELVIEW);
	  glPushMatrix();
	   glLoadIdentity();
	   glTranslated(d->movx, d->movy, d->movz);
	   ay_build_rotmatrix(m, d->quat);
	   glMultMatrixf(&(m[0][0]));
	   glScaled(d->scalx, d->scaly, d->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	}
      else
	{
	  dd = NULL;
	  ay_status = ay_provide_object(d, AY_IDNCURVE, &dd);
	  if(dd)
	    got_object = AY_TRUE;

	  c = (ay_nurbcurve_object *)dd->object;

	  glMatrixMode(GL_MODELVIEW);
	  glPushMatrix();
	   glLoadIdentity();
	   glTranslated(dd->movx, dd->movy, dd->movz);
	   ay_build_rotmatrix(m, dd->quat);
	   glMultMatrixf(&(m[0][0]));
	   glScaled(dd->scalx, dd->scaly, dd->scalz);
	   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
	  glPopMatrix();
	} /* if */

      if(!(dtmp = calloc(c->length*4, sizeof(double))))
	{ return AY_EOMEM; } /* XXXX Memory Leak? */

      /* apply transformations */

      for(j = 0; j < c->length; j++)
	{
	  memcpy(p1, &(c->controlv[j*4]), 4*sizeof(double));
	  AY_APTRAN4(p2, p1, mm)
	  memcpy(&(dtmp[j*4]), p2, 4*sizeof(double));
	}

      ay_stess_2DCurvePoints4D(c->length-1, c->order-1, c->knotv, dtmp,
			       num, &(tls[i]), &(tts[i]));

      free(dtmp);
      dtmp = NULL;

      /* get orientation of trimloop */
      angle = 0.0;
      ay_nct_getorientation(c, &angle);
      if(angle > 0.0)
	tds[i] = 1;

      if(got_object)
	ay_object_deletemulti(dd);

      i++;
      d = d->next;
    } /* while */

  *nt = numtrims;
  *tt = tts;
  *tl = tls;
  *td = tds;

 return AY_OK;
} /* ay_stess_TessTrimCurves */


/* ay_stess_MergeUVectors:
 *   merge two vectors (lines) of points on a surface, removing
 *   double points in the progress
 */
int
ay_stess_MergeUVectors(ay_stess_uvp *a, ay_stess_uvp *b)
{
 ay_stess_uvp *p1, *p2, *p3;
 int done = AY_FALSE, inserted = 0, count = 0, toggle = 0;

  p2 = b;
  while(p2)
    {
      if(toggle)
	toggle = 0;
      else
	toggle = 1;

      count++;
      p2 = p2->next;
    } /* while */

  /* never insert uneven numbers of points! */
  if(toggle)
    {
      /*
     fprintf(stderr,"Uneven number of trimloop points (%d) detected!\n",count);
      */

      /* free b */
      while(b)
	{
	  p2 = b->next;
	  free(b);
	  b = p2;
	}

      return AY_OK;
    } /* if */

  while(!done)
    {
      inserted = 0;
      p1 = a;
      p2 = b;
      while(!inserted)
	{
	  if(p1->next)
	    {
	      if(p2)
		if(p2->v == p1->v)
		  { /* Danger! Check for intersecting trimloops: */
		    if(p1->type == 1)
		      return AY_ERROR; /* XXXX early exit! */

		    /* We, accidentally, have here a trimloop
		     * point that is identical to a wanted uv-point;
		     * we therefore have to transmogrify the uv-point to
		     * a trimloop point and delete the original trimloop
		     * point!
		     */
		    fprintf(stderr,"Transmogrifying point!\n");
		    p1->type = 1;
		    p1->dir = p2->dir;
		    p3 = p2->next;
		    free(p2);
		    p2 = p3;
		    b = p3;
		    inserted = 1;
		  }

	      if(p2)
		if((p2->v > p1->v) && (p2->v < p1->next->v))
		  {
		    p3 = p1->next;
		    p1->next = p2;
		    b = p2->next;
		    p2->next = p3;
		    inserted = 1;
		  }
		else
		  {
		    p1 = p1->next;
		  } /* if */
	    } /* if */
	} /* while */

      if(!b)
	done = AY_TRUE;

    } /* while */

 return AY_OK;
} /* ay_stess_MergeUVectors */


/* ay_stess_MergeVVectors:
 *   merge two vectors (lines) of points on a surface, removing
 *   double points in the progress
 */
int
ay_stess_MergeVVectors(ay_stess_uvp *a, ay_stess_uvp *b)
{
 ay_stess_uvp *p1, *p2, *p3;
 int done = AY_FALSE, inserted = 0, count = 0, toggle = 0;

  p2 = b;
  while(p2)
    {
      if(toggle)
	toggle = 0;
      else
	toggle = 1;

      count++;
      p2 = p2->next;
    } /* while */

  /* never insert uneven numbers of points! */
  if(toggle)
    {
      /*
     fprintf(stderr,"Uneven number of trimloop points (%d) detected!\n",count);
      */

      /* free b */
      while(b)
	{
	  p2 = b->next;
	  free(b);
	  b = p2;
	}

      return AY_OK;
    } /* if */

  while(!done)
    {
      inserted = 0;
      p1 = a;
      p2 = b;
      while(!inserted)
	{
	  if(p1->next)
	    {
	      if(p2)
		if(p2->u == p1->u)
		  { /* Danger! Check for intersecting trimloops: */
		    if(p1->type == 1)
		      return AY_ERROR; /* XXXX early exit! */

		    /* We, accidentally, have here a trimloop
		     * point that is identical to a wanted uv-point;
		     * we therefore have to transmogrify the uv-point to
		     * a trimloop point and delete the original trimloop
		     * point!
		     */
		    fprintf(stderr,"Transmogrifying point!\n");
		    p1->type = 1;
		    p1->dir = p2->dir;
		    p3 = p2->next;
		    free(p2);
		    p2 = p3;
		    b = p3;
		    inserted = 1;
		  }

	      if(p2)
		if((p2->u > p1->u) && (p2->u < p1->next->u))
		  {
		    p3 = p1->next;
		    p1->next = p2;
		    b = p2->next;
		    p2->next = p3;
		    inserted = 1;
		  }
		else
		  {
		    p1 = p1->next;
		  } /* if */
	    } /* if */
	} /* while */

      if(!b)
	done = AY_TRUE;

    } /* while */

 return AY_OK;
} /* ay_stess_MergeVVectors */


/* ay_stess_TessTrimmedNPU:
 *  tesselate NURBS patch <o> into lines in parametric direction u
 */
int
ay_stess_TessTrimmedNPU(Tcl_Interp *interp, ay_object *o, int num,
			int numtrims,
			double **tcs, int *tcslens, int *tcsdirs,
			int *flcw, ay_stess_uvp ***result)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *p = NULL;
 ay_stess_uvp **uvps = NULL, *uvpptr, *newuvp, **olduvp, *trimuvp;
 ay_stess_uvp *uvpptr2, *uvpptr3;
 double *tt, ipoint[2] = {0};
 double p3[2], p4[2], *U, *V, u, v;
 double umin, umax, vmin, vmax, ud, vd;
 int i, j, k, l, ind;
 int out = 0, first_loop = AY_TRUE, first_loop_cw = AY_FALSE;
 int Cm, Cn, trimloop_point, done;
 char fname[] = "TessTrimmedNPU";

  p = (ay_nurbpatch_object *)o->object;

  /* calc desired uv coords for patch tesselation */
  U = p->uknotv;
  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p->width-1+p->uorder-1;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  Cn = (p->width-1)*num;
  ud = (umax-umin)/((Cn)-1);
  u = umin;

  if(!(uvps = calloc(Cn, sizeof(ay_stess_uvp *))))
    {
      return AY_EOMEM;
    }
  V = p->vknotv;
  j = 0;
  while(V[j] == V[j+1])
    {
      j++;
    }
  vmin = V[j];

  j = p->height-1+p->vorder-1;
  while(V[j] == V[j-1])
    {
      j--;
    }
  vmax = V[j];

  Cm = (p->height-1)*num;
  vd = (umax-umin)/((Cm)-1);

  /* fill desired uv-coords */
  for(i = 0; i < Cn; i++)
    {
      v = vmin;
      olduvp = &(uvps[i]);
      for(j = 0; j < Cm; j++)
	{
	  if(!(newuvp = calloc(1, sizeof(ay_stess_uvp))))
	    {
	      return AY_EOMEM;
	    }
	  /* type == 0 */
	  newuvp->u = u;
	  newuvp->v = v;
	  *olduvp = newuvp;
	  olduvp = &(newuvp->next);
	  v += vd;
	} /* for */
      u += ud;
    } /* for */

  u = umin;
  /* match desired uv coords of patch tesselation with trimloops */
  for(i = 0; i < Cn; i++)
    {
      v = vmin;
      olduvp = &trimuvp;
      trimuvp = NULL;
      p3[0] = u;
      p4[0] = u;
      /* calc all intersections of all trimloops with current u */
      for(j = 0; j < (Cm-1); j++)
	{
	  p3[1] = v;
	  p4[1] = v+vd;
	  for(k = 0; k < numtrims; k++)
	    {
	      tt = tcs[k];

	      for(l = 0; l < (tcslens[k]-1); l++)
		{
		  /* pre-select trimloop section */
		  ind = l*2;
		  if(((tt[ind] <= u) && (tt[ind+2] >= u)) ||
		     ((tt[ind] >= u) && (tt[ind+2] <= u)))
		    {
		      ipoint[0] = 0.0;
		      ipoint[1] = 0.0;

		      if((ay_stess_IntersectLines2D(&(tt[ind]), &(tt[ind+2]),
						    p3, p4, ipoint)))
			{ /* u-line intersects with trimcurve */

			  /* test commented out, because it will make
			     problems with uvp-types */

			  /* check whether ipoint is farther away enough
			     from current v, v+vd */
			  /* if((fabs(ipoint[1] - v) > AY_STESSEPSILON) &&
			     (fabs(ipoint[1] - (v+vd)) > AY_STESSEPSILON))
			     {*/ /* yes it is */
			  /* add new point */
			  if(!(newuvp = calloc(1, sizeof(ay_stess_uvp))))
			    {
			      return AY_EOMEM;
			    }
			  newuvp->type = 1;
			  newuvp->dir = tcsdirs[k];
			  newuvp->u = ipoint[0];
			  newuvp->v = ipoint[1];
			  *olduvp = newuvp;
			  olduvp = &(newuvp->next);
			  /*		    }*/ /* if */
			} /* if */
		    } /* if */
		} /* for */
	    } /* for */

	  v += vd;
	} /* for */

      /* merge vectors */
      if(trimuvp)
	{
	  ay_status = AY_OK;
	  ay_status = ay_stess_MergeUVectors(uvps[i], trimuvp);
	  if(ay_status)
	    {
	      ay_error(interp, AY_ERROR, fname,
		       "Intersecting or misoriented Trimcurves!\n");

	      if(trimuvp)
		{
		  uvpptr = trimuvp;
		  while(uvpptr)
		    {
		      uvpptr2 = uvpptr->next;
		      free(uvpptr);
		      uvpptr = uvpptr2;
		    }
		} /* if */
	      return AY_ERROR;
	    } /* if */
	} /* if */

      u += ud;
    } /* for */


  /* remove unwanted uvps */
  first_loop = 1;
  for(i = 0; i < Cn; i++)
    {
      olduvp = &(uvps[i]);
      uvpptr = uvps[i];

      while(uvpptr)
	{
	  /* act only on trimloop-points */
	  if(uvpptr->type == 1)
	    {
	      if(first_loop)
		{
		  if(uvpptr->dir == 0)
		    { /* cw */
		      /* remember that we probably have to delete whole
			 lines */
		      first_loop_cw = AY_TRUE;
		      /* since this loop is cw, we came from outside */
		      out = 1;
		    }
		}
	      first_loop = AY_FALSE;

	      if(out)
		{
		  /*
		   * we enter the patch again, and remove all
		   * uvps that we skipped since we left the patch
		   */
		  /* free unwanted vertices */
		  if(uvpptr != (*olduvp))
		    {
		      uvpptr2 = (*olduvp)->next;
		      (*olduvp)->next = uvpptr;
		      while(uvpptr2 != uvpptr)
			{
			  uvpptr3 = uvpptr2->next;
			  free(uvpptr2);
			  uvpptr2 = uvpptr3;
			} /* while */
		    } /* if */

		  /* we are now "in" */
		  out = 0;
		}
	      else
		{
		  /*
		   * we are about to leave the patch and remember
		   * where we are; we will delete later from
		   * uvpptr->next on, all intermediate uvps
		   */
		  olduvp = &(uvpptr->next);
		  /* we are now out */
		  out = 1;
		}
	    } /* if */
	  uvpptr = uvpptr->next;
	} /* while */
    } /* for */

  /* remove unwanted lines (all lines that contain no trimloop point) */
  if(first_loop_cw)
    {
      for(i=0; i < Cn; i++)
	{
	  trimloop_point = AY_FALSE;
	  done = AY_FALSE;
	  uvpptr = uvps[i];

	  while(uvpptr && (!done))
	    {
	      if(uvpptr->type == 1)
		{
		  trimloop_point = AY_TRUE;
		  done = AY_TRUE;
		} /* if */
	      uvpptr = uvpptr->next;
	    } /* while */

	  if(!trimloop_point)
	    {
	      uvpptr = uvps[i];
	      while(uvpptr)
		{
		  uvpptr2 = uvpptr->next;
		  free(uvpptr);
		  uvpptr = uvpptr2;
		} /* while */
	      uvps[i] = NULL;
	    } /* if */
	} /* for */
    } /* if */

  /* finally, calculate surfacepoints */
  for(i = 0; i < Cn; i++)
    {
      uvpptr = uvps[i];

      while(uvpptr)
	{
	  ay_nb_SurfacePoint4D(p->width-1, p->height-1,
			    p->uorder-1, p->vorder-1, p->uknotv, p->vknotv,
			    p->controlv, uvpptr->u, uvpptr->v, uvpptr->C);

	  uvpptr = uvpptr->next;
	} /* while */
    } /* for */

  *flcw = first_loop_cw;
  *result = uvps;

 return ay_status;
} /* ay_stess_TessTrimmedNPU */


/* ay_stess_TessTrimmedNPV:
 *  tesselate NURBS patch <o> into lines in parametric direction v
 */
int
ay_stess_TessTrimmedNPV(Tcl_Interp *interp, ay_object *o, int num,
			int numtrims,
			double **tcs, int *tcslens, int *tcsdirs,
			int flcw, ay_stess_uvp ***result)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *p = NULL;
 ay_stess_uvp **uvps = NULL, *uvpptr, *newuvp, **olduvp, *trimuvp;
 ay_stess_uvp *uvpptr2, *uvpptr3;
 double *tt, ipoint[2] = {0};
 double p3[2], p4[2], *U, *V, u, v;
 double umin, umax, vmin, vmax, ud, vd;
 int i, j, k, l, ind;
 int out = 0, first_loop = AY_TRUE, first_loop_cw = AY_FALSE;
 int Cm, Cn, trimloop_point, done;
 char fname[] = "TessTrimmedNPV";

  p = (ay_nurbpatch_object *)o->object;
  first_loop_cw = flcw;


  /* calc desired uv coords for patch tesselation */
  U = p->uknotv;
  j = 0;
  while(U[j] == U[j+1])
    {
      j++;
    }
  umin = U[j];

  j = p->width-1+p->uorder-1;
  while(U[j] == U[j-1])
    {
      j--;
    }
  umax = U[j];

  Cn = (p->width-1)*num;
  ud = (umax-umin)/((Cn)-1);
  u = umin;

  V = p->vknotv;
  j = 0;
  while(V[j] == V[j+1])
    {
      j++;
    }
  vmin = V[j];

  j = p->height-1+p->vorder-1;
  while(V[j] == V[j-1])
    {
      j--;
    }
  vmax = V[j];

  Cm = (p->height-1)*num;
  vd = (umax-umin)/((Cm)-1);

  if(!(uvps = calloc(Cm, sizeof(ay_stess_uvp *))))
    {
      return AY_EOMEM;
    }

  /* fill desired uv-coords */
  v = vmin;
  for(i = 0; i < Cm; i++)
    {
      u = umin;
      olduvp = &(uvps[i]);
      for(j = 0; j < Cn; j++)
	{
	  if(!(newuvp = calloc(1, sizeof(ay_stess_uvp))))
	    {
	      return AY_EOMEM;
	    }
	  /* type == 0 */
	  newuvp->u = u;
	  newuvp->v = v;
	  *olduvp = newuvp;
	  olduvp = &(newuvp->next);
	  u += ud;
	}
      v += vd;
    } /* for */

  v = vmin;
  /* match desired uv coords of patch tesselation with trimloops */
  for(i = 0; i < Cm; i++)
    {
      u = umin;
      olduvp = &trimuvp;
      trimuvp = NULL;
      p3[1] = v;
      p4[1] = v;
      /* calc all intersections of all trimloops with current u */
      for(j = 0; j < (Cn-1); j++)
	{
	  p3[0] = u;
	  p4[0] = u+ud;
	  for(k = 0; k < numtrims; k++)
	    {
	      tt = tcs[k];

	      for(l = 0; l < (tcslens[k]-1); l++)
		{
		  /* pre-select trimloop section */
		  ind = l*2;
		  if(((tt[ind+1] <= v) && (tt[ind+2+1] >= v)) ||
		     ((tt[ind+1] >= v) && (tt[ind+2+1] <= v)))
		    {
		      ipoint[0] = 0.0;
		      ipoint[1] = 0.0;

		      if((ay_stess_IntersectLines2D(&(tt[ind]), &(tt[ind+2]),
						    p3, p4, ipoint)))
			{ /* u-line intersects with trimcurve */

			  /* test commented out, because it will make
			     problems with uvp-types */

			  /* check whether ipoint is farther away enough
			     from current v, v+vd */
			  /* if((fabs(ipoint[1] - v) > AY_STESSEPSILON) &&
			     (fabs(ipoint[1] - (v+vd)) > AY_STESSEPSILON))
			     {*/ /* yes it is */
			  /* add new point */
			  if(!(newuvp = calloc(1, sizeof(ay_stess_uvp))))
			    {
			      return AY_EOMEM;
			    }
			  newuvp->type = 1;
			  newuvp->dir = tcsdirs[k];
			  newuvp->u = ipoint[0];
			  newuvp->v = ipoint[1];
			  *olduvp = newuvp;
			  olduvp = &(newuvp->next);
			  /*		    }*/ /* if */
			} /* if */
		    } /* if */
		} /* for */
	    } /* for */

	  u += ud;
	} /* for */

      /* merge vectors */
      if(trimuvp)
	{
	  ay_status = AY_OK;
	  ay_status = ay_stess_MergeVVectors(uvps[i], trimuvp);
	  if(ay_status)
	    {
	      ay_error(interp, AY_ERROR, fname,
		       "Intersecting or misoriented Trimcurves!\n");

	      if(trimuvp)
		{
		  uvpptr = trimuvp;
		  while(uvpptr)
		    {
		      uvpptr2 = uvpptr->next;
		      free(uvpptr);
		      uvpptr = uvpptr2;
		    }
		}
	      return AY_ERROR;
	    }
	}

      v += vd;
    } /* for */


  /* remove unwanted uvps */
  first_loop = AY_TRUE;
  for(i = 0; i < Cm; i++)
   {
    olduvp = &(uvps[i]);
    uvpptr = uvps[i];

    while(uvpptr)
      {
	/* act only on trimloop-points */
	if(uvpptr->type == 1)
	  {
	    if(first_loop)
	      {
		if(uvpptr->dir == 0)
		  { /* cw */
		    /* since this loop is cw, we came from outside */
		    out = 1;
		  }
	      }
	    first_loop = AY_FALSE;

	    if(out)
	      {
		/*
		 * we enter the patch again, and remove all
		 * uvps that we skipped since we left the patch
		 */
		/* free unwanted vertices */
		if(uvpptr != (*olduvp))
		  {
		    uvpptr2 = (*olduvp)->next;
		    (*olduvp)->next = uvpptr;
		    while(uvpptr2 != uvpptr)
		      {
			uvpptr3 = uvpptr2->next;
			free(uvpptr2);
			uvpptr2 = uvpptr3;
		      } /* while */
		  }

		/* we are now "in" */
		out = 0;
	      }
	    else
	      {
		/*
		 * we are about to leave the patch and remember
		 * where we are; we will delete later from
		 * uvpptr->next on, all intermediate uvps
		 */
		olduvp = &(uvpptr->next);
		/* we are now out */
		out = 1;
	      }
	  } /* if */
	uvpptr = uvpptr->next;
      } /* while */
   } /* for */

  /* remove unwanted lines (all lines that contain no trimloop point) */
  if(first_loop_cw)
    {
      for(i = 0; i < Cn; i++)
	{
	  trimloop_point = AY_FALSE;
	  done = AY_FALSE;
	  uvpptr = uvps[i];

	  while(uvpptr && (!done))
	    {
	      if(uvpptr->type == 1)
		{
		  trimloop_point = AY_TRUE;
		  done = AY_TRUE;
		} /* if */
	      uvpptr = uvpptr->next;
	    } /* while */

	  if(!trimloop_point)
	    {
	      uvpptr = uvps[i];
	      while(uvpptr)
		{
		  uvpptr2 = uvpptr->next;
		  free(uvpptr);
		  uvpptr = uvpptr2;
		} /* while */
	      uvps[i] = NULL;
	    } /* if */
	} /* for */
    } /* if */

  /* finally, calculate surfacepoints */
  for(i = 0; i < Cm; i++)
    {
      uvpptr = uvps[i];

      while(uvpptr)
	{

	  ay_nb_SurfacePoint4D(p->width-1, p->height-1,
			       p->uorder-1, p->vorder-1, p->uknotv, p->vknotv,
			       p->controlv, uvpptr->u, uvpptr->v, uvpptr->C);

	  uvpptr = uvpptr->next;
	} /* while */
    } /* for */

  *result = uvps;

 return ay_status;
} /* ay_stess_TessTrimmedNPV */


/* ay_stess_DrawTrimmedSurface:
 *
 */
int
ay_stess_DrawTrimmedSurface(Tcl_Interp *interp, ay_object *o, int num)
{
 int ay_status = AY_OK;
 double **tcs = NULL;
 double p4[4] = {0};
 int numtrims = 0, *tcslens = NULL, *tcsdirs = NULL;
 int i, j, Cm, Cn, out = 0, first_loop_cw = 0;
 ay_stess_uvp **ups = NULL, **vps = NULL, *uvpptr, *uvpptr2;
 ay_nurbpatch_object *p = NULL;

  p = (ay_nurbpatch_object *)o->object;
  Cn = (p->width-1)*num;
  Cm = (p->height-1)*num;

  ay_status = ay_stess_TessTrimCurves(o, num, &numtrims,  &tcs,
				      &tcslens,  &tcsdirs);

  if(ay_status)
    goto cleanup;

  ay_status = ay_stess_TessTrimmedNPU(interp, o, num, numtrims, tcs,
				       tcslens, tcsdirs, &first_loop_cw, &ups);

  if(first_loop_cw != 1)
    out = 0;
  else
    out = 1;
  /* draw iso-u lines */
  for(i = 0; i < Cn; i++)
    {
      uvpptr = ups[i];

      if(!out)
	glBegin(GL_LINE_STRIP);

      while(uvpptr)
	{

	  if(uvpptr->type == 1)
	    {
	      if(out)
		{
		  glBegin(GL_LINE_STRIP);
		  glVertex3dv((GLdouble*)(uvpptr->C));
		  out = 0;
		}
	      else
		{
		  glVertex3dv((GLdouble*)(uvpptr->C));
		  glEnd();
		  out = 1;
		} /* if */
	    }
	  else
	    {
	      glVertex3dv((GLdouble*)(uvpptr->C));
	    } /* if */

	  uvpptr = uvpptr->next;
	} /* while */

      if(!out)
	glEnd();

    } /* for */

  ay_status = ay_stess_TessTrimmedNPV(interp, o, num, numtrims, tcs,
				      tcslens, tcsdirs, first_loop_cw, &vps);

  if(first_loop_cw != 1)
    out = 0;
  else
    out = 1;

  /* draw iso-v lines */
  for(i = 0; i < Cm; i++)
    {
      uvpptr = vps[i];

      if(!out)
	glBegin(GL_LINE_STRIP);

      while(uvpptr)
	{

	  if(uvpptr->type == 1)
	    {
	      if(out)
		{
		  glBegin(GL_LINE_STRIP);
		  glVertex3dv((GLdouble*)(uvpptr->C));
		  out = 0;
		}
	      else
		{
		  glVertex3dv((GLdouble*)(uvpptr->C));
		  glEnd();
		  out = 1;
		} /* if */
	    }
	  else
	    {
	      glVertex3dv((GLdouble*)(uvpptr->C));
	    } /* if */

	  uvpptr = uvpptr->next;
	}  /* while */

      if(!out)
	glEnd();

    } /* for */


  /* draw trimcurves (outlines) */
  for(i = 0; i < numtrims; i++)
    {
      glBegin(GL_LINE_STRIP);
       for(j = 0; j < tcslens[i]; j++)
	 {
	   ay_nb_SurfacePoint4D(p->width-1, p->height-1,
	       	       p->uorder-1, p->vorder-1, p->uknotv, p->vknotv,
	       	       p->controlv, tcs[i][j*2], tcs[i][j*2+1], p4);

	   glVertex3dv((GLdouble*)(p4));
	 } /* for */
      glEnd();
    } /* for */

  /* clean up the mess */
cleanup:

  if(tcs)
    {
      for(i = 0; i < numtrims; i++)
	{
	  free(tcs[i]);
	}
      free(tcs);
    } /* if */

  if(tcslens)
    free(tcslens);

  if(tcsdirs)
    free(tcsdirs);

  if(ups)
    {
      for(i = 0; i < Cn; i++)
	{
	  uvpptr = ups[i];
	  while(uvpptr)
	    {
	      uvpptr2 = uvpptr->next;
	      free(uvpptr);
	      uvpptr = uvpptr2;
	    }
	  ups[i] = NULL;
	} /* for */
      free(ups);
    } /* if */

  if(vps)
    {
      for(i = 0; i < Cm; i++)
	{
	  uvpptr = vps[i];
	  while(uvpptr)
	    {
	      uvpptr2 = uvpptr->next;
	      free(uvpptr);
	      uvpptr = uvpptr2;
	    }
	  vps[i] = NULL;
	} /* for */
      free(vps);
    } /* if */

 return AY_OK;
} /* ay_stess_DrawTrimmedSurface */

/* remove local preprocessor definitions */
#undef AY_STESSEPSILON
