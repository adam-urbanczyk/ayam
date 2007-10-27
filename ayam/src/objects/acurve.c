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

/* acurve.c - acurve (approximating curve) object */

static char *ay_acurve_name = "ACurve";

int
ay_acurve_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "crtacurve";
 int order = 4, length = 4, closed = AY_FALSE, i = 0;
 double *cv = NULL, dx = 0.25;
 ay_acurve_object *new = NULL;
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
	{
	  i++;
	}
    }

  if(!(new = calloc(1, sizeof(ay_acurve_object))))
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
  new->order = order;
  new->closed = closed;
  new->length = length;
  new->controlv = cv;

  ay_status = ay_act_leastSquares( new->length, cv,
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
} /* ay_acurve_createcb */


int
ay_acurve_deletecb(void *c)
{
 ay_acurve_object *acurve = NULL;

  if(!c)
    return AY_ENULL;

  acurve = (ay_acurve_object *)(c);

  /* free controlv */
  if(acurve->controlv)
    free(acurve->controlv);

  ay_object_delete(acurve->ncurve);

  free(acurve);

 return AY_OK;
} /* ay_acurve_deletecb */


int
ay_acurve_copycb(void *src, void **dst)
{
 ay_acurve_object *acurve = NULL, *acurvesrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  acurvesrc = (ay_acurve_object *)src;

  if(!(acurve = calloc(1, sizeof(ay_acurve_object))))
    return AY_EOMEM;

  memcpy(acurve, src, sizeof(ay_acurve_object));

  /* copy controlv */
  if(!(acurve->controlv = calloc(3 * acurve->length, sizeof(double))))
    return AY_EOMEM;
  memcpy(acurve->controlv, acurvesrc->controlv,
	 3 * acurve->length * sizeof(double));

  /* copy ncurve */
  ay_object_copy(acurvesrc->ncurve, &(acurve->ncurve));

  *dst = (void *)acurve;

 return AY_OK;
} /* ay_acurve_copycb */


int
ay_acurve_drawcb(struct Togl *togl, ay_object *o)
{
 ay_acurve_object *acurve = NULL;
 ay_nurbcurve_object *ncurve = NULL;
 int display_mode = ay_prefs.nc_display_mode;
 int length = 0, i = 0, a = 0, drawch = AY_FALSE;
  if(!o)
    return AY_ENULL;

  acurve = (ay_acurve_object *)o->refine;

  if(!acurve)
    return AY_ENULL;

  if(acurve->display_mode != 0)
    {
      display_mode = acurve->display_mode-1;
    }

  switch(display_mode)
    {
    case 0: /* ControlHull */
      drawch = AY_TRUE;
      break;
    case 1: /* CurveAndHull (GLU) */
      drawch = AY_TRUE;
      if(acurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)acurve->ncurve->refine;
	  ncurve->display_mode = 3;
	  ay_draw_object(togl, acurve->ncurve, AY_TRUE);
	}
      break;
    case 2: /* Curve (GLU) */
      if(acurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)acurve->ncurve->refine;
	  ncurve->display_mode = 3;
	  ay_draw_object(togl, acurve->ncurve, AY_TRUE);
	}
      break;
    case 3: /* CurveAndHull (STESS) */
      drawch = AY_TRUE;
      if(acurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)acurve->ncurve->refine;
	  ncurve->display_mode = 5;
	  ay_draw_object(togl, acurve->ncurve, AY_TRUE);
	}
      break;
    case 4: /* Curve (STESS) */
      if(acurve->ncurve)
	{
	  ncurve = (ay_nurbcurve_object *)acurve->ncurve->refine;
	  ncurve->display_mode = 5;
	  ay_draw_object(togl, acurve->ncurve, AY_TRUE);
	}
      break;
    } /* switch */

  if(drawch)
    {
      length = acurve->length;
      a = 0;
      glBegin(GL_LINE_STRIP);
      for(i = 0; i < length; i++)
	{
	  glVertex3dv((GLdouble *)&(acurve->controlv[a]));
	  a += 3;
	}
      glEnd();
    } /* if */

 return AY_OK;
} /* ay_acurve_drawcb */


int
ay_acurve_shadecb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_acurve_shadecb */


int
ay_acurve_drawhcb(struct Togl *togl, ay_object *o)
{
 int length = 0, i = 0, a = 0;
 ay_acurve_object *curve = NULL;
 GLdouble *ver = NULL;
 double point_size = ay_prefs.handle_size;

  curve = (ay_acurve_object *) o->refine;
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
} /* ay_acurve_drawhcb */


int
ay_acurve_getpntcb(int mode, ay_object *o, double *p)
{
 ay_acurve_object *acurve = NULL;
 double min_dist = ay_prefs.pick_epsilon, dist = 0.0;
 double *pecoord = NULL, **pecoords = NULL, *control = NULL, *c = NULL;
 int i = 0, j = 0, a = 0;

  if(!o || !p)
    return AY_ENULL;

  acurve = (ay_acurve_object *)(o->refine);

  if(min_dist == 0.0)
    min_dist = DBL_MAX;

  if(ay_point_edit_coords)
    free(ay_point_edit_coords);

  ay_point_edit_coords = NULL;

  /* select all points? */
  if(mode == 0)
    { /* yes */

      if(!(ay_point_edit_coords = calloc(acurve->length, sizeof(double*))))
	return AY_EOMEM;

      for(i = 0; i < acurve->length; i++)
	{
	  ay_point_edit_coords[i] = &(acurve->controlv[a]);
	  a += 3;
	}

      ay_point_edit_coords_homogenous = AY_FALSE;
      ay_point_edit_coords_number = acurve->length;
    }
  else
    { /* no */

      /* selection based on a single point? */
      if(mode == 1)
	{ /* yes */

	  control = acurve->controlv;
	  for(i = 0; i < acurve->length; i++)
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
	  control = acurve->controlv;
	  j = 0;
	  a = 0;
	  for(i = 0; i < acurve->length; i++)
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
} /* ay_acurve_getpntcb */


/* Tcl -> C! */
int
ay_acurve_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "ACurveAttrData";
 char fname[] = "acurve_setpropcb";
 int new_length;
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_acurve_object *acurve = NULL;


  if(!o)
    return AY_ENULL;

  acurve = (ay_acurve_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Length",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &new_length);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(acurve->closed));

  Tcl_SetStringObj(ton,"Order",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(acurve->order));
  /*
  Tcl_SetStringObj(ton,"Mode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(acurve->mode));

  Tcl_SetStringObj(ton,"Param",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(acurve->param));
  */
  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(acurve->glu_sampling_tolerance));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(acurve->display_mode));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  if(new_length != acurve->length)
    {
      if(new_length > 2)
	{
	  ay_status = ay_act_resize(acurve, new_length);
	}
      else
	{
	  ay_error(AY_ERROR, fname, "Length must be > 2!");
	}
    }

  o->modified = AY_TRUE;

  ay_status = ay_notify_force(o);

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_acurve_setpropcb */


/* C -> Tcl! */
int
ay_acurve_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="ACurveAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_acurve_object *acurve = NULL;

  if(!o)
    return AY_ENULL;

  acurve = (ay_acurve_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Length",-1);
  to = Tcl_NewIntObj(acurve->length);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_NewIntObj(acurve->closed);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order",-1);
  to = Tcl_NewIntObj(acurve->order);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  /*
  Tcl_SetStringObj(ton,"Mode",-1);
  to = Tcl_NewIntObj(acurve->mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"IParam",-1);
  to = Tcl_NewDoubleObj(acurve->param);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);
  */
  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(acurve->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(acurve->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getncinfo(interp, n1, acurve->ncurve);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_acurve_getpropcb */


int
ay_acurve_readcb(FILE *fileptr, ay_object *o)
{
 ay_acurve_object *acurve = NULL;
 int i, a;

 if(!o)
   return AY_ENULL;

  if(!(acurve = calloc(1, sizeof(ay_acurve_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&acurve->length);
  fscanf(fileptr,"%d\n",&acurve->closed);
  fscanf(fileptr,"%d\n",&acurve->order);
  /*
  fscanf(fileptr,"%d\n",&acurve->imode);
  fscanf(fileptr,"%lg\n",&acurve->iparam);
  */
  fscanf(fileptr,"%lg\n",&acurve->glu_sampling_tolerance);
  fscanf(fileptr,"%d\n",&acurve->display_mode);

  if(!(acurve->controlv = calloc(acurve->length*3, sizeof(double))))
    { free(acurve); return AY_EOMEM;}

  a = 0;
  for(i = 0; i < acurve->length; i++)
    {
      fscanf(fileptr,"%lg %lg %lg\n",
	     &(acurve->controlv[a]),
	     &(acurve->controlv[a+1]),
	     &(acurve->controlv[a+2]));
      a += 3;
    }

  o->refine = acurve;

 return AY_OK;
} /* ay_acurve_readcb */


int
ay_acurve_writecb(FILE *fileptr, ay_object *o)
{
 ay_acurve_object *acurve = NULL;
 int i, a;

  if(!o)
    return AY_ENULL;

  acurve = (ay_acurve_object *)(o->refine);

  fprintf(fileptr, "%d\n", acurve->length);
  fprintf(fileptr, "%d\n", acurve->closed);
  fprintf(fileptr, "%d\n", acurve->order);
  /*
  fprintf(fileptr, "%d\n", acurve->imode);
  fprintf(fileptr, "%g\n", acurve->iparam);
  */
  fprintf(fileptr, "%g\n", acurve->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", acurve->display_mode);

  a = 0;
  for(i = 0; i < acurve->length; i++)
    {
      fprintf(fileptr,"%g %g %g\n",
	      acurve->controlv[a],
	      acurve->controlv[a+1],
	      acurve->controlv[a+2]);
      a += 3;
    }

 return AY_OK;
} /* ay_acurve_writecb */


int
ay_acurve_wribcb(char *file, ay_object *o)
{
 ay_acurve_object *acurve = NULL;

  if(!o)
   return AY_ENULL;

  acurve = (ay_acurve_object*)o->refine;

 return AY_OK;
} /* ay_acurve_wribcb */


int
ay_acurve_bbccb(ay_object *o, double *bbox, int *flags)
{
 double xmin, xmax, ymin, ymax, zmin, zmax;
 double *controlv = NULL;
 int i, a;
 ay_acurve_object *acurve = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  acurve = (ay_acurve_object *)o->refine;

  controlv = acurve->controlv;

  xmin = controlv[0];
  xmax = xmin;
  ymin = controlv[1];
  ymax = ymin;
  zmin = controlv[2];
  zmax = zmin;

  a = 0;
  for(i = 0; i < acurve->length; i++)
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
} /* ay_acurve_bbccb */


int
ay_acurve_notifycb(ay_object *o)
{
 ay_acurve_object *acurve = NULL;
 ay_nurbcurve_object *nc = NULL;
 ay_object *ncurve = NULL;
 int i, a, b;
 int ay_status = AY_OK;

  if(!o)
    return AY_ENULL;

  acurve = (ay_acurve_object *)(o->refine);


  ay_object_delete(acurve->ncurve);
  acurve->ncurve = NULL;

  /* create new approximating curve */
  if(!(ncurve = calloc(1, sizeof(ay_object))))
    {
      return AY_ERROR;
    }

  ay_object_defaults(ncurve);
  ncurve->type = AY_IDNCURVE;


  ay_status = ay_act_leastSquares(acurve->iparam, acurve->closed,
				  acurve->length, acurve->controlv,
				  (ay_nurbcurve_object **)(&(ncurve->refine)));

  if(ay_status)
    return ay_status;

  nc = (ay_nurbcurve_object *)ncurve->refine;
  nc->display_mode = acurve->display_mode;
  nc->glu_sampling_tolerance = acurve->glu_sampling_tolerance;

  acurve->ncurve = ncurve;

 return AY_OK;
} /* ay_acurve_notifycb */


int
ay_acurve_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_acurve_object *ic = NULL;
 ay_object *new = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  ic = (ay_acurve_object *) o->refine;

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
} /* ay_acurve_convertcb */


int
ay_acurve_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 ay_acurve_object *ic = NULL;
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

  ic = (ay_acurve_object *) o->refine;

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
} /* ay_acurve_providecb */


int
ay_acurve_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_acurve_name,
				    ay_acurve_createcb,
				    ay_acurve_deletecb,
				    ay_acurve_copycb,
				    ay_acurve_drawcb,
				    ay_acurve_drawhcb,
				    NULL, /* no shading */
				    ay_acurve_setpropcb,
				    ay_acurve_getpropcb,
				    ay_acurve_getpntcb,
				    ay_acurve_readcb,
				    ay_acurve_writecb,
				    NULL, /* no RIB export */
				    ay_acurve_bbccb,
				    AY_IDACURVE);


  ay_status = ay_notify_register(ay_acurve_notifycb, AY_IDACURVE);

  ay_status = ay_convert_register(ay_acurve_convertcb, AY_IDACURVE);

  ay_status = ay_provide_register(ay_acurve_providecb, AY_IDACURVE);

  ay_matt_nomaterial(AY_IDACURVE);

 return ay_status;
} /* ay_acurve_init */

