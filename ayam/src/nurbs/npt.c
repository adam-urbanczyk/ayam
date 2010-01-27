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

/* npt.c NURBS patch tools */

/* local variables */

char ay_npt_npname[] = "NPatch";

/* prototypes of functions local to this module: */
int ay_npt_rescaletrim(ay_object *trim,
		       int mode, double omin, double omax,
		       double nmin, double nmax);

/* functions */

/* ay_npt_create:
 *   create a NURBS patch object
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

  *patchptr = patch;

 return AY_OK;
} /* ay_npt_create */


/* ay_npt_destroy:
 *   destroy a NURBS patch object
 */
int
ay_npt_destroy(ay_nurbpatch_object *patch)
{

  if(!patch)
    return AY_ENULL;

  if(patch->mpoints)
    ay_npt_clearmp(patch);

  if(patch->controlv)
    free(patch->controlv);

  if(patch->uknotv)
    free(patch->uknotv);

  if(patch->vknotv)
    free(patch->vknotv);

  free(patch);

 return AY_OK;
} /* ay_npt_destroy */


/* ay_npt_createnpatchobject:
 *   properly create and set up an ay_object structure to be used
 *   with a NURBS patch object
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


/* ay_npt_resizearrayw:
 *  change width of a 2D control point array <controlvptr> with
 *  stride <stride>, width <width>, and height <height> to new
 *  width <new_width> inserting new control points obtained by
 *  linear interpolation
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

  if(!(ncontrolv = calloc(height*new_width*stride,sizeof(double))))
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
	return AY_EOMEM;

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

		  ncontrolv[b] = controlv[a]+v[0];
		  ncontrolv[b+1] = controlv[a+1]+v[1];
		  ncontrolv[b+2] = controlv[a+2]+v[2];
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


/* ay_npt_resizew:
 *  change width of a NURBPatch
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

  np->width = new_width;

 return ay_status;
} /* ay_npt_resizew */


/* ay_npt_resizearrayh:
 *  change height of a 2D control point array <controlvptr> with
 *  stride <stride>, width <width>, and height <height> to new
 *  height <new_height> inserting new control points obtained by
 *  linear interpolation
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

  if(!(ncontrolv = calloc(width*new_height*stride,sizeof(double))))
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
	return AY_EOMEM;

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
	      b+=stride;

	      for(j = 1; j <= newpersec[i]; j++)
		{
		  v[0] = controlv[a+stride] - controlv[a];
		  v[1] = controlv[a+stride+1] - controlv[a+1];
		  v[2] = controlv[a+stride+2] - controlv[a+2];


		  t = j/(newpersec[i]+1.0);

		  AY_V3SCAL(v,t);

		  ncontrolv[b] = controlv[a]+v[0];
		  ncontrolv[b+1] = controlv[a+1]+v[1];
		  ncontrolv[b+2] = controlv[a+2]+v[2];
		  ncontrolv[b+3] = 1.0;

		  b+=stride;
		} /* for */

	      a+=stride;

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


/* ay_npt_resizeh:
 *  change height of a NURBPatch
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

  np->height = new_height;

 return ay_status;
} /* ay_npt_resizeh */


/* ay_npt_swaparray:
 *  swap u and v dimensions of a 2D control point array
 */
int
ay_npt_swaparray(double **controlvptr, int stride,
		 int width, int height)
{
 int i1 = 0, i2 = 0, i, j;
 double *ncontrolv = NULL;

  if(!controlvptr || !(*controlvptr))
    return AY_ENULL;

  if(!(ncontrolv = calloc(width*height*stride, sizeof(double))))
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


/* ay_npt_swapuv:
 *  swap u and v dimensions of the NURBS patch <np>
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

  i = np->width;
  np->width = np->height;
  np->height = i;

  i = np->uorder;
  np->uorder = np->vorder;
  np->vorder = i;

  i = np->uknot_type;
  np->uknot_type = np->vknot_type;
  np->vknot_type = i;

  dt = np->uknotv;
  np->uknotv = np->vknotv;
  np->vknotv = dt;

  ay_status = ay_npt_recreatemp(np);

 return ay_status;
} /* ay_npt_swapuv */


/* ay_npt_revertu:
 *  revert control vector of NURBS patch <np> in u dimension
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

  ay_status = ay_npt_recreatemp(np);

 return ay_status;
} /* ay_npt_revertu */


/* ay_npt_revertutcmd:
 *  Tcl interface for revertu() above
 */
int
ay_npt_revertutcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 /*int ay_status;*/
 char fname[] = "revertuS";
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *np = NULL;
 ay_pamesh_object *pm = NULL;
 ay_bpatch_object *bp = NULL;
 double pt[3];

  while(sel)
    {
      switch(sel->object->type)
	{
	case AY_IDNPATCH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  np = (ay_nurbpatch_object *)sel->object->refine;

	  ay_npt_revertu(np);

	  sel->object->modified = AY_TRUE;
	  break;
	case AY_IDPAMESH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  pm = (ay_pamesh_object *)sel->object->refine;

	  ay_pmt_revertu(pm);

	  sel->object->modified = AY_TRUE;
	  break;
	case AY_IDBPATCH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

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
	  ay_error(AY_EWTYPE, fname, "NPatch, PaMesh, BPatch");
	  break;
	} /* switch */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_npt_revertutcmd */


/* ay_npt_revertv:
 *  revert control vector of NURBS patch <np> in v dimension
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

  ay_status = ay_npt_recreatemp(np);

 return ay_status;
} /* ay_npt_revertv */


/* ay_npt_revertvtcmd:
 *  Tcl interface for revertv() above
 */
int
ay_npt_revertvtcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 /*int ay_status;*/
 char fname[] = "revertvS";
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *np = NULL;
 ay_pamesh_object *pm = NULL;
 ay_bpatch_object *bp = NULL;
 double pt[3];

  while(sel)
    {
      switch(sel->object->type)
	{
	case AY_IDNPATCH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  np = (ay_nurbpatch_object *)sel->object->refine;

	  ay_npt_revertv(np);

	  sel->object->modified = AY_TRUE;
	  break;
	case AY_IDPAMESH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  pm = (ay_pamesh_object *)sel->object->refine;

	  ay_pmt_revertv(pm);

	  sel->object->modified = AY_TRUE;
	  break;
	case AY_IDBPATCH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

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
	  ay_error(AY_EWTYPE, fname, "NPatch, PaMesh, BPatch");
	  break;
	} /* switch */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_npt_revertvtcmd */


/* ay_npt_drawtrimcurve:
 *  draw trimcurves with GLU
 */
int
ay_npt_drawtrimcurve(struct Togl *togl, ay_object *o, GLUnurbsObj *no)
{
 int order = 0, length = 0, knot_count = 0, i = 0, a = 0, b = 0;
 ay_nurbcurve_object *curve = (ay_nurbcurve_object *) o->refine;
 static GLfloat *knots = NULL, *controls = NULL;
 double m[16], x = 0.0, y = 0.0, w = 1.0;

  /* get curves transformation-matrix */
  ay_trafo_creatematrix(o, m);

  if(controls)
    {
      free(controls);
      controls = NULL;
    }

  if(knots)
    {
      free(knots);
      knots = NULL;
    }

  order = curve->order;
  length = curve->length;

  knot_count = length + order;

  if((knots = calloc(knot_count, sizeof(GLfloat))) == NULL)
    return AY_EOMEM;
  if((controls = calloc(length*(curve->is_rat?3:2),
			sizeof(GLfloat))) == NULL)
    { free(knots); knots = NULL; return AY_EOMEM; }

  a = 0;
  for(i = 0; i < knot_count; i++)
    {
      knots[a] = (GLfloat)curve->knotv[a];
      a++;
    }
  a = 0; b = 0;
  for(i = 0; i < length; i++)
    {
      x = (GLdouble)curve->controlv[b]; b++;
      y = (GLdouble)curve->controlv[b]; b++;

      b++; /* for z */

      if(curve->is_rat)
	{
	  w = (GLdouble)curve->controlv[b];

	  controls[a] = (GLfloat)(m[0]*x + m[4]*y + m[12]*w);
	  controls[a+1] = (GLfloat)(m[1]*x + m[5]*y + m[13]*w);
	  controls[a+2] = (GLfloat)(w /*m[3]*x + m[7]*y + m[15]*w*/);
	  a += 3;
	}
      else
	{
	  controls[a] = (GLfloat)(m[0]*x + m[4]*y + m[12]);
	  controls[a+1] = (GLfloat)(m[1]*x + m[5]*y + m[13]);
	  a += 2;
	}
      b++;
    } /* for */

  if(curve->order != 2)
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

 return AY_OK;
} /* ay_npt_drawtrimcurve */


/* ay_npt_wribtrimcurves
 *  export trim curves to RIB
 */
int
ay_npt_wribtrimcurves(ay_object *o)
{
 int ay_status = AY_OK;
 int curvecount = 0, k, a, b, c, totalcurves, totalcontrol, totalknots;
 RtInt nloops = 0, *ncurves, *order, *n;
 RtFloat *knot, *min, *max, *u, *v, *w;
 ay_object *trim = NULL, *loop = NULL, *nc = NULL;
 ay_nurbcurve_object *curve = NULL;
 double m[16];
 double x, y, z, w2;

  if(!o)
    return AY_OK;

  if(!(o->type == AY_IDNPATCH))
    return AY_OK;

  /* parse trimcurves */
  /* count loops */
  trim = o->down;

  while(trim->next)
    {
      switch(trim->type)
	{
	case AY_IDNCURVE:
	  nloops++;
	  break;
	case AY_IDLEVEL:
	  /* XXXX this check should be more restrictive! */
	  if(trim->down && trim->down->next)
	    nloops++;
	  break;
	default:
	  nc = NULL;
	  ay_status = ay_provide_object(trim, AY_IDNCURVE, &nc);
	  if(nc)
	    {
	      nloops++;
	      ay_object_delete(nc);
	    }
	  break;
	} /* switch */
      trim = trim->next;
    } /* while */

  if(nloops == 0)
    return AY_OK;

  /* count curves per loop */
  if(!(ncurves = calloc(nloops, sizeof(RtInt))))
    return AY_EOMEM;

  trim = o->down;

  curvecount = 0;
  totalcurves = 0;
  totalcontrol = 0;
  totalknots = 0;
  nloops = 0;
  while(trim->next)
    {
      switch(trim->type)
	{
	case AY_IDNCURVE:
	  totalcurves++;
	  curvecount++;
	  curve = (ay_nurbcurve_object *)(trim->refine);
	  totalcontrol += curve->length;

	  totalknots += curve->length;
	  totalknots += curve->order;

	  ncurves[nloops] = 1;
	  curvecount = 0;
	  nloops++;
	  break;
	case AY_IDLEVEL:
	  loop = trim->down;
	  while(loop->next)
	    {
	      nc = NULL;
	      if(loop->type == AY_IDNCURVE)
		{
		  curve = (ay_nurbcurve_object *)(loop->refine);
		}
	      else
		{
		  ay_status = ay_provide_object(loop, AY_IDNCURVE, &nc);
		  if(nc)
		    {
		      curve = (ay_nurbcurve_object *)(nc->refine);
		    }
		  else
		    {
		      curve = NULL;
		    }
		} /* if */

	      if(curve)
		{
		  totalcurves++;
		  curvecount++;

		  totalcontrol += curve->length;

		  totalknots += curve->length;
		  totalknots += curve->order;

		  ncurves[nloops] += 1;
		  curvecount = 0;
		} /* if */

	      if(nc)
		{
		  ay_object_delete(nc);
		}

	      loop = loop->next;
	    } /* while */

	  /* XXXX this check should be more restrictive! */
	  if(trim->down)
	    nloops++;
	  break;
	default:
	  nc = NULL;
	  ay_status = ay_provide_object(trim, AY_IDNCURVE, &nc);
	  if(nc)
	    {
	      totalcurves++;
	      curvecount++;
	      curve = (ay_nurbcurve_object *)(nc->refine);
	      totalcontrol += curve->length;

	      totalknots += curve->length;
	      totalknots += curve->order;

	      ncurves[nloops] = 1;
	      curvecount = 0;
	      nloops++;
	      ay_object_delete(nc);
	    } /* if */
	  break;
	} /* switch */
      trim = trim->next;
    } /* while */

  if(!(order = calloc(totalcurves,sizeof(RtInt))))
    return AY_EOMEM;
  if(!(n = calloc(totalcurves,sizeof(RtInt))))
    { free(order); return AY_EOMEM; }
  if(!(knot = calloc(totalknots,sizeof(RtFloat))))
    { free(order); free(n); return AY_EOMEM; }
  if(!(min = calloc(totalcurves,sizeof(RtFloat))))
    { free(order); free(n); free(knot); return AY_EOMEM; }
  if(!(max = calloc(totalcurves,sizeof(RtFloat))))
    { free(order); free(n); free(knot); free(min); return AY_EOMEM; }

  if(!(u = calloc(totalcontrol,sizeof(RtFloat))))
    { free(order); free(n); free(knot); free(min); free(max);
    return AY_EOMEM; }
  if(!(v = calloc(totalcontrol,sizeof(RtFloat))))
    { free(order); free(n); free(knot); free(min); free(max); free(u);
    return AY_EOMEM; }
  if(!(w = calloc(totalcontrol,sizeof(RtFloat))))
    { free(order); free(n); free(knot); free(min); free(max); free(u);
    free(v); return AY_EOMEM; }

  trim = o->down;
  a = 0;
  b = 0;
  c = 0;
  /* compile arguments */
  while(trim->next)
    {
      switch(trim->type)
	{
	case AY_IDNCURVE:
	  curve = (ay_nurbcurve_object *)(trim->refine);

	  /* fill order[], n[], min[], max[] */
	  order[a] = (RtInt)curve->order;
	  n[a] = (RtInt)curve->length;
	  min[a] = (RtFloat)((curve->knotv)[curve->order - 1]);
	  max[a] = (RtFloat)((curve->knotv)[curve->length]);
	  a++;

	  /* get curves transformation-matrix */
	  ay_trafo_creatematrix(trim, m);

	  /* copy & revert control (fill u[] v[] w[]) */
	  for(k = 0; k < curve->length ; k++)
	    {
	      x = (RtFloat)((curve->controlv)[k*4]);
	      y = (RtFloat)((curve->controlv)[(k*4)+1]);
	      z = (RtFloat)((curve->controlv)[(k*4)+2]);
	      w2 = (RtFloat)((curve->controlv)[(k*4)+3]);

	      /* apply transformation */
	      u[b] = (RtFloat)(m[0]*x + m[4]*y + m[8]*z + m[12]*w2);
	      v[b] = (RtFloat)(m[1]*x + m[5]*y + m[9]*z + m[13]*w2);
	      w[b] = (RtFloat)w2;

	      b++;
	    } /* for */

	  /* copy knots */
	  for(k = 0; k < curve->length + curve->order; k++)
	    {
	      knot[c] = (RtFloat)curve->knotv[k];
	      c++;
	    }
	  break;
	case AY_IDLEVEL:
	  if(trim->down && trim->down->next)
	    {
	      loop = trim->down;

	      while(loop->next)
		{
		  nc = NULL;

		  if(loop->type == AY_IDNCURVE)
		    {
		      curve = (ay_nurbcurve_object *)(loop->refine);
		    }
		  else
		    {
		      ay_status = ay_provide_object(loop, AY_IDNCURVE, &nc);
		      if(nc)
			{
			  curve = (ay_nurbcurve_object *)(nc->refine);
			}
		      else
			{
			  curve = NULL;
			}
		    } /* if */

		  if(curve)
		    {
		      /* fill order[], n[], min[], max[] */
		      order[a] = (RtInt)curve->order;
		      n[a] = (RtInt)curve->length;
		      min[a] = (RtFloat)((curve->knotv)[curve->order - 1]);
		      max[a] = (RtFloat)((curve->knotv)[curve->length]);
		      a++;

		      /* get curves transformation-matrix */
		      if(nc)
			ay_trafo_creatematrix(nc, m);
		      else
			ay_trafo_creatematrix(loop, m);

		      /* copy & revert control (fill u[] v[] w[]) */
		      for(k = 0; k < curve->length; k++)
			{
			  x = (RtFloat)((curve->controlv)[k*4]);
			  y = (RtFloat)((curve->controlv)[(k*4)+1]);
			  z = (RtFloat)((curve->controlv)[(k*4)+2]);
			  w2 = (RtFloat)((curve->controlv)[(k*4)+3]);

			  /* apply transformation */
			  u[b] = (RtFloat)
			    (m[0]*x + m[4]*y + m[8]*z + m[12]*w2);
			  v[b] = (RtFloat)
			    (m[1]*x + m[5]*y + m[9]*z + m[13]*w2);
			  w[b] = (RtFloat)w2;

			  b++;
			} /* for */

		      /* copy knots */
		      for(k = 0; k < curve->length + curve->order; k++)
			{
			  knot[c] = (RtFloat)curve->knotv[k];
			  c++;
			} /* for */
		    } /* if */

		  if(nc)
		    {
		      ay_object_delete(nc);
		    }

		  loop = loop->next;
		} /* while */
	    } /* if */
	  break;
	default:
	  nc = NULL;
	  ay_status = ay_provide_object(trim, AY_IDNCURVE, &nc);
	  if(nc)
	    {
	      curve = (ay_nurbcurve_object *)(nc->refine);

	      /* fill order[], n[], min[], max[] */
	      order[a] = (RtInt)curve->order;
	      n[a] = (RtInt)curve->length;
	      min[a] = (RtFloat)((curve->knotv)[curve->order - 1]);
	      max[a] = (RtFloat)((curve->knotv)[curve->length]);
	      a++;

	      /* get curves transformation-matrix */
	      ay_trafo_creatematrix(nc, m);

	      /* copy & revert control (fill u[] v[] w[]) */
	      for(k = 0; k < curve->length ; k++)
		{
		  x = (RtFloat)((curve->controlv)[k*4]);
		  y = (RtFloat)((curve->controlv)[(k*4)+1]);
		  z = (RtFloat)((curve->controlv)[(k*4)+2]);
		  w2 = (RtFloat)((curve->controlv)[(k*4)+3]);

		  /* apply transformation */
		  u[b] = (RtFloat)(m[0]*x + m[4]*y + m[8]*z + m[12]*w2);
		  v[b] = (RtFloat)(m[1]*x + m[5]*y + m[9]*z + m[13]*w2);
		  w[b] = (RtFloat)w2;

		  b++;
		} /* for */

	      /* copy knots */
	      for(k = 0; k < curve->length + curve->order; k++)
		{
		  knot[c] = (RtFloat)curve->knotv[k];
		  c++;
		} /* for */
	      ay_object_delete(nc);
	    } /* if */
	  break;
	} /* switch */

      trim = trim->next;
    } /* while */

  if(nloops)
    {
      /* Finally, write the TrimCurves */
      RiTrimCurve(nloops, ncurves, order, knot, min, max, n, u, v, w);

      /* clean up the mess */
      free(ncurves);
      free(order);
      free(knot);
      free(min);
      free(max);
      free(n);
      free(u);
      free(v);
      free(w);
    } /* if */

  return ay_status;
} /* ay_npt_wribtrimcurves */


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

  if(!(controls = calloc(100, sizeof(double))))
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
				 (ay_nurbcurve_object **)(&(newc->refine)));
  if(ay_status)
    {
      free(newc);
      return ay_status;
    }

  /* second, we revolve the half circle around the Y axis */
  ay_status = ay_npt_revolve(newc, 360.0, 0, 0, nsphere);

  ay_object_delete(newc);

 return ay_status;
} /* ay_npt_crtnsphere */


/* ay_npt_crtnspheretcmd:
 *  create a simple NURBS sphere by revolving a half circle
 */
int
ay_npt_crtnspheretcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 int ay_status;
 ay_object *o = NULL;
 char fname[] = "crtNSphere";
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
	  i += 2;
	} /* while */
    } /* if */

  if(!(o = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  o->type = AY_IDNPATCH;
  ay_object_defaults(o);
  o->parent = AY_TRUE;
  o->hide_children = AY_TRUE;

  ay_status = ay_object_crtendlevel(&(o->down));
  if(ay_status)
    {
      free(o);
      ay_error(ay_status, fname, NULL);
      return TCL_OK;
    }

  ay_status = ay_npt_crtnsphere(radius, (ay_nurbpatch_object **)&(o->refine));
  if(ay_status)
    {
      ay_object_delete(o->down);
      free(o);
      ay_error(ay_status, fname, NULL);
      return TCL_OK;
    }

  ay_object_link(o);

 return TCL_OK;
} /* ay_npt_crtnspheretcmd */


/* ay_npt_crtnsphere2tcmd:
 *  create a "Cobb" NURBS sphere
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
 char fname[] = "create_cobbsphere";

  for(i = 0; i < 6; i++)
    {
      if(!(new = calloc(1,sizeof(ay_object))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return TCL_OK;
	}

      new->type = AY_IDNPATCH;
      ay_object_defaults(new);
      new->parent = AY_TRUE;
      new->hide_children = AY_TRUE;
      ay_status = ay_object_crtendlevel(&(new->down));

      ay_status = ay_npt_crtcobbsphere(
			(ay_nurbpatch_object **)&(new->refine));
      if(ay_status)
	{
	  ay_object_delete(new);
	  ay_error(AY_ERROR, fname, NULL);
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
      ay_status = ay_object_link(new);
      if(ay_status)
	{
	  ay_object_delete(new);
	  ay_error(AY_ERROR, fname, NULL);
	  return TCL_OK;
	}

    } /* for */

 return TCL_OK;
} /* ay_npt_crtnsphere2tcmd */


/* ay_npt_splittocurvesu:
 *  split NURBS patch object <o> into curves along u
 */
int
ay_npt_splittocurvesu(ay_object *o, int apply_trafo,
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

  patch = o->refine;
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
      new = NULL;
      if(!(new = calloc(1, sizeof(ay_object))))
	{
	  return AY_EOMEM;
	}

      new->type = AY_IDNCURVE;

      knotv = NULL;
      if(!(knotv = calloc(knots, sizeof(double))))
	{
	  free(new);
	  return AY_EOMEM;
	}
      memcpy(knotv, patch->vknotv, (size_t)(knots*sizeof(double)));

      controlv = NULL;
      if(!(controlv = calloc(stride*dstlen, sizeof(double))))
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
	      ay_trafo_apply4(&(controlv[j]), m);
	      j += stride;
	    } /* for */
	}

      ay_status = ay_nct_create(patch->vorder, dstlen, patch->vknot_type,
				controlv, knotv,
				(ay_nurbcurve_object **)&(new->refine));
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
} /* ay_npt_splittocurvesu */


/* ay_npt_splittocurvesv:
 *  split NURBS patch object <o> into curves along v
 */
int
ay_npt_splittocurvesv(ay_object *o, int apply_trafo,
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

  patch = o->refine;
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
      new = NULL;
      if(!(new = calloc(1, sizeof(ay_object))))
	{
	  return AY_EOMEM;
	}

      new->type = AY_IDNCURVE;

      knotv = NULL;
      if(!(knotv = calloc(knots, sizeof(double))))
	{
	  free(new);
	  return AY_EOMEM;
	}
      memcpy(knotv, patch->uknotv, (size_t)(knots*sizeof(double)));

      controlv = NULL;
      if(!(controlv = calloc(stride*dstlen, sizeof(double))))
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
	  k = 0;
	  for(i = 0; i < dstlen; i++)
	    {
	      ay_trafo_apply4(&(controlv[k]), m);
	      k += stride;
	    } /* for */
	}

      ay_status = ay_nct_create(patch->uorder, dstlen, patch->uknot_type,
				controlv, knotv,
				(ay_nurbcurve_object **)&(new->refine));
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
} /* ay_npt_splittocurvesv */


/* ay_npt_splittocurvestcmd:
 *  Tcl interface for splittocurvesu() and splittocurvesv() above
 *
 */
int
ay_npt_splittocurvestcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL, *curves = NULL, *next = NULL;
 int u = 0;
 char fname[] = "split_to_curves";

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  /* parse args */
  if(argc != 2)
    {
      ay_error(AY_EARGS, fname, "u|v");
      return TCL_OK;
    }

  if(!strcmp(argv[1], "u"))
    u = 1;

  while(sel)
    {
      curves = NULL;
      src = sel->object;
      if(src->type != AY_IDNPATCH)
	{
	  ay_error(AY_EWTYPE, fname, ay_npt_npname);
	  break;
	}

      if(u)
	{
	  ay_status = ay_npt_splittocurvesu(src, AY_TRUE, &curves, NULL);
	}
      else
	{
   	  ay_status = ay_npt_splittocurvesv(src, AY_TRUE, &curves, NULL);
	} /* if */

      if(ay_status || !curves)
	{
	  ay_error(AY_ERROR, fname, "Split failed");
	}

      while(curves)
	{
	  next = curves->next;
	  ay_trafo_copy(src, curves);
	  ay_status = ay_object_link(curves);
	  curves = next;
	}

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_npt_splittocurvestcmd */


/* ay_npt_buildfromcurves:
 *  build a new patch from a number of (compatible) curves
 *
 */
int
ay_npt_buildfromcurves(ay_list_object *curves, int ncurves, int type,
		       int knot_type, int apply_trafo,
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
  if(newwidth < 4)
    newuorder = newwidth;
  else
    newuorder = 4;
  newuknot_type = knot_type;

  if(type == AY_CTCLOSED)
    newwidth++;

  if(type == AY_CTPERIODIC)
    newwidth += newuorder-1;

  newheight = nc->length;
  newvorder = nc->order;
  newvknots = newheight + newvorder;
  newvknot_type = nc->knot_type;
  if(!(newvknotv = calloc(newvknots, sizeof(double))))
    {
      return AY_EOMEM;
    }

  memcpy(newvknotv, nc->knotv, (size_t)(newvknots*sizeof(double)));

  /* create new patch */
  if(!(new = calloc(1, sizeof(ay_object))))
    {
      free(newvknotv);
      return AY_EOMEM;
    }

  new->type = AY_IDNPATCH;
  ay_object_defaults(new);
  new->parent = AY_TRUE;
  new->hide_children = AY_TRUE;
  ay_status = ay_object_crtendlevel(&(new->down));

  if(!(newcontrolv = calloc(newwidth*newheight*4, sizeof(double))))
    {
      free(newvknotv); free(new);
      return AY_EOMEM;
    }

  /* fill newcontrolv */
  curve = curves;
  while(curve)
    {
      c = curve->object;
      nc = (ay_nurbcurve_object*)c->refine;

      memcpy(&(newcontrolv[i]), nc->controlv, newheight*stride*sizeof(double));

      if(apply_trafo && AY_ISTRAFO(c))
	{
	  /* get curves transformation-matrix */
	  ay_trafo_creatematrix(c, m);
	  /* apply curves transformation-matrix */
	  for(j = 0; j < newheight; j++)
	    {
	      ay_trafo_apply4(&(newcontrolv[i]), m);
	      i += stride;
	    } /* for */
	}
      else
	{
	  i += newheight*stride;
	}

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
			    (ay_nurbpatch_object **)&(new->refine));

  if(ay_status)
    {
      free(new); free(newvknotv); free(newcontrolv);
      return ay_status;
    }

  ay_status = ay_npt_recreatemp((ay_nurbpatch_object *)new->refine);

  if(ay_status)
    {
      ay_object_delete(new);
      return ay_status;
    }

  /* return result */
  *patch = new;

 return ay_status;
} /* ay_npt_buildfromcurves */



/* ay_npt_buildfromcurvestcmd:
 *  Tcl interface for ay_npt_buildfromcurves() above
 *
 */
int
ay_npt_buildfromcurvestcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = NULL, *curves = NULL, *new = NULL, **next = NULL;
 ay_object *o = NULL, *patch = NULL;
 ay_nurbcurve_object *nc = NULL;
 int length = 0, ncurves = 0;
 char fname[] = "build_from_curves";

  sel = ay_selection;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  /* parse args */

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
		  /* XXXX fix leak! */
		  return TCL_OK;
		}

	      new->object = o;

	      if(next)
		*next = new;
	      else
		curves = new;

	      next = &(new->next);

	      ncurves++;
	    }
	}
      sel = sel->next;
    } /* while */

  ay_status = ay_npt_buildfromcurves(curves, ncurves, AY_CTOPEN,
				     AY_KTNURB, AY_TRUE, &patch);

  if(ay_status || !patch)
    {
      ay_error(AY_ERROR, fname, "Build failed");
    }

  if(patch)
    ay_object_link(patch);

  /* free list */
  while(curves)
    {
      new = curves->next;
      free(curves);
      curves = new;
    }

 return TCL_OK;
} /* ay_npt_buildfromcurvestcmd */


/* ay_npt_concat:
 *  concatenate the patches in <o> by splitting them to
 *  curves, making the curves compatible, and building a
 *  new patch from the compatible curves
 *  returns resulting patch object in <result>
 */
int
ay_npt_concat(ay_object *o, int type, int knot_type, int fillet_type,
	      ay_object **result)
{
 int ay_status = AY_OK;
 ay_object *patches = NULL, *new = NULL, *tmp = NULL;
 ay_object *curve = NULL, *allcurves = NULL, **nextcurve = NULL;
 ay_list_object *curvelist, **nextlist = NULL, *rem;
 ay_nurbpatch_object *np = NULL;
 int i = 0, ncurves = 0;

  if(!o || !result)
    return AY_ENULL;

  patches = o;
  nextcurve = &allcurves;

  while(o)
    {
      ay_npt_splittocurvesu(o, AY_TRUE, nextcurve, &nextcurve);
      o = o->next;
    }

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
    }

  ay_status = ay_nct_makecompatible(allcurves);

  if(ay_status)
    goto cleanup;


  /* create fillets (or remove double boundary curves) */
  o = patches;
  if(fillet_type != 0 && o->next)
    {
      curve = allcurves;
      np = (ay_nurbpatch_object *)o->refine;
      for(i = 0; i < np->width; i++)
	{
	  curve = curve->next;
	}
      o = o->next;
      while(o)
	{
	  tmp = curve->next;
	  curve->next = tmp->next;
	  ay_object_delete(tmp);

	  np = (ay_nurbpatch_object *)o->refine;
	  for(i = 0; i < (np->width-1); i++)
	    {
	      curve = curve->next;
	    }

	  o = o->next;
	}
    }


  /* build a new patch from the compatible curves */
  ay_status = ay_npt_buildfromcurves(curvelist, ncurves, type, knot_type,
				     AY_FALSE, &new);

  if(ay_status)
    goto cleanup;

  /* return result */
  *result = new;

cleanup:

  ay_object_deletemulti(allcurves);

  /* XXXX delete list */

  while(curvelist)
    {
      rem = curvelist;
      curvelist = rem->next;
      free(rem);
    }

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
 ay_nurbcurve_object *curve, *tmpnc = NULL;
 double *uknotv = NULL, *tcontrolv = NULL;
 double radius = 0.0, w = 0.0, x, y/*, z*/;
 int i = 0, j = 0, a = 0, b = 0, c = 0;
 double m[16], point[4] = {0};

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

  if(!(uknotv = calloc(curve->length+curve->order, sizeof(double))))
    {
      free(new); return AY_EOMEM;
    }

  memcpy(uknotv, curve->knotv, (curve->length+curve->order)*sizeof(double));
  new->uknotv = uknotv;
  new->uorder = curve->order;
  new->vorder = order;
  new->uknot_type = curve->knot_type;
  new->vknot_type = AY_KTCUSTOM;
  new->width = curve->length;

  /* fill controlv */
  a = 0;
  for(j = 0; j < curve->length; j++)
    {
      /* transform point */
      x = curve->controlv[a];
      y = curve->controlv[a+1];
      /*z = curve->controlv[a+2];*/
      w = curve->controlv[a+3];

      point[0] = m[0]*x + m[4]*y + m[8]*point[2] + m[12]*w;
      point[1] = m[1]*x + m[5]*y + m[9]*point[2] + m[13]*w;
      point[3] = m[3]*x + m[7]*y + m[11]*point[2] + m[15]*w;

      /* project point onto XY-Plane! */
      point[2] = 0.0; /* XXXX loss of data! */

      radius = point[0];

      if(tcontrolv)
	free(tcontrolv);
      tcontrolv = NULL;
      if(new->vknotv)
	free(new->vknotv);
      new->vknotv = NULL;
      if(sections == 0)
	{
	  if(arc > 0.0)
	    {
	      ay_status = ay_nb_CreateNurbsCircleArc(radius, 0.0, arc,
						  &(new->height), &new->vknotv,
						  &tcontrolv);
	    }
	  else
	    {
	      ay_status = ay_nb_CreateNurbsCircleArc(radius, arc, 0.0,
						  &(new->height), &new->vknotv,
						  &tcontrolv);
	    } /* if */
	  new->is_rat = AY_TRUE;
	}
      else
	{
	  if(arc == 360.0)
	    {
	      tmpnc = NULL;
	      ay_status = ay_nct_crtcircbsp(sections, radius, 360.0, order,
					    &tmpnc);
	      if(!tmpnc)
		{
		  if(new->uknotv)
		    free(new->uknotv);
		  if(new->vknotv)
		    free(new->vknotv);
		  if(new->controlv)
		    free(new->controlv);
		  free(new);
		  return AY_ERROR;
		} /* if */

	      tcontrolv = tmpnc->controlv;
	      new->vknotv = tmpnc->knotv;
	      new->height = tmpnc->length;
	      free(tmpnc);
	    }
	  else
	    {
	      /* unsupported case, just bail out */
	      if(new->uknotv)
		free(new->uknotv);
	      free(new);
	      return AY_ERROR;
	    } /* if */
	} /* if */

      if(!new->controlv)
	{
	  if(!(new->controlv = calloc(new->height*new->width*4,
				      sizeof(double))))
	    return AY_EOMEM;
	}

      /* copy to real controlv */
      b = 0;
      c = j*new->height*4;
      for(i = 0; i < new->height; i++)
	{
	  new->controlv[c]   = tcontrolv[b];
	  new->controlv[c+1] = point[1]*tcontrolv[b+3];
	  new->controlv[c+2] = tcontrolv[b+1];
	  new->controlv[c+3] = tcontrolv[b+3]*w;
	  b += 4;
	  c += 4;
	} /* for */

	a += 4;
    } /* for */

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

  /* check parameters */
  if(!o1 || !o2 || !swing)
    return AY_ENULL;

  if((o1->type != AY_IDNCURVE) || (o2->type != AY_IDNCURVE))
    return AY_OK;

  cs = (ay_nurbcurve_object *)(o1->refine);
  tr = (ay_nurbcurve_object *)(o2->refine);

  /* apply trafos to cross-section curves controlv */
  if(!(cscv = calloc(cs->length * stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(cscv, cs->controlv, cs->length * stride * sizeof(double));

  ay_trafo_creatematrix(o1, m);

  a = 0;
  for(i = 0; i < cs->length; i++)
    {
      ay_trafo_apply4(&(cscv[a]), m);
      a += stride;
    }

  /* apply trafos to trajectory curves controlv */
  if(!(trcv = calloc(tr->length * stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(trcv, tr->controlv, tr->length * stride * sizeof(double));

  ay_trafo_creatematrix(o2, m);
  a = 0;
  for(i = 0; i < tr->length; i++)
    {
      ay_trafo_apply4(&(trcv[a]), m);
      a += stride;
    }

  /* calloc and parametrize the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(new->uknotv = calloc(tr->length+tr->order, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  memcpy(new->uknotv, tr->knotv, (tr->length+tr->order)*sizeof(double));
  new->uorder = tr->order;
  new->uknot_type = tr->knot_type;
  new->width = tr->length;

  if(!(new->vknotv = calloc(cs->length+cs->order, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  memcpy(new->vknotv, cs->knotv, (cs->length+cs->order)*sizeof(double));
  new->vorder = cs->order;
  new->vknot_type = cs->knot_type;
  new->height = cs->length;

  if(!(new->controlv = calloc(cs->length*tr->length, stride*sizeof(double))))
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
	  ay_trafo_apply4(p, m);
	  if(fabs(1.0-trcv[j*stride+3]) > AY_EPSILON)
	    {
	      p[1] *= trcv[j*stride+3];
	      p[3] *= trcv[j*stride+3];
	    }
	  p += 4;
	} /* for */
    } /* for */

  new->is_rat = ay_npt_israt(new);

  /* return result */
  *swing = new;
  new = NULL;

cleanup:
  if(cscv)
    free(cscv);

  if(trcv)
    free(trcv);

  if(new)
    {
      if(new->uknotv)
	free(new->uknotv);
      if(new->vknotv)
	free(new->vknotv);
      if(new->controlv)
	free(new->controlv);
      free(new);
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
	     int rotate, int closed, ay_nurbpatch_object **sweep,
	     int has_start_cap, ay_object **start_cap,
	     int has_end_cap, ay_object **end_cap)
{
 int ay_status = AY_OK;
 ay_object *curve = NULL;
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
 double mr[16];
 double quat[4] = {0};
 double *cscv = NULL, *trcv = NULL, *sfcv = NULL, *rots = NULL;

  if(!o1 || !o2 || !sweep)
    return AY_ENULL;

  if((o1->type != AY_IDNCURVE) || (o2->type != AY_IDNCURVE) ||
     (o3 && (o3->type != AY_IDNCURVE)))
    return AY_OK;

  cs = (ay_nurbcurve_object *)(o1->refine);
  tr = (ay_nurbcurve_object *)(o2->refine);

  stride = 4;

  /* apply scale and rotation to cross-section curves controlv */
  if(!(cscv = calloc(cs->length * stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(cscv, cs->controlv, cs->length * stride * sizeof(double));

  ay_quat_torotmatrix(o1->quat, mcs);
  ay_trafo_scalematrix(o1->scalx, o1->scaly, o1->scalz, mcs);

  a = 0;
  for(i = 0; i < cs->length; i++)
    {
      ay_trafo_apply4(&(cscv[a]), mcs);
      a += stride;
    }

  /* apply all transformations to trajectory curves controlv */
  if(!(trcv = calloc(tr->length * stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(trcv, tr->controlv, tr->length * stride * sizeof(double));

  ay_trafo_creatematrix(o2, mtr);
  a = 0;
  for(i = 0; i < tr->length; i++)
    {
      ay_trafo_apply4(&(trcv[a]), mtr);
      a += stride;
    }

  /* apply all transformations to scaling curves controlv */
  if(o3)
    {
      sf = (ay_nurbcurve_object *)(o3->refine);

      if(!(sfcv = calloc(sf->length * stride, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      memcpy(sfcv, sf->controlv, sf->length * stride * sizeof(double));

      ay_trafo_creatematrix(o3, mtr);
      a = 0;
      for(i = 0; i < sf->length; i++)
	{
	  ay_trafo_apply4(&(sfcv[a]), mtr);
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
  if(!(controlv = calloc(new->width*new->height*stride, sizeof(double))))
    { free(new); ay_status = AY_EOMEM; goto cleanup; }
  new->controlv = controlv;
  if(!(new->vknotv = calloc(new->height + new->vorder, sizeof(double))))
    { free(new); free(controlv); ay_status = AY_EOMEM; goto cleanup; }
  if(!(new->uknotv = calloc(new->width + new->uorder, sizeof(double))))
    {
      free(new->vknotv); free(new); free(controlv);
      ay_status = AY_EOMEM; goto cleanup;
    }

  ay_status = ay_knots_createnp(new);
  if(ay_status)
    {
      free(new->uknotv); free(new->vknotv); free(new); free(controlv);
      ay_status = AY_EOMEM; goto cleanup;
    }

  if(cs->knot_type == AY_KTCUSTOM)
    {
      memcpy(new->vknotv,cs->knotv,(cs->length+cs->order)*sizeof(double));
    }

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

  if(!(rots = calloc((sections+1)*4, sizeof(double))))
    {
      free(new->uknotv); free(new->vknotv); free(new); free(controlv);
      ay_status = AY_EOMEM; goto cleanup;
    }

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
	  AY_V3SCAL(T1,(1.0/len))

	  if(((fabs(fabs(T1[0])-fabs(T0[0])) > AY_EPSILON) ||
	      (fabs(fabs(T1[1])-fabs(T0[1])) > AY_EPSILON) ||
	      (fabs(fabs(T1[2])-fabs(T0[2])) > AY_EPSILON)) ||
	     (T1[0] * T0[0] < 0) ||
	     (T1[1] * T0[1] < 0) ||
	     (T1[2] * T0[2] < 0))
	    {
	      AY_V3CROSS(A,T0,T1);
	      len = AY_V3LEN(A);
	      AY_V3SCAL(A,(1.0/len));

	      rots[i*4+0] = AY_R2D(acos(AY_V3DOT(T0,T1)));
	      memcpy(&(rots[i*4+1]), A, 3*sizeof(double));

	      if(fabs(rots[i*4]) > AY_EPSILON)
		{
		  ay_trafo_rotatematrix(-rots[i*4], rots[i*4+1],
					rots[i*4+2], rots[i*4+3], mr);
		}
	    } /* if */

	  memcpy(T0, T1, 3*sizeof(double));

	  ay_trafo_multmatrix4(m, mr);
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

      ay_trafo_invmatrix4(m, mi);

      /* sweep profile */
      for(j = 0; j < cs->length; j++)
	{
	  ay_trafo_apply4(&(controlv[i*cs->length*stride+j*stride]), mi);
	} /* for */


      /* create caps (if sweep is not closed) */
      if(i == 0)
	{
	  if(has_start_cap && !closed)
	    {
	      curve = NULL;
	      ay_status = ay_object_copy(o1, &curve);
	      ay_trafo_defaults(curve);
	      ay_status = ay_capt_createfromcurve(curve, start_cap);
	      if(*start_cap)
		{
		  /* transform cap */
		  /* move it */
		  ay_nb_CurvePoint4D(tr->length-1,tr->order-1,tr->knotv,
				     trcv, tr->knotv[tr->order-1], p2);
		  ay_trafo_copy(o1, *start_cap);
		  (*start_cap)->movx = p2[0];
		  (*start_cap)->movy = p2[1];
		  (*start_cap)->movz = p2[2];
		  /* apply scaling function (if present) */
		  if(o3)
		    {
		      u = sf->knotv[sf->order-1];
		      ay_nb_CurvePoint4D(sf->length-1, sf->order-1, sf->knotv,
					 sfcv, u, p3);
		      p3[1] = fabs(p3[1]);
		      p3[2] = fabs(p3[2]);
		      if(p3[1] > AY_EPSILON)
			{
			  (*start_cap)->scaly *= p3[1];
			}
		      if(sfis3d)
			{
			  if(p3[2] > AY_EPSILON)
			    {
			      (*start_cap)->scalx *= p3[2];
			    }
			}
		      else
			{
			  if(p3[1] > AY_EPSILON)
			    {
			      (*start_cap)->scalx *= p3[1];
			    }
			}
		    } /* if */
		  /* fix direction for aycsg */
		  (*start_cap)->scalz *= -1.0;
		  /* rotate it */
		  if(rotate)
		    {
		      if(fabs(rots[0]) > AY_EPSILON)
			{
			  ay_quat_axistoquat(&(rots[1]), AY_D2R(-rots[0]),
					     quat);
			  ay_quat_add(quat, (*start_cap)->quat,
				      (*start_cap)->quat);
			} /* if */
		    } /* if */
		}
	      else
		{
		  ay_object_delete(curve);
		} /* if */
	    } /* if */
	} /* if */
      if(i == sections)
	{
	  if(has_end_cap && !closed)
	    {
	      curve = NULL;
	      ay_status = ay_object_copy(o1, &curve);
	      ay_trafo_defaults(curve);
	      ay_status = ay_capt_createfromcurve(curve, end_cap);
	      if(*end_cap)
		{
		  /* transform cap */
		  /* move it */
		  ay_nb_CurvePoint4D(tr->length-1, tr->order-1, tr->knotv,
				     trcv, tr->knotv[tr->length], p2);
		  ay_trafo_copy(o1, *end_cap);
		  (*end_cap)->movx = p2[0];
		  (*end_cap)->movy = p2[1];
		  (*end_cap)->movz = p2[2];
		  /* apply scaling function (if present) */
		  if(o3)
		    {
		      u = sf->knotv[sf->length];
		      ay_nb_CurvePoint4D(sf->length-1, sf->order-1, sf->knotv,
					 sfcv, u, p3);
		      p3[1] = fabs(p3[1]);
		      p3[2] = fabs(p3[2]);
		      if(p3[1] > AY_EPSILON)
			{
			  (*end_cap)->scaly *= p3[1];
			}
		      if(sfis3d)
			{
			  if(p3[2] > AY_EPSILON)
			    {
			      (*end_cap)->scalx *= p3[2];
			    }
			}
		      else
			{
			  if(p3[1] > AY_EPSILON)
			    {
			      (*end_cap)->scalx *= p3[1];
			    }
			}
		    } /* if */
		  /* rotate it */
		  if(rotate)
		    {
		      for(j = 0; j <= sections; j++)
			{
			  if(fabs(rots[j*4]) > AY_EPSILON)
			    {
			      ay_quat_axistoquat(&(rots[j*4+1]),
						 AY_D2R(-rots[j*4]), quat);
			      ay_quat_add(quat, (*end_cap)->quat,
					  (*end_cap)->quat);
			    } /* if */
			} /* for */
		    } /* if */
		}
	      else
		{
		  ay_object_delete(curve);
		} /* if */
	    } /* if */
	} /* if */
    } /* for i = 0; i <= sections; i++ */

  new->is_rat = ay_npt_israt(new);

  /* return result */
  *sweep = new;

  /* clean up */
 cleanup:
  if(rots)
    free(rots);
  if(cscv)
    free(cscv);
  if(trcv)
    free(trcv);
  if(sfcv)
    free(sfcv);

 return ay_status;
} /* ay_npt_sweep */


/* ay_npt_closedsweep:
 *  Sweep cross section <o1> along path <o2> possibly rotating it,
 *  so that it is always perpendicular to the path, possibly
 *  scaling it by a factor derived from the difference of the
 *  y and z coordinates of scaling curve <o3> to the y and z values 1.0.
 *  In contrast to ay_npt_sweep() above, this function creates a
 *  periodic patch (in the direction of the trajectory) and should
 *  probably rather be called ay_npt_periodicsweep().
 *  Rotation code derived from J. Bloomenthals "Reference Frames"
 *  (Graphic Gems I).
 */
int
ay_npt_closedsweep(ay_object *o1, ay_object *o2, ay_object *o3, int sections,
		   int rotate, ay_nurbpatch_object **closedsweep)
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
 double mr[16];
 double *cscv = NULL, *trcv = NULL, *sfcv = NULL, *rots = NULL;

  if(!o1 || !o2 || !closedsweep)
    return AY_ENULL;

  if((o1->type != AY_IDNCURVE) || (o2->type != AY_IDNCURVE) ||
     (o3 && (o3->type != AY_IDNCURVE)))
    return AY_ERROR;

  if(sections == 1)
    return AY_ERROR;

  cs = (ay_nurbcurve_object *)(o1->refine);
  tr = (ay_nurbcurve_object *)(o2->refine);

  /* apply scale and rotation to cross-section curves controlv */
  if(!(cscv = calloc(cs->length * stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(cscv, cs->controlv, cs->length * stride * sizeof(double));

  ay_quat_torotmatrix(o1->quat, mcs);
  ay_trafo_scalematrix(o1->scalx, o1->scaly, o1->scalz, mcs);

  a = 0;
  for(i = 0; i < cs->length; i++)
    {
      ay_trafo_apply4(&(cscv[a]), mcs);
      a += stride;
    }

  /* apply all transformations to trajectory curves controlv */
  if(!(trcv = calloc(tr->length * stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(trcv, tr->controlv, tr->length * stride * sizeof(double));

  ay_trafo_creatematrix(o2, mtr);
  a = 0;
  for(i = 0; i < tr->length; i++)
    {
      ay_trafo_apply4(&(trcv[a]), mtr);
      a += stride;
    }

  /* apply all transformations to scaling curves controlv */
  if(o3)
    {
      sf = (ay_nurbcurve_object *)(o3->refine);

      if(!(sfcv = calloc(sf->length * stride, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      memcpy(sfcv, sf->controlv, sf->length * stride * sizeof(double));

      ay_trafo_creatematrix(o3, mtr);
      a = 0;
      for(i = 0; i < sf->length; i++)
	{
	  ay_trafo_apply4(&(sfcv[a]), mtr);
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
  if(!(controlv = calloc(cs->length*new->width*stride, sizeof(double))))
    { free(new); ay_status = AY_EOMEM; goto cleanup; }
  new->controlv = controlv;
  if(!(new->vknotv = calloc(cs->length+cs->order, sizeof(double))))
    {
      free(new); free(controlv);
      ay_status = AY_EOMEM; goto cleanup;
    }
  if(!(new->uknotv = calloc(sections+4, sizeof(double))))
    {
      free(new->vknotv); free(controlv); free(new);
      ay_status = AY_EOMEM; goto cleanup;
    }

  ay_status = ay_knots_createnp(new);
  if(ay_status)
    {
      free(new->uknotv); free(new->vknotv); free(new); free(controlv);
      ay_status = AY_EOMEM; goto cleanup;
    }

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

  if(!(rots = calloc(new->width*4, sizeof(double))))
    {
      free(new->uknotv); free(new->vknotv); free(new); free(controlv);
      ay_status = AY_EOMEM; goto cleanup;
    }

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
	}

      /* now, apply rotation (if requested) */
      u = tr->knotv[tr->order-1]+(((double)i/sections)*plen);

      if(rotate)
	{
	  ay_nb_ComputeFirstDer4D(tr->length-1, tr->order-1, tr->knotv,
				  trcv, u, T1);

	  len = AY_V3LEN(T1);
	  AY_V3SCAL(T1,(1.0/len))

	  if(((fabs(fabs(T1[0])-fabs(T0[0])) > AY_EPSILON) ||
	      (fabs(fabs(T1[1])-fabs(T0[1])) > AY_EPSILON) ||
	      (fabs(fabs(T1[2])-fabs(T0[2])) > AY_EPSILON)) ||
	     (T1[0] * T0[0] < 0) ||
	     (T1[1] * T0[1] < 0) ||
	     (T1[2] * T0[2] < 0))
	    {
	      AY_V3CROSS(A,T0,T1);
	      len = AY_V3LEN(A);
	      AY_V3SCAL(A,(1.0/len));

	      rots[i*4+0] = AY_R2D(acos(AY_V3DOT(T0,T1)));
	      memcpy(&(rots[i*4+1]), A, 3*sizeof(double));

	      if(fabs(rots[i*4]) > AY_EPSILON)
		{
		  ay_trafo_rotatematrix(-rots[i*4], rots[i*4+1],
					rots[i*4+2], rots[i*4+3], mr);
		}
	    } /* if */

	  memcpy(T0, T1, 3*sizeof(double));

	  ay_trafo_multmatrix4(m, mr);
	} /* if rotate */

      /* now, add translation to current point on trajectory */
      ay_nb_CurvePoint4D(tr->length-1, tr->order-1, tr->knotv,
			 trcv, u, p2);

      ay_trafo_translatematrix(-p2[0], -p2[1], -p2[2], m);

      ay_trafo_invmatrix4(m, mi);

      /* sweep profile */
      for(j = 0; j < cs->length; j++)
	{
	  ay_trafo_apply4(&(controlv[i*cs->length*stride+j*stride]), mi);
	} /* for */

    } /* for */

  new->is_rat = ay_npt_israt(new);

  ay_status = ay_npt_closeu(new);

  /* return result */
  *closedsweep = new;

  /* clean up */
 cleanup:
  if(rots)
    free(rots);
  if(cscv)
    free(cscv);
  if(trcv)
    free(trcv);
  if(sfcv)
    free(sfcv);

 return ay_status;
} /* ay_npt_closedsweep */


/* ay_npt_birail1:
 *  sweep cross section <o1> along rails <o2> and <o3>;
 *  Rotation code derived from J. Bloomenthals "Reference Frames"
 *  (Graphic Gems I).
 */
int
ay_npt_birail1(ay_object *o1, ay_object *o2, ay_object *o3, int sections,
	       int closed, ay_nurbpatch_object **birail1,
	       int has_start_cap, ay_object **start_cap,
	       int has_end_cap, ay_object **end_cap)
{
 int ay_status = AY_OK;
 ay_object *curve = NULL;
 ay_nurbpatch_object *new = NULL;
 ay_nurbcurve_object *cs, *r1, *r2, *tc;
 double *controlv = NULL;
 int i = 0, j = 0, a = 0, stride;
 double u, p1[4], p2[4], p5[4], p6[4], p7[4], p8[4];
 double T0[3] = {0.0,0.0,-1.0};
 double T1[3] = {0.0,0.0,0.0};
 double A[3] = {0.0,0.0,0.0};
 double lent0 = 0.0, lent1 = 0.0, plenr1 = 0.0, plenr2 = 0.0;
 double m[16] = {0}, mi[16] = {0}, mcs[16], mr1[16], mr2[16];
 double mr[16], mrs[16];
 /*double quat[4] = {0};*/
 double *cscv = NULL, *r1cv = NULL, *r2cv = NULL, *rots = NULL;
 double scalx, scaly, scalz;

  if(!o1 || !o2 || !o3 || !birail1 ||
     (has_start_cap && !start_cap) || (has_end_cap && !end_cap))
    return AY_ENULL;

  if((o1->type != AY_IDNCURVE) || (o2->type != AY_IDNCURVE) ||
     (o3->type != AY_IDNCURVE))
    return AY_OK;

  cs = (ay_nurbcurve_object *)(o1->refine);
  r1 = (ay_nurbcurve_object *)(o2->refine);
  r2 = (ay_nurbcurve_object *)(o3->refine);

  stride = 4;

  /* apply all transformations to cross-section curves controlv */
  if(!(cscv = calloc(cs->length * stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(cscv, cs->controlv, cs->length * stride * sizeof(double));

  ay_trafo_creatematrix(o1, mcs);
  a = 0;
  for(i = 0; i < cs->length; i++)
    {
      ay_trafo_apply4(&(cscv[a]), mcs);
      a += stride;
    }

  /* apply all transformations to rail1 curves controlv */
  if(!(r1cv = calloc(r1->length * stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(r1cv, r1->controlv, r1->length * stride * sizeof(double));

  ay_trafo_creatematrix(o2, mr1);
  a = 0;
  for(i = 0; i < r1->length; i++)
    {
      ay_trafo_apply4(&(r1cv[a]), mr1);
      a += stride;
    }

  /* apply all transformations to rail2 curves controlv */
  if(!(r2cv = calloc(r2->length * stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(r2cv, r2->controlv, r2->length * stride * sizeof(double));

  ay_trafo_creatematrix(o3, mr2);
  a = 0;
  for(i = 0; i < r2->length; i++)
    {
      ay_trafo_apply4(&(r2cv[a]), mr2);
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
  if(!(controlv = calloc(new->width*new->height*stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  new->controlv = controlv;
  if(!(new->uknotv = calloc(new->width+new->uorder, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(new->vknotv = calloc(new->height+new->vorder, sizeof(double))))
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

  if(!(rots = calloc((sections+1)*4, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

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

      AY_V3SUB(T1, p2, p1)
      lent1 = AY_V3LEN(T1);
      AY_V3SCAL(T1,(1.0/lent1))

      /* create rotation matrix */
      if(((fabs(fabs(T1[0])-fabs(T0[0])) > AY_EPSILON) ||
	  (fabs(fabs(T1[1])-fabs(T0[1])) > AY_EPSILON) ||
	  (fabs(fabs(T1[2])-fabs(T0[2])) > AY_EPSILON)))
	{
	  ay_trafo_translatematrix(((p5[0])),
				   ((p5[1])),
				   ((p5[2])),
				   mr);
	  AY_V3CROSS(A,T0,T1);
	  lent0 = AY_V3LEN(A);
	  AY_V3SCAL(A,(1.0/lent0));

	  rots[i*4+0] = AY_R2D(acos(AY_V3DOT(T0,T1)));
	  memcpy(&rots[i*4+1], A, 3*sizeof(double));

	  if(fabs(rots[i*4]) > AY_EPSILON)
	    {
	      ay_trafo_rotatematrix(-rots[i*4], rots[i*4+1],
				    rots[i*4+2], rots[i*4+3], mr);
	      /*
		the mrs matrix is used to properly rotate the cross
		section curve for the calculation of the scale factors
	      */
	      ay_trafo_rotatematrix(-rots[i*4], rots[i*4+1],
				    rots[i*4+2], rots[i*4+3], mrs);
	      /*
	      printf("i: %d, angle %g, axis %g %g %g\n",i,
		     rots[i*4], rots[i*4+1],
		     rots[i*4+2], rots[i*4+3]);
	      */
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
      ay_trafo_multmatrix4(m, mr);

      /* add translation */
      ay_trafo_translatematrix((p5[0]-p1[0]),
			       (p5[1]-p1[1]),
			       (p5[2]-p1[2]),
			       m);

      /* invert matrix */
      ay_trafo_invmatrix4(m, mi);

      /* sweep profile */
      for(j = 0; j < cs->length; j++)
	{
	  ay_trafo_apply4(&controlv[i*cs->length*stride+j*stride], mi);
	} /* for */

      /* create end-cap (if birail is not closed) */
      if(i == sections)
	{

	  if(has_end_cap && !closed)
	    {
	      curve = NULL;
	      ay_status = ay_object_copy(o1, &curve);
	      tc = (ay_nurbcurve_object*)curve->refine;
	      ay_trafo_creatematrix(curve, mcs);
	      ay_trafo_defaults(curve);

	      for(j = 0; j < tc->length; j++)
	        {
		  ay_trafo_apply4(&(tc->controlv[j*stride]), mcs);
		  ay_trafo_apply4(&(tc->controlv[j*stride]), mi);
	        } /* for */

	      ay_status = ay_capt_createfromcurve(curve, end_cap);

	      if(!*end_cap)
		{
		  ay_object_delete(curve);
		}

	    } /* if */

#if 0
	  if(has_end_cap && !closed)
	    {
	      curve = NULL;
	      ay_status = ay_object_copy(o1, &curve);
	      tc = (ay_nurbcurve_object*)curve->refine;
	      /*ay_trafo_defaults(curve);*/
	      curve->scalx *= scalx;
	      curve->scaly *= scaly;
	      curve->scalz *= scalz;
	      ay_status = ay_capt_createfromcurve(curve, end_cap);

	      if(*end_cap)
		{
		  /*ay_trafo_copy(o1, *end_cap);*/

		  (*end_cap)->movx += p1[0]+((p2[0]-p1[0])/2.0);
		  (*end_cap)->movy += p1[1]+((p2[1]-p1[1])/2.0);
		  (*end_cap)->movz += p1[2]+((p2[2]-p1[2])/2.0);

		  /* rotate it */
		  for(j = 1; j <= sections; j++)
		    {
		      if(fabs(rots[j*4]) > AY_EPSILON)
			{
			  ay_quat_axistoquat(&(rots[j*4+1]),
					     AY_D2R(-rots[j*4]), quat);
			  ay_quat_add(quat, (*end_cap)->quat,
				      (*end_cap)->quat);
			} /* if */
		    } /* for */
		} /* if */
	    } /* if */
#endif
	} /* if */

      /* save rail vector for next iteration */
      memcpy(T0, T1, 3*sizeof(double));
      lent0 = lent1;
    } /* for */

  /* create start cap (if birail is not closed) */
  if(has_start_cap && !closed)
    {
      curve = NULL;
      ay_status = ay_object_copy(o1, &curve);
      /*ay_trafo_defaults(curve);*/
      ay_status = ay_capt_createfromcurve(curve, start_cap);
      /* transform cap */
      if(*start_cap)
	{
	    /*ay_trafo_copy(o1, *start_cap);*/
	  /* fix direction for aycsg */
	  (*start_cap)->scalz *= -1.0;
	}
      else
	{
	  ay_object_delete(curve);
	} /* if */
    } /* if */

  /* return result */
  *birail1 = new;

  new = NULL;
  controlv = NULL;

  /* clean up */
cleanup:
  if(rots)
    free(rots);
  if(cscv)
    free(cscv);
  if(r1cv)
    free(r1cv);
  if(r2cv)
    free(r2cv);
  if(new)
    {
      if(new->uknotv)
	free(new->uknotv);
      if(new->vknotv)
	free(new->vknotv);
      free(new);
    }
  if(controlv)
    free(controlv);

 return ay_status;
} /* ay_npt_birail1 */


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
	       ay_nurbpatch_object **birail2,
	       int has_start_cap, ay_object **start_cap,
	       int has_end_cap, ay_object **end_cap)
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
 double mr[16], mrs[16];
 /*double quat[4] = {0};*/
 double *cs1cv = NULL, *cs2cv = NULL, *r1cv = NULL, *r2cv = NULL, *rots = NULL;
 double *cs2cvi = NULL, *iccv = NULL;
 double scalx, scaly, scalz;
 double rotv[4] = {0};

  stride = 4;

  if(!o1 || !o2 || !o3 || !o4 || !birail2 ||
     (has_start_cap && !start_cap) || (has_end_cap && !end_cap))
    return AY_ENULL;

  if((o1->type != AY_IDNCURVE) || (o2->type != AY_IDNCURVE) ||
     (o3->type != AY_IDNCURVE) || (o4->type != AY_IDNCURVE) ||
     (o5 && o5->type != AY_IDNCURVE))
    return AY_OK;

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
	  ay_object_delete(curve);
	  return ay_status;
	}
      ay_status = ay_nct_makecompatible(curve);
      if(ay_status)
	{
	  ay_object_deletemulti(curve);
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
      if(!(iccv = calloc(ic->length * stride, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      memcpy(iccv, ic->controlv, ic->length * stride * sizeof(double));

      ay_trafo_creatematrix(o5, mr1);
      a = 0;
      for(i = 0; i < ic->length; i++)
	{
	  ay_trafo_apply4(&(iccv[a]), mr1);
	  a += stride;
	}
      plenic = fabs(ic->knotv[ic->length] - ic->knotv[ic->order-1]);
    }  /* if */

  /* apply all transformations to first cross-section curves controlv */
  if(!(cs1cv = calloc(cs1->length * stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(cs1cv, cs1->controlv, cs1->length * stride * sizeof(double));

  ay_trafo_creatematrix(o1, mcs);
  a = 0;
  for(i = 0; i < cs1->length; i++)
    {
      ay_trafo_apply4(&(cs1cv[a]), mcs);
      a += stride;
    }

  /* apply all transformations to rail1 curves controlv */
  if(!(r1cv = calloc(r1->length * stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(r1cv, r1->controlv, r1->length * stride * sizeof(double));

  ay_trafo_creatematrix(o2, mr1);
  a = 0;
  for(i = 0; i < r1->length; i++)
    {
      ay_trafo_apply4(&(r1cv[a]), mr1);
      a += stride;
    }

  /* apply all transformations to rail2 curves controlv */
  if(!(r2cv = calloc(r2->length * stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(r2cv, r2->controlv, r2->length * stride * sizeof(double));

  ay_trafo_creatematrix(o3, mr2);
  a = 0;
  for(i = 0; i < r2->length; i++)
    {
      ay_trafo_apply4(&(r2cv[a]), mr2);
      a += stride;
    }

  /* apply all transformations to second cross-section curves controlv */
  if(!(cs2cv = calloc(cs2->length * stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  memcpy(cs2cv, cs2->controlv, cs2->length * stride * sizeof(double));

  ay_trafo_creatematrix(o4, mcs);
  a = 0;
  for(i = 0; i < cs2->length; i++)
    {
      ay_trafo_apply4(&(cs2cv[a]), mcs);
      a += stride;
    }

  /* get scratch memory for interpolated curve */
  if(!(cs2cvi = calloc(cs2->length * stride, sizeof(double))))
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
  if(!(controlv = calloc(new->width*new->height*stride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  new->controlv = controlv;
  if(!(new->uknotv = calloc(new->width+new->uorder, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(new->vknotv = calloc(new->height+new->vorder, sizeof(double))))
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

  if(((fabs(fabs(T1[0])-fabs(T0[0])) > AY_EPSILON) ||
      (fabs(fabs(T1[1])-fabs(T0[1])) > AY_EPSILON) ||
      (fabs(fabs(T1[2])-fabs(T0[2])) > AY_EPSILON)))
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
      ay_trafo_apply4(&cs2cv[j*stride], mcs);
    } /* for */

  ay_trafo_identitymatrix(mr);
  ay_trafo_identitymatrix(mrs);

  if(!(rots = calloc((sections+1)*4, sizeof(double))))
    {
      free(new->uknotv); free(new->vknotv); free(new); free(controlv);
      ay_status = AY_EOMEM; goto cleanup;
    }

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
      AY_V3SCAL(T1,(1.0/lent1))

      /* create rotation matrix */
      if(((fabs(fabs(T1[0])-fabs(T0[0])) > AY_EPSILON) ||
	  (fabs(fabs(T1[1])-fabs(T0[1])) > AY_EPSILON) ||
	  (fabs(fabs(T1[2])-fabs(T0[2])) > AY_EPSILON)))
	{
	  ay_trafo_translatematrix(((p5[0])),
				   ((p5[1])),
				   ((p5[2])),
				   mr);

	  AY_V3CROSS(A,T0,T1);
	  lent0 = AY_V3LEN(A);
	  AY_V3SCAL(A,(1.0/lent0));

	  rots[i*4+0] = AY_R2D(acos(AY_V3DOT(T0,T1)));
	  memcpy(&rots[i*4+1], A, 3*sizeof(double));

	  if(fabs(rots[i*4]) > AY_EPSILON)
	    {
	      ay_trafo_rotatematrix(-rots[i*4], rots[i*4+1],
				    rots[i*4+2], rots[i*4+3], mr);
	      /*
		the mrs matrix is used to properly rotate the cross
		section curve for the calculation of the scale factors
	      */
	      ay_trafo_rotatematrix(-rots[i*4], rots[i*4+1],
				    rots[i*4+2], rots[i*4+3], mrs);
	      /*
	      printf("i: %d, angle %g, axis %g %g %g\n",i,
		     rots[i*4], rots[i*4+1],
		     rots[i*4+2], rots[i*4+3]);
	      */
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
      ay_trafo_multmatrix4(m, mr);

      /* add translation */
      ay_trafo_translatematrix((p5[0]-p1[0]),
			       (p5[1]-p1[1]),
			       (p5[2]-p1[2]),
			       m);

      /* invert matrix */
      ay_trafo_invmatrix4(m, mi);

      /* sweep profile */
      for(j = 0; j < cs1->length; j++)
	{
	  ay_trafo_apply4(&controlv[i*cs1->length*stride+j*stride], mi);
	} /* for */

      /* save rail vector for next iteration */
      memcpy(T0, T1, 3*sizeof(double));
      lent0 = lent1;
    } /* for */

  /* create start cap (if birail is not closed) */
  if(has_start_cap && !closed)
    {
      curve = NULL;
      ay_status = ay_object_copy(o1, &curve);
      /*ay_trafo_defaults(curve);*/
      ay_status = ay_capt_createfromcurve(curve, start_cap);
      /* transform cap */
      if(*start_cap)
	{
	  /*ay_trafo_copy(o1, *start_cap);*/
	  /* fix direction for aycsg */
	  (*start_cap)->scalz *= -1.0;
	}
      else
	{
	  ay_object_delete(curve);
	} /* if */
    } /* if */


  /* create end-cap (if birail is not closed) */
  if(has_end_cap && !closed)
    {
      curve = NULL;
      ay_status = ay_object_copy(o4, &curve);
      /*ay_trafo_defaults(curve);*/
      ay_status = ay_capt_createfromcurve(curve, end_cap);
      if(!*end_cap)
	{
	  ay_object_delete(curve);
	}
    } /* if */


  /* return result */
  *birail2 = new;

  new = NULL;
  controlv = NULL;

  /* clean up */
cleanup:
  if(rots)
    free(rots);
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
      if(new->uknotv)
	free(new->uknotv);
      if(new->vknotv)
	free(new->vknotv);
      free(new);
    }
  if(controlv)
    free(controlv);
  if(incompatible)
    {
      ay_status = ay_object_delete(o1);
      ay_status = ay_object_delete(o4);
    }

 return ay_status;
} /* ay_npt_birail2 */


/* ay_npt_interpolateu:
 *
 *
 */
int
ay_npt_interpolateu(ay_nurbpatch_object *np, int order)
{
 int ay_status = AY_OK;
 char fname[] = "npt_interpolateu";
 int i, k, N, K, stride, ind, ind2, pu;
 double *uk = NULL, *d = NULL, *Pw = NULL, v[3] = {0};
 double *U = NULL, *Q = NULL;

  if(!np)
    return AY_ENULL;

  K = np->width;
  N = np->height;
  stride = 4;
  Pw = np->controlv;
  pu = order-1;

  if(!(uk = calloc(K, sizeof(double))))
    return AY_EOMEM;

  if(!(d = calloc(N, sizeof(double))))
    {
      free(uk); return AY_EOMEM;
    }

  if(!(U = calloc(K+np->uorder, sizeof(double))))
    {
      free(uk); free(d); return AY_EOMEM;
    }

  if(!(Q = calloc(K*4, sizeof(double))))
    {
      free(uk); free(d); free(U); return AY_EOMEM;
    }

  /* find average chord length */
  for(i = 0; i < N; i++)
    {
      d[i] = 0.0;
      ind = i*stride;
      ind2 = i*stride;
      for(k = 1; k < K; k++)
	{
	  ind += N*stride;
	  v[0] = Pw[ind]   - Pw[ind2];
	  v[1] = Pw[ind+1] - Pw[ind2+1];
	  v[2] = Pw[ind+2] - Pw[ind2+2];
	  d[i] += AY_V3LEN(v);
	  if(AY_V3LEN(v) < AY_EPSILON)
	    {
	      ay_error(AY_ERROR, fname, "Can not interpolate this patch!" );
	      free(uk); free(d); free(U); return AY_OK;
	    }
	  ind2 += N*stride;
	} /* for */
    } /* for */

  /* create knotv */
  ind = N*stride;
  ind2 = 0;
  uk[0] = 0.0;
  for(k = 1; k < K; k++)
    {
      uk[k] = 0.0;
      for(i = 0; i < N; i++)
	{
	  v[0] = Pw[ind]   - Pw[ind2];
	  v[1] = Pw[ind+1] - Pw[ind2+1];
	  v[2] = Pw[ind+2] - Pw[ind2+2];

	  uk[k] += (AY_V3LEN(v)/d[i]);
	  ind += stride;
	  ind2 += stride;
	} /* for */

      uk[k] /= N;
      uk[k] += uk[k-1];
    } /* for */
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
	{ free(d); free(uk); free(U); free(Q); return ay_status; }

      ind = i*stride;
      for(k = 0; k < K; k++)
	{
	  memcpy(&(Pw[ind]), &(Q[k*4]), stride*sizeof(double));
	  ind += N*stride;
	} /* for */
    } /* for */

  free(np->uknotv);
  np->uknotv = U;
  np->uknot_type = AY_KTCUSTOM;
  np->uorder = pu+1;

  free(uk);
  free(d);
  free(Q);

 return AY_OK;
} /* ay_npt_interpolateu */


/* ay_npt_interpolatev:
 *
 *
 */
int
ay_npt_interpolatev(ay_nurbpatch_object *np, int order)
{
 int ay_status = AY_OK;
 char fname[] = "npt_interpolatev";
 int i, k, N, K, stride, ind, ind2, pv;
 double *vk = NULL, *d = NULL, *Pw = NULL, v[3] = {0};
 double *V = NULL;

  if(!np)
    return AY_ENULL;

  K = np->width;
  N = np->height;
  stride = 4;
  Pw = np->controlv;
  pv = np->vorder-1;

  if(!(vk = calloc(N, sizeof(double))))
    return AY_EOMEM;

  if(!(d = calloc(K, sizeof(double))))
    {
      free(vk); return AY_EOMEM;
    }

  if(!(V = calloc(N+np->vorder, sizeof(double))))
    {
      free(vk); free(d); return AY_EOMEM;
    }

  /* find average chord length */
  for(i = 0; i < K; i++)
    {
      ind = (i*N)*stride;
      d[i] = 0.0;
      for(k = 1; k < N; k++)
	{
	  ind2 = ind+stride;
	  v[0] = Pw[ind2]   - Pw[ind];
	  v[1] = Pw[ind2+1] - Pw[ind+1];
	  v[2] = Pw[ind2+2] - Pw[ind+2];
	  d[i] += AY_V3LEN(v);
	  if(AY_V3LEN(v) < AY_EPSILON)
	    {
	      ay_error(AY_ERROR, fname, "Can not interpolate this patch!" );
	      free(vk); free(d); free(V); return AY_OK;
	    }
	  ind += stride;
	} /* for */
    } /* for */

  /* create knotv */
  vk[0] = 0.0;
  for(k = 1; k < N; k++)
    {
      ind2 = k*stride;
      ind = ind2-stride;
      vk[k] = 0.0;
      for(i = 0; i < K; i++)
	{
	  v[0] = Pw[ind2]   - Pw[ind];
	  v[1] = Pw[ind2+1] - Pw[ind+1];
	  v[2] = Pw[ind2+2] - Pw[ind+2];

	  vk[k] += (AY_V3LEN(v)/d[i]);
	  ind += N*stride;
	  ind2 += N*stride;
	} /* for */

      vk[k] /= K;
      vk[k] += vk[k-1];
    } /* for */
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
	{ free(d); free(vk); free(V); return ay_status; }

    } /* for */

  free(np->vknotv);
  np->vknotv = V;
  np->vknot_type = AY_KTCUSTOM;
  np->vorder = pv+1;

  free(vk);
  free(d);

 return AY_OK;
} /* ay_npt_interpolatev */


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

  ay_status = ay_nct_makecompatible(curves);
  if(ay_status)
    {goto cleanup;}

  o = curves;
  while(o)
    {
      numcurves++;
      o = o->next;
    }

  curve = (ay_nurbcurve_object*)(curves->refine);
  if(!(V = calloc(curve->length+curve->order, sizeof(double))))
    {ay_status = AY_EOMEM; goto cleanup;}
  memcpy(V, curve->knotv, (curve->length+curve->order)*sizeof(double));

  K = numcurves;
  N = curve->length;

  if(order < 3)
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
	  if(d[i] < AY_EPSILON)
	    {ay_status = AY_ERROR; goto cleanup;}
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
 	      v[0] = (c2->controlv[ind]   - c1->controlv[ind])/d[i];
	      v[1] = (c2->controlv[ind+1] - c1->controlv[ind+1])/d[i];
	      v[2] = (c2->controlv[ind+2] - c1->controlv[ind+2])/d[i];
	      uk[k] += AY_V3LEN(v);
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

  /* construct patch */
  o = curves;
  curve = (ay_nurbcurve_object *) o->refine;

  if(!(skc = calloc((curve->length * numcurves * 4), sizeof(double))))
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

  U = NULL;
  V = NULL;
  skc = NULL;

  if(interpolate && degU > 1)
    ay_status = ay_npt_interpolateu(*skin, order);

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

  ay_status = ay_nct_makecompatible(curves);
  if(ay_status)
    {goto cleanup;}

  o = curves;
  while(o)
    {
      numcurves++;
      o = o->next;
    }

  curve = (ay_nurbcurve_object*)(curves->refine);
  if(!(U = calloc(curve->length+curve->order, sizeof(double))))
    {ay_status = AY_EOMEM; goto cleanup;}
  memcpy(U, curve->knotv, (curve->length+curve->order)*sizeof(double));

  K = numcurves;
  N = curve->length;

  if(order < 3)
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
	  if(d[i] < AY_EPSILON)
	    {ay_status = AY_ERROR; goto cleanup;}
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
 	      v[0] = (c2->controlv[ind]   - c1->controlv[ind])/d[i];
	      v[1] = (c2->controlv[ind+1] - c1->controlv[ind+1])/d[i];
	      v[2] = (c2->controlv[ind+2] - c1->controlv[ind+2])/d[i];
	      vk[k] += AY_V3LEN(v);
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

  /* construct patch */
  o = curves;
  curve = (ay_nurbcurve_object *) o->refine;

  if(!(skc = calloc((curve->length * numcurves * stride), sizeof(double))))
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

  U = NULL;
  V = NULL;
  skc = NULL;

  if(interpolate && degV > 1)
    ay_status = ay_npt_interpolatev(*skin, order);

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
    return AY_OK;

  curve = (ay_nurbcurve_object *)(o->refine);

  /* get curves transformation-matrix */
  ay_trafo_creatematrix(o, m);

  /* calloc the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    return AY_EOMEM;
  if(!(controlv = calloc(4*4*curve->length, sizeof(double))))
    { free(new); return AY_EOMEM; }
  if(!(uknotv = calloc(4, sizeof(double))))
    { free(new); free(controlv); return AY_EOMEM; }
  if(!(vknotv = calloc(curve->length+curve->order,sizeof(double))))
    { free(new); free(controlv); free(uknotv); return AY_EOMEM; }

  memcpy(vknotv,curve->knotv,(curve->length+curve->order)*sizeof(double));
  new->vknotv = vknotv;
  memcpy(uknotv,uknots,4*sizeof(double));
  new->uknotv = uknotv;
  new->vorder = curve->order;
  new->uorder = 2; /* linear! */
  new->vknot_type = curve->knot_type;
  new->uknot_type = AY_KTCUSTOM;
  new->width = 2;
  new->height = curve->length;
  new->glu_sampling_tolerance = curve->glu_sampling_tolerance;
  new->is_rat = curve->is_rat;

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

      point[2] = 0.0;

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
      controlv[c+2] = point[2]+(height*curve->controlv[a+3]);
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
	  if((P[i1] != P[i2]) || (P[i1+1] != P[i2+1]))
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
	  if((P[i1] != P[i2]) || (P[i1+1] != P[i2+1]))
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
  T[0] = (P[after*stride]/P[after*stride+3]) -
    (P[before*stride]/P[before*stride+3]);
  T[1] = (P[(after*stride)+1]/P[(after*stride)+3]) -
    (P[(before*stride)+1]/P[(before*stride)+3]);

  /* normalize tangent vector */
  l = sqrt(T[0]*T[0]+T[1]*T[1]);
  if(l > AY_EPSILON)
    {
      T[0] /= l;
      T[1] /= l;
    } /* if */

 return ay_status;
} /* ay_npt_gettangentfromcontrol2D */


/* ay_npt_getnormalfromcontrol3D:
 *
 *
 */
int
ay_npt_getnormalfromcontrol3D(int ctype, int n, int p,
			      int stride, double *P, int a, double *N)
{
 int ay_status = AY_OK;
 int found = AY_FALSE, wrapped = AY_FALSE;
 int b, i1, i2;
 int before, after;
 double P1[4], P2[4], P3[4];
 double t1[3], t2[3];

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
      return(ay_npt_getnormalfromcontrol3D(ctype, n, p,
					   stride, P, a-1, N));
      /*after = a;*/
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
	  if((P[i1] != P[i2]) || (P[i1+1] != P[i2+1]) || (P[i1+2] != P[i2+2]))
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
      return(ay_npt_getnormalfromcontrol3D(ctype, n, p,
					   stride, P, 1, N));
      /*before = a;*/
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
	  if((P[i1] != P[i2]) || (P[i1+1] != P[i2+1]) || (P[i1+2] != P[i2+2]))
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

  /* calculate the normal */
  memcpy(P1, &(P[after*stride]), stride*sizeof(double));
  memcpy(P2, &(P[a*stride]), stride*sizeof(double));
  memcpy(P3, &(P[before*stride]), stride*sizeof(double));

  for(i1 = 0; i1 < 3; i1++)
    {
      P1[i1] /= P1[3];
      P2[i1] /= P2[3];
      P3[i1] /= P3[3];
    }

  ay_geom_calcnfrom3(P1, P2, P3, t1);

  /* now calculate the tangent */
  t2[0] = (P[after*stride]/P[after*stride+3]) -
    (P[before*stride]/P[before*stride+3]);
  t2[1] = (P[(after*stride)+1]/P[(after*stride)+3]) -
    (P[(before*stride)+1]/P[(before*stride)+3]);
  t2[2] = (P[(after*stride)+2]/P[(after*stride)+3]) -
    (P[(before*stride)+2]/P[(before*stride)+3]);

  AY_V3NORM(t2);

  AY_V3CROSS(N, t1, t2);

  AY_V3NORM(N);

 return ay_status;
} /* ay_npt_getnormalfromcontrol3D */


/* ay_npt_bevel:
 *  create a bevel in <bevel> from a planar closed NURB curve <o>;
 *  direction of curve defines, whether bevel rounds inwards or outwards;
 *  type: 0 - round (quarter circle), 1 - linear, 2 - ridge
 *  radius: radius of the bevel
 *  align: AY_TRUE - curve is not defined in XY plane and needs to be
 *  rotated accordingly first
 */
int
ay_npt_bevel(int type, double radius, int align, ay_object *o,
	     ay_nurbpatch_object **bevel)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *patch = NULL;
 ay_nurbcurve_object *curve = NULL;
 double uknots_round[6] = {0.0, 0.0, 0.0, 1.0, 1.0, 1.0};
 double uknots_linear[4] = {0.0, 0.0, 1.0, 1.0};
 double uknots_ridge[8] = {0.0, 0.0, 0.0, 0.25, 0.75, 1.0, 1.0, 1.0};
 double uknots_round_cap[8] = {0.0, 0.0, 0.0, 1.0, 1.0, 1.5, 1.5, 1.5};
 double uknots_linear_cap[5] = {0.0, 0.0, 0.75, 1.0, 1.0};
 /* vknots are taken from curve! */
 double *uknotv = NULL, *vknotv = NULL, *controlv = NULL, *tccontrolv = NULL;
 double x, y, z, w;
 double tangent[3] = {0}, normal[3] = {0}, zaxis[3] = {0.0, 0.0, -1.0};
 double ww = sqrt(2.0)/2.0, displacex, displacey;
 int i = 0, j = 0, a = 0, b = 0, k = 0;
 double m[16], point[4] = {0}, middle[4] = {0};

  if(!o || !bevel)
    return AY_ENULL;

  if(o->type != AY_IDNCURVE)
    return AY_ERROR;

  curve = (ay_nurbcurve_object *)o->refine;

  /* calloc the new patch */
  if(!(patch = calloc(1, sizeof(ay_nurbpatch_object))))
    return AY_EOMEM;
  switch(type)
    {
    case 0:
      {
	if(!(controlv = calloc(4*3*curve->length, sizeof(double))))
	  { free(patch); return AY_EOMEM; }
	if(!(uknotv = calloc(6, sizeof(double))))
	  { free(patch); free(controlv); return AY_EOMEM; }
	memcpy(uknotv, uknots_round, 6*sizeof(double));
	patch->uorder = 3;
	patch->width = 3;
	patch->uknot_type = AY_KTNURB;
	patch->is_rat = AY_TRUE;
      }
      break;
    case 1:
      {
	if(!(controlv = calloc(4*2*curve->length, sizeof(double))))
	  { free(patch); return AY_EOMEM; }
	if(!(uknotv = calloc(4, sizeof(double))))
	  { free(patch); free(controlv); return AY_EOMEM; }
	memcpy(uknotv, uknots_linear, 4*sizeof(double));
	patch->uorder = 2;
	patch->width = 2;
	patch->uknot_type = AY_KTNURB;
	patch->is_rat = curve->is_rat;
      }
      break;
    case 2:
      {
	if(!(controlv = calloc(4*5*curve->length, sizeof(double))))
	  { free(patch); return AY_EOMEM; }
	if(!(uknotv = calloc(8, sizeof(double))))
	  { free(patch); free(controlv); return AY_EOMEM; }
	memcpy(uknotv, uknots_ridge, 8*sizeof(double));
	patch->uorder = 3;
	patch->width = 5;
	patch->uknot_type = AY_KTCUSTOM;
	patch->is_rat = AY_TRUE;
      }
      break;
    case 3:
      {
	if(!(controlv = calloc(5*4*curve->length, sizeof(double))))
	  { free(patch); return AY_EOMEM; }
	if(!(uknotv = calloc(8, sizeof(double))))
	  { free(patch); free(controlv); return AY_EOMEM; }
	memcpy(uknotv, uknots_round_cap, 8*sizeof(double));
	patch->uorder = 3;
	patch->width = 5;
	patch->uknot_type = AY_KTCUSTOM;
	patch->is_rat = AY_TRUE;
      }
      break;
    case 4:
      {
	if(!(controlv = calloc(4*3*curve->length, sizeof(double))))
	  { free(patch); return AY_EOMEM; }
	if(!(uknotv = calloc(5, sizeof(double))))
	  { free(patch); free(controlv); return AY_EOMEM; }
	memcpy(uknotv, uknots_linear_cap, 5*sizeof(double));
	patch->uorder = 2;
	patch->width = 3;
	patch->uknot_type = AY_KTNURB;
	patch->is_rat = curve->is_rat;
      }
      break;
    default:
      /* XXXX issue proper error message */
      break;
    } /* switch */

  if(!(vknotv = calloc(curve->length+curve->order,sizeof(double))))
    { free(patch); free(controlv); free(uknotv); return AY_EOMEM; }
  memcpy(vknotv,curve->knotv,(curve->length+curve->order)*sizeof(double));
  patch->vknotv = vknotv;
  patch->uknotv = uknotv;
  patch->vorder = curve->order;
  patch->vknot_type = curve->knot_type;
  patch->height = curve->length;
  patch->glu_sampling_tolerance = curve->glu_sampling_tolerance;

  /* fill controlv */
  /* first loop */
  if(align)
    {
      if(!(tccontrolv = calloc(curve->length*4, sizeof(double))))
	{
	  free(patch); free(controlv); free(uknotv); free(vknotv);
	  return AY_EOMEM;
	}
      memcpy(tccontrolv, curve->controlv, curve->length*4*sizeof(double));

      /* adjust orientation of curve (next sections only work properly,
	 when curve is planar in XY plane) */
      ay_status = ay_nct_toxy(o);
      a = 0;
      for(i = 0; i < patch->width; i++)
	{
	  memcpy(&(controlv[a]), curve->controlv,
		 curve->length*4*sizeof(double));
	  a += curve->length*4;
	} /* for */
    }
  else
    {
      /*ay_trafo_creatematrix(o, m);*/

      b = 0;
      for(i = 0; i < patch->width; i++)
	{
	  a = 0;
	  for(j = 0; j < curve->length; j++)
	    {
	      memcpy(&(controlv[b]), &(curve->controlv[a]), 4*sizeof(double));
	      /*ay_trafo_apply4(&(controlv[b]), m);*/
	      controlv[b+2] = 0.0;
	      a += 4;
	      b += 4;
	    } /* for */
	} /* for */
    } /* if */

  b = curve->length*4;
  /* transform second loop */
  if((type == 0) || (type == 3))
    {

      for(j = 0; j < curve->length; j++)
	{
	  /* get displacement direction */
	  ay_npt_gettangentfromcontrol2D(curve->type, curve->length,
					 curve->order-1, 4, controlv, j,
					 tangent);

	  w = controlv[b+3];
	  x = controlv[b]/w;
	  y = controlv[b+1]/w;
	  z = controlv[b+2]/w;

	  AY_V3CROSS(normal, tangent, zaxis)
	  AY_V3SCAL(normal, radius-(radius*w))

	  /* create transformation matrix */
	  ay_trafo_identitymatrix(m);
	  ay_trafo_translatematrix(normal[0], normal[1], radius*w, m);

	  /* transform point */
	  point[0] = m[0]*x + m[4]*y + m[8]*z + m[12]*1.0;
	  point[1] = m[1]*x + m[5]*y + m[9]*z + m[13]*1.0;
	  point[2] = m[2]*x + m[6]*y + m[10]*z + m[14]*1.0;

	  controlv[b]   = point[0]*(w*ww);
	  controlv[b+1] = point[1]*(w*ww);
	  controlv[b+2] = point[2]*(w*ww);

	  controlv[b+3] = ww*w;

	  b += 4;
	} /* for */
    } /* if */

  if(type == 2)
    {
      /* transform the middle 3 loops */
      for(k = 0; k < 3; k++)
	{
	  if((k == 0) || (k == 2))
	    {
	      ww = 0.8535;
	      if(k == 0)
		{
		  displacex = 0.8535/ww;
		  displacey = 0.3535/ww;
		}
	      else
		{
		  displacex = 0.3535/ww;
		  displacey = 0.8535/ww;
		}
	    }
	  else
	    {
	      ww = 1.1;
	      displacex = 0.5/ww;
	      displacey = 0.5/ww;
	    }

	  for(j = 0; j < curve->length; j++)
	    {
	      /* get displacement direction */
	      ay_npt_gettangentfromcontrol2D(curve->type, curve->length,
					     curve->order-1, 4, controlv, j,
					     tangent);

	      x = controlv[b];
	      y = controlv[b+1];
	      z = controlv[b+2];
	      w = controlv[b+3];

	      AY_V3CROSS(normal, tangent, zaxis)
	      AY_V3SCAL(normal,(radius*(1.0-displacex)))

	      /* create transformation matrix */
	      ay_trafo_identitymatrix(m);
	      ay_trafo_translatematrix(normal[0], normal[1],
				       displacey * radius * w, m);

	      /* transform point */
	      point[0] = m[0]*x + m[4]*y + m[8]*z + m[12]*1.0;
	      point[1] = m[1]*x + m[5]*y + m[9]*z + m[13]*1.0;
	      point[2] = m[2]*x + m[6]*y + m[10]*z + m[14]*1.0;

	      controlv[b]   = point[0]*ww;
	      controlv[b+1] = point[1]*ww;
	      controlv[b+2] = point[2]*ww;

	      controlv[b+3] = w*ww;

	      b += 4;
	    } /* for */
	} /* for */
    } /* if */

  /* transform last normal loop (before any cap loops) */
  for(j = 0; j < curve->length; j++)
    {
      /* get displacement direction */
      ay_npt_gettangentfromcontrol2D(curve->type, curve->length,
				     curve->order-1, 4, controlv, j, tangent);

      x = controlv[b];
      y = controlv[b+1];
      z = controlv[b+2];
      w = controlv[b+3];

      AY_V3CROSS(normal, tangent, zaxis)
      AY_V3SCAL(normal, radius)

      /* create transformation matrix */
      ay_trafo_identitymatrix(m);
      ay_trafo_translatematrix(normal[0], normal[1], radius*w, m);

      /* transform point */
      point[0] = m[0]*x + m[4]*y + m[8]*z + m[12]*1.0;
      point[1] = m[1]*x + m[5]*y + m[9]*z + m[13]*1.0;
      point[2] = m[2]*x + m[6]*y + m[10]*z + m[14]*1.0;
      /*point[3] = m[3]*x + m[7]*y + m[11]*z + m[15]*1.0;*/

      controlv[b]   = point[0];
      controlv[b+1] = point[1];
      controlv[b+2] = point[2];
      controlv[b+3] = w;

      b += 4;
    } /* for */

  /* transform cap loops */
  if(type > 2)
    {
      /* calculate middle point, curve->type tells how many control
	 points should be considered */
      ay_status = ay_npt_extractmiddlepoint(curve->controlv, curve->length,
					    1, 4, 0, 1, middle);

      middle[2] = controlv[b-2];
    } /* if */

  if(type == 3)
    {
      memcpy(&(controlv[b]), &(controlv[b-(curve->length*4)]),
	     curve->length*4*sizeof(double));
      b += (curve->length*4);

      /* now set complete last loop to middle point */
      for(k = 0; k < curve->length; k++)
	{
	  memcpy(&(controlv[b]), middle, 3*sizeof(double));
	  controlv[b+3] = 1.0;
	  b += 4;
	}
    } /* if */

  if(type == 4)
    {
      /* now set complete last loop to middle point */
      for(k = 0; k < curve->length; k++)
	{
	  memcpy(&(controlv[b]), middle, 3*sizeof(double));
	  controlv[b+3] = 1.0;
	  b += 4;
	}
    } /* if */

  /* re-process first loop? */
  if(align)
    {
      /* overwrite first loop with original saved curve data (to avoid
         losing precision due to double rotation) */
      memcpy(controlv, tccontrolv, curve->length*4*sizeof(double));
      /* rotate other loops into position */
      ay_trafo_creatematrix(o, m);
      b = curve->length*4;
      for(i = 1; i < patch->width; i++)
	{
	  for(j = 0; j < curve->length; j++)
	    {
	      ay_trafo_apply4(&(controlv[b]), m);
	      b += 4;
	    } /* for */
	} /* for */
    } /* if */

  patch->controlv = controlv;

  *bevel = patch;

  /* clean-up */
  if(tccontrolv)
    free(tccontrolv);

 return ay_status;
} /* ay_npt_bevel */


/* ay_npt_createcap:
 *
 *
 */
int
ay_npt_createcap(double z, ay_nurbcurve_object *curve,
		 double *ominx, double *omaxx,
		 double *ominy, double *omaxy, double *oangle,
		 ay_nurbpatch_object **cap)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *patch = NULL;
 double knotv[4] = {0.0,0.0,1.0,1.0};
 double minx, miny, maxx, maxy, angle;
 int i, stride;

  /* calloc the new patch */
  if(!(patch = calloc(1, sizeof(ay_nurbpatch_object))))
    return AY_EOMEM;
  if(!(patch->vknotv = calloc(4, sizeof(double))))
    { free(patch); return AY_EOMEM; }
  if(!(patch->uknotv = calloc(4, sizeof(double))))
    { free(patch); free(patch->vknotv); return AY_EOMEM; }
  if(!(patch->controlv = calloc(4*4, sizeof(double))))
    { free(patch); free(patch->vknotv); free(patch->uknotv); return AY_EOMEM; }

  patch->width = 2;
  patch->height = 2;
  patch->uorder = 2;
  patch->vorder = 2;
  memcpy(patch->uknotv, knotv, 4*sizeof(double));
  memcpy(patch->vknotv, knotv, 4*sizeof(double));

  i = 0;
  minx = curve->controlv[0]; maxx = minx;
  miny = curve->controlv[1]; maxy = miny;
  angle = 0.0;
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

      /* compute direction */
      if((i < (curve->length-1) * stride) && (i > stride))
	{
	  angle +=
	    ((curve->controlv[i+stride] - curve->controlv[i-stride])*
	     (curve->controlv[i+1] - curve->controlv[i+1-stride])) -
	    ((curve->controlv[i] - curve->controlv[i-stride])*
	     (curve->controlv[i+stride+1] - curve->controlv[i+1-stride]));
	}


      i += stride;
    } /* while */

  patch->controlv[0] = minx;
  patch->controlv[1] = miny;

  patch->controlv[4] = minx;
  patch->controlv[5] = maxy;

  patch->controlv[8] = maxx;
  patch->controlv[9] = miny;

  patch->controlv[12] = maxx;
  patch->controlv[13] = maxy;

  for(i = 2; i <= 15; i += 4)
    {
      patch->controlv[i] = z;
      patch->controlv[i+1] = 1.0;
    }

  *ominx = minx;
  *omaxx = maxx;
  *ominy = miny;
  *omaxy = maxy;
  *oangle = angle;

  *cap = patch;

 return ay_status;
} /* ay_npt_createcap */


/* ay_npt_applytrafo:
 *
 */
int
ay_npt_applytrafo(ay_object *p)
{
 int i, stride;
 double m[16];
 ay_nurbpatch_object *np = NULL;

  if(!p)
    return AY_ENULL;

  if(!p->type == AY_IDNPATCH)
    return AY_EWTYPE;

  np = (ay_nurbpatch_object *)(p->refine);

  /* get curves transformation-matrix */
  ay_trafo_creatematrix(p, m);

  stride = 4;
  i = 0;
  while(i < np->width*np->height*stride)
    {
      ay_trafo_apply4(&(np->controlv[i]), m);

      i += stride;
    } /* while */

  /* reset surfaces transformation attributes */
  ay_trafo_defaults(p);

 return AY_OK;
} /* ay_npt_applytrafo */


/* ay_npt_getpntfromindex:
 *
 *
 */
int
ay_npt_getpntfromindex(ay_nurbpatch_object *patch, int indexu, int indexv,
		       double **p)
{
 int stride = 4;
 char fname[] = "npt_getpntfromindex";

  if(indexu >= patch->width || indexu < 0)
    {
      ay_error(AY_ERROR, fname, "index u out of range");
      return AY_ERROR;
    }

  if(indexv >= patch->height || indexv < 0)
    {
      ay_error(AY_ERROR, fname, "index v out of range");
      return AY_ERROR;
    }

  *p = &(patch->controlv[(indexu*patch->height+indexv)*stride]);

 return AY_OK;
} /* ay_npt_getpntfromindex */


/* ay_npt_elevateu:
 *
 */
int
ay_npt_elevateu(ay_nurbpatch_object *patch, int t)
{
 int ay_status = AY_OK;
 double u, *Uh = NULL, *Qw = NULL, *realQw = NULL, *realUh = NULL;
 int i, j, a, b, clamp_me = AY_FALSE, nw = 0;
 char fname[] = "npt_elevateu";

  if(patch->uknot_type == AY_KTBSPLINE)
    {
      clamp_me = AY_TRUE;
    }
  else
    {
      if(patch->uknot_type == AY_KTCUSTOM)
	{
	  a = 1;
	  u = patch->uknotv[0];
	  for(i = 1; i < patch->uorder; i++)
	    if(u == patch->uknotv[i])
	      a++;

	  j = patch->width+patch->uorder-1;
	  b = 1;
	  u = patch->uknotv[j];
	  for(i = j; i >= patch->width; i--)
	    if(u == patch->uknotv[i])
	      b++;

	  if((a < patch->uorder) || (b < patch->uorder))
	    {
	      clamp_me = AY_TRUE;
	    } /* if */
	} /* if */
    } /* if */

  if(clamp_me)
    {
      ay_status = ay_npt_clampu(patch);
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "clamp operation failed");
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
  ay_status = ay_nb_DegreeElevateSurfU(4, patch->width-1,
				       patch->height-1,
				       patch->uorder-1, patch->uknotv,
				       patch->controlv, t, &nw, Uh, Qw);

  if(ay_status)
    {
      free(Uh); free(Qw);
      ay_error(ay_status, fname, "degree elevation failed");
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

  if(patch->createmp)
    ay_status = ay_npt_recreatemp(patch);

 return ay_status;
} /* ay_npt_elevateu */


/* ay_npt_elevateutcmd:
 *
 */
int
ay_npt_elevateutcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *patch = NULL;
 int t = 1;
 char fname[] = "elevateNPU";

  if(argc >= 2)
    Tcl_GetInt(interp, argv[1], &t);

  while(sel)
    {
      if(sel->object->type == AY_IDNPATCH)
	{
	  if(sel->object->selp)
	    {
	      ay_selp_clear(sel->object);
	    }
	  patch = (ay_nurbpatch_object *)sel->object->refine;

	  ay_status = ay_npt_elevateu(patch, t);

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Elevate failed");
	    }
	  else
	    {
	      sel->object->modified = AY_TRUE;

	      /* update pointers to controlv */
	      ay_object_ccp(sel->object);

	      /* re-create tesselation of patch */
	      ay_notify_force(sel->object);
	    }
	}
      else
	{
	  ay_error(AY_EWTYPE, fname, ay_npt_npname);
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_npt_elevateutcmd */


/* ay_npt_elevatev:
 *
 */
int
ay_npt_elevatev(ay_nurbpatch_object *patch, int t)
{
 int ay_status = AY_OK;
 double v, *Vh = NULL, *Qw = NULL, *realQw = NULL, *realVh = NULL;
 int i, j, a, b, clamp_me = AY_FALSE, nh = 0, ind1, ind2;
 char fname[] = "npt_elevatev";

  if(patch->vknot_type == AY_KTBSPLINE)
    {
      clamp_me = AY_TRUE;
    }
  else
    {
      if(patch->vknot_type == AY_KTCUSTOM)
	{
	  a = 1;
	  v = patch->vknotv[0];
	  for(i = 1; i < patch->vorder; i++)
	    if(v == patch->vknotv[i])
	      a++;

	  j = patch->height+patch->vorder-1;
	  b = 1;
	  v = patch->vknotv[j];
	  for(i = j; i >= patch->height; i--)
	    if(v == patch->vknotv[i])
	      b++;

	  if((a < patch->vorder) || (b < patch->vorder))
	    {
	      clamp_me = AY_TRUE;
	    } /* if */
	} /* if */
    } /* if */

  if(clamp_me)
    {
      ay_status = ay_npt_clampv(patch);
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "clamp operation failed");
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
  ay_status = ay_nb_DegreeElevateSurfV(4, patch->width-1,
				       patch->height-1,
				       patch->vorder-1, patch->vknotv,
				       patch->controlv, t, &nh, Vh, Qw);

  if(ay_status)
    {
      free(Vh); free(Qw);
      ay_error(ay_status, fname, "degree elevation failed");
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

  if(patch->createmp)
    ay_status = ay_npt_recreatemp(patch);

 return ay_status;
} /* ay_npt_elevatev */


/* ay_npt_elevatevtcmd:
 *
 */
int
ay_npt_elevatevtcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *patch = NULL;
 int t = 1;
 char fname[] = "elevateNPV";

  if(argc >= 2)
    Tcl_GetInt(interp, argv[1], &t);

  while(sel)
    {
      if(sel->object->type == AY_IDNPATCH)
	{
	  if(sel->object->selp)
	    {
	      ay_selp_clear(sel->object);
	    }

	  patch = (ay_nurbpatch_object *)sel->object->refine;
	  ay_status = ay_npt_elevatev(patch, t);

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Elevate failed");
	    }
	  else
	    {
	      sel->object->modified = AY_TRUE;

	      /* update pointers to controlv */
	      ay_object_ccp(sel->object);

	      /* re-create tesselation of patch */
	      ay_notify_force(sel->object);
	    }
	}
      else
	{
	  ay_error(AY_EWTYPE, fname, ay_npt_npname);
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_npt_elevatevtcmd */


/* ay_npt_swapuvtcmd:
 *
 */
int
ay_npt_swapuvtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 /*int ay_status;*/
 char fname[] = "swapuvS";
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *np = NULL;
 ay_pamesh_object *pm = NULL;
 ay_bpatch_object *bp = NULL;
 double pt[3];

  while(sel)
    {
      switch(sel->object->type)
	{
	case AY_IDNPATCH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  np = (ay_nurbpatch_object *)sel->object->refine;

	  ay_npt_swapuv(np);

	  sel->object->modified = AY_TRUE;
	  break;
	case AY_IDPAMESH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  pm = (ay_pamesh_object *)sel->object->refine;

	  ay_pmt_swapuv(pm);

	  sel->object->modified = AY_TRUE;
	  break;
	case AY_IDBPATCH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

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
	  ay_error(AY_EWTYPE, fname, "NPatch, PaMesh, BPatch");
	  break;
	} /* switch */
      sel = sel->next;
    } /* while */

  ay_notify_parent();

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
 ay_object *c;
 ay_object *lcu = NULL/*, *lcv = NULL*/; /* last cu/cv curve */
 ay_nurbcurve_object *nc = NULL;
 ay_nurbpatch_object *interpatch = NULL, *skinu = NULL, *skinv = NULL;
 int uo, vo;
 int i, j, k, numcu = 0, numcv = 0; /* numbers of parameter curves */
 int need_interpol = AY_FALSE;
 double *intersections = NULL; /* matrix of intersection points */
 double *unifiedU = NULL, *unifiedV = NULL;
 int uUlen, uVlen;

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
      /*lcv = c;*/
      c = c->next;
    }

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
			   "unsupported number of u/v curves");
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
      ay_status = ay_npt_interpolateu(interpatch, uorder);
      ay_status = ay_npt_interpolatev(interpatch, vorder);
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
  for(i = 0; i < skinu->width; i++)
    {
      for(j = 0; j < skinu->height; j++)
	{
	  k = (i*skinu->height+j)*4;
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
	} /* for */
    } /* for */

  /* return result */
  *gordon = skinu;

  /* prevent cleanup code from doing something harmful */
  skinu = NULL;

cleanup:
  if(skinu)
    {
      free(skinu->uknotv);
      free(skinu->vknotv);
      free(skinu->controlv);
      free(skinu);
    }
  if(skinv)
    {
      free(skinv->uknotv);
      free(skinv->vknotv);
      free(skinv->controlv);
      free(skinv);
    }
  if(interpatch && !in)
    {
      free(interpatch->uknotv);
      free(interpatch->vknotv);
      free(interpatch->controlv);
      free(interpatch);
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
int
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
		  ay_trafo_apply3(pp2, m2);
		  if(stride == 4)
		    pp2[3] = 1.0;
		} /* if */
	    }
	  else
	    {
	      /* modify o1 (p1 <= p2) */
	      if(pp1)
		{
		  memcpy(pp1, p2, stride*sizeof(double));
		  ay_trafo_apply3(pp1, m1);
		  if(stride == 4)
		    pp1[3] = 1.0;
		} /* if */
	    } /* if */
	} /* if */
    } /* if */

 return AY_OK;
} /* ay_npt_gordoncc */


/* ay_npt_gordonwc:
 *  watch and properly configure the outer four parameter curves of a
 *  gordon surface <g> (compare and possibly correct their endpoints)
 *  XXXX should be extended to cover also the inner curve endpoints
 */
int
ay_npt_gordonwc(ay_object *g)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *nc;
 ay_icurve_object  *ic;
 ay_object *firstu = NULL, *lastu = NULL, *firstv = NULL, *lastv = NULL;
 ay_object *last = NULL, *down;
 double fum[16], lum[16], fvm[16], lvm[16];
 double fumi[16], lumi[16], fvmi[16], lvmi[16];
 double p1[3], p2[3], p3[3], p4[3], p5[3], p6[3], p7[3], p8[3];
 double *pp1 = NULL, *pp2 = NULL, *pp3 = NULL, *pp4 = NULL;
 double *pp5 = NULL, *pp6 = NULL, *pp7 = NULL, *pp8 = NULL;

  if(!g || !g->down || !g->down->next)
    return AY_ENULL;

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
    return AY_OK;

  /* get transformation matrices */
  ay_trafo_creatematrix(firstu, fum);
  ay_trafo_invmatrix4(fum, fumi);
  ay_trafo_creatematrix(lastu, lum);
  ay_trafo_invmatrix4(lum, lumi);
  ay_trafo_creatematrix(firstv, fvm);
  ay_trafo_invmatrix4(fvm, fvmi);
  ay_trafo_creatematrix(lastv, lvm);
  ay_trafo_invmatrix4(lvm, lvmi);

  /* get all 8 corner points; transform them to gordon object space */
  if(firstu->type == AY_IDNCURVE)
    {
      nc = (ay_nurbcurve_object *)firstu->refine;
      pp1 = &(nc->controlv[0]);
      pp2 = &(nc->controlv[(nc->length-1)*4]);
    }
  if(firstu->type == AY_IDICURVE)
    {
      ic = (ay_icurve_object *)firstu->refine;
      pp1 = &(ic->controlv[0]);
      pp2 = &(ic->controlv[(ic->length-1)*3]);
    }
  if(pp1)
    AY_APTRAN3(p1, pp1, fum)
  if(pp2)
    AY_APTRAN3(p2, pp2, fum)

  if(lastu->type == AY_IDNCURVE)
    {
      nc = (ay_nurbcurve_object *)lastu->refine;
      pp3 = &(nc->controlv[0]);
      pp4 = &(nc->controlv[(nc->length-1)*4]);
    }
  if(lastu->type == AY_IDICURVE)
    {
      ic = (ay_icurve_object *)lastu->refine;
      pp3 = &(ic->controlv[0]);
      pp4 = &(ic->controlv[(ic->length-1)*3]);
    }
  if(pp3)
    AY_APTRAN3(p3, pp3, lum)
  if(pp4)
    AY_APTRAN3(p4, pp4, lum)

  if(firstv->type == AY_IDNCURVE)
    {
      nc = (ay_nurbcurve_object *)firstv->refine;
      pp5 = &(nc->controlv[0]);
      pp6 = &(nc->controlv[(nc->length-1)*4]);
    }
  if(firstv->type == AY_IDICURVE)
    {
      ic = (ay_icurve_object *)firstv->refine;
      pp5 = &(ic->controlv[0]);
      pp6 = &(ic->controlv[(ic->length-1)*3]);
    }
  if(pp5)
    AY_APTRAN3(p5, pp5, fvm)
  if(pp6)
    AY_APTRAN3(p6, pp6, fvm)

  if(lastv->type == AY_IDNCURVE)
    {
      nc = (ay_nurbcurve_object *)lastv->refine;
      pp7 = &(nc->controlv[0]);
      pp8 = &(nc->controlv[(nc->length-1)*4]);
    }
  if(lastv->type == AY_IDICURVE)
    {
      ic = (ay_icurve_object *)lastv->refine;
      pp7 = &(ic->controlv[0]);
      pp8 = &(ic->controlv[(ic->length-1)*3]);
    }
  if(pp7)
    AY_APTRAN3(p7, pp7, lvm)
  if(pp8)
    AY_APTRAN3(p8, pp8, lvm)

  /* now compare and possibly correct the points */
  /* XXXX make those "&&" below "||", when NCurve providing objects are
     supported by the endpoint calculating code above; gordoncc() can
     already work with one pp == NULL, if only the point data is complete */
  if(pp1 && pp5)
    {
      ay_status = ay_npt_gordoncc(firstu, firstv,
				  ((firstu->type == AY_IDNCURVE) &&
				   (firstv->type == AY_IDNCURVE))?4:3,
				  p1, p5, pp1, pp5, fumi, fvmi);

    }

  if(pp2 && pp7)
    {
      ay_status = ay_npt_gordoncc(firstu, lastv,
				  ((firstu->type == AY_IDNCURVE) &&
				   (lastv->type == AY_IDNCURVE))?4:3,
				  p2, p7, pp2, pp7, fumi, lvmi);
    }

  if(pp3 && pp6)
    {
      ay_status = ay_npt_gordoncc(lastu, firstv,
				  ((lastu->type == AY_IDNCURVE) &&
				   (firstv->type == AY_IDNCURVE))?4:3,
				  p3, p6, pp3, pp6, lumi, fvmi);
    }


  if(pp4 && pp8)
    {
      ay_status = ay_npt_gordoncc(lastu, lastv,
				  ((lastu->type == AY_IDNCURVE) &&
				   (lastv->type == AY_IDNCURVE))?4:3,
				  p4, p8, pp4, pp8, lumi, lvmi);
    }

 return ay_status;
} /* ay_npt_gordonwc */


/* ay_npt_extractboundary:
 *  extract boundary NURBS curve from the NURBS patch <o>
 *  apply_trafo: this parameter controls whether trafos of <o> should be
 *   copied to the curve, or applied to the control points of the curve
 */
int
ay_npt_extractboundary(ay_object *o, int apply_trafo,
		       ay_nurbcurve_object **result)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *u0 = NULL, *un = NULL, *v0 = NULL, *vn = NULL;
 ay_object o0 = {0}, o1 = {0}, o2 = {0}, o3 = {0}, *c;

  if(!o || !result)
    return AY_ENULL;

  if(o->type != AY_IDNPATCH)
    return AY_EWTYPE;

  ay_status = ay_npt_extractnc(o, 0, 0.0, AY_FALSE, apply_trafo, &u0);
  ay_status = ay_npt_extractnc(o, 1, 0.0, AY_FALSE, apply_trafo, &un);
  ay_status = ay_npt_extractnc(o, 2, 0.0, AY_FALSE, apply_trafo, &v0);
  ay_status = ay_npt_extractnc(o, 3, 0.0, AY_FALSE, apply_trafo, &vn);

  ay_nct_revert(un);
  ay_nct_revert(v0);

  ay_object_defaults(&o0);
  ay_object_defaults(&o1);
  ay_object_defaults(&o2);
  ay_object_defaults(&o3);

  o0.refine = u0;
  o0.type = AY_IDNCURVE;
  o1.refine = vn;
  o1.type = AY_IDNCURVE;
  o2.refine = un;
  o2.type = AY_IDNCURVE;
  o3.refine = v0;
  o3.type = AY_IDNCURVE;

  o0.next = &o1;
  o1.next = &o2;
  o2.next = &o3;

  ay_status = ay_nct_concatmultiple(AY_TRUE, 1, AY_FALSE, &o0, &c);
  if(!c)
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

 return ay_status;
} /* ay_npt_extractboundary */


/* ay_npt_extractmiddlepoint:
 *  helper to extract one point of the middle axis curve from a NURBS patch
 *  control matrix with control points: <cv>, <width>, <height>, <stride>
 *  along dimension <side> (0 - u, 1 - v) at control mesh position <index>,
 *  outputs resulting point in <result>
 */
int
ay_npt_extractmiddlepoint(double *cv, int width, int height, int stride,
			  int index, int side,
			  double *result)
{
 int ay_status = AY_OK;
 int a, i, j;
 double *tcv = NULL;

  memset(result, 0, 4*sizeof(double));

  if(side == 0)
    {
      if(!(tcv = calloc(height*stride, sizeof(double))))
	return AY_EOMEM;
      a = index*height*stride;
      for(i = 0; i < height; i++)
	{
	  memcpy(&(tcv[i*stride]), &(cv[a]), stride*sizeof(double));
	  a += stride;
	}

      qsort(tcv, height, stride*sizeof(double), ay_nct_cmppnt);

      a = 0;
      i = 0;
      j = height;
      while(i < height)
	{
	  result[0] += tcv[a];
	  result[1] += tcv[a+1];
	  result[2] += tcv[a+2];
	  result[3] += tcv[a+3];

	  /* skip over sequence of equal points */
	  if((i < (height-1)) &&
	     !ay_nct_cmppnt((void*)(&(tcv[a])),(void*)(&(tcv[a+stride]))))
	    {
	      do
		{
		  i++;
		  a += stride;
		  j--;
		}
	      while((i < (height-1)) &&
		!ay_nct_cmppnt((void*)(&(tcv[a])),(void*)(&(tcv[a+stride]))));
	    }

	  i++;
	  a += stride;
	} /* while */

      result[0] /= j;
      result[1] /= j;
      result[2] /= j;
      result[3] /= j;

      free(tcv);
    }
  else
    {
      if(!(tcv = calloc(width*stride, sizeof(double))))
	return AY_EOMEM;

      a = index*stride;
      for(i = 0; i < width; i++)
	{
	  memcpy(&(tcv[i*stride]), &(cv[a]), stride*sizeof(double));
	  a += height*stride;
	}

      qsort(tcv, width, stride*sizeof(double), ay_nct_cmppnt);

      a = 0;
      i = 0;
      j = width;
      while(i < width)
	{
	  result[0] += tcv[a];
	  result[1] += tcv[a+1];
	  result[2] += tcv[a+2];
	  result[3] += tcv[a+3];

	  /* skip over sequence of equal points */
	  if((i < (width-1)) &&
	     !ay_nct_cmppnt((void*)(&(tcv[a])),(void*)(&(tcv[a+stride]))))
	    {
	      do
		{
		  j--;
		  i++;
		  a += stride;
		}
	      while((i < (width-1)) &&
		!ay_nct_cmppnt((void*)(&(tcv[a])),(void*)(&(tcv[a+stride]))));
	    }

	  i++;
	  a += stride;
	} /* while */

      result[0] /= j;
      result[1] /= j;
      result[2] /= j;
      result[3] /= j;

      free(tcv);
    } /* if */

 return ay_status;
} /* ay_npt_extractmiddlepoint */


/* ay_npt_extractnc:
 *  extract a NURBS curve from the NURBS patch <o>
 *  side: specifies extraction of a boundary curve (0-3), of a curve at a
 *   specific parametric value (4 - along u dimension, 5 - along v dimension),
 *   the complete boundary curve (6), or the middle axis (7,8)
 *  param: parametric value at which curve is extracted; this parameter is
 *   ignored for the extraction of boundary curves
 *  relative: should param be interpreted in a relative way wrt. the knot
 *   vector?; this parameter is ignored for the extraction of boundary curves
 *  apply_trafo: this parameter controls whether trafos of <o> should be
 *   copied to the curve, or applied to the control points of the curve
 */
int
ay_npt_extractnc(ay_object *o, int side, double param, int relative,
		 int apply_trafo,
		 ay_nurbcurve_object **result)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *np = NULL;
 ay_nurbcurve_object *nc = NULL;
 double *cv, m[16], *Qw = NULL, *UVQ = NULL;
 double uv, uvmin, uvmax;
 int stride = 4, i, a, k = 0, s = 0, r = 0;

  if(!o || !result)
    return AY_ENULL;

  if(o->type != AY_IDNPATCH)
    return AY_EWTYPE;

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
      nc->order =  np->uorder;
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
      nc->order =  np->uorder;
      nc->knot_type = np->uknot_type;
      nc->length = np->width;
      break;
    case 5:
      nc->order = np->vorder;
      nc->knot_type = np->vknot_type;
      nc->length = np->height;
      break;
    case 7:
      nc->order =  np->uorder;
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

  ay_trafo_creatematrix(o, m);

  if(!(nc->knotv = calloc(nc->length+nc->order, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(nc->controlv = calloc(nc->length*stride, sizeof(double))))
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
	  if(!(Qw = calloc(((np->height+r)*np->width)*stride, sizeof(double))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  if(!(UVQ = calloc((np->height+np->vorder+r), sizeof(double))))
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
	  if(!(Qw = calloc(((np->width+r)*np->height)*stride, sizeof(double))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  if(!(UVQ = calloc((np->width+np->uorder+r), sizeof(double))))
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

      if(r < 1)
	Qw = NULL;

      memcpy(nc->knotv, np->vknotv, (nc->length+nc->order)*sizeof(double));
      break;
    case 7:
      /* middle u */
      for(i = 0; i < nc->length; i++)
	{
	  ay_npt_extractmiddlepoint(np->controlv, np->width, np->height,
				    stride, i, 0, &cv[i*stride]);
	  if(!np->is_rat)
	    cv[i*stride+3] = 1.0;
	}
      memcpy(nc->knotv, np->uknotv, (nc->length+nc->order)*sizeof(double));
      break;
    case 8:
      /* middle v */
      for(i = 0; i < nc->length; i++)
	{
	  ay_npt_extractmiddlepoint(np->controlv, np->width, np->height,
				    stride, i, 1, &cv[i*stride]);
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
      cv = nc->controlv;
      a = 0;
      for(i = 0; i < nc->length; i++)
	{
	  ay_trafo_apply4(&(cv[a]), m);
	  a += stride;
	}
    }

  nc->is_rat = ay_nct_israt(nc);

  ay_nct_settype(nc);

  /* return result */
  *result = nc;
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


/* ay_npt_israt:
 *  check whether any control point of NURBS patch <np>
 *  uses a weight value (!= 1.0)
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


/* ay_npt_isboundcurve:
 *  helper for ay_npt_istrimmed() below
 *  check, whether curve <o> runs along the bounds <b1>, <b2>, <b3>, or <b4>
 *  returns AY_TRUE or AY_FALSE in <result>
 */
int
ay_npt_isboundcurve(ay_object *o, double b1, double b2, double b3, double b4,
		    int *result)
{
 int ay_status = AY_OK;
 int i = 0, stride = 4;
 int on_bound = AY_FALSE;
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

  /* apply transformations */
  /*if(AY_ISTRAFO(o))
    {*/
  ay_trafo_creatematrix(o, m);

  tcv = calloc(ncurve->length*stride, sizeof(double));

  if(!tcv)
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  p = tcv;
  for(i = 0; i < ncurve->length-1; i++)
    {
      AY_APTRAN4(p,cv,m);
      p += stride;
      cv += stride;
    }
  cv = tcv;
  /*}*/

  for(i = 0; i < ncurve->length-1; i++)
    {
      on_bound = AY_FALSE;
      /* check against bound1 */
      if(!on_bound && (fabs(cv[i]-b1) < AY_EPSILON))
	{
	  if(fabs(cv[i+stride]-b1) < AY_EPSILON)
	    {
	      on_bound = AY_TRUE;
	    }
	} /* if */
      /* check against bound2 */
      if(!on_bound && (fabs(cv[i]-b2) < AY_EPSILON))
	{
	  if(fabs(cv[i+stride]-b2) < AY_EPSILON)
	    {
	      on_bound = AY_TRUE;
	    }
	} /* if */
      /* check against bound3 */
      if(!on_bound && (fabs(cv[i+1]-b3) < AY_EPSILON))
	{
	  if(fabs(cv[i+stride+1]-b3) < AY_EPSILON)
	    {
	      on_bound = AY_TRUE;
	    }
	} /* if */
      /* check against bound4 */
      if(!on_bound && (fabs(cv[i+1]-b4) < AY_EPSILON))
	{
	  if(fabs(cv[i+stride+1]-b4) < AY_EPSILON)
	    {
	      on_bound = AY_TRUE;
	    }
	} /* if */

      if(!on_bound)
	{
	  *result = AY_FALSE;
	  goto cleanup;
	}

      cv += stride;
    } /* for */

  /*
   * if we get here, all curve segments are each on one boundary
   */
  *result = AY_TRUE;

cleanup:

  if(c)
    ay_object_delete(c);

  if(tcv)
    free(tcv);

 return ay_status;
} /* ay_npt_isboundcurve */


/* ay_npt_istrimmed:
 *  mode: 0 - check if non-trivially trimmed:
 *        returns: AY_FALSE in error and if trivially trimmed
 *                 AY_TRUE if non-trivially trimmed
 *        Does not work well for degenerate patches.
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
    {
      return AY_EWTYPE;
    }

  npatch = (ay_nurbpatch_object *)o->refine;

  if(!npatch)
    return AY_FALSE;

  b1 = npatch->uknotv[0];
  b2 = npatch->uknotv[npatch->width];
  b3 = npatch->vknotv[0];
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
    default:
      break;
    } /* switch */

 return AY_FALSE;
} /* ay_npt_istrimmed */


/* ay_npt_closeu:
 *  close NURBS patch <np> by copying the first p control point
 *  lines (in v direction) over to the last p control point lines
 *  (where p is the degree of the NURBS surface in u direction
 *  Note: this does not guarantee a closed surface unless the u knot
 *  vector is e.g. of type AY_KTBSPLINE
 *  (this function should rather be named ay_npt_makeperiodicu())
 */
int
ay_npt_closeu(ay_nurbpatch_object *np)
{
 double *controlv, *end;
 int stride = 4;
 /* int i; */

  if(np->width >= ((np->uorder-1)*2))
    {
      controlv = np->controlv;
      end = &(controlv[np->height*(np->width-(np->uorder-1))*stride]);

      memcpy(end, controlv, (np->uorder-1)*np->height*stride*sizeof(double));
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

 return AY_OK;
} /* ay_npt_closeu */


/* ay_npt_closeutcmd:
 *
 */
int
ay_npt_closeutcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "closeuNP";
 int stride = 4;
 double *newcontrolv = NULL, *tknotv;
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *np = NULL;

  while(sel)
    {
      if(!sel->object)
	{
	  ay_error(AY_ENULL, fname, NULL);
	  return TCL_OK;
	}

      switch(sel->object->type)
	{
	case AY_IDNPATCH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  np = (ay_nurbpatch_object *)sel->object->refine;

	  if(!(newcontrolv = calloc((np->width + (np->uorder-1)) *
				    np->height * stride,
				    sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }

	  memcpy(newcontrolv, np->controlv, np->width * np->height * stride *
		 sizeof(double));

	  free(np->controlv);
	  np->controlv = newcontrolv;

	  np->width += (np->uorder-1);
	  /*
	  if(!(newknotv = calloc(np->width + np->uorder + (np->uorder-1),
				 sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }

	  memcpy(newknotv, np->uknotv, (np->width + np->uorder +
					(np->uorder-1)) *
		 sizeof(double));

	  free(np->uknotv);
	  np->uknotv = newknotv;
	  */

	  tknotv = np->vknotv;
	  np->vknotv = NULL;
	  np->uknot_type = AY_KTBSPLINE;
	  ay_status = ay_knots_createnp(np);
	  free(np->vknotv);
	  np->vknotv = tknotv;

	  ay_status = ay_npt_closeu(np);

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Error closing object!");
	    }

	  ay_status = ay_npt_recreatemp(np);

	  sel->object->modified = AY_TRUE;

	  /* update pointers to controlv */
	  ay_object_ccp(sel->object);

	  /* re-create tesselation of patch */
	  ay_notify_force(sel->object);
	  break;
	default:
	  ay_error(AY_EWTYPE, fname, ay_npt_npname);
	  break;
	} /* switch */
      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_npt_closeutcmd */


/* ay_npt_closev:
 *  close NURBS patch <np> by copying the first q control point
 *  lines (in u direction) over to the last q control point lines
 *  (where q is the degree of the NURBS surface in v direction
 *  Note: this does not guarantee a closed surface unless the v knot
 *  vector is e.g. of type AY_KTBSPLINE
 *  (this function should rather be named ay_npt_makeperiodicv())
 */
int
ay_npt_closev(ay_nurbpatch_object *np)
{
 double *controlv, *a, *b;
 int i, stride = 4;

  if(np->height >= ((np->vorder-1)*2))
    {
      controlv = np->controlv;
      for(i = 0; i < np->width; i++)
	{
	  b = &(controlv[i*np->height*stride]);
	  a = b+((np->height-(np->vorder-1))*stride);

	  memcpy(a, b, (np->vorder-1)*stride*sizeof(double));
	} /* for */
    }
  else
    {
      return AY_ERROR;
    } /* if */

 return AY_OK;
} /* ay_npt_closev */


/* ay_npt_closevtcmd:
 *
 */
int
ay_npt_closevtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "closevNP";
 int stride = 4, i;
 double *a, *b;
 double *newcontrolv = NULL, *tknotv;
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *np = NULL;

  while(sel)
    {
      if(!sel->object)
	return TCL_OK;

      switch(sel->object->type)
	{
	case AY_IDNPATCH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  np = (ay_nurbpatch_object *)sel->object->refine;

	  if(!(newcontrolv = calloc(np->width *
				    (np->height + (np->vorder-1)) * stride,
				     sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
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

	  tknotv = np->uknotv;
	  np->uknotv = NULL;
	  np->vknot_type = AY_KTBSPLINE;
	  ay_status = ay_knots_createnp(np);
	  free(np->uknotv);
	  np->uknotv = tknotv;

	  ay_status = ay_npt_closev(np);

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Error closing object!");
	    }

	  ay_status = ay_npt_recreatemp(np);

	  sel->object->modified = AY_TRUE;

	  /* update pointers to controlv */
	  ay_object_ccp(sel->object);

	  /* re-create tesselation of patch */
	  ay_notify_force(sel->object);
	  break;
	default:
	  ay_error(AY_EWTYPE, fname, ay_npt_npname);
	  break;
	} /* switch */
      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_npt_closevtcmd */


/* ay_npt_isclosedu:
 *  check whether NURBS patch <np> is closed in u direction
 *  XXXX unfinished
 */
int
ay_npt_isclosedu(ay_nurbpatch_object *np)
{
#if 0
 int i;
 double u;

  if(!np)
    return AY_ENULL;

  /* check closedness in U direction */
  for(i = 0; i < np->height; i++)
    {
      u = np->uknotv[np->uorder-1];

    } /* for */
#endif
 return AY_OK;
} /* ay_npt_isclosedu */


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
int
ay_npt_recreatemp(ay_nurbpatch_object *np)
{
 int ay_status = AY_OK;
 ay_mpoint *mp = NULL, *new = NULL;
 double *ta, **tmpp = NULL;
 unsigned int *tmpi = NULL;
 int found = AY_FALSE, a, b, i, j, ii, jj, count;

  if(!np)
    return AY_OK;

  ay_npt_clearmp(np);

  if(!np->createmp)
    return AY_OK;

  if(!(tmpp = calloc(np->width*np->height, sizeof(double *))))
    return AY_EOMEM;

  if(!(tmpi = calloc(np->width*np->height, sizeof(unsigned int))))
    return AY_EOMEM;

  a = 0;
  for(ii = 0; ii < np->width; ii++)
    {
      for(jj = 0; jj < np->height; jj++)
	{
	  ta = &(np->controlv[a]);

	  /* count identical points */
	  count = 0;
	  b = 0;
	  for(i = 0; i < np->width; i++)
	    {
	      for(j = 0; j < np->height; j++)
		{
		  if(!memcmp(ta, &(np->controlv[b]), 4*sizeof(double)))
		    {
		      tmpp[count] = &(np->controlv[b]);
		      tmpi[count] = i*j;
		      count++;
		    }

		  b += 4;
		} /* for */
	    } /* for */

	  /* create new mp, if it is not already there */
	  if(count > 1)
	    {
	      mp = np->mpoints;
	      found = AY_FALSE;
	      while(mp && !found)
		{
		  if(!memcmp(ta, mp->points[0], 4*sizeof(double)))
		    {
		      found = AY_TRUE;
		    }

		  mp = mp->next;
		} /* while */

	      if(!found)
		{
		  if(!(new = calloc(1, sizeof(ay_mpoint))))
		    { free(tmpp); return AY_EOMEM; }
		  if(!(new->points = calloc(count, sizeof(double *))))
		    { free(tmpp); free(new); return AY_EOMEM; }
		  if(!(new->indices = calloc(count, sizeof(unsigned int))))
		    { free(new->points); free(tmpp); free(new);
		      return AY_EOMEM; }
		  new->multiplicity = count;
		  memcpy(new->points, tmpp, count*sizeof(double *));
		  memcpy(new->indices, tmpi, count*sizeof(unsigned int));

		  new->next = np->mpoints;
		  np->mpoints = new;
		} /* if */
	    } /* if */

	  a += 4;
	} /* for */
    } /* for */

  free(tmpp);
  free(tmpi);

 return ay_status;
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
      ay_error(AY_ERROR, fname, "Select (<t>ag) atleast two points first!");
      return AY_ERROR;
    }

  if(!(new = calloc(1, sizeof(ay_mpoint))))
    return AY_EOMEM;
  if(!(new->points = calloc(count, sizeof(double *))))
    { free(new); return AY_EOMEM; }
  if(!(new->indices = calloc(count, sizeof(double *))))
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
      ay_error(AY_ERROR, fname, "Select (<t>ag) some multiple points first!");
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
      ay_error(AY_ERROR, fname, "Select (<t>ag) some multiple points first!");
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


/* ay_npt_copytptag:
 *  copy the first TP (tesselation parameter) tag from object
 *  <src> to all objects in <dst> (dst may actually be a list,
 *  connected via ->next of each object!)
 */
int
ay_npt_copytptag(ay_object *src, ay_object *dst)
{
 int ay_status = AY_OK;
 ay_object *o = NULL;
 ay_tag *t = NULL, *s = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!src->tags)
    return AY_OK;

  t = src->tags;

  while(t)
    {
      if(t->type == ay_tp_tagtype)
	{
	  o = dst;
	  while(o)
	    {
	      s = o->tags;
	      ay_tags_copy(t, &(o->tags));
	      o->tags->next = s;
	      o = o->next;
	    } /* while */
	  break;
	} /* if */
      t = t->next;
    } /* while */

 return ay_status;
} /* ay_npt_copytptag */


/* ay_npt_clampu:
 *  clamp patch <np> in u direction
 *  Warning: only works correctly for unclamped patches
 */
int
ay_npt_clampu(ay_nurbpatch_object *np)
{
 int ay_status = AY_OK;
 double *newcontrolv = NULL, *newuknotv = NULL;
 double u;
 int stride, r, k, s;

  if(!np)
    return AY_ENULL;

  stride = 4;

  /* clamp start */
  k = np->uorder - 1;
  u = np->uknotv[k];
  s = 0;
  r = np->uorder - 1;
  np->width += r;

  newcontrolv = NULL;
  if(!(newcontrolv = calloc(np->width*np->height*stride, sizeof(double))))
    return AY_EOMEM;

  newuknotv = NULL;
  if(!(newuknotv = calloc(np->width+np->uorder, sizeof(double))))
    { free(newcontrolv); return AY_EOMEM; }

  ay_status = ay_nb_InsertKnotSurfU(stride, np->width-r-1, np->height-1,
		np->uorder-1, np->uknotv, np->controlv, u, k,
		s, r, newuknotv, newcontrolv);

  if(ay_status)
    {
      free(newcontrolv); free(newuknotv);
      return ay_status;
    }

  free(np->controlv);
  np->controlv = newcontrolv;

  free(np->uknotv);
  np->uknotv = newuknotv;

  /* clamp end */
  k = np->width;
  u = np->uknotv[k];
  k--;
  s = 0;
  r = np->uorder - 1;
  np->width += r;

  newcontrolv = NULL;
  if(!(newcontrolv = calloc(np->width*np->height*stride, sizeof(double))))
    return AY_EOMEM;

  newuknotv = NULL;
  if(!(newuknotv = calloc(np->width+np->uorder, sizeof(double))))
    { free(newcontrolv); return AY_EOMEM; }

  ay_status = ay_nb_InsertKnotSurfU(stride, np->width-r-1, np->height-1,
		       np->uorder-1, np->uknotv, np->controlv, u, k,
		       s, r, newuknotv, newcontrolv);

  if(ay_status)
    {
      free(newcontrolv); free(newuknotv);
      return ay_status;
    }

  free(np->controlv);
  np->controlv = newcontrolv;

  free(np->uknotv);
  np->uknotv = newuknotv;

  /* create new controlv, uknotv, discarding the first p and last p knots */
  np->width -= (np->uorder-1)*2;
  newcontrolv = NULL;
  if(!(newcontrolv = calloc(np->width*np->height*stride, sizeof(double))))
    return AY_EOMEM;

  newuknotv = NULL;
  if(!(newuknotv = calloc(np->width+np->uorder, sizeof(double))))
    { free(newcontrolv); return AY_EOMEM; }

  memcpy(newcontrolv, &(np->controlv[(np->uorder-1)*np->height*stride]),
	 np->width*np->height*stride*sizeof(double));

  memcpy(newuknotv, &(np->uknotv[np->uorder-1]),
	 (np->width+np->uorder)*sizeof(double));

  free(np->controlv);
  np->controlv = newcontrolv;

  free(np->uknotv);
  np->uknotv = newuknotv;

  np->uknot_type = AY_KTCUSTOM;

  ay_status = ay_npt_recreatemp(np);

 return AY_OK;
} /* ay_npt_clampu */


/* ay_npt_clampv:
 *  clamp patch <np> in v direction
 *  Warning: only works correctly for unclamped patches
 */
int
ay_npt_clampv(ay_nurbpatch_object *np)
{
 int ay_status = AY_OK;
 double *newcontrolv = NULL, *newvknotv = NULL;
 double v;
 int stride, r, k, s;
 int i, a, b, oldheight;

  if(!np)
    return AY_ENULL;

  stride = 4;

  /* clamp start */
  k = np->vorder - 1;
  v = np->vknotv[k];
  s = 0;
  r = np->vorder - 1;
  np->height += r;

  newcontrolv = NULL;
  if(!(newcontrolv = calloc(np->width*np->height*stride, sizeof(double))))
    return AY_EOMEM;

  newvknotv = NULL;
  if(!(newvknotv = calloc(np->height+np->vorder, sizeof(double))))
    { free(newcontrolv); return AY_EOMEM; }

  ay_status = ay_nb_InsertKnotSurfV(stride, np->width-1, np->height-r-1,
		np->vorder-1, np->vknotv, np->controlv, v, k,
		s, r, newvknotv, newcontrolv);

  if(ay_status)
    {
      free(newcontrolv); free(newvknotv);
      return ay_status;
    }

  free(np->controlv);
  np->controlv = newcontrolv;

  free(np->vknotv);
  np->vknotv = newvknotv;

  /* clamp end */
  k = np->height;
  v = np->vknotv[k];
  k--;
  s = 0;
  r = np->vorder - 1;
  np->height += r;

  newcontrolv = NULL;
  if(!(newcontrolv = calloc(np->width*np->height*stride, sizeof(double))))
    return AY_EOMEM;

  newvknotv = NULL;
  if(!(newvknotv = calloc(np->height+np->vorder, sizeof(double))))
    { free(newcontrolv); return AY_EOMEM; }

  ay_status = ay_nb_InsertKnotSurfV(stride, np->width-1, np->height-r-1,
		       np->vorder-1, np->vknotv, np->controlv, v, k,
		       s, r, newvknotv, newcontrolv);

  if(ay_status)
    {
      free(newcontrolv); free(newvknotv);
      return ay_status;
    }

  free(np->controlv);
  np->controlv = newcontrolv;

  free(np->vknotv);
  np->vknotv = newvknotv;

  /* create new controlv, vknotv, discarding the first p and last p knots */
  oldheight = np->height;
  np->height -= (np->vorder-1)*2;
  newcontrolv = NULL;
  if(!(newcontrolv = calloc(np->width*np->height*stride, sizeof(double))))
    return AY_EOMEM;

  newvknotv = NULL;
  if(!(newvknotv = calloc(np->width+np->vorder, sizeof(double))))
    { free(newcontrolv); return AY_EOMEM; }

  for(i = 0; i < np->width; i++)
    {
      a = i*np->height*stride;
      b = (i*oldheight+(np->vorder-1))*stride;
      memcpy(&(newcontrolv[a]), &(np->controlv[b]),
	     np->height*stride*sizeof(double));
    } /* for */

  memcpy(newvknotv, &(np->vknotv[np->vorder-1]),
	 (np->height+np->vorder)*sizeof(double));

  free(np->controlv);
  np->controlv = newcontrolv;

  free(np->vknotv);
  np->vknotv = newvknotv;

  np->vknot_type = AY_KTCUSTOM;

  ay_status = ay_npt_recreatemp(np);

 return AY_OK;
} /* ay_npt_clampv */


/* ay_npt_clamputcmd:
 *  Tcl interface for ay_npt_clampu above
 */
int
ay_npt_clamputcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "clampuNP";
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *np = NULL;
 double *knotv, u;
 int i, j, count_start, count_end;

  while(sel)
    {
      if(!sel->object)
	{
	  /* something is very wrong */
	  return TCL_OK;
	}

      if(sel->object->type == AY_IDNPATCH)
	{
	  /* remove all selected points */
	  if(sel->object->selp)
	    {
	      ay_selp_clear(sel->object);
	    }

	  np = (ay_nurbpatch_object *)sel->object->refine;

	  /* check whether we need to clamp at all */
	  knotv = np->uknotv;

	  if((np->uknot_type == AY_KTNURB) ||
	     (np->uknot_type == AY_KTBEZIER))
	    {
	      ay_error(AY_ERROR, fname, "Patch is already clamped!");
	      break;
	    }

	  if(np->uknot_type == AY_KTCUSTOM)
	    {
	      count_start = 0;
	      count_end = 0;

	      /* compare knots at start */
	      u = knotv[0];
	      for(i = 1; i < np->uorder; i++)
		if(u == knotv[i])
		  count_start++;

	      /* compare knots at end */
	      j = np->width+np->uorder-1;
	      u = knotv[j];
	      for(i = j-1; i >= np->width; i--)
		if(u == knotv[i])
		  count_end++;

	      if(count_start || count_end)
		{
		  ay_error(AY_ERROR, fname, "Cannot clamp this patch!");
		  break;
		}
	    } /* if */

	  ay_status = ay_npt_clampu(np);

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Error clamping object!");
	    }

	  ay_status = ay_npt_recreatemp(np);

	  sel->object->modified = AY_TRUE;

	  /* update pointers to controlv */
	  ay_object_ccp(sel->object);

	  /* re-create tesselation of patch */
	  ay_notify_force(sel->object);
	}
      else
	{
	  ay_error(AY_EWTYPE, fname, ay_npt_npname);
	} /* if */
      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_npt_clamputcmd */


/* ay_npt_clampvtcmd:
 *  Tcl interface for ay_npt_clampv above
 */
int
ay_npt_clampvtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "clampvNP";
 ay_list_object *sel = ay_selection;
 ay_nurbpatch_object *np = NULL;
 double *knotv, v;
 int i, j, count_start, count_end;

  while(sel)
    {
      if(!sel->object)
	{
	  ay_error(AY_ENULL, fname, NULL);
	  return TCL_OK;
	}

      if(sel->object->type == AY_IDNPATCH)
	{
	  /* remove all selected points */
	  if(sel->object->selp)
	    {
	      ay_selp_clear(sel->object);
	    }

	  np = (ay_nurbpatch_object *)sel->object->refine;

	  /* check whether we need to clamp at all */
	  knotv = np->vknotv;

	  if((np->vknot_type == AY_KTNURB) ||
	     (np->vknot_type == AY_KTBEZIER))
	    {
	      ay_error(AY_ERROR, fname, "Patch is already clamped!");
	      break;
	    }

	  if(np->vknot_type == AY_KTCUSTOM)
	    {
	      count_start = 0;
	      count_end = 0;

	      /* compare knots at start */
	      v = knotv[0];
	      for(i = 1; i < np->vorder; i++)
		if(v == knotv[i])
		  count_start++;

	      /* compare knots at end */
	      j = np->height+np->vorder-1;
	      v = knotv[j];
	      for(i = j-1; i >= np->height; i--)
		if(v == knotv[i])
		  count_end++;

	      if(count_start || count_end)
		{
		  ay_error(AY_ERROR, fname, "Cannot clamp this patch!");
		  break;
		}
	    } /* if */

	  ay_status = ay_npt_clampv(np);

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Error clamping object!");
	    }

	  ay_status = ay_npt_recreatemp(np);

	  sel->object->modified = AY_TRUE;

	  /* update pointers to controlv */
	  ay_object_ccp(sel->object);

	  /* re-create tesselation of patch */
	  ay_notify_force(sel->object);
	}
      else
	{
	  ay_error(AY_EWTYPE, fname, ay_npt_npname);
	} /* if */
      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_npt_clampvtcmd */


/* ay_npt_rescaletrim:
 *  rescale a single trim curve (<trim>) from old knot domain
 *  (<omin>,<omax>) to new domain (<nmin>,<nmax>) along dimension
 *  x/u (<mode> 0), y/v (<mode> 1), or both (<mode> 2)
 */
int
ay_npt_rescaletrim(ay_object *trim,
		   int mode, double omin, double omax,
		   double nmin, double nmax)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *nc = NULL;
 double olen = 0.0;
 double nlen = 0.0;
 int stride = 4, i;

  if(!trim)
    return AY_ENULL;

  if(!trim->type == AY_IDNCURVE)
    return AY_ERROR;

  nc = (ay_nurbcurve_object *)trim->refine;

  olen = omax-omin;
  nlen = nmax-nmin;

  /* map old values to range 0-1 then to new range */
  if((mode == 0) || (mode == 2))
    {
      for(i = 0; i < nc->length; i++)
	{
	  nc->controlv[i*stride] = nmin +
	    ((nc->controlv[i*stride] - omin) / olen * nlen);
	}
    }

  if((mode == 1) || (mode == 2))
    {
      for(i = 0; i < nc->length; i++)
	{
	  nc->controlv[i*stride+1] = nmin +
	    ((nc->controlv[i*stride+1] - omin) / olen * nlen);
	}
    }

 return ay_status;
} /* ay_npt_rescaletrim */


/* ay_npt_rescaletrims:
 *  rescale the trim curves in <trim> from old knot domain
 *  (<omin>,<omax>) to new domain (<nmin>,<nmax>) along dimension
 *  x/u (<mode> 0), y/v (<mode> 1), or both (<mode> 2)
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
      if(trim->type == AY_IDNCURVE)
	{
	  ay_status = ay_npt_rescaletrim(trim, mode, omin, omax,
					 nmin, nmax);
	}

      if(trim->type == AY_IDLEVEL)
	{
	  if(trim->down && trim->down->next)
	    {
	      ay_status = ay_npt_rescaletrims(trim->down, mode, omin, omax,
					      nmin, nmax);
	    }
	}

      trim = trim->next;
    } /* while */

 return ay_status;
} /* ay_npt_rescaletrims */


/* ay_npt_rescaleknvnptcmd:
 *  rescale the knot vectors of the selected NURBS patches
 *  - to the range 0.0 - 1.0 (no arguments)
 *  - to a specific range ([-r|-ru|-rv] min max)
 *  - so that all knots have a minimum guaranteed distance
 *    ([-d|-du|-dv] mindist)
 *  where -r/-d operate on both dimensions, -ru/-du only on u
 *  and -rv/-dv only on the v dimension
 */
int
ay_npt_rescaleknvnptcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;
 ay_nurbpatch_object *patch = NULL;
 char fname[] = "rescaleknNP";
 int i = 1, mode = 0, dim = 0;
 double rmin = 0.0, rmax = 1.0, mindist = 1.0e-04;
 double oldmin, oldmax;

  /* parse args */
  if(argc > 2)
    {
      while(i+1 < argc)
	{
	  if(!strncmp(argv[i], "-r", 2))
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
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  while(sel)
    {
      src = sel->object;
      if(src->type != AY_IDNPATCH)
	{
	  ay_error(AY_EWTYPE, fname, ay_npt_npname);
	}
      else
	{
	  patch = (ay_nurbpatch_object*)src->refine;

	  if(dim == 0 || dim == 1)
	    {
	      /* process u dimension */
	      if(patch->uknot_type == AY_KTCUSTOM)
		{
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
			  ay_status = ay_npt_rescaletrims(src->down,
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
			  ay_status = ay_npt_rescaletrims(src->down,
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
		      ay_error(ay_status, fname, "Could not rescale u-knots!");
		      break;
		    }

		  src->modified = AY_TRUE;
		}
	      else
		{
		  ay_error(AY_EWARN, fname, "Need a custom knot vector!");
		} /* if */
	    } /* if */

	  if(dim == 0 || dim == 2)
	    {
	      /* process v dimension */
	      if(patch->vknot_type == AY_KTCUSTOM)
		{
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
			  ay_status = ay_npt_rescaletrims(src->down,
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
			  ay_status = ay_npt_rescaletrims(src->down,
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
		      ay_error(ay_status, fname, "Could not rescale v-knots!");
		    }
		  src->modified = AY_TRUE;
		}
	      else
		{
		  ay_error(AY_EWARN, fname, "Need a custom knot vector!");
		} /* if */
	    } /* if */
	} /* if */
      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_npt_rescaleknvnptcmd */


/* ay_npt_insertknutcmd:
 *
 */
int
ay_npt_insertknutcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;
 ay_nurbpatch_object *patch = NULL;
 double u, *knots = NULL, *newcontrolv = NULL, *newknotv = NULL;
 int stride = 4, k = 0, s = 0, r = 0;
 char fname[] = "insknuNP";

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
      if(src->type != AY_IDNPATCH)
	{
	  ay_error(AY_EWTYPE, fname, ay_npt_npname);
	}
      else
	{
	  /* remove all selected points */
	  if(sel->object->selp)
	    {
	      ay_selp_clear(sel->object);
	    }

	  patch = (ay_nurbpatch_object*)src->refine;
	  knots = patch->uknotv;

	  Tcl_GetDouble(interp, argv[1], &u);

	  if((u < knots[patch->uorder-1]) || (u > knots[patch->width]))
	    {
	      ay_error(AY_ERROR, fname, "Parameter u out of range.");
	      return TCL_OK;
	    }

	  k = 0;

	  k = ay_nb_FindSpanMult(patch->width-1, patch->uorder-1, u,
				 knots, &s);

	  Tcl_GetInt(interp, argv[2], &r);

	  if(patch->uorder < r+s)
	    {
	      ay_error(AY_ERROR, fname,
			 "Knot insertion leads to illegal knot sequence.");
	      return TCL_OK;
	    }

	  patch->width += r;

	  if(!(newcontrolv = calloc(patch->width*patch->height*stride,
				    sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  if(!(newknotv = calloc(patch->width+patch->uorder, sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  ay_status = ay_nb_InsertKnotSurfU(stride,
					    patch->width-r-1, patch->height-1,
		        patch->uorder-1, patch->uknotv, patch->controlv, u, k,
		        s, r, newknotv, newcontrolv);

	  free(patch->controlv);
	  patch->controlv = newcontrolv;

	  free(patch->uknotv);
	  patch->uknotv = newknotv;
	  patch->uknot_type = AY_KTCUSTOM;

	  ay_npt_recreatemp(patch);

	  src->modified = AY_TRUE;

	  /* update pointers to controlv */
	  ay_object_ccp(sel->object);

	  /* re-create tesselation of patch */
	  ay_notify_force(sel->object);
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_npt_insertknutcmd */


/* ay_npt_insertknvtcmd:
 *
 */
int
ay_npt_insertknvtcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL;
 ay_nurbpatch_object *patch = NULL;
 double v, *knots = NULL, *newcontrolv = NULL, *newknotv = NULL;
 int stride = 4, k = 0, s = 0, r = 0;
 char fname[] = "insknvNP";

  if(argc < 3)
    {
      ay_error(AY_EARGS, fname, "v r");
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
      if(src->type != AY_IDNPATCH)
	{
	  ay_error(AY_EWTYPE, fname, ay_npt_npname);
	}
      else
	{
	  /* remove all selected points */
	  if(sel->object->selp)
	    {
	      ay_selp_clear(sel->object);
	    }

	  patch = (ay_nurbpatch_object*)src->refine;
	  knots = patch->vknotv;

	  Tcl_GetDouble(interp, argv[1], &v);

	  if((v < knots[patch->vorder-1]) || (v > knots[patch->height]))
	    {
	      ay_error(AY_ERROR, fname, "Parameter v out of range.");
	      return TCL_OK;
	    }

	  k = 0;

	  k = ay_nb_FindSpanMult(patch->height-1, patch->vorder-1, v,
				 knots, &s);

	  Tcl_GetInt(interp, argv[2], &r);

	  if(patch->vorder < r+s)
	    {
	      ay_error(AY_ERROR, fname,
			 "Knot insertion leads to illegal knot sequence.");
	      return TCL_OK;
	    }

	  patch->height += r;

	  if(!(newcontrolv = calloc(patch->width*patch->height*stride,
				    sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  if(!(newknotv = calloc(patch->height+patch->vorder, sizeof(double))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  ay_status = ay_nb_InsertKnotSurfV(stride,
					    patch->width-1, patch->height-r-1,
		        patch->vorder-1, patch->vknotv, patch->controlv, v, k,
		        s, r, newknotv, newcontrolv);

	  free(patch->controlv);
	  patch->controlv = newcontrolv;

	  free(patch->vknotv);
	  patch->vknotv = newknotv;
	  patch->vknot_type = AY_KTCUSTOM;

	  ay_npt_recreatemp(patch);

	  src->modified = AY_TRUE;

	  /* update pointers to controlv */
	  ay_object_ccp(sel->object);

	  /* re-create tesselation of patch */
	  ay_notify_force(sel->object);
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

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

      if((u <= knots[0/*patch->uorder...?*/]) || (u >= knots[patch->width]))
	{
	  ay_error(AY_ERROR, fname, "Parameter u out of range!");
	  return AY_ERROR;
	}

      k = 0;

      k = ay_nb_FindSpanMult(patch->width-1, patch->uorder-1, u,
			     knots, &s);

      r = patch->uorder-1-s;

      patch->width += r;

      if(r != 0)
	{
	  newcontrolv = NULL;
	  if(!(newcontrolv = calloc(patch->width*patch->height*stride,
				    sizeof(double))))
	    { return AY_EOMEM; }
	  newknotv = NULL;
	  if(!(newknotv = calloc(patch->width+patch->uorder, sizeof(double))))
	    { free(newcontrolv); return AY_EOMEM; }

	  ay_status = ay_nb_InsertKnotSurfU(stride,
			patch->width-r-1, patch->height-1,
		        patch->uorder-1, patch->uknotv, patch->controlv, u, k,
		        s, r, newknotv, newcontrolv);

	  free(patch->controlv);
	  patch->controlv = newcontrolv;

	  free(patch->uknotv);
	  patch->uknotv = newknotv;
	} /* if */

      patch->uknot_type = AY_KTCUSTOM;

      /* create two new patches */
      np1 = patch;
      /*np1->type = AY_CTOPEN;*/
      ay_status = ay_object_copy(src, &new);

      if(r != 0)
	np1len = k - (np1->uorder-1) + 1 + (patch->uorder-1-s+r-1)/2 + 1;
      else
	np1len = k - (np1->uorder-1) + 1;

      np2 = (ay_nurbpatch_object*)new->refine;
      np2->width = (np1->width+1) - np1len;

      np1->width = np1len;

      newcontrolv = NULL;
      if(!(newcontrolv = calloc(np1->width*np1->height*stride,
				sizeof(double))))
	{ ay_object_delete(new); return AY_EOMEM; }
      newknotv = NULL;
      if(!(newknotv = calloc(np1->width+np1->uorder, sizeof(double))))
	{ ay_object_delete(new); free(newcontrolv); return AY_EOMEM; }

      memcpy(newcontrolv, np1->controlv,
	     np1->width*np1->height*stride*sizeof(double));

      memcpy(newknotv, np1->uknotv, (np1->width+np1->uorder)*sizeof(double));

      free(np2->controlv);
      np2->controlv = NULL;
      free(np2->uknotv);
      np2->uknotv = NULL;

      if(!(np2->controlv = calloc(np2->width*np2->height*stride,
				  sizeof(double))))
	{ ay_object_delete(new); return AY_EOMEM; }

      if(!(np2->uknotv = calloc(np2->width+np2->uorder, sizeof(double))))
	{ ay_object_delete(new); free(np2->controlv); return AY_EOMEM; }

      memcpy(np2->controlv,
	     &(np1->controlv[((np1->width-1)*np2->height)*stride]),
	     np2->width*np2->height*stride*sizeof(double));

      memcpy(np2->uknotv, &(np1->uknotv[np1->width-1]),
	     (np2->width+np2->uorder)*sizeof(double));

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

    } /* if */

 return AY_OK;
} /* ay_npt_splitu */


/* ay_npt_splitutcmd:
 *
 */
int
ay_npt_splitutcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *new = NULL;
 double u = 0.0;
 char fname[] = "splituNP";

  if(argc < 2)
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
	  if(sel->object->type == AY_IDNPATCH)
	    {
	      /* remove all selected points */
	      if(sel->object->selp)
		{
		  ay_selp_clear(sel->object);
		}

	      new = NULL;

	      ay_status = ay_npt_splitu(sel->object, u, &new);

	      if(ay_status)
		{
		  ay_error(ay_status, fname, NULL);
		  return TCL_OK;
		} /* if */

	      ay_status = ay_object_link(new);

	      sel->object->modified = AY_TRUE;

	      /* update pointers to controlv */
	      ay_object_ccp(sel->object);

	      /* re-create tesselation of original patch */
	      ay_notify_force(sel->object);
	    }
	  else
	    {
	      ay_error(AY_EWTYPE, fname, ay_npt_npname);
	      return TCL_OK;
	    } /* if */
	} /* if */
      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_npt_splitutcmd */


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

      if((v <= knots[0/*patch->vorder...?*/]) || (v >= knots[patch->height]))
	{
	  ay_error(AY_ERROR, fname, "Parameter v out of range!");
	  return AY_ERROR;
	}

      k = 0;

      k = ay_nb_FindSpanMult(patch->height-1, patch->vorder-1, v,
			     knots, &s);

      r = patch->vorder-1-s;

      patch->height += r;

      if(r != 0)
	{
	  newcontrolv = NULL;
	  if(!(newcontrolv = calloc(patch->width*patch->height*stride,
				    sizeof(double))))
	    { return AY_EOMEM; }
	  newknotv = NULL;
	  if(!(newknotv = calloc(patch->height+patch->vorder, sizeof(double))))
	    { free(newcontrolv); return AY_EOMEM; }

	  ay_status = ay_nb_InsertKnotSurfV(stride,
			patch->width-1, patch->height-r-1,
		        patch->vorder-1, patch->vknotv, patch->controlv, v, k,
		        s, r, newknotv, newcontrolv);

	  free(patch->controlv);
	  patch->controlv = newcontrolv;

	  free(patch->vknotv);
	  patch->vknotv = newknotv;
	} /* if */

      patch->vknot_type = AY_KTCUSTOM;

      /* create two new patches */
      np1 = patch;
      /*np1->type = AY_CTOPEN;*/
      ay_status = ay_object_copy(src, &new);

      if(r != 0)
	np1len = k - (np1->vorder-1) + 1 + (patch->vorder-1-s+r-1)/2 + 1;
      else
	np1len = k - (np1->vorder-1) + 1;

      np2 = (ay_nurbpatch_object*)new->refine;
      np2->height = (np1->height+1) - np1len;
      oldnp1height = np1->height;
      np1->height = np1len;

      newcontrolv = NULL;
      if(!(newcontrolv = calloc(np1->width*np1->height*stride,
				sizeof(double))))
	{ ay_object_delete(new); return AY_EOMEM; }
      newknotv = NULL;
      if(!(newknotv = calloc(np1->height+np1->vorder, sizeof(double))))
	{ ay_object_delete(new); free(newcontrolv); return AY_EOMEM; }

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

      free(np2->controlv);
      np2->controlv = NULL;
      free(np2->vknotv);
      np2->vknotv = NULL;

      if(!(np2->controlv = calloc(np2->width*np2->height*stride,
				  sizeof(double))))
	{ ay_object_delete(new); return AY_EOMEM; }

      if(!(np2->vknotv = calloc(np2->height+np2->vorder, sizeof(double))))
	{ ay_object_delete(new); free(np2->controlv); return AY_EOMEM; }

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

    } /* if */

 return AY_OK;
} /* ay_npt_splitv */


/* ay_npt_splitvtcmd:
 *
 */
int
ay_npt_splitvtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *new = NULL;
 double v = 0.0;
 char fname[] = "splitvNP";

  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "v");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  Tcl_GetDouble(interp, argv[1], &v);

  while(sel)
    {
      if(sel->object)
	{
	  if(sel->object->type == AY_IDNPATCH)
	    {
	      /* remove all selected points */
	      if(sel->object->selp)
		{
		  ay_selp_clear(sel->object);
		}

	      new = NULL;

	      ay_status = ay_npt_splitv(sel->object, v, &new);

	      if(ay_status)
		{
		  ay_error(ay_status, fname, NULL);
		  return TCL_OK;
		} /* if */

	      ay_status = ay_object_link(new);

	      sel->object->modified = AY_TRUE;

	      /* update pointers to controlv */
	      ay_object_ccp(sel->object);

	      /* re-create tesselation of original patch */
	      ay_notify_force(sel->object);
	    }
	  else
	    {
	      ay_error(AY_EWTYPE, fname, ay_npt_npname);
	      return TCL_OK;
	    } /* if */
	} /* if */
      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_npt_splitvtcmd */


/* ay_npt_extractnp:
 *  extract subpatch from patch <src>, returns patch in <result>;
 *  XXXX leaks memory in error condition
 */
int
ay_npt_extractnp(ay_object *src, double umin, double umax,
		 double vmin, double vmax, int relative,
		 ay_object **result)
{
 int ay_status = AY_OK;
 ay_object *copy = NULL, *np1 = NULL, *np2 = NULL;
 ay_nurbpatch_object *patch = NULL;
 char fname[] = "npt_extractnp", split_errmsg[] = "Split failed!";
 double uv, uvmin, uvmax;

  if(!src || !result)
    return AY_ENULL;

  /**/
  if(((umin + AY_EPSILON) > umax) || ((vmin + AY_EPSILON) > vmax))
    {
      ay_error(AY_ERROR, fname, "min must be smaller than max");
      return AY_ERROR;
    }

  if(src->type != AY_IDNPATCH)
    {
      ay_error(AY_EWTYPE, fname, ay_npt_npname);
      return AY_ERROR;
    }
  else
    {
      ay_object_copy(src, &copy);
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
	  ay_error(AY_ERROR, fname, "Parameters umin/umax out of range!");
	  return AY_ERROR;
	}
      if((vmin < patch->vknotv[patch->vorder-1]) ||
	 (vmax > patch->vknotv[patch->height]))
	{
	  ay_error(AY_ERROR, fname, "Parameters vmin/vmax out of range!");
	  return AY_ERROR;
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
	      return AY_ERROR;
	    }
	  /* <np1> is the sub surface we want
	     => remove <copy>; then move <np1> to <copy> */

	  if(np1)
	    {
	      np2 = copy;
	      copy = np1;
	      ay_object_delete(np2);
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
	      return AY_ERROR;
	    }
	  /* <copy> is the sub surface we want
	     => remove <np1> */
	  if(np1)
	    {
	      ay_object_delete(np1);
	      np1 = NULL;
	    }
	}
      if(vmin > patch->vknotv[0/*patch->vorder...?*/])
	{
	  ay_status = ay_npt_splitv(copy, vmin, &np1);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, split_errmsg);
	      return AY_ERROR;
	    }
	  /* <np1> is the sub surface we want
	     => remove <copy>; then move <np1> to <copy> */
	  if(np1)
	    {
	      np2 = copy;
	      copy = np1;
	      ay_object_delete(np2);
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
	      return AY_ERROR;
	    }
	  /* <copy> is the sub surface we want
	     => remove <np1> */
	  if(np1)
	    {
	      ay_object_delete(np1);
	      np1 = NULL;
	    }
	}

      /* return result */
      *result = copy;
    } /* if */

 return AY_OK;
} /* ay_npt_extractnp */


/* ay_npt_extractnptcmd:
 *
 */
int
ay_npt_extractnptcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *new = NULL;
 double umin = 0.0, umax = 0.0, vmin = 0.0, vmax = 0.0;
 int relative = AY_FALSE;
 char fname[] = "extrNP";

  if(argc < 5)
    {
      ay_error(AY_EARGS, fname, "umin umax vmin vmax [relative]");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  Tcl_GetDouble(interp, argv[1], &umin);
  Tcl_GetDouble(interp, argv[2], &umax);
  Tcl_GetDouble(interp, argv[3], &vmin);
  Tcl_GetDouble(interp, argv[4], &vmax);

  if(argc > 5)
    {
      Tcl_GetInt(interp, argv[5], &relative);
    }

  while(sel)
    {
      if(sel->object)
	{
	  if(sel->object->type == AY_IDNPATCH)
	    {
	      new = NULL;

	      ay_status = ay_npt_extractnp(sel->object, umin, umax, vmin, vmax,
					   relative, &new);

	      if(ay_status)
		{
		  ay_error(ay_status, fname, NULL);
		  return TCL_OK;
		} /* if */

	      ay_status = ay_object_link(new);
	    }
	  else
	    {
	      ay_error(AY_EWTYPE, fname, ay_npt_npname);
	      return TCL_OK;
	    } /* if */
	} /* if */
      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_npt_extractnptcmd */


/* ay_npt_offset:
 *  create offset surface from <o>
 *  the new surface is <offset> away from the original surface
 *  returns new patch in <np>
 *  WIP
 */
int
ay_npt_offset(ay_object *o, int mode, double offset, ay_nurbpatch_object **np)
{
 int ay_status = AY_OK;
 int i, j, a, stride = 4;
 double normal1[3] = {0}, normal2[3] = {0};
 double normal3[3] = {0}, normal4[3] = {0};
 double normal[3] = {0}, normal11[3] = {0}, normal22[3] = {0};
 double *newcv = NULL, *newukv = NULL, *newvkv = NULL;
 double *p1, *p2, *p3;
 ay_nurbpatch_object *patch = NULL;

  /* sanity check */
  if(!o || !np)
    return AY_ENULL;

  patch = (ay_nurbpatch_object*)o->refine;

  if(!(newcv = calloc(patch->width*patch->height*stride, sizeof(double))))
    return AY_EOMEM;

  /* first row */

  /* first point in first row */
  p1 = &(patch->controlv[0]);
  p2 = &(patch->controlv[stride]);
  p3 = &(patch->controlv[patch->height*stride]);
  ay_geom_calcnfrom3(p1, p2, p3, normal1);
  newcv[0] = p1[0] + (normal1[0] * offset);
  newcv[1] = p1[1] + (normal1[1] * offset);
  newcv[2] = p1[2] + (normal1[2] * offset);
  newcv[3] = p1[3];

  /* middle points in first row */
  a = stride;
  for(j = 1; j < (patch->height - 1); j++)
    {
      p1 = &(patch->controlv[a]);
      p2 = &(patch->controlv[a+stride]);
      p3 = &(patch->controlv[a+patch->height*stride]);
      ay_geom_calcnfrom3(p1, p2, p3, normal1);

      p2 = &(patch->controlv[a-stride]);
      ay_geom_calcnfrom3(p1, p3, p2, normal2);

      normal[0] = normal1[0] + (normal2[0]-normal1[0]);
      normal[1] = normal1[1] + (normal2[1]-normal1[1]);
      normal[2] = normal1[2] + (normal2[2]-normal1[2]);

      newcv[a]   = p1[0] + (normal[0] * offset);
      newcv[a+1] = p1[1] + (normal[1] * offset);
      newcv[a+2] = p1[2] + (normal[2] * offset);
      newcv[a+3] = p1[3];
      a += stride;
    }

  /* last point in first row */
  p1 = &(patch->controlv[a]);
  p2 = &(patch->controlv[a-stride]);
  p3 = &(patch->controlv[a+patch->height*stride]);
  ay_geom_calcnfrom3(p1, p3, p2, normal1);
  newcv[a]   = p1[0] + (normal1[0] * offset);
  newcv[a+1] = p1[1] + (normal1[1] * offset);
  newcv[a+2] = p1[2] + (normal1[2] * offset);
  newcv[a+3] = p1[3];

  /* midle rows */
  a = patch->height*stride;
  for(i = 1; i < (patch->width-1); i++)
    {
      /* first point in this row */
      p1 = &(patch->controlv[a]);
      p2 = &(patch->controlv[a+stride]);
      p3 = &(patch->controlv[a+patch->height*stride]);
      ay_geom_calcnfrom3(p1, p2, p3, normal1);

      p3 = &(patch->controlv[a-patch->height*stride]);
      ay_geom_calcnfrom3(p1, p3, p2, normal2);

      normal[0] = normal1[0] + (normal2[0]-normal1[0]);
      normal[1] = normal1[1] + (normal2[1]-normal1[1]);
      normal[2] = normal1[2] + (normal2[2]-normal1[2]);

      newcv[a]   = p1[0] + (normal[0] * offset);
      newcv[a+1] = p1[1] + (normal[1] * offset);
      newcv[a+2] = p1[2] + (normal[2] * offset);
      newcv[a+3] = p1[3];
      a += stride;

      /* middle points in this row */
      for(j = 1; j < (patch->height-1); j++)
	{
	  p1 = &(patch->controlv[a]);
	  p2 = &(patch->controlv[a+stride]);
	  p3 = &(patch->controlv[a+patch->height*stride]);
	  ay_geom_calcnfrom3(p1, p2, p3, normal1);

	  p2 = &(patch->controlv[a-stride]);
	  ay_geom_calcnfrom3(p1, p3, p2, normal2);

	  p3 = &(patch->controlv[a-patch->height*stride]);
	  ay_geom_calcnfrom3(p1, p2, p3, normal3);

	  p2 = &(patch->controlv[a+stride]);
	  ay_geom_calcnfrom3(p1, p3, p2, normal4);

	  normal11[0] = normal1[0] + (normal3[0]-normal1[0]);
	  normal11[1] = normal1[1] + (normal3[1]-normal1[1]);
	  normal11[2] = normal1[2] + (normal3[2]-normal1[2]);

	  normal22[0] = normal4[0] + (normal2[0]-normal4[0]);
	  normal22[1] = normal4[1] + (normal2[1]-normal4[1]);
	  normal22[2] = normal4[2] + (normal2[2]-normal4[2]);

	  normal[0] = normal11[0] + (normal22[0]-normal11[0]);
	  normal[1] = normal11[1] + (normal22[1]-normal11[1]);
	  normal[2] = normal11[2] + (normal22[2]-normal11[2]);

	  newcv[a]   = p1[0] + (normal[0] * offset);
	  newcv[a+1] = p1[1] + (normal[1] * offset);
	  newcv[a+2] = p1[2] + (normal[2] * offset);
	  newcv[a+3] = p1[3];

	  a += stride;
	}

      /* last point in this row */
      p1 = &(patch->controlv[a]);
      p2 = &(patch->controlv[a-stride]);
      p3 = &(patch->controlv[a+patch->height*stride]);
      ay_geom_calcnfrom3(p1, p3, p2, normal1);

      p3 = &(patch->controlv[a-patch->height*stride]);
      ay_geom_calcnfrom3(p1, p2, p3, normal2);

      normal[0] = normal1[0] + (normal2[0]-normal1[0]);
      normal[1] = normal1[1] + (normal2[1]-normal1[1]);
      normal[2] = normal1[2] + (normal2[2]-normal1[2]);

      newcv[a]   = p1[0] + (normal[0] * offset);
      newcv[a+1] = p1[1] + (normal[1] * offset);
      newcv[a+2] = p1[2] + (normal[2] * offset);
      newcv[a+3] = p1[3];
      a += stride;

    } /* for */

  /* last row */

  /* first point in last row */
  p1 = &(patch->controlv[a]);
  p2 = &(patch->controlv[a+stride]);
  p3 = &(patch->controlv[a-patch->height*stride]);
  ay_geom_calcnfrom3(p1, p3, p2, normal1);
  newcv[a]   = p1[0] + (normal1[0] * offset);
  newcv[a+1] = p1[1] + (normal1[1] * offset);
  newcv[a+2] = p1[2] + (normal1[2] * offset);
  newcv[a+3] = p1[3];

  /* middle points in last row */
  a += stride;
  for(j = 1; j < (patch->height - 1); j++)
    {
      p1 = &(patch->controlv[a]);
      p2 = &(patch->controlv[a+stride]);
      p3 = &(patch->controlv[a-patch->height*stride]);
      ay_geom_calcnfrom3(p1, p3, p2, normal1);

      p2 = &(patch->controlv[a-stride]);
      ay_geom_calcnfrom3(p1, p2, p3, normal2);

      normal[0] = normal1[0] + (normal2[0]-normal1[0]);
      normal[1] = normal1[1] + (normal2[1]-normal1[1]);
      normal[2] = normal1[2] + (normal2[2]-normal1[2]);

      newcv[a]   = p1[0] + (normal[0] * offset);
      newcv[a+1] = p1[1] + (normal[1] * offset);
      newcv[a+2] = p1[2] + (normal[2] * offset);
      newcv[a+3] = p1[3];
      a += stride;
    }

  /* last point in last row */
  p1 = &(patch->controlv[a]);
  p2 = &(patch->controlv[a-stride]);
  p3 = &(patch->controlv[a-patch->height*stride]);
  ay_geom_calcnfrom3(p1, p2, p3, normal1);
  newcv[a]   = p1[0] + (normal1[0] * offset);
  newcv[a+1] = p1[1] + (normal1[1] * offset);
  newcv[a+2] = p1[2] + (normal1[2] * offset);
  newcv[a+3] = p1[3];


  /* copy knot vectors */
  if(patch->uknot_type == AY_KTCUSTOM)
    {
      if(!(newukv = calloc(patch->width+patch->uorder, sizeof(double))))
	{
	  free(newcv);
	  return AY_EOMEM;
	}
      memcpy(newukv, patch->uknotv,
	     (patch->width+patch->vorder)*sizeof(double));
    }
  if(patch->vknot_type == AY_KTCUSTOM)
    {
      if(!(newvkv = calloc(patch->height+patch->vorder, sizeof(double))))
	{
	  if(newukv)
	    free(newukv);
	  free(newcv);
	  return AY_EOMEM;
	}
      memcpy(newvkv, patch->vknotv,
	     (patch->height+patch->vorder)*sizeof(double));
    }

  ay_status = ay_npt_create(patch->uorder, patch->vorder,
			    patch->width, patch->height,
			    patch->uknot_type, patch->vknot_type,
			    newcv, newukv, newvkv, np);

  if(ay_status || !np)
    {
      free(newcv);
      if(newukv)
	free(newukv);
      if(newvkv)
	free(newvkv);
    }

 return ay_status;
} /* ay_npt_offset */



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
 int ay_status;
 char fname[] = "xxxx";
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_nurbpatch_object *patch = NULL;

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

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;
      if(o->type != AY_IDNPATCH)
	{
	  ay_error(AY_EWTYPE, fname, ay_npt_npname);
	}
      else
	{
	  patch = (ay_nurbpatch_object*)o->refine;

	  /* do magic */

	  /* clean up */
	  /* re-create multiple points */
	  ay_status = ay_npt_recreatemp(patch);

	  /* update pointers to controlv */
	  ay_status = ay_object_ccp(o);
	  ay_selp_clear(o);
	  /* notify notification about changes */
	  o->modified = AY_TRUE;
	} /* if */

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_npt_xxxxtcmd */

#endif
