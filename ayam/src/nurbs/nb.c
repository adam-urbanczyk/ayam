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

/* nb.c - various NURBS related functions */

/*
 * code adapted from "The NURBS Book" by Les Piegl, Wayne Tiller;
 * code marked NURBS++ derived from (or contains changes
 * to the original NURBS-Book code from) the NURBS++-Library
 * by Philippe Lavoie.
 */


/*
 * ay_nb_LUDecompose: (NURBS++)
 * Although taken from NURBS++, this code is originally
 * from Skip Carters "Kalman library":
 * ftp://usc.edu/pub/C-numanal/kalman.tar.gz ;
 * it does the LU decomposition of the nxn matrix A;
 * pivot[n] has to be allocated outside! and fed into the
 * inversion below.
 */
int
ay_nb_LUDecompose(int n, double *A, int *pivot)
{
 int errval, i, j, k, l, kp1, nm1, sign;
 double t, q;
 double den = 0.0, ten = 0.0;
 double *elem = NULL;

  if(!(elem = calloc(n*n, sizeof(double))))
    return AY_EOMEM;

  memcpy(elem, A, n*n*sizeof(double));

  errval = 0;
  nm1 = n - 1;
  for(k = 0; k < n; k++)
    pivot[k] = k;

  sign = 1;

  if(nm1 >= 1)	/* non-trivial problem */
    {
      for(k = 0; k < nm1; k++)
	{
	  kp1 = k + 1;
	  /* partial pivoting ROW exchanges */
	  /* -- search over column          */

	  ten = fabs(A[k*n+k]);
	  l = k;
	  for(i = kp1; i < n; i++)
	    {
	      den = fabs(A[i*n+k]);
	      if(den > ten)
		{
		  ten = den;
		  l = i;
		}
	    }
	  pivot[k] = l;

	  if(elem[l*n+k] != 0.0)
	    {
	      /* nonsingular pivot found */
	      /* interchange needed */
	      if(l != k)
		{
		  for(i = k; i < n; i++)
		    {
		      t = elem[l*n+i];
		      elem[l*n+i] = elem[k*n+i];
		      elem[k*n+i] = t;
		    }
		  sign = -sign;
		}
	      q =  elem[k*n+k];	/* scale row */
	      for(i = kp1; i < n; i++)
		{
		  t = - elem[i*n+k]/q;
		  elem[i*n+k] = t;
		  for(j = kp1; j < n; j++)
		    elem[i*n+j] += (t * elem[k*n+j]);
		}
	    }
	  else		/* pivot singular */
	    errval = k;
	}

    }

  pivot[nm1] = nm1;
  if(elem[nm1*n+nm1] == 0.0)
    errval = nm1;

  memcpy(A, elem, n*n*sizeof(double));

  free(elem);

  return AY_OK;
} /* ay_nb_LUDecompose */


/*
 * ay_nb_LUInvert: (NURBS++)
 * Although taken from NURBS++, this code is originally
 * from Skip Carters "Kalman library":
 * ftp://usc.edu/pub/C-numanal/kalman.tar.gz ;
 * it does the inversion of the nxn matrix inv;
 * pivot[n] has to be allocated outside! and fed with
 * data by the LU decomposition above.
 */
int
ay_nb_LUInvert(int n, double *inv, int *pivot)
{
 double ten, *work = NULL;
 int i, j, k, l = 0, kb, kp1, nm1, coln;

  if(!(work = calloc(n, sizeof(double))))
    return AY_EOMEM;

  coln = n;

  nm1 = n - 1;

  /* inverse U */
  for(k = 0; k < n; k++)
    {
      ten = 1.0 / inv[k*n+k];
      inv[k*n+k] = ten;
      ten = -ten;
      for(i = 0; i < k; i++)
	inv[i*n+k] *= ten;
      kp1 = k + 1;
      if(nm1 >= kp1)
	{
	  for(j = kp1; j < n; j++)
	    {
	      ten = inv[k*n+j];
	      inv[k*n+j] = 0.0;
	      for(i = 0; i < kp1; i++)
		inv[i*n+j] += (ten * inv[i*n+k]);
	    }
	}
    }

  /* INV(U) * INV(L) */
  if(nm1 >= 1)
    {
      for(kb = 0; kb < nm1; kb++)
	{
	  k = nm1 - kb - 1;
	  kp1 = k + 1;
	  for(i = kp1; i < n; i++)
	    {
	      work[i] = inv[i*n+k];
	      inv[i*n+k] = 0.0;
	    }
	  for(j = kp1; j < n; j++)
	    {
	      ten = work[j];
	      for(i = 0; i < n; i++)
		inv[i*n+k] += (ten * inv[i*n+j]);
	    }
	  l = pivot[k];
	  if(l != k)
	    {
	      for(i = 0; i < n; i++)
		{
		  ten = inv[i*n+k];
		  inv[i*n+k] = inv[i*n+l];
		  inv[i*n+l] = ten;
		}
	    }

	} /* for */

    } /* if(nm >= 1) */

  free(work);

 return AY_OK;
} /* ay_nb_LUInvert */


/*
 * ay_nb_GlobalInterpolation4D: (NURBS++)
 * interpolate the n+1 4D points in Q[] with
 * n+1 precalculated parametric values in ub[]
 * and n+d+1 knots in Uc[] with desired degree d (d <= n!)
 */
int
ay_nb_GlobalInterpolation4D(int n, double *Q, double *ub, double *Uc, int d)
{
 int ay_status = AY_OK;
 int i, j, k, span, ind, *pivot = NULL;
 double t, *A = NULL, *U, *N = NULL;
 double *qq = NULL, *xx = NULL;

  if(!(A = calloc((n+1)*(n+1), sizeof(double))))
    return AY_EOMEM;

  if(!(N = calloc((d+1), sizeof(double))))
    { free(A); return AY_EOMEM; }

  if(!(qq = calloc((n+1)*4, sizeof(double))))
    { free(A); free(N); return AY_EOMEM; }

  if(!(xx = calloc((n+1)*4, sizeof(double))))
    { free(A); free(N); free(qq); return AY_EOMEM; }

  if(!(pivot = calloc(n+1, sizeof(int))))
    { free(A); free(N); free(qq); free(xx); return AY_EOMEM; }

  U = Uc;

  /* Fill A */
  for(i = 1; i < n; i++)
    {
      span = ay_nb_FindSpan(n, d, ub[i], U);
      ay_nb_BasisFuns(span, ub[i], d, U, N);
      for(j = 0; j <= d; j++)
	{
	  ind = (i*(n+1)) + (span-d+j);
	  A[ind] = N[j];
	}
  }

  A[0] = 1.0;
  A[n*(n+1)+n] = 1.0;

  ay_status = ay_nb_LUDecompose(n+1, A, pivot);

  if(ay_status)
    { free(A); free(N); free(qq); free(xx); free(pivot); return ay_status; }

  ay_status = ay_nb_LUInvert(n+1, A, pivot);

  if(ay_status)
    { free(A); free(N); free(qq); free(xx); free(pivot); return ay_status; }

  /* Init matrix for LSE */
  memcpy(qq,Q,(n+1)*4*sizeof(double));

  /* Multiply through: xx = A(^-1)*qq */
  for(i = 0; i < (n+1); i++)
    {
      for(j = 0; j < 4; j++)
	{
	  t = 0.0;
	  for(k = 0; k < (n+1); k++)
	    {
	      t += (A[i*(n+1)+k] * qq[k*4+j]);
	    }
	  xx[i*4+j] = t;
	}
    }

  /* Store the results */
  memcpy(Q,xx,(n+1)*4*sizeof(double));

  free(A);
  free(N);
  free(qq);
  free(xx);
  free(pivot);

 return AY_OK;
} /* ay_nb_GlobalInterpolation4D */


/*
 * ay_nb_GlobalInterpolation4DD:
 * interpolate the n+1 4D points in Q[] with
 * n+1 precalculated parametric values in ub[]
 * and n+d+3 knots in Uc[] with desired degree d (d <= n+2!)
 * and end derivatives D1 (start) and D2 (end)
 * Q has to be of size n+3 and filled sparsely:
 * P[0],,P[1],...,P[n-1],,P[n]!
 */
int
ay_nb_GlobalInterpolation4DD(int n, double *Q, double *ub, double *Uc, int d,
			       double *D1, double *D2)
{
 int ay_status = AY_OK;
 int i, j, k, span, ind, *pivot = NULL;
 double t, *A = NULL, *U, *N = NULL;
 double *qq = NULL, *xx = NULL;


  if(!(A = calloc((n+3)*(n+3), sizeof(double))))
    return AY_EOMEM;

  if(!(N = calloc((d+1), sizeof(double))))
    { free(A); return AY_EOMEM; }

  if(!(qq = calloc((n+3)*4, sizeof(double))))
    { free(A); free(N); return AY_EOMEM; }

  if(!(xx = calloc((n+3)*4, sizeof(double))))
    { free(A); free(N); free(qq); return AY_EOMEM; }

  if(!(pivot = calloc(n+3, sizeof(int))))
    { free(A); free(N); free(qq); free(xx); return AY_EOMEM; }

  U = Uc;

  /* Fill A */
  k = 1;
  for(i = 2; i < n+1; i++)
    {
      span = ay_nb_FindSpan(n+2, d, ub[k], U);
      ay_nb_BasisFuns(span, ub[k], d, U, N);
      for(j = 0; j <= d; j++)
	{
	  ind = (i*(n+3)) + (span-d+j);
	  A[ind] = N[j];
	}
      k++;
  }

  A[0] = 1.0;
  A[(n+3)] = -1.0;
  A[(n+3)+1] = 1.0;

  A[((n+3)*(n+3))-1-(n+3)-1] = -1.0;
  A[((n+3)*(n+3))-1-(n+3)] = 1.0;
  A[((n+3)*(n+3))-1] = 1.0;

  ay_status = ay_nb_LUDecompose(n+3, A, pivot);

  if(ay_status)
    { free(A); free(N); free(qq); free(xx); free(pivot); return ay_status; }

  ay_status = ay_nb_LUInvert(n+3, A, pivot);

  if(ay_status)
    { free(A); free(N); free(qq); free(xx); free(pivot); return ay_status; }

  /* Init matrix for LSE */
  memcpy(qq,Q,(n+3)*4*sizeof(double));

  /* Insert Derivatives */
  qq[4] = /*(U[d+1]/d)**/D1[0];
  qq[5] = /*(U[d+1]/d)**/D1[1];
  qq[6] = /*(U[d+1]/d)**/D1[2];
  qq[7] = 1.0;
  ind = n+2;
  qq[(n+1)*4] = /*((1.0-U[ind])/d)**/D2[0];
  qq[((n+1)*4)+1] = /*((1.0-U[ind])/d)**/D2[1];
  qq[((n+1)*4)+2] = /*((1.0-U[ind])/d)**/D2[2];
  qq[((n+1)*4)+3] = 1.0;


  /* Multiply through: xx = A(^-1)*qq */
  for(i = 0; i < (n+3); i++)
    {
      for(j = 0; j < 4; j++)
	{
	  t = 0.0;
	  for(k = 0; k < (n+3); k++)
	    {
	      t += (A[i*(n+3)+k] * qq[k*4+j]);
	    }
	  xx[i*4+j] = t;
	}
    }

  /* Store the results */
  memcpy(Q,xx,(n+3)*4*sizeof(double));

  j = 3;
  for(i = 0; i < (n+3); i++)
    {
      Q[j] = 1.0;
      j += 4;
    }

  free(A);
  free(N);
  free(qq);
  free(xx);
  free(pivot);

 return AY_OK;
} /* ay_nb_GlobalInterpolation4DD */


/*
 * ay_nb_removeKnot: (NURBS++)
 * remove knot r (with multiplicity s) num times from curve
 * (stride, n, p, U, Pw)
 * result: new controls Qw and knots Ubar (allocated outside!)
 * does not check for curve changes!
 */
int
ay_nb_RemoveKnot(int stride, int n, int p, double *U, double *Pw,
		 int r, int s, int num, double *Ubar, double *Qw)
{
 int i, j, k, ii, jj, off, t;
 int m, ord, fout, last, first;
 double alfi, alfj, *temp = NULL, u;
 int ti, tj, tk;

  m = p+n+2;
  ord = p+1;
  fout = (2*r-s-p)/2; /* first control point out */
  last = r-s;
  first = r-p;

  if(num < 1)
    return AY_ERROR;

  if(!(temp = calloc(stride*(2*p+1), sizeof(double))))
    return AY_EOMEM;

  u = U[r];

  for(t = 0; t < num; t++)
    {
      off = first-1;
      /* temp[0] = Pw[off]; */
      memcpy(&(temp[0]), &(Pw[off*stride]), stride*sizeof(double));
      /* temp[last+1-off] = Pw[last+1]; */
      memcpy(&(temp[(last+1-off)*stride]), &(Pw[(last+1)*stride]),
	     stride*sizeof(double));
      i = first;
      j = last;
      ii = 1;
      jj = last-off;
      while((j - i) > t)
	{
	  alfi = (u - U[i]) / (U[i+ord+t] - U[i]);
	  alfj = (u - U[j-t]) / (U[j+ord] - U[j-t]);

	  /* temp[ii] = (Pw[i] - (1.0 - alfi) * temp[ii-1]) / alfi; */
	  ti = ii*stride;
	  tj = i*stride;
	  tk = (ii-1)*stride;
	  temp[ti]   = (Pw[tj]   - (1.0 - alfi) * temp[tk])   / alfi;
	  temp[ti+1] = (Pw[tj+1] - (1.0 - alfi) * temp[tk+1]) / alfi;
	  temp[ti+2] = (Pw[tj+2] - (1.0 - alfi) * temp[tk+2]) / alfi;
	  if(stride == 4)
	    temp[ti+3] = 1.0;
	  /* temp[jj] = (Pw[j] - alfj * temp[jj+1]) / (1.0-alfj); */
	  ti = jj*stride;
	  tj = j*stride;
	  tk = (jj+1)*stride;
	  temp[ti]   = (Pw[tj]   - alfj * temp[tk])   / (1.0-alfj);
	  temp[ti+1] = (Pw[tj+1] - alfj * temp[tk+1]) / (1.0-alfj);
	  temp[ti+2] = (Pw[tj+2] - alfj * temp[tk+2]) / (1.0-alfj);
	  if(stride == 4)
	    temp[ti+3] = 1.0;

	  i++;
	  ii++;
	  j--;
	  jj--;
	} /* while */

      i = first;
      j = last;

      while((j-i) > t)
	{
	  /* Pw[i] = temp[i-off]; */
	  memcpy(&(Pw[i*stride]), &(temp[(i-off)*stride]),
		 stride*sizeof(double));

	  /* Pw[j] = temp[j-off]; */
	  memcpy(&(Pw[j*stride]), &(temp[(j-off)*stride]),
		 stride*sizeof(double));

	  i++;
	  j--;
	}

      first--;
      last++;
    } /* for(t = 0; t < num; t++) */

  if(t == 0)
    {
      free(temp); return AY_ERROR;
    }

  for(k = (r+1); k < m; k++) /* Shift knots */
    U[k-t] = U[k];

  j = fout; /* Pj through Pi will be overwritten */
  i = j;

  for(k = 1; k < t; k++)
    {
      if((k%2) == 1)
	{
	  i++;
	}
      else
	{
	  j--;
	}
    }

  for(k = (i+1); k < (n+1); k++) /* Shift */
    {
      /* Pw[j++] = Pw[k]; */
      memcpy(&(Pw[j*stride]), &(Pw[k*stride]), stride*sizeof(double));
      j++;
    }

  /* copy results */
  memcpy(Ubar, U, ((n+p+2)-t)*sizeof(double));
  memcpy(Qw, Pw, ((n+1)-t)*stride*sizeof(double));

  free(temp);

 return AY_OK;
} /* ay_nb_removeKnot */


/*
 * ay_nb_Bin: (NURBS++)
 * Fills the matrix bin with binomial coefficients
 * bin must point to an array of size maxn*maxk
 */
void
ay_nb_Bin(int maxn, int maxk, double *bin)
{
 int n, k, bini, bini2, bini3;

  /* Setup the first line */
  bin[0] = 1.0 ;
  for(k = (maxk-1); k > 0; --k)
    {
      bin[k] = 0.0 ;
    }

  /* Setup the other lines */
  for(n = 0; n < (maxn-1); n++)
    {
      bini = (n+1)*maxk;
      bin[bini] = 1.0;

      for(k = 1; k < maxk; k++)
	{
	  if(n+1 < k)
	    {
	      bini = n*maxk+k;
	      bin[bini] = 0.0;
	    }
	  else
	    {
	      bini = (n+1)*maxk+k;
	      bini2= n*maxk+k;
	      bini3 = n*maxk+(k-1);
	      bin[bini] = bin[bini2] + bin[bini3];
	    }
	}
    }

 return;
} /* ay_nb_Bin */


/*
 * ay_nb_DegreeElevateCurve:
 * Elevates the degree of curve: stride, n, p, U[], Pw[]
 * t times
 * nh: new length, Uh: new knots, Qw: new controls
 * Uh[] and Qw[] should be sized appropriately before elevation
 * and probably _resized_ according to nh after elevation!
 */
int
ay_nb_DegreeElevateCurve(int stride, int n, int p, double *U, double *Pw,
			 int t, int *nh, double *Uh, double *Qw)
{
 int m, ph, ph2, bai, bai2, bal, ki, ki2, kj;
 int i, j, k, mpi, maxit, kind, cind, r, oldr, a, b, mh, mul;
 int s, save, first, last, tr, bi, bi2, lbz, rbz;
 double *bezalfs = NULL, *bpts = NULL, *ebpts = NULL;
 double *Nextbpts = NULL, *alfs = NULL, *bin = NULL;
 double ua, ub, numer, den, alf, bet, gam, inv;

  m = n + p + 1;

  ph = p + t;
  ph2 = ph/2;
  bal = p + 1;

  if(!(bezalfs = calloc((p+t+1)*(p+1), sizeof(double))))
    return AY_EOMEM;

  if(!(bpts = calloc((p+1)*stride, sizeof(double))))
    {free(bezalfs); return AY_EOMEM;}

  if(!(ebpts = calloc((p+t+1)*stride, sizeof(double))))
    {free(bezalfs); free(bpts); return AY_EOMEM;}

  if(!(Nextbpts = calloc((p-1)*stride, sizeof(double))))
    {free(bezalfs); free(bpts); free(ebpts);return AY_EOMEM;}

  if(!(alfs = calloc((p-1), sizeof(double))))
    {
      free(bezalfs); free(bpts); free(ebpts);
      free(Nextbpts); return AY_EOMEM;
    }

  if(!(bin = calloc((ph+1)*(ph2+1), sizeof(double))))
    {
      free(bezalfs); free(bpts); free(ebpts);
      free(Nextbpts); free(alfs); return AY_EOMEM;
    }

  ay_nb_Bin((ph+1), (ph2+1), bin);

  /* Compute Bezier degree elevation coefficients */
  /* bezalfs[0][0] = bezalfs[ph][p] = 1.0; */
  bezalfs[0] = 1.0;
  bai = ph*bal+p;
  bezalfs[bai] = 1.0;

  for(i = 1; i <= ph2; i++)
    {
      bi = (ph2+1)*ph+i;
      inv = 1.0/bin[bi];
      mpi = p<i?p:i;
      maxit = (0>(i-t))?0:(i-t);
      for(j = maxit; j <= mpi; j++)
	{
	  /* bezalfs[i][j] = inv*Bin(p,j)*Bin(t,i-j); */
	  bai = i*bal+j;
	  bi = (ph2+1)*p+j;
	  bi2 = (ph2+1)*t+(i-j);
	  bezalfs[bai] = inv*bin[bi]*bin[bi2];
	}
    }

  for(i = ph2+1; i <= ph-1; i++)
    {
      mpi = p<i?p:i;
      maxit = (0>(i-t))?0:(i-t);
      for(j = maxit; j <= mpi; j++)
	{
	  /* bezalfs[i][j] = bezalfs[ph-i][p-j]; */
	  bai = i*bal+j;
	  bai2 = (ph-i)*bal+(p-j);
	  bezalfs[bai] = bezalfs[bai2];
	}
    }

  mh = ph;
  kind = ph+1;
  r = -1;
  a = p;
  b = p+1;
  cind = 1;
  ua = U[0];

  /* Qw[0] = Pw[0]; */
  memcpy(&(Qw[0]), &(Pw[0]), stride*sizeof(double));

  for(i = 0; i <= ph; i++)
    {
      Uh[i] = ua;
    }
  /* Initialize first Bezier segment */
  for(i = 0; i <= p; i++)
    {
      /* bpts[i] = Pw[i];*/
      memcpy(&(bpts[i*stride]), &(Pw[i*stride]), stride*sizeof(double));
    }

  while(b < m)
    {
      i = b;
      while((b < m) && (U[b] >= U[b+1])) /* was == */
	b++;

      mul = b-i+1;
      mh += mul+t;
      ub = U[b];
      oldr = r;
      r = p-mul;
      /* Insert knot u[b] r times */
      if(oldr > 0)
	lbz = (oldr+2)/2;
      else
	lbz = 1;

      if(r > 0)
	rbz = ph-(r+1)/2;
      else
	rbz = ph;

      if(r > 0)
	{ /* Insert knot to get Bezier segment */
	  numer = ub - ua;
	  for(k = p; k > mul; k--)
	    {
	      alfs[k-mul-1] = numer/(U[a+k]-ua);
	    }
	  for(j = 1; j <= r; j++)
	    {
	      save = r-j;
	      s = mul+j;

	      for(k = p; k >= s; k--)
		{
		  /* bpts[k] = alfs[k-s]*bpts[k] +
		               (1.0-alfs[k-s])*bpts[k-1]; */
		  ki = k*stride;
		  ki2 = (k-1)*stride;

		  bpts[ki]   = alfs[k-s]*bpts[ki] +
		                (1.0-alfs[k-s])*bpts[ki2];
		  bpts[ki+1] = alfs[k-s]*bpts[ki+1] +
		                (1.0-alfs[k-s])*bpts[ki2+1];
		  bpts[ki+2] = alfs[k-s]*bpts[ki+2] +
		                (1.0-alfs[k-s])*bpts[ki2+2];
		  if(stride == 4)
		    bpts[ki+3] = 1.0;

		}

	      /* Nextbpts[save] = bpts[p]; */
	      memcpy(&(Nextbpts[save*stride]), &(bpts[p*stride]),
		     stride*sizeof(double));
	    }

	} /* End of insert knot */

      /* Degree elevate Bezier */
      for(i = lbz; i <= ph; i++)
	{ /* only points lbz,...,ph are used below */

	  /* ebpts[i] = 0.0; */
	  memset(&(ebpts[i*stride]),0,3*sizeof(double));
	  if(stride == 4)
	    ebpts[(i*stride)+3] = 1.0;
	  mpi = p<i?p:i;
	  maxit = (0>(i-t))?0:(i-t);
	  for(j = maxit; j <= mpi; j++)
	    {
	      /* ebpts[i] = ebpts[i] + bezalfs[i][j]*bpts[j]; */
	      bai = i*bal+j;
	      ki = i*stride;
	      ki2 = j*stride;

	      ebpts[ki]   += bezalfs[bai]*bpts[ki2];
	      ebpts[ki+1] += bezalfs[bai]*bpts[ki2+1];
	      ebpts[ki+2] += bezalfs[bai]*bpts[ki2+2];
	      if(stride == 4)
		ebpts[ki+3] = 1.0;
	    }
	} /* end of degree elevate Bezier */

      if( oldr > 1 )
	{ /* Must remove knot u=U[a] oldr times */
	  first = kind-2;
	  last = kind;
	  den = ub-ua;
	  bet = (ub-Uh[kind-1])/den;
	  for(tr = 1; tr < oldr; tr++)
	    { /* Knot removal loop */
	      i = first;
	      j = last;
	      kj = j-kind+1;

	      while((j-i) > tr) /* Loop and compute the new        */
		{                /* control points for removal step */
		  if(i < cind)
		    {
		      alf = (ub-Uh[i])/(ua-Uh[i]);
		      /* Qw[i] = alf*Qw[i] + (1.0-alf)*Qw[i-1]; */

		      ki = i*stride;
		      ki2 = (i-1)*stride;
		      Qw[ki]   = alf*Qw[ki]   + (1.0-alf)*Qw[ki2];
		      Qw[ki+1] = alf*Qw[ki+1] + (1.0-alf)*Qw[ki2+1];
		      Qw[ki+2] = alf*Qw[ki+2] + (1.0-alf)*Qw[ki2+2];
		      if(stride == 4)
			Qw[ki+3] = 1.0;
		    } /* if */
		  if(j >= lbz)
		    {
		      if((j-tr) <= (kind-ph+oldr))
			{
			  gam = (ub-Uh[j-tr])/den;
			  /* ebpts[kj]=gam*ebpts[kj]+(1.0-gam)*ebpts[kj+1]; */
			  ki = kj*stride;
			  ki2 = (kj+1)*stride;

			  ebpts[ki]   = gam*ebpts[ki] +
			                 (1.0-gam)*ebpts[ki2];
			  ebpts[ki+1] = gam*ebpts[ki+1] +
			                 (1.0-gam)*ebpts[ki2+1];
			  ebpts[ki+2] = gam*ebpts[ki+2] +
			                 (1.0-gam)*ebpts[ki2+2];
			  if(stride == 4)
			    ebpts[ki+3] = 1.0;

			}
		      else
			{
			  /* ebpts[kj]=bet*ebpts[kj]+(1.0-bet)*ebpts[kj+1];*/
			  ki = kj*stride;
			  ki2 = (kj+1)*stride;

			  ebpts[ki]   = bet*ebpts[ki] +
			                 (1.0-bet)*ebpts[ki2];
			  ebpts[ki+1] = bet*ebpts[ki+1] +
			                 (1.0-bet)*ebpts[ki2+1];
			  ebpts[ki+2] = bet*ebpts[ki+2] +
			                 (1.0-bet)*ebpts[ki2+2];
			  if(stride == 4)
			    ebpts[ki+3] = 1.0;
			} /* if */

		    }
		  i++;
		  j--;
		  kj--;
		}
	      first--;
	      last++;
	    }

	} /* End of removing knot, u = U[a] */

      if( a != p) /* load the knot ua */
	{
	  for(i = 0; i < (ph-oldr); i++)
	    {
	      Uh[kind] = ua;
	      kind++;
	    }
	}
      for(j = lbz; j <= rbz; j++) /* load ctrl pts into Qw */
	{
	  /* Qw[cind] = ebpts[j] */
	  memcpy(&(Qw[cind*stride]), &(ebpts[j*stride]),
		 stride*sizeof(double));
	  cind++;
	}
      if(b < m)
	{ /* set up for next pass thru loop */
	  for(j = 0; j < r; j++)
	    {
	      /* bpts[j] = Nextbpts[j]; */
	      memcpy(&(bpts[j*stride]), &(Nextbpts[j*stride]),
		     stride*sizeof(double));
	    }
	  for(j = r; j <= p; j++)
	    {
	      /* bpts[j] = Pw[b-p+j]; */
	      memcpy(&(bpts[j*stride]), &(Pw[(b-p+j)*stride]),
		     stride*sizeof(double));
	    }
	  a=b;
	  b++;
	  ua = ub;
	}
      else
	{ /* End knot */
	  for(i = 0; i <= ph; i++)
	    {
	      Uh[kind+i] = ub;
	    }
	}
    } /* while(b<m) */

  *nh = mh-ph/*-1*/;

  free(bezalfs);
  free(bpts);
  free(ebpts);
  free(Nextbpts);
  free(alfs);
  free(bin);

 return AY_OK;
} /* ay_nb_DegreeElevateCurve */


/*
 * ay_nb_SolveTridiagonal:
 * solve tridiagonal equation system for
 * interpolating C2 cubic splines
 * n, Q[]: points to interpolate
 * U: knot vector
 * P: controls for the spline, of size n+2!
 * P must already contain P[0], P[1], P[n+1] and P[n+2]
 * (precalculated using ends condition)
 */
int
ay_nb_SolveTridiagonal(int n, double *Q, double *U, double *P)
{
 int ay_status = AY_OK;
 double *abc = NULL, *R = NULL, *dd = NULL;
 double den;
 int i, a;

  if(!(abc = calloc(4, sizeof(double))))
    return AY_EOMEM;

  if(!(R = calloc((n+1)*3, sizeof(double))))
    return AY_EOMEM;

  if(!(dd = calloc(n+1, sizeof(double))))
    return AY_EOMEM;

  a = 9;
  for(i = 3; i < n; i++)
    {
      R[a]   = Q[a-3];
      R[a+1] = Q[a-2];
      R[a+2] = Q[a-1];
      a += 3;
    }

  ay_nb_BasisFuns(4,U[4],3,U, abc);

  den = abc[1];

  /* P[2] */
  P[6] = (Q[3]-abc[0]*P[3])/den;
  P[7] = (Q[4]-abc[0]*P[4])/den;
  P[8] = (Q[5]-abc[0]*P[5])/den;

  a = 9;
  for(i = 3; i < n; i++)
    {
      dd[i] = abc[2]/den;

      ay_nb_BasisFuns(i+2, U[i+2], 3, U, abc);

      den = abc[1] - abc[0] * dd[i];

      P[a]   = (R[a]   - abc[0] * P[a-3])/den;
      P[a+1] = (R[a+1] - abc[0] * P[a-2])/den;
      P[a+2] = (R[a+2] - abc[0] * P[a-1])/den;

      a += 3;
    }

  dd[n] = abc[2]/den;

  ay_nb_BasisFuns(n+2, U[n+2], 3, U, abc);

  den = abc[1] - abc[0]*dd[n];

  a = n*3;

  P[a]   = (Q[a-3]-abc[2]*P[a+3]-abc[0]*P[a-3])/den;
  P[a+1] = (Q[a-2]-abc[2]*P[a+4]-abc[0]*P[a-2])/den;
  P[a+2] = (Q[a-1]-abc[2]*P[a+5]-abc[0]*P[a-1])/den;

  a = (n-1)*3;
  for(i = (n-1); i >= 2; i--)
    {
      P[a]   = P[a]   - dd[i+1]*P[a+3];
      P[a+1] = P[a+1] - dd[i+1]*P[a+4];
      P[a+2] = P[a+2] - dd[i+1]*P[a+5];
      a-=3;
    }

  free(abc);
  free(dd);
  free(R);

 return ay_status;
} /* ay_nb_SolveTridiagonal */


int
ay_nb_CurveInsertKnot4D(int np, int p, double *UP, double *Pw, double u,
			int k, int s, int r, int *nq, double *UQ,
			double *Qw)
{
 int i,j,L=0,i1,i2;
 double mp = 0.0, alpha = 0.0, *Rw = NULL;

  mp = np+p+1;
  *nq = np + r;

  if(!(Rw = calloc((p+1)*4,sizeof(double))))
    return AY_EOMEM;

  /* load new knot vector */
  for(i=0; i<=k; i++) UQ[i] = UP[i];
  for(i=1; i<=r; i++) UQ[k+i] = u;
  for(i=k+1; i<=mp; i++) UQ[i+r] = UP[i];

  /* save unaltered control points */
  for(i=0; i <= (k-p); i++)
    {
      i1 = i*4;
      Qw[i1] = Pw[i1];
      Qw[i1+1] = Pw[i1+1];
      Qw[i1+2] = Pw[i1+2];
      Qw[i1+3] = Pw[i1+3];
    }

  for(i=(k-s); i<=np; i++)
    {
      i1 = (i+r)*4;
      i2 = i*4;
      Qw[i1] = Pw[i2];
      Qw[i1+1] = Pw[i2+1];
      Qw[i1+2] = Pw[i2+2];
      Qw[i1+3] = Pw[i2+3];
    }

  for(i=0; i<=(p-s); i++)
    {
      i1 = i*4;
      i2 = (k-p+i)*4;
      Rw[i1] = Pw[i2];
      Rw[i1+1] = Pw[i2+1];
      Rw[i1+2] = Pw[i2+2];
      Rw[i1+3] = Pw[i2+3];
    }
  for(j=1; j<=r; j++)
    {
      L = k-p+j;
      for(i=0; i<=p-j-s; i++)
	{
	  alpha = (u-UP[L+i])/(UP[i+k+1]-UP[L+i]);

	  i1 = (i+1)*4;
	  i2 = (i)*4;
	  Rw[i2] = alpha*Rw[i1] + (1.0-alpha)*Rw[i2];
	  Rw[i2+1] = alpha*Rw[i1+1] + (1.0-alpha)*Rw[i2+1];
	  Rw[i2+2] = alpha*Rw[i1+2] + (1.0-alpha)*Rw[i2+2];
	  Rw[i2+3] = alpha*Rw[i1+3] + (1.0-alpha)*Rw[i2+3];
	}
      i1 = L*4;
      Qw[i1] = Rw[0];
      Qw[i1+1] = Rw[1];
      Qw[i1+2] = Rw[2];
      Qw[i1+3] = Rw[3];

      i1 = (k+r-j-s)*4;
      i2 = (p-j-s)*4;
      Qw[i1] = Rw[i2];
      Qw[i1+1] = Rw[i2+1];
      Qw[i1+2] = Rw[i2+2];
      Qw[i1+3] = Rw[i2+3];

    }

  for(i=L+1; i<k-s; i++)
    {
      i1 = i*4;
      i2 = (i-L)*4;
      Qw[i1] = Rw[i2];
      Qw[i1+1] = Rw[i2+1];
      Qw[i1+2] = Rw[i2+2];
      Qw[i1+3] = Rw[i2+3];
    }

  free(Rw);

 return AY_OK;
} /* ay_nb_CurveInsertKnot4D */


int
ay_nb_CurveInsertKnot3D(int np, int p, double *UP, double *P, double u,
			int k, int s, int r, int *nq, double *UQ,
			double *Q)
{
 int i,j,L=0,i1,i2;
 double mp = 0.0, alpha = 0.0, *R = NULL;

  mp = np+p+1;
  *nq = np + r;

  if(!(R = calloc((p+1)*3,sizeof(double))))
    return AY_EOMEM;

  /* load new knot vector */
  for(i=0; i<=k; i++) UQ[i] = UP[i];
  for(i=1; i<=r; i++) UQ[k+i] = u;
  for(i=k+1; i<=mp; i++) UQ[i+r] = UP[i];

  /* save unaltered control points */
  for(i=0; i <= (k-p); i++)
    {
      i1 = i*3;
      Q[i1] = P[i1];
      Q[i1+1] = P[i1+1];
      Q[i1+2] = P[i1+2];
    }

  for(i=(k-s); i<=np; i++)
    {
      i1 = (i+r)*3;
      i2 = i*3;
      Q[i1] = P[i2];
      Q[i1+1] = P[i2+1];
      Q[i1+2] = P[i2+2];
    }

  for(i=0; i<=(p-s); i++)
    {
      i1 = i*3;
      i2 = (k-p+i)*3;
      R[i1] = P[i2];
      R[i1+1] = P[i2+1];
      R[i1+2] = P[i2+2];
    }

  for(j=1; j<=r; j++)
    {
      L = k-p+j;
      for(i=0; i<=p-j-s; i++)
	{
	  alpha = (u-UP[L+i])/(UP[i+k+1]-UP[L+i]);

	  i1 = (i+1)*3;
	  i2 = (i)*3;
	  R[i2] = alpha*R[i1] + (1.0-alpha)*R[i2];
	  R[i2+1] = alpha*R[i1+1] + (1.0-alpha)*R[i2+1];
	  R[i2+2] = alpha*R[i1+2] + (1.0-alpha)*R[i2+2];
	}
      i1 = L*3;
      Q[i1] = R[0];
      Q[i1+1] = R[1];
      Q[i1+2] = R[2];

      i1 = (k+r-j-s)*3;
      i2 = (p-j-s)*3;
      Q[i1] = R[i2];
      Q[i1+1] = R[i2+1];
      Q[i1+2] = R[i2+2];
    }

  for(i=L+1; i<k-s; i++)
    {
      i1 = i*3;
      i2 = (i-L)*3;
      Q[i1] = R[i2];
      Q[i1+1] = R[i2+1];
      Q[i1+2] = R[i2+2];
    }

  free(R);

 return AY_OK;
} /* ay_nb_CurveInsertKnot3D */


/*
 * ay_nb_FindSpan: (NURBS++)
 * changes to original FindSpan
 * for unclamped splines
 */
int
ay_nb_FindSpan(int n, int p, double u, double *U)
{
 int low, mid, high;

  if(u >= U[n])
    return n;
  /* never return a span 0-p, because this would make
     BasisFuns() creash badly */
  if(u <= U[p])
    return p;

  /* was: */
  /*  if(u == U[n+1]) return(n);*/

  low = 0; /* was: low = p; */
  high = n+1;
  mid = (low+high)/2;

  while(u < U[mid] || u >= U[mid+1])
    {
      if(u < U[mid])
	high = mid;
      else
	low = mid;

      mid = (low+high)/2;
    }

 return(mid);
} /* ay_nb_FindSpan */


/*
 * changes to original FindSpanMult
 * for unclamped knot vectors
 * from NURBS++
 */
int
ay_nb_FindSpanMult(int n, int p, double u, double *U, int *s)
{
 int low, mid, high, l;

  if(u >= U[n])
    {
      mid = n;
    }
  else
    {
      if(u <= U[p])
	{
	  mid = p;
	}
      else
	{
	  low = p;
	  high = n+1;
	  mid = (low+high)/2;

	  while(u < U[mid] || u >= U[mid+1])
	    {
	      if(u < U[mid])
		high = mid;
	      else
		low = mid;

	      mid = (low+high)/2;
	    } /* while */
	} /* if */
    } /* if */

  l = mid;

  while(l >= 0)
    {
      if(U[l] != u)
	break;
      l--;
    }
  *s = mid - l;

 return(mid);
} /* ay_nb_FindSpanMult */


/*
 * ay_nb_BasisFuns:
 *
 */
void
ay_nb_BasisFuns(int i, double u, int p, double *U, double *N)
{
 double *left = NULL, *right = NULL, saved, temp;
 int j, r;

 if(!(left = calloc(p+1,sizeof(double))))
   return;
 if(!(right = calloc(p+1,sizeof(double))))
   {free(left); return;}

  N[0] = 1.0;

  for(j=1; j<=p; j++)
    {
      left[j] = u-U[i+1-j];
      right[j] = U[i+j]-u;
      saved = 0.0;

      for(r=0; r<j; r++)
	{
	  temp = N[r]/(right[r+1]+left[j-r]);
	  N[r] = saved+right[r+1]*temp;
	  saved = left[j-r]*temp;
	}

      N[j] = saved;
    }

  free(left);
  free(right);

 return;
} /* ay_nb_BasisFuns */


int
ay_nb_CurvePoint4D(int n, int p, double *U, double *Pw, double u, double *C)
{
 int span, j, k;
 double *N = NULL, Cw[4] = {0};

 if(!(N = calloc(p+1,sizeof(double))))
   return AY_EOMEM;

  span = ay_nb_FindSpan(n,p,u,U);

  ay_nb_BasisFuns(span,u,p,U,N);

  for(j=0; j<=p; j++)
    {

      k = (span-p+j)*4;
      Cw[0] = Cw[0] + N[j]*Pw[k];
      Cw[1] = Cw[1] + N[j]*Pw[k+1];
      Cw[2] = Cw[2] + N[j]*Pw[k+2];
      Cw[3] = Cw[3] + N[j]*Pw[k+3];
    }

  for(j=0;j<4;j++)
    C[j] = Cw[j]/Cw[3];

  free(N);

 return AY_OK;
} /* ay_nb_CurvePoint4D */


int
ay_nb_CurvePoint3D(int n, int p, double *U, double *P, double u, double *C)
{
 int span, j, k;
 double *N = NULL;

 if(!(N = calloc(p+1,sizeof(double))))
   return AY_EOMEM;

  span = ay_nb_FindSpan(n,p,u,U);

  ay_nb_BasisFuns(span,u,p,U,N);

  C[0] = 0.0;
  C[1] = 0.0;
  C[2] = 0.0;
  for(j=0; j<=p; j++)
    {
      k = (span-p+j)*3;
      C[0] = C[0] + N[j]*P[k];
      C[1] = C[1] + N[j]*P[k+1];
      C[2] = C[2] + N[j]*P[k+2];
    }

  free(N);

 return AY_OK;
} /* ay_nb_CurvePoint3D */


int
ay_nb_SurfacePoint4D(int n, int m, int p, int q, double *U, double *V,
		       double *Pw, double u, double v, double *C)
{
 int spanu = 0, spanv = 0, indu = 0, indv = 0, l = 0, k = 0, i = 0, j = 0;
 double *Nu = NULL, *Nv = NULL, Cw[4] = {0}, temp[4] = {0};

  if(!(Nu = calloc(p+1,sizeof(double))))
    return AY_EOMEM;
  if(!(Nv = calloc(q+1,sizeof(double))))
    return AY_EOMEM;

  spanu = ay_nb_FindSpan(n,p,u,U);
  ay_nb_BasisFuns(spanu,u,p,U,Nu);
  spanv = ay_nb_FindSpan(m,q,v,V);
  ay_nb_BasisFuns(spanv,v,q,V,Nv);

  indu = spanu - p;
  for(l=0; l<=q; l++)
    {
      memset(temp, 0, 4*sizeof(double));
      indv = spanv - q + l;

      for(k=0; k<=p; k++)
	{
	  /* was: temp = temp + Nu[k]*Pw[indu+k][indv]; */
	  i = (((indu+k)*(m+1))+indv)*4;

	  temp[0] += Nu[k]*Pw[i];
	  temp[1] += Nu[k]*Pw[i+1];
	  temp[2] += Nu[k]*Pw[i+2];
	  temp[3] += Nu[k]*Pw[i+3];
	}
      /* was: Cw = Cw + Nv[l]*temp */
      Cw[0] += Nv[l]*temp[0];
      Cw[1] += Nv[l]*temp[1];
      Cw[2] += Nv[l]*temp[2];
      Cw[3] += Nv[l]*temp[3];
    }

  for(j = 0; j < 4; j++)
    C[j] = Cw[j]/Cw[3];

  free(Nu);
  free(Nv);

 return AY_OK;
} /* ay_nb_SurfacePoint4D */


int
ay_nb_SurfacePoint3D(int n, int m, int p, int q, double *U, double *V,
		     double *P, double u, double v, double *C)
{
 int spanu = 0, spanv = 0, indu = 0, indv = 0, l = 0, k = 0, i = 0;
 double *Nu = NULL, *Nv = NULL, temp[3] = {0};

  if(!(Nu = calloc(p+1,sizeof(double))))
    return AY_EOMEM;
  if(!(Nv = calloc(q+1,sizeof(double))))
    return AY_EOMEM;

  spanu = ay_nb_FindSpan(n,p,u,U);
  ay_nb_BasisFuns(spanu,u,p,U,Nu);
  spanv = ay_nb_FindSpan(m,q,v,V);
  ay_nb_BasisFuns(spanv,v,q,V,Nv);

  memset(C, 0, 3*sizeof(double));
  C[0] = 0.0;
  C[1] = 0.0;
  C[2] = 0.0;

  indu = spanu - p;
  for(l=0; l<=q; l++)
    {
      memset(temp, 0, 3*sizeof(double));

      indv = spanv - q + l;

      for(k=0; k<=p; k++)
	{
	  /* was: temp = temp + Nu[k]*P[indu+k][indv]; */
	  i = (((indu+k)*(m+1))+indv)*3;
	  temp[0] += Nu[k]*P[i];
	  temp[1] += Nu[k]*P[i+1];
	  temp[2] += Nu[k]*P[i+2];
	}
      /* was: C = C + Nv[l]*temp */
      C[0] += Nv[l]*temp[0];
      C[1] += Nv[l]*temp[1];
      C[2] += Nv[l]*temp[2];
    }

  free(Nu);
  free(Nv);

 return AY_OK;
} /* ay_nb_SurfacePoint3D */


void
ay_nb_DersBasisFuns(int i, double u, int p, int n, double *U, double *ders)
{
 double *left = NULL, *right = NULL, *ndu = NULL, *a = NULL, saved, temp, d;
 int j,j1,j2,r,k,rk,pk,s1,s2;

  if(!(left = calloc(p+1,sizeof(double))))
    return;
  if(!(right = calloc(p+1,sizeof(double))))
    {free(left); return;}
  if(!(ndu = calloc((p+1)*(p+1), sizeof(double))))
    {free(left); free(right); return;}
  if(!(a = calloc(2*(p+1), sizeof(double))))
    {free(left); free(right); free(ndu); return;}

  ndu[0] = 1.0;
  for(j=1;j<=p;j++)
    {
      left[j] = u-U[i+1-j];
      right[j] = U[i+j]-u;
      saved = 0.0;
      for(r=0;r<j;r++)
	{
	  ndu[(j)*(p+1) + r] = right[r+1]+left[j-r];
	  temp = ndu[r*(p+1) + (j-1)]/ndu[j*(p+1) + r];

	  ndu[r*(p+1) + j] = saved+right[r+1]*temp;
	  saved = left[j-r]*temp;
	}
      ndu[j*(p+1)+j] = saved;
    }

  for(j=0;j<=p;j++)
    ders[j] = ndu[j*(p+1) + p];

  for(r=0;r<=p;r++)
    {
      s1 = 0; s2 = 1;
      a[0] = 1.0;
      for(k=1;k<=n;k++)
	{
	  d = 0.0;
	  rk = r-k; pk = p-k;
	  if(r >= k)
	    {
	      a[s2*(p+1)] = a[s1*(p+1)]/ndu[(pk+1)*(p+1) + rk];
	      d = a[s2*(p+1)]*ndu[rk*(p+1) + pk];
	    }

	  if(rk >= -1)
	    j1 = 1;
	  else
	    j1 = -rk;

	  if(r-1 <= pk)
	    j2 = k-1;
	  else
	    j2 = p-r;

	  for(j=j1;j<=j2;j++)
	    {
	      a[s2*(p+1)+j] = (a[s1*(p+1) + j]-a[s1*(p+1)+(j-1)])/
		ndu[(pk+1)*(p+1) + (rk+j)];
	      d += a[s2*(p+1)+j]*ndu[(rk+j)*(p+1)+pk];
	    }

	  if(r <= pk)
	    {
	      a[s2*(p+1)+k] = -a[s1*(p+1)+(k-1)]/ndu[(pk+1)*(p+1)+r];
	      d += a[s2*(p+1)+k]*ndu[r*(p+1)+pk];
	    }
	  ders[k*(p+1)+r] = d;
	  j = s1; s1 = s2; s2 = j;
	}
    }
  r = p;
  for(k=1;k<=n;k++)
    {
      for(j=0;j<=p;j++)
	ders[k*(p+1)+j] *= r;

      r *= (p-k);
    }

  free(left);
  free(right);
  free(ndu);
  free(a);

 return;
} /* ay_nb_DersBasisFuns */


void
ay_nb_ComputeFirstDer3D(int n, int p, double *U, double *P, double u,
			double *CK)
{
 int span = 0,j,r;
 double *nders = NULL;

  if(!(nders = calloc((p+1)*(p+1), sizeof(double))))
    return;

  span = ay_nb_FindSpan(n,p,u,U);

  ay_nb_DersBasisFuns(span,u,p,1,U,nders);

  CK[0]=0.0;
  CK[1]=0.0;
  CK[2]=0.0;

  for(j=0;j<=p;j++)
    {
      r = (span-p+j)*3;

      CK[0] = CK[0] + nders[(p+1)+j] * P[r];
      CK[1] = CK[1] + nders[(p+1)+j] * P[r+1];
      CK[2] = CK[2] + nders[(p+1)+j] * P[r+2];
    }

  if(nders)
    free(nders);

 return;
} /* ay_nb_ComputeFirstDer3D */


void
ay_nb_ComputeSecDer3D(int n, int p, double *U, double *P, double u,
			double *CK)
{
 int span = 0,j,r;
 double *nders = NULL;

  if(!(nders = calloc((p+1)*(p+1), sizeof(double))))
    return;

  span = ay_nb_FindSpan(n,p,u,U);

  ay_nb_DersBasisFuns(span,u,p,2,U,nders);

  CK[0]=0.0;
  CK[1]=0.0;
  CK[2]=0.0;

  for(j=0;j<=p;j++)
    {
      r = (span-p+j)*3;

      CK[0] = CK[0] + nders[((p+1)*2)+j] * P[r];
      CK[1] = CK[1] + nders[((p+1)*2)+j] * P[r+1];
      CK[2] = CK[2] + nders[((p+1)*2)+j] * P[r+2];
    }

  if(nders)
    free(nders);

 return;
} /* ay_nb_ComputeSecDer3D */


void
ay_nb_ComputeFirstDer4D(int n, int p, double *U, double *Pw, double u,
			double *CK)
{
 int span = 0,j,k;
 double *nders = NULL;

  if(!(nders = calloc((p+1)*(p+1), sizeof(double))))
    return;

  span = ay_nb_FindSpan(n,p,u,U);

  ay_nb_DersBasisFuns(span,u,p,1,U,nders);

  CK[0]=0.0;
  CK[1]=0.0;
  CK[2]=0.0;

  for(j=0;j<=p;j++)
    {
      k = (span-p+j)*4;
      CK[0] = CK[0] + nders[(p+1)+j] * (Pw[k]/Pw[k+3]);
      CK[1] = CK[1] + nders[(p+1)+j] * (Pw[k+1]/Pw[k+3]);
      CK[2] = CK[2] + nders[(p+1)+j] * (Pw[k+2]/Pw[k+3]);
    }

  if(nders)
    free(nders);

 return;
} /* ay_nb_ComputeFirstDer4D */

void
ay_nb_ComputeSecDer4D(int n, int p, double *U, double *Pw, double u,
		      double *CK)
{
 int span = 0,j,k;
 double *nders = NULL;

  if(!(nders = calloc((p+1)*(p+1), sizeof(double))))
    return;

  span = ay_nb_FindSpan(n,p,u,U);

  ay_nb_DersBasisFuns(span,u,p,2,U,nders);

  CK[0]=0.0;
  CK[1]=0.0;
  CK[2]=0.0;

  for(j=0;j<=p;j++)
    {
      k = (span-p+j)*4;
      CK[0] = CK[0] + nders[((p+1)*2)+j] * (Pw[k]/Pw[k+3]);
      CK[1] = CK[1] + nders[((p+1)*2)+j] * (Pw[k+1]/Pw[k+3]);
      CK[2] = CK[2] + nders[((p+1)*2)+j] * (Pw[k+2]/Pw[k+3]);
    }

  if(nders)
    free(nders);

 return;
} /* ay_nb_ComputeSecDer4D */


void
ay_nb_CompFirstDerSurf4D(int n, int m, int p, int q, double *U, double *V,
			 double *Pw, double u, double v, double *C)
{
 int i = 0, j = 0, k = 0, l = 0, h = 0, r = 0, s = 0;
 int uspan = 0, vspan = 0;
 double *Nu = NULL, *Nv = NULL, *temp = NULL, *Ct = NULL, *bin = NULL;
 double w[3] = {0}, w2[3] = {0};

  /* du == 1, dv == 1 */

  if(!(Nu = calloc((p+1)*(p+1), sizeof(double))))
    return;
  if(!(Nv = calloc((q+1)*(q+1), sizeof(double))))
    return;
  if(!(temp = calloc((q+1)*4, sizeof(double))))
    return;
  if(!(Ct = calloc(4*4, sizeof(double))))
    return;
  if(!(bin = calloc(4, sizeof(double))))
    return;

  ay_nb_Bin(2, 2, bin);

  uspan = ay_nb_FindSpan(n,p,u,U);
  ay_nb_DersBasisFuns(uspan,u,p,1,U,Nu);
  vspan = ay_nb_FindSpan(m,q,v,V);
  ay_nb_DersBasisFuns(vspan,v,q,1,V,Nv);

  Ct[0] = 0.0;
  Ct[1] = 0.0;
  Ct[2] = 0.0;
  Ct[3] = 0.0;

  for(k = 0; k <= 1; k++)
    {
      for(s = 0; s <= q; s++)
	{
	  temp[s*4]   = 0.0;
	  temp[s*4+1] = 0.0;
	  temp[s*4+2] = 0.0;
	  temp[s*4+3] = 0.0;

	  for(r = 0; r <= p; r++)
	    {
	      /* was: temp[s] = temp[s] + Nu[k][r]*P[uspan-p+r][vspan-q+s]; */
	      i = (((uspan-p+r)*(m+1))+(vspan-q+s))*4;
	      temp[s*4]   += Nu[(k*(p+1))+r]*Pw[i];
	      temp[s*4+1] += Nu[(k*(p+1))+r]*Pw[i+1];
	      temp[s*4+2] += Nu[(k*(p+1))+r]*Pw[i+2];
	      temp[s*4+3] += Nu[(k*(p+1))+r]*Pw[i+3];
	    }
	}

      for(l = 0; l <= 1; l++)
	{
	  /* was: C[k][l] = 0; */
	  Ct[(k*2+l)*4]   = 0.0;
	  Ct[(k*2+l)*4+1] = 0.0;
	  Ct[(k*2+l)*4+2] = 0.0;
	  Ct[(k*2+l)*4+3] = 0.0;

	  for(s = 0; s <= q; s++)
	    {
	      /* was C[k][l] = C[k][l] + Nv[l][s] * temp[s]; */
	      i = (k*2+l)*4;
	      Ct[i]   += Nv[(l*(q+1))+s] * temp[s*4];
	      Ct[i+1] += Nv[(l*(q+1))+s] * temp[s*4+1];
	      Ct[i+2] += Nv[(l*(q+1))+s] * temp[s*4+2];
	      Ct[i+3] += Nv[(l*(q+1))+s] * temp[s*4+3];
	    } /* for */
	} /* for */
    } /* for */
  /***/
  for(k = 0; k <= 1; k++)
    {
      for(l = 0; l <= 1-k; l++)
	{
	  /* was: w = Ct[k][l]; */
	  i = (k*2+l)*4;
	  w[0] = Ct[i];
	  w[1] = Ct[i+1];
	  w[2] = Ct[i+2];

	  for(j = 1; j <= l; j++)
	    {
	      /* was: w -= bin[l][j]*wders[0][j]*C[k][l-j]; */
	      i = (k*2+(l-j))*3;
	      w[0] -= bin[l*2+j] * Ct[(j*4)+3] * C[i];
	      w[1] -= bin[l*2+j] * Ct[(j*4)+3] * C[i+1];
	      w[2] -= bin[l*2+j] * Ct[(j*4)+3] * C[i+2];
	    } /* for */
	  for(h = 1; h <= k; h++)
	    {
	      /* was: w -= bin[k][h]*wders[h][0]*C[k-h][l]; */
	      i = ((k-h)*2+l)*3;
	      w[0] -= bin[k*2+h] * Ct[(h*2*4)+3] * C[i];
	      w[1] -= bin[k*2+h] * Ct[(h*2*4)+3] * C[i+1];
	      w[2] -= bin[k*2+h] * Ct[(h*2*4)+3] * C[i+2];

	      w2[0] = 0.0;
	      w2[1] = 0.0;
	      w2[2] = 0.0;

	      for(j = 1; j <= l; j++)
		{
		  /* was: w2 += bin[l][j]*wders[h][j]*C[k-h][l-j]; */
		  i = ((k-h)*2+(l-j))*3;
		  w2[0] += bin[l*2+j] * Ct[((h*2+j)*4)+3] * C[i];
		  w2[1] += bin[l*2+j] * Ct[((h*2+j)*4)+3] * C[i+1];
		  w2[2] += bin[l*2+j] * Ct[((h*2+j)*4)+3] * C[i+2];
		} /* for */

	      /* was: w -= bin[k][h]*w2; */
	      w[0] -= bin[k*2+h] * w2[0];
	      w[1] -= bin[k*2+h] * w2[1];
	      w[2] -= bin[k*2+h] * w2[2];

	    } /* for */

	  i = (k*2+l)*3;
	  C[i]   = w[0]/Ct[3];
	  C[i+1] = w[1]/Ct[3];
	  C[i+2] = w[2]/Ct[3];

	} /* for */

    } /* for */


  free(Nu);
  free(Nv);
  free(temp);
  free(Ct);
  free(bin);

 return;
} /* ay_nb_CompFirstDerSurf4D */


void
ay_nb_CompFirstDerSurf3D(int n, int m, int p, int q, double *U, double *V,
			 double *P, double u, double v, double *C)
{
 int i = 0, k = 0, l = 0, r = 0, s = 0;
 int uspan = 0, vspan = 0;
 double *Nu = NULL, *Nv = NULL, *temp = NULL;

  /* du == 1, dv == 1 */

  if(!(Nu = calloc((p+1)*(p+1), sizeof(double))))
    return;
  if(!(Nv = calloc((q+1)*(q+1), sizeof(double))))
    return;
  if(!(temp = calloc((q+1)*3, sizeof(double))))
    return;

  uspan = ay_nb_FindSpan(n,p,u,U);
  ay_nb_DersBasisFuns(uspan,u,p,1,U,Nu);
  vspan = ay_nb_FindSpan(m,q,v,V);
  ay_nb_DersBasisFuns(vspan,v,q,1,V,Nv);

  fprintf(stderr,"%d\n", uspan);
  fprintf(stderr,"%g %g %g %g\n",Nu[0],Nu[1],Nu[2],Nu[3]);

  C[0] = 0.0;
  C[1] = 0.0;
  C[2] = 0.0;

  for(k = 0; k <= 1; k++)
    {
      for(s = 0; s <= q; s++)
	{
	  temp[s*3]   = 0.0;
	  temp[s*3+1] = 0.0;
	  temp[s*3+2] = 0.0;

	  for(r = 0; r <= p; r++)
	    {
	      /* was: temp[s] = temp[s] + Nu[k][r]*P[uspan-p+r][vspan-q+s]; */
	      i = (((uspan-p+r)*(m+1))+(vspan-q+s))*3;
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

  free(Nu);
  free(Nv);
  free(temp);

 return;
} /* ay_nb_CompFirstDerSurf3D */


/*
   To make things easier the following assumptions
   have been made:
   O = {0.0,0.0,0.0};
   X = {1.0,0.0,0.0};
   Y = {0.0,1.0,0.0};
*/
int
ay_nb_CreateNurbsCircle(double r, double ths, double the,
			int *length, double **knotv, double **controlv)
{
 double theta = 0.0, dtheta = 0.0, angle = 0.0, w1 = 0.0;
 double *Pw = NULL, *U = NULL;
 int i, j, index = 0, narcs = 0, n = 0;
 double P0[4] = {0}, P1[4] = {0}, P2[4] = {0};
 double T0[4] = {0}, T2[4] = {0};

  if(the < ths)
    the = 360 - the;

  theta = the - ths;

  if(theta <= 90.0)
    narcs = 1;
  else
    if(theta <= 180.0)
      narcs = 2;
    else
      if(theta <= 270.0)
	narcs = 3;
      else
	narcs = 4;

  dtheta = theta/narcs;
  n = 2 * narcs;                  /* n+1 control points */
  w1 = cos(AY_D2R(dtheta/2.0)); /* dtheta/2 == base angle */

  /* alloc mem for Pw and U */
  if(!(Pw = calloc((n+1)*4, sizeof(double))))
    return AY_EOMEM;

  if(!(U = calloc((n+1)+4, sizeof(double))))
    { free(Pw); return AY_EOMEM; }

  P0[0] = r * cos(AY_D2R(ths));
  P0[1] = r * sin(AY_D2R(ths));
  P0[3] = 1.0;
  T0[0] = -sin(AY_D2R(ths));
  T0[1] = cos(AY_D2R(ths));
  Pw[0] = P0[0];
  Pw[1] = P0[1];
  Pw[3] = 1.0;
  index=0; angle = ths;
  for(i=1; i<=narcs; i++)
    {
      angle += dtheta;
      P2[0] = r * cos(AY_D2R(angle));
      P2[1] = r * sin(AY_D2R(angle));
      P2[3] = 1.0;

      memcpy(&Pw[(index+2)*4],P2,4*sizeof(double));

      T2[0] = -sin(AY_D2R(angle));
      T2[1] = cos(AY_D2R(angle));

      ay_geom_intersectlines3D(P0,T0,P2,T2,P1);

      Pw[(index+1)*4]   = w1*P1[0];
      Pw[((index+1)*4)+1] = w1*P1[1];
      Pw[((index+1)*4)+3] = w1;

      index += 2;
      if(i<index)
	{
	  memcpy(P0,P2,4*sizeof(double));
	  memcpy(T0,T2,2*sizeof(double));
	}
    }

  j = 2*narcs+1;
  for(i=0; i<3; i++)
    {
      U[i] = 0.0;
      U[i+j] = 1.0;
    }
  switch(narcs)
    {
    case 1:
      break;
    case 2:
      U[3] = 0.5;
      U[4] = 0.5;
      break;
    case 3:
      U[3] = 1.0/3.0;
      U[4] = 1.0/3.0;
      U[5] = 2.0/3.0;
      U[6] = 2.0/3.0;
      break;
    case 4:
      U[3] = 0.25;
      U[4] = 0.25;
      U[5] = 0.5;
      U[6] = 0.5;
      U[7] = 0.75;
      U[8] = 0.75;
      break;
    }

  *knotv = U;
  *controlv = Pw;
  *length = n+1;

 return AY_OK;
} /* ay_nb_CreateNurbsCircle */


void
ay_nb_FinduFromPoint(ay_nurbcurve_object *curve, double *point,
		     double guess, int max_try, double e1, double e2,
		     double *u)
{
 double un = 0.0;
 double *U = NULL;
 double *p = point, c[4] = {0}, cd[4] = {0}, cdd[4] = {0};
 double c1[4] = {0}, c2[4] = {0};
 double cp[3], temp, temp2;
 int t = 0;
 int n = curve->length+curve->order;

  *u = guess;
  U  = curve->knotv;

  if( *u < U[0] ) *u = U[0];
  if( *u > U[n-1] ) *u = U[n-1];

  /* XXXX while ( 1 ) ! */
  while(1)
    {
      t++;
      if(t > max_try)
	{
	  return;
	}


      ay_nb_CurvePoint4D(curve->length-1, curve->order-1, curve->knotv,
			 curve->controlv, *u, c);

      ay_nb_ComputeFirstDer4D(curve->length-1, curve->order-1,
			      curve->knotv, curve->controlv, *u, cd);

      ay_nb_ComputeSecDer4D(curve->length-1, curve->order-1,
			    curve->knotv, curve->controlv, *u, cdd);

      AY_V3SUB(cp,c,p)
      AY_V3MUL(c1,cp,cp)
      if( AY_V3LEN(c1) < (e1*e1))
	{
	  return;
	}

      AY_V3SUB(cp,c,p)
      AY_V3MUL(c2,cd,cp)
      c2[0] = fabs(c2[0]);
      c2[1] = fabs(c2[1]);
      c2[2] = fabs(c2[2]);

      if(fabs(cd[0])*fabs(cp[0]))
	c2[0] = fabs(c2[0]) / (fabs(cd[0])*fabs(cp[0]));
      else
	c2[0] = 0.0;
      if(fabs(cd[1])*fabs(cp[1]))
	c2[1] = fabs(c2[1]) / (fabs(cd[1])*fabs(cp[1]));
      else
	c2[1] = 0.0;
      if(fabs(cd[2])*fabs(cp[2]))
	c2[2] = fabs(c2[2]) / (fabs(cd[2])*fabs(cp[2]));
      else
	c2[2] = 0.0;

      if( AY_V3LEN(c2) < e2)
	{
	  return;
	}

      AY_V3MUL(c1,cd,cp)
      temp = AY_V3LEN(c1);

      AY_V3MUL(c1,cd,cd)
      c2[0] = cdd[0]*cp[0]+c1[0];
      c2[1] = cdd[1]*cp[1]+c1[1];
      c2[2] = cdd[2]*cp[2]+c1[2];
      temp2 = AY_V3LEN(c2);

      un = *u - temp/temp2;

      if(un < U[0]) un = U[0];
      if(un > U[n-1]) un = U[n-1];

      cd[0] *= (un - *u);
      cd[1] *= (un - *u);
      cd[2] *= (un - *u);
      AY_V3MUL(c2, cd, cd)
      if(AY_V3LEN(c2) < (e1*e1))
	{
	  return;
	}

      *u = un;

    } /* while 1 */

  /* XXXX never reached anyway... */
  /* return;*/
} /* ay_nb_FinduFromPoint */


/*
 * ay_nb_RefineKnotVectCurve:
 * original algorithm by Boehm and Prautzsch
 * refine the knot vector of the curve
 * (stride, n, p, U, Pw) with the new knots in X[r];
 * results in new knots in Ubar[n+p+r] and new
 * control points in Qw[n+r] both allocated outside!
 */
void
ay_nb_RefineKnotVectCurve(int stride, int n, int p, double *U, double *Pw,
			  double *X, int r, double *Ubar, double *Qw)
{
 int m, a, b;
 int i, j, k, l, ind;
 double alfa, td;
 int ti, tj;

  m = n+p+1;

  a = ay_nb_FindSpan(n, p, X[0], U);
  b = ay_nb_FindSpan(n, p, X[r], U);

  b++;

  for(j = 0; j <= (a-p); j++)
    {
      /* Qw[j] = Pw[j]; */
      memcpy(&(Qw[j*stride]), &(Pw[j*stride]), stride*sizeof(double));
    }

  for(j = b-1; j <= n; j++)
    {
      /* Qw[j+r+1] = Pw[j]; */
      memcpy(&(Qw[(j+r+1)*stride]), &(Pw[(j)*stride]), stride*sizeof(double));
    }

  for(j = 0; j <= a; j++)
    {
      Ubar[j] = U[j];
    }
  for(j = b+p; j <= m; j++)
    {
      Ubar[j+r+1] = U[j];
    }

  i = b+p-1;
  k = b+p+r;

  for(j = r; j >= 0; j--)
    {
      while((X[j] <= U[i]) && (i > a))
	{
	  /* Qw[k-p-1] = Pw[i-p-1]; */
	  memcpy(&(Qw[(k-p-1)*stride]), &(Pw[(i-p-1)*stride]),
		 stride*sizeof(double));

	  Ubar[k] = U[i];
	  k--;
	  i--;
	} /* while */

      /* Qw[k-p-1] = Qw[k-p]; */
      memcpy(&(Qw[(k-p-1)*stride]), &(Qw[(k-p)*stride]),
	     stride*sizeof(double));

      for(l = 1; l <= p; l++)
	{
	  ind = k-p+l;
	  alfa = Ubar[k+l] - X[j];
	  if(fabs(alfa) == 0.0)
	    {
	      /* Qw[ind-1] = Qw[ind]; */
	      memcpy(&(Qw[(ind-1)*stride]), &(Qw[ind*stride]),
		     stride*sizeof(double));

	    }
	  else
	    {
	      /* alfa = alfa / (Ubar[k+l] - U[i-p+l]); */
	      td = (Ubar[k+l] - U[i-p+l]);
	      if(td != 0.0)
		alfa /= td;
	      else
		alfa = 0.0;

	      /* Qw[ind-1] = alfa * Qw[ind-1] + (1.0 - alfa) * Qw[ind]; */
	      ti = (ind-1)*stride;
	      tj = ind*stride;
	      Qw[ti]   = alfa * Qw[ti]   + (1.0 - alfa) * Qw[tj];
	      Qw[ti+1] = alfa * Qw[ti+1] + (1.0 - alfa) * Qw[tj+1];
	      Qw[ti+2] = alfa * Qw[ti+2] + (1.0 - alfa) * Qw[tj+2];
	      if(stride == 4)
		Qw[ti+3] = 1.0;

	    }
	}

      Ubar[k] = X[j];
      k--;
    } /* for */

 return;
} /* ay_nb_RefineKnotVectCurve */

