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

/* icurve.c - icurve object */

static char *ay_icurve_name = "ICurve";

int
ay_icurve_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "crticurve";
 int order = 4, length = 4, closed = AY_FALSE, i = 0;
 double *cv = NULL, dx = 0.25;
 ay_icurve_object *new = NULL;
 ay_object *ncurve = NULL;

  if(!o)
    return AY_ENULL;

  /* parse args */
  while(i+1 < argc)
    {
      if(!strcmp(argv[i],"-length"))
	{
	  Tcl_GetInt(ay_interp, argv[i+1], &length);
	  if(length <= 2) length = 4;
	  i+=2;
	}
      else
	i++;
    }


  if(!(new = calloc(1, sizeof(ay_icurve_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  if(!(cv = calloc(1, length*3*sizeof(double))))
    {
      free(new);
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  for(i=0;i<(length);i++)
    {
      cv[i*3] = (double)i*dx;
    }

  if(!(ncurve = calloc(1, sizeof(ay_object))))
    {
      free(cv); free(new);
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  ay_object_defaults(ncurve);
  ncurve->type = AY_IDNCURVE;

  new->glu_sampling_tolerance = 0.0;
  new->iparam = 1.0/8.0;
  new->iorder = order;
  new->closed = closed;
  new->length = length;
  new->controlv = cv;

  ay_status = ay_ict_interpolateC2C(new->iparam, new->closed, new->length, cv,
				  (ay_nurbcurve_object **)(&(ncurve->refine)));
  new->ncurve = ncurve;

  if(ay_status)
    {
      free(new->ncurve); free(cv); free(new);
      ay_error(ay_status, fname, NULL);
      return AY_ERROR;
    }

  o->refine = new;

 return AY_OK;
} /* ay_icurve_createcb */


int
ay_icurve_deletecb(void *c)
{
 ay_icurve_object *icurve = NULL;

  if(!c)
    return AY_ENULL;

  icurve = (ay_icurve_object *)(c);

  /* free controlv */
  if(icurve->controlv)
    free(icurve->controlv);

  ay_object_delete(icurve->ncurve);

  free(icurve);

 return AY_OK;
} /* ay_icurve_deletecb */


int
ay_icurve_copycb(void *src, void **dst)
{
 ay_icurve_object *icurve = NULL, *icurvesrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  icurvesrc = (ay_icurve_object *)src;

  if(!(icurve = calloc(1, sizeof(ay_icurve_object))))
    return AY_EOMEM;

  memcpy(icurve, src, sizeof(ay_icurve_object));

  /* copy controlv */
  if(!(icurve->controlv = calloc(3 * icurve->length, sizeof(double))))
    return AY_EOMEM;
  memcpy(icurve->controlv, icurvesrc->controlv,
	 3 * icurve->length * sizeof(double));

  /* copy ncurve */
  ay_object_copy(icurvesrc->ncurve, &(icurve->ncurve));

  *dst = (void *)icurve;

 return AY_OK;
} /* ay_icurve_copycb */


int
ay_icurve_drawcb(struct Togl *togl, ay_object *o)
{
 ay_icurve_object *icurve = NULL;
 ay_nurbcurve_object *ncurve = NULL;
 int display_mode = ay_prefs.nc_display_mode;
 int length = 0, i = 0, a = 0, drawch = AY_FALSE;
  if(!o)
    return AY_ENULL;

  icurve = (ay_icurve_object *)o->refine;

  if(!icurve)
    return AY_ENULL;

  if(icurve->display_mode != 0)
    {
      display_mode = icurve->display_mode-1;
    }

  switch(display_mode)
    {
    case 0: /* ControlHull */
      drawch = AY_TRUE;
      break;
    case 1: /* CurveAndHull (GLU) */
      drawch = AY_TRUE;
      if(icurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)icurve->ncurve->refine;
	  ncurve->display_mode = 3;
	  ay_draw_object(togl, icurve->ncurve, AY_TRUE);
	}
      break;
    case 2: /* Curve (GLU) */
      if(icurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)icurve->ncurve->refine;
	  ncurve->display_mode = 3;
	  ay_draw_object(togl, icurve->ncurve, AY_TRUE);
	}
      break;
    case 3: /* CurveAndHull (STESS) */
      drawch = AY_TRUE;
      if(icurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)icurve->ncurve->refine;
	  ncurve->display_mode = 5;
	  ay_draw_object(togl, icurve->ncurve, AY_TRUE);
	}
      break;
    case 4: /* Curve (STESS) */
      if(icurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)icurve->ncurve->refine;
	  ncurve->display_mode = 5;
	  ay_draw_object(togl, icurve->ncurve, AY_TRUE);
	}
      break;
    } /* switch */

  if(drawch)
    {
      length = icurve->length;
      a = 0;
      glBegin(GL_LINE_STRIP);
      for(i = 0; i < length; i++)
	{
	  glVertex3dv((GLdouble *)&(icurve->controlv[a]));
	  a += 3;
	}
      glEnd();
    } /* if */

 return AY_OK;
} /* ay_icurve_drawcb */


int
ay_icurve_shadecb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_icurve_shadecb */


int
ay_icurve_drawhcb(struct Togl *togl, ay_object *o)
{
 int length = 0, i = 0, a = 0;
 ay_icurve_object *curve = NULL;
 GLdouble *ver = NULL;
 double point_size = ay_prefs.handle_size;

  curve = (ay_icurve_object *) o->refine;
  length = curve->length;

  ver = curve->controlv;

  glPointSize((GLfloat)point_size);

  glBegin(GL_POINTS);
   for(i=0; i<length; i++)
     {
       glVertex3dv((GLdouble *)&ver[a]);
       a += 3;
     }
  glEnd();

  /* draw arrow */
  ay_draw_arrow(togl, &(ver[curve->length*3-6]), &(ver[curve->length*3-3]));

 return AY_OK;
} /* ay_icurve_drawhcb */


int
ay_icurve_getpntcb(int mode, ay_object *o, double *p)
{
 ay_icurve_object *icurve = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **pecoords = NULL, *control = NULL, *c = NULL;
 int i = 0, j = 0, a = 0;

  if(!o || !p)
    return AY_ENULL;

  icurve = (ay_icurve_object *)(o->refine);

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(ay_point_edit_coords)
    free(ay_point_edit_coords);

  ay_point_edit_coords = NULL;

  /* select all points? */
  if(mode == 0)
    { /* yes */

      if(!(ay_point_edit_coords = calloc(icurve->length, sizeof(double*))))
	return AY_EOMEM;

      for(i = 0; i < icurve->length; i++)
	{
	  ay_point_edit_coords[i] = &(icurve->controlv[a]);
	  a += 3;
	}

      ay_point_edit_coords_homogenous = AY_FALSE;
      ay_point_edit_coords_number = icurve->length;
    }
  else
    { /* no */

      /* selection based on a single point? */
      if(mode == 1)
	{ /* yes */

	  control = icurve->controlv;
	  for(i = 0; i < icurve->length; i++)
	    {
	      dist = AY_VLEN((p[0] - control[j]),
			     (p[1] - control[j+1]),
			     (p[2] - control[j+2]));

	      if(dist < min_dist)
		{
		  pecoord = &(control[j]);
		  min_dist = dist;
		}

	      j += 3;
	    } /* for */

	  if(!pecoord)
	    return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

	  ay_point_edit_coords_homogenous = AY_FALSE;

	  if(!(ay_point_edit_coords = calloc(1, sizeof(double*))))
	    return AY_EOMEM;

	  ay_point_edit_coords[0] = pecoord;
	  ay_point_edit_coords_number = 1;

	}
      else
	{ /* no */

	  /* selection based on planes */
	  control = icurve->controlv;
	  j = 0;
	  a = 0;
	  for(i = 0; i < icurve->length; i++)
	    {
	      c = &(control[j]);

	      /* test point c against the four planes in p */
	      if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
		 ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
		 ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
		 ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
		{

		  if(!(pecoords = realloc(pecoords, (a+1)*sizeof(double *))))
		    return AY_EOMEM;
		  pecoords[a] = &(control[j]);
		  a++;
		} /* if */

	      j += 3;
	    } /* for */

	  if(!pecoords)
	    return AY_OK; /* XXXX should this return a 'AY_EPICK' ? */

	  ay_point_edit_coords_homogenous = AY_FALSE;
	  ay_point_edit_coords = pecoords;
	  ay_point_edit_coords_number = a;

	} /* if */
    } /* if */

 return AY_OK;
} /* ay_icurve_getpntcb */


/* Tcl -> C! */
int
ay_icurve_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "ICurveAttrData";
 char fname[] = "icurve_setpropcb";
 int new_length;
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_icurve_object *icurve = NULL;


  if(!o)
    return AY_ENULL;

  icurve = (ay_icurve_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Length",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_length);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(icurve->closed));

  Tcl_SetStringObj(ton,"Order",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(icurve->iorder));

  Tcl_SetStringObj(ton,"Mode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(icurve->imode));

  Tcl_SetStringObj(ton,"IParam",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(icurve->iparam));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(icurve->glu_sampling_tolerance));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(icurve->display_mode));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  if(new_length != icurve->length)
    {
      if(new_length > 2)
	{
	  ay_status = ay_ict_resize(icurve, new_length);
	}
      else
	{
	  ay_error(AY_ERROR,fname,"Length must be > 2!");
	}
    }

  ay_status = ay_notify_force(o);

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_icurve_setpropcb */


/* C -> Tcl! */
int
ay_icurve_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="ICurveAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_icurve_object *icurve = NULL;

  if(!o)
    return AY_ENULL;

  icurve = (ay_icurve_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Length",-1);
  to = Tcl_NewIntObj(icurve->length);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_NewIntObj(icurve->closed);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order",-1);
  to = Tcl_NewIntObj(icurve->iorder);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Mode",-1);
  to = Tcl_NewIntObj(icurve->imode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"IParam",-1);
  to = Tcl_NewDoubleObj(icurve->iparam);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(icurve->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(icurve->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_icurve_getpropcb */


int
ay_icurve_readcb(FILE *fileptr, ay_object *o)
{
 ay_icurve_object *icurve = NULL;
 int i, a;

 if(!o)
   return AY_ENULL;

  if(!(icurve = calloc(1, sizeof(ay_icurve_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&icurve->length);
  fscanf(fileptr,"%d\n",&icurve->closed);
  fscanf(fileptr,"%d\n",&icurve->imode);
  fscanf(fileptr,"%d\n",&icurve->iorder);
  fscanf(fileptr,"%lg\n",&icurve->iparam);
  fscanf(fileptr,"%lg\n",&icurve->glu_sampling_tolerance);
  fscanf(fileptr,"%d\n",&icurve->display_mode);

  if(!(icurve->controlv = calloc(icurve->length*3, sizeof(double))))
    { free(icurve); return AY_EOMEM;}

  a = 0;
  for(i=0; i < icurve->length; i++)
    {
      fscanf(fileptr,"%lg %lg %lg\n",&(icurve->controlv[a]),
	     &(icurve->controlv[a+1]),
	     &(icurve->controlv[a+2]));
      a+=3;
    }

  o->refine = icurve;

 return AY_OK;
} /* ay_icurve_readcb */


int
ay_icurve_writecb(FILE *fileptr, ay_object *o)
{
 ay_icurve_object *icurve = NULL;
 int i, a;

  if(!o)
    return AY_ENULL;

  icurve = (ay_icurve_object *)(o->refine);

  fprintf(fileptr, "%d\n", icurve->length);
  fprintf(fileptr, "%d\n", icurve->closed);
  fprintf(fileptr, "%d\n", icurve->imode);
  fprintf(fileptr, "%d\n", icurve->iorder);
  fprintf(fileptr, "%g\n", icurve->iparam);
  fprintf(fileptr, "%g\n", icurve->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", icurve->display_mode);

  a = 0;
  for(i=0;i<icurve->length;i++)
    {
      fprintf(fileptr,"%g %g %g\n", icurve->controlv[a],
	      icurve->controlv[a+1],
	      icurve->controlv[a+2]);
      a+=3;
    }

 return AY_OK;
} /* ay_icurve_writecb */


int
ay_icurve_wribcb(char *file, ay_object *o)
{
 ay_icurve_object *icurve = NULL;

  if(!o)
   return AY_ENULL;

  icurve = (ay_icurve_object*)o->refine;


 return AY_OK;
} /* ay_icurve_wribcb */


int
ay_icurve_bbccb(ay_object *o, double *bbox, int *flags)
{
 double xmin, xmax, ymin, ymax, zmin, zmax;
 double *controlv = NULL;
 int i, a;
 ay_icurve_object *icurve = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  icurve = (ay_icurve_object *)o->refine;

  controlv = icurve->controlv;

  xmin = controlv[0];
  xmax = xmin;
  ymin = controlv[1];
  ymax = ymin;
  zmin = controlv[2];
  zmax = zmin;

  a = 0;
  for(i = 0; i < icurve->length; i++)
    {
      if(controlv[a] < xmin)
	xmin = controlv[a];
      if(controlv[a] > xmax)
	xmax = controlv[a];

      if(controlv[a+1] < ymin)
	ymin = controlv[a+1];
      if(controlv[a+1] > ymax)
	ymax = controlv[a+1];

      if(controlv[a+2] < zmin)
	zmin = controlv[a+2];
      if(controlv[a+2] > zmax)
	zmax = controlv[a+2];

      a += 3;
    }

  /* P1 */
  bbox[0] = xmin; bbox[1] = ymax; bbox[2] = zmax;
  /* P2 */
  bbox[3] = xmin; bbox[4] = ymax; bbox[5] = zmin;
  /* P3 */
  bbox[6] = xmax; bbox[7] = ymax; bbox[8] = zmin;
  /* P4 */
  bbox[9] = xmax; bbox[10] = ymax; bbox[11] = zmax;

  /* P5 */
  bbox[12] = xmin; bbox[13] = ymin; bbox[14] = zmax;
  /* P6 */
  bbox[15] = xmin; bbox[16] = ymin; bbox[17] = zmin;
  /* P7 */
  bbox[18] = xmax; bbox[19] = ymin; bbox[20] = zmin;
  /* P8 */
  bbox[21] = xmax; bbox[22] = ymin; bbox[23] = zmax;

 return AY_OK;
} /* ay_icurve_bbccb */


int
ay_icurve_notifycb(ay_object *o)
{
 ay_icurve_object *icurve = NULL;
 ay_nurbcurve_object *nc = NULL;
 ay_object *ncurve = NULL;
 int i, a, b;
 int ay_status = AY_OK;

  if(!o)
    return AY_ENULL;

  icurve = (ay_icurve_object *)(o->refine);


  /* do we have a child? */
  if(o->down)
    {
      /* yes, copy it's controlv and delete it */
      if(o->down->type == AY_IDNCURVE)
	{
	  nc = (ay_nurbcurve_object *)o->down->refine;
	  icurve->length = nc->length;
	  if(icurve->controlv)
	    free(icurve->controlv);
	  icurve->controlv = NULL;
	  if(!(icurve->controlv = calloc(icurve->length*3, sizeof(double))))
	    return AY_EOMEM;
	  a = 0; b = 0;
	  for(i=0;i<nc->length;i++)
	    {
	      memcpy(&(icurve->controlv[a]), &(nc->controlv[b]),
		     3*sizeof(double));

	      a += 3;
	      b += 4;
	    } /* for */
	  ay_status = ay_object_deletemulti(o->down);
	  o->down = NULL;

	} /* if */
    } /* if */

  /* create new interpolated curve */
  ay_object_delete(icurve->ncurve);
  icurve->ncurve = NULL;

  if(!(ncurve = calloc(1, sizeof(ay_object))))
    {
      return AY_ERROR;
    }

  ay_object_defaults(ncurve);
  ncurve->type = AY_IDNCURVE;

  if(icurve->imode)
    {
      if(icurve->closed)
	{
	  ay_status = ay_ict_interpolateG4DC(icurve->iorder, icurve->length,
					     icurve->iparam, icurve->controlv,
				 (ay_nurbcurve_object **)(&(ncurve->refine)));
	}
      else
	{
	  ay_status = ay_ict_interpolateG4D(icurve->iorder, icurve->length,
					     icurve->controlv,
				 (ay_nurbcurve_object **)(&(ncurve->refine)));
	}
    }
  else
    {
      ay_status = ay_ict_interpolateC2C(icurve->iparam, icurve->closed,
					icurve->length, icurve->controlv,
				 (ay_nurbcurve_object **)(&(ncurve->refine)));

    }

  if(ay_status)
    return ay_status;

  nc = (ay_nurbcurve_object *)ncurve->refine;
  nc->display_mode = icurve->display_mode;
  nc->glu_sampling_tolerance = icurve->glu_sampling_tolerance;

  icurve->ncurve = ncurve;

 return AY_OK;
} /* ay_icurve_notifycb */


int
ay_icurve_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_icurve_object *ic = NULL;
 ay_object *new = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  ic = (ay_icurve_object *) o->refine;

  if(ic->ncurve)
    {
      ay_status = ay_object_copy(ic->ncurve, &new);

      if(new)
	{
	  /* reset display mode of new curve to "global" */
	  nc = (ay_nurbcurve_object *)(new->refine);
	  nc->display_mode = 0;

	  ay_trafo_copy(o, new);

	  if(!in_place)
	    {
	      ay_status = ay_object_link(new);
	    }
	  else
	    {
	      ay_status = ay_object_replace(new, o);
	    } /* if */
	} /* if */
    } /* if */

 return ay_status;
} /* ay_icurve_convertcb */


int
ay_icurve_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 ay_icurve_object *ic = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNCURVE)
	return AY_OK;
      else
	return AY_ERROR;
    }

  ic = (ay_icurve_object *) o->refine;

  if(type == AY_IDNCURVE)
    {
      if(ic->ncurve)
	{
	  nc = (ay_nurbcurve_object *)ic->ncurve->refine;
	  nc->display_mode = ic->display_mode;
	  ay_status = ay_object_copy(ic->ncurve, result);
	  if(*result)
	    {
	      ay_trafo_copy(o, *result);
	    } /* if */
	} /* if */
    } /* if */

 return ay_status;
} /* ay_icurve_providecb */


int
ay_icurve_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_icurve_name,
				    ay_icurve_createcb,
				    ay_icurve_deletecb,
				    ay_icurve_copycb,
				    ay_icurve_drawcb,
				    ay_icurve_drawhcb,
				    NULL, /* no shading */
				    ay_icurve_setpropcb,
				    ay_icurve_getpropcb,
				    ay_icurve_getpntcb,
				    ay_icurve_readcb,
				    ay_icurve_writecb,
				    NULL, /* no RIB export */
				    ay_icurve_bbccb,
				    AY_IDICURVE);


  ay_status = ay_notify_register(ay_icurve_notifycb, AY_IDICURVE);

  ay_status = ay_convert_register(ay_icurve_convertcb, AY_IDICURVE);

  ay_status = ay_provide_register(ay_icurve_providecb, AY_IDICURVE);

  ay_matt_nomaterial(AY_IDICURVE);

 return ay_status;
} /* ay_icurve_init */

