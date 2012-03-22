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


/* prototypes of functions local to this module: */
int ay_stess_FindMultiplePoints(int n, int p, double *U, double *P,
				int dim, int is_rat, int stride,
				int *m, double **V);

int ay_stess_IntersectLines2D(double *p1, double *p2, double *p3, double *p4,
			      double *ip);


int ay_stess_TessTrimCurves(ay_object *o, int qf, int *nt, double ***tt,
			    int **tl, int **td);

int ay_stess_MergeUVectors(ay_stess_uvp *a, ay_stess_uvp *b);

int ay_stess_MergeVVectors(ay_stess_uvp *a, ay_stess_uvp *b);

int ay_stess_TessTrimmedNPU(ay_object *o, int qf,
			    int numtrims,
			    double **tcs, int *tcslens, int *tcsdirs,
			    int *flcw, double *resud, double *resvd,
			    int *reslen, ay_stess_uvp ***result);

int ay_stess_TessTrimmedNPV(ay_object *o, int qf,
			    int numtrims,
			    double **tcs, int *tcslens, int *tcsdirs,
			    int flcw, int *reslen, ay_stess_uvp ***result);


/* functions: */

/* ay_stess_destroy:
 *  properly destroy an stess object
 */
int
ay_stess_destroy(ay_nurbpatch_object *np)
{
 ay_stess_uvp *p = NULL;
 ay_stess *stess = NULL;
 int i;

  if(!np)
    return AY_ENULL;

  if(!np->stess)
    {
      return AY_OK;
    }

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
    }

  if(stess->tcs)
    {
      for(i = 0; i < stess->tcslen; i++)
	{
	  free(stess->tcs[i]);
	}
      free(stess->tcs);
    }

  if(stess->tcslens)
    free(stess->tcslens);

  /* now free the stess object */
  free(stess);
  np->stess = NULL;

 return AY_OK;
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
int
ay_stess_FindMultiplePoints(int n, int p, double *U, double *P,
			    int dim, int is_rat, int stride,
			    int *m, double **V)
{
 int i, j, eq;
 double *p1, *p2, *t;

  if(!U || !P || !m || !V)
    return AY_ENULL;

  *m = 0;
  *V = NULL;

  p1 = P;

  for(i = 0; i < n-1; i++)
    {
      eq = AY_TRUE;

      p2 = p1 + stride;
      for(j = 0; j < p-1; j++)
	{
	  if((fabs(p1[0]-p2[0]) > AY_EPSILON) ||
	     ((dim>1) && (fabs(p1[1]-p2[1]) > AY_EPSILON)) ||
	     ((dim>2) && (fabs(p1[2]-p2[2]) > AY_EPSILON)))
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
	      return AY_EOMEM;
	    }
	  *V = t;
	  (*V)[(*m)] = U[i + p + 1];
	  (*m)++;
	  p1 += ((p-1)*stride);
	  i  += (p-1);
	}
      else
	{
	  p1 += stride;
	} /* if */
    } /* for */

 return AY_OK;
} /* ay_stess_FindMultiplePoints */


/* ay_stess_CurvePoints2D:
 *   calculate all points of a 2D curve (TrimCurve)
 */
int
ay_stess_CurvePoints2D(int n, int p, double *U, double *Pw, int is_rat, int qf,
		       int *Clen, double **C)
{
 int span, j, k, l, m, mc = 0, vi, incu, mc1 = 0, mc2 = 0;
 double *N = NULL, Cw[3], *Ct = NULL, u, ud, u1, *V;

  if(!U || !Pw || !Clen || !C)
    return AY_ENULL;

  if(!(N = calloc(p+1, sizeof(double))))
    return AY_EOMEM;
  /*
  ay_status = ay_stess_FindMultiplePoints(n, p, U, Pw, 2, is_rat, 4, &mc, &V);
  */
  *Clen = ((4 + n) * qf);

  if(!(Ct = calloc((*Clen + mc) * 2, sizeof(double))))
    {
      free(N);
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

	  memset(Cw, 0, 3*sizeof(double));

	  for(j = 0; j <= p; j++)
	    {
	      k = (span-p+j)*3;
	      Cw[0] = Cw[0] + N[j]*(Pw[k]*Pw[k+2]);
	      Cw[1] = Cw[1] + N[j]*(Pw[k+1]*Pw[k+2]);
	      Cw[2] = Cw[2] + N[j]*Pw[k+2];
	    }

	  Ct[m]   = Cw[0]/Cw[2];
	  Ct[m+1] = Cw[1]/Cw[2];

	  /* make sure that there are no consecutive identical
	     points in the output, as Merge(U/V)Vectors() below
	     would choke on that (unnecessarily) */

	  if(!l || (fabs(Ct[m-2] - Ct[m]) > AY_EPSILON) ||
	     (fabs(Ct[m-1] - Ct[m+1]) > AY_EPSILON))
	    {
	      m += 2;
	    }
	  else
	    {
	      mc2++;
	    }

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
	      k = (span-p+j)*2;
	      Ct[m]   = Ct[m]   + N[j]*Pw[k];
	      Ct[m+1] = Ct[m+1] + N[j]*Pw[k+1];
	    }

	  /* make sure that there are no consecutive identical
	     points in the output, as Merge(U/V)Vectors() below
	     would choke on that (unnecessarily) */
	  if(!l || (fabs(Ct[m-2] - Ct[m]) > AY_EPSILON) ||
	     (fabs(Ct[m-1] - Ct[m+1]) > AY_EPSILON))
	    {
	      m += 2;
	    }
	  else
	    {
	      mc2++;
	    }

	  if(incu)
	    u += ud;
	} /* for */

    } /* if */

  *C = Ct;
  *Clen += mc1-mc2;

  free(N);

 return AY_OK;
} /* ay_stess_CurvePoints2D */


/* ay_stess_CurvePoints3D:
 *   calculate all points of a 3D curve
 */
int
ay_stess_CurvePoints3D(int n, int p, double *U, double *Pw, int is_rat, int qf,
		       int *Clen, double **C)
{
 int ay_status = AY_OK;
 int span, j, k, l, m, mc = 0, vi, incu, mc1 = 0;
 double *N = NULL, Cw[4], *Ct = NULL, u, ud, u1, *V;

  if(!U || !Pw || !Clen || !C)
    return AY_ENULL;

  if(!(N = calloc(p+1, sizeof(double))))
    return AY_EOMEM;

  ay_status = ay_stess_FindMultiplePoints(n, p, U, Pw, 3, is_rat, 4, &mc, &V);

  *Clen = ((4 + n) * qf);

  if(!(Ct = calloc((*Clen + mc) * 3, sizeof(double))))
    {
      free(N);
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
ay_stess_TessTrimCurves(ay_object *o, int qf, int *nt, double ***tt,
			int **tl, int **td)
{
 int ay_status = AY_OK;
 double *dtmp = NULL, angle, **tts, p1[4], p2[4], *loopelem = NULL;
 double *tmp = NULL;
 int i, j, numtrims = 0, *tls, *tds, loop_empty, ti, stride;
 ay_object *d = NULL, *dd = NULL, *loop = NULL;
 ay_nurbcurve_object *c = NULL;
 double mm[16];

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
    return AY_ERROR;

  /* tesselate trimloops and get their orientation */
  if(!(tts = calloc(numtrims, sizeof(double *))))
    return AY_EOMEM;
  if(!(tls = calloc(numtrims, sizeof(int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(tds = calloc(numtrims, sizeof(int))))
    { ay_status = AY_EOMEM; goto cleanup; }

  i = 0;
  d = o->down;
  while(d->next)
    {
      c = NULL;
      switch(d->type)
	{
	case AY_IDNCURVE:
	  c = (ay_nurbcurve_object *)d->refine;
	  /* get curves transformation-matrix */
	  ay_trafo_creatematrix(d, mm);
	  break;
	case AY_IDLEVEL:
	  loop_empty = AY_TRUE;
	  loop = d->down;
	  while(loop->next)
	    {
	      if(loop->type == AY_IDNCURVE)
		{
		  c = (ay_nurbcurve_object *)loop->refine;
		  ay_trafo_creatematrix(loop, mm);
		}
	      else
		{
		  dd = NULL;
		  ay_status = ay_provide_object(loop, AY_IDNCURVE, &dd);
		  if(dd)
		    {
		      c = (ay_nurbcurve_object *)dd->refine;
		      ay_trafo_creatematrix(dd, mm);
		    }
		} /* if */

	      if(c)
		{
		  loop_empty = AY_FALSE;

		  if(c->is_rat)
		    stride = 3;
		  else
		    stride = 2;

		  if(!(dtmp = calloc(c->length*stride, sizeof(double))))
		    { ay_status = AY_EOMEM; goto cleanup; }

		  /* apply transformations */
		  for(j = 0; j < c->length; j++)
		    {
		      memcpy(p1, &(c->controlv[j*4]), 4*sizeof(double));
		      AY_APTRAN3(p2, p1, mm)
		      memcpy(&(dtmp[j*stride]), p2, 3*sizeof(double));
		      if(c->is_rat)
			dtmp[j*stride+2] = c->controlv[j*4+3];
		    }
		  ti = 0;
		  ay_stess_CurvePoints2D(c->length, c->order-1, c->knotv, dtmp,
					 c->is_rat, qf, &ti, &loopelem);

		  if(loopelem)
		    {
		      /* for all loop elements (!= first), arrange to
			 piece the elements together correctly (without
			 double endpoints) */
		      if(loop != d->down)
			(tls[i])--;
		      /* now append new element to tts[i] */
		      tls[i] += ti;
		      tmp = realloc(tts[i], tls[i]*2*sizeof(double));
		      if(!tmp)
			{ ay_status = AY_EOMEM; goto cleanup; }
		      tts[i] = tmp;
		      memcpy(&((tts[i])[(tls[i]-ti)*2]), loopelem,
			     ti*2*sizeof(double));
		      free(loopelem);
		      loopelem = NULL;
		    }

		  free(dtmp);
		  dtmp = NULL;
		  if(dd)
		    {
		      ay_object_deletemulti(dd);
		      dd = NULL;
		    }
		} /* if */
	      loop = loop->next;
	    } /* while */
	  if(!loop_empty)
	    {
	      /* get orientation of trimloop */
	      /* XXXX fake here a new c based on tls tts...*/
	      angle = 0.0;
	      ay_nct_getorientation(c, &angle);
	      if(angle < 0.0)
		tds[i] = 1;

	      i++;
	    }
	  dd = NULL;
	  c = NULL;
	  break;
	default:
	  dd = NULL;
	  ay_status = ay_provide_object(d, AY_IDNCURVE, &dd);
	  if(dd)
	    {
	      c = (ay_nurbcurve_object *)dd->refine;
	      ay_trafo_creatematrix(dd, mm);
	    }
	  break;
	} /* switch */

      if(c)
	{
	  if(c->is_rat)
	    stride = 3;
	  else
	    stride = 2;

	  if(!(dtmp = calloc(c->length*stride, sizeof(double))))
	    { ay_status = AY_EOMEM; goto cleanup; }

	  /* apply transformations */
	  for(j = 0; j < c->length; j++)
	    {
	      memcpy(p1, &(c->controlv[j*4]), 4*sizeof(double));
	      AY_APTRAN3(p2, p1, mm)
	      memcpy(&(dtmp[j*stride]), p2, stride*sizeof(double));
	      if(c->is_rat)
		dtmp[j*stride+2] = c->controlv[j*4+3];
	    }

	  ay_stess_CurvePoints2D(c->length, c->order-1, c->knotv, dtmp,
				 c->is_rat, qf, &(tls[i]), &(tts[i]));

	  free(dtmp);
	  dtmp = NULL;

	  /* get orientation of trimloop */
	  angle = 0.0;
	  ay_nct_getorientation(c, &angle);
	  if(angle < 0.0)
	    tds[i] = 1;

	  if(dd)
	    {
	      ay_object_deletemulti(dd);
	      dd = NULL;
	    }
	  i++;
	} /* if */

      d = d->next;
    } /* while */

  *nt = numtrims;
  *tt = tts;
  *tl = tls;
  *td = tds;

  /* prevent cleanup code from doing something harmful */
  tts = NULL;
  tls = NULL;
  td = NULL;

cleanup:
  if(tts)
    free(tts); 
  if(tls)
    free(tls);
  if(td)
    free(td);
  if(dtmp)
    free(dtmp);

 return ay_status;
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
	printf("Uneven number of trimloop points detected!\n");
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
      while(!inserted && b)
	{
	  if(p1->next)
	    {
	      if(p2)
		{
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
		      /*
		      printf("Transmogrifying point!\n");
		      */
		      p1->type = 1;
		      p1->dir = p2->dir;
		      p3 = p2->next;
		      free(p2);
		      p2 = p3;
		      b = p3;
		      inserted = 1;
		    }
		}

	      if(p2)
		{
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
	    }
	  else
	    {
	      /* falling off the border of the patch
		 => discard remaining trim points */
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
	printf("Uneven number of trimloop points detected!\n");
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
      while(!inserted && b)
	{
	  if(p1->next)
	    {
	      if(p2)
		{
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
		      /*
		      fprintf(stderr,"Transmogrifying point!\n");
		      */
		      p1->type = 1;
		      p1->dir = p2->dir;
		      p3 = p2->next;
		      free(p2);
		      p2 = p3;
		      b = p3;
		      inserted = 1;
		    } /* if */
		} /* if */

	      if(p2)
		{
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
	    }
	  else
	    {
	      /* falling off the border of the patch
		 => discard remaining trim points */
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


/* ay_stess_TessTrimmedNPU:
 *  tesselate NURBS patch <o> into lines in parametric direction u
 */
int
ay_stess_TessTrimmedNPU(ay_object *o, int qf,
			int numtrims,
			double **tcs, int *tcslens, int *tcsdirs,
			int *flcw, double *resud, double *resvd,
			int *reslen, ay_stess_uvp ***result)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *p = NULL;
 ay_stess_uvp **uvps = NULL, *uvpptr, *newuvp, **olduvp, *trimuvp;
 ay_stess_uvp *uvpptr2, *uvpptr3;
 double *tt, ipoint[2] = {0};
 double p3[2], p4[2], *U, *V, u, v;
 double *fd1, *fd2, temp[3] = {0}, ders[12] = {0};
 double umin, umax, vmin, vmax, ud, vd;
 int i, j, k, l, ind;
 int out = 0, first_loop = AY_TRUE, first_loop_cw = AY_FALSE;
 int Cm, Cn, trimloop_point, done;
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
  u = umin;
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
	      ay_error(AY_ERROR, fname,
		       "Intersecting or misoriented trimcurves!");

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
		      uvpptr2 = *olduvp;
		      *olduvp = uvpptr;
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

      /* free left over outer uvps */
      if(*olduvp != uvps[i] && out)
	{
	  while(*olduvp)
	    {
	      uvpptr2 = *olduvp;
	      *olduvp = (*olduvp)->next;
	      free(uvpptr2);
	    }
	}

    } /* for */

  if(first_loop_cw)
    {
      /* remove unwanted lines (all lines that contain no trimloop point) */
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
ay_stess_TessTrimmedNPV(ay_object *o, int qf,
			int numtrims,
			double **tcs, int *tcslens, int *tcsdirs,
			int flcw, int *reslen, ay_stess_uvp ***result)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *p = NULL;
 ay_stess_uvp **uvps = NULL, *uvpptr, *newuvp, **olduvp, *trimuvp;
 ay_stess_uvp *uvpptr2, *uvpptr3;
 double *tt, ipoint[2] = {0};
 double p3[2], p4[2], *U, *V, u, v;
 double *fd1, *fd2, temp[3] = {0}, ders[12] = {0};
 double umin, umax, vmin, vmax, ud, vd;
 int i, j, k, l, ind;
 int out = 0, first_loop = AY_TRUE, first_loop_cw = AY_FALSE;
 int Cm, Cn, trimloop_point, done;
 char fname[] = "TessTrimmedNPV";

  p = (ay_nurbpatch_object *)o->refine;
  first_loop_cw = flcw;

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
  u = umin;
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
	      ay_error(AY_ERROR, fname,
		       "Intersecting or misoriented trimcurves!");

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
		      uvpptr2 = *olduvp;
		      *olduvp = uvpptr;
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

      /* free left over outer uvps */
      if(*olduvp != uvps[i] && out)
	{
	  while(*olduvp)
	    {
	      uvpptr2 = *olduvp;
	      *olduvp = (*olduvp)->next;
	      free(uvpptr2);
	    }
	}

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
	  ay_nb_CompFirstDerSurf4D(p->width-1, p->height-1,
			    p->uorder-1, p->vorder-1, p->uknotv, p->vknotv,
			    p->controlv, uvpptr->u, uvpptr->v, ders);

	  memcpy(uvpptr->C, ders, 3*sizeof(double));
	  fd1 = &(ders[3]);
	  fd2 = &(ders[6]);
	  AY_V3CROSS(temp, fd2, fd1);
	  memcpy(&(uvpptr->C[3]), temp, 3*sizeof(double));

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
ay_stess_DrawTrimmedSurface(ay_object *o)
{
 double p4[4] = {0};
 int i, j, out = 0;
 ay_stess *stess = NULL;
 ay_stess_uvp *uvpptr;
 ay_nurbpatch_object *p = NULL;

  p = (ay_nurbpatch_object *)o->refine;

  stess = p->stess;

  if(!stess)
    return AY_ENULL;

  /* draw iso-u lines */
  for(i = 0; i < stess->upslen; i++)
    {
      uvpptr = stess->ups[i];

      if(!stess->ft_cw)
	{
	  out = 0;
	  glBegin(GL_LINE_STRIP);
	}
      else
	out = 1;

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


  /* draw iso-v lines */
  for(i = 0; i < stess->vpslen; i++)
    {
      uvpptr = stess->vps[i];

      if(!stess->ft_cw)
	{
	  out = 0;
	  glBegin(GL_LINE_STRIP);
	}
      else
	out = 1;

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
	      if(!out)
		glVertex3dv((GLdouble*)(uvpptr->C));
	    } /* if */

	  uvpptr = uvpptr->next;
	}  /* while */

      if(!out)
	glEnd();

    } /* for */


  /* draw trimcurves (outlines) */
  for(i = 0; i < stess->tcslen; i++)
    {
      glBegin(GL_LINE_STRIP);
       for(j = 0; j < stess->tcslens[i]; j++)
	 {
	   ay_nb_SurfacePoint4D(p->width-1, p->height-1,
	       	       p->uorder-1, p->vorder-1, p->uknotv, p->vknotv,
	       	       p->controlv, stess->tcs[i][j*2],
				stess->tcs[i][j*2+1], p4);

	   glVertex3dv((GLdouble*)(p4));
	 } /* for */
      glEnd();
    } /* for */

 return AY_OK;
} /* ay_stess_DrawTrimmedSurface */


/* ay_stess_ShadeTrimmedSurface:
 *
 */
int
ay_stess_ShadeTrimmedSurface(ay_object *o)
{
 int i, forwardu1;
 ay_stess *stess = NULL;
 ay_stess_uvp *u1, *u2/*, *v1, *v2*/;
 ay_nurbpatch_object *p = NULL;

  p = (ay_nurbpatch_object *)o->refine;

  stess = p->stess;

  if(!stess)
    return AY_ENULL;

  for(i = 0; i < (stess->upslen-1); i++)
    {
      u1 = stess->ups[i];
      u2 = stess->ups[i+1];

      if(!u1 || !u2 || !u1->next || !u2->next)
	continue;

      glBegin(GL_TRIANGLE_STRIP);
      glNormal3dv((GLdouble*)&((u1->C)[3]));
      glVertex3dv((GLdouble*)(u1->C));
      glNormal3dv((GLdouble*)&((u2->C)[3]));
      glVertex3dv((GLdouble*)(u2->C));

      u1 = u1->next;
      u2 = u2->next;

      while(u1 && u2)
	{
	  glNormal3dv((GLdouble*)&((u1->C)[3]));
	  glVertex3dv((GLdouble*)(u1->C));
	  glNormal3dv((GLdouble*)&((u2->C)[3]));
	  glVertex3dv((GLdouble*)(u2->C));


	  if(u1->type == 1 || u2->type == 1)
	    {
	      /* we cross a trimcurve/leave the surface
		 => forward to next complete cell (where both,
		 u1 and u2, point to the surface again) */

	      /* break current strip */
	      glEnd();

	      forwardu1 = AY_FALSE;

	      if(u1->type == 0 && u2->type == 1)
		{
		  forwardu1 = AY_TRUE;
		}

	      if(u1->type == 1 && u2->type == 1)
		{
		  u1 = u1->next;
		  u2 = u2->next;
		}
	      else
		{
		  if(forwardu1)
		    {
		      /* u2 is off, u1 still on, forward u1 to next boundary */
		      /* but stop right before u2 in any case */
		      u2 = u2->next;
		      if(u2)
			{
			  while(u1 && u1->next &&
				((u1->next->v < u2->v) || u1->next->type))
			    {
			      u1 = u1->next;
			      if(u1 && u1->type)
				{
				  u1 = u1->next;
				  break;
				}
			    }
			}
		    }
		  else
		    {
		      /* forwardu2 would be AY_TRUE */
		      /* u1 is off, u2 still on, forward u2 to next boundary */
		      /* but stop right before u1 in any case */
		      u1 = u1->next;
		      if(u1)
			{
			  while(u2 && u2->next &&
				((u2->next->v < u1->v) || u2->next->type))
			    {
			      u2 = u2->next;
			      if(u2 && u2->type)
				{
				  u2 = u2->next;
				  break;
				}
			    }
			}
		    }
		}

	      while(u1 && u2)
		{
		  if(fabs(u1->v - u2->v) < stess->vd)
		    {
		      if((int)(u1->v/stess->vd) == (int)(u2->v/stess->vd))
			{
			  break;
			}
		    }

		  if(u1->v < u2->v)
		    {
		      while(u1 && u1->next && (u1->next->v < u2->v))
			u1 = u1->next;

		      if(!u1)
			break;

		      if(fabs(u1->v - u2->v) < stess->vd)
			{
			  if((int)(u1->v/stess->vd) == (int)(u2->v/stess->vd))
			    {
			      break;
			    }
			}

		      while(u2 && u2->next && (u2->next->v < u1->v))
			u2 = u2->next;

		      if(!u2)
			break;

		      if(fabs(u1->v - u2->v) < stess->vd)
			{
			  if((int)(u1->v/stess->vd) == (int)(u2->v/stess->vd))
			    {
			      break;
			    }
			}
		    }
		  else
		    {

		      while(u2 && u2->next && (u2->next->v < u1->v))
			u2 = u2->next;

		      if(!u2)
			break;

		      if(fabs(u1->v - u2->v) < stess->vd)
			{
			  if((int)(u1->v/stess->vd) == (int)(u2->v/stess->vd))
			    {
			      break;
			    }
			}

		      while(u1 && u1->next && (u1->next->v < u2->v))
			u1 = u1->next;

		      if(!u1)
			break;

		      if(fabs(u1->v - u2->v) < stess->vd)
			{
			  if((int)(u1->v/stess->vd) == (int)(u2->v/stess->vd))
			    {
			      break;
			    }
			}

		    }

		  /* avoid infinite loop */
		  if(u1 && u2)
		    {
		      if(u1->v < u2->v)
			u1 = u1->next;
		      else
			u2 = u2->next;
		    }
		} /* while */

	      if(!u1 || !u2)
		{
		  glBegin(GL_TRIANGLE_STRIP);
		  break;
		}

	      /* if we get here, u1 and u2 point to the next
		 complete cell in the surface */

	      /* start a new strip */
	      glBegin(GL_TRIANGLE_STRIP);
	      glNormal3dv((GLdouble*)&((u1->C)[3]));
	      glVertex3dv((GLdouble*)(u1->C));
	      glNormal3dv((GLdouble*)&((u2->C)[3]));
	      glVertex3dv((GLdouble*)(u2->C));
	    } /* if */

	  u1 = u1->next;
	  u2 = u2->next;
	} /* while */

      glEnd();

    } /* for */
#if 0
  for(i = 0; i < (stess->vpslen-1); i++)
    {
      v1 = stess->vps[i+1];
      v2 = stess->vps[i];

      if(!v1 || !v2 || !v1->next || !v2->next)
	continue;

      glBegin(GL_TRIANGLE_STRIP);
      glNormal3dv((GLdouble*)&((v1->C)[3]));
      glVertex3dv((GLdouble*)(v1->C));
      glNormal3dv((GLdouble*)&((v2->C)[3]));
      glVertex3dv((GLdouble*)(v2->C));

      v1 = v1->next;
      v2 = v2->next;

      while(v1 && v2)
	{
	  glNormal3dv((GLdouble*)&((v1->C)[3]));
	  glVertex3dv((GLdouble*)(v1->C));
	  glNormal3dv((GLdouble*)&((v2->C)[3]));
	  glVertex3dv((GLdouble*)(v2->C));

	  if(v1->type == 1 && v2->type == 1)
	    {
	      v1 = v1->next;
	      v2 = v2->next;
	      if(!v1 || !v2)
		{
		  glBegin(GL_TRIANGLE_STRIP);
		  break;
		}

	      glEnd();
	      glBegin(GL_TRIANGLE_STRIP);
	      glNormal3dv((GLdouble*)&((v1->C)[3]));
	      glVertex3dv((GLdouble*)(v1->C));
	      glNormal3dv((GLdouble*)&((v2->C)[3]));
	      glVertex3dv((GLdouble*)(v2->C));
	    } /* if */

	  if(v1->type == 1 && v2->type == 0)
	    {
	      if(!v1->next)
		{
		  glEnd();
		  glBegin(GL_TRIANGLE_STRIP);
		  glNormal3dv((GLdouble*)&((v1->C)[3]));
		  glVertex3dv((GLdouble*)(v1->C));
		  while(v2)
		    {
		      glNormal3dv((GLdouble*)&((v2->C)[3]));
		      glVertex3dv((GLdouble*)(v2->C));
		      glNormal3dv((GLdouble*)&((v1->C)[3]));
		      glVertex3dv((GLdouble*)(v1->C));
		      v2 = v2->next;
		    }
		  glEnd();
		  glBegin(GL_TRIANGLE_STRIP);
		  break;
		}

	      v1 = v1->next;
	      glEnd();

	      while(v2 && (v2->u+stess->ud < v1->u))
		v2 = v2->next;

	      while(v1 && (v1->u+stess->ud < v2->u))
		v1 = v1->next;

	      if(!v1 || !v2)
		{
		  glBegin(GL_TRIANGLE_STRIP);
		  break;
		}

	      glBegin(GL_TRIANGLE_STRIP);
	      glNormal3dv((GLdouble*)&((v1->C)[3]));
	      glVertex3dv((GLdouble*)(v1->C));
	      glNormal3dv((GLdouble*)&((v2->C)[3]));
	      glVertex3dv((GLdouble*)(v2->C));
	    } /* if */

	  if(v1->type == 0 && v2->type == 1)
	    {
	      if(!v2->next)
		{
		  glEnd();
		  glBegin(GL_TRIANGLE_STRIP);
		  while(v1)
		    {
		      glNormal3dv((GLdouble*)&((v1->C)[3]));
		      glVertex3dv((GLdouble*)(v1->C));
		      glNormal3dv((GLdouble*)&((v2->C)[3]));
		      glVertex3dv((GLdouble*)(v2->C));
		      v1 = v1->next;
		    }
		  glEnd();
		  glBegin(GL_TRIANGLE_STRIP);
		  break;
		}

	      v2 = v2->next;
	      glEnd();

	      while(v1 && (v1->u+stess->ud < v2->u))
		v1 = v1->next;

	      while(v2 && (v2->u+stess->ud < v1->u))
		v2 = v2->next;

	      if(!v1 || !v2)
		{
		  glBegin(GL_TRIANGLE_STRIP);
		  break;
		}

	      glBegin(GL_TRIANGLE_STRIP);
	      glNormal3dv((GLdouble*)&((v1->C)[3]));
	      glVertex3dv((GLdouble*)(v1->C));
	      glNormal3dv((GLdouble*)&((v2->C)[3]));
	      glVertex3dv((GLdouble*)(v2->C));
	    } /* if */

	  v1 = v1->next;
	  v2 = v2->next;
	} /* while */

      glEnd();

    } /* for */
#endif
 return AY_OK;
} /* ay_stess_ShadeTrimmedSurface */


/* ay_stess_TessTrimmedNP:
 *
 */
int
ay_stess_TessTrimmedNP(ay_object *o, int qf)
{
 int ay_status = AY_OK;
 int first_loop_cw = 0;
 ay_nurbpatch_object *p = NULL;
 ay_stess *st = NULL;

  p = (ay_nurbpatch_object *)o->refine;

  if(p->stess)
    {
      ay_stess_destroy(p);
    }

  if(!(p->stess = calloc(1, sizeof(ay_stess))))
    return AY_EOMEM;

  st = p->stess;

  ay_status = ay_stess_TessTrimCurves(o, qf,
				      &st->tcslen, &st->tcs,
				      &st->tcslens, &st->tcsdirs);

  if(ay_status)
    goto cleanup;

  ay_status = ay_stess_TessTrimmedNPU(o, qf, st->tcslen, st->tcs,
				      st->tcslens, st->tcsdirs,
				      &first_loop_cw, &st->ud, &st->vd,
				      &st->upslen, &st->ups);

  if(ay_status)
    goto cleanup;

  /**/

  ay_status = ay_stess_TessTrimmedNPV(o, qf, st->tcslen, st->tcs,
				      st->tcslens, st->tcsdirs,
				      first_loop_cw, &st->vpslen, &st->vps);

  if(ay_status)
    goto cleanup;

  st->ft_cw = first_loop_cw;

  /* return result */
  p = NULL;

  /* clean up the mess */
cleanup:

  if(p)
    {
      ay_stess_destroy(p);
    }

 return AY_OK;
} /* ay_stess_TessTrimmedNP */


/* ay_stess_TessNP:
 *  tesselate NURBS patch object <o> with quality factor <qf>;
 *  stores results in the object!
 */
int
ay_stess_TessNP(ay_object *o, int qf)
{
 int ay_status = AY_OK;
 /*char fname[] = "stess_TessNP";*/
 ay_nurbpatch_object *npatch;

  if(!o)
    return AY_ENULL;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_ENULL;

  if(ay_npt_istrimmed(o, 0))
    {
      /* this is a nontrivially trimmed NURBS patch */
      ay_status = ay_stess_TessTrimmedNP(o, qf);
    }
  else
    {
      /* this is an untrimmed or trivially trimmed NURBS patch,
         we can safely ignore potentially present trim curves... */
      if(npatch->stess)
	{
	  ay_stess_destroy(npatch);
	}

      if(!(npatch->stess = calloc(1, sizeof(ay_stess))))
	return AY_EOMEM;

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

      npatch->tessqf = qf;
    } /* if */

 return AY_OK;
} /* ay_stess_TessNP */

/* remove local preprocessor definitions */
#undef AY_STESSEPSILON
