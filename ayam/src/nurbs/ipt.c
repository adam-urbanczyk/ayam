/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2011 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/** \file ipt.c \brief interpolating surface tools */


/** ay_ipt_getpntfromindex:
 * get address of a single control point from its indices
 * (performing bounds checking)
 *
 * \param[in] patch IPatch object to process
 * \param[in] indexu index of desired control point in U dimension (width)
 * \param[in] indexv index of desired control point in V dimension (height)
 * \param[in,out] p pointer to pointer where to store the resulting address
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_getpntfromindex(ay_ipatch_object *patch, int indexu, int indexv,
		       double **p)
{
 int stride = 3;
 char fname[] = "ipt_getpntfromindex";

  if(!patch || !p)
    return AY_ENULL;

  if(indexu >= patch->width || indexu < 0)
    return ay_error_reportirange(fname, "\"indexu\"", 0, patch->width-1);

  if(indexv >= patch->height || indexv < 0)
    return ay_error_reportirange(fname, "\"indexv\"", 0, patch->height-1);

  *p = &(patch->controlv[(indexu*patch->height+indexv)*stride]);

 return AY_OK;
} /* ay_ipt_getpntfromindex */


/** ay_ipt_swapuv:
 * swap U and V dimensions of a IPatch
 *
 * \param[in,out] ip IPatch object to process
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_swapuv(ay_ipatch_object *ip)
{
 int ay_status = AY_OK;
 int i;
 double d, *dp;

  if(!ip)
    return AY_ENULL;

  ay_status = ay_npt_swaparray(&(ip->controlv), 3, ip->width, ip->height);

  if(ay_status)
    return ay_status;

  i = ip->width;
  ip->width = ip->height;
  ip->height = i;

  i = ip->order_u;
  ip->order_u = ip->order_v;
  ip->order_v = i;

  i = ip->close_u;
  ip->close_u = ip->close_v;
  ip->close_v = i;

  i = ip->ktype_u;
  ip->ktype_u = ip->ktype_v;
  ip->ktype_v = i;

  d = ip->sdlen_u;
  ip->sdlen_u = ip->sdlen_v;
  ip->sdlen_v = d;

  d = ip->edlen_u;
  ip->edlen_u = ip->edlen_v;
  ip->edlen_v = d;

  i = ip->derivs_u;
  ip->derivs_u = ip->derivs_v;
  ip->derivs_v = i;

  dp = ip->sderiv_u;
  ip->sderiv_u = ip->sderiv_v;
  ip->sderiv_v = dp;

  dp = ip->ederiv_u;
  ip->ederiv_u = ip->ederiv_v;
  ip->ederiv_v = dp;

 return ay_status;
} /* ay_ipt_swapuv */


/** ay_ipt_revertu:
 * revert IPatch along U (width)
 *
 * \param[in,out] ip IPatch object to revert
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_revertu(ay_ipatch_object *ip)
{
 int i, j, ii, jj, stride = 3;
 double t[4], *dp;

  if(!ip)
    return AY_ENULL;

  for(i = 0; i < ip->height; i++)
    {
      for(j = 0; j < ip->width/2; j++)
	{
	  ii = (j*ip->height+i)*stride;
	  jj = ((ip->width-1-j)*ip->height+i)*stride;
	  memcpy(t, &(ip->controlv[ii]), stride*sizeof(double));
	  memcpy(&(ip->controlv[ii]), &(ip->controlv[jj]),
		 stride*sizeof(double));
	  memcpy(&(ip->controlv[jj]), t, stride*sizeof(double));
	}
    }

  dp = ip->sderiv_u;
  ip->sderiv_u = ip->ederiv_u;
  ip->ederiv_u = dp;

  if(ip->sderiv_v && ip->ederiv_v)
    {
      for(i = 0; i < ip->width/2; i++)
	{
	  ii = i*stride;
	  jj = (ip->width-1-i)*stride;
	  memcpy(t, &(ip->sderiv_v[ii]), stride*sizeof(double));
	  memcpy(&(ip->sderiv_v[ii]), &(ip->sderiv_v[jj]),
		 stride*sizeof(double));
	  memcpy(&(ip->sderiv_v[jj]), t, stride*sizeof(double));

	  memcpy(t, &(ip->ederiv_v[ii]), stride*sizeof(double));
	  memcpy(&(ip->ederiv_v[ii]), &(ip->ederiv_v[jj]),
		 stride*sizeof(double));
	  memcpy(&(ip->ederiv_v[jj]), t, stride*sizeof(double));
	}
    }

 return AY_OK;
} /* ay_ipt_revertu */


/** ay_ipt_revertv:
 * revert IPatch along V (height)
 *
 * \param[in,out] ip IPatch object to revert
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_revertv(ay_ipatch_object *ip)
{
 int ay_status = AY_OK;
 int i, j, ii, jj, stride = 3;
 double t[4], *dp;

  if(!ip)
    return AY_ENULL;

  for(i = 0; i < ip->width; i++)
    {
      ii = i*ip->height*stride;
      jj = ii + ((ip->height-1)*stride);
      for(j = 0; j < ip->height/2; j++)
	{
	  memcpy(t, &(ip->controlv[ii]), stride*sizeof(double));
	  memcpy(&(ip->controlv[ii]), &(ip->controlv[jj]),
		 stride*sizeof(double));
	  memcpy(&(ip->controlv[jj]), t, stride*sizeof(double));
	  ii += stride;
	  jj -= stride;
	}
    }

  dp = ip->sderiv_v;
  ip->sderiv_v = ip->ederiv_v;
  ip->ederiv_v = dp;

  if(ip->sderiv_u && ip->ederiv_u)
    {
      for(i = 0; i < ip->height/2; i++)
	{
	  ii = i*stride;
	  jj = (ip->height-1-i)*stride;
	  memcpy(t, &(ip->sderiv_u[ii]), stride*sizeof(double));
	  memcpy(&(ip->sderiv_u[ii]), &(ip->sderiv_u[jj]),
		 stride*sizeof(double));
	  memcpy(&(ip->sderiv_u[jj]), t, stride*sizeof(double));

	  memcpy(t, &(ip->ederiv_u[ii]), stride*sizeof(double));
	  memcpy(&(ip->ederiv_u[ii]), &(ip->ederiv_u[jj]),
		 stride*sizeof(double));
	  memcpy(&(ip->ederiv_u[jj]), t, stride*sizeof(double));
	}
    }

 return ay_status;
} /* ay_ipt_revertv */


/** ay_ipt_interpolateu:
 * interpolate NURBS patch along U (width)
 *
 * \param[in,out] np NURBS patch object to interpolate
 * \param[in] order desired interpolation order
 * \param[in] ktype parameterization type (AY_KTCHORDAL,
 *            AY_KTCENTRI, or AY_KTUNIFORM)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_interpolateu(ay_nurbpatch_object *np, int order, int ktype)
{
 int ay_status = AY_OK;
 char fname[] = "ipt_interpolateu";
 int i, k, N, K, stride, ind1, ind2, pu, num;
 double *uk = NULL, *cds = NULL, *Pw = NULL, v[3] = {0};
 double *U = NULL, *Q = NULL, total, d;

  if(!np)
    return AY_ENULL;

  K = np->width;
  N = np->height;
  stride = 4;
  Pw = np->controlv;
  pu = order-1;
  num = np->height;

  if(!(uk = calloc(K, sizeof(double))))
    return AY_EOMEM;

  if(!(cds = calloc(K+1, sizeof(double))))
    {
      free(uk); return AY_EOMEM;
    }

  if(!(U = calloc(K+np->uorder, sizeof(double))))
    {
      free(uk); free(cds); return AY_EOMEM;
    }

  if(!(Q = calloc(K*stride, sizeof(double))))
    {
      free(uk); free(cds); free(U); return AY_EOMEM;
    }

  /* calculate parameterization */
  for(i = 0; i < N; i++)
    {
      ind1 = i*stride;
      ind2 = i*stride;
      total = 0.0;

      for(k = 1; k < K; k++)
	{
	  ind1 += N*stride;

	  v[0] = Pw[ind1]   - Pw[ind2];
	  v[1] = Pw[ind1+1] - Pw[ind2+1];
	  v[2] = Pw[ind1+2] - Pw[ind2+2];

	  if(fabs(v[0]) > AY_EPSILON ||
	     fabs(v[1]) > AY_EPSILON ||
	     fabs(v[2]) > AY_EPSILON)
	    {
	      if(ktype == AY_KTCENTRI)
		{
		  cds[k] = sqrt(AY_V3LEN(v));
		}
	      else
		{
		  if(ktype == AY_KTUNIFORM)
		    {
		      cds[k] = 0.01;
		    }
		  else
		    {
		      cds[k] = AY_V3LEN(v);
		    }
		}
	      total += cds[k];
	    }
	  ind2 += N*stride;
	} /* for */

      if(total < AY_EPSILON)
	{
	  num--;
	}
      else
	{
	  d = 0.0;
	  for(k = 1; k < K; k++)
	    {
	      d += cds[k];
	      uk[k] += d/total;
	    }
	}
    } /* for */

  if(num == 0)
    {
      ay_error(AY_ERROR, fname, "Can not interpolate this patch.");
      free(Q); free(uk); free(cds); free(U); return AY_ERROR;
    }

  uk[0] = 0.0;
  for(k = 1; k < K; k++)
    {
      uk[k] /= num;
    }
  uk[K-1] = 1.0;

  for(i = 1; i < (K-pu); i++)
    {
      U[i+pu] = 0.0;

      for(k = i; k < (i+pu); k++)
	{
	  U[i+pu] += uk[k];
	} /* for */

      U[i+pu] /= pu;
    } /* for */
  for(i = 0; i <= pu; i++)
    U[i] = 0.0;
  for(i = (K/*-pu-1*/); i < (K+pu+1); i++)
    U[i] = 1.0;

  /* interpolate */
  for(i = 0; i < N; i++)
    {
      ind1 = i*stride;
      for(k = 0; k < K; k++)
	{
	  memcpy(&(Q[k*4]), &(Pw[ind1]), stride*sizeof(double));
	  if(stride != 4)
	    Q[k*4+3] = 1.0;
	  ind1 += N*stride;
	} /* for */

      ay_status = ay_nb_GlobalInterpolation4D(K-1, Q, uk, U, pu);

      if(ay_status)
	{ free(cds); free(uk); free(U); free(Q); return ay_status; }

      ind1 = i*stride;
      for(k = 0; k < K; k++)
	{
	  memcpy(&(Pw[ind1]), &(Q[k*stride]), stride*sizeof(double));
	  ind1 += N*stride;
	} /* for */
    } /* for */

  if(np->uknotv)
    free(np->uknotv);
  np->uknotv = U;
  np->uknot_type = AY_KTCUSTOM;
  np->uorder = pu+1;

  free(uk);
  free(cds);
  free(Q);

 return AY_OK;
} /* ay_ipt_interpolateu */


/** ay_ipt_interpolateud:
 * interpolate NURBS patch along U (width) with end derivatives
 *
 * \param[in,out] np NURBS patch object to interpolate
 * \param[in] order desired interpolation order
 * \param[in] ktype parameterization type (AY_KTCHORDAL,
 *            AY_KTCENTRI, or AY_KTUNIFORM)
 * \param[in] dmode derivative calculation mode (0 - automatic, 1 - manual)
 * \param[in] sdlen length of automatically generated start derivatives
 * \param[in] edlen length of automatically generated end derivatives
 * \param[in] sd start derivatives (may be NULL if dmode is 0)
 * \param[in] ed end derivatives (may be NULL if dmode is 0)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_interpolateud(ay_nurbpatch_object *np, int order, int ktype,
		     int dmode, double sdlen, double edlen,
		     double *sd, double *ed)
{
 int ay_status = AY_OK;
 char fname[] = "ipt_interpolateud";
 int i, k, N, K, stride, ind1, ind2, pu, num;
 double *uk = NULL, *cds = NULL, *Pw = NULL, v[3] = {0};
 double *U = NULL, *Qt = NULL, *Q = NULL, total, d;
 double ds[3] = {0}, de[3] = {0};

  if(!np || (dmode && (!sd || !ed)))
    return AY_ENULL;

  K = np->width+2;
  N = np->height;
  stride = 4;
  Pw = np->controlv;
  pu = order-1;
  num = np->height;

  if(!(uk = calloc(np->width, sizeof(double))))
    return AY_EOMEM;

  if(!(cds = calloc(np->width+1, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(U = calloc(K+np->uorder, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(Qt = calloc(K*stride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(Q = calloc(N*K*stride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  /* calculate parameterization */
  for(i = 0; i < N; i++)
    {
      ind1 = i*stride;
      ind2 = i*stride;
      total = 0.0;

      for(k = 1; k < np->width; k++)
	{
	  ind1 += N*stride;

	  v[0] = Pw[ind1]   - Pw[ind2];
	  v[1] = Pw[ind1+1] - Pw[ind2+1];
	  v[2] = Pw[ind1+2] - Pw[ind2+2];

	  if(fabs(v[0]) > AY_EPSILON ||
	     fabs(v[1]) > AY_EPSILON ||
	     fabs(v[2]) > AY_EPSILON)
	    {
	      if(ktype == AY_KTCENTRI)
		{
		  cds[k] = sqrt(AY_V3LEN(v));
		}
	      else
		{
		  if(ktype == AY_KTUNIFORM)
		    {
		      cds[k] = 0.01;
		    }
		  else
		    {
		      cds[k] = AY_V3LEN(v);
		    }
		}
	      total += cds[k];
	    }
	  ind2 += N*stride;
	} /* for */

      if(total < AY_EPSILON)
	{
	  num--;
	}
      else
	{
	  d = 0.0;
	  for(k = 1; k < np->width; k++)
	    {
	      d += cds[k];
	      uk[k] += d/total;
	    }
	}
    } /* for */

  if(num == 0)
    {
      ay_error(AY_ERROR, fname, "Can not interpolate this patch.");
      ay_status = AY_ERROR;
      goto cleanup;
    }

  uk[0] = 0.0;
  for(k = 1; k < np->width; k++)
    {
      uk[k] /= num;
    }
  uk[np->width-1] = 1.0;

  for(i = 0; i < (np->width+2-pu); i++)
    {
      ind1 = i+pu+1;
      U[ind1] = 0.0;
      for(k = i; k < (i+pu); k++)
	{
	  U[ind1] += uk[k];
	} /* for */

      U[ind1] /= pu;
    } /* for */
  for(i = 0; i <= pu; i++)
    U[i] = 0.0;
  for(i = K; i < (K+pu+1); i++)
    U[i] = 1.0;

  /* interpolate */
  for(i = 0; i < N; i++)
    {
      ind1 = i*stride;

      /* set up a sparse control vector */
      /* first point */
      memcpy(Qt, &(Pw[ind1]), stride*sizeof(double));
      ind1 += N*stride;

      /* inner points */
      for(k = 2; k < K-2; k++)
	{
	  memcpy(&(Qt[k*4]), &(Pw[ind1]), stride*sizeof(double));
	  ind1 += N*stride;
	} /* for */

      /* last point */
      memcpy(&(Qt[(K-1)*4]), &(Pw[ind1]), stride*sizeof(double));

      /* derivatives */
      if(dmode)
	{
	  /* manual mode (peruse sd/ed) */
	  ind1 = i*stride;
	  ds[0] = sd[i*3] - Pw[ind1];
	  ds[1] = sd[i*3+1] - Pw[ind1+1];
	  ds[2] = sd[i*3+2] - Pw[ind1+2];

	  ind1 = (((np->width-1)*N)+i)*stride;
	  de[0] = Pw[ind1]   - ed[i*3];
	  de[1] = Pw[ind1+1] - ed[i*3+1];
	  de[2] = Pw[ind1+2] - ed[i*3+2];
	}
      else
	{
	  /* automatic mode (peruse sdlen/edlen) */
	  ind1 = i*stride;
	  ind2 = ind1 + N*stride;

	  ds[0] = Pw[ind2] - Pw[ind1];
	  ds[1] = Pw[ind2+1] - Pw[ind1+1];
	  ds[2] = Pw[ind2+2] - Pw[ind1+2];

	  AY_V3SCAL(ds, sdlen);

	  ind1 = (K-4)*N*stride;
	  ind2 = ind1 + N*stride;

	  de[0] = Pw[ind2] - Pw[ind1];
	  de[1] = Pw[ind2+1] - Pw[ind1+1];
	  de[2] = Pw[ind2+2] - Pw[ind1+2];

	  AY_V3SCAL(de, edlen);
	}

      /* interpolate */
      ay_status = ay_nb_GlobalInterpolation4DD(np->width-1, Qt, uk, U, pu,
					       ds, de);

      if(ay_status)
	{ goto cleanup; }

      /* copy results back */
      ind1 = i*stride;
      for(k = 0; k < K; k++)
	{
	  memcpy(&(Q[ind1]), &(Qt[k*stride]), stride*sizeof(double));
	  ind1 += N*stride;
	} /* for */
    } /* for */

  if(np->uknotv)
    free(np->uknotv);
  np->uknotv = U;
  np->uknot_type = AY_KTCUSTOM;
  np->uorder = pu+1;
  np->width = K;

  free(np->controlv);
  np->controlv = Q;

  /* prevent cleanup code from doing something harmful */
  U = NULL;
  Q = NULL;

cleanup:

  if(uk)
    free(uk);
  if(cds)
    free(cds);
  if(U)
    free(U);
  if(Qt)
    free(Qt);
  if(Q)
    free(Q);

 return ay_status;
} /* ay_ipt_interpolateud */


/** ay_ipt_interpolateudc:
 * interpolate NURBS patch along U (width) with end derivatives
 * creates a closed surface (in U)
 *
 * \param[in,out] np NURBS patch object to interpolate
 * \param[in] order desired interpolation order
 * \param[in] ktype parameterization type (AY_KTCHORDAL,
 *            AY_KTCENTRI, or AY_KTUNIFORM)
 * \param[in] dmode derivative calculation mode (0 - automatic, 1 - manual)
 * \param[in] sdlen length of automatically generated start derivatives
 * \param[in] edlen length of automatically generated end derivatives
 * \param[in] sd start derivatives (may be NULL if dmode is 0)
 * \param[in] ed end derivatives (may be NULL if dmode is 0)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_interpolateudc(ay_nurbpatch_object *np, int order, int ktype,
		      int dmode, double sdlen, double edlen,
		      double *sd, double *ed)
{
 int ay_status = AY_OK;
 char fname[] = "ipt_interpolateudc";
 int i, k, N, K, stride, ind1, ind2, pu, num;
 double *uk = NULL, *cds = NULL, *Pw = NULL, v[3] = {0};
 double *U = NULL, *Qt = NULL, *Q = NULL, total, d;
 double ds[3] = {0}, de[3] = {0};

  if(!np || (dmode && (!sd || !ed)))
    return AY_ENULL;

  K = np->width+3;
  N = np->height;
  stride = 4;
  Pw = np->controlv;
  pu = order-1;
  num = np->height;

  if(!(uk = calloc(np->width+1, sizeof(double))))
    return AY_EOMEM;

  if(!(cds = calloc(np->width+2, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(U = calloc(K+np->uorder, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(Qt = calloc(K*stride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(Q = calloc(N*K*stride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  /* calculate parameterization */
  for(i = 0; i < N; i++)
    {
      ind1 = i*stride;
      ind2 = i*stride;
      total = 0.0;

      for(k = 1; k < np->width; k++)
	{
	  ind1 += N*stride;

	  v[0] = Pw[ind1]   - Pw[ind2];
	  v[1] = Pw[ind1+1] - Pw[ind2+1];
	  v[2] = Pw[ind1+2] - Pw[ind2+2];

	  if(fabs(v[0]) > AY_EPSILON ||
	     fabs(v[1]) > AY_EPSILON ||
	     fabs(v[2]) > AY_EPSILON)
	    {
	      if(ktype == AY_KTCENTRI)
		{
		  cds[k] = sqrt(AY_V3LEN(v));
		}
	      else
		{
		  if(ktype == AY_KTUNIFORM)
		    {
		      cds[k] = 0.01;
		    }
		  else
		    {
		      cds[k] = AY_V3LEN(v);
		    }
		}
	      total += cds[k];
	    }
	  ind2 += N*stride;
	} /* for */

      ind1 = i*stride;
      ind2 = ((np->width-1)*N+i)*stride;
      k = np->width;

      v[0] = Pw[ind1]   - Pw[ind2];
      v[1] = Pw[ind1+1] - Pw[ind2+1];
      v[2] = Pw[ind1+2] - Pw[ind2+2];

      if(fabs(v[0]) > AY_EPSILON ||
	 fabs(v[1]) > AY_EPSILON ||
	 fabs(v[2]) > AY_EPSILON)
	{
	  if(ktype == AY_KTCENTRI)
	    {
	      cds[k] = sqrt(AY_V3LEN(v));
	    }
	  else
	    {
	      if(ktype == AY_KTUNIFORM)
		{
		  cds[k] = 0.01;
		}
	      else
		{
		  cds[k] = AY_V3LEN(v);
		}
	    }
	  total += cds[k];
	}

      if(total < AY_EPSILON)
	{
	  num--;
	}
      else
	{
	  d = 0.0;
	  for(k = 1; k < np->width+1; k++)
	    {
	      d += cds[k];
	      uk[k] += d/total;
	    }
	}
    } /* for */

  if(num == 0)
    {
      ay_error(AY_ERROR, fname, "Can not interpolate this patch.");
      ay_status = AY_ERROR;
      goto cleanup;
    }

  uk[0] = 0.0;
  for(k = 1; k < np->width+1; k++)
    {
      uk[k] /= num;
    }
  uk[np->width] = 1.0;

  for(i = 0; i < (np->width+2-pu); i++)
    {
      ind1 = i+pu+1;
      U[ind1] = 0.0;
      for(k = i; k < (i+pu); k++)
	{
	  U[ind1] += uk[k];
	} /* for */

      U[ind1] /= pu;
    } /* for */
  for(i = 0; i <= pu; i++)
    U[i] = 0.0;
  for(i = K; i < (K+pu+1); i++)
    U[i] = 1.0;

  /* interpolate */
  for(i = 0; i < N; i++)
    {
      ind1 = i*stride;

      /* set up a sparse control vector */
      /* first point */
      memcpy(Qt, &(Pw[ind1]), stride*sizeof(double));
      ind1 += N*stride;

      /* inner points */
      for(k = 2; k < K-2; k++)
	{
	  memcpy(&(Qt[k*4]), &(Pw[ind1]), stride*sizeof(double));
	  ind1 += N*stride;
	} /* for */

      /* last point */
      memcpy(&(Qt[(K-1)*4]), &(Pw[i*stride]), stride*sizeof(double));

      /* derivatives */
      if(dmode)
	{
	  /* manual mode (peruse sd/ed) */
	  ind1 = i*stride;
	  ds[0] = sd[i*3] - Pw[ind1];
	  ds[1] = sd[i*3+1] - Pw[ind1+1];
	  ds[2] = sd[i*3+2] - Pw[ind1+2];

	  de[0] = Pw[ind1]   - ed[i*3];
	  de[1] = Pw[ind1+1] - ed[i*3+1];
	  de[2] = Pw[ind1+2] - ed[i*3+2];
	}
      else
	{
	  /* automatic mode (peruse sdlen/edlen) */
	  ind1 = i*stride;
	  ind2 = ind1 + N*stride;

	  ds[0] = Pw[ind2] - Pw[ind1];
	  ds[1] = Pw[ind2+1] - Pw[ind1+1];
	  ds[2] = Pw[ind2+2] - Pw[ind1+2];

	  AY_V3SCAL(ds, sdlen);

	  ind2 = ind1;
	  ind1 = ((np->width-1)*N+i)*stride;

	  de[0] = Pw[ind2] - Pw[ind1];
	  de[1] = Pw[ind2+1] - Pw[ind1+1];
	  de[2] = Pw[ind2+2] - Pw[ind1+2];

	  AY_V3SCAL(de, edlen);
	}

      /* interpolate */
      ay_status = ay_nb_GlobalInterpolation4DD(np->width, Qt, uk, U, pu,
					       ds, de);

      if(ay_status)
	{ goto cleanup; }

      /* copy results back */
      ind1 = i*stride;
      for(k = 0; k < K; k++)
	{
	  memcpy(&(Q[ind1]), &(Qt[k*stride]), stride*sizeof(double));
	  ind1 += N*stride;
	} /* for */
    } /* for */

  if(np->uknotv)
    free(np->uknotv);
  np->uknotv = U;
  np->uknot_type = AY_KTCUSTOM;
  np->uorder = pu+1;
  np->width = K;

  free(np->controlv);
  np->controlv = Q;

  /* prevent cleanup code from doing something harmful */
  U = NULL;
  Q = NULL;

cleanup:

  if(uk)
    free(uk);
  if(cds)
    free(cds);
  if(U)
    free(U);
  if(Qt)
    free(Qt);
  if(Q)
    free(Q);

 return ay_status;
} /* ay_ipt_interpolateudc */


/** ay_ipt_interpolatev:
 * interpolate NURBS patch along V (height)
 *
 * \param[in,out] np NURBS patch object to interpolate
 * \param[in] order desired interpolation order
 * \param[in] ktype parameterization type (AY_KTCHORDAL,
 *            AY_KTCENTRI, or AY_KTUNIFORM)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_interpolatev(ay_nurbpatch_object *np, int order, int ktype)
{
 int ay_status = AY_OK;
 char fname[] = "ipt_interpolatev";
 int i, k, N, K, stride, ind1, ind2, pv, num;
 double *vk = NULL, *cds = NULL, *Pw = NULL, v[3] = {0};
 double *V = NULL, total, d;

  if(!np)
    return AY_ENULL;

  K = np->width;
  N = np->height;
  stride = 4;
  Pw = np->controlv;
  pv = np->vorder-1;
  num = np->width;

  if(!(vk = calloc(N, sizeof(double))))
    return AY_EOMEM;

  if(!(cds = calloc(N+1, sizeof(double))))
    {
      free(vk); return AY_EOMEM;
    }

  if(!(V = calloc(N+np->vorder, sizeof(double))))
    {
      free(vk); free(cds); return AY_EOMEM;
    }

  /* calculate parameterization */
  for(i = 0; i < K; i++)
    {
      ind1 = (i*N)*stride;
      total = 0.0;
      for(k = 1; k < N; k++)
	{
	  ind2 = ind1+stride;
	  v[0] = Pw[ind2]   - Pw[ind1];
	  v[1] = Pw[ind2+1] - Pw[ind1+1];
	  v[2] = Pw[ind2+2] - Pw[ind1+2];

	  if(fabs(v[0]) > AY_EPSILON ||
	     fabs(v[1]) > AY_EPSILON ||
	     fabs(v[2]) > AY_EPSILON)
	    {
	      if(ktype == AY_KTCENTRI)
		{
		  cds[k] = sqrt(AY_V3LEN(v));
		}
	      else
		{
		  if(ktype == AY_KTUNIFORM)
		    {
		      cds[k] = 0.01;
		    }
		  else
		    {
		      cds[k] += AY_V3LEN(v);
		    }
		}
	      total += cds[k];
	    }
	  ind1 += stride;
	} /* for */

      if(total < AY_EPSILON)
	{
	  num--;
	}
      else
	{
	  d = 0.0;
	  for(k = 1; k < N; k++)
	    {
	      d += cds[k];
	      vk[k] += d/total;
	    }
	}
    } /* for */

  if(num == 0)
    {
      ay_error(AY_ERROR, fname, "Can not interpolate this patch.");
      free(vk); free(cds); free(V); return AY_ERROR;
    }

  vk[0] = 0.0;
  for(k = 1; k < N; k++)
    {
      vk[k] /= num;
    }
  vk[N-1] = 1.0;

  for(i = 1; i < (N-pv); i++)
    {
      V[i+pv] = 0.0;

      for(k = i; k < (i+pv); k++)
	{
	  V[i+pv] += vk[k];
	}

      V[i+pv] /= pv;
    } /* for */
  for(i = 0; i <= pv; i++)
    V[i] = 0.0;
  for(i = (N/*-pu-1*/); i < (N+pv+1); i++)
    V[i] = 1.0;

  /* interpolate */
  for(i = 0; i < K; i++)
    {
      ind1 = i*N*stride;

      ay_status = ay_nb_GlobalInterpolation4D(N-1,
			&(np->controlv[ind1]), vk, V, pv);

      if(ay_status)
	{ free(cds); free(vk); free(V); return ay_status; }

    } /* for */

  if(np->vknotv)
    free(np->vknotv);
  np->vknotv = V;
  np->vknot_type = AY_KTCUSTOM;
  np->vorder = pv+1;

  free(vk);
  free(cds);

 return AY_OK;
} /* ay_ipt_interpolatev */


/** ay_ipt_interpolatevd:
 * interpolate NURBS patch along V (height) with end derivatives
 *
 * \param[in,out] np NURBS patch object to interpolate
 * \param[in] order desired interpolation order
 * \param[in] ktype parameterization type (AY_KTCHORDAL,
 *            AY_KTCENTRI, or AY_KTUNIFORM)
 * \param[in] dmode derivative calculation mode (0 - automatic, 1 - manual)
 * \param[in] sdlen length of automatically generated start derivatives
 * \param[in] edlen length of automatically generated end derivatives
 * \param[in] sd start derivatives (may be NULL if dmode is 0)
 * \param[in] ed end derivatives (may be NULL if dmode is 0)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_interpolatevd(ay_nurbpatch_object *np, int order, int ktype,
		     int dmode, double sdlen, double edlen,
		     double *sd, double *ed)
{
 int ay_status = AY_OK;
 char fname[] = "ipt_interpolatevd";
 int i, k, N, K, stride, ind, pv, num;
 double *vk = NULL, *cds = NULL, *Pw = NULL, v[3] = {0};
 double *V = NULL, *Qt = NULL, *Q = NULL, total, d;
 double ds[3] = {0}, de[3] = {0};

  if(!np || (dmode && (!sd || !ed)))
    return AY_ENULL;

  K = np->height+2;
  N = np->width;
  stride = 4;
  Pw = np->controlv;
  pv = order-1;
  num = np->width;

  if(!(vk = calloc(np->height, sizeof(double))))
    return AY_EOMEM;

  if(!(cds = calloc(np->height+1, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(V = calloc(K+np->vorder, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(Qt = calloc(K*stride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(Q = calloc(N*K*stride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  ind = 0;
  /* calculate parameterization */
  for(i = 0; i < N; i++)
    {
      total = 0.0;

      for(k = 1; k < np->height; k++)
	{
	  v[0] = Pw[ind]   - Pw[ind+4];
	  v[1] = Pw[ind+1] - Pw[ind+5];
	  v[2] = Pw[ind+2] - Pw[ind+6];

	  if(fabs(v[0]) > AY_EPSILON ||
	     fabs(v[1]) > AY_EPSILON ||
	     fabs(v[2]) > AY_EPSILON)
	    {
	      if(ktype == AY_KTCENTRI)
		{
		  cds[k] = sqrt(AY_V3LEN(v));
		}
	      else
		{
		  if(ktype == AY_KTUNIFORM)
		    {
		      cds[k] = 0.01;
		    }
		  else
		    {
		      cds[k] = AY_V3LEN(v);
		    }
		}
	      total += cds[k];
	    }
	  ind += stride;
	} /* for */

      if(total < AY_EPSILON)
	{
	  num--;
	}
      else
	{
	  d = 0.0;
	  for(k = 1; k < np->height; k++)
	    {
	      d += cds[k];
	      vk[k] += d/total;
	    }
	}
    } /* for */

  if(num == 0)
    {
      ay_error(AY_ERROR, fname, "Can not interpolate this patch.");
      ay_status = AY_ERROR;
      goto cleanup;
    }

  vk[0] = 0.0;
  for(k = 1; k < np->height; k++)
    {
      vk[k] /= num;
    }
  vk[np->height-1] = 1.0;

  for(i = 0; i < (np->height+2-pv); i++)
    {
      ind = i+pv+1;
      V[ind] = 0.0;
      for(k = i; k < (i+pv); k++)
	{
	  V[ind] += vk[k];
	} /* for */

      V[ind] /= pv;
    } /* for */
  for(i = 0; i <= pv; i++)
    V[i] = 0.0;
  for(i = K; i < (K+pv+1); i++)
    V[i] = 1.0;

  /* interpolate */
  for(i = 0; i < N; i++)
    {
      /* set up a sparse control vector */
      /* first point */
      ind = np->height*i*stride;
      memcpy(Qt, &(Pw[ind]), stride*sizeof(double));

      /* inner points */
      memcpy(&(Qt[8]), &(Pw[ind+stride]), (K-4)*stride*sizeof(double));

      /* last point */
      memcpy(&(Qt[(K-1)*4]), &(Pw[ind+(np->height-1)*stride]),
	     stride*sizeof(double));

      /* derivatives */
      if(dmode)
	{
	  /* manual mode (peruse sd/ed) */
	  ds[0] = sd[i*3]   - Pw[ind];
	  ds[1] = sd[i*3+1] - Pw[ind+1];
	  ds[2] = sd[i*3+2] - Pw[ind+2];

	  ind += (np->height-1)*stride;
	  de[0] = Pw[ind]   - ed[i*3];
	  de[1] = Pw[ind+1] - ed[i*3+1];
	  de[2] = Pw[ind+2] - ed[i*3+2];
	}
      else
	{
	  /* automatic mode (peruse sdlen/edlen) */
	  ds[0] = Pw[ind+4] - Pw[ind];
	  ds[1] = Pw[ind+5] - Pw[ind+1];
	  ds[2] = Pw[ind+6] - Pw[ind+2];

	  AY_V3SCAL(ds, sdlen);

	  ind += (np->height-1)*stride;
	  de[0] = Pw[ind]   - Pw[ind-4];
	  de[1] = Pw[ind+1] - Pw[ind-3];
	  de[2] = Pw[ind+2] - Pw[ind-2];

	  AY_V3SCAL(de, edlen);
	}

      /* interpolate */
      ay_status = ay_nb_GlobalInterpolation4DD(np->height-1, Qt, vk, V, pv,
					       ds, de);

      if(ay_status)
	{ goto cleanup; }

      /* copy results back */
      memcpy(&(Q[i*K*stride]), Qt, K*stride*sizeof(double));
    } /* for */

  if(np->vknotv)
    free(np->vknotv);
  np->vknotv = V;
  np->vknot_type = AY_KTCUSTOM;
  np->vorder = pv+1;
  np->height = K;

  free(np->controlv);
  np->controlv = Q;

  /* prevent cleanup code from doing something harmful */
  V = NULL;
  Q = NULL;

cleanup:

  if(vk)
    free(vk);
  if(cds)
    free(cds);
  if(V)
    free(V);
  if(Qt)
    free(Qt);
  if(Q)
    free(Q);

 return ay_status;
} /* ay_ipt_interpolatevd */


/** ay_ipt_interpolatevdc:
 * interpolate NURBS patch along V (height) with end derivatives
 * creates a closed surface (in V)
 *
 * \param[in,out] np NURBS patch object to interpolate
 * \param[in] order desired interpolation order
 * \param[in] ktype parameterization type (AY_KTCHORDAL,
 *            AY_KTCENTRI, or AY_KTUNIFORM)
 * \param[in] dmode derivative calculation mode (0 - automatic, 1 - manual)
 * \param[in] sdlen length of automatically generated start derivatives
 * \param[in] edlen length of automatically generated end derivatives
 * \param[in] sd start derivatives (may be NULL if dmode is 0)
 * \param[in] ed end derivatives (may be NULL if dmode is 0)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_interpolatevdc(ay_nurbpatch_object *np, int order, int ktype,
		      int dmode, double sdlen, double edlen,
		      double *sd, double *ed)
{
 int ay_status = AY_OK;
 char fname[] = "ipt_interpolatevdc";
 int i, k, N, K, stride, ind1, ind2, pv, num;
 double *vk = NULL, *cds = NULL, *Pw = NULL, v[3] = {0};
 double *V = NULL, *Qt = NULL, *Q = NULL, total, d;
 double ds[3] = {0}, de[3] = {0};

  if(!np || (dmode && (!sd || !ed)))
    return AY_ENULL;

  K = np->height+3;
  N = np->width;
  stride = 4;
  Pw = np->controlv;
  pv = order-1;
  num = np->width;

  if(!(vk = calloc(np->height+1, sizeof(double))))
    return AY_EOMEM;

  if(!(cds = calloc(np->height+2, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(V = calloc(K+np->vorder, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(Qt = calloc(K*stride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(!(Q = calloc(N*K*stride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  ind1 = 0;
  /* calculate parameterization */
  for(i = 0; i < N; i++)
    {
      total = 0.0;

      for(k = 1; k < np->height; k++)
	{
	  v[0] = Pw[ind1]   - Pw[ind1+4];
	  v[1] = Pw[ind1+1] - Pw[ind1+5];
	  v[2] = Pw[ind1+2] - Pw[ind1+6];

	  if(fabs(v[0]) > AY_EPSILON ||
	     fabs(v[1]) > AY_EPSILON ||
	     fabs(v[2]) > AY_EPSILON)
	    {
	      if(ktype == AY_KTCENTRI)
		{
		  cds[k] = sqrt(AY_V3LEN(v));
		}
	      else
		{
		  if(ktype == AY_KTUNIFORM)
		    {
		      cds[k] = 0.01;
		    }
		  else
		    {
		      cds[k] = AY_V3LEN(v);
		    }
		}
	      total += cds[k];
	    }
	  ind1 += stride;
	} /* for */

      ind1 = 0;
      v[0] = Pw[ind1]   - Pw[ind1+4];
      v[1] = Pw[ind1+1] - Pw[ind1+5];
      v[2] = Pw[ind1+2] - Pw[ind1+6];

      if(fabs(v[0]) > AY_EPSILON ||
	 fabs(v[1]) > AY_EPSILON ||
	 fabs(v[2]) > AY_EPSILON)
	{
	  if(ktype == AY_KTCENTRI)
	    {
	      cds[k] = sqrt(AY_V3LEN(v));
	    }
	  else
	    {
	      if(ktype == AY_KTUNIFORM)
		{
		  cds[k] = 0.01;
		}
	      else
		{
		  cds[k] = AY_V3LEN(v);
		}
	    }
	  total += cds[k];
	}

      if(total < AY_EPSILON)
	{
	  num--;
	}
      else
	{
	  d = 0.0;
	  for(k = 1; k < np->height+1; k++)
	    {
	      d += cds[k];
	      vk[k] += d/total;
	    }
	}
    } /* for */

  if(num == 0)
    {
      ay_error(AY_ERROR, fname, "Can not interpolate this patch.");
      ay_status = AY_ERROR;
      goto cleanup;
    }

  vk[0] = 0.0;
  for(k = 1; k < np->height+1; k++)
    {
      vk[k] /= num;
    }
  vk[np->height] = 1.0;

  for(i = 0; i < (np->height+2-pv); i++)
    {
      ind1 = i+pv+1;
      V[ind1] = 0.0;
      for(k = i; k < (i+pv); k++)
	{
	  V[ind1] += vk[k];
	} /* for */

      V[ind1] /= pv;
    } /* for */
  for(i = 0; i <= pv; i++)
    V[i] = 0.0;
  for(i = K; i < (K+pv+1); i++)
    V[i] = 1.0;

  /* interpolate */
  for(i = 0; i < N; i++)
    {
      /* set up a sparse control vector */
      /* first point */
      ind1 = np->height*i*stride;
      memcpy(Qt, &(Pw[ind1]), stride*sizeof(double));

      /* inner points */
      memcpy(&(Qt[8]), &(Pw[ind1+stride]), (K-3)*stride*sizeof(double));

      /* last point */
      memcpy(&(Qt[(K-1)*4]), &(Pw[ind1]), stride*sizeof(double));

      /* derivatives */
      if(dmode)
	{
	  /* manual mode (peruse sd/ed) */
	  ds[0] = sd[i*3]   - Pw[ind1];
	  ds[1] = sd[i*3+1] - Pw[ind1+1];
	  ds[2] = sd[i*3+2] - Pw[ind1+2];

	  de[0] = Pw[ind1]   - ed[i*3];
	  de[1] = Pw[ind1+1] - ed[i*3+1];
	  de[2] = Pw[ind1+2] - ed[i*3+2];
	}
      else
	{
	  /* automatic mode (peruse sdlen/edlen) */
	  ds[0] = Pw[ind1+4] - Pw[ind1];
	  ds[1] = Pw[ind1+5] - Pw[ind1+1];
	  ds[2] = Pw[ind1+6] - Pw[ind1+2];

	  AY_V3SCAL(ds, sdlen);

	  ind2 = ind1 + (np->height-1)*stride;
	  de[0] = Pw[ind1]   - Pw[ind2];
	  de[1] = Pw[ind1+1] - Pw[ind2+1];
	  de[2] = Pw[ind1+2] - Pw[ind2+2];

	  AY_V3SCAL(de, edlen);
	}

      /* interpolate */
      ay_status = ay_nb_GlobalInterpolation4DD(np->height, Qt, vk, V, pv,
					       ds, de);

      if(ay_status)
	{ goto cleanup; }

      /* copy results back */
      memcpy(&(Q[i*K*stride]), Qt, K*stride*sizeof(double));
    } /* for */

  if(np->vknotv)
    free(np->vknotv);
  np->vknotv = V;
  np->vknot_type = AY_KTCUSTOM;
  np->vorder = pv+1;
  np->height = K;

  free(np->controlv);
  np->controlv = Q;

  /* prevent cleanup code from doing something harmful */
  V = NULL;
  Q = NULL;

cleanup:

  if(vk)
    free(vk);
  if(cds)
    free(cds);
  if(V)
    free(V);
  if(Qt)
    free(Qt);
  if(Q)
    free(Q);

 return ay_status;
} /* ay_ipt_interpolatevdc */


/** ay_ipt_interpuvtcmd:
 *  Interpolate the selected NURBS patches along U/V.
 *  Implements the \a interpuNP and \a interpvNP scripting interface commands.
 *  See also the corresponding section in the \ayd{interpunp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_ipt_interpuvtcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int tcl_status = TCL_OK, ay_status = AY_OK;
 int i = 1;
 ay_object *o = NULL;
 ay_nurbpatch_object *patch = NULL;
 ay_list_object *sel = ay_selection;
 int interpolv = AY_FALSE, order = 4, ktype = AY_KTCHORDAL;
 int closed = AY_FALSE;
 double sdlen = 0.0, edlen = 0.0;

  if(!strcmp(argv[0], "interpvNP"))
    interpolv = AY_TRUE;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  /* parse args */
  while(i+1 < argc)
    {
      if(argv[i] && argv[i][0] != '\0')
	{
	  switch(argv[i][1])
	    {
	    case 'c':
	      /* -closed */
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &closed);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	      break;
	    case 'e':
	      /* -edlen */
	      tcl_status = Tcl_GetDouble(interp, argv[i+1], &edlen);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	      break;
	    case 'o':
	      /* -order */
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &order);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	      if(order <= 2)
		{
		  ay_error(AY_ERROR, argv[0], "Order must be > 2.");
		  return TCL_OK;
		}
	      break;
	    case 'k':
	      /* -ktype */
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &ktype);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	      switch(ktype)
		{
		case 0:
		  ktype = AY_KTCHORDAL;
		  break;
		case 1:
		  ktype = AY_KTCENTRI;
		  break;
		case 2:
		  ktype = AY_KTUNIFORM;
		  break;
		default:
		  ktype = AY_KTCHORDAL;
		  break;
		} /* switch */
	      break;
	    case 's':
	      /* -sdlen */
	      tcl_status = Tcl_GetDouble(interp, argv[i+1], &sdlen);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	      break;
	    default:
	      break;
	    } /* switch */
	} /* if */
      i += 2;
    } /* while */

  while(sel)
    {
      o = sel->object;
      if(o->type == AY_IDNPATCH)
	{
	  patch = (ay_nurbpatch_object*)o->refine;

	  if(closed)
	    {
	      if(interpolv)
		ay_status = ay_ipt_interpolatevdc(patch, order, ktype,
						  /*dmode*/0, sdlen, edlen,
						  NULL, NULL);
	      else
		ay_status = ay_ipt_interpolateudc(patch, order, ktype,
						  /*dmode*/0, sdlen, edlen,
						  NULL, NULL);
	    }
	  else
	    {
	      if(sdlen == 0.0 && edlen == 0.0)
		{
		  if(interpolv)
		    ay_status = ay_ipt_interpolatev(patch, order, ktype);
		  else
		    ay_status = ay_ipt_interpolateu(patch, order, ktype);
		}
	      else
		{
		  if(interpolv)
		    ay_status = ay_ipt_interpolatevd(patch, order, ktype,
						  /*dmode*/0, sdlen, edlen,
						  NULL, NULL);
		  else
		    ay_status = ay_ipt_interpolateud(patch, order, ktype,
						  /*dmode*/0, sdlen, edlen,
						  NULL, NULL);
		}
	    } /* if closed */

	  if(!ay_status)
	    {
	      ay_npt_recreatemp(patch);

	      /* remove all selected points */
	      if(o->selp)
		{
		  ay_selp_clear(o);
		}

	      o->modified = AY_TRUE;

	      /* re-create tesselation of patch */
	      (void)ay_notify_object(o);
	    }
	  else
	    {
	      ay_error(AY_EWARN, argv[0], "Interpolation failed.");
	    }
	}
      else
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	} /* if */
      sel = sel->next;
    } /* while */

  (void)ay_notify_parent();

 return TCL_OK;
} /* ay_ipt_interpuvtcmd */


/** ay_ipt_crtderiv:
 * create default derivative vectors (from data points and
 * sdlen/edlen parameters)
 *
 * \param[in] mode designates which vector to create
 * \param[in,out] ip IPatch object to process
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_crtderiv(int mode, ay_ipatch_object *ip)
{
 int ay_status = AY_OK;
 int stride = 3;
 int i = 0, j = 0, a = 0, b = 0, c = 0, aoff = 0, boff = 0;
 double *cv = NULL, *dv = NULL, t[3] = {0};
 double len = 0.125;

  if(!ip)
    return AY_ENULL;

  cv = ip->controlv;

  switch(mode)
    {
    case 0:
      /* sderiv_u */
      if(!(ip->sderiv_u = calloc(ip->height*stride, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      dv = ip->sderiv_u;
      a = 0;
      b = ip->height*stride;
      j = ip->height;
      aoff = stride;
      boff = stride;
      len = ip->sdlen_u;
      break;
    case 1:
      /* ederiv_u */
      if(!(ip->ederiv_u = calloc(ip->height*stride, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      dv = ip->ederiv_u;
      a = (ip->width-1)*ip->height*stride;
      b = a - ip->height*stride;
      j = ip->height;
      aoff = stride;
      boff = stride;
      len = ip->edlen_u;
      break;
    case 2:
      /* sderiv_v */
      if(!(ip->sderiv_v = calloc(ip->width*stride, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      dv = ip->sderiv_v;
      a = 0;
      b = stride;
      j = ip->height;
      aoff = ip->height*stride;
      boff = ip->height*stride;
      len = ip->sdlen_v;
      break;
    case 3:
      /* ederiv_v */
      if(!(ip->ederiv_v = calloc(ip->width*stride, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      dv = ip->ederiv_v;
      a = (ip->height-1)*stride;
      b = a-stride;
      j = ip->height;
      aoff = ip->height*stride;
      boff = ip->height*stride;
      len = ip->edlen_v;
      break;
    default:
      break;
    } /* switch */

  if(dv)
    {
      c = 0;
      for(i = 0; i < j; i++)
	{
	  t[0] = cv[b]   - cv[a];
	  t[1] = cv[b+1] - cv[a+1];
	  t[2] = cv[b+2] - cv[a+2];

	  if(fabs(t[0]) > AY_EPSILON || fabs(t[1]) > AY_EPSILON ||
	     fabs(t[2]) > AY_EPSILON)
	    AY_V3SCAL(t, len);

	  t[0] += cv[a];
	  t[1] += cv[a+1];
	  t[2] += cv[a+2];

	  memcpy(&(dv[c]), t, stride*sizeof(double));
	  b += boff;
	  a += aoff;
	  c += stride;
	} /* for */
    } /* if */

cleanup:

 return ay_status;
} /* ay_ipt_crtderiv */
