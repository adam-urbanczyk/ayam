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

/* npt.c NURBS patch tools */

/* functions */

/* ay_npt_create:
 *   create a NURBS patch
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

  patch->controlv = controlv;
  if(!controlv)
    {
      if(!(newcontrolv = calloc(4*width*height, sizeof(double))))
	{
	  free(patch);
	  return AY_EOMEM;
	}

      patch->controlv = newcontrolv;

    }

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
    }

  if((uknot_type == AY_KTCUSTOM) && uknotv)
    {
      if(patch->uknotv)
	free(patch->uknotv);
      patch->uknotv = uknotv;
    }

  if((vknot_type == AY_KTCUSTOM) && vknotv)
    {
      if(patch->vknotv)
	free(patch->vknotv);
      patch->vknotv = vknotv;
    }

  *patchptr = patch;

 return AY_OK;
} /* ay_npt_create */


/* ay_npt_revolve:
 *
 */
int
ay_npt_revolve(ay_object *o, double arc, int sections, int order,
	       ay_nurbpatch_object **patch)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *new = NULL;
 ay_nurbcurve_object *curve, *tmpnc = NULL;
 double *uknotv = NULL, *tcontrolv = NULL;
 double radius = 0.0, w = 0.0, ww = 0.0, x, y, z;
 int i = 0, j = 0, a = 0, b = 0, c = 0;
 double m[16], point[4] = {0};

  ww = sqrt(2.0)/2.0;

  if(!o)
    return AY_ENULL;

  if(o->type != AY_IDNCURVE)
    return AY_ERROR;

  curve = (ay_nurbcurve_object *)(o->refine);

  /* get curves transformation-matrix */
  ay_trafo_creatematrix(o, m);

  if(arc >= 360.0 || arc < -360.0 || arc == 0.0)
    {
      arc = 360.0;
    }

  if((sections == 0) || (order <= 1))
    order = 3;

  /* calloc the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    return AY_EOMEM;

  if(!(uknotv = calloc(curve->length+curve->order,sizeof(double))))
    {
      free(new); return AY_EOMEM;
    }

  memcpy(uknotv,curve->knotv,(curve->length+curve->order)*sizeof(double));
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
      z = curve->controlv[a+2];
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
	  if(arc>0.0)
	    {
	      ay_status = ay_nb_CreateNurbsCircle(radius, 0.0, arc,
						  &(new->height),&new->vknotv,
						  &tcontrolv);
	    }
	  else
	    {
	      ay_status = ay_nb_CreateNurbsCircle(radius, arc, 0.0,
						  &(new->height), &new->vknotv,
						  &tcontrolv);
	    } /* if */
	}
      else
	{
	  if(arc == 360.0)
	    {
	      tmpnc = NULL;
	      ay_status = ay_nct_crtcircbsp(sections, radius, 360.0, order,
					    &tmpnc);
	      if(!tmpnc)
		return AY_ERROR;
	      
	      tcontrolv = tmpnc->controlv;
	      new->vknotv = tmpnc->knotv;
	      new->height = tmpnc->length;
	      free(tmpnc);
	      
	    }
	  else
	    {
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
      for(i=0;i<new->height;i++)
	{
	  new->controlv[c] = tcontrolv[b];
	  new->controlv[c+1] = point[1]*tcontrolv[b+3];
	  new->controlv[c+2] = tcontrolv[b+1];
	  new->controlv[c+3] = tcontrolv[b+3]*w;
	  b+=4;
	  c+=4;
	} /* for */


	a += 4;
    } /* for */

  if(tcontrolv)
    free(tcontrolv);
  tcontrolv = NULL;

  *patch = new;

 return ay_status;
} /* ay_npt_revolve */


/* ay_npt_drawtrimcurve:
 *
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
  if((controls = calloc(length*3, sizeof(GLfloat))) == NULL)
    { free(knots); knots = NULL; return AY_EOMEM; }

  a=0;
  for(i=0;i<knot_count;i++)
    {
      knots[a] = (GLfloat)curve->knotv[a];
      a++;
    }
  a=0; b=0;
  for(i=0;i<length;i++)
    {
      x = (GLdouble)curve->controlv[b];b++;
      y = (GLdouble)curve->controlv[b];b++;

      b++;
      w = (GLdouble)curve->controlv[b];b++;

      controls[a] = (GLfloat)(m[0]*x + m[4]*y + m[12]*w);
      controls[a+1] = (GLfloat)(m[1]*x + m[5]*y + m[13]*w);
      controls[a+2] = (GLfloat)(w /*m[3]*x + m[7]*y + m[15]*w*/);
      a+=3;
    }

  if(curve->order != 2)
    gluNurbsCurve(no, (GLint)knot_count, knots,
		  (GLint)3, controls,
		  (GLint)curve->order, GLU_MAP1_TRIM_3);
  else
    gluPwlCurve(no, (GLint)curve->length, controls,
		(GLint)3, GLU_MAP1_TRIM_3);

 return AY_OK;
} /* ay_npt_drawtrimcurve */


/* ay_npt_resizearrayw:
 *  change width of a 2D control point array
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
	  memcpy(&(ncontrolv[b]),&(controlv[a]),
		 height*stride*sizeof(double));

	  a += (height*stride);
	  b += (height*stride);
	}

    }
  else
    {
      /* distribute new points */
      new = new_width-width;

      if(!(newpersec = calloc((width-1), sizeof(int))))
	return AY_EOMEM;

      while(new)
	for(i = 0; i < (width-1); i++)
	  {
	    if(new)
	      {
		(newpersec[i])++;
		new--;
	      }
	  }

      a = 0;
      b = 0;
      for(k = 0; k < (width-1); k++)
	{

	  memcpy(&(ncontrolv[b]),&(controlv[a]),
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
ay_npt_resizew(ay_nurbpatch_object *patch, int new_width)
{
 int ay_status = AY_OK;

  if(new_width == patch->width)
    return ay_status;

  ay_status = ay_npt_resizearrayw(&(patch->controlv), 4,
				  patch->width, patch->height,
				  new_width);

  patch->width = new_width;

 return ay_status;
} /* ay_npt_resizew */


/* ay_npt_resizearrayh:
 *  change height of a 2D control point array
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
	  memcpy(&(ncontrolv[b]),&(controlv[a]),
		 new_height*stride*sizeof(double));

	  a += (height*stride);
	  b += (new_height*stride);
	}

    }
  else
    {
      /* distribute new points */
      new = new_height-height;

      if(!(newpersec = calloc((height-1), sizeof(int))))
	return AY_EOMEM;

      while(new)
	for(i = 0; i < (height-1); i++)
	  {
	    if(new)
	      {
		(newpersec[i])++;
		new--;
	      }
	  }

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
ay_npt_resizeh(ay_nurbpatch_object *patch, int new_height)
{
 int ay_status = AY_OK;

  if(new_height == patch->height)
    return ay_status;

  ay_status = ay_npt_resizearrayh(&(patch->controlv), 4,
				  patch->width, patch->height,
				  new_height);

  patch->height = new_height;

 return ay_status;
} /* ay_npt_resizeh */


/* ay_npt_swapuv:
 *
 */
int
ay_npt_swapuv(ay_nurbpatch_object *np)
{
 int ay_status = AY_OK;
 int stride = 4, i1 = 0, i2 = 0, i, j;
 double *dt, *ncontrolv = NULL;

  if(!np)
    return AY_ENULL;

  if(!(ncontrolv = calloc(np->width*np->height*stride, sizeof(double))))
    return AY_EOMEM;

  for(i = 0; i < np->width; i++)
    {
      i2 = i*stride;
      for(j = 0; j < np->height; j++)
	{
	  memcpy(&(ncontrolv[i2]), &(np->controlv[i1]),
		 stride*sizeof(double));

	  i1 += stride;
	  i2 += (np->width*stride);
	} /* for */
    } /* for */

  free(np->controlv);
  np->controlv = ncontrolv;

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

 return ay_status;
} /* ay_npt_swapuv */


/* ay_npt_revertu:
 *  
 */
int
ay_npt_revertu(ay_nurbpatch_object *patch)
{
 int ay_status = AY_OK;
 int i, j, ii, jj, stride = 4;
 double t[4];

  for(i = 0; i < patch->height; i++)
    {
      for(j = 0; j < patch->width/2; j++)
	{
	  ii = (j*patch->height+i)*stride;
	  jj = ((patch->width-1-j)*patch->height+i)*stride;
	  memcpy(t, &(patch->controlv[ii]), stride*sizeof(double));
	  memcpy(&(patch->controlv[ii]), &(patch->controlv[jj]),
		 stride*sizeof(double));
	  memcpy(&(patch->controlv[jj]), t, stride*sizeof(double));
	}
    }

 return ay_status;
} /* ay_npt_revertu */


/* ay_npt_revertutcmd:
 *
 */
int
ay_npt_revertutcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status;
 char fname[] = "revertU";
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

	  ay_status = ay_npt_revertu(np);
	  break;
	case AY_IDPAMESH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  pm = (ay_pamesh_object *)sel->object->refine;

	  ay_status = ay_pmt_revertu(pm);
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
	  break;
	default:
	  ay_error(AY_ERROR, fname, "Do not know how to revert this object!");
	  break;
	} /* switch */

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_npt_revertutcmd */


/* ay_npt_revertv:
 *  
 */
int
ay_npt_revertv(ay_nurbpatch_object *patch)
{
 int ay_status = AY_OK;
 int i, j, ii, jj, stride = 4;
 double t[4];

  for(i = 0; i < patch->width; i++)
    {
      ii = i*patch->height*stride;
      jj = ii + ((patch->height-1)*stride);
      for(j = 0; j < patch->height/2; j++)
	{
	  memcpy(t, &(patch->controlv[ii]), stride*sizeof(double));
	  memcpy(&(patch->controlv[ii]), &(patch->controlv[jj]),
		 stride*sizeof(double));
	  memcpy(&(patch->controlv[jj]), t, stride*sizeof(double));
	  ii += stride;
	  jj -= stride;
	}
    }

 return ay_status;
} /* ay_npt_revertv */


/* ay_npt_revertvtcmd:
 *  
 */
int
ay_npt_revertvtcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status;
 char fname[] = "revertV";
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

	  ay_status = ay_npt_revertv(np);
	  break;
	case AY_IDPAMESH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  pm = (ay_pamesh_object *)sel->object->refine;

	  ay_status = ay_pmt_revertv(pm);
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
	  break;
	default:
	  ay_error(AY_ERROR, fname, "Do not know how to revert this object!");
	  break;
	} /* switch */

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_npt_revertvtcmd */


/* ay_npt_wribtrimcurves
 *
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
	    }
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
			}

		      /* copy knots */
		      for(k = 0; k < curve->length + curve->order; k++)
			{
			  knot[c] = (RtFloat)curve->knotv[k];
			  c++;
			}
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
		}
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
    }

  return ay_status;
} /* ay_npt_wribtrimcurves */


/* ay_npt_crtcobbsphere:
 *  create a single patch (out of 6) of a NURBS Cobb Sphere
 *  controls taken from:
 *  "NURB Curves and Surfaces, from Projective Geometry to Practical Use",
 *  by G. Farin
 */
int
ay_npt_crtcobbsphere(ay_nurbpatch_object **patch)
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

  ay_status = ay_npt_create(5, 5, 5, 5,
			    AY_KTNURB, AY_KTNURB, controls, NULL, NULL, &new);

  if(ay_status)
    {
      free(controls);
      return ay_status;
    }

  *patch = new;

 return ay_status;
} /* ay_npt_crtcobbsphere */


/* ay_npt_crtnspheretcmd:
 *  create a simple NURBS Sphere by revolving a half circle
 */
int
ay_npt_crtnspheretcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 int ay_status;
 ay_object *newc = NULL, *o = NULL;
 char fname[] = "create_nsphere";


  if(!(o = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  o->type = AY_IDNPATCH;
  ay_status = ay_object_defaults(o);
  o->parent = AY_TRUE;
  o->hide_children = AY_TRUE;
  ay_status = ay_object_crtendlevel(&(o->down));

  if(!(newc = calloc(1,sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_ERROR;
    }

  newc->type = AY_IDNCURVE;
  ay_status = ay_object_defaults(newc);

  /* first, we create a half circle nurbcurve-object */
  ay_status = ay_nct_crtnhcircle((ay_nurbcurve_object **)(&(newc->refine)));
  if(ay_status)
    {
      free (newc);
      ay_error(ay_status, fname, NULL);
      return TCL_OK;
    }

  ay_status = ay_npt_revolve(newc, 360.0, 0, 0,
			     (ay_nurbpatch_object **)&(o->refine));

  if(ay_status)
    {
      ay_error(ay_status, fname, NULL);
      return TCL_OK;
    }

  /* we do not need the half-circle any longer */
  ay_status = ay_object_delete(newc);

  ay_object_link(o);

 return TCL_OK;
} /* ay_npt_crtnspheretcmd */


/* ay_npt_crtnsphere2tcmd:
 *
 *
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

  for(i=0;i<6;i++)
    {
      if(!(new = calloc(1,sizeof(ay_object))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return TCL_OK;
	}

      new->type = AY_IDNPATCH;
      ay_status = ay_object_defaults(new);
      new->parent = AY_TRUE;
      new->hide_children = AY_TRUE;
      ay_status = ay_object_crtendlevel(&(new->down));

      ay_status = ay_npt_crtcobbsphere(
			(ay_nurbpatch_object **)&(new->refine));
      if(ay_status)
	{
	  ay_object_delete(new);
	  return TCL_ERROR;
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
	  return TCL_OK;
	}

    } /* for */

 return TCL_OK;
} /* ay_npt_crtnsphere2tcmd */


/* ay_npt_splittocurvestcmd:
 *
 *
 */
int
ay_npt_splittocurvestcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL, *new = NULL;
 ay_nurbpatch_object *patch = NULL;
 double *knotv = NULL, *controlv = NULL;
 int knots = 0, stride = 0,  dstlen = 0;
 int u = 0, i = 0, j = 0;
 double m[16];
 char fname[] = "split_to_curves";

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  /* parse args */
  if(argc != 2)
    {
      ay_error(AY_EARGS,fname,"u|v");
      return TCL_OK;
    }

  if(!strcmp(argv[1],"u"))
    u = 1;

  src = sel->object;
  if(src->type != AY_IDNPATCH)
    {
      ay_error(AY_ERROR, fname, "object is not a NURBPatch");
      return TCL_OK;
    }

  patch = src->refine;

  /* get patch transformation-matrix */
  ay_trafo_creatematrix(src, m);

  if(u)
    {
      dstlen = patch->width;
      knots = dstlen+patch->uorder;
      stride = 4;

      for(i=0; i<patch->height; i++)
	{
	  new = NULL;
	  if(!(new = calloc(1, sizeof(ay_object))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }

	  new->type = AY_IDNCURVE;

	  if(!(knotv = calloc(knots, sizeof(double))))
	    {
	      free(new);
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }

	  memcpy(knotv, patch->uknotv, (size_t)(knots*sizeof(double)));

	  if(!(controlv = calloc(stride*dstlen, sizeof(double))))
	    {
	      free(new);
	      free(knotv);
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }

	  for(j=0;j<dstlen;j++)
	    {
	      memcpy(&(controlv[j*stride]),
		     &(patch->controlv[(i+(j*patch->height))*stride]),
		     (size_t)(stride*sizeof(double)));


	      ay_trafo_apply4(&(controlv[j*stride]),m);

	    }

	  ay_status = ay_nct_create(patch->uorder, dstlen, patch->uknot_type,
				    controlv, knotv,
				    (ay_nurbcurve_object **)&(new->refine));

	  if(ay_status)
	    {
	      free(new); free(knotv); free(controlv);
	      ay_error(ay_status, fname, NULL);
	      return TCL_OK;
	    }

	  ay_status = ay_object_defaults(new);
	  ay_status = ay_object_link(new);
	  if(ay_status)
	    {
	      free(new); free(knotv); free(controlv);
	      ay_error(ay_status, fname, NULL);
	      return TCL_OK;
	    }

	  ay_nct_recreatemp((ay_nurbcurve_object *)new->refine);

	} /* for */

    }
  else
    {
      dstlen = patch->height;
      knots = dstlen + patch->vorder;
      stride = 4 * dstlen;

      for(i=0; i<patch->width; i++)
	{
	  new = NULL;
	  if(!(new = calloc(1, sizeof(ay_object))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }

	  new->type = AY_IDNCURVE;

	  knotv = NULL;
	  if(!(knotv = calloc(knots, sizeof(double))))
	    {
	      free(new);
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  memcpy(knotv, patch->vknotv, (size_t)(knots*sizeof(double)));

	  controlv = NULL;
	  if(!(controlv = calloc(stride, sizeof(double))))
	    {
	      free(new); free(knotv);
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }

	  memcpy(controlv, &(patch->controlv[i*stride]),
		 (size_t)(stride*sizeof(double)));

	  for(j=0;j<dstlen;j++)
	    {
	      ay_trafo_apply4(&(controlv[j*4]),m);
	    }

	  ay_status = ay_nct_create(patch->vorder, dstlen, patch->vknot_type,
				    controlv, knotv,
				    (ay_nurbcurve_object **)&(new->refine));
	  if(ay_status)
	    {
	      free(new); free(knotv); free(controlv);
	      ay_error(ay_status, fname, NULL);
	      return TCL_OK;
	    }

	  ay_status = ay_object_defaults(new);
	  ay_status = ay_object_link(new);

	  if(ay_status)
	    {
	      free(new); free(knotv); free(controlv);
	      ay_error(ay_status, fname, NULL);
	      return TCL_OK;
	    }

	} /* for */
    }

 return TCL_OK;
} /* ay_npt_splittocurvestcmd */


/* ay_npt_buildfromcurvestcmd:
 *
 *
 */
int
ay_npt_buildfromcurvestcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *src = NULL, *new = NULL;
 ay_nurbcurve_object *curve = NULL;
 double *newvknotv = NULL, *newcontrolv = NULL;
 int newwidth = 0, newheight = 0;
 int newvorder = 0, newvknots = 0, newuorder = 0;
 int newuknot_type = AY_KTNURB, newvknot_type = 0;
 int i = 0, j = 0, a = 0;
 double m[16];
 char fname[] = "build_from_curves";

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  src = sel->object;
  if(src->type != AY_IDNCURVE)
    {
      ay_error(AY_ERROR, fname, "First object is not a NURBS curve!");
      return TCL_OK;
    }

  curve = src->refine;
  newheight = curve->length;
  newvorder = curve->order;
  newvknots = newheight + newvorder;
  newvknot_type = curve->knot_type;
  if(!(newvknotv = calloc(newvknots, sizeof(double))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  memcpy(newvknotv, curve->knotv, (size_t)(newvknots*sizeof(double)));

  /* parse selection */
  while(sel)
    {
      src = sel->object;
      curve = src->refine;
      if(curve->length >= newheight)
	newwidth++;

      sel = sel->next;
    }

  /* enough curves to form a patch ? */
  if(newwidth < 2)
    {
      ay_error(AY_ERROR, fname,
		 "Not enough suitable curves selected!");
      return TCL_OK;
    }

  /* create new patch */
  if(!(new = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      free(newvknotv);
      return TCL_OK;
    }

  new->type = AY_IDNPATCH;
  ay_status = ay_object_defaults(new);
  new->parent = AY_TRUE;
  new->hide_children = AY_TRUE;
  ay_status = ay_object_crtendlevel(&(new->down));

  if(!(newcontrolv = calloc(newwidth*newheight*4, sizeof(double))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      free(newvknotv); free(new);
      return TCL_OK;
    }
  glMatrixMode(GL_MODELVIEW);

  /* fill newcontrolv */
  sel = ay_selection;
  while(sel)
    {
      src = sel->object;
      curve = src->refine;
      if(curve->length >= newheight)
	{
	  /* get curves transformation-matrix */
	  ay_trafo_creatematrix(src, m);

	  a = 0;
	  for(j = 0; j < newheight; j++)
	    {
	      newcontrolv[i++] = curve->controlv[a++];
	      newcontrolv[i++] = curve->controlv[a++];
	      newcontrolv[i++] = curve->controlv[a++];


	      newcontrolv[i++] = curve->controlv[a++];

	      ay_trafo_apply4(&(newcontrolv[i-4]), m);

	    } /* for */
	} /* if */

      sel = sel->next;
    } /* while */

  if(newwidth < 4)
    newuorder = newwidth;
  else
    newuorder = 4;

  ay_status = ay_npt_create(newuorder, newvorder,
			    newwidth, newheight,
			    newuknot_type, newvknot_type,
			    newcontrolv, NULL, newvknotv,
			    (ay_nurbpatch_object **)&(new->refine));

  if(ay_status)
    {
      ay_error(ay_status, fname, NULL);
      free(new); free(newvknotv); free(newcontrolv);
      return TCL_OK;
    }

  ay_status = ay_object_link(new);

  if(ay_status)
    {
      ay_error(ay_status, fname, NULL);
      ay_object_delete(new);
      return TCL_OK;
    }

 return TCL_OK;
} /* ay_npt_buildfromcurvestcmd */


/* ay_npt_sweep:
 *  sweep cross section o1 along path o2 possibly rotating it,
 *  so that it is always perpendicular to the path, possibly
 *  scaling it by a factor derived from the difference of the
 *  y coordinate of scaling curve o3 to y value 1.0.
 *  Rotation code derived from J. Bloomenthals "Reference Frames"
 *  (Graphic Gems I).
 */
int
ay_npt_sweep(ay_object *o1, ay_object *o2, ay_object *o3, int sections,
	     int rotate, int closed, ay_nurbpatch_object **patch,
	     int has_start_cap, ay_object **start_cap,
	     int has_end_cap, ay_object **end_cap)
{
 int ay_status = AY_OK;
 ay_object *curve = NULL;
 ay_nurbpatch_object *new = NULL;
 ay_nurbcurve_object *tr, *cs, *sf;
 double *controlv = NULL;
 int i = 0, j = 0, a = 0, stride;
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

  if(!o1 || !o2 || !patch)
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
	  a += stride;
	}
    } /* if */

  /* calloc the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(controlv = calloc(cs->length*(sections+1)*stride, sizeof(double))))
    { free(new); ay_status = AY_EOMEM; goto cleanup; }
  if(!(new->vknotv = calloc(cs->length+cs->order, sizeof(double))))
    { free(new); free(controlv); ay_status = AY_EOMEM; goto cleanup; }
  if(!(new->uknotv = calloc(sections+4, sizeof(double))))
    {
      free(new->vknotv); free(new); free(controlv);
      ay_status = AY_EOMEM; goto cleanup;
    }

  new->vorder = cs->order;
  new->uorder = 4;
  new->controlv = controlv;

  new->vknot_type = cs->knot_type;
  new->uknot_type = AY_KTNURB;
  new->width = sections+1;
  new->height = cs->length;

  new->glu_sampling_tolerance = cs->glu_sampling_tolerance;

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
    plensf = fabs(sf->knotv[sf->length] - sf->knotv[sf->order-1]);

  T0[0] = 1.0;
  T0[1] = 0.0;
  T0[2] = 0.0;

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
	  if(p3[1] > AY_EPSILON)
	    ay_trafo_scalematrix(1.0/p3[1], 1.0/p3[1], 1.0/p3[1], m);
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
	      (fabs(fabs(T1[2])-fabs(T0[2])) > AY_EPSILON)))
	    {
	      AY_V3CROSS(A,T0,T1)
	      len = AY_V3LEN(A);
	      AY_V3SCAL(A,(1.0/len))

	      rots[i*4+0] = AY_R2D(acos(AY_V3DOT(T0,T1)));
	      memcpy(&rots[i*4+1], A, 3*sizeof(double));

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
	  ay_trafo_apply4(&controlv[i*cs->length*stride+j*stride], mi);
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
		  if(p3[1] > AY_EPSILON)
		    {
		      (*start_cap)->scalx *= p3[1];
		      (*start_cap)->scaly *= p3[1];
		    }
		} /* if */
	      /* fix direction for aycsg */
	      (*start_cap)->scalz *= -1.0;
	      /* rotate it */
	      if(rotate)
		{
		  if(fabs(rots[0]) > AY_EPSILON)
		    {
		      ay_quat_axistoquat(&(rots[1]), AY_D2R(-rots[0]), quat);
		      ay_quat_add(quat, (*start_cap)->quat,
				  (*start_cap)->quat);
		    } /* if */
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
		  if(p3[1] > AY_EPSILON)
		    {
		      (*end_cap)->scalx *= p3[1];
		      (*end_cap)->scaly *= p3[1];
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
	    } /* if */
	} /* if */

    } /* for */

  /* return result */
  *patch = new;

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


/* ay_npt_birail1:
 *  sweep cross section o1 along rails o2 and o3;
 *  Rotation code derived from J. Bloomenthals "Reference Frames"
 *  (Graphic Gems I).
 */
int
ay_npt_birail1(ay_object *o1, ay_object *o2, ay_object *o3, int sections,
	       int closed, ay_nurbpatch_object **patch,
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

  if(!o1 || !o2 || !o3 || !patch ||
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

  /* calloc the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(controlv = calloc(cs->length*(sections+1)*stride, sizeof(double))))
    { free(new); ay_status = AY_EOMEM; goto cleanup; }
  if(!(new->vknotv = calloc(cs->length+cs->order, sizeof(double))))
    { free(new); free(controlv); ay_status = AY_EOMEM; goto cleanup; }
  if(!(new->uknotv = calloc(sections+4, sizeof(double))))
    {
      free(new->vknotv); free(new); free(controlv);
      ay_status = AY_EOMEM; goto cleanup;
    }

  new->vorder = cs->order;
  new->uorder = 4;
  new->controlv = controlv;

  new->vknot_type = cs->knot_type;
  new->uknot_type = AY_KTNURB;
  new->width = sections+1;
  new->height = cs->length;

  new->glu_sampling_tolerance = cs->glu_sampling_tolerance;

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
    {
      free(new->uknotv); free(new->vknotv); free(new); free(controlv);
      ay_status = AY_EOMEM; goto cleanup;
    }

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
	  AY_V3CROSS(A,T0,T1)
	  lent0 = AY_V3LEN(A);
	  AY_V3SCAL(A,(1.0/lent0))

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
	} /* if */
    } /* if */

  /* return result */
  *patch = new;

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

 return ay_status;
} /* ay_npt_birail1 */


/* ay_npt_birail2:
 *  sweep cross section o1 along rails o2 and o3;
 *  Rotation code derived from J. Bloomenthals "Reference Frames"
 *  (Graphic Gems I).
 */
int
ay_npt_birail2(ay_object *o1, ay_object *o2, ay_object *o3, ay_object *o4,
	       int sections, int closed, ay_nurbpatch_object **patch,
	       int has_start_cap, ay_object **start_cap,
	       int has_end_cap, ay_object **end_cap)
{
 int ay_status = AY_OK;
 ay_object *curve = NULL, *ot = NULL;
 ay_nurbpatch_object *new = NULL;
 ay_nurbcurve_object *cs1, *cs2, *r1, *r2, *tc;
 double *controlv = NULL;
 int i = 0, j = 0, a = 0, stride;
 double u, p1[4], p2[4], p5[4], p6[4], p7[4], p8[4];
 double T0[3] = {0.0,0.0,-1.0};
 double T1[3] = {0.0,0.0,0.0};
 double A[3] = {0.0,0.0,0.0};
 double lent0 = 0.0, lent1 = 0.0, lentn = 0.0, plenr1 = 0.0, plenr2 = 0.0;
 double m[16] = {0}, mi[16] = {0}, mcs[16], mr1[16], mr2[16];
 double mr[16], mrs[16];
 /*double quat[4] = {0};*/
 double *cs1cv = NULL, *cs2cv = NULL, *r1cv = NULL, *r2cv = NULL, *rots = NULL;
 double scalx, scaly, scalz;

  if(!o1 || !o2 || !o3 || !o4 || !patch ||
     (has_start_cap && !start_cap) || (has_end_cap && !end_cap))
    return AY_ENULL;

  if((o1->type != AY_IDNCURVE) || (o2->type != AY_IDNCURVE) ||
     (o3->type != AY_IDNCURVE) || (o4->type != AY_IDNCURVE))
    return AY_OK;

  cs1 = (ay_nurbcurve_object *)(o1->refine);
  r1 = (ay_nurbcurve_object *)(o2->refine);
  r2 = (ay_nurbcurve_object *)(o3->refine);
  cs2 = (ay_nurbcurve_object *)(o4->refine);
  
  stride = 4;

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

  /* calloc the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(controlv = calloc(cs1->length*(sections+1)*stride, sizeof(double))))
    { free(new); ay_status = AY_EOMEM; goto cleanup; }
  if(!(new->vknotv = calloc(cs1->length+cs1->order, sizeof(double))))
    { free(new); free(controlv); ay_status = AY_EOMEM; goto cleanup; }
  if(!(new->uknotv = calloc(sections+4, sizeof(double))))
    {
      free(new->vknotv); free(new); free(controlv);
      ay_status = AY_EOMEM; goto cleanup;
    }

  new->vorder = cs1->order;
  new->uorder = 4;
  new->controlv = controlv;

  new->vknot_type = cs1->knot_type;
  new->uknot_type = AY_KTNURB;
  new->width = sections+1;
  new->height = cs1->length;

  new->glu_sampling_tolerance = cs1->glu_sampling_tolerance;

  ay_status = ay_knots_createnp(new);
  if(ay_status)
    {
      free(new->uknotv); free(new->vknotv); free(new); free(controlv);
      ay_status = AY_EOMEM; goto cleanup;
    }

  if(cs1->knot_type == AY_KTCUSTOM)
    {
      memcpy(new->vknotv,cs1->knotv,(cs1->length+cs1->order)*sizeof(double));
    }

  /* calculate last point of rail1 and rail2 */
  ay_nb_CurvePoint4D(r1->length-1, r1->order-1, r1->knotv, r1cv,
		     r1->knotv[r1->length], p1);

  ay_nb_CurvePoint4D(r2->length-1, r2->order-1, r2->knotv, r2cv,
		     r2->knotv[r2->length], p2);
  /* from rail endpoints calculate distance between them
     (this is needed to calculate the correct scale factor
     for the interpolated cross sections later on) */
  AY_V3SUB(T0, p2, p1)
  lentn = AY_V3LEN(T0);

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
    {
      free(new->uknotv); free(new->vknotv); free(new); free(controlv);
      ay_status = AY_EOMEM; goto cleanup;
    }

  /* copy first section */
  memcpy(&(controlv[0]), &(cs1cv[0]), cs1->length * stride * sizeof(double));

  /* copy cross sections controlv section times and sweep it along the rails */
  for(i = 1; i < sections; i++)
    {
      ot = NULL;

      ay_status = ay_interpol_ncurves((double)i/sections, o1, o4, &ot);
      if(ay_status)
	goto cleanup;

      tc = (ay_nurbcurve_object*)(ot->refine);
      
      memcpy(&(controlv[i * stride * cs1->length]), &(tc->controlv[0]),
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
	  AY_V3CROSS(A,T0,T1)
	  lent0 = AY_V3LEN(A);
	  AY_V3SCAL(A,(1.0/lent0))

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
	} /* if */
    } /* if */

  /* return result */
  *patch = new;

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

 return ay_status;
} /* ay_npt_birail2 */


/* ay_npt_interpolateu:
 *
 *
 */
int
ay_npt_interpolateu(ay_nurbpatch_object *patch, int order)
{
 int ay_status = AY_OK;
 char fname[] = "npt_interpolateu";
 int i, k, N, K, stride, ind, ind2, pu;
 double *uk = NULL, *d = NULL, *Pw = NULL, v[3] = {0};
 double *U = NULL, *Q = NULL;

  K = patch->width;
  N = patch->height;
  stride = 4;
  Pw = patch->controlv;
  pu = order-1;

  if(!(uk = calloc(K, sizeof(double))))
    return AY_EOMEM;

  if(!(d = calloc(N, sizeof(double))))
    {
      free(uk); return AY_EOMEM;
    }

  if(!(U = calloc(K+patch->uorder, sizeof(double))))
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
	}

      uk[k] /= N;
      uk[k] += uk[k-1];
    }
  uk[K-1] = 1.0;


  for(i = 1; i < (K-pu); i++)
    {
      U[i+pu] = 0.0;

      for(k = i; k < (i+pu); k++)
	{
	  U[i+pu] += uk[k];
	}

      U[i+pu] /= pu;
    }
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
	}

      ay_status = ay_nb_GlobalInterpolation4D(K-1, Q, uk, U, pu);

      if(ay_status)
	{ free(d); free(uk); free(U); free(Q); return ay_status; }

      ind = i*stride;
      for(k = 0; k < K; k++)
	{
	  memcpy(&(Pw[ind]), &(Q[k*4]), stride*sizeof(double));
	  ind += N*stride;
	}
    }

  free(patch->uknotv);
  patch->uknotv = U;
  patch->uknot_type = AY_KTCUSTOM;
  patch->uorder = pu+1;

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
ay_npt_interpolatev(ay_nurbpatch_object *patch, int order)
{
 int ay_status = AY_OK;
 char fname[] = "npt_interpolatev";
 int i, k, N, K, stride, ind, ind2, pv;
 double *vk = NULL, *d = NULL, *Pw = NULL, v[3] = {0};
 double *V = NULL;

  K = patch->width;
  N = patch->height;
  stride = 4;
  Pw = patch->controlv;
  pv = patch->vorder-1;

  if(!(vk = calloc(N, sizeof(double))))
    return AY_EOMEM;

  if(!(d = calloc(K, sizeof(double))))
    {
      free(vk); return AY_EOMEM;
    }

  if(!(V = calloc(N+patch->vorder, sizeof(double))))
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
	}

      vk[k] /= K;
      vk[k] += vk[k-1];
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
    }
  for(i = 0; i <= pv; i++)
    V[i] = 0.0;
  for(i = (N/*-pu-1*/); i < (N+pv+1); i++)
    V[i] = 1.0;

  /* interpolate */
  for(i = 0; i < K; i++)
    {
      ind = i*N*stride;

      ay_status = ay_nb_GlobalInterpolation4D(N-1,
			&(patch->controlv[ind]), vk, V, pv);

      if(ay_status)
	{ free(d); free(vk); free(V); return ay_status; }

    } /* for */

  free(patch->vknotv);
  patch->vknotv = V;
  patch->vknot_type = AY_KTCUSTOM;
  patch->vorder = pv+1;

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
	}
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
ay_npt_extrude(double height, ay_object *o,
	       ay_nurbpatch_object **patch)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *new = NULL;
 ay_nurbcurve_object *curve;
 double vknots[4] = {0.0, 0.0, 1.0, 1.0}; /* uknots are taken from curve! */
 double *uknotv = NULL, *vknotv = NULL, *controlv = NULL;
 double x,y,z,w;
 int j=0,a=0,b=0;
 double m[16], point[4] = {0};

  if(!o)
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
  if(!(vknotv = calloc(4, sizeof(double))))
    { free(new); free(controlv); return AY_EOMEM; }
  if(!(uknotv = calloc(curve->length+curve->order,sizeof(double))))
    { free(new); free(controlv); free(vknotv); return AY_EOMEM; }

  memcpy(uknotv,curve->knotv,(curve->length+curve->order)*sizeof(double));
  new->uknotv = uknotv;
  memcpy(vknotv,vknots,4*sizeof(double));
  new->vknotv = vknotv;
  new->uorder = curve->order;
  new->vorder = 2; /* linear! */
  new->uknot_type = curve->knot_type;
  new->vknot_type = AY_KTCUSTOM;
  new->width = curve->length;
  new->height = 2;
  new->glu_sampling_tolerance = curve->glu_sampling_tolerance;

  /* fill controlv */
  a = 0;
  b = 0;
  for(j = 0; j < curve->length; j++)
    {

      /*      memcpy(point, &(curve->controlv[a]), 4*sizeof(GLdouble));*/

      /* transform point */
      x = curve->controlv[a];
      y = curve->controlv[a+1];
      z = curve->controlv[a+2];
      w = curve->controlv[a+3];

      point[0] = m[0]*x + m[4]*y + m[8]*z + m[12]*w;
      point[1] = m[1]*x + m[5]*y + m[9]*z + m[13]*w;
      point[2] = 0.0;
      point[3] = m[3]*x + m[7]*y + m[11]*z + m[15]*w;

      /* build a profile */
      controlv[b] = point[0];
      controlv[b+1] = point[1];
      controlv[b+2] = point[2];
      controlv[b+3] = w;

      b+=4;

      controlv[b] = point[0];
      controlv[b+1] = point[1];
      controlv[b+2] = point[2]+(height*w);
      controlv[b+3] = w;

      b += 4;

      a += 4;

    } /* for */

  new->controlv = controlv;

  *patch = new;

 return ay_status;
} /* ay_npt_extrude */


/* ay_npt_gettangentfromcontrol:
 *
 *
 */
int
ay_npt_gettangentfromcontrol(int closed, int n, int p,
			     int stride, double *P, int a, double *t)
{
 int ay_status = AY_OK;
 int found = AY_FALSE, wrapped = AY_FALSE;
 int b, i1, i2;
 int before, after;
 double l;

  if(closed)
    {
      if(a == 0)
	a = (n-p);
      if(a == n)
	a = p;
      if(a > (n-p))
	a -= (n-p);
    }

  /* find a good point after P[a] */
  b = a+1;
  while(!found)
    {
      if(b >= n)
	{
	  if(wrapped)
	    return AY_ERROR;
	  wrapped = AY_TRUE;
	  b = 0;
	}

      i1 = a*stride;
      i2 = b*stride;
      if((P[i1] != P[i2]) || (P[i1+1] != P[i2+1]))
	{
	  found = AY_TRUE;
	}
      else
	{
	  b++;
	}

    }

  after = b;

  /* find a good point before P[a] */
  found = AY_FALSE;
  wrapped = AY_FALSE;
  b = a-1;
  while(!found)
    {
      if(b < 0)
	{
	  if(wrapped)
	    return AY_ERROR;
	  wrapped = AY_TRUE;
	  b = (n-1);
	}

      i1 = a*stride;
      i2 = b*stride;
      if((P[i1] != P[i2]) || (P[i1+1] != P[i2+1]))
	{
	  found = AY_TRUE;
	}
      else
	{
	  b--;
	}

    }

  before = b;

  /* now calc the tangent */
  t[0] = P[after*stride] - P[before*stride];
  t[1] = P[(after*stride)+1] - P[(before*stride)+1];

  /* normalize tangent vector */
  l = sqrt(t[0]*t[0]+t[1]*t[1]);
  if(l > AY_EPSILON)
    {
      t[0] /= l;
      t[1] /= l;
    }

  return ay_status;
} /* ay_npt_gettangentfromcontrol */


/* ay_npt_bevel:
 *
 *
 */
int
ay_npt_bevel(int type, double radius, ay_object *o,
	     ay_nurbpatch_object **patch)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *new = NULL;
 ay_nurbcurve_object *curve = NULL;
 double uknots_round[6] = {0.0, 0.0, 0.0, 1.0, 1.0, 1.0};
 double uknots_linear[4] = {0.0, 0.0, 1.0, 1.0};
 double uknots_ridge[8] = {0.0, 0.0, 0.0, 0.25, 0.75, 1.0, 1.0, 1.0};
 /* vknots are taken from curve! */
 double *uknotv = NULL, *vknotv = NULL, *controlv = NULL;
 double x,y,z,w;
 double tangent[3]={0}, normal[3]={0}, zaxis[3]={0.0,0.0,-1.0};
 double ww = sqrt(2.0)/2.0, displacex, displacey;
 int i=0, j=0, a=0, b=0, k = 0;
 double m[16], point[4] = {0};

 curve = (ay_nurbcurve_object *)o->refine;

  /* calloc the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    return AY_EOMEM;
  if(type == 0)
    {
      if(!(controlv = calloc(4*3*curve->length, sizeof(double))))
	{ free(new); return AY_EOMEM; }
      if(!(uknotv = calloc(6, sizeof(double))))
	{ free(new); free(controlv); return AY_EOMEM; }
      memcpy(uknotv,uknots_round,6*sizeof(double));
      new->uorder = 3;
      new->width = 3;
      new->uknot_type = AY_KTNURB;
    }
  if(type == 1)
    {
      if(!(controlv = calloc(4*2*curve->length, sizeof(double))))
	{ free(new); return AY_EOMEM; }
      if(!(uknotv = calloc(4, sizeof(double))))
	{ free(new); free(controlv); return AY_EOMEM; }
      memcpy(uknotv,uknots_linear,4*sizeof(double));
      new->uorder = 2;
      new->width = 2;
      new->uknot_type = AY_KTNURB;
    }
  if(type == 2)
    {
      if(!(controlv = calloc(4*5*curve->length, sizeof(double))))
	{ free(new); return AY_EOMEM; }
      if(!(uknotv = calloc(8, sizeof(double))))
	{ free(new); free(controlv); return AY_EOMEM; }
      memcpy(uknotv,uknots_ridge,8*sizeof(double));
      new->uorder = 3;
      new->width = 5;
      new->uknot_type = AY_KTCUSTOM;
    }

  if(!(vknotv = calloc(curve->length+curve->order,sizeof(double))))
    { free(new); free(controlv); free(vknotv); return AY_EOMEM; }
  memcpy(vknotv,curve->knotv,(curve->length+curve->order)*sizeof(double));
  new->vknotv = vknotv;
  new->uknotv = uknotv;
  new->vorder = curve->order;
  new->vknot_type = curve->knot_type;
  new->height = curve->length;
  new->glu_sampling_tolerance = curve->glu_sampling_tolerance;

  ay_trafo_creatematrix(o, m);

  /* fill controlv */
  /* first loop */
  b = 0;
  for(i = 0; i < new->width; i++)
    {
      a = 0;
      for(j = 0; j < curve->length; j++)
	{
	  memcpy(&controlv[b],&curve->controlv[a],4*sizeof(double));
	  ay_trafo_apply4(&controlv[b],m);
	  a+=4;

	  controlv[b+2]=0.0;

	  b+=4;
	}
    }
  b = curve->length*4;
  /* transform second loop */
  if(type == 0)
    {
      /*
      if(curve->knot_type == AY_KTBSPLINE)
	{
	  x = controlv[(curve->length-curve->order)*4];
	  y = controlv[(curve->length-curve->order)*4+1];
	  z = controlv[(curve->length-curve->order)*4+2];
	  w = controlv[(curve->length-curve->order)*4+3];
	}
      else
	{
	  x = controlv[(curve->length-2)*4];
	  y = controlv[(curve->length-2)*4+1];
	  z = controlv[(curve->length-2)*4+2];
	  w = controlv[(curve->length-2)*4+3];
	}
      */
      for(j = 0; j < curve->length; j++)
	{
	  /* get displacement direction */
	  /*
	  if(j == (curve->length-1))
	    {
	      if(curve->knot_type == AY_KTBSPLINE)
		{
		  tangent[0] = x-controlv[(curve->order-1)*4];
		  tangent[1] = y-controlv[(curve->order-1)*4+1];
		}
	      else
		{
		  tangent[0] = x-controlv[4];
		  tangent[1] = y-controlv[5];
		}
	    }
	  else
	    {
	      tangent[0] = x-controlv[b+4];
	      tangent[1] = y-controlv[b+5];
	    }
	  */

	  ay_npt_gettangentfromcontrol(curve->closed, curve->length,
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

	  controlv[b] = point[0]*(w*ww);
	  controlv[b+1] = point[1]*(w*ww);
	  controlv[b+2] = point[2]*(w*ww);

	  controlv[b+3] = ww*w;

	  b+=4;
	}
    }

  if(type == 2)
    {
      /* transform the middle 3 loops */
      for(k=0;k<3;k++)
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
	  /*
	  if(curve->knot_type == AY_KTBSPLINE)
	    {
	      x = controlv[(curve->length-curve->order)*4];
	      y = controlv[(curve->length-curve->order)*4+1];
	      z = controlv[(curve->length-curve->order)*4+2];
	      w = controlv[(curve->length-curve->order)*4+3];
	    }
	  else
	    {
	      x = controlv[(curve->length-2)*4];
	      y = controlv[(curve->length-2)*4+1];
	      z = controlv[(curve->length-2)*4+2];
	      w = controlv[(curve->length-2)*4+3];
	    }
	  */
	  for(j = 0; j < curve->length; j++)
	    {
	      /* get displacement direction */
	      /*
	      if(j == (curve->length-1))
		{
		  if(curve->knot_type == AY_KTBSPLINE)
		    {
		      tangent[0] = x-controlv[(curve->order-1)*4];
		      tangent[1] = y-controlv[(curve->order-1)*4+1];
		    }
		  else
		    {
		      tangent[0] = x-controlv[4];
		      tangent[1] = y-controlv[5];
		    }
		}
	      else
		{
		  tangent[0] = x-controlv[b+4];
		  tangent[1] = y-controlv[b+5];
		}
	      */
	      ay_npt_gettangentfromcontrol(curve->closed, curve->length,
					   curve->order-1, 4, controlv, j,
					   tangent);
	      w = controlv[b+3];
	      x = controlv[b];
	      y = controlv[b+1];
	      z = controlv[b+2];

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

	      controlv[b] = point[0]*ww;
	      controlv[b+1] = point[1]*ww;
	      controlv[b+2] = point[2]*ww;

	      controlv[b+3] = w*ww;

	      b+=4;
	    } /* for */
	} /* for */
    } /* if */

  /* transform last loop */
  /*
  if(curve->knot_type == AY_KTBSPLINE)
    {
      x = controlv[(curve->length-curve->order)*4];
      y = controlv[(curve->length-curve->order)*4+1];
      z = controlv[(curve->length-curve->order)*4+2];
      w = controlv[(curve->length-curve->order)*4+3];
    }
  else
    {
      x = controlv[(curve->length-2)*4];
      y = controlv[(curve->length-2)*4+1];
      z = controlv[(curve->length-2)*4+2];
      w = controlv[(curve->length-2)*4+3];
    }
  */
  for(j = 0; j < curve->length; j++)
    {
      /* get displacement direction */
      /*
      if(j == (curve->length-1))
	{
	  if(curve->knot_type == AY_KTBSPLINE)
	    {
	      tangent[0] = x-controlv[(curve->order-1)*4];
	      tangent[1] = y-controlv[(curve->order-1)*4+1];
	    }
	  else
	    {
	      tangent[0] = x-controlv[4];
	      tangent[1] = y-controlv[5];
	    }
	}
      else
	{
	  tangent[0] = x-controlv[b+4];
	  tangent[1] = y-controlv[b+5];
	}
      */

      ay_npt_gettangentfromcontrol(curve->closed, curve->length,
				   curve->order-1, 4, controlv, j, tangent);

      x = controlv[b];
      y = controlv[b+1];
      z = controlv[b+2];
      w = controlv[b+3];

      AY_V3CROSS(normal,tangent,zaxis)
      AY_V3SCAL(normal, radius)

      /* create transformation matrix */
      ay_trafo_identitymatrix(m);
      ay_trafo_translatematrix(normal[0], normal[1], radius*w, m);

      /* transform point */
      point[0] = m[0]*x + m[4]*y + m[8]*z + m[12]*1.0;
      point[1] = m[1]*x + m[5]*y + m[9]*z + m[13]*1.0;
      point[2] = m[2]*x + m[6]*y + m[10]*z + m[14]*1.0;
      /*point[3] = m[3]*x + m[7]*y + m[11]*z + m[15]*1.0;*/

      controlv[b] = point[0];
      controlv[b+1] = point[1];
      controlv[b+2] = point[2];
      controlv[b+3] = w;

      b+=4;
    }

  new->controlv = controlv;

  *patch = new;

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
 ay_nurbpatch_object *new = NULL;
 double knotv[4] = {0.0,0.0,1.0,1.0};
 double minx, miny, maxx, maxy, angle;
 int i, stride;

  /* calloc the new patch */
  if(!(new = calloc(1, sizeof(ay_nurbpatch_object))))
    return AY_EOMEM;
  if(!(new->vknotv = calloc(4, sizeof(double))))
    { free(new); return AY_EOMEM; }
  if(!(new->uknotv = calloc(4, sizeof(double))))
    { free(new); free(new->vknotv); return AY_EOMEM; }
  if(!(new->controlv = calloc(4*4, sizeof(double))))
    { free(new); free(new->vknotv); free(new->uknotv); return AY_EOMEM; }

  new->width = 2;
  new->height = 2;
  new->uorder = 2;
  new->vorder = 2;
  memcpy(new->uknotv,knotv,4*sizeof(double));
  memcpy(new->vknotv,knotv,4*sizeof(double));

  i = 0;
  minx = curve->controlv[0]; maxx = minx;
  miny = curve->controlv[1]; maxy = miny;
  angle = 0.0;
  stride = 4;
  while(i<curve->length*stride)
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
      if((i<(curve->length-1)*stride)&&(i>stride))
	{
	  angle +=
	    ((curve->controlv[i+stride] - curve->controlv[i-stride])*
	     (curve->controlv[i+1] - curve->controlv[i+1-stride])) -
	    ((curve->controlv[i] - curve->controlv[i-stride])*
	     (curve->controlv[i+stride+1] - curve->controlv[i+1-stride]));
	}


      i+=stride;
    }

  new->controlv[0] = minx;
  new->controlv[1] = miny;

  new->controlv[4] = minx;
  new->controlv[5] = maxy;

  new->controlv[8] = maxx;
  new->controlv[9] = miny;

  new->controlv[12] = maxx;
  new->controlv[13] = maxy;

  for(i=2;i<=15;i+=4)
    {
      new->controlv[i] = z;
      new->controlv[i+1] = 1.0;
    }

  *ominx = minx;
  *omaxx = maxx;
  *ominy = miny;
  *omaxy = maxy;
  *oangle = angle;

  *cap = new;

 return ay_status;
} /* ay_npt_createcap */


/* ay_npt_getpntfromindex:
 *
 *
 */
int
ay_npt_getpntfromindex(ay_nurbpatch_object *patch, int indexu, int indexv,
		       double **p)
{
 int stride = 4;
 char fname[] = "ay_npt_getpntfromindex";

  if(indexu >= patch->width || indexu < 0)
    {
      ay_error(AY_ERROR, fname, "index u out of range");
      return TCL_OK;
    }

  if(indexv >= patch->height || indexv < 0)
    {
      ay_error(AY_ERROR, fname, "index v out of range");
      return TCL_OK;
    }

  *p = &(patch->controlv[(indexu*patch->width+indexv)*stride]);

 return TCL_OK;
} /* ay_npt_getpntfromindex */


/* ay_npt_elevateu:
 *
 */
int
ay_npt_elevateu(ay_nurbpatch_object *patch, int t)
{
 int ay_status = AY_OK;
 double *Uh = NULL, *Qw = NULL, *realQw = NULL, *realUh = NULL;
 int nw = 0;
 char fname[] = "ay_npt_elevateu";

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
	    ay_selp_clear(sel->object);

	  patch = (ay_nurbpatch_object *)sel->object->refine;
	  ay_status = ay_npt_elevateu(patch, t);
	}
      else
	{
	  ay_error(AY_ERROR, fname, "object is not a NPatch");
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
 double *Vh = NULL, *Qw = NULL, *realQw = NULL, *realVh = NULL;
 int nh = 0, i, ind1, ind2;
 char fname[] = "ay_npt_elevatev";

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
	    ay_selp_clear(sel->object);

	  patch = (ay_nurbpatch_object *)sel->object->refine;
	  ay_status = ay_npt_elevatev(patch, t);
	}
      else
	{
	  ay_error(AY_ERROR, fname, "object is not a NPatch");
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
 int ay_status;
 char fname[] = "swapUV";
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

	  ay_status = ay_npt_swapuv(np);
	  break;
	case AY_IDPAMESH:
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  pm = (ay_pamesh_object *)sel->object->refine;

	  ay_status = ay_pmt_revertu(pm);
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
	  break;
	default:
	  ay_error(AY_ERROR, fname, "Do not know how to revert this object!");
	  break;
	} /* switch */
      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_npt_swapuvtcmd */


/* ay_npt_gordon:
 *
 */
int
ay_npt_gordon(ay_object *cu, ay_object *cv, ay_object *in,
	      int uorder, int vorder,
	      ay_nurbpatch_object **gordon)
{
 int ay_status = AY_OK;
 char fname[] = "ay_npt_gordon";
 ay_object *c;
 ay_object *lcu = NULL, *lcv = NULL; /* last cu/cv curve */
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
      lcv = c;
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
      ay_nb_CurvePoint4D(nc->length-1, nc->order-1, nc->knotv, nc->controlv,
			 nc->knotv[nc->order-1], &(intersections[i]));
      i = (numcv-1)*numcu*4;
      ay_nb_CurvePoint4D(nc->length-1, nc->order-1, nc->knotv, nc->controlv,
			 nc->knotv[nc->length], &(intersections[i]));

      nc = (ay_nurbcurve_object *)lcu->refine;
      i = (numcu-1)*4;
      ay_nb_CurvePoint4D(nc->length-1, nc->order-1, nc->knotv, nc->controlv,
			 nc->knotv[nc->order-1], &(intersections[i]));
      i = (numcu*numcv-1)*4;
      ay_nb_CurvePoint4D(nc->length-1, nc->order-1, nc->knotv, nc->controlv,
			 nc->knotv[nc->length], &(intersections[i]));

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

  if(skinu->uorder > uo)
    uo = skinu->uorder;
  if(skinv->uorder > uo)
    uo = skinv->uorder;
  if(interpatch->uorder > uo)
    uo = interpatch->uorder;

  if(skinu->vorder > vo)
    vo = skinu->vorder;
  if(skinv->vorder > vo)
    vo = skinv->vorder;
  if(interpatch->vorder > vo)
    vo = interpatch->vorder;

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

  ay_status = ay_knots_mergesurf(skinu, unifiedU, uUlen, unifiedV, uVlen);
  
  ay_status = ay_knots_mergesurf(skinv, unifiedU, uUlen, unifiedV, uVlen);
  
  ay_status = ay_knots_mergesurf(interpatch, unifiedU, uUlen, unifiedV, uVlen);

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
	  /*
	  skinu->controlv[k+3] += skinv->controlv[k+3];
	  skinu->controlv[k+3] -= interpatch->controlv[k+3];
	  */
	  skinu->controlv[k+3] = 1.0;
	} /* for */
    } /* for */

  *gordon = skinu;
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
		}
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
		}
	    }
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


/* ay_npt_extractnc:
 *  extract a NURBS curve from the NURBS patch <npatch>
 *  side: specifies extraction of a boundary curve (0-3) or of a curve at
 *   a specific parametric value (4 - along u dimension, 5 - along v dimension)
 *  param: parametric value at which curve is extracted; this parameter is
 *   ignored for the extraction of boundary curves
 */
int
ay_npt_extractnc(ay_object *npatch, int side, double param,
		 ay_nurbcurve_object **result)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *np  = NULL;
 ay_nurbcurve_object *nc = NULL;
 double *cv, m[16];
 int stride = 4, i, a;

  if(!npatch || !result)
    return AY_ENULL;

  if(!npatch->type == AY_IDNPATCH)
    return AY_ERROR;

  if(!(nc = calloc(1, sizeof(ay_nurbcurve_object))))
    return AY_EOMEM;
  
  np = (ay_nurbpatch_object *)npatch->refine;

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
    default:
      ay_status = AY_ERROR;
      goto cleanup;
    } /* switch */

  ay_trafo_creatematrix(npatch, m);

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
    default:
      ay_status = AY_ERROR;
      goto cleanup;
    } /* switch */

  /* apply trafos */
  cv = nc->controlv;
  a = 0;
  for(i = 0; i < nc->length; i++)
    {
      ay_trafo_apply4(&(cv[a]), m);
      a += stride;
    }

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

 return ay_status;
} /* ay_npt_extractnc */
