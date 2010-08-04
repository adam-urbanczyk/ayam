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

/* nct.c NURBS curve tools */

/* local variables: */

char ay_nct_ncname[] = "NCurve";


/* functions: */

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

  if(!controlv)
    {
      if(!(newcontrolv = calloc(4*length, sizeof(double))))
	{
	  free(curve);
	  return AY_EOMEM;
	}

      curve->controlv = newcontrolv;
    }
  else
    {
      curve->controlv = controlv;
      /* XXXX check user supplied control vector? */
      curve->is_rat = ay_nct_israt(curve);
    } /* if */

  if(knot_type != AY_KTCUSTOM && knotv == NULL)
    {
      /* we need to create knots */
      ay_status = ay_knots_createnc(curve);
      if(ay_status)
	{
	  if(newcontrolv)
	    free(newcontrolv);
	  free(curve);
	  return ay_status;
	}
    }
  else
    {
      /* caller specified own knots */
      curve->knotv = knotv;
    } /* if */

  ay_nct_settype(curve);

  /* return result */
  *curveptr = curve;

 return AY_OK;
} /* ay_nct_create */


/* ay_nct_destroy:
 *   destroy a NURBS curve object
 */
int
ay_nct_destroy(ay_nurbcurve_object *curve)
{

  if(!curve)
    return AY_ENULL;

  if(curve->mpoints)
    ay_nct_clearmp(curve);

  if(curve->controlv)
    free(curve->controlv);

  if(curve->knotv)
    free(curve->knotv);

  free(curve);

  return AY_OK;
} /* ay_nct_destroy */


/* ay_nct_clearmp:
 *  delete all mpoints from curve c
 */
void
ay_nct_clearmp(ay_nurbcurve_object *c)
{
  ay_mpoint *next = NULL, *p = NULL;

  if(!c)
    return;

  p = c->mpoints;

  while(p)
    {
      next = p->next;
      if(p->points)
	free(p->points);
      if(p->indices)
	free(p->indices);
      free(p);
      p = next;
    } /* while */

  c->mpoints = NULL;

 return;
} /* ay_nct_clearmp */


/* ay_nct_recreatemp:
 *  recreate mpoints from identical control points
 */
int
ay_nct_recreatemp(ay_nurbcurve_object *c)
{
 int ay_status = AY_OK;
 ay_mpoint *p = NULL, *new = NULL;
 double *ta, *tb, **tmpp = NULL;
 unsigned int *tmpi = NULL;
 int found = AY_FALSE, i, j, count;
 int stride = 4;

  if(!c)
    return AY_ENULL;

  ay_nct_clearmp(c);

  if(!c->createmp)
    return AY_OK;

  if(!(tmpp = calloc(c->length, sizeof(double *))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(tmpi = calloc(c->length, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }

  ta = c->controlv;
  for(j = 0; j < (c->length-1); j++)
    {
      tb = ta;

      /* count identical points */
      count = 0;
      for(i = j; i < c->length; i++)
	{
	  if(AY_V4COMP(ta, tb))
	    {
	      tmpp[count] = tb;
	      tmpi[count] = i;
	      count++;
	    }
	  tb += stride;
	} /* for */

      /* create new mp, if it is not already there */
      if(count > 1)
	{
	  p = c->mpoints;
	  found = AY_FALSE;
	  while(p && !found)
	    {
	      if(AY_V4COMP(ta, p->points[0]))
		{
		  found = AY_TRUE;
		  break;
		}

	      p = p->next;
	    } /* while */

	  if(!found)
	    {
	      if(!(new = calloc(1, sizeof(ay_mpoint))))
		{ ay_status = AY_EOMEM; goto cleanup; }
	      if(!(new->points = calloc(count, sizeof(double *))))
		{ ay_status = AY_EOMEM; goto cleanup; }
	      if(!(new->indices = calloc(count, sizeof(unsigned int))))
		{ ay_status = AY_EOMEM; goto cleanup; }
	      new->multiplicity = count;
	      memcpy(new->points, tmpp, count*sizeof(double *));
	      memcpy(new->indices, tmpi, count*sizeof(unsigned int));

	      new->next = c->mpoints;
	      c->mpoints = new;
	      new = NULL;
	    } /* if */

	} /* if */

      ta += stride;
    } /* for */

cleanup:

  if(tmpp)
    free(tmpp);
  if(tmpi)
    free(tmpi);

  if(new)
    {
      if(new->points)
	free(new->points);
      if(new->indices)
	free(new->indices);
      free(new);
    }

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
 ay_mpoint *new = NULL, *p = NULL, *t = NULL, **last = NULL;
 ay_point *selp = NULL;
 double *first = NULL;
 int count = 0, i, found = AY_FALSE;
 char fname[] = "collapseselp";

  if(!o)
    return AY_ENULL;

  if(o->type != AY_IDNCURVE)
    {
      ay_error(AY_EWTYPE, fname, ay_nct_ncname);
      return AY_ERROR;
    }

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

  if(!(new = calloc(1,sizeof(ay_mpoint))))
    return AY_EOMEM;
  if(!(new->points = calloc(count, sizeof(double *))))
    { free(new); return AY_EOMEM; }
  if(!(new->indices = calloc(count, sizeof(unsigned int))))
    { free(new->points); free(new); return AY_EOMEM; }

  /* fill mpoint */
  selp = o->selp;
  i = 0;
  first = selp->point;
  while(selp)
    {
      new->points[i] = selp->point;
      new->indices[i] = selp->index;
      i++;
      if(selp->homogenous)
	memcpy(selp->point, first, 4*sizeof(double));
      else
	memcpy(selp->point, first, 3*sizeof(double));

      selp = selp->next;
    } /* while */
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
	      free(p->indices);
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
 ay_mpoint *p = NULL, *t = NULL, **last = NULL;
 ay_point *selp = NULL;
 int found = AY_FALSE, i, err = AY_TRUE;
 char fname[] = "explodemp";

  if(!o)
    return AY_ENULL;

  if(o->type != AY_IDNCURVE)
    {
      ay_error(AY_EWTYPE, fname, ay_nct_ncname);
      return AY_ERROR;
    }

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
		{
		  found = AY_TRUE;
		}
	    } /* for */

	  if(found)
	    {
	      *last = p->next;
	      t = p->next;
	      free(p->points);
	      free(p->indices);
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

  if(!curve)
    return AY_ENULL;

  /* nothing to do? */
  if(new_length == curve->length)
    return AY_OK;

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
	{
	  free(ncontrolv);
	  return AY_EOMEM;
	}
      cv = curve->controlv;

      while(new)
	{
	  for(i = 0; i < (curve->length-1); i++)
	    {
	      if((cv[i*4] != cv[(i+1)*4]) ||
		 (cv[i*4+1] != cv[(i+1)*4+1]) ||
		 (cv[i*4+2] != cv[(i+1)*4+2]))
		{
		  if(!((curve->type == AY_CTPERIODIC) &&
		       (i > (curve->length-curve->order))))
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
	  b += 4;

	  if((cv[i*4] != cv[(i+1)*4]) ||
	     (cv[i*4+1] != cv[(i+1)*4+1]) ||
	     (cv[i*4+2] != cv[(i+1)*4+2]))
	    {
	      if(!((curve->type == AY_CTPERIODIC) &&
		   (i > (curve->length-curve->order))))
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
		} /* for */
	    } /* if */

	  a += 4;
	} /* for */
      memcpy(&ncontrolv[(new_length-1)*4],
	     &(curve->controlv[(curve->length-1)*4]),
	     4*sizeof(double));

      free(newpersec);

    } /* if */

  free(curve->controlv);
  curve->controlv = ncontrolv;

  curve->length = new_length;

  if(curve->type)
    {
      ay_status = ay_nct_close(curve);
      if(ay_status)
	ay_error(AY_ERROR, fname, "Could not close curve!");
    }

 return ay_status;
} /* ay_nct_resize */


/* ay_nct_close:
 *  close a NURBS curve, or make it periodic
 */
int
ay_nct_close(ay_nurbcurve_object *curve)
{
 int ay_status = AY_OK;
 double *controlv = NULL, *end = NULL;
 int i;

  if(!curve)
    return AY_ENULL;

  /* close curve */
  if(curve->type == AY_CTCLOSED)
    {
      end = &(curve->controlv[(curve->length*4)-4]);
      memcpy(end, curve->controlv, 4*sizeof(double));
    } /* if */

  /* make curve periodic */
  if(curve->type == AY_CTPERIODIC)
    {
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
	}
      else
	{
	  curve->type = AY_CTOPEN;
	  return AY_ERROR;
	} /* if */
    } /* if */

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

      i += 4;
      j -= 4;
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
    } /* if */

 return AY_OK;
} /* ay_nct_revert */


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

  if(!curve)
    return AY_ENULL;

  knotv = curve->knotv;
  if(newknotv)
    {
      X = newknotv;
    }

  if((curve->type == AY_CTPERIODIC) && (X == NULL))
    {
      /* special case: curves marked periodic;
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
		} /* if */

	      j++;
	    } /* for */

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
	      return AY_ERROR;
	    }
	} /* if */

      if(!(Ubar = calloc((curve->length + curve->order + count),
			 sizeof(double))))
	{
	  free(X);
	  ay_error(AY_EOMEM, fname, NULL);
	  return AY_ERROR;
	}
      if(!(Qw = calloc((curve->length + count+2)*4, sizeof(double))))
	{
	  free(X); free(Ubar);
	  ay_error(AY_EOMEM, fname, NULL);
	  return AY_ERROR;
	}

      if(newknotv == NULL)
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
	    } /* for */
	}
      else
	{
	  count = newknotvlen;

	  if(!(Ubar = calloc((curve->length + curve->order + count),
			     sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return AY_ERROR;
	    }
	  if(!(Qw = calloc((curve->length + count+2)*4, sizeof(double))))
	    {
	      free(Ubar);
	      ay_error(AY_EOMEM, fname, NULL);
	      return AY_ERROR;
	    }
	} /* if */

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
      if(newknotvlen > 0)
	curve->knot_type = AY_KTCUSTOM;
      ay_nct_recreatemp(curve);

    } /* if curve periodic */

 return AY_OK;
} /* ay_nct_refine */


/* ay_nct_refinetcmd:
 *  Tcl interface for NURBS curve refine tool
 */
int
ay_nct_refinetcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_nurbcurve_object *curve = NULL;
 int aknotc = 0, i;
 double *X = NULL;
 char **aknotv;

  if(argc > 1)
    {
      Tcl_SplitList(interp, argv[1], &aknotc, &aknotv);

      if(!(X = calloc(aknotc, sizeof(double))))
	{
	  ay_error(AY_EOMEM,argv[0],NULL);
	  Tcl_Free((char *) aknotv);
	  return TCL_OK;
	}

      for(i = 0; i < aknotc; i++)
	{
	  Tcl_GetDouble(interp, aknotv[i], &X[i]);
	} /* for */

      Tcl_Free((char *) aknotv);
    } /* if */

  while(sel)
    {
      o = sel->object;

      if(o->type == AY_IDNCURVE)
	{
	  if(o->selp)
	    {
	      ay_selp_clear(o);
	    }
	  curve = (ay_nurbcurve_object *)o->refine;
	  ay_status = ay_nct_refine(curve, X, aknotc);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "refine operation failed");
	    }

	  o->modified = AY_TRUE;

	  /* re-create tesselation of curve */
	  ay_notify_force(sel->object);
	}
      else
	{
	  ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
	}

      sel = sel->next;
    } /* while */

  if(X)
    {
      free(X);
    }

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_refinetcmd */


/* ay_nct_clamp:
 *  clamp NURBS curve, it is safe to call this with half clamped curves
 *  side: 0 - clamp both ends, 1 - clamp only start, 2 - clamp only end
 */
int
ay_nct_clamp(ay_nurbcurve_object *curve, int side)
{
 int ay_status = AY_OK;
 double *newcontrolv = NULL, *newknotv = NULL;
 double u;
 int stride = 4, i, j, k, s, nq, rs = 0, re = 0;

  if(!curve)
    return AY_ENULL;

  if(side == 0)
    {
      if((curve->knot_type == AY_KTBSPLINE) ||
	 ((curve->type == AY_CTPERIODIC) &&
	  ((curve->knot_type == AY_KTCHORDAL) ||
	   (curve->knot_type == AY_KTCENTRI))))
	{
	  return ay_nct_clampperiodic(curve);
	}
    }

  /* clamp start? */
  if(side == 0 || side == 1)
    {
      /* the next fragment also counts the phantom knot! */
      u = curve->knotv[0];
      s = 1;
      for(i = 1; i < curve->order; i++)
	{
	  if(fabs(u - curve->knotv[i]) < AY_EPSILON)
	    s++;
	  else
	    break;
	}

      if(s < curve->order)
	{
	  /* clamp start */
	  u = curve->knotv[curve->order-1];

	  k = ay_nb_FindSpanMult(curve->length-1, curve->order-1, u,
				 curve->knotv, &s);

	  rs = (curve->order - 1) - s;
	  curve->length += rs;

	  newcontrolv = NULL;
	  if(!(newcontrolv = calloc(curve->length*stride, sizeof(double))))
	    return AY_EOMEM;

	  newknotv = NULL;
	  if(!(newknotv = calloc(curve->length+curve->order, sizeof(double))))
	    { free(newcontrolv); return AY_EOMEM; }

	  ay_status = ay_nb_CurveInsertKnot4D(curve->length-rs-1,
			 curve->order-1, curve->knotv, curve->controlv, u, k,
			 s, rs, &nq, newknotv, newcontrolv);

	  if(ay_status)
	    {
	      free(newknotv);
	      free(newcontrolv);
	      return ay_status;
	    }

	  free(curve->controlv);
	  curve->controlv = newcontrolv;

	  free(curve->knotv);
	  curve->knotv = newknotv;
	} /* if */
    } /* if */

  /* clamp end? */
  if(side == 0 || side == 2)
    {
      /* the next fragment also counts the phantom knot! */
      s = 1;
      j = curve->length+curve->order-1;
      u = curve->knotv[j];
      for(i = 1; i < curve->order; i++)
	{
	  if(fabs(u - curve->knotv[j-i]) < AY_EPSILON)
	    s++;
	  else
	    break;
	}

      if(s < curve->order)
	{
	  /* clamp end */
	  u = curve->knotv[curve->length];

	  k = ay_nb_FindSpanMult(curve->length-1, curve->order-1, u,
				 curve->knotv, &s);

	  re = (curve->order - 1) - s;
	  curve->length += re;

	  newcontrolv = NULL;
	  if(!(newcontrolv = calloc(curve->length*stride, sizeof(double))))
	    return AY_EOMEM;

	  newknotv = NULL;
	  if(!(newknotv = calloc(curve->length+curve->order, sizeof(double))))
	    { free(newcontrolv); return AY_EOMEM; }

	  ay_status = ay_nb_CurveInsertKnot4D(curve->length-re-1,
			 curve->order-1, curve->knotv, curve->controlv, u, k,
			 s, re, &nq, newknotv, newcontrolv);

	  if(ay_status)
	    {
	      free(newknotv);
	      free(newcontrolv);
	      return ay_status;
	    }

	  free(curve->controlv);
	  curve->controlv = newcontrolv;

	  free(curve->knotv);
	  curve->knotv = newknotv;
	} /* if */
    } /* if */

  if(rs == 0 && re == 0)
    {
      return AY_OK;
    }

  /* create new controlv, knotv, discarding the first rs and last re knots */
  switch(side)
    {
    case 0:
      curve->length -= rs+re;
      break;
    case 1:
      curve->length -= rs;
      break;
    case 2:
      curve->length -= re;
      break;
    default:
      break;
    }

  newcontrolv = NULL;
  if(!(newcontrolv = calloc(curve->length*stride, sizeof(double))))
    return AY_EOMEM;

  newknotv = NULL;
  if(!(newknotv = calloc(curve->length+curve->order, sizeof(double))))
    { free(newcontrolv); return AY_EOMEM; }

  switch(side)
    {
    case 0:
    case 1:
      /* clamped both or start: ignore first rs knots */
      memcpy(newcontrolv, &(curve->controlv[rs*stride]),
	     curve->length*stride*sizeof(double));

      memcpy(newknotv, &(curve->knotv[rs]),
	     (curve->length+curve->order)*sizeof(double));
      /* improve phantom knot */
      newknotv[0] = newknotv[1];
      break;
    case 2:
      /* clamped end: ignore last re knots */
      memcpy(newcontrolv, curve->controlv,
	     curve->length*stride*sizeof(double));

      memcpy(newknotv, curve->knotv,
	     (curve->length+curve->order)*sizeof(double));
      /* improve phantom knot */
      newknotv[curve->length+curve->order-1] =
	newknotv[curve->length+curve->order-2];
      break;
    default:
      break;
    }

  free(curve->controlv);
  curve->controlv = newcontrolv;

  free(curve->knotv);
  curve->knotv = newknotv;

 return AY_OK;
} /* ay_nct_clamp */


/* ay_nct_clampperiodic:
 *  fast clamp for curves with periodic knot vectors (e.g. AY_KTBSPLINE),
 *  always clamps both ends
 */
int
ay_nct_clampperiodic(ay_nurbcurve_object *curve)
{
 int ay_status = AY_OK;
 double *newcontrolv = NULL, *newknotv = NULL;
 int stride = 4, p, np, nq;

  if(!curve)
    return AY_ENULL;

  p = curve->order-1;
  np = curve->length;
  nq = np+(p*2);

  /* get some fresh memory to work on */
  if(!(newcontrolv = calloc(nq*stride, sizeof(double))))
    return AY_EOMEM;

  if(!(newknotv = calloc(nq+curve->order, sizeof(double))))
    { free(newcontrolv); return AY_EOMEM; }

  /* insert knots at start */
  ay_status = ay_nb_CurveInsertKnot4D(np-1, p, curve->knotv, curve->controlv,
                        curve->knotv[p], p, 0, p, &nq, newknotv, newcontrolv);

  if(ay_status)
    {
      free(newknotv);
      free(newcontrolv);
      return ay_status;
    }

  /* insert knots at end (nq is now np+p-1!) */
  ay_status = ay_nb_CurveInsertKnot4D(nq, p, newknotv, newcontrolv,
			newknotv[nq+1], nq, 0, p, &nq, newknotv, newcontrolv);

  if(ay_status)
    {
      free(newknotv);
      free(newcontrolv);
      return ay_status;
    }

  /* copy results back to curve, ignoring the first p and last p cv/knots */
  memcpy(curve->controlv, &(newcontrolv[p*stride]),
	 curve->length*stride*sizeof(double));

  memcpy(curve->knotv, &(newknotv[p]),
	 (curve->length+curve->order)*sizeof(double));

  free(newcontrolv);

  free(newknotv);

 return AY_OK;
} /* ay_nct_clampperiodic */


/* ay_nct_clamptcmd:
 *  Tcl interface for NURBS curve clamping tool
 */
int
ay_nct_clamptcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_nurbcurve_object *curve = NULL;
 int side = 0;


  if(argc >= 2)
    {
      Tcl_GetInt(interp, argv[1], &side);
    }

  while(sel)
    {

      if(sel->object)
	{
	  if(sel->object->type == AY_IDNCURVE)
	    {
	      if(sel->object->selp)
		ay_selp_clear(sel->object);

	      curve = (ay_nurbcurve_object *)sel->object->refine;

	      if((curve->knot_type == AY_KTNURB) ||
		 (curve->knot_type == AY_KTBEZIER))
		break;

	      ay_status = ay_nct_clamp(curve, side);

	      if(ay_status)
		{
		  ay_error(ay_status, argv[0], "clamp operation failed");
		  return TCL_OK;
		}

	      curve->knot_type = AY_KTCUSTOM;

	      /* update pointers to controlv */
	      ay_status = ay_nct_recreatemp(curve);
	      sel->object->modified = AY_TRUE;

	      /* re-create tesselation of curve */
	      ay_notify_force(sel->object);
	    }
	  else
	    {
	      ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
	    } /* if */
	} /* if */

    sel = sel->next;
  } /* while */

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
 char fname[] = "elevate";
 int i, j, a, b, clamp_me;
 double u, *Uh = NULL, *Qw = NULL, *realQw = NULL, *realUh = NULL;
 int t = 1, nh = 0;

  if(!curve)
    return AY_ENULL;

  if(curve->order >= new_order)
    return AY_OK;
  else
    t = new_order - curve->order;

  /* clamp the curve? */
  clamp_me = AY_FALSE;

  if((curve->knot_type == AY_KTBSPLINE) ||
     ((curve->type == AY_CTPERIODIC) &&
      ((curve->knot_type == AY_KTCHORDAL) ||
       (curve->knot_type == AY_KTCENTRI))))
    {
      ay_status = ay_nct_clampperiodic(curve);
    }
  else
    {
      if(curve->knot_type == AY_KTCUSTOM)
	{
	  a = 1;
	  u = curve->knotv[0];
	  for(i = 1; i < curve->order; i++)
	    if(fabs(u - curve->knotv[i]) < AY_EPSILON)
	      a++;

	  j = curve->length+curve->order-1;
	  b = 1;
	  u = curve->knotv[j];
	  for(i = j; i >= curve->length; i--)
	    if(fabs(u - curve->knotv[i]) < AY_EPSILON)
	      b++;

	  if((a < curve->order) || (b < curve->order))
	    {
	      clamp_me = AY_TRUE;
	    } /* if */
	} /* if */
    } /* if */

  if(clamp_me)
    {
      ay_status = ay_nct_clamp(curve, 0);
    } /* if */

  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "clamp operation failed");
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
      free(Uh); free(Qw);
      return AY_EOMEM;
    }

  if(!(realUh = realloc(Uh, (nh+curve->order+t)*sizeof(double))))
    {
      free(Uh); free(realQw);
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
 *  Tcl interface for NURBS curve order elevation tool
 */
int
ay_nct_elevatetcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_nurbcurve_object *curve = NULL;
 int clamp_me;
 double *Uh = NULL, *Qw = NULL, *realQw = NULL, *realUh = NULL;
 int t = 1, nh = 0;

  if(argc >= 2)
    {
      Tcl_GetInt(interp, argv[1], &t);
    }

  while(sel)
    {
      if(sel->object->type == AY_IDNCURVE)
	{
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  curve = (ay_nurbcurve_object *)sel->object->refine;

	  /* clamp the curve? */
	  clamp_me = AY_FALSE;

	  if((curve->knot_type == AY_KTBSPLINE) ||
	     ((curve->type == AY_CTPERIODIC) &&
	      ((curve->knot_type == AY_KTCHORDAL) ||
	       (curve->knot_type == AY_KTCENTRI))))
	    {
	      ay_status = ay_nct_clampperiodic(curve);
	    }
	  else
	    {
	      if(curve->knot_type == AY_KTCUSTOM)
		{
		  clamp_me = AY_TRUE;
		}
	    }

	  if(clamp_me)
	    {
	      ay_status = ay_nct_clamp(curve, 0);
	    }

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "clamp operation failed");
	    }

	  /* alloc new knotv & new controlv */
	  if(!(Uh = calloc((curve->length + curve->length*t +
			    curve->order + t),
			     sizeof(double))))
	    {
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }
	  if(!(Qw = calloc((curve->length + curve->length*t)*4,
			   sizeof(double))))
	    {
	      free(Uh);
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }

	  /* fill Uh & Qw */
	  ay_status = ay_nb_DegreeElevateCurve(4, curve->length-1,
			curve->order-1, curve->knotv,
			curve->controlv, t, &nh, Uh, Qw);

	  if(ay_status)
	    {
	      ay_error(ay_status,argv[0],"degree elevation failed");
	      free(Uh); free(Qw); return TCL_OK;
	    }

	  if(!(realQw = realloc(Qw, nh*4* sizeof(double))))
	    {
	      free(Uh); free(Qw);
	      return TCL_OK;
	    }

	  if(!(realUh = realloc(Uh, (nh+curve->order+t)*sizeof(double))))
	    {
	      free(Uh); free(realQw);
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

	  /* update pointers to controlv */
	  ay_nct_recreatemp(curve);
	  sel->object->modified = AY_TRUE;

	  /* re-create tesselation of curve */
	  ay_notify_force(sel->object);
	}
      else
	{
	  ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_elevatetcmd */


/* ay_nct_insertkntcmd:
 *  Tcl interface for NURBS curve knot insertion tool
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
 int stride = 4, k = 0, s = 0, r = 0, nq = 0;

  if(argc < 3)
    {
      ay_error(AY_EARGS, argv[0], "u r");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      src = sel->object;
      if(src->type != AY_IDNCURVE)
	{
	  ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
	}
      else
	{
	  /* remove all selected points */
	  if(sel->object->selp)
	    {
	      ay_selp_clear(sel->object);
	    }

	  curve = (ay_nurbcurve_object*)src->refine;
	  knots = curve->knotv;

	  Tcl_GetDouble(interp, argv[1], &u);

	  if((u < knots[curve->order-1]) || (u > knots[curve->length]))
	    {
	      ay_error(AY_ERROR, argv[0], "Parameter u out of range.");
	      return TCL_OK;
	    }

	  k = 0;

	  k = ay_nb_FindSpanMult(curve->length-1, curve->order-1, u,
				 knots, &s);

	  Tcl_GetInt(interp, argv[2], &r);

	  if(curve->order < r+s)
	    {
	      ay_error(AY_ERROR, argv[0],
			 "Knot insertion leads to illegal knot sequence.");
	      return TCL_OK;
	    }

	  curve->length += r;

	  if(!(newcontrolv = calloc(curve->length*stride, sizeof(double))))
	    {
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }
	  if(!(newknotv = calloc(curve->length+curve->order, sizeof(double))))
	    {
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }

	  ay_status = ay_nb_CurveInsertKnot4D(curve->length-r-1,
			  curve->order-1, curve->knotv, curve->controlv, u, k,
			  s, r, &nq, newknotv, newcontrolv);

	  if(ay_status)
	    return ay_status;

	  free(curve->controlv);
	  curve->controlv = newcontrolv;

	  free(curve->knotv);
	  curve->knotv = newknotv;
	  curve->knot_type = AY_KTCUSTOM;

	  ay_nct_recreatemp(curve);

	  src->modified = AY_TRUE;

	  /* re-create tesselation of curve */
	  ay_notify_force(sel->object);
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

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      switch(sel->object->type)
	{
	case AY_IDNCURVE:
	  {
	    ay_status = ay_nct_collapseselp(sel->object);
	    if(ay_status)
	      {
		ay_error(ay_status, argv[0], "Collapse operation failed!");
	      }

	    if(sel->object->selp)
	      {
		ay_selp_clear(sel->object);
	      }
	    sel->object->modified = AY_TRUE;

	    /* re-create tesselation of curve */
	    ay_notify_force(sel->object);
	  }
	  break;
	case AY_IDNPATCH:
	  {
	    ay_status = ay_npt_collapseselp(sel->object);
	    if(ay_status)
	      {
		ay_error(ay_status, argv[0], "Collapse operation failed!");
	      }

	    if(sel->object->selp)
	      {
		ay_selp_clear(sel->object);
	      }
	    sel->object->modified = AY_TRUE;
	  }
	  break;
	default:
	  {
	    ay_error(AY_EWTYPE, argv[0], "NCurve, NPatch");
	  }
	  break;
	} /* switch */

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

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      switch(sel->object->type)
	{
	case AY_IDNCURVE:
	  {
	    ay_status = ay_nct_explodemp(sel->object);
	    if(ay_status)
	      {
		ay_error(ay_status,argv[0],"Explode operation failed!");
	      }

 	    if(sel->object->selp)
	      {
		ay_selp_clear(sel->object);
	      }
	    sel->object->modified = AY_TRUE;
	    /* re-create tesselation of curve */
	    ay_notify_force(sel->object);
	  }
	  break;
	case AY_IDNPATCH:
	  {
	    ay_status = ay_npt_explodemp(sel->object);
	    if(ay_status)
	      {
		ay_error(ay_status,argv[0],"Explode operation failed!");
	      }

 	    if(sel->object->selp)
	      {
		ay_selp_clear(sel->object);
	      }
	    sel->object->modified = AY_TRUE;
	  }
	  break;
	default:
	  {
	    ay_error(AY_EWTYPE, argv[0], "NCurve, NPatch");
	    return TCL_OK;
	  }
	  break;
	} /* switch */

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
 *
 *  XXXX ToDo: use gluPickMatrix() to speed this up
 *
 */
int
ay_nct_findu(struct Togl *togl, ay_object *o,
	     double *winXY, double *worldXYZ, double *u)
{
 int ay_status = AY_OK;
 /*  int width = Togl_Width(togl);*/
 int height = Togl_Height(togl);
 GLint viewport[4];
 GLdouble modelMatrix[16], projMatrix[16], winx, winy;
 GLfloat winz = 0.0f;
 double m[16], mi[16];
 GLfloat pixel1[3] = {0.9f,0.9f,0.9f}, pixel[3] = {0.0f,0.0f,0.0f};
 ay_nurbcurve_object *c = NULL;
 int dx[25] = {0,1,1,0,-1,-1,-1,0,1, 2,2,2,1,0,-1,-2,-2,-2,-2,-2,-1,0,1,2,2};
 int dy[25] = {0,0,-1,-1,-1,0,1,1,1, 0,-1,-2,-2,-2,-2,-2,-1,0,1,2,2,2,2,2,1};
 int found, i = 0, j = 0, k = 0, /*maxtry = 1000,*/ stride, samples = 10;
 int starti = 0, endi = 0;
 double point[4] = {0}/*, guess = 0.0, e1 = 0.05, e2 = 0.05*/;
 double distance = 0.0, min_distance = 0.0;
 double *cp = NULL, U[10/* XXXX samples! */] = {0}, startu, endu;
 ay_voidfp *arr = NULL;
 ay_drawcb *cb = NULL;

  if(!o)
    return AY_ENULL;

  if(!o->type == AY_IDNCURVE)
    return AY_EWTYPE;

  c = (ay_nurbcurve_object *)o->refine;

  arr = ay_drawcbt.arr;
  cb = (ay_drawcb *)(arr[o->type]);

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
      winx = winXY[0]+dx[i];
      winy = height-winXY[1]-dy[i];
      i++;

      if(i > 24)
	{
	  return AY_ERROR;
	}

      glReadPixels((GLint)winx,(GLint)winy,1,1,
		   GL_RGB,GL_FLOAT,&pixel);

      if((pixel1[0] <= pixel[0]) &&
	 (pixel1[1] <= pixel[1]) &&
	 (pixel1[2] <= pixel[2]))
	{
	  found = AY_TRUE;
	}
    }

  /* get object coordinates of point on curve */
  glGetIntegerv(GL_VIEWPORT, viewport);

  glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
   ay_trafo_getall(ay_currentlevel->next);

   glTranslated(o->movx, o->movy, o->movz);
   ay_quat_torotmatrix(o->quat, m);
   glMultMatrixd(m);
   glScaled(o->scalx, o->scaly, o->scalz);

   /* we operate on selected objects, but those are drawn with
      disabled depth test, which means: no correct z buffer data;
      thus, we simply call the draw callback of the selected object
      here again on an empty z buffer to get correct z buffer data */
   glClear(GL_DEPTH_BUFFER_BIT);
   if(cb)
     ay_status = cb(togl, o);

   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
  glPopMatrix();

  /* get winz */
  glReadPixels((GLint)winx,(GLint)winy,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&winz);

  gluUnProject(winx, winy, (GLdouble)winz, modelMatrix, projMatrix, viewport,
	       &(point[0]), &(point[1]), &(point[2]));

  /* get guess */
  stride = 4;
  if(!(cp = calloc(samples*stride, sizeof(double))))
    return AY_EOMEM;

  startu = c->knotv[c->order-1];
  endu = c->knotv[c->length];

  for(k = 0; k < 7; k++)
    {
      U[0] = startu;
      for(i = 1; i < samples-1; i++)
	{
	  U[i] = U[i-1]+(endu - startu)/(samples-1);
	}
      U[samples-1] = endu;

      for(i = 0; i < samples-1; i++)
       {
	 ay_status = ay_nb_CurvePoint4D(c->length-1, c->order-1, c->knotv,
					c->controlv,
					U[i]+((U[i+1]-U[i])/2.0),
					&(cp[i*stride]));

       }

      j = 0;
      min_distance = DBL_MAX;
      for(i = 0; i < (samples-1); i++)
	{
	  distance = AY_VLEN((point[0] - cp[j]),
			     (point[1] - cp[j+1]),
			     (point[2] - cp[j+2]));

	  if(distance < min_distance)
	    {
	      *u = U[i]+((U[i+1]-U[i])/2.0);
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

  winXY[0] = winx;
  winXY[1] = winy;

  ay_nb_CurvePoint4D(c->length-1, c->order-1, c->knotv,
		     c->controlv, *u, point);



  ay_trafo_invmatrix4(m, mi);

  ay_trafo_apply4(point, mi);

  worldXYZ[0] = point[0];
  worldXYZ[1] = point[1];
  worldXYZ[2] = point[2];

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
 char fname[] = "findU_cb";
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 Tcl_Interp *interp = Togl_Interp(togl);
 int height = Togl_Height(togl);
 double winXY[2] = {0}, worldXYZ[3] = {0};
 static int fvalid = AY_FALSE;
 static double fX = 0.0, fY = 0.0, fwX = 0.0, fwY = 0.0, fwZ = 0.0;
 double u = 0.0;
 Tcl_Obj *to = NULL, *ton = NULL;
 char cmd[] = "puts $u";
 ay_object *o, *pobject = NULL;

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
	      view->markworld[0] = fwX;
	      view->markworld[1] = fwY;
	      view->markworld[2] = fwZ;
	      view->drawmark = AY_TRUE;

	      ay_viewt_updatemark(togl, AY_FALSE);
	    }

	  fvalid = AY_FALSE;
	  return TCL_OK;
	}
    }
  else
    {
      return TCL_OK;
    }

  if(ay_selection)
    {
      if(ay_selection->object->type != AY_IDNCURVE)
	{
	  ay_status = ay_provide_object(ay_selection->object,
					AY_IDNCURVE, &pobject);
	  if(!pobject)
	    {
	      ay_error(AY_EWTYPE, fname, ay_nct_ncname);
	      return TCL_OK;
	    }
	  o = pobject;
	}
      else
	{
	  o = ay_selection->object;
	}

      Tcl_GetDouble(interp, argv[2], &(winXY[0]));
      Tcl_GetDouble(interp, argv[3], &(winXY[1]));

      ay_status = ay_nct_findu(togl, o, winXY, worldXYZ, &u);

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Could not find point on curve!");
	  goto cleanup;
	}

      fvalid = AY_TRUE;
      fX = winXY[0];
      fY = winXY[1];
      fwX = worldXYZ[0];
      fwY = worldXYZ[1];
      fwZ = worldXYZ[2];

      ton = Tcl_NewStringObj("u", -1);
      to = Tcl_NewDoubleObj(u);
      Tcl_ObjSetVar2(interp,ton,NULL,to,TCL_LEAVE_ERR_MSG|TCL_GLOBAL_ONLY);
      Tcl_Eval(interp, cmd);
      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
    }
  else
    {
      ay_error(AY_ENOSEL, fname, NULL);
    }

cleanup:

  if(pobject)
    {
      ay_object_deletemulti(pobject);
    }

 return TCL_OK;
} /* ay_nct_finducb */


/* ay_nct_split:
 *  split NURBCurve object <src> at parametric value <u> into two;
 *  modifies <src>, returns second curve in <result>.
 */
int
ay_nct_split(ay_object *src, double u, ay_object **result)
{
 int ay_status = AY_OK;
 ay_object *new = NULL;
 ay_nurbcurve_object *curve = NULL;
 ay_nurbcurve_object *nc1 = NULL, *nc2 = NULL;
 double *knots = NULL, *newcontrolv = NULL, *newknotv = NULL;
 int stride = 4, k = 0, r = 0, s = 0, nq = 0, nc1len = 0;
 char fname[] = "split";

  if(!src || !result)
    return AY_ENULL;

  if(src->type != AY_IDNCURVE)
    {
      ay_error(AY_EWTYPE, fname, ay_nct_ncname);
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
	    { free(newcontrolv); return AY_EOMEM; }

	  ay_status = ay_nb_CurveInsertKnot4D(curve->length-r-1,
			  curve->order-1, curve->knotv, curve->controlv, u, k,
			  s, r, &nq, newknotv, newcontrolv);

	  if(ay_status)
	    {
	      free(newcontrolv); free(newknotv);
	      return ay_status;
	    }

	  free(curve->controlv);
	  curve->controlv = newcontrolv;

	  free(curve->knotv);
	  curve->knotv = newknotv;
	} /* if */

      curve->knot_type = AY_KTCUSTOM;
      /* create two new curves */
      nc1 = curve;
      nc1->type = AY_CTOPEN;
      ay_status = ay_object_copy(src, &new);

      /*XXXX check result*/

      if(r != 0)
	nc1len = k - (nc1->order-1) + 1 + (curve->order-1-s+r-1)/2 + 1;
      else
	nc1len = k - (nc1->order-1) + 1;

      nc2 = (ay_nurbcurve_object*)new->refine;
      nc2->length = (nc1->length+1) - nc1len;
      nc1->length = nc1len;

      newcontrolv = NULL;
      if(!(newcontrolv = calloc(nc1->length*stride, sizeof(double))))
	{ ay_object_delete(new); return AY_EOMEM; }
      newknotv = NULL;
      if(!(newknotv = calloc(nc1->length+nc1->order, sizeof(double))))
	{ ay_object_delete(new); free(newcontrolv); return AY_EOMEM; }

      memcpy(newcontrolv,nc1->controlv,nc1->length*stride*sizeof(double));

      memcpy(newknotv,nc1->knotv,(nc1->length+nc1->order)*sizeof(double));

      /* improve phantom knot */
      newknotv[curve->length+curve->order-1] =
	newknotv[curve->length+curve->order-2];

      free(nc2->controlv);
      nc2->controlv = NULL;
      free(nc2->knotv);
      nc2->knotv = NULL;

      if(!(nc2->controlv = calloc(nc2->length*stride, sizeof(double))))
	{ ay_object_delete(new); return AY_EOMEM; }

      if(!(nc2->knotv = calloc(nc2->length+nc2->order, sizeof(double))))
	{ ay_object_delete(new); free(nc2->controlv); return AY_EOMEM; }

      memcpy(nc2->controlv,&(nc1->controlv[(nc1->length-1)*stride]),
	     nc2->length*stride*sizeof(double));

      memcpy(nc2->knotv,&(nc1->knotv[nc1->length-1]),
	     (nc2->length+nc2->order)*sizeof(double));
      /* improve phantom knot */
      nc2->knotv[0] = nc2->knotv[1];

      free(nc1->controlv);
      nc1->controlv = newcontrolv;
      free(nc1->knotv);
      nc1->knotv = newknotv;

      ay_nct_recreatemp(nc1);
      ay_nct_recreatemp(nc2);

      nc2->is_rat = nc1->is_rat;

      new->selected = AY_FALSE;
      new->modified = AY_TRUE;
      src->modified = AY_TRUE;

      /* return result */
      *result = new;
    } /* if */

 return AY_OK;
} /* ay_nct_split */


/* ay_nct_splittcmd:
 *  Tcl interface for NURBS curve split tool
 */
int
ay_nct_splittcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *new = NULL;
 double u = 0.0;

  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "u");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  Tcl_GetDouble(interp, argv[1], &u);

  while(sel)
    {
      if(sel->object)
	{
	  /* remove all selected points */
	  if(sel->object->selp)
	    {
	      ay_selp_clear(sel->object);
	    }

	  if(sel->object->type == AY_IDNCURVE)
	    {
	      new = NULL;

	      ay_status = ay_nct_split(sel->object, u, &new);

	      if(ay_status)
		{
		  ay_error(ay_status, argv[0], NULL);
		  return TCL_OK;
		} /* if */

	      ay_status = ay_object_link(new);

	      sel->object->modified = AY_TRUE;

	      /* re-create tesselation of original curve */
	      ay_notify_force(sel->object);
	    }
	  else
	    {
	      ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
	      return TCL_OK;
	    } /* if */
	} /* if */
      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_splittcmd */


/* ay_nct_concattcmd:
 *  Tcl interface for NURBS curve concatenation tool
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
 ay_object *o = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(!sel->next)
    {
      ay_error(AY_ERROR, argv[0], "select two NURB curves!");
      return TCL_OK;
    }

  if((sel->object->type != AY_IDNCURVE) ||
     (sel->next->object->type != AY_IDNCURVE))
    {
      ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
      return TCL_OK;
    }

  if(!(o = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, argv[0], NULL);
      return TCL_OK;
    }

  o->type = AY_IDNCURVE;
  ay_object_defaults(o);

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
      ay_error(AY_EOMEM, argv[0], NULL);
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
      ay_error(ay_status, argv[0], NULL);
      return TCL_OK;
    }

  ay_object_link(o);

  ay_nct_recreatemp((ay_nurbcurve_object *)o->refine);

  o->modified = AY_TRUE;

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_concattcmd */


/* ay_nct_crtncircle:
 *
 */
int
ay_nct_crtncircle(double radius, ay_nurbcurve_object **curve)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *new = NULL;
 double *controlv = NULL, *knotv = NULL;
 int i = 0, stride = 4;
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

  if(!curve)
    return AY_ENULL;

  i = 7;
  controls[i] = sqrt(2.0)/2.0;
  controls[i-2] *= sqrt(2.0)/2.0;
  controls[i-3] *= sqrt(2.0)/2.0;
  i += 8;
  controls[i] = sqrt(2.0)/2.0;
  controls[i-2] *= sqrt(2.0)/2.0;
  controls[i-3] *= sqrt(2.0)/2.0;
  i += 8;
  controls[i] = sqrt(2.0)/2.0;
  controls[i-2] *= sqrt(2.0)/2.0;
  controls[i-3] *= sqrt(2.0)/2.0;
  i += 8;
  controls[i] = sqrt(2.0)/2.0;
  controls[i-2] *= sqrt(2.0)/2.0;
  controls[i-3] *= sqrt(2.0)/2.0;

  for(i = 0; i < 9; i++)
    {
      controls[i*stride] *= radius;
      controls[i*stride+1] *= radius;
    }

  if(!(new = calloc(1, sizeof(ay_nurbcurve_object))))
    return AY_EOMEM;
  if(!(controlv = calloc(9*4, sizeof(double))))
    { free(new); return AY_EOMEM; }
  if(!(knotv = calloc(12, sizeof(double))))
    { free(new); free(controlv); return AY_EOMEM; }

  new->order = 3;
  new->length = 9;
  new->knot_type = AY_KTCUSTOM;
  new->type = AY_CTCLOSED;

  memcpy(controlv, controls, 9*4*sizeof(double));
  new->controlv = controlv;

  memcpy(knotv, knots, 12*sizeof(double));
  new->knotv = knotv;

  new->createmp = AY_TRUE;
  new->is_rat = AY_TRUE;

  *curve = new;

 return ay_status;
} /* ay_nct_crtncircle */


/* ay_nct_crtncirclearc:
 *
 */
int
ay_nct_crtncirclearc(double radius, double arc, ay_nurbcurve_object **curve)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *new = NULL;

  if(!curve)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_nurbcurve_object))))
    return AY_EOMEM;

  if(arc < 0.0)
    {
      ay_status = ay_nb_CreateNurbsCircleArc(radius, arc, 0.0, &(new->length),
					     &new->knotv, &new->controlv);
    }
  else
    {
      ay_status = ay_nb_CreateNurbsCircleArc(radius, 0.0, arc, &(new->length),
					     &new->knotv, &new->controlv);
    }

  if(ay_status)
    { free(new); return ay_status; }

  new->order = 3;
  new->knot_type = AY_KTCUSTOM;
  new->is_rat = AY_TRUE;

  *curve = new;

 return ay_status;
} /* ay_nct_crtncirclearc */


/* ay_nct_crtnhcircle:
 *
 */
int
ay_nct_crtnhcircle(double radius, ay_nurbcurve_object **curve)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *new = NULL;
 double *controlv = NULL, *knotv = NULL;
 int i = 0, stride = 4;
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

  if(!curve)
    return AY_ENULL;

  i = 4;
  controls[i] *= sqrt(2.0)/2.0;
  controls[i+1] *= sqrt(2.0)/2.0;
  controls[i+3] = sqrt(2.0)/2.0;
  i += 8;
  controls[i] *= sqrt(2.0)/2.0;
  controls[i+1] *= sqrt(2.0)/2.0;
  controls[i+3] = sqrt(2.0)/2.0;

  for(i = 0; i < 5; i++)
    {
      controls[i*stride] *= radius;
      controls[i*stride+1] *= radius;
    }

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

  new->is_rat = AY_TRUE;

  *curve = new;

 return ay_status;
} /* ay_nct_crtnhcircle */


/* ay_nct_crtncircletcmd:
 *
 */
int
ay_nct_crtncircletcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 int ay_status;
 double arc = 360.0;
 ay_object *o = NULL;
 double radius = 1.0;
 int i = 1;

  if(argc > 2)
    {
      /* parse args */
      while(i+1 < argc)
	{
	  if(!strcmp(argv[i], "-r"))
	    {
	      sscanf(argv[i+1], "%lg", &radius);
	    }
	  else
	  if(!strcmp(argv[i], "-a"))
	    {
	      sscanf(argv[i+1], "%lg", &arc);
	    }
	  i += 2;
	} /* while */
    } /* if */

  if(!(o = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, argv[0], NULL);
      return TCL_OK;
    }

  o->type = AY_IDNCURVE;
  ay_object_defaults(o);

  /* we create the nurbcurve-object */
  if(argc < 2)
    {
      ay_status = ay_nct_crtncircle(radius,
				    (ay_nurbcurve_object **)&(o->refine));
    }
  else
    {
      if(arc >= 360.0 || arc <= -360.0 || arc == 0.0)
	{
	  ay_status = ay_nct_crtncircle(radius,
					(ay_nurbcurve_object **)&(o->refine));
	}
      else
	{
	  ay_status = ay_nct_crtncirclearc(radius, arc,
					(ay_nurbcurve_object **)&(o->refine));
	}
    } /* if */

  if(ay_status)
    {
      ay_error(ay_status, argv[0], NULL);
      return TCL_OK;
    }

  ay_nct_recreatemp((ay_nurbcurve_object *)o->refine);

  ay_object_link(o);

 return TCL_OK;
} /* ay_nct_crtncircletcmd */


/* ay_nct_crtrecttcmd:
 *
 */
int
ay_nct_crtrecttcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 /*int ay_status = AY_OK;*/
 ay_list_object *lev = ay_currentlevel;
 ay_object *parent = NULL, **last = NULL;
 ay_nurbpatch_object *patch = NULL;
 ay_nurbcurve_object *curve = NULL;
 double def_controls[20] = { 1.0, 1.0, 0.0, 1.0,
			     1.0,-1.0, 0.0, 1.0,
			     -1.0,-1.0, 0.0, 1.0,
			     -1.0, 1.0, 0.0, 1.0,
			     1.0, 1.0, 0.0, 1.0};
 double knots[7] = {0.0, 0.0, 0.25, 0.5, 0.75, 1.0, 1.0};
 int i = 0, create_trim = AY_FALSE;
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;

  if(!(o = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, argv[0], NULL);
      return TCL_OK;
    }

  o->type = AY_IDNCURVE;
  ay_object_defaults(o);

  if(sel && sel->object && (sel->object->type == AY_IDNPATCH))
    {
      create_trim = AY_TRUE;
      patch = (ay_nurbpatch_object *)sel->object->refine;
    }
  else
    {
      if(lev->next)
	{
	  parent = lev->next->object;
	  if(parent)
	    {
	      if(parent->type == AY_IDNPATCH)
		{
		  create_trim = AY_TRUE;
		  patch = (ay_nurbpatch_object *)parent->refine;
		} /* if */
	    } /* if */
	} /* if */
    } /* if */

  if(!(curve = calloc(1,sizeof(ay_nurbcurve_object))))
    {
      ay_error(AY_EOMEM, argv[0], NULL);
      return TCL_OK;
    }

  if(!(curve->controlv = calloc(20, sizeof(double))))
    {
      free(curve);
      ay_error(AY_EOMEM, argv[0], NULL);
      return TCL_OK;
    }

  if(!(curve->knotv = calloc(7, sizeof(double))))
    {
      free(curve->controlv); free(curve);
      ay_error(AY_EOMEM, argv[0], NULL);
      return TCL_OK;
    }

  curve->length = 5;
  curve->order = 2;
  curve->knot_type = AY_KTCUSTOM;
  curve->type = AY_CTCLOSED;
  curve->createmp = AY_TRUE;

  /* fill knotv */
  for(i = 0; i < 7; i++)
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
      curve->controlv[19] = 1.0;
    }
  else
    {
      memcpy(curve->controlv, def_controls, 20*sizeof(double));
    }

  o->type = AY_IDNCURVE;
  o->refine = curve;

  ay_nct_recreatemp(curve);

  if(!patch)
    {
      ay_object_link(o);
    }
  else
    {
      if(!(sel && sel->object && (sel->object->type == AY_IDNPATCH)))
	{
	  ay_object_link(o);
	}
      else
	{
	  parent = sel->object->down;
	  if(!parent)
	    {
	      sel->object->down = o;
	      o->next = ay_endlevel;
	    }
	  else
	    {
	      last = &(sel->object->down);
	      while(parent->next)
		{
		  last = &(parent->next);
		  parent = parent->next;
		} /* while */
	      o->next = *last;
	      *last = o;
	    } /* if */
	} /* if */
    } /* if */

 return TCL_OK;
} /* ay_nct_crtrecttcmd */


/* ay_nct_crtcircbsp:
 */
int
ay_nct_crtcircbsp(int sections, double radius, double arc, int order,
		  ay_nurbcurve_object **result)
{
 int ay_status;
 ay_nurbcurve_object *curve = NULL;
 double *controlv, m[16], angle;
 int len, a, i;

  if((sections < 2) && (fabs(arc) > 180.0))
    return AY_ERROR;

  if((fabs(radius) < AY_EPSILON) || (order < 2))
    return AY_ERROR;

  if(!result)
    return AY_ENULL;

  len = sections+(order-1);

  if(!(controlv = calloc(len*4, sizeof(double))))
    {
      return AY_EOMEM;
    }

  angle = arc/sections;

  ay_trafo_identitymatrix(m);

  if(fabs(arc) < 360.0)
    {
      ay_trafo_rotatematrix((order/2.0)*-angle, 0.0, 0.0, 1.0, m);
    }

  for(i = 0; i < len; i++)
    {
      a = i*4;
      controlv[a] = radius;
      controlv[a+1] = 0.0;
      controlv[a+2] = 0.0;
      controlv[a+3] = 1.0;
      ay_trafo_rotatematrix(angle, 0.0, 0.0, 1.0, m);
      ay_trafo_apply3(&(controlv[a]), m);
    } /* for */

  ay_status = ay_nct_create(order, len, AY_KTBSPLINE, controlv, NULL,
			    &curve);

  *result = curve;

 return ay_status;
} /* ay_nct_crtcircbsp */


/* ay_nct_crtclosedbsptcmd:
 *
 */
int
ay_nct_crtclosedbsptcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_object *o = NULL;
 ay_nurbcurve_object *curve = NULL;
 int sections = 0, order = 4;
 double radius = 1.0, arc = 360.0;

  /* parse args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "sections");
      return TCL_OK;
    }

  Tcl_GetInt(interp, argv[1], &sections);

  if(sections < 1)
    {
      ay_error(AY_ERROR, argv[0], "sections must be >= 1");
      return TCL_OK;
    }

  if(argc > 2)
    {
      Tcl_GetInt(interp, argv[2], &order);
      if(order < 2)
	order = 4;
    }

  if(argc > 3)
    {
      Tcl_GetDouble(interp, argv[3], &arc);
    }

  if(argc > 4)
    {
      Tcl_GetDouble(interp, argv[4], &radius);
    }

  /* create object */
  if(!(o = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, argv[0], NULL);
      return TCL_OK;
    }

  o->type = AY_IDNCURVE;
  ay_object_defaults(o);

  ay_status = ay_nct_crtcircbsp(sections, radius, arc, order,
				(ay_nurbcurve_object**)&(o->refine));

  if(ay_status || !o->refine)
    {
      free(o);
      return TCL_OK;
    }

  curve = (ay_nurbcurve_object*)(o->refine);
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
int
ay_nct_getorientation(ay_nurbcurve_object *curve, double *orient)
{
 double minx, miny;
 int i, j, n, m, stride, found, wrap;
 double *cv = NULL, a[2], b[2], c[2];
 char fname[] = "nct_getorientation";

  if(!curve || !orient)
    return AY_ENULL;

  cv = curve->controlv;
  n = curve->length;
  if(n <= 2)
    {
      ay_error(AY_EWARN, fname, "Need more than 2 control points!");
      return AY_ERROR;
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
      ay_error(AY_ERROR, fname, "Could not determine 3 different points!");
      return AY_ERROR;
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
      ay_error(AY_ERROR, fname, "Could not determine 3 different points!");
      return AY_ERROR;
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

 return AY_OK;
} /* ay_nct_getorientation */


/* ay_nct_isclosed:
 *
 */
int
ay_nct_isclosed(ay_nurbcurve_object *nc)
{
 double u, P1[4], P2[4];

  if(!nc)
    return AY_ENULL;

  u = nc->knotv[nc->order-1];
  ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
		     nc->knotv, nc->controlv,
		     u, P1);

  u = nc->knotv[nc->length];
  ay_nb_CurvePoint4D(nc->length-1, nc->order-1,
		     nc->knotv, nc->controlv,
		     u, P2);

  if((fabs(P1[0]-P2[0]) < AY_EPSILON) &&
     (fabs(P1[1]-P2[1]) < AY_EPSILON) &&
     (fabs(P1[2]-P2[2]) < AY_EPSILON))
    return AY_TRUE;
  else
    return AY_FALSE;

} /* ay_nct_isclosed */


/* ay_nct_settype:
 *  set the type attribute according to the actual configuration
 *  of the NURBS curve <nc>
 */
int
ay_nct_settype(ay_nurbcurve_object *nc)
{
 int stride = 4;

  if(!nc)
    return AY_ENULL;

  if(!ay_nct_isclosed(nc))
    {
      nc->type = AY_CTOPEN;
    }
  else
    {
      if(!memcmp(nc->controlv, &(nc->controlv[(nc->length-1)*stride]),
		 stride*sizeof(double)))
	nc->type = AY_CTCLOSED;
      else
	nc->type = AY_CTPERIODIC;
    } /* if */

 return AY_OK;
} /* ay_nct_settype */


/* ay_nct_applytrafo:
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
    return AY_EWTYPE;

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
 char fname[] = "nct_getpntfromindex";

  if(!curve || !p)
    return AY_ENULL;

  if(index >= curve->length || index < 0)
    {
      ay_error(AY_ERROR, fname, "index out of range");
      return AY_ERROR;
    }

  *p = &(curve->controlv[index*stride]);

 return AY_OK;
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
 int numcurves = 0, i, j, k, a, order = -1, length = 0, ktype;
 int glu_display_mode = 0;
 double glu_sampling_tolerance = 0.0;
 ay_nurbcurve_object *nc;
 ay_object *o = NULL, *newo = NULL;

  /* check arguments */
  if(!curves || !result)
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
  ay_object_defaults(newo);

  /* count curves; calculate length of concatenated curve */
  o = curves;
  while(o)
    {
      if(o->type == AY_IDNCURVE)
	{
	  nc = (ay_nurbcurve_object *)o->refine;

	  length += nc->length;
	  numcurves++;
	}
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

  /* construct new knot vector */
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
      k = 0;
      o = curves;
      while(o)
	{
	  if(o->type == AY_IDNCURVE)
	    {
	      nc = (ay_nurbcurve_object *)o->refine;

	      for(i = k; i < nc->length+nc->order; i++)
		{
		  newknotv[a] = nc->knotv[i]+j;
		  a++;
		}

	      o = o->next;
	      if(o)
		{
		  if(o->name && !(strcmp(o->name, "Fillet")))
		    {
		      o = o->next;
		      j += 2;
		      k = 0;
		    }
		  else
		    {
		      nc = (ay_nurbcurve_object *)o->refine;
		      k = nc->order;
		      j++;
		    }
		}
	    }
	  else
	    {
	      o = o->next;
	    }
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
      if(o->type == AY_IDNCURVE)
	{
	  nc = (ay_nurbcurve_object *)o->refine;
	  memcpy(ncv, nc->controlv, nc->length * 4 * sizeof(double));
	  ncv += (nc->length * 4);
	}
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
 char *oname = "Fillet";

  if(!c1 || !c2 || !result)
    return AY_ENULL;

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

  if(!(o->name = calloc(strlen(oname)+1, sizeof(char))))
    { free(o); free(controlv); return AY_EOMEM; }

  strcpy(o->name, oname);

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

  if(!curves)
    return AY_ENULL;

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


/* ay_nct_rescaleknvtcmd:
 *  rescale the knot vectors of the selected NURBS curves
 *  - to the range 0.0 - 1.0 (no arguments)
 *  - to a specific range (-r min max)
 *  - so that all knots have a minimum guaranteed distance (-d mindist)
 */
int
ay_nct_rescaleknvtcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;
 ay_nurbcurve_object *curve = NULL;
 int i = 1, mode = 0;
 double rmin = 0.0, rmax = 1.0, mindist = 1.0e-04;

  /* parse args */
  if(argc > 2)
    {
      while(i+1 < argc)
	{
	  if(!strcmp(argv[i], "-r"))
	    {
	      mode = 0;
	      sscanf(argv[i+1], "%lg", &rmin);
	      sscanf(argv[i+2], "%lg", &rmax);
	    }
	  if(!strcmp(argv[i], "-d"))
	    {
	      mode = 1;
	      sscanf(argv[i+1], "%lg", &mindist);
	    }
	  i += 2;
	} /* while */
    } /* if */

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      src = sel->object;
      if(src->type != AY_IDNCURVE)
	{
	  ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
	}
      else
	{
	  curve = (ay_nurbcurve_object*)src->refine;

	  if(curve->knot_type == AY_KTCUSTOM)
	    {
	      if(mode)
		{
		  ay_status = ay_knots_rescaletomindist(curve->length+
							curve->order,
							curve->knotv,
							mindist);
		}
	      else
		{
		  ay_status = ay_knots_rescaletorange(curve->length+
						      curve->order,
						      curve->knotv,
						      rmin, rmax);
		}
	      if(ay_status)
		{
		  ay_error(ay_status, argv[0], "Could not rescale knots!");
		}
	      src->modified = AY_TRUE;
	    }
	  else
	    {
	      ay_error(AY_ERROR, argv[0], "Need a custom knot vector!");
	    } /* if */

	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_rescaleknvtcmd */


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
 double width = 5.0, scale = 1.0, t, dt, *controlv, umin = 0.0, umax = 0.0;
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
	  if(!(controlv = calloc((samples+1)*4, sizeof(double))))
	    {
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }
	  o = NULL;
	  if(!(o = calloc(1, sizeof(ay_object))))
	    {
	      free(controlv);
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }
	  ay_object_defaults(o);
	  o->type = AY_IDNCURVE;

	  umin = c->knotv[c->order-1];
	  umax = c->knotv[c->length];

	  dt = (umax-umin)/((double)samples);
	  a = 0;
	  b = 0;
	  for(t = umin; t < umax; t += dt)
	    {
	      controlv[a] = (double)b*width/samples;
	      controlv[a+1] = ay_nct_getcurvature(c, t)*scale;
	      controlv[a+3] = 1.0;
	      a += 4;
	      b++;
	    }

	  ay_status = ay_nct_create(4, samples, AY_KTNURB, controlv, NULL,
				    &c2);

	  if(!c2 || ay_status)
	    {
	      free(o); free(controlv);
	      if(freepo)
		ay_object_deletemulti(po);
	      sel = sel->next;
	      continue;
	    }

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
 int nbu = 0, nbv = 0;
 int stride = 4, i, j, k, overlap;

  if(!cu || !cv || !intersection)
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
ay_nct_makecompatible(ay_object *curves)
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

  if(!curves)
    return AY_ENULL;

  /* clamp curves */
  o = curves;
  while(o)
    {
      curve = (ay_nurbcurve_object *) o->refine;
      clamp_me = AY_FALSE;
      if((curve->knot_type == AY_KTBSPLINE) ||
	 ((curve->type == AY_CTPERIODIC) &&
	  ((curve->knot_type == AY_KTCHORDAL) ||
	   (curve->knot_type == AY_KTCENTRI))))
	{
	  ay_status = ay_nct_clampperiodic(curve);
	}
      else
	{
	  if(curve->knot_type == AY_KTCUSTOM)
	    {
	      a = 1;
	      u = curve->knotv[0];
	      for(i = 1; i < curve->order; i++)
		if(fabs(u - curve->knotv[i]) < AY_EPSILON)
		  a++;

	      j = curve->length+curve->order-1;
	      b = 1;
	      u = curve->knotv[j];
	      for(i = j-1; i >= curve->length; i--)
		if(fabs(u - curve->knotv[i]) < AY_EPSILON)
		  b++;

	      if((a < (curve->order)) || (b < (curve->order)))
		{
		  clamp_me = AY_TRUE;
		}
	    } /* if */
	} /* if */

      if(clamp_me)
	ay_status = ay_nct_clamp(curve, 0);

      if(ay_status)
	return ay_status;

      o = o->next;
    } /* while */

  /* rescale knots to range 0.0 - 1.0 */
  o = curves;
  while(o)
    {
      curve = (ay_nurbcurve_object *) o->refine;
      if(curve->knotv[0] != 0.0 || curve->knotv[
	  curve->length+curve->order-1] != 1.0)
	{
	  ay_status = ay_knots_rescaletorange(curve->length+curve->order,
					      curve->knotv, 0.0, 1.0);
	}
      o = o->next;
    }

  /* find max order */
  o = curves;
  while(o)
    {
      curve = (ay_nurbcurve_object *) o->refine;
      if(curve->order > max_order)
	max_order = curve->order;

      o = o->next;
    }

  /* degree elevate */
  o = curves;
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
	      free(Uh); free(Qw); return AY_EOMEM;
	    }

	  if(!(realUh = realloc(Uh, (nh+curve->order+t)*sizeof(double))))
	    {
	      free(Uh); free(realQw); return AY_EOMEM;
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
  o = curves;
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
	  goto cleanup;
	}

      Ua = Ubar;
      Ualen = Ubarlen;

      o = o->next;
    } /* while */

  /* merge knots */
  o = curves;
  while(o)
    {
      curve = (ay_nurbcurve_object *) o->refine;

      ay_status = ay_knots_mergenc(curve, Ubar, Ubarlen);
      if(ay_status)
	{
	  goto cleanup;
	}

      if(curve->mpoints)
	ay_status = ay_nct_recreatemp(curve);

      o = o->next;
    } /* while */

cleanup:

  if(Ubar)
    free(Ubar);

 return ay_status;
} /* ay_nct_makecompatible */


/* ay_nct_shiftcbs:
 *  shift the control points of a closed B-Spline one time so that the
 *  second control point will be the first (and so on) afterwards
 */
int
ay_nct_shiftcbs(ay_nurbcurve_object *curve)
{
 int ay_status = AY_OK;
 int a, i, stride = 4;
 double t[4] = {0};

  if(!curve)
    return AY_ENULL;

  memcpy(t, curve->controlv, stride * sizeof(double));

  a = 0;
  for(i = 0; i < (curve->length-(curve->order-1)-1); i++)
    {
      memcpy(&(curve->controlv[a]), &(curve->controlv[a+stride]),
	     stride * sizeof(double));
      a += stride;
    }

  memcpy(&(curve->controlv[(curve->length-curve->order)*stride]), t,
	 stride * sizeof(double));

  ay_status = ay_nct_close(curve);

 return ay_status;
} /* ay_nct_shiftcbs */


/* ay_nct_shiftcbstcmd:
 *  shift the control points of a closed B-Spline
 */
int
ay_nct_shiftcbstcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_nurbcurve_object *curve = NULL;
 ay_object *src = NULL;
 int i, times = 1;

  if(argc > 1)
    {
      Tcl_GetInt(interp, argv[1], &times);
    }

  if(times == 0)
    {
      ay_error(AY_ERROR, argv[0], "Parameter must be different from 0.");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      src = sel->object;
      if(src->type != AY_IDNCURVE)
	{
	  ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
	}
      else
	{
	  curve = (ay_nurbcurve_object*)src->refine;

	  if(times < 1)
	    {
	      times = (curve->length-curve->order+1)-abs(times);
	      if(times <= 1)
		{
		  ay_error(AY_ERROR, argv[0],
			   "Parameter out of range. Could not shift curve!");
		  continue;
		}
	    }

	  for(i = 0; i < times; i++)
	    {
	      ay_status = ay_nct_shiftcbs(curve);
	      if(ay_status)
		{
		  ay_error(ay_status, argv[0], "Could not shift curve!");
		}
	    } /* for */
	  src->modified = AY_TRUE;
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_shiftcbstcmd */


/* ay_nct_toxy :
 *  modify the planar NURBS curve <c>, so that it is defined in the XY plane
 *  by detecting the current orientation, adding the relevant rotation
 *  information to the transformation attributes and rotating the control
 *  points of the planar curve to the XY plane
 */
int
ay_nct_toxy(ay_object *c)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *nc = NULL;
 double *p, *tp1, *tp2, *tp3, V1[3], V2[3], A[3], B[3];
 double Z[3] = {0,0,1};
 double angle, len, m[16], quat[4], euler[3];
 int i, stride = 4, have_good_points = AY_FALSE, max_tries = 0;

  if(!c)
    return AY_ENULL;

  if(!c->type == AY_IDNCURVE)
    return AY_EWTYPE;

  nc = (ay_nurbcurve_object *)c->refine;

  if(nc->length < 3)
    return AY_ERROR;

  ay_trafo_identitymatrix(m);
  ay_trafo_scalematrix(c->scalx, c->scaly, c->scalz, m);

  /* apply scale/translate matrix to all points */
  p = nc->controlv;
  for(i = 0; i < nc->length; i++)
    {
      ay_trafo_apply4(p, m);
      p += stride;
    } /* for */

  c->scalx = 1.0;
  c->scaly = 1.0;
  c->scalz = 1.0;

  /* try to get three "good" points,
     they should not be equal and not be colinear
     (i.e. span a triangle to get the orientation from */
  tp1 = nc->controlv;
  tp2 = &(nc->controlv[(nc->length/2)*stride]);

  /* check, whether we, probably, operate on a closed B-Spline curve,
     if yes, we choose a different set of initial points */
  p = &(nc->controlv[(nc->length-(nc->order-1))*stride]);
  if(AY_V3COMP(tp1, p))
    {
      /*printf("Detected closed BSpline!\n");*/
      tp1 = &(nc->controlv[(nc->order/2)*stride]);
      tp3 = &(nc->controlv[(nc->length-(nc->order/2))*stride]);
    }
  else
    {
      /*printf("Detected normal curve!\n");*/
      tp3 = &(nc->controlv[(nc->length-2)*stride]);
    }

  /*
    printf("first indices %d %d %d\n",
    (tp1-nc->controlv)/stride,
    (tp2-nc->controlv)/stride,
    (tp3-nc->controlv)/stride);
  */

  /* check and correct the points */
  while(!have_good_points  && (max_tries < (nc->length/2)))
    {
      have_good_points = AY_TRUE;
      if(AY_V3COMP(tp1, tp2))
	{
	  tp2 += stride;
	  have_good_points = AY_FALSE;
	}
      if(AY_V3COMP(tp2, tp3))
	{
	  tp3 -= stride;
	  have_good_points = AY_FALSE;
	}
      if(AY_V3COMP(tp1, tp3))
	{
	  tp3 -= stride;
	  have_good_points = AY_FALSE;
	}

      if(have_good_points)
	{
	  AY_V3SUB(V1, tp2, tp1);
	  len = AY_V3LEN(V1);
	  AY_V3SCAL(V1,(1.0/len));

	  AY_V3SUB(V2, tp3, tp1);
	  len = AY_V3LEN(V2);
	  AY_V3SCAL(V2, (1.0/len));

	  angle = AY_R2D(acos(AY_V3DOT(V1,V2)));
	  if(angle < AY_EPSILON)
	    {
	      /* V1 and V2 are parallel */
	      have_good_points = AY_FALSE;
	      tp2 += stride;
	    }
	} /* if */

      max_tries++;
    } /* while */

  if(!have_good_points)
    return AY_ERROR;
  /*
    printf("indices after correction %d %d %d\n",
    (tp1-nc->controlv)/stride,
    (tp2-nc->controlv)/stride,
    (tp3-nc->controlv)/stride);
  */

  /* now we may calculate the orientation of the curve */
  AY_V3CROSS(A, V1, V2);
  len = AY_V3LEN(A);
  AY_V3SCAL(A, (1.0/len));

  /* A is now perpendicular to the plane in which the curve is defined
     thus, we calculate angle and rotation axis (B) between A and Z (0,0,1) */
  angle = AY_R2D(acos(AY_V3DOT(A, Z)));
  if((fabs(angle) < AY_EPSILON) || (fabs(angle - 180.0) < AY_EPSILON))
    {
      /* Nothing to do, as curve ist properly aligned with
	 XY plane already...*/
      return AY_OK;
    }
  AY_V3CROSS(B, A, Z);
  len = AY_V3LEN(B);
  AY_V3SCAL(B, (1.0/len));

  /*printf("angle %g, B: %g %g %g\n",angle,B[0],B[1],B[2]);*/
  if(angle > AY_EPSILON)
    {
      /* calculate rotation matrix */
      ay_trafo_identitymatrix(m);
      /*ay_trafo_translatematrix(-c->movx, -c->movy, -c->movz, m);*/
      ay_trafo_rotatematrix(angle, B[0], B[1], B[2], m);
      /*ay_trafo_translatematrix(c->movx, c->movy, c->movz, m);*/

      /* apply rotation matrix to all points */
      p = nc->controlv;
      for(i = 0; i < nc->length; i++)
	{
	  ay_trafo_apply4(p, m);
	  p += stride;
	} /* for */

      /* calculate new transformation attributes */
      ay_quat_axistoquat(B, AY_D2R(angle), quat);
      ay_quat_add(c->quat, quat, c->quat);
      ay_quat_toeuler(c->quat, euler);
      c->rotx = AY_R2D(euler[0]);
      c->roty = AY_R2D(euler[1]);
      c->rotz = AY_R2D(euler[2]);
    } /* if */

 return ay_status;
} /* ay_nct_toxy */


/* ay_nct_toxytcmd:
 *  Tcl interface for NURBS curve toxy tool
 */
int
ay_nct_toxytcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      src = sel->object;
      if(src->type != AY_IDNCURVE)
	{
	  ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
	}
      else
	{
	  ay_status = ay_nct_toxy(src);
	  if(ay_status)
	    {
	      ay_error(ay_status, argv[0],
		       "Could not align object to XY plane!");
	    }
	  else
	    {
	      src->modified = AY_TRUE;
	      /* re-create tesselation of curve */
	      ay_notify_force(src);
	    } /* if */
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_toxytcmd */


/* ay_nct_makecomptcmd:
 *
 */
int
ay_nct_makecomptcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_nurbcurve_object *nc = NULL;
 ay_object *o = NULL, *p = NULL, *src = NULL, **nxt = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  nxt = &(src);

  while(sel)
    {
      o = sel->object;
      if(o->type != AY_IDNCURVE)
	{
	  ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
	}
      else
	{
	  ay_status = ay_object_copy(o, nxt);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "Could not copy object!");
	      goto cleanup;
	    }
	  nxt = &((*nxt)->next);
	} /* if */

      sel = sel->next;
    } /* while */

  if(src && src->next)
    {
      ay_status = ay_nct_makecompatible(src);
      if(ay_status)
	{
	  ay_error(AY_ERROR, argv[0],
		   "Failed to make selected curves compatible!");
	  goto cleanup;
	}
      /* now exchange the nurbcurve objects */
      p = src;
      sel = ay_selection;
      while(sel)
	{
	  o = sel->object;
	  if(o->type == AY_IDNCURVE)
	    {
	      nc = (ay_nurbcurve_object*)o->refine;
	      o->refine = p->refine;
	      p->refine = nc;
	      /* update pointers to controlv */
	      ay_selp_clear(o);
	      o->modified = AY_TRUE;

	      /* re-create tesselation of curve */
	      ay_notify_force(o);

	      p = p->next;
	    } /* if */
	  sel = sel->next;
	} /* while */
    }
  else
    {
      ay_error(AY_ERROR, argv[0], "Please select atleast two NURBS curves!");
    } /* if */

  ay_notify_parent();

 cleanup:
  if(src)
    ay_status = ay_object_deletemulti(src);

 return TCL_OK;
} /* ay_nct_makecomptcmd */


/* ay_nct_findufrompoint:
 *
 */
void
ay_nct_findufrompoint(ay_nurbcurve_object *curve, double *point,
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

  if(!curve || !point || !u)
    return;

  *u = guess;
  U  = curve->knotv;

  if(*u < U[0])
    *u = U[0];
  if(*u > U[n-1])
    *u = U[n-1];

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
      if(AY_V3LEN(c1) < (e1*e1))
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

      if(AY_V3LEN(c2) < e2)
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
} /* ay_nct_findufrompoint */


/* ay_nct_israt:
 *
 */
int
ay_nct_israt(ay_nurbcurve_object *curve)
{
 double *p;
 int i;

  if(!curve)
    return AY_FALSE;

  p = &(curve->controlv[3]);
  for(i = 0; i < curve->length; i++)
    {
      if((fabs(*p) < (1.0-AY_EPSILON)) || (fabs(*p) > (1.0+AY_EPSILON)))
	return AY_TRUE;
      p += 4;
    } /* for */

 return AY_FALSE;
} /* ay_nct_israt */


/* ay_nct_center:
 *  center a NURBS curve
 *  mode - control in which dimensions centering shall occur
 *   0: all dimensions
 *   1: only x-y
 *   2: only y-z
 *   3: only x-z
 */
int
ay_nct_center(int mode, ay_nurbcurve_object *curve)
{
 int ay_status = AY_OK;
 double *controlv = NULL, *p1, *p2, x, y, z;
 int a, i, k;

  if(!curve)
    return AY_ENULL;

  controlv = curve->controlv;

  /* calculate middle point, curve->type tells how many control
     points should be considered */
  i = curve->length;
  if(curve->type == AY_CTCLOSED)
    i--;
  if(curve->type == AY_CTPERIODIC)
    i -= (curve->order-1);
  x = controlv[0]/i;
  y = controlv[1]/i;
  z = controlv[2]/i;
  a = 4;
  p1 = controlv;
  for(k = 1; k < i; k++)
    {
      p2 = &(controlv[a]);
      /* compute weighted sum of all coordinate values (points)
	 to be considered, omitting consecutive equal points */
      /* XXXX What about other multiple points? Should we not rather
	 build a list of unique points and iterate over that? */
      if(!AY_V3COMP(p1,p2))
	{
	  x += (controlv[a]/i);
	  y += (controlv[a+1]/i);
	  z += (controlv[a+2]/i);
	}
      a += 4;
      p1 = p2;
    }
  switch(mode)
    {
    case 0:
      a = 0;
      for(k = 0; k < curve->length; k++)
	{
	  controlv[a]   -= x;
	  controlv[a+1] -= y;
	  controlv[a+2] -= z;
	  a += 4;
	}
      break;
    case 1:
      a = 0;
      for(k = 0; k < curve->length; k++)
	{
	  controlv[a]   -= x;
	  controlv[a+1] -= y;
	  a += 4;
	}
      break;
    case 2:
      a = 0;
      for(k = 0; k < curve->length; k++)
	{
	  controlv[a+1] -= y;
	  controlv[a+2] -= z;
	  a += 4;
	}
      break;
    case 3:
      a = 0;
      for(k = 0; k < curve->length; k++)
	{
	  controlv[a]   -= x;
	  controlv[a+2] -= z;
	  a += 4;
	}
      break;
    default:
      break;
    } /* switch */

 return ay_status;
} /* ay_nct_center */


/* ay_nct_centertcmd:
 *  Tcl interface for NURBS curve center tool
 */
int
ay_nct_centertcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *c = NULL;
 int mode = 0;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(argc > 1)
    {
      Tcl_GetInt(interp, argv[1], &mode);
    }

  while(sel)
    {
      c = sel->object;
      if(c->type != AY_IDNCURVE)
	{
	  ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
	}
      else
	{
	  ay_status = ay_nct_center(mode, (ay_nurbcurve_object*)c->refine);
	  if(ay_status)
	    {
	      ay_error(ay_status, argv[0], "Could not center object!");
	    }

	  c->modified = AY_TRUE;

	  /* re-create tesselation of curve */
	  ay_notify_force(c);
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_centertcmd */


/* ay_nct_coarsen:
 *
 */
int
ay_nct_coarsen(ay_nurbcurve_object *curve)
{
 int ay_status = AY_OK;
 char fname[] = "nct_coarsen";
 double *newcontrolv = NULL, *newknotv = NULL;
 int i, a, b, stride = 4, newlength, p, t;

  if(!curve)
    return AY_ENULL;

  if(curve->type == AY_CTPERIODIC)
    {
      /* special case: curves marked periodic;
       * we keep the p multiple points at the ends
       * and remove points just from the other sections
       */

      /* no control points to remove? */
      p = curve->order-1;
      if((curve->length - p*2) < 2)
	return AY_OK;

      /* calc number of points to remove */
      t = (curve->length-(p*2))/2+(curve->length-(p*2)) % 2;
      newlength = curve->length-t;

      /* coarsen (custom-) knots */
      if(curve->knot_type == AY_KTCUSTOM)
	{
	  ay_status = ay_knots_coarsen(curve->order,
				       curve->length+curve->order,
				       curve->knotv, t, &newknotv);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Could not coarsen knots!");
	      return ay_status;
	    }
	}

      /* coarsen control points */

      if(!(newcontrolv = calloc(newlength*stride, sizeof(double))))
	return AY_EOMEM;

      /* copy first p points */
      memcpy(&(newcontrolv[0]), &(curve->controlv[0]),
	     p*stride*sizeof(double));

      /* copy middle points omitting every second */
      a = p*stride;
      /* omit first point after the p'th */
      b = a+stride;
      for(i = 0; i < t; i++)
	{
	  memcpy(&(newcontrolv[a]), &(curve->controlv[b]),
		 stride*sizeof(double));
	  a += stride;
	  b += 2*stride;
	}

      /* copy last p points */
      a = (newlength-p)*stride;
      b = (curve->length-p)*stride;
      memcpy(&(newcontrolv[a]), &(curve->controlv[b]),
	     p*stride*sizeof(double));

      curve->length = newlength;
    }
  else
    {
      /* calc number of points to remove */
      t = (curve->length-2)/2;
      newlength = curve->length-t;

      /* no control points to remove? */
      if(newlength < curve->order)
	return AY_OK;

      /* coarsen (custom-) knots */
      if(curve->knot_type == AY_KTCUSTOM)
	{
	  ay_status = ay_knots_coarsen(curve->order,
				       curve->length+curve->order,
				       curve->knotv, t, &newknotv);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Could not coarsen knots!");
	      return ay_status;
	    }
	}

      /* coarsen control points */

      if(!(newcontrolv = calloc(newlength*stride, sizeof(double))))
	return AY_EOMEM;

      /* copy first point */
      memcpy(newcontrolv, curve->controlv, stride*sizeof(double));

      /* copy middle points omitting every second */
      a = stride;
      b = a+stride;
      for(i = 0; i < t; i++)
	{
	  memcpy(&(newcontrolv[a]), &(curve->controlv[b]),
		 stride*sizeof(double));
	  a += stride;
	  b += 2*stride;
	}

      /* copy last point */
      a = (newlength-1)*stride;
      b = (curve->length-1)*stride;
      memcpy(&(newcontrolv[a]), &(curve->controlv[b]),
	     stride*sizeof(double));

      curve->length = newlength;
    } /* if */

  free(curve->controlv);
  curve->controlv = newcontrolv;

  if(!newknotv)
    {
      free(curve->knotv);
      curve->knotv = NULL;

      ay_status = ay_knots_createnc(curve);
      if(ay_status)
	ay_error(AY_ERROR, fname, "Could not create knots!");
    }
  else
    {
      free(curve->knotv);
      curve->knotv = newknotv;
    }

  if(curve->type == AY_CTCLOSED)
    {
      ay_status = ay_nct_close(curve);
      if(ay_status)
	ay_error(AY_ERROR, fname, "Could not close curve!");
    }

  ay_nct_recreatemp(curve);

 return ay_status;
} /* ay_nct_coarsen */


/* ay_nct_coarsentcmd:
 *  Tcl interface for NURBS curve coarsen tool
 */
int
ay_nct_coarsentcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;
      if(o->type != AY_IDNCURVE)
	{
	  ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
	}
      else
	{
	  /* remove all selected points */
	  if(sel->object->selp)
	    {
	      ay_selp_clear(sel->object);
	    }

	  ay_status = ay_nct_coarsen((ay_nurbcurve_object*)o->refine);
	  if(ay_status)
	    {
	      ay_error(ay_status, argv[0], "Could not coarsen object!");
	      break;
	    }

	  o->modified = AY_TRUE;

	  /* re-create tesselation of curve */
	  ay_notify_force(sel->object);
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_coarsentcmd */


/* ay_nct_removekntcmd:
 *  Tcl interface for NURBS curve knot removal tool
 */
int
ay_nct_removekntcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
 int i = 0, s = 0, r = 0;
 double tol = DBL_MAX/*AY_EPSILON*/;
 double u = 0.0, *newknotv = NULL, *newcontrolv = NULL;
 ay_nurbcurve_object *curve;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;

  if(argc < 3)
    {
      ay_error(AY_EARGS, argv[0], "u r [tol]");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  Tcl_GetDouble(interp, argv[1], &u);
  Tcl_GetInt(interp, argv[2], &r);

  if(argc > 3)
    {
      Tcl_GetDouble(interp, argv[3], &tol);
    }

  printf("%lg\n",tol);

  while(sel)
    {
      o = sel->object;
      if(o->type != AY_IDNCURVE)
	{
	  ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
	}
      else
	{
	  /* remove all selected points */
	  if(sel->object->selp)
	    {
	      ay_selp_clear(sel->object);
	    }

	  curve = (ay_nurbcurve_object *)o->refine;

	  /* find knot to remove */
	  while((i<(curve->length+curve->order)) &&
		(fabs(curve->knotv[i]-u) > AY_EPSILON))
	    {
	      i++;
	    }

	  if(fabs(curve->knotv[i]-u) >= AY_EPSILON)
	    {
	      ay_error(AY_ERROR, argv[0], "could not find knot to remove");
	      break;
	    }

	  /* calculate knot multiplicity */
	  s = 1;
	  while(curve->knotv[i] == curve->knotv[i+s])
	    {
	      s++;
	    }

	  newcontrolv = calloc(curve->length*4, sizeof(double));
	  newknotv = calloc(curve->length+curve->order, sizeof(double));

	  /* remove the knot */
	  ay_status = ay_nb_CurveRemoveKnot4D(curve->length-1, curve->order-1,
					      curve->knotv, curve->controlv,
					      tol, i, s, r,
					      newknotv, newcontrolv);

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "knot removal failed");
	      free(newcontrolv);
	      newcontrolv = NULL;
	      free(newknotv);
	      newknotv = NULL;
	      break;
	    }

	  /* save results */
	  curve->length -= r;
	  memcpy(curve->controlv, newcontrolv, curve->length*4*sizeof(double));
	  memcpy(curve->knotv, newknotv,
		 curve->length+curve->order*sizeof(double));

	  free(newcontrolv);
	  newcontrolv = NULL;
	  free(newknotv);
	  newknotv = NULL;

	  curve->knot_type = AY_KTCUSTOM;

	  ay_status = ay_nct_recreatemp(curve);

	  o->modified = AY_TRUE;

	  /* re-create tesselation of curve */
	  ay_notify_force(sel->object);
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_removekntcmd */


/* ay_nct_trim:
 *  trim NURBS curve (cut off pieces at start and/or end)
 */
int
ay_nct_trim(ay_nurbcurve_object **curve, double umin, double umax)
{
 int ay_status = AY_OK;
 ay_object t1 = {0}, *t2 = NULL;
 void *t;
 double knotmin, knotmax;

  if(!curve || !*curve)
    return AY_ENULL;

  if(umax <= umin)
    return AY_ERROR;

  knotmin = (*curve)->knotv[(*curve)->order-1];
  knotmax = (*curve)->knotv[(*curve)->length];

  /* prepare t1 for splitting (XXXX do we need ay_object_defaults()?) */
  t1.type = AY_IDNCURVE;
  t1.refine = *curve;

  if(umin > knotmin)
    {
      ay_status = ay_nct_split(&t1, umin, &t2);

      if(ay_status)
	return ay_status;

      /* remove superfluous curve and save result in t1 */
      t = t2->refine;
      t2->refine = t1.refine;
      t1.refine = t;
      ay_object_delete(t2);
      t2 = NULL;
    } /* if */

  if(umax < knotmax)
    {
      ay_status = ay_nct_split(&t1, umax, &t2);

      if(ay_status)
	return ay_status;

      /* remove superfluous curve */
      ay_object_delete(t2);
      t2 = NULL;
    } /* if */

  /* return result */
  *curve = t1.refine;

 return ay_status;
} /* ay_nct_trim */


/* ay_nct_trimtcmd:
 *  Tcl interface for NURBS curve trimming tool
 */
int
ay_nct_trimtcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 int ay_status = AY_OK;
 double umin = 0.0, umax = 0.0;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;

  if(argc < 3)
    {
      ay_error(AY_EARGS, argv[0], "umin umax");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  Tcl_GetDouble(interp, argv[1], &umin);
  Tcl_GetDouble(interp, argv[2], &umax);

  if(umax <= umin)
    {
      ay_error(AY_ERROR, argv[0], "umin must be smaller than umax");
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;
      if(o->type != AY_IDNCURVE)
	{
	  ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
	}
      else
	{
	  /* remove all selected points */
	  if(sel->object->selp)
	    {
	      ay_selp_clear(sel->object);
	    }

	  ay_status = ay_nct_trim((ay_nurbcurve_object**)(&(o->refine)),
				  umin, umax);

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "trim failed");
	      break;
	    }

	  ay_status = ay_nct_recreatemp(o->refine);

	  o->modified = AY_TRUE;

	  /* re-create tesselation of curve */
	  ay_notify_force(sel->object);
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_trimtcmd */


/* ay_nct_isdegen:
 *  check curve for degeneracy
 */
int
ay_nct_isdegen(ay_nurbcurve_object *curve)
{
 int i, stride = 4;
 double *p1, *p2;

  if(!curve)
    return AY_FALSE;

  for(i = 0; i < curve->length-1; i++)
    {
      p1 = &(curve->controlv[i*stride]);
      p2 = &(curve->controlv[(i+1)*stride]);
      if(!AY_V3COMP(p1, p2))
	return AY_FALSE;
    } /* for */

 return AY_TRUE;
} /* ay_nct_isdegen */


/* ay_nct_offset:
 *  create offset curve from <o>
 *  the new curve is <offset> away from the original
 *  returns new curve in <nc>
 *  WIP
 */
int
ay_nct_offset(ay_object *o, int mode, double offset, ay_nurbcurve_object **nc)
{
 int ay_status = AY_OK;
 int i, j, k, stride = 4;
 double tangent[3] = {0}, normal[3] = {0}, *newcv = NULL, *newkv = NULL;
 double zaxis[3] = {0.0,0.0,1.0};
 ay_nurbcurve_object *curve = NULL;
 ay_tag *tag = NULL;
 int p1len, p2len, p3len;
 double *p1, *p2, *p3, *pt, *po, p1s1[2], p2s1[2], p1s2[2], p2s2[2];
 double t1[2], t2[2], n[2];
 char *nname = ay_prefs.normalname;
 unsigned int vnlen = 0;
 double *vn = NULL;

  /* sanity check */
  if(!o || !nc)
    return AY_ENULL;

  curve = (ay_nurbcurve_object*)o->refine;

  if(!(newcv = calloc(curve->length*stride, sizeof(double))))
    return AY_EOMEM;

  /* special case for simple lines */
  if(curve->length == 2)
    {
      p1 = &(curve->controlv[0]);
      p2 = &(curve->controlv[stride]);

      /* calc tangent */
      t1[0] = p2[0] - p1[0];
      t1[1] = p2[1] - p1[1];

      /* calc normal */
      n[0] =  t1[1];
      n[1] = -t1[0];

      /* scale normal to be offset length */
      AY_V2NORM(n);
      AY_V2SCAL(n, offset);

      /* offset the line */
      newcv[0] = p1[0] + n[0];
      newcv[1] = p1[1] + n[1];

      newcv[stride]   = p2[0] + n[0];
      newcv[stride+1] = p2[1] + n[1];
    }
  else
    {
      if(mode == 0)
	{
	  /*
	    "Bevel" mode:
	     offset corner points according to normals derived
	     from surrounding control points
	  */
	  for(j = 0; j < curve->length; j++)
	    {

	      ay_npt_gettangentfromcontrol2D(curve->type, curve->length,
				       curve->order-1, 4, curve->controlv, j,
				       tangent);

	      AY_V3CROSS(normal, tangent, zaxis);
	      AY_V3SCAL(normal, offset);

	      newcv[j*stride]   = curve->controlv[j*stride]   + normal[0];
	      newcv[j*stride+1] = curve->controlv[j*stride+1] + normal[1];
	      newcv[j*stride+2] = curve->controlv[j*stride+2] + normal[2];
	      newcv[j*stride+3] = curve->controlv[j*stride+3];

	    } /* for */
	} /* if */

      if(mode == 1)
	{
	  /*
	    "Section" mode:
	    offset control polygon sections
	  */
	  p1 = &(curve->controlv[0]);
	  po = &(curve->controlv[curve->length*stride]);
	  /* get length of p1 (count multiple points) */
	  pt = p1+stride;
	  p1len = 1;
	  while((pt != po) && (AY_V2COMP(p1, pt)))
	    {
	      p1len++;
	      pt += stride;
	    }

	  if(pt == po)
	    {
	      /* this curve, apparently, has no sections
		 (is degenerated to one point) */
	      free(newcv);
	      return AY_ERROR;
	    }

	  p2 = pt;
	  pt = p2+stride;
	  p2len = 1;
	  while((pt != po) && (AY_V2COMP(p2, pt)))
	    {
	      p2len++;
	      pt += stride;
	    }

	  /* calc tangent of first original control polygon segment */
	  t1[0] = p2[0] - p1[0];
	  t1[1] = p2[1] - p1[1];
	  /*
	    AY_V2NORM(t1);
	  */
	  /* calc normal of first original control polygon segment */
	  n[0] =  t1[1];
	  n[1] = -t1[0];

	  /* scale normal to be offset length */
	  AY_V2NORM(n);
	  AY_V2SCAL(n, offset);

	  /* offset the first control polygon segment */
	  p1s1[0] = p1[0] + n[0];
	  p1s1[1] = p1[1] + n[1];

	  p2s1[0] = p2[0] + n[0];
	  p2s1[1] = p2[1] + n[1];

	  /* first point of offset curves control polygon */
	  for(i = 0; i < p1len; i++)
	    {
	      newcv[i*stride]   = p1s1[0];
	      newcv[i*stride+1] = p1s1[1];
	    }

	  /* special case: curve has one section and multiple
	     points at the beginning _and_ at the end */
	  if((p1len+p2len) == curve->length)
	    {
	      for(i = p1len; i < p1len+p2len; i++)
		{
		  newcv[i*stride]   = p2s1[0];
		  newcv[i*stride+1] = p2s1[1];
		}
	    }

	  if((p1len+p2len) < curve->length)
	    {
	      j = p1len;
	      while((j+p2len) < curve->length)
		{
		  p3 = &(curve->controlv[(j+p2len)*stride]);
		  p3len = 1;
		  if((j+p2len) < (curve->length-1))
		    {
		      pt = p3+stride;
		      while((pt != po) && (AY_V2COMP(p3, pt)))
			{
			  p3len++;
			  pt += stride;
			}
		    }

		  /* calc tangent of next original control polygon segment */
		  t2[0] = p3[0]-p2[0];
		  t2[1] = p3[1]-p2[1];

		  /* calc normal of next original control polygon segment */
		  n[0] =  t2[1];
		  n[1] = -t2[0];

		  /* scale normal to be offset length */
		  AY_V2NORM(n);
		  AY_V2SCAL(n, offset);

		  /* offset the control polygon segment */
		  p1s2[0] = p2[0] + n[0];
		  p1s2[1] = p2[1] + n[1];

		  p2s2[0] = p3[0] + n[0];
		  p2s2[1] = p3[1] + n[1];

		  /* intersect two offset segments, intersection is new cv */
		  if(!ay_geom_intersectlines2D(p1s1, t1, p1s2, t2,
					       &(newcv[j*stride])))
		    {
		      /*
		       * if the intersection failed (e.g. due to colinear
		       * segments) we simply pick one of the inner segment
		       * points
		       */
		      for(k = 0; k < p2len; k++)
			{
			  memcpy(&(newcv[(j+k)*stride]), p2s1,
				 2*sizeof(double));
			}
		    }
		  else
		    {
		      if(p2len > 1)
			{
			  for(k = 1; k < p2len; k++)
			    {
			      memcpy(&(newcv[(j+k)*stride]),
				     &(newcv[j*stride]),
				     2*sizeof(double));
			    }
			}
		    }

		  /*
		   * XXXX the next block is a "replacement" for the
		   * intersection test, that is faster, but delivers
		   * sub-optimal offset quality (collisions occur)
		   */
		  /*
		  AY_V2NORM(t2);
		  for(i = 0; i < p2len; i++)
		    {
		      newcv[(j+i)*stride]   = p2s1[0]+((p1s2[0]-p2s1[0])/2.0);
		      newcv[(j+i)*stride+1] = p2s1[1]+((p1s2[1]-p2s1[1])/2.0);
		    }
		  */

		  /* prepare next iteration */
		  p1 = p2;
		  p2 = p3;

		  memcpy(t1, t2, 2*sizeof(double));
		  memcpy(p1s1, p1s2, 2*sizeof(double));
		  memcpy(p2s1, p2s2, 2*sizeof(double));

		  j += p2len;
		  p2len = p3len;
		} /* while */

	      /* last point of offset curves control polygon */
	      for(i = 0; i < p3len; i++)
		{
		  newcv[(j+i)*stride]   = p2s2[0];
		  newcv[(j+i)*stride+1] = p2s2[1];
		}

	    } /* if */

	  /* set weights */
	  if(curve->is_rat)
	    {
	      for(j = 0; j < curve->length; j++)
		{
		  newcv[j*stride+3] = curve->controlv[j*stride+3];
		}
	    }
	  else
	    {
	      for(j = 0; j < curve->length; j++)
		{
		  newcv[j*stride+3] = 1.0;
		}
	    } /* if */

	  /* another special case: curve is periodic */
	  if((curve->type == AY_CTPERIODIC) && (curve->order > 2))
	    {
	      j = (curve->length-(curve->order-1))*stride;
	      memcpy(&(newcv[0]), &(newcv[j]), stride*sizeof(double));
	    }

	} /* if */

      if(mode == 2)
	{
	  /*
	    "Bevel3D" mode:
	     offset corner points according to normals derived
	     from surrounding control points
	  */
	  for(j = 0; j < curve->length; j++)
	    {

	      ay_npt_getnormalfromcontrol3D(curve->type, curve->length,
				       curve->order-1, 4, curve->controlv, j,
				       normal);
	      AY_V3SCAL(normal, offset);
	      newcv[j*stride]   = curve->controlv[j*stride]   + normal[0];
	      newcv[j*stride+1] = curve->controlv[j*stride+1] + normal[1];
	      newcv[j*stride+2] = curve->controlv[j*stride+2] + normal[2];
	      newcv[j*stride+3] = curve->controlv[j*stride+3];

	    } /* for */
	} /* if */

      if(mode == 3)
	{
	  /*
	    "3DPVN" mode:
	     offset points according to normal delivered as primitive variable
	  */
	  tag = o->tags;
	  while(tag)
	    {
	      if(ay_pv_checkndt(tag, nname, "varying", "n"))
		{
		  ay_pv_convert(tag,0,&vnlen,(void**)&vn);
		  break;
		}
	      tag = tag->next;
	    }
	  if(!vn || (vnlen != curve->length))
	    {
	      ay_status = AY_ERROR;
	      goto cleanup;
	    }
	  for(j = 0; j < curve->length; j++)
	    {
	      p1 = &(vn[j*3]);
	      AY_V3SCAL(p1, offset);
	      newcv[j*stride]   = curve->controlv[j*stride]   + p1[0];
	      newcv[j*stride+1] = curve->controlv[j*stride+1] + p1[1];
	      newcv[j*stride+2] = curve->controlv[j*stride+2] + p1[2];
	      newcv[j*stride+3] = curve->controlv[j*stride+3];
	    } /* for */
	} /* if */
    } /* if */

  if(curve->knot_type == AY_KTCUSTOM)
    {
      if(!(newkv = calloc(curve->length+curve->order, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      memcpy(newkv, curve->knotv, (curve->length+curve->order)*sizeof(double));
    }

  ay_status = ay_nct_create(curve->order, curve->length, curve->knot_type,
			    newcv, newkv, nc);

cleanup:

  if(ay_status || !nc)
    {
      if(newcv)
	free(newcv);
      if(newkv)
	free(newkv);
    }

  if(vn)
    free(vn);

 return ay_status;
} /* ay_nct_offset */


/* ay_nct_cmppnt:
 *  compare two points (helper for qsort)
 */
int
ay_nct_cmppnt(const void *p1, const void *p2)
{

  if((((double*)p1)[0] < ((double*)p2)[0]) &&
     (fabs(((double*)p2)[0] - ((double*)p1)[0]) > AY_EPSILON))
    return -1;
  if((((double*)p1)[0] > ((double*)p2)[0]) &&
     (fabs(((double*)p1)[0] - ((double*)p2)[0]) > AY_EPSILON))
    return 1;

  if((((double*)p1)[1] < ((double*)p2)[1]) &&
     (fabs(((double*)p2)[1] - ((double*)p1)[1]) > AY_EPSILON))
    return -1;
  if((((double*)p1)[1] > ((double*)p2)[1]) &&
     (fabs(((double*)p1)[1] - ((double*)p2)[1]) > AY_EPSILON))
    return 1;

  if((((double*)p1)[2] < ((double*)p2)[2]) &&
     (fabs(((double*)p2)[2] - ((double*)p1)[2]) > AY_EPSILON))
    return -1;
  if((((double*)p1)[2] > ((double*)p2)[2]) &&
     (fabs(((double*)p1)[2] - ((double*)p2)[2]) > AY_EPSILON))
    return 1;

  return 0;
} /* ay_nct_cmppnt */


/* ay_nct_cmppntp:
 *  compare two points given as pointers (helper for qsort)
 */
int
ay_nct_cmppntp(const void *p1, const void *p2)
{

  if(((*(double**)p1)[0] < (*(double**)p2)[0]) &&
     (fabs((*(double**)p2)[0] - (*(double**)p1)[0]) > AY_EPSILON))
    return -1;
  if(((*(double**)p1)[0] > (*(double**)p2)[0]) &&
     (fabs((*(double**)p1)[0] - (*(double**)p2)[0]) > AY_EPSILON))
    return 1;

  if(((*(double**)p1)[1] < (*(double**)p2)[1]) &&
     (fabs((*(double**)p2)[1] - (*(double**)p1)[1]) > AY_EPSILON))
    return -1;
  if(((*(double**)p1)[1] > (*(double**)p2)[1]) &&
     (fabs((*(double**)p1)[1] - (*(double**)p2)[1]) > AY_EPSILON))
    return 1;

  if(((*(double**)p1)[2] < (*(double**)p2)[2]) &&
     (fabs((*(double**)p2)[2] - (*(double**)p1)[2]) > AY_EPSILON))
    return -1;
  if(((*(double**)p1)[2] > (*(double**)p2)[2]) &&
     (fabs((*(double**)p1)[2] - (*(double**)p2)[2]) > AY_EPSILON))
    return 1;

  return 0;
} /* ay_nct_cmppntp */


/* ay_nct_estlen:
 *  estimate length of NURBS curve <nc>,
 *  outputs result in <len>
 */
int
ay_nct_estlen(ay_nurbcurve_object *nc, double *len)
{
 int ay_status = AY_OK;
 double v[3], slen, tlen, *Qw = NULL;
 int a, i, j, nb = 0;
 int stride = 4, freeQw = AY_FALSE;

  if(!nc || !len)
    return AY_ENULL;

  *len = 0.0;

  /* special case for polygons */
  if(nc->order == 2)
    {
      a = 0;
      Qw = nc->controlv;
      for(j = 0; j < (nc->length-1); j++)
	{
	  v[0] = Qw[a+stride] - Qw[a];
	  v[1] = Qw[a+stride+1] - Qw[a+1];
	  v[2] = Qw[a+stride+2] - Qw[a+2];

	  if(fabs(v[0]) > AY_EPSILON ||
	     fabs(v[1]) > AY_EPSILON ||
	     fabs(v[2]) > AY_EPSILON)
	    {
	      *len += AY_V3LEN(v);
	    }
	  a += stride;
	} /* for */

      return ay_status;
    } /* if */

  if(nc->length != nc->order)
    {
      if(!(Qw = calloc(nc->order*stride,sizeof(double))))
	return AY_EOMEM;

      freeQw = AY_TRUE;

      ay_status = ay_nb_DecomposeCurve(stride, nc->length-1, nc->order-1,
				       nc->knotv, nc->controlv, &nb, &Qw);
    }
  else
    {
      nb = 1;
      Qw = nc->controlv;
    }

  a = 0;
  for(i = 0; i < nb; i++)
    {
      v[0] = Qw[a+((nc->order-1)*stride)] - Qw[a];
      v[1] = Qw[a+((nc->order-1)*stride)+1] - Qw[a+1];
      v[2] = Qw[a+((nc->order-1)*stride)+2] - Qw[a+2];

      if(fabs(v[0]) > AY_EPSILON ||
	 fabs(v[1]) > AY_EPSILON ||
	 fabs(v[2]) > AY_EPSILON)
	{
	  tlen = AY_V3LEN(v);
	}
      else
	{
	  tlen = 0.0;
	}

      slen = 0.0;
      for(j = 0; j < (nc->order-1); j++)
	{
	  v[0] = Qw[a+stride] - Qw[a];
	  v[1] = Qw[a+stride+1] - Qw[a+1];
	  v[2] = Qw[a+stride+2] - Qw[a+2];

	  if(fabs(v[0]) > AY_EPSILON ||
	     fabs(v[1]) > AY_EPSILON ||
	     fabs(v[2]) > AY_EPSILON)
	    {
	      slen += AY_V3LEN(v);
	    }

	  a += stride;
	} /* for */

      if(tlen > AY_EPSILON && fabs(slen-tlen) > AY_EPSILON)
	{
	  *len += tlen+((slen-tlen)/2.0);
	}
      else
	{
	  *len += slen;
	}

      /* next segment */
      a += stride;
    } /* for */

  if(freeQw)
    {
      free(Qw);
    }

 return ay_status;
} /* ay_nct_estlen */


/* ay_nct_estlentcmd:
 *  Tcl interface for NURBS curve length estimation tool
 */
int
ay_nct_estlentcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *curve = NULL;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL, *po = NULL;
 double len;
 Tcl_Obj *to = NULL, *ton = NULL;

  /* parse args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "vname");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  ton = Tcl_NewStringObj(argv[1], -1);

  /* get curve to work on */
  o = sel->object;
  if(o->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(sel->object, AY_IDNCURVE, &po);
      if(po)
	{
	  curve = (ay_nurbcurve_object *)po->refine;
	}
      else
	{
	  ay_error(AY_ERROR, argv[0], "provide failed");
	  goto cleanup;
	}
    }
  else
    {
      curve = (ay_nurbcurve_object *)o->refine;
    }

  /* get len */
  ay_status = ay_nct_estlen(curve, &len);

  if(ay_status)
    goto cleanup;

  /* put len into Tcl context */

  to = Tcl_NewDoubleObj(len);
  Tcl_ObjSetVar2(interp,ton,NULL,to,TCL_LEAVE_ERR_MSG);

  /* cleanup */
cleanup:
  if(po)
    {
      ay_object_deletemulti(po);
    }

  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return TCL_OK;
} /* ay_nct_estlentcmd */


/* ay_nct_reparamtcmd:
 *  Tcl interface for NURBS curve reparameterisation tool
 */
int
ay_nct_reparamtcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *curve;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 int i, stride = 4, type = 0;
 double *vtemp = NULL;

  /* parse args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "t");
      return TCL_OK;
    }

  Tcl_GetInt(interp, argv[1], &type);

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;
      if(o->type != AY_IDNCURVE)
	{
	  ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
	}
      else
	{
	  curve = (ay_nurbcurve_object *)o->refine;

	  switch(type)
	    {
	    case 0:
	      /* chordal */
	      ay_status = ay_knots_chordparam(curve->controlv, curve->length,
					      stride, &vtemp);
	      if(ay_status)
		{
		  ay_error(AY_ERROR, argv[0], "Knot creation failed.");
		  return(TCL_OK);
		}
	      for(i=0; i<curve->order-1; i++)
		(curve->knotv)[i] = 0.0;
	      memcpy(&(curve->knotv[curve->order-2]), vtemp,
		       curve->length*sizeof(double));
	      for(i=curve->length+2; i<curve->length+curve->order-1; i++)
		(curve->knotv)[i] = 1.0;
	      curve->knot_type = AY_KTCUSTOM;
	      free(vtemp);
	      break;
	    case 1:
	      /* centripetal */
	      ay_status = ay_knots_centriparam(curve->controlv, curve->length,
					       stride, &vtemp);
	      if(ay_status)
		{
		  ay_error(AY_ERROR, argv[0], "Knot creation failed.");
		  return(TCL_OK);
		}
	      for(i=0; i<curve->order-1; i++)
		(curve->knotv)[i] = 0.0;
	      memcpy(&(curve->knotv[curve->order-2]), vtemp,
		       curve->length*sizeof(double));
	      for(i=curve->length+2; i<curve->length+curve->order-1; i++)
		(curve->knotv)[i] = 1.0;
	      curve->knot_type = AY_KTCUSTOM;
	      free(vtemp);
	      break;
	    default:
	      break;
	    } /* switch */

	  /* clean up */
	  ay_status = ay_nct_recreatemp(curve);
	  o->modified = AY_TRUE;

	  /* re-create tesselation of curve */
	  ay_notify_force(sel->object);
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_reparamtcmd */


/* ay_nct_evaltcmd:
 *  Tcl interface for NURBS curve evaluation
 */
int
ay_nct_evaltcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *curve;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 int apply_trafo = AY_FALSE, to_world = AY_FALSE, argi = 0;
 double u, point[4] = {0}, m[16] = {0};
 Tcl_Obj *to = NULL, *ton = NULL;

  /* parse args */
  if(argc < 5)
    {
      ay_error(AY_EARGS, argv[0], "[-trafo|-world] u vnx vny vnz");
      return TCL_OK;
    }

  if(argv[1][0] == '-' && argv[1][1] == 't')
    {
      apply_trafo = AY_TRUE;
      argi++;
    }

  if(argv[1][0] == '-' && argv[1][1] == 'w')
    {
      apply_trafo = AY_TRUE;
      to_world = AY_TRUE;
      argi++;
    }

  Tcl_GetDouble(interp, argv[argi+1], &u);

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  if(o->type != AY_IDNCURVE)
    {
      ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
    }
  else
    {
      curve = (ay_nurbcurve_object *)o->refine;

      if((u < curve->knotv[curve->order-1]) ||
	 (u > curve->knotv[curve->length]))
	{
	  ay_error(AY_ERROR, argv[0], "Parameter out of range.");

	  return TCL_OK;
	}

      if(curve->is_rat)
	{
	  ay_status = ay_nb_CurvePoint4D(curve->length-1, curve->order-1,
					 curve->knotv, curve->controlv,
					 u, point);
	}
      else
	{
	  ay_status = ay_nb_CurvePoint3D(curve->length-1, curve->order-1,
					 curve->knotv, curve->controlv,
					 u, point);
	}

      if(ay_status)
	{
	  ay_error(AY_ERROR, argv[0], "Evaluation failed.");
	  return TCL_OK;
	}
      else
	{
	  if(apply_trafo)
	    {
	      if(to_world)
		{
		  glMatrixMode(GL_MODELVIEW);
		  glPushMatrix();
		   glLoadIdentity();
		   if(ay_currentlevel->object != ay_root)
		     {
		       ay_trafo_getall(ay_currentlevel->next);
		     }
		   glTranslated(o->movx, o->movy, o->movz);
		   ay_quat_torotmatrix(o->quat, m);
		   glMultMatrixd((GLdouble*)m);
		   glScaled(o->scalx, o->scaly, o->scalz);
		   glGetDoublev(GL_MODELVIEW_MATRIX, m);
		  glPopMatrix();
		}
	      else
		{
		  ay_trafo_creatematrix(o, m);
		}
	      ay_trafo_apply3(point, m);
	    } /* if */

	  ton = Tcl_NewStringObj(argv[argi+2],-1);
	  to = Tcl_NewDoubleObj(point[0]);
	  Tcl_ObjSetVar2(interp,ton,NULL,to, TCL_LEAVE_ERR_MSG);

	  Tcl_SetStringObj(ton,argv[argi+3],-1);
	  to = Tcl_NewDoubleObj(point[1]);
	  Tcl_ObjSetVar2(interp,ton,NULL,to, TCL_LEAVE_ERR_MSG);

	  Tcl_SetStringObj(ton,argv[argi+4],-1);
	  to = Tcl_NewDoubleObj(point[2]);
	  Tcl_ObjSetVar2(interp,ton,NULL,to, TCL_LEAVE_ERR_MSG);

	  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
	} /* if */

    } /* if */

 return TCL_OK;
} /* ay_nct_evaltcmd */


/* templates */
#if 0

/* Tcl command */

/* ay_nct_xxxxtcmd:
 *
 */
int
ay_nct_xxxxtcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *curve;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;

  /* parse args */
  if(argc < 3)
    {
      ay_error(AY_EARGS, argv[0], "u r");
      return TCL_OK;
    }

  Tcl_GetDouble(interp, argv[1], &u);
  Tcl_GetInt(interp, argv[2], &r);

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;
      if(o->type != AY_IDNCURVE)
	{
	  ay_error(AY_EWTYPE, argv[0], ay_nct_ncname);
	}
      else
	{
	  curve = (ay_nurbcurve_object *)o->refine;

	  /* do magic */

	  /* clean up */
	  ay_status = ay_nct_recreatemp(curve);
	  ay_selp_clear(o);
	  o->modified = AY_TRUE;

	  /* re-create tesselation of curve */
	  ay_notify_force(sel->object);
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_nct_xxxxtcmd */

#endif
