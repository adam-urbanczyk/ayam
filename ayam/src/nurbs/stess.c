/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2007 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* stess.c simple NURB tesselators */

/* local preprocessor definitions: */
#define AY_STESSEPSILON 0.000001
/*#define AY_STESSDBG 1*/

/* prototypes of functions local to this module: */
void ay_stess_FindMultiplePoints(int n, int p, double *U, double *P,
				 int dim, int is_rat, int stride,
				 int *m, double **V);

int ay_stess_IntersectLines2D(double *p1, double *p2, double *p3, double *p4,
			      double *ip);

void ay_stess_TessTrimCurve(ay_object *o, double **tts, int *tls, int *tds,
			    int *i, double qf);

void ay_stess_TessLinearTrimCurve(ay_object *o, double **tts, int *tls,
				  int *tds, int *i);

int ay_stess_MergeUVectors(ay_stess_uvp *a, ay_stess_uvp *b);

int ay_stess_MergeVVectors(ay_stess_uvp *a, ay_stess_uvp *b);

void ay_stess_SortIntersections(ay_stess_uvp *list, int u);

int ay_stess_TessTrimmedNPU(ay_object *o, int qf,
			    int numtrims,
			    double **tcs, int *tcslens, int *tcsdirs,
			    double *resud, double *resvd,
			    int *reslen, ay_stess_uvp ***result);

int ay_stess_TessTrimmedNPV(ay_object *o, int qf,
			    int numtrims,
			    double **tcs, int *tcslens, int *tcsdirs,
			    int *reslen, ay_stess_uvp ***result);


/* functions: */

/* ay_stess_destroy:
 *  properly destroy an stess object
 */
void
ay_stess_destroy(ay_nurbpatch_object *np)
{
 ay_voidfp *arr = NULL;
 ay_deletecb *cb = NULL;
 ay_stess_uvp *p = NULL;
 ay_stess *stess = NULL;
 int i;


  if(!np)
    return;

  if(!np->stess)
    return;

  stess = np->stess;

  if(stess->tessv)
    free(stess->tessv);

  if(stess->ups)
    {
      for(i = 0; i < stess->upslen; i++)
	{
	  while(stess->ups[i])
	    {
	      p = stess->ups[i]->next;
	      free(stess->ups[i]);
	      stess->ups[i] = p;
	    }
	}
      free(stess->ups);
    }

  if(stess->vps)
    {
      for(i = 0; i < stess->vpslen; i++)
	{
	  while(stess->vps[i])
	    {
	      p = stess->vps[i]->next;
	      free(stess->vps[i]);
	      stess->vps[i] = p;
	    }
	}
      free(stess->vps);
    }

  if(stess->tcslens)
    free(stess->tcslens);

  if(stess->tcspnts)
    free(stess->tcspnts);

  if(stess->pomesh)
    {
      arr = ay_deletecbt.arr;
      cb = (ay_deletecb *)(arr[AY_IDPOMESH]);
      if(cb)
	(void)cb(stess->pomesh);
    }

  /* now free the stess object */
  free(stess);
  np->stess = NULL;

 return;
} /* ay_stess_destroy */


/* ay_stess_GetQF:
 *  calculate stess quality factor (QF) from GLU sampling tolerance (GST)
 *
 */
int
ay_stess_GetQF(double gst)
{
 static double oldgst = 0.0;
 static int oldqf;
 int qf = 1;
 double base = 50.0;

  if(gst == 0.0)
    {
      oldgst = 0.0;
      return 0;
    }

  if(gst == oldgst)
    {
      return oldqf;
    }

  while(gst < base)
    {
      base /= 2.0;
      qf *= 2;
    }

  oldqf = qf;
  oldgst = gst;

 return qf;
} /* ay_stess_GetQF */


/* ay_stess_FindMultiplePoints:
 *  find all consecutive multiple (equal) control points in the
 *  NURBS curve <n>, <p>, <U>, <P> of dimensionality <dim> (2/3),
 *  rational coordinates <is_rat> (0/1), and coordinate size <stride> (4);
 *  returns corresponding parametric values in <V[<m>]>
 */
void
ay_stess_FindMultiplePoints(int n, int p, double *U, double *P,
			    int dim, int is_rat, int stride,
			    int *m, double **V)
{
 int i, j, eq;
 double *p1, *p2, *t;

  if(!U || !P || !m || !V)
    return;

  *m = 0;
  *V = NULL;

  p1 = P;

  for(i = 0; i < n-1; i++)
    {
      eq = AY_TRUE;

      p2 = p1 + stride;
      for(j = 0; j < p-1; j++)
	{
	  if((fabs(p1[0] - p2[0]) > AY_EPSILON) ||
	     ((dim > 1) && (fabs(p1[1] - p2[1]) > AY_EPSILON)) ||
	     ((dim > 2) && (fabs(p1[2] - p2[2]) > AY_EPSILON)))
	    {
	      eq = AY_FALSE;
	      break;
	    }
	  if(eq && is_rat)
	    {
	      if(fabs(p1[3]-p2[3]) > AY_EPSILON)
		{
		  eq = AY_FALSE;
		  break;
		}
	    } /* if */
	  p2 += stride;
	} /* for */

      if(eq)
	{
	  if(!(t = realloc(*V, ((*m)+1)*sizeof(double))))
	    {
	      if(*V)
		free(*V);
	      *m = 0; *V = NULL;
	      return;
	    }
	  *V = t;
	  (*V)[(*m)] = U[i + p + 1];
	  (*m)++;
	  p1 += ((p-1)*stride);
	  i += (p-1);
	}
      else
	{
	  p1 += stride;
	} /* if */
    } /* for */

 return;
} /* ay_stess_FindMultiplePoints */


/* ay_stess_CurvePoints2D:
 *   calculate all points of a 2D curve (TrimCurve)
 */
int
ay_stess_CurvePoints2D(int n, int p, double *U, double *Pw, int stride,
		       int is_rat, int qf,
		       int *Clen, double **C)
{
 int span, j, k, l, m, mc = 0, vi, incu, mc1 = 0;
 double *N = NULL, Cw[3], *Ct = NULL, u, ud, u1, *V = NULL;

  if(!U || !Pw || !Clen || !C)
    return AY_ENULL;

  if(!(N = calloc(p+1, sizeof(double))))
    return AY_EOMEM;

  ay_stess_FindMultiplePoints(n, p, U, Pw, 2, is_rat, stride, &mc, &V);

  *Clen = ((4 + n) * qf);

  if(*C)
    {
      free(*C);
      *C = NULL;
    }

  if(!(Ct = calloc((*Clen + mc) * 2, sizeof(double))))
    {
      free(N);
      if(V)
	free(V);
      return AY_EOMEM;
    }
  m = 0;
  ud = (U[n]-U[p])/((*Clen)-1);
  u = U[p];
  vi = 0;
  if(is_rat)
    {
      /* curve is rational */
      for(l = 0; l < (*Clen) + mc; l++)
	{
	  u1 = u;

	  if(u1 > U[n])
	    u1 = U[n];

	  incu = AY_TRUE;
	  /* are there unprocessed multiple points? */
	  if((mc > 0) && (vi < mc))
	    { /* yes */
	      /* is V[vi] between u-ud and u? (by calculating u we would
		 step over the multiple point V[vi]) */
	      if((u-ud < V[vi]) && (V[vi] < u))
		{
		  /* is V[vi] sufficiently different from u? */
		  if(fabs(u - V[vi]) > AY_EPSILON)
		    { /* yes */
		      /* calculate multiple point before u and remember
			 to not increase u by ud in this iteration */
		      u1 = V[vi];
		      vi++;
		      mc1++;
		      incu = AY_FALSE;
		    }
		  else
		    { /* no */
		      /* simply skip this multiple point, it would not
			 add value to the tesselation anyway */
		      vi++;
		    } /* if */
		} /* if */
	    } /* if */
	  span = ay_nb_FindSpan(n-1, p, u1, U);

	  ay_nb_BasisFuns(span, u1, p, U, N);

	  memset(Cw, 0, 3*sizeof(double));

	  for(j = 0; j <= p; j++)
	    {
	      k = (span-p+j)*stride;
	      Cw[0] = Cw[0] + N[j]*(Pw[k]*Pw[k+2]);
	      Cw[1] = Cw[1] + N[j]*(Pw[k+1]*Pw[k+2]);
	      Cw[2] = Cw[2] + N[j]*Pw[k+2];
	    }

	  Ct[m]   = Cw[0]/Cw[2];
	  Ct[m+1] = Cw[1]/Cw[2];

#ifdef AY_STESSDBG
	  if(Ct[m] != Ct[m] || Ct[m+1] != Ct[m+1])
	    {
	      printf("NAN, at u %lg!\n",u);
	    }
#endif

	  m += 2;

	  if(incu)
	    u += ud;
	} /* for */
    }
  else
    {
      /* curve is not rational */
      for(l = 0; l < (*Clen) + mc; l++)
	{
	  u1 = u;

	  if(u1 > U[n])
	    u1 = U[n];

	  incu = AY_TRUE;
	  /* are there unprocessed multiple points? */
	  if((mc > 0) && (vi < mc))
	    { /* yes */
	      /* is V[vi] between u-ud and u (by calculating u we would
		 step over the multiple point V[vi]) */
	      if((u-ud < V[vi]) && (V[vi] < u))
		{ /* yes */
		  /* is V[vi] sufficiently different from u? */
		  if(fabs(u - V[vi]) > AY_EPSILON)
		    { /* yes */
		      /* calculate multiple point before u and remember
			 to not increase u by ud in this iteration */
		      u1 = V[vi];
		      vi++;
		      mc1++;
		      incu = AY_FALSE;
		    }
		  else
		    { /* no */
		      /* simply skip this multiple point, it would not
			 add value to the tesselation anyway */
		      vi++;
		    } /* if */
		} /* if */
	    } /* if */

	  span = ay_nb_FindSpan(n-1, p, u1, U);

	  ay_nb_BasisFuns(span, u1, p, U, N);

	  for(j = 0; j <= p; j++)
	    {
	      k = (span-p+j)*stride;
	      Ct[m]   = Ct[m]   + N[j]*Pw[k];
	      Ct[m+1] = Ct[m+1] + N[j]*Pw[k+1];
	    }

#ifdef AY_STESSDBG
	  if(Ct[m] != Ct[m] || Ct[m+1] != Ct[m+1])
	    {
	      printf("NAN, at u %lg!\n",u);
	    }
#endif

	  m += 2;

	  if(incu)
	    u += ud;
	} /* for */

    } /* if */

  *C = Ct;
  *Clen += mc1;

  free(N);
  if(V)
    free(V);

 return AY_OK;
} /* ay_stess_CurvePoints2D */


/* ay_stess_CurvePoints3D:
 *   calculate all points of a 3D curve
 */
int
ay_stess_CurvePoints3D(int n, int p, double *U, double *Pw, int is_rat, int qf,
		       int *Clen, double **C)
{
 int span, j, k, l, m, mc = 0, vi, incu, mc1 = 0;
 double *N = NULL, Cw[4], *Ct = NULL, u, ud, u1, *V;

  if(!U || !Pw || !Clen || !C)
    return AY_ENULL;

  if(!(N = calloc(p+1, sizeof(double))))
    return AY_EOMEM;

  ay_stess_FindMultiplePoints(n, p, U, Pw, 3, is_rat, 4, &mc, &V);

  *Clen = ((4 + n) * qf);

  if(!(Ct = calloc((*Clen + mc) * 3, sizeof(double))))
    {
      free(N);
      if(V)
	free(V);
      return AY_EOMEM;
    }

  m = 0;
  ud = (U[n]-U[p])/((*Clen)-1);
  u = U[p];
  vi = 0;
  if(is_rat)
    {
      /* curve is rational */
      for(l = 0; l < (*Clen) + mc; l++)
	{
	  u1 = u;
	  incu = AY_TRUE;
	  /* are there unprocessed multiple points? */
	  if((mc > 0) && (vi < mc))
	    { /* yes */
	      /* is V[vi] between u-ud and u? (by calculating u we would
		 step over the multiple point V[vi]) */
	      if((u-ud < V[vi]) && (V[vi] < u))
		{
		  /* is V[vi] sufficiently different from u? */
		  if(fabs(u - V[vi]) > AY_EPSILON)
		    { /* yes */
		      /* calculate multiple point before u and remember
			 to not increase u by ud in this iteration */
		      u1 = V[vi];
		      vi++;
		      mc1++;
		      incu = AY_FALSE;
		    }
		  else
		    { /* no */
		      /* simply skip this multiple point, it would not
			 add value to the tesselation anyway */
		      vi++;
		    } /* if */
		} /* if */
	    } /* if */

	  span = ay_nb_FindSpan(n-1, p, u1, U);
	  ay_nb_BasisFuns(span, u1, p, U, N);

	  memset(Cw, 0, 4*sizeof(double));

	  for(j = 0; j <= p; j++)
	    {
	      k = (span-p+j)*4;
	      Cw[0] = Cw[0] + N[j]*Pw[k]*Pw[k+3];
	      Cw[1] = Cw[1] + N[j]*Pw[k+1]*Pw[k+3];
	      Cw[2] = Cw[2] + N[j]*Pw[k+2]*Pw[k+3];
	      Cw[3] = Cw[3] + N[j]*Pw[k+3];
	    }

	  Ct[m]   = Cw[0]/Cw[3];
	  Ct[m+1] = Cw[1]/Cw[3];
	  Ct[m+2] = Cw[2]/Cw[3];

	  m += 3;
	  if(incu)
	    u += ud;
	} /* for */
    }
  else
    {
      /* curve is not rational */
      for(l = 0; l < (*Clen) + mc; l++)
	{
	  u1 = u;
	  incu = AY_TRUE;
	  /* are there unprocessed multiple points? */
	  if((mc > 0) && (vi < mc))
	    { /* yes */
	      /* is V[vi] between u-ud and u (by calculating u we would
		 step over the multiple point V[vi]) */
	      if((u-ud < V[vi]) && (V[vi] < u))
		{ /* yes */
		  /* is V[vi] sufficiently different from u? */
		  if(fabs(u - V[vi]) > AY_EPSILON)
		    { /* yes */
		      /* calculate multiple point before u and remember
			 to not increase u by ud in this iteration */
		      u1 = V[vi];
		      vi++;
		      mc1++;
		      incu = AY_FALSE;
		    }
		  else
		    { /* no */
		      /* simply skip this multiple point, it would not
			 add value to the tesselation anyway */
		      vi++;
		    } /* if */
		} /* if */
	    } /* if */

	  span = ay_nb_FindSpan(n-1, p, u1, U);

	  ay_nb_BasisFuns(span, u1, p, U, N);

	  for(j = 0; j <= p; j++)
	    {
	      k = (span-p+j)*4;
	      Ct[m]   = Ct[m]   + N[j]*Pw[k];
	      Ct[m+1] = Ct[m+1] + N[j]*Pw[k+1];
	      Ct[m+2] = Ct[m+2] + N[j]*Pw[k+2];
	    }

	  m += 3;
	  if(incu)
	    u += ud;
	} /* for */
    } /* if */

  *C = Ct;
  *Clen += mc1;

  free(N);
  if(V)
    free(V);

 return AY_OK;
} /* ay_stess_CurvePoints3D */


/* ay_stess_SurfacePoints3D:
 *   calculate all points of an untrimmed NURBS surface
 */
int
ay_stess_SurfacePoints3D(int n, int m, int p, int q, double *U, double *V,
			 double *P, int qf, int *Cn, int *Cm, double **C)
{
 int spanu = 0, spanv = 0, indu = 0, indv = 0, l = 0, k = 0, i = 0, j = 0;
 int a, b;
 double u, v, ud, vd, *Nu = NULL, *Nv = NULL;
 double temp[3] = {0}, *Ct = NULL, fder[12] = {0}, *fd1, *fd2;
 int *spanus = NULL, *spanvs = NULL;

  if(!(Nu = calloc(p+1+q+1, sizeof(double))))
    return AY_EOMEM;
  Nv = Nu + (p+1);

  *Cn = (4 + n) * qf;
  ud = (U[n] - U[p]) / ((*Cn) - 1);

  *Cm = (4 + m) * qf;
  vd = (V[m] - V[q]) / ((*Cm) - 1);

  if(!(Ct = calloc((*Cn)*(*Cm)*6, sizeof(double))))
    { free(Nu); return AY_EOMEM; }

  if(!(spanus = calloc((*Cn)+(*Cm), sizeof(int))))
    { free(Nu); free(Ct); return AY_EOMEM; }
  spanvs = spanus + (*Cn);

  /* employ linear variants of FindSpan() as they are much faster
     than a binary search; especially, since we calculate
     spans for all parameters in order */

  u = U[p];
  spanu = p;
  for(a = 0; a < (*Cn)-1; a++)
    {
      /*
      if(u < U[p+1])
	{
	  spanus[a] = p;
	}
      else
	{
	  while(u >= U[spanu])
	    {
	      spanu++;
	    }
	  spanus[a] = spanu-1;
	}
      */

      if(u > U[p+1])
	{
	  while(u > U[spanu+1])
	    {
	      spanu++;
	    }
	}
      spanus[a] = spanu;

      u += ud;
    }
  spanus[a] = spanus[a-1];

  v = V[q];
  spanv = q;
  for(a = 0; a < (*Cm)-1; a++)
    {
      /*
      if(v < V[q+1])
	{
	  spanvs[a] = q;
	}
      else
	{
	  while(v > V[spanv])
	    {
	      spanv++;
	    }
	  spanvs[a] = spanv-1;
	}
      */

      if(v > V[q+1])
	{
	  while(v > V[spanv+1])
	    {
	      spanv++;
	    }
	}
      spanvs[a] = spanv;
      v += vd;
    }
  spanvs[a] = spanvs[a-1];

  u = U[p];
  for(a = 0; a < (*Cn); a++)
    {
      spanu = spanus[a];
      ay_nb_BasisFuns(spanu, u, p, U, Nu);
      indu = spanu - p;
      v = V[q];
      for(b = 0; b < (*Cm); b++)
	{
	  spanv = spanvs[b];
	  ay_nb_BasisFuns(spanv, v, q, V, Nv);

	  /*j = (a*(*Cn)+b)*3;*/
	  for(l = 0; l <= q; l++)
	    {
	      memset(temp, 0, 3*sizeof(double));
	      indv = spanv - q + l;

	      for(k = 0; k <= p; k++)
		{
		  /* was: temp = temp + Nu[k]*P[indu+k][indv]; */
		  i = (((indu+k)*m)+indv)*4;

		  temp[0] += Nu[k]*P[i];
		  temp[1] += Nu[k]*P[i+1];
		  temp[2] += Nu[k]*P[i+2];
		} /* for */
	      /* was: Cw = Cw + Nv[l]*temp */
	      Ct[j]   += Nv[l]*temp[0];
	      Ct[j+1] += Nv[l]*temp[1];
	      Ct[j+2] += Nv[l]*temp[2];
	    } /* for */

	  /* calculate normal */
	  ay_nb_CompFirstDerSurf3D(n-1, m-1, p, q, U, V, P, u, v, fder);
	  fd1 = &(fder[3]);
	  fd2 = &(fder[6]);
	  AY_V3CROSS(temp, fd2, fd1);
	  memcpy(&(Ct[j+3]), temp, 3*sizeof(double));

	  j += 6;
	  v += vd;
	} /* for */

      u += ud;
    } /* for */

  *C = Ct;

  free(Nu);
  free(spanus);

 return AY_OK;
} /* ay_stess_SurfacePoints3D */

#if 0
/* ay_stess_SurfacePoints3DF:
 *   calculate all points of an untrimmed NURBS surface
 */
int
ay_stess_SurfacePoints3DF(int n, int m, int p, int q, double *U, double *V,
			 double *P, int qf, int *Cn, int *Cm, double **C)
{
 int spanu = 0, spanv = 0, indu = 0, indv = 0, l = 0, k = 0, i = 0, j = 0;
 int a, b;
 double u, v, ud, vd, *Nu = NULL, *Nv = NULL;
 double *Ct = NULL, fder[12] = {0}, *fd1, *fd2;
 int *spanus = NULL, *spanvs = NULL;

  if(!(Nu = calloc(((p+1)*(p+1)+(q+1)*(q+1)+(q+1)*3), sizeof(double))))
    return AY_EOMEM;
  Nv = Nu + (p+1)*(p+1);
  temp = Nv + (q+1)*(q+1);

  *Cn = (4 + n) * qf;
  ud = (U[n] - U[p]) / ((*Cn) - 1);

  *Cm = (4 + m) * qf;
  vd = (V[m] - V[q]) / ((*Cm) - 1);

  if(!(Ct = calloc((*Cn)*(*Cm)*6, sizeof(double))))
    { free(Nu); return AY_EOMEM; }

  if(!(spanus = calloc((*Cn)+(*Cm), sizeof(int))))
    { free(Nu); free(Ct); return AY_EOMEM; }
  spanvs = spanus + (*Cn);

  /* employ linear variants of FindSpan() as they are much faster
     than a binary search; especially, since we calculate
     spans for all parameters in order */

  u = U[p];
  spanu = p;
  for(a = 0; a < (*Cn)-1; a++)
    {
      /*
      if(u < U[p+1])
	{
	  spanus[a] = p;
	}
      else
	{
	  while(u >= U[spanu])
	    {
	      spanu++;
	    }
	  spanus[a] = spanu-1;
	}
      */

      if(u > U[p+1])
	{
	  while(u > U[spanu+1])
	    {
	      spanu++;
	    }
	}
      spanus[a] = spanu;

      u += ud;
    }
  spanus[a] = spanus[a-1];

  v = V[q];
  spanv = q;
  for(a = 0; a < (*Cm)-1; a++)
    {
      /*
      if(v < V[q+1])
	{
	  spanvs[a] = q;
	}
      else
	{
	  while(v > V[spanv])
	    {
	      spanv++;
	    }
	  spanvs[a] = spanv-1;
	}
      */

      if(v > V[q+1])
	{
	  while(v > V[spanv+1])
	    {
	      spanv++;
	    }
	}
      spanvs[a] = spanv;
      v += vd;
    }
  spanvs[a] = spanvs[a-1];

  u = U[p];
  for(a = 0; a < (*Cn); a++)
    {
      spanu = spanus[a];
      ay_nb_DersBasisFuns(spanu, u, p, 1, U, Nu);
      indu = spanu - p;
      v = V[q];
      for(b = 0; b < (*Cm); b++)
	{
	  spanv = spanvs[b];
	  ay_nb_DersBasisFuns(spanv, v, q, 1, V, Nv);

	  /*j = (a*(*Cn)+b)*3;*/
	  for(l = 0; l <= q; l++)
	    {
	      memset(temp, 0, 3*sizeof(double));
	      indv = spanv - q + l;

	      for(k = 0; k <= p; k++)
		{
		  /* was: temp = temp + Nu[k]*P[indu+k][indv]; */
		  i = (((indu+k)*m)+indv)*4;

		  temp[0] += Nu[k]*P[i];
		  temp[1] += Nu[k]*P[i+1];
		  temp[2] += Nu[k]*P[i+2];
		} /* for */
	      /* was: Cw = Cw + Nv[l]*temp */
	      Ct[j]   += Nv[l]*temp[0];
	      Ct[j+1] += Nv[l]*temp[1];
	      Ct[j+2] += Nv[l]*temp[2];
	    } /* for */

	  /* calculate normal */
	  ay_nb_CompFirstDerSurf3D(n-1, m-1, p, q, U, V, P, u, v, fder);

	  for(k = 0; k <= 1; k++)
	    {
	      for(s = 0; s <= q; s++)
		{
		  temp[s*3]   = 0.0;
		  temp[s*3+1] = 0.0;
		  temp[s*3+2] = 0.0;

		  for(r = 0; r <= p; r++)
		    {
		      /* was: temp[s] = temp[s] +
			 Nu[k][r]*P[uspan-p+r][vspan-q+s]; */
		      i = (((uspan-p+r)*(m+1))+(vspan-q+s))*4;
		      temp[s*3]   += Nu[(k*(p+1))+r]*P[i];
		      temp[s*3+1] += Nu[(k*(p+1))+r]*P[i+1];
		      temp[s*3+2] += Nu[(k*(p+1))+r]*P[i+2];
		    }
		}

	      for(l = 0; l <= 1; l++)
		{
		  /* was: C[k][l] = 0; */
		  C[(k*2+l)*3]   = 0.0;
		  C[(k*2+l)*3+1] = 0.0;
		  C[(k*2+l)*3+2] = 0.0;

		  for(s = 0; s <= q; s++)
		    {
		      /* was C[k][l] = C[k][l] + Nv[l][s] * temp[s]; */
		      i = (k*2+l)*3;
		      C[i]   += Nv[(l*(q+1))+s] * temp[s*3];
		      C[i+1] += Nv[(l*(q+1))+s] * temp[s*3+1];
		      C[i+2] += Nv[(l*(q+1))+s] * temp[s*3+2];
		    } /* for */
		} /* for */
	    } /* for */


	  fd1 = &(fder[3]);
	  fd2 = &(fder[6]);
	  AY_V3CROSS(temp, fd2, fd1);
	  memcpy(&(Ct[j+3]), temp, 3*sizeof(double));

	  j += 6;
	  v += vd;
	} /* for */

      u += ud;
    } /* for */

  *C = Ct;

  free(Nu);
  free(spanus);

 return AY_OK;
} /* ay_stess_SurfacePoints3DF */
#endif

/* ay_stess_SurfacePoints4D:
 *   calculate all points of an untrimmed NURBS surface with weights
 */
int
ay_stess_SurfacePoints4D(int n, int m, int p, int q, double *U, double *V,
			 double *Pw, int qf, int *Cn, int *Cm, double **C)
{
 int ay_status = AY_OK;
 int spanu = 0, spanv = 0, indu = 0, indv = 0, l = 0, k = 0, i = 0, j = 0;
 int a, b, ti;
 double u, v, ud, vd, *Nu = NULL, *Nv = NULL;
 double Cw[4] = {0}, *Ct = NULL, *temp = NULL, fder[12] = {0}, *fd1, *fd2;
 int *spanus = NULL, *spanvs = NULL;

  if(!(Nu = calloc(p+1+q+1+((q+1)*4), sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  Nv = Nu + (p+1);
  temp = Nv + (q+1);

  *Cn = (4 + n) * qf;
  ud = (U[n] - U[p]) / ((*Cn) - 1);

  *Cm = (4 + m) * qf;
  vd = (V[m] - V[q]) / ((*Cm) - 1);

  if(!(spanus = calloc((*Cn)+(*Cm), sizeof(int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  spanvs = spanus + (*Cn);

  if(!(Ct = calloc((*Cn)*(*Cm)*6, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* employ linear variants of FindSpan() as they are much faster
     than a binary search; especially, since we calculate
     spans for all parameters in order */

  u = U[p];
  spanu = p;
  for(a = 0; a < (*Cn)-1; a++)
    {
      /*
      if(u < U[p+1])
	{
	  spanus[a] = p;
	}
      else
	{
	  while(u > U[spanu])
	    {
	      spanu++;
	    }
	  spanus[a] = spanu-1;
	}
      */

      if(u > U[p+1])
	{
	  while(u > U[spanu+1])
	    {
	      spanu++;
	    }
	}
      spanus[a] = spanu;
      u += ud;
    }
  spanus[a] = spanus[a-1];


  v = V[q];
  spanv = q;
  for(a = 0; a < (*Cm)-1; a++)
    {
      /*
      if(v < V[q+1])
	{
	  spanvs[a] = q;
	}
      else
	{
	  while(v > V[spanv])
	    {
	      spanv++;
	    }
	  spanvs[a] = spanv-1;
	}
      */

      if(v > V[q+1])
	{
	  while(v > V[spanv+1])
	    {
	      spanv++;
	    }
	}
      spanvs[a] = spanv;
      v += vd;
    }
  spanvs[a] = spanvs[a-1];

  u = U[p];
  for(a = 0; a < (*Cn); a++)
    {
      spanu = spanus[a];
      ay_nb_BasisFuns(spanu, u, p, U, Nu);

      indu = spanu - p;

      v = V[q];

      for(b = 0; b < (*Cm); b++)
	{
	  spanv = spanvs[b];
	  ay_nb_BasisFuns(spanv, v, q, V, Nv);

	  ti = 0;
	  for(l = 0; l <= q; l++)
	    {
	      memset(&(temp[l*4]), 0, 4*sizeof(double));
	      indv = spanv - q + l;
	      for(k = 0; k <= p; k++)
		{
		  /* was: temp = temp + Nu[k]*Pw[indu+k][indv]; */
		  i = (((indu+k)*m)+indv)*4;

		  temp[ti+0] += Nu[k]*Pw[i]*Pw[i+3];
		  temp[ti+1] += Nu[k]*Pw[i+1]*Pw[i+3];
		  temp[ti+2] += Nu[k]*Pw[i+2]*Pw[i+3];
		  temp[ti+3] += Nu[k]*Pw[i+3];
		} /* for */
	      ti += 4;
	    } /* for */

	  memset(Cw, 0, 4*sizeof(double));
	  ti = 0;
	  for(l = 0; l <= q; l++)
	    {
	      /* was: Cw = Cw + Nv[l]*temp */
	      Cw[0] += Nv[l]*temp[ti+0];
	      Cw[1] += Nv[l]*temp[ti+1];
	      Cw[2] += Nv[l]*temp[ti+2];
	      Cw[3] += Nv[l]*temp[ti+3];
	      ti += 4;
	    }

	  /*j = (a*(*Cn)+b)*3;*/
	  Ct[j]   = Cw[0]/Cw[3];
	  Ct[j+1] = Cw[1]/Cw[3];
	  Ct[j+2] = Cw[2]/Cw[3];

	  /* calculate normal */
	  ay_nb_CompFirstDerSurf4D(n-1, m-1, p, q, U, V, Pw, u, v, fder);
	  fd1 = &(fder[3]);
	  fd2 = &(fder[6]);
	  AY_V3CROSS(temp, fd2, fd1);
	  memcpy(&(Ct[j+3]), temp, 3*sizeof(double));

	  j += 6;
	  v += vd;
	} /* for */

      u += ud;
    } /* for */

  /* return result */
  *C = Ct;

cleanup:

  if(Nu)
    free(Nu);

  if(spanus)
    free(spanus);

 return ay_status;
} /* ay_stess_SurfacePoints4D */


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

  if(((fabs(p1[0]-p3[0])<=2*AY_EPSILON)&&(fabs(p1[1]-p3[1])<=2*AY_EPSILON)) ||
     ((fabs(p1[0]-p4[0])<=2*AY_EPSILON)&&(fabs(p1[1]-p4[1])<=2*AY_EPSILON)))
    {
      ip[0] = p1[0];
      ip[1] = p1[1];
      return 1;
    }

  if(((fabs(p2[0]-p3[0])<=2*AY_EPSILON)&&(fabs(p2[1]-p3[1])<=2*AY_EPSILON)) ||
     ((fabs(p2[0]-p4[0])<=2*AY_EPSILON)&&(fabs(p2[1]-p4[1])<=2*AY_EPSILON)))
    {
      ip[0] = p2[0];
      ip[1] = p2[1];
      return 1;
    }

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


/* ay_stess_TessTrimCurve:
 * Helper for ay_stess_TessTrimCurves() below.
 * Tesselate a single trim curve/loop.
 * qf < 0 => ReTess-mode, re-tesselate linear loops for drawing
 */
void
ay_stess_TessTrimCurve(ay_object *o, double **tts, int *tls, int *tds,
		       int *i, double qf)
{
 ay_nurbcurve_object *nc = NULL;
 double angle, m[16], *dtmp, p1[4], p2[4];
 int apply_trafo = AY_FALSE, stride, j;

  nc = (ay_nurbcurve_object*)o->refine;

  if(nc->order == 2 && qf > 0)
    {
      ay_stess_TessLinearTrimCurve(o, tts, tls, tds, i);
      return;
    }

  if(qf < 0)
    qf = fabs(qf);

  if(nc->is_rat)
    stride = 3;
  else
    stride = 2;

  if(AY_ISTRAFO(o))
    {
      apply_trafo = AY_TRUE;
      ay_trafo_creatematrix(o, m);
    }

  if(apply_trafo)
    {
      if(!(dtmp = malloc(nc->length*stride*sizeof(double))))
	{ return; }

      /* apply transformations */
      for(j = 0; j < nc->length; j++)
	{
	  memcpy(p1, &(nc->controlv[j*4]), 4*sizeof(double));
	  AY_APTRAN3(p2, p1, m)
	  memcpy(&(dtmp[j*stride]), p2, 2*sizeof(double));
	  if(nc->is_rat)
	    dtmp[j*stride+2] = nc->controlv[j*4+3];
	}

      ay_stess_CurvePoints2D(nc->length, nc->order-1, nc->knotv, dtmp,
			     stride, nc->is_rat, qf, &(tls[*i]), &(tts[*i]));
      free(dtmp);
      dtmp = NULL;
    }
  else
    {
      ay_stess_CurvePoints2D(nc->length, nc->order-1,
			     nc->knotv, nc->controlv, 4,
			     nc->is_rat, qf, &(tls[*i]), &(tts[*i]));
    }

  /* get orientation of trimloop */
  if(tds)
    {
      angle = 0.0;
      ay_nct_getorientation(nc, 4, 1, 0, &angle);
      if(angle >= 0.0)
	{
	  tds[*i] = 1;
	}
    }

  /* advance trim loop counter */
  *i = *i+1;

 return;
} /* ay_stess_TessTrimCurve */


/* ay_stess_TessLinearTrimCurve:
 * Helper for ay_stess_TessTrimCurves() below.
 * Tesselate a single linear (i.e. order == 2) trim curve/loop.
 */
void
ay_stess_TessLinearTrimCurve(ay_object *o, double **tts, int *tls, int *tds,
			     int *i)
{
 ay_nurbcurve_object *nc = NULL;
 double angle, m[16], *dtmp, p1[4], p2[4];
 int apply_trafo = AY_FALSE, j;

  nc = (ay_nurbcurve_object*)o->refine;

  if(AY_ISTRAFO(o))
    {
      apply_trafo = AY_TRUE;
      ay_trafo_creatematrix(o, m);
    }

  if(!(dtmp = malloc(nc->length*2*sizeof(double))))
    { return; }

  if(apply_trafo)
    {
      /* apply transformations */
      for(j = 0; j < nc->length; j++)
	{
	  memcpy(p1, &(nc->controlv[j*4]), 4*sizeof(double));
	  AY_APTRAN3(p2, p1, m)
	  memcpy(&(dtmp[j*2]), p2, 2*sizeof(double));
	  if(nc->is_rat)
	    {
	      dtmp[j*2]   /= nc->controlv[j*4+3];
	      dtmp[j*2+1] /= nc->controlv[j*4+3];
	    }
	}
    }
  else
    {
      for(j = 0; j < nc->length; j++)
	{
	  memcpy(&(dtmp[j*2]), &(nc->controlv[j*4]), 2*sizeof(double));
	  if(nc->is_rat)
	    {
	      dtmp[j*2]   /= nc->controlv[j*4+3];
	      dtmp[j*2+1] /= nc->controlv[j*4+3];
	    }
	}
    } /* if */

  tls[*i] = nc->length;
  tts[*i] = dtmp;

  /* get orientation of trimloop */
  if(tds)
    {
      angle = 0.0;
      ay_nct_getorientation(nc, 4, 1, 0, &angle);
      if(angle >= 0.0)
	{
	  tds[*i] = 1;
	}
    }

  /* advance trim loop counter */
  *i = *i+1;

 return;
} /* ay_stess_TessLinearTrimCurve */


/* ay_stess_TessTrimCurves:
 *  tesselate all trim curves of object <o>
 */
int
ay_stess_TessTrimCurves(ay_object *o, int qf, int *nt, double ***tt,
			int **tl, int **td)
{
 int ay_status = AY_OK;
 double **tts = NULL;
 int i, numtrims = 0, *tls = NULL, *tds = NULL;
 ay_object *trim = NULL, *loop = NULL, *p, *nc = NULL, *cnc = NULL;

  /* count trimloops */
  trim = o->down;
  while(trim->next)
    {
      switch(trim->type)
	{
	case AY_IDNCURVE:
	  numtrims++;
	  break;
	case AY_IDLEVEL:
	  loop = trim->down;
	  while(loop->next)
	    {
	      if(loop->type == AY_IDNCURVE)
		{
		  numtrims++;
		  break;
		}
	      else
		{
		  if(ay_provide_object(loop, AY_IDNCURVE, NULL) == AY_OK)
		    {
		      numtrims++;
		      break;
		    }
		}
	      loop = loop->next;
	    } /* while */
	  break;
	default:
	  p = NULL;
	  ay_status = ay_provide_object(trim, AY_IDNCURVE, &p);
	  nc = p;
	  while(nc)
	    {
	      numtrims++;
	      nc = nc->next;
	    }
	  (void)ay_object_deletemulti(p, AY_FALSE);
	  break;
	} /* switch */
      trim = trim->next;
    } /* while */

  if(numtrims == 0)
    return AY_ERROR;

  /* tesselate trimloops and get their orientation */
  if(!(tts = calloc(numtrims, sizeof(double *))))
    return AY_EOMEM;
  if(!(tls = calloc(numtrims, sizeof(int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(td)
    if(!(tds = calloc(numtrims, sizeof(int))))
      { ay_status = AY_EOMEM; goto cleanup; }

  i = 0;
  trim = o->down;
  while(trim->next)
    {
      switch(trim->type)
	{
	case AY_IDNCURVE:
	  ay_stess_TessTrimCurve(trim, tts, tls, tds, &i, qf);
	  break;
	case AY_IDLEVEL:
	  loop = trim->down;
	  if(loop->next)
	    {
	      cnc = NULL;
	      ay_nct_concatobjs(loop, &cnc);
	      if(cnc)
		{
		  ay_stess_TessTrimCurve(cnc, tts, tls, tds, &i, qf);
		  ay_object_delete(cnc);
		}
	    }
	  break;
	default:
	  /* trim is curve providing object */
	  p = NULL;
	  ay_status = ay_provide_object(trim, AY_IDNCURVE, &p);
	  nc = p;
	  while(nc)
	    {
	      ay_stess_TessTrimCurve(nc, tts, tls, tds, &i, qf);
	      nc = nc->next;
	    }
	  (void)ay_object_deletemulti(p, AY_FALSE);
	  break;
	} /* switch */

      trim = trim->next;
    } /* while */

  /* return results */
  *nt = numtrims;
  *tt = tts;
  *tl = tls;
  if(td)
    *td = tds;

  /* prevent cleanup code from doing something harmful */
  tts = NULL;
  tls = NULL;
  tds = NULL;

cleanup:
  if(tts)
    free(tts);
  if(tls)
    free(tls);
  if(tds)
    free(tds);

 return ay_status;
} /* ay_stess_TessTrimCurves */


/* ay_stess_ReTessTrimCurves:
 *  re tesselate all linear trims of object <o>
 */
int
ay_stess_ReTessTrimCurves(ay_object *o, int qf, int numtrims, double **tt,
			  int *tl, double **tp)
{
 int ay_status = AY_OK;
 double *tps = NULL, p4[4];
 int i, j, a;
 ay_object *trim = NULL, *loop = NULL, *p, *nc = NULL, *cnc = NULL;
 ay_nurbcurve_object *c = NULL;
 ay_nurbpatch_object *np = NULL;
 int npnts = 0;

  np = (ay_nurbpatch_object*)o->refine;

  /* XXXX todo: adapt qf to NPatch dimensions to accomodate for the
     worst case: a single linear trim segment running along the complete
     patch surface; even in this case, we should provide an adequate
     amount of tesselated points and avoid leaving the surface too
     much/often */
  qf *= 2;

  i = 0;
  trim = o->down;
  while(trim->next)
    {
      c = NULL;
      switch(trim->type)
	{
	case AY_IDNCURVE:
	  c = (ay_nurbcurve_object*)trim->refine;
	  if(c->order == 2)
	    ay_stess_TessTrimCurve(trim, tt, tl, NULL, &i, -qf);
	  break;
	case AY_IDLEVEL:
	  loop = trim->down;
	  if(loop->next)
	    {
	      cnc = NULL;
	      ay_nct_concatobjs(loop, &cnc);
	      if(cnc)
		{
		  c = (ay_nurbcurve_object*)cnc->refine;
		  if(c->order == 2)
		    ay_stess_TessTrimCurve(cnc, tt, tl, NULL, &i, -qf);
		  ay_object_delete(cnc);
		}
	    }
	  break;
	default:
	  /* trim is curve providing object */
	  p = NULL;
	  ay_status = ay_provide_object(trim, AY_IDNCURVE, &p);
	  nc = p;
	  while(nc)
	    {
	      c = (ay_nurbcurve_object*)nc->refine;
	      if(c->order == 2)
		ay_stess_TessTrimCurve(nc, tt, tl, NULL, &i, -qf);
	      nc = nc->next;
	    }
	  (void)ay_object_deletemulti(p, AY_FALSE);
	  break;
	} /* switch */

      trim = trim->next;
    } /* while */

  if(tp)
    {
      /* calculate 3D surface points */
      for(i = 0; i < numtrims; i++)
	{
	  npnts += tl[i];
	}

      if(!(tps = calloc(npnts*3, sizeof(double))))
	{ return AY_EOMEM; }

      a = 0;
      for(i = 0; i < numtrims; i++)
	{
	  for(j = 0; j < tl[i]; j++)
	    {
	      if(np->is_rat)
		{
		  ay_nb_SurfacePoint4D(np->width-1, np->height-1, np->uorder-1,
				       np->vorder-1, np->uknotv, np->vknotv,
				       np->controlv, tt[i][j*2], tt[i][j*2+1],
				       p4);
		  memcpy(&(tps[a]), p4, 3*sizeof(double));
		}
	      else
		{
		  ay_nb_SurfacePoint3D(np->width-1, np->height-1, np->uorder-1,
				       np->vorder-1, np->uknotv, np->vknotv,
				       np->controlv, tt[i][j*2], tt[i][j*2+1],
				       &(tps[a]));
		}
	      a += 3;
	    } /* for */
	} /* for */

      /* return result */
      *tp = tps;
    } /* if tp */

 return ay_status;
} /* ay_stess_ReTessTrimCurves */


/* ay_stess_MergeUVectors:
 *   merge two vectors (lines) of points on a surface, removing
 *   double points in the progress
 */
int
ay_stess_MergeUVectors(ay_stess_uvp *a, ay_stess_uvp *b)
{
 ay_stess_uvp *p1, *p2, *p3;
 int done = AY_FALSE, inserted = 0;

  p1 = a;
  while(!done)
    {
      inserted = 0;
      p2 = b;
      while(!inserted && b)
	{
	  if(p1->next)
	    {
	      if(p2)
		{
		  if(fabs(p2->v - p1->v) < AY_EPSILON)
		    {
		      /* We, accidentally, have here a trimloop
		       * point that is identical to another point;
		       * if the other point is a uv-point we transmogrify
		       * the uv-point to a trimloop point and delete the
		       * original trimloop point!
		       */
#ifdef AY_STESSDBG
		      printf("Transmogrify u-line point at %lg %lg.\n",
			     p1->u, p1->v);
#endif /* AY_STESSDBG */
		      if((p1->type > 0) && (p1->dir != p2->dir))
			{
			  /* intersecting trimcurves */
			  while(b)
			    {
			      p2 = b->next;
			      free(b);
			      b = p2;
			    }
			  return AY_ERROR; /* XXXX early exit! */
			}

		      p1->type = 2;
		      p1->dir = p2->dir;
		      p3 = p2->next;
		      free(p2);
		      p2 = p3;
		      b = p3;
		      inserted = 1;
		    }
		}

	      if(!inserted && p2)
		{
		  if((p2->v > p1->v) && (p2->v < p1->next->v))
		    {
		      p3 = p1->next;
		      p1->next = p2;
		      b = p2->next;
		      p2->next = p3;
		      p1 = p2;
		      inserted = 1;
		    }
		  else
		    {
		      p1 = p1->next;
		    } /* if */
		} /* if */
	    }
	  else
	    {
	      /* falling off the border of the patch
		 => discard remaining trim points */
	      if(p1 && b)
		{
		  p1->type = 3;
		}

	      while(b)
		{
		  p2 = b->next;
		  free(b);
		  b = p2;
		}

	    } /* if */
	} /* while */

      if(!b)
	{
	  done = AY_TRUE;
	}
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
 int done = AY_FALSE, inserted = 0;

  p1 = a;
  while(!done)
    {
      inserted = 0;
      p2 = b;
      while(!inserted && b)
	{
	  if(p1->next)
	    {
	      if(p2)
		{
		  if(fabs(p2->u - p1->u) < AY_EPSILON)
		    {
		      /* We, accidentally, have here a trimloop
		       * point that is identical to another point;
		       * if the other point is a uv-point we transmogrify
		       * the uv-point to a trimloop point and delete the
		       * original trimloop point!
		       */
#ifdef AY_STESSDBG
		      printf("Transmogrify v-line point at %lg %lg.\n",
			     p1->u, p1->v);
#endif /* AY_STESSDBG */
		      if((p1->type > 0) && (p1->dir != p2->dir))
			{
			  /* intersecting trimcurves */
			  while(b)
			    {
			      p2 = b->next;
			      free(b);
			      b = p2;
			    }
			  return AY_ERROR; /* XXXX early exit! */
			}

		      p1->type = 2;
		      p1->dir = p2->dir;
		      p3 = p2->next;
		      free(p2);
		      p2 = p3;
		      b = p3;
		      inserted = 1;
		    } /* if */
		} /* if */

	      if(!inserted && p2)
		{
		  if((p2->u > p1->u) && (p2->u < p1->next->u))
		    {
		      p3 = p1->next;
		      p1->next = p2;
		      b = p2->next;
		      p2->next = p3;
		      p1 = p2;
		      inserted = 1;
		    }
		  else
		    {
		      p1 = p1->next;
		    } /* if */
		} /* if */
	    }
	  else
	    {
	      /* falling off the border of the patch
		 => discard remaining trim points */
	      if(p1 && b)
		{
		  p1->type = 3;
		}
	      while(b)
		{
		  p2 = b->next;
		  free(b);
		  b = p2;
		}

	    } /* if */
	} /* while */

      if(!b)
	{
	  done = AY_TRUE;
	}
    } /* while */

 return AY_OK;
} /* ay_stess_MergeVVectors */


/* ay_stess_SortIntersections:
 *  sort a list of intersections for faster merging
 */
void
ay_stess_SortIntersections(ay_stess_uvp *list, int u)
{
 int done = AY_FALSE;
 ay_stess_uvp *p1, *p2;
 double t;

  if(!list || !list->next)
    return;

  while(!done)
    {
      done = AY_TRUE;

      p1 = list;
      p2 = p1->next;
      while(p1 && p2)
	{
	  if(u)
	    {
	      if(p1->u > p2->u)
		{
		  t = p1->u;
		  p1->u = p2->u;
		  p2->u = t;
		  done = AY_FALSE;
		}
	    }
	  else
	    {
	      if(p1->v > p2->v)
		{
		  t = p1->v;
		  p1->v = p2->v;
		  p2->v = t;
		  done = AY_FALSE;
		}
	    }

	  p1 = p1->next;
	  p2 = p2->next;
	} /* while p1 && p2 */
    } /* while !done */

 return;
} /* ay_stess_SortIntersections */


/* ay_stess_TessTrimmedNPU:
 *  tesselate NURBS patch <o> into lines in parametric direction u
 */
int
ay_stess_TessTrimmedNPU(ay_object *o, int qf,
			int numtrims,
			double **tcs, int *tcslens, int *tcsdirs,
			double *resud, double *resvd,
			int *reslen, ay_stess_uvp ***result)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *p = NULL;
 ay_stess_uvp **uvps = NULL, *uvpptr, *newuvp, **nextuvp, *trimuvp;
 ay_stess_uvp *olduvp, *uvpptr2, *uvpptr3;
 double *tt, ipoint[2] = {0};
 double p3[2], p4[2], *U, *V, u, v;
 double *fd1, *fd2, temp[3] = {0}, ders[12] = {0};
 double umin, umax, vmin, vmax, ud, vd;
 int i, j, k, l, ind;
 int out = 0, first_loop = AY_TRUE, first_loop_cw = AY_FALSE;
 int Cm, Cn;
 char fname[] = "TessTrimmedNPU";

  p = (ay_nurbpatch_object *)o->refine;

  /* calc desired uv coords for patch tesselation */
  Cn = (p->width + 4) * qf;
  *reslen = Cn;
  Cm = (p->height + 4) * qf;
  if(!(uvps = calloc(Cn, sizeof(ay_stess_uvp *))))
    {
      return AY_EOMEM;
    }

  U = p->uknotv;
  umin = U[p->uorder-1];
  umax = U[p->width];
  ud = (umax-umin)/((Cn)-1);
  *resud = ud;

  V = p->vknotv;
  vmin = V[p->vorder-1];
  vmax = V[p->height];
  vd = (vmax-vmin)/((Cm)-1);
  *resvd = vd;

  /* fill desired uv-coords */
  /* XXXX Todo: check surface for double control points (important
     surface features where we should place a tesselated point/line)
   */
  u = umin;
  for(i = 0; i < Cn; i++)
    {
      v = vmin;
      nextuvp = &(uvps[i]);
      for(j = 0; j < Cm; j++)
	{
	  if(!(newuvp = calloc(1, sizeof(ay_stess_uvp))))
	    {
	      /* XXXX memory leak! */
	      return AY_EOMEM;
	    }
	  /* type == 0 */
	  newuvp->u = u;
	  newuvp->v = v;
	  *nextuvp = newuvp;
	  nextuvp = &(newuvp->next);
	  v += vd;
	} /* for */
      u += ud;
    } /* for */

  first_loop_cw = !tcsdirs[0];

  /* match desired uv coords of patch tesselation with trimloops */
  u = umin;
  p3[1] = vmin - AY_EPSILON;
  p4[1] = vmax + AY_EPSILON;

  for(i = 0; i < Cn; i++)
    {
      nextuvp = &trimuvp;
      olduvp = NULL;
      trimuvp = NULL;

      /* calc all intersections of all trimloops with current u */

      if(i == Cn-1)
	u = umax;
      p3[0] = u;
      p4[0] = u;

      for(k = 0; k < numtrims; k++)
	{
	  tt = tcs[k];

	  for(l = 0; l < (tcslens[k]-1); l++)
	    {
	      ind = l*2;

	      /* is section crossing or touching u? */
	      if(((tt[ind] <= (u + AY_EPSILON))
		  && (tt[ind+2] >= (u - AY_EPSILON))) ||
		 ((tt[ind] >= (u - AY_EPSILON)) &&
		  (tt[ind+2] <= (u + AY_EPSILON))))
		{
		  /* weed out all sections that run (more or less)
		     exactly along the current u-line, nothing good
		     comes of them */
		  if((fabs(tt[ind] - u) < AY_EPSILON) &&
		     (fabs(tt[ind+2] - u) < AY_EPSILON))
		    {
#ifdef AY_STESSDBG
		      printf("Discarding parallel section.\n");
#endif
		      continue;
		    }

		  ipoint[0] = 0.0;
		  ipoint[1] = 0.0;

		  if((ay_stess_IntersectLines2D(&(tt[ind]),
						&(tt[ind+2]),
						p3, p4, ipoint)))
		    {
		      /* u-line intersects with trimcurve */
		      /* => add new point (but avoid consecutive
			 equal points; those appear if a loop touches
			 start or end of the current u-line) */
		      if(!olduvp ||
			 fabs(olduvp->v - ipoint[1]) > AY_EPSILON)
			{
			  if(!(newuvp = calloc(1, sizeof(ay_stess_uvp))))
			    {
			      /* XXXX memory leak! */
			      return AY_EOMEM;
			    }
			  newuvp->type = 1;
			  newuvp->dir = tcsdirs[k];
			  newuvp->u = ipoint[0];
			  newuvp->v = ipoint[1];
			  olduvp = newuvp;
			  *nextuvp = newuvp;
			  nextuvp = &(newuvp->next);
			}
		    } /* if have intersection */
		} /* if is not parallel */
	    } /* for */
	} /* for */

#ifdef AY_STESSDBG
      if(trimuvp && !trimuvp->next)
	{
	  printf("Discarding u-line with just one trim point (%lg, %lg).\n",
		 trimuvp->u,trimuvp->v);
	}
#endif /* AY_STESSDBG */

      if(trimuvp && trimuvp->next)
	{
	  /* we had trimloop points => merge vectors */
	  ay_stess_SortIntersections(trimuvp, AY_FALSE);
	  ay_status = ay_stess_MergeUVectors(uvps[i], trimuvp);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname,
		       "Intersecting or misoriented trimcurves!");
	      return AY_ERROR;
	    } /* if */
	}
      else
	{
	  /* we had no trimloop points */
	  if(first_loop_cw)
	    {
	      /* remove unwanted lines (all lines that contain
		 no trimloop points) */
	      uvpptr = uvps[i];
	      while(uvpptr)
		{
		  uvpptr2 = uvpptr->next;
		  free(uvpptr);
		  uvpptr = uvpptr2;
		} /* while */
	      uvps[i] = NULL;
	    } /* if */
	} /* if */

      u += ud;
    } /* for */

  /* remove unwanted uvps */
  for(i = 0; i < Cn; i++)
    {
      first_loop = AY_TRUE;
      out = !first_loop_cw;
      nextuvp = &(uvps[i]);
      uvpptr = uvps[i];

      while(uvpptr)
	{
	  /* act only on trimloop-points */
	  if(uvpptr->type)
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
		  if(uvpptr != (*nextuvp))
		    {
		      uvpptr2 = *nextuvp;
		      *nextuvp = uvpptr;
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
		  nextuvp = &(uvpptr->next);
		  /* we are now "out" */
		  out = 1;
		}
	    } /* if is trimloop point */
	  uvpptr = uvpptr->next;
	} /* while */

      if(*nextuvp != uvps[i] && out)
	{
	  /* free left over outer uvps */
	  while(*nextuvp)
	    {
	      uvpptr2 = *nextuvp;
	      *nextuvp = (*nextuvp)->next;
	      free(uvpptr2);
	    }
	}
    } /* for */

  /* finally, calculate surfacepoints */
  for(i = 0; i < Cn; i++)
    {
      uvpptr = uvps[i];

      while(uvpptr)
	{
	  ay_nb_CompFirstDerSurf4D(p->width-1, p->height-1,
			    p->uorder-1, p->vorder-1, p->uknotv, p->vknotv,
			    p->controlv, uvpptr->u, uvpptr->v, ders);
	  memcpy(uvpptr->C, ders, 3*sizeof(double));
	  fd1 = &(ders[3]);
	  fd2 = &(ders[6]);
	  AY_V3CROSS(temp, fd2, fd1);
	  memcpy(&(uvpptr->C[3]), temp, 3*sizeof(double));
	  if(uvpptr->next)
	    {
	      uvpptr->next->prev = uvpptr;
	    }
	  uvpptr = uvpptr->next;
	} /* while */
    } /* for */

  *result = uvps;

 return ay_status;
} /* ay_stess_TessTrimmedNPU */


/* ay_stess_TessTrimmedNPV:
 *  tesselate NURBS patch <o> into lines in parametric direction v
 */
int
ay_stess_TessTrimmedNPV(ay_object *o, int qf,
			int numtrims,
			double **tcs, int *tcslens, int *tcsdirs,
			int *reslen, ay_stess_uvp ***result)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *p = NULL;
 ay_stess_uvp **uvps = NULL, *uvpptr, *newuvp, **nextuvp, *trimuvp;
 ay_stess_uvp *olduvp, *uvpptr2, *uvpptr3;
 double *tt, ipoint[2] = {0};
 double p3[2], p4[2], *U, *V, u, v;
 double *fd1, *fd2, temp[3] = {0}, ders[12] = {0};
 double umin, umax, vmin, vmax, ud, vd;
 int i, j, k, l, ind;
 int out = 0, first_loop = AY_TRUE, first_loop_cw = AY_FALSE;
 int Cm, Cn;
 char fname[] = "TessTrimmedNPV";

  p = (ay_nurbpatch_object *)o->refine;

  /* calc desired uv coords for patch tesselation */
  Cn = (p->width + 4) * qf;
  Cm = (p->height + 4) * qf;
  *reslen = Cm;
  if(!(uvps = calloc(Cm, sizeof(ay_stess_uvp *))))
    {
      return AY_EOMEM;
    }

  U = p->uknotv;
  umin = U[p->uorder-1];
  umax = U[p->width];
  ud = (umax-umin)/((Cn)-1);

  V = p->vknotv;
  vmin = V[p->vorder-1];
  vmax = V[p->height];
  vd = (vmax-vmin)/((Cm)-1);

  /* fill desired uv-coords */
  /* XXXX Todo: check surface for double control points (important
     surface features where we should place a tesselated point/line)
   */
  v = vmin;
  for(i = 0; i < Cm; i++)
    {
      u = umin;
      nextuvp = &(uvps[i]);
      for(j = 0; j < Cn; j++)
	{
	  if(!(newuvp = calloc(1, sizeof(ay_stess_uvp))))
	    {
	      /* XXXX memory leak! */
	      return AY_EOMEM;
	    }
	  /* type == 0 */
	  newuvp->u = u;
	  newuvp->v = v;
	  *nextuvp = newuvp;
	  nextuvp = &(newuvp->next);
	  u += ud;
	}
      v += vd;
    } /* for */

  first_loop_cw = !tcsdirs[0];

  /* match desired uv coords of patch tesselation with trimloops */
  v = vmin;
  p3[0] = umin - AY_EPSILON;
  p4[0] = umax + AY_EPSILON;

  for(i = 0; i < Cm; i++)
    {
      nextuvp = &trimuvp;
      olduvp = NULL;
      trimuvp = NULL;
      /* calc all intersections of all trimloops with current v */

      if(i == Cm-1)
	v = vmax;
      p3[1] = v;
      p4[1] = v;

      for(k = 0; k < numtrims; k++)
	{
	  tt = tcs[k];

	  for(l = 0; l < (tcslens[k]-1); l++)
	    {
	      ind = l*2;

	      /* is section crossing or touching v? */
	      if(((tt[ind+1] <= (v + AY_EPSILON)) &&
		  (tt[ind+2+1] >= (v - AY_EPSILON))) ||
		 ((tt[ind+1] >= (v - AY_EPSILON)) &&
		  (tt[ind+2+1] <= (v + AY_EPSILON))))
		{
		  /* weed out all sections that run (more or less)
		     exactly along the current v-line, nothing good
		     comes of them */
		  if((fabs(tt[ind+1] - v) < AY_EPSILON) &&
		     (fabs(tt[ind+2+1] - v) < AY_EPSILON))
		    continue;

		  ipoint[0] = 0.0;
		  ipoint[1] = 0.0;

		  if((ay_stess_IntersectLines2D(&(tt[ind]),
						&(tt[ind+2]),
						p3, p4, ipoint)))
		    {
		      /* v-line intersects with trimcurve */
		      /* => add new point (but avoid consecutive
			 equal points; those appear if a loop touches
			 start or end of the current v-line) */
		      if(!olduvp ||
			 fabs(olduvp->u - ipoint[0]) > AY_EPSILON)
			{
			  if(!(newuvp = calloc(1, sizeof(ay_stess_uvp))))
			    {
			      /* XXXX memory leak! */
			      return AY_EOMEM;
			    }
			  newuvp->type = 1;
			  newuvp->dir = tcsdirs[k];
			  newuvp->u = ipoint[0];
			  newuvp->v = ipoint[1];

			  olduvp = newuvp;
			  *nextuvp = newuvp;
			  nextuvp = &(newuvp->next);
			}
		    } /* if have intersection */
		} /* if is not parallel */
	    } /* for */
	} /* for */

#ifdef AY_STESSDBG
      if(trimuvp && !trimuvp->next)
	{
	  printf("Discarding v-line with just one trim point (%lg, %lg).\n",
		 trimuvp->u,trimuvp->v);
	}
#endif /* AY_STESSDBG */

      if(trimuvp && trimuvp->next)
	{
	  /* we had trimloop points => merge vectors */
	  ay_stess_SortIntersections(trimuvp, AY_TRUE);
	  ay_status = ay_stess_MergeVVectors(uvps[i], trimuvp);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname,
		       "Intersecting or misoriented trimcurves!");
	      return AY_ERROR;
	    }
	}
      else
	{
	  /* we had no trimloop points */
	  if(first_loop_cw)
	    {
	      /* remove unwanted lines (all lines that contain
		 no trimloop points) */
	      uvpptr = uvps[i];
	      while(uvpptr)
		{
		  uvpptr2 = uvpptr->next;
		  free(uvpptr);
		  uvpptr = uvpptr2;
		} /* while */
	      uvps[i] = NULL;
	    } /* if */
	} /* if */

      v += vd;
    } /* for */

  /* remove unwanted uvps */
  for(i = 0; i < Cm; i++)
    {
      first_loop = AY_TRUE;
      out = !first_loop_cw;
      nextuvp = &(uvps[i]);
      uvpptr = uvps[i];

      while(uvpptr)
	{
	  /* act only on trimloop-points */
	  if(uvpptr->type)
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
		  if(uvpptr != (*nextuvp))
		    {
		      uvpptr2 = *nextuvp;
		      *nextuvp = uvpptr;
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
		  nextuvp = &(uvpptr->next);
		  /* we are now "out" */
		  out = 1;
		}
	    } /* if is trimloop point */
	  uvpptr = uvpptr->next;
	} /* while */

      /* free left over outer uvps */
      if(*nextuvp != uvps[i] && out)
	{
	  while(*nextuvp)
	    {
	      uvpptr2 = *nextuvp;
	      *nextuvp = (*nextuvp)->next;
	      free(uvpptr2);
	    }
	}
    } /* for */

  /* finally, calculate surfacepoints */
  for(i = 0; i < Cm; i++)
    {
      uvpptr = uvps[i];

      while(uvpptr)
	{
	  ay_nb_CompFirstDerSurf4D(p->width-1, p->height-1,
			    p->uorder-1, p->vorder-1, p->uknotv, p->vknotv,
			    p->controlv, uvpptr->u, uvpptr->v, ders);

	  memcpy(uvpptr->C, ders, 3*sizeof(double));
	  fd1 = &(ders[3]);
	  fd2 = &(ders[6]);
	  AY_V3CROSS(temp, fd2, fd1);
	  memcpy(&(uvpptr->C[3]), temp, 3*sizeof(double));
	  if(uvpptr->next)
	    {
	      uvpptr->next->prev = uvpptr;
	    }
	  uvpptr = uvpptr->next;
	} /* while */
    } /* for */

  *result = uvps;

 return ay_status;
} /* ay_stess_TessTrimmedNPV */


/* ay_stess_DrawTrimmedSurface:
 *
 */
void
ay_stess_DrawTrimmedSurface(ay_stess *stess)
{
 int i, j, a, out = 0;
 ay_stess_uvp *uvpptr;

  if(!stess)
    return;

  /* draw iso-u lines */
  for(i = 0; i < stess->upslen; i++)
    {
      uvpptr = stess->ups[i];
      if(uvpptr && uvpptr->next)
	{
	  if(stess->ft_cw)
	    {
	      out = 0;
	      glBegin(GL_LINE_STRIP);
	    }
	  else
	    {
	      out = 1;
	    }

	  while(uvpptr)
	    {
	      if(uvpptr->type > 0)
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
		    } /* if out */
		}
	      else
		{
		  glVertex3dv((GLdouble*)(uvpptr->C));
		} /* if trim point */

	      uvpptr = uvpptr->next;
	    } /* while */

	  if(!out)
	    {
	      glEnd();
	    }
	} /* if have two points */
    } /* for all u lines */

  /* draw iso-v lines */
  out = 0;
  for(i = 0; i < stess->vpslen; i++)
    {
      uvpptr = stess->vps[i];

      if(uvpptr && uvpptr->next)
	{
	  if(stess->ft_cw)
	    {
	      out = 0;
	      glBegin(GL_LINE_STRIP);
	    }
	  else
	    {
	      out = 1;
	    }

	  while(uvpptr)
	    {
	      if(uvpptr->type > 0)
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
		    } /* if out */
		}
	      else
		{
		  glVertex3dv((GLdouble*)(uvpptr->C));
		} /* if trim point */

	      uvpptr = uvpptr->next;
	    }  /* while */

	  if(!out)
	    {
	      glEnd();
	    }
	} /* if have two points */
    } /* for all v lines */

  /* draw trimcurves (outlines) */
  a = 0;
  for(i = 0; i < stess->tcslen; i++)
    {
      glBegin(GL_LINE_STRIP);
       for(j = 0; j < stess->tcslens[i]; j++)
	 {
	   glVertex3dv(&(stess->tcspnts[a]));
	   a += 3;
	 } /* for */
      glEnd();
    } /* for */

 return;
} /* ay_stess_DrawTrimmedSurface */


/* ay_stess_ShadeTrimmedSurface:
 *
 */
void
ay_stess_ShadeTrimmedSurface(ay_stess *stess)
{
 int i, instrip = AY_FALSE;
 unsigned int j;
 ay_stess_uvp *u1, *u2, *v1, *v2;
 ay_pomesh_object *po;
 double *p;

  if(!stess)
    return;

  if(stess->pomesh)
    {
      /* assume the pomesh is a triangle soup (from the tesselation) */
      po = stess->pomesh;
      glBegin(GL_TRIANGLES);
       p = stess->normal;
       glNormal3dv((GLdouble*)p);
       p = po->controlv;
       for(j = 0; j < po->npolys; j++)
	 {
	   glVertex3dv((GLdouble*)p);
	   p += 3;
	   glVertex3dv((GLdouble*)p);
	   p += 3;
	   glVertex3dv((GLdouble*)p);
	   p += 3;
	 }
      glEnd();
      return;
    }

  /* search for complete cells (all types 0) in u-direction */
  for(i = 0; i < (stess->upslen-1); i++)
    {
      u1 = stess->ups[i];
      u2 = stess->ups[i+1];

      if(stess->ft_cw)
	{
	  /* forward to next complete grid-cell */
	  while(u1)
	    {
	      if(u1->type == 0 || u1->type == 2)
		break;
	      u1 = u1->next;
	    }
	  while(u2)
	    {
	      if(u2->type == 0 || u2->type == 2)
		break;
	      u2 = u2->next;
	    }
	}
      else
	{
	  /* forward to first trim */
	  while(u1 && u1->type == 0)
	    u1 = u1->next;

	  while(u2 && u2->type == 0)
	    u2 = u2->next;
	}

      if(!u1 || !u2 || !u1->next || !u2->next)
	{
	  continue;
	}

      while(u1 && u1->type != 0)
	u1 = u1->next;

      while(u2 && u2->type != 0)
	u2 = u2->next;

      while(u1 && u1->next && u2 && u2->next)
	{
	  if(u1->type == 0 && u1->next->type == 0 &&
	     u2->type == 0 && u2->next->type == 0 &&
	     u1->v == u2->v)
	    {
	      if(!instrip)
		{
		  glBegin(GL_TRIANGLE_STRIP);
		  instrip = AY_TRUE;
		}
	      glNormal3dv((GLdouble*)&((u1->C)[3]));
	      glVertex3dv((GLdouble*)(u1->C));
	      glNormal3dv((GLdouble*)&((u2->C)[3]));
	      glVertex3dv((GLdouble*)(u2->C));
	      u1 = u1->next;
	      u2 = u2->next;
	      glNormal3dv((GLdouble*)&((u1->C)[3]));
	      glVertex3dv((GLdouble*)(u1->C));
	      glNormal3dv((GLdouble*)&((u2->C)[3]));
	      glVertex3dv((GLdouble*)(u2->C));

	      /* check next cell */
	      if(!u1->next || !u2->next ||
		 u1->next->type != 0 || u2->next->type != 0)
		{
		  if(instrip)
		    {
		      glEnd();
		      instrip = AY_FALSE;
		    }
		}
	    } /* have complete cell */

	  /* forward to next candidate cell */
	  while((u1 && u1->type != 0) ||
		(u1 && u1->next && u1->next->type != 0))
	    u1 = u1->next;

	  while((u2 && u2->type != 0) ||
		(u2 && u2->next && u2->next->type != 0))
	    u2 = u2->next;

	  if(!u1 || !u2)
	    break;

	  if(u1->v != u2->v)
	    {
	      if(u1->v < u2->v)
		{
		  while(u1 && u1->v < u2->v)
		    u1 = u1->next;
		}
	      else
		{
		  while(u2 && u1->v > u2->v)
		    u2 = u2->next;
		}
	    }
	} /* while */

      if(instrip)
	{
	  glEnd();
	  instrip = AY_FALSE;
	}

      /*
       * search for incomplete cells
       */
      u1 = stess->ups[i];
      u2 = stess->ups[i+1];

      if(stess->ft_cw)
	{
	  while(u1)
	    {
	      if(u1->type == 0 || u1->type == 2)
		break;
	      u1 = u1->next;
	    }
	  while(u2)
	    {
	      if(u2->type == 0 || u2->type == 2)
		break;
	      u2 = u2->next;
	    }
	}
      else
	{
	  /* forward to first trim */
	  while(u1 && u1->type == 0)
	    u1 = u1->next;

	  while(u2 && u2->type == 0)
	    u2 = u2->next;
	}

      if(!u1 || !u2 || !u1->next || !u2->next)
	continue;

      while(u1 && u1->type != 0)
	u1 = u1->next;

      while(u2 && u2->type != 0)
	u2 = u2->next;

      if(!u1 || !u2 || !u1->next || !u2->next)
	continue;

      while(u1 && u1->next && u2 && u2->next)
	{
	  if(u1->v != u2->v)
	    {
	      if(u1->v < u2->v)
		{
		  while(u1 && u1->v < u2->v)
		    u1 = u1->next;
		}
	      else
		{
		  while(u2 && u1->v > u2->v)
		    u2 = u2->next;
		}
	    }

	  if(!u1 || !u2 || !u1->next || !u2->next)
	    break;

	  if(u1->type == 0 && u2->type == 0 && u1->v == u2->v)
	    {
	      /* check previous cell */
	      if(u1->prev && u2->prev &&
		 (u1->prev->type || u2->prev->type))
		{
		  glBegin(GL_TRIANGLE_STRIP);
		   glNormal3dv((GLdouble*)&((u1->prev->C)[3]));
		   glVertex3dv((GLdouble*)(u1->prev->C));
		   glNormal3dv((GLdouble*)&((u2->prev->C)[3]));
		   glVertex3dv((GLdouble*)(u2->prev->C));
		   glNormal3dv((GLdouble*)&((u1->C)[3]));
		   glVertex3dv((GLdouble*)(u1->C));
		   glNormal3dv((GLdouble*)&((u2->C)[3]));
		   glVertex3dv((GLdouble*)(u2->C));
		  glEnd();
		}

	      /* check next cell */
	      if(u1->next->type || u2->next->type)
		{
		  glBegin(GL_TRIANGLE_STRIP);
		   glNormal3dv((GLdouble*)&((u1->C)[3]));
		   glVertex3dv((GLdouble*)(u1->C));
		   glNormal3dv((GLdouble*)&((u2->C)[3]));
		   glVertex3dv((GLdouble*)(u2->C));
		   glNormal3dv((GLdouble*)&((u1->next->C)[3]));
		   glVertex3dv((GLdouble*)(u1->next->C));
		   glNormal3dv((GLdouble*)&((u2->next->C)[3]));
		   glVertex3dv((GLdouble*)(u2->next->C));
		  glEnd();
		}
	    } /* if */

	  /* forward to next candidate cell */
	  do
	    u1 = u1->next;
	  while(u1 && u1->type != 0);

	  do
	    u2 = u2->next;
	  while(u2 && u2->type != 0);

	} /* while */
    } /* for */

  /****************************************************/

  /* search for complete cells (all types 0) in v-direction, but
     do not render them, just process the possibly incomplete
     cells right before/behind them */
  for(i = 0; i < (stess->vpslen-1); i++)
    {
      v1 = stess->vps[i];
      v2 = stess->vps[i+1];

      if(stess->ft_cw)
	{
	  /* forward to next complete grid-cell */
	  while(v1)
	    {
	      if(v1->type == 0 || v1->type == 2)
		break;
	      v1 = v1->next;
	    }
	  while(v2)
	    {
	      if(v2->type == 0 || v2->type == 2)
		break;
	      v2 = v2->next;
	    }
	}
      else
	{
	  /* forward to first trim */
	  while(v1 && v1->type == 0)
	    v1 = v1->next;

	  while(v2 && v2->type == 0)
	    v2 = v2->next;
	}

      if(!v1 || !v2 || !v1->next || !v2->next)
	continue;

      while(v1 && v1->type != 0)
	v1 = v1->next;

      while(v2 && v2->type != 0)
	v2 = v2->next;

      while(v1 && v1->next && v2 && v2->next)
	{
	  if(v1->type == 0 && v1->next->type == 0 &&
	     v2->type == 0 && v2->next->type == 0 &&
	     v1->u == v2->u)
	    {
	      if(!instrip)
		{
		  /* here we would start the strip of complete cells,
		     but in v direction, we only check, whether there
		     is an incomplete cell before, and shade it */
		  if(v1->prev && v2->prev &&
		     (v1->prev->type || v2->prev->type))
		    {
		      glBegin(GL_TRIANGLE_STRIP);
		       glNormal3dv((GLdouble*)&((v1->prev->C)[3]));
		       glVertex3dv((GLdouble*)(v1->prev->C));
		       glNormal3dv((GLdouble*)&((v1->C)[3]));
		       glVertex3dv((GLdouble*)(v1->C));
		       glNormal3dv((GLdouble*)&((v2->prev->C)[3]));
		       glVertex3dv((GLdouble*)(v2->prev->C));
		       glNormal3dv((GLdouble*)&((v2->C)[3]));
		       glVertex3dv((GLdouble*)(v2->C));
		      glEnd();
		    }
		  instrip = AY_TRUE;
		}

	      v1 = v1->next;
	      v2 = v2->next;

	      /* check next cell */
	      if(!v1->next || !v2->next ||
		 v1->next->type != 0 || v2->next->type != 0)
		{
		  if(instrip)
		    {

		      instrip = AY_FALSE;

		      /* here we would end the strip of complete cells,
			 but in v direction, we only check, whether there
			 is an incomplete cell following, and shade it */
		      if(v1->next && v2->next &&
			 (v1->next->type || v2->next->type))
			{
			  glBegin(GL_TRIANGLE_STRIP);
			   glNormal3dv((GLdouble*)&((v1->C)[3]));
			   glVertex3dv((GLdouble*)(v1->C));
			   glNormal3dv((GLdouble*)&((v1->next->C)[3]));
			   glVertex3dv((GLdouble*)(v1->next->C));
			   glNormal3dv((GLdouble*)&((v2->C)[3]));
			   glVertex3dv((GLdouble*)(v2->C));
			   glNormal3dv((GLdouble*)&((v2->next->C)[3]));
			   glVertex3dv((GLdouble*)(v2->next->C));
			  glEnd();
			}
		    }
		}
	    } /* have complete cell */

	  /* forward to next candidate cell */
	  while((v1 && v1->type != 0) ||
		(v1 && v1->next && v1->next->type != 0))
	    v1 = v1->next;

	  while((v2 && v2->type != 0) ||
		(v2 && v2->next && v2->next->type != 0))
	    v2 = v2->next;

	  if(!v1 || !v2)
	    break;

	  if(v1->u != v2->u)
	    {
	      if(v1->u < v2->u)
		{
		  while(v1 && v1->u < v2->u)
		    v1 = v1->next;
		}
	      else
		{
		  while(v2 && v1->u > v2->u)
		    v2 = v2->next;
		}
	    }
	} /* while */

      if(instrip)
	{
	  instrip = AY_FALSE;
	}
    } /* for */

 return;
} /* ay_stess_ShadeTrimmedSurface */


/* ay_stess_TessTrimmedNP:
 *
 */
int
ay_stess_TessTrimmedNP(ay_object *o, int qf)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *p = NULL;
 ay_stess *st = NULL;
 double **tcs = NULL; /**< tesselated trim curves [tcslen][tcslens[i]] */
 int i, *tcsdirs = NULL; /**< directions of trim curves [tcslen] */

  p = (ay_nurbpatch_object *)o->refine;

  if(p->stess)
    {
      ay_stess_destroy(p);
    }

  if(!(p->stess = calloc(1, sizeof(ay_stess))))
    return AY_ERROR;

  st = p->stess;

  ay_status = ay_stess_TessTrimCurves(o, qf,
				      &(st->tcslen), &tcs,
				      &(st->tcslens), &tcsdirs);

  if(ay_status)
    goto cleanup;

  ay_status = ay_stess_TessTrimmedNPU(o, qf, st->tcslen, tcs,
				      st->tcslens, tcsdirs,
				      &(st->ud), &(st->vd),
				      &(st->upslen), &(st->ups));

  if(ay_status)
    goto cleanup;

  ay_status = ay_stess_TessTrimmedNPV(o, qf, st->tcslen, tcs,
				      st->tcslens, tcsdirs,
				      &(st->vpslen), &(st->vps));

  if(ay_status)
    goto cleanup;

  st->ft_cw = !tcsdirs[0];

  ay_status = ay_stess_ReTessTrimCurves(o, qf,
					st->tcslen, tcs,
					st->tcslens,
					&(st->tcspnts));

  if(ay_status)
    goto cleanup;

  /* prevent cleanup code from doing something harmful */
  p = NULL;

  /* clean up the mess */
cleanup:

  if(tcs)
    {
      for(i = 0; i < st->tcslen; i++)
	{
	  if(tcs[i])
	    free(tcs[i]);
	}
      free(tcs);
    }

  if(tcsdirs)
    free(tcsdirs);

  if(p)
    {
      ay_stess_destroy(p);
    }

  if(ay_status)
    ay_status = AY_ERROR;

 return ay_status;
} /* ay_stess_TessTrimmedNP */


/* ay_stess_TessTrimmedPlanarNP:
 *
 */
int
ay_stess_TessTrimmedPlanarNP(ay_object *o, int qf)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *np = NULL;
 ay_tag *tag;
 ay_stess *st = NULL;
 double **tcs = NULL; /**< tesselated trim curves [tcslen][tcslens[i]] */
 int tcslen, *tcslens = NULL, *tcsdirs = NULL;
 unsigned int i, j, a, totalverts = 0;
 ay_pomesh_object *po = NULL, *tpo = NULL;
 double u, v, *p;

  np = (ay_nurbpatch_object *)o->refine;

  if(np->stess)
    {
      ay_stess_destroy(np);
    }

  ay_status = ay_stess_TessTrimCurves(o, qf,
				      &tcslen, &tcs,
				      &tcslens, &tcsdirs);

  if(ay_status)
    goto cleanup;

  if(tcs)
    {
      if(!(np->stess = calloc(1, sizeof(ay_stess))))
	return AY_ERROR;

      st = np->stess;

      st->tcslen = tcslen;
      st->tcslens = tcslens;

      if(!(po = calloc(1, sizeof(ay_pomesh_object))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      po->npolys = 1;
      if(!(po->nloops = calloc(1, sizeof(unsigned int))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      po->nloops[0] = tcslen;
      if(!(po->nverts = calloc(tcslen, sizeof(unsigned int))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      for(i = 0; i < (unsigned int)tcslen; i++)
	{
	  po->nverts[i] = tcslens[i];
	  totalverts += tcslens[i];
	}

      if(!(po->verts = calloc(totalverts, sizeof(unsigned int))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      for(i = 0; i < totalverts; i++)
	{
	  po->verts[i] = i;
	}

      /* fill controlv */
      if(!(po->controlv = calloc(totalverts*3, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      st->tcspnts = po->controlv;

      p = po->controlv;
      for(i = 0; i < (unsigned int)tcslen; i++)
	{
	  a = 0;
	  for(j = 0; j < (unsigned int)tcslens[i]; j++)
	    {
	      u = tcs[i][a];
	      v = tcs[i][a+1];

	      if(np->is_rat)
		{
		  ay_nb_SurfacePoint4D(np->width-1, np->height-1, np->uorder-1,
				       np->vorder-1, np->uknotv, np->vknotv,
				       np->controlv, u, v, p);
		}
	      else
		{
		  ay_nb_SurfacePoint3D(np->width-1, np->height-1, np->uorder-1,
				       np->vorder-1, np->uknotv, np->vknotv,
				       np->controlv, u, v, p);
		}

	      a += 2;
	      p += 3;
	    }
	}

      /* set normal */
      st->normal[0] = 0.0;
      st->normal[1] = 0.0;
      st->normal[2] = 1.0;
      p = NULL;
      tag = o->tags;
      while(tag)
	{
	  if(tag->type == ay_nt_tagtype)
	    {
	      memcpy(st->normal, ((ay_btval*)tag->val)->payload,
		     3*sizeof(double));
	      p = st->normal;
	      break;
	    }
	  tag = tag->next;
	}

      /* tesselate the polygon */
      ay_status = ay_tess_pomesh(po, /*optimize=*/AY_FALSE, p, &tpo);

      st->pomesh = tpo;
    }

  /* prevent cleanup code from doing something harmful */
  np = NULL;

  /* clean up the mess */
cleanup:

  if(tcs)
    {
      for(i = 0; i < (unsigned int)tcslen; i++)
	{
	  if(tcs[i])
	    free(tcs[i]);
	}
      free(tcs);
    }

  if(tcsdirs)
    free(tcsdirs);

  if(np)
    {
      ay_stess_destroy(np);
    }

  /* free temporary polymesh */
  if(po)
    {
      if(po->verts)
	free(po->verts);
      if(po->nloops)
	free(po->nloops);
      if(po->nverts)
	free(po->nverts);
      /* deliberately not freeing po->controlv as that is re-used from the
	 polygonal trim loops */
      free(po);
    }

  if(ay_status)
    ay_status = AY_ERROR;

 return ay_status;
} /* ay_stess_TessTrimmedPlanarNP */


/* ay_stess_TessNP:
 *  tesselate NURBS patch object <o> with quality factor <qf>;
 *  stores results in the object!
 */
int
ay_stess_TessNP(ay_object *o, int qf)
{
 int ay_status = AY_ERROR;
 char fname[] = "stess_TessNP";
 ay_nurbpatch_object *npatch;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_ENULL;

  if(ay_npt_istrimmed(o, 0))
    {
      /* this is a nontrivially trimmed NURBS patch */
      if(npatch->is_planar)
	ay_status = ay_stess_TessTrimmedPlanarNP(o, qf);
      else
	ay_status = ay_stess_TessTrimmedNP(o, qf);
    }
  else
    {
      if(o->down && o->down->next && npatch->is_planar &&
	 ay_npt_istrimmed(o, 1))
	{
	  /* this is a trivially trimmed planar NURBS patch */
	  ay_status = ay_stess_TessTrimmedPlanarNP(o, qf);
	}
    }

  if(ay_status == AY_ERROR)
    {
      /* trimmed tesselation failed or
	 this is an untrimmed or trivially trimmed NURBS patch,
         where we can safely ignore potentially present trim curves... */
      if(npatch->stess)
	{
	  ay_stess_destroy(npatch);
	}

      if(!(npatch->stess = calloc(1, sizeof(ay_stess))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      if(npatch->is_rat)
	{
	  ay_status = ay_stess_SurfacePoints4D(
					   npatch->width, npatch->height,
					   npatch->uorder-1, npatch->vorder-1,
					   npatch->uknotv, npatch->vknotv,
					   npatch->controlv, qf,
				 &npatch->stess->tessw, &npatch->stess->tessh,
				 &npatch->stess->tessv);
	}
      else
	{
	  ay_status = ay_stess_SurfacePoints3D(
                                           npatch->width, npatch->height,
					   npatch->uorder-1, npatch->vorder-1,
					   npatch->uknotv, npatch->vknotv,
					   npatch->controlv, qf,
				 &npatch->stess->tessw, &npatch->stess->tessh,
				 &npatch->stess->tessv);
	} /* if */
    } /* if */

  if(npatch->stess)
    npatch->tessqf = qf;

cleanup:

  if(ay_status)
    {
      ay_error(ay_status, fname, NULL);
    }

 return ay_status;
} /* ay_stess_TessNP */
