/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2007 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
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
	  a = j*n;
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
      if(!(A = calloc(n*n*stride, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      if(!(pivot = calloc(n, sizeof(int))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      memcpy(A, M1, stride*n*n*sizeof(double));

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
 int a, i, j, stride = 3, span;
 double d, *ub = NULL;
 double *Ns = NULL, *Nt = NULL, *NN = NULL;
 double *R = NULL, *rk = NULL, *N = NULL, *X = NULL, *B = NULL;
 double *funs = NULL;
 stride = 3;

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

  if(!(*P = calloc(n*stride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  for(i = 0; i < p+1; i++)
    {
      *U[i] = 0.0;
    }

  d = (m+1) / (double)(n-p+1);
  for(j = 1; j < n-p; j++)
    {
      i = (int)(j*d);
      a = j*d-i;
      *U[p+j] = (1-a)*ub[i-1] + a*ub[i];
    }

  for(i = n; i < n+p+1; i++)
    {
      *U[i] = 1.0;
    }

  if(!(R = calloc(n*stride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(rk = calloc(m*stride, sizeof(double))))
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
      span = ay_nb_FindSpan(m, p, ub[i], *U);
      ay_nb_BasisFuns(span, ub[i], p, *U, funs);
      for(j = 0; j <= p; j++)
	{
	  a = i*n+(span-p+j);
	  N[a] = funs[j] ;
	}

      /*rk[i] = Q[i]-N(i,0)*Q[0]-N(i,n-1)*Q[m-1];*/
      rk[i*stride]   = Q[i*stride]  - N[i*n]*Q[0] -N[i*n+n-1]*Q[(m-1)*stride];
      rk[i*stride+1] = Q[i*stride+1]- N[i*n]*Q[1] -N[i*n+n-1]*Q[(m-1)*stride+1];
      rk[i*stride+2] = Q[i*stride+2]- N[i*n]*Q[2] -N[i*n+n-1]*Q[(m-1)*stride+2];
    } /* for */

  /* set up R */
  for(i = 0; i < n; i++)
    {
      /*R[i*stride] = 0.0;*/
      memset(&(R[i*stride]), 0, stride*sizeof(double));

      for(j = 0; j < m; j++)
	{
	  /*R[i] += N(j,i)*rk[j] ;*/
	  R[i*stride]   += N[j*n+i]*rk[j*stride];
	  R[i*stride+1] += N[j*n+i]*rk[j*stride+1];
	  R[i*stride+2] += N[j*n+i]*rk[j*stride+2];
	}

      if(R[i*stride]*R[i*stride]     < AY_EPSILON &&
	 R[i*stride+1]*R[i*stride+1] < AY_EPSILON &&
	 R[i*stride+2]*R[i*stride+2] < AY_EPSILON)
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
      if(!(X = calloc((n-2)*stride, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      if(!(B = calloc((n-2)*stride, sizeof(double))))
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
      memcpy(B, &(R[stride]), (n-2)*stride*sizeof(double));

      /* do N^T*N */
      ay_act_multmatrixmn(Nt, Ns, m-2, n-2, NN);

      /* solve the linear equation system NN*X=B */
      ay_status = ay_act_solve(NN, B, m, n, X);

      if(ay_status)
	{ goto cleanup; }

      /* save results from X */
      memcpy(&(*P[stride]), X, (n-2)*stride*sizeof(double));

    } /* if */

  /* first and last points are data points */
  /*P[0] = Q[0];*/
  memcpy(*P, Q, stride*sizeof(double));
  /*P[n-1] = Q[m-1];*/
  memcpy(&(*P[(n-1)*stride]), &(Q[(m-1)*stride]), stride*sizeof(double));

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

