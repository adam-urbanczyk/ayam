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

/* types local to this module */
typedef struct ay_npt_tesstri_s {
  struct ay_npt_tesstri_s *next;

  double p1[3];
  double p2[3];
  double p3[3];

  double n1[3];
  double n2[3];
  double n3[3];

} ay_npt_tesstri;


typedef struct ay_npt_tessobject_s {
  GLenum type;

  int count;
  int startup;

  double *p1;
  double *p2;
  double *p3;
  double *p4;

  double *n1;
  double *n2;
  double *n3;
  double *n4;

  double **nextpd;
  double **nextnd;

  struct ay_npt_tesstri_s *tris;
} ay_npt_tessobject;


/* prototypes of functions local to this module */
int ay_npt_tpmchecktri(double *p1, double *p2, double *p3);

void ay_npt_tpmbegindata(GLenum type, void *userData);

void ay_npt_tpmvertexdata(GLfloat *vertex, void *userData);

void ay_npt_tpmnormaldata(GLfloat *normal, void *userData);

void ay_npt_tpmenddata(void *userData);


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
ay_npt_revolve(ay_object *o, double arc, ay_nurbpatch_object **patch)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *new = NULL;
 ay_nurbcurve_object *curve;
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
  new->vorder = 3;
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
	}

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
} /* ay_npt_drawtrim */


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

  ay_status = ay_npt_revolve(newc, 360.0,
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
      memcpy(&controlv[i * stride * cs->length], cscv,
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
      mi[15] = 1.0;

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


/* ay_npt_interpolateu:
 *
 *
 */
int
ay_npt_interpolateu(ay_nurbpatch_object *patch)
{
 int ay_status = AY_OK;
 char fname[] = "npt_interpolateu";
 int i, k, N, K, stride, ind, ind2, pu;
 double *vk = NULL, *d = NULL, *Pw = NULL, v[3] = {0};
 double *V = NULL, *Q = NULL;

  K = patch->width;
  N = patch->height;
  stride = 4;
  Pw = patch->controlv;
  pu = patch->uorder-1;

  if(!(vk = calloc(K>N?K:N, sizeof(double))))
    return AY_EOMEM;

  if(!(d = calloc(N, sizeof(double))))
    {
      free(vk); return AY_EOMEM;
    }

  if(!(V = calloc(K+patch->uorder, sizeof(double))))
    {
      free(vk); free(d); return AY_EOMEM;
    }

  if(!(Q = calloc(K*4, sizeof(double))))
    {
      free(vk); free(d); free(V); return AY_EOMEM;
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
	      free(vk); free(d); free(V); return AY_OK;
	    }
	  ind2 += N*stride;
	}

    }

  /* create knotv */
  ind = N*stride;
  ind2 = 0;
  vk[0] = 0.0;
  for(k = 1; k < K; k++)
    {
      vk[k] = 0.0;
      for(i = 0; i < N; i++)
	{
	  v[0] = Pw[ind]   - Pw[ind2];
	  v[1] = Pw[ind+1] - Pw[ind2+1];
	  v[2] = Pw[ind+2] - Pw[ind2+2];

	  vk[k] += (AY_V3LEN(v)/d[i]);
	  ind += stride;
	  ind2 += stride;
	}

      vk[k] /= N;
      vk[k] += vk[k-1];
    }
  vk[(K>N?K:N)-1] = 1.0;


  for(i = 1; i < (K-pu); i++)
    {
      V[i+pu] = 0.0;

      for(k = i; k < (i+pu); k++)
	{
	  V[i+pu] += vk[k];
	}

      V[i+pu] /= pu;
    }
  for(i = 0; i <= pu; i++)
    V[i] = 0.0;
  for(i = (K/*-pu-1*/); i < (K+patch->uorder); i++)
    V[i] = 1.0;

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

      ay_status = ay_nb_GlobalInterpolation4D(K-1, Q, vk, V, pu);

      if(ay_status)
	{ free(d); free(vk); free(V); free(Q); return ay_status; }

      ind = i*stride;
      for(k = 0; k < K; k++)
	{
	  memcpy(&(Pw[ind]), &(Q[k*4]), stride*sizeof(double));
	  ind += N*stride;
	}
    }

  free(patch->uknotv);
  patch->uknotv = V;
  patch->uknot_type = AY_KTCUSTOM;

  free(vk);
  free(d);
  free(Q);

 return AY_OK;
} /* ay_npt_interpolateu */


/* ay_npt_skin:
 *
 *
 */
int
ay_npt_skin(ay_object *curves, int order, int knot_type,
	    int interpolate, ay_nurbpatch_object **skin)
{
 int ay_status = AY_OK;
 ay_object *o = NULL;
 ay_nurbcurve_object *curve = NULL;
 int numcurves = 0, max_order = 0;
 int stride, nh = 0, numknots = 0, t = 0, i, j, a, b;
 int Ualen, Ublen, Ubarlen, clamp_me;
 double *Uh = NULL, *Qw = NULL, *realUh = NULL, *realQw = NULL;
 double *Ubar = NULL, *Ua = NULL, *Ub = NULL;
 double *skc = NULL, u = 0.0;
 double m[16];

  /* clamp curves */
  o = curves;
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
	    }
	}

      if(clamp_me)
	ay_status = ay_nct_clamp(curve);

      o = o->next;
    }

  /* rescale knots to range 0.0 - 1.0 */
  o = curves;
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
  o = curves;
  while(o)
    {
      curve = (ay_nurbcurve_object *) o->refine;
      if(curve->order > max_order)
	max_order = curve->order;

      numcurves++;
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
	  fprintf(stderr,"Memory may have leaked!\n");
	  return ay_status;
	}

      Ua = Ubar;
      Ualen = Ubarlen;

      o = o->next;
    }

  /* merge knots */
  o = curves;
  while(o)
    {
      curve = (ay_nurbcurve_object *) o->refine;

      ay_status = ay_knots_merge(curve, Ubar, Ubarlen);
      if(ay_status)
	{
	  free(Ubar); return ay_status;
	}
      o = o->next;
    }

  /* construct patch */
  o = curves;
  curve = (ay_nurbcurve_object *) o->refine;

  if(!(skc = calloc((curve->length * numcurves * 4), sizeof(double))))
    {
      free(Ubar); return AY_EOMEM;
    }

  b = 0;
  i = 0;

  while(o)
    {
      curve = (ay_nurbcurve_object *) o->refine;

      /* get curves transformation-matrix */
      ay_trafo_creatematrix(o, m);

      stride = 4;

      for(i = 0; i < curve->length; i++)
	{
	  a = i*stride;


	  ay_trafo_apply4(&(curve->controlv[a]), m);

	  memcpy(&(skc[b]), &(curve->controlv[a]), stride*sizeof(double));

	  b += 4;
	}

      o = o->next;
    } /* while */

  if(order > numcurves)
    order = numcurves;
  if(order < 2)
    order = 2;

  if(knot_type == AY_KTBEZIER)
    if(order < numcurves)
      order = numcurves;

  ay_status = ay_npt_create(order, curve->order,
			    numcurves, curve->length,
			    knot_type, AY_KTCUSTOM,
			    skc, NULL, Ubar, skin);

  if(ay_status)
    {
      free(Ubar); free(skc); return ay_status;
    }

  if(interpolate)
    ay_status = ay_npt_interpolateu(*skin);

 return ay_status;
} /* ay_npt_skin */


/* ay_npt_skinv:
 *
 *
 */
int
ay_npt_skinv(ay_object *curves, int order, int knot_type,
	     int interpolate, ay_nurbpatch_object **skin)
{
 int ay_status = AY_OK;

  ay_status = ay_npt_skin(curves, order, knot_type, interpolate, skin);

  ay_status = ay_npt_swapuv(*skin);

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
    }

  if(!(vknotv = calloc(curve->length+curve->order,sizeof(double))))
    { free(new); free(controlv); free(vknotv); return AY_EOMEM; }
  memcpy(vknotv,curve->knotv,(curve->length+curve->order)*sizeof(double));
  new->vknotv = vknotv;
  new->uknotv = uknotv;
  new->vorder = curve->order;
  new->uknot_type = curve->knot_type;
  new->vknot_type = AY_KTCUSTOM;
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


/* ay_npt_tpmchecktri:
 *  check triangle built from p1,p2,p3 for degeneracy;
 *  returns AY_FALSE if triangle is degenerated,
 *  otherwise returns TRUE
 */
int
ay_npt_tpmchecktri(double *p1, double *p2, double *p3)
{

  /* check p1 vs. p2 */
  if((fabs(p1[0] - p2[0]) < AY_EPSILON) &&
     (fabs(p1[1] - p2[1]) < AY_EPSILON) &&
     (fabs(p1[2] - p2[2]) < AY_EPSILON))
      return(AY_FALSE);

  /* check p2 vs. p3 */
  if((fabs(p2[0] - p3[0]) < AY_EPSILON) &&
     (fabs(p2[1] - p3[1]) < AY_EPSILON) &&
     (fabs(p2[2] - p3[2]) < AY_EPSILON))
      return(AY_FALSE);

  /* check p3 vs. p1 */
  if((fabs(p3[0] - p1[0]) < AY_EPSILON) &&
     (fabs(p3[1] - p1[1]) < AY_EPSILON) &&
     (fabs(p3[2] - p1[2]) < AY_EPSILON))
      return(AY_FALSE);

 return AY_TRUE;
} /* ay_npt_tpmchecktri */


/* ay_npt_tpmbegindata:
 *  tesselation callback for ay_npt_topolymesh() below
 *
 */
void
ay_npt_tpmbegindata(GLenum type, void *userData)
{
 ay_npt_tessobject *to;

  to = (ay_npt_tessobject *)userData;

  to->type = type;

  switch(type)
    {
    case GL_TRIANGLE_FAN:
      to->count = 3;
      to->startup = 3;
      break;
    case GL_TRIANGLE_STRIP:
      to->count = 3;
      to->startup = 3;
      break;
    case GL_TRIANGLES:
      to->count = 3;
      to->startup = 3;
      break;
    case GL_QUAD_STRIP:
      to->count = 4;
      to->startup = 4;
      break;
    } /* switch */

 return;
} /* ay_npt_tpmbegindata */


/* ay_npt_tpmvertexdata:
 *  tesselation callback for ay_npt_topolymesh() below
 *
 */
void
ay_npt_tpmvertexdata(GLfloat *vertex, void *userData)
{
 ay_npt_tessobject *to;
 double *t = NULL;
 double *nextpd;
 ay_npt_tesstri *tri = NULL;

  to = (ay_npt_tessobject *)userData;

  to->count--;

  if(to->startup > 0)
    {
      to->startup--;
    }

  nextpd = *to->nextpd;

  nextpd[0] = (double)vertex[0];
  nextpd[1] = (double)vertex[1];
  nextpd[2] = (double)vertex[2];

  if((*(to->nextpd)) == to->p1)
    {
      to->nextpd = &(to->p2);
    }
  else
  if((*(to->nextpd)) == to->p2)
    {
      to->nextpd = &(to->p3);
    }
  else
  if((*(to->nextpd)) == to->p3)
    {
      to->nextpd = &(to->p4);
    }
  else
  if((*(to->nextpd)) == to->p4)
    {
      to->nextpd = &(to->p1);
    }

  if(to->count == 0)
    {
      switch(to->type)
	{
	case GL_TRIANGLE_FAN:
	  to->count = 1;
	  /*printf("Fan\n");*/
	  if(to->startup == 0)
	    {
	      /* create new triangle */
	      if(ay_npt_tpmchecktri(to->p1, to->p2, to->p3))
		{
		  if(!(tri = calloc(1, sizeof(ay_npt_tesstri))))
		    return;

		  tri->next = to->tris;
		  to->tris = tri;

		  memcpy(tri->p1, to->p1, 3*sizeof(double));
		  memcpy(tri->p2, to->p2, 3*sizeof(double));
		  memcpy(tri->p3, to->p3, 3*sizeof(double));

		  memcpy(tri->n1, to->n1, 3*sizeof(double));
		  memcpy(tri->n2, to->n2, 3*sizeof(double));
		  memcpy(tri->n3, to->n3, 3*sizeof(double));
		} /* if */

	      /* shift vertex/normal data */
	      t = to->p2;
	      to->p2 = to->p3;
	      to->p3 = t;

	      t = to->n2;
	      to->n2 = to->n3;
	      to->n3 = t;

	      to->nextpd = &(to->p3);
	      to->nextnd = &(to->n3);
	    } /* if */
	  break;
	case GL_TRIANGLE_STRIP:
	  to->count = 1;
	  /*printf("Strip\n");*/
	  if(to->startup == 0)
	    {
	      /* create new triangle */
	      if(ay_npt_tpmchecktri(to->p1, to->p2, to->p3))
		{
		  if(!(tri = calloc(1, sizeof(ay_npt_tesstri))))
		    return;
		  tri->next = to->tris;
		  to->tris = tri;

		  memcpy(tri->p1, to->p1, 3*sizeof(double));
		  memcpy(tri->p2, to->p2, 3*sizeof(double));
		  memcpy(tri->p3, to->p3, 3*sizeof(double));

		  memcpy(tri->n1, to->n1, 3*sizeof(double));
		  memcpy(tri->n2, to->n2, 3*sizeof(double));
		  memcpy(tri->n3, to->n3, 3*sizeof(double));
		} /* if */

	      /* shift vertex/normal data */
	      t = to->p1;
	      to->p1 = to->p2;
	      to->p2 = to->p3;
	      to->p3 = to->p4;
	      to->p4 = t;

	      t = to->n1;
	      to->n1 = to->n2;
	      to->n2 = to->n3;
	      to->n3 = to->n4;
	      to->n4 = t;

	      to->nextpd = &(to->p3);
	      to->nextnd = &(to->n3);
	    } /* if */

	  break;
	case GL_TRIANGLES:
	  to->count = 3;
	  /*printf("Tri\n");*/
	  if(to->startup == 0)
	    {
	      /* create new triangle */
	      if(ay_npt_tpmchecktri(to->p1, to->p2, to->p3))
		{
		  if(!(tri = calloc(1, sizeof(ay_npt_tesstri))))
		    return;
		  tri->next = to->tris;
		  to->tris = tri;

		  memcpy(tri->p1, to->p1, 3*sizeof(double));
		  memcpy(tri->p2, to->p2, 3*sizeof(double));
		  memcpy(tri->p3, to->p3, 3*sizeof(double));

		  memcpy(tri->n1, to->n1, 3*sizeof(double));
		  memcpy(tri->n2, to->n2, 3*sizeof(double));
		  memcpy(tri->n3, to->n3, 3*sizeof(double));
		} /* if */

	      to->nextpd = &(to->p1);
	      to->nextnd = &(to->n1);
	    } /* if */
	  break;
	case GL_QUAD_STRIP:
	  to->count = 2;
	  /*printf("QuadStrip\n");*/
	  if(to->startup == 0)
	    {
	      /* create two new triangles */
	      if(ay_npt_tpmchecktri(to->p1, to->p2, to->p3))
		{
		  if(!(tri = calloc(1, sizeof(ay_npt_tesstri))))
		    return;
		  tri->next = to->tris;
		  to->tris = tri;

		  memcpy(tri->p1, to->p1, 3*sizeof(double));
		  memcpy(tri->p2, to->p2, 3*sizeof(double));
		  memcpy(tri->p3, to->p3, 3*sizeof(double));

		  memcpy(tri->n1, to->n1, 3*sizeof(double));
		  memcpy(tri->n2, to->n2, 3*sizeof(double));
		  memcpy(tri->n3, to->n3, 3*sizeof(double));
		} /* if */

	      if(ay_npt_tpmchecktri(to->p2, to->p4, to->p3))
		{
		  if(!(tri = calloc(1, sizeof(ay_npt_tesstri))))
		    return;
		  tri->next = to->tris;
		  to->tris = tri;

		  memcpy(tri->p1, to->p2, 3*sizeof(double));
		  memcpy(tri->p2, to->p4, 3*sizeof(double));
		  memcpy(tri->p3, to->p3, 3*sizeof(double));

		  memcpy(tri->n1, to->n2, 3*sizeof(double));
		  memcpy(tri->n2, to->n4, 3*sizeof(double));
		  memcpy(tri->n3, to->n3, 3*sizeof(double));
		} /* if */

	      /* shift vertex/normal data */
	      t = to->p1;
	      to->p1 = to->p3;
	      to->p3 = t;
	      t = to->p2;
	      to->p2 = to->p4;
	      to->p4 = t;

	      t = to->n1;
	      to->n1 = to->n3;
	      to->n3 = t;
	      t = to->n2;
	      to->n2 = to->n4;
	      to->n4 = t;

	      to->nextpd = &(to->p3);
	      to->nextnd = &(to->n3);
	    } /* if */
	  break;
	} /* switch */
    } /* if */

 return;
} /* ay_npt_tpmvertexdata */


/* ay_npt_tpmnormaldata:
 *  tesselation callback for ay_npt_topolymesh() below
 *
 */
void
ay_npt_tpmnormaldata(GLfloat *normal, void *userData)
{
 ay_npt_tessobject *to;
 double *nextnd;

  to = (ay_npt_tessobject *)userData;

  nextnd = *to->nextnd;

  nextnd[0] = (double)normal[0];
  nextnd[1] = (double)normal[1];
  nextnd[2] = (double)normal[2];

  if((*(to->nextnd)) == to->n1)
    {
      to->nextnd = &(to->n2);
    }
  else
  if((*(to->nextnd)) == to->n2)
    {
      to->nextnd = &(to->n3);
    }
  else
  if((*(to->nextnd)) == to->n3)
    {
      to->nextnd = &(to->n4);
    }
  else
  if((*(to->nextnd)) == to->n4)
    {
      to->nextnd = &(to->n1);
    }

 return;
} /* ay_npt_tpmnormaldata */


/* ay_npt_tpmenddata:
 *  tesselation callback for ay_npt_topolymesh() below
 *
 */
void
ay_npt_tpmenddata(void *userData)
{
 ay_npt_tessobject *to;

  to = (ay_npt_tessobject *)userData;

  to->nextpd = &(to->p1);
  to->nextnd = &(to->n1);

 return;
} /* ay_npt_tpmenddata */


/* ay_npt_topolymesh:
 *  tesselate the NURBS patch object o into a PolyMesh object
 *  using the GLU (V1.3+) tesselation facility
 *  smethod - sampling method:
 *   1: GLU_OBJECT_PARAMETRIC_ERROR
 *   2: GLU_OBJECT_PATH_LENGTH
 *   3: GLU_DOMAIN_DISTANCE
 *  smparam - sampling method parameter
 */
int
ay_npt_topolymesh(ay_object *o, int smethod, double sparam,
		  ay_object **pm)
{
#ifndef GLU_VERSION_1_3
 char fname[] = "ay_npt_topolymesh";
 ay_error(AY_ERROR, fname, "This function is just available on GLU V1.3+ !");
 return AY_ERROR;
#else
 int ay_status = AY_OK;
 ay_object *new = NULL;
 ay_nurbpatch_object *npatch = NULL;
 int uorder = 0, vorder = 0, width = 0, height = 0;
 unsigned int uknot_count = 0, vknot_count = 0, i = 0, a = 0;
 GLdouble sampling_tolerance = ay_prefs.glu_sampling_tolerance;
 GLfloat *uknots = NULL, *vknots = NULL, *controls = NULL;
 ay_object *trim = NULL, *loop = NULL, *nc = NULL;
 ay_npt_tessobject to = {0};
 double p1[3], p2[3], p3[3], p4[3], n1[3], n2[3], n3[3], n4[4];
 ay_npt_tesstri *tri = NULL, *tt;
 ay_pomesh_object *po = NULL;
 unsigned int numtris = 0;

  if(!o || !pm)
   return AY_ENULL;

  if(o->type != AY_IDNPATCH)
    return AY_ERROR;

  npatch = (ay_nurbpatch_object *)(o->refine);

  /* create new object (the PolyMesh) */
  if(!(new = calloc(1, sizeof(ay_object))))
    return AY_EOMEM;

  ay_object_defaults(new);
  ay_trafo_copy(o, new);
  new->type = AY_IDPOMESH;
  if(!(new->refine = calloc(1, sizeof(ay_pomesh_object))))
    return AY_EOMEM;
  po = (ay_pomesh_object*)new->refine;

  to.p1 = p1;
  to.p2 = p2;
  to.p3 = p3;
  to.p4 = p4;
  to.n1 = n1;
  to.n2 = n2;
  to.n3 = n3;
  to.n4 = n4;

  to.nextpd = &(to.p1);
  to.nextnd = &(to.n1);

  /* convert npatch data from double to float */

  uorder = npatch->uorder;
  vorder = npatch->vorder;
  width = npatch->width;
  height = npatch->height;

  if(npatch->glu_sampling_tolerance > 0.0)
    sampling_tolerance = npatch->glu_sampling_tolerance;

  uknot_count = width + uorder;
  vknot_count = height + vorder;

  if(!(uknots = calloc(uknot_count, sizeof(GLfloat))))
    { free(new->refine); free(new); return AY_EOMEM; }
  if((vknots = calloc(vknot_count, sizeof(GLfloat))) == NULL)
    { free(new->refine); free(new); free(uknots); return AY_EOMEM; }
  if((controls = calloc(width*height*4, sizeof(GLfloat))) == NULL)
    {
      free(new->refine); free(new); free(uknots); free(vknots);
      return AY_EOMEM;
    }

  a=0;
  for(i = 0; i < uknot_count; i++)
    {
      uknots[a] = (GLfloat)npatch->uknotv[a];
      a++;
    }
  a=0;
  for(i = 0; i < vknot_count; i++)
    {
      vknots[a] = (GLfloat)npatch->vknotv[a];
      a++;
    }
  a=0;
  for(i = 0; i < (unsigned int)width*height*4; i++)
    {
      controls[a] = (GLfloat)npatch->controlv[a];
      a++;
    }

  if(npatch->no)
    {
      gluDeleteNurbsRenderer(npatch->no);
      npatch->no = NULL;
    }

  npatch->no = gluNewNurbsRenderer();
  if(npatch->no == NULL)
    {
      free(new->refine); free(new); free(uknots); free(vknots); free(controls);
      return AY_EOMEM;
    }

  /* register error handling callback */
#if defined(WIN32) && !defined(AYUSESUPERGLU)
  gluNurbsCallback(npatch->no, GLU_ERROR, (GLUnurbsErrorProc)ay_error_glucb);
#else
  gluNurbsCallback(npatch->no, GLU_ERROR, ay_error_glucb);
#endif

  /* set properties */
  gluNurbsProperty(npatch->no, GLU_NURBS_MODE, GLU_NURBS_TESSELLATOR);

  gluNurbsProperty(npatch->no, GLU_DISPLAY_MODE, GLU_FILL);

  if(smethod == 1)
    {
      gluNurbsProperty(npatch->no, GLU_SAMPLING_METHOD,
		       GLU_OBJECT_PARAMETRIC_ERROR);
      gluNurbsProperty(npatch->no, GLU_PARAMETRIC_TOLERANCE,
		       (GLfloat)sparam);
    }

  if(smethod == 2)
    {
      gluNurbsProperty(npatch->no, GLU_SAMPLING_METHOD,
		       GLU_OBJECT_PATH_LENGTH);
      gluNurbsProperty(npatch->no, GLU_SAMPLING_TOLERANCE,
		       (GLfloat)sparam);
    }

  if(smethod == 3)
    {
      gluNurbsProperty(npatch->no, GLU_SAMPLING_METHOD,
		       GLU_DOMAIN_DISTANCE);
      gluNurbsProperty(npatch->no, GLU_U_STEP,
		       (GLfloat)sparam);
      gluNurbsProperty(npatch->no, GLU_V_STEP,
		       (GLfloat)sparam);
    }

  /*
  gluNurbsProperty(npatch->no, GLU_AUTO_LOAD_MATRIX, GL_FALSE);
  */

  /* register callbacks to get tesselated data back from GLU */
  gluNurbsCallbackData(npatch->no, (void *)(&to));
  gluNurbsCallback(npatch->no, GLU_NURBS_BEGIN_DATA, ay_npt_tpmbegindata);
  gluNurbsCallback(npatch->no, GLU_NURBS_VERTEX_DATA, ay_npt_tpmvertexdata);
  gluNurbsCallback(npatch->no, GLU_NURBS_NORMAL_DATA, ay_npt_tpmnormaldata);
  gluNurbsCallback(npatch->no, GLU_NURBS_END_DATA, ay_npt_tpmenddata);

  /* tesselate the patch */
  gluBeginSurface(npatch->no);

  gluNurbsSurface(npatch->no, (GLint)uknot_count, uknots,
		  (GLint)vknot_count, vknots,
		  (GLint)height*4, (GLint)4, controls,
		  (GLint)npatch->uorder, (GLint)npatch->vorder,
		  GL_MAP2_VERTEX_4);

  /* draw trimcurves */
  if(o->down && o->down->next)
    {
      trim = o->down;

      while(trim->next)
	{
	  switch(trim->type)
	    {
	    case AY_IDNCURVE:
	      gluBeginTrim(npatch->no);
	       ay_status = ay_npt_drawtrimcurve(NULL, trim, npatch->no);
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
			  ay_status = ay_npt_drawtrimcurve(NULL, loop,
							   npatch->no);
			}
		      else
			{
			  nc = NULL;
			  ay_status = ay_provide_object(loop, AY_IDNCURVE,
							&nc);
			  if(nc)
			    {
			      ay_status = ay_npt_drawtrimcurve(NULL, nc,
							       npatch->no);
			      ay_object_delete(nc);
			    } /* if */
			} /* if */
		      loop = loop->next;
		    } /* while */
		  gluEndTrim(npatch->no);
		} /* if */
	      break;
	    default:
	      nc = NULL;
	      ay_status = ay_provide_object(trim, AY_IDNCURVE, &nc);
	      if(nc)
		{
		  gluBeginTrim(npatch->no);
		   ay_status = ay_npt_drawtrimcurve(NULL, nc, npatch->no);
		  gluEndTrim(npatch->no);
		  ay_object_delete(nc);
		}
	      break;
	    } /* switch */
	  trim = trim->next;
	} /* while */
    } /* if */

  gluEndSurface(npatch->no);

  gluDeleteNurbsRenderer(npatch->no);
  npatch->no = NULL;

  /* the tessobject should now contain lots of triangles;
     copy them to the PolyMesh object */

  /* first, count the triangles */
  tri = to.tris;
  while(tri)
    {
      numtris++;
      tri = tri->next;
    }

  po->npolys = numtris;

  /* XXXX what happens, when numtris/numcontrolv gets too big for
     an unsigned int? */

  /* create index structures */
  if(!(po->nloops = calloc(numtris, sizeof(unsigned int))))
    {
      free(new->refine); free(new); free(uknots); free(vknots); free(controls);
      return AY_EOMEM;
    }
  for(i = 0; i < numtris; i++)
    {
      /* each polygon has just one loop */
      po->nloops[i] = 1;
    } /* for */

  if(!(po->nverts = calloc(numtris, sizeof(unsigned int))))
    {
      free(new->refine); free(new); free(uknots); free(vknots); free(controls);
      free(po->nloops);
      return AY_EOMEM;
    }
  for(i = 0; i < numtris; i++)
    {
      /* each loop has just three vertices (is a triangle) */
      po->nverts[i] = 3;
    } /* for */

  if(!(po->verts = calloc(numtris*3, sizeof(unsigned int))))
    {
      free(new->refine); free(new); free(uknots); free(vknots); free(controls);
      free(po->nloops); free(po->nverts);
      return AY_EOMEM;
    }
  for(i = 0; i < numtris*3; i++)
    {
      /* vertex indizes are simply ordered (user may remove multiply used
	 vertices using PolyMesh optimization later) */
      po->verts[i] = i;
    } /* for */

  po->ncontrols = numtris*3;
  po->has_normals = AY_TRUE;

  /* now copy all the vertices and normals */
  if(!(po->controlv = calloc(numtris*3*6, sizeof(double))))
    {
      free(new->refine); free(new); free(uknots); free(vknots); free(controls);
      free(po->nloops); free(po->nverts); free(po->verts);
      return AY_EOMEM;
    }

  i = 0;
  tri = to.tris;
  while(tri)
    {
      memcpy(&(po->controlv[i]), tri->p1, 3*sizeof(double));
      memcpy(&(po->controlv[i+3]), tri->n1, 3*sizeof(double));
      memcpy(&(po->controlv[i+6]), tri->p2, 3*sizeof(double));
      memcpy(&(po->controlv[i+9]), tri->n2, 3*sizeof(double));
      memcpy(&(po->controlv[i+12]), tri->p3, 3*sizeof(double));
      memcpy(&(po->controlv[i+15]), tri->n3, 3*sizeof(double));

      i += 18;

      tt = tri;
      tri = tri->next;
      free(tt);
    } /* while */
  to.tris = NULL;

  /* return result */
  *pm = new;

  /* immediately optimize the polymesh (remove multiply used vertices) */
  ay_status = ay_pomesht_optimizecoords(po, AY_FALSE);

  /* clean up */
  free(uknots);
  free(vknots);
  free(controls);

 return AY_OK;
#endif
} /* ay_npt_topolymesh */


/* ay_npt_topolytcmd:
 *
 *
 */
int
ay_npt_topolytcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status;
 char fname[] = "topoly";
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL, *new = NULL;
 double sparam = ay_prefs.sparam;
 int smethod = ay_prefs.smethod+1;

  if(argc > 1)
    {
      Tcl_GetInt(interp, argv[1], &smethod);

      if(smethod == 1)
	sparam = 0.5;

      if(smethod == 2)
	sparam = 50.0;

      if(argc > 2)
	{
	  Tcl_GetDouble(interp, argv[2], &sparam);
	}
    }

  while(sel)
    {
      o = sel->object;

      if(o->type == AY_IDNPATCH)
	{
	  new = NULL;
	  ay_status = ay_npt_topolymesh(o, smethod, sparam, &new);
	  if(!ay_status)
	    {
	      ay_object_link(new);
	    }
	  else
	    {
	      ay_error(AY_ERROR, fname, "Could not convert object!");
	      return TCL_OK;
	    }
	}
      else
	{
	  ay_error(AY_ERROR, fname, "object is not a NPatch");
	}

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_npt_topolytcmd */


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
 ay_nurbpatch_object *patch = NULL;

  while(sel)
    {
      if(sel->object->type == AY_IDNPATCH)
	{
	  if(sel->object->selp)
	    ay_selp_clear(sel->object);

	  patch = (ay_nurbpatch_object *)sel->object->refine;

	  ay_status = ay_npt_swapuv(patch);
	}
      else
	{
	  ay_error(AY_ERROR, fname, "object is not a NPatch");
	} /* if */
      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_npt_swapuvtcmd */


/* ay_npt_gordon:
 *
 */
int
ay_npt_gordon(ay_object *cu, ay_object *cv, int uorder, int vorder,
	      ay_nurbpatch_object **gordon)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *skinu = NULL, *skinv = NULL;
 int knot_type = AY_KTNURB;

  if(!cu || !cv || !gordon)
    return AY_ENULL;


  ay_status = ay_npt_skin(cu, uorder, knot_type, AY_FALSE, &skinu);
  ay_status = ay_npt_skinv(cv, vorder, knot_type, AY_FALSE, &skinv);




 return ay_status;
} /* ay_npt_gordon */
