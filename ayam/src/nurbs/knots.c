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

/* knots.c */

/* ay_knots_createnp:
 *   create knot vectors according to
 *   the knot_type fields for patch *patch
 */
int
ay_knots_createnp(ay_nurbpatch_object *patch)
{
 int uorder = 0, vorder = 0, width = 0, height = 0;
 int uknot_count = 0, vknot_count = 0;
 int i = 0, j = 0, kts = 0;
 int start = 0;
 double *newuknotv = NULL, *newvknotv = NULL;

  uorder = patch->uorder;
  vorder = patch->vorder;
  width = patch->width;
  height = patch->height;
  uknot_count = width + uorder;
  vknot_count = height + vorder;

  /* calloc new knot-arrays */
  if(!(newuknotv = calloc(uknot_count, sizeof(double))))
    return AY_EOMEM;

  if(!(newvknotv = calloc(vknot_count, sizeof(double))))
    { free(newuknotv); return AY_EOMEM; }

  /* free old knot-arrays */
  if(patch->uknotv != NULL)
    free(patch->uknotv);

  patch->uknotv = newuknotv;

  if(patch->vknotv != NULL)
    free(patch->vknotv);

  patch->vknotv = newvknotv;

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
      j=0;
      start = (int)(floor(uknot_count/2));
      for(i=-start; i<start; i++)
	{
	  (patch->uknotv)[j] = i/fabs((double)(uknot_count-(uorder-1)));
	  j++;
	}
      if(fmod(uknot_count,2.0) > 0.0)
	(patch->uknotv)[j] = i/fabs((double)(uknot_count-(uorder-1)));
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

    case AY_KTCUSTOM:
      /* user specified own knot vertices */
      break;
    }

  switch(patch->vknot_type)
    {
    case AY_KTBEZIER:
      for(i=0; i<vknot_count/2; i++)
	(patch->vknotv)[i] = 0.0;
      for(i=vknot_count/2; i<vknot_count; i++)
	(patch->vknotv)[i] = 1.0;
      break;

    case AY_KTBSPLINE:
      j=0;
      start = (int)(floor(vknot_count/2));
      for(i=-start; i<start; i++)
	{
	  (patch->vknotv)[j] = i/fabs((double)(vknot_count-(vorder-1)));
	  j++;
	}
      if(fmod(vknot_count,2.0) > 0.0)
	(patch->vknotv)[j] = i/fabs((double)(vknot_count-(vorder-1)));
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

    case AY_KTCUSTOM:
      /* user specified own knot vertices */
      break;
    }

 return AY_OK;
} /* ay_knots_createnp */


/* ay_knots_createnc:
 */
int
ay_knots_createnc(ay_nurbcurve_object *curve)
{
 int order = 0, length = 0, knot_count = 0;
 int i = 0, j = 0, kts = 0;
 double *newknotv = NULL;

  order = curve->order;
  length = curve->length;

  knot_count = length + order;

  /* calloc new knot-arrays */
  if((newknotv = calloc(knot_count, sizeof(double))) == NULL)
    return AY_EOMEM;

  /* free old knot-array */
  if(curve->knotv) free(curve->knotv);

  curve->knotv = newknotv;

  /* fill knot-arrays */
  switch(curve->knot_type)
    {
    case AY_KTBEZIER:
      for(i=0; i<knot_count/2; i++)
	(curve->knotv)[i] = 0.0;
      for(i=knot_count/2; i<knot_count; i++)
	(curve->knotv)[i] = 1.0;
      break;

    case AY_KTBSPLINE:
      j=0;
      /*
      start = floor(knot_count/2);
      for(i=-start; i<start; i++)
	{
	  (curve->knotv)[j] = i/fabs((double)(knot_count-(order-1)));
	  j++;
	}
      if(fmod(knot_count,2.0) > 0.0)
	(curve->knotv)[j] = i/fabs((double)(knot_count-(order-1)));
      */
      for(i = 0; i < knot_count; i++)
	{
	  (curve->knotv)[i] = (double)i/knot_count;

	}
      break;

    case AY_KTNURB:
      for(i=0; i<order; i++)
	(curve->knotv)[i] = 0.0;
      j=1;
      kts = 1 + knot_count - (order*2);
      for(i=order; i<=knot_count-order; i++)
	(curve->knotv)[i] = j++/((double)kts);
      for(i=knot_count-order; i<knot_count; i++)
	(curve->knotv)[i] = 1.0;
      break;

    case AY_KTCUSTOM:
      /* user specified own knot vertices */
      break;
    }

 return AY_OK;
} /* ay_knots_createnc */


/* ay_knots_check:
 *   returns:
 *   o 0 - the knots are ok
 *   o 1 - too few knots
 *   o 2 - too much knots
 *   o 3 - knot multiplicity too high
 *   o 4 - decreasing knots
 */
int
ay_knots_check(int length, int order, int knot_count, double *knotv)
{
 int i, mult_count = 1;

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
	}
    }

 return 0;
} /* ay_knots_check */


/* ay_knots_rescaleknotv:
 *
 */
int
ay_knots_rescaleknotv(int n, double *knotv)
{
 double *tmpknv = NULL, min, max, len;
 int i;

  max = knotv[0];
  min = knotv[0];

  for(i = 0; i < n; i++)
    {
      if(knotv[i] > max)
	max = knotv[i];
     
      if(knotv[i] < min)
	min = knotv[i];
    }

  if(min < 0.0 && max > 0.0)
    len = max+fabs(min);
  else
    len = max - min;

  if(!(tmpknv = calloc(n, sizeof(double))))
    return AY_EOMEM;

  memcpy(tmpknv, knotv, n*sizeof(double));

  knotv[0] = 0.0;
  for(i = 1; i < n-1; i++)
    {
      knotv[i] = (tmpknv[i] - tmpknv[0])/len;
    }
  knotv[n-1] = 1.0;

  free(tmpknv);

 return AY_OK;
} /* ay_knots_rescaleknotv */


/* ay_knots_unify:
 *
 */
int
ay_knots_unify(double *Ua, int Ualen, double *Ub, int Ublen,
	       double **Ubar, int *Ubarlen)
{
 int i = 0, ia = 0, ib = 0, done = AY_FALSE;
 double t = 0.0, *U = NULL;

  if(!(U = calloc(Ualen+Ublen, sizeof(double))))
   {
     return AY_EOMEM;
   }

  while(!done)
    {
      if(Ua[ia] == Ub[ib])
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
	    }
	}
      U[i] = t;
      i++;
      if((ia >= Ualen || ib >= Ublen))
	done = AY_TRUE;
    }

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


/* ay_knots_merge:
 *
 */
int
ay_knots_merge(ay_nurbcurve_object *curve, double *Ubar, int Ubarlen)
{
 double *X = NULL, *Ufoo = NULL, *U = NULL, *Qw = NULL;
 int r, ia, ib, done;

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
      if(Ubar[ib] == U[ia])
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
} /* ay_knots_merge */


/* ay_knots_mergesurf:
 *
 */
int
ay_knots_mergesurf(ay_nurbpatch_object *patch,
		   double *Ubar, int Ubarlen, double *Vbar, int Vbarlen)
{
 double *X = NULL, *Ufoo = NULL, *U = NULL, *Vfoo = NULL, *V = NULL;
 double *Qw = NULL;
 int r, ia, ib, done;

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
	 if(Ubar[ib] == U[ia])
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
	 if(Vbar[ib] == V[ia])
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
			       patch->vorder-1, patch->vknotv, patch->controlv,
			       X, r-1, Vfoo, Qw);

     free(patch->vknotv);
     patch->vknotv = Vfoo;
  
     free(patch->controlv);
     patch->controlv = Qw;

     free(X);
	  
     patch->height += r;
   } /* if */

 return AY_OK;
} /* ay_knots_mergesurf */
