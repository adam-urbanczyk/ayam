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

/* ipt.c - interpolating surface tools */

/** ay_ipt_swapuv:
 * swap U and V dimensions of a IPatch
 *
 * @param[in,out] ip IPatch object to process
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_swapuv(ay_ipatch_object *ip)
{
 int ay_status = AY_OK;
 int i;

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

 return ay_status;
} /* ay_ipt_swapuv */


/** ay_ipt_revertu:
 * revert IPatch along U (width)
 *
 * @param[in,out] ip IPatch object to revert
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_revertu(ay_ipatch_object *ip)
{
 int i, j, ii, jj, stride = 3;
 double t[4];

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

 return AY_OK;
} /* ay_ipt_revertu */


/** ay_ipt_revertv:
 * revert IPatch along V (height)
 *
 * @param[in,out] ip IPatch object to revert
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_revertv(ay_ipatch_object *ip)
{
 int ay_status = AY_OK;
 int i, j, ii, jj, stride = 3;
 double t[4];

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

 return ay_status;
} /* ay_ipt_revertv */


/** ay_ipt_interpolateu:
 * interpolate NURBS patch along U (width) 
 *
 * @param[in,out] np NURBS patch object to interpolate
 * @param[in] order desired interpolation order
 * @param[in] ktype parameterization type (AY_KTCHORDAL or AY_KTCENTRI) 
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_interpolateu(ay_nurbpatch_object *np, int order, int ktype)
{
 int ay_status = AY_OK;
 char fname[] = "npt_interpolateu";
 int i, k, N, K, stride, ind, ind2, pu, num;
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
      ind = i*stride;
      ind2 = i*stride;
      total = 0.0;

      for(k = 1; k < K; k++)
	{
	  ind += N*stride;

	  v[0] = Pw[ind]   - Pw[ind2];
	  v[1] = Pw[ind+1] - Pw[ind2+1];
	  v[2] = Pw[ind+2] - Pw[ind2+2];

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
		  cds[k] = AY_V3LEN(v);
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
      free(uk); free(cds); free(U); return AY_ERROR;
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
      ind = i*stride;
      for(k = 0; k < K; k++)
	{
	  memcpy(&(Q[k*4]), &(Pw[ind]), stride*sizeof(double));
	  if(stride != 4)
	    Q[k*4+3] = 1.0;
	  ind += N*stride;
	} /* for */

      ay_status = ay_nb_GlobalInterpolation4D(K-1, Q, uk, U, pu);

      if(ay_status)
	{ free(cds); free(uk); free(U); free(Q); return ay_status; }

      ind = i*stride;
      for(k = 0; k < K; k++)
	{
	  memcpy(&(Pw[ind]), &(Q[k*stride]), stride*sizeof(double));
	  ind += N*stride;
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


/** ay_ipt_interpolatev:
 * interpolate NURBS patch along V (height) 
 *
 * @param[in,out] np NURBS patch object to interpolate
 * @param[in] order desired interpolation order
 * @param[in] ktype parameterization type (AY_KTCHORDAL or AY_KTCENTRI) 
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_ipt_interpolatev(ay_nurbpatch_object *np, int order, int ktype)
{
 int ay_status = AY_OK;
 char fname[] = "npt_interpolatev";
 int i, k, N, K, stride, ind, ind2, pv, num;
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
      ind = (i*N)*stride;
      total = 0.0;
      for(k = 1; k < N; k++)
	{
	  ind2 = ind+stride;
	  v[0] = Pw[ind2]   - Pw[ind];
	  v[1] = Pw[ind2+1] - Pw[ind+1];
	  v[2] = Pw[ind2+2] - Pw[ind+2];

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
		  cds[k] += AY_V3LEN(v);
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
      ind = i*N*stride;

      ay_status = ay_nb_GlobalInterpolation4D(N-1,
			&(np->controlv[ind]), vk, V, pv);

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
 ay_object *o = NULL;
 ay_nurbpatch_object *patch = NULL;
 ay_list_object *sel = ay_selection;
 int interpolv = AY_FALSE, order = 0, ktype = AY_KTCHORDAL;

  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "order");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  tcl_status = Tcl_GetInt(interp, argv[1], &order);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  if(order <= 2)
    {
      ay_error(AY_ERROR, argv[0], "Parameter order must be > 2.");
      return TCL_OK;
    }

  if(argc > 2)
    {
      tcl_status = Tcl_GetInt(interp, argv[2], &ktype);
      AY_CHTCLERRRET(tcl_status, argv[0], interp);
      if(ktype)
	ktype = AY_KTCENTRI;
    }

  if(!strcmp(argv[0], "interpvNP"))
    interpolv = AY_TRUE;

  while(sel)
    {
      o = sel->object;
      if(o->type == AY_IDNPATCH)
	{
	  patch = (ay_nurbpatch_object*)o->refine;

	  if(interpolv)
	    ay_status = ay_ipt_interpolatev(patch, order, ktype);
	  else
	    ay_status = ay_ipt_interpolateu(patch, order, ktype);

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
	      ay_notify_force(o);
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

  ay_notify_parent();

 return TCL_OK;
} /* ay_ipt_interpuvtcmd */
