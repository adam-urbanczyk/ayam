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

/* prototypes of functions local to this module: */

int ay_act_svd(int m, int n, int withu, int withv,
	       double eps, double tol,
	       double *a, double *q, double *u, double *v);

int ay_act_multmatrixmn(int m, int n, double *M1, double *M2, double *R);

int ay_act_solve(int m, int n, double *A, double *B, double *X);


/* functions: */

/* ay_act_svd:
 *  singular value decomposition of matrix <a[m][n]> into <q>, <u> and <v>
 *  <withu> and <withv> should be 1, <eps>, and <tol> >= AY_EPSILON
 */
int
ay_act_svd(int m, int n, int withu, int withv, double eps, double tol,
	   double *a, double *q, double *u, double *v)
{
 int i, j, k, l = 0, l1, iter, retval;
 double c, f, g, h, s, x, y, z;
 double *e;

  if(!(e = (double *)calloc(n, sizeof(double))))
    return 1;
  retval = 0;

  /* Copy 'a' to 'u' */
  for(i = 0; i < m; i++)
    {
      for(j = 0; j < n; j++)
	{
	  /*u[i][j] = a[i][j];*/
	  u[i*m+j] = a[i*m+j];
	}
    }
  /* Householder's reduction to bidiagonal form. */
  g = x = 0.0;
  for(i = 0; i < n; i++)
    {
      e[i] = g;
      s = 0.0;
      l = i+1;
      for(j = i; j < m; j++)
	{
	  s += (u[j*m+i]*u[j*m+i]);
	}
      if(s < tol)
	{
	  g = 0.0;
	}
      else
	{
	  f = u[i*m+i];
	  g = (f < 0) ? sqrt(s) : -sqrt(s);
	  h = f * g - s;
	  u[i*m+i] = f - g;
	  for(j = l; j < n; j++)
	    {
	      s = 0.0;
	      for(k = i; k < m; k++)
		{
		  s += (u[k*m+i] * u[k*m+j]);
		}
	      f = s / h;
	      for(k = i; k < m; k++)
		{
		  u[k*m+j] += (f * u[k*m+i]);
		}
	    } /* for */
	} /* if */
      q[i] = g;
      s = 0.0;
      for(j = l; j < n; j++)
	{
	  s += (u[i*m+j] * u[i*m+j]);
	}
      if(s < tol)
	{
	  g = 0.0;
	}
      else
	{
	  f = u[i*m+i+1];
	  g = (f < 0) ? sqrt(s) : -sqrt(s);
	  h = f * g - s;
	  u[i*m+i+1] = f - g;
	  for(j=l;j<n;j++)
	    {
	      e[j] = u[i*m+j]/h;
	    }
	  for(j=l;j<m;j++)
	    {
	      s = 0.0;
	      for(k = l; k < n; k++)
		{
		  s += (u[j*m+k] * u[i*m+k]);
		}
	      for(k = l; k < n; k++)
		{
		  u[j*m+k] += (s * e[k]);
		}
	    } /* for */
	} /* if */
      y = fabs(q[i]) + fabs(e[i]);
      if(y > x)
	x = y;
    } /* end i */

  /* accumulation of right-hand transformations */
  if(withv)
    {
      for(i = n-1; i >= 0; i--)
	{
	  if(g != 0.0)
	    {
	      h = u[i*m+i+1] * g;
	      for(j = l; j < n; j++)
		{
		  v[j*m+i] = u[i*m+j]/h;
		}
	      for(j = l; j < n; j++)
		{
		  s = 0.0;
		  for(k = l; k < n; k++)
		    {
		      s += (u[i*m+k] * v[k*m+j]);
		    }
		  for(k = l; k < n; k++)
		    {
		      v[k*m+j] += (s * v[k*m+i]);
		    }
		} /* for */
	    } /* if */
	  for(j = l; j < n; j++)
	    {
	      v[i*m+j] = v[j*m+i] = 0.0;
	    }
	  v[i*m+i] = 1.0;
	  g = e[i];
	  l = i;
	} /* for */
    } /* end withv, parens added for clarity */

  /* accumulation of left-hand transformations */
  if(withu)
    {
      for(i = n; i < m; i++)
	{
	  for(j = n; j < m; j++)
	    {
	      u[i*m+j] = 0.0;
	    }
	  u[i*m+i] = 1.0;
	}

      for(i = n-1; i >= 0; i--)
	{
	  l = i + 1;
	  g = q[i];
	  for(j = l; j < m; j++)  /* upper limit was 'n' */
	    {
	      u[i*m+j] = 0.0;
	    }
	  if(g != 0.0)
	    {
	      h = u[i*m+i] * g;
	      for(j = l; j < m; j++) /* upper limit was 'n' */
		{
		  s = 0.0;
		  for(k = l; k < m; k++)
		    {
		      s += (u[k*m+i] * u[k*m+j]);
		    }
		  f = s / h;
		  for(k = i; k < m; k++)
		    {
		      u[k*m+j] += (f * u[k*m+i]);
		    }
		} /* end j */
	      for(j = i; j < m; j++)
		{
		  u[j*m+i] /= g;
		}
	    } /* end g */
	  else
	    {
	      for(j = i; j < m; j++)
		{
		  u[j*m+i] = 0.0;
		}
	    }
	  u[i*m+i] += 1.0;
	} /* end i*/
    } /* end withu, parens added for clarity */

  /* diagonalization of the bidiagonal form */
  eps *= x;
  for(k = n-1; k >= 0; k--)
    {
      iter = 0;
test_f_splitting:
      for(l = k; l >= 0; l--)
	{
	  if(fabs(e[l]) <= eps) goto test_f_convergence;
	  if(fabs(q[l-1]) <= eps) goto cancellation;
	} /* end l */

      /* cancellation of e[l] if l > 0 */
cancellation:
      c = 0.0;
      s = 1.0;
      l1 = l - 1;
      for(i = l; i <= k; i++)
	{
	  f = s * e[i];
	  e[i] *= c;
	  if(fabs(f) <= eps) goto test_f_convergence;
	  g = q[i];
	  h = q[i] = sqrt(f*f + g*g);
	  c = g / h;
	  s = -f / h;
	  if(withu)
	    {
	      for(j = 0; j < m; j++)
		{
		  y = u[j*m+l1];
		  z = u[j*m+i];
		  u[j*m+l1] = y * c + z * s;
		  u[j*m+i] = -y * s + z * c;
		} /* end j */
	    } /* end withu, parens added for clarity */
	} /* end i */
test_f_convergence:
      z = q[k];
      if(l == k) goto convergence;

      /* shift from bottom 2x2 minor */
      iter++;
      if(iter > 30) {
	retval = k;
	break;
      }
      x = q[l];
      y = q[k-1];
      g = e[k-1];
      h = e[k];
      f = ((y-z)*(y+z) + (g-h)*(g+h)) / (2*h*y);
      g = sqrt(f*f + 1.0);
      f = ((x-z)*(x+z) + h*(y/((f<0)?(f-g):(f+g))-h))/x;
      /* next QR transformation */
      c = s = 1.0;
      for(i = l+1; i <= k; i++)
	{
	  g = e[i];
	  y = q[i];
	  h = s * g;
	  g *= c;
	  e[i-1] = z = sqrt(f*f+h*h);
	  c = f / z;
	  s = h / z;
	  f = x * c + g * s;
	  g = -x * s + g * c;
	  h = y * s;
	  y *= c;
	  if(withv)
	    {
	      for(j = 0; j < n; j++)
		{
		  x = v[j*m+i-1];
		  z = v[j*m+i];
		  v[j*m+i-1] = x * c + z * s;
		  v[j*m+i] = -x * s + z * c;
		} /* end j */
	    } /* end withv, parens added for clarity */
	  q[i-1] = z = sqrt(f*f + h*h);
	  c = f/z;
	  s = h/z;
	  f = c * g + s * y;
	  x = -s * g + c * y;
	  if(withu)
	    {
	      for(j = 0; j < m; j++)
		{
		  y = u[j*m+i-1];
		  z = u[j*m+i];
		  u[j*m+i-1] = y * c + z * s;
		  u[j*m+i] = -y * s + z * c;
		} /* end j */
	    } /* end withu, parens added for clarity */
	} /* end i */
      e[l] = 0.0;
      e[k] = f;
      q[k] = x;
      goto test_f_splitting;
convergence:
      if(z < 0.0)
	{
	  /* q[k] is made non-negative */
	  q[k] = - z;
	  if(withv)
	    {
	      for(j = 0; j < n; j++)
		v[j*m+k] = -v[j*m+k];
	    } /* end withv, parens added for clarity */
	} /* end z */
    } /* end k */

  free(e);

 return retval;
} /* ay_act_svd */


/* ay_act_multmatrixmn:
 *  multiply two compatible rectangular matrices <M1[m][n]> and <M2[n][m]>,
 *  write result to <R[n][n]> (allocated outside!)
 *  compatibility will not be checked
 */
int
ay_act_multmatrixmn(int m, int n, double *M1, double *M2, double *R)
{
 int i, j, k;
 int a, b, c = 0;
 double t;

  if(!M1 || !M2 || !R)
    return AY_ENULL;

  /* M1 is mxn, M2 is nxm; R will be nxn */

  /* i shall be column number for R (and M1) */
  for(i = 0; i < n; i++)
    {
      /* j shall be row number for R (and M2) */
      for(j = 0; j < n; j++)
	{
	  /* arrange to access row j in M1 */
	  a = j;
	  /* arrange to access column i in M2 */
	  b = i*m;
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
	  /* forward one row in R */
	  c++;
	} /* for */
    } /* for */

 return AY_OK;
} /* ay_act_multmatrixmn */


/* ay_act_solve:
 *  solve the linear equation system: A*X=B
 *  (A[m*n], X[n*stride], B[n*stride], m >= n)
 *  will change A! X allocated outside!
 */
int
ay_act_solve(int m, int n, double *A, double *B, double *X)
{
 int ay_status = AY_OK, svd_status = 0;
 int stride = 3, i, j, k, l;
 double t, *q = NULL, *u = NULL, *v = NULL, *tmp = NULL;
 int *pivot = NULL;

  if(!A || !B || !X)
    return AY_ENULL;

  if(m == n)
    {
      /* A is square => use LU decomposition to solve */
      if(!(pivot = calloc(n, sizeof(int))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      ay_status = ay_nb_LUDecompose(n, A, pivot);

      if(ay_status)
	{ goto cleanup; }

      ay_status = ay_nb_LUInvert(n, A, pivot);

      if(ay_status)
	{ goto cleanup; }

      /* multiply through: X = A(^-1)*B, and store results */
      for(i = 0; i < n; i++)
	{
	  for(j = 0; j < stride; j++)
	    {
	      t = 0.0;
	      for(k = 0; k < n; k++)
		{
		  t += (A[i*n+k] * B[k*stride+j]);
		}
	      X[i*stride+j] = t;
	    }
	}
    }
  else
    {
      /* A is not square => use SV decomposition to solve */
      if(!(q = calloc(m, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      if(!(u = calloc(m*m, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      if(!(v = calloc(m*m, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      if(!(tmp = (double *)calloc(m, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      svd_status = ay_act_svd(m, n, 1, 1,
			      1.5/*AY_EPSILON*/, /*0.25*/AY_EPSILON,
			      A, q, u, v);

      if(svd_status)
	{ ay_status = AY_ERROR; goto cleanup; }

      for(l = 0; l < stride; l++)
	{
	  for(i = 0; i < n; i++)
	    {
	      tmp[i] = 0.0;
	      /* XXXX ?Use different constant/tol? */
	      if(fabs(q[i]) > AY_EPSILON)
		{
		  for(j = 0; j < m; j++)
		    {
		      tmp[i] += (u[j*m+i] * B[i*stride+l]); /* U'.b */
		    }
		  tmp[i] /= q[i]; /* (1/q).U'.b */
		}
	      /* XXXX ?report error? */
	      /*
	      else
		{
		 ay_status = AY_ERROR; goto cleanup;
		}
	      */
	    } /* for */
	  for(i = 0; i < n; i++)
	    {
	      X[i*stride+l] = 0.0;
	      for(j = 0; j < n; j++)
		{
		  X[i*stride+l] += (v[i*m+j] * tmp[j]); /* V.(1/q).U'.b */
		}
	    }
	} /* for */
    } /* if */

cleanup:

  if(pivot)
    free(pivot);

  if(q)
    free(q);

  if(u)
    free(u);

  if(v)
    free(v);

  if(tmp)
    free(tmp);

 return ay_status;
} /* ay_act_solve */


/* ay_act_leastSquares:
 *  approximate the data points in <Q[m>] with a NURBS curve of degree <p>
 *  with <n> control points, return results in <U> and <P>
 */
int
ay_act_leastSquares(double *Q, int m, int n, int p, double **U, double **P)
{
 int ay_status = AY_OK;
 int a, i, i2, j, k, istride = 3, ostride = 4, span;
 double da, d, *ub = NULL;
 double *Ns = NULL, *Nt = NULL, *NN = NULL;
 double *R = NULL, *rk = NULL, *N = NULL, *X = NULL;
 double *funs = NULL;

  if(!Q || !U || !P)
    return AY_ENULL;

  if(n > m)
    return AY_ERROR;

  ay_status = ay_knots_chordparam(Q, m/*Qlen*/, istride, &ub);

  if(ay_status || !ub)
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

  /* create clamped knot vector by averaging */

  for(i = 0; i < p+1; i++)
    {
      (*U)[i] = 0.0;
    }

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

  /* Note well: N is nxm! (not mxn, as suggested by the NURBS book) */
  if(!(N = calloc(n*m, sizeof(double))))
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
	  a = (span-p+j)*m+i;
	  N[a] = funs[j];
	  /*
	  if(isnan(N[a]))
	  {
	    ay_status = AY_ERROR; goto cleanup;
	  }
	  */
	}

      /*rk[i] = Q[i]-N(0,i)*Q[0]-N(n-1,i)*Q[m-1];*/
      rk[i*istride]   = Q[i*istride]   - N[i] * Q[0] -
	N[(n-1)*m+i] * Q[(m-1)*istride];

      rk[i*istride+1] = Q[i*istride+1] - N[i] * Q[1] -
	N[(n-1)*m+i] * Q[(m-1)*istride+1];

      rk[i*istride+2] = Q[i*istride+2] - N[i] * Q[2] -
	N[(n-1)*m+i] * Q[(m-1)*istride+2];
    } /* for */

  /* set up R */
  for(i = 0; i < n; i++)
    {
      /*R[i*istride] = 0.0;*/
      memset(&(R[i*istride]), 0, istride*sizeof(double));

      for(j = 0; j < m; j++)
	{
	  /*R[i] += N(i,j)*rk[j] ;*/
	  R[i*istride]   += N[i*m+j]*rk[j*istride];
	  R[i*istride+1] += N[i*m+j]*rk[j*istride+1];
	  R[i*istride+2] += N[i*m+j]*rk[j*istride+2];
	}
      /*
      if(R[i*istride]   * R[i*istride]   < AY_EPSILON &&
	 R[i*istride+1] * R[i*istride+1] < AY_EPSILON &&
	 R[i*istride+2] * R[i*istride+2] < AY_EPSILON)
	{
	  ay_status = AY_ERROR;
	  goto cleanup;
	}
      */
    } /* for */

  /* solve N^T*N*P = R */

  if(n > 2)
    {
      if(!(Ns = calloc((n-2)*(m-2), sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      if(!(Nt = calloc((m-2)*(n-2), sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      if(!(NN = calloc((n-2)*(n-2), sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      if(!(X = calloc((n-2)*istride, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      /* fill Ns & Nt */
      a = 0;
      for(i = 1; i < n-1; i++)
	{
	  for(j = 1; j < m-1; j++)
	    {
	      Ns[a] = N[i*m+j];
	      a++;
	    }
	}
      a = 0;
      for(i = 1; i < m-1; i++)
	{
	  for(j = 1; j < n-1; j++)
	    {
	      Nt[a] = N[j*m+i];
	      a++;
	    }
	}

      /* do NN=N^T*N */
      ay_act_multmatrixmn(m-2, n-2, Nt, Ns, NN);

      /* solve the linear equation system NN*X=R */
      ay_status = ay_act_solve(n-2, n-2, NN, &(R[istride]), X);

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

 return ay_status;
} /* ay_act_leastSquares */


/* ay_act_leastSquaresClosed:
 *  approximate the data points in <Q[<m>]> with a periodic NURBS curve of
 *  degree <p> with <n> control points, return results in <U> and <P>
 */
int
ay_act_leastSquaresClosed(double *Q, int m, int n, int p,
			  double **U, double **P)
{
 int ay_status = AY_OK;
 int a, i, j, istride = 3, ostride = 4, span;
 double d, *ub = NULL;
 double *Nt = NULL, *NN = NULL;
 double *R = NULL, *N = NULL, *X = NULL;
 double *funs = NULL, alpha;

  if(!Q || !U || !P)
    return AY_ENULL;

  if(n > m+p)
    {
      return AY_ERROR;
    }

  ay_status = ay_knots_chordparam(Q, m/*Qlen*/, istride, &ub);

  if(ay_status || !ub)
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

  /* create unclamped (periodic) knot vector by knot averaging */

  /* adapted from NURBS++ */

  d = (m+1) / (double)(n-p);
  for(j = 1; j < n-p; j++)
    {
      i = (int)(j*d);
      alpha = j*d-i;
      /*      (*U)[p+j] = (1-alpha)*ub[(i-1)%(n+p)] + alpha*ub[(i)%(n+p)];*/
      (*U)[p+j] = (1-alpha)*ub[(i-1)%(m)] + alpha*ub[(i)%(m)];
    }

  for(i = 0; i < p; i++)
    {
      (*U)[i] = (*U)[i+(n-p)] - 1;
    }

  for(i = n; i < n+p+1; i++)
    {
      (*U)[i] = 1 + (*U)[i-(n-p)];
    }

  /* the following section operates on a reduced number of output points
     since the last p output points will be equal to the first p output
     points anyway */
  n -= p;

  if(!(R = calloc(n*istride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  /* Note well: N is nxm! (not mxn, as suggested by the NURBS book) */
  if(!(N = calloc(n*m, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(funs = calloc(p+1, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  /* set up N */
  for(i = 0; i < m; i++)
    {
      span = ay_nb_FindSpan(n, p, ub[i], &((*U)[p-1])/**U*/);

      /* protect BasisFuns() from bad spans */
      if(span >= n)
	span = n-1;

      memset(funs, 0, (p+1)*sizeof(double));
      ay_nb_BasisFuns(span, ub[i], p, &((*U)[p-1])/**U*/, funs);

      for(j = 0; j <= p; j++)
	{
	  a = (span-p+j)*m+i;
	  N[a] = funs[j];
	  /*
	  if(isnan(N[a]))
	  {
	    ay_status = AY_ERROR; goto cleanup;
	  }
	  */
	}
    } /* for */

  /* set up R */
  for(i = 0; i < n; i++)
    {
      /*R[i*istride] = 0.0;*/
      memset(&(R[i*istride]), 0, istride*sizeof(double));

      for(j = 0; j < m; j++)
	{
	  /*R[i] += N(i,j)*Q[j] ;*/
	  R[i*istride]   += N[i*m+j]*Q[j*istride];
	  R[i*istride+1] += N[i*m+j]*Q[j*istride+1];
	  R[i*istride+2] += N[i*m+j]*Q[j*istride+2];
	}
      /*
      if(R[i*istride]   * R[i*istride]   < AY_EPSILON &&
	 R[i*istride+1] * R[i*istride+1] < AY_EPSILON &&
	 R[i*istride+2] * R[i*istride+2] < AY_EPSILON)
	{
	  ay_status = AY_ERROR;
	  goto cleanup;
	}
      */
    } /* for */

  /* solve N^T*N*P = R */

  if(!(Nt = calloc(m*n, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(NN = calloc(n*n, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  /* fill Nt */
  a = 0;
  for(i = 0; i < m; i++)
    {
      for(j = 0; j < n; j++)
	{
	  Nt[a] = N[j*m+i];
	  a++;
	}
    }

  /* do NN=N^T*N */
  ay_act_multmatrixmn(m, n, Nt, N, NN);

  if(!(X = calloc(n*istride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  /* solve the linear equation system NN*X=R */
  ay_status = ay_act_solve(n, n, NN, R, X);

  if(ay_status)
    { goto cleanup; }

  /* save results from X */
  j = 0;
  for(i = 0; i < n; i++)
    {
      memcpy(&((*P)[i*ostride]), &(X[j*istride]), istride*sizeof(double));
      j++;
    }

  /* for the final operations we increase n again to the full output length */
  n += p;

  /* copy the periodic points */
  memcpy(&((*P)[(n-p)*ostride]), *P, p*ostride*sizeof(double));

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

  if(N)
    free(N);

  if(funs)
    free(funs);

  if(Nt)
    free(Nt);

  if(NN)
    free(NN);

  if(X)
    free(X);

 return ay_status;
} /* ay_act_leastSquaresClosed */


/** ay_ict_resize:
 * Resize an approximating curve.
 *
 * \param[in,out] curve approximating curve to revert
 * \param[in] new_length desired new length
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_act_resize(ay_acurve_object *curve, int new_length)
{
 int ay_status = AY_OK;
 int stride = 3, a, b, i, j;
 int *newpersec = NULL, new = 0;
 double *ncontrolv = NULL, v[3] = {0}, t = 0.0, *cv = NULL;

  if(new_length == curve->length)
    return ay_status;

  if(!(ncontrolv = calloc(stride*new_length, sizeof(double))))
    return AY_EOMEM;

  if(new_length < curve->length)
    {
      a = 0;
      for(i = 0; i < new_length; i++)
	{
	  memcpy(&ncontrolv[a], &(curve->controlv[a]), stride*sizeof(double));
	  a+=stride;
	}
    }
  else
    {
      /* distribute new points */
      new = new_length - curve->length;

      if(!(newpersec = calloc((curve->length-1), sizeof(int))))
	{free(ncontrolv); return AY_EOMEM;}
      cv = curve->controlv;

      while(new)
	{
	  for(i = 0; i < (curve->length-1); i++)
	    {
	      if((cv[i*stride]   != cv[(i+1)*stride]) ||
		 (cv[i*stride+1] != cv[(i+1)*stride+1]) ||
		 (cv[i*stride+2] != cv[(i+1)*stride+2]))
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
	  memcpy(&ncontrolv[b], &(curve->controlv[a]), stride*sizeof(double));
	  b+=stride;

	  if((cv[i*stride] != cv[(i+1)*stride]) ||
	     (cv[i*stride+1] != cv[(i+1)*stride+1]) ||
	     (cv[i*stride+2] != cv[(i+1)*stride+2]))
	    {
	      for(j = 1; j <= newpersec[i]; j++)
		{

		  v[0] = curve->controlv[a+stride]   - curve->controlv[a];
		  v[1] = curve->controlv[a+stride+1] - curve->controlv[a+1];
		  v[2] = curve->controlv[a+stride+2] - curve->controlv[a+2];

		  t = j/(newpersec[i]+1.0);

		  AY_V3SCAL(v,t);

		  ncontrolv[b]   = curve->controlv[a]   + v[0];
		  ncontrolv[b+1] = curve->controlv[a+1] + v[1];
		  ncontrolv[b+2] = curve->controlv[a+2] + v[2];
		  ncontrolv[b+3] = 1.0;

		  b+=stride;
		} /* for */
	    } /* if */

	  a+=stride;
	} /* for */
      memcpy(&ncontrolv[(new_length-1)*stride],
	     &(curve->controlv[(curve->length-1)*stride]),
	     stride*sizeof(double));

      free(newpersec);
    } /* if */

  free(curve->controlv);
  curve->controlv = ncontrolv;

  curve->length = new_length;

 return ay_status;
} /* ay_act_resize */


/** ay_act_revert:
 * Revert an approximating curve.
 *
 * \param[in,out] curve approximating curve to revert
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_act_revert(ay_acurve_object *curve)
{
 int stride = 3, i, j;
 double dtemp;

  if(!curve)
    return AY_ENULL;

  /* revert control */
  j = (curve->length - 1)*stride;
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

      i+=stride;
      j-=stride;
    } /* while */

 return AY_OK;
} /* ay_act_revert */


/** ay_act_getpntfromindex:
 * Get address of a single control point from its indices
 * (performing bounds checking).
 *
 * \param[in] curve approximating curve object to process
 * \param[in] index index of desired control point
 * \param[in,out] p pointer to pointer where to store the resulting address
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_act_getpntfromindex(ay_acurve_object *curve, int index, double **p)
{
 int stride = 3;
 char fname[] = "act_getpntfromindex";

  if(!curve || !p)
    return AY_ENULL;

  if(index >= curve->length || index < 0)
    return ay_error_reportirange(fname, "\"index\"", 0, curve->length-1);

  *p = &(curve->controlv[index*stride]);

 return AY_OK;
} /* ay_act_getpntfromindex */
