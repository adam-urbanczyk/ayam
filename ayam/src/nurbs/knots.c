/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2001 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* knots.c */

/* ay_knots_createnp:
 *   create knot vectors according to
 *   the knot_type fields for patch *patch
 */
int
ay_knots_createnp(ay_nurbpatch_object *patch)
{
 int uorder = 0, vorder = 0, deg = 0, width = 0, height = 0;
 int uknot_count = 0, vknot_count = 0;
 int index = 0, i = 0, j = 0, kts = 0;
 double *ub = NULL;
 /*int start = 0;*/
 double *U = NULL, *V = NULL;

  /* sanity check */
  if(!patch)
    return AY_ENULL;

  uorder = patch->uorder;
  vorder = patch->vorder;
  width = patch->width;
  height = patch->height;
  uknot_count = width + uorder;
  vknot_count = height + vorder;

  if(patch->uknot_type != AY_KTCUSTOM)
    {
      /* calloc new knot-arrays */
      if(!(U = calloc(uknot_count, sizeof(double))))
	return AY_EOMEM;

      /* free old knot-arrays */
      if(patch->uknotv != NULL)
	free(patch->uknotv);

      patch->uknotv = U;
    }

  if(patch->vknot_type != AY_KTCUSTOM)
    {
      if(!(V = calloc(vknot_count, sizeof(double))))
	{
	  if(U)
	    {
	      free(U);
	      patch->uknotv = NULL;
	    }
	  return AY_EOMEM;
	}
      if(patch->vknotv != NULL)
	free(patch->vknotv);

      patch->vknotv = V;
    }

  /* fill knot-arrays */
  switch(patch->uknot_type)
    {
    case AY_KTBEZIER:
      for(i=0; i<uknot_count/2; i++)
	(patch->uknotv)[i] = 0.0;
      for(i=uknot_count/2; i<uknot_count; i++)
	(patch->uknotv)[i] = 1.0;
      break;

    case AY_KTBSPLINE:
      /*
      j=0;
      start = (int)(floor(uknot_count/2));
      for(i=-start; i<start; i++)
	{
	  (patch->uknotv)[j] = i/fabs((double)(uknot_count-(uorder-1)));
	  j++;
	}
      if(fmod(uknot_count,2.0) > 0.0)
	(patch->uknotv)[j] = i/fabs((double)(uknot_count-(uorder-1)));
      */
      (patch->uknotv)[0] = 0.0;
      for(i = 1; i < uknot_count; i++)
	{
	  (patch->uknotv)[i] = (double)i/(uknot_count-1);
	}
      break;

    case AY_KTNURB:
      for(i=0; i<uorder; i++)
	(patch->uknotv)[i] = 0.0;
      j=1;
      kts = 1 + uknot_count - (uorder*2);
      for(i=uorder; i<=uknot_count-uorder; i++)
	(patch->uknotv)[i] = j++/(double)kts;
      for(i=uknot_count-uorder; i<uknot_count; i++)
	(patch->uknotv)[i] = 1.0;
      break;

    case AY_KTCHORDAL:
    case AY_KTCENTRI:

      deg = uorder-1;

      if(patch->uknot_type == AY_KTCHORDAL)
	ay_knots_chordparamnp(0, patch->controlv, width, height, 4, &ub);
      else
	ay_knots_centriparamnp(0, patch->controlv, width, height, 4, &ub);

      if(!ub)
	{
	  /*
	    failed to create parameters (curve degenerated?);
	    create a standard NURBS knot vector instead
	   */
	  for(i = 0; i < uorder; i++)
	    U[i] = 0.0;
	  j = 1;
	  kts = 1 + uknot_count - (uorder*2);
	  for(i = uorder; i <= uknot_count - uorder; i++)
	    U[i] = j++/((double)kts);
	  for(i = uknot_count - uorder; i < uknot_count; i++)
	    U[i] = 1.0;
	  break;
	}

      /* knot averaging */
      for(j = 1; j < width-deg; j++)
	{
	  index = j + (uorder - 1);
	  U[index] = 0.0;
	  for(i = j; i < j + deg; i++)
	    {
	      U[index] += ub[i];
	    }
	  U[index] /= ((double)deg);
	}

      if(patch->utype != AY_CTPERIODIC)
	{
	  for(i = 0; i < uorder; i++)
	    U[i] = 0.0;
	  for(i = width; i < uknot_count; i++)
	    U[i] = 1.0;
	}
      else
	{
	  /* periodic curves get periodic knot extensions */
	  for(i = 0; i < deg; i++)
	    U[i] = U[i+(width-deg)] - 1.0;

	  for(i = width; i < uknot_count; i++)
	    U[i] = 1.0 + U[i-(width-deg)];
	}

      free(ub);
      break;

    case AY_KTCUSTOM:
      /* user specified own knot vertices */
      break;
    default:
      break;
    } /* switch */

  switch(patch->vknot_type)
    {
    case AY_KTBEZIER:
      for(i=0; i<vknot_count/2; i++)
	(patch->vknotv)[i] = 0.0;
      for(i=vknot_count/2; i<vknot_count; i++)
	(patch->vknotv)[i] = 1.0;
      break;

    case AY_KTBSPLINE:
      /*
      j=0;
      start = (int)(floor(vknot_count/2));
      for(i=-start; i<start; i++)
	{
	  (patch->vknotv)[j] = i/fabs((double)(vknot_count-(vorder-1)));
	  j++;
	}
      if(fmod(vknot_count,2.0) > 0.0)
	(patch->vknotv)[j] = i/fabs((double)(vknot_count-(vorder-1)));
      */
      (patch->vknotv)[0] = 0.0;
      for(i = 1; i < vknot_count; i++)
	{
	  (patch->vknotv)[i] = (double)i/(vknot_count-1);
	}
      break;

    case AY_KTNURB:
      for(i=0; i<vorder; i++)
	(patch->vknotv)[i] = 0.0;
      j=1;
      kts = 1 + vknot_count - (vorder*2);
      for(i=vorder; i<=vknot_count-vorder; i++)
	(patch->vknotv)[i] = j++/((double)kts);
      for(i=vknot_count-vorder; i<vknot_count; i++)
	(patch->vknotv)[i] = 1.0;
      break;

    case AY_KTCHORDAL:
    case AY_KTCENTRI:

      deg = vorder-1;

      if(patch->uknot_type == AY_KTCHORDAL)
	ay_knots_chordparamnp(1, patch->controlv, width, height, 4, &ub);
      else
	ay_knots_centriparamnp(1, patch->controlv, width, height, 4, &ub);

      if(!ub)
	{
	  /*
	    failed to create parameters (curve degenerated?);
	    create a standard NURBS knot vector instead
	   */
	  for(i = 0; i < vorder; i++)
	    V[i] = 0.0;
	  j = 1;
	  kts = 1 + vknot_count - (vorder*2);
	  for(i = vorder; i <= vknot_count - vorder; i++)
	    V[i] = j++/((double)kts);
	  for(i = vknot_count - vorder; i < vknot_count; i++)
	    V[i] = 1.0;
	  break;
	}

      /* knot averaging */
      for(j = 1; j < height-deg; j++)
	{
	  index = j + (vorder - 1);
	  V[index] = 0.0;
	  for(i = j; i < j + deg; i++)
	    {
	      V[index] += ub[i];
	    }
	  V[index] /= ((double)deg);
	}

      if(patch->utype != AY_CTPERIODIC)
	{
	  for(i = 0; i < vorder; i++)
	    V[i] = 0.0;
	  for(i = height; i < vknot_count; i++)
	    V[i] = 1.0;
	}
      else
	{
	  /* periodic curves get periodic knot extensions */
	  for(i = 0; i < deg; i++)
	    V[i] = V[i+(height-deg)] - 1.0;

	  for(i = height; i < vknot_count; i++)
	    V[i] = 1.0 + V[i-(height-deg)];
	}

      free(ub);
      break;

    case AY_KTCUSTOM:
      /* user specified own knot vertices */
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_knots_createnp */


/* ay_knots_createnc:
 *  create knot vector according to
 *  the knot_type field for curve <curve>
 */
int
ay_knots_createnc(ay_nurbcurve_object *curve)
{
 int order = 0, deg = 0, length = 0, knot_count = 0;
 int index, i = 0, j = 0, kts = 0;
 double *ub = NULL, *U = NULL;

  /* sanity check */
  if(!curve)
    return AY_ENULL;

  order = curve->order;
  length = curve->length;

  knot_count = length + order;

  /* calloc new knot-arrays */
  if((U = calloc(knot_count, sizeof(double))) == NULL)
    return AY_EOMEM;

  /* free old knot-array */
  if(curve->knotv)
    free(curve->knotv);

  curve->knotv = U;

  /* fill knot-arrays */
  switch(curve->knot_type)
    {
    case AY_KTBEZIER:
      for(i = 0; i < knot_count/2; i++)
	U[i] = 0.0;
      for(i = knot_count/2; i < knot_count; i++)
	U[i] = 1.0;
      break;

    case AY_KTBSPLINE:
      /*
      j=0;
      start = floor(knot_count/2);
      for(i=-start; i<start; i++)
	{
	  U[j] = i/fabs((double)(knot_count-(order-1)));
	  j++;
	}
      if(fmod(knot_count,2.0) > 0.0)
	U[j] = i/fabs((double)(knot_count-(order-1)));
      */
      U[0] = 0.0;
      for(i = 1; i < knot_count; i++)
	{
	  U[i] = (double)i/(knot_count-1);
	}
      break;

    case AY_KTNURB:
      for(i = 0; i < order; i++)
	U[i] = 0.0;
      j = 1;
      kts = 1 + knot_count - (order*2);
      for(i = order; i <= knot_count - order; i++)
	U[i] = j++/((double)kts);
      for(i = knot_count - order; i < knot_count; i++)
	U[i] = 1.0;
      break;

    case AY_KTCUSTOM:
      /* user specified own knot vertices */
      break;

    case AY_KTCHORDAL:
    case AY_KTCENTRI:

      deg = order-1;

      if(curve->knot_type == AY_KTCHORDAL)
	ay_knots_chordparam(curve->controlv, length, 4, &ub);
      else
	ay_knots_centriparam(curve->controlv, length, 4, &ub);

      if(!ub)
	{
	  /*
	    failed to create parameters (curve degenerated?);
	    create a standard NURBS knot vector instead, and
	    return the error
	   */
	  for(i = 0; i < order; i++)
	    U[i] = 0.0;
	  j = 1;
	  kts = 1 + knot_count - (order*2);
	  for(i = order; i <= knot_count - order; i++)
	    U[i] = j++/((double)kts);
	  for(i = knot_count - order; i < knot_count; i++)
	    U[i] = 1.0;
	  return AY_ERROR;
	}

      /* knot averaging */
      for(j = 1; j < length-deg; j++)
	{
	  index = j + (order - 1);
	  U[index] = 0.0;
	  for(i = j; i < j + deg; i++)
	    {
	      U[index] += ub[i];
	    }
	  U[index] /= ((double)deg);
	}

      if(curve->type != AY_CTPERIODIC)
	{
	  for(i = 0; i < order; i++)
	    U[i] = 0.0;
	  for(i = length; i < knot_count; i++)
	    U[i] = 1.0;
	}
      else
	{
	  /* periodic curves get periodic knot extensions */
	  for(i = 0; i < deg; i++)
	    U[i] = U[i+(length-deg)] - 1.0;

	  for(i = length; i < knot_count; i++)
	    U[i] = 1.0 + U[i-(length-deg)];
	}

      free(ub);
      break;

    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_knots_createnc */


/* ay_knots_check:
 *   returns:
 *   o 0 - the knots are ok
 *   o 1 - too few knots
 *   o 2 - too much knots
 *   o 3 - knot multiplicity too high
 *   o 4 - decreasing knots
 *   returns -1 if NULL pointer is delivered
 */
int
ay_knots_check(int length, int order, int knot_count, double *knotv)
{
 int i, mult_count = 1;

  /* sanity check */
  if(!knotv)
    return -1;

  if(knot_count < (length+order))
    return 1;

  if(knot_count > (length+order))
    return 2;

  for(i = 0; i < (knot_count-1); i++)
    {
      if(knotv[i] == knotv[i+1])
	{
	  mult_count++;
	  if(mult_count > order)
	    return 3;
	}
      else
	{
	  mult_count = 1;

	  if(knotv[i] > knotv[i+1])
	    return 4;
	} /* if */
    } /* for */

 return 0;
} /* ay_knots_check */


/* ay_knots_printerr:
 *  print knot error as returned from ay_knots_check()
 *  to the Ayam console
 */
void
ay_knots_printerr(char *location, int errcode)
{

  if(!location)
    return;

  switch(errcode)
    {
    case 1:
      ay_error(AY_ERROR, location, "Knot sequence has too few knots!");
      break;
    case 2:
      ay_error(AY_ERROR, location, "Knot sequence has too much knots!");
      break;
    case 3:
      ay_error(AY_ERROR, location, "Knot multiplicity higher than order!");
      break;
    case 4:
      ay_error(AY_ERROR, location, "Knot sequence has decreasing knots!");
      break;
    default:
      break;
    } /* switch */

 return;
} /* ay_knots_printerr */


/* ay_knots_rescaletorange:
 *  rescale knot vector knotv[n] to the new range [rmin, rmax] (rmin < rmax)
 */
int
ay_knots_rescaletorange(int n, double *knotv, double rmin, double rmax)
{
 double *tmpknv = NULL, min, max, len, newlen;
 int i;

  /* sanity check */
  if(!knotv)
    return AY_ENULL;

  if(rmin >= rmax)
    return AY_EARGS;

  max = knotv[0];
  min = knotv[0];
  newlen = rmax-rmin;

  for(i = 0; i < n; i++)
    {
      if(knotv[i] > max)
	max = knotv[i];

      if(knotv[i] < min)
	min = knotv[i];
    } /* for */

  if(min < 0.0 && max > 0.0)
    len = max + fabs(min);
  else
    len = max - min;

  if(!(tmpknv = calloc(n, sizeof(double))))
    return AY_EOMEM;

  memcpy(tmpknv, knotv, n*sizeof(double));

  knotv[0] = rmin;
  for(i = 1; i < n-1; i++)
    {
      knotv[i] = rmin+((tmpknv[i] - tmpknv[0])/len*newlen);
    }
  knotv[n-1] = rmax;

  free(tmpknv);

 return AY_OK;
} /* ay_knots_rescaletorange */


/* ay_knots_rescaletomindist:
 *  rescale knot vector n, knotv to the minimum distance mindist
 *  after scaling, no distance between two knots is smaller than mindist
 *  except for multiple knots; this is most useful for drawing with GLU
 *  which has a very low epsilon (about 1.0e-04) to decide whether two
 *  knot values are the same
 */
int
ay_knots_rescaletomindist(int n, double *knotv, double mindist /*1.0e-04*/)
{
 double knotv_mindist = DBL_MAX, sf = 0.0;
 int i;

  /* sanity check */
  if(!knotv)
    return AY_ENULL;

  for(i = 1; i < n; i++)
    {
      /* multiple knot? */
      if(knotv[i] != knotv[i-1])
	{
	  /* No! Compute distance... */
	  if((knotv[i] - knotv[i-1]) < knotv_mindist)
	    knotv_mindist = (knotv[i] - knotv[i-1]);
	}
    } /* for */

  if(knotv_mindist > mindist)
    return AY_OK;

  /* compute (safe) scale factor */
  sf = (mindist*1.1)/knotv_mindist;

  /* scale knot vector */
  for(i = 0; i < n; i++)
    {
      knotv[i] *= sf;
    } /* for */

 return AY_OK;
} /* ay_knots_rescaletomindist */


/* ay_knots_unify:
 *  unify knot vectors Ua[Ualen] and Ub[Ublen] (add to Ua[] all knots
 *  from Ub[], that are not already in Ua[]), returns result
 *  in newly allocated Ubar[Ubarlen]
 */
int
ay_knots_unify(double *Ua, int Ualen, double *Ub, int Ublen,
	       double **Ubar, int *Ubarlen)
{
 int i = 0, ia = 0, ib = 0, done = AY_FALSE;
 double t = 0.0, *U = NULL;

  /* sanity check */
  if(!Ua || !Ub || !Ubar || !Ubarlen)
    return AY_ENULL;

  if(!(U = calloc(Ualen+Ublen, sizeof(double))))
   {
     return AY_EOMEM;
   }

  while(!done)
    {
      /* was: if(Ua[ia] == Ub[ib]) */
      if(fabs(Ua[ia]-Ub[ib]) < AY_EPSILON)
	{
	  t = Ua[ia];
	  ia++;
	  ib++;
	}
      else
	{
	  if(Ua[ia] < Ub[ib])
	    {
	      t = Ua[ia];
	      ia++;
	    }
	  else
	    {
	      t = Ub[ib];
	      ib++;
	    } /* if */
	} /* if */
      U[i] = t;
      i++;
      if((ia >= Ualen || ib >= Ublen))
	done = AY_TRUE;
    } /* while */

  if(*Ubar)
    {
      free(*Ubar);
      *Ubar = NULL;
    }

  if(!(*Ubar = calloc(i, sizeof(double))))
    {
      free(U);
      return AY_EOMEM;
    }

  memcpy(*Ubar, U, i*sizeof(double));

  free(U);

  *Ubarlen = i;

 return AY_OK;
} /* ay_knots_unify */


/* ay_knots_mergenc:
 *  merge knots from Ubar[Ubarlen] into the knot vector of curve
 */
int
ay_knots_mergenc(ay_nurbcurve_object *curve, double *Ubar, int Ubarlen)
{
 double *X = NULL, *Ufoo = NULL, *U = NULL, *Qw = NULL;
 int r, ia, ib, done;

  /* sanity check */
  if(!curve || !Ubar)
    return AY_ENULL;

  if(!(X = calloc(Ubarlen, sizeof(double))))
   {
     return AY_EOMEM;
   }

  U = curve->knotv;

  /* find knots to insert */
  r = 0;
  ia = 0;
  ib = 0;
  done = AY_FALSE;
  while(!done)
    {
      /* was: if(Ubar[ib] == U[ia]) */
      if(fabs(Ubar[ib]-U[ia]) < AY_EPSILON)
	{
	  ib++;
	  ia++;
	}
      else
	{
	  X[r] = Ubar[ib];
	  r++;
	  ib++;
	}
      done = ((ia >= (curve->length+curve->order)) || (ib >= Ubarlen));
    }

  if(r == 0)
    {
      free(X); return AY_OK;
    }

  if(!(Ufoo = calloc((curve->length + curve->order + r),
		     sizeof(double))))
    {
      free(X); return AY_EOMEM;
    }

  if(!(Qw = calloc((curve->length + r+2)*4,
		   sizeof(double))))
    {
      free(X); free(Ufoo); return AY_EOMEM;
    }

  ay_nb_RefineKnotVectCurve(4, curve->length-1,
			    curve->order-1, curve->knotv, curve->controlv,
			    X, r-1, Ufoo, Qw);

  free(curve->knotv);
  curve->knotv = Ufoo;

  free(curve->controlv);
  curve->controlv = Qw;

  free(X);

  curve->length += r;

 return AY_OK;
} /* ay_knots_mergenc */


/* ay_knots_mergenp:
 *  merge knots from Ubar[Ubarlen] and Vbar[Vbarlen] into the
 *  respective knot vectors of patch
 */
int
ay_knots_mergenp(ay_nurbpatch_object *patch,
		 double *Ubar, int Ubarlen, double *Vbar, int Vbarlen)
{
 double *X = NULL, *Ufoo = NULL, *U = NULL, *Vfoo = NULL, *V = NULL;
 double *Qw = NULL;
 int r, ia, ib, done;

  /* sanity check */
  if(!patch || (!Ubar && !Vbar))
    return AY_EOMEM;

  if(Ubar)
    {
      if(!(X = calloc(Ubarlen, sizeof(double))))
	{
	  return AY_EOMEM;
	}

      U = patch->uknotv;

      /* find knots to insert */
      r = 0;
      ia = 0;
      ib = 0;
      done = AY_FALSE;
      while(!done)
	{
	  /* was: if(Ubar[ib] == U[ia]) */
	  if(fabs(Ubar[ib]-U[ia]) < AY_EPSILON)
	    {
	      ib++;
	      ia++;
	    }
	  else
	    {
	      X[r] = Ubar[ib];
	      r++;
	      ib++;
	    }
	  done = ((ia >= (patch->width+patch->uorder)) || (ib >= Ubarlen));
	}

      if(r == 0)
	{
	  free(X);
	}
      else
	{
	  if(!(Ufoo = calloc((patch->width + patch->uorder + r),
			     sizeof(double))))
	    {
	      free(X); return AY_EOMEM;
	    }

	  if(!(Qw = calloc((patch->width + r)*patch->height*4,
			   sizeof(double))))
	    {
	      free(X); free(Ufoo); return AY_EOMEM;
	    }

	  ay_nb_RefineKnotVectSurfU(4, patch->width-1, patch->height-1,
				    patch->uorder-1, patch->uknotv,
				    patch->controlv,
				    X, r-1, Ufoo, Qw);

	  free(patch->uknotv);
	  patch->uknotv = Ufoo;

	  free(patch->controlv);
	  patch->controlv = Qw;

	  free(X);
	  X = NULL;
	  Qw = NULL;

	  patch->width += r;
	} /* if */
    } /* if */

  if(Vbar)
    {
      X = NULL;
      if(!(X = calloc(Vbarlen, sizeof(double))))
	{
	  return AY_EOMEM;
	}

      V = patch->vknotv;

      /* find knots to insert */
      r = 0;
      ia = 0;
      ib = 0;
      done = AY_FALSE;
      while(!done)
	{
	  /* was: if(Vbar[ib] == V[ia]) */
	  if(fabs(Vbar[ib] - V[ia]) < AY_EPSILON)
	    {
	      ib++;
	      ia++;
	    }
	  else
	    {
	      X[r] = Vbar[ib];
	      r++;
	      ib++;
	    }
	  done = ((ia >= (patch->height+patch->vorder)) || (ib >= Vbarlen));
	}

      if(r == 0)
	{
	  free(X); return AY_OK;
	}

      if(!(Vfoo = calloc((patch->height + patch->vorder + r),
			 sizeof(double))))
	{
	  free(X); return AY_EOMEM;
	}

      if(!(Qw = calloc(patch->width*(patch->height + r)*4,
		       sizeof(double))))
	{
	  free(X); free(Vfoo); return AY_EOMEM;
	}

      ay_nb_RefineKnotVectSurfV(4, patch->width-1, patch->height-1,
				patch->vorder-1, patch->vknotv,
				patch->controlv,
				X, r-1, Vfoo, Qw);

      free(patch->vknotv);
      patch->vknotv = Vfoo;

      free(patch->controlv);
      patch->controlv = Qw;

      free(X);

      patch->height += r;
    } /* if */

 return AY_OK;
} /* ay_knots_mergenp */


/* ay_knots_getuminmax:
 *  get minimum/maximum u parametric values from knot vector <knotv>
 *  of object <o> with order <order> and number of knots (encodes
 *  size in u parametric dimension) <knots> from UMM tag (if present)
 *  or directly from the knot vector (else)
 */
int
ay_knots_getuminmax(ay_object *o, int order, int knots, double *knotv,
		    double *umin, double *umax)
{
 ay_tag *tag = NULL;
 int have_valid_umm_tag = AY_FALSE;

  /* sanity check */
  if(!o || !knotv || !umin || !umax)
    return AY_ENULL;

  if(o->tags)
    {
      tag = o->tags;
      while(tag)
	{
	  if(tag->type == ay_umm_tagtype)
	    {
	      if(sscanf(tag->val, "%lg,%lg", umin, umax) == 2)
		have_valid_umm_tag = AY_TRUE;
	    }
	  tag = tag->next;
	} /* while */
    } /* if */

  if(!have_valid_umm_tag)
    {
      *umin = knotv[order-1];
      *umax = knotv[knots-order];
    }

 return AY_OK;
} /* ay_knots_getuminmax */


/* ay_knots_getvminmax:
 *  get minimum/maximum v parametric values from knot vector <knotv>
 *  of object <o> with order <order> and number of knots (encodes
 *  size in v parametric dimension) <knots> from UMM tag (if present)
 *  or directly from the knot vector (else)
 */
int
ay_knots_getvminmax(ay_object *o, int order, int knots, double *knotv,
		    double *vmin, double *vmax)
{
 ay_tag *tag = NULL;
 int have_valid_vmm_tag = AY_FALSE;

  /* sanity check */
  if(!o || !knotv || !vmin || !vmax)
    return AY_ENULL;

  if(o->tags)
    {
      tag = o->tags;
      while(tag)
	{
	  if(tag->type == ay_vmm_tagtype)
	    {
	      if(sscanf(tag->val, "%lg,%lg", vmin, vmax) == 2)
		have_valid_vmm_tag = AY_TRUE;
	    }
	  tag = tag->next;
	} /* while */
    } /* if */

  if(!have_valid_vmm_tag)
    {
      *vmin = knotv[order-1];
      *vmax = knotv[knots-order];
    }

 return AY_OK;
} /* ay_knots_getvminmax */


/* ay_knots_setuminmax:
 *  add UMM tag with values from umin, umax to object o
 */
int
ay_knots_setuminmax(ay_object *o, double umin, double umax)
{
 int ay_status = AY_OK;
 ay_tag *tag = NULL, *nt = NULL;
 char buf[128], *ct = NULL;

  /* sanity check */
  if(!o)
    return AY_ENULL;

  sprintf(buf, "%g,%g", umin, umax);

  if(!(ct = calloc(strlen(buf)+1, sizeof(char))))
    return AY_EOMEM;

  strcpy(ct, buf);

  if(o->tags)
    {
      tag = o->tags;
      while(tag)
	{
	  if(tag->type == ay_umm_tagtype)
	    {
	      if(tag->val)
		free(tag->val);
	      tag->val = ct;
	      return AY_OK;
	    }

	  tag = tag->next;
	} /* while */
    } /* if */

  if(!(nt = calloc(1, sizeof(ay_tag))))
    {free(ct); return AY_EOMEM;}
  nt->type = ay_umm_tagtype;
  if(!(nt->name = calloc(4, sizeof(char))))
    {free(ct); free(nt); return AY_EOMEM;}
  strcpy(nt->name, "UMM");

  nt->val = ct;

  nt->next = o->tags;
  o->tags = nt;

 return ay_status;
} /* ay_knots_setuminmax */


/* ay_knots_setvminmax:
 *  add VMM tag with values from vmin, vmax to object o
 */
int
ay_knots_setvminmax(ay_object *o, double vmin, double vmax)
{
 int ay_status = AY_OK;
 ay_tag *tag = NULL, *nt = NULL;
 char buf[128], *ct = NULL;

  /* sanity check */
  if(!o)
    return AY_ENULL;

  sprintf(buf, "%g,%g", vmin, vmax);

  if(!(ct = calloc(strlen(buf)+1, sizeof(char))))
    return AY_EOMEM;

  strcpy(ct, buf);

  if(o->tags)
    {
      tag = o->tags;
      while(tag)
	{
	  if(tag->type == ay_vmm_tagtype)
	    {
	      if(tag->val)
		free(tag->val);
	      tag->val = ct;
	      return AY_OK;
	    }

	  tag = tag->next;
	} /* while */
    } /* if */

  if(!(nt = calloc(1, sizeof(ay_tag))))
    {free(ct); return AY_EOMEM;}
  nt->type = ay_vmm_tagtype;
  if(!(nt->name = calloc(4, sizeof(char))))
    {free(ct); free(nt); return AY_EOMEM;}
  strcpy(nt->name, "VMM");

  nt->val = ct;

  nt->next = o->tags;
  o->tags = nt;

 return ay_status;
} /* ay_knots_setvminmax */


/* ay_knots_coarsen:
 *
 */
int
ay_knots_coarsen(int order, int knotvlen, double *knotv, int count,
		 double **newknotv)
{
 int ay_status = AY_OK;
 double *nknotv = NULL;
 int i, a;

  /* sanity check */
  if(!knotv || !newknotv)
    return AY_ENULL;

  /* further parameter check */
 /*
 if(count >= (knotvlen-2*order)/2)
    return AY_ERROR;
 */

  if(!(nknotv = calloc(knotvlen-count, sizeof(double))))
    return AY_EOMEM;

  memcpy(nknotv, knotv, order*sizeof(double));
  a = order;
  for(i = order; i < (knotvlen-order-count); i++)
    {
      nknotv[i] = knotv[a];
      a += 2;
    }

  memcpy(&(nknotv[knotvlen-count-order]),
	 &(knotv[knotvlen-order]), order*sizeof(double));

  *newknotv = nknotv;

 return ay_status;
} /* ay_knots_coarsen */


/* ay_knots_chordparam:
 *  create chordal parameterization in <U[Ulen]> from points in <Q[Qlen]>
 */
int
ay_knots_chordparam(double *Q, int Qlen, int stride, double **U)
{
 double t, *vk = NULL, totallen = 0.0, *lens = NULL;
 int i, j;

  /* sanity check */
  if(!Q || !U)
    return AY_ENULL;

  /* get some memory */
  if(!(vk = calloc(Qlen, sizeof(double))))
    {
      return AY_EOMEM;
    }

  if(!(lens = calloc(Qlen-1, sizeof(double))))
    {
      free(vk);
      return AY_EOMEM;
    }

  /* compute total length and partial lengths */
  j = 0;
  for(i = 0; i < (Qlen-1); i++)
    {
      if((fabs(Q[j+stride] - Q[j])>AY_EPSILON) ||
	 (fabs(Q[j+stride+1] - Q[j+1])>AY_EPSILON) ||
	 (fabs(Q[j+stride+2] - Q[j+2])>AY_EPSILON))
	{
	  lens[i] = AY_VLEN((Q[j+stride] - Q[j]),
			    (Q[j+stride+1] - Q[j+1]),
			    (Q[j+stride+2] - Q[j+2]));
	}
      else
	{
	  lens[i] = 0.0;
	}

      totallen += lens[i];

      j += stride;
    }

  if(totallen < AY_EPSILON)
    {
      free(vk);
      free(lens);
      return AY_ERROR;
    }

  /* compute the chordal parameterization */
  vk[0] = 0.0;
  j = 0;
  t = 0.0;
  for(i = 1; i < (Qlen-1); i++)
    {
      t += lens[j]/totallen;
      vk[i] = t;

      /*printf("vk[%d]:%f\n",i,t);*/

      j++;
    }
  vk[Qlen-1] = 1.0;

  /* return result */
  *U = vk;

  /* clean up */
  free(lens);

 return AY_OK;
} /* ay_knots_chordparam */


/* ay_knots_centriparam:
 *  create centripetal parameterization in <U[Ulen]> from points in <Q[Qlen]>
 */
int
ay_knots_centriparam(double *Q, int Qlen, int stride, double **U)
{
 double t, *vk = NULL, totallen = 0.0, *lens = NULL;
 int i, j;

  /* sanity check */
  if(!Q || !U)
    return AY_ENULL;

  /* get some memory */
  if(!(vk = calloc(Qlen, sizeof(double))))
    {
      return AY_EOMEM;
    }

  if(!(lens = calloc(Qlen-1, sizeof(double))))
    {
      free(vk);
      return AY_EOMEM;
    }

  /* compute total length and partial lengths */
  j = 0;
  for(i = 0; i < (Qlen-1); i++)
    {
      if((fabs(Q[j+stride] - Q[j])>AY_EPSILON) ||
	 (fabs(Q[j+stride+1] - Q[j+1])>AY_EPSILON) ||
	 (fabs(Q[j+stride+2] - Q[j+2])>AY_EPSILON))
	{
	  lens[i] = sqrt(AY_VLEN((Q[j+stride] - Q[j]),
				 (Q[j+stride+1] - Q[j+1]),
				 (Q[j+stride+2] - Q[j+2])));
	}
      else
	{
	  lens[i] = 0.0;
	}

      totallen += lens[i];

      j += stride;
    }

  if(totallen < AY_EPSILON)
    {
      free(vk);
      free(lens);
      return AY_ERROR;
    }

  /* compute the centripetal parameterization */
  vk[0] = 0.0;
  j = 0;
  t = 0.0;
  for(i = 1; i < (Qlen-1); i++)
    {
      t += lens[j]/totallen;
      vk[i] = t;

      /*printf("vk[%d]:%f\n",i,t);*/

      j++;
    }
  vk[Qlen-1] = 1.0;

  /* return result */
  *U = vk;

  /* clean up */
  free(lens);

 return AY_OK;
} /* ay_knots_centriparam */


/* ay_knots_chordparamnp:
 *  create chordal parameterization in <U[Ulen]> from points in <Q[Qlen]>
 */
int
ay_knots_chordparamnp(int dir, double *Q, int width, int height, int stride,
		      double **U)
{
 double t, *vk = NULL, totallen = 0.0, *lens = NULL;
 int i, j, Ulen = 0;

  /* sanity check */
  if(!Q || !U)
    return AY_ENULL;

  if(dir)
    {
      ay_npt_avglensv(Q, width, height, stride, &lens);
      Ulen = height;
    }
  else
    {
      ay_npt_avglensu(Q, width, height, stride, &lens);
      Ulen = width;
    }

  /* get some memory */
  if(!(vk = calloc(Ulen, sizeof(double))))
    {
      return AY_EOMEM;
    }

  /* compute total length and partial lengths */
  j = 0;
  for(i = 0; i < Ulen-1; i++)
    {
      totallen += lens[i];

      j += stride;
    }

  if(totallen < AY_EPSILON)
    {
      free(vk);
      free(lens);
      return AY_ERROR;
    }

  /* compute the chordal parameterization */
  vk[0] = 0.0;
  j = 0;
  t = 0.0;
  for(i = 1; i < Ulen-1; i++)
    {
      t += lens[j]/totallen;
      vk[i] = t;

      j++;
    }
  vk[Ulen-1] = 1.0;

  /* return result */
  *U = vk;

  /* clean up */
  free(lens);

 return AY_OK;
} /* ay_knots_chordparamnp */


/* ay_knots_centriparamnp:
 *  create centripetal parameterization in <U[Ulen]> from points in <Q[Qlen]>
 */
int
ay_knots_centriparamnp(int dir, double *Q, int width, int height, int stride,
		      double **U)
{
 double t, *vk = NULL, totallen = 0.0, *lens = NULL;
 int i, j, Ulen = 0;

  /* sanity check */
  if(!Q || !U)
    return AY_ENULL;

  if(dir)
    {
      ay_npt_avglensv(Q, width, height, stride, &lens);
      Ulen = height;
    }
  else
    {
      ay_npt_avglensu(Q, width, height, stride, &lens);
      Ulen = width;
    }

  /* get some memory */
  if(!(vk = calloc(Ulen, sizeof(double))))
    {
      return AY_EOMEM;
    }

  /* compute total length and partial lengths */
  j = 0;
  for(i = 0; i < Ulen-1; i++)
    {
      if(fabs(lens[i])>AY_EPSILON)
	{
	  lens[i] = sqrt(lens[i]);
	  totallen += lens[i];
	}
    }

  if(totallen < AY_EPSILON)
    {
      free(vk);
      free(lens);
      return AY_ERROR;
    }

  /* compute the chordal parameterization */
  vk[0] = 0.0;
  j = 0;
  t = 0.0;
  for(i = 1; i < Ulen-1; i++)
    {
      t += lens[j]/totallen;
      vk[i] = t;

      j++;
    }
  vk[Ulen-1] = 1.0;

  /* return result */
  *U = vk;

  /* clean up */
  free(lens);

 return AY_OK;
} /* ay_knots_centriparamnp */


/* ay_knots_classify:
 *  
 */
int
ay_knots_classify(unsigned int order, double *U, unsigned int Ulen,
		  double eps)
{
 unsigned int i = 0;
 double d;
 int is_bspline = AY_TRUE;
 int is_clamped = AY_TRUE;
 int is_nurb = AY_TRUE;

  if(Ulen < 2 || !U)
    return AY_KTCUSTOM;

  d = U[1]-U[0];
  /* check all knot intervals */
  for(i = 1; i < (Ulen-1); i++)
    {
      if(fabs((U[i+1]-U[i])-d) > eps)
	{
	  is_bspline = AY_FALSE;
	  break;
	}
    }
  if(is_bspline)
    return AY_KTBSPLINE;

  /* check first order intervals */
  for(i = 1; i < order; i++)
    {
      if(fabs(U[i+1]-U[i]) > eps)
	{
	  is_clamped = AY_FALSE;
	  break;
	}
    }
  if(is_clamped)
    {
      /* check last order intervals */
      for(i = Ulen-2; i >= (Ulen-order); i--)
	{
	  if(fabs(U[i+1] - U[i]) > eps)
	    {
	      is_clamped = AY_FALSE;
	      break;
	    }
	}
    }
  if(is_clamped)
    {
      /* check intermediate intervals */
      d = U[order+1]-U[order];
      for(i = order+1; i < (Ulen-order); i++)
	{
	  if(fabs((U[i+1]-U[i]) - d) > eps)
	    {
	      is_nurb = AY_FALSE;
	      break;
	    }
	}
      if(is_nurb)
	{
	  return AY_KTNURB;
	}
    }

 return AY_KTCUSTOM;
} /* ay_knots_classify */


/* ay_knots_init:
 *  initialize the knots module
 */
int
ay_knots_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  /* sanity check */
  if(!interp)
    return AY_ENULL;

  /* register UMM tag type */
  ay_status = ay_tags_register(interp, ay_umm_tagname, &ay_umm_tagtype);
  if(ay_status)
    return ay_status;

  /* register VMM tag type */
  ay_status = ay_tags_register(interp, ay_vmm_tagname, &ay_vmm_tagtype);

 return ay_status;
} /* ay_knots_init */
