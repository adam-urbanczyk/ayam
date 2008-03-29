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

/* act.c - approximating curve tools */

int ay_act_multmatrixmn(double *M1, double *M2, int m, int n, double *R);


/* ay_act_multmatrixmn:
 */
int
ay_act_multmatrixmn(double *M1, double *M2, int m, int n, double *R)
{
 int i, j, k;
 int a, b, c = 0;
 double t;

  if(!M1 || !M2 || !R)
    return AY_ENULL;

  /* M1 is m*n, M2 is n*m; R will be n*n */

  /* i shall be column number for R */
  for(i = 0; i < n; i++)
    {
      /* j shall be row number for R */
      for(j = 0; j < n; j++)
	{
	  /* arrange to access row j in M1 */
	  a = j;
	  /* arrange to access column i in M2 */
	  b = i*n;
	  t = 0.0;
	  for(k = 0; k < m; k++)
	    {
	      t += M1[a] * M2[b];
	      /* forward one column in M1 */
	      a += n;
	      /* forward one row in M2 */
	      b++;
	    } /* for */
	  R[c] = t;
	  c++;
	} /* for */
    } /* for */

 return AY_OK;
} /* ay_act_multmatrixmn */


/* ay_act_solve:
 *  solve the linear equation system: M1*R=M2
 *  (M1[m*n], M2[n], R[n])
 */
int
ay_act_solve(double *M1, double *M2, int m, int n, double *R)
{
 int ay_status = AY_OK;
 int stride = 3, i, j, k;
 double t, *A = NULL;
 int *pivot = NULL;

  if(!M1 || !M2 || !R)
    return AY_ENULL;

  if(m == n)
    {
      /* M1 is square => use LU decomposition to solve */
      if(!(A = calloc(n*n, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      if(!(pivot = calloc(n, sizeof(int))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      memcpy(A, M1, n*n*sizeof(double));

      ay_status = ay_nb_LUDecompose(n, A, pivot);

      if(ay_status)
	{ goto cleanup; }

      ay_status = ay_nb_LUInvert(n, A, pivot);

      if(ay_status)
	{ goto cleanup; }

      /* multiply through: R = A(^-1)*M2, and store results */
      for(i = 0; i < n; i++)
	{
	  for(j = 0; j < stride; j++)
	    {
	      t = 0.0;
	      for(k = 0; k < n; k++)
		{
		  t += (A[i*n+k] * M2[k*stride+j]);
		}
	      R[i*stride+j] = t;
	    }
	}

    }
  else
    {
      /* M1 is not square => use SV decomposition to solve */

    }

cleanup:

  if(A)
    free(A);

  if(pivot)
    free(pivot);

 return ay_status;
} /* ay_act_solve */


/* ay_act_leastSquares:
 */
int
ay_act_leastSquares(double *Q, int m, int n, int p, double **U, double **P)
{
 int ay_status = AY_OK;
 int a, i, i2, j, k, istride = 3, ostride = 4, span;
 double da, d, *ub = NULL;
 double *Ns = NULL, *Nt = NULL, *NN = NULL;
 double *R = NULL, *rk = NULL, *N = NULL, *X = NULL, *B = NULL;
 double *funs = NULL;

  if(!Q || !U || !P)
    return AY_ENULL;

  ay_knots_chordparam(Q, m/*Qlen*/, 3, &ub);

  if(!ub)
    {
      return AY_ERROR;
    }

  if(!(*U = calloc(n+p+1, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(*P = calloc(n*ostride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  for(i = 0; i < p+1; i++)
    {
      (*U)[i] = 0.0;
    }
  /*
  d = (m+1) / (double)(n-p+1);
  for(j = 1; j < n-p; j++)
    {
      i = (int)(j*d);
      a = j*d-i;
      (*U)[p+j] = (1-a)*ub[i-1] + a*ub[i];
    }
  */
  /* from NURBS++ */
  d = m / (double)n;
  for(j = 1; j < n-p; j++)
    {
      (*U)[p+j] = 0.0;
      for(k = j; k < j+p; k++)
	{
	  i = (int)(k*d);
	  da = k*d-i;
	  i2 = (int)((k-1)*d);
	  (*U)[p+j] += da*ub[i2] + (1.0-da)*ub[i];
	}
      (*U)[p+j] /= p;
    }

  for(i = n; i < n+p+1; i++)
    {
      (*U)[i] = 1.0;
    }

  if(!(R = calloc(n*istride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(rk = calloc(m*istride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(N = calloc(m*n, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(funs = calloc(p+1, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  /* set up N and rk */
  N[0] = 1.0;
  N[(m-1)*n+(n-1)] = 1.0;
  for(i = 0; i < m; i++)
    {
      span = ay_nb_FindSpan(n, p, ub[i], *U);

      /* protect BasisFuns() from bad spans */
      if(span >= n)
	span = n-1;

      memset(funs, 0, (p+1)*sizeof(double));
      ay_nb_BasisFuns(span, ub[i], p, *U, funs);
      for(j = 0; j <= p; j++)
	{
	  a = i*n+(span-p+j);
	  N[a] = funs[j];
	  /*
	  if(isnan(N[a]))
	    goto cleanup;
	  */
	}

      /*rk[i] = Q[i]-N(i,0)*Q[0]-N(i,n-1)*Q[m-1];*/
      rk[i*istride]   = Q[i*istride]  - N[i*n]*Q[0] -
	N[i*n+n-1]*Q[(m-1)*istride];
      rk[i*istride+1] = Q[i*istride+1]- N[i*n]*Q[1] -
	N[i*n+n-1]*Q[(m-1)*istride+1];
      rk[i*istride+2] = Q[i*istride+2]- N[i*n]*Q[2] -
	N[i*n+n-1]*Q[(m-1)*istride+2];
    } /* for */

  /* set up R */
  for(i = 0; i < n; i++)
    {
      /*R[i*istride] = 0.0;*/
      memset(&(R[i*istride]), 0, istride*sizeof(double));

      for(j = 0; j < m; j++)
	{
	  /*R[i] += N(j,i)*rk[j] ;*/
	  R[i*istride]   += N[j*n+i]*rk[j*istride];
	  R[i*istride+1] += N[j*n+i]*rk[j*istride+1];
	  R[i*istride+2] += N[j*n+i]*rk[j*istride+2];
	}

      if(R[i*istride]*R[i*istride]     < AY_EPSILON &&
	 R[i*istride+1]*R[i*istride+1] < AY_EPSILON &&
	 R[i*istride+2]*R[i*istride+2] < AY_EPSILON)
	{
	  ay_status = AY_ERROR;
	  goto cleanup;
	}
    } /* for */

  /* solve N^T*N*P = R */

  if((n-2) > 0)
    {
      if(!(Ns = calloc((m-2)*(n-2), sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      if(!(Nt = calloc((m-2)*(n-2), sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      if(!(NN = calloc((m-2)*(n-2), sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      if(!(X = calloc((n-2)*istride, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      if(!(B = calloc((n-2)*istride, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      /* fill Ns & Nt */
      a = 0;
      for(i = 1; i < m-1; i++)
	{
	  for(j = 1; j < n-1; j++)
	    {
	      Ns[a] = N[j*n+i];
	      Nt[a] = N[i*n+j];
	      a++;
	    }
	}

      /* fill B */
      memcpy(B, &(R[istride]), (n-2)*istride*sizeof(double));

      /* do N^T*N */
      ay_act_multmatrixmn(Nt, Ns, m-2, n-2, NN);

      /* solve the linear equation system NN*X=B */
      ay_status = ay_act_solve(NN, B, m-2, n-2, X);

      if(ay_status)
	{ goto cleanup; }

      /* save results from X */
      j = 0;
      for(i = 1; i < n-1; i++)
	{
	  memcpy(&((*P)[i*ostride]), &(X[j*istride]), istride*sizeof(double));
	  j++;
	}

    } /* if */

  /* first and last points are data points */
  /*P[0] = Q[0];*/
  memcpy(*P, Q, istride*sizeof(double));
  /*P[n-1] = Q[m-1];*/
  memcpy(&((*P)[(n-1)*ostride]), &(Q[(m-1)*istride]), istride*sizeof(double));

  /* set weights */
  a = 3;
  for(i = 0; i < n; i++)
    {
      (*P)[a] = 1.0;
      a += ostride;
    }

cleanup:

  if(ay_status)
    {
      if(*U)
	free(*U);
      *U = NULL;

      if(*P)
	free(*P);
      *P = NULL;
    }

  if(ub)
    free(ub);

  if(R)
    free(R);

  if(rk)
    free(rk);

  if(N)
    free(N);

  if(funs)
    free(funs);

  if(Ns)
    free(Ns);

  if(Nt)
    free(Nt);

  if(NN)
    free(NN);

  if(X)
    free(X);

  if(B)
    free(B);

 return ay_status;
} /* ay_act_leastSquares */


/* ay_act_resize:
 *  resize an interpolating curve
 */
int
ay_act_resize(ay_acurve_object *curve, int new_length)
{
 int ay_status = AY_OK;
 int a, b, i, j;
 int *newpersec = NULL, new = 0;
 double *ncontrolv = NULL, v[3] = {0}, t = 0.0, *cv = NULL;

  if(new_length == curve->length)
    return ay_status;

  if(!(ncontrolv = calloc(3*new_length, sizeof(double))))
    return AY_EOMEM;

  if(new_length < curve->length)
    {
      a = 0;
      for(i = 0; i < new_length; i++)
	{
	  memcpy(&ncontrolv[a], &(curve->controlv[a]), 3*sizeof(double));
	  a+=3;
	}
    }
  else
    {
      /* distribute new points */
      new = new_length - curve->length;

      if(!(newpersec = calloc((curve->length-1), sizeof(int))))
	return AY_EOMEM;
      cv = curve->controlv;

      while(new)
	{
	  for(i = 0; i < (curve->length-1); i++)
	    {
	      if((cv[i*3] != cv[(i+1)*3]) ||
		 (cv[i*3+1] != cv[(i+1)*3+1]) ||
		 (cv[i*3+2] != cv[(i+1)*3+2]))
		{
		  if(new)
		    {
		      (newpersec[i])++;
		      new--;
		    } /* if */
		} /* if */
	    } /* for */
	} /* while */

      a = 0;
      b = 0;
      for(i = 0; i < (curve->length-1); i++)
	{
	  memcpy(&ncontrolv[b], &(curve->controlv[a]), 3*sizeof(double));
	  b+=3;

	  if((cv[i*3] != cv[(i+1)*3]) ||
	     (cv[i*3+1] != cv[(i+1)*3+1]) ||
	     (cv[i*3+2] != cv[(i+1)*3+2]))
	    {
	      for(j = 1; j <= newpersec[i]; j++)
		{

		  v[0] = curve->controlv[a+3]   - curve->controlv[a];
		  v[1] = curve->controlv[a+3+1] - curve->controlv[a+1];
		  v[2] = curve->controlv[a+3+2] - curve->controlv[a+2];

		  t = j/(newpersec[i]+1.0);

		  AY_V3SCAL(v,t);

		  ncontrolv[b]   = curve->controlv[a]   + v[0];
		  ncontrolv[b+1] = curve->controlv[a+1] + v[1];
		  ncontrolv[b+2] = curve->controlv[a+2] + v[2];
		  ncontrolv[b+3] = 1.0;

		  b+=3;
		} /* for */
	    } /* if */

	  a+=3;
	} /* for */
      memcpy(&ncontrolv[(new_length-1)*3],
	     &(curve->controlv[(curve->length-1)*3]),
	     3*sizeof(double));

      free(newpersec);

    } /* if */

  free(curve->controlv);
  curve->controlv = ncontrolv;

  curve->length = new_length;

 return ay_status;
} /* ay_act_resize */


/* ay_act_revert:
 *  revert an approximating curve
 */
int
ay_act_revert(ay_acurve_object *curve)
{
 int i, j;
 double dtemp;

  if(!curve)
    return AY_ENULL;

  /* revert control */
  j = (curve->length - 1)*3;
  i = 0;
  while(i < j)
    {
      dtemp = curve->controlv[j];
      curve->controlv[j] = curve->controlv[i];
      curve->controlv[i] = dtemp;

      dtemp = curve->controlv[j+1];
      curve->controlv[j+1] = curve->controlv[i+1];
      curve->controlv[i+1] = dtemp;

      dtemp = curve->controlv[j+2];
      curve->controlv[j+2] = curve->controlv[i+2];
      curve->controlv[i+2] = dtemp;

      i+=3;
      j-=3;
    } /* while */

 return AY_OK;
} /* ay_act_revert */


/* ay_act_getpntfromindex:
 *
 *
 */
int
ay_act_getpntfromindex(ay_acurve_object *curve, int index, double **p)
{
 int stride = 3;
 char fname[] = "ay_act_getpntfromindex";

  if(index >= curve->length || index < 0)
    {
      ay_error(AY_ERROR, fname, "index out of range");
      return TCL_OK;
    }

  *p = &(curve->controlv[index*stride]);

 return TCL_OK;
} /* ay_act_getpntfromindex */
