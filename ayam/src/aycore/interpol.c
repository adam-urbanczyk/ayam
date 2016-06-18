/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2004 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* interpol.c - helpers for interpolation of various elements */

/* ay_interpol_1DA4DIC:
 *  interpolate between two 1D arrays of 4D points
 *  with a interpolation control curve
 *
 */
int
ay_interpol_1DA4DIC(ay_nurbcurve_object *nc, int len, double *st, double *en,
		    double *ta)
{
 int i;
 double p, V1[3], ulen;

  if(!nc || !st || !en || !ta)
    return AY_ENULL;

  ulen = fabs(nc->knotv[nc->length] - nc->knotv[nc->order-1]);

  for(i = 0; i < (len*4); i += 4)
    {
      p = nc->knotv[nc->order-1]+(double)i/len*ulen;
      ay_nb_CurvePoint4D(nc->length+1, nc->order-1, nc->knotv, nc->controlv,
			 p, V1);
      p = V1[1];
      V1[0] = en[i]   - st[i];
      V1[1] = en[i+1] - st[i+1];
      V1[2] = en[i+2] - st[i+2];
      if((fabs(V1[0]) > AY_EPSILON) ||
	 (fabs(V1[1]) > AY_EPSILON) ||
	 (fabs(V1[2]) > AY_EPSILON))
	{
	  AY_V3SCAL(V1,p);
	  V1[0] += st[i];
	  V1[1] += st[i+1];
	  V1[2] += st[i+2];
	  memcpy(&(ta[i]),V1,3*sizeof(double));
	}
      else
	{
	  memcpy(&(ta[i]),&(st[i]),3*sizeof(double));
	}
      /* interpolate weight */
      ta[i+3] = st[i+3]+(p*(en[i+3]-st[i+3]));
    } /* for */

 return AY_OK;
} /* ay_interpol_1DA4DIC */


/* ay_interpol_1DA4D:
 *  use <p> as parameter (between 0.0 and 1.0) to interpolate between
 *  two 1D arrays of 4D points (<st> and <en>) of length <len>
 *  and store the results in target array <ta>
 */
int
ay_interpol_1DA4D(double p, int len, double *st, double *en, double *ta)
{
 int i;
 double V1[3];

  if(!st || !en || !ta)
    return AY_ENULL;

  for(i = 0; i < (len*4); i += 4)
    {
      V1[0] = en[i]   - st[i];
      V1[1] = en[i+1] - st[i+1];
      V1[2] = en[i+2] - st[i+2];
      if((fabs(V1[0]) > AY_EPSILON) ||
	 (fabs(V1[1]) > AY_EPSILON) ||
	 (fabs(V1[2]) > AY_EPSILON))
	{
	  AY_V3SCAL(V1,p);
	  V1[0] += st[i];
	  V1[1] += st[i+1];
	  V1[2] += st[i+2];
	  memcpy(&(ta[i]),V1,3*sizeof(double));
	}
      else
	{
	  memcpy(&(ta[i]),&(st[i]),3*sizeof(double));
	}
      /* interpolate weight */
      ta[i+3] = st[i+3]+(p*(en[i+3]-st[i+3]));
    } /* for */

 return AY_OK;
} /* ay_interpol_1DA4D */


/* ay_interpol_1DA1D:
 *  use <p> as parameter (between 0.0 and 1.0) to interpolate between
 *  two 1D arrays of floating point numbers (<st> and <en>) of length <len>
 *  and store the results in target array <ta>
 */
int
ay_interpol_1DA1D(double p, int len, double *st, double *en, double *ta)
{
 int i;

  if(!st || !en || !ta)
    return AY_ENULL;

  for(i = 0; i < len; i++)
    {
      ta[i] = st[i]+(p*(en[i]-st[i]));
    } /* for */

 return AY_OK;
} /* ay_interpol_1DA1D */


/* ay_interpol_trafos:
 *  use <p> as parameter (between 0.0 and 1.0) to interpolate between
 *  the transformation attributes of two objects (<o1> and <o2>)
 *  and store the resulting transformation attributes in object <ta>
 */
void
ay_interpol_trafos(double p, ay_object *o1, ay_object *o2, ay_object *ta)
{
 double euler[3];

  if(!o1 || !o2 || !ta)
    return;

  ta->movx = o1->movx+(p*(o2->movx - o1->movx));
  ta->movy = o1->movy+(p*(o2->movy - o1->movy));
  ta->movz = o1->movz+(p*(o2->movz - o1->movz));

  ta->scalx = o1->scalx+(p*(o2->scalx - o1->scalx));
  ta->scaly = o1->scaly+(p*(o2->scaly - o1->scaly));
  ta->scalz = o1->scalz+(p*(o2->scalz - o1->scalz));

  if((fabs(o2->quat[0] - o1->quat[0]) > AY_EPSILON) ||
     (fabs(o2->quat[1] - o1->quat[1]) > AY_EPSILON) ||
     (fabs(o2->quat[2] - o1->quat[2]) > AY_EPSILON) ||
     (fabs(o2->quat[3] - o1->quat[3]) > AY_EPSILON))
    {
      ay_quat_slerp(p, o1->quat, o2->quat, ta->quat);

      ay_quat_toeuler(ta->quat, euler);

      ta->rotx = euler[0];
      ta->roty = euler[1];
      ta->rotz = euler[2];
    }
  else
    {
      memcpy(ta->quat, o1->quat, 4*sizeof(double));

      ta->rotx = o1->rotx;
      ta->roty = o1->roty;
      ta->rotz = o1->rotz;
    } /* if */

 return;
} /* ay_interpol_trafos */


/** ay_interpol_ncurves:
 * Interpolate (tween) between two curves.
 *
 * \param[in] p ratio of c1 and c2 (0.0 - 1.0)
 * \param[in] c1 first NURBS curve
 * \param[in] c2 second NURBS curve
 * \param[in] ic interpolation control NURBS curve (may be NULL)
 * \param[in,out] ta where to store the resulting curve
 *
 * \returns AY_OK on success, error code otherwise
 */
int
ay_interpol_ncurves(double p, ay_object *c1, ay_object *c2, ay_object *ic,
		    ay_object **ta)
{
 int ay_status = AY_OK;
 ay_object *newo = NULL;
 ay_nurbcurve_object *nc1, *nc2, *nc3 = NULL, *newnc = NULL;
 double *newknotv = NULL, *newcontrolv = NULL;
 int stride = 4;

  if(!c1 || !c2 || !ta)
    return AY_ENULL;

  if((c1->type != AY_IDNCURVE) || (c2->type != AY_IDNCURVE))
    return AY_EARGS;

  nc1 = (ay_nurbcurve_object *)c1->refine;
  nc2 = (ay_nurbcurve_object *)c2->refine;

  if(ic)
    {
      if(ic->type != AY_IDNCURVE)
	return AY_EARGS;
      nc3 = (ay_nurbcurve_object *)ic->refine;
    }

  if(nc1->length != nc2->length)
    return AY_ERROR;

  if(nc1->order != nc2->order)
    return AY_ERROR;

  if(!(newcontrolv = calloc(nc1->length*stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(newnc = calloc(1, sizeof(ay_nurbcurve_object))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(newo = calloc(1, sizeof(ay_object))))
    { ay_status = AY_EOMEM; goto cleanup; }
  ay_object_defaults(newo);
  newo->type = AY_IDNCURVE;
  newo->refine = newnc;
  if(nc3)
    {
      ay_status = ay_interpol_1DA4DIC(nc3, nc1->length,
				      nc1->controlv, nc2->controlv,
				      newcontrolv);
    }
  else
    {
      ay_status = ay_interpol_1DA4D(p, nc1->length,
				    nc1->controlv, nc2->controlv,
				    newcontrolv);
    }
  if(ay_status)
    goto cleanup;

  newnc->length = nc1->length;
  newnc->order = nc1->order;
  newnc->controlv = newcontrolv;

  /* infer new knot type */
  if(nc1->knot_type != nc2->knot_type)
    newnc->knot_type = AY_KTCUSTOM;
  else
    newnc->knot_type = nc1->knot_type;

  if(newnc->knot_type == AY_KTCUSTOM)
    {
      if((fabs(nc1->knotv[0] - nc2->knotv[0]) > AY_EPSILON) ||
	 (fabs(nc1->knotv[nc1->length+nc1->order-1] -
	       nc2->knotv[nc2->length+nc2->order-1]) > AY_EPSILON))
	{
	  ay_status = ay_knots_rescaletorange(nc2->length+nc2->order,
					      nc2->knotv, nc1->knotv[0],
				       nc1->knotv[nc1->length+nc1->order-1]);
	  if(ay_status)
	    goto cleanup;
	}

      if(!(newknotv = calloc(nc1->length+nc1->order, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      ay_status = ay_interpol_1DA1D(p, nc1->length+nc1->order,
				    nc1->knotv, nc2->knotv,
				    newknotv);
      newnc->knotv = newknotv;
    }
  else
    {
      ay_status = ay_knots_createnc(newnc);
    }

  if(ay_status)
    goto cleanup;

  newnc->is_rat = ay_nct_israt(newnc);
  ay_nct_settype(newnc);

  ay_interpol_trafos(p, c1, c2, newo);

  *ta = newo;

  newo = NULL;
  newknotv = NULL;
  newcontrolv = NULL;
  newnc = NULL;

cleanup:
  if(newknotv)
    free(newknotv);
  if(newcontrolv)
    free(newcontrolv);
  if(newnc)
    free(newnc);
  if(newo)
    free(newo);

 return ay_status;
} /* ay_interpol_ncurves */


/** ay_interpol_curvestcmd:
 * Interpolate/tween curves.
 *
 *  Implements the \a tweenNC scripting interface command.
 *  See also the corresponding section in the \ayd{sctweennc}.
 *  \returns TCL_OK in any case.
 */
int
ay_interpol_curvestcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 double r = 0.5;

  /* parse args */
  if(argc > 1)
    {
      tcl_status = Tcl_GetDouble(interp, argv[1], &r);
      AY_CHTCLERRRET(tcl_status, argv[0], interp);
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if((!sel->next) || (sel->object->type != AY_IDNCURVE) ||
     (sel->next->object->type != AY_IDNCURVE))
    {
      ay_error(AY_ERROR, argv[0], "Select two NURBS curves.");
      return TCL_OK;
    }

  if(sel->next && sel->next->next &&
     sel->next->next->object->type == AY_IDNCURVE)
    {
      ay_status = ay_interpol_ncurves(r, sel->object, sel->next->object,
				      sel->next->next->object, &o);
    }
  else
    {
      ay_status = ay_interpol_ncurves(r, sel->object, sel->next->object,
				      NULL, &o);
    }

  if(ay_status)
    {
      ay_error(AY_ERROR, argv[0], "Interpolation failed.");
    }
  else
    {
      ay_object_link(o);
      ay_notify_parent();
    }

 return TCL_OK;
} /* ay_interpol_curvestcmd */


/** ay_interpol_npatches:
 * Interpolate (tween) between two NURBS patches.
 *
 * \param[in] r ratio of p1 and p2 (0.0 - 1.0)
 * \param[in] p1 first NURBS patch
 * \param[in] p2 second NURBS patch
 * \param[in,out] ta where to store the resulting patch
 *
 * \returns AY_OK on success, error code otherwise
 */
int
ay_interpol_npatches(double r, ay_object *p1, ay_object *p2, ay_object **ta)
{
 int ay_status = AY_OK;
 ay_object *newo = NULL;
 ay_nurbpatch_object *np1, *np2, *newnp = NULL;
 double *newuknotv = NULL, *newvknotv = NULL, *newcontrolv = NULL;
 int stride = 4;

  if(!p1 || !p2 || !ta)
    return AY_ENULL;

  if((p1->type != AY_IDNPATCH) || (p2->type != AY_IDNPATCH))
    return AY_EARGS;

  np1 = (ay_nurbpatch_object *)p1->refine;
  np2 = (ay_nurbpatch_object *)p2->refine;

  if((np1->width != np2->width)||(np1->height != np2->height))
    return AY_ERROR;

  if((np1->uorder != np2->uorder)||(np1->vorder != np2->vorder))
    return AY_ERROR;

  if(!(newcontrolv = calloc(np1->width*np1->height*stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(newnp = calloc(1, sizeof(ay_nurbpatch_object))))
    { ay_status = AY_EOMEM; goto cleanup; }

  ay_status = ay_npt_createnpatchobject(&newo);

  if(ay_status || !newo)
    goto cleanup;

  newo->refine = newnp;

  ay_status = ay_interpol_1DA4D(r, np1->width*np1->height,
				np1->controlv, np2->controlv,
				newcontrolv);
  if(ay_status)
    goto cleanup;

  newnp->width = np1->width;
  newnp->height = np1->height;
  newnp->uorder = np1->uorder;
  newnp->vorder = np1->vorder;
  newnp->controlv = newcontrolv;

  /* infer new knot types */
  if(np1->uknot_type != np2->uknot_type)
    newnp->uknot_type = AY_KTCUSTOM;
  else
    newnp->uknot_type = np1->uknot_type;

  if(newnp->uknot_type == AY_KTCUSTOM)
    {
      if((fabs(np1->uknotv[0] - np2->uknotv[0]) > AY_EPSILON) ||
	 (fabs(np1->uknotv[np1->width+np1->uorder-1] -
	       np2->uknotv[np2->width+np2->uorder-1]) > AY_EPSILON))
	{
	  ay_status = ay_knots_rescaletorange(np2->width+np2->uorder,
					      np2->uknotv, np1->uknotv[0],
				       np1->uknotv[np1->width+np1->uorder-1]);
	  if(ay_status)
	    goto cleanup;
	}

      if(!(newuknotv = calloc(np1->width+np1->uorder, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      ay_status = ay_interpol_1DA1D(r, np1->width+np1->uorder,
				    np1->uknotv, np2->uknotv,
				    newuknotv);
      newnp->uknotv = newuknotv;

      if(ay_status)
	goto cleanup;
    }

  if(np1->vknot_type != np2->vknot_type)
    newnp->vknot_type = AY_KTCUSTOM;
  else
    newnp->vknot_type = np1->vknot_type;

  if(newnp->vknot_type == AY_KTCUSTOM)
    {
      if((fabs(np1->vknotv[0] - np2->vknotv[0]) > AY_EPSILON) ||
	 (fabs(np1->vknotv[np1->height+np1->vorder-1] -
	       np2->vknotv[np2->height+np2->vorder-1]) > AY_EPSILON))
	{
	  ay_status = ay_knots_rescaletorange(np2->height+np2->vorder,
					      np2->vknotv, np1->vknotv[0],
				       np1->vknotv[np1->height+np1->vorder-1]);
	  if(ay_status)
	    goto cleanup;
	}

      if(!(newvknotv = calloc(np1->height+np1->vorder, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      ay_status = ay_interpol_1DA1D(r, np1->height+np1->vorder,
				    np1->vknotv, np2->vknotv,
				    newvknotv);
      newnp->vknotv = newvknotv;

      if(ay_status)
	goto cleanup;
    }

  if((newnp->uknot_type != AY_KTCUSTOM) || (newnp->vknot_type != AY_KTCUSTOM))
    {
      ay_status = ay_knots_createnp(newnp);

      if(ay_status)
	goto cleanup;
    }

  newnp->is_rat = ay_npt_israt(newnp);
  ay_npt_setuvtypes(newnp, 0);

  ay_interpol_trafos(r, p1, p2, newo);

  *ta = newo;

  newo = NULL;
  newuknotv = NULL;
  newvknotv = NULL;
  newcontrolv = NULL;
  newnp = NULL;

cleanup:
  if(newuknotv)
    free(newuknotv);
  if(newvknotv)
    free(newvknotv);
  if(newcontrolv)
    free(newcontrolv);
  if(newnp)
    free(newnp);
  if(newo)
    free(newo);

 return ay_status;
} /* ay_interpol_npatches */


/** ay_interpol_surfacestcmd:
 *
 *  Implements the \a tweenNP scripting interface command.
 *  See also the corresponding section in the \ayd{sctweennp}.
 *  \returns TCL_OK in any case.
 */
int
ay_interpol_surfacestcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[])
{
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 double r = 0.5;

  /* parse args */
  if(argc > 1)
    {
      tcl_status = Tcl_GetDouble(interp, argv[1], &r);
      AY_CHTCLERRRET(tcl_status, argv[0], interp);
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if((!sel->next) || (sel->object->type != AY_IDNPATCH) ||
     (sel->next->object->type != AY_IDNPATCH))
    {
      ay_error(AY_ERROR, argv[0], "Select two NURBS patches.");
      return TCL_OK;
    }

  ay_status = ay_interpol_npatches(r, sel->object, sel->next->object, &o);

  if(ay_status)
    {
      ay_error(AY_ERROR, argv[0], "Interpolation failed.");
    }
  else
    {
      ay_object_link(o);
      ay_notify_parent();
    }

 return TCL_OK;
} /* ay_interpol_surfacestcmd */
