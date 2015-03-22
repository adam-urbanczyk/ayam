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
 * \param p ratio of c1 and c2 (0.0 - 1.0)
 * \param c1 first NURBS curve
 * \param c2 second NURBS curve
 * \param ta where to store the resulting curve
 *
 * \returns AY_OK on success, error code otherwise
 */
int
ay_interpol_ncurves(double p, ay_object *c1, ay_object *c2, ay_object **ta)
{
 int ay_status = AY_OK;
 ay_object *newo = NULL;
 ay_nurbcurve_object *nc1, *nc2, *newnc = NULL;
 double *newknotv = NULL, *newcontrolv = NULL;
 int stride = 4;

  if(!c1 || !c2 || !ta)
    return AY_ENULL;

  if((c1->type != AY_IDNCURVE) || (c2->type != AY_IDNCURVE))
    return AY_EARGS;

  nc1 = (ay_nurbcurve_object *)c1->refine;
  nc2 = (ay_nurbcurve_object *)c2->refine;

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

  ay_status = ay_interpol_1DA4D(p, nc1->length, nc1->controlv, nc2->controlv,
				newcontrolv);
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
 *
 *  Implements the \a getMaster scripting interface command.
 *  See also the corresponding section in the \ayd{scgetmaster}.
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

  ay_status = ay_interpol_ncurves(r, sel->object, sel->next->object, &o);

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
