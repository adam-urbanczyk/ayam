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

/* nct.c NURBS curve tools */

/* ay_nct_create:
 *   create a NURBS curve
 */
int
ay_nct_create(int order, int length, int knot_type,
	      double *controlv, double *knotv,
	      ay_nurbcurve_object **curveptr)
{
 int ay_status = AY_OK;
 double *newcontrolv = NULL;
 ay_nurbcurve_object *curve = NULL;
  
  if(!(curve = calloc(1, sizeof(ay_nurbcurve_object))))
    return AY_EOMEM;
  
  curve->order = order;
  curve->length = length;
  curve->knot_type = knot_type;
  
  curve->controlv = controlv;
  if(!controlv)
    {
      if(!(newcontrolv = calloc(4*length, sizeof(double))))
	{
	  free(curve);
	  return AY_EOMEM;
	}

      curve->controlv = newcontrolv;

    }

  if(knot_type != AY_KTCUSTOM && knotv == NULL)
    { /* we need to create knots */
      ay_status = ay_knots_createnc(curve);
      if(ay_status)
	{
	  if(newcontrolv) free(newcontrolv);
	  free(curve);
	  return ay_status;
	}
    }
  else /* user specified own knots */
    {
      curve->knotv = knotv;
    }

  *curveptr = curve;

 return AY_OK;
} /* ay_nct_create */


/* ay_nct_clearmp:
 *  delete all mpoints from curve c
 */
void
ay_nct_clearmp(ay_nurbcurve_object *c)
{
  ay_mpoint_object *next = NULL, *p = NULL;

  if(!c)
    return;

  p = c->mpoints;

  while(p)
    {
      next = p->next;
      if(p->points)
	free(p->points);
      free(p);
      p = next;
    }

  c->mpoints = NULL;

 return;
} /* ay_nct_clearmp */


/* ay_nct_recreatemp:
 *  recreate mpoints from identical controlpoints
 */
int
ay_nct_recreatemp(ay_nurbcurve_object *c)
{
 int ay_status = AY_OK;
 ay_mpoint_object *p = NULL, *new = NULL;
 double *ta, **tmp = NULL;
 int found = AY_FALSE, a, b, i, j, count;

  if(!c)
    return AY_OK;

  ay_nct_clearmp(c);

  if(!c->createmp)
    return AY_OK;

  if(!(tmp = calloc(c->length, sizeof(double *))))
    return AY_EOMEM;

  a = 0;
  for(j = 0; j < c->length; j++)
    {
      ta = &(c->controlv[a]);

      /* count identical points */
      count = 0;
      b = 0;
      for(i = 0; i < c->length; i++)
	{
	  if(!memcmp(ta, &(c->controlv[b]), 4*sizeof(double)))
	    {
	      tmp[count] = &(c->controlv[b]);
	      count++;
	    }

	  b += 4;
	}

      /* create new mp, if it is not already there */
      if(count > 1)
	{
	  p = c->mpoints;
	  found = AY_FALSE;
	  while(p && !found)
	    {
	      if(!memcmp(ta, p->points[0], 4*sizeof(double)))
		found = AY_TRUE;

	      p = p->next;
	    }

	  if(!found)
	    {
	      if(!(new = calloc(1, sizeof(ay_mpoint_object))))
		return AY_EOMEM;
	      if(!(new->points = calloc(count, sizeof(double *))))
		return AY_EOMEM;
	      new->multiplicity = count;
	      memcpy(new->points, tmp, count*sizeof(double *));

	      new->next = c->mpoints;
	      c->mpoints = new;
	    }

	} /* if */

      a += 4;
    } /* for */


  free(tmp);

 return ay_status;
} /* ay_nct_recreatemp */


/* ay_nct_collapseselp:
 *  collapse selected points
 */
int
ay_nct_collapseselp(ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *c = NULL;
 ay_mpoint_object *new = NULL, *p = NULL, *t = NULL, **last = NULL;
 ay_point_object *selp = NULL;
 double *first = NULL;
 int count = 0, i, found = AY_FALSE;
 char fname[] = "collapseselp";

  if(!o)
    return AY_OK;

  if(!o->type == AY_IDNCURVE)
    return AY_ERROR;

  c = (ay_nurbcurve_object *)o->refine;

  selp = o->selp;

  /* count points to collapse */
  while(selp)
    {
      count++;
      selp = selp->next;
    }

  if((!o->selp) || (count < 2))
    {
      ay_error(AY_ERROR, fname, "Select (<t>ag) atleast two points first!");
      return AY_ERROR;
    }

  if(!(new = calloc(1,sizeof(ay_mpoint_object))))
    return AY_EOMEM;
  if(!(new->points = calloc(count, sizeof(double *))))
    return AY_EOMEM;

  /* fill mpoint */
  selp = o->selp;
  i = 0;
  first = selp->point;
  while(selp)
    {
      new->points[i] = selp->point;
      i++;
      if(selp->homogenous)
	memcpy(selp->point, first, 4*sizeof(double));
      else
	memcpy(selp->point, first, 3*sizeof(double));

      selp = selp->next;
    }
  new->multiplicity = count;

  /* find and delete all mpoints eventually
     containing a selected point */
  selp = o->selp;
  while(selp)
    {
      p = c->mpoints;
      last = &(c->mpoints);
      while(p)
	{
	  found = AY_FALSE;
	  for(i = 0; i < p->multiplicity; i++)
	    {
	      if(p->points[i] == selp->point)
		found = AY_TRUE;
	    }

	  if(found)
	    {
	      *last = p->next;
	      t = p->next;
	      free(p->points);
	      free(p);
	      p = t;
	    }
	  else
	    {
	      last = &(p->next);
	      p = p->next;
	    } /* if */
	} /* while */
      selp = selp->next;
    } /* while */

  /* link new mpoint */
  new->next = c->mpoints;
  c->mpoints = new;

 return ay_status;
} /* ay_nct_collapseselp */


/* ay_nct_explodemp:
 *  explode selected mpoints
 */
int
ay_nct_explodemp(ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *c = NULL;
 ay_mpoint_object *p = NULL, *t = NULL, **last = NULL;
 ay_point_object *selp = NULL;
 int found = AY_FALSE, i, err = AY_TRUE;
 char fname[] = "explodemp";

  if(!o)
    return AY_OK;

  if(!o->type == AY_IDNCURVE)
    return AY_ERROR;

  c = (ay_nurbcurve_object *)o->refine;

  selp = o->selp;

  if(!selp)
    {
      ay_error(AY_ERROR, fname, "Select (<t>ag) some multiple points first!");
      return AY_ERROR;
    }

  /* we simply delete all mpoints, that contain a selected point */
  while(selp)
    {      
      p = c->mpoints;
      last = &(c->mpoints);
      while(p)
	{
	  found = AY_FALSE;
	  for(i = 0; i < p->multiplicity; i++)
	    {
	      if(p->points[i] == selp->point)
		found = AY_TRUE;
	    }

	  if(found)
	    {
	      *last = p->next;
	      t = p->next;
	      free(p->points);
	      free(p);
	      p = t;
	      err = AY_FALSE;
	    }
	  else
	    {
	      last = &(p->next);
	      p = p->next;
	    } /* if */
	} /* while */
      selp = selp->next;
    } /* while */

  if(err)
    {
      ay_error(AY_ERROR, fname, "Select (<t>ag) some multiple points first!");
      ay_status = AY_ERROR;
    }

 return ay_status;
} /* ay_nct_explodemp */


/* ay_nct_resize:
 *  resize a NURBS curve
 */
int
ay_nct_resize(ay_nurbcurve_object *curve, int new_length)
{
 int ay_status = AY_OK;
 char fname[] = "ncurve_resize";
 int a, b, i, j;
 int *newpersec = NULL, new = 0;
 double *ncontrolv = NULL, v[3] = {0}, t = 0.0, *cv = NULL;

  if(new_length == curve->length)
    return ay_status;


  if(!(ncontrolv = calloc(4*new_length, sizeof(double))))
    return AY_EOMEM;

  if(new_length < curve->length)
    {
      a = 0;
      for(i=0; i<new_length; i++)
	{
	  memcpy(&ncontrolv[a], &(curve->controlv[a]), 4*sizeof(double));
	  a+=4;
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
	      if((cv[i*4] != cv[(i+1)*4]) ||
		 (cv[i*4+1] != cv[(i+1)*4+1]) ||
		 (cv[i*4+2] != cv[(i+1)*4+2]))
		{
		  if(!(curve->closed && (i>(curve->length-curve->order))))
		    if(new)
		      {
			(newpersec[i])++;
			new--;
		      } /* if */
		}  /* if */
	    } /* for */
	} /* while */

      a = 0;
      b = 0;
      for(i = 0; i < (curve->length-1); i++)
	{
	  memcpy(&ncontrolv[b], &(curve->controlv[a]), 4*sizeof(double));
	  b+=4;

	  if((cv[i*4] != cv[(i+1)*4]) ||
	     (cv[i*4+1] != cv[(i+1)*4+1]) ||
	     (cv[i*4+2] != cv[(i+1)*4+2]))
	    {
	      if(!(curve->closed && (i>(curve->length-curve->order))))
	      for(j = 1; j <= newpersec[i]; j++)
		{

		  v[0] = curve->controlv[a+4] - curve->controlv[a];
		  v[1] = curve->controlv[a+4+1] - curve->controlv[a+1];
		  v[2] = curve->controlv[a+4+2] - curve->controlv[a+2];

		  t = j/(newpersec[i]+1.0);
	      
		  AY_V3SCAL(v,t);

		  ncontrolv[b] = curve->controlv[a]+v[0];
		  ncontrolv[b+1] = curve->controlv[a+1]+v[1];
		  ncontrolv[b+2] = curve->controlv[a+2]+v[2];
		  ncontrolv[b+3] = 1.0;

		  b+=4;
		}
	    }

	  a+=4;	       
	}
      memcpy(&ncontrolv[(new_length-1)*4],
	     &(curve->controlv[(curve->length-1)*4]),
	     4*sizeof(double));

      free(newpersec);

    }

  free(curve->controlv);
  curve->controlv = ncontrolv;

  curve->length = new_length;

  if(curve->closed)
    {
      ay_status = ay_nct_close(curve);
      if(ay_status)
	ay_error(AY_ERROR, fname, "Could not close curve!");
    }

 return ay_status;
} /* ay_nct_resize */


/* ay_nct_close:
 *  close a NURBS curve
 */
int
ay_nct_close(ay_nurbcurve_object *curve)
{
 int ay_status = AY_OK;
 double *controlv = NULL, *end = NULL;
 int i;

 /* close curve */
 if(curve->length >= ((curve->order-1)*2))
   {
     controlv = curve->controlv;
     end = &(controlv[(curve->length-(curve->order-1))*4]);

     for(i = 0; i < (curve->order-1); i++)
       {
	 memcpy(end, controlv, 4*sizeof(double));
	 controlv += 4;
	 end += 4;
       }

     curve->closed = AY_TRUE;
   }
 else
   {
     curve->closed = AY_FALSE;
     return AY_ERROR;
   }

 return ay_status;
} /* ay_nct_close */


/* ay_nct_revert:
 *  revert a NURBS curve
 */
int
ay_nct_revert(ay_nurbcurve_object *curve)
{
 int i, j, kncount;
 double dtemp;
 double *newknotv = NULL;

  if(!curve)
    return AY_ENULL;

  /* revert control */
  j = (curve->length - 1)*4;
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
      

      dtemp = curve->controlv[j+3];
      curve->controlv[j+3] = curve->controlv[i+3];
      curve->controlv[i+3] = dtemp;
      
      i+=4;
      j-=4;
    } /* while */

  /* revert knots */
  if(curve->knot_type == AY_KTCUSTOM)
    {
      kncount = curve->length+curve->order;

      if(!(newknotv = calloc(kncount, sizeof(double))))
	return AY_EOMEM;

      newknotv[0] = curve->knotv[0];
      newknotv[kncount-1] = curve->knotv[kncount-1];
      j = kncount-2;
      for(i=1; i < (kncount-1); i++)
	{
	  newknotv[i] = newknotv[0]+(newknotv[kncount-1]-curve->knotv[j]);

	  j--;
	}
      free(curve->knotv);
      curve->knotv = newknotv;
    }

 return AY_OK;
} /* ay_nct_revert */


/* ay_nct_reverttcmd:
 *  revert a NURBS curve
 */
int
ay_nct_reverttcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;
 ay_nurbcurve_object *curve = NULL;
 char fname[] = "revert_ncurve";

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  while(sel)
    {
      src = sel->object;
      if(src->type != AY_IDNCURVE)
	{
	  ay_error(AY_ERROR, fname, "Object is not a NURBCurve!");
	}
      else
	{
	  if(src->selp)
	    {
	      ay_selp_clear(src);
	    }

	  curve = (ay_nurbcurve_object*)src->refine;

	  ay_status = ay_nct_revert(curve);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, "Could not revert NCurve!");
	    }

	  ay_status = ay_nct_recreatemp(curve);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Error re-creating MPoints!");
	    }

	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_reverttcmd */


/* ay_nct_refine:
 *  refine a NURBS curve by inserting knots at the right places,
 *  thus not changing the shape of the curve, but changing the
 *  knot type to "custom" all the time
 */
int
ay_nct_refine(ay_nurbcurve_object *curve, double *newknotv, int newknotvlen)
{
 double *X = NULL, *Ubar = NULL, *Qw = NULL, *knotv, *Q = NULL;
 int count = 0, i, j;
 char fname[] = "nct_refine";


  knotv = curve->knotv;
  if(newknotv)
    {
      X = newknotv;
    }

  if(curve->closed)
    {
      /* special case: curves marked closed;
       * we keep the p multiple points at the ends
       * and add new points into the other sections
       * taking care of other multiple points...
       */
      /* first, count new points */
      count = 0;
      Q = curve->controlv;
      for(i = curve->order-2; i < curve->length-(curve->order-1); i++)
	{
	  if((Q[i*4]   != Q[(i+1)*4]) ||
	     (Q[i*4+1] != Q[(i+1)*4+1]) ||
	     (Q[i*4+2] != Q[(i+1)*4+2]))
	    {
	      count++;
	    }
	}
	      
      if(count)
	{
	  /* alloc new control vector */
	  if(!(Qw = calloc((curve->length + count)*4, sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return AY_ERROR;
	    }

	  /* copy first p points */
	  memcpy(Qw, Q, (curve->order-1)*4*sizeof(double));

	  /* copy old & create new points */
	  j = curve->order-2;
	  for(i=curve->order-2; i<curve->length-(curve->order-1); i++)
	    {
	      memcpy(&(Qw[j*4]), &(Q[i*4]), 4*sizeof(double));

	      if((Q[i*4] != Q[(i+1)*4]) ||
		 (Q[i*4+1] != Q[(i+1)*4+1]) ||
		 (Q[i*4+2] != Q[(i+1)*4+2]))
		{
		  Qw[(j+1)*4] = Q[i*4] +
		    ((Q[(i+1)*4] - Q[i*4])/2.0);

		  Qw[(j+1)*4+1] = Q[i*4+1] +
		    ((Q[(i+1)*4+1] - Q[i*4+1])/2.0);

		  Qw[(j+1)*4+2] = Q[i*4+2] +
		    ((Q[(i+1)*4+2] - Q[i*4+2])/2.0);

		  Qw[(j+1)*4+3] = Q[i*4+3] +
		    ((Q[(i+1)*4+3] - Q[i*4+3])/2.0);

		  j++;
		}

	      j++;
	    }

	  /* copy last p points */
	  memcpy(&(Qw[(curve->length+count-(curve->order-1))*4]),
		 &(Q[(curve->length-(curve->order-1))*4]),
		 (curve->order-1)*4*sizeof(double));

	  curve->length += count;
	  free(curve->controlv);
	  curve->controlv = Qw;
	  ay_nct_recreatemp(curve);
	  curve->knot_type = AY_KTBSPLINE;
	  ay_knots_createnc(curve);
	} /* if count */
    }
  else
    {
      /* alloc X, new knotv & new controlv */
      if(!X)
	{
	  count = 0;
	  for(i = curve->order-1; i < curve->length; i++)
	    {
	      if(knotv[i] != knotv[i+1])
		count++;
	    }

	  if(!(X = calloc(count, sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	}

      if(!(Ubar = calloc((curve->length + curve->order + count), 
			 sizeof(double))))
	{
	  free(X);
	  ay_error(AY_EOMEM, fname, NULL);
	  return TCL_OK;
	}
      if(!(Qw = calloc((curve->length + count+2)*4, sizeof(double))))
	{
	  free(X); free(Ubar);
	  ay_error(AY_EOMEM, fname, NULL);
	  return TCL_OK;
	}

      if( newknotv == NULL )
	{
	  /* fill X (contains just the new u values) */
	  count = 0;

	  for(i = curve->order-1; i < curve->length; i++)
	    {
	      if(knotv[i] != knotv[i+1])
		{
		  X[count] = knotv[i]+((knotv[i+1]-knotv[i])/2.0);
		  count++;
		}
	    }
	}
      else
	{
	  count = newknotvlen;
	}

      /* fill Ubar & Qw */
      ay_nb_RefineKnotVectCurve(4, curve->length-1, curve->order-1,
				curve->knotv, curve->controlv,
				X, count-1, Ubar, Qw);

      free(curve->knotv);
      curve->knotv = Ubar;

      free(curve->controlv);
      curve->controlv = Qw;

      if(!newknotv)
	{
	  free(X);
	}

      curve->length += count;

      ay_nct_recreatemp(curve);

    } /* if curve closed */

 return AY_OK;
} /* ay_nct_refine */


/* ay_nct_refinetcmd:
 *
 */
int
ay_nct_refinetcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_nurbcurve_object *curve = NULL;
 int aknotc = 0, i, count = 0;
 char fname[] = "refine";
 double *X = NULL;
 char **aknotv;

  if(argc > 1)
    {
      Tcl_SplitList(interp, argv[1], &aknotc, &aknotv);

      if(!(X = calloc(aknotc, sizeof(double))))
	{
	  ay_error(AY_EOMEM,fname,NULL);
	  Tcl_Free((char *) aknotv);
	  return TCL_OK;
	}

      for(i = 0; i < aknotc; i++)
	{
	  Tcl_GetDouble(interp,aknotv[i],&X[i]);
	} /* for */

      count = aknotc;

      Tcl_Free((char *) aknotv);
    }


  while(sel)
    {
      o = sel->object;

      if(o->type == AY_IDNCURVE)
	{
	  if(o->selp)
	    ay_selp_clear(o);
	  curve = (ay_nurbcurve_object *)o->refine;
	  ay_status = ay_nct_refine(curve, X, aknotc);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR,fname, "refine operation failed");
	    }
	}
      else
	{
	  ay_error(AY_ERROR, fname, "object is not a NURBCurve");
	}

      sel = sel->next; 
    } /* while */

  if(X)
    free(X);

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_refinetcmd */


/* ay_nct_clamp:
 *
 */
int
ay_nct_clamp(ay_nurbcurve_object *curve)
{
 int ay_status = AY_OK;
 double *newcontrolv = NULL, *newknotv = NULL;
 double u;
 int stride, r, k, s, nq = 0;

  stride = 4;  

  /* clamp start */
  k = curve->order-1;
  u = curve->knotv[k];
  s = 0;
  r = curve->order - 1;
  curve->length += r;

  newcontrolv = NULL;
  if(!(newcontrolv = calloc(curve->length*stride, sizeof(double))))
    return AY_EOMEM;

  newknotv = NULL;	  
  if(!(newknotv = calloc(curve->length+curve->order, sizeof(double))))
    return AY_EOMEM;


  ay_status = ay_nb_CurveInsertKnot4D(curve->length-r-1,
		curve->order-1, curve->knotv, curve->controlv, u, k,
		s, r, &nq, newknotv, newcontrolv);

  free(curve->controlv);
  curve->controlv = newcontrolv;

  free(curve->knotv);
  curve->knotv = newknotv;

  /* clamp end */
  k = curve->length;
  u = curve->knotv[k];
  k--;
  s = 0;
  r = curve->order - 1;
  curve->length += r;

  newcontrolv = NULL;
  if(!(newcontrolv = calloc(curve->length*stride, sizeof(double))))
    return AY_EOMEM;

  newknotv = NULL;	  
  if(!(newknotv = calloc(curve->length+curve->order, sizeof(double))))
    return AY_EOMEM;


  ay_status = ay_nb_CurveInsertKnot4D(curve->length-r-1,
		       curve->order-1, curve->knotv, curve->controlv, u, k,
		       s, r, &nq, newknotv, newcontrolv);

  free(curve->controlv);
  curve->controlv = newcontrolv;

  free(curve->knotv);
  curve->knotv = newknotv;

  /* create new controlv, knotv, discarding the first p and last p knots */
  curve->length -= (curve->order-1)*2;
  newcontrolv = NULL;
  if(!(newcontrolv = calloc(curve->length*stride, sizeof(double))))
    return AY_EOMEM;
  
  newknotv = NULL;
  if(!(newknotv = calloc(curve->length+curve->order, sizeof(double))))
    return AY_EOMEM;

  memcpy(newcontrolv, &(curve->controlv[(curve->order-1)*stride]),
	 curve->length*stride*sizeof(double));

  memcpy(newknotv, &(curve->knotv[curve->order-1]),
	 (curve->length+curve->order)*sizeof(double));

  free(curve->controlv);
  curve->controlv = newcontrolv;

  free(curve->knotv);
  curve->knotv = newknotv;

 return AY_OK;
} /* ay_nct_clamp */


/* ay_nct_clamptcmd:
 *
 */
int
ay_nct_clamptcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_nurbcurve_object *curve = NULL;
 double *knotv, u;
 int count_start, count_end, i, j;
 char fname[] = "clamp";

  /*
  if(argc >= 2)
    Tcl_GetInt(interp, argv[1], &t);
  */
  while(sel)
  {

    if(sel->object)
      {
	if(sel->object->type == AY_IDNCURVE)
	  {
	    if(sel->object->selp)
	      ay_selp_clear(sel->object);

	    curve = (ay_nurbcurve_object *)sel->object->refine;
	    knotv = curve->knotv;

	    if((curve->knot_type == AY_KTNURB) ||
	       (curve->knot_type == AY_KTBEZIER))
	      return TCL_OK;
	    
	    if(curve->knot_type == AY_KTCUSTOM)
	      {
		count_start = 0;
		count_end = 0;
		u = knotv[0];
		for(i = 1; i < curve->order; i++)
		 if(u == knotv[i])
		   count_start++;

		j = curve->length+curve->order-1;
		u = knotv[j];
		for(i = j-1; i >= curve->length; i--)
		 if(u == knotv[i])
		   count_end++;

		/*
		if(count_start || count_end)
		  {
		    ay_error(AY_ERROR, fname,
			       "cannot clamp this curve");
		    return TCL_OK;
		  }
		*/
	      }


	    ay_status = ay_nct_clamp(curve);

	    if(ay_status)
	      {
		ay_error(ay_status, fname, "clamp operation failed");
		return TCL_OK;
	      }

	    curve->knot_type = AY_KTCUSTOM;

	    ay_status = ay_nct_recreatemp(curve);

	  }
	else
	  {
	    ay_error(AY_ERROR, fname, "object is not a NURBCurve");
	  }
    }

  sel = sel->next;
  }

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_clamptcmd */


/* ay_nct_elevate:
 *  elevate NURBS curve <curve> to the new order <new_order>
 */
int
ay_nct_elevate(ay_nurbcurve_object *curve, int new_order)
{
 int ay_status = AY_OK;
 int i, j, a, b, clamp_me;
 double u, *Uh = NULL, *Qw = NULL, *realQw = NULL, *realUh = NULL;
 int t = 1, nh = 0;
 char fname[] = "elevate";

  if(curve->order >= new_order)
    return AY_OK;
  else
    t = new_order - curve->order;

 /* clamp the curve? */
  clamp_me = AY_FALSE;

  if(curve->knot_type == AY_KTBSPLINE)
    {
      clamp_me = AY_TRUE;
    }
  else
    {
      if(curve->knot_type == AY_KTCUSTOM)
	{
	  a = 1;
	  u = curve->knotv[0];
	  for(i = 1; i < curve->order; i++)
	    if(u == curve->knotv[i])
	      a++;

	  j = curve->length+curve->order-1;
	  b = 1;
	  u = curve->knotv[j];
	  for(i = j; i >= curve->length; i--)
	    if(u == curve->knotv[i])
	      b++;

	  if((a < curve->order) || (b < curve->order))
	    {
	      clamp_me = AY_TRUE;
	    } /* if */
	} /* if */
    } /* if */
	  
  if(clamp_me)
    {
      ay_status = ay_nct_clamp(curve);
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "clamp operation failed");
	} /* if */
    } /* if */

  /* alloc new knotv & new controlv */
  if(!(Uh = calloc((curve->length + curve->length*t +
		    curve->order + t), 
		   sizeof(double))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }
  if(!(Qw = calloc((curve->length + curve->length*t)*4,
		   sizeof(double))))
    {
      free(Uh);
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  /* fill Uh & Qw */
  ay_status = ay_nb_DegreeElevateCurve(4, curve->length-1,
				       curve->order-1, curve->knotv,
				       curve->controlv, t, &nh, Uh, Qw);

  if(ay_status)
    {
      ay_error(ay_status,fname,"degree elevation failed");
      free(Uh); free(Qw); return AY_ERROR;
    }

  if(!(realQw = realloc(Qw, nh*4* sizeof(double))))
    {
      ay_error(AY_ERROR, fname, "Memory may have leaked!");
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  if(!(realUh = realloc(Uh, (nh+curve->order+t)*sizeof(double))))
    {
      ay_error(AY_ERROR, fname, "Memory may have leaked!");
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  free(curve->knotv);
  curve->knotv = realUh;

  free(curve->controlv);
  curve->controlv = realQw;

  curve->knot_type = AY_KTCUSTOM;
	  
  curve->order += t;
	  
  curve->length = nh;

  Qw = NULL;
  Uh = NULL;
  realQw = NULL;
  realUh = NULL;

  ay_nct_recreatemp(curve);

 return AY_OK;
} /* ay_nct_elevate */


/* ay_nct_elevatetcmd:
 *
 */
int
ay_nct_elevatetcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_nurbcurve_object *curve = NULL;
 int i, j, a, b, clamp_me;
 double u, *Uh = NULL, *Qw = NULL, *realQw = NULL, *realUh = NULL;
 int t = 1, nh = 0;
 char fname[] = "elevate";

  if(argc >= 2)
    Tcl_GetInt(interp, argv[1], &t);

  while(sel)
    {
      if(sel->object->type == AY_IDNCURVE)
	{
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  curve = (ay_nurbcurve_object *)sel->object->refine;

	  /* clamp the curve? */
	  clamp_me = AY_FALSE;

	  if(curve->knot_type == AY_KTBSPLINE)
	    {
	      clamp_me = AY_TRUE;
	    }
	  else
	    {
	      if(curve->knot_type == AY_KTCUSTOM)
		{
		  a = 1;
		  u = curve->knotv[0];
		  for(i = 1; i < curve->order; i++)
		    if(u == curve->knotv[i])
		      a++;

		  j = curve->length+curve->order-1;
		  b = 1;
		  u = curve->knotv[j];
		  for(i = j; i >= curve->length; i--)
		    if(u == curve->knotv[i])
		      b++;

		  if((a < curve->order) || (b < curve->order))
		    {
		      clamp_me = AY_TRUE;
		    }
		}
	    }
	  
	  if(clamp_me)
	    {
	      ay_status = ay_nct_clamp(curve);
	      if(ay_status)
		{
		  ay_error(AY_ERROR, fname, "clamp operation failed");
		}
	    }

	  /* alloc new knotv & new controlv */
	  if(!(Uh = calloc((curve->length + curve->length*t +
			    curve->order + t), 
			     sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  if(!(Qw = calloc((curve->length + curve->length*t)*4,
			   sizeof(double))))
	    {
	      free(Uh);
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }

	  /* fill Uh & Qw */
	  ay_status = ay_nb_DegreeElevateCurve(4, curve->length-1,
			curve->order-1, curve->knotv,
			curve->controlv, t, &nh, Uh, Qw);

	  if(ay_status)
	    {
	      ay_error(ay_status,fname,"degree elevation failed");
	      free(Uh); free(Qw); return TCL_OK;
	    }

	  if(!(realQw = realloc(Qw, nh*4* sizeof(double))))
	    {
	      ay_error(AY_ERROR, fname, "Memory may have leaked!");
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }

	  if(!(realUh = realloc(Uh, (nh+curve->order+t)*sizeof(double))))
	    {
	      ay_error(AY_ERROR, fname, "Memory may have leaked!");
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }

	  free(curve->knotv);
	  curve->knotv = realUh;

	  free(curve->controlv);
	  curve->controlv = realQw;

	  curve->knot_type = AY_KTCUSTOM;
	  
	  curve->order += t;
	  
	  curve->length = nh;

	  Qw = NULL;
	  Uh = NULL;
	  realQw = NULL;
	  realUh = NULL;

	  ay_nct_recreatemp(curve);

	}
      else
	{
	  ay_error(AY_ERROR, fname, "object is not a NURBCurve");

	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_elevatetcmd */


/* ay_nct_insertkntcmd:
 *
 */
int
ay_nct_insertkntcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;
 ay_nurbcurve_object *curve = NULL;
 double u, *knots = NULL, *newcontrolv = NULL, *newknotv = NULL;
 int stride = 4, i, k = 0, s = 0, r = 0, nq = 0;
 char fname[] = "insertkn";

  if(argc < 3)
    {
      ay_error(AY_EARGS, fname, "u r");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  while(sel)
    {
      src = sel->object;
      if(src->type != AY_IDNCURVE)
	{
	  ay_error(AY_ERROR, fname, "Object is not a NURBCurve!");
	}
      else
	{
	  curve = (ay_nurbcurve_object*)src->refine;
	  knots = curve->knotv;

	  Tcl_GetDouble(interp, argv[1], &u);

	  if(u<knots[curve->order-1] || u>knots[curve->length])
	    {
	      ay_error(AY_ERROR, fname, "Parameter u out of range.");
	      return TCL_OK;
	    }

	  i = 0; k = 0;

	  k = ay_nb_FindSpanMult(curve->length-1, curve->order-1, u,
				 knots, &s);

	  Tcl_GetInt(interp, argv[2], &r);

	  if(curve->order < r+s)
	    {
	      ay_error(AY_ERROR, fname,
			 "Knot insertion leads to illegal knot sequence.");
	      return TCL_OK;
	    }

	  curve->length += r;

	  if(!(newcontrolv = calloc(curve->length*stride, sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  if(!(newknotv = calloc(curve->length+curve->order, sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }

	  ay_status = ay_nb_CurveInsertKnot4D(curve->length-r-1,
			  curve->order-1, curve->knotv, curve->controlv, u, k,
			  s, r, &nq, newknotv, newcontrolv);

	  free(curve->controlv);
	  curve->controlv = newcontrolv;

	  free(curve->knotv);
	  curve->knotv = newknotv;
	  curve->knot_type = AY_KTCUSTOM;

	  ay_nct_recreatemp(curve);

	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_insertkntcmd */


/* ay_nct_collapsetcmd:
 *
 */
int
ay_nct_collapsetcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 char fname[] = "collNC";

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  while(sel)
    {
      if(sel->object->type == AY_IDNCURVE)
	{
	  ay_status = ay_nct_collapseselp(sel->object);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, "Collapse operation failed!");
	    }

	  if(sel->object->selp)
	    {
	      ay_selp_clear(sel->object);
	    }
	}
      else
	{
	  ay_error(AY_ERROR, fname, "Object is not a NURBCurve!");
	}

      sel = sel->next;
    } /* while */
  
  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_collapsetcmd */


/* ay_nct_explodetcmd:
 *
 */
int
ay_nct_explodetcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 char fname[] = "explNC";

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  while(sel)
    {
      if(sel->object->type == AY_IDNCURVE)
	  {
	    ay_status = ay_nct_explodemp(sel->object);
	    if(ay_status)
	      {
		ay_error(ay_status,fname,"Explode operation failed!");
	      }

 	    if(sel->object->selp)
	      ay_selp_clear(sel->object);

	  }
	else
	  {
	    ay_error(AY_ERROR, fname, "Object is not a NURBCurve!");
	    return TCL_OK;
	  }

   sel = sel->next;
  } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_explodetcmd */



/* ay_nct_findu:
 *  transforms the window coordinates (winX, winY)
 *  to the corresponding parametric value u
 *  on the NURB curve o
 *  This function needs a valid OpenGL rendering context!
 */
int
ay_nct_findu(struct Togl *togl, ay_object *o,
	     double *winX, double *winY, double *u)
{
 int ay_status = AY_OK;
 /*  int width = Togl_Width(togl);*/
 int height = Togl_Height(togl);
 GLint viewport[4];
 GLdouble modelMatrix[16], projMatrix[16], winx, winy;
 GLfloat winz = 0.0f;
 double m[16];
 GLfloat pixel1[3] = {0.9f,0.9f,0.9f}, pixel[3] = {0.0f,0.0f,0.0f};
 ay_nurbcurve_object *c = NULL;
 int dx[25] = {0,1,1,0,-1,-1,-1,0,1, 2,2,2,1,0,-1,-2,-2,-2,-2,-2,-1,0,1,2,2};
 int dy[25] = {0,0,-1,-1,-1,0,1,1,1, 0,-1,-2,-2,-2,-2,-2,-1,0,1,2,2,2,2,2,1};
 int found, i = 0, j = 0, k = 0, /*maxtry = 1000,*/ stride, samples = 10;
 int starti = 0, endi = 0;
 double point[3] = {0}/*, guess = 0.0, e1 = 0.05, e2 = 0.05*/;
 double distance = 0.0, min_distance = 0.0;
 double *cp = NULL, U[10/* XXXX samples! */] = {0}, startu, endu;

  if(!o)
    return AY_ENULL;

  if(!o->type == AY_IDNCURVE)
    return AY_ERROR;

  c = (ay_nurbcurve_object *)o->refine;

  pixel1[0] = (float)ay_prefs.ser;
  pixel1[1] = (float)ay_prefs.seg;
  pixel1[2] = (float)ay_prefs.seb;
  
  /*
  winx = winX;
  winy = height - winY;
  */

  /*
   * first, we try to find a point on the curve in window coordinates;
   * we do this by comparing colors of rendered pixels
   */
  found = AY_FALSE;
  while(!found)
    {
      winx = *winX+dx[i];
      winy = height-*winY-dy[i];
      i++;

      if(i > 24)
	{
	  return AY_ERROR;
	}

      glReadPixels((GLint)winx,(GLint)winy,1,1,
		   GL_RGB,GL_FLOAT,&pixel);

      if((pixel1[0] <= pixel[0]) && (pixel1[1] <= pixel[1]) &&
	 (pixel1[2] <= pixel[2]))
	{
	  found = AY_TRUE;
	}
    }

  /* get winz */
  glReadPixels((GLint)winx,(GLint)winy,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&winz);

  /* get object coordinates */
  glGetIntegerv(GL_VIEWPORT, viewport);

  glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

  glMatrixMode (GL_MODELVIEW);
  glPushMatrix();
   ay_trafo_getall(ay_currentlevel->next);
   glTranslated(o->movx, o->movy, o->movz);
   ay_quat_torotmatrix(o->quat, m);
   glMultMatrixd(m);

   glScaled (o->scalx, o->scaly, o->scalz);
   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
   gluUnProject(winx, winy, (GLdouble)winz, modelMatrix, projMatrix, viewport,
	        &(point[0]), &(point[1]), &(point[2]));
  glPopMatrix ();

  /* get guess */
  stride = 4;
  if(!(cp = calloc(samples*stride, sizeof(double))))
    return TCL_OK;

  startu = c->knotv[c->order-1];
  endu = c->knotv[c->length];
  for(k = 0; k < 3; k++)
    {
      U[0] = startu;

      for(i = 1; i < samples; i++)
	{
	  U[i] = U[i-1]+(endu - startu)/samples;
	}

      for(i = 0; i < samples; i++)
       {
	 ay_status = ay_nb_CurvePoint4D(c->length-1, c->order-1, c->knotv,
					c->controlv, U[i], &(cp[i*stride]));

       }

      min_distance = DBL_MAX;
      j = 0;
      for(i = 0; i < samples; i++)
	{
	  distance = AY_VLEN((point[0] - cp[j]),
			     (point[1] - cp[j+1]),
			     (point[2] - cp[j+2]));

	  if(distance < min_distance)
	    {
	      *u = U[i];
	      starti = i-1;
	      endi = i+1;
	      min_distance = distance;
	    }

	  j += stride;
	} /* for */

      if(starti < 0)
	{
	  startu = U[0];
	}
      else
	{
	  startu = U[starti];
	}

      if(endi >= samples)
	{
	  endu = U[samples-1];
	}
      else
	{
	  endu = U[endi];
	}
    } /* for */

  /* unfortunately, ay_nb_FinduFromPoint() does not converge well...

  if(guess <= c->knotv[c->order-1])
    guess = U[1];

  if(guess > c->knotv[c->length])
    guess = c->knotv[c->length];


  ay_nb_FinduFromPoint(c, point, guess, maxtry, e1, e2, u);


  if((*u <= c->knotv[c->order-1]) || (*u > c->knotv[c->length]))
    { free(cp); return AY_ERROR; }
  */

  *winX = winx;
  *winY = winy;

 free(cp);

 return ay_status;
} /* ay_nct_findu */


/* ay_nct_finducb:
 *  Togl callback to implement find parametric value
 *  u for a picked point on a NURBS curve
 */
int
ay_nct_finducb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 Tcl_Interp *interp = Togl_Interp(togl);
int height = Togl_Height(togl);
 double winX = 0.0, winY = 0.0;
 static int fvalid = AY_FALSE;
 static double fX = 0.0, fY = 0.0; 
 char fname[] = "findU_cb";
 double u = 0.0;
 Tcl_Obj *to = NULL, *ton = NULL;
 char cmd[] = "puts $u";

  if(argc > 2)
    {
      if(!strcmp(argv[2],"-start"))
	{
	  /*	  view->drawhandles = AY_FALSE;
		  display_cb(togl);*/
	  fvalid = AY_FALSE;
	  return TCL_OK;
	}
      if(!strcmp(argv[2],"-end"))
	{

	  /* draw cross */
	  if(fvalid)
	    {
	      view->markx = fX;
	      view->marky = height-fY;
	      view->drawmarker = AY_TRUE;
	    }

	  fvalid = AY_FALSE;
	  return TCL_OK;
	}
    }
  else
    return TCL_OK;

  if(ay_selection)
    {
      if(ay_selection->object->type != AY_IDNCURVE)
	{
	  ay_error(AY_ERROR,fname,"Object is not a NURB curve!");
	  return TCL_OK;
	}
      /*
      ay_selp_clear(ay_selection->object);
      */

      Tcl_GetDouble(interp, argv[2], &winX);
      Tcl_GetDouble(interp, argv[3], &winY);

      ay_status = ay_nct_findu(togl, ay_selection->object,
			       &winX, &winY, &u);

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Could not find point on curve!");
	  return TCL_OK;
	}
      ton = Tcl_NewStringObj("u",-1);
      to = Tcl_NewDoubleObj(u);
      Tcl_ObjSetVar2(interp,ton,NULL,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      Tcl_Eval(interp, cmd);
      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

      ay_status = ay_notify_parent();

      fvalid = AY_TRUE;
      fX = winX;
      fY = winY;
    }
  else
    {
      ay_error(AY_ENOSEL, fname, NULL);
    }

 return TCL_OK;
} /* ay_nct_finducb */


/* ay_nct_split:
 *  
 *  
 */
int
ay_nct_split(ay_object *src, double u)
{
 int ay_status = AY_OK;
 ay_object *new = NULL;
 ay_nurbcurve_object *curve = NULL;
 ay_nurbcurve_object *nc1 = NULL, *nc2 = NULL;
 double *knots = NULL, *newcontrolv = NULL, *newknotv = NULL;
 int stride = 3, k = 0, r = 0, s = 0, nq = 0, nc1len = 0;
 char fname[] = "split";


 if(!src)
   return AY_ENULL;

 if(src->type != AY_IDNCURVE)
   {
     ay_error(AY_ERROR, fname, "Object is not a NURBCurve!");
     return AY_ERROR;
   }
 else
   {
     curve = (ay_nurbcurve_object*)src->refine;
     stride = 4;
     knots = curve->knotv;

     if((u <= knots[0/*curve->order-2*/]) || (u >= knots[curve->length]))
       {
	 ay_error(AY_ERROR, fname, "Parameter u out of range!");
	 return AY_ERROR;
       }

     k = 0;

     k = ay_nb_FindSpanMult(curve->length-1, curve->order-1, u,
			      knots, &s);

     r = curve->order-1-s;
	  

     curve->length += r;

     if(r != 0)
       {
	 newcontrolv = NULL;
	 if(!(newcontrolv = calloc(curve->length*stride, sizeof(double))))
	   return AY_EOMEM;
	 newknotv = NULL;	  
	 if(!(newknotv = calloc(curve->length+curve->order,
				sizeof(double))))
	   return AY_EOMEM;

	 ay_status = ay_nb_CurveInsertKnot4D(curve->length-r-1,
			curve->order-1, curve->knotv, curve->controlv, u, k,
			s, r, &nq, newknotv, newcontrolv);

	 free(curve->controlv);
	 curve->controlv = newcontrolv;

	 free(curve->knotv);
	 curve->knotv = newknotv;
       }

     curve->knot_type = AY_KTCUSTOM;
     /* create two new curves */
     nc1 = curve;
     nc1->closed = AY_FALSE;
     ay_status = ay_object_copy(src, &new);

     ay_status = ay_object_link(new);  


       if(r != 0)
	 nc1len = k - (nc1->order-1) + 1 + (curve->order-1-s+r-1)/2 + 1;
       else
	 nc1len = k - (nc1->order-1) + 1;

     nc2 = (ay_nurbcurve_object*)new->refine;
     nc2->length = (nc1->length+1) - nc1len;
     nc1->length = nc1len;

     newcontrolv = NULL;
     if(!(newcontrolv = calloc(nc1->length*stride, sizeof(double))))
       return AY_EOMEM;
     newknotv = NULL;	  
     if(!(newknotv = calloc(nc1->length+nc1->order, sizeof(double))))
       return AY_EOMEM;

     memcpy(newcontrolv,nc1->controlv,nc1->length*stride*sizeof(double));

     memcpy(newknotv,nc1->knotv,(nc1->length+nc1->order)*sizeof(double));

     free(nc2->controlv);
     nc2->controlv = NULL;
     free(nc2->knotv);
     nc2->knotv = NULL;

     if(!(nc2->controlv = calloc(nc2->length*stride, sizeof(double))))
       return AY_EOMEM;

     if(!(nc2->knotv = calloc(nc2->length+nc2->order, sizeof(double))))
       return AY_EOMEM;

     memcpy(nc2->controlv,&(nc1->controlv[(nc1->length-1)*stride]),
	    nc2->length*stride*sizeof(double));

     memcpy(nc2->knotv,&(nc1->knotv[nc1->length-1]),
	    (nc2->length+nc2->order)*sizeof(double));

     free(nc1->controlv);
     nc1->controlv = newcontrolv;
     free(nc1->knotv);
     nc1->knotv = newknotv;

     ay_nct_recreatemp(nc1);
     ay_nct_recreatemp(nc2);

     new->selected = AY_FALSE;
     new->modified = AY_TRUE;
     src->modified = AY_TRUE;

   } /* if */

 return AY_OK;
} /* ay_nct_split */


/* ay_nct_splittcmd:
 *  
 *  
 */
int
ay_nct_splittcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 double u = 0.0;
 char fname[] = "split";

  if(argc<2)
    {
      ay_error(AY_EARGS, fname, "u");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  Tcl_GetDouble(interp, argv[1], &u);

  while(sel)
    {
      if(sel->object)
	{
	  if(sel->object->selp)
	    {
	      ay_selp_clear(sel->object);
	    }

	  if(sel->object->type == AY_IDNCURVE)
	    {
	      ay_status = ay_nct_split(sel->object, u);

	      if(ay_status)
		{
		  ay_error(ay_status, fname, NULL);
		  return TCL_OK;
		} /* if */
	    }
	  else
	    {
	      ay_error(AY_ERROR, fname,
			 "object is not a NURBCurve");
	      return TCL_OK;
	    } /* if */
	} /* if */
      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_splittcmd */


/* ay_nct_concattcmd:
 *  
 *  
 */
int
ay_nct_concattcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status;
 double *newcontrolv = NULL;
 double *controlv1, *controlv2;
 int ktype = AY_KTNURB;
 int i = 0, a = 0, b = 0;
 double m1[16], m2[16];
 ay_list_object *sel = ay_selection;
 ay_nurbcurve_object *nc1 = NULL, *nc2 = NULL;
 char fname[] = "concat";
 ay_object *o = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }
  if(!sel->next)
    {
      ay_error(AY_ERROR,fname,"select two NURB curves!");
      return TCL_OK;
    }
  if((sel->object->type != AY_IDNCURVE) ||
     (sel->next->object->type != AY_IDNCURVE))
    {
      ay_error(AY_ERROR, fname, "object is not a NURBCurve");
      return TCL_OK;
    }

  if(!(o = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  o->type = AY_IDNCURVE;
  ay_status = ay_object_defaults(o);

  /* get curves transformation-matrix */
  ay_trafo_creatematrix(sel->object, m1);
  ay_trafo_creatematrix(sel->next->object, m2);

  nc1 = (ay_nurbcurve_object *)sel->object->refine;
  nc2 = (ay_nurbcurve_object *)sel->next->object->refine;

  if(nc1->knot_type != AY_KTCUSTOM)
    ktype = nc1->knot_type;

  controlv1 = nc1->controlv;
  controlv2 = nc2->controlv;

  if(!(newcontrolv = calloc(nc1->length*4+nc2->length*4, sizeof(double))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      free(o);
      return TCL_OK;
    }

  a = 0; b = 0;
  for(i=0;i<(nc1->length);i++)
    {
      newcontrolv[b] = controlv1[a];
      newcontrolv[b+1] = controlv1[a+1];
      newcontrolv[b+2] = controlv1[a+2];
      newcontrolv[b+3] = controlv1[a+3];

      ay_trafo_apply4(&(newcontrolv[b]), m1);

      a += 4;
      b += 4;
    }
  a = 0;
  for(i=0;i<(nc2->length);i++)
    {
      newcontrolv[b] = controlv2[a];
      newcontrolv[b+1] = controlv2[a+1];
      newcontrolv[b+2] = controlv2[a+2];
      newcontrolv[b+3] = controlv2[a+3];

      ay_trafo_apply4(&(newcontrolv[b]), m2);

      a += 4;
      b += 4;
    }

  ay_status = ay_nct_create(nc1->order, nc1->length+nc2->length,
			    ktype, newcontrolv, NULL,
			    (ay_nurbcurve_object **)&(o->refine));

  if(ay_status)
    {
      ay_error(ay_status, fname, NULL);
      return TCL_OK;
    }

  ay_object_link(o);

  ay_nct_recreatemp((ay_nurbcurve_object *)o->refine);

 return TCL_OK;
} /* ay_nct_concattcmd */


/* ay_nct_crtncircle:
 *  
 *  
 */
int
ay_nct_crtncircle(ay_nurbcurve_object **curve)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *new = NULL;
 double *controlv = NULL, *knotv = NULL;
 int i = 0;
 double controls[36] = {
   0.0,1.0,0.0,1.0,
   1.0,1.0,0.0,1.0,
   1.0,0.0,0.0,1.0,
   1.0,-1.0,0.0,1.0,
   0.0,-1.0,0.0,1.0, 
   -1.0,-1.0,0.0,1.0, 
   -1.0,0.0,0.0,1.0, 
   -1.0,1.0,0.0,1.0,
   0.0,1.0,0.0,1.0
 };
 double knots[12] = {
   0.0, 0.0, 0.0, 0.25, 0.25, 0.5, 0.5, 0.75, 0.75, 1.0, 1.0, 1.0
 };

  i = 7;
  controls[i] = sqrt(2.0)/2.0;
  controls[i-2] *= sqrt(2.0)/2.0;
  controls[i-3] *= sqrt(2.0)/2.0;
  i+=8;
  controls[i] = sqrt(2.0)/2.0;
  controls[i-2] *= sqrt(2.0)/2.0;
  controls[i-3] *= sqrt(2.0)/2.0;
  i+=8;
  controls[i] = sqrt(2.0)/2.0;
  controls[i-2] *= sqrt(2.0)/2.0;
  controls[i-3] *= sqrt(2.0)/2.0;
  i+=8;
  controls[i] = sqrt(2.0)/2.0;
  controls[i-2] *= sqrt(2.0)/2.0;
  controls[i-3] *= sqrt(2.0)/2.0;

  if(!(new = calloc(1, sizeof(ay_nurbcurve_object))))
    return AY_EOMEM;
  if(!(controlv = calloc(9*4, sizeof(double))))
    { free(new); return AY_EOMEM; }
  if(!(knotv = calloc(12, sizeof(double))))
    { free(new); free(controlv); return AY_EOMEM; }
  
  new->order = 3;
  new->length = 9;
  new->knot_type = AY_KTCUSTOM;

  memcpy(controlv,controls,9*4*sizeof(double)); 
  new->controlv = controlv;
  
  memcpy(knotv,knots,12*sizeof(double)); 
  new->knotv = knotv;

  new->createmp = AY_TRUE;

  *curve = new;

 return ay_status;
} /* ay_nct_crtncircle */


/* ay_nct_crtncirclearc:
 *  
 *  
 */
int
ay_nct_crtncirclearc(double arc, ay_nurbcurve_object **curve)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *new = NULL;

  if(!(new = calloc(1, sizeof(ay_nurbcurve_object))))
    return AY_EOMEM;

  if(arc<0.0)
    {
      ay_status = ay_nb_CreateNurbsCircle(1.0, arc, 0.0, &(new->length),
					      &new->knotv, &new->controlv);
    }
  else
    {
      ay_status = ay_nb_CreateNurbsCircle(1.0, 0.0, arc, &(new->length),
					      &new->knotv, &new->controlv);
    }

  if(ay_status)
    { free(new); return ay_status; }

  new->order = 3;
  new->knot_type = AY_KTCUSTOM;

  *curve = new;

 return ay_status;
} /* ay_nct_crtncirclearc */


/* ay_nct_crtnhcircle:
 *  
 *  
 */
int
ay_nct_crtnhcircle(ay_nurbcurve_object **curve)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *new = NULL;
 double *controlv = NULL, *knotv = NULL;
 int i = 0;
 double controls[20] = {
   0.0,1.0,0.0,1.0,
   1.0,1.0,0.0,1.0,
   1.0,0.0,0.0,1.0,
   1.0,-1.0,0.0,1.0,
   0.0,-1.0,0.0,1.0, 
 };
 double knots[8] = {
   0.0, 0.0, 0.0, 0.5, 0.5, 1.0, 1.0, 1.0
 };

  i = 4;
  controls[i] *= sqrt(2.0)/2.0;
  controls[i+1] *= sqrt(2.0)/2.0;
  controls[i+3] = sqrt(2.0)/2.0;
  i+=8;
  controls[i] *= sqrt(2.0)/2.0;
  controls[i+1] *= sqrt(2.0)/2.0;
  controls[i+3] = sqrt(2.0)/2.0;


  if(!(new = calloc(1, sizeof(ay_nurbcurve_object))))
    return AY_EOMEM;
  if(!(controlv = calloc(5*4, sizeof(double))))
    { free(new); return AY_EOMEM; }
  if(!(knotv = calloc(12, sizeof(double))))
    { free(new); free(controlv); return AY_EOMEM; }
  
  new->order = 3;
  new->length = 5;
  new->knot_type = AY_KTCUSTOM;

  memcpy(controlv,controls,5*4*sizeof(double)); 
  new->controlv = controlv;

  memcpy(knotv,knots,8*sizeof(double)); 
  new->knotv = knotv;

  *curve = new;

 return ay_status;
} /* ay_nct_crtnhcircle */


/* ay_nct_crtncircletcmd:
 *  
 *  
 */
int
ay_nct_crtncircletcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 int ay_status;
 char fname[] = "create_ncircle";
 double arc = 0;
 ay_object *o = NULL;

  if(!(o = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  o->type = AY_IDNCURVE;
  ay_status = ay_object_defaults(o);

  /* we create the nurbcurve-object */
  if(argc < 2)
    {
      ay_status = ay_nct_crtncircle((ay_nurbcurve_object **)&(o->refine));
    }
  else
    {
      Tcl_GetDouble(interp, argv[1], &arc);
      if(arc >= 360.0 || arc <= -360.0 || arc == 0.0)
	{
	  ay_status = ay_nct_crtncircle((ay_nurbcurve_object **)&(o->refine));
	}
      else
	{
	  ay_status = ay_nct_crtncirclearc(arc,
					(ay_nurbcurve_object **)&(o->refine));
	}

    }

  if(ay_status)
    {
      ay_error(ay_status,fname,NULL);
      return TCL_OK;
    }

  ay_nct_recreatemp((ay_nurbcurve_object *)o->refine);

  ay_object_link(o);

 return TCL_OK;
} /* ay_nct_crtncircletcmd */


/* ay_nct_crtrecttcmd:
 *  
 *  
 */
int
ay_nct_crtrecttcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status;
 ay_list_object *lev = ay_currentlevel;
 ay_object *parent = NULL;
 ay_nurbpatch_object *patch = NULL;
 ay_nurbcurve_object *curve = NULL;
 char fname[] = "create_trimrect"; 
 double def_controls[20] = { 1.0, 1.0, 0.0, 1.0,
			     1.0,-1.0, 0.0, 1.0,
			     -1.0,-1.0, 0.0, 1.0,
			     -1.0, 1.0, 0.0, 1.0,
			     1.0, 1.0, 0.0, 1.0};
 double knots[7] = {0.0, 0.0, 0.25, 0.5, 0.75, 1.0, 1.0};
 int i = 0, create_trim = AY_FALSE;
 ay_object *o = NULL;


  if(!(o = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  o->type = AY_IDNCURVE;
  ay_status = ay_object_defaults(o);

  if(lev->next)
    {
      parent = lev->next->object;
      if(parent)
	{
	  if(parent->type == AY_IDNPATCH)
	    {
	      create_trim = AY_TRUE;
	      patch = (ay_nurbpatch_object *)parent->refine;
	    }
	}
    }


  if(!(curve = calloc(1,sizeof(ay_nurbcurve_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  if(!(curve->controlv = calloc(20, sizeof(double))))
    {
      free(curve);
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  if(!(curve->knotv = calloc(7, sizeof(double))))
    {
      free(curve->controlv); free(curve);
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  curve->length = 5;
  curve->order = 2;
  curve->knot_type = AY_KTCUSTOM;

  /* fill knotv */
  for(i=0;i<7;i++)
    curve->knotv[i] = knots[i];

  /* fill controlv */
  if(create_trim)
    {
      curve->controlv[0] = patch->uknotv[0];
      curve->controlv[1] = patch->vknotv[0];
      curve->controlv[3] = 1.0;
      curve->controlv[4] = patch->uknotv[patch->width+patch->uorder-1];
      curve->controlv[5] = patch->vknotv[0];
      curve->controlv[7] = 1.0;
      curve->controlv[8] = patch->uknotv[patch->width+patch->uorder-1];
      curve->controlv[9] = patch->vknotv[patch->height+patch->vorder-1];
      curve->controlv[11] = 1.0; 
      curve->controlv[12] = patch->uknotv[0];
      curve->controlv[13] = patch->vknotv[patch->height+patch->vorder-1];
      curve->controlv[15] = 1.0;
      curve->controlv[16] = patch->uknotv[0];
      curve->controlv[17] = patch->vknotv[0];
    }
  else
    {
      memcpy(curve->controlv, def_controls, 20*sizeof(double));
    }

  o->type = AY_IDNCURVE;
  o->refine = curve;

  ay_object_link(o);

  ay_nct_recreatemp(curve);

 return TCL_OK;
} /* ay_nct_split */


/* ay_nct_crtclosedbsptcmd:
 *  
 *  
 */
int
ay_nct_crtclosedbsptcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[])
{
 int ay_status;
 ay_nurbcurve_object *curve = NULL;
 char fname[] = "create_closedbsp"; 
 double *controlv;
 int i = 0, num = 0, a = 0;
 double angle = 0.0;
 double m[16];
 ay_object *o = NULL;

  if(!(o = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  o->type = AY_IDNCURVE;
  ay_status = ay_object_defaults(o);

  if(argc < 2)
    {
      ay_error(AY_EARGS,fname,"numpoints");
      return TCL_OK;
    }
  
  Tcl_GetInt(interp,argv[1],&num);
 
  if(num<=2)
    {
      ay_error(AY_ERROR, fname, "numpoints must be >= 3");
      return TCL_OK;
    }

  if(!(controlv = calloc((num+3)*4, sizeof(double))))
    {
      free(curve);
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  angle = 360.0/num;
  
  ay_trafo_identitymatrix(m);
  for(i=0;i<num+3;i++)
    {
      a = i*4;
      controlv[a] = 1.0;
      controlv[a+1] = 0.0;
      controlv[a+2] = 0.0;
      controlv[a+3] = 1.0;
      ay_trafo_rotatematrix(angle, 0.0, 0.0, 1.0, m);
      ay_trafo_apply3(&(controlv[a]), m);
    }

  ay_status = ay_nct_create(4, num+3, AY_KTBSPLINE, controlv, NULL, &curve);

  curve->closed = AY_TRUE;
  ay_nct_close(curve);
  o->refine = curve;
  o->type = AY_IDNCURVE;

  curve->createmp = AY_TRUE;
  ay_nct_recreatemp(curve);

  ay_object_link(o);

 return TCL_OK;
} /* ay_nct_crtclosedbsptcmd */


/* ay_nct_getorientation:
 *  return a negative value if curve is ccw (or cw? :)
 *  this is my third attempt on (and a complete rewrite of) this routine
 *  this time with support from the c.g.algorithms FAQ and sample
 *  code from Joseph O'Rourke from:
 *  ftp://cs.smith.edu/pub/code/polyorient.C
 */
void
ay_nct_getorientation(ay_nurbcurve_object *curve, double *orient)
{
 double minx, miny;
 int i, j, n, m, stride, found, wrap;
 double *cv = NULL, a[2], b[2], c[2];
 char fname[] = "nct_getorientation";

  cv = curve->controlv;
  n = curve->length;
  if(n <= 2)
    {
      ay_error(AY_ERROR, fname, "Need more than 2 control points!\n");
      return;
    }

  minx = cv[0];
  miny = cv[1];

  stride = 4;
  j = stride;
  m = 0;
  for(i = 0; i < curve->length-1; i++)
    {
      if((cv[j+1] < miny) || ((cv[j+1] == miny) && (cv[j] < minx)))
	{
	  minx = cv[j];
	  miny = cv[j+1];
	  m = i+1;
	} /* if */

      j += stride;
    } /* for */

  if(m >= curve->length)
    m = 0;

  found = AY_FALSE;
  wrap = 0;
  j = m;
  while(!found && (wrap<2))
    {
      if((fabs(cv[m*stride] - cv[j*stride]) > AY_EPSILON) &&
	 (fabs(cv[m*stride+1] - cv[j*stride+1]) > AY_EPSILON))
	{
	  found = AY_TRUE;
	}
      else
	{
	  if(j<(n-1))
	    {
	      j++;
	    }
	  else
	    {
	      j = 0;
	      wrap++;
	    }
	}
    } /* while */

  if(found)
    {
      c[0] = cv[j*stride];
      c[1] = cv[j*stride+1];
    }
  else
    {
      ay_error(AY_ERROR, fname, "Could not determine 3 different points!\n");
      return;
    }

  found = AY_FALSE;
  wrap = 0;
  j = m;
  while(!found && (wrap<2))
    {
      if((fabs(cv[m*stride] != cv[j*stride]) > AY_EPSILON) &&
	 (fabs(cv[m*stride+1] != cv[j*stride+1]) > AY_EPSILON))
	{
	  found = AY_TRUE;
	}
      else
	{
	  if(j > 0)
	    {
	      j--;
	    }
	  else
	    {
	      j = n-1;
	      wrap++;
	    }
	}
    } /* while */

  if(found)
    {
      a[0] = cv[j*stride];
      a[1] = cv[j*stride+1];
    }
  else
    {
      ay_error(AY_ERROR, fname, "Could not determine 3 different points!\n");
      return;
    }

  b[0] = cv[m*stride];
  b[1] = cv[m*stride+1];


  *orient = a[0] * b[1] - a[1] * b[0] +
            a[1] * c[0] - a[0] * c[1] +
            b[0] * c[1] - c[0] * b[1];

  /*
  printf("%g %g|%g %g|%g %g => %g\n",
	 a[0], a[1], b[0], b[1], c[0], c[1], *orient);
  */

 return;
} /* ay_nct_getorientation */


/* ay_nct_isclosed:
 *  
 *  
 */
int
ay_nct_isclosed(ay_nurbcurve_object *nc)
{
 double u, P1[4], P2[4];

 u = nc->knotv[nc->order-1];
 ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
		    nc->knotv, nc->controlv,
		    u, P1);

 u = nc->knotv[nc->length];
 ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
		    nc->knotv, nc->controlv,
		    u, P2);

 if((fabs(P1[0]-P2[0]) < AY_EPSILON) &&
    (fabs(P1[0]-P2[0]) < AY_EPSILON) &&
    (fabs(P1[0]-P2[0]) < AY_EPSILON))
   return AY_TRUE;
 else
   return AY_FALSE;

} /* ay_nct_isclosed */


/* ay_nct_createfrompatch:
 *   create a curve from a n*m NURBS patch p
 *   mode:
 *    0 - along u(0)
 *    1 - along u(n)
 *    2 - along v(0)
 *    3 - along v(m)
 */
int
ay_nct_createfrompatch(ay_object *p, int where, ay_object **cu)
{
 int ay_status = AY_OK;
 int n, i, a, b;
 double *cv = NULL, *kn = NULL;
 ay_nurbpatch_object *pa = NULL;
 ay_object *c = NULL;

  if(!p || !cu)
    return AY_ENULL;

  if(p->type != AY_IDNPATCH)
    return AY_ERROR;

  pa = (ay_nurbpatch_object *)p->refine;

  if(!(c = calloc(1, sizeof(ay_object))))
    return AY_EOMEM;

  ay_object_defaults(c);

  c->type = AY_IDNCURVE;

  switch(where)
    {
    case 0:
      n = pa->width;
      
      if(!(cv = calloc(n*4, sizeof(double))))
	{ free(c); return AY_EOMEM; }
      a = 0;
      b = 0;
      for(i = 0; i < n; i++)
	{
	  memcpy(&(cv[a]), &(pa->controlv[b]), 4*sizeof(double));
	  a += 4;
	  b += pa->height*4;
	}

      if(!(kn = calloc(n+pa->uorder, sizeof(double))))
	{
	  free(c); free(cv); return AY_EOMEM;
	}
      memcpy(kn, pa->uknotv, (n+pa->uorder)*sizeof(double));

      ay_status = ay_nct_create(pa->uorder, n, pa->uknot_type,
				cv, kn,
				(ay_nurbcurve_object **)(&c->refine));

      break;
    case 1:
      n = pa->width;
      
      if(!(cv = calloc(n*4, sizeof(double))))
	{ free(c); return AY_EOMEM; }
      a = 0;
      b = (pa->height-1)*4;
      for(i = 0; i < n; i++)
	{
	  memcpy(&(cv[a]), &(pa->controlv[b]), 4*sizeof(double));
	  a += 4;
	  b += pa->height*4;
	}

      if(!(kn = calloc(n+pa->uorder, sizeof(double))))
	{
	  free(c); free(cv); return AY_EOMEM;
	}
      memcpy(kn, pa->uknotv, (n+pa->uorder)*sizeof(double));

      ay_status = ay_nct_create(pa->uorder, n, pa->uknot_type,
				cv, kn,
				(ay_nurbcurve_object **)(&c->refine));

      break;
    case 2:
      n = pa->height;
      
      if(!(cv = calloc(n*4, sizeof(double))))
	{ free(c); return AY_EOMEM; }
      a = 0;
      b = 0;
      for(i = 0; i < n; i++)
	{
	  memcpy(&(cv[a]), &(pa->controlv[b]), 4*sizeof(double));
	  a += 4;
	  b += 4;
	}

      if(!(kn = calloc(n+pa->vorder, sizeof(double))))
	{
	  free(c); free(cv); return AY_EOMEM;
	}
      memcpy(kn, pa->vknotv, (n+pa->vorder)*sizeof(double));

      ay_status = ay_nct_create(pa->vorder, n, pa->vknot_type,
				cv, kn,
				(ay_nurbcurve_object **)(&c->refine));

      break;
    case 3:
      n = pa->height;
      
      if(!(cv = calloc(n*4, sizeof(double))))
	{ free(c); return AY_EOMEM; }
      a = 0;
      b = n*(pa->width-1)*4;
      for(i = 0; i < n; i++)
	{
	  memcpy(&(cv[a]), &(pa->controlv[b]), 4*sizeof(double));
	  a += 4;
	  b += 4;
	}

      if(!(kn = calloc(n+pa->vorder, sizeof(double))))
	{
	  free(c); free(cv); return AY_EOMEM;
	}
      memcpy(kn, pa->vknotv, (n+pa->vorder)*sizeof(double));

      ay_status = ay_nct_create(pa->vorder, n, pa->vknot_type,
				cv, kn,
				(ay_nurbcurve_object **)(&c->refine));

      break;
    default:
      break;

    } /* switch */

  *cu = c;

 return ay_status;
} /* ay_nct_createfrompatch */


/* ay_nct_applytrafo:
 *  
 *  
 */
int
ay_nct_applytrafo(ay_object *c)
{
 int i, stride;
 double m[16];
 ay_nurbcurve_object *nc = NULL;

  if(!c)
    return AY_ENULL;

  if(!c->type == AY_IDNCURVE)
    return AY_ERROR;

  nc = (ay_nurbcurve_object *)(c->refine);

  /* get curves transformation-matrix */
  ay_trafo_creatematrix(c, m);

  stride = 4;
  i = 0;
  while(i < nc->length*stride)
    {
      ay_trafo_apply4(&(nc->controlv[i]), m);
      
      i += stride;
    } /* while */

  /* reset curves transformation attributes */
  ay_trafo_defaults(c);

 return AY_OK;
} /* ay_nct_applytrafo */


/* ay_nct_getpntfromindex:
 *  return the adress of the control point designated by <index>
 *  of the NURBS curve <curve> in <p>
 */
int
ay_nct_getpntfromindex(ay_nurbcurve_object *curve, int index, double **p)
{
 int stride = 4;
 char fname[] = "ay_nct_getpntfromindex";

  if(index >= curve->length || index < 0)
    {
      ay_error(AY_ERROR, fname, "index out of range");
      return TCL_OK;
    }

  *p = &(curve->controlv[index*stride]);

 return TCL_OK;
} /* ay_nct_getpntfromindex */


/* ay_nct_concatmultiple:
 *  concat multiple NURBS curves in curves to a new single
 *  curve, which is returned via result; order and knot type
 *  are taken from the first curve as well as glu_sampling_tolerance
 *  and display_mode settings; the curves are expected to overlap
 *  at their ends
 */
int
ay_nct_concatmultiple(int closed, int knot_type, int fillgaps,
		      ay_object *curves,
		      ay_object **result)
{
 int ay_status;
 char fname[] = "nct_concatmultiple";
 double *newknotv = NULL, *newcontrolv = NULL, *ncv = NULL;
 int numcurves = 0, i, j, a, order = -1, length = 0, ktype;
 int glu_display_mode = 0;
 double glu_sampling_tolerance = 0.0;
 ay_nurbcurve_object *nc;
 ay_object *o = NULL, *newo = NULL;

  /* check arguments */
  if(!curves)
    {
      ay_error(AY_ENULL, fname, NULL);
      return AY_ERROR;
    }

  /* create new object */
  if(!(newo = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  newo->type = AY_IDNCURVE;
  ay_status = ay_object_defaults(newo);

  /* count curves; calculate length of concatenated curve */
  o = curves;
  while(o)
    {
      nc = (ay_nurbcurve_object *)o->refine;

      length += nc->length;
      numcurves++;
      o = o->next;
    } /* while */

  /* take order, tolerance, and display_mode from first curve */
  o = curves;
  nc = (ay_nurbcurve_object *)o->refine;
  order = nc->order;
  glu_sampling_tolerance = nc->glu_sampling_tolerance;
  glu_display_mode = nc->display_mode;

  if((order == 2) && closed && (knot_type == 0) && (!fillgaps))
    {
      length++; 
    }

  /* construct new knotvector */
  if(knot_type == 0)
    {
      ktype = AY_KTNURB;
    }
  else
    {
      ktype = AY_KTCUSTOM;

      if(!(newknotv = calloc(length + order, sizeof(double))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  free(newo);
	  return AY_ERROR;
	}

      a = 0;
      j = 0;
      o = curves;
      while(o)
	{
	  nc = (ay_nurbcurve_object *)o->refine;

	  for(i = 0; i < nc->length+nc->order; i++)
	    {
	      newknotv[a] = nc->knotv[i]+j;
	      a++;
	    }

	  j+=2;

	  o = o->next;
	  if(o)
	    o = o->next;
	} /* while */

      if(closed)
	{
	  for(i = a; i < length+order; i++)
	    {
	      newknotv[i] = newknotv[a-1]+1;
	    }
	}

    } /* if */

  /* construct new control point vector */
  if(!(newcontrolv = calloc(length * 4, sizeof(double))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      free(newo);
      if(knot_type == 0)
	free(newknotv);
      return AY_ERROR;
    }

  ncv = newcontrolv;
  o = curves;
  while(o)
    {
      nc = (ay_nurbcurve_object *)o->refine;
      memcpy(ncv, nc->controlv, nc->length * 4 * sizeof(double));
      ncv += (nc->length * 4);
      o = o->next;
    } /* while */

  ay_status = ay_nct_create(order, length,
			    ktype, newcontrolv, newknotv,
			    (ay_nurbcurve_object **)&(newo->refine));

  if(ay_status)
    {
      free(newo);
      if(knot_type == 0)
	free(newknotv);
      free(newcontrolv);
      ay_error(ay_status, fname, NULL);
      return AY_ERROR;
    }

  nc = (ay_nurbcurve_object *)newo->refine;
  if(nc)
    {
      ay_nct_recreatemp(nc);

      nc->glu_sampling_tolerance = glu_sampling_tolerance;
      nc->display_mode = glu_display_mode;
    }

  *result = newo;

 return AY_OK;
} /* ay_nct_concatmultiple */


/* ay_nct_fillgap:
 *  create a fillet curve between the last point of curve c1
 *  and the first point of curve c2; the fillet will be a simple
 *  <order> point Bezier curve with tangents matching the tangents
 *  of endpoints of the curves c1/c2, tangent length will be scaled
 *  additionally by <tanlen>
 */
int
ay_nct_fillgap(int order, double tanlen,
	       ay_nurbcurve_object *c1, ay_nurbcurve_object *c2,
	       ay_object **result)
{
 double p1[4], p2[4], p3[4], p4[4], n1[3], n2[3], l[3];
 double *U, *Pw, u, d, w;
 double *controlv = NULL;
 int n, p, numcontrol, i;
 ay_object *o = NULL;
 ay_nurbcurve_object *c = NULL;
 int ay_status = AY_OK;

  n = c1->length;
  p = c1->order-1;
  U = c1->knotv;
  Pw = c1->controlv;

  /* get coordinates of the first and last point of the curve
     as well as the first derivative in those points */
  u = U[n];
  ay_nb_CurvePoint4D(n-1, p, U, Pw, u, p1);
  w = p1[3];
  AY_V3SCAL(p1, 1.0/w)
  p1[3] = 1.0;
  ay_nb_ComputeFirstDer4D(n-1, p, U, Pw, u, n1);
  AY_V3NORM(n1)
  n = c2->length;
  p = c2->order-1;
  U = c2->knotv;
  Pw = c2->controlv;
  u = U[p];
  ay_nb_CurvePoint4D(n-1, p, U, Pw, u, p2);
  w = p2[3];
  AY_V3SCAL(p2, 1.0/w)
  p2[3] = 1.0;
  ay_nb_ComputeFirstDer4D(n-1, p, U, Pw, u, n2);
  AY_V3NORM(n2)

  /* first, check whether p1 and p2 are sufficiently different */
  if((fabs(p1[0] - p2[0]) < AY_EPSILON) &&
     (fabs(p1[1] - p2[1]) < AY_EPSILON) &&
     (fabs(p1[2] - p2[2]) < AY_EPSILON))
    {
      /* No, no fillet needs to be created, just bail out! */
      return AY_OK;
    }

  AY_V3SCAL(n1, -1.0)

  p3[3] = 1.0;
  p4[3] = 1.0;

  AY_V3SUB(l, p2, p1)
  d = AY_V3LEN(l);
  AY_V3SCAL(n1, d*tanlen)
  AY_V3SCAL(n2, d*tanlen)
  AY_V3SUB(p3, p1, n1)
  AY_V3SUB(p4, p2, n2)

  /*numcontrol = (order>4?order:4)*4;*/
  numcontrol = order;

  /* fill new controlv */
  if(!(controlv = calloc(numcontrol*4, sizeof(double))))
    return AY_EOMEM;

  memcpy(&(controlv[0]), p1, 4*sizeof(double));

  if(numcontrol > 2)
    {
      memcpy(&(controlv[(numcontrol-2)*4]), p4, 4*sizeof(double));
    }

  memcpy(&(controlv[(numcontrol-1)*4]), p2, 4*sizeof(double));

  if(numcontrol > 3)
    {
      memcpy(&(controlv[4]), p3, 4*sizeof(double));
    }

  if(numcontrol == 3)
    {
      AY_V3SUB(p4, p4, p3)
      AY_V3SCAL(p4, 0.5)
      AY_V3ADD(p1, p3, p4)
      memcpy(&(controlv[4]), p1, 4*sizeof(double));
    }

  if(numcontrol > 4)
    {
      AY_V3SUB(p1, p4, p2)
      d = AY_V3LEN(p1);
      for(i = 1; i < (numcontrol - 4); i++)
	{
	  memcpy(p3, p1, 4*sizeof(double));
	  AY_V3SCAL(p3, i*(d/(numcontrol-3)))
	  memcpy(&(controlv[(1 + i) * 4]), p3, 4*sizeof(double));
	}
    }

  if(!(o = calloc(1, sizeof(ay_object))))
    { free(controlv); return AY_EOMEM; }

  ay_object_defaults(o);

  ay_status = ay_nct_create(numcontrol, order, AY_KTBEZIER, controlv,
			    NULL, &c);
  if(ay_status)
    { free(o); free(controlv); return AY_ERROR; }

  o->refine = c;
  o->type = AY_IDNCURVE;

  *result = o;

 return AY_OK;
} /* ay_nct_fillgap */


/* ay_nct_fillgaps:
 *  create fillet curves (using ay_nct_fillgap() above) for all
 *  gaps in the list of curves pointed to by curves and insert
 *  the fillets right in this list
 */
int
ay_nct_fillgaps(int closed, int order, double tanlen, ay_object *curves)
{
 ay_object *c = NULL, *fillet = NULL, *last = NULL;
 int ay_status = AY_OK;

  c = curves;
  while(c)
    {
      if(c->next)
	{
	  fillet = NULL;
	  ay_status = ay_nct_fillgap(order, tanlen,
				     (ay_nurbcurve_object *)c->refine,
				     (ay_nurbcurve_object *)c->next->refine,
				     &fillet);
	  if(ay_status)
	    {return AY_ERROR;}
	  if(fillet)
	    {
	      fillet->next = c->next;
	      c->next = fillet;
	      c = c->next;
	    } /* if */
	} /* if */
      last = c;
      c = c->next;
    } /* while */

  if(closed)
    {
      fillet = NULL;
      ay_status = ay_nct_fillgap(order, tanlen,
				 (ay_nurbcurve_object *)last->refine,
				 (ay_nurbcurve_object *)curves->refine,
				 &fillet);
      if(ay_status)
	{return AY_ERROR;}
      if(fillet)
	{
	  last->next = fillet;
	} /* if */
    } /* if */

 return AY_OK;
} /* ay_nct_fillgaps */


/* ay_nct_arrange:
 *  arrange objects in o along trajectory t (a NURBS curve);
 *  if rotate is AY_TRUE, additionally rotate all objects in
 *  o so that their local X axis is parallel to the curve
 *  points derivative
 */
int
ay_nct_arrange(ay_object *o, ay_object *t, int rotate)
{
 int ay_status = AY_OK;
 ay_object *l;
 ay_nurbcurve_object *tr;
 int i = 0, a = 0, stride;
 double u, p1[4];
 double T0[3] = {0.0,0.0,-1.0};
 double T1[3] = {0.0,0.0,0.0};
 double A[3] = {0.0,0.0,0.0};
 double len = 0.0, plen = 0.0;
 double mtr[16];
 double *trcv = NULL, angle, quat[4], euler[3];
 unsigned int n = 0;

  if(!o || !t)
    return AY_ENULL;

  if(t->type != AY_IDNCURVE)
    return AY_OK;

  /* count objects */
  l = o;
  while(l)
    {
      n++;
      l = l->next;
    }

  stride = 4;

  /* apply all transformations to trajectory curves controlv */
  tr = (ay_nurbcurve_object *)(t->refine);
  if(!(trcv = calloc(tr->length*stride, sizeof(double))))
    { return AY_EOMEM; }
  memcpy(trcv, tr->controlv, tr->length*stride*sizeof(double));
  
  ay_trafo_creatematrix(t, mtr);
  a = 0;
  for(i = 0; i < tr->length; i++)
    {
      ay_trafo_apply4(&(trcv[a]), mtr);
      a += stride;
    }

  plen = fabs(tr->knotv[tr->length] - tr->knotv[tr->order-1]);

  T0[0] = 1.0;
  T0[1] = 0.0;
  T0[2] = 0.0;

  l = NULL;
  i = 0;

  while(o)
    {
      u = tr->knotv[tr->order-1]+(((double)i/n)*plen);

      /* calculate new translation */
      ay_nb_CurvePoint4D(tr->length-1, tr->order-1, tr->knotv, trcv,
			 u, p1);

      o->movx = p1[0];
      o->movy = p1[1];
      o->movz = p1[2];

      if(l)
	{
	  memcpy(o->quat, l->quat, 4*sizeof(double));
	  o->rotx = l->rotx;
	  o->roty = l->roty;
	  o->rotz = l->rotz;
	}
      else
	{
	  memset(o->quat, 0, 3*sizeof(double));
	  o->quat[3] = 1.0;
	}

      if(rotate)
	{
	  
	  ay_nb_ComputeFirstDer4D(tr->length-1, tr->order-1, tr->knotv,
				  trcv, u, T1);

	  len = AY_V3LEN(T1);
	  AY_V3SCAL(T1,(1.0/len))

	  if(((fabs(fabs(T1[0])-fabs(T0[0])) > AY_EPSILON) ||
	      (fabs(fabs(T1[1])-fabs(T0[1])) > AY_EPSILON) ||
	      (fabs(fabs(T1[2])-fabs(T0[2])) > AY_EPSILON)))
	    {
	      AY_V3CROSS(A,T0,T1)
	      len = AY_V3LEN(A);
	      AY_V3SCAL(A,(1.0/len))
	      
	      angle = acos(AY_V3DOT(T0,T1));

	      if(fabs(angle) > AY_EPSILON)
		{
		  ay_quat_axistoquat(A, -angle, quat);
		  ay_quat_add(quat, o->quat, o->quat);
		  ay_quat_toeuler(o->quat, euler);
		  o->rotx = AY_R2D(euler[0]);
		  o->roty = AY_R2D(euler[1]);
		  o->rotz = AY_R2D(euler[2]);
		} /* if */
	    } /* if */

	  memcpy(T0, T1, 3*sizeof(double));

	} /* if rotate */

      i++;
      l = o;
      o = o->next;
    } /* while */


  /* clean up */
  free(trcv);

 return ay_status;
} /* ay_nct_arrange */


/* ay_nct_addinternalcps:
 * add <order>-1 internal colinear control points to the curve <curve>;
 * if <where> is AY_TRUE: add at beginning of the curve,
 * else add them at the end of the curve
 */
int
ay_nct_addinternalcps(ay_object *curve, int where)
{
 int ay_status = AY_OK;
 int num, i, a, b;
 double *ncv = NULL, v[4] = {0.0,0.0,0.0,0.0}, sf = 0.0;
 ay_nurbcurve_object *nc =  NULL;

 if(curve && curve->refine)
   {
     nc = (ay_nurbcurve_object *)(curve->refine);
     if(nc->knot_type == AY_KTCUSTOM)
       return AY_OK;

     num = nc->order - 1;
     if(num > 0)
       {
	 if(!(ncv = calloc((nc->length+num)*4, sizeof(double))))
	   return AY_EOMEM;

	 if(where)
	   {
	     /* insert at start */

	     /* copy first point */
	     memcpy(ncv, nc->controlv, 4 * sizeof(double));
	     /* copy trailing points */
	     memcpy(&(ncv[4*(num+1)]), &((nc->controlv)[4]), (nc->length - 1) *
		    4 * sizeof(double));
	     /* fill new points using interpolated values */
	     v[0] = (nc->controlv)[4] - (nc->controlv)[0];
	     v[1] = (nc->controlv)[5] - (nc->controlv)[1];
	     v[2] = (nc->controlv)[6] - (nc->controlv)[2];
	     v[3] = (nc->controlv)[7] - (nc->controlv)[3];

	     sf = (1.0/(num + 1));
	     AY_V3SCAL(v, sf);
	     for(i = 1; i <= num; i++)
	       {
		 a = i*4;
		 ncv[a]   = (nc->controlv)[0] + i*v[0];
		 ncv[a+1] = (nc->controlv)[1] + i*v[1];
		 ncv[a+2] = (nc->controlv)[2] + i*v[2];
		 ncv[a+3] = 1.0/*(nc->controlv)[3] + i*v[3]*/;
	       }
	   }
	 else
	   {
	     /* insert at end */

	     /* copy first points */
	     memcpy(ncv, nc->controlv, (nc->length - 1) * 4 * sizeof(double));
	     /* copy trailing point */
	     memcpy(&(ncv[(nc->length+num-1)*4]),
		    &((nc->controlv)[(nc->length-1) * 4]), 4 * sizeof(double));
	     a = (nc->length - 2) * 4;
	     /* fill new points using interpolated values */
	     v[0] = (nc->controlv)[a+4] - (nc->controlv)[a];
	     v[1] = (nc->controlv)[a+5] - (nc->controlv)[a+1];
	     v[2] = (nc->controlv)[a+6] - (nc->controlv)[a+2];
	     v[3] = (nc->controlv)[a+7] - (nc->controlv)[a+3];

	     sf = (1.0/(num + 1));
	     AY_V3SCAL(v, sf);
	     b = (nc->length-2) * 4;
	     for(i = 1; i <= num; i++)
	       {
		 a = (nc->length-2+i)*4;
		 ncv[a]   = (nc->controlv)[b] + i*v[0];
		 ncv[a+1] = (nc->controlv)[b+1] + i*v[1];
		 ncv[a+2] = (nc->controlv)[b+2] + i*v[2];
		 ncv[a+3] = 1.0/*(nc->controlv)[3] + i*v[3]*/;
	       } /* for */
	   } /* if */


	 nc->length += num;
	 free(nc->controlv);
	 nc->controlv = ncv;
	 free(nc->knotv);
	 nc->knotv = NULL;
	 ay_knots_createnc(nc);

       } /* if */
   }
 else
   {
     ay_status = AY_ENULL;
   } /* if */

 return ay_status;
} /* ay_nct_addinternalcps */


/* ay_nct_rescaleknvnctcmd:
 *  rescale the knot vector of a NURBS curve to the range 0.0 - 1.0
 */
int
ay_nct_rescaleknvnctcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;
 ay_nurbcurve_object *curve = NULL;
 char fname[] = "rescaleKnNC";

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  while(sel)
    {
      src = sel->object;
      if(src->type != AY_IDNCURVE)
	{
	  ay_error(AY_ERROR, fname, "Object is not a NURBCurve!");
	}
      else
	{

	  curve = (ay_nurbcurve_object*)src->refine;

	  if(curve->knot_type == AY_KTCUSTOM)
	    {

	      ay_status = ay_knots_rescaleknotv(curve->length+curve->order,
						curve->knotv);
	      if(ay_status)
		{
		  ay_error(ay_status, fname, "Could not rescale knots!");
		}
	    }
	  else
	    {
	      ay_error(ay_status, fname, "Need a custom knot vector!");
	    } /* if */

	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_rescaleknvnctcmd */


/* ay_nct_getcurvature:
 *  return the curvature of curve <c> at parametric value <t>
 */
double
ay_nct_getcurvature(ay_nurbcurve_object *c, double t)
{
 double vel[3], acc[3], cross[3];
 double velsqrlen, numer, denom;

  if(!c)
    return 0.0;

  if((t < c->knotv[0]) || (t > c->knotv[c->length+c->order-1]))
    return 0.0;

  ay_nb_ComputeFirstDer4D(c->length, c->order-1, c->knotv, c->controlv, t,
			  vel);
  velsqrlen = (vel[0]*vel[0])+(vel[1]*vel[1])+(vel[2]*vel[2]);

  if(velsqrlen > AY_EPSILON)
    {
      ay_nb_ComputeSecDer4D(c->length, c->order-1, c->knotv, c->controlv, t,
			    acc);
      AY_V3CROSS(cross, vel, acc);
      numer = AY_V3LEN(cross);
      denom = pow(velsqrlen, 1.5);
      return (numer/denom);
    }
  else
    {
      return 0.0;
    }

 return 0.0;
} /* ay_nct_getcurvature */


/* ay_nct_curvplottcmd:
 *  create a curvature plot NURBS curve from the selected NURBS curve(s)
 */
int
ay_nct_curvplottcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o, *po = NULL;
 ay_nurbcurve_object *c, *c2 = NULL;
 char fname[] = "curvPlot";
 double width = 5.0, scale = 1.0, t, dt, *controlv;
 int a = 0, b = 0, samples = 100, freepo;
 char *cname;
 Tcl_DString ds;

  if(argc >= 2)
    Tcl_GetInt(interp, argv[1], &samples);

  if(argc >= 3)
    Tcl_GetDouble(interp, argv[2], &width);

  if(argc >= 4)
    Tcl_GetDouble(interp, argv[3], &scale);

  while(sel)
    {
      freepo = AY_FALSE;
      c = NULL;
      cname = NULL;
      if(sel->object->type == AY_IDNCURVE)
	{
	  c = (ay_nurbcurve_object *)sel->object->refine;
	  cname = sel->object->name;
	}
      else
	{
	  ay_status = ay_provide_object(sel->object, AY_IDNCURVE, &po);
	  if(po)
	    {
	      freepo = AY_TRUE;
	      c = (ay_nurbcurve_object *)po->refine;
	      cname = sel->object->name;
	    }
	}

      if(c)
	{
	  controlv = NULL;
	  if(!(controlv = calloc(samples*4, sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  o = NULL;
	  if(!(o = calloc(1, sizeof(ay_object))))
	    {
	      free(controlv);
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  ay_object_defaults(o);
	  o->type = AY_IDNCURVE;

	  dt = (c->knotv[c->length+c->order-1]-c->knotv[0])/((double)samples);
	  a = 0;
	  b = 0;
	  for(t = c->knotv[0]; t < c->knotv[c->length+c->order-1]; t += dt)
	    {
	      controlv[a] = (double)b*width/samples;
	      controlv[a+1] = ay_nct_getcurvature(c, t)*scale;
	      controlv[a+3] = 1.0;
	      a += 4;
	      b++;
	    }
	  
	  ay_status = ay_nct_create(4, samples, AY_KTNURB, controlv, NULL,
				    &c2);
	  Tcl_DStringInit(&ds);
	  Tcl_DStringAppend(&ds, "Curvature", -1);
	  if(cname)
	    {
	      Tcl_DStringAppend(&ds, "_of_", -1);
	      Tcl_DStringAppend(&ds, cname, -1);
	    }
	  if((o->name = calloc(Tcl_DStringLength(&ds)+1, sizeof(char))))
	    {
	      strcpy(o->name, Tcl_DStringValue(&ds));
	    }
	  o->refine = c2;
	  ay_object_link(o);
	  Tcl_DStringFree(&ds);
	} /* if */	  

      if(freepo)
	{
	  ay_object_deletemulti(po);
	}

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_curvplottcmd */


/* ay_nct_intersect:
 *
 */
int
ay_nct_intersect(ay_nurbcurve_object *cu, ay_nurbcurve_object *cv,
		 double *intersection)
{
 int ay_status = AY_OK;
 double *bu = NULL, *bv = NULL; /* Bezier segment arrays */
 double *su, *sv;
 double cuminx, cumaxx, cvminx, cvmaxx;
 double cuminy, cumaxy, cvminy, cvmaxy;
 double cuminz, cumaxz, cvminz, cvmaxz;
 int nbu = 0, nbv = 0, order_u, order_v;
 int stride = 4, i, j, k, l, overlap;

  if(!cu || !cv)
    return AY_ENULL;

  /* decompose both curves into Bezier segments */
  if(!(bu = calloc(cu->order*stride, sizeof(double))))
    return AY_EOMEM;

  ay_status = ay_nb_DecomposeCurve(4, cu->length, cu->order-1, cu->knotv,
				   cu->controlv, &nbu, &bu);

  if(!(bv = calloc(cv->order*stride, sizeof(double))))
    return AY_EOMEM;

  ay_status = ay_nb_DecomposeCurve(4, cv->length, cv->order-1, cv->knotv,
				   cv->controlv, &nbv, &bv);

  /* find intersecting Bezier segments (use convex hull property to
     exclude all pairs that cannot intersect) */
  for(i = 0; i < nbu; i++)
    {
      /* get min/max of segment i in bu (X) */
      su = &bu[i*stride];
      cuminx = DBL_MAX;
      cumaxx = DBL_MIN;
      for(k = 0; k < cu->order; k++)
	{
	  if(*su < cuminx)
	    cuminx = *su;
	  if(*su > cumaxx)
	    cumaxx = *su;
	  su += stride;
	}
      /* get min/max of segment i in bu (Y) */
      su = &bu[i*stride+1];
      cuminy = DBL_MAX;
      cumaxy = DBL_MIN;
      for(k = 0; k < cu->order; k++)
	{
	  if(*su < cuminy)
	    cuminy = *su;
	  if(*su > cumaxy)
	    cumaxy = *su;
	  su += stride;
	}
      /* get min/max of segment i in bu (Z) */
      su = &bu[i*stride+2];
      cuminz = DBL_MAX;
      cumaxz = DBL_MIN;
      for(k = 0; k < cu->order; k++)
	{
	  if(*su < cuminz)
	    cuminz = *su;
	  if(*su > cumaxz)
	    cumaxz = *su;
	  su += stride;
	}

      /* now check against all segments in nbv */
      for(j = 0; j < nbv; j++)
	{
	  overlap = AY_TRUE;
	  /* get min/max of segment j in bv (Z) */
	  sv = &bv[j*stride];
	  cvminx = DBL_MAX;
	  cvmaxx = DBL_MIN;
	  for(k = 0; k < cv->order; k++)
	    {
	      if(*sv < cvminx)
		cvminx = *su;
	      if(*sv > cvmaxx)
		cvmaxx = *sv;
	      sv += stride;
	    }
	  if(!((cumaxx < cvminx)||(cvmaxx < cuminx)))
	    {
	      /* overlap in X => need to check Y */
	      sv = &bv[j*stride+1];
	      cvminy = DBL_MAX;
	      cvmaxy = DBL_MIN;
	      for(k = 0; k < cv->order; k++)
		{
		  if(*sv < cvminy)
		    cvminy = *su;
		  if(*sv > cvmaxy)
		    cvmaxy = *sv;
		  sv += stride;
		}
	      if(!((cumaxy < cvminy)||(cvmaxy < cuminy)))
		{
		  /* overlap in Y => need to check Z */
		  sv = &bv[j*stride+2];
		  cvminz = DBL_MAX;
		  cvmaxz = DBL_MIN;
		  for(k = 0; k < cv->order; k++)
		    {
		      if(*sv < cvminz)
			cvminz = *su;
		      if(*sv > cvmaxz)
			cvmaxz = *sv;
		      sv += stride;
		    }
		  if((cumaxz < cvminz)||(cvmaxz < cuminz))
		    {
		      /* no overlap in Z */
		      overlap = AY_FALSE;
		    }
		}
	      else
		{
		  overlap = AY_FALSE;
		}
	    }
	  else
	    {
	      overlap = AY_FALSE;
	    }
	  
	  if(overlap)
	    {
	      /* segments bu[i] and bv[j] overlap atleast in one
		 dimension => calculate smallest distance */


	    }

	} /* for */
    } /* for */


  /* first, calc middle point of control hull */
  su = &(bu[i*stride]);
  for(i = 0; i < nbu; i++)
    {
      k = 0;
      for(j = 0; j < cu->order; j++)
	{
	  

	}
      k+=stride;
    } /* for */




  sv = &(bv[j*stride]);

  /* compare x component */


 return ay_status;
} /* ay_nct_intersect */


/* ay_nct_intersectca:
 *
 */
int
ay_nct_intersectca(ay_object *cu, ay_object *cv, double *intersections)
{
 int ay_status = AY_OK;

 return ay_status;
} /* ay_nct_intersectca */


/* ay_nct_makecompatible:
 *
 */
int
ay_nct_makecompatible(ay_object *cu)
{
 int ay_status = AY_OK;
 ay_object *o;
 ay_nurbcurve_object *curve = NULL;
 int max_order = 0;
 int stride, nh = 0, numknots = 0, t = 0, i, j, a, b;
 int Ualen = 0, Ublen = 0, Ubarlen = 0, clamp_me;
 double *Uh = NULL, *Qw = NULL, *realUh = NULL, *realQw = NULL;
 double *Ubar = NULL, *Ua = NULL, *Ub = NULL;
 double u = 0.0;

  /* clamp curves */
  o = cu;
  while(o)
    {
      curve = (ay_nurbcurve_object *) o->refine;
      clamp_me = AY_FALSE;
      if(curve->knot_type == AY_KTBSPLINE)
	{
	  clamp_me = AY_TRUE;
	}
      else
	{
	  if(curve->knot_type == AY_KTCUSTOM)
	    {
	      a = 1;
	      u = curve->knotv[0];
	      for(i = 1; i < curve->order; i++)
		if(u == curve->knotv[i])
		  a++;

	      j = curve->length+curve->order-1;
	      b = 1;
	      u = curve->knotv[j];
	      for(i = j-1; i >= curve->length; i--)
		if(u == curve->knotv[i])
		  b++;

	      if((a < curve->order) || (b < curve->order))
		{
		  clamp_me = AY_TRUE;
		}
	    } /* if */
	} /* if */

      if(clamp_me)
	ay_status = ay_nct_clamp(curve);

      o = o->next;
    } /* while */

  /* rescale knots to range 0.0 - 1.0 */
  o = cu;
  while(o)
    {
      curve = (ay_nurbcurve_object *) o->refine;
      if(curve->knotv[0] != 0.0 || curve->knotv[
	  curve->length+curve->order-1] != 1.0)
	{
	  ay_status = ay_knots_rescaleknotv(curve->length+curve->order,
					    curve->knotv);
	}
      o = o->next;
    }

  /* find max order */
  o = cu;
  while(o)
    {
      curve = (ay_nurbcurve_object *) o->refine;
      if(curve->order > max_order)
	max_order = curve->order;

      o = o->next;
    }

  /* degree elevate */
  o = cu;
  while(o)
    {
      curve = (ay_nurbcurve_object *) o->refine;
      if(curve->order < max_order)
	{
	  stride = 4;
	  t = max_order - curve->order;

	  /* alloc new knotv & new controlv */
	  if(!(Uh = calloc((curve->length + curve->length*t +
			    curve->order + t),
			   sizeof(double))))
	    {
	      return AY_EOMEM;
	    }
	  if(!(Qw = calloc((curve->length + curve->length*t)*4,
			   sizeof(double))))
	    {
	      free(Uh);
	      return AY_EOMEM;
	    }

	  ay_status = ay_nb_DegreeElevateCurve(stride, curve->length-1,
		       curve->order-1, curve->knotv, curve->controlv,
		       t, &nh, Uh, Qw);

	  if(ay_status)
	    {
	      free(Uh); free(Qw); return ay_status;
	    }

	  if(!(realQw = realloc(Qw, nh*4*sizeof(double))))
	    {
	      return AY_EOMEM;
	    }

	  if(!(realUh = realloc(Uh, (nh+curve->order+t)*sizeof(double))))
	    {
	      return AY_EOMEM;
	    }

	  free(curve->knotv);
	  curve->knotv = realUh;

	  free(curve->controlv);
	  curve->controlv = realQw;

	  curve->knot_type = AY_KTCUSTOM;

	  curve->order += t;

	  curve->length = nh;

	  numknots += (curve->order + curve->length);

	  Qw = NULL;
	  Uh = NULL;
	  realQw = NULL;
	  realUh = NULL;
	} /* if */
      o = o->next;
    } /* while */

  /* unify knots */
  o = cu;
  curve = (ay_nurbcurve_object *) o->refine;
  Ua = curve->knotv;
  Ualen = curve->length+curve->order;

  o = o->next;
  while(o)
    {
      curve = (ay_nurbcurve_object *)o->refine;
      Ub = curve->knotv;
      Ublen = curve->length+curve->order;

      ay_status = ay_knots_unify(Ua, Ualen, Ub, Ublen, &Ubar, &Ubarlen);

      if(ay_status)
	{
	  fprintf(stderr,"Memory may have leaked!\n");
	  return ay_status;
	}

      Ua = Ubar;
      Ualen = Ubarlen;

      o = o->next;
    } /* while */

  /* merge knots */
  o = cu;
  while(o)
    {
      curve = (ay_nurbcurve_object *) o->refine;

      ay_status = ay_knots_merge(curve, Ubar, Ubarlen);
      if(ay_status)
	{
	  free(Ubar); return ay_status;
	}
      o = o->next;
    } /* while */

 return ay_status;
} /* ay_nct_makecompatible */


