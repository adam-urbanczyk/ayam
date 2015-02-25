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

/** \file npt.c \brief NURBS patch tools */

/* local variables: */

char ay_npt_npname[] = "NPatch";

typedef void (ay_npt_gndcb) (char dir, ay_nurbpatch_object *np,
			     int i, double *p, double **dp);

/* prototypes of functions local to this module: */

/** Helper for gordonwc().
 */
int ay_npt_gordonmodw(ay_object *o1, ay_object *o2);

/** Helper for gordonwc().
 */
void ay_npt_gordoncc(ay_object *o1, ay_object *o2, int stride,
		     double *p1, double *p2, double *pp1, double *pp2,
		     double *m1, double *m2);

/** Helper for gordonwc().
 */
void ay_npt_gordonwcgetends(ay_object *o,
			    double *s, double *e, double **ss, double **se,
			    int *trafo, double *tm, double *tmi);

int ay_npt_rescaletrim(ay_object *trim,
		       int mode, double omin, double omax,
		       double nmin, double nmax);

void ay_npt_gndu(char dir, ay_nurbpatch_object *np, int i, double *p,
		 double **dp);

void ay_npt_gndv(char dir, ay_nurbpatch_object *np, int j, double *p,
		 double **dp);

void ay_npt_gnduc(char dir, ay_nurbpatch_object *np, int i, double *p,
		  double **dp);

void ay_npt_gndvc(char dir, ay_nurbpatch_object *np, int j, double *p,
		  double **dp);

void ay_npt_gndup(char dir, ay_nurbpatch_object *np, int i, double *p,
		  double **dp);

void ay_npt_gndvp(char dir, ay_nurbpatch_object *np, int j, double *p,
		  double **dp);

int ay_npt_getnormal(ay_nurbpatch_object *np, int i, int j,
		     ay_npt_gndcb *gnducb, ay_npt_gndcb *gndvcb,
		     int store_tangents, double *result);

/* functions: */

/** ay_npt_create:
 *  Create a NURBS patch object.
 *
 * \param[in] uorder Order of new patch in U direction (2 - 10, unchecked)
 * \param[in] vorder Order of new patch in V direction (2 - 10, unchecked)
 * \param[in] width Width of new patch (U direction) (2 - 100, unchecked)
 * \param[in] height Height of new patch (V direction) (2 - 100, unchecked)
 * \param[in] uknot_type Knot type of new patch in U direction (AY_KT*)
 * \param[in] vknot_type Knot type of new patch in V direction (AY_KT*)
 * \param[in] controlv Pointer to control points [width*height*stride]
 *            may be NULL
 * \param[in] uknotv Pointer to knots [width+uorder]
 *            may be NULL unless uknot_type is AY_KTCUSTOM
 * \param[in] vknotv Pointer to knots [height+vorder]
 *            may be NULL unless vknot_type is AY_KTCUSTOM
 * \param[in,out] patchptr new NURBS patch object
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_create(int uorder, int vorder, int width, int height,
	      int uknot_type, int vknot_type,
	      double *controlv, double *uknotv, double *vknotv,
	      ay_nurbpatch_object **patchptr)
{
 int ay_status = AY_OK;
 double *newcontrolv = NULL;
 ay_nurbpatch_object *patch = NULL;

  if(!(patch = calloc(1, sizeof(ay_nurbpatch_object))))
    return AY_EOMEM;

  patch->uorder = uorder;
  patch->vorder = vorder;
  patch->width = width;
  patch->height = height;
  patch->uknot_type = uknot_type;
  patch->vknot_type = vknot_type;

  if(!controlv)
    {
      if(!(newcontrolv = calloc(4*width*height, sizeof(double))))
	{
	  free(patch);
	  return AY_EOMEM;
	}

      patch->controlv = newcontrolv;
    }
  else
    {
      patch->controlv = controlv;
      /* XXXX check user supplied control vector? */
      patch->is_rat = ay_npt_israt(patch);
    } /* if */

  if(((uknot_type != AY_KTCUSTOM) && !uknotv) ||
     ((vknot_type != AY_KTCUSTOM) && !vknotv))
    {
      /* we need to create knots */
      ay_status = ay_knots_createnp(patch);
      if(ay_status)
	{
	  if(newcontrolv)
	    {
	      free(newcontrolv);
	      patch->controlv = NULL;
	    }
	  free(patch);
	  return ay_status;
	} /* if */

      if((uknot_type == AY_KTCUSTOM) && uknotv)
	{
	  if(patch->uknotv)
	    free(patch->uknotv);
	  patch->uknotv = uknotv;
	} /* if */

      if((vknot_type == AY_KTCUSTOM) && vknotv)
	{
	  if(patch->vknotv)
	    free(patch->vknotv);
	  patch->vknotv = vknotv;
	} /* if */
    }
  else
    {
      /* caller specified own knots */
      patch->uknotv = uknotv;
      patch->vknotv = vknotv;
    } /* if */

  if(controlv && patch->uknotv && patch->vknotv)
    {
      ay_npt_setuvtypes(patch, 0);
    }

  /* return result */
  *patchptr = patch;

 return AY_OK;
} /* ay_npt_create */


/** ay_npt_destroy:
 *  gracefully destroy a NURBS patch object
 *
 * \param[in,out] patch NURBS patch object to destroy
 */
void
ay_npt_destroy(ay_nurbpatch_object *patch)
{

  if(!patch)
    return;

  /* free multiple points */
  if(patch->mpoints)
    ay_npt_clearmp(patch);

  /* free gluNurbsRenderer */
  if(patch->no)
    gluDeleteNurbsRenderer(patch->no);

  /* free tesselation */
  if(patch->stess)
    ay_stess_destroy(patch);

  if(patch->controlv)
    free(patch->controlv);

  if(patch->uknotv)
    free(patch->uknotv);

  if(patch->vknotv)
    free(patch->vknotv);

  free(patch);

 return;
} /* ay_npt_destroy */


/** ay_npt_createnpatchobject:
 *  properly create and set up an ay_object structure to be used
 *  with a NURBS patch object
 *
 * \param[in,out] result new Ayam object
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_createnpatchobject(ay_object **result)
{
 ay_object *o = NULL;

  if(!result)
    return AY_ENULL;

  o = calloc(1, sizeof(ay_object));
  if(!o)
    return AY_EOMEM;
  ay_object_defaults(o);
  o->type = AY_IDNPATCH;
  o->parent = AY_TRUE;
  o->hide_children = AY_TRUE;
  o->inherit_trafos = AY_FALSE;
  *result = o;

 return AY_OK;
} /* ay_npt_createnpatchobject */


/** ay_npt_resetdisplay:
 *   reset the display attributes of a NURBS patch
 *
 * \param[in,out] o NURBS patch object to reset
 */
void
ay_npt_resetdisplay(ay_object *o)
{
 ay_nurbpatch_object *patch;

  if(!o || (o->type != AY_IDNPATCH))
    return;

  patch = (ay_nurbpatch_object *)o->refine;

  patch->glu_sampling_tolerance = 0.0;
  patch->display_mode = 0;

 return;
} /* ay_npt_resetdisplay */


/** ay_npt_euctohom:
 *  convert rational coordinates from euclidean to homogeneous style
 *
 * \param[in,out] np NURBS patch object to process
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_euctohom(ay_nurbpatch_object *np)
{
 int stride = 4;
 double *cv;
 int i, a;

  if(!np)
    return AY_ENULL;

  if(!np->is_rat)
    return AY_OK;

  cv = np->controlv;

  a = 0;
  for(i = 0; i < np->width*np->height; i++)
    {
      cv[a]   *= cv[a+3];
      cv[a+1] *= cv[a+3];
      cv[a+2] *= cv[a+3];
      a += stride;
    }

 return AY_OK;
} /* ay_npt_euctohom */


/** ay_npt_homtoeuc:
 *  convert rational coordinates from homogeneous to euclidean style
 *
 * \param[in,out] np NURBS patch object to process
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_homtoeuc(ay_nurbpatch_object *np)
{
 int stride = 4;
 double *cv;
 int i, a;

  if(!np)
    return AY_ENULL;

  if(!np->is_rat)
    return AY_OK;

  cv = np->controlv;

  a = 0;
  for(i = 0; i < np->width*np->height; i++)
    {
      cv[a]   /= cv[a+3];
      cv[a+1] /= cv[a+3];
      cv[a+2] /= cv[a+3];
      a += stride;
    }

 return AY_OK;
} /* ay_npt_homtoeuc */


/** ay_npt_resizearrayw:
 *  change width of a 2D control point array;
 *  if the new width is smaller than the current width, the array
 *  will be simply truncated, otherwise new control points obtained by
 *  linear interpolation will be inserted in all sections
 *
 * \param[in,out] controlvptr 2D control point array to process
 * \param[in] stride size of a point
 * \param[in] width width of array
 * \param[in] height height of array
 * \param[in] new_width new width
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_resizearrayw(double **controlvptr, int stride,
		    int width, int height, int new_width)
{
 int ay_status = AY_OK;
 int i, j, k, a, b;
 int *newpersec = NULL, new = 0;
 double *ncontrolv = NULL, *controlv = NULL, v[3] = {0}, t = 0.0;

  if(new_width == width)
    return ay_status;

  if(!controlvptr)
    return AY_ENULL;

  controlv = *controlvptr;

  if(!controlv)
    return AY_ENULL;

  if(!(ncontrolv = malloc(height*new_width*stride*sizeof(double))))
    return AY_EOMEM;

  if(new_width < width)
    {
      a = 0; b = 0;
      for(i = 0; i < new_width; i++)
	{
	  memcpy(&(ncontrolv[b]), &(controlv[a]),
		 height*stride*sizeof(double));

	  a += (height*stride);
	  b += (height*stride);
	} /* for */
    }
  else
    {
      /* distribute new points */
      new = new_width-width;

      if(!(newpersec = calloc((width-1), sizeof(int))))
	{
	  free(ncontrolv);
	  return AY_EOMEM;
	}

      while(new)
	{
	  for(i = 0; i < (width-1); i++)
	    {
	      if(new)
		{
		  (newpersec[i])++;
		  new--;
		}
	    } /* for */
	} /* while */

      a = 0;
      b = 0;
      for(k = 0; k < (width-1); k++)
	{
	  memcpy(&(ncontrolv[b]), &(controlv[a]),
		 height*stride*sizeof(double));

	  b += (height*stride);

	  for(j = 1; j <= newpersec[k]; j++)
	    {
	      t = j/(newpersec[k]+1.0);
	      a = k*height*stride;
	      for(i = 0; i < height; i++)
		{
		  v[0] = controlv[a+(stride*height)] -
		    controlv[a];
		  v[1] = controlv[a+(stride*height)+1] -
		    controlv[a+1];
		  v[2] = controlv[a+(stride*height)+2] -
		    controlv[a+2];

		  AY_V3SCAL(v,t);

		  ncontrolv[b]   = controlv[a]   + v[0];
		  ncontrolv[b+1] = controlv[a+1] + v[1];
		  ncontrolv[b+2] = controlv[a+2] + v[2];
		  if(stride > 3)
		    ncontrolv[b+3] = 1.0;

		  a += stride;
		  b += stride;
		} /* for i */
	    } /* for j */

	  if(newpersec[k] == 0)
	    a += (height*stride);

	} /* for k */

      memcpy(&ncontrolv[(new_width-1)*height*stride],
	     &(controlv[(width-1)*height*stride]),
	     height*stride*sizeof(double));

      free(newpersec);
    } /* if */

  free(controlv);
  *controlvptr = ncontrolv;

 return ay_status;
} /* ay_npt_resizearrayw */


/** ay_npt_resizew:
 *  change width of a NURBS patch
 *  does _not_ maintain multiple points
 *
 * \param[in,out] np NURBS patch to process
 * \param[in] new_width new width
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_resizew(ay_nurbpatch_object *np, int new_width)
{
 int ay_status = AY_OK;

  if(new_width == np->width)
    return ay_status;

  ay_status = ay_npt_resizearrayw(&(np->controlv), 4,
				  np->width, np->height,
				  new_width);

  if(!ay_status)
    np->width = new_width;

 return ay_status;
} /* ay_npt_resizew */


/** ay_npt_resizearrayh:
 *  change height of a 2D control point array;
 *  if the new height is smaller than the current height, the array
 *  will be simply truncated, otherwise new control points obtained by
 *  linear interpolation will be inserted in all sections
 *
 * \param[in,out] controlvptr 2D control point array to process
 * \param[in] stride size of a point
 * \param[in] width width of array
 * \param[in] height height of array
 * \param[in] new_height new height
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_resizearrayh(double **controlvptr, int stride,
		    int width, int height, int new_height)
{
 int ay_status = AY_OK;
 int i, j, k, a, b;
 int *newpersec = NULL, new = 0;
 double *ncontrolv = NULL, *controlv = NULL, v[3] = {0}, t = 0.0;

  if(new_height == height)
    return ay_status;

  if(!controlvptr)
    return AY_ENULL;

  controlv = *controlvptr;

  if(!controlv)
    return AY_ENULL;

  if(!(ncontrolv = malloc(width*new_height*stride*sizeof(double))))
    return AY_EOMEM;

  if(new_height < height)
    {
      a = 0; b = 0;
      for(i = 0; i < width; i++)
	{
	  memcpy(&(ncontrolv[b]), &(controlv[a]),
		 new_height*stride*sizeof(double));

	  a += (height*stride);
	  b += (new_height*stride);
	} /* for */
    }
  else
    {
      /* distribute new points */
      new = new_height-height;

      if(!(newpersec = calloc((height-1), sizeof(int))))
	{
	  free(ncontrolv);
	  return AY_EOMEM;
	}

      while(new)
	{
	  for(i = 0; i < (height-1); i++)
	    {
	      if(new)
		{
		  (newpersec[i])++;
		  new--;
		}
	    } /* for */
	} /* while */

      a = 0;
      b = 0;
      for(k = 0; k < width; k++)
	{
	  for(i = 0; i < (height-1); i++)
	    {
	      memcpy(&ncontrolv[b], &(controlv[a]),
		     stride*sizeof(double));
	      b += stride;

	      for(j = 1; j <= newpersec[i]; j++)
		{
		  v[0] = controlv[a+stride]   - controlv[a];
		  v[1] = controlv[a+stride+1] - controlv[a+1];
		  v[2] = controlv[a+stride+2] - controlv[a+2];

		  t = j/(newpersec[i]+1.0);

		  AY_V3SCAL(v,t);

		  ncontrolv[b]   = controlv[a]   + v[0];
		  ncontrolv[b+1] = controlv[a+1] + v[1];
		  ncontrolv[b+2] = controlv[a+2] + v[2];
		  if(stride > 3)
		    ncontrolv[b+3] = 1.0;

		  b += stride;
		} /* for */
	      a += stride;
	    } /* for */

	  memcpy(&ncontrolv[b/*+(new_height-1)*stride*/],
		 &(controlv[a/*+(height-1)*stride*/]),
		 stride*sizeof(double));

	  a += stride;
	  b += stride;
	} /* for */

      free(newpersec);
    } /* if */

  free(controlv);
  *controlvptr = ncontrolv;

 return ay_status;
} /* ay_npt_resizearrayh */


/** ay_npt_resizeh:
 *  change height of a NURBS patch
 *  does _not_ maintain multiple points
 *
 * \param[in,out] np NURBS patch to process
 * \param[in] new_height new height
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_resizeh(ay_nurbpatch_object *np, int new_height)
{
 int ay_status = AY_OK;

  if(new_height == np->height)
    return ay_status;

  ay_status = ay_npt_resizearrayh(&(np->controlv), 4,
				  np->width, np->height,
				  new_height);

  if(!ay_status)
    np->height = new_height;

 return ay_status;
} /* ay_npt_resizeh */


/** ay_npt_swaparray:
 *  swap u and v dimensions of a 2D control point array
 *
 * \param[in,out] controlvptr 2D control point array to process
 * \param[in] stride size of a point
 * \param[in] width width of array
 * \param[in] height height of array
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_swaparray(double **controlvptr, int stride,
		 int width, int height)
{
 int i1 = 0, i2 = 0, i, j;
 double *ncontrolv = NULL;

  if(!controlvptr || !(*controlvptr))
    return AY_ENULL;

  if(!(ncontrolv = malloc(width*height*stride*sizeof(double))))
    return AY_EOMEM;

  for(i = 0; i < width; i++)
    {
      i2 = i*stride;
      for(j = 0; j < height; j++)
	{
	  memcpy(&(ncontrolv[i2]), &((*controlvptr)[i1]),
		 stride*sizeof(double));

	  i1 += stride;
	  i2 += (width*stride);
	} /* for */
    } /* for */

  free(*controlvptr);
  *controlvptr = ncontrolv;

 return AY_OK;
} /* ay_npt_swaparray */


/** ay_npt_swapuv:
 *  swap u and v dimensions of a NURBS patch; considers control points and
 *  knot vectors
 *
 * \param[in,out] np NURBS patch to process
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_swapuv(ay_nurbpatch_object *np)
{
 int ay_status = AY_OK;
 int i;
 double *dt;

  if(!np)
    return AY_ENULL;

  ay_status = ay_npt_swaparray(&(np->controlv), 4, np->width, np->height);

  if(ay_status)
    return ay_status;

  i = np->width;
  np->width = np->height;
  np->height = i;

  i = np->uorder;
  np->uorder = np->vorder;
  np->vorder = i;

  /* swap knots */
  i = np->uknot_type;
  np->uknot_type = np->vknot_type;
  np->vknot_type = i;

  dt = np->uknotv;
  np->uknotv = np->vknotv;
  np->vknotv = dt;

  /* swap closeness types */
  i = np->utype;
  np->utype = np->vtype;
  np->vtype = i;

  /* since we do not create new multiple points
     we only need to re-create them if there were
     already multiple points in the original surface */
  if(np->mpoints)
    ay_npt_recreatemp(np);

 return ay_status;
} /* ay_npt_swapuv */


/** ay_npt_revertu:
 *  revert control vector and knots of NURBS patch in u dimension
 *
 * \param[in,out] patch NURBS patch object to revert
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_revertu(ay_nurbpatch_object *np)
{
 int ay_status = AY_OK;
 int i, j, ii, jj, stride = 4;
 double t[4];

  for(i = 0; i < np->height; i++)
    {
      for(j = 0; j < np->width/2; j++)
	{
	  ii = (j*np->height+i)*stride;
	  jj = ((np->width-1-j)*np->height+i)*stride;
	  memcpy(t, &(np->controlv[ii]), stride*sizeof(double));
	  memcpy(&(np->controlv[ii]), &(np->controlv[jj]),
		 stride*sizeof(double));
	  memcpy(&(np->controlv[jj]), t, stride*sizeof(double));
	} /* for */
    } /* for */

  /* revert knots */
  if(np->uknot_type >= AY_KTCUSTOM)
    {
      ay_status = ay_knots_revert(np->uknotv, np->width+np->uorder);
    } /* if */

  /* since we do not create new multiple points
     we only need to re-create them if there were
     already multiple points in the original surface */
  if(np->mpoints)
    ay_npt_recreatemp(np);

 return ay_status;
} /* ay_npt_revertu */


/** ay_npt_revertutcmd:
 *  Revert selected surfaces in U direction.
 *  Implements the \a revertuS scripting interface command.
 *  See also the corresponding section in the \ayd{screvertus}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_revertutcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status;
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *np = NULL;
 ay_ipatch_object *ip = NULL;
 ay_pamesh_object *pm = NULL;
 ay_bpatch_object *bp = NULL;
 double pt[3];
 int notify_parent = AY_FALSE;

  while(sel)
    {
      switch(sel->object->type)
	{
	case AY_IDNPATCH:
	  np = (ay_nurbpatch_object *)sel->object->refine;

	  ay_status = ay_npt_revertu(np);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "Revert failed.");
	      goto cleanup;
	    }
	  sel->object->modified = AY_TRUE;
	  break;
	case AY_IDIPATCH:
	  ip = (ay_ipatch_object *)sel->object->refine;

	  ay_ipt_revertu(ip);

	  sel->object->modified = AY_TRUE;
	  break;
	case AY_IDPAMESH:
	  pm = (ay_pamesh_object *)sel->object->refine;

	  ay_pmt_revertu(pm);

	  sel->object->modified = AY_TRUE;
	  break;
	case AY_IDBPATCH:
	  bp = (ay_bpatch_object *)sel->object->refine;

	  memcpy(pt, bp->p1, 3*sizeof(double));
	  memcpy(bp->p1, bp->p2, 3*sizeof(double));
	  memcpy(bp->p2, pt, 3*sizeof(double));
	  memcpy(pt, bp->p3, 3*sizeof(double));
	  memcpy(bp->p3, bp->p4, 3*sizeof(double));
	  memcpy(bp->p4, pt, 3*sizeof(double));

	  sel->object->modified = AY_TRUE;
	  break;
	default:
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	  break;
	} /* switch */

      if(sel->object->modified)
	{
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);
	  (void)ay_notify_object(sel->object);
	  notify_parent = AY_TRUE;
	}

      sel = sel->next;
    } /* while */

cleanup:

  if(notify_parent)
    (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_revertutcmd */


/** ay_npt_revertv:
 *  revert control vector and knots of NURBS patch in v dimension
 *
 * \param[in,out] patch NURBS patch object to revert
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_revertv(ay_nurbpatch_object *np)
{
 int ay_status = AY_OK;
 int i, j, ii, jj, stride = 4;
 double t[4];

  for(i = 0; i < np->width; i++)
    {
      ii = i*np->height*stride;
      jj = ii + ((np->height-1)*stride);
      for(j = 0; j < np->height/2; j++)
	{
	  memcpy(t, &(np->controlv[ii]), stride*sizeof(double));
	  memcpy(&(np->controlv[ii]), &(np->controlv[jj]),
		 stride*sizeof(double));
	  memcpy(&(np->controlv[jj]), t, stride*sizeof(double));
	  ii += stride;
	  jj -= stride;
	} /* for */
    } /* for */

  /* revert knots */
  if(np->vknot_type >= AY_KTCUSTOM)
    {
      ay_status = ay_knots_revert(np->vknotv, np->height+np->vorder);
    } /* if */

  /* since we do not create new multiple points
     we only need to re-create them if there were
     already multiple points in the original surface */
  if(np->mpoints)
    ay_npt_recreatemp(np);

 return ay_status;
} /* ay_npt_revertv */


/** ay_npt_revertvtcmd:
 *  Revert selected surfaces in V direction.
 *  Implements the \a revertvS scripting interface command.
 *  See also the corresponding section in the \ayd{screvertvs}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_revertvtcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status;
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *np = NULL;
 ay_ipatch_object *ip = NULL;
 ay_pamesh_object *pm = NULL;
 ay_bpatch_object *bp = NULL;
 double pt[3];
 int notify_parent = AY_FALSE;

  while(sel)
    {
      switch(sel->object->type)
	{
	case AY_IDNPATCH:
	  np = (ay_nurbpatch_object *)sel->object->refine;

	  ay_status = ay_npt_revertv(np);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "Revert failed.");
	      goto cleanup;
	    }
	  sel->object->modified = AY_TRUE;
	  break;
	case AY_IDIPATCH:
	  ip = (ay_ipatch_object *)sel->object->refine;

	  ay_ipt_revertv(ip);

	  sel->object->modified = AY_TRUE;
	  break;
	case AY_IDPAMESH:
	  pm = (ay_pamesh_object *)sel->object->refine;

	  ay_pmt_revertv(pm);

	  sel->object->modified = AY_TRUE;
	  break;
	case AY_IDBPATCH:
	  bp = (ay_bpatch_object *)sel->object->refine;

	  memcpy(pt, bp->p1, 3*sizeof(double));
	  memcpy(bp->p1, bp->p4, 3*sizeof(double));
	  memcpy(bp->p4, pt, 3*sizeof(double));
	  memcpy(pt, bp->p2, 3*sizeof(double));
	  memcpy(bp->p2, bp->p3, 3*sizeof(double));
	  memcpy(bp->p3, pt, 3*sizeof(double));

	  sel->object->modified = AY_TRUE;
	  break;
	default:
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	  break;
	} /* switch */

      if(sel->object->modified)
	{
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);
	  (void)ay_notify_object(sel->object);
	  notify_parent = AY_TRUE;
	}

      sel = sel->next;
    } /* while */

cleanup:

  if(notify_parent)
    (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_revertvtcmd */


/** ay_npt_drawtrimcurve:
 *  draw a single trimcurve with GLU
 *
 * \param[in] o NURBS curve object (a trim curve)
 * \param[in] no GLU NURBS Renderer object of associated NURBS patch object
 * \param[in] refine how many times shall this trim curve be refined
 *  (for increased tesselation fidelity along trim edges)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_drawtrimcurve(ay_object *o, GLUnurbsObj *no, unsigned int refine)
{
 int ay_status = AY_OK;
 int order = 0, length = 0, knot_count = 0, i = 0, a = 0, b = 0;
 int apply_trafo = AY_FALSE;
 ay_object *n = NULL;
 ay_nurbcurve_object *curve = (ay_nurbcurve_object *) o->refine;
 GLfloat *knots = NULL, *controls = NULL;
 double m[16], x = 0.0, y = 0.0, w = 1.0;

  /* get curves transformation-matrix */
  if(AY_ISTRAFO(o))
    {
      apply_trafo = AY_TRUE;
      ay_trafo_creatematrix(o, m);
    }

  if(refine)
    {
      ay_status = ay_object_copy(o, &n);
      if(n)
	{
	  curve = n->refine;
	}
      else
	{
	  goto cleanup;
	}
      while(refine)
	{
	  ay_nct_refinekn(curve, AY_FALSE, NULL, 0);
	  refine--;
	}
    }

  order = curve->order;
  length = curve->length;

  knot_count = length + order;

  if(ay_prefs.glu_avoid_pwlcurve || curve->order != 2)
    {
      if((knots = malloc(knot_count * sizeof(GLfloat))) == NULL)
	{ ay_status = AY_EOMEM; goto cleanup; }

      a = 0;
      for(i = 0; i < knot_count; i++)
	{
	  knots[a] = (GLfloat)curve->knotv[a];
	  a++;
	}
    }

  if((controls = malloc(length*(curve->is_rat?3:2)*sizeof(GLfloat))) == NULL)
    { ay_status = AY_EOMEM; goto cleanup; }

  a = 0; b = 0;
  for(i = 0; i < length; i++)
    {
      x = (GLdouble)curve->controlv[b];
      y = (GLdouble)curve->controlv[b+1];
      b += 3;

      if(apply_trafo)
	{
	  if(curve->is_rat)
	    {
	      w = (GLdouble)curve->controlv[b];
	      controls[a]   = (GLfloat)((m[0]*x + m[4]*y + m[12])*w);
	      controls[a+1] = (GLfloat)((m[1]*x + m[5]*y + m[13])*w);
	      controls[a+2] = (GLfloat)(w /*m[3]*x + m[7]*y + m[15]*w*/);
	      a += 3;
	    }
	  else
	    {
	      controls[a] = (GLfloat)(m[0]*x + m[4]*y + m[12]);
	      controls[a+1] = (GLfloat)(m[1]*x + m[5]*y + m[13]);
	      a += 2;
	    }
	}
      else
	{
	  controls[a]   = (GLfloat)x;
	  controls[a+1] = (GLfloat)y;
	  if(curve->is_rat)
	    {
	      controls[a+2] = (GLfloat)w;
	      a += 3;
	    }
	  else
	    a += 2;
	} /* if apply_trafo */
      b++;
    } /* for */

  if(ay_prefs.glu_avoid_pwlcurve || curve->order != 2)
    {
      gluNurbsCurve(no, (GLint)knot_count, knots,
		    (GLint)(curve->is_rat?3:2), controls,
		    (GLint)curve->order,
		    (curve->is_rat?GLU_MAP1_TRIM_3:GLU_MAP1_TRIM_2));
    }
  else
    {
      gluPwlCurve(no, (GLint)curve->length, controls,
		  (GLint)(curve->is_rat?3:2),
		  (curve->is_rat?GLU_MAP1_TRIM_3:GLU_MAP1_TRIM_2));
    }

cleanup:

  if(knots)
    free(knots);

  if(controls)
    free(controls);

  if(n)
    ay_object_delete(n);

 return ay_status;
} /* ay_npt_drawtrimcurve */


/** ay_npt_drawtrimcurves:
 *  draw all trimcurves with GLU
 *
 * \param[in] o NURBS patch object with trim curves
 * \param[in] refine_trims how many times are the trim curves to be refined
 *  (for increased tesselation fidelity along trim edges)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_drawtrimcurves(ay_object *o, unsigned int refine_trims)
{
 int ay_status = AY_OK;
 ay_object *trim = NULL, *loop = NULL, *p = NULL, *nc = NULL;
 ay_nurbpatch_object *npatch = (ay_nurbpatch_object *)o->refine;

  trim = o->down;
  while(trim->next)
    {
      switch(trim->type)
	{
	case AY_IDNCURVE:
	  gluBeginTrim(npatch->no);
	   (void)ay_npt_drawtrimcurve(trim, npatch->no, refine_trims);
	  gluEndTrim(npatch->no);
	  break;
	case AY_IDLEVEL:
	  /* XXXX check, whether level is of type trimloop? */
	  loop = trim->down;
	  if(loop && loop->next)
	    {
	      gluBeginTrim(npatch->no);
	      while(loop->next)
		{
		  if(loop->type == AY_IDNCURVE)
		    {
		      (void)ay_npt_drawtrimcurve(loop, npatch->no,
						 refine_trims);
		    }
		  else
		    {
		      /* loop element is a curve providing object */
		      p = NULL;
		      (void)ay_provide_object(loop, AY_IDNCURVE, &p);
		      nc = p;
		      while(nc)
			{
			  (void)ay_npt_drawtrimcurve(nc, npatch->no,
						     refine_trims);
			  nc = nc->next;
			} /* if */
		      (void)ay_object_deletemulti(p, AY_FALSE);
		    } /* if */
		  loop = loop->next;
		} /* while */
	      gluEndTrim(npatch->no);
	    } /* if */
	  break;
	default:
	  /* trim is a curve providing object */
	  p = NULL;
	  (void)ay_provide_object(trim, AY_IDNCURVE, &p);
	  nc = p;
	  while(nc)
	    {
	      gluBeginTrim(npatch->no);
	       (void)ay_npt_drawtrimcurve(nc, npatch->no, refine_trims);
	      gluEndTrim(npatch->no);
	      nc = nc->next;
	    }
	  (void)ay_object_deletemulti(p, AY_FALSE);
	  break;
	} /* switch */
      trim = trim->next;
    } /* while */

 return ay_status;
} /* ay_npt_drawtrimcurves */


/** ay_npt_drawrohandles:
 *  draw read only handles helper
 */
void
ay_npt_drawrohandles(ay_nurbpatch_object *patch)
{
 int i;
 double *pnts;

  if(patch)
    {
      pnts = patch->controlv;

      glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
		(GLfloat)ay_prefs.obb);

      glBegin(GL_POINTS);
       if(ay_prefs.rationalpoints)
	 {
	   for(i = 0; i < patch->width*patch->height; i++)
	     {
	       glVertex3d((GLdouble)pnts[0]*pnts[3],
			  (GLdouble)pnts[1]*pnts[3],
			  (GLdouble)pnts[2]*pnts[3]);
	       pnts += 4;
	     }
	 }
       else
	 {
	   for(i = 0; i < patch->width*patch->height; i++)
	     {
	       glVertex3dv((GLdouble *)pnts);
	       pnts += 4;
	     }
	 }
      glEnd();

      glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
		(GLfloat)ay_prefs.seb);
    }

 return;
} /* ay_npt_drawrohandles */


/* ay_npt_crtcobbsphere:
 *  create a single patch (out of 6) of a NURBS Cobb Sphere
 *  controls taken from:
 *  "NURB Curves and Surfaces, from Projective Geometry to Practical Use",
 *  by G. Farin
 */
int
ay_npt_crtcobbsphere(ay_nurbpatch_object **cobbsphere)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *new = NULL;
 double *controls = NULL;
 double t = 0.0, d = 0.0;
 int i, a;

  if(!(controls = malloc(100 * sizeof(double))))
    return AY_EOMEM;

  t = sqrt(3.0);
  d = sqrt(2.0);

  controls[0] = 4.0*(1.0-t);
  controls[1] = 4.0*(1.0-t);
  controls[2] = 4.0*(1.0-t);
  controls[3] = 4.0*(3.0-t);

  controls[4] = -d;
  controls[5] = d * (t - 4.0);
  controls[6] = d * (t - 4.0);
  controls[7] = d * (3.0*t - 2.0);

  controls[8] = 0.0;
  controls[9] = 4.0*(1.0-2.0*t)/3.0;
  controls[10] = 4.0*(1.0-2.0*t)/3.0;
  controls[11] = 4.0*(5.0-t)/3.0;

  controls[12] = d;
  controls[13] = d * (t - 4.0);
  controls[14] = d * (t - 4.0);
  controls[15] = d * (3*t - 2.0);

  controls[16] = 4.0*(t - 1.0);
  controls[17] = 4.0*(1.0-t);
  controls[18] = 4.0*(1.0-t);
  controls[19] = 4.0*(3.0-t);

  controls[20] = d*(t - 4.0);
  controls[21] = -d;
  controls[22] = d*(t-4.0);
  controls[23] = d*(3.0*t-2.0);

  controls[24] = (2.0-3.0*t)/2.0;
  controls[25] = (2.0-3.0*t)/2.0;
  controls[26] = -(t+6.0)/2.0;
  controls[27] = (t+6.0)/2.0;

  controls[28] = 0.0;
  controls[29] = d*(2.0*t-7.0)/3.0;
  controls[30] = -5.0*sqrt(6.0)/3.0;
  controls[31] = d*(t+6.0)/3.0;

  controls[32] = (3.0*t-2.0)/2.0;
  controls[33] = (2.0-3.0*t)/2.0;
  controls[34] = -(t+6.0)/2.0;
  controls[35] = (t+6.0)/2.0;

  controls[36] = d*(4.0-t);
  controls[37] = -d;
  controls[38] = d*(t-4.0);
  controls[39] = d*(3.0*t-2.0);

  controls[40] = 4.0*(1.0-2*t)/3.0;
  controls[41] = 0.0;
  controls[42] = 4.0*(1.0-2*t)/3.0;
  controls[43] = 4.0*(5.0-t)/3.0;

  controls[44] = d*(2.0*t-7.0)/3.0;
  controls[45] = 0.0;
  controls[46] = -5.0*sqrt(6.0)/3.0;
  controls[47] = d*(t+6.0)/3.0;

  controls[48] = 0.0;
  controls[49] = 0.0;
  controls[50] = 4.0*(t-5.0)/3.0;
  controls[51] = 4.0*(5.0*t-1.0)/9.0;

  controls[52] = -d*(2*t-7.0)/3.0;
  controls[53] = 0.0;
  controls[54] = -5.0*sqrt(6.0)/3.0;
  controls[55] = d*(t+6.0)/3.0;

  controls[56] = -4.0*(1.0-2.0*t)/3.0;
  controls[57] = 0.0;
  controls[58] = 4.0*(1.0-2.0*t)/3.0;
  controls[59] = 4.0*(5.0-t)/3.0;

  controls[60] = d*(t-4.0);
  controls[61] = d;
  controls[62] = d*(t-4.0);
  controls[63] = d*(3.0*t-2);

  controls[64] = (2.0-3.0*t)/2.0;
  controls[65] = -(2.0-3.0*t)/2.0;
  controls[66] = -(t+6.0)/2.0;
  controls[67] = (t+6.0)/2.0;

  controls[68] = 0.0;
  controls[69] = -d*(2.0*t-7.0)/3.0;
  controls[70] = -5.0*sqrt(6.0)/3.0;
  controls[71] = d*(t+6.0)/3.0;

  controls[72] = (3.0*t-2.0)/2.0;
  controls[73] = -(2.0-3.0*t)/2.0;
  controls[74] = -(t+6.0)/2.0;
  controls[75] = (t+6.0)/2.0;

  controls[76] = d*(4.0-t);
  controls[77] = d;
  controls[78] = d*(t-4.0);
  controls[79] = d*(3.0*t-2.0);

  controls[80] = 4.0*(1.0-t);
  controls[81] = -4.0*(1.0-t);
  controls[82] = 4.0*(1.0-t);
  controls[83] = 4.0*(3.0-t);

  controls[84] = -d;
  controls[85] = -d*(t-4.0);
  controls[86] = d*(t-4.0);
  controls[87] = d*(3.0*t-2.0);

  controls[88] = 0.0;
  controls[89] = -4.0*(1.0-2.0*t)/3.0;
  controls[90] = 4.0*(1.0-2.0*t)/3.0;
  controls[91] = 4.0*(5.0-t)/3.0;

  controls[92] = d;
  controls[93] = -d*(t-4.0);
  controls[94] = d*(t-4.0);
  controls[95] = d*(3.0*t-2.0);

  controls[96] = 4.0*(t-1.0);
  controls[97] = -4.0*(1.0-t);
  controls[98] = 4.0*(1.0-t);
  controls[99] = 4.0*(3.0-t);

  a = 0;
  for(i = 0; i < 25; i++)
    {
      controls[a]   /= controls[a+3];
      controls[a+1] /= controls[a+3];
      controls[a+2] /= controls[a+3];
      a += 4;
    }

  ay_status = ay_npt_create(5, 5, 5, 5, AY_KTNURB, AY_KTNURB,
			    controls, NULL, NULL, &new);

  if(ay_status)
    {
      free(controls);
      return ay_status;
    }

  *cobbsphere = new;

 return ay_status;
} /* ay_npt_crtcobbsphere */


/* ay_npt_crtnsphere:
 *  create a simple NURBS sphere by revolving a half circle
 */
int
ay_npt_crtnsphere(double radius, ay_nurbpatch_object **nsphere)
{
 int ay_status = AY_OK;
 char fname[] = "crtnsphere";
 ay_object *newc = NULL;

  if(!nsphere)
    return AY_ENULL;

  if(!(newc = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_EOMEM;
    }

  newc->type = AY_IDNCURVE;
  ay_object_defaults(newc);

  /* first, we create a half circle in the XY plane */
  ay_status = ay_nct_crtnhcircle(radius,
			    (ay_nurbcurve_object **)(void*)(&(newc->refine)));
  if(ay_status)
    {
      free(newc);
      return ay_status;
    }

  /* second, we revolve the half circle around the Y axis */
  ay_status = ay_npt_revolve(newc, 360.0, 0, 0, nsphere);

  (void)ay_object_delete(newc);

 return ay_status;
} /* ay_npt_crtnsphere */


/** ay_npt_crtnspheretcmd:
 *  Create a simple NURBS sphere by revolving a half circle.
 *  Implements the \a crtNSphere scripting interface command.
 *  See also the corresponding section in the \ayd{sccrtnsphere}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_crtnspheretcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 int ay_status, tcl_status;
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
	      tcl_status = Tcl_GetDouble(interp, argv[i+1], &radius);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);

	      if(radius <= AY_EPSILON)
		{
		  ay_error(AY_ERROR, argv[0], "Parameter radius must be > 0.");
		  return TCL_OK;
		}
	    }
	  i += 2;
	} /* while */
    } /* if */

  ay_status = ay_npt_createnpatchobject(&o);

  if(ay_status || !o)
    {
      ay_error(ay_status, argv[0], NULL);
      return TCL_OK;
    }

  ay_object_placemark(o);

  ay_status = ay_npt_crtnsphere(radius,
				(ay_nurbpatch_object **)(void*)&(o->refine));
  if(ay_status)
    {
      (void)ay_object_delete(o);
      ay_error(ay_status, argv[0], NULL);
      return TCL_OK;
    }

  ay_object_link(o);

 return TCL_OK;
} /* ay_npt_crtnspheretcmd */


/** ay_npt_crtnsphere2tcmd:
 *  Create a "Cobb" NURBS sphere.
 *  Implements the \a crtNSphere2 scripting interface command.
 *  See also the corresponding section in the \ayd{sccrtnsphere2}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_crtnsphere2tcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{
 int ay_status;
 int i;
 double rot[18] = {
   0.0, 0.0, 0.0,
   0.0, 90.0, 0.0,
   0.0, 180.0, 0.0,
   0.0, -90.0, 0.0,
   90.0, 0.0, 0.0,
   -90.0, 0.0, 0.0
 };
 double xaxis[3]={1.0,0.0,0.0};
 double yaxis[3]={0.0,1.0,0.0};
 double zaxis[3]={0.0,0.0,1.0};
 double quat[4];
 ay_object *new = NULL;

  for(i = 0; i < 6; i++)
    {
      new = NULL;
      ay_status = ay_npt_createnpatchobject(&new);

      if(ay_status || !new)
	{
	  ay_error(ay_status, argv[0], NULL);
	  return TCL_OK;
	}

      ay_object_placemark(new);

      ay_status = ay_npt_crtcobbsphere(
			(ay_nurbpatch_object **)(void*)&(new->refine));
      if(ay_status)
	{
	  (void)ay_object_delete(new);
	  ay_error(AY_ERROR, argv[0], NULL);
	  return TCL_OK;
	}

      new->rotx = rot[i*3];
      if(new->rotx != 0.0)
	{
	  ay_quat_axistoquat(xaxis, AY_D2R(rot[i*3]), quat);
	  ay_quat_add(quat, new->quat, new->quat);
	}
      new->roty = rot[(i*3)+1];
      if(new->roty != 0.0)
	{
	  ay_quat_axistoquat(yaxis, AY_D2R(rot[i*3+1]), quat);
	  ay_quat_add(quat, new->quat, new->quat);
	}
      new->rotz = rot[(i*3)+2];
      if(new->rotz != 0.0)
	{
	  ay_quat_axistoquat(zaxis, AY_D2R(rot[i*3+2]), quat);
	  ay_quat_add(quat, new->quat, new->quat);
	}

      ay_object_link(new);
    } /* for */

 return TCL_OK;
} /* ay_npt_crtnsphere2tcmd */


/** ay_npt_breakintocurvesu:
 *  break NURBS patch object \a o into curves along u
 *
 * \param[in] o NURBS patch object to process
 * \param[in] apply_trafo if AY_TRUE the transformation attributes
 * of \a o will be applied to the control points prior to breaking
 * \param[in,out] curves where to store the resulting curve objects
 * \param[in,out] last the last curve object, may be NULL
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_breakintocurvesu(ay_object *o, int apply_trafo,
			ay_object **curves, ay_object ***last)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *patch = NULL;
 ay_object *new = NULL, **next = NULL;
 double *knotv = NULL, *controlv = NULL;
 double m[16];
 int i, j, k, stride, dstlen, knots;

  if(!o || !curves)
    return AY_ENULL;

  if(o->type != AY_IDNPATCH)
    return AY_ERROR;

  patch = (ay_nurbpatch_object *)o->refine;
  dstlen = patch->height;
  knots = dstlen + patch->vorder;
  stride = 4;

  if(apply_trafo && AY_ISTRAFO(o))
    {
      /* get patch transformation-matrix */
      ay_trafo_creatematrix(o, m);
    }
  else
    {
      apply_trafo = AY_FALSE;
    }

  for(i = 0; i < patch->width; i++)
    {
      if(!(new = calloc(1, sizeof(ay_object))))
	{
	  return AY_EOMEM;
	}

      new->type = AY_IDNCURVE;

      if(!(knotv = malloc(knots * sizeof(double))))
	{
	  free(new);
	  return AY_EOMEM;
	}
      memcpy(knotv, patch->vknotv, (size_t)(knots*sizeof(double)));

      if(!(controlv = malloc(stride*dstlen*sizeof(double))))
	{
	  free(new); free(knotv);
	  return AY_EOMEM;
	}
      memcpy(controlv, &(patch->controlv[i*dstlen*stride]),
	     (size_t)(stride*dstlen*sizeof(double)));

      if(apply_trafo)
	{
	  /* apply transformation-matrix */
	  j = 0;
	  for(k = 0; k < dstlen; k++)
	    {
	      ay_trafo_apply3(&(controlv[j]), m);
	      j += stride;
	    } /* for */
	}

      ay_status = ay_nct_create(patch->vorder, dstlen, patch->vknot_type,
				controlv, knotv,
				(ay_nurbcurve_object **)(void*)&(new->refine));
      if(ay_status)
	{
	  free(new); free(knotv); free(controlv);
	  return ay_status;
	}

      ay_object_defaults(new);

      /* link result */
      if(next)
	{
	  *next = new;
	}
      else
	{
	  *curves = new;
	}
      next = &(new->next);
    } /* for */

  if(last)
    {
      *last = next;
    }

 return ay_status;
} /* ay_npt_breakintocurvesu */


/** ay_npt_breakintocurvesv:
 *  break NURBS patch object \a o into curves along v
 *
 * \param[in] o NURBS patch object to process
 * \param[in] apply_trafo if AY_TRUE the transformation attributes
 * of \a o will be applied to the control points prior to breaking
 * \param[in,out] curves where to store the resulting curve objects
 * \param[in,out] last the last curve object, may be NULL
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_breakintocurvesv(ay_object *o, int apply_trafo,
			ay_object **curves, ay_object ***last)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *patch = NULL;
 ay_object *new = NULL, **next = NULL;
 double *knotv = NULL, *controlv = NULL;
 double m[16];
 int i, j, k, a, stride, dstlen, knots;

  if(!o || !curves)
    return AY_ENULL;

  if(o->type != AY_IDNPATCH)
    return AY_ERROR;

  patch = (ay_nurbpatch_object *)o->refine;
  dstlen = patch->width;
  knots = dstlen + patch->uorder;
  stride = 4;

  if(apply_trafo && AY_ISTRAFO(o))
    {
      /* get patch transformation-matrix */
      ay_trafo_creatematrix(o, m);
    }
  else
    {
      apply_trafo = AY_FALSE;
    }

  for(i = 0; i < patch->height; i++)
    {
      if(!(new = calloc(1, sizeof(ay_object))))
	{
	  return AY_EOMEM;
	}

      new->type = AY_IDNCURVE;

      if(!(knotv = malloc(knots * sizeof(double))))
	{
	  free(new);
	  return AY_EOMEM;
	}
      memcpy(knotv, patch->uknotv, (size_t)(knots*sizeof(double)));

      if(!(controlv = malloc(stride*dstlen*sizeof(double))))
	{
	  free(new); free(knotv);
	  return AY_EOMEM;
	}
      a = i*stride;
      for(j = 0; j < dstlen; j++)
	{
	  memcpy(&(controlv[j*stride]), &(patch->controlv[a]),
		 (size_t)(stride*sizeof(double)));
	  a += patch->height*stride;
	}

      if(apply_trafo)
	{
	  /* apply transformation-matrix */
	  j = 0;
	  for(k = 0; k < dstlen; k++)
	    {
	      ay_trafo_apply3(&(controlv[j]), m);
	      j += stride;
	    } /* for */
	}

      ay_status = ay_nct_create(patch->uorder, dstlen, patch->uknot_type,
				controlv, knotv,
				(ay_nurbcurve_object **)(void*)&(new->refine));
      if(ay_status)
	{
	  free(new); free(knotv); free(controlv);
	  return ay_status;
	}

      ay_object_defaults(new);

      /* link result */
      if(next)
	{
	  *next = new;
	}
      else
	{
	  *curves = new;
	}
      next = &(new->next);
    } /* for */

  if(last)
    {
      *last = next;
    }

 return ay_status;
} /* ay_npt_breakintocurvesv */


/** ay_npt_breakintocurvestcmd:
 *  Break the selected NURBS surfaces into curves.
 *  Tcl interface for breakintocurvesu() and breakintocurvesv() above.
 *
 *  Implements the \a breakNP scripting interface command.
 *  See also the corresponding section in the \ayd{scbreaknp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_breakintocurvestcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL, *curves = NULL, *next = NULL;
 int i = 1, apply_trafo = AY_FALSE, u = 0;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(argc > 1)
    {
      /* parse args */
      while(i < argc)
	{
	  if(argv[i] && argv[i][0] == '-')
	    {
	      switch(argv[i][1])
		{
		case 'a':
		  apply_trafo = AY_TRUE;
		  break;
		case 'u':
		  u = 1;
		  break;
		case 'v':
		  u = 0;
		  break;
		default:
		  break;
		}
	    } /* if */
	  i++;
	} /* while */
    } /* if */

  while(sel)
    {
      curves = NULL;
      src = sel->object;

      if(src->type != AY_IDNPATCH)
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	}
      else
	{
	  if(u)
	    {
	      ay_status = ay_npt_breakintocurvesu(src, apply_trafo,
						  &curves, NULL);
	    }
	  else
	    {
	      ay_status = ay_npt_breakintocurvesv(src, apply_trafo,
						  &curves, NULL);
	    } /* if */

	  if(ay_status || !curves)
	    {
	      ay_error(AY_ERROR, argv[0], "Break into curves failed.");
	      break;
	    }

	  while(curves)
	    {
	      next = curves->next;
	      if(!apply_trafo)
		ay_trafo_copy(src, curves);
	      ay_object_link(curves);
	      curves = next;
	    }
	} /* if */

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_npt_breakintocurvestcmd */


/** ay_npt_buildfromcurves:
 *  build a new patch from a number of (compatible) curves
 *
 * \param[in] curves a list of compatible NURBS curve objects;
 *  atleast two curves are required
 * \param[in] ncurves number of curves in \a curves, also the
 *  width of the new patch (if \a type is open)
 * \param[in] type desired surface type (AY_CTOPEN, AY_CTCLOSED,
 *  or AY_CTPERIODIC);
 *  if the type is closed or periodic the first n curves will be copied
 *  multiple times and the width of the new patch will be adapted
 *  accordingly
 * \param[in] order desired order of new patch in U; may be 0, in
 *  which case a default value of 4 will be used;
 *  will also be silently corrected to width, if the width is smaller than
 *  the provided/chosen order
 * \param[in] knot_type desired knot type of new patch in U;
 *  if AY_KTCUSTOM is provided, the resulting patch will not have knots
 *  for U and the caller must correct this
 * \param[in] apply_trafo if AY_TRUE, the transformation attributes
 *  of the curves will be applied to the respective control points;
 *  otherwise the transformation atributes will be ignored and the
 *  control points will be copied verbatim
 * \param[in,out] patch new patch
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_buildfromcurves(ay_list_object *curves, int ncurves, int type,
		       int order, int knot_type, int apply_trafo,
		       ay_object **patch)
{
 int ay_status = AY_OK;
 ay_list_object *curve;
 ay_object *c = NULL, *new = NULL;
 ay_nurbcurve_object *nc = NULL;
 double *newvknotv = NULL, *newcontrolv = NULL;
 int newwidth = 0, newheight = 0;
 int newvorder = 0, newvknots = 0, newuorder = 0;
 int newuknot_type = AY_KTNURB, newvknot_type = 0;
 int i = 0, j = 0, stride = 4;
 double m[16];

  if(!curves || !patch)
    {
      return AY_ENULL;
    }

  if(ncurves < 2)
    {
      return AY_ERROR;
    }

  /* parse curves */
  curve = curves;
  c = curve->object;
  nc = (ay_nurbcurve_object*)c->refine;

  /* calculate patch parameters */
  newwidth = ncurves;

  if(order == 0)
    {
      newuorder = 4;
    }
  else
    {
      newuorder = order;
    }

  if(type == AY_CTCLOSED)
    newwidth++;

  if(type == AY_CTPERIODIC)
    newwidth += newuorder-1;

  if(newuorder > newwidth)
    newuorder = newwidth;

  newuknot_type = knot_type;

  newheight = nc->length;
  newvorder = nc->order;
  newvknots = newheight + newvorder;
  newvknot_type = nc->knot_type;
  if(nc->knot_type == AY_KTCUSTOM)
    {
      if(!(newvknotv = malloc(newvknots * sizeof(double))))
	{
	  return AY_EOMEM;
	}

      memcpy(newvknotv, nc->knotv, (size_t)(newvknots*sizeof(double)));
    }

  /* create new patch */
  ay_status = ay_npt_createnpatchobject(&new);
  if(ay_status || !new)
    {
      if(newvknotv)
	free(newvknotv);
      return ay_status;
    }

  if(!(newcontrolv = malloc(newwidth*newheight*4*sizeof(double))))
    {
      if(newvknotv)
	free(newvknotv);
      free(new);
      return AY_EOMEM;
    }

  /* fill newcontrolv */
  curve = curves;
  while(curve)
    {
      c = curve->object;
      if(c->type == AY_IDNCURVE)
	{
	  nc = (ay_nurbcurve_object*)c->refine;

	  memcpy(&(newcontrolv[i]), nc->controlv,
		 newheight*stride*sizeof(double));

	  if(apply_trafo && AY_ISTRAFO(c))
	    {
	      /* get curves transformation-matrix */
	      ay_trafo_creatematrix(c, m);
	      /* apply curves transformation-matrix */
	      for(j = 0; j < newheight; j++)
		{
		  ay_trafo_apply3(&(newcontrolv[i]), m);
		  i += stride;
		} /* for */
	    }
	  else
	    {
	      i += newheight*stride;
	    }
	} /* if */

      curve = curve->next;
    } /* while */

  /* close patch? */
  if(type == AY_CTCLOSED)
    {
      memcpy(&(newcontrolv[i]), newcontrolv,
	     newheight*stride*sizeof(double));
    }

  if(type == AY_CTPERIODIC)
    {
      for(j = 0; j < (newuorder-1); j++)
	{
	  memcpy(&(newcontrolv[i]), &(newcontrolv[j*newheight*stride]),
	    newheight*stride*sizeof(double));
	  i += newheight*stride;
	}
    }

  /* create patch object */
  ay_status = ay_npt_create(newuorder, newvorder,
			    newwidth, newheight,
			    newuknot_type, newvknot_type,
			    newcontrolv, NULL, newvknotv,
			    (ay_nurbpatch_object **)(void*)&(new->refine));

  if(ay_status)
    {
      free(new);
      if(newvknotv)
	free(newvknotv);
      free(newcontrolv);
      return ay_status;
    }

  ay_npt_recreatemp((ay_nurbpatch_object *)new->refine);

  /* return result */
  *patch = new;

 return ay_status;
} /* ay_npt_buildfromcurves */


/** ay_npt_buildfromcurvestcmd:
 *  Build a NURBS surface from all selected (compatible) NURBS curves.
 *  Tcl interface for ay_npt_buildfromcurves() above
 *
 *  Implements the \a buildNP scripting interface command.
 *  See also the corresponding section in the \ayd{scbuildnp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_buildfromcurvestcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[])
{
 int ay_status = AY_OK, tcl_status = TCL_OK;
 ay_list_object *sel = NULL, *curves = NULL, *new = NULL, **next = NULL;
 ay_object *o = NULL, *patch = NULL;
 ay_nurbcurve_object *nc = NULL;
 int i = 1, length = 0, ncurves = 0;
 int order = 0, knots = AY_KTNURB, type = AY_CTOPEN;
 int apply_trafo = AY_TRUE;

  sel = ay_selection;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  /* parse args */
  if(argc > 1)
    {
      while(i < argc)
	{
	  if(argv[i] && argv[i][0] == '-')
	    {
	      switch(argv[i][1])
		{
		case 'a':
		  tcl_status = Tcl_GetInt(interp, argv[i+1], &apply_trafo);
		  AY_CHTCLERRRET(tcl_status, argv[0], interp);
		  break;
		case 'o':
		  tcl_status = Tcl_GetInt(interp, argv[i+1], &order);
		  AY_CHTCLERRRET(tcl_status, argv[0], interp);
		  break;
		case 'k':
		  tcl_status = Tcl_GetInt(interp, argv[i+1], &knots);
		  AY_CHTCLERRRET(tcl_status, argv[0], interp);
		  break;
		case 't':
		  tcl_status = Tcl_GetInt(interp, argv[i+1], &type);
		  AY_CHTCLERRRET(tcl_status, argv[0], interp);
		  break;
		default:
		  break;
		} /* switch */
	    } /* if */
	  i += 2;
	} /* while */
    } /* if */

  /* parse selection */
  while(sel)
    {
      o = sel->object;
      if(o->type == AY_IDNCURVE)
	{
	  nc = (ay_nurbcurve_object*)o->refine;

	  if(length == 0)
	    {
	      /* get length from first curve */
	      length = nc->length;
	    }

	  if(nc->length >= length)
	    {
	      if(!(new = calloc(1, sizeof(ay_list_object))))
		{
		  ay_error(AY_EOMEM, argv[0], NULL);
		  goto cleanup;
		}

	      new->object = o;

	      if(next)
		*next = new;
	      else
		curves = new;

	      next = &(new->next);

	      ncurves++;
	    } /* if */
	} /* if */
      sel = sel->next;
    } /* while */

  ay_status = ay_npt_buildfromcurves(curves, ncurves, type, order,
				     knots, apply_trafo, &patch);

  if(ay_status || !patch)
    {
      ay_error(AY_ERROR, argv[0], "Build from curves failed.");
    }
  else
    {
      ay_object_link(patch);
    }

cleanup:

  /* free list */
  while(curves)
    {
      new = curves->next;
      free(curves);
      curves = new;
    }

 return TCL_OK;
} /* ay_npt_buildfromcurvestcmd */


/** ay_npt_setback:
 *  set back the borders of the connected patches \a o1 and \a o2
 *
 * \param[in,out] o1 first NURBS patch object
 * \param[in,out] o2 second NURBS patch object
 * \param[in] tanlen if != 0.0, scale of tangents, expressed as ratio
 *  of the distance between the respective border points in o1 and o2
 * \param[in] uv specification on which sides the patches are connected
 *  may be NULL, in which case the patches are connected via un(o1)-u0(o2)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_setback(ay_object *o1, ay_object *o2,
	       double tanlen, char *uv)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *np1, *np2;
 double *cv11, *cv12;
 double *cv21, *cv22;
 double dist, v1[3], v2[3];
 int i, l1, l2, stride = 4, stride1 = 4, stride2 = 4;

  if(!o1 || !o2)
    return AY_ENULL;

  if((o1->type != AY_IDNPATCH) || (o2->type != AY_IDNPATCH))
    return AY_ERROR;

  np1 = (ay_nurbpatch_object *)o1->refine;
  np2 = (ay_nurbpatch_object *)o2->refine;

  l1 = np1->height;
  l2 = np2->height;

  cv11 = &(np1->controlv[(np1->width*np1->height*stride)]);
  cv11 -= np1->height*stride;
  cv12 = cv11 - np1->height*stride;

  cv21 = np2->controlv;
  cv22 = cv21 + np1->height*stride;

  if(uv)
    {
      switch(uv[0])
	{
	case 'U':
	  cv11 = np1->controlv;
	  cv12 = cv11+(np1->height*stride);
	  break;
	case 'v':
	  l1 = np1->width;
	  cv11 = np1->controlv;
	  cv12 = cv11+stride;
	  stride1 = np1->height;
	  break;
	case 'V':
	  l1 = np1->width;
	  cv11 = np1->controlv;
	  cv11 += (np1->height-1)*stride;
	  cv12 = cv11-stride;
	  stride1 = np1->height;
	  break;
	default:
	  break;
	}

      if(uv[0] != '\0')
	{
	  switch(uv[1])
	    {
	    case 'U':
	      cv21 = &(np2->controlv[(np2->width*np2->height*stride)]);
	      cv21 -= np2->height*stride;
	      cv22 = cv21 - np2->height*stride;
	      break;
	    case 'v':
	      l2 = np2->width;
	      cv21 = np2->controlv;
	      cv22 = cv21+stride;
	      stride2 = np2->height;
	      break;
	    case 'V':
	      l2 = np2->width;
	      cv21 = np2->controlv;
	      cv21 += (np2->height-1)*stride;
	      cv22 = cv21-stride;
	      stride2 = np2->height;
	      break;
	    default:
	      break;
	    }
	} /* if */
    } /* if */

  if(l1 != l2)
    return AY_ERROR;

  if(AY_ISTRAFO(o1))
    ay_npt_applytrafo(o1);

  if(AY_ISTRAFO(o2))
    ay_npt_applytrafo(o2);

  /* set back the respective border control points */
  for(i = 0; i < l1; i++)
    {
      v1[0] = (cv11[0]-cv12[0]);
      v1[1] = (cv11[1]-cv12[1]);
      v1[2] = (cv11[2]-cv12[2]);

      dist = AY_V3LEN(v1);
      if(dist > AY_EPSILON)
	{
	  AY_V3SCAL(v1, 1.0/dist);

	  if(tanlen > 0)
	    {
	      AY_V3SCAL(v1, tanlen);
	    }
	  else
	    {
	      AY_V3SCAL(v1, fabs(tanlen)*dist);
	    }

	  cv11[0] -= v1[0];
	  cv11[1] -= v1[1];
	  cv11[2] -= v1[2];
	}
      cv11 += stride1;
      cv12 += stride1;
    } /* for */

  for(i = 0; i < l2; i++)
    {
      v2[0] = (cv21[0]-cv22[0]);
      v2[1] = (cv21[1]-cv22[1]);
      v2[2] = (cv21[2]-cv22[2]);

      dist = AY_V3LEN(v2);
      if(dist > AY_EPSILON)
	{
	  AY_V3SCAL(v2, 1.0/dist);

	  if(tanlen > 0)
	    {
	      AY_V3SCAL(v2, tanlen);
	    }
	  else
	    {
	      AY_V3SCAL(v2, fabs(tanlen)*dist);
	    }

	  cv21[0] -= v2[0];
	  cv21[1] -= v2[1];
	  cv21[2] -= v2[2];
	}
      cv21 += stride2;
      cv22 += stride2;
    } /* for */

 return ay_status;
} /* ay_npt_setback */


/** ay_npt_fillgap:
 *  fill the gap between the patches \a o1 and \a o2
 *  with another NURBS patch (fillet); no fillet will be created
 *  if the respective border curves match
 *
 * \param[in] o1 first NURBS patch object
 * \param[in] o2 second NURBS patch object
 * \param[in] tanlen if != 0.0, scale of tangents, expressed as ratio
 *  of the distance between the respective border points in o1 and o2
 * \param[in] uv specification of which sides of the patches to connect
 *  may be NULL, in which case the patches are connected via un(o1)-u0(o2)
 * \param[in,out] result fillet patch (unless the respective border
 *  curves match)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_fillgap(ay_object *o1, ay_object *o2,
	       double tanlen, char *uv, ay_object **result)
{
 int ay_status = AY_OK;
 ay_object *of1 = NULL, *of2 = NULL;
 ay_nurbpatch_object *np1, *np2, *new = NULL;
 double *cv1, *cv2, *ncv;
 double dist, v1[3], v2[3];
 int i, a, b, c, d, stride = 4, is_eq;

  if(!o1 || !o2 || !result)
    return AY_ENULL;

  if((o1->type != AY_IDNPATCH) || (o2->type != AY_IDNPATCH))
    return AY_ERROR;

  np1 = (ay_nurbpatch_object *)o1->refine;
  np2 = (ay_nurbpatch_object *)o2->refine;

  if(AY_ISTRAFO(o1))
    ay_npt_applytrafo(o1);

  if(AY_ISTRAFO(o2))
    ay_npt_applytrafo(o2);

  if(uv)
    {
      if(uv[0] == 'U')
	{
	  ay_status = ay_object_copy(o1, &of1);
	  if(ay_status || !of1)
	    goto cleanup;
	  np1 = (ay_nurbpatch_object *)of1->refine;
	  ay_status = ay_npt_revertu(np1);
	  if(ay_status)
	    goto cleanup;
	}
      else
      if(uv[0] == 'v' || uv[0] == 'V')
	{
	  ay_status = ay_object_copy(o1, &of1);
	  if(ay_status || !of1)
	    goto cleanup;
	  np1 = (ay_nurbpatch_object *)of1->refine;

	  if(uv[0] == 'V')
	    {
	      ay_status = ay_npt_revertv(np1);
	      if(ay_status)
		goto cleanup;
	    }
	  ay_status = ay_npt_swapuv(np1);
	  if(ay_status)
	    goto cleanup;
	}
      if(uv[0] != '\0' && uv[1] == 'U')
	{
	  ay_status = ay_object_copy(o2, &of2);
	  if(ay_status || !of2)
	    goto cleanup;
	  np2 = (ay_nurbpatch_object *)of2->refine;
	  ay_status = ay_npt_revertu(np2);
	  if(ay_status)
	    goto cleanup;
	}
      else
      if(uv[0] != '\0' && (uv[1] == 'v' || uv[1] == 'V'))
	{
	  ay_object_copy(o2, &of2);
	  if(ay_status || !of2)
	    goto cleanup;
	  np2 = (ay_nurbpatch_object *)of2->refine;

	  if(uv[1] == 'V')
	    {
	      ay_status = ay_npt_revertv(np2);
	      if(ay_status)
		goto cleanup;
	    }
	  ay_status = ay_npt_swapuv(np2);
	  if(ay_status)
	    goto cleanup;
	}
    } /* if */

  if(np1->height != np2->height)
    return AY_ERROR;

  /* check for matching borders */
  cv1 = &(np1->controlv[(np1->width - 1) * np1->height * stride]);
  cv2 = np2->controlv;
  is_eq = AY_TRUE;
  for(i = 0; i < np1->height; i++)
    {
      if(!AY_V4COMP(cv1,cv2))
	{
	  is_eq = AY_FALSE;
	  break;
	}
      cv1 += stride;
      cv2 += stride;
    }

  if(is_eq)
    return AY_OK;

  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    { ay_status = AY_EOMEM; goto cleanup; }

  new->uorder = 4;
  new->uknot_type = AY_KTNURB;
  new->width = 4;

  if(!(new->vknotv = malloc((np1->height + np1->vorder) * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  memcpy(new->vknotv, np1->vknotv, (np1->height + np1->vorder) *
	 sizeof(double));
  new->vorder = np1->vorder;
  new->height = np1->height;

  /* temporarily set the v knot type to custom so that createnp()
     does not create a new knot vector for us (we copied it already) */
  new->vknot_type = AY_KTCUSTOM;
  ay_status = ay_knots_createnp(new);
  if(ay_status)
    goto cleanup;
  new->vknot_type = np1->vknot_type;

  if(!(new->controlv = malloc(new->width * new->height * stride *
			      sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  ncv = new->controlv;

  /* calculate the fillet control points */
  cv1 = np1->controlv;
  cv2 = np2->controlv;
  memcpy(ncv, &(cv1[(np1->width - 1) * np1->height * stride]),
	 np1->height * stride * sizeof(double));
  if(new->width > 2)
    {
      a = np1->height * stride;
      b = (np1->width-2) * np1->height * stride;
      c = (np1->width-1) * np1->height * stride;
      d = 0;
      for(i = 0; i < np1->height; i++)
	{
	  v1[0] = (cv1[c]   - cv1[b]);
	  v1[1] = (cv1[c+1] - cv1[b+1]);
	  v1[2] = (cv1[c+2] - cv1[b+2]);

	  dist = AY_V3LEN(v1);
	  if(dist > AY_EPSILON)
	    {
	      AY_V3SCAL(v1, 1.0/dist);

	      if(tanlen > 0)
		{
		  AY_V3SCAL(v1, tanlen);
		}
	      else
		{
		  /* negative tanlen
		     => factor in patch distance */
		  v2[0] = (cv1[c]   - cv2[d]);
		  v2[1] = (cv1[c+1] - cv2[d+1]);
		  v2[2] = (cv1[c+2] - cv2[d+2]);

		  dist = AY_V3LEN(v2);
		  AY_V3SCAL(v1, (dist*-tanlen));
		  d += stride;
		} /* if */
	    } /* if */

	  ncv[a]   = cv1[c]   + v1[0];
	  ncv[a+1] = cv1[c+1] + v1[1];
	  ncv[a+2] = cv1[c+2] + v1[2];
	  ncv[a+3] = cv1[c+3] + (cv1[c+3] - cv1[b+3]) / 2.0;
	  a += stride;
	  b += stride;
	  c += stride;
	} /* for */

      b = np1->height * stride;
      c = 0;
      d = (np1->width-1) * np1->height * stride;
      for(i = 0; i < np1->height; i++)
	{
	  v1[0] = (cv2[c]   - cv2[b]);
	  v1[1] = (cv2[c+1] - cv2[b+1]);
	  v1[2] = (cv2[c+2] - cv2[b+2]);

	  dist = AY_V3LEN(v1);
	  if(dist > AY_EPSILON)
	    {
	      AY_V3SCAL(v1, 1.0/dist);

	      if(tanlen > 0)
		{
		  AY_V3SCAL(v1, tanlen);
		}
	      else
		{
		  /* negative tanlen
		     => factor in patch distance */
		  v2[0] = (cv2[c]   - cv1[d]);
		  v2[1] = (cv2[c+1] - cv1[d+1]);
		  v2[2] = (cv2[c+2] - cv1[d+2]);

		  dist = AY_V3LEN(v2);

		  AY_V3SCAL(v1, (dist * -tanlen));
		  d += stride;
		} /* if */
	    } /* if */

	  ncv[a]   = cv2[c]   + v1[0];
	  ncv[a+1] = cv2[c+1] + v1[1];
	  ncv[a+2] = cv2[c+2] + v1[2];
	  ncv[a+3] = cv2[c+3] + (cv2[c+3] - cv2[b+3]) / 2.0;
	  a += stride;
	  b += stride;
	  c += stride;
	} /* for */
    } /* if */

  memcpy(&(ncv[(new->width-1) * new->height * stride]), cv2,
	 np1->height * stride * sizeof(double));

  /* return result */
  ay_status = ay_npt_createnpatchobject(result);
  if(ay_status || !*result)
    goto cleanup;

  (*result)->refine = new;

  /* prevent cleanup code from doing something harmful */
  new = NULL;

cleanup:

  if(of1)
    (void)ay_object_delete(of1);

  if(of2)
    (void)ay_object_delete(of2);

  if(new)
    {
      ay_npt_destroy(new);
    }

 return ay_status;
} /* ay_npt_fillgap */


/** ay_npt_fillgaps:
 *  fill all the gaps between the patches in \a o
 *  with another NURBS patch; the fillets will be inserted right into
 *  the list of objects provided, the fillets will be marked as selected
 *  so that the caller can tell them apart from the original patches
 *
 * \param[in,out] o a number of NURBS patch objects (NURBS curves can
 *  be mixed in), the resulting fillets will be inserted in this list
 * \param[in] type if AY_TRUE, an additional fillet between the last
 *  and the first patch will be created
 * \param[in] fillet_type if 1, fillets will be created, if -1,
 *  no fillets but set backs will be computed
 * \param[in] ftlen fillet tangent length
 * \param[in] uv controls which sides of the patches to connect
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_fillgaps(ay_object *o, int type, int fillet_type,
		double ftlen, char *uv)
{
 int ay_status = AY_OK;
 ay_object *fillet, *first = NULL, *last = NULL;
 char *fuv, uv2[3] = {0};

  first = o;
  fuv = uv;
  while(o)
    {
      fillet = NULL;
      o->selected = 0;
      /* only create a fillet between two patches */
      if((o->type == AY_IDNPATCH) && o->next &&
	 (o->next->type == AY_IDNPATCH))
	{
	  if(fillet_type > 0)
	    {
	      ay_status = ay_npt_fillgap(o, o->next, ftlen, fuv, &fillet);
	      if(ay_status)
		return ay_status;

	      if(fillet)
		{
		  /* mark as fillet */
		  fillet->selected = 1;
		  /* link fillet to list of patches */
		  fillet->next = o->next;
		  o->next = fillet;
		  /* adjust o for next iteration */
		  o = fillet->next;
		}
	    }
	  else
	    {
	      ay_status = ay_npt_setback(o, o->next, ftlen, fuv);
	      if(ay_status)
		return ay_status;
	    }
	} /* if */

      if(fuv)
	{
	  if(*fuv != '\0')
	    {
	      fuv++;
	    }
	  else
	    {
	      fuv = NULL;
	    }
	}

      if(!fillet)
	o = o->next;
    } /* while */

  if(type)
    {
      /* create fillet between last and first patch */

      /* find first and last patch and their uv-select parameters */
      o = first;
      if(uv)
	uv2[1] = *uv;
      fuv = uv;
      while(o)
	{
	  if(o->type == AY_IDNPATCH)
	    last = o;
	  if(fuv && !o->selected)
	    {
	      if(*fuv != '\0')
		{
		  uv2[0] = *fuv;
		  fuv++;
		}
	      else
		{
		  /* premature end of uv-select => use default 'u' */
		  uv2[0] = 'u';
		  fuv = NULL;
		}
	    } /* if */
	  o = o->next;
	} /* while */

      fillet = NULL;
      if(!uv)
	fuv = NULL;
      else
	fuv = uv2;

      /* create the fillet */
      if(last->type == AY_IDNPATCH && first->type == AY_IDNPATCH)
	{
	  if(fillet_type > 0)
	    {
	      ay_status = ay_npt_fillgap(last, first, ftlen, fuv, &fillet);
	      if(ay_status)
		return ay_status;

	      if(fillet)
		{
		  /* mark as fillet */
		  fillet->selected = 1;
		  /* link fillet to list of patches */
		  if(last)
		    {
		      fillet->next = last->next;
		      last->next = fillet;
		    }
		  else
		    {
		      (void)ay_object_delete(fillet);
		    }
		}
	    }
	  else
	    {
	      ay_status = ay_npt_setback(o, o->next, ftlen, fuv);
	      if(ay_status)
		return ay_status;
	    }
	} /* if */
    } /* if not open */

 return ay_status;
} /* ay_npt_fillgaps */


/** ay_npt_fliptrim:
 * Flip a single trim curve.
 *
 * \param[in] np NURBS surface the trim curve belongs to
 * \param[in,out] trim trim curve object
 * \param[in] mode dimension to flip
 */
void
ay_npt_fliptrim(ay_nurbpatch_object *np, ay_object *trim, int mode)
{
 double u1, u2, v1, v2, mu, mv, m[16], mt[16];
 /*double s;*/

  if(!trim)
    return;

  u1 = np->uknotv[np->uorder-1];
  u2 = np->uknotv[np->width];
  mu = u1+((u2-u1)*0.5);

  ay_trafo_identitymatrix(mt);

  if(mode == 0)
    {
      /* U */
      ay_trafo_translatematrix(mu, 0.0, 0.0, mt);
      ay_trafo_scalematrix(-1.0, 1.0, 1.0, mt);
      ay_trafo_translatematrix(-mu, 0.0, 0.0, mt);
    }
  else
    {
      /* v/V */
      v1 = np->vknotv[np->vorder-1];
      v2 = np->vknotv[np->height];
      mv = v1+((v2-v1)*0.5);

      ay_trafo_translatematrix(mu, mv, 0.0, mt);
      ay_trafo_rotatematrix(90.0, 0.0, 0.0, 1.0, mt);
      /*
      s = (u2-u1)/(v2-v1);
      if(fabs(s-1.0)>AY_EPSILON)
	{
	  ay_trafo_scalematrix((v2-v1)/(u2-u1), (u2-u1)/(v2-v1), 1.0, mt);
	}
      */
      if(mode == 2)
	{
	  /* V */
	  ay_trafo_scalematrix(1.0, -1.0, 1.0, mt);
	}
      ay_trafo_translatematrix(-mu, -mv, 0.0, mt);
    }

  ay_trafo_creatematrix(trim, m);
  ay_trafo_multmatrix(mt, m);
  ay_trafo_decomposematrix(mt, trim);

 return;
} /* ay_npt_fliptrim */


/** ay_npt_fliptrims:
 *  Flip the trim curves.
 *
 * \param[in] np NURBS surface the trims belong to
 * \param[in,out] trim trim curves to process
 * \param[in] mode dimension to flip
 *
 */
void
ay_npt_fliptrims(ay_nurbpatch_object *np, ay_object *trim, int mode)
{

  if(!trim)
    return;

  while(trim)
    {
      if(trim->type == AY_IDLEVEL)
	{
	  if(trim->down && trim->down->next)
	    {
	      ay_npt_fliptrim(np, trim->down, mode);
	    }
	}
      else
	{
	  ay_npt_fliptrim(np, trim, mode);
	}

      trim = trim->next;
    } /* while */

 return;
} /* ay_npt_fliptrims */


/** ay_npt_copytrims:
 *  copy all trim curves from NURBS patch \a o to \a target
 *  (complete bounding trims will be omitted)
 *
 * \param[in] o NURBS surface with trim curves to copy
 * \param[in] u1 new u minimum
 * \param[in] u2 new u maximum
 * \param[in] uv di
 * \param[in,out] target NURBS surface where copies will be added
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_copytrims(ay_object *o, double u1, double u2, char *uv,
		 ay_object *target)
{
 int ay_status = AY_OK;
 int is_bound = AY_FALSE;
 ay_object *t, **d;
 ay_nurbpatch_object *np = NULL;

  if(!o || !target)
    return AY_ENULL;

  if(o->type == AY_IDNPATCH)
    {
      np = (ay_nurbpatch_object *)o->refine;

      ay_npt_isboundcurve(o->down, np->uknotv[np->uorder-1],
			  np->uknotv[np->width],
			  np->vknotv[np->vorder-1],
			  np->vknotv[np->height],
			  &is_bound);
      if(is_bound)
	{
	  ay_status = ay_object_copymulti(o->down->next, &t);
	}
      else
	{
	  ay_status = ay_object_copymulti(o->down, &t);
	}

      if(!t)
	return AY_OK;

      if(uv)
	{
	  switch(*uv)
	    {
	    case 'U':
	      ay_npt_fliptrims(np, t, 0);
	      break;
	    case 'V':
	      ay_npt_fliptrims(np, t, 1);
	      break;
	    case 'v':
	      ay_npt_fliptrims(np, t, 2);
	      break;
	    default:
	      break;
	    }
	}

      ay_status = ay_npt_rescaletrims(t, 0,
				      np->uknotv[np->uorder-1],
				      np->uknotv[np->width],
				      u1, u2);

      /* append new trims */
      d = &(target->down);
      o = target->down;
      while(o && o->next)
	o = o->next;
      *d = t;
    } /* if o is npatch */

 return ay_status;
} /* ay_npt_copytrims */


/** ay_npt_createtrimrect:
 * Create bounding rectangular trim curve.
 *
 * \param[in,out] o NURBS surface where the trim should be added
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_createtrimrect(ay_object *o)
{
 ay_object *r = NULL;
 ay_nurbpatch_object *patch = NULL;
 ay_nurbcurve_object *curve = NULL;
 double knots[7] = {0.0, 0.0, 0.25, 0.5, 0.75, 1.0, 1.0};
 int i;

  if(!o)
    return AY_ENULL;

  if(o->type == AY_IDNPATCH)
    {
      patch = (ay_nurbpatch_object *)o->refine;

      if(!(r = calloc(1, sizeof(ay_object))))
	{
	  return AY_EOMEM;
	}

      r->type = AY_IDNCURVE;
      ay_object_defaults(r);

      if(!(curve = calloc(1, sizeof(ay_nurbcurve_object))))
	{
	  free(r);
	  return AY_EOMEM;
	}

      if(!(curve->controlv = malloc(20*sizeof(double))))
	{
	  free(r);
	  free(curve);
	  return AY_EOMEM;
	}

      if(!(curve->knotv = malloc(7*sizeof(double))))
	{
	  free(r); free(curve->controlv); free(curve);
	  return AY_EOMEM;
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
      curve->controlv[0] = patch->uknotv[patch->uorder-1];
      curve->controlv[1] = patch->vknotv[patch->vorder-1];
      curve->controlv[2] = 0.0;
      curve->controlv[3] = 1.0;
      curve->controlv[4] = patch->uknotv[patch->width];
      curve->controlv[5] = patch->vknotv[patch->vorder-1];
      curve->controlv[6] = 0.0;
      curve->controlv[7] = 1.0;
      curve->controlv[8] = patch->uknotv[patch->width];
      curve->controlv[9] = patch->vknotv[patch->height];
      curve->controlv[10] = 0.0;
      curve->controlv[11] = 1.0;
      curve->controlv[12] = patch->uknotv[patch->uorder-1];
      curve->controlv[13] = patch->vknotv[patch->height];
      curve->controlv[14] = 0.0;
      curve->controlv[15] = 1.0;
      curve->controlv[16] = patch->uknotv[patch->uorder-1];
      curve->controlv[17] = patch->vknotv[patch->vorder-1];
      curve->controlv[18] = 0.0;
      curve->controlv[19] = 1.0;

      r->refine = curve;

      r->next = o->down;
      o->down = r;
    }

 return AY_OK;
} /* ay_npt_createtrimrect */


/** ay_npt_concat:
 *  concatenate the patches in \a o by breaking them into
 *  curves, making the curves compatible, and building a
 *  new patch from the compatible curves;
 *  returns resulting patch object in \a result
 *
 * \param[in,out] o a number of NURBS patch objects (NURBS curves can
 *  be mixed in), the selected attribute should be cleared
 * \param[in] type if AY_CTCLOSED or AY_CTPERIODIC the new patch will
 *  be closed or periodic in U
 * \param[in] order desired order of the concatenated patch in U
 * \param[in] knot_type desired knot type of the concatenated patch in U;
 *  if knot_type is AY_KTCUSTOM, a special knot vector will be created,
 *  that makes the concatenated surface 'interpolate' all input surfaces,
 *  however, this comes at the cost of multiple internal knots
 * \param[in] fillet_type if 0, no fillets will be created;
 *  if 1, fillets are created for all gaps in the list of provided patches
 *  prior to concatenation;
 *  if -1, the patches are considered to be connected already and for
 *  a smooth transition, the respective borders will be set back
 * \param[in] ftlen fillet tangent length
 * \param[in] compatible if AY_TRUE, the patches and curves are considered
 *  compatible, and no clamping/elevating along V will take place
 * \param[in] uv controls which sides of the patches to connect
 * \param[in,out] result pointer where the resulting patch will be stored
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_concat(ay_object *o, int type, int order,
	      int knot_type, int fillet_type, double ftlen,
	      int compatible, char *uv, ay_object **result)
{
 int ay_status = AY_OK;
 ay_object *patches = NULL, *new = NULL;
 ay_object *curve = NULL, *allcurves = NULL, **nextcurve = NULL;
 ay_list_object *curvelist = NULL, **nextlist = NULL, *rem;
 ay_nurbpatch_object *np = NULL;
 double *newknotv = NULL, u1, u2;
 int a = 0, i = 0, j = 0, k = 0, l = 0, ncurves = 0, uvlen = 0;
 int max_order = 0, create_trimrect = AY_FALSE;
 char *swapuv;

  if(!o || !result)
    return AY_ENULL;

  patches = o;
  nextcurve = &allcurves;

  if(uv)
    {
      uvlen = strlen(uv);
    }

  /* create fillets */
  if(fillet_type != 0)
    {
      ay_status = ay_npt_fillgaps(o, type, fillet_type, ftlen, uv);
      if(ay_status)
	goto cleanup;
    } /* if */

  /* for custom knot vectors, elevate all patches to max_order
     or order (whatever is higher) and make sure they are clamped */
  if(knot_type == AY_KTCUSTOM)
    {
      /* get max order */
      while(o)
	{
	  if(o->type == AY_IDNPATCH)
	    {
	      np = (ay_nurbpatch_object *)o->refine;
	      if(o->selected)
		{
		  /* this is a fillet patch => do not consume
		     a character from uvselect */
		  if(np->uorder > max_order)
		    max_order = np->uorder;
		}
	      else
		{
		  if(uvlen > 0 && uvlen > i && (uv[i] == 'v' || uv[i] == 'V'))
		    {
		      if(np->vorder > max_order)
			max_order = np->vorder;
		    }
		  else
		    {
		      if(np->uorder > max_order)
			max_order = np->uorder;
		    } /* if */
		  i++;
		} /* if */
	    } /* if */
	  o = o->next;
	} /* while */

      if(order < max_order)
	order = max_order;
      else
	max_order = order;

      /* elevate or clamp */
      o = patches;
      i = 0;
      while(o)
	{
	  if(o->type == AY_IDNPATCH)
	    {
	      np = (ay_nurbpatch_object *)o->refine;
	      if(o->selected)
		{
		  /* this is a fillet patch => do not consume
		     a character from uvselect */
		  if(np->uorder < max_order)
		    {
		      ay_status = ay_npt_elevateu(np, max_order-np->uorder);
		      if(ay_status)
			goto cleanup;
		    }
		  if((fabs(np->uknotv[0] - 0.0) > AY_EPSILON) ||
		     (fabs(np->uknotv[np->width+np->uorder-1] - 1.0) >
		      AY_EPSILON))
		    {
		      ay_status = ay_knots_rescaletorange(np->width+np->uorder,
							  np->uknotv, 0, 1);
		      if(ay_status)
			goto cleanup;
		    }
		}
	      else
		{
		  if(uvlen > 0 && uvlen > i && (uv[i] == 'v' || uv[i] == 'V'))
		    {
		      if(np->vorder < max_order)
			{
			  ay_status = ay_npt_elevatev(np, max_order-np->vorder);
			  if(ay_status)
			    goto cleanup;
			}
		      else
			{
			  ay_status = ay_npt_clampv(np, 0);
			  if(ay_status)
			    goto cleanup;
			}
		      if((fabs(np->vknotv[0] - 0.0) > AY_EPSILON) ||
			 (fabs(np->vknotv[np->height+np->vorder-1] - 1.0) >
			  AY_EPSILON))
			{
			  if(o->down && o->down->next)
			    {
			    (void)ay_npt_rescaletrims(o->down, 1,
						      np->vknotv[0],
			      np->vknotv[np->height+np->vorder-1], 0, 1);
			    }

			  ay_status = ay_knots_rescaletorange(np->height +
							      np->vorder,
							      np->vknotv, 0, 1);
			  if(ay_status)
			    goto cleanup;
			} /* if */
		    }
		  else
		    {
		      if(np->uorder < max_order)
			{
			  ay_status = ay_npt_elevateu(np, max_order-np->uorder);
			  if(ay_status)
			    goto cleanup;
			}
		      else
			{
			  ay_status = ay_npt_clampu(np, 0);
			  if(ay_status)
			    goto cleanup;
			}
		      if((fabs(np->uknotv[0] - 0.0) > AY_EPSILON) ||
			 (fabs(np->uknotv[np->width+np->uorder-1] - 1.0) >
			  AY_EPSILON))
			{
			  if(o->down && o->down->next)
			    {
			    (void)ay_npt_rescaletrims(o->down, 0,
						      np->uknotv[0],
				     np->uknotv[np->width+np->uorder-1], 0, 1);
			    }

			  ay_status = ay_knots_rescaletorange(np->width+
							      np->uorder,
							  np->uknotv, 0, 1);
			  if(ay_status)
			    goto cleanup;
			} /* if */
		    } /* if */
		  i++;
		} /* if */
	    } /* if */
	  o = o->next;
	} /* while */
    } /* if */

  /* (possibly) revert the patches and break them to curves */
  o = patches;
  i = 0;
  while(o)
    {
      if(o->type == AY_IDNPATCH)
	{
	  if(o->selected)
	    {
	      /* this is a fillet patch => do not consume
		 a character from uvselect */
	      ay_status = ay_npt_breakintocurvesu(o, AY_TRUE, nextcurve,
						  &nextcurve);
	      if(ay_status)
		goto cleanup;
	    }
	  else
	    {
	      if(uvlen > 0 && uvlen > i && (uv[i] == 'v' || uv[i] == 'V'))
		{
		  if(uv[i] == 'V')
		    {
		      ay_status =
			ay_npt_revertv((ay_nurbpatch_object *)o->refine);
		      if(ay_status)
			goto cleanup;
		    }

		  if(order == 1)
		    {
		      np = (ay_nurbpatch_object *)o->refine;
		      order = np->vorder;
		    }

		  ay_status = ay_npt_breakintocurvesv(o, AY_TRUE, nextcurve,
						      &nextcurve);
		  if(ay_status)
		    goto cleanup;
		}
	      else
		{
		  if(uvlen > 0 && uvlen > i && uv[i] == 'U')
		    {
		      ay_status =
			ay_npt_revertu((ay_nurbpatch_object *)o->refine);
		      if(ay_status)
			goto cleanup;
		    }

		  if(order == 1)
		    {
		      np = (ay_nurbpatch_object *)o->refine;
		      order = np->uorder;
		    }

		  ay_status = ay_npt_breakintocurvesu(o, AY_TRUE, nextcurve,
						      &nextcurve);
		  if(ay_status)
		    goto cleanup;
		} /* if */
	      i++;
	    } /* if */
	}
      else
	{
	  /* must be a curve, just copy it... */
	  ay_status = ay_object_copy(o, nextcurve);
	  if(ay_status)
	    goto cleanup;
	  ay_nct_applytrafo(*nextcurve);
	  nextcurve = &((*nextcurve)->next);
	} /* if */
      o = o->next;
    } /* while */

  /* count the curves and build a list */
  nextlist = &curvelist;
  curve = allcurves;
  while(curve)
    {
      ncurves++;

      if(!(*nextlist = calloc(1, sizeof(ay_list_object))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      (*nextlist)->object = curve;

      nextlist = &((*nextlist)->next);

      curve = curve->next;
    } /* while */

  /* make all curves compatible */
  if(!compatible)
    {
      ay_status = ay_nct_makecompatible(allcurves);
      if(ay_status)
	goto cleanup;
    }

  if((type == AY_CTCLOSED || type == AY_CTPERIODIC) && fillet_type)
    type = AY_CTOPEN;

  /* build a new patch from the compatible curves */
  ay_status = ay_npt_buildfromcurves(curvelist, ncurves, type, order,
				     knot_type, AY_FALSE, &new);

  if(ay_status)
    goto cleanup;

  /* create custom knot vector */
  if(knot_type == AY_KTCUSTOM)
    {
      np = (ay_nurbpatch_object *)new->refine;
      if(!(newknotv = calloc(np->width + np->uorder, sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      np->uknotv = newknotv;
      a = order;
      j = 0;
      o = patches;
      i = 0;

      while(o)
	{
	  if(o->type == AY_IDNPATCH)
	    {
	      np = (ay_nurbpatch_object *)o->refine;

	      if(!o->selected &&
		 uvlen > 0 && uvlen > i && (uv[i] == 'v' || uv[i] == 'V'))
		{
		  k = np->vorder;
		  for(l = k; l < np->height+np->vorder; l++)
		    {
		      newknotv[a] = np->vknotv[l]+j;
		      a++;
		    }
		}
	      else
		{
		  k = np->uorder;
		  u1 = np->uknotv[k]+j;
		  for(l = k; l < np->width+np->uorder; l++)
		    {
		      newknotv[a] = np->uknotv[l]+j;
		      a++;
		    }
		}

	      if(!o->selected && o->down && o->down->next)
		{
		  swapuv = NULL;
		  if(uvlen > 0 && uvlen > i)
		    swapuv = &(uv[i]);
		  if(j == 0)
		    u1 = 0.0;
		  else
		    u1 = newknotv[a-(l-1)];
		  u2 = newknotv[a-1];
		  ay_status = ay_npt_copytrims(o, u1, u2, swapuv, new);
		  if(ay_status)
		    goto cleanup;
		  create_trimrect = AY_TRUE;
		}

	      if(!o->selected)
		i++;
	      j++;
	    }
	  else
	    {
	      /* must be a curve => add a single knot */
	      newknotv[a] = j;
	      a++;
	      j++;
	    }
	  o = o->next;
	} /* while */

      /* last knots */
      if(type == AY_CTCLOSED && fillet_type == 0)
	{
	  np = (ay_nurbpatch_object *)new->refine;
	  for(i = a; i < np->width+np->uorder; i++)
	    {
	      newknotv[i] = newknotv[a-1]+1;
	    }
	}

      /* create bounding trim */
      if(create_trimrect)
	{
	  ay_status = ay_npt_createtrimrect(new);
	  if(ay_status)
	    goto cleanup;
	}
    } /* if knot type custom*/

  np = (ay_nurbpatch_object *)new->refine;
  np->is_rat = ay_npt_israt(np);
  if(knot_type == AY_KTCUSTOM)
    ay_npt_setuvtypes(np, 0);

  /* return result */
  *result = new;

  /* prevent cleanup code from doing something harmful */
  new = NULL;

cleanup:

  (void)ay_object_deletemulti(allcurves, AY_TRUE);

  /* delete list */
  while(curvelist)
    {
      rem = curvelist;
      curvelist = rem->next;
      free(rem);
    }

  if(new)
    ay_object_delete(new);

 return ay_status;
} /* ay_npt_concat */


/* ay_npt_revolve:
 *  create a surface of revolution from the NURBS curve in <o>
 *  (that will be projected to the XY-plane for revolution)
 *  with revolution angle <arc>; if <sections> is > 0, not a
 *  standard NURBS circle geometry will be used for the surface
 *  but a circular B-Spline with appropriate number of sections
 *  and desired order <order>
 */
int
ay_npt_revolve(ay_object *o, double arc, int sections, int order,
	       ay_nurbpatch_object **revolution)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *new = NULL;
 ay_nurbcurve_object *curve;
 double *uknotv = NULL, *tcontrolv = NULL;
 double radius = 0.0, w = 0.0, x, y/*, z*/;
 int i = 0, j = 0, a = 0, b = 0, c = 0, knot_count = 0;
 double m[16], point[4] = {0}, *V = NULL;

  if(!o)
    return AY_ENULL;

  if(o->type != AY_IDNCURVE)
    return AY_ERROR;

  curve = (ay_nurbcurve_object *)(o->refine);

  /* get curves transformation-matrix */
  ay_trafo_creatematrix(o, m);

  if((arc >= 360.0) || (arc < -360.0) || (arc == 0.0))
    {
      arc = 360.0;
    }

  if((sections == 0) || (order <= 1))
    {
      order = 3;
    }

  /* calloc the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    return AY_EOMEM;

  if(!(uknotv = malloc((curve->length+curve->order)*sizeof(double))))
    {
      free(new);
      return AY_EOMEM;
    }

  memcpy(uknotv, curve->knotv, (curve->length+curve->order)*sizeof(double));
  new->uknotv = uknotv;
  new->uorder = curve->order;
  new->vorder = order;
  new->uknot_type = curve->knot_type;
  new->vknot_type = AY_KTCUSTOM;
  new->width = curve->length;

  if(sections > 0)
    {
      new->height = sections+(order-1);
      knot_count = new->height+order;
      if(!(V = malloc(knot_count * sizeof(double))))
	{
	  ay_npt_destroy(new);
	  return AY_EOMEM;
	}
      V[0] = 0.0;
      for(i = 1; i < knot_count; i++)
	{
	  V[i] = (double)i/(knot_count-1);
	}
      new->vknotv = V;
    }
  else
    {
      if(arc > 0.0)
	{
	  ay_status = ay_nb_CreateNurbsCircleArc(radius, 0.0, arc,
						 &new->height, &new->vknotv,
						 NULL);
	}
      else
	{
	  ay_status = ay_nb_CreateNurbsCircleArc(radius, arc, 0.0,
						 &new->height, &new->vknotv,
						 NULL);
	} /* if */
      if(ay_status)
	{
	  ay_npt_destroy(new);
	  return ay_status;
	}
    } /* if */

  if(!(new->controlv = malloc(new->height*new->width*4*
			      sizeof(double))))
    {
      ay_npt_destroy(new);
      return AY_EOMEM;
    }

  /* fill controlv */
  a = 0;
  for(j = 0; j < curve->length; j++)
    {
      /* transform point */
      x = curve->controlv[a];
      y = curve->controlv[a+1];
      /*z = curve->controlv[a+2];*/
      w = curve->controlv[a+3];

      /* project point onto XY-Plane! */
      point[2] = 0.0; /* XXXX loss of data! */

      point[0] = m[0]*x + m[4]*y /*+ m[8]*point[2]*/ + m[12];
      point[1] = m[1]*x + m[5]*y /*+ m[9]*point[2]*/ + m[13];
      /*point[3] = m[3]*x + m[7]*y + m[11]*point[2] + m[15]*w;*/

      radius = point[0];

      if(sections == 0)
	{
	  if(arc > 0.0)
	    {
	      ay_status = ay_nb_CreateNurbsCircleArc(radius, 0.0, arc,
						  &new->height, NULL,
						  &tcontrolv);
	    }
	  else
	    {
	      ay_status = ay_nb_CreateNurbsCircleArc(radius, arc, 0.0,
						  &new->height, NULL,
						  &tcontrolv);
	    } /* if */
	}
      else
	{
	  ay_status = ay_nct_crtcircbspcv(sections, radius, arc, order,
					  &tcontrolv);
	} /* if */

      if(ay_status)
	{
	  ay_npt_destroy(new);
	  return ay_status;
	}

      /* copy to real controlv */
      b = 0;
      c = j*new->height*4;
      for(i = 0; i < new->height; i++)
	{
	  new->controlv[c]   = tcontrolv[b];
	  new->controlv[c+1] = point[1];
	  new->controlv[c+2] = tcontrolv[b+1];
	  new->controlv[c+3] = tcontrolv[b+3]*w;
	  b += 4;
	  c += 4;
	} /* for */

	a += 4;
    } /* for */

  if(curve->is_rat || (sections == 0))
    new->is_rat = AY_TRUE;

  new->utype = curve->type;
  if(sections == 0)
    {
      if(fabs(arc) == 360.0)
	new->vtype = AY_CTCLOSED;
    }
  else
    {
      if(fabs(arc) == 360.0)
	new->vtype = AY_CTPERIODIC;
    }

  if(tcontrolv)
    free(tcontrolv);
  tcontrolv = NULL;

  *revolution = new;

 return ay_status;
} /* ay_npt_revolve */


/* ay_npt_swing:
 *  Sweep cross section <o1> around Y-axis, while placing and scaling
 *  it according to <o2> (creating a swung surface).
 */
int
ay_npt_swing(ay_object *o1, ay_object *o2,
	     ay_nurbpatch_object **swing)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *new = NULL;
 ay_nurbcurve_object *cs, *tr;
 double *cscv = NULL, *trcv = NULL, *p;
 double m[16];
 double angle, scal;
 int stride = 4;
 int i = 0, j = 0, a;

  if(!o1 || !o2 || !swing)
    return AY_ENULL;

  /* check parameters */
  if((o1->type != AY_IDNCURVE) || (o2->type != AY_IDNCURVE))
    return AY_ERROR;

  cs = (ay_nurbcurve_object *)(o1->refine);
  tr = (ay_nurbcurve_object *)(o2->refine);

  /* apply trafos to cross-section curves controlv */
  if(!(cscv = malloc(cs->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(cscv, cs->controlv, cs->length * stride * sizeof(double));

  ay_trafo_creatematrix(o1, m);

  a = 0;
  for(i = 0; i < cs->length; i++)
    {
      ay_trafo_apply3(&(cscv[a]), m);
      a += stride;
    }

  /* apply trafos to trajectory curves controlv */
  if(!(trcv = malloc(tr->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(trcv, tr->controlv, tr->length * stride * sizeof(double));

  ay_trafo_creatematrix(o2, m);
  a = 0;
  for(i = 0; i < tr->length; i++)
    {
      ay_trafo_apply3(&(trcv[a]), m);
      a += stride;
    }

  /* calloc and parametrize the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(new->uknotv = malloc((tr->length + tr->order) * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  memcpy(new->uknotv, tr->knotv, (tr->length + tr->order) * sizeof(double));
  new->uorder = tr->order;
  new->uknot_type = tr->knot_type;
  new->width = tr->length;

  if(!(new->vknotv = malloc((cs->length + cs->order) * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  memcpy(new->vknotv, cs->knotv, (cs->length+cs->order)*sizeof(double));
  new->vorder = cs->order;
  new->vknot_type = cs->knot_type;
  new->height = cs->length;

  if(!(new->controlv = malloc(cs->length * tr->length * stride *
			      sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* fill controlv */
  for(j = 0; j < tr->length; j++)
    {
      /* calculate angle */
      p = &(trcv[j*stride]);
      if(fabs(p[2]) > AY_EPSILON)
	{
	  if(p[2] > 0.0)
	    {
	      angle = atan(p[0]/p[2]);
	    }
	  else
	    {
	      if(fabs(p[0]) < AY_EPSILON)
		angle = AY_D2R(180.0);
	      else
		angle = AY_D2R(180.0)+atan(p[0]/p[2]);
	    }
	}
      else
	{
	  if(p[0] > 0.0)
	    angle = AY_D2R(90.0);
	  else
	    angle = AY_D2R(270.0);
	} /* if */

      /* calculate scale factor */
      scal = AY_V3LEN(p);

      /* set up transformation matrix */
      ay_trafo_identitymatrix(m);
      ay_trafo_scalematrix(scal, 1.0, scal, m);
      ay_trafo_rotatematrix(AY_R2D(angle), 0.0, 1.0, 0.0, m);

      /* copy and transform section */
      p = &(new->controlv[j*cs->length*stride]);
      memcpy(p, cscv, cs->length*stride*sizeof(double));

      for(i = 0; i < cs->length; i++)
	{
	  ay_trafo_apply3(p, m);

	  if(fabs(1.0-trcv[j*stride+3]) > AY_EPSILON)
	    {
	      p[3] *= trcv[j*stride+3];
	    }
	  p += 4;
	} /* for */
    } /* for */

  new->is_rat = ay_npt_israt(new);
  ay_npt_setuvtypes(new, 0);

  /* return result */
  *swing = new;

  /* prevent cleanup code from doing something harmful */
  new = NULL;

cleanup:
  if(cscv)
    free(cscv);

  if(trcv)
    free(trcv);

  if(new)
    {
      ay_npt_destroy(new);
    }

 return ay_status;
} /* ay_npt_swing */


/* ay_npt_sweep:
 *  Sweep cross section <o1> along path <o2> possibly rotating it,
 *  so that it is always perpendicular to the path, possibly
 *  scaling it by a factor derived from the difference of the
 *  y and z coordinates of scaling curve <o3> to y and z values 1.0.
 *  Rotation code derived from J. Bloomenthals "Reference Frames"
 *  (Graphic Gems I).
 */
int
ay_npt_sweep(ay_object *o1, ay_object *o2, ay_object *o3, int sections,
	     int rotate, int closed, ay_nurbpatch_object **sweep)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *new = NULL;
 ay_nurbcurve_object *tr, *cs, *sf = NULL;
 double *controlv = NULL;
 int i = 0, j = 0, a = 0, stride, sfis3d = AY_FALSE;
 double u, p1[4], p2[4], p3[4];
 double T0[3] = {0.0,0.0,-1.0};
 double T1[3] = {0.0,0.0,0.0};
 double T2[3] = {0.0,0.0,0.0};
 double A[3] = {0.0,0.0,0.0};
 double len = 0.0, plen = 0.0, plensf = 0.0;
 double m[16] = {0}, mi[16] = {0}, mcs[16], mtr[16];
 double mr[16], axisrot[4] = {0};
 double *cscv = NULL, *trcv = NULL, *sfcv = NULL;

  if(!o1 || !o2 || !sweep)
    return AY_ENULL;

  if((o1->type != AY_IDNCURVE) || (o2->type != AY_IDNCURVE) ||
     (o3 && (o3->type != AY_IDNCURVE)))
    return AY_ERROR;

  cs = (ay_nurbcurve_object *)(o1->refine);
  tr = (ay_nurbcurve_object *)(o2->refine);

  stride = 4;

  /* apply scale and rotation to cross-section curves controlv */
  if(!(cscv = malloc(cs->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(cscv, cs->controlv, cs->length * stride * sizeof(double));

  ay_quat_torotmatrix(o1->quat, mcs);
  ay_trafo_scalematrix(o1->scalx, o1->scaly, o1->scalz, mcs);

  a = 0;
  for(i = 0; i < cs->length; i++)
    {
      ay_trafo_apply3(&(cscv[a]), mcs);
      a += stride;
    }

  /* apply all transformations to trajectory curves controlv */
  if(!(trcv = malloc(tr->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(trcv, tr->controlv, tr->length * stride * sizeof(double));

  ay_trafo_creatematrix(o2, mtr);
  a = 0;
  for(i = 0; i < tr->length; i++)
    {
      ay_trafo_apply3(&(trcv[a]), mtr);
      a += stride;
    }

  /* apply all transformations to scaling curves controlv */
  if(o3)
    {
      sf = (ay_nurbcurve_object *)(o3->refine);

      if(!(sfcv = malloc(sf->length * stride * sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      memcpy(sfcv, sf->controlv, sf->length * stride * sizeof(double));

      ay_trafo_creatematrix(o3, mtr);
      a = 0;
      for(i = 0; i < sf->length; i++)
	{
	  ay_trafo_apply3(&(sfcv[a]), mtr);
	  if(fabs(sfcv[a+2]) > AY_EPSILON)
	    sfis3d = AY_TRUE;
	  a += stride;
	}
    } /* if */

  /* allocate the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* calculate number of sections and parameterize the new patch */
  if(sections <= 0)
    {
      if(tr->length == 2)
	{
	  sections = 1;
	}
      else
	{
	  sections = tr->length + 1;
	}
      new->uorder = tr->order;
    }
  else
    {
      if(sections > 2)
	{
	  new->uorder = 4;
	}
      else
	{
	  new->uorder = sections + 1;
	}
    } /* if */

  new->vorder = cs->order;
  new->vknot_type = cs->knot_type;
  new->uknot_type = AY_KTNURB;
  new->width = sections+1;
  new->height = cs->length;
  new->glu_sampling_tolerance = cs->glu_sampling_tolerance;

  /* allocate control point and knot arrays */
  if(!(controlv = malloc(new->width*new->height*stride*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  new->controlv = controlv;
  if(!(new->vknotv = malloc((new->height + new->vorder) * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(new->uknotv = malloc((new->width + new->uorder) * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  ay_status = ay_knots_createnp(new);
  if(ay_status)
    { goto cleanup; }

  if(cs->knot_type == AY_KTCUSTOM)
    {
      memcpy(new->vknotv,cs->knotv,(cs->length+cs->order)*sizeof(double));
    }

  /* get first point/derivative on trajectory */
  ay_nb_CurvePoint4D(tr->length-1, tr->order-1, tr->knotv, trcv,
		     tr->knotv[tr->order-1], p1);

  if(closed)
    {
      ay_nb_CurvePoint4D(tr->length-1, tr->order-1, tr->knotv, trcv,
			 tr->knotv[tr->length], p2);

      p1[0] = p1[0] + ((p2[0]-p1[0])/2.0);
      p1[1] = p1[1] + ((p2[1]-p1[1])/2.0);
      p1[2] = p1[2] + ((p2[2]-p1[2])/2.0);
    }


  plen = fabs(tr->knotv[tr->length] - tr->knotv[tr->order-1]);
  if(o3)
    {
      plensf = fabs(sf->knotv[sf->length] - sf->knotv[sf->order-1]);
    }

  T0[0] = 1.0;
  T0[1] = 0.0;
  T0[2] = 0.0;

  ay_nb_ComputeFirstDer4D(tr->length-1, tr->order-1, tr->knotv,
			  trcv, tr->knotv[tr->order-1], T1);

  len = AY_V3LEN(T1);
  AY_V3SCAL(T1,(1.0/len));

  if(fabs(-1.0 - T1[0]) < AY_EPSILON &&
     fabs(T1[1]) < AY_EPSILON && fabs(T1[2]) < AY_EPSILON)
    {
      T0[0] = -1.0;
    }

  ay_trafo_identitymatrix(mr);

  /* copy cross sections controlv section+1 times and sweep it */
  for(i = 0; i <= sections; i++)
    {
      memcpy(&(controlv[i * stride * cs->length]), &(cscv[0]),
	     cs->length * stride * sizeof(double));

      /* create transformation matrix */
      /* first, set it to identity */
      ay_trafo_identitymatrix(m);

      /* now, apply scaling function (if present) */
      if(o3)
	{
	  u = sf->knotv[sf->order-1]+(((double)i/sections) * plensf);

	  ay_nb_CurvePoint4D(sf->length-1, sf->order-1, sf->knotv,
			     sfcv, u, p3);
	  p3[1] = fabs(p3[1]);
	  p3[2] = fabs(p3[2]);
	  if(p3[1] > AY_EPSILON)
	    {
	      ay_trafo_scalematrix(1.0, 1.0/p3[1], 1.0, m);
	    }
	  if(sfis3d)
	    {
	      if(p3[2] > AY_EPSILON)
		{
		  ay_trafo_scalematrix(1.0, 1.0, 1.0/p3[2], m);
		}
	    }
	  else
	    {
	      if(p3[1] > AY_EPSILON)
		{
		  ay_trafo_scalematrix(1.0, 1.0, 1.0/p3[1], m);
		}
	    }
	}

      /* now, apply rotation (if requested) */
      u = tr->knotv[tr->order-1]+(((double)i/sections)*plen);
      if(rotate)
	{
	  ay_nb_ComputeFirstDer4D(tr->length-1, tr->order-1, tr->knotv,
				  trcv, u, T1);

	  if(closed && (i == 0 || i == sections))
	    {
	      /* compute average between first and last derivative */
	      ay_nb_ComputeFirstDer4D(tr->length-1, tr->order-1, tr->knotv,
				      trcv, tr->knotv[tr->order-1], T1);
	      ay_nb_ComputeFirstDer4D(tr->length-1, tr->order-1, tr->knotv,
				      trcv, tr->knotv[tr->length], T2);

	      T1[0] = T1[0] + ((T2[0]-T1[0])/2.0);
	      T1[1] = T1[1] + ((T2[1]-T1[1])/2.0);
	      T1[2] = T1[2] + ((T2[2]-T1[2])/2.0);
	    }

	  len = AY_V3LEN(T1);
	  AY_V3SCAL(T1,(1.0/len));

	  if(((fabs(T1[0]-T0[0]) > AY_EPSILON) ||
	      (fabs(T1[1]-T0[1]) > AY_EPSILON) ||
	      (fabs(T1[2]-T0[2]) > AY_EPSILON)))
	    {
	      AY_V3CROSS(A,T0,T1);
	      len = AY_V3LEN(A);
	      AY_V3SCAL(A,(1.0/len));

	      axisrot[0] = AY_R2D(acos(AY_V3DOT(T0,T1)));
	      memcpy(&(axisrot[1]), A, 3*sizeof(double));

	      if(fabs(axisrot[0]) > AY_EPSILON)
		{
		  ay_trafo_rotatematrix(-axisrot[0], axisrot[1],
					axisrot[2], axisrot[3], mr);
		}
	    } /* if */

	  memcpy(T0, T1, 3*sizeof(double));

	  ay_trafo_multmatrix(m, mr);
	} /* if rotate */

      /* now, add translation to current point on trajectory */
      if(closed && (i == 0 || i == sections))
	{
	  memcpy(p2, p1, 3*sizeof(double));
	}
      else
	{
	  ay_nb_CurvePoint4D(tr->length-1, tr->order-1, tr->knotv,
			     trcv, u, p2);
	}
      ay_trafo_translatematrix(-p2[0], -p2[1], -p2[2], m);

      ay_trafo_invmatrix(m, mi);

      /* sweep profile */
      for(j = 0; j < cs->length; j++)
	{
	  ay_trafo_apply3(&(controlv[i*cs->length*stride+j*stride]), mi);
	} /* for */
    } /* for i = 0; i <= sections; i++ */

  new->is_rat = ay_npt_israt(new);
  ay_npt_setuvtypes(new, 0);

  /* return result */
  *sweep = new;

  /* prevent cleanup code from doing something harmful */
  new = NULL;

  /* clean up */
cleanup:

  if(cscv)
    free(cscv);
  if(trcv)
    free(trcv);
  if(sfcv)
    free(sfcv);

  if(new)
    {
      ay_npt_destroy(new);
    }

 return ay_status;
} /* ay_npt_sweep */


/* ay_npt_sweepperiodic:
 *  Sweep cross section <o1> along path <o2> possibly rotating it,
 *  so that it is always perpendicular to the path, possibly
 *  scaling it by a factor derived from the difference of the
 *  y and z coordinates of scaling curve <o3> to the y and z values 1.0.
 *  In contrast to ay_npt_sweep() above, this function creates a
 *  periodic patch (in the direction of the trajectory).
 *  Rotation code derived from J. Bloomenthals "Reference Frames"
 *  (Graphic Gems I).
 */
int
ay_npt_sweepperiodic(ay_object *o1, ay_object *o2, ay_object *o3, int sections,
		     int rotate, ay_nurbpatch_object **sweep)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *new = NULL;
 ay_nurbcurve_object *tr, *cs, *sf = NULL;
 double *controlv = NULL;
 int i = 0, j = 0, a = 0, stride = 4, sfis3d = AY_FALSE;
 double u, p1[4], p2[4], p3[4];
 double T0[3] = {0.0,0.0,-1.0};
 double T1[3] = {0.0,0.0,0.0};
 double A[3] = {0.0,0.0,0.0};
 double len = 0.0, plen = 0.0, plensf = 0.0;
 double m[16] = {0}, mi[16] = {0}, mcs[16], mtr[16];
 double mr[16], axisrot[4] = {0};
 double *cscv = NULL, *trcv = NULL, *sfcv = NULL;

  if(!o1 || !o2 || !sweep)
    return AY_ENULL;

  if((o1->type != AY_IDNCURVE) || (o2->type != AY_IDNCURVE) ||
     (o3 && (o3->type != AY_IDNCURVE)))
    return AY_ERROR;

  if(sections == 1)
    return AY_ERROR;

  cs = (ay_nurbcurve_object *)(o1->refine);
  tr = (ay_nurbcurve_object *)(o2->refine);

  /* apply scale and rotation to cross-section curves controlv */
  if(!(cscv = malloc(cs->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(cscv, cs->controlv, cs->length * stride * sizeof(double));

  ay_quat_torotmatrix(o1->quat, mcs);
  ay_trafo_scalematrix(o1->scalx, o1->scaly, o1->scalz, mcs);

  a = 0;
  for(i = 0; i < cs->length; i++)
    {
      ay_trafo_apply3(&(cscv[a]), mcs);
      a += stride;
    }

  /* apply all transformations to trajectory curves controlv */
  if(!(trcv = malloc(tr->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(trcv, tr->controlv, tr->length * stride * sizeof(double));

  ay_trafo_creatematrix(o2, mtr);
  a = 0;
  for(i = 0; i < tr->length; i++)
    {
      ay_trafo_apply3(&(trcv[a]), mtr);
      a += stride;
    }

  /* apply all transformations to scaling curves controlv */
  if(o3)
    {
      sf = (ay_nurbcurve_object *)(o3->refine);

      if(!(sfcv = malloc(sf->length * stride * sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      memcpy(sfcv, sf->controlv, sf->length * stride * sizeof(double));

      ay_trafo_creatematrix(o3, mtr);
      a = 0;
      for(i = 0; i < sf->length; i++)
	{
	  ay_trafo_apply3(&(sfcv[a]), mtr);
	  if(fabs(sfcv[a+2]) > AY_EPSILON)
	    sfis3d = AY_TRUE;
	  a += stride;
	}
    } /* if */

  /* allocate the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* calculate number of sections and parameterize the new patch */
  if(sections <= 0)
    {
      sections = tr->length+1;
      new->uorder = tr->order;
    }
  else
    {
      if(sections == 2)
	{
	  new->uorder = 3;
	}
      else
	{
	  new->uorder = 4;
	}
    }

  new->vorder = cs->order;
  new->vknot_type = cs->knot_type;
  new->uknot_type = AY_KTBSPLINE;
  new->width = sections+new->uorder-1;
  new->height = cs->length;

  new->glu_sampling_tolerance = cs->glu_sampling_tolerance;

  /* allocate control point and knot arrays */
  if(!(controlv = malloc(cs->length*new->width*stride*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  new->controlv = controlv;
  if(!(new->vknotv = malloc((cs->length+cs->order)*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(new->uknotv = malloc((sections+4)*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  ay_status = ay_knots_createnp(new);
  if(ay_status)
    { goto cleanup; }

  if(cs->knot_type == AY_KTCUSTOM)
    {
      memcpy(new->vknotv,cs->knotv,(cs->length+cs->order)*sizeof(double));
    }

  ay_nb_CurvePoint4D(tr->length-1, tr->order-1, tr->knotv, trcv,
		     tr->knotv[0], p1);

  plen = fabs(tr->knotv[tr->length] - tr->knotv[tr->order-1]);

  if(o3)
    {
      plensf = fabs(sf->knotv[sf->length] - sf->knotv[sf->order-1]);
    }

  T0[0] = 1.0;
  T0[1] = 0.0;
  T0[2] = 0.0;

  len = AY_V3LEN(T1);
  AY_V3SCAL(T1,(1.0/len));

  if(fabs(-1.0 - T1[0]) < AY_EPSILON &&
     fabs(T1[1]) < AY_EPSILON && fabs(T1[2]) < AY_EPSILON)
    {
      T0[0] = -1.0;
    }

  ay_trafo_identitymatrix(mr);

  /* copy cross sections controlv section times and sweep it */
  for(i = 0; i < sections; i++)
    {
      memcpy(&(controlv[i * stride * cs->length]), &(cscv[0]),
	     cs->length * stride * sizeof(double));

      /* create transformation matrix */
      /* first, set it to identity */
      ay_trafo_identitymatrix(m);

      /* now, apply scaling function (if present) */
      if(o3)
	{
	  u = sf->knotv[sf->order-1]+(((double)i/sections) * plensf);

	  ay_nb_CurvePoint4D(sf->length-1, sf->order-1, sf->knotv,
			     sfcv, u, p3);
	  p3[1] = fabs(p3[1]);
	  p3[2] = fabs(p3[2]);
	  if(p3[1] > AY_EPSILON)
	    {
	      ay_trafo_scalematrix(1.0, 1.0/p3[1], 1.0, m);
	    }
	  if(sfis3d)
	    {
	      if(p3[2] > AY_EPSILON)
		{
		  ay_trafo_scalematrix(1.0, 1.0, 1.0/p3[2], m);
		}
	    }
	  else
	    {
	      if(p3[1] > AY_EPSILON)
		{
		  ay_trafo_scalematrix(1.0, 1.0, 1.0/p3[1], m);
		}
	    }
	} /* if */

      /* now, apply rotation (if requested) */
      u = tr->knotv[tr->order-1]+(((double)i/sections)*plen);

      if(rotate)
	{
	  ay_nb_ComputeFirstDer4D(tr->length-1, tr->order-1, tr->knotv,
				  trcv, u, T1);

	  len = AY_V3LEN(T1);
	  AY_V3SCAL(T1,(1.0/len));

	  if(((fabs(T1[0]-T0[0]) > AY_EPSILON) ||
	      (fabs(T1[1]-T0[1]) > AY_EPSILON) ||
	      (fabs(T1[2]-T0[2]) > AY_EPSILON)))
	    {
	      AY_V3CROSS(A,T0,T1);
	      len = AY_V3LEN(A);
	      AY_V3SCAL(A,(1.0/len));

	      axisrot[0] = AY_R2D(acos(AY_V3DOT(T0,T1)));
	      memcpy(&(axisrot[1]), A, 3*sizeof(double));

	      if(fabs(axisrot[0]) > AY_EPSILON)
		{
		  ay_trafo_rotatematrix(-axisrot[0], axisrot[1],
					axisrot[2], axisrot[3], mr);
		}
	    } /* if */

	  memcpy(T0, T1, 3*sizeof(double));

	  ay_trafo_multmatrix(m, mr);
	} /* if rotate */

      /* now, add translation to current point on trajectory */
      ay_nb_CurvePoint4D(tr->length-1, tr->order-1, tr->knotv,
			 trcv, u, p2);

      ay_trafo_translatematrix(-p2[0], -p2[1], -p2[2], m);

      ay_trafo_invmatrix(m, mi);

      /* sweep profile */
      for(j = 0; j < cs->length; j++)
	{
	  ay_trafo_apply3(&(controlv[i*cs->length*stride+j*stride]), mi);
	} /* for */

    } /* for */

  new->is_rat = ay_npt_israt(new);

  (void)ay_npt_closeu(new, 3);
  new->utype = AY_CTPERIODIC;
  ay_npt_setuvtypes(new, 2);

  /* return result */
  *sweep = new;

  /* prevent cleanup code from doing something harmful */
  new = NULL;

  /* clean up */
cleanup:

  if(cscv)
    free(cscv);
  if(trcv)
    free(trcv);
  if(sfcv)
    free(sfcv);

  if(new)
    {
      ay_npt_destroy(new);
    }

 return ay_status;
} /* ay_npt_sweepperiodic */


/* ay_npt_birail1:
 *  sweep cross section <o1> along rails <o2> and <o3>;
 *  Rotation code derived from J. Bloomenthals "Reference Frames"
 *  (Graphic Gems I).
 */
int
ay_npt_birail1(ay_object *o1, ay_object *o2, ay_object *o3, int sections,
	       int closed, ay_nurbpatch_object **birail1)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *new = NULL;
 ay_nurbcurve_object *cs, *r1, *r2;
 double *controlv = NULL;
 int i = 0, j = 0, a = 0, stride;
 double u, p1[4], p2[4], p5[4], p6[4], p7[4], p8[4];
 double T0[3] = {0.0,0.0,-1.0};
 double T1[3] = {0.0,0.0,0.0};
 double A[3] = {0.0,0.0,0.0};
 double lent0 = 0.0, lent1 = 0.0, plenr1 = 0.0, plenr2 = 0.0;
 double m[16] = {0}, mi[16] = {0}, mcs[16], mr1[16], mr2[16];
 double mr[16], mrs[16], axisrot[4] = {0};
 double *cscv = NULL, *r1cv = NULL, *r2cv = NULL;
 double scalx, scaly, scalz;

  if(!o1 || !o2 || !o3 || !birail1)
    return AY_ENULL;

  if((o1->type != AY_IDNCURVE) || (o2->type != AY_IDNCURVE) ||
     (o3->type != AY_IDNCURVE))
    return AY_ERROR;

  cs = (ay_nurbcurve_object *)(o1->refine);
  r1 = (ay_nurbcurve_object *)(o2->refine);
  r2 = (ay_nurbcurve_object *)(o3->refine);

  stride = 4;

  /* apply all transformations to cross-section curves controlv */
  if(!(cscv = malloc(cs->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(cscv, cs->controlv, cs->length * stride * sizeof(double));

  ay_trafo_creatematrix(o1, mcs);
  a = 0;
  for(i = 0; i < cs->length; i++)
    {
      ay_trafo_apply3(&(cscv[a]), mcs);
      a += stride;
    }

  /* apply all transformations to rail1 curves controlv */
  if(!(r1cv = malloc(r1->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(r1cv, r1->controlv, r1->length * stride * sizeof(double));

  ay_trafo_creatematrix(o2, mr1);
  a = 0;
  for(i = 0; i < r1->length; i++)
    {
      ay_trafo_apply3(&(r1cv[a]), mr1);
      a += stride;
    }

  /* apply all transformations to rail2 curves controlv */
  if(!(r2cv = malloc(r2->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(r2cv, r2->controlv, r2->length * stride * sizeof(double));

  ay_trafo_creatematrix(o3, mr2);
  a = 0;
  for(i = 0; i < r2->length; i++)
    {
      ay_trafo_apply3(&(r2cv[a]), mr2);
      a += stride;
    }

  /* allocate the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* calculate number of sections */
  if(sections <= 0)
    {
      if(r2->length > r1->length)
	{
	  a = r2->length;
	}
      else
	{
	  a = r1->length;
	}
      if(a == 2)
	{
	  sections = 1;
	}
      else
	{
	  sections = a + 1;
	}
      new->uorder = r1->order;
    }
  else
    {
      if(sections > 2)
	{
	  new->uorder = 4;
	}
      else
	{
	  new->uorder = sections+1;
	}
    } /* if */

  new->vorder = cs->order;
  new->uknot_type = AY_KTNURB;
  new->vknot_type = cs->knot_type;
  new->width = sections+1;
  new->height = cs->length;

  new->glu_sampling_tolerance = cs->glu_sampling_tolerance;

  /* allocate control point and knot arrays */
  if(!(controlv = malloc(new->width*new->height*stride*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  new->controlv = controlv;
  if(!(new->uknotv = malloc((new->width+new->uorder)*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(new->vknotv = malloc((new->height+new->vorder)* sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  ay_status = ay_knots_createnp(new);
  if(ay_status)
    { goto cleanup; }

  if(cs->knot_type == AY_KTCUSTOM)
    {
      memcpy(new->vknotv,cs->knotv,(cs->length+cs->order)*sizeof(double));
    }

  /* calculate first point of rail1 and rail2 */
  ay_nb_CurvePoint4D(r1->length-1, r1->order-1, r1->knotv, r1cv,
		     r1->knotv[r1->order-1], p1);

  ay_nb_CurvePoint4D(r2->length-1, r2->order-1, r2->knotv, r2cv,
		     r2->knotv[r2->order-1], p2);

  AY_V3SUB(T0, p2, p1)
  lent0 = AY_V3LEN(T0);
  AY_V3SCAL(T0,(1.0/lent0))
  memcpy(p5, p1, 3*sizeof(double));
  memcpy(p6, p2, 3*sizeof(double));

  plenr1 = fabs(r1->knotv[r1->length] - r1->knotv[r1->order-1]);
  plenr2 = fabs(r2->knotv[r2->length] - r2->knotv[r2->order-1]);

  ay_trafo_identitymatrix(mr);
  ay_trafo_identitymatrix(mrs);

  /* copy first section */
  memcpy(&(controlv[0]), &(cscv[0]), cs->length * stride * sizeof(double));

  /* copy cross sections controlv section times and sweep it along the rails */
  for(i = 1; i <= sections; i++)
    {
      memcpy(&(controlv[i * stride * cs->length]), &(cscv[0]),
	     cs->length * stride * sizeof(double));

      u = r1->knotv[r1->order-1]+(((double)i/sections)*plenr1);
      ay_nb_CurvePoint4D(r1->length-1, r1->order-1, r1->knotv, r1cv,
			 u, p1);
      u = r2->knotv[r2->order-1]+(((double)i/sections)*plenr2);
      ay_nb_CurvePoint4D(r2->length-1, r2->order-1, r2->knotv, r2cv,
			 u, p2);

      AY_V3SUB(T1, p2, p1);
      lent1 = AY_V3LEN(T1);
      AY_V3SCAL(T1,(1.0/lent1));

      /* create rotation matrix */
      if(((fabs(T1[0]-T0[0]) > AY_EPSILON) ||
	  (fabs(T1[1]-T0[1]) > AY_EPSILON) ||
	  (fabs(T1[2]-T0[2]) > AY_EPSILON)))
	{
	  ay_trafo_translatematrix(((p5[0])),
				   ((p5[1])),
				   ((p5[2])),
				   mr);
	  AY_V3CROSS(A,T0,T1);
	  lent0 = AY_V3LEN(A);
	  AY_V3SCAL(A,(1.0/lent0));

	  axisrot[0] = AY_R2D(acos(AY_V3DOT(T0,T1)));
	  memcpy(&axisrot[1], A, 3*sizeof(double));

	  if(fabs(axisrot[0]) > AY_EPSILON)
	    {
	      ay_trafo_rotatematrix(-axisrot[0], axisrot[1],
				    axisrot[2], axisrot[3], mr);
	      /*
		the mrs matrix is used to properly rotate the cross
		section curve for the calculation of the scale factors
	      */
	      ay_trafo_rotatematrix(-axisrot[0], axisrot[1],
				    axisrot[2], axisrot[3], mrs);

	    }
	  ay_trafo_translatematrix(-(p5[0]),
				   -(p5[1]),
				   -(p5[2]),
				   mr);
	} /* if */

      /* create transformation matrix */

      /* first, set it to identity */
      ay_trafo_identitymatrix(m);

      /* apply scaling */
      memcpy(p7, p1, 3*sizeof(double));
      memcpy(p8, p2, 3*sizeof(double));
      ay_trafo_apply3(p7, mrs);
      ay_trafo_apply3(p8, mrs);

      if(fabs(p6[0]-p5[0]) > AY_EPSILON)
        {
	  if(fabs(p8[0]-p7[0])/fabs(p6[0]-p5[0]) > AY_EPSILON)
	    scalx = fabs(p8[0]-p7[0])/fabs(p6[0]-p5[0]);
	  else
	    scalx = 1.0;
        }
      else
        {
	  scalx = 1.0;
	}

      if(fabs(p6[1]-p5[1]) > AY_EPSILON)
        {
	  if(fabs(p8[1]-p7[1])/fabs(p6[1]-p5[1]) > AY_EPSILON)
	    scaly = fabs(p8[1]-p7[1])/fabs(p6[1]-p5[1]);
	  else
	    scaly = 1.0;
        }
      else
        {
	  scaly = 1.0;
	}

      if(fabs(p6[2]-p5[2]) > AY_EPSILON)
        {
	  if(fabs(p8[2]-p7[2])/fabs(p6[2]-p5[2]) > AY_EPSILON)
	    scalz = fabs(p8[2]-p7[2])/fabs(p6[2]-p5[2]);
	  else
	    scalz = 1.0;
        }
      else
        {
	  scalz = 1.0;
	}

      ay_trafo_translatematrix(((p5[0])),
			       ((p5[1])),
			       ((p5[2])),
			       m);
      ay_trafo_scalematrix(1.0/scalx,
			   1.0/scaly,
			   1.0/scalz,
			   m);
      ay_trafo_translatematrix(-(p5[0]),
			       -(p5[1]),
			       -(p5[2]),
			       m);

      /* apply rotation */
      ay_trafo_multmatrix(m, mr);

      /* add translation */
      ay_trafo_translatematrix((p5[0]-p1[0]),
			       (p5[1]-p1[1]),
			       (p5[2]-p1[2]),
			       m);

      /* invert matrix */
      ay_trafo_invmatrix(m, mi);

      /* sweep profile */
      for(j = 0; j < cs->length; j++)
	{
	  ay_trafo_apply3(&controlv[i*cs->length*stride+j*stride], mi);
	} /* for */

      /* save rail vector for next iteration */
      memcpy(T0, T1, 3*sizeof(double));
    } /* for */

  if(closed)
    {
      /* make sure we create a closed surface (even if the
	 rails are not closed) by setting the first and
	 last section to a mean between the two */
      a = 0;
      j = sections*cs->length*stride;
      for(i = 0; i < cs->length; i++)
	{
	  controlv[a]   += ((controlv[j]-controlv[a])/2.0);
	  controlv[a+1] += ((controlv[j+1]-controlv[a+1])/2.0);
	  controlv[a+2] += ((controlv[j+2]-controlv[a+2])/2.0);

	  a += stride;
	  j += stride;
	} /* for */

      memcpy(&(controlv[sections*cs->length*stride]), controlv,
	     cs->length*stride*sizeof(double));
    }

  new->is_rat = ay_npt_israt(new);
  ay_npt_setuvtypes(new, 0);

  /* return result */
  *birail1 = new;

  /* prevent cleanup code from doing something harmful */
  new = NULL;

  /* clean up */
cleanup:

  if(cscv)
    free(cscv);
  if(r1cv)
    free(r1cv);
  if(r2cv)
    free(r2cv);

  if(new)
    {
      ay_npt_destroy(new);
    }

 return ay_status;
} /* ay_npt_birail1 */


/* ay_npt_birail1periodic:
 *  sweep cross section <o1> along rails <o2> and <o3>;
 *  Rotation code derived from J. Bloomenthals "Reference Frames"
 *  (Graphic Gems I).
 */
int
ay_npt_birail1periodic(ay_object *o1, ay_object *o2, ay_object *o3,
		       int sections,
		       ay_nurbpatch_object **birail1)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *new = NULL;
 ay_nurbcurve_object *cs, *r1, *r2;
 double *controlv = NULL;
 int i = 0, j = 0, a = 0, stride;
 double u, p1[4], p2[4], p5[4], p6[4], p7[4], p8[4];
 double T0[3] = {0.0,0.0,-1.0};
 double T1[3] = {0.0,0.0,0.0};
 double A[3] = {0.0,0.0,0.0};
 double lent0 = 0.0, lent1 = 0.0, plenr1 = 0.0, plenr2 = 0.0;
 double m[16] = {0}, mi[16] = {0}, mcs[16], mr1[16], mr2[16];
 double mr[16], mrs[16], axisrot[4] = {0};
 double *cscv = NULL, *r1cv = NULL, *r2cv = NULL;
 double scalx, scaly, scalz;

  if(!o1 || !o2 || !o3 || !birail1)
    return AY_ENULL;

  if((o1->type != AY_IDNCURVE) || (o2->type != AY_IDNCURVE) ||
     (o3->type != AY_IDNCURVE))
    return AY_ERROR;

  cs = (ay_nurbcurve_object *)(o1->refine);
  r1 = (ay_nurbcurve_object *)(o2->refine);
  r2 = (ay_nurbcurve_object *)(o3->refine);

  stride = 4;

  /* apply all transformations to cross-section curves controlv */
  if(!(cscv = malloc(cs->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(cscv, cs->controlv, cs->length * stride * sizeof(double));

  ay_trafo_creatematrix(o1, mcs);
  a = 0;
  for(i = 0; i < cs->length; i++)
    {
      ay_trafo_apply3(&(cscv[a]), mcs);
      a += stride;
    }

  /* apply all transformations to rail1 curves controlv */
  if(!(r1cv = malloc(r1->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(r1cv, r1->controlv, r1->length * stride * sizeof(double));

  ay_trafo_creatematrix(o2, mr1);
  a = 0;
  for(i = 0; i < r1->length; i++)
    {
      ay_trafo_apply3(&(r1cv[a]), mr1);
      a += stride;
    }

  /* apply all transformations to rail2 curves controlv */
  if(!(r2cv = malloc(r2->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(r2cv, r2->controlv, r2->length * stride * sizeof(double));

  ay_trafo_creatematrix(o3, mr2);
  a = 0;
  for(i = 0; i < r2->length; i++)
    {
      ay_trafo_apply3(&(r2cv[a]), mr2);
      a += stride;
    }

  /* allocate the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* calculate number of sections and uorder */
  if(sections <= 0)
    {
      if(r2->length > r1->length)
	{
	  a = r2->length;
	}
      else
	{
	  a = r1->length;
	}
      if(a == 2)
	{
	  sections = 1;
	}
      else
	{
	  sections = a + 1;
	}
      new->uorder = r1->order;
    }
  else
    {
      if(sections > 2)
	{
	  new->uorder = 4;
	}
      else
	{
	  new->uorder = sections+1;
	}
    } /* if */

  new->vorder = cs->order;
  new->uknot_type = AY_KTBSPLINE;
  new->vknot_type = cs->knot_type;
  new->width = sections+new->uorder-1;
  new->height = cs->length;

  new->glu_sampling_tolerance = cs->glu_sampling_tolerance;

  /* allocate control point and knot arrays */
  if(!(controlv = malloc(new->width*new->height*stride*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  new->controlv = controlv;
  if(!(new->uknotv = malloc((new->width+new->uorder)*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(new->vknotv = malloc((new->height+new->vorder)*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  ay_status = ay_knots_createnp(new);
  if(ay_status)
    { goto cleanup; }

  if(cs->knot_type == AY_KTCUSTOM)
    {
      memcpy(new->vknotv,cs->knotv,(cs->length+cs->order)*sizeof(double));
    }

  /* calculate first point of rail1 and rail2 */
  ay_nb_CurvePoint4D(r1->length-1, r1->order-1, r1->knotv, r1cv,
		     r1->knotv[r1->order-1], p1);

  ay_nb_CurvePoint4D(r2->length-1, r2->order-1, r2->knotv, r2cv,
		     r2->knotv[r2->order-1], p2);

  AY_V3SUB(T0, p2, p1)
  lent0 = AY_V3LEN(T0);
  AY_V3SCAL(T0,(1.0/lent0))
  memcpy(p5, p1, 3*sizeof(double));
  memcpy(p6, p2, 3*sizeof(double));

  plenr1 = fabs(r1->knotv[r1->length] - r1->knotv[r1->order-1]);
  plenr2 = fabs(r2->knotv[r2->length] - r2->knotv[r2->order-1]);

  ay_trafo_identitymatrix(mr);
  ay_trafo_identitymatrix(mrs);

  /* copy first section */
  memcpy(&(controlv[0]), &(cscv[0]), cs->length * stride * sizeof(double));

  /* copy cross sections controlv section times and sweep it along the rails */
  for(i = 1; i <= sections; i++)
    {
      memcpy(&(controlv[i * stride * cs->length]), &(cscv[0]),
	     cs->length * stride * sizeof(double));

      u = r1->knotv[r1->order-1]+(((double)i/sections)*plenr1);
      ay_nb_CurvePoint4D(r1->length-1, r1->order-1, r1->knotv, r1cv,
			 u, p1);
      u = r2->knotv[r2->order-1]+(((double)i/sections)*plenr2);
      ay_nb_CurvePoint4D(r2->length-1, r2->order-1, r2->knotv, r2cv,
			 u, p2);

      AY_V3SUB(T1, p2, p1);
      lent1 = AY_V3LEN(T1);
      AY_V3SCAL(T1,(1.0/lent1));

      /* create rotation matrix */
      if(((fabs(T1[0]-T0[0]) > AY_EPSILON) ||
	  (fabs(T1[1]-T0[1]) > AY_EPSILON) ||
	  (fabs(T1[2]-T0[2]) > AY_EPSILON)))
	{
	  ay_trafo_translatematrix(((p5[0])),
				   ((p5[1])),
				   ((p5[2])),
				   mr);
	  AY_V3CROSS(A,T0,T1);
	  lent0 = AY_V3LEN(A);
	  AY_V3SCAL(A,(1.0/lent0));

	  axisrot[0] = AY_R2D(acos(AY_V3DOT(T0,T1)));
	  memcpy(&axisrot[1], A, 3*sizeof(double));

	  if(fabs(axisrot[0]) > AY_EPSILON)
	    {
	      ay_trafo_rotatematrix(-axisrot[0], axisrot[1],
				    axisrot[2], axisrot[3], mr);
	      /*
		the mrs matrix is used to properly rotate the cross
		section curve for the calculation of the scale factors
	      */
	      ay_trafo_rotatematrix(-axisrot[0], axisrot[1],
				    axisrot[2], axisrot[3], mrs);

	    }
	  ay_trafo_translatematrix(-(p5[0]),
				   -(p5[1]),
				   -(p5[2]),
				   mr);
	} /* if */

      /* create transformation matrix */

      /* first, set it to identity */
      ay_trafo_identitymatrix(m);

      /* apply scaling */
      memcpy(p7, p1, 3*sizeof(double));
      memcpy(p8, p2, 3*sizeof(double));
      ay_trafo_apply3(p7, mrs);
      ay_trafo_apply3(p8, mrs);

      if(fabs(p6[0]-p5[0]) > AY_EPSILON)
        {
	  if(fabs(p8[0]-p7[0])/fabs(p6[0]-p5[0]) > AY_EPSILON)
	    scalx = fabs(p8[0]-p7[0])/fabs(p6[0]-p5[0]);
	  else
	    scalx = 1.0;
        }
      else
        {
	  scalx = 1.0;
	}

      if(fabs(p6[1]-p5[1]) > AY_EPSILON)
        {
	  if(fabs(p8[1]-p7[1])/fabs(p6[1]-p5[1]) > AY_EPSILON)
	    scaly = fabs(p8[1]-p7[1])/fabs(p6[1]-p5[1]);
	  else
	    scaly = 1.0;
        }
      else
        {
	  scaly = 1.0;
	}

      if(fabs(p6[2]-p5[2]) > AY_EPSILON)
        {
	  if(fabs(p8[2]-p7[2])/fabs(p6[2]-p5[2]) > AY_EPSILON)
	    scalz = fabs(p8[2]-p7[2])/fabs(p6[2]-p5[2]);
	  else
	    scalz = 1.0;
        }
      else
        {
	  scalz = 1.0;
	}

      ay_trafo_translatematrix(((p5[0])),
			       ((p5[1])),
			       ((p5[2])),
			       m);
      ay_trafo_scalematrix(1.0/scalx,
			   1.0/scaly,
			   1.0/scalz,
			   m);
      ay_trafo_translatematrix(-(p5[0]),
			       -(p5[1]),
			       -(p5[2]),
			       m);

      /* apply rotation */
      ay_trafo_multmatrix(m, mr);

      /* add translation */
      ay_trafo_translatematrix((p5[0]-p1[0]),
			       (p5[1]-p1[1]),
			       (p5[2]-p1[2]),
			       m);

      /* invert matrix */
      ay_trafo_invmatrix(m, mi);

      /* sweep profile */
      for(j = 0; j < cs->length; j++)
	{
	  ay_trafo_apply3(&controlv[i*cs->length*stride+j*stride], mi);
	} /* for */

      /* save rail vector for next iteration */
      memcpy(T0, T1, 3*sizeof(double));
    } /* for */


  new->is_rat = ay_npt_israt(new);

  (void)ay_npt_closeu(new, 3);
  new->utype = AY_CTPERIODIC;
  ay_npt_setuvtypes(new, 2);

  /* return result */
  *birail1 = new;

  /* prevent cleanup code from doing something harmful */
  new = NULL;

  /* clean up */
cleanup:

  if(cscv)
    free(cscv);
  if(r1cv)
    free(r1cv);
  if(r2cv)
    free(r2cv);

  if(new)
    {
      ay_npt_destroy(new);
    }

 return ay_status;
} /* ay_npt_birail1periodic */


/* ay_npt_birail2:
 *  sweep cross section curve <o1> along rail curves <o2> and <o3>
 *  morphing it into cross section curve <o4>, interpolation control
 *  may be present using curve <o5>;
 *  Rotation code derived from J. Bloomenthals "Reference Frames"
 *  (Graphic Gems I).
 */
int
ay_npt_birail2(ay_object *o1, ay_object *o2, ay_object *o3, ay_object *o4,
	       ay_object *o5,
	       int sections, int closed, int fullinterpolctrl,
	       ay_nurbpatch_object **birail2)
{
 int ay_status = AY_OK;
 ay_object *curve = NULL;
 ay_nurbpatch_object *new = NULL;
 ay_nurbcurve_object *cs1, *cs2, *r1, *r2, *ic = NULL;
 double *controlv = NULL;
 int i = 0, j = 0, a = 0, stride, incompatible = AY_FALSE;
 double u, p1[4], p2[4], p5[4], p6[4], p7[4], p8[4], p9[4], p10[4];
 double T0[3] = {0.0,0.0,-1.0};
 double T1[3] = {0.0,0.0,0.0};
 double A[3] = {0.0,0.0,0.0};
 double lent0 = 0.0, lent1 = 0.0, lentn = 0.0, plenr1 = 0.0, plenr2 = 0.0;
 double plenic = 0.0;
 double m[16] = {0}, mi[16] = {0}, mcs[16], mr1[16], mr2[16];
 double mr[16], mrs[16], axisrot[4];
 /*double quat[4] = {0};*/
 double *cs1cv = NULL, *cs2cv = NULL, *r1cv = NULL, *r2cv = NULL;
 double *cs2cvi = NULL, *iccv = NULL;
 double scalx, scaly, scalz;
 double rotv[4] = {0};

  stride = 4;

  if(!o1 || !o2 || !o3 || !o4 || !birail2)
    return AY_ENULL;

  if((o1->type != AY_IDNCURVE) || (o2->type != AY_IDNCURVE) ||
     (o3->type != AY_IDNCURVE) || (o4->type != AY_IDNCURVE) ||
     (o5 && o5->type != AY_IDNCURVE))
    return AY_ERROR;

  cs1 = (ay_nurbcurve_object *)(o1->refine);
  r1 = (ay_nurbcurve_object *)(o2->refine);
  r2 = (ay_nurbcurve_object *)(o3->refine);
  cs2 = (ay_nurbcurve_object *)(o4->refine);

  /* check, whether cross section curves are compatible */
  if((cs1->length != cs2->length) || (cs1->order != cs2->order))
    {
      incompatible = AY_TRUE;
    }
  else
    {
      /* check knot vectors */
      for(i = 0; i < (cs1->length+cs1->order); i++)
	{
	  if(fabs(cs1->knotv[i]-cs2->knotv[i]) > AY_EPSILON)
	    {
	      incompatible = AY_TRUE;
	      break;
	    }
	} /* for */
    } /* if */

  if(incompatible)
    {
      /* make curves compatible */
      ay_status = ay_object_copy(o1, &curve);
      if(ay_status)
	return ay_status;
      ay_status = ay_object_copy(o4, &(curve->next));
      if(ay_status)
	{
	  (void)ay_object_delete(curve);
	  return ay_status;
	}
      ay_status = ay_nct_makecompatible(curve);
      if(ay_status)
	{
	  (void)ay_object_deletemulti(curve, AY_TRUE);
	  return ay_status;
	}
      o1 = curve;
      cs1 = (ay_nurbcurve_object *)(o1->refine);
      o4 = curve->next;
      cs2 = (ay_nurbcurve_object *)(o4->refine);
      curve = NULL;
    } /* if */

  /* do we have an interpolation control curve? */
  if(o5)
    {
      /* yes */
      ic = (ay_nurbcurve_object *)(o5->refine);
      /* apply all transformations to interpolation control curves controlv */
      if(!(iccv = malloc(ic->length * stride * sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      memcpy(iccv, ic->controlv, ic->length * stride * sizeof(double));

      ay_trafo_creatematrix(o5, mr1);
      a = 0;
      for(i = 0; i < ic->length; i++)
	{
	  ay_trafo_apply3(&(iccv[a]), mr1);
	  a += stride;
	}
      plenic = fabs(ic->knotv[ic->length] - ic->knotv[ic->order-1]);
    }  /* if */

  /* apply all transformations to first cross-section curves controlv */
  if(!(cs1cv = malloc(cs1->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(cs1cv, cs1->controlv, cs1->length * stride * sizeof(double));

  ay_trafo_creatematrix(o1, mcs);
  a = 0;
  for(i = 0; i < cs1->length; i++)
    {
      ay_trafo_apply3(&(cs1cv[a]), mcs);
      a += stride;
    }

  /* apply all transformations to rail1 curves controlv */
  if(!(r1cv = malloc(r1->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(r1cv, r1->controlv, r1->length * stride * sizeof(double));

  ay_trafo_creatematrix(o2, mr1);
  a = 0;
  for(i = 0; i < r1->length; i++)
    {
      ay_trafo_apply3(&(r1cv[a]), mr1);
      a += stride;
    }

  /* apply all transformations to rail2 curves controlv */
  if(!(r2cv = malloc(r2->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(r2cv, r2->controlv, r2->length * stride * sizeof(double));

  ay_trafo_creatematrix(o3, mr2);
  a = 0;
  for(i = 0; i < r2->length; i++)
    {
      ay_trafo_apply3(&(r2cv[a]), mr2);
      a += stride;
    }

  /* apply all transformations to second cross-section curves controlv */
  if(!(cs2cv = malloc(cs2->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(cs2cv, cs2->controlv, cs2->length * stride * sizeof(double));

  ay_trafo_creatematrix(o4, mcs);
  a = 0;
  for(i = 0; i < cs2->length; i++)
    {
      ay_trafo_apply3(&(cs2cv[a]), mcs);
      a += stride;
    }

  /* get scratch memory for interpolated curve */
  if(!(cs2cvi = malloc(cs2->length * stride * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* allocate the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* calculate number of sections */
  if(sections <= 0)
    {
      if(r2->length > r1->length)
	{
	  a = r2->length;
	}
      else
	{
	  a = r1->length;
	}
      if(a == 2)
	{
	  sections = 1;
	}
      else
	{
	  sections = a + 1;
	}
     new->uorder = r1->order;
    }
  else
    {
      if(sections > 2)
	{
	  new->uorder = 4;
	}
      else
	{
	  new->uorder = sections+1;
	}
    } /* if */

  new->vorder = cs1->order;
  new->uknot_type = AY_KTNURB;
  new->vknot_type = cs1->knot_type;
  new->width = sections+1;
  new->height = cs1->length;

  new->glu_sampling_tolerance = cs1->glu_sampling_tolerance;

  /* allocate control point and knot arrays */
  if(!(controlv = malloc(new->width*new->height*stride*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  new->controlv = controlv;
  if(!(new->uknotv = malloc((new->width+new->uorder) * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(new->vknotv = malloc((new->height+new->vorder) * sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  ay_status = ay_knots_createnp(new);
  if(ay_status)
    { goto cleanup; }

  if(cs1->knot_type == AY_KTCUSTOM)
    {
      memcpy(new->vknotv,cs1->knotv,(cs1->length+cs1->order)*sizeof(double));
    }

  /* copy last section */
  if(fullinterpolctrl && o5)
    {
      u = ic->knotv[ic->length];
      ay_nb_CurvePoint4D(ic->length-1, ic->order-1, ic->knotv, iccv,
			 u, p10);
      ay_status = ay_interpol_1DA4D(p10[1], cs1->length,
				    cs1cv, cs2cv, cs2cvi);
      if(ay_status)
	{ goto cleanup; }
      memcpy(&(controlv[sections * cs2->length * stride]), &(cs2cvi[0]),
	     cs2->length * stride * sizeof(double));
    }
  else
    {
      memcpy(&(controlv[sections * cs2->length * stride]), &(cs2cv[0]),
	     cs2->length * stride * sizeof(double));
    }

  /* calculate last point of rail1 and rail2 */
  ay_nb_CurvePoint4D(r1->length-1, r1->order-1, r1->knotv, r1cv,
		     r1->knotv[r1->length], p7);
  memcpy(p9, p7, 3*sizeof(double));

  ay_nb_CurvePoint4D(r2->length-1, r2->order-1, r2->knotv, r2cv,
		     r2->knotv[r2->length], p8);
  /* from rail endpoints calculate distance between them
     (this is needed to calculate the correct scale factor
     for the interpolated cross sections later on) */
  AY_V3SUB(T1, p8, p7)
  lentn = AY_V3LEN(T1);
  AY_V3SCAL(T1,(1.0/lentn))

  /* calculate first point of rail1 and rail2 */
  ay_nb_CurvePoint4D(r1->length-1, r1->order-1, r1->knotv, r1cv,
		     r1->knotv[r1->order-1], p1);

  ay_nb_CurvePoint4D(r2->length-1, r2->order-1, r2->knotv, r2cv,
		     r2->knotv[r2->order-1], p2);

  AY_V3SUB(T0, p2, p1)
  lent0 = AY_V3LEN(T0);
  AY_V3SCAL(T0,(1.0/lent0))
  memcpy(p5, p1, 3*sizeof(double));
  memcpy(p6, p2, 3*sizeof(double));

  plenr1 = fabs(r1->knotv[r1->length] - r1->knotv[r1->order-1]);
  plenr2 = fabs(r2->knotv[r2->length] - r2->knotv[r2->order-1]);

  /* transform control points of second cross-section back to
     first section (using the end vectors of the rail curves) */
  ay_trafo_identitymatrix(mcs);
  ay_trafo_identitymatrix(mr);

  if(((fabs(T1[0]-T0[0]) > AY_EPSILON) ||
      (fabs(T1[1]-T0[1]) > AY_EPSILON) ||
      (fabs(T1[2]-T0[2]) > AY_EPSILON)))
    {
      AY_V3CROSS(A,T0,T1);
      lent0 = AY_V3LEN(A);
      AY_V3SCAL(A,(1.0/lent0));

      rotv[0] = AY_R2D(acos(AY_V3DOT(T0,T1)));
      memcpy(&rotv[1], A, 3*sizeof(double));

      if(fabs(rotv[0]) > AY_EPSILON)
        {
	  ay_trafo_rotatematrix(-rotv[0], rotv[1],
				rotv[2], rotv[3], mr);
        } /* if */
    } /* if */

  ay_trafo_apply3(p7, mr);
  ay_trafo_apply3(p8, mr);

  if(fabs(p6[0]-p5[0]) > AY_EPSILON)
    {
      if(fabs(p8[0]-p7[0])/fabs(p6[0]-p5[0]) > AY_EPSILON)
	  scalx = fabs(p8[0]-p7[0])/fabs(p6[0]-p5[0]);
      else
	  scalx = 1.0;
    }
  else
    {
      scalx = 1.0;
    }

  if(fabs(p6[1]-p5[1]) > AY_EPSILON)
    {
      if(fabs(p8[1]-p7[1])/fabs(p6[1]-p5[1]) > AY_EPSILON)
	scaly = fabs(p8[1]-p7[1])/fabs(p6[1]-p5[1]);
      else
	scaly = 1.0;
    }
  else
    {
      scaly = 1.0;
    }

  if(fabs(p6[2]-p5[2]) > AY_EPSILON)
    {
      if(fabs(p8[2]-p7[2])/fabs(p6[2]-p5[2]) > AY_EPSILON)
	  scalz = fabs(p8[2]-p7[2])/fabs(p6[2]-p5[2]);
      else
	  scalz = 1.0;
    }
  else
    {
      scalz = 1.0;
    }

  ay_trafo_translatematrix(((p5[0])),
			   ((p5[1])),
			   ((p5[2])),
			   mcs);

  ay_trafo_scalematrix(1.0/scalx,
		       1.0/scaly,
		       1.0/scalz,
		       mcs);

  ay_trafo_translatematrix(-(p5[0]),
			   -(p5[1]),
			   -(p5[2]),
			   mcs);

  if(fabs(rotv[0]) > AY_EPSILON)
    {
      ay_trafo_translatematrix(((p5[0])),
			       ((p5[1])),
			       ((p5[2])),
			       mcs);

      ay_trafo_rotatematrix(-rotv[0], rotv[1],
			    rotv[2], rotv[3], mcs);

      ay_trafo_translatematrix(-(p5[0]),
			       -(p5[1]),
			       -(p5[2]),
			       mcs);
    } /* if */

  ay_trafo_translatematrix(-(p9[0]-p1[0]),
			   -(p9[1]-p1[1]),
			   -(p9[2]-p1[2]),
			   mcs);

  for(j = 0; j < cs2->length; j++)
    {
      ay_trafo_apply3(&cs2cv[j*stride], mcs);
    } /* for */

  ay_trafo_identitymatrix(mr);
  ay_trafo_identitymatrix(mrs);

  /* copy first section */
  if(fullinterpolctrl && o5)
    {
      u = ic->knotv[ic->order-1];
      ay_nb_CurvePoint4D(ic->length-1, ic->order-1, ic->knotv, iccv,
			 u, p10);
      ay_status = ay_interpol_1DA4D(p10[1], cs1->length,
				    cs1cv, cs2cv, cs2cvi);
      if(ay_status)
	{ goto cleanup; }
      memcpy(&(controlv[0]), cs2cvi, cs1->length * stride * sizeof(double));
    }
  else
    {
      memcpy(&(controlv[0]), &(cs1cv[0]),
	     cs1->length * stride * sizeof(double));
    }

  /* copy cross sections controlv n times and sweep it along the rails */
  for(i = 1; i < ((fullinterpolctrl && o5)?(sections+1):sections); i++)
    {
      if(o5)
	{
	  u = ic->knotv[ic->order-1]+(((double)i/sections)*plenic);
	  ay_nb_CurvePoint4D(ic->length-1, ic->order-1, ic->knotv, iccv,
			     u, p10);
	  ay_status = ay_interpol_1DA4D(p10[1], cs1->length,
					cs1cv, cs2cv, cs2cvi);

	}
      else
	{
	  ay_status = ay_interpol_1DA4D((double)i/sections, cs1->length,
					cs1cv, cs2cv, cs2cvi);
	}
      if(ay_status)
	{ goto cleanup; }

      memcpy(&(controlv[i * stride * cs1->length]), cs2cvi,
	     cs1->length * stride * sizeof(double));

      u = r1->knotv[r1->order-1]+(((double)i/sections)*plenr1);
      ay_nb_CurvePoint4D(r1->length-1, r1->order-1, r1->knotv, r1cv,
			 u, p1);
      u = r2->knotv[r2->order-1]+(((double)i/sections)*plenr2);
      ay_nb_CurvePoint4D(r2->length-1, r2->order-1, r2->knotv, r2cv,
			 u, p2);

      AY_V3SUB(T1, p2, p1)
      lent1 = AY_V3LEN(T1);
      AY_V3SCAL(T1,(1.0/lent1));

      /* create rotation matrix */
      if(((fabs(T1[0]-T0[0]) > AY_EPSILON) ||
	  (fabs(T1[1]-T0[1]) > AY_EPSILON) ||
	  (fabs(T1[2]-T0[2]) > AY_EPSILON)))
	{
	  ay_trafo_translatematrix(((p5[0])),
				   ((p5[1])),
				   ((p5[2])),
				   mr);

	  AY_V3CROSS(A,T0,T1);
	  lent0 = AY_V3LEN(A);
	  AY_V3SCAL(A,(1.0/lent0));

	  axisrot[0] = AY_R2D(acos(AY_V3DOT(T0,T1)));
	  memcpy(&axisrot[1], A, 3*sizeof(double));

	  if(fabs(axisrot[0]) > AY_EPSILON)
	    {
	      ay_trafo_rotatematrix(-axisrot[0], axisrot[1],
				    axisrot[2], axisrot[3], mr);
	      /*
		the mrs matrix is used to properly rotate the cross
		section curve for the calculation of the scale factors
	      */
	      ay_trafo_rotatematrix(-axisrot[0], axisrot[1],
				    axisrot[2], axisrot[3], mrs);

	    }
	  ay_trafo_translatematrix(-(p5[0]),
				   -(p5[1]),
				   -(p5[2]),
				   mr);
	} /* if */

      /* create transformation matrix */

      /* first, set it to identity */
      ay_trafo_identitymatrix(m);

      /* apply scaling */
      memcpy(p7, p1, 3*sizeof(double));
      memcpy(p8, p2, 3*sizeof(double));
      ay_trafo_apply3(p7, mrs);
      ay_trafo_apply3(p8, mrs);

      if(fabs(p6[0]-p5[0]) > AY_EPSILON)
        {
	  if(fabs(p8[0]-p7[0])/fabs(p6[0]-p5[0]) > AY_EPSILON)
	    scalx = fabs(p8[0]-p7[0])/fabs(p6[0]-p5[0]);
	  else
	    scalx = 1.0;
        }
      else
        {
	  scalx = 1.0;
	}

      if(fabs(p6[1]-p5[1]) > AY_EPSILON)
        {
	  if(fabs(p8[1]-p7[1])/fabs(p6[1]-p5[1]) > AY_EPSILON)
	    scaly = fabs(p8[1]-p7[1])/fabs(p6[1]-p5[1]);
	  else
	    scaly = 1.0;
        }
      else
        {
	  scaly = 1.0;
	}

      if(fabs(p6[2]-p5[2]) > AY_EPSILON)
        {
	  if(fabs(p8[2]-p7[2])/fabs(p6[2]-p5[2]) > AY_EPSILON)
	    scalz = fabs(p8[2]-p7[2])/fabs(p6[2]-p5[2]);
	  else
	    scalz = 1.0;
        }
      else
        {
	  scalz = 1.0;
	}

      ay_trafo_translatematrix(((p5[0])),
			       ((p5[1])),
			       ((p5[2])),
			       m);
      ay_trafo_scalematrix(1.0/scalx,
			   1.0/scaly,
			   1.0/scalz,
			   m);
      ay_trafo_translatematrix(-(p5[0]),
			       -(p5[1]),
			       -(p5[2]),
			       m);


      /* apply rotation */
      ay_trafo_multmatrix(m, mr);

      /* add translation */
      ay_trafo_translatematrix((p5[0]-p1[0]),
			       (p5[1]-p1[1]),
			       (p5[2]-p1[2]),
			       m);

      /* invert matrix */
      ay_trafo_invmatrix(m, mi);

      /* sweep profile */
      for(j = 0; j < cs1->length; j++)
	{
	  ay_trafo_apply3(&controlv[i*cs1->length*stride+j*stride], mi);
	} /* for */

      /* save rail vector for next iteration */
      memcpy(T0, T1, 3*sizeof(double));
    } /* for */

  new->is_rat = ay_npt_israt(new);
  ay_npt_setuvtypes(new, 0);

  /* return result */
  *birail2 = new;

  /* prevent cleanup code from doing something harmful */
  new = NULL;

  /* clean up */
cleanup:

  if(cs1cv)
    free(cs1cv);
  if(r1cv)
    free(r1cv);
  if(r2cv)
    free(r2cv);
  if(cs2cv)
    free(cs2cv);
  if(cs2cvi)
    free(cs2cvi);
  if(iccv)
    free(iccv);
  if(new)
    {
      ay_npt_destroy(new);
    }

  if(incompatible)
    {
      (void)ay_object_delete(o1);
      (void)ay_object_delete(o4);
    }

 return ay_status;
} /* ay_npt_birail2 */


/* ay_npt_skinu:
 *
 *
 */
int
ay_npt_skinu(ay_object *curves, int order, int knot_type,
	     int interpolate, ay_nurbpatch_object **skin)
{
 int ay_status = AY_OK;
 ay_object *o = NULL, *o2 = NULL;
 ay_nurbcurve_object *curve = NULL, *c1 = NULL, *c2 = NULL;
 int numcurves = 0;
 int stride = 4, K, N, i, b, k, degU, ind;
 double *skc = NULL, *U = NULL, *uk = NULL, *d = NULL, *V = NULL;
 double v[3] = {0};

  if(!curves || !skin)
    return AY_ENULL;

  i = AY_TRUE;
  ay_status = ay_nct_iscompatible(curves, &i);
  if(ay_status)
    {goto cleanup;}

  if(!i)
    {
      ay_status = ay_nct_makecompatible(curves);
      if(ay_status)
	{goto cleanup;}
    }

  o = curves;
  while(o)
    {
      numcurves++;
      o = o->next;
    }

  curve = (ay_nurbcurve_object*)(curves->refine);
  if(!(V = malloc((curve->length+curve->order)*sizeof(double))))
    {ay_status = AY_EOMEM; goto cleanup;}
  memcpy(V, curve->knotv, (curve->length+curve->order)*sizeof(double));

  K = numcurves;
  N = curve->length;

  if(order < 2)
    order = 3;
  if(order > numcurves)
    order = numcurves;
  degU = order-1;

  if(knot_type == AY_KTBEZIER)
    if(order < numcurves)
      order = numcurves;

  if(knot_type == AY_KTCUSTOM)
    {
      /* knot averaging */
      if(!(uk = calloc(numcurves, sizeof(double))))
	{ay_status = AY_EOMEM; goto cleanup;}
      if(!(d = calloc(curve->length, sizeof(double))))
	{ay_status = AY_EOMEM; goto cleanup;}
      if(!(U = calloc(numcurves+order, sizeof(double))))
	{ay_status = AY_EOMEM; goto cleanup;}

      for(i = 0; i < N; i++)
	{
	  d[i] = 0;
	  o = curves;
	  c1 = (ay_nurbcurve_object*)(o->refine);
	  ind = i*stride;
	  for(k = 1; k < K; k++)
	    {
	      o2 = o->next;
	      c2 = (ay_nurbcurve_object*)(o2->refine);
	      v[0] = c2->controlv[ind]   - c1->controlv[ind];
	      v[1] = c2->controlv[ind+1] - c1->controlv[ind+1];
	      v[2] = c2->controlv[ind+2] - c1->controlv[ind+2];
	      d[i] += AY_V3LEN(v);
	      o = o2;
	      c1 = c2;
	    } /* for */
	} /* for */

      o = curves;
      c1 = (ay_nurbcurve_object*)(o->refine);
      uk[0] = 0;
      for(k = 1; k < K; k++)
	{
	  o2 = o->next;
	  c2 = (ay_nurbcurve_object*)(o2->refine);
	  uk[k] = 0;
	  ind = 0;
	  for(i = 0; i < N; i++)
	    {
	      if(d[i] > AY_EPSILON)
		{
		  v[0] = (c2->controlv[ind]   - c1->controlv[ind])/d[i];
		  v[1] = (c2->controlv[ind+1] - c1->controlv[ind+1])/d[i];
		  v[2] = (c2->controlv[ind+2] - c1->controlv[ind+2])/d[i];
		  uk[k] += AY_V3LEN(v);
		}
	      ind += stride;
	    } /* for */
	  uk[k] /= N;
	  uk[k] += uk[k-1];
	  o = o2;
	  c1 = c2;
	} /* for */
      uk[numcurves-1] = 1.0;

      for(i = 1; i < K-degU; i++)
	{
	  U[i+degU] = 0.0;
	  for(k = i; k < i+degU; k++)
	    {
	      U[i+degU] += uk[k];
	    }
	  U[i+degU] /= degU;
	} /* for */
      for(i = 0; i <= degU; i++)
	U[i] = 0.0;
      for(i = K; i <= K+degU; i++)
	U[i] = 1.0;
    } /* if */

  if(U && ay_knots_check(order, numcurves, order+numcurves, U))
    {ay_status = AY_ERROR; goto cleanup;}

  /* construct patch */
  o = curves;
  curve = (ay_nurbcurve_object *) o->refine;

  if(!(skc = malloc((curve->length * numcurves * 4) * sizeof(double))))
    {ay_status = AY_EOMEM; goto cleanup;}

  b = 0;
  while(o)
    {
      curve = (ay_nurbcurve_object *) o->refine;

      memcpy(&(skc[b]), curve->controlv, N*stride*sizeof(double));
      b += N*stride;
      o = o->next;
    } /* while */

  ay_status = ay_npt_create(order, curve->order,
			    numcurves, curve->length,
			    knot_type, AY_KTCUSTOM,
			    skc, U, V, skin);

  if(ay_status)
    {goto cleanup;}

  /* prevent cleanup code from doing something harmful */
  U = NULL;
  V = NULL;
  skc = NULL;

  if(interpolate && degU > 1)
    ay_status = ay_ipt_interpolateu(*skin, order, AY_KTCHORDAL);

cleanup:
  if(uk)
    free(uk);
  if(d)
    free(d);
  if(U)
    free(U);
  if(V)
    free(V);
  if(skc)
    free(skc);

 return ay_status;
} /* ay_npt_skinu */


/* ay_npt_skinv:
 *
 *
 */
int
ay_npt_skinv(ay_object *curves, int order, int knot_type,
	     int interpolate, ay_nurbpatch_object **skin)
{
 int ay_status = AY_OK;
 ay_object *o = NULL, *o2 = NULL;
 ay_nurbcurve_object *curve = NULL, *c1 = NULL, *c2 = NULL;
 int numcurves = 0;
 int stride = 4, K, N, i, a, b, c, k, degV, ind;
 double *skc = NULL, *U = NULL, *vk = NULL, *d = NULL, *V = NULL;
 double v[3] = {0};

  if(!curves || !skin)
    return AY_ENULL;

  i = AY_TRUE;
  ay_status = ay_nct_iscompatible(curves, &i);
  if(ay_status)
    {goto cleanup;}

  if(!i)
    {
      ay_status = ay_nct_makecompatible(curves);
      if(ay_status)
	{goto cleanup;}
    }

  o = curves;
  while(o)
    {
      numcurves++;
      o = o->next;
    }

  curve = (ay_nurbcurve_object*)(curves->refine);
  if(!(U = malloc((curve->length+curve->order)*sizeof(double))))
    {ay_status = AY_EOMEM; goto cleanup;}
  memcpy(U, curve->knotv, (curve->length+curve->order)*sizeof(double));

  K = numcurves;
  N = curve->length;

  if(order < 2)
    order = 3;
  if(order > numcurves)
    order = numcurves;
  degV = order-1;

  if(knot_type == AY_KTBEZIER)
    if(order < numcurves)
      order = numcurves;

  if(knot_type == AY_KTCUSTOM)
    {
      /* knot averaging */
      if(!(vk = calloc(numcurves, sizeof(double))))
	{ay_status = AY_EOMEM; goto cleanup;}
      if(!(d = calloc(curve->length, sizeof(double))))
	{ay_status = AY_EOMEM; goto cleanup;}
      if(!(V = calloc(numcurves+order, sizeof(double))))
	{ay_status = AY_EOMEM; goto cleanup;}

      for(i = 0; i < N; i++)
	{
	  d[i] = 0;
	  o = curves;
	  c1 = (ay_nurbcurve_object*)(o->refine);
	  ind = i*stride;
	  for(k = 1; k < K; k++)
	    {
	      o2 = o->next;
	      c2 = (ay_nurbcurve_object*)(o2->refine);
	      v[0] = c2->controlv[ind]   - c1->controlv[ind];
	      v[1] = c2->controlv[ind+1] - c1->controlv[ind+1];
	      v[2] = c2->controlv[ind+2] - c1->controlv[ind+2];
	      d[i] += AY_V3LEN(v);
	      o = o2;
	      c1 = c2;
	    } /* for */
	} /* for */

      o = curves;
      c1 = (ay_nurbcurve_object*)(o->refine);
      vk[0] = 0;
      for(k = 1; k < K; k++)
	{
	  o2 = o->next;
	  c2 = (ay_nurbcurve_object*)(o2->refine);
	  vk[k] = 0;
	  ind = 0;
	  for(i = 0; i < N; i++)
	    {
	      if(d[i] > AY_EPSILON)
		{
		  v[0] = (c2->controlv[ind]   - c1->controlv[ind])/d[i];
		  v[1] = (c2->controlv[ind+1] - c1->controlv[ind+1])/d[i];
		  v[2] = (c2->controlv[ind+2] - c1->controlv[ind+2])/d[i];
		  vk[k] += AY_V3LEN(v);
		}
	      ind += stride;
	    } /* for */
	  vk[k] /= N;
	  vk[k] += vk[k-1];
	  o = o2;
	  c1 = c2;
	} /* for */
      vk[numcurves-1] = 1.0;

      for(i = 1; i < K-degV; i++)
	{
	  V[i+degV] = 0.0;
	  for(k = i; k < i+degV; k++)
	    {
	      V[i+degV] += vk[k];
	    }
	  V[i+degV] /= degV;
	} /* for */
      for(i = 0; i <= degV; i++)
	V[i] = 0.0;
      for(i = K; i <= K+degV; i++)
	V[i] = 1.0;
    } /* if */

  if(V && ay_knots_check(order, numcurves, order+numcurves, V))
    {ay_status = AY_ERROR; goto cleanup;}

  /* construct patch */
  o = curves;
  curve = (ay_nurbcurve_object *) o->refine;

  if(!(skc = malloc((curve->length * numcurves * stride)*sizeof(double))))
    {ay_status = AY_EOMEM; goto cleanup;}

  c = 0;
  while(o)
    {
      curve = (ay_nurbcurve_object *) o->refine;

      b = c*stride;
      a = 0;
      for(i = 0; i < curve->length; i++)
	{
	  memcpy(&(skc[b]), &(curve->controlv[a]), stride*sizeof(double));
	  b += K*stride;
	  a += stride;
	} /* for */
      c++;
      o = o->next;
    } /* while */

  ay_status = ay_npt_create(curve->order, order,
			    curve->length, numcurves,
			    AY_KTCUSTOM, knot_type,
			    skc, U, V, skin);

  if(ay_status)
    {goto cleanup;}

  /* prevent cleanup code from doing something harmful */
  U = NULL;
  V = NULL;
  skc = NULL;

  if(interpolate && degV > 1)
    ay_status = ay_ipt_interpolatev(*skin, order, AY_KTCHORDAL);

cleanup:
  if(vk)
    free(vk);
  if(d)
    free(d);
  if(U)
    free(U);
  if(V)
    free(V);
  if(skc)
    free(skc);

 return ay_status;
} /* ay_npt_skinv */


/* ay_npt_extrude:
 *
 *
 */
int
ay_npt_extrude(double height, ay_object *o, ay_nurbpatch_object **extrusion)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *new = NULL;
 ay_nurbcurve_object *curve;
 double uknots[4] = {0.0, 0.0, 1.0, 1.0}; /* vknots are taken from curve! */
 double *uknotv = NULL, *vknotv = NULL, *controlv = NULL;
 int j = 0, a = 0, b = 0, c = 0;
 double m[16], point[4] = {0};

  if(!o || !extrusion)
    return AY_OK;

  if(o->type != AY_IDNCURVE)
    return AY_ERROR;

  curve = (ay_nurbcurve_object *)(o->refine);

  /* get curves transformation-matrix */
  ay_trafo_creatematrix(o, m);

  /* calloc the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    return AY_EOMEM;
  if(!(controlv = malloc(4*4*curve->length*sizeof(double))))
    { free(new); return AY_EOMEM; }
  if(!(uknotv = malloc(4*sizeof(double))))
    { free(new); free(controlv); return AY_EOMEM; }
  if(!(vknotv = malloc((curve->length+curve->order)*sizeof(double))))
    { free(new); free(controlv); free(uknotv); return AY_EOMEM; }

  memcpy(vknotv,curve->knotv,(curve->length+curve->order)*sizeof(double));
  new->vknotv = vknotv;
  memcpy(uknotv,uknots,4*sizeof(double));
  new->uknotv = uknotv;
  new->vorder = curve->order;
  new->uorder = 2; /* linear! */
  new->vknot_type = curve->knot_type;
  new->uknot_type = AY_KTNURB;
  new->width = 2;
  new->height = curve->length;
  new->glu_sampling_tolerance = curve->glu_sampling_tolerance;
  new->is_rat = curve->is_rat;
  new->vtype = curve->type;

  /* fill controlv */
  a = 0;
  b = 0;
  c = curve->length*4;
  for(j = 0; j < curve->length; j++)
    {
      /* transform point */
      point[0] = m[0]*curve->controlv[a] +
	m[4]*curve->controlv[a+1] +
	m[8]*curve->controlv[a+2] +
	m[12]*curve->controlv[a+3];

      point[1] = m[1]*curve->controlv[a] +
	m[5]*curve->controlv[a+1] +
	m[9]*curve->controlv[a+2] +
	m[13]*curve->controlv[a+3];

      point[2] = m[2]*curve->controlv[a] +
	m[6]*curve->controlv[a+1] +
	m[10]*curve->controlv[a+2] +
	m[14]*curve->controlv[a+3];

      point[3] = m[3]*curve->controlv[a] +
	m[7]*curve->controlv[a+1] +
	m[11]*curve->controlv[a+2] +
	m[15]*curve->controlv[a+3];

      /* build a profile */
      controlv[b] = point[0];
      controlv[b+1] = point[1];
      controlv[b+2] = point[2];
      controlv[b+3] = curve->controlv[a+3];

      controlv[c] = point[0];
      controlv[c+1] = point[1];
      controlv[c+2] = point[2]+height;
      controlv[c+3] = curve->controlv[a+3];

      c += 4;
      b += 4;
      a += 4;
    } /* for */

  new->controlv = controlv;

  *extrusion = new;

 return ay_status;
} /* ay_npt_extrude */


/* ay_npt_gettangentfromcontrol2D:
 *
 *
 */
int
ay_npt_gettangentfromcontrol2D(int ctype, int n, int p, int stride,
			       double *P, int a, double *T)
{
 int ay_status = AY_OK;
 int found = AY_FALSE, wrapped = AY_FALSE;
 int b, i1, i2;
 int before, after;
 double l;

  if(ctype == AY_CTPERIODIC)
    {
      if(a == 0)
	a = (n-p);
      if(a == n)
	a = p;
      if(a > (n-p))
	a -= (n-p);
    } /* if */

  /* find a good point after P[a] */
  if((ctype == AY_CTOPEN) && (a == (n-1)))
    {
      after = a;
    }
  else
    {
      b = a+1;
      i1 = a*stride;
      while(!found)
	{
	  if(b >= n)
	    {
	      if(wrapped)
		return AY_ERROR;
	      wrapped = AY_TRUE;
	      b = 0;
	    } /* if */

	  i2 = b*stride;
	  if((fabs(P[i1] - P[i2]) > AY_EPSILON) ||
	     (fabs(P[i1+1] - P[i2+1]) > AY_EPSILON))
	    {
	      found = AY_TRUE;
	    }
	  else
	    {
	      b++;
	    } /* if */
	} /* while */
      after = b;
    } /* if */

  /* find a good point before P[a] */
  if((ctype == AY_CTOPEN) && (a == 0))
    {
      before = a;
    }
  else
    {
      found = AY_FALSE;
      wrapped = AY_FALSE;
      b = a-1;
      i1 = a*stride;
      while(!found)
	{
	  if(b < 0)
	    {
	      if(wrapped)
		return AY_ERROR;
	      wrapped = AY_TRUE;
	      b = (n-1);
	    } /* if */

	  i2 = b*stride;
	  if((fabs(P[i1] - P[i2]) > AY_EPSILON) ||
	     (fabs(P[i1+1] - P[i2+1]) > AY_EPSILON))
	    {
	      found = AY_TRUE;
	    }
	  else
	    {
	      b--;
	    } /* if */
	} /* while */
      before = b;
    } /* if */

  /* now calculate the tangent */
  T[0] = P[after*stride] - P[before*stride];
  T[1] = P[(after*stride)+1] - P[(before*stride)+1];

  /* normalize tangent vector */
  l = sqrt(T[0]*T[0]+T[1]*T[1]);
  if(l > AY_EPSILON)
    {
      T[0] /= l;
      T[1] /= l;
    } /* if */

 return ay_status;
} /* ay_npt_gettangentfromcontrol2D */


/* ay_npt_createcap:
 *
 *
 */
int
ay_npt_createcap(double z, ay_nurbcurve_object *curve,
		 double *ominx, double *omaxx,
		 double *ominy, double *omaxy,
		 ay_nurbpatch_object **cap)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *patch = NULL;
 double knotv[4] = {0.0,0.0,1.0,1.0};
 double minx, miny, maxx, maxy;
 int i, stride;

  /* calloc the new patch */
  if(!(patch = calloc(1, sizeof(ay_nurbpatch_object))))
    return AY_EOMEM;
  if(!(patch->vknotv = malloc(4*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(patch->uknotv = malloc(4*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(patch->controlv = malloc(4*4*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  patch->width = 2;
  patch->height = 2;
  patch->uorder = 2;
  patch->vorder = 2;
  memcpy(patch->uknotv, knotv, 4*sizeof(double));
  memcpy(patch->vknotv, knotv, 4*sizeof(double));

  patch->is_planar = AY_TRUE;

  i = 0;
  minx = curve->controlv[0];
  maxx = minx;
  miny = curve->controlv[1];
  maxy = miny;

  stride = 4;
  while(i < curve->length*stride)
    {
      if(curve->controlv[i] > maxx)
	maxx = curve->controlv[i];
      if(curve->controlv[i] < minx)
	minx = curve->controlv[i];
      if(curve->controlv[i+1] > maxy)
	maxy = curve->controlv[i+1];
      if(curve->controlv[i+1] < miny)
	miny = curve->controlv[i+1];

      i += stride;
    } /* while */

  patch->controlv[0] = minx;
  patch->controlv[1] = miny;
  patch->controlv[2] = 0.0;

  patch->controlv[4] = minx;
  patch->controlv[5] = maxy;
  patch->controlv[6] = 0.0;

  patch->controlv[8] = maxx;
  patch->controlv[9] = miny;
  patch->controlv[10] = 0.0;

  patch->controlv[12] = maxx;
  patch->controlv[13] = maxy;
  patch->controlv[14] = 0.0;

  for(i = 2; i <= 15; i += 4)
    {
      patch->controlv[i] = z;
      patch->controlv[i+1] = 1.0;
    }

  *ominx = minx;
  *omaxx = maxx;
  *ominy = miny;
  *omaxy = maxy;

  /* return result */
  *cap = patch;

  /* prevent cleanup code from doing something harmful */
  patch = NULL;

cleanup:

  if(patch)
    {
      ay_npt_destroy(patch);
    }

 return ay_status;
} /* ay_npt_createcap */


/** ay_npt_applytrafo:
 *  applies transformations from object to all control points,
 *  then resets the objects transformation attributes
 *
 * \param[in,out] p object of type NPatch to process
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_applytrafo(ay_object *p)
{
 int i, stride;
 double m[16];
 ay_nurbpatch_object *np = NULL;

  if(!p)
    return AY_ENULL;

  if(p->type != AY_IDNPATCH)
    return AY_EWTYPE;

  np = (ay_nurbpatch_object *)(p->refine);

  /* get surfaces transformation-matrix */
  ay_trafo_creatematrix(p, m);

  stride = 4;
  i = 0;
  while(i < np->width*np->height*stride)
    {
      ay_trafo_apply3(&(np->controlv[i]), m);

      i += stride;
    } /* while */

  /* reset surfaces transformation attributes */
  ay_trafo_defaults(p);

 return AY_OK;
} /* ay_npt_applytrafo */


/** ay_npt_getpntfromindex:
 *  get address of a single control point from its indices
 *  (performing bounds checking)
 *
 * \param[in] patch NPatch object to process
 * \param[in] indexu index of desired control point in U dimension (width)
 * \param[in] indexv index of desired control point in V dimension (height)
 * \param[in,out] p pointer to pointer where to store the resulting address
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_getpntfromindex(ay_nurbpatch_object *patch, int indexu, int indexv,
		       double **p)
{
 int stride = 4;
 char fname[] = "npt_getpntfromindex";

  if(!patch || !p)
    return AY_ENULL;

  if(indexu >= patch->width || indexu < 0)
    return ay_error_reportirange(fname, "\"indexu\"", 0, patch->width-1);

  if(indexv >= patch->height || indexv < 0)
    return ay_error_reportirange(fname, "\"indexv\"", 0, patch->height-1);

  *p = &(patch->controlv[(indexu*patch->height+indexv)*stride]);

 return AY_OK;
} /* ay_npt_getpntfromindex */


/** ay_npt_elevateu:
 *  Elevate the u order of a NURBS patch.
 *
 * \param[in,out] patch NURBS patch object to process
 * \param[in] t how many times shall the order be increased
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_elevateu(ay_nurbpatch_object *patch, int t)
{
 int ay_status = AY_OK;
 double *Uh = NULL, *Qw = NULL, *realQw = NULL, *realUh = NULL;
 int clamp_me = AY_FALSE, nw = 0;
 char fname[] = "npt_elevateu";

  if(patch->uknot_type == AY_KTBSPLINE)
    {
      clamp_me = AY_TRUE;
    }
  else
    {
      if(patch->uknot_type == AY_KTCUSTOM)
	{
	  clamp_me = AY_TRUE;
	}
      else
	{
	  if(patch->uknot_type > AY_KTCUSTOM && patch->utype == AY_CTPERIODIC)
	    {
	      clamp_me = AY_TRUE;
	    }
	} /* if */
    } /* if */

  if(clamp_me)
    {
      ay_status = ay_npt_clampu(patch, 0);
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Clamp operation failed.");
	  return AY_ERROR;
	} /* if */
    } /* if */

  /* alloc new knotv & new controlv */
  if(!(Uh = calloc((patch->width + patch->width*t +
		    patch->uorder + t),
		   sizeof(double))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_EOMEM;
    }
  if(!(Qw = calloc((patch->width + patch->width*t) * patch->height * 4,
		   sizeof(double))))
    {
      free(Uh);
      ay_error(AY_EOMEM, fname, NULL);
      return AY_EOMEM;
    }

  /* fill Uh & Qw */
  ay_status = ay_nb_DegreeElevateSurfU4D(4, patch->width-1,
					 patch->height-1,
					 patch->uorder-1, patch->uknotv,
					 patch->controlv, t, &nw, Uh, Qw);

  if(ay_status)
    {
      free(Uh); free(Qw);
      ay_error(ay_status, fname, "Degree elevation failed.");
      return AY_ERROR;
    }

  if(!(realQw = realloc(Qw, nw*patch->height*4*sizeof(double))))
    {
      free(Qw); free(Uh);
      ay_error(AY_EOMEM, fname, NULL);
      return AY_EOMEM;
    }

  if(!(realUh = realloc(Uh, (nw+patch->uorder+t)*sizeof(double))))
    {
      free(realQw); free(Uh);
      ay_error(AY_EOMEM, fname, NULL);
      return AY_EOMEM;
    }

  free(patch->uknotv);
  patch->uknotv = realUh;

  free(patch->controlv);
  patch->controlv = realQw;

  patch->uknot_type = AY_KTCUSTOM;

  patch->uorder += t;

  patch->width = nw;

  ay_npt_recreatemp(patch);

 return ay_status;
} /* ay_npt_elevateu */


/** ay_npt_elevatev:
 *  Elevate the v order of a NURBS patch.
 *
 * \param[in,out] patch NURBS patch object to process
 * \param[in] t how many times shall the order be increased
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_elevatev(ay_nurbpatch_object *patch, int t)
{
 int ay_status = AY_OK;
 double *Vh = NULL, *Qw = NULL, *realQw = NULL, *realVh = NULL;
 int i, clamp_me = AY_FALSE, nh = 0, ind1, ind2;
 char fname[] = "npt_elevatev";

  if(patch->vknot_type == AY_KTBSPLINE)
    {
      clamp_me = AY_TRUE;
    }
  else
    {
      if(patch->vknot_type == AY_KTCUSTOM)
	{
	  clamp_me = AY_TRUE;
	}
      else
	{
	  if(patch->vknot_type > AY_KTCUSTOM && patch->vtype == AY_CTPERIODIC)
	    {
	      clamp_me = AY_TRUE;
	    }
	} /* if */
    } /* if */

  if(clamp_me)
    {
      ay_status = ay_npt_clampv(patch, 0);
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Clamp operation failed.");
	  return AY_ERROR;
	} /* if */
    } /* if */

  /* alloc new knotv & new controlv */
  if(!(Vh = calloc((patch->height + patch->height*t +
		    patch->vorder + t),
		   sizeof(double))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_EOMEM;
    }
  if(!(Qw = calloc((patch->height + patch->height*t) * patch->width * 4,
		   sizeof(double))))
    {
      free(Vh);
      ay_error(AY_EOMEM, fname, NULL);
      return AY_EOMEM;
    }

  /* fill Vh & Qw */
  ay_status = ay_nb_DegreeElevateSurfV4D(4, patch->width-1,
					 patch->height-1,
					 patch->vorder-1, patch->vknotv,
					 patch->controlv, t, &nh, Vh, Qw);

  if(ay_status)
    {
      free(Vh); free(Qw);
      ay_error(ay_status, fname, "Degree elevation failed.");
      return AY_ERROR;
    }

  if(!(realQw = calloc(nh*patch->width*4, sizeof(double))))
    {
      free(Vh); free(Qw);
      ay_error(AY_EOMEM, fname, NULL);
      return AY_EOMEM;
    }

  for(i = 0; i < patch->width; i++)
    {
      ind1 = (i*nh)*4;
      ind2 = (i*(patch->height+patch->height*t))*4;
      memcpy(&(realQw[ind1]), &(Qw[ind2]), nh*4*sizeof(double));
    }

  free(Qw);

  if(!(realVh = realloc(Vh, (nh+patch->vorder+t)*sizeof(double))))
    {
      free(realQw); free(Vh);
      ay_error(AY_EOMEM, fname, NULL);
      return AY_EOMEM;
    }

  free(patch->vknotv);
  patch->vknotv = realVh;

  free(patch->controlv);
  patch->controlv = realQw;

  patch->vknot_type = AY_KTCUSTOM;

  patch->vorder += t;

  patch->height = nh;

  ay_npt_recreatemp(patch);

 return ay_status;
} /* ay_npt_elevatev */


/** ay_npt_elevateuvtcmd:
 *  Elevate U/V order of selected NURBS patches.
 *  Implements the \a elevateuNP and \a elevatevNP scripting
 *  interface commands.
 *  See also the corresponding section in the \ayd{scelevateunp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_elevateuvtcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *patch = NULL;
 int elevatev = AY_FALSE, t = 1;
 int notify_parent = AY_FALSE;

  if(argc >= 2)
    {
      tcl_status = Tcl_GetInt(interp, argv[1], &t);
      AY_CHTCLERRRET(tcl_status, argv[0], interp);

      if(t <= 0)
	{
	  ay_error(AY_ERROR, argv[0], "Argument must be > 0.");
	  return TCL_OK;
	}
    }

  if(!strcmp(argv[0], "elevatevNP"))
    elevatev = AY_TRUE;

  while(sel)
    {
      if(sel->object->type == AY_IDNPATCH)
	{
	  patch = (ay_nurbpatch_object *)sel->object->refine;

	  if(elevatev)
	    ay_status = ay_npt_elevatev(patch, t);
	  else
	    ay_status = ay_npt_elevateu(patch, t);

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "Elevate failed.");
	    }
	  else
	    {
	      sel->object->modified = AY_TRUE;

	      if(sel->object->selp)
		{
		  ay_selp_clear(sel->object);
		}

	      /* re-create tesselation of patch */
	      (void)ay_notify_object(sel->object);
	      notify_parent = AY_TRUE;
	    }
	}
      else
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	} /* if */

      sel = sel->next;
    } /* while */

  if(notify_parent)
    (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_elevateuvtcmd */


/** ay_npt_swapuvtcmd:
 *  Swap U and V of selected surfaces.
 *  Implements the \a swapuvS scripting interface command.
 *  See also the corresponding section in the \ayd{scswapuvs}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_swapuvtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 /*int ay_status;*/
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *np = NULL;
 ay_ipatch_object *ip = NULL;
 ay_pamesh_object *pm = NULL;
 ay_bpatch_object *bp = NULL;
 double pt[3];
 int notify_parent = AY_FALSE;

  while(sel)
    {
      switch(sel->object->type)
	{
	case AY_IDNPATCH:
	  np = (ay_nurbpatch_object *)sel->object->refine;

	  ay_npt_swapuv(np);

	  sel->object->modified = AY_TRUE;
	  break;
	case AY_IDIPATCH:
	  ip = (ay_ipatch_object *)sel->object->refine;

	  ay_ipt_swapuv(ip);

	  sel->object->modified = AY_TRUE;
	  break;
	case AY_IDPAMESH:
	  pm = (ay_pamesh_object *)sel->object->refine;

	  ay_pmt_swapuv(pm);

	  sel->object->modified = AY_TRUE;
	  break;
	case AY_IDBPATCH:
	  bp = (ay_bpatch_object *)sel->object->refine;

	  memcpy(pt, bp->p1, 3*sizeof(double));
	  memcpy(bp->p1, bp->p4, 3*sizeof(double));
	  memcpy(bp->p4, pt, 3*sizeof(double));
	  memcpy(pt, bp->p2, 3*sizeof(double));
	  memcpy(bp->p2, bp->p3, 3*sizeof(double));
	  memcpy(bp->p3, pt, 3*sizeof(double));

	  sel->object->modified = AY_TRUE;
	  break;
	default:
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	  break;
	} /* switch */

      if(sel->object->modified)
	{
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  (void)ay_notify_object(sel->object);
	  notify_parent = AY_TRUE;
	}

      sel = sel->next;
    } /* while */

  if(notify_parent)
    (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_swapuvtcmd */


/* ay_npt_gordon:
 *  create a gordon surface of order <uorder>/<vorder> from the
 *  curves in <cu> and <cv> possibly using the NURBS patch <in>
 *  to get the intersections of the curves from
 *  returns result in <gordon>
 */
int
ay_npt_gordon(ay_object *cu, ay_object *cv, ay_object *in,
	      int uorder, int vorder,
	      ay_nurbpatch_object **gordon)
{
 int ay_status = AY_OK;
 char fname[] = "npt_gordon";
 int stride = 4;
 ay_object *c;
 ay_object *lcu = NULL, *lcv = NULL; /* last cu/cv curve */
 ay_nurbcurve_object *nc = NULL;
 ay_nurbpatch_object *interpatch = NULL, *skinu = NULL, *skinv = NULL;
 int uo, vo;
 int i, j, k, numcu = 0, numcv = 0; /* numbers of parameter curves */
 int need_interpol = AY_FALSE;
 double *intersections = NULL; /* matrix of intersection points */
 double *unifiedU = NULL, *unifiedV = NULL;
 int uUlen, uVlen/*, closeu = AY_FALSE, closev = AY_FALSE*/;

  if(!cu || !cv || !gordon)
    return AY_ENULL;

  /* count parameter curves, assuming all objects are of the right
     type (NURBS Curve) */
  c = cu;
  while(c)
    {
      numcu++;
      lcu = c;
      c = c->next;
    }

  c = cv;
  while(c)
    {
      numcv++;
      lcv = c;
      c = c->next;
    }
#if 0
  if((((ay_nurbcurve_object *)cu->refine)->type == AY_CTCLOSED ||
     ((ay_nurbcurve_object *)cu->refine)->type == AY_CTPERIODIC) &&
     (((ay_nurbcurve_object *)lcu->refine)->type == AY_CTCLOSED ||
      ((ay_nurbcurve_object *)lcu->refine)->type == AY_CTPERIODIC))
    closev = AY_TRUE;

  if((((ay_nurbcurve_object *)cv->refine)->type == AY_CTCLOSED ||
     ((ay_nurbcurve_object *)cv->refine)->type == AY_CTPERIODIC) &&
     (((ay_nurbcurve_object *)lcv->refine)->type == AY_CTCLOSED ||
      ((ay_nurbcurve_object *)lcv->refine)->type == AY_CTPERIODIC))
    closeu = AY_TRUE;
#endif
  /* assume, we always get enough parameter curves (>=2 in each dimension) */

  /* check and adapt desired orders */
  if(uorder < 2)
    uorder = 4;

  if(vorder < 2)
    vorder = 4;

  uo = uorder;
  if(numcv < uorder)
    uorder = numcv;

  vo = vorder;
  if(numcu < vorder)
    vorder = numcu;

  /* make curves compatible (defined on the same knot vector) */
  ay_status = ay_nct_makecompatible(cu);
  ay_status = ay_nct_makecompatible(cv);

  if(!in)
    {
      /* calculate intersection points */
      if(!(intersections = calloc(numcu*numcv*4, sizeof(double))))
	return AY_EOMEM;

      /* immediately put the four endpoints of the first/last u curve into the
	 corners of the intersections array, we need them in any case */
      nc = (ay_nurbcurve_object *)cu->refine;
      i = 0;
      ay_nb_CurvePoint3D(nc->length-1, nc->order-1, nc->knotv, nc->controlv,
			 nc->knotv[nc->order-1], &(intersections[i]));
      intersections[i+3] = nc->controlv[3];
      i = (numcv-1)*numcu*4;
      ay_nb_CurvePoint3D(nc->length-1, nc->order-1, nc->knotv, nc->controlv,
			 nc->knotv[nc->length], &(intersections[i]));
      intersections[i+3] = nc->controlv[(nc->length-1)*4+3];

      nc = (ay_nurbcurve_object *)lcu->refine;
      i = (numcu-1)*4;
      ay_nb_CurvePoint3D(nc->length-1, nc->order-1, nc->knotv, nc->controlv,
			 nc->knotv[nc->order-1], &(intersections[i]));
      intersections[i+3] = nc->controlv[3];
      i = (numcu*numcv-1)*4;
      ay_nb_CurvePoint3D(nc->length-1, nc->order-1, nc->knotv, nc->controlv,
			 nc->knotv[nc->length], &(intersections[i]));
      intersections[i+3] = nc->controlv[(nc->length-1)*4+3];

      /* first, check for some easy cases */
      if((numcu == 2) && (numcv == 2))
	{
	  /* dead easy, we have just 4 intersection points, which we already
	     got from the start/end points of the two u curves, no
	     interpolation needed */
	  need_interpol = AY_FALSE;
	}
      else
	{
	  if(numcu == 2) /* && numcv > 2 */
	    {
	      /* still easy, we can get all intersections from curve
		 endpoints */

	      /* get missing intersection points */
	      c = cv->next;
	      i = 2*4;
	      for(j = 2; j < numcv; j++)
		{
		  nc = (ay_nurbcurve_object *)c->refine;

		  ay_nb_CurvePoint4D(nc->length-1, nc->order-1, nc->knotv,
				     nc->controlv, nc->knotv[nc->order-1],
				     &(intersections[i]));
		  i += 4;
		  ay_nb_CurvePoint4D(nc->length-1, nc->order-1, nc->knotv,
				     nc->controlv, nc->knotv[nc->length],
				     &(intersections[i]));
		  i += 4;
		  c = c->next;
		} /* for */

	      need_interpol = AY_TRUE;
	    }
	  else
	    {
	      if(numcv == 2) /* && numcu > 2 */
		{
		  /* still easy, we can get all intersections from
		     curve endpoints */

		  /* get missing intersection points */
		  c = cu->next;
		  i = 4;
		  for(j = 2; j < numcu; j++)
		    {
		      nc = (ay_nurbcurve_object *)c->refine;
		      ay_nb_CurvePoint4D(nc->length-1, nc->order-1, nc->knotv,
					 nc->controlv, nc->knotv[nc->order-1],
					 &(intersections[i]));
		      i += 4;
		      c = c->next;
		    } /* for */

		  c = cu->next;
		  i = ((numcv-1)*numcu+1)*4;
		  for(j = 2; j < numcu; j++)
		    {
		      nc = (ay_nurbcurve_object *)c->refine;
		      ay_nb_CurvePoint4D(nc->length-1, nc->order-1, nc->knotv,
					 nc->controlv, nc->knotv[nc->length],
					 &(intersections[i]));
		      i += 4;
		      c = c->next;
		    } /* for */

		  need_interpol = AY_TRUE;
		}
	      else
		{
		  /* this is the general, hard case... */

		  /* XXXX ...which is not supported yet... */
		  free(intersections);
		  ay_error(AY_ERROR, fname,
			   "Unsupported number of u/v curves.");
		  return AY_ERROR;
		  /*
		    ay_status = ay_nct_intersectca(cu, cv, intersections);
		    need_interpol = AY_TRUE;
		  */
		} /* if */
	    } /* if */
	} /* if */

      ay_status = ay_npt_create(uorder, vorder, numcv, numcu,
				AY_KTNURB, AY_KTNURB,
				intersections, NULL, NULL,
				&interpatch);
    }
  else
    {
      /* use intersection point patch delivered in argument <in> */
      interpatch = (ay_nurbpatch_object*)in->refine;
      if((numcu == 2) && (numcv == 2))
	{
	  need_interpol = AY_FALSE;
	}
      else
	{
	  need_interpol = AY_TRUE;
	}
    } /* if */

  if(need_interpol)
    {
      ay_status = ay_ipt_interpolateu(interpatch, uorder, AY_KTCHORDAL);
      ay_status = ay_ipt_interpolatev(interpatch, vorder, AY_KTCHORDAL);
    }

  ay_status = ay_npt_skinv(cu, uorder, AY_KTCUSTOM, AY_TRUE, &skinu);
  if(!skinu)
    goto cleanup;

  ay_status = ay_npt_skinu(cv, vorder, AY_KTCUSTOM, AY_TRUE, &skinv);
  if(!skinv)
    goto cleanup;

  /* get highest uorder */
  if(skinu->uorder > uo)
    uo = skinu->uorder;
  if(skinv->uorder > uo)
    uo = skinv->uorder;
  if(interpatch->uorder > uo)
    uo = interpatch->uorder;

  /* get highest vorder */
  if(skinu->vorder > vo)
    vo = skinu->vorder;
  if(skinv->vorder > vo)
    vo = skinv->vorder;
  if(interpatch->vorder > vo)
    vo = interpatch->vorder;

  /* elevate surfaces to highest uorder/vorder */
  if(skinu->uorder < uo)
    ay_status = ay_npt_elevateu(skinu, uo-skinu->uorder);

  if(skinu->vorder < vo)
    ay_status = ay_npt_elevatev(skinu, vo-skinu->vorder);

  if(skinv->uorder < uo)
    ay_status = ay_npt_elevateu(skinv, uo-skinv->uorder);

  if(skinv->vorder < vo)
    ay_status = ay_npt_elevatev(skinv, vo-skinv->vorder);

  if(interpatch->uorder < uo)
    ay_status = ay_npt_elevateu(interpatch, uo-interpatch->uorder);

  if(interpatch->vorder < vo)
    ay_status = ay_npt_elevatev(interpatch, vo-interpatch->vorder);

  /* make surfaces compatible (defined on the same knot vector) */
  ay_status = ay_knots_unify(skinu->uknotv, skinu->width+skinu->uorder,
			     skinv->uknotv, skinv->width+skinu->uorder,
			     &unifiedU, &uUlen);
  ay_status = ay_knots_unify(unifiedU, uUlen,
			     interpatch->uknotv, interpatch->width+
			     interpatch->uorder,
			     &unifiedU, &uUlen);

  ay_status = ay_knots_unify(skinu->vknotv, skinu->height+skinu->vorder,
			     skinv->vknotv, skinv->height+skinv->vorder,
			     &unifiedV, &uVlen);
  ay_status = ay_knots_unify(unifiedV, uVlen,
			     interpatch->vknotv, interpatch->height+
			     interpatch->vorder,
			     &unifiedV, &uVlen);

  ay_status = ay_knots_mergenp(skinu, unifiedU, uUlen, unifiedV, uVlen);

  ay_status = ay_knots_mergenp(skinv, unifiedU, uUlen, unifiedV, uVlen);

  ay_status = ay_knots_mergenp(interpatch, unifiedU, uUlen, unifiedV, uVlen);

  /* combine surfaces */
  k = 0;
  for(i = 0; i < skinu->width; i++)
    {
      for(j = 0; j < skinu->height; j++)
	{

	  skinu->controlv[k] += skinv->controlv[k];
	  skinu->controlv[k] -= interpatch->controlv[k];

	  skinu->controlv[k+1] += skinv->controlv[k+1];
	  skinu->controlv[k+1] -= interpatch->controlv[k+1];

	  skinu->controlv[k+2] += skinv->controlv[k+2];
	  skinu->controlv[k+2] -= interpatch->controlv[k+2];

	  /* handle special case Coons patch (curves may be rational) */
	  if((numcu == 2) && (numcv == 2))
	    {
	      skinu->controlv[k+3] += skinv->controlv[k+3];
	      skinu->controlv[k+3] -= interpatch->controlv[k+3];
	    }
	  else
	    {
	      skinu->controlv[k+3] = 1.0;
	    }
	  k += stride;
	} /* for */
    } /* for */
#if 0
  if(closeu)
    {
      memcpy(&(skinu->controlv[0]),
	     &(skinu->controlv[skinu->height*4]), 4*sizeof(double));
      memcpy(&(skinu->controlv[(skinu->width*skinu->width-skinu->height)*4]),
	     &(skinu->controlv[skinu->width*skinu->height*4]), 4*sizeof(double));
    }

  if(closev)
    {
      memcpy(&(skinu->controlv[(skinu->width*skinu->width-skinu->height)*4]),
	     &(skinu->controlv[0]), 4*sizeof(double));
      memcpy(&(skinu->controlv[skinu->width*skinu->height*4]),
	     &(skinu->controlv[skinu->height*4]), 4*sizeof(double));
    }
#endif

  /* return result */
  *gordon = skinu;

  /* prevent cleanup code from doing something harmful */
  skinu = NULL;

cleanup:

  if(skinu)
    {
      ay_npt_destroy(skinu);
    }
  if(skinv)
    {
      ay_npt_destroy(skinv);
    }
  if(interpatch && !in)
    {
      ay_npt_destroy(interpatch);
    }
  if(unifiedU)
    {
      free(unifiedU);
    }
  if(unifiedV)
    {
      free(unifiedV);
    }

 return ay_status;
} /* ay_npt_gordon */


/* ay_npt_gordonmodw:
 *  calculate which curve to modify, in order to follow the latest changes
 *  in the geometry of the other; decision is based on whether one object
 *  of the two is flagged "modified" or "currently selected"
 *  (XXXX could be extended by comparing lists of selected points, but
 *  seems to work ok for gordon); returns:
 *  0 - modify o1
 *  1 - modify o2
 */
int
ay_npt_gordonmodw(ay_object *o1, ay_object *o2)
{

  if(o1->modified && !o2->modified)
    {
      return 1;
    }
  else
    {
      if(!o1->modified && o2->modified)
	{
	  return 0;
	}
      else
	{
	  if(o1->selected && !o2->selected)
	    {
	      return 1;
	    }
	  else
	    {
	      if(!o1->selected && o2->selected)
		{
		  return 0;
		}
	      else
		{
		  /* always modify o2 in favour of o1 if */
		  return 1;
		}
	    } /* if */
	} /* if */
    } /* if */

} /* ay_npt_gordonmodw */


/* ay_npt_gordoncc:
 *  compare and possibly correct two curve endpoints (<p1> and <p2>)
 *  of curve objects <o1> and <o2>; work with stride <stride>;
 *  copy new point data to object at position <pp1>, <pp2> respectively;
 *  use precalculated inverse transformation matrix <m1> or <m2>
 *  (the two curve objects do not necessarily have the same trafos)
 */
void
ay_npt_gordoncc(ay_object *o1, ay_object *o2, int stride,
		double *p1, double *p2, double *pp1, double *pp2,
		double *m1, double *m2)
{
 double tp[3];
 int modify;

  AY_V3SUB(tp, p1, p2)
  if((fabs(tp[0]) > AY_EPSILON) || (fabs(tp[1]) > AY_EPSILON) ||
     (fabs(tp[2]) > AY_EPSILON))
    {
      if(AY_V3LEN(tp) > AY_EPSILON)
	{
	  /* the points do not match, find out which
	     curve is to be modified */
	  modify = ay_npt_gordonmodw(o1, o2);

	  if(modify)
	    {
	      /* modify o2 (p2 <= p1) */
	      if(pp2)
		{
		  memcpy(pp2, p1, stride*sizeof(double));
		  if(m2)
		    ay_trafo_apply3(pp2, m2);
		  if(o2->type == AY_IDNCURVE && stride == 3)
		    pp2[3] = 1.0;
		}
	      else
		{
		  if(pp1)
		    {
		      memcpy(pp1, p2, stride*sizeof(double));
		      if(m1)
			ay_trafo_apply3(pp1, m1);
		      if(o1->type == AY_IDNCURVE && stride == 3)
			pp1[3] = 1.0;
		    }
		}
	    }
	  else
	    {
	      /* modify o1 (p1 <= p2) */
	      if(pp1)
		{
		  memcpy(pp1, p2, stride*sizeof(double));
		  if(m1)
		    ay_trafo_apply3(pp1, m1);
		  if(o1->type == AY_IDNCURVE && stride == 3)
		    pp1[3] = 1.0;
		}
	      else
		{
		  if(pp2)
		    {
		      memcpy(pp2, p1, stride*sizeof(double));
		      if(m2)
			ay_trafo_apply3(pp2, m2);
		      if(o2->type == AY_IDNCURVE && stride == 3)
			pp2[3] = 1.0;
		    }
		}
	    } /* if */
	} /* if */
    } /* if */

 return;
} /* ay_npt_gordoncc */


/* ay_npt_gordonwcgetends:
 *
 */
void
ay_npt_gordonwcgetends(ay_object *o,
                       double *s, double *e, double **ss, double **se,
                       int *trafo, double *tm, double *tmi)
{
 ay_object *p = NULL;
 ay_nurbcurve_object *nc;
 ay_icurve_object *ic;
 ay_acurve_object *ac;
 double *pps, *ppe;

  switch(o->type)
    {
    case AY_IDNCURVE:
      nc = (ay_nurbcurve_object *)o->refine;
      pps = &(nc->controlv[0]);
      ppe = &(nc->controlv[(nc->length-1)*4]);
      *ss = pps;
      *se = ppe;
      break;
    case AY_IDICURVE:
      ic = (ay_icurve_object *)o->refine;
      pps = &(ic->controlv[0]);
      ppe = &(ic->controlv[(ic->length-1)*3]);
      *ss = pps;
      *se = ppe;
      break;
    case AY_IDACURVE:
      ac = (ay_acurve_object *)o->refine;
      pps = &(ac->controlv[0]);
      ppe = &(ac->controlv[(ac->length-1)*3]);
      *ss = pps;
      *se = ppe;
      break;
    default:
      ay_provide_object(o, AY_IDNCURVE, &p);
      if(p)
        {
          nc = (ay_nurbcurve_object *)p->refine;

          pps = &(nc->controlv[0]);
          ppe = &(nc->controlv[(nc->length-1)*4]);
          if(AY_ISTRAFO(p))
            {
              ay_trafo_creatematrix(p, tm);
              AY_APTRAN3(s, pps, tm)
              AY_APTRAN3(e, ppe, tm)
            }
          else
            {
              memcpy(s, pps, 3*sizeof(double));
              memcpy(e, ppe, 3*sizeof(double));
            }
          ay_object_deletemulti(p, AY_FALSE);
        }
      else
	{
	  /* report error? */
	  return;
	}
      *ss = NULL;
      *se = NULL;
      break;
    } /* switch */

  if(!p)
    {
      if(AY_ISTRAFO(o))
        {
          *trafo = AY_TRUE;
          ay_trafo_creatematrix(o, tm);
          ay_trafo_invmatrix(tm, tmi);
          AY_APTRAN3(s, pps, tm)
          AY_APTRAN3(e, ppe, tm)
        }
      else
        {
          memcpy(s, pps, 3*sizeof(double));
          memcpy(e, ppe, 3*sizeof(double));
        }
    }

 return;
} /* ay_npt_gordonwcgetends */


/* ay_npt_gordonwc:
 *  watch and properly configure the outer four parameter curves of a
 *  gordon surface <g> (compare and possibly correct their endpoints)
 *  XXXX should be extended to cover also the inner curve endpoints
 */
void
ay_npt_gordonwc(ay_object *g)
{
 ay_object *firstu = NULL, *lastu = NULL, *firstv = NULL, *lastv = NULL;
 ay_object *last = NULL, *down;
 int istrafo[4] = {0};
 double fum[16], lum[16], fvm[16], lvm[16];
 double fumi[16], lumi[16], fvmi[16], lvmi[16];
 double p1[3], p2[3], p3[3], p4[3], p5[3], p6[3], p7[3], p8[3];
 double *pp1 = NULL, *pp2 = NULL, *pp3 = NULL, *pp4 = NULL;
 double *pp5 = NULL, *pp6 = NULL, *pp7 = NULL, *pp8 = NULL;
 double *mi1, *mi2;

  if(!g || !g->down || !g->down->next)
    return;

  /* get curves */
  down = g->down;
  firstu = down;
  while(down->next)
    {
      if(down->type == AY_IDLEVEL)
	{
	  lastu = last;
	  firstv = down->next;
	}
      last = down;
      lastv = down;
      down = down->next;
    }

  if((!lastu) || (!firstv) || (!lastv) || (firstu == lastu) ||
     (firstv == lastv))
    return;

  /* get all eight curve end coordinates and transform them to
     Gordon object space */
  ay_npt_gordonwcgetends(firstu, p1, p2, &pp1, &pp2, &(istrafo[0]),
			 fum, fumi);
  ay_npt_gordonwcgetends(lastu, p3, p4, &pp3, &pp4, &(istrafo[1]),
			 lum, lumi);
  ay_npt_gordonwcgetends(firstv, p5, p6, &pp5, &pp6, &(istrafo[2]),
			 fvm, fvmi);
  ay_npt_gordonwcgetends(lastv, p7, p8, &pp7, &pp8, &(istrafo[3]),
			 lvm, lvmi);

  /* now compare and possibly correct the points */
  if(pp1 || pp5)
    {
      mi1 = fumi;
      if(!istrafo[0])
	mi1 = NULL;
      mi2 = fvmi;
      if(!istrafo[2])
	mi2 = NULL;
      ay_npt_gordoncc(firstu, firstv,
		      ((firstu->type == AY_IDNCURVE) &&
		       (firstv->type == AY_IDNCURVE))?4:3,
		      p1, p5, pp1, pp5, mi1, mi2);
    }

  if(pp2 || pp7)
    {
      mi1 = fumi;
      if(!istrafo[0])
	mi1 = NULL;
      mi2 = lvmi;
      if(!istrafo[3])
	mi2 = NULL;
      ay_npt_gordoncc(firstu, lastv,
		      ((firstu->type == AY_IDNCURVE) &&
		       (lastv->type == AY_IDNCURVE))?4:3,
		      p2, p7, pp2, pp7, mi1, mi2);
    }

  if(pp3 || pp6)
    {
      mi1 = lumi;
      if(!istrafo[1])
	mi1 = NULL;
      mi2 = fvmi;
      if(!istrafo[2])
	mi2 = NULL;
      ay_npt_gordoncc(lastu, firstv,
		      ((lastu->type == AY_IDNCURVE) &&
		       (firstv->type == AY_IDNCURVE))?4:3,
		      p3, p6, pp3, pp6, mi1, mi2);
    }


  if(pp4 || pp8)
    {
      mi1 = lumi;
      if(!istrafo[1])
	mi1 = NULL;
      mi2 = lvmi;
      if(!istrafo[3])
	mi2 = NULL;
      ay_npt_gordoncc(lastu, lastv,
		      ((lastu->type == AY_IDNCURVE) &&
		       (lastv->type == AY_IDNCURVE))?4:3,
		      p4, p8, pp4, pp8, mi1, mi2);
    }

 return;
} /* ay_npt_gordonwc */


/** ay_npt_extractboundary:
 * extract complete boundary NURBS curve from the NURBS patch \a o
 *
 * \param[in] o NURBS patch object to process
 * \param[in] apply_trafo this parameter controls whether transformation
 *  attributes of \a o should be applied to the control points of the curve
 * \param[in,out] result pointer where to store the resulting curve
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_extractboundary(ay_object *o, int apply_trafo,
		       ay_nurbcurve_object **result)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *u0 = NULL, *un = NULL, *v0 = NULL, *vn = NULL;
 ay_nurbpatch_object *np;
 ay_object o0 = {0}, o1 = {0}, o2 = {0}, o3 = {0}, *c = NULL;

  if(!o || !result)
    return AY_ENULL;

  if(o->type != AY_IDNPATCH)
    return AY_EWTYPE;

  np = (ay_nurbpatch_object *)o->refine;

  /* extract four curves (from each boundary) */
  if(np->vknot_type == AY_KTNURB || np->vknot_type == AY_KTBEZIER)
    {
      ay_status = ay_npt_extractnc(o, 0, 0.0, AY_FALSE, apply_trafo,
				   AY_FALSE, NULL, &u0);
      if(ay_status)
	goto cleanup;
      ay_status = ay_npt_extractnc(o, 1, 0.0, AY_FALSE, apply_trafo,
				   AY_FALSE, NULL, &un);
      if(ay_status)
	goto cleanup;
    }
  else
    {
      ay_status = ay_npt_extractnc(o, 4, 0.0, AY_TRUE, apply_trafo,
				   AY_FALSE, NULL, &u0);
      if(ay_status)
	goto cleanup;
      ay_status = ay_npt_extractnc(o, 4, 1.0, AY_TRUE, apply_trafo,
				   AY_FALSE, NULL, &un);
      if(ay_status)
	goto cleanup;
    }

  if(np->uknot_type == AY_KTNURB || np->uknot_type == AY_KTBEZIER)
    {
      ay_status = ay_npt_extractnc(o, 2, 0.0, AY_FALSE, apply_trafo,
				   AY_FALSE, NULL, &v0);
      if(ay_status)
	goto cleanup;
      ay_status = ay_npt_extractnc(o, 3, 0.0, AY_FALSE, apply_trafo,
				   AY_FALSE, NULL, &vn);
      if(ay_status)
	goto cleanup;
    }
  else
    {
      ay_status = ay_npt_extractnc(o, 5, 0.0, AY_TRUE, apply_trafo,
				   AY_FALSE, NULL, &v0);
      if(ay_status)
	goto cleanup;
      ay_status = ay_npt_extractnc(o, 5, 1.0, AY_TRUE, apply_trafo,
				   AY_FALSE, NULL, &vn);
      if(ay_status)
	goto cleanup;
    }

  /* arrange the curves properly */
  ay_nct_revert(un);
  ay_nct_revert(v0);

  if(ay_nct_isdegen(u0))
    o0.type = AY_IDLAST;
  else
    {
      ay_object_defaults(&o0);
      o0.refine = u0;
      o0.type = AY_IDNCURVE;
    }
  if(ay_nct_isdegen(vn))
    o1.type = AY_IDLAST;
  else
    {
      ay_object_defaults(&o1);
      o1.refine = vn;
      o1.type = AY_IDNCURVE;
    }
  if(ay_nct_isdegen(un))
    o2.type = AY_IDLAST;
  else
    {
      ay_object_defaults(&o2);
      o2.refine = un;
      o2.type = AY_IDNCURVE;
    }
  if(ay_nct_isdegen(v0))
    o3.type = AY_IDLAST;
  else
    {
      ay_object_defaults(&o3);
      o3.refine = v0;
      o3.type = AY_IDNCURVE;
    }

  o0.next = &o1;
  o1.next = &o2;
  o2.next = &o3;

  /* in case the surface has different orders for U/V
     make the curves compatible (this also clamps them) */
  ay_status = ay_nct_makecompatible(&o0);
  if(ay_status)
    {ay_status = AY_ERROR; goto cleanup;}

  /* concatenate all four extracted curves */
  ay_status = ay_nct_concatmultiple(AY_FALSE, 1, AY_FALSE, &o0, &c);
  if(ay_status || !c)
    {ay_status = AY_ERROR; goto cleanup;}

  if(apply_trafo)
    {
      ay_trafo_copy(o, c);
      ay_nct_applytrafo(c);
    }

  /* return result */
  *result = (ay_nurbcurve_object*)c->refine;

cleanup:
  ay_nct_destroy(u0);
  ay_nct_destroy(un);
  ay_nct_destroy(v0);
  ay_nct_destroy(vn);

  if(c)
    free(c);

 return ay_status;
} /* ay_npt_extractboundary */


/** ay_npt_extracttrim:
 * extract a trim curve defined boundary from the NURBS patch \a o
 *
 * \param[in] o NURBS patch object to process
 * \param[in] tnum index of trim curve to extract
 * \param[in] param sampling density
 * \param[in] apply_trafo this parameter controls whether transformation
 *  attributes of \a o should be applied to the control points of the curve
 * \param[in] extractnt should normals/tangents be extracted (0 - no,
 *            1 - normals, 2 - normals and tangents)
 * \param[in,out] pvnt pointer where to store the extracted normals/tangents,
 *            may be NULL, if \a extractnt is 0
 * \param[in,out] result pointer where to store the resulting curve
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_extracttrim(ay_object *o, int tnum, double param, int apply_trafo,
		   int extractnt, double **pvnt, ay_nurbcurve_object **result)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *nc;
 ay_nurbpatch_object *np;
 int stride, qf, i = 0, a = 0, tcslen = 0;
 int *tcslens = NULL;
 double **tcs = NULL, *tps = NULL, *p3, *p2, *nt;
 double *fd1, *fd2, l, N[3] = {0}, fder[16] = {0};

  if(!o || !result)
    return AY_ENULL;

  if(o->type != AY_IDNPATCH)
    return AY_EWTYPE;

  if(!o->down || !o->down->next)
    return AY_ERROR;

  np = (ay_nurbpatch_object *)o->refine;

  if(param == 0.0)
    qf = ay_stess_GetQF(ay_prefs.glu_sampling_tolerance);
  else
    qf = ay_stess_GetQF(param);

  ay_status = ay_stess_TessTrimCurves(o, qf,
				      &tcslen, &tcs,
				      &tcslens, NULL);
  if(ay_status)
    goto cleanup;

  if(tnum > tcslen)
    { ay_status = AY_ERROR; goto cleanup; }

  ay_status = ay_stess_ReTessTrimCurves(o, qf,
					tcslen, tcs,
					tcslens, &tps);
  if(ay_status)
    goto cleanup;

  p3 = tps;
  for(i = 0; i < tnum-1; i++)
    {
      p3 += tcslens[i]*3;
    }

  ay_status = ay_nct_create(4, tcslens[tnum-1]+2, AY_KTBSPLINE, NULL, NULL,
			    &nc);
  if(ay_status)
    goto cleanup;

  memcpy(nc->controlv, p3+(tcslens[tnum-1]-2)*3, 3*sizeof(double));
  nc->controlv[3] = 1.0;
  p2 = &(nc->controlv[4]);
  for(i = 0; i < tcslens[tnum-1]-1; i++)
    {
      memcpy(p2, p3, 3*sizeof(double));
      p2[3] = 1.0;
      p3 += 3;
      p2 += 4;
    }
  memcpy(p2, &(nc->controlv[4]), 8*sizeof(double));

  nc->type = AY_CTPERIODIC;

  if(extractnt)
    {
      p2 = tcs[tnum-1];
      if(extractnt == 2)
	stride = 9;
      else
	stride = 3;

      if(!(nt = malloc(nc->length*stride*sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      a = stride;
      for(i = 0; i < tcslens[tnum-1]-1; i++)
	{
	  if(np->is_rat)
	    ay_nb_CompFirstDerSurf4D(np->width-1, np->height-1,
				     np->uorder-1, np->vorder-1,
				     np->uknotv, np->vknotv, np->controlv,
				     *p2, *(p2+1),
				     fder);
	  else
	    ay_nb_CompFirstDerSurf3D(np->width-1, np->height-1,
				     np->uorder-1, np->vorder-1,
				     np->uknotv, np->vknotv, np->controlv,
				     *p2, *(p2+1),
				     fder);

	  fd1 = &(fder[3]);
	  fd2 = &(fder[6]);
	  AY_V3CROSS(N, fd2, fd1);
	  l = AY_V3LEN(N);
	  if(fabs(l) > AY_EPSILON)
	    AY_V3SCAL(N, 1.0/l);

	  memcpy(&(nt[a]), N, 3*sizeof(double));
	  a += 3;

	  if(extractnt == 2)
	    {
	      memcpy(&(nt[a]), fd1, 3*sizeof(double));
	      a += 3;
	      memcpy(&(nt[a]), fd2, 3*sizeof(double));
	      a += 3;
	    }

	  p2 += 2;
	} /* for */

      memcpy(nt, &(nt[a-stride]), stride*sizeof(double));
      memcpy(&(nt[a]), &(nt[stride]), 2*stride*sizeof(double));

      *pvnt = nt;
    } /* if extractnt */

  /* return result */
  *result = (ay_nurbcurve_object*)nc;

cleanup:

  if(tcs)
    {
      for(i = 0; i < tcslen; i++)
	{
	  free(tcs[i]);
	}
      free(tcs);
    }

  if(tcslens)
    free(tcslens);

  if(tps)
    free(tps);

 return ay_status;
} /* ay_npt_extracttrim */


/** ay_npt_extractnc:
 *  extract a NURBS curve from the NURBS patch \a o
 *
 * \param[in] o NURBS patch object to process
 * \param[in] side specifies extraction of a boundary curve (0-3), of a
 *  curve at a specific parametric value (4 - along u dimension,
 *  5 - along v dimension), the complete boundary curve (6), the
 *  middle axis (7 along u, 8 along v), or a trim curve (>8)
 * \param[in] param parametric value at which curve is extracted;
 *  this parameter is ignored for the extraction of boundary curves;
 *  this parameter controls the sampling density for trim curves
 * \param[in] relative should \a param be interpreted in a relative way
 *  wrt. the knot vector?; this parameter is ignored for the extraction
 *  of boundary and trim curves
 * \param[in] apply_trafo this parameter controls whether trafos of \a o
 *  should be copied to the curve, or applied to the control points of the
 *  curve
 * \param[in] extractnt should the normals/tangents be calculated and stored
 *  in PV tags?; 0 - no, 1 - normals, 2 - normals and tangents
 * \param[in] pvnt pointer where to store the normals/tangents, may be
 *  NULL, if \a extractnt is 0
 * \param[in,out] result pointer where to store the resulting curve
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_extractnc(ay_object *o, int side, double param, int relative,
		 int apply_trafo, int extractnt,
		 double **pvnt,
		 ay_nurbcurve_object **result)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *np = NULL;
 ay_nurbpatch_object npt = {0};
 ay_nurbcurve_object *nc = NULL;
 double *cv = NULL, *nv = NULL, m[16] = {0}, *Qw = NULL, *UVQ = NULL;
 double uv, uvmin, uvmax;
 int stride = 4, i, a, b, k = 0, s = 0, r = 0;
 ay_npt_gndcb *gnducb = ay_npt_gndu;
 ay_npt_gndcb *gndvcb = ay_npt_gndv;

  if(!o || !result)
    return AY_ENULL;

  if(o->type != AY_IDNPATCH)
    return AY_EWTYPE;

  if(side > 8)
    {
      return ay_npt_extracttrim(o, side-8, param, apply_trafo, extractnt,
				pvnt, result);
    }

  if(side == 6)
    {
      return ay_npt_extractboundary(o, apply_trafo, result);
    }

  if(!(nc = calloc(1, sizeof(ay_nurbcurve_object))))
    return AY_EOMEM;

  np = (ay_nurbpatch_object *)o->refine;

  switch(side)
    {
    case 0:
    case 1:
      nc->order = np->uorder;
      nc->knot_type = np->uknot_type;
      nc->length = np->width;
      break;
    case 2:
    case 3:
      nc->order = np->vorder;
      nc->knot_type = np->vknot_type;
      nc->length = np->height;
      break;
    case 4:
      nc->order = np->uorder;
      nc->knot_type = np->uknot_type;
      nc->length = np->width;
      break;
    case 5:
      nc->order = np->vorder;
      nc->knot_type = np->vknot_type;
      nc->length = np->height;
      break;
    case 7:
      nc->order = np->uorder;
      nc->knot_type = np->uknot_type;
      nc->length = np->width;
      break;
    case 8:
      nc->order = np->vorder;
      nc->knot_type = np->vknot_type;
      nc->length = np->height;
      break;
    default:
      ay_status = AY_ERROR;
      goto cleanup;
    } /* switch */

  if(!(nc->knotv = malloc((nc->length+nc->order)*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(nc->controlv = malloc(nc->length*stride*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* copy controlv and knotv */
  cv = nc->controlv;
  switch(side)
    {
    case 0:
      /* u0 */
      a = 0;
      for(i = 0; i < nc->length*stride; i += stride)
	{
	  memcpy(&(cv[i]), &(np->controlv[a]), stride*sizeof(double));
	  a += np->height*stride;
	}
      memcpy(nc->knotv, np->uknotv, (nc->length+nc->order)*sizeof(double));
      break;
    case 1:
      /* un */
      a = (np->height-1)*stride;
      for(i = 0; i < nc->length*stride; i += stride)
	{
	  memcpy(&(cv[i]), &(np->controlv[a]), stride*sizeof(double));
	  a += np->height*stride;
	}
      memcpy(nc->knotv, np->uknotv, (nc->length+nc->order)*sizeof(double));
      break;
    case 2:
      /* v0 */
      memcpy(nc->controlv, np->controlv, nc->length*stride*sizeof(double));
      memcpy(nc->knotv, np->vknotv, (nc->length+nc->order)*sizeof(double));
      break;
    case 3:
      /* vn */
      memcpy(nc->controlv,
	     &(np->controlv[((np->width*np->height)-np->height)*stride]),
	nc->length*stride*sizeof(double));
      memcpy(nc->knotv, np->vknotv, (nc->length+nc->order)*sizeof(double));
      break;
    case 4:
      /* along u */

      if(relative)
	{
	  uvmin = np->vknotv[np->vorder-1];
	  uvmax = np->vknotv[np->height];
	  uv = uvmin + ((uvmax - uvmin) * param);
	}
      else
	{
	  uv = param;
	}

      k = ay_nb_FindSpanMult(np->height-1, np->vorder-1, uv,
			     np->vknotv, &s);

      r = np->vorder-s-1;
      if(r > 0)
	{
	  if(!(Qw = malloc(((np->height+r)*np->width)*stride*sizeof(double))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  if(!(UVQ = malloc((np->height+np->vorder+r)*sizeof(double))))
	    { ay_status = AY_EOMEM; goto cleanup; }

	  ay_status = ay_nb_InsertKnotSurfV(stride, np->width-1, np->height-1,
				       np->vorder-1, np->vknotv, np->controlv,
					    uv, k, s, r,
					    UVQ, Qw);
	  if(ay_status)
	    { goto cleanup; }
	}
      else
	{
	  Qw = np->controlv;
	}


      if(r > 0)
	 a = k - (np->vorder-1) + (np->vorder-1-s+r-1)/2 + 1;
       else
	 a = k - (np->vorder-1);

      a *= stride;

      for(i = 0; i < nc->length*stride; i += stride)
	{
	  memcpy(&(cv[i]), &(Qw[a]), stride*sizeof(double));
	  a += (np->height+((r>0)?r:0))*stride;
	}

      /* prevent cleanup code from doing something harmful */
      if(r < 1)
	Qw = NULL;

      memcpy(nc->knotv, np->uknotv, (nc->length+nc->order)*sizeof(double));

      break;
    case 5:
      /* along v */

      if(relative)
	{
	  uvmin = np->uknotv[np->uorder-1];
	  uvmax = np->uknotv[np->width];
	  uv = uvmin + ((uvmax - uvmin) * param);
	}
      else
	{
	  uv = param;
	}

      k = ay_nb_FindSpanMult(np->width-1, np->uorder-1, uv,
			     np->uknotv, &s);

      r = np->uorder-s-1;
      if(r > 0)
	{
	  if(!(Qw = malloc(((np->width+r)*np->height)*stride*sizeof(double))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  if(!(UVQ = malloc((np->width+np->uorder+r)*sizeof(double))))
	    { ay_status = AY_EOMEM; goto cleanup; }

	  ay_status = ay_nb_InsertKnotSurfU(stride, np->width-1, np->height-1,
				       np->uorder-1, np->uknotv, np->controlv,
					    uv, k, s, r,
					    UVQ, Qw);
	  if(ay_status)
	    { goto cleanup; }
	}
      else
	{
	  Qw = np->controlv;
	}

      if(r > 0)
	 a = k - (np->uorder-1) + (np->uorder-1-s+r-1)/2 + 1;
       else
	 a = k - (np->uorder-1);

      a *= np->height*stride;

      memcpy(&(cv[0]), &(Qw[a]), np->height*stride*sizeof(double));

      /* prevent cleanup code from doing something harmful */
      if(r < 1)
	Qw = NULL;

      memcpy(nc->knotv, np->vknotv, (nc->length+nc->order)*sizeof(double));
      break;
    case 7:
      /* middle u */
      for(i = 0; i < nc->length; i++)
	{
	  ay_geom_extractmiddlepoint(0, &(np->controlv[i*np->height*stride]),
				     np->height, 4,
				     NULL, &cv[i*stride]);
	  if(!np->is_rat)
	    cv[i*stride+3] = 1.0;
	}
      memcpy(nc->knotv, np->uknotv, (nc->length+nc->order)*sizeof(double));
      break;
    case 8:
      /* middle v */
      for(i = 0; i < nc->length; i++)
	{
	  ay_geom_extractmiddlepoint(0, &(np->controlv[i*stride]),
				     np->width, np->height*stride,
				     NULL, &cv[i*stride]);
	  if(!np->is_rat)
	    cv[i*stride+3] = 1.0;
	}
      memcpy(nc->knotv, np->vknotv, (nc->length+nc->order)*sizeof(double));
      break;
    default:
      ay_status = AY_ERROR;
      goto cleanup;
    } /* switch */

  /* apply trafos */
  if(apply_trafo)
    {
      ay_trafo_creatematrix(o, m);

      cv = nc->controlv;
      b = 0;
      for(i = 0; i < nc->length; i++)
	{
	  ay_trafo_apply3(&(cv[b]), m);
	  b += stride;
	}
    }

  /* calculate curve normals */
  if(extractnt)
    {
      if(extractnt == 2)
	stride = 9;
      else
	stride = 3;

      if(!(nv = malloc(nc->length*stride*sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      if(np->utype == AY_CTCLOSED)
	gnducb = ay_npt_gnduc;
      else
	if(np->utype == AY_CTPERIODIC)
	  gnducb = ay_npt_gndup;

      if(np->vtype == AY_CTCLOSED)
	gndvcb = ay_npt_gndvc;
      else
	if(np->vtype == AY_CTPERIODIC)
	  gndvcb = ay_npt_gndvp;

      switch(side)
	{
	case 0:
	  a = 0;
	  for(i = 0; i < nc->length; i++)
	    {
	      ay_npt_getnormal(np, i, 0, gnducb, gndvcb, (extractnt==2),
			       &(nv[a]));
	      a += stride;
	    }
	  break;
	case 1:
	  a = 0;
	  for(i = 0; i < nc->length; i++)
	    {
	      ay_npt_getnormal(np, i, np->height-1, gnducb, gndvcb,
			       (extractnt==2), &(nv[a]));
	      a += stride;
	    }
	  break;
	case 2:
	  a = 0;
	  for(i = 0; i < nc->length; i++)
	    {
	      ay_npt_getnormal(np, 0, i, gnducb, gndvcb, (extractnt==2),
			       &(nv[a]));
	      a += stride;
	    }
	  break;
	case 3:
	  a = 0;
	  for(i = 0; i < nc->length; i++)
	    {
	      ay_npt_getnormal(np, np->width-1, i, gnducb, gndvcb,
			       (extractnt==2), &(nv[a]));
	      a += stride;
	    }
	  break;
	case 4:
	  /* assume k, s, r are still correctly set */
	  memcpy(&npt, np, sizeof(ay_nurbpatch_object));

	  /* if we inserted knots... */
	  if(r > 0)
	    {
	      /* ...correct npt */
	      npt.height += r;
	      npt.controlv = Qw;
	      npt.vknotv = UVQ;

	      a = k - (np->vorder-1) + (np->vorder-1-s+r-1)/2 + 1;
	    }
	  else
	    {
	      a = k - (np->vorder-1);
	    }

	  b = 0;
	  for(i = 0; i < nc->length; i++)
	    {
	      ay_npt_getnormal(&npt, i, a, gnducb, gndvcb, (extractnt==2),
			       &(nv[b]));
	      b += stride;
	    }
	  break;
	case 5:
	  /* assume k, s, r are still correctly set */
	  memcpy(&npt, np, sizeof(ay_nurbpatch_object));

	  /* if we inserted knots... */
	  if(r > 0)
	    {
	      /* ...correct npt */
	      npt.width += r;
	      npt.controlv = Qw;
	      npt.uknotv = UVQ;

	      a = k - (np->uorder-1) + (np->uorder-1-s+r-1)/2 + 1;
	    }
	  else
	    {
	      a = k - (np->uorder-1);
	    }

	  b = 0;
	  for(i = 0; i < nc->length; i++)
	    {
	      ay_npt_getnormal(&npt, a, i, gnducb, gndvcb, (extractnt==2),
			       &(nv[b]));
	      b += stride;
	    }
	  break;
	default:
	  free(nv);
	  nv = NULL;
	  break;
	} /* switch */
    } /* if */

  nc->is_rat = ay_nct_israt(nc);

  ay_nct_settype(nc);

  /* return result */
  *result = nc;
  if(nv)
    *pvnt = nv;

  /* prevent cleanup code from doing something harmful */
  nc = NULL;

cleanup:

  if(nc)
    {
      if(nc->knotv)
	free(nc->knotv);

      if(nc->controlv)
	free(nc->controlv);

      free(nc);
    } /* if */

  if(Qw)
    free(Qw);

  if(UVQ)
    free(UVQ);

 return ay_status;
} /* ay_npt_extractnc */


/** ay_npt_israt:
 *  check whether any control point of a NURBS patch
 *  uses a weight value (!= 1.0)
 *
 * \param[in] np NURBS patch to check
 *
 * \returns AY_TRUE if patch is rational, AY_FALSE else.
 */
int
ay_npt_israt(ay_nurbpatch_object *np)
{
 int i, j;
 double *p;

  if(!np)
    return AY_FALSE;

  p = &(np->controlv[3]);
  for(i = 0; i < np->width; i++)
    {
      for(j = 0; j < np->height; j++)
	{
	  if((fabs(*p) < (1.0-AY_EPSILON)) || (fabs(*p) > (1.0+AY_EPSILON)))
	    return AY_TRUE;
	  p += 4;
	} /* for */
    } /* for */

 return AY_FALSE;
} /* ay_npt_israt */


/** ay_npt_isboundcurve:
 * Check whether curve is boundary.
 *  (helper for ay_npt_istrimmed() below)
 *
 * \param[in] o curve to check
 * \param[in] b1 parametric value of left boundary (u0)
 * \param[in] b2 parametric value of right boundary (un)
 * \param[in] b3 parametric value of lower boundary (v0)
 * \param[in] b4 parametric value of upper boundary (vn)
 * \param[in,out] result AY_TRUE if each section is on exactly one
 *                boundary else AY_FALSE
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_isboundcurve(ay_object *o, double b1, double b2, double b3, double b4,
		    int *result)
{
 int ay_status = AY_OK;
 int i = 0, j = 0, stride = 4;
 int apply_trafo = AY_FALSE, on_bound = AY_FALSE;
 double *cv = NULL, *tcv = NULL, *p = NULL, m[16];
 ay_nurbcurve_object *ncurve = NULL;
 ay_object *c = NULL;

  if(!o)
    return AY_ENULL;

  if(o->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(o, AY_IDNCURVE, &c);
      if(c)
	{
	  ncurve = (ay_nurbcurve_object *)c->refine;
	}
      else
	{
	  goto cleanup;
	}
    }
  else
    {
      ncurve = (ay_nurbcurve_object *)o->refine;
    }

  /* check, whether each curve section is on one boundary */
  cv = ncurve->controlv;

  /* apply transformations? */
  if(AY_ISTRAFO(o))
    {
      apply_trafo = AY_TRUE;
      ay_trafo_creatematrix(o, m);
    }

  tcv = malloc(ncurve->length*stride*sizeof(double));

  if(!tcv)
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  p = tcv;
  for(i = 0; i < ncurve->length; i++)
    {
      if(apply_trafo)
	{
	  AY_APTRAN3(p,cv,m);
	  /* multiply in the weights to not get fooled
	     by e.g. a 9-point-circle */
	  p[0] *= cv[3];
	  p[1] *= cv[3];
	}
      else
	{
	  p[0] = cv[0]*cv[3];
	  p[1] = cv[1]*cv[3];
	}
      p += stride;
      cv += stride;
    }

  cv = tcv;
  for(i = 0; i < ncurve->length-1; i++)
    {
      j = i*stride;
      on_bound = AY_FALSE;
      /* check against bound1 */
      if(!on_bound && (fabs(cv[j]-b1) < AY_EPSILON))
	{
	  if(fabs(cv[j+stride]-b1) < AY_EPSILON)
	    {
	      on_bound = AY_TRUE;
	    }
	} /* if */
      /* check against bound2 */
      if(!on_bound && (fabs(cv[j]-b2) < AY_EPSILON))
	{
	  if(fabs(cv[j+stride]-b2) < AY_EPSILON)
	    {
	      on_bound = AY_TRUE;
	    }
	} /* if */
      /* check against bound3 */
      if(!on_bound && (fabs(cv[j+1]-b3) < AY_EPSILON))
	{
	  if(fabs(cv[j+stride+1]-b3) < AY_EPSILON)
	    {
	      on_bound = AY_TRUE;
	    }
	} /* if */
      /* check against bound4 */
      if(!on_bound && (fabs(cv[j+1]-b4) < AY_EPSILON))
	{
	  if(fabs(cv[j+stride+1]-b4) < AY_EPSILON)
	    {
	      on_bound = AY_TRUE;
	    }
	} /* if */

      if(!on_bound)
	{
	  *result = AY_FALSE;
	  goto cleanup;
	}
    } /* for */

  /*
   * if we get here, all curve segments are each on one boundary
   */
  *result = AY_TRUE;

cleanup:

  if(c)
    (void)ay_object_deletemulti(c, AY_TRUE);

  if(tcv)
    free(tcv);

 return ay_status;
} /* ay_npt_isboundcurve */


/** ay_npt_istrimmed:
 * Check whether NURBS patch is trimmed.
 * Does not work well for degenerate patches.
 *
 * \param[in] mode operation mode: 0 - check if non-trivially trimmed,
 * 1 - check if trivially trimmed
 *
 * \returns AY_FALSE in error and if trivially trimmed,
 *          AY_TRUE if non-trivially trimmed
 */
int
ay_npt_istrimmed(ay_object *o, int mode)
{
 /*int ay_status = AY_OK;*/
 int is_bound = AY_FALSE;
 ay_object *down;
 ay_nurbpatch_object *npatch;
 double b1, b2, b3, b4;

  if(!o)
    return AY_FALSE;

  if(o->type != AY_IDNPATCH)
    return AY_FALSE;

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_FALSE;

  b1 = npatch->uknotv[npatch->uorder-1];
  b2 = npatch->uknotv[npatch->width];
  b3 = npatch->vknotv[npatch->vorder-1];
  b4 = npatch->vknotv[npatch->height];

  switch(mode)
    {
    case 0:
      if(!o->down || (o->down && !o->down->next))
	return AY_FALSE; /* no child or just one child (EndLevel) */

      /* if we get here o has atleast one real child */
      if(o->down->next->next)
	return AY_TRUE; /* more than one real child -> non-trivially trimmed */

      /* check the one real child */
      if(o->down->type != AY_IDLEVEL)
	{
	  /* process single trim curve */
	  ay_npt_isboundcurve(o->down, b1, b2, b3, b4, &is_bound);
	  if(!is_bound)
	    return AY_TRUE;
	  else
	    return AY_FALSE;
	}
      else
	{
	  /* process trim loop */
	  down = o->down->down;
	  while(down->next)
	    {
	      ay_npt_isboundcurve(down, b1, b2, b3, b4, &is_bound);
	      if(!is_bound)
		return AY_TRUE;

	      down = down->next;
	    } /* while */

	  /*
	   * if we get here, all curve segments of all trim loop elements
	   * are each on one boundary => conclude trivial trim
	   */
	  return AY_FALSE;
	} /* if */
      break;
    case 1:
      if(!o->down || !o->down->next)
	return AY_FALSE; /* no child or just one child (EndLevel) */

      /* if we get here o has atleast one real child */
      if(o->down->next->next)
	return AY_FALSE;

      if(o->down->type != AY_IDLEVEL)
	{
	  /* process single trim curve */
	  ay_npt_isboundcurve(o->down, b1, b2, b3, b4, &is_bound);
	  return is_bound;
	}
      else
	{
	  /* process trim loop */
	  down = o->down->down;
	  while(down->next)
	    {
	      ay_npt_isboundcurve(down, b1, b2, b3, b4, &is_bound);
	      if(!is_bound)
		return AY_FALSE;

	      down = down->next;
	    } /* while */

	  /*
	   * if we get here, all curve segments of all trim loop elements
	   * are each on one boundary => conclude trivial trim
	   */
	  return AY_TRUE;
	}
      break;
    default:
      break;
    } /* switch */

 return AY_FALSE;
} /* ay_npt_istrimmed */


/* ay_npt_closeu:
 *  close NURBS patch \a np in u direction according to \a mode:
 *  0:
 *  by copying the first control point line to the last,
 *  1:
 *  by copying the last control point line to the first,
 *  2:
 *  by copying the mean of the first and last control point line
 *  to the first and last control point line,
 *  3:
 *  by copying the first p control point
 *  lines over to the last p control point lines,
 *  4:
 *  by copying the last p control point
 *  lines over to the first p control point lines,
 *  5:
 *  by copying the mean of the first p and last p control point
 *  lines over to the first p and last p control point lines
 *  respectively;
 *
 *  where p is the degree of the NURBS surface in u direction,
 *  and the lines run in v direction.
 *
 *  Note: this does not guarantee a closed surface unless the u knot
 *  vector is of the correct configuration or type e.g. of type AY_KTBSPLINE
 *  for the modes 3-5.
 *
 * \param[in,out] np NURBS patch to process
 * \param[in] mode operation mode, see above
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_closeu(ay_nurbpatch_object *np, int mode)
{
 double *controlv, *start, *end, mean[3];
 int stride = 4, lstride;
 int i, j;

  controlv = np->controlv;

  switch(mode)
    {
    case 0:
      for(i = 0; i < np->height; i++)
	{
	  start = &(controlv[i*stride]);
	  end = start+(np->width-1)*np->height*stride;
	  memcpy(end, start, stride*sizeof(double));
	}
      break;
    case 1:
      for(i = 0; i < np->height; i++)
	{
	  start = &(controlv[i*stride]);
	  end = start+(np->width-1)*np->height*stride;
	  memcpy(start, end, stride*sizeof(double));
	}
      break;
    case 2:
      for(i = 0; i < np->height; i++)
	{
	  start = &(controlv[i*stride]);
	  end = start+(np->width-1)*np->height*stride;
	  mean[0] = start[0]+((end[0]-start[0])/2.0);
	  mean[1] = start[1]+((end[1]-start[1])/2.0);
	  mean[2] = start[2]+((end[2]-start[2])/2.0);

	  memcpy(start, mean, stride*sizeof(double));
	  memcpy(end, mean, stride*sizeof(double));
	}
      break;
    case 3:
      if(np->width >= ((np->uorder-1)*2))
	{
	  start = controlv;
	  end = &(controlv[np->height*(np->width-(np->uorder-1))*stride]);
	  memcpy(end, start, (np->uorder-1)*np->height*stride*sizeof(double));
	  /*
	    for(i = 0; i < (np->uorder-1); i++)
	    {
	    memcpy(end, controlv, np->height*stride*sizeof(double));
	    controlv += np->height*stride;
	    end += np->height*stride;
	    }
	  */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
      break;
    case 4:
      if(np->width >= ((np->uorder-1)*2))
	{
	  start = controlv;
	  end = &(controlv[np->height*(np->width-(np->uorder-1))*stride]);
	  memcpy(start, end, (np->uorder-1)*np->height*stride*sizeof(double));
	  /*
	    for(i = 0; i < (np->uorder-1); i++)
	    {
	    memcpy(end, controlv, np->height*stride*sizeof(double));
	    controlv += np->height*stride;
	    end += np->height*stride;
	    }
	  */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
      break;
    case 5:
      if(np->width >= ((np->uorder-1)*2))
	{
	  lstride = np->height*stride;
	  for(i = 0; i < np->height; i++)
	    {
	      start = &(controlv[i*np->height*stride]);
	      end = start+(np->width-(np->uorder-1)*lstride);
	      for(j = 0; j < np->uorder-1; j++)
		{
		  mean[0] = start[j*lstride] +
		    ((end[j*lstride]-start[j*lstride])/2.0);
		  mean[1] = start[j*lstride+1] +
		    ((end[j*lstride+1]-start[j*lstride+1])/2.0);
		  mean[2] = start[j*lstride+2] +
		    ((end[j*lstride+2]-start[j*lstride+2])/2.0);

		  memcpy(&(start[j*lstride]), mean, 3*sizeof(double));
		  memcpy(&(end[j*lstride]), mean, 3*sizeof(double));
		} /* for */
	    } /* for */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
      break;
    default:
      break;
    } /* if */

 return AY_OK;
} /* ay_npt_closeu */


/** ay_npt_closeutcmd:
 *  Close selected NURBS patches in U direction.
 *  Implements the \a closeuS scripting interface command.
 *  See also the corresponding section in the \ayd{sccloseus}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_closeutcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int tcl_status, ay_status = AY_OK;
 int i = 1, mode = 3, extend = AY_TRUE, knots = -1, stride = 4;
 int notify_parent = AY_FALSE;
 double *newcontrolv = NULL, *tknotv;
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *np = NULL;

  /* parse args */
  if(argc > 2)
    {
      while(i+1 < argc)
	{
	  if(argv[i][0] == '-' && argv[i][1] == 'm')
	    {
	      /* -mode */
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &mode);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	    }
	  if(argv[i][0] == '-' && argv[i][1] == 'e')
	    {
	      /* -extend */
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &extend);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	    }
	  if(argv[i][0] == '-' && argv[i][1] == 'k')
	    {
	      /* -knottype */
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &knots);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	    }
	  i += 2;
	} /* while */
    } /* if */

  while(sel)
    {
      switch(sel->object->type)
	{
	case AY_IDNPATCH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  np = (ay_nurbpatch_object *)sel->object->refine;

	  if(mode < 3)
	    {
	      if(extend)
		{
		  if(!(newcontrolv = malloc((np->width+1) *
					    np->height * stride *
					    sizeof(double))))
		    {
		      ay_error(AY_EOMEM, argv[0], NULL);
		      return TCL_OK;
		    }

		  memcpy(newcontrolv, np->controlv,
			 np->width * np->height * stride *
			 sizeof(double));

		  free(np->controlv);
		  np->controlv = newcontrolv;

		  np->width++;
		}

	      if(knots > -1 || extend)
		{
		  if(knots == -1)
		    knots = AY_KTNURB;
		  tknotv = np->vknotv;
		  np->vknotv = NULL;
		  np->uknot_type = knots;
		  ay_status = ay_knots_createnp(np);
		  free(np->vknotv);
		  np->vknotv = tknotv;
		}
	    }
	  else
	    {
	      /* mode > 3 => make periodic*/
	      if(extend)
		{
		  if(!(newcontrolv = malloc((np->width + (np->uorder-1)) *
					    np->height * stride *
					    sizeof(double))))
		    {
		      ay_error(AY_EOMEM, argv[0], NULL);
		      return TCL_OK;
		    }

		  memcpy(newcontrolv, np->controlv,
			 np->width * np->height * stride *
			 sizeof(double));

		  free(np->controlv);
		  np->controlv = newcontrolv;

		  np->width += (np->uorder-1);
		  /*
		    if(!(newknotv = calloc(np->width + np->uorder +
                         (np->uorder-1),
			 sizeof(double))))
			 {
			 ay_error(AY_EOMEM, argv[0], NULL);
			 return TCL_OK;
			 }

			 memcpy(newknotv, np->uknotv, (np->width + np->uorder +
			 (np->uorder-1)) *
			 sizeof(double));

			 free(np->uknotv);
			 np->uknotv = newknotv;
		  */
		}

	      if(knots > -1 || extend)
		{
		  if(knots == -1)
		    knots = AY_KTBSPLINE;
		  tknotv = np->vknotv;
		  np->vknotv = NULL;
		  np->uknot_type = knots;
		  ay_status = ay_knots_createnp(np);
		  free(np->vknotv);
		  np->vknotv = tknotv;
		}
	    } /* if */

	  ay_status = ay_npt_closeu(np, mode);

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "Error closing object.");
	    }

	  ay_npt_recreatemp(np);

	  sel->object->modified = AY_TRUE;

	  /* re-create tesselation of patch */
	  (void)ay_notify_object(sel->object);
	  notify_parent = AY_TRUE;
	  break;
	default:
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	  break;
	} /* switch */
      sel = sel->next;
    } /* while */

  if(notify_parent)
    (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_closeutcmd */


/* ay_npt_closev:
 *  close NURBS patch \a np in v direction according to \a mode:
 *  0:
 *  by copying the first control point line to the last,
 *  1:
 *  by copying the last control point line to the first,
 *  2:
 *  by copying the mean of the first and the last control point line
 *  to the first and last control point line,
 *  3:
 *  by copying the first q control point
 *  lines over to the last q control point lines,
 *  4:
 *  by copying the last q control point
 *  lines over to the first q control point lines,
 *  5:
 *  by copying the mean of the first q and last q control point
 *  lines over to the first q and last q control point lines
 *  respectively;
 *
 *  where q is the degree of the NURBS surface in v direction,
 *  and the lines run in u direction.
 *
 *  Note: this does not guarantee a closed surface unless the v knot
 *  vector is of the correct configuration or type e.g. of type AY_KTBSPLINE
 *  for the modes 3-5.
 *
 * \param[in,out] np NURBS patch to process
 * \param[in] mode operation mode, see above
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_closev(ay_nurbpatch_object *np, int mode)
{
 double *controlv, *start, *end, mean[3];
 int stride = 4;
 int i, j;

  controlv = np->controlv;

  switch(mode)
    {
    case 0:
      for(i = 0; i < np->width; i++)
	{
	  start = &(controlv[i*np->height*stride]);
	  end = start+(np->height-1)*stride;
	  memcpy(end, start, stride*sizeof(double));
	}
      break;
    case 1:
      for(i = 0; i < np->width; i++)
	{
	  start = &(controlv[i*np->height*stride]);
	  end = start+(np->height-1)*stride;
	  memcpy(start, end, stride*sizeof(double));
	}
      break;
    case 2:
      for(i = 0; i < np->width; i++)
	{
	  start = &(controlv[i*np->height*stride]);
	  end = start+(np->height-1)*stride;
	  mean[0] = start[0]+((end[0]-start[0])/2.0);
	  mean[1] = start[1]+((end[1]-start[1])/2.0);
	  mean[2] = start[2]+((end[2]-start[2])/2.0);

	  memcpy(start, mean, 3*sizeof(double));
	  memcpy(end, mean, 3*sizeof(double));
	}
      break;
    case 3:
      if(np->height >= ((np->vorder-1)*2))
	{
	  for(i = 0; i < np->width; i++)
	    {
	      start = &(controlv[i*np->height*stride]);
	      end = start+((np->height-(np->vorder-1))*stride);

	      memcpy(end, start, (np->vorder-1)*stride*sizeof(double));
	    } /* for */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
      break;
    case 4:
      if(np->height >= ((np->vorder-1)*2))
	{
	  for(i = 0; i < np->width; i++)
	    {
	      start = &(controlv[i*np->height*stride]);
	      end = start+((np->height-(np->vorder-1))*stride);

	      memcpy(start, end, (np->vorder-1)*stride*sizeof(double));
	    } /* for */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
      break;
    case 5:
      if(np->height >= ((np->vorder-1)*2))
	{
	  for(i = 0; i < np->width; i++)
	    {
	      start = &(controlv[i*np->height*stride]);
	      end = start+((np->height-(np->vorder-1))*stride);
	      for(j = 0; j < np->vorder-1; j++)
		{
		  mean[0] = start[j*stride] +
		    ((end[j*stride]-start[j*stride])/2.0);
		  mean[1] = start[j*stride+1] +
		    ((end[j*stride+1]-start[j*stride+1])/2.0);
		  mean[2] = start[j*stride+2] +
		    ((end[j*stride+2]-start[j*stride+2])/2.0);

		  memcpy(&(start[j*stride]), mean, 3*sizeof(double));
		  memcpy(&(end[j*stride]), mean, 3*sizeof(double));
		} /* for */
	    } /* for */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
      break;
    default:
      break;
    } /* if */

 return AY_OK;
} /* ay_npt_closev */


/** ay_npt_closevtcmd:
 *  Close selected NURBS patches in V direction.
 *  Implements the \a closevS scripting interface command.
 *  See also the corresponding section in the \ayd{scclosevs}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_closevtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int tcl_status, ay_status = AY_OK;
 int i = 1, mode = 3, extend = AY_TRUE, knots = -1, stride = 4;
 int notify_parent = AY_FALSE;
 double *a, *b;
 double *newcontrolv = NULL, *tknotv;
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *np = NULL;

  /* parse args */
  if(argc > 2)
    {
      while(i+1 < argc)
	{
	  if(argv[i][0] == '-' && argv[i][1] == 'm')
	    {
	      /* -mode */
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &mode);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	    }
	  if(argv[i][0] == '-' && argv[i][1] == 'e')
	    {
	      /* -extend */
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &extend);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	    }
	  if(argv[i][0] == '-' && argv[i][1] == 'k')
	    {
	      /* -knottype */
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &knots);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	    }
	  i += 2;
	} /* while */
    } /* if */

  while(sel)
    {
      switch(sel->object->type)
	{
	case AY_IDNPATCH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  np = (ay_nurbpatch_object *)sel->object->refine;

	  if(mode < 3)
	    {
	      if(extend)
		{
		  if(!(newcontrolv = malloc(np->width *
					    (np->height + 1) * stride *
					    sizeof(double))))
		    {
		      ay_error(AY_EOMEM, argv[0], NULL);
		      return TCL_OK;
		    }

		  a = &(newcontrolv[0]);
		  b = &(np->controlv[0]);
		  for(i = 0; i < np->width; i++)
		    {
		      memcpy(a, b, np->height * stride * sizeof(double));
		      a += (np->height + 1) * stride;
		      b += np->height * stride;
		    }

		  free(np->controlv);
		  np->controlv = newcontrolv;

		  np->height++;
		}

	      if(knots >- 1 || extend)
		{
		  if(knots == -1)
		    knots = AY_KTNURB;
		  tknotv = np->uknotv;
		  np->uknotv = NULL;
		  np->vknot_type = knots;
		  ay_status = ay_knots_createnp(np);
		  free(np->uknotv);
		  np->uknotv = tknotv;
		}
	    }
	  else
	    {
	      /* mode > 3 => make periodic*/
	      if(extend)
		{
		  if(!(newcontrolv = malloc(np->width * (np->height +
					    (np->vorder-1)) * stride *
					    sizeof(double))))
		    {
		      ay_error(AY_EOMEM, argv[0], NULL);
		      return TCL_OK;
		    }

		  a = &(newcontrolv[0]);
		  b = &(np->controlv[0]);
		  for(i = 0; i < np->width; i++)
		    {
		      memcpy(a, b, np->height * stride * sizeof(double));
		      a += (np->height + (np->vorder-1)) * stride;
		      b += np->height * stride;
		    }

		  free(np->controlv);
		  np->controlv = newcontrolv;
		  np->height += (np->vorder-1);
		}

	      if(knots > -1 || extend)
		{
		  if(knots == -1)
		    knots = AY_KTBSPLINE;
		  tknotv = np->uknotv;
		  np->uknotv = NULL;
		  np->vknot_type = knots;
		  ay_status = ay_knots_createnp(np);
		  free(np->uknotv);
		  np->uknotv = tknotv;
		}
	    } /* if */

	  ay_status = ay_npt_closev(np, mode);

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "Error closing object.");
	    }

	  ay_npt_recreatemp(np);

	  sel->object->modified = AY_TRUE;

	  /* re-create tesselation of patch */
	  (void)ay_notify_object(sel->object);
	  notify_parent = AY_TRUE;
	  break;
	default:
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	  break;
	} /* switch */
      sel = sel->next;
    } /* while */

  if(notify_parent)
    (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_closevtcmd */


/** ay_npt_isplanar:
 *  check whether NURBS patch \a np is planar
 *
 *  \param[in] np NURBS patch to check
 *  \param[in,out] n pointer where the normal will be stored, may be NULL
 *
 *  \returns
 *   AY_TRUE - surface is planar
 *   AY_FALSE - else
 */
int
ay_npt_isplanar(ay_nurbpatch_object *np, double *n)
{
 double n1[3], n2[3];
 double *p1, *p2, *p3, *p4;
 int i, j, have_n1, have_n2;

  if(np->width > 2 || np->height > 2)
    {
      if((np->utype == AY_CTCLOSED) || (np->utype == AY_CTPERIODIC))
	return AY_FALSE;

      if((np->vtype == AY_CTCLOSED) || (np->vtype == AY_CTPERIODIC))
	return AY_FALSE;

      have_n1 = AY_FALSE;
      have_n2 = AY_FALSE;
      for(i = 0; i < np->width; i++)
	{
	  for(j = 0; j < np->height; j++)
	    {
	      if(!have_n1)
		{
		  memset(n1, 0, 3*sizeof(double));
		  ay_npt_getnormal(np, i, j,
				   ay_npt_gndu, ay_npt_gndv, AY_FALSE, n1);
		  if(fabs(n1[0]) > AY_EPSILON ||
		     fabs(n1[1]) > AY_EPSILON ||
		     fabs(n1[2]) > AY_EPSILON)
		    have_n1 = AY_TRUE;
		}
	      else
		{
		  memset(n2, 0, 3*sizeof(double));
		  ay_npt_getnormal(np, i, j,
				   ay_npt_gndu, ay_npt_gndv, AY_FALSE, n2);
		  if(fabs(n2[0]) > AY_EPSILON ||
		     fabs(n2[1]) > AY_EPSILON ||
		     fabs(n2[2]) > AY_EPSILON)
		    {
		      have_n2 = AY_TRUE;
		      if(!AY_V3COMP(n1, n2))
			return AY_FALSE;
		    }
		} /* if have_n1 */
	    } /* for j */
	} /* for i */

      if(!have_n2)
	return AY_FALSE;

      goto storen;
    } /* if width/height > 2 */

  p1 = np->controlv;
  p2 = &(np->controlv[4]);
  p3 = &(np->controlv[8]);
  p4 = &(np->controlv[12]);

  /* check for degenerate triangular patch first (which is planar
     by definition) */
  if(AY_V3COMP(p1, p2))
    {
      ay_geom_calcnfrom3(p1, p3, p4, n1);
      goto storen;
    }

  if(AY_V3COMP(p1, p3))
    {
      ay_geom_calcnfrom3(p1, p2, p4, n1);
      goto storen;
    }

  if(AY_V3COMP(p2, p4))
    {
      ay_geom_calcnfrom3(p2, p3, p1, n1);
      goto storen;
    }

  if(AY_V3COMP(p3, p4))
    {
      ay_geom_calcnfrom3(p3, p1, p2, n1);
      goto storen;
    }

  /* patch seems to be quadrilateral, check all normals */
  ay_geom_calcnfrom3(p1, p3, p2, n1);

  ay_geom_calcnfrom3(p1, p4, p2, n2);
  if(!AY_V3COMP(n1, n2))
    return AY_FALSE;

  ay_geom_calcnfrom3(p2, p3, p4, n2);
  if(!AY_V3COMP(n1, n2))
    return AY_FALSE;

  ay_geom_calcnfrom3(p4, p1, p3, n2);
  if(!AY_V3COMP(n1, n2))
    return AY_FALSE;

storen:

  if(n)
    memcpy(n, n1, 3*sizeof(double));

 return AY_TRUE;
} /* ay_npt_isplanar */


/** ay_npt_isclosedu:
 *  check whether NURBS patch \a np is closed in u direction
 *  by sampling the surface at the respective borders at parametric
 *  values defined by the knot vector (plus one intermediate value
 *  for each knot interval) and check the surface points at those
 *  parametric values for equality
 *
 *  \param[in] np NURBS patch to check
 *
 *  \returns
 *   AY_TRUE - surface is closed in u
 *   AY_FALSE - surface is open in u
 */
int
ay_npt_isclosedu(ay_nurbpatch_object *np)
{
 int i;
 double u1, u2, v;
 double p1[4], p2[4];

  if(np->width <= 2)
    return AY_FALSE;

  u1 = np->uknotv[np->uorder-1];
  u2 = np->uknotv[np->width];

  /* check closedness in U direction */
  for(i = np->vorder-1; i < np->height; i++)
    {
      /* check knot */
      v = np->vknotv[i];

      /* calculate and compare surface points */
      (void)ay_nb_SurfacePoint4D(np->width-1, np->height-1,
				 np->uorder-1, np->vorder-1,
				 np->uknotv, np->vknotv,
				 np->controlv,
				 u1,
				 v,
				 p1);

      (void)ay_nb_SurfacePoint4D(np->width-1, np->height-1,
				 np->uorder-1, np->vorder-1,
				 np->uknotv, np->vknotv,
				 np->controlv,
				 u2,
				 v,
				 p2);

      if(!AY_V3COMP(p1, p2))
	return AY_FALSE;

      /* check intermediate knot */
      v += (np->vknotv[i+1] - np->vknotv[i])/2.0;

      /* calculate and compare surface points */
      (void)ay_nb_SurfacePoint4D(np->width-1, np->height-1,
				 np->uorder-1, np->vorder-1,
				 np->uknotv, np->vknotv,
				 np->controlv,
				 u1,
				 v,
				 p1);

      (void)ay_nb_SurfacePoint4D(np->width-1, np->height-1,
				 np->uorder-1, np->vorder-1,
				 np->uknotv, np->vknotv,
				 np->controlv,
				 u2,
				 v,
				 p2);

      if(!AY_V3COMP(p1, p2))
	return AY_FALSE;
    } /* for */

  /* check last knot */
  v = np->vknotv[np->height];

  /* calculate and compare surface points */
  (void)ay_nb_SurfacePoint4D(np->width-1, np->height-1,
			     np->uorder-1, np->vorder-1,
			     np->uknotv, np->vknotv,
			     np->controlv,
			     u1,
			     v,
			     p1);

  (void)ay_nb_SurfacePoint4D(np->width-1, np->height-1,
			     np->uorder-1, np->vorder-1,
			     np->uknotv, np->vknotv,
			     np->controlv,
			     u2,
			     v,
			     p2);

  if(!AY_V3COMP(p1,p2))
    return AY_FALSE;

 return AY_TRUE;
} /* ay_npt_isclosedu */


/** ay_npt_isclosedv:
 *  check whether NURBS patch \a np is closed in v direction
 *  by sampling the surface at the respective borders at parametric
 *  values defined by the knot vector (plus one intermediate value
 *  for each knot interval) and check the surface points at those
 *  parametric values for equality
 *
 *  \param[in] np NURBS patch to check
 *
 *  \returns
 *   AY_TRUE - surface is closed in v
 *   AY_FALSE - surface is open in v
 */
int
ay_npt_isclosedv(ay_nurbpatch_object *np)
{
 int i;
 double u, v1, v2;
 double p1[4], p2[4];

  if(np->height <= 2)
    return AY_FALSE;

  v1 = np->vknotv[np->vorder-1];
  v2 = np->vknotv[np->height];

  /* check closedness in V direction */
  for(i = np->uorder-1; i < np->width; i++)
    {
      /* check knot */
      u = np->uknotv[i];

      /* calculate and compare surface points */
      (void)ay_nb_SurfacePoint4D(np->width-1, np->height-1,
				 np->uorder-1, np->vorder-1,
				 np->uknotv, np->vknotv,
				 np->controlv,
				 u,
				 v1,
				 p1);

      (void)ay_nb_SurfacePoint4D(np->width-1, np->height-1,
				 np->uorder-1, np->vorder-1,
				 np->uknotv, np->vknotv,
				 np->controlv,
				 u,
				 v2,
				 p2);

      if(!AY_V3COMP(p1,p2))
	return AY_FALSE;

      /* check intermediate knot */
      u += (np->uknotv[i+1] - np->uknotv[i])/2.0;

      /* calculate and compare surface points */
      (void)ay_nb_SurfacePoint4D(np->width-1, np->height-1,
				 np->uorder-1, np->vorder-1,
				 np->uknotv, np->vknotv,
				 np->controlv,
				 u,
				 v1,
				 p1);

      (void)ay_nb_SurfacePoint4D(np->width-1, np->height-1,
				 np->uorder-1, np->vorder-1,
				 np->uknotv, np->vknotv,
				 np->controlv,
				 u,
				 v2,
				 p2);

      if(!AY_V3COMP(p1,p2))
	return AY_FALSE;
    } /* for */

  /* check last knot */
  u = np->uknotv[np->width];

  /* calculate and compare surface points */
  (void)ay_nb_SurfacePoint4D(np->width-1, np->height-1,
			     np->uorder-1, np->vorder-1,
			     np->uknotv, np->vknotv,
			     np->controlv,
			     u,
			     v1,
			     p1);

  (void)ay_nb_SurfacePoint4D(np->width-1, np->height-1,
			     np->uorder-1, np->vorder-1,
			     np->uknotv, np->vknotv,
			     np->controlv,
			     u,
			     v2,
			     p2);

  if(!AY_V3COMP(p1,p2))
    return AY_FALSE;

 return AY_TRUE;
} /* ay_npt_isclosedv */


/** ay_npt_setuvtypes:
 *  set the utype and vtype (closedness) attributes according to
 *  the actual configuration of a NURBS patch
 *
 *  \param[in, out] np NURBS patch to process
 *  \param[in] dir direction to check: 0 - both, 1 - only U, 2 - only V
 */
void
ay_npt_setuvtypes(ay_nurbpatch_object *np, int dir)
{
 int stride = 4;
 int i, j, is_closed;
 double *s, *e;

  if(!np)
    return;

  /* U */
  if(dir == 0 || dir == 1)
    {
      np->utype = AY_CTOPEN;
      if(np->width > 2)
	{
	  is_closed = AY_TRUE;
	  for(i = 0; i < np->height; i++)
	    {
	      s = &(np->controlv[i*stride]);
	      e = s+((np->width-1)*np->height*stride);

	      if(!AY_V4COMP(s, e))
		{
		  is_closed = AY_FALSE;
		  break;
		}
	    }

	  if(is_closed == AY_TRUE)
	    {
	      np->utype = AY_CTCLOSED;
	    }
	  else
	    {
	      is_closed = AY_FALSE;
	      for(i = 0; i < np->height; i++)
		{
		  s = &(np->controlv[i*stride]);
		  e = s+((np->width-1-(np->uorder-2))*np->height*stride);

		  for(j = 0; j < np->uorder-1; j++)
		    {
		      if(!AY_V4COMP(s, e))
			{
			  is_closed = AY_FALSE;
			  break;
			}
		      s += np->height*stride;
		      e += np->height*stride;
		    }
		  if(is_closed == AY_TRUE)
		    {
		      np->utype = AY_CTPERIODIC;
		    }
		  else
		    {
		      np->utype = AY_CTOPEN;
		      break;
		    }
		} /* for */
	    } /* if */
	} /* if */
    } /* if */

  /* V */
  if(dir == 0 || dir == 2)
    {
      np->vtype = AY_CTOPEN;
      if(np->height > 2)
	{
	  is_closed = AY_TRUE;
	  for(i = 0; i < np->width; i++)
	    {
	      s = &(np->controlv[i*np->height*stride]);
	      e = s+((np->height-1)*stride);

	      if(!AY_V4COMP(s, e))
		{
		  is_closed = AY_FALSE;
		  break;
		}
	    }
	  if(is_closed == AY_TRUE)
	    {
	      np->vtype = AY_CTCLOSED;
	    }
	  else
	    {
	      is_closed = AY_FALSE;
	      for(i = 0; i < np->height; i++)
		{
		  s = &(np->controlv[i*np->height*stride]);
		  e = s+((np->height-1-(np->vorder-2))*stride);

		  for(j = 0; j < np->vorder-1; j++)
		    {
		      if(!AY_V4COMP(s, e))
			{
			  is_closed = AY_FALSE;
			  break;
			}
		      s += stride;
		      e += stride;
		    }
		  if(is_closed == AY_TRUE)
		    {
		      np->vtype = AY_CTPERIODIC;
		    }
		  else
		    {
		      np->vtype = AY_CTOPEN;
		      break;
		    }
		} /* for */
	    } /* if */
	} /* if */
    } /* if */

 return;
} /* ay_npt_setuvtypes */


/* ay_npt_clearmp:
 *  delete all mpoints from patch <np>
 */
void
ay_npt_clearmp(ay_nurbpatch_object *np)
{
 ay_mpoint *next = NULL, *mp = NULL;

  if(!np)
    return;

  mp = np->mpoints;

  while(mp)
    {
      next = mp->next;
      if(mp->points)
	free(mp->points);
      if(mp->indices)
	free(mp->indices);
      free(mp);
      mp = next;
    } /* while */

  np->mpoints = NULL;

 return;
} /* ay_npt_clearmp */


/* ay_npt_recreatemp:
 *  recreate mpoints from identical controlpoints for
 *  patch <np>
 */
void
ay_npt_recreatemp(ay_nurbpatch_object *np)
{
 ay_mpoint *mp = NULL, *new = NULL;
 double *ta, *tb, **tmpp = NULL;
 unsigned int *tmpi = NULL;
 int found = AY_FALSE, i, j, count;
 int stride = 4;

  if(!np)
    return;

  ay_npt_clearmp(np);

  if(!np->createmp)
    return;

  if(!(tmpp = calloc(np->width*np->height, sizeof(double *))))
    { goto cleanup; }

  if(!(tmpi = calloc(np->width*np->height, sizeof(unsigned int))))
    { goto cleanup; }

  ta = np->controlv;
  for(i = 0; i < np->width*np->height; i++)
    {

      /* count identical points */
      count = 0;
      tb = ta;
      for(j = i; j < np->width*np->height; j++)
	{
	  if(AY_V4COMP(ta, tb))
	    {
	      tmpp[count] = tb;
	      tmpi[count] = j;
	      count++;
	    }

	  tb += stride;
	} /* for */

      /* create new mp, if it is not already there */
      if(count > 1)
	{
	  mp = np->mpoints;
	  found = AY_FALSE;
	  while(mp && !found)
	    {
	      if(AY_V3COMP(ta, mp->points[0]))
		{
		  found = AY_TRUE;
		  break;
		}

	      mp = mp->next;
	    } /* while */

	  if(!found)
	    {
	      if(!(new = calloc(1, sizeof(ay_mpoint))))
		{ goto cleanup; }
	      if(!(new->points = malloc(count*sizeof(double *))))
		{ goto cleanup; }
	      if(!(new->indices = malloc(count*sizeof(unsigned int))))
		{ goto cleanup; }
	      new->multiplicity = count;
	      memcpy(new->points, tmpp, count*sizeof(double *));
	      memcpy(new->indices, tmpi, count*sizeof(unsigned int));

	      new->next = np->mpoints;
	      np->mpoints = new;
	      /* prevent cleanup code from doing something harmful */
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

 return;
} /* ay_npt_recreatemp */


/* ay_npt_collapseselp:
 *  collapse selected points
 */
int
ay_npt_collapseselp(ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *np = NULL;
 ay_mpoint *new = NULL, *p = NULL, *t = NULL, **last = NULL;
 ay_point *selp = NULL;
 double *first = NULL;
 int count = 0, i, found = AY_FALSE;
 char fname[] = "collapseselp";

  if(!o)
    return AY_ENULL;

  if(o->type != AY_IDNPATCH)
    {
      ay_error(AY_EWTYPE, fname, ay_npt_npname);
      return AY_ERROR;
    }

  np = (ay_nurbpatch_object *)o->refine;

  selp = o->selp;

  /* count points to collapse */
  while(selp)
    {
      count++;
      selp = selp->next;
    }

  if((!o->selp) || (count < 2))
    {
      ay_error(AY_ERROR, fname, "Select (<t>ag) atleast two points first.");
      return AY_ERROR;
    }

  if(!(new = calloc(1, sizeof(ay_mpoint))))
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
      if(selp->rational)
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
      p = np->mpoints;
      last = &(np->mpoints);
      while(p)
	{
	  found = AY_FALSE;
	  for(i = 0; i < p->multiplicity; i++)
	    {
	      if(p->points[i] == selp->point)
		found = AY_TRUE;
	    } /* for */
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
  new->next = np->mpoints;
  np->mpoints = new;

 return ay_status;
} /* ay_npt_collapseselp */


/* ay_npt_explodemp:
 *  explode selected mpoints
 */
int
ay_npt_explodemp(ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *np = NULL;
 ay_mpoint *p = NULL, *t = NULL, **last = NULL;
 ay_point *selp = NULL;
 int found = AY_FALSE, i, err = AY_TRUE;
 char fname[] = "explodemp";

  if(!o)
    return AY_ENULL;

  if(o->type != AY_IDNPATCH)
    {
      ay_error(AY_EWTYPE, fname, ay_npt_npname);
      return AY_ERROR;
    }

  np = (ay_nurbpatch_object *)o->refine;

  selp = o->selp;

  if(!selp)
    {
      ay_error(AY_ERROR, fname, "Select (<t>ag) some multiple points first.");
      return AY_ERROR;
    }

  /* we simply delete all mpoints, that contain a selected point */
  while(selp)
    {
      p = np->mpoints;
      last = &(np->mpoints);
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
      ay_error(AY_ERROR, fname, "Select (<t>ag) some multiple points first.");
      ay_status = AY_ERROR;
    }

 return ay_status;
} /* ay_npt_explodemp */


/* ay_npt_getbeveltags:
 *  get bevel parameter values from BP tag of object <o> for bevel place
 *  <place> (e.g.: 0 - bottom, 1 - top, 2 - start, 3 - end; actual semantic
 *  depends on type/capabilities of o!)
 *  returns in <has_bevel> whether a matching BP tag was present
 *  returns bevel parameters in <type>, <radius>, and <sense>
 */
int
ay_npt_getbeveltags(ay_object *o, int place,
		    int *has_bevel, int *type, double *radius, int *sense)
{
 int ay_status = AY_OK;
 ay_tag *tag = NULL;
 int where;

  *has_bevel = AY_FALSE;

  tag = o->tags;
  while(tag)
    {
      if(tag->type == ay_bp_tagtype)
	{
	  if(tag->val)
	    {
	      sscanf(tag->val, "%d,%d,%lg,%d", &where, type,
		     radius, sense);
	      if(where == place)
		{
		  *has_bevel = AY_TRUE;
		  return AY_OK;
		}
	    } /* if */
	} /* if */
      tag = tag->next;
    } /* while */

 return ay_status;
} /* ay_npt_getbeveltags */


/* ay_npt_clampu:
 *  clamp NURBS patch, it is safe to call this with half clamped patches
 *  side: 0 - clamp both ends, 1 - clamp only start, 2 - clamp only end
 */
int
ay_npt_clampu(ay_nurbpatch_object *patch, int side)
{
 int ay_status = AY_OK;
 double *newcontrolv = NULL, *newknotv = NULL;
 double u;
 int stride = 4, i, j, k, s, rs = 0, re = 0;

  if(!patch)
    return AY_ENULL;

  /* clamp start? */
  if(side == 0 || side == 1)
    {
      /* the next fragment also counts the phantom knot! */
      u = patch->uknotv[0];
      s = 1;
      for(i = 1; i < patch->uorder; i++)
	{
	  if(fabs(u - patch->uknotv[i]) < AY_EPSILON)
	    s++;
	  else
	    break;
	}

      if(s < patch->uorder)
	{
	  /* clamp start */
	  u = patch->uknotv[patch->uorder-1];

	  k = ay_nb_FindSpanMult(patch->width-1, patch->uorder-1, u,
				 patch->uknotv, &s);

	  rs = (patch->uorder - 1) - s;
	  patch->width += rs;

	  if(!(newcontrolv = malloc(patch->width*patch->height*stride*
				    sizeof(double))))
	    return AY_EOMEM;

	  if(!(newknotv = malloc((patch->width+patch->uorder)*sizeof(double))))
	    { free(newcontrolv); return AY_EOMEM; }

	  ay_status = ay_nb_InsertKnotSurfU(stride, patch->width-rs-1,
			 patch->height-1, patch->uorder-1, patch->uknotv,
			 patch->controlv, u, k, s, rs,
			 newknotv, newcontrolv);

	  if(ay_status)
	    {
	      free(newknotv);
	      free(newcontrolv);
	      return ay_status;
	    }

	  free(patch->controlv);
	  patch->controlv = newcontrolv;

	  free(patch->uknotv);
	  patch->uknotv = newknotv;
	} /* if */
    } /* if */

  /* clamp end? */
  if(side == 0 || side == 2)
    {
      /* the next fragment also counts the phantom knot! */
      s = 1;
      j = patch->width+patch->uorder-1;
      u = patch->uknotv[j];
      for(i = 1; i < patch->uorder; i++)
	{
	  if(fabs(u - patch->uknotv[j-i]) < AY_EPSILON)
	    s++;
	  else
	    break;
	}

      if(s < patch->uorder)
	{
	  /* clamp end */
	  u = patch->uknotv[patch->width];

	  k = ay_nb_FindSpanMult(patch->width-1, patch->uorder-1, u,
				 patch->uknotv, &s);

	  re = (patch->uorder - 1) - s;
	  patch->width += re;

	  if(!(newcontrolv = malloc(patch->width*patch->height*stride*
				    sizeof(double))))
	    return AY_EOMEM;

	  if(!(newknotv = malloc((patch->width+patch->uorder)*sizeof(double))))
	    { free(newcontrolv); return AY_EOMEM; }

	  ay_status = ay_nb_InsertKnotSurfU(stride, patch->width-re-1,
			 patch->height-1, patch->uorder-1, patch->uknotv,
			 patch->controlv, u, k, s, re,
			 newknotv, newcontrolv);

	  if(ay_status)
	    {
	      free(newknotv);
	      free(newcontrolv);
	      return ay_status;
	    }

	  free(patch->controlv);
	  patch->controlv = newcontrolv;

	  free(patch->uknotv);
	  patch->uknotv = newknotv;
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
      patch->width -= rs+re;
      break;
    case 1:
      patch->width -= rs;
      break;
    case 2:
      patch->width -= re;
      break;
    default:
      break;
    }

  if(!(newcontrolv = malloc(patch->width*patch->height*stride*
			    sizeof(double))))
    return AY_EOMEM;

  if(!(newknotv = malloc((patch->width+patch->uorder)*sizeof(double))))
    { free(newcontrolv); return AY_EOMEM; }

  switch(side)
    {
    case 0:
    case 1:
      /* clamped both or start: ignore first rs knots */
      memcpy(newcontrolv, &(patch->controlv[rs*patch->height*stride]),
	     patch->width*patch->height*stride*sizeof(double));

      memcpy(newknotv, &(patch->uknotv[rs]),
	     (patch->width+patch->uorder)*sizeof(double));
      /* improve phantom knot */
      newknotv[0] = newknotv[1];
      break;
    case 2:
      /* clamped end: ignore last re knots */
      memcpy(newcontrolv, patch->controlv,
	     patch->width*patch->height*stride*sizeof(double));

      memcpy(newknotv, patch->uknotv,
	     (patch->width+patch->uorder)*sizeof(double));
      /* improve phantom knot */
      newknotv[patch->width+patch->uorder-1] =
	newknotv[patch->width+patch->uorder-2];
      break;
    default:
      break;
    }

  free(patch->controlv);
  patch->controlv = newcontrolv;

  free(patch->uknotv);
  patch->uknotv = newknotv;

 return AY_OK;
} /* ay_npt_clampu */


/* ay_npt_clampv:
 *  clamp NURBS patch, it is safe to call this with half clamped patches
 *  side: 0 - clamp both ends, 1 - clamp only start, 2 - clamp only end
 */
int
ay_npt_clampv(ay_nurbpatch_object *patch, int side)
{
 int ay_status = AY_OK;
 double *newcontrolv = NULL, *newknotv = NULL;
 double v;
 int stride = 4, i, j, k, s, oh, rs = 0, re = 0;

  if(!patch)
    return AY_ENULL;

  /* clamp start? */
  if(side == 0 || side == 1)
    {
      /* the next fragment also counts the phantom knot! */
      v = patch->vknotv[0];
      s = 1;
      for(i = 1; i < patch->vorder; i++)
	{
	  if(fabs(v - patch->vknotv[i]) < AY_EPSILON)
	    s++;
	  else
	    break;
	}

      if(s < patch->vorder)
	{
	  /* clamp start */
	  v = patch->vknotv[patch->vorder-1];

	  k = ay_nb_FindSpanMult(patch->height-1, patch->vorder-1, v,
				 patch->vknotv, &s);

	  rs = (patch->vorder - 1) - s;
	  patch->height += rs;

	  if(!(newcontrolv = malloc(patch->width*patch->height*stride*
				    sizeof(double))))
	    return AY_EOMEM;

	  if(!(newknotv = malloc((patch->height+patch->vorder)*
				 sizeof(double))))
	    { free(newcontrolv); return AY_EOMEM; }

	  ay_status = ay_nb_InsertKnotSurfV(stride, patch->width-1,
			 patch->height-rs-1, patch->vorder-1, patch->vknotv,
			 patch->controlv, v, k, s, rs,
			 newknotv, newcontrolv);

	  if(ay_status)
	    {
	      free(newknotv);
	      free(newcontrolv);
	      return ay_status;
	    }

	  free(patch->controlv);
	  patch->controlv = newcontrolv;

	  free(patch->vknotv);
	  patch->vknotv = newknotv;
	} /* if */
    } /* if */

  /* clamp end? */
  if(side == 0 || side == 2)
    {
      /* the next fragment also counts the phantom knot! */
      s = 1;
      j = patch->height+patch->vorder-1;
      v = patch->vknotv[j];
      for(i = 1; i < patch->vorder; i++)
	{
	  if(fabs(v - patch->vknotv[j-i]) < AY_EPSILON)
	    s++;
	  else
	    break;
	}

      if(s < patch->vorder)
	{
	  /* clamp end */
	  v = patch->vknotv[patch->height];

	  k = ay_nb_FindSpanMult(patch->height-1, patch->vorder-1, v,
				 patch->vknotv, &s);

	  re = (patch->vorder - 1) - s;
	  patch->height += re;

	  if(!(newcontrolv = malloc(patch->width*patch->height*stride*
				    sizeof(double))))
	    return AY_EOMEM;

	  if(!(newknotv = malloc((patch->height+patch->vorder)*
				 sizeof(double))))
	    { free(newcontrolv); return AY_EOMEM; }

	  ay_status = ay_nb_InsertKnotSurfV(stride, patch->width-1,
			 patch->height-re-1, patch->vorder-1, patch->vknotv,
			 patch->controlv, v, k, s, re,
			 newknotv, newcontrolv);

	  if(ay_status)
	    {
	      free(newknotv);
	      free(newcontrolv);
	      return ay_status;
	    }

	  free(patch->controlv);
	  patch->controlv = newcontrolv;

	  free(patch->vknotv);
	  patch->vknotv = newknotv;
	} /* if */
    } /* if */

  if(rs == 0 && re == 0)
    {
      return AY_OK;
    }

  oh = patch->height;

  /* create new controlv, knotv, discarding the first rs and last re knots */
  switch(side)
    {
    case 0:
      patch->height -= rs+re;
      break;
    case 1:
      patch->height -= rs;
      break;
    case 2:
      patch->height -= re;
      break;
    default:
      break;
    }

  if(!(newcontrolv = malloc(patch->width*patch->height*stride*
			    sizeof(double))))
    return AY_EOMEM;

  if(!(newknotv = malloc((patch->height+patch->vorder)*sizeof(double))))
    { free(newcontrolv); return AY_EOMEM; }

  switch(side)
    {
    case 0:
    case 1:
      /* clamped both or start: ignore first rs knots */
      for(i = 0; i < patch->width; i++)
	{
	  memcpy(&(newcontrolv[i*patch->height*stride]),
		 &(patch->controlv[(i*oh+rs)*stride]),
	     patch->height*stride*sizeof(double));
	}

      memcpy(newknotv, &(patch->vknotv[rs]),
	     (patch->height+patch->vorder)*sizeof(double));
      /* improve phantom knot */
      newknotv[0] = newknotv[1];
      break;
    case 2:
      /* clamped end: ignore last re knots */
      for(i = 0; i < patch->width; i++)
	{
	  memcpy(&(newcontrolv[i*patch->height*stride]),
		 &(patch->controlv[i*oh*stride]),
	     patch->height*stride*sizeof(double));
	}

      memcpy(newknotv, patch->vknotv,
	     (patch->height+patch->vorder)*sizeof(double));
      /* improve phantom knot */
      newknotv[patch->height+patch->vorder-1] =
	newknotv[patch->height+patch->vorder-2];
      break;
    default:
      break;
    }

  free(patch->controlv);
  patch->controlv = newcontrolv;

  free(patch->vknotv);
  patch->vknotv = newknotv;

 return AY_OK;
} /* ay_npt_clampv */


/** ay_npt_clampuvtcmd:
 *  Clamp selected NURBS patches in U/V direction.
 *  Implements the \a clampuNP and \a clampvNP scripting interface commands.
 *  See also the corresponding section in the \ayd{scclampunp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_clampuvtcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *np = NULL;
 ay_object *o = NULL;
 int clampv = AY_FALSE, side = 0;
 int notify_parent = AY_FALSE;

  if(argc >= 2)
    {
     if((argv[1][0] == '-') && (argv[1][1] == 's'))
       side = 1;
     else
     if((argv[1][0] == '-') && (argv[1][1] == 'e'))
       side = 2;
     else
       tcl_status = Tcl_GetInt(interp, argv[1], &side);
      AY_CHTCLERRRET(tcl_status, argv[0], interp);
    }

  if(!strcmp(argv[0], "clampvNP"))
    clampv = AY_TRUE;

  while(sel)
    {
      o = sel->object;
      /* so that we may use continue */
      sel = sel->next;

      if(o->type == AY_IDNPATCH)
	{

	  np = (ay_nurbpatch_object *)o->refine;

	  if(clampv)
	    {
	      /* check whether we need to clamp at all */
	      if((np->vknot_type == AY_KTNURB) ||
		 (np->vknot_type == AY_KTBEZIER))
		{
		  continue;
		}
	      ay_status = ay_npt_clampv(np, side);
	    }
	  else
	    {
	      /* check whether we need to clamp at all */
	      if((np->uknot_type == AY_KTNURB) ||
		 (np->uknot_type == AY_KTBEZIER))
		{
		  continue;
		}
	      ay_status = ay_npt_clampu(np, side);
	    }

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "Error clamping object.");
	    }
	  else
	    {
	      if(clampv)
		{
		  np->vknot_type = ay_knots_classify(np->vorder, np->vknotv,
						     np->vorder+np->height,
						     AY_EPSILON);

		}
	      else
		{
		  np->uknot_type = ay_knots_classify(np->uorder, np->uknotv,
						     np->uorder+np->width,
						     AY_EPSILON);
		}

	      /* remove all selected points */
	      if(o->selp)
		{
		  ay_selp_clear(o);
		}

	      ay_npt_recreatemp(np);

	      o->modified = AY_TRUE;

	      /* re-create tesselation of patch */
	      (void)ay_notify_object(o);
	      notify_parent = AY_TRUE;
	    } /* if */
	}
      else
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	} /* if */
    } /* while */

  if(notify_parent)
    (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_clampuvtcmd */


/* ay_npt_rescaletrim:
 *  rescale a single trim curve (<trim>) from old knot domain
 *  (<omin>,<omax>) to new domain (<nmin>,<nmax>) along dimension
 *  x/u (<mode> 0) or y/v (<mode> 1)
 */
int
ay_npt_rescaletrim(ay_object *trim,
		   int mode, double omin, double omax,
		   double nmin, double nmax)
{
 int ay_status = AY_OK;
 double s, mu, m[16], mt[16];

  if(!trim)
    return AY_ENULL;

  s = (nmax-nmin)/(omax-omin);

  /* check whether the new domain is just offset */
  if(fabs(s-1.0) < AY_EPSILON)
    {
      /* new domain is offset */
      if(mode == 0)
	{
	  trim->movx += (nmin-omin);
	}
      else
	{
	  trim->movy += (nmin-omin);
	}
    }
  else
    {
      /* new domain is scaled plus, possibly, offset */
      ay_trafo_identitymatrix(m);

      if(mode == 0)
	{
	  mu = nmin+((nmax-nmin)*0.5);
	  ay_trafo_translatematrix(mu, 0.0, 0.0, m);
	  ay_trafo_scalematrix((nmax-nmin)/(omax-omin), 1.0, 1.0, m);
	  mu = omin+((omax-omin)*0.5);
	  ay_trafo_translatematrix(-mu, 0.0, 0.0, m);
	}
      else
	{
	  mu = nmin+((nmax-nmin)*0.5);
	  ay_trafo_translatematrix(0.0, mu, 0.0, m);
	  ay_trafo_scalematrix(1.0, (nmax-nmin)/(omax-omin), 1.0, m);
	  mu = omin+((omax-omin)*0.5);
	  ay_trafo_translatematrix(0.0, -mu, 0.0, m);
	}

      ay_trafo_creatematrix(trim, mt);

      ay_trafo_multmatrix(m, mt);

      ay_trafo_decomposematrix(m, trim);
    }

 return ay_status;
} /* ay_npt_rescaletrim */


/* ay_npt_rescaletrims:
 *  rescale the trim curves in <trim> from old knot domain
 *  (<omin>,<omax>) to new domain (<nmin>,<nmax>) along dimension
 *  x/u (<mode> 0) or y/v (<mode> 1)
 *  Note that <trim> may actually be a list _and_ contain trim
 *  loop level objects (in the latter case, ay_npt_rescaletrims()
 *  will invoke itself again _recursively_).
 */
int
ay_npt_rescaletrims(ay_object *trim,
		    int mode, double omin, double omax,
		    double nmin, double nmax)
{
 int ay_status = AY_OK;

  if(!trim)
    return AY_ENULL;

  while(trim)
    {
      if(trim->type == AY_IDLEVEL)
	{
	  if(trim->down && trim->down->next)
	    {
	      ay_status = ay_npt_rescaletrims(trim->down, mode, omin, omax,
					      nmin, nmax);
	    }
	}
      else
	{
	  ay_status = ay_npt_rescaletrim(trim, mode, omin, omax,
					 nmin, nmax);
	}

      trim = trim->next;
    } /* while */

 return ay_status;
} /* ay_npt_rescaletrims */


/** ay_npt_rescaleknvnptcmd:
 *  rescale the knot vectors of the selected NURBS patches
 *  - to the range 0.0 - 1.0 (no arguments)
 *  - to a specific range ([-r|-ru|-rv] min max)
 *  - so that all knots have a minimum guaranteed distance
 *    ([-d|-du|-dv] mindist)
 *  where -r/-d operate on both dimensions, -ru/-du only on u
 *  and -rv/-dv only on the v dimension
 *
 *  Implements the \a rescaleknNP scripting interface command.
 *  See also the corresponding section in the \ayd{screscaleknnp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_rescaleknvnptcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;
 ay_nurbpatch_object *patch = NULL;
 double rmin = 0.0, rmax = 1.0, mindist = 1.0e-04;
 double oldmin, oldmax;
 int i = 1, mode = 0, dim = 0;
 int notify_parent = AY_FALSE;

  /* parse args */
  if(argc > 2)
    {
      while(i+1 < argc)
	{
	  if(!strncmp(argv[i], "-r", 2))
	    {
	      if(argc > i+2)
		{
		  mode = 0;
		  dim = 0;
		  if(!strcmp(argv[i], "-ru"))
		    {
		      dim = 1;
		    }
		  if(!strcmp(argv[i], "-rv"))
		    {
		      dim = 2;
		    }
		  sscanf(argv[i+1], "%lg", &rmin);
		  sscanf(argv[i+2], "%lg", &rmax);
		}
	      else
		{
		  ay_error(AY_EARGS, argv[0], "-r|-ru|-rv min max");
		}
	    } /* if */
	  if(!strncmp(argv[i], "-d", 2))
	    {
	      mode = 1;
	      dim = 0;
	      if(!strcmp(argv[i], "-du"))
		{
		  dim = 1;
		}
	      if(!strcmp(argv[i], "-dv"))
		{
		  dim = 2;
		}
	      sscanf(argv[i+1], "%lg", &mindist);
	    } /* if */
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
      if(src->type != AY_IDNPATCH)
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	}
      else
	{
	  patch = (ay_nurbpatch_object*)src->refine;

	  if(dim == 0 || dim == 1)
	    {
	      /* process u dimension */
	      if(mode)
		{
		  /* save old knot range */
		  oldmin = patch->uknotv[0];
		  oldmax = patch->uknotv[patch->width+patch->uorder-1];

		  /* rescale knots */
		  ay_status = ay_knots_rescaletomindist(patch->width +
							patch->uorder,
							patch->uknotv,
							mindist);
		  /* scale trim curves */
		  if(src->down && src->down->next)
		    {
		      (void)ay_npt_rescaletrims(src->down,
						0,
						oldmin,
						oldmax,
						patch->uknotv[0],
				patch->uknotv[patch->width+patch->uorder-1]);
		    }
		}
	      else
		{
		  /* first scale trim curves */
		  if(src->down && src->down->next)
		    {
		      (void)ay_npt_rescaletrims(src->down,
						0,
						patch->uknotv[0],
				 patch->uknotv[patch->width+patch->uorder-1],
						rmin, rmax);
		    }
		  /* now scale the knots */
		  ay_status = ay_knots_rescaletorange(patch->width +
						      patch->uorder,
						      patch->uknotv,
						      rmin, rmax);
		} /* if */
	      if(ay_status)
		{
		  ay_error(ay_status, argv[0], "Could not rescale u-knots.");
		  break;
		}
	      src->modified = AY_TRUE;
	    } /* if */

	  if(dim == 0 || dim == 2)
	    {
	      /* process v dimension */
	      if(mode)
		{
		  /* save old knot range */
		  oldmin = patch->vknotv[0];
		  oldmax = patch->vknotv[patch->height+patch->vorder-1];

		  /* rescale knots */
		  ay_status = ay_knots_rescaletomindist(patch->height +
							patch->vorder,
							patch->vknotv,
							mindist);

		  /* scale trim curves */
		  if(src->down && src->down->next)
		    {
		      (void)ay_npt_rescaletrims(src->down,
						1,
						oldmin,
						oldmax,
						patch->vknotv[0],
			       patch->vknotv[patch->height+patch->vorder-1]);
		    }
		}
	      else
		{
		  /* first scale trim curves */
		  if(src->down && src->down->next)
		    {
		      (void)ay_npt_rescaletrims(src->down,
						1,
						patch->vknotv[0],
				patch->vknotv[patch->height+patch->vorder-1],
						rmin, rmax);
		    }
		  /* now scale the knots */
		  ay_status = ay_knots_rescaletorange(patch->height +
						      patch->vorder,
						      patch->vknotv,
						      rmin, rmax);
		} /* if */
	      if(ay_status)
		{
		  ay_error(ay_status, argv[0], "Could not rescale v-knots.");
		  break;
		}
	      src->modified = AY_TRUE;
	    } /* if */
	  (void)ay_notify_object(src);
	  notify_parent = AY_TRUE;
	} /* if */
      sel = sel->next;
    } /* while */

  if(notify_parent)
    (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_rescaleknvnptcmd */


/** ay_npt_insertknutcmd:
 *  Insert knots into selected NURBS patches (U direction).
 *  Implements the \a insknuNP scripting interface command.
 *  See also the corresponding section in the \ayd{scinsknunp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_insertknutcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;
 ay_nurbpatch_object *patch = NULL;
 double u, *knots = NULL, *newcontrolv = NULL, *newknotv = NULL;
 int stride = 4, k = 0, s = 0, r = 0;
 int notify_parent = AY_FALSE;

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

  tcl_status = Tcl_GetDouble(interp, argv[1], &u);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  tcl_status = Tcl_GetInt(interp, argv[2], &r);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  if(r <= 0)
    {
      ay_error(AY_ERROR, argv[0], "Parameter r must be > 0.");
      return TCL_OK;
    }

  while(sel)
    {
      src = sel->object;
      if(src->type != AY_IDNPATCH)
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	}
      else
	{
	  patch = (ay_nurbpatch_object*)src->refine;
	  knots = patch->uknotv;

	  if((u < knots[patch->uorder-1]) || (u > knots[patch->width]))
	    {
	      (void)ay_error_reportdrange(argv[0], "\"u\"",
					  knots[patch->uorder-1],
					  knots[patch->width]);
	      return TCL_OK;
	    }

	  k = ay_nb_FindSpanMult(patch->width-1, patch->uorder-1, u,
				 knots, &s);

	  if(patch->uorder < r+s)
	    {
	      ay_error(AY_ERROR, argv[0],
			 "Knot insertion leads to illegal knot sequence.");
	      return TCL_OK;
	    }

	  /* remove all selected points */
	  if(src->selp)
	    {
	      ay_selp_clear(src);
	    }

	  patch->width += r;

	  if(!(newcontrolv = malloc(patch->width*patch->height*stride*
				    sizeof(double))))
	    {
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }
	  if(!(newknotv = malloc((patch->width+patch->uorder)*sizeof(double))))
	    {
	      free(newcontrolv);
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }
	  ay_status = ay_nb_InsertKnotSurfU(stride,
					    patch->width-r-1, patch->height-1,
		        patch->uorder-1, patch->uknotv, patch->controlv, u, k,
		        s, r, newknotv, newcontrolv);

	  if(ay_status)
	    {
	      free(newknotv);
	      free(newcontrolv);
	      ay_error(AY_ERROR, argv[0], "Knot insertion failed.");
	      return TCL_OK;
	    }

	  free(patch->controlv);
	  patch->controlv = newcontrolv;

	  free(patch->uknotv);
	  patch->uknotv = newknotv;
	  patch->uknot_type = AY_KTCUSTOM;

	  ay_npt_recreatemp(patch);

	  src->modified = AY_TRUE;

	  /* re-create tesselation of patch */
	  (void)ay_notify_object(src);
	  notify_parent = AY_TRUE;
	} /* if */

      sel = sel->next;
    } /* while */

  if(notify_parent)
    (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_insertknutcmd */


/** ay_npt_insertknvtcmd:
 *  Insert knots into selected NURBS patches (V direction).
 *  Implements the \a insknvNP scripting interface command.
 *  See also the corresponding section in the \ayd{scinsknvnp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_insertknvtcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;
 ay_nurbpatch_object *patch = NULL;
 double v, *knots = NULL, *newcontrolv = NULL, *newknotv = NULL;
 int stride = 4, k = 0, s = 0, r = 0;
 int notify_parent = AY_FALSE;

  if(argc < 3)
    {
      ay_error(AY_EARGS, argv[0], "v r");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  tcl_status = Tcl_GetDouble(interp, argv[1], &v);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  tcl_status = Tcl_GetInt(interp, argv[2], &r);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  if(r <= 0)
    {
      ay_error(AY_ERROR, argv[0], "Parameter r must be > 0.");
      return TCL_OK;
    }

  while(sel)
    {
      src = sel->object;
      if(src->type != AY_IDNPATCH)
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	}
      else
	{
	  patch = (ay_nurbpatch_object*)src->refine;
	  knots = patch->vknotv;

	  if((v < knots[patch->vorder-1]) || (v > knots[patch->height]))
	    {
	      (void)ay_error_reportdrange(argv[0], "\"v\"",
					  knots[patch->vorder-1],
					  knots[patch->height]);
	      return TCL_OK;
	    }

	  k = ay_nb_FindSpanMult(patch->height-1, patch->vorder-1, v,
				 knots, &s);

	  if(patch->vorder < r+s)
	    {
	      ay_error(AY_ERROR, argv[0],
			 "Knot insertion leads to illegal knot sequence.");
	      return TCL_OK;
	    }

	  /* remove all selected points */
	  if(sel->object->selp)
	    {
	      ay_selp_clear(sel->object);
	    }

	  patch->height += r;

	  if(!(newcontrolv = malloc(patch->width*patch->height*stride*
				    sizeof(double))))
	    {
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }
	  if(!(newknotv = malloc((patch->height+patch->vorder)*
				 sizeof(double))))
	    {
	      free(newcontrolv);
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }
	  ay_status = ay_nb_InsertKnotSurfV(stride,
					    patch->width-1, patch->height-r-1,
		        patch->vorder-1, patch->vknotv, patch->controlv, v, k,
		        s, r, newknotv, newcontrolv);

	  if(ay_status)
	    {
	      free(newknotv);
	      free(newcontrolv);
	      ay_error(AY_ERROR, argv[0], "Knot insertion failed.");
	      return TCL_OK;
	    }

	  free(patch->controlv);
	  patch->controlv = newcontrolv;

	  free(patch->vknotv);
	  patch->vknotv = newknotv;
	  patch->vknot_type = AY_KTCUSTOM;

	  ay_npt_recreatemp(patch);

	  src->modified = AY_TRUE;

	  /* re-create tesselation of patch */
	  (void)ay_notify_object(sel->object);
	  notify_parent = AY_TRUE;
	} /* if */

      sel = sel->next;
    } /* while */

  if(notify_parent)
    (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_insertknvtcmd */


/* ay_npt_splitu:
 *  split NURBPatch object <src> at parametric value <u> into two;
 *  modifies <src>, returns second patch in <result>.
 */
int
ay_npt_splitu(ay_object *src, double u, ay_object **result)
{
 int ay_status = AY_OK;
 ay_object *new = NULL;
 ay_nurbpatch_object *patch = NULL;
 ay_nurbpatch_object *np1 = NULL, *np2 = NULL;
 double *knots = NULL, *newcontrolv = NULL, *newknotv = NULL;
 int stride = 4, k = 0, r = 0, s = 0, np1len = 0;
 char fname[] = "npt_splitu";

  if(!src || !result)
    return AY_ENULL;

  if(src->type != AY_IDNPATCH)
    {
      ay_error(AY_EWTYPE, fname, ay_npt_npname);
      return AY_ERROR;
    }
  else
    {
      patch = (ay_nurbpatch_object*)src->refine;
      stride = 4;
      knots = patch->uknotv;

      if((u <= knots[patch->uorder-1]) || (u >= knots[patch->width]))
	return ay_error_reportdrange(fname, "\"u\"",
			      knots[patch->uorder-1], knots[patch->width]);

      k = ay_nb_FindSpanMult(patch->width-1, patch->uorder-1, u,
			     knots, &s);

      r = patch->uorder-1-s;

      patch->width += r;

      if(r != 0)
	{
	  if(!(newcontrolv = malloc(patch->width*patch->height*stride*
				    sizeof(double))))
	    { return AY_EOMEM; }

	  if(!(newknotv = malloc((patch->width+patch->uorder)*sizeof(double))))
	    { free(newcontrolv); return AY_EOMEM; }

	  ay_status = ay_nb_InsertKnotSurfU(stride,
			patch->width-r-1, patch->height-1,
		        patch->uorder-1, patch->uknotv, patch->controlv, u, k,
		        s, r, newknotv, newcontrolv);

	  if(ay_status)
	    {
	      free(newknotv);
	      free(newcontrolv);
	      return ay_status;
	    }

	  free(patch->controlv);
	  patch->controlv = newcontrolv;

	  free(patch->uknotv);
	  patch->uknotv = newknotv;
	} /* if */

      patch->uknot_type = AY_KTCUSTOM;

      /* create two new patches */
      np1 = patch;
      np1->utype = AY_CTOPEN;
      ay_status = ay_object_copy(src, &new);
      if(ay_status || !new)
	{
	  return ay_status;
	}

      if(r != 0)
	np1len = k - (np1->uorder-1) + 1 + (patch->uorder-1-s+r-1)/2 + 1;
      else
	np1len = k - (np1->uorder-1) + 1;

      np2 = (ay_nurbpatch_object*)new->refine;
      np2->width = (np1->width+1) - np1len;

      np1->width = np1len;

      if(!(newcontrolv = malloc(np1->width*np1->height*stride*
				sizeof(double))))
	{ (void)ay_object_delete(new); return AY_EOMEM; }

      if(!(newknotv = malloc((np1->width+np1->uorder)*sizeof(double))))
	{ (void)ay_object_delete(new); free(newcontrolv); return AY_EOMEM; }

      memcpy(newcontrolv, np1->controlv,
	     np1->width*np1->height*stride*sizeof(double));

      memcpy(newknotv, np1->uknotv, (np1->width+np1->uorder)*sizeof(double));

      /* improve phantom knot */
      newknotv[patch->width+patch->uorder-1] =
	newknotv[patch->width+patch->uorder-2];

      /* check new knots for validity */
      if(ay_knots_check(np1->width, np1->uorder,
			np1->width+np1->uorder, newknotv))
	{
	  (void)ay_object_delete(new);
	  free(newcontrolv); free(newknotv);
	  return AY_ERROR;
	}

      free(np2->controlv);
      np2->controlv = NULL;
      free(np2->uknotv);
      np2->uknotv = NULL;

      if(!(np2->controlv = malloc(np2->width*np2->height*stride*
				  sizeof(double))))
	{ (void)ay_object_delete(new); free(newcontrolv); free(newknotv);
	  return AY_EOMEM; }

      if(!(np2->uknotv = malloc((np2->width+np2->uorder)*sizeof(double))))
	{ (void)ay_object_delete(new); free(newcontrolv); free(newknotv);
	  return AY_EOMEM; }

      memcpy(np2->controlv,
	     &(np1->controlv[((np1->width-1)*np2->height)*stride]),
	     np2->width*np2->height*stride*sizeof(double));

      memcpy(np2->uknotv, &(np1->uknotv[np1->width-1]),
	     (np2->width+np2->uorder)*sizeof(double));

      /* improve phantom knot */
      np2->uknotv[0] = np2->uknotv[1];

      /* check new knots for validity */
      if(ay_knots_check(np2->width, np2->uorder,
			np2->width+np2->uorder,	np2->uknotv))
	{ (void)ay_object_delete(new); free(newcontrolv); free(newknotv);
	  return AY_ERROR; }

      free(np1->controlv);
      np1->controlv = newcontrolv;
      free(np1->uknotv);
      np1->uknotv = newknotv;

      ay_npt_recreatemp(np1);
      ay_npt_recreatemp(np2);

      np2->is_rat = np1->is_rat;

      new->selected = AY_FALSE;
      new->modified = AY_TRUE;
      src->modified = AY_TRUE;

      /* return result */
      *result = new;
    } /* if npatch */

 return AY_OK;
} /* ay_npt_splitu */


/* ay_npt_splitv:
 *  split NURBPatch object <src> at parametric value <v> into two;
 *  modifies <src>, returns second patch in <result>.
 */
int
ay_npt_splitv(ay_object *src, double v, ay_object **result)
{
 int ay_status = AY_OK;
 ay_object *new = NULL;
 ay_nurbpatch_object *patch = NULL;
 ay_nurbpatch_object *np1 = NULL, *np2 = NULL;
 double *knots = NULL, *newcontrolv = NULL, *newknotv = NULL;
 int stride = 4, k = 0, r = 0, s = 0, np1len = 0;
 int i, a, b, oldnp1height;
 char fname[] = "npt_splitv";

  if(!src || !result)
    return AY_ENULL;

  if(src->type != AY_IDNPATCH)
    {
      ay_error(AY_EWTYPE, fname, ay_npt_npname);
      return AY_ERROR;
    }
  else
    {
      patch = (ay_nurbpatch_object*)src->refine;
      stride = 4;
      knots = patch->vknotv;

      if((v <= knots[patch->vorder-1]) || (v >= knots[patch->height]))
	return ay_error_reportdrange(fname, "\"v\"",
			     knots[patch->vorder-1], knots[patch->height]);

      k = ay_nb_FindSpanMult(patch->height-1, patch->vorder-1, v,
			     knots, &s);

      r = patch->vorder-1-s;

      patch->height += r;

      if(r != 0)
	{
	  if(!(newcontrolv = malloc(patch->width*patch->height*stride*
				    sizeof(double))))
	    { return AY_EOMEM; }

	  if(!(newknotv = malloc((patch->height+patch->vorder)*
				 sizeof(double))))
	    { free(newcontrolv); return AY_EOMEM; }

	  ay_status = ay_nb_InsertKnotSurfV(stride,
			patch->width-1, patch->height-r-1,
		        patch->vorder-1, patch->vknotv, patch->controlv, v, k,
		        s, r, newknotv, newcontrolv);

	  if(ay_status)
	    {
	      free(newknotv);
	      free(newcontrolv);
	      return ay_status;
	    }

	  free(patch->controlv);
	  patch->controlv = newcontrolv;

	  free(patch->vknotv);
	  patch->vknotv = newknotv;
	} /* if */

      patch->vknot_type = AY_KTCUSTOM;

      /* create two new patches */
      np1 = patch;
      np1->vtype = AY_CTOPEN;
      ay_status = ay_object_copy(src, &new);
      if(ay_status || !new)
	{
	  return ay_status;
	}

      if(r != 0)
	np1len = k - (np1->vorder-1) + 1 + (patch->vorder-1-s+r-1)/2 + 1;
      else
	np1len = k - (np1->vorder-1) + 1;

      np2 = (ay_nurbpatch_object*)new->refine;
      np2->height = (np1->height+1) - np1len;
      oldnp1height = np1->height;
      np1->height = np1len;

      if(!(newcontrolv = malloc(np1->width*np1->height*stride*
				sizeof(double))))
	{ (void)ay_object_delete(new); return AY_EOMEM; }

      if(!(newknotv = malloc((np1->height+np1->vorder)*sizeof(double))))
	{ (void)ay_object_delete(new); free(newcontrolv); return AY_EOMEM; }

      a = 0;
      b = 0;
      for(i = 0; i < np1->width; i++)
	{
	  memcpy(&(newcontrolv[a]), &(np1->controlv[b]),
		 np1->height*stride*sizeof(double));
	  a += (np1->height*stride);
	  b += (oldnp1height*stride);
	}

      memcpy(newknotv, np1->vknotv, (np1->height+np1->vorder)*sizeof(double));

      /* improve phantom knot */
      newknotv[patch->height+patch->vorder-1] =
	newknotv[patch->height+patch->vorder-2];

      /* check new knots for validity */
      if(ay_knots_check(np1->height, np1->vorder,
			np1->height+np1->vorder, newknotv))
	{ (void)ay_object_delete(new); free(newcontrolv); free(newknotv);
	  return AY_ERROR; }

      free(np2->controlv);
      np2->controlv = NULL;
      free(np2->vknotv);
      np2->vknotv = NULL;

      if(!(np2->controlv = malloc(np2->width*np2->height*stride*
				  sizeof(double))))
	{ (void)ay_object_delete(new); free(newcontrolv); free(newknotv);
	  return AY_EOMEM; }

      if(!(np2->vknotv = malloc((np2->height+np2->vorder)*sizeof(double))))
	{ (void)ay_object_delete(new); free(newcontrolv); free(newknotv);
	  return AY_EOMEM; }

      a = 0;
      b = (np1->height-1)*stride;
      for(i = 0; i < np2->width; i++)
	{
	  memcpy(&(np2->controlv[a]), &(np1->controlv[b]),
		 np2->height*stride*sizeof(double));
	  a += (np2->height*stride);
	  b += (oldnp1height*stride);
	}

      memcpy(np2->vknotv, &(np1->vknotv[np1->height-1]),
	     (np2->height+np2->vorder)*sizeof(double));

      /* improve phantom knot */
      np2->vknotv[0] = np2->vknotv[1];

      /* check new knots for validity */
      if(ay_knots_check(np2->height, np2->vorder,
			np2->height+np2->vorder, np2->vknotv))
	{ (void)ay_object_delete(new); free(newcontrolv); free(newknotv);
	  return AY_ERROR; }

      free(np1->controlv);
      np1->controlv = newcontrolv;
      free(np1->vknotv);
      np1->vknotv = newknotv;

      ay_npt_recreatemp(np1);
      ay_npt_recreatemp(np2);

      np2->is_rat = np1->is_rat;

      new->selected = AY_FALSE;
      new->modified = AY_TRUE;
      src->modified = AY_TRUE;

      /* return result */
      *result = new;
    } /* if npatch */

 return AY_OK;
} /* ay_npt_splitv */


/** ay_npt_splituvtcmd:
 *  Split selected NURBS patches in U/V direction.
 *  Implements the \a splituNP scripting interface command.
 *  Also implements the \a splitvNP scripting interface command.
 *  See also the corresponding section in the \ayd{scsplitunp}.
 *  See also the corresponding section in the \ayd{scsplitvnp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_splituvtcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *new = NULL;
 double t = 0.0;
 int splitv = AY_FALSE;
 int notify_parent = AY_FALSE;

  /* distinguish between
     splituNP and
     splitvNP
     01234^    */
  if(argv[0][5] == 'v')
    splitv = AY_TRUE;

  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "t");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  tcl_status = Tcl_GetDouble(interp, argv[1], &t);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  while(sel)
    {
      if(sel->object->type == AY_IDNPATCH)
	{
	  /* remove all selected points */
	  if(sel->object->selp)
	    {
	      ay_selp_clear(sel->object);
	    }

	  new = NULL;
	  if(splitv)
	    ay_status = ay_npt_splitv(sel->object, t, &new);
	  else
	    ay_status = ay_npt_splitu(sel->object, t, &new);

	  if(ay_status)
	    {
	      ay_error(ay_status, argv[0], NULL);
	      return TCL_OK;
	    } /* if */

	  ay_object_link(new);

	  sel->object->modified = AY_TRUE;

	  /* re-create tesselation of original patch */
	  (void)ay_notify_object(sel->object);
	  notify_parent = AY_TRUE;
	}
      else
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	} /* if */

      sel = sel->next;
    } /* while */

  if(notify_parent)
    (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_splituvtcmd */


/* ay_npt_extractnp:
 *  extract subpatch from patch <src>, returns patch in <result>;
 */
int
ay_npt_extractnp(ay_object *src, double umin, double umax,
		 double vmin, double vmax, int relative,
		 ay_object **result)
{
 int ay_status = AY_OK;
 ay_object *copy = NULL, *np1 = NULL, *np2 = NULL;
 ay_nurbpatch_object *patch = NULL;
 char fname[] = "npt_extractnp", split_errmsg[] = "Split failed.";
 double uv, uvmin, uvmax;

  if(!src || !result)
    return AY_ENULL;

  /* check parameters */
  if(((umin + AY_EPSILON) > umax) || ((vmin + AY_EPSILON) > vmax))
    {
      ay_error(AY_ERROR, fname, "Parameter min must be smaller than max.");
      return AY_ERROR;
    }

  if(src->type != AY_IDNPATCH)
    {
      ay_error(AY_EWTYPE, fname, ay_npt_npname);
      return AY_ERROR;
    }
  else
    {
      ay_status = ay_object_copy(src, &copy);
      if(ay_status || !copy)
	{
	  return AY_ERROR;
	}
      patch = (ay_nurbpatch_object*)copy->refine;

      if(relative)
	{
	  uvmin = patch->uknotv[patch->uorder-1];
	  uvmax = patch->uknotv[patch->width];
	  uv = uvmin + ((uvmax - uvmin) * umin);
	  umin = uv;
	  uv = uvmin + ((uvmax - uvmin) * umax);
	  umax = uv;

	  uvmin = patch->vknotv[patch->vorder-1];
	  uvmax = patch->vknotv[patch->height];
	  uv = uvmin + ((uvmax - uvmin) * vmin);
	  vmin = uv;
	  uv = uvmin + ((uvmax - uvmin) * vmax);
	  vmax = uv;
	}

      /* check parameters */
      if((umin < patch->uknotv[patch->uorder-1]) ||
	 (umax > patch->uknotv[patch->width]))
	{
	  (void)ay_error_reportdrange(fname, "\"umin/umax\"",
				      patch->uknotv[patch->uorder-1],
				      patch->uknotv[patch->width]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

      if((vmin < patch->vknotv[patch->vorder-1]) ||
	 (vmax > patch->vknotv[patch->height]))
	{
	  (void)ay_error_reportdrange(fname, "\"vmin/vmax\"",
				      patch->vknotv[patch->vorder-1],
				      patch->vknotv[patch->height]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

      /* split off areas outside umin/umax/vmin/vmax */
      /* note that this approach supports e.g. umin to be exactly uknotv[0]
	 and in this case does not execute the (unneeded) split */
      if(umin > patch->uknotv[0/*patch->uorder...?*/])
	{
	  ay_status = ay_npt_splitu(copy, umin, &np1);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, split_errmsg);
	      goto cleanup;
	    }
	  /* <np1> is the sub surface we want
	     => remove <copy>; then move <np1> to <copy> */
	  if(np1)
	    {
	      np2 = copy;
	      copy = np1;
	      (void)ay_object_delete(np2);
	      np1 = NULL;
	      patch = (ay_nurbpatch_object*)copy->refine;
	    }
	}
      if(umax < patch->uknotv[patch->width])
	{
	  ay_status = ay_npt_splitu(copy, umax, &np1);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, split_errmsg);
	      goto cleanup;
	    }
	  /* <copy> is the sub surface we want
	     => remove <np1> */
	  if(np1)
	    {
	      (void)ay_object_delete(np1);
	      np1 = NULL;
	    }
	}
      if(vmin > patch->vknotv[0/*patch->vorder...?*/])
	{
	  ay_status = ay_npt_splitv(copy, vmin, &np1);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, split_errmsg);
	      goto cleanup;
	    }
	  /* <np1> is the sub surface we want
	     => remove <copy>; then move <np1> to <copy> */
	  if(np1)
	    {
	      np2 = copy;
	      copy = np1;
	      (void)ay_object_delete(np2);
	      np1 = NULL;
	      patch = (ay_nurbpatch_object*)copy->refine;
	    }
	}
      if(vmax < patch->vknotv[patch->height])
	{
	  ay_status = ay_npt_splitv(copy, vmax, &np1);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, split_errmsg);
	      goto cleanup;
	    }
	  /* <copy> is the sub surface we want
	     => remove <np1> */
	  if(np1)
	    {
	      (void)ay_object_delete(np1);
	      np1 = NULL;
	    }
	}

      /* return result */
      *result = copy;
      copy = NULL;
    } /* if */

cleanup:

  if(copy)
    {
      ay_object_delete(copy);
    }

 return ay_status;
} /* ay_npt_extractnp */


/** ay_npt_extractnptcmd:
 *  Extract a sub surface from the selected NURBS patches.
 *  Implements the \a extrNP scripting interface command.
 *  See also the corresponding section in the \ayd{scextrnp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_extractnptcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o, *new = NULL, *pobject = NULL;
 double umin = 0.0, umax = 0.0, vmin = 0.0, vmax = 0.0;
 int relative = AY_FALSE;

  if(argc < 5)
    {
      ay_error(AY_EARGS, argv[0], "umin umax vmin vmax [relative]");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  tcl_status = Tcl_GetDouble(interp, argv[1], &umin);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  tcl_status = Tcl_GetDouble(interp, argv[2], &umax);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  tcl_status = Tcl_GetDouble(interp, argv[3], &vmin);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  tcl_status = Tcl_GetDouble(interp, argv[4], &vmax);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);

  if(argc > 5)
    {
      tcl_status = Tcl_GetInt(interp, argv[5], &relative);
      AY_CHTCLERRRET(tcl_status, argv[0], interp);
    }

  while(sel)
    {
      o = sel->object;
      if(o->type != AY_IDNPATCH)
	{
	  ay_status = ay_provide_object(o, AY_IDNPATCH, &pobject);

	  if(!pobject)
	    {
	      ay_error(AY_EWARN, argv[0], ay_error_igntype);
	      o = NULL;
	    }
	  else
	    {
	      o = pobject;
	    } /* if */
	} /* if */

      if(o)
	{
	  new = NULL;

	  ay_status = ay_npt_extractnp(o, umin, umax, vmin, vmax,
				       relative, &new);

	  if(ay_status)
	    {
	      ay_error(ay_status, argv[0], NULL);
	      return TCL_OK;
	    } /* if */

	  ay_object_link(new);
	}

      if(pobject)
	{
	  ay_object_deletemulti(pobject, AY_FALSE);
	}

      sel = sel->next;
    } /* while */

  (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_extractnptcmd */


/* ay_npt_gndu:
 * get next different control point in dimension u and direction <dir>
 * in a open NURBS surface <np> where the current points location in
 * its row is <i> and its memory address is <p>;
 * <dir> must be one of:
 * 1 - East
 * 3 - West
 * returns result (new address) in <dp>
 * <dp> is set to NULL if there is no different point in the designated
 * direction (e.g. if the patch is degenerated in this row)
 */
void
ay_npt_gndu(char dir, ay_nurbpatch_object *np, int i, double *p,
	    double **dp)
{
 int offset, stride = 4;
 double *p2 = NULL;

  if(dir == AY_EAST)
    {
      /* calculate offset */
      offset = stride*np->height;
      if(i == np->width-1)
	{
	  *dp = NULL;
	  return;
	}
      else
	{
	  p2 = p + offset;
	}
      /* apply offset to p2 until p2 points to
	 a different control point than p
	 (in terms of their coordinate values)
	 or we run off the edge of the patch */
      while(AY_V3COMP(p, p2))
	{
	  p2 += offset;
	  i++;
	  if(i == np->width-1)
	    {
	      *dp = NULL;
	      return;
	    }
	}
    }
  else
    {
      /* dir == AY_WEST */

      /* calculate offset */
      offset = -stride*np->height;
      if(i == 0)
	{
	  *dp = NULL;
	  return;
	}
      else
	{
	  p2 = p + offset;
	}
      /* apply offset to p2 until p2 points to
	 a different control point than p
	 (in terms of their coordinate values)
	 or we run off the edge of the patch */
      while(AY_V3COMP(p, p2))
	{
	  p2 += offset;
	  i--;
	  if(i == 0)
	    {
	      *dp = NULL;
	      return;
	    }
	}
    } /* if */

  *dp = p2;

 return;
} /* ay_npt_gndu */


/* ay_npt_gndv:
 * get next different control point in dimension v and direction <dir>
 * in a open NURBS surface <np> where the current points location in
 * its column is <j> and its memory address is <p>;
 * <dir> must be one of:
 * 0 - North
 * 2 - South
 * returns result (new address) in <dp>
 * <dp> is set to NULL if there is no different point in the designated
 * direction (e.g. if the patch is degenerated in this column)
 */
void
ay_npt_gndv(char dir, ay_nurbpatch_object *np, int j, double *p,
	   double **dp)
{
 int offset, stride = 4;
 double *p2 = NULL;

  if(dir == AY_NORTH)
    {
      /* calculate offset */
      offset = -stride;
      if(j == 0)
	{
	  *dp = NULL;
	  return;
	}
      else
	{
	  p2 = p + offset;
	}
      /* apply offset to p2 until p2 points to
	 a different control point than p
	 (in terms of their coordinate values)
	 or we run off the edge of the patch */
      while(AY_V3COMP(p, p2))
	{
	  p2 += offset;
	  j--;
	  if(j == 0)
	    {
	      *dp = NULL;
	      return;
	    }
	} /* while */
    }
  else
    {
      /* dir == AY_SOUTH */

      /* calculate offset */
      offset = stride;
      if(j == np->height-1)
	{
	  *dp = NULL;
	  return;
	}
      else
	{
	  p2 = p + offset;
	}
      /* apply offset to p2 until p2 points to
	 a different control point than p
	 (in terms of their coordinate values)
	 or we run off the edge of the patch */
      while(AY_V3COMP(p, p2))
	{
	  p2 += offset;
	  j++;
	  if(j == np->height-1)
	    {
	      *dp = NULL;
	      return;
	    }
	} /* while */
    } /* if */

  *dp = p2;

 return;
} /* ay_npt_gndv */


/* ay_npt_gnduc:
 * get next different control point in dimension u and direction <dir>
 * in a closed NURBS surface <np> where the current points location in
 * its row is <i> and its memory address is <p>;
 * <dir> must be one of:
 * 1 - East
 * 3 - West
 * returns result (new address) in <dp>
 * <dp> is set to NULL if there is no different point in the designated
 * direction (e.g. if the patch is degenerated in this row)
 */
void
ay_npt_gnduc(char dir, ay_nurbpatch_object *np, int i, double *p,
	     double **dp)
{
 int offset, stride = 4;
 double *p2 = NULL;

  if(dir == AY_EAST)
    {
      offset = stride*np->height;
      if(i == np->width-1)
	{
	  /* wrap around */
	  i = 1;
	  p2 = p - ((np->width-2) * np->height * stride);
	}
      else
	{
	  p2 = p + offset;
	}

      /* apply offset to p2 until p2 points to
	 a different control point than p
	 (in terms of their coordinate values)
	 or we get to p again or we fall off the patch */
      while(AY_V3COMP(p, p2))
	{
	  p2 += offset;

	  /* degeneracy checks */
	  if(p == p2 || i == np->width-2)
	    {
	      *dp = NULL;
	      return;
	    }
	  i++;
	} /* while */
    }
  else
    {
      /* dir == AY_WEST */
      offset = -stride*np->height;
      if(i == 0)
	{
	  /* wrap around */
	  i = np->width-2;
	  p2 = p + ((np->width-2) * np->height * stride);
	}
      else
	{
	  p2 = p + offset;
	}

      /* apply offset to p2 until p2 points to
	 a different control point than p
	 (in terms of their coordinate values)
	 or we get to p again or we fall off the patch */
      while(AY_V3COMP(p, p2))
	{
	  p2 += offset;

	  /* degeneracy checks */
	  if(p == p2 || i == 1)
	    {
	      *dp = NULL;
	      return;
	    }
	  i--;
	} /* while */
    } /* if */

  *dp = p2;

 return;
} /* ay_npt_gnduc */


/* ay_npt_gndvc:
 * get next different control point in dimension v and direction <dir>
 * in a closed NURBS surface <np> where the current points location in
 * its column is <j> and its memory address is <p>;
 * <dir> must be one of:
 * 0 - North
 * 2 - South
 * returns result (new address) in <dp>
 * <dp> is set to NULL if there is no different point in the designated
 * direction (e.g. if the patch is degenerated in this column)
 */
void
ay_npt_gndvc(char dir, ay_nurbpatch_object *np, int j, double *p,
	     double **dp)
{
 int offset, stride = 4;
 double *p2 = NULL;

  if(dir == AY_NORTH)
    {
      offset = -stride;
      if(j == 0)
	{
	  /* wrap around */
	  j = np->height-2;
	  p2 = p + ((np->height-2) * stride);
	}
      else
	{
	  p2 = p + offset;
	}

      /* apply offset to p2 until p2 points to
	 a different control point than p
	 (in terms of their coordinate values)
	 or we get to p again or we fall off the patch */
      while(AY_V3COMP(p, p2))
	{
	  p2 += offset;

	  /* degeneracy check */
	  if(p == p2 || j == 1)
	    {
	      *dp = NULL;
	      return;
	    }
	  j--;
	} /* while */
    }
  else
    {
      /* dir == AY_SOUTH */
      offset = stride;
      if(j == np->height-1)
	{
	  /* wrap around */
	  j = 1;
	  p2 = p - ((np->height-2) * stride);
	}
      else
	{
	  p2 = p + offset;
	}

      /* apply offset to p2 until p2 points to
	 a different control point than p
	 (in terms of their coordinate values)
	 or we get to p again or we fall off the patch */
      while(AY_V3COMP(p, p2))
	{
	  p2 += offset;

	  /* degeneracy check */
	  if(p == p2 || j == np->height-2)
	    {
	      *dp = NULL;
	      return;
	    }
	  j++;
	} /* while */
    } /* if */

  *dp = p2;

 return;
} /* ay_npt_gndvc */


/* ay_npt_gndup:
 * get next different control point in dimension u and direction <dir>
 * in a periodic NURBS surface <np> where the current points location in
 * its row is <i> and its memory address is <p>;
 * <dir> must be one of:
 * 1 - East
 * 3 - West
 * returns result (new address) in <dp>
 * <dp> is set to NULL if there is no different point in the designated
 * direction (e.g. if the patch is degenerated in this row)
 */
void
ay_npt_gndup(char dir, ay_nurbpatch_object *np, int i, double *p,
	     double **dp)
{
 int offset, stride = 4;
 double *p2 = NULL;

  if(dir == AY_EAST)
    {
      offset = stride*np->height;
      if(i == np->width-1)
	{
	  /* wrap around */
	  p2 = p - ((np->width-np->uorder) * np->height * stride);
	}
      else
	{
	  p2 = p + offset;
	}
    }
  else
    {
      /* dir == AY_WEST */
      offset = -stride*np->height;
      if(i == 0)
	{
	  /* wrap around */
	  p2 = p + ((np->width-np->uorder) * np->height * stride);
	}
      else
	{
	  p2 = p + offset;
	}
    }

  /* apply offset to p2 until p2 points to
     a different control point than p
     (in terms of their coordinate values)
     or we get to p again */
  while(AY_V3COMP(p, p2))
    {
      p2 += offset;

      /* degeneracy check */
      if(p == p2)
	{
	  *dp = NULL;
	  return;
	}
    } /* while */

  *dp = p2;

 return;
} /* ay_npt_gndup */


/* ay_npt_gndvp:
 * get next different control point in dimension v and direction <dir>
 * in a periodic NURBS surface <np> where the current points location in
 * its column is <j> and its memory address is <p>;
 * <dir> must be one of:
 * 0 - North
 * 2 - South
 * returns result (new address) in <dp>
 * <dp> is set to NULL if there is no different point in the designated
 * direction (e.g. if the patch is degenerated in this column)
 */
void
ay_npt_gndvp(char dir, ay_nurbpatch_object *np, int j, double *p,
	     double **dp)
{
 int offset, stride = 4;
 double *p2 = NULL;

  if(dir == AY_NORTH)
    {
      offset = -stride;
      if(j == 0)
	{
	  /* wrap around */
	  p2 = p + ((np->height-np->vorder) * stride);
	}
      else
	{
	  p2 = p + offset;
	}
    }
  else
    {
      /* dir == AY_SOUTH */
      offset = stride;
      if(j == np->height-1)
	{
	  /* wrap around */
	  p2 = p - ((np->height-np->vorder) * stride);
	}
      else
	{
	  p2 = p + offset;
	}
    }

  /* apply offset to p2 until p2 points to
     a different control point than p
     (in terms of their coordinate values)
     or we get to p again */
  while(AY_V3COMP(p, p2))
    {
      p2 += offset;

      /* degeneracy check */
      if(p == p2)
	{
	  *dp = NULL;
	  return;
	}
    } /* while */

  *dp = p2;

 return;
} /* ay_npt_gndvp */


#define AY_UPDATENORMAL(p1,p2,p3) {\
    ay_geom_calcnfrom3(p1, p2, p3, normal2);\
    if(nnormals)\
      {\
	normal1[0] += normal2[0];\
	normal1[1] += normal2[1];\
	normal1[2] += normal2[2];\
      }\
    else\
      {\
	memcpy(normal1, normal2, 3*sizeof(double));\
      }\
    nnormals++;\
}


/* ay_npt_offset:
 *  create offset surface from <o>
 *  the new surface is <offset> away from the original surface
 *  <mode> is currently unused
 *  returns new patch in <np>
 */
int
ay_npt_offset(ay_object *o, int mode, double offset, ay_nurbpatch_object **np)
{
 int ay_status = AY_OK;
 int i, j, a, stride = 4;
 int nnormals;
 double normal1[3], normal2[3];
 double *newcv = NULL, *newukv = NULL, *newvkv = NULL;
 double *p0, *p1, *p2, *p3, *p4;
 ay_nurbpatch_object *patch = NULL;
 ay_npt_gndcb *gnducb = ay_npt_gndu;
 ay_npt_gndcb *gndvcb = ay_npt_gndv;

  /* sanity check */
  if(!o || !np)
    return AY_ENULL;

  if(o->type != AY_IDNPATCH)
    return AY_ERROR;

  patch = (ay_nurbpatch_object*)o->refine;

  if(patch->utype == AY_CTCLOSED)
    gnducb = ay_npt_gnduc;
  else
    if(patch->utype == AY_CTPERIODIC)
      gnducb = ay_npt_gndup;

  if(patch->vtype == AY_CTCLOSED)
    gndvcb = ay_npt_gndvc;
  else
    if(patch->vtype == AY_CTPERIODIC)
      gndvcb = ay_npt_gndvp;

  if(!(newcv = malloc(patch->width*patch->height*stride*sizeof(double))))
    return AY_EOMEM;

  a = 0;
  for(i = 0; i < patch->width; i++)
    {
      for(j = 0; j < patch->height; j++)
	{
	  p0 = &(patch->controlv[a]);

	  /* get 4 surrounding and different points from p0 */
	  gnducb(AY_EAST, patch, i, p0, &p1);
	  gndvcb(AY_SOUTH, patch, j, p0, &p2);
	  gnducb(AY_WEST, patch, i, p0, &p3);
	  gndvcb(AY_NORTH, patch, j, p0, &p4);

	  /* calulate mean normal from 1-4 normals */
	  nnormals = 0;
	  memset(normal1, 0, 3*sizeof(double));
	  memset(normal2, 0, 3*sizeof(double));

	  if(p1 && p2)
	    {
	      AY_UPDATENORMAL(p0,p1,p2);
	    }
	  if(p2 && p3)
	    {
	      AY_UPDATENORMAL(p0,p2,p3);
	    }
	  if(p3 && p4)
	    {
	      AY_UPDATENORMAL(p0,p3,p4);
	    }
	  if(p4 && p1)
	    {
	      AY_UPDATENORMAL(p0,p4,p1);
	    }

	  if(nnormals == 0)
	    {
	      memset(normal1, 0, 3*sizeof(double));
	      if(p2 || p4)
		{
		  /* degenerate in horizontal direction */
		  /* p2 and p4 are possibly valid */
		  if(p2)
		    {
		      gnducb(AY_EAST, patch, i, p2, &p1);
		      if(p1)
			{
			  AY_UPDATENORMAL(p0,p1,p2);
			}
		      gnducb(AY_WEST, patch, i, p2, &p3);
		      if(p3)
			{
			  AY_UPDATENORMAL(p0,p2,p3);
			}
		    } /* if p2 */
		  if(p4)
		    {
		      gnducb(AY_EAST, patch, i, p4, &p1);
		      if(p1)
			{
			  AY_UPDATENORMAL(p0,p4,p1);
			}
		      gnducb(AY_WEST, patch, i, p4, &p3);
		      if(p3)
			{
			  AY_UPDATENORMAL(p0,p3,p4);
			}
		    } /* if p4 */
		}
	      else
		{
		  /* degenerate in vertical direction */
		  /* p1 and p3 are possibly valid */
		  if(p1)
		    {
		      gndvcb(AY_SOUTH, patch, j, p1, &p2);
		      if(p2)
			{
			  AY_UPDATENORMAL(p0,p1,p2);
			}
		      gndvcb(AY_NORTH, patch, j, p1, &p4);
		      if(p4)
			{
			  AY_UPDATENORMAL(p0,p4,p1);
			}
		    } /* if p1 */
		  if(p3)
		    {
		      gndvcb(AY_SOUTH, patch, j, p3, &p2);
		      if(p2)
			{
			  AY_UPDATENORMAL(p0,p2,p3);
			}
		      gndvcb(AY_NORTH, patch, j, p3, &p4);
		      if(p4)
			{
			  AY_UPDATENORMAL(p0,p3,p4);
			}
		    } /* if p3 */
		} /* if horizontal / vertical */
	    } /* if degenerate */

	  if(nnormals > 1)
	    {
	      normal1[0] /= nnormals;
	      normal1[1] /= nnormals;
	      normal1[2] /= nnormals;
	    }

	  if(nnormals > 0)
	    {
	      newcv[a]   = (p0[0] + (normal1[0] * offset));
	      newcv[a+1] = (p0[1] + (normal1[1] * offset));
	      newcv[a+2] = (p0[2] + (normal1[2] * offset));
	      newcv[a+3] = p0[3];
	    }
	  else
	    {
	      memcpy(&(newcv[a]), p0, stride*sizeof(double));
	    }

	  a += stride;
	} /* for */
    } /* for */

  /* copy knot vectors */
  if(patch->uknot_type == AY_KTCUSTOM)
    {
      if(!(newukv = malloc((patch->width+patch->uorder)*sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      memcpy(newukv, patch->uknotv,
	     (patch->width+patch->uorder)*sizeof(double));
    }
  if(patch->vknot_type == AY_KTCUSTOM)
    {
      if(!(newvkv = malloc((patch->height+patch->vorder)*sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      memcpy(newvkv, patch->vknotv,
	     (patch->height+patch->vorder)*sizeof(double));
    }

  ay_status = ay_npt_create(patch->uorder, patch->vorder,
			    patch->width, patch->height,
			    patch->uknot_type, patch->vknot_type,
			    newcv, newukv, newvkv, np);

cleanup:

  if(ay_status || !*np)
    {
      if(newcv)
	free(newcv);
      if(newukv)
	free(newukv);
      if(newvkv)
	free(newvkv);
    }

 return ay_status;
} /* ay_npt_offset */


/* ay_npt_getnormal:
 *
 */
int
ay_npt_getnormal(ay_nurbpatch_object *np, int i, int j,
		 ay_npt_gndcb *gnducb, ay_npt_gndcb *gndvcb,
		 int store_tangents, double *result)
{
 int nnormals = 0, stride = 4;
 double *p0, *p1, *p2, *p3, *p4;
 double normal1[3] = {0}, normal2[3] = {0};
 double tangentu[3] = {0}, tangentv[3] = {0};

  if(!np || !gnducb || !gndvcb || !result)
    return AY_ENULL;

  p0 = &(np->controlv[(i*np->height+j)*stride]);

  /* get 4 surrounding and different points from p0 */
  gnducb(AY_EAST, np, i, p0, &p1);
  gndvcb(AY_SOUTH, np, j, p0, &p2);
  gnducb(AY_WEST, np, i, p0, &p3);
  gndvcb(AY_NORTH, np, j, p0, &p4);

  /* calulate mean normal from 1-4 normals */
  if(p1 && p2)
    {
      AY_UPDATENORMAL(p0,p1,p2);
    }
  if(p2 && p3)
    {
      AY_UPDATENORMAL(p0,p2,p3);
    }
  if(p3 && p4)
    {
      AY_UPDATENORMAL(p0,p3,p4);
    }
  if(p4 && p1)
    {
      AY_UPDATENORMAL(p0,p4,p1);
    }

  if(nnormals == 0)
    {
      memset(normal1, 0, 3*sizeof(double));
      if(p2 || p4)
	{
	  /* degenerate in horizontal direction */
	  /* p2 and p4 are possibly valid */
	  if(p2)
	    {
	      gnducb(AY_EAST, np, i, p2, &p1);
	      if(p1)
		{
		  AY_UPDATENORMAL(p0,p1,p2);
		}
	      gnducb(AY_WEST, np, i, p2, &p3);
	      if(p3)
		{
		  AY_UPDATENORMAL(p0,p2,p3);
		}
	    } /* if p2 */
	  if(p4)
	    {
	      gnducb(AY_EAST, np, i, p4, &p1);
	      if(p1)
		{
		  AY_UPDATENORMAL(p0,p4,p1);
		}
	      gnducb(AY_WEST, np, i, p4, &p3);
	      if(p3)
		{
		  AY_UPDATENORMAL(p0,p3,p4);
		}
	    } /* if p4 */
	}
      else
	{
	  /* degenerate in vertical direction */
	  /* p1 and p3 are possibly valid */
	  if(p1)
	    {
	      gndvcb(AY_SOUTH, np, j, p1, &p2);
	      if(p2)
		{
		  AY_UPDATENORMAL(p0,p1,p2);
		}
	      gndvcb(AY_NORTH, np, j, p1, &p4);
	      if(p4)
		{
		  AY_UPDATENORMAL(p0,p4,p1);
		}
	    } /* if p1 */
	  if(p3)
	    {
	      gndvcb(AY_SOUTH, np, j, p3, &p2);
	      if(p2)
		{
		  AY_UPDATENORMAL(p0,p2,p3);
		}
	      gndvcb(AY_NORTH, np, j, p3, &p4);
	      if(p4)
		{
		  AY_UPDATENORMAL(p0,p3,p4);
		}
	    } /* if p3 */
	} /* if horizontal / vertical */
    } /* if degenerate */

  if(nnormals > 1)
    {
      normal1[0] /= nnormals;
      normal1[1] /= nnormals;
      normal1[2] /= nnormals;
    }

  /* return result */
  if(nnormals > 0)
    memcpy(result, normal1, 3*sizeof(double));

  if(store_tangents)
    {
      if(p1 && p3)
	{
	  AY_V3SUB(tangentu, p3, p1);
	}
      else
	{
	  if(p1)
	    AY_V3SUB(tangentu, p0, p1);
	  if(p3)
	    AY_V3SUB(tangentu, p3, p0);
	}
      memcpy(result+3, tangentu, 3*sizeof(double));
      if(p2 && p4)
	{
	  AY_V3SUB(tangentv, p4, p2);
	}
      else
	{
	  if(p2)
	    AY_V3SUB(tangentv, p0, p2);
	  if(p4)
	    AY_V3SUB(tangentv, p4, p0);
	}
      memcpy(result+6, tangentv, 3*sizeof(double));
    }

 return AY_OK;
} /* ay_npt_getnormal */


/* ay_npt_finduv:
 *  transforms the window coordinates (winX, winY)
 *  to the corresponding parametric values u, v
 *  on the NURBS surface o
 *  This function needs a valid OpenGL rendering context!
 *
 *  XXXX ToDo: use gluPickMatrix() to speed this up
 *
 */
int
ay_npt_finduv(struct Togl *togl, ay_object *o,
	      double *winXY, double *worldXYZ, double *u, double *v)
{
 int ay_status = AY_OK;
 /*  int width = Togl_Width(togl);*/
 int height = Togl_Height(togl);
 GLint viewport[4];
 GLdouble modelMatrix[16], projMatrix[16], winx, winy;
 GLfloat winz = 0.0f;
 double m[16], mi[16];
 GLfloat pixel1[3] = {0.9f,0.9f,0.9f}, pixel[3] = {0.0f,0.0f,0.0f};
 ay_nurbpatch_object *np = NULL;
 int dx[25] = {0,1,1,0,-1,-1,-1,0,1, 2,2,2,1,0,-1,-2,-2,-2,-2,-2,-1,0,1,2,2};
 int dy[25] = {0,0,-1,-1,-1,0,1,1,1, 0,-1,-2,-2,-2,-2,-2,-1,0,1,2,2,2,2,2,1};
 int found, i = 0, j = 0, k = 0, /*maxtry = 1000,*/ stride;
 int usamples = 10, vsamples = 10;
 int starti = 0, endi = 0;
 int startj = 0, endj = 0;
 double point[4] = {0}/*, guess = 0.0, e1 = 0.05, e2 = 0.05*/;
 double distance = 0.0, min_distance = 0.0;
 double *cp = NULL, *p = NULL;
 double U[10/* XXXX usamples! */] = {0}, startu, endu;
 double V[10/* XXXX vsamples! */] = {0}, startv, endv;
 ay_voidfp *arr = NULL;
 ay_drawcb *cb = NULL;

  if(!o)
    return AY_ENULL;

  if(o->type != AY_IDNPATCH)
    return AY_EWTYPE;

  np = (ay_nurbpatch_object *)o->refine;

  /* shade the selected object without lighting as we are mostly
     interested in the corresponding Z-Buffer values */
  arr = ay_shadecbt.arr;
  cb = (ay_drawcb *)(arr[o->type]);

  glClear(GL_DEPTH_BUFFER_BIT);
  glDisable(GL_LIGHTING);
  /* set color for selected objects */
  glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
	    (GLfloat)ay_prefs.seb);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
   if(ay_currentlevel->object != ay_root)
     ay_trafo_concatparent(ay_currentlevel->next);

   glTranslated(o->movx, o->movy, o->movz);
   ay_quat_torotmatrix(o->quat, m);
   glMultMatrixd(m);
   glScaled(o->scalx, o->scaly, o->scalz);

   if(cb)
     ay_status = cb(togl, o);

   glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
  glPopMatrix();

  glEnable(GL_LIGHTING);

  pixel1[0] = (float)ay_prefs.ser;
  pixel1[1] = (float)ay_prefs.seg;
  pixel1[2] = (float)ay_prefs.seb;

  /*
  winx = winX;
  winy = height - winY;
  */

  /*
   * first, we try to find a point on the surface in window coordinates;
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

  /* get object coordinates of point on surface */
  glGetIntegerv(GL_VIEWPORT, viewport);

  glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

  /* get Z-Buffer value of picked point (winz) */
  glReadPixels((GLint)winx,(GLint)winy,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&winz);

  gluUnProject(winx, winy, (GLdouble)winz, modelMatrix, projMatrix, viewport,
	       &(point[0]), &(point[1]), &(point[2]));

  /* in 7 iterations, search for matching parametric values of <point> */
  stride = 4;
  if(!(cp = malloc(usamples*vsamples*stride*sizeof(double))))
    return AY_EOMEM;

  startu = np->uknotv[np->uorder-1];
  endu = np->uknotv[np->width];

  startv = np->vknotv[np->vorder-1];
  endv = np->vknotv[np->height];

  for(k = 0; k < 7; k++)
    {
      U[0] = startu;
      for(i = 1; i < usamples-1; i++)
	{
	  U[i] = U[i-1]+(endu - startu)/(usamples-1);
	}
      U[usamples-1] = endu;

      V[0] = startv;
      for(i = 1; i < vsamples-1; i++)
	{
	  V[i] = V[i-1]+(endv - startv)/(vsamples-1);
	}
      V[vsamples-1] = endv;
      p = cp;
      for(i = 0; i < usamples-1; i++)
       {
	 for(j = 0; j < vsamples-1; j++)
	   {
	     (void)ay_nb_SurfacePoint4D(np->width-1, np->height-1,
					np->uorder-1, np->vorder-1,
					np->uknotv, np->vknotv,
					np->controlv,
					U[i]+((U[i+1]-U[i])/2.0),
					V[j]+((V[j+1]-V[j])/2.0),
					p);
	     p += stride;
	   } /* for */
       } /* for */

      min_distance = DBL_MAX;
      p = cp;
      for(i = 0; i < (usamples-1); i++)
	{
	 for(j = 0; j < (vsamples-1); j++)
	   {
	     distance = AY_VLEN((point[0] - p[0]),
				(point[1] - p[1]),
				(point[2] - p[2]));

	     if(distance < min_distance)
	       {
		 *u = U[i]+((U[i+1]-U[i])/2.0);
		 starti = i-1;
		 endi = i+1;
		 *v = V[i]+((V[i+1]-V[i])/2.0);
		 startj = j-1;
		 endj = j+1;
		 min_distance = distance;
	       }

	     p += stride;
	   } /* for */
	} /* for */

      if(starti < 0)
	{
	  startu = U[0];
	}
      else
	{
	  startu = U[starti];
	}

      if(endi >= usamples)
	{
	  endu = U[usamples-1];
	}
      else
	{
	  endu = U[endi];
	}

      if(startj < 0)
	{
	  startv = V[0];
	}
      else
	{
	  startv = V[startj];
	}

      if(endj >= vsamples)
	{
	  endv = V[vsamples-1];
	}
      else
	{
	  endv = V[endj];
	}
    } /* for */

  /* compile/return results */
  winXY[0] = winx;
  winXY[1] = winy;

  (void)ay_nb_SurfacePoint4D(np->width-1, np->height-1,
			     np->uorder-1, np->vorder-1,
			     np->uknotv, np->vknotv,
			     np->controlv, *u, *v, point);

  ay_trafo_invmatrix(m, mi);

  ay_trafo_apply4(point, mi);

  worldXYZ[0] = point[0];
  worldXYZ[1] = point[1];
  worldXYZ[2] = point[2];

  free(cp);

 return ay_status;
} /* ay_npt_finduv */


/* ay_npt_finduvcb:
 *  Togl callback to implement find parametric values
 *  u/v for a picked point on a NURBS surface
 */
int
ay_npt_finduvcb(struct Togl *togl, int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "findUV_cb";
 ay_view_object *view = (ay_view_object *)Togl_GetClientData(togl);
 Tcl_Interp *interp = Togl_Interp(togl);
 int height = Togl_Height(togl);
 double winXY[2] = {0}, worldXYZ[3] = {0};
 static int fvalid = AY_FALSE;
 static double fX = 0.0, fY = 0.0, fwX = 0.0, fwY = 0.0, fwZ = 0.0;
 double u = 0.0, v = 0.0;
 Tcl_Obj *to = NULL, *ton = NULL;
 char cmd[] = "puts \"$u $v\"";
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

	      if(ay_prefs.globalmark)
		{
		  ay_viewt_updatemark(togl, AY_FALSE);
		}
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
      if(ay_selection->object->type != AY_IDNPATCH)
	{
	  ay_status = ay_provide_object(ay_selection->object,
					AY_IDNPATCH, &pobject);
	  if(!pobject)
	    {
	      ay_error(AY_EWTYPE, fname, ay_npt_npname);
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

      ay_status = ay_npt_finduv(togl, o, winXY, worldXYZ, &u, &v);

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Could not find point on surface.");
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
      Tcl_SetStringObj(ton, "v", -1);
      to = Tcl_NewDoubleObj(v);
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
      (void)ay_object_deletemulti(pobject, AY_TRUE);
    }

 return TCL_OK;
} /* ay_npt_finduvcb */


/** ay_npt_avglensu:
 *  Compute average control point distances in U direction.
 *
 * \param[in] cv control points [width*height*stride]
 * \param[in] width width of cv
 * \param[in] height height of cv
 * \param[in] stride stride in cv
 * \param[in,out] avlens resulting average distances [width-1]
 *  avlens[0] is the average distance of column0 to column1
 *  avlens[1] the average distance of column1 to column2...
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_avglensu(double *cv, int width, int height, int stride,
		double **avlens)
{
 double *lens;
 int i, j, a, b;

  if(!cv || !avlens)
    {
      return AY_ENULL;
    }

  if(!(lens = malloc((width-1)*sizeof(double))))
    {
      return AY_EOMEM;
    }

  /* compute average partial lengths */
  a = 0;
  b = height;
  for(i = 0; i < width-1; i++)
    {
      lens[i] = 0.0;
      for(j = 0; j < height; j++)
	{
	  if((fabs(cv[b] - cv[a]) > AY_EPSILON) ||
	     (fabs(cv[b+1] - cv[a+1]) > AY_EPSILON) ||
	     (fabs(cv[b+2] - cv[a+2]) > AY_EPSILON))
	    {
	      lens[i] += AY_VLEN((cv[b] - cv[a]),
				 (cv[b+1] - cv[a+1]),
				 (cv[b+2] - cv[a+2]))/width;
	    }
	  a += stride;
	  b += stride;
	} /* for */
    } /* for */

  /* return result */
  *avlens = lens;

 return AY_OK;
} /* ay_npt_avglensu */


/** ay_npt_avglensv:
 *  Compute average control point distances in V direction.
 *
 * \param[in] cv control points [width*height*stride]
 * \param[in] width width of cv
 * \param[in] height height of cv
 * \param[in] stride stride in cv
 * \param[in,out] avlens resulting average distances [height-1]
 *  avlens[0] is the average distance of row0 to row1
 *  avlens[1] the average distance of row1 to row2...
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_avglensv(double *cv, int width, int height, int stride,
		double **avlens)
{
 double *lens;
 int i, j, a, b;

  if(!cv || !avlens)
    {
      return AY_ENULL;
    }

  if(!(lens = malloc((height-1)*sizeof(double))))
    {
      return AY_EOMEM;
    }

  /* compute average partial lengths */
  for(i = 0; i < height-1; i++)
    {
      lens[i] = 0.0;
      a = i*stride;
      b = a+stride;
      for(j = 0; j < width; j++)
	{
	  if((fabs(cv[b] - cv[a]) > AY_EPSILON) ||
	     (fabs(cv[b+1] - cv[a+1]) > AY_EPSILON) ||
	     (fabs(cv[b+2] - cv[a+2]) > AY_EPSILON))
	    {
	      lens[i] += AY_VLEN((cv[b] - cv[a]),
				 (cv[b+1] - cv[a+1]),
				 (cv[b+2] - cv[a+2]))/height;
	    }

	  a += (height*stride);
	  b += (height*stride);
	} /* for */
    } /* for */

  /* return result */
  *avlens = lens;

 return AY_OK;
} /* ay_npt_avglensv */


/** ay_npt_concatstcmd:
 *  Concatenate selected surfaces.
 *  Implements the \a concatS scripting interface command.
 *  See also the corresponding section in the \ayd{scconcats}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_concatstcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status, tcl_status;
 int i = 1, type = AY_CTOPEN, knot_type = AY_KTNURB, order = 0;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL, *patches = NULL, **next = NULL;
 ay_object *newo = NULL;
 double ftlen = 1.0;
 int fillet_type = 0, compat = AY_FALSE;
 char *uv = NULL;

  /* parse args */
  if(argc > 2)
    {
      while(i+1 < argc)
	{
	  if(!strcmp(argv[i], "-c"))
	    {
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &compat);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	    }
	  if(!strcmp(argv[i], "-fl"))
	    {
	      tcl_status = Tcl_GetDouble(interp, argv[i+1], &ftlen);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	    }
	  if(!strcmp(argv[i], "-ft"))
	    {
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &fillet_type);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);
	    }
	  if(!strcmp(argv[i], "-t"))
	    {
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &type);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);

	      if(type < 0)
		{
		  ay_error(AY_ERROR, argv[0],
			   "Parameter <type> must be >= 0.");
		  return TCL_OK;
		}
	    }
	  if(!strcmp(argv[i], "-o"))
	    {
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &order);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);

	      if(type < 0)
		{
		  ay_error(AY_ERROR, argv[0],
			   "Parameter <order> must be >= 0.");
		  return TCL_OK;
		}
	    }
	  if(!strcmp(argv[i], "-k"))
	    {
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &knot_type);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);

	      if(knot_type < 0)
		{
		  ay_error(AY_ERROR, argv[0],
			   "Parameter <knot type> must be >= 0.");
		  return TCL_OK;
		}
	    }
	  if(!strcmp(argv[i], "-u"))
	    {
	      if(argc > i)
		uv = argv[i+1];
	    }
	  i += 2;
	} /* while */
    } /* if */

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  next = &patches;
  while(sel)
    {
      o = sel->object;

      /* copy or provide */
      if(o->type == AY_IDNPATCH)
	{
	  ay_status = ay_object_copy(o, next);
	  if(ay_status)
	    goto cleanup;
	  next = &((*next)->next);
	}
      else
	{
	  if(ay_provide_object(o, AY_IDNPATCH, NULL) == AY_OK)
	    {
	      ay_provide_object(o, AY_IDNPATCH, next);
	      while(*next)
		{
		  next = &((*next)->next);
		}
	    }
	  else
	    {
	      if(o->type == AY_IDNCURVE)
		{
		  ay_status = ay_object_copy(o, next);
		  if(ay_status)
		    goto cleanup;
		  next = &((*next)->next);
		}
	      else
		{
		  ay_provide_object(o, AY_IDNCURVE, next);
		  while(*next)
		    {
		      next = &((*next)->next);
		    }
		} /* if */
	    } /* if */
	} /* if */

      sel = sel->next;
    } /* while */

  /* gracefully exit, if there are no
     (or not enough?) patches to concat */
  if(!patches)
    {
      ay_error(AY_EWARN, argv[0], "Not enough patches to concat!");
      return TCL_OK;
    }

  ay_status = ay_npt_concat(patches, type, order, knot_type, fillet_type,
			    ftlen, compat, uv, &newo);

  if(ay_status)
    {
      ay_error(AY_ERROR, argv[0], "Failed to concat!");
    }
  else
    {
      ay_object_link(newo);
    } /* if */

  (void)ay_notify_parent();

cleanup:
  /* free list of temporary patches/curves */
  (void)ay_object_deletemulti(patches, AY_TRUE);

 return TCL_OK;
} /* ay_npt_concatstcmd */


/** ay_npt_remknunptcmd:
 *  Remove a knot from the selected NURBS patches.
 *  Implements the \a remknuNP scripting interface command.
 *  See also the corresponding section in the \ayd{scextrnp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_remknunptcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_object *o = NULL;
 ay_nurbpatch_object *patch = NULL;
 ay_list_object *sel = ay_selection;
 double u = 0.0, tol = DBL_MAX, *newcontrolv = NULL, *newknotv = NULL;
 int have_index = AY_FALSE, i = 1, j = 0, r = 0, s = 0;
 int notify_parent = AY_FALSE;

  if(argc < 3)
    {
      ay_error(AY_EARGS, argv[0], "(u | -i ind) r [tol]");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if((argv[1][0] == '-') && (argv[1][1] == 'i'))
    {
      tcl_status = Tcl_GetInt(interp, argv[2], &j);
      AY_CHTCLERRRET(tcl_status, argv[0], interp);
      if(j < 0)
	{
	  ay_error(AY_ERROR, argv[0], "Index must be > 0.");
	  return TCL_OK;
	}
      have_index = AY_TRUE;
      i++;
    }
  else
    {
      tcl_status = Tcl_GetDouble(interp, argv[1], &u);
      AY_CHTCLERRRET(tcl_status, argv[0], interp);
    }
  i++;
  tcl_status = Tcl_GetInt(interp, argv[i], &r);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  i++;

  if(r <= 0)
    {
      ay_error(AY_ERROR, argv[0], "Parameter r must be > 0.");
      return TCL_OK;
    }

  if(argc > 3+have_index)
    {
      if((argv[i][0] != 'I') && (argv[i][0] != 'i'))
	{
	  tcl_status = Tcl_GetDouble(interp, argv[i], &tol);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	}
    }

  while(sel)
    {
      o = sel->object;
      if(o->type == AY_IDNPATCH)
	{
	  patch = (ay_nurbpatch_object*)o->refine;

	  /* find knot to remove */
	  if(have_index)
	    {
	      if(j >= (patch->width+patch->uorder))
		{
		  (void)ay_error_reportirange(argv[0], "\"index\"", 0,
					      patch->width+patch->uorder-1);
		  break;
		}
	      u = patch->uknotv[j];
	    }

	  /* even if we have an index already, this makes sure we get to
	     know the first of the possibly multiple knots (to correctly
	     compute the current multiplicity) */
	  i = 0;
	  while((i<(patch->width+patch->uorder)) &&
		(fabs(patch->uknotv[i]-u) > AY_EPSILON))
	    {
	      i++;
	    }

	  if((i == (patch->width+patch->uorder)) ||
	     (fabs(patch->uknotv[i]-u) >= AY_EPSILON))
	    {
	      ay_error(AY_ERROR, argv[0], "Could not find knot to remove.");
	      break;
	    }

	  /* calculate knot multiplicity */
	  s = 1;
	  while(fabs(patch->uknotv[i] - patch->uknotv[i+s]) < AY_EPSILON)
	    {
	      s++;
	    }

	  /* we can not remove knots more often than they appear */
	  if(r > s)
	    {
	      r = s;
	    }

	  /* swap U/V, for there is no RemoveKnotSurfU() */
	  ay_status = ay_npt_swapuv(patch);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "SwapUV failed.");
	      return TCL_OK;
	    }

	  if(!(newcontrolv = malloc(patch->width*(patch->height-r)*4*
				    sizeof(double))))
	    {
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }
	  if(!(newknotv = malloc((patch->height+patch->vorder)*
				 sizeof(double))))
	    {
	      free(newcontrolv);
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }

	  /* remove the knot */
	  ay_status = ay_nb_RemoveKnotSurfV(patch->width-1, patch->height-1,
					    patch->vorder-1,
					    patch->vknotv, patch->controlv,
					    tol, i, s, r,
					    newknotv, newcontrolv);

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "Knot removal failed.");
	      free(newcontrolv);
	      newcontrolv = NULL;
	      free(newknotv);
	      newknotv = NULL;
	      break;
	    }

	  /* save results */
	  patch->height -= r;

	  free(patch->controlv);
	  free(patch->vknotv);
	  patch->controlv = newcontrolv;
	  patch->vknotv = newknotv;

	  patch->vknot_type = ay_knots_classify(patch->vorder, patch->vknotv,
						patch->vorder+patch->height,
						AY_EPSILON);

	  ay_npt_recreatemp(patch);

	  /* swap back */
	  ay_status = ay_npt_swapuv(patch);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "SwapUV failed.");
	      return TCL_OK;
	    }

	  /* remove all selected points */
	  if(o->selp)
	    {
	      ay_selp_clear(o);
	    }

	  o->modified = AY_TRUE;

	  /* re-create tesselation of patch */
	  (void)ay_notify_object(o);
	  notify_parent = AY_TRUE;
	}
      else
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	} /* if */
      sel = sel->next;
    } /* while */

  if(notify_parent)
    (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_remknunptcmd */


/** ay_npt_remknvnptcmd:
 *  Remove a knot from the selected NURBS patches.
 *  Implements the \a remknvNP scripting interface command.
 *  See also the corresponding section in the \ayd{scextrnp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_remknvnptcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_object *o = NULL;
 ay_nurbpatch_object *patch = NULL;
 ay_list_object *sel = ay_selection;
 double v = 0.0, tol = DBL_MAX, *newcontrolv = NULL, *newknotv = NULL;
 int have_index = AY_FALSE, i = 1, j = 0, r = 0, s = 0;
 int notify_parent = AY_FALSE;

  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "(v | -i ind) r [tol]");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if((argv[1][0] == '-') && (argv[1][1] == 'i'))
    {
      tcl_status = Tcl_GetInt(interp, argv[2], &j);
      AY_CHTCLERRRET(tcl_status, argv[0], interp);
      if(j < 0)
	{
	  ay_error(AY_ERROR, argv[0], "Index must be > 0.");
	  return TCL_OK;
	}
      have_index = AY_TRUE;
      i++;
    }
  else
    {
      tcl_status = Tcl_GetDouble(interp, argv[1], &v);
      AY_CHTCLERRRET(tcl_status, argv[0], interp);
    }

  i++;
  tcl_status = Tcl_GetInt(interp, argv[2], &r);
  AY_CHTCLERRRET(tcl_status, argv[0], interp);
  i++;

  if(r <= 0)
    {
      ay_error(AY_ERROR, argv[0], "Parameter r must be > 0.");
      return TCL_OK;
    }

  if(argc > 3+have_index)
    {
      if((argv[i][0] != 'I') && (argv[i][0] != 'i'))
	{
	  tcl_status = Tcl_GetDouble(interp, argv[i], &tol);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	}
    }

  while(sel)
    {
      o = sel->object;
      if(o->type == AY_IDNPATCH)
	{
	  patch = (ay_nurbpatch_object*)o->refine;

	  /* find knot to remove */
	  if(have_index)
	    {
	      if(j >= (patch->height+patch->vorder))
		{
		  (void)ay_error_reportirange(argv[0], "\"index\"", 0,
					      patch->height+patch->vorder-1);
		  break;
		}
	      v = patch->vknotv[j];
	    }

	  /* even if we have an index already, this makes sure we get to
	     know the first of the possibly multiple knots (to correctly
	     compute the current multiplicity) */
	  i = 0;
	  while((i<(patch->height+patch->vorder)) &&
		(fabs(patch->vknotv[i]-v) > AY_EPSILON))
	    {
	      i++;
	    }

	  if((i == (patch->height+patch->vorder)) ||
	     (fabs(patch->vknotv[i]-v) >= AY_EPSILON))
	    {
	      ay_error(AY_ERROR, argv[0], "Could not find knot to remove.");
	      break;
	    }

	  /* calculate knot multiplicity */
	  s = 1;
	  while(fabs(patch->vknotv[i] - patch->vknotv[i+s]) < AY_EPSILON)
	    {
	      s++;
	    }

	  /* we can not remove knots more often than they appear */
	  if(r > s)
	    {
	      r = s;
	    }

	  if(!(newcontrolv = malloc(patch->width*(patch->height-r)*4*
				    sizeof(double))))
	    {
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }
	  if(!(newknotv = malloc((patch->height+patch->vorder)*
				 sizeof(double))))
	    {
	      free(newcontrolv);
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }

	  /* remove the knot */
	  ay_status = ay_nb_RemoveKnotSurfV(patch->width-1, patch->height-1,
					    patch->vorder-1,
					    patch->vknotv, patch->controlv,
					    tol, i, s, r,
					    newknotv, newcontrolv);

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "Knot removal failed.");
	      free(newcontrolv);
	      newcontrolv = NULL;
	      free(newknotv);
	      newknotv = NULL;
	      break;
	    }

	  /* save results */
	  patch->height -= r;

	  free(patch->controlv);
	  free(patch->vknotv);
	  patch->controlv = newcontrolv;
	  patch->vknotv = newknotv;

	  patch->vknot_type = ay_knots_classify(patch->vorder, patch->vknotv,
						patch->vorder+patch->height,
						AY_EPSILON);

	  ay_npt_recreatemp(patch);

	  /* remove all selected points */
	  if(o->selp)
	    {
	      ay_selp_clear(o);
	    }

	  o->modified = AY_TRUE;

	  /* re-create tesselation of patch */
	  (void)ay_notify_object(o);
	  notify_parent = AY_TRUE;
	}
      else
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	} /* if */
      sel = sel->next;
    } /* while */

  if(notify_parent)
    (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_remknvnptcmd */


/** ay_npt_refineu:
 *  refine a NURBS surface in direction u by inserting knots at
 *  the right places, thus not changing the shape of the surface
 *
 * \param[in,out] patch NURBS surface object to refine
 * \param[in] newknotv vector of new knot values (may be NULL)
 * \param[in] newknotvlen length of vector
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_refineu(ay_nurbpatch_object *patch, double *newknotv, int newknotvlen)
{
 double *X = NULL, *Ubar = NULL, *Qw = NULL, *knotv;
 int count = 0, i, j;
 char fname[] = "npt_refineu";

  if(!patch)
    return AY_ENULL;

  knotv = patch->uknotv;
  if(newknotv)
    {
      if(newknotvlen == 0)
	return AY_ERROR;

      X = newknotv;
    }

  /* alloc X, new knotv & new controlv */
  if(!X)
    {
      count = 0;
      for(i = patch->uorder-1; i < patch->width; i++)
	{
	  if(knotv[i] != knotv[i+1])
	    count++;
	}

      if(!(X = malloc(count*sizeof(double))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return AY_ERROR;
	}

      /* fill X (contains just the new u values) */
      j = 0;

      for(i = patch->uorder-1; i < patch->width; i++)
	{
	  if(knotv[i] != knotv[i+1])
	    {
	      X[j] = knotv[i]+((knotv[i+1]-knotv[i])/2.0);
	      j++;
	    }
	} /* for */
    }
  else
    {
      count = newknotvlen;
    } /* if */

  if(!(Ubar = malloc((patch->width + patch->uorder + count)*
		     sizeof(double))))
    {
      free(X);
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }
  if(!(Qw = malloc((patch->width + count)*patch->height*4*sizeof(double))))
    {
      free(X); free(Ubar);
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  /* fill Ubar & Qw */
  ay_nb_RefineKnotVectSurfU4D(4, patch->width-1, patch->height-1,
			      patch->uorder-1, patch->uknotv, patch->controlv,
			      X, count-1, Ubar, Qw);

  free(patch->uknotv);
  patch->uknotv = Ubar;

  free(patch->controlv);
  patch->controlv = Qw;

  if(!newknotv)
    {
      free(X);
    }

  patch->width += count;

  patch->uknot_type = ay_knots_classify(patch->uorder, patch->uknotv,
				       patch->uorder+patch->width,
				       AY_EPSILON);

  /* since we do not create new multiple points
     we only need to re-create them if there were
     already multiple points in the original patch */
  if(patch->mpoints)
    ay_npt_recreatemp(patch);

 return AY_OK;
} /* ay_nct_refineu */


/** ay_npt_refinev:
 *  refine a NURBS surface in v direction by inserting knots at
 *  the right places, thus not changing the shape of the surface
 *
 * \param[in,out] patch NURBS surface object to refine
 * \param[in] newknotv vector of new knot values (may be NULL)
 * \param[in] newknotvlen length of vector
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_npt_refinev(ay_nurbpatch_object *patch, double *newknotv, int newknotvlen)
{
 double *X = NULL, *Vbar = NULL, *Qw = NULL, *knotv;
 int count = 0, i, j;
 char fname[] = "npt_refinev";

  if(!patch)
    return AY_ENULL;

  knotv = patch->vknotv;
  if(newknotv)
    {
      if(newknotvlen == 0)
	return AY_ERROR;

      X = newknotv;
    }

  /* alloc X, new knotv & new controlv */
  if(!X)
    {
      count = 0;
      for(i = patch->vorder-1; i < patch->height; i++)
	{
	  if(knotv[i] != knotv[i+1])
	    count++;
	}

      if(!(X = malloc(count*sizeof(double))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return AY_ERROR;
	}

      /* fill X (contains just the new u values) */
      j = 0;

      for(i = patch->vorder-1; i < patch->height; i++)
	{
	  if(knotv[i] != knotv[i+1])
	    {
	      X[j] = knotv[i]+((knotv[i+1]-knotv[i])/2.0);
	      j++;
	    }
	} /* for */
    }
  else
    {
      count = newknotvlen;
    } /* if */

  if(!(Vbar = malloc((patch->height + patch->vorder + count)*
		     sizeof(double))))
    {
      free(X);
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }
  if(!(Qw = malloc((patch->height + count)*patch->width*4*sizeof(double))))
    {
      free(X); free(Vbar);
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  /* fill Vbar & Qw */
  ay_nb_RefineKnotVectSurfV4D(4, patch->width-1, patch->height-1,
			      patch->vorder-1, patch->vknotv, patch->controlv,
			      X, count-1, Vbar, Qw);

  free(patch->vknotv);
  patch->vknotv = Vbar;

  free(patch->controlv);
  patch->controlv = Qw;

  if(!newknotv)
    {
      free(X);
    }

  patch->height += count;

  patch->vknot_type = ay_knots_classify(patch->vorder, patch->vknotv,
				       patch->vorder+patch->height,
				       AY_EPSILON);

  /* since we do not create new multiple points
     we only need to re-create them if there were
     already multiple points in the original patch */
  if(patch->mpoints)
    ay_npt_recreatemp(patch);

 return AY_OK;
} /* ay_nct_refinev */


/** ay_npt_refineuvtcmd:
 *  Refine U/V direction of selected NURBS patches.
 *  Implements the \a refineuNP and \a refinevNP scripting
 *  interface commands.
 *  See also the corresponding section in the \ayd{screfineunp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_refineuvtcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int tcl_status = TCL_OK, ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *patch = NULL;
 double *X = NULL;
 int refinev = AY_FALSE;
 int aknotc = 0, i;
 int notify_parent = AY_FALSE;
 char **aknotv = NULL;

  if(argc >= 2)
    {
      Tcl_SplitList(interp, argv[1], &aknotc, &aknotv);

      if(!(X = malloc(aknotc*sizeof(double))))
	{
	  ay_error(AY_EOMEM,argv[0],NULL);
	  if(aknotv)
	    Tcl_Free((char *) aknotv);
	  return TCL_OK;
	}

      for(i = 0; i < aknotc; i++)
	{
	  tcl_status = Tcl_GetDouble(interp, aknotv[i], &X[i]);
	  AY_CHTCLERRGOT(tcl_status, argv[0], interp);
	} /* for */

      Tcl_Free((char *) aknotv);
      aknotv = NULL;
    }

  if(!strcmp(argv[0], "refinevNP"))
    refinev = AY_TRUE;

  while(sel)
    {
      if(sel->object->type == AY_IDNPATCH)
	{
	  patch = (ay_nurbpatch_object *)sel->object->refine;

	  if(refinev)
	    ay_status = ay_npt_refinev(patch, X, aknotc);
	  else
	    ay_status = ay_npt_refineu(patch, X, aknotc);

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, argv[0], "Refine failed.");
	      goto cleanup;
	    }
	  else
	    {
	      sel->object->modified = AY_TRUE;

	      if(sel->object->selp)
		{
		  ay_selp_clear(sel->object);
		}

	      /* re-create tesselation of patch */
	      (void)ay_notify_object(sel->object);
	      notify_parent = AY_TRUE;
	    }
	}
      else
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	} /* if */

      sel = sel->next;
    } /* while */

  if(notify_parent)
    (void)ay_notify_parent();

cleanup:

  if(X)
    {
      free(X);
    }

  if(aknotv)
    {
      Tcl_Free((char *) aknotv);
    }

 return TCL_OK;
} /* ay_npt_refineuvtcmd */


/** ay_npt_getcvnormals:
 *  Get normals for all control points of a NURBS patch; the normals
 *  will be derived from the surrounding control points.
 *
 * \param[in] np NURBS surface object to process
 * \param[in,out] n pointer to array [3*np->width*np->height]
 *   where to store the normals
 */
void
ay_npt_getcvnormals(ay_nurbpatch_object *np, double *n)
{
 int a, i, j;
 ay_npt_gndcb *gnducb = ay_npt_gndu;
 ay_npt_gndcb *gndvcb = ay_npt_gndv;

  if(np->utype == AY_CTCLOSED)
    gnducb = ay_npt_gnduc;
  else
    if(np->utype == AY_CTPERIODIC)
      gnducb = ay_npt_gndup;

  if(np->vtype == AY_CTCLOSED)
    gndvcb = ay_npt_gndvc;
  else
    if(np->vtype == AY_CTPERIODIC)
      gndvcb = ay_npt_gndvp;

  a = 0;
  for(i = 0; i < np->width; i++)
    {
      for(j = 0; j < np->height; j++)
	{
	  ay_npt_getnormal(np, i, j, gnducb, gndvcb, 0, &(n[a]));
	  a += 3;
	}
    }

 return;
} /* ay_npt_getcvnormals */


/** ay_npt_unclamptcmd:
 *  Unclamp the selected NURBS surfaces.
 *  Implements the \a unclampuNP scripting interface command.
 *  Also implements the \a unclampvNP scripting interface command.
 *  See also the corresponding section in the \ayd{scunclampnp}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_npt_unclamptcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK, tcl_status = TCL_OK, unclampv = AY_FALSE, side = 0;
 ay_nurbpatch_object *patch;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 int free_selp = AY_FALSE;
 int notify_parent = AY_FALSE;

  /* parse args */
  if(argc > 1)
   {
     if((argv[1][0] == '-') && (argv[1][1] == 's'))
       side = 1;
     else
       if((argv[1][0] == '-') && (argv[1][1] == 'e'))
	 side = 2;
       else
	 {
	   tcl_status = Tcl_GetInt(interp, argv[1], &side);
	   AY_CHTCLERRRET(tcl_status, argv[0], interp);
	 }
   }

  if(!strcmp(argv[0], "unclampvNP"))
    unclampv = AY_TRUE;

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;
      if(o->type != AY_IDNPATCH)
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	}
      else
	{
	  patch = (ay_nurbpatch_object *)o->refine;

	  if(!unclampv)
	    {
	      /* unclamping only works correctly on completely clamped
		 knot vectors... */
	      if(!ay_knots_isclamped(side, patch->uorder, patch->uknotv,
				     patch->width+patch->uorder, AY_EPSILON))
		{
		  ay_status = ay_npt_clampu(patch, side);
		  if(ay_status)
		    break;
		  free_selp = AY_TRUE;
		}

	      ay_nb_UnclampSurfaceU(patch->is_rat,
				    patch->width-1, patch->height-1,
				    patch->uorder-1, side,
				    patch->uknotv, patch->controlv);

	      patch->uknot_type = ay_knots_classify(patch->uorder,patch->uknotv,
						    patch->uorder+patch->width,
						    AY_EPSILON);
	    }
	  else
	    {
	      /* unclamping only works correctly on completely clamped
		 knot vectors... */
	      if(!ay_knots_isclamped(side, patch->vorder, patch->vknotv,
				     patch->height+patch->vorder, AY_EPSILON))
		{
		  ay_status = ay_npt_clampv(patch, side);
		  if(ay_status)
		    break;
		  free_selp = AY_TRUE;
		}

	      ay_nb_UnclampSurfaceV(patch->is_rat,
				    patch->width-1, patch->height-1,
				    patch->vorder-1, side,
				    patch->vknotv, patch->controlv);

	      patch->vknot_type = ay_knots_classify(patch->vorder,patch->vknotv,
						  patch->vorder+patch->height,
						  AY_EPSILON);
	    }

	  if(free_selp)
	    {
	      ay_selp_clear(o);
	    }

	  /* clean up */
	  ay_npt_recreatemp(patch);

	  /* show ay_notify_parent() the changed objects */
	  o->modified = AY_TRUE;

	  /* re-create tesselation of patch */
	  (void)ay_notify_object(o);
	  notify_parent = AY_TRUE;
	} /* if */

      sel = sel->next;
    } /* while */

  if(notify_parent)
    (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_unclamptcmd */


/** ay_npt_gentexcoords:
 *  Generate texture coordinates for all control points of a NURBS patch.
 *
 * \param[in] np NURBS surface object to process
 * \param[in] tags list of tags that potentially contains a TC tag
 * \param[in,out] result pointer to array [2*np->width*np->height]
 *   where to store the texture coordinates
 */
void
ay_npt_gentexcoords(ay_nurbpatch_object *np, ay_tag *tags, double **result)
{
 ay_tag *tag;
 int a, i, j;
 double *tc, u0, u, ud, v0, v, vd;
 double q[8];

  if(!(tc = malloc(2*np->width*np->height*sizeof(double))))
    return;

  tag = tags;
  while(tag)
    {
      if(tag->type == ay_tc_tagtype)
	{
	  if(sscanf(tag->val, "%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg",
		    &(q[0]),&(q[1]),&(q[2]),&(q[3]),
		    &(q[4]),&(q[5]),&(q[6]),&(q[7])) != 8)
	    {
	      break;
	    }
	  else
	    {
	      a = 0;
	      for(i = 0; i < np->width; i++)
		{
		  for(j = 0; j < np->height; j++)
		    {
		      u0 = q[0]+(q[2]-q[0])*(i/((double)np->width-1));
		      ud = q[4]+(q[6]-q[4])*(i/((double)np->width-1));
		      u = u0+(ud-u0)*(j/((double)np->height-1));

		      v0 = q[1]+(q[3]-q[1])*(i/((double)np->width-1));
		      vd = q[5]+(q[7]-q[5])*(i/((double)np->width-1));
		      v = v0+(vd-v0)*(j/((double)np->height-1));

		      tc[a] = u;
		      tc[a+1] = v;
		      a += 2;
		    }
		}

	      /* return result */
	      *result = tc;

	      return;
	    } /* if */
	} /* if */
      tag = tag->next;
    } /* while tag */

  u0 = np->uknotv[np->uorder-1];
  ud = (np->uknotv[np->width]-u0)/np->width;
  v0 = np->vknotv[np->vorder-1];
  vd = (np->vknotv[np->height]-v0)/np->height;

  a = 0;
  u = u0;
  for(i = 0; i < np->width; i++)
    {
      v = v0;
      for(j = 0; j < np->height; j++)
	{
	  tc[a] = u;
	  tc[a+1] = v;
	  a += 2;
	  v = v0+j*vd;
	}
      u = u0+i*ud;
    }

  /* return result */
  *result = tc;

 return;
} /* ay_npt_gentexcoords */


/* templates */
#if 0

/* Tcl command */

/* ay_npt_xxxxtcmd:
 *
 */
int
ay_npt_xxxxtcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 int ay_status, tcl_status;
 int i = 1;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_nurbpatch_object *patch = NULL;
 int notify_parent = AY_FALSE;

  /* parse args */
  if(argc > 2)
    {
      while(i+1 < argc)
	{
	  if(!strcmp(argv[i], "-r"))
	    {
	      mode = 0;
	      tcl_status = Tcl_GetDouble(interp, argv[i+1], &rmin);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);

	      if(rmin <= 0)
		{
		  ay_error(AY_ERROR, argv[0], "rmin must be > 0");
		  return TCL_OK;
		}
	    }
	  if(!strcmp(argv[i], "-d"))
	    {
	      mode = 1;
	      tcl_status = Tcl_GetInt(interp, argv[i+1], &mindist_type);
	      AY_CHTCLERRRET(tcl_status, argv[0], interp);

	      if(mindist_type <= 0)
		{
		  ay_error(AY_ERROR, argv[0], "mindist must be > 0");
		  return TCL_OK;
		}
	    }
	  i += 2;
	} /* while */
    } /* if */

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;
      if(o->type != AY_IDNPATCH)
	{
	  ay_error(AY_EWARN, argv[0], ay_error_igntype);
	}
      else
	{
	  patch = (ay_nurbpatch_object*)o->refine;

	  /* do magic */

	  /* clean up */
	  /* re-create multiple points */
	  ay_npt_recreatemp(patch);

	  ay_selp_clear(o);

	  /* show ay_notify_parent() the changed objects */
	  o->modified = AY_TRUE;

	  /* re-create tesselation of patch */
	  (void)ay_notify_object(o);
	  notify_parent = AY_TRUE;
	} /* if */

      sel = sel->next;
    } /* while */

  if(notify_parent)
    (void)ay_notify_parent();

 return TCL_OK;
} /* ay_npt_xxxxtcmd */

#endif
