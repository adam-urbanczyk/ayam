/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2012 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* sfcurve.c - super formula curve custom object */

char sfcurve_version_ma[] = AY_VERSIONSTR;
char sfcurve_version_mi[] = AY_VERSIONSTRMI;

static char *sfcurve_name = "SfCurve";

static unsigned int sfcurve_id;

typedef struct sfcurve_object_s
{
  double m, n1, n2, n3; /**< superformula parameters */
  double tmin, tmax; /**< minimum/maximum angles */

  int sections; /**< number of control points to create */
  int order; /**< desired order of NURBS curve to create */

  ay_object *ncurve; /**< cached NURBS curve */

  int display_mode; /**< drawing quality */
  double glu_sampling_tolerance; /**< drawing mode */
} sfcurve_object;


#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int Sfcurve_Init(Tcl_Interp *interp);


/* sfcurve_createcb:
 *  create callback function of sfcurve object
 */
int
sfcurve_createcb(int argc, char *argv[], ay_object *o)
{
 sfcurve_object *sfcurve = NULL;
 char fname[] = "crtsfcurve";

  if(!o)
    return AY_ENULL;

  if(!(sfcurve = calloc(1, sizeof(sfcurve_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  sfcurve->m = 4;
  sfcurve->n1 = 4;
  sfcurve->n2 = 7;
  sfcurve->n3 = 7;

  sfcurve->sections = 9;
  sfcurve->order = 3;

  sfcurve->tmin = 0.0;
  sfcurve->tmax = 360.0;

  o->refine = sfcurve;

  ay_notify_object(o);

 return AY_OK;
} /* sfcurve_createcb */


/* sfcurve_deletecb:
 *  delete callback function of sfcurve object
 */
int
sfcurve_deletecb(void *c)
{
 sfcurve_object *sfcurve = NULL;

  if(!c)
    return AY_ENULL;

  sfcurve = (sfcurve_object *)(c);

  /* free cached ncurve */
  ay_object_delete(sfcurve->ncurve);

  free(sfcurve);

 return AY_OK;
} /* sfcurve_deletecb */


/* sfcurve_copycb:
 *  copy callback function of sfcurve object
 */
int
sfcurve_copycb(void *src, void **dst)
{
 sfcurve_object *sfcurve = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(sfcurve = calloc(1, sizeof(sfcurve_object))))
    return AY_EOMEM;

  memcpy(sfcurve, src, sizeof(sfcurve_object));

  sfcurve->ncurve = NULL;

  *dst = (void *)sfcurve;

 return AY_OK;
} /* sfcurve_copycb */


/* sfcurve_drawcb:
 *  draw (display in an Ayam view window) callback function of sfcurve object
 */
int
sfcurve_drawcb(struct Togl *togl, ay_object *o)
{
 sfcurve_object *sfcurve = NULL;

  if(!o)
    return AY_ENULL;

  sfcurve = (sfcurve_object *)o->refine;

  if(!sfcurve)
    return AY_ENULL;

  if(sfcurve->ncurve)
    {
      ay_draw_object(togl, sfcurve->ncurve, AY_TRUE);
    }

 return AY_OK;
} /* sfcurve_drawcb */


/* sfcurve_drawhcb:
 *  draw handles callback function of sfcurve object
 */
int
sfcurve_drawhcb(struct Togl *togl, ay_object *o)
{
 sfcurve_object *sfcurve = NULL;
 ay_nurbcurve_object *ncurve = NULL;
 double *cv = NULL;
 double point_size = ay_prefs.handle_size;
 int i, a = 0, stride = 4;

  if(!o)
    return AY_ENULL;

  sfcurve = (sfcurve_object *)o->refine;

  if(!sfcurve)
    return AY_ENULL;

  if(sfcurve->ncurve)
    {
      ncurve = (ay_nurbcurve_object *)sfcurve->ncurve->refine;
      cv = ncurve->controlv;

      glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
		(GLfloat)ay_prefs.obb);

      glPointSize((GLfloat)point_size);

      glBegin(GL_POINTS);
       for(i = 0; i < ncurve->length; i++)
	 {
	   glVertex3dv((GLdouble *)&cv[a]);
	   a += stride;
	 }
      glEnd();

      glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
		(GLfloat)ay_prefs.seb);
    } /* if */

 return AY_OK;
} /* sfcurve_drawhcb */


/* sfcurve_getpntcb:
 *  get point (editing and selection) callback function of sfcurve object
 */
int
sfcurve_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 sfcurve_object *sfcurve = NULL;
 ay_nurbcurve_object *ncurve = NULL;

  if(!o)
    return AY_ENULL;

  sfcurve = (sfcurve_object *)o->refine;

  if(!sfcurve)
    return AY_ENULL;

  if(sfcurve->ncurve)
    {
      ncurve = (ay_nurbcurve_object*)sfcurve->ncurve->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, ncurve->length, 4,
			     ay_prefs.rationalpoints, ncurve->controlv);
    }
  else
    {
      return AY_OK;
    }

} /* sfcurve_getpntcb */


/* sfcurve_setpropcb:
 *  set property (from Tcl to C context) callback function of sfcurve object
 */
int
sfcurve_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char fname[] = "sfcurve_setpropcb";
 char *arr = "SfCurveAttrData";
 Tcl_Obj *to = NULL;
 sfcurve_object *sfcurve = NULL;

  if(!interp || !o)
    return AY_ENULL;

  sfcurve = (sfcurve_object *)o->refine;

  if(!sfcurve)
    return AY_ENULL;

  to = Tcl_GetVar2Ex(interp, arr, "Sections",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(sfcurve->sections));

  if(sfcurve->sections < 2)
    {
      ay_error(AY_ERROR, fname, "sections must be >1");
      sfcurve->sections = 2;
    }

  to = Tcl_GetVar2Ex(interp, arr, "Order",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(sfcurve->order));

  to = Tcl_GetVar2Ex(interp, arr, "M",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(sfcurve->m));

  to = Tcl_GetVar2Ex(interp, arr, "N1",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(sfcurve->n1));

  to = Tcl_GetVar2Ex(interp, arr, "N2",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(sfcurve->n2));

  to = Tcl_GetVar2Ex(interp, arr, "N3",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(sfcurve->n3));

  to = Tcl_GetVar2Ex(interp, arr, "TMin",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(sfcurve->tmin));

  to = Tcl_GetVar2Ex(interp, arr, "TMax",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(sfcurve->tmax));

  if(fabs(sfcurve->tmax-sfcurve->tmin) < AY_EPSILON)
    {
      ay_error(AY_ERROR, fname, "TMin must be different from TMax");
      sfcurve->tmin = 0.0;
      sfcurve->tmax = 360.0;
    }

  to = Tcl_GetVar2Ex(interp, arr, "Tolerance",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp, to, &(sfcurve->glu_sampling_tolerance));

  to = Tcl_GetVar2Ex(interp, arr, "DisplayMode",
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(sfcurve->display_mode));

  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* sfcurve_setpropcb */


/* sfcurve_getpropcb:
 *  get property (from C to Tcl context) callback function of sfcurve object
 */
int
sfcurve_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *arr = "SfCurveAttrData";
 sfcurve_object *sfcurve = NULL;

  if(!interp || !o)
    return AY_ENULL;

  sfcurve = (sfcurve_object *)(o->refine);

  if(!sfcurve)
    return AY_ENULL;

  Tcl_SetVar2Ex(interp, arr, "Sections",
		Tcl_NewIntObj(sfcurve->sections),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Order",
		Tcl_NewIntObj(sfcurve->order),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "M",
		Tcl_NewDoubleObj(sfcurve->m),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "N1",
		Tcl_NewDoubleObj(sfcurve->n1),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "N2",
		Tcl_NewDoubleObj(sfcurve->n2),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "N3",
		Tcl_NewDoubleObj(sfcurve->n3),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "TMin",
		Tcl_NewDoubleObj(sfcurve->tmin),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "TMax",
		Tcl_NewDoubleObj(sfcurve->tmax),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "Tolerance",
		Tcl_NewDoubleObj(sfcurve->glu_sampling_tolerance),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetVar2Ex(interp, arr, "DisplayMode",
		Tcl_NewIntObj(sfcurve->display_mode),
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

 return AY_OK;
} /* sfcurve_getpropcb */


/* sfcurve_readcb:
 *  read (from scene file) callback function of sfcurve object
 */
int
sfcurve_readcb(FILE *fileptr, ay_object *o)
{
 sfcurve_object *sfcurve = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(sfcurve = calloc(1, sizeof(sfcurve_object))))
    { return AY_EOMEM; }

  fscanf(fileptr, "%d\n", &sfcurve->sections);
  fscanf(fileptr, "%d\n", &sfcurve->order);

  fscanf(fileptr, "%lg\n", &sfcurve->m);
  fscanf(fileptr, "%lg\n", &sfcurve->n1);
  fscanf(fileptr, "%lg\n", &sfcurve->n2);
  fscanf(fileptr, "%lg\n", &sfcurve->n3);
  fscanf(fileptr, "%lg\n", &sfcurve->tmin);
  fscanf(fileptr, "%lg\n", &sfcurve->tmax);

  fscanf(fileptr, "%lg\n", &(sfcurve->glu_sampling_tolerance));
  fscanf(fileptr, "%d\n", &(sfcurve->display_mode));

  o->refine = sfcurve;

 return AY_OK;
} /* sfcurve_readcb */


/* sfcurve_writecb:
 *  write (to scene file) callback function of sfcurve object
 */
int
sfcurve_writecb(FILE *fileptr, ay_object *o)
{
 sfcurve_object *sfcurve = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  sfcurve = (sfcurve_object *)(o->refine);

  if(!sfcurve)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", sfcurve->sections);
  fprintf(fileptr, "%d\n", sfcurve->order);

  fprintf(fileptr, "%g\n", sfcurve->m);
  fprintf(fileptr, "%g\n", sfcurve->n1);
  fprintf(fileptr, "%g\n", sfcurve->n2);
  fprintf(fileptr, "%g\n", sfcurve->n3);

  fprintf(fileptr, "%g\n", sfcurve->tmin);
  fprintf(fileptr, "%g\n", sfcurve->tmax);

  fprintf(fileptr, "%g\n", sfcurve->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", sfcurve->display_mode);

 return AY_OK;
} /* sfcurve_writecb */


/* sfcurve_bbccb:
 *  bounding box calculation callback function of sfcurve object
 */
int
sfcurve_bbccb(ay_object *o, double *bbox, int *flags)
{
 sfcurve_object *sfcurve = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  sfcurve = (sfcurve_object *)o->refine;

  if(!sfcurve)
    return AY_ENULL;

  if(sfcurve->ncurve)
    {
      return ay_bbc_get(sfcurve->ncurve, bbox);
    }
  else
    {
      return AY_ERROR;
    }

 return AY_OK;
} /* sfcurve_bbccb */


/* sfcurve_notifycb:
 *  notification callback function of sfcurve object
 */
int
sfcurve_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 sfcurve_object *sfcurve = NULL;
 ay_nurbcurve_object *nc = NULL;
 ay_object *ncurve = NULL;
 double *cv = NULL;
 int stride = 4, i, a;
 double r, angle, delta;

  if(!o)
    return AY_ENULL;

  sfcurve = (sfcurve_object *)(o->refine);

  if(!sfcurve)
    return AY_ENULL;

  /* remove old NURBS curve */
  ay_object_delete(sfcurve->ncurve);
  sfcurve->ncurve = NULL;

  /* create new NURBS curve */
  if(!(ncurve = calloc(1, sizeof(ay_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  ay_object_defaults(ncurve);
  ncurve->type = AY_IDNCURVE;

  if(!(nc = calloc(1, sizeof(ay_nurbcurve_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  ncurve->refine = nc;

  if(!(cv = calloc((sfcurve->sections+1)*stride, sizeof(double))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  delta = (AY_D2R(sfcurve->tmax-sfcurve->tmin))/sfcurve->sections;
  angle = sfcurve->tmin;
  a = 0;

  for(i = 0; i < sfcurve->sections; i++)
    {
      r = pow((pow(fabs(cos(sfcurve->m*angle/4.0)), sfcurve->n2) +
	       pow(fabs(sin(sfcurve->m*angle/4.0)), sfcurve->n3)),
	      -(1.0/sfcurve->n1));

      cv[a] =   r*cos(angle);
      cv[a+1] = r*sin(angle);
      cv[a+3] = 1.0;
      a += stride;
      angle += delta;
    } /* for */

  memcpy(&(cv[a]), cv, stride*sizeof(double));

  nc->controlv = cv;
  nc->length = sfcurve->sections+1;
  nc->order = sfcurve->order;

  nc->knot_type = AY_KTNURB;
  ay_status = ay_knots_createnc(nc);

  if(ay_status)
    {
      goto cleanup;
    }

  nc->display_mode = sfcurve->display_mode;
  nc->glu_sampling_tolerance = sfcurve->glu_sampling_tolerance;

  sfcurve->ncurve = ncurve;

  /* prevent cleanup code from doing something harmful */
  ncurve = NULL;

cleanup:

  if(ncurve)
    {
      ay_object_delete(ncurve);
    }

  /* recover selected points */
  if(o->selp)
    {
      if(sfcurve->ncurve)
	sfcurve_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

 return ay_status;
} /* sfcurve_notifycb */


/* sfcurve_convertcb:
 *  convert callback function of sfcurve object
 */
int
sfcurve_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 sfcurve_object *sfc = NULL;
 ay_object *new = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  sfc = (sfcurve_object *) o->refine;

  if(!sfc)
    return AY_ENULL;

  if(sfc->ncurve)
    {
      ay_status = ay_object_copy(sfc->ncurve, &new);

      if(new)
	{
	  nc = (ay_nurbcurve_object *)(new->refine);

	  /* reset display mode and sampling tolerance
	     of new curve to "global"? */
	  if(!in_place && ay_prefs.conv_reset_display)
	    {
	      nc->display_mode = 0;
	      nc->glu_sampling_tolerance = 0.0;
	    }

	  ay_trafo_copy(o, new);

	  if(!in_place)
	    {
	      ay_object_link(new);
	    }
	  else
	    {
	      ay_status = ay_object_replace(new, o);
	    } /* if */
	} /* if */
    } /* if */

 return ay_status;
} /* sfcurve_convertcb */


/* sfcurve_providecb:
 *  provide callback function of sfcurve object
 */
int
sfcurve_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 sfcurve_object *sfc = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if((type == AY_IDNCURVE) || (type == UINT_MAX-AY_IDNCURVE))
	return AY_OK;
      else
	return AY_ERROR;
    }

  sfc = (sfcurve_object *) o->refine;

  if(!sfc)
    return AY_ENULL;

  if(type == UINT_MAX-AY_IDNCURVE)
    {
      /* peek */
      if(!sfc->ncurve)
	return AY_ERROR;
      *result = sfc->ncurve;
      return AY_OK;
    }

  if(type == AY_IDNCURVE)
    {
      if(sfc->ncurve)
	{
	  ay_status = ay_object_copy(sfc->ncurve, result);
	  if(*result)
	    {
	      ay_trafo_copy(o, *result);
	    } /* if */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
    } /* if */

 return ay_status;
} /* sfcurve_providecb */


/** sfcurve_genericopcb:
 * Execute generic operation (AY_OP*) on SfCurve.
 *
 * \param[in,out] o curve object to process
 * \param[in] op operation designation
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
sfcurve_genericopcb(ay_object *o, int op)
{
 int ay_status = AY_OK;
 sfcurve_object *sf = NULL;
 double t;

  if(!o)
    return AY_ENULL;

  if(o->type != sfcurve_id)
    return AY_ERROR;

  sf = (sfcurve_object *)o->refine;

  switch(op)
    {
    case AY_OPREVERT:
      t = sf->tmin;
      sf->tmax = sf->tmin;
      sf->tmin = t;
      break;
    case AY_OPREFINE:
      sf->sections *= 2;
      break;
    case AY_OPCOARSEN:
      sf->sections /= 2;
      if(sf->sections < 2)
	sf->sections = 2;
      break;
    default:
      break;
    } /* switch op */

 return ay_status;
} /* sfcurve_genericopcb */


/* Sfcurve_Init:
 * initializes the sfcurve module/plugin by registering a new
 * object type (SfCurve) and loading the accompanying Tcl script file.
 */
#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int
Sfcurve_Init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;
 char fname[] = "sfcurve_init";
 int i, ops[3] = {AY_OPREVERT, AY_OPREFINE, AY_OPCOARSEN};

#ifdef WIN32
  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }
#endif /* WIN32 */

  if(ay_checkversion(fname, sfcurve_version_ma, sfcurve_version_mi))
    {
      return TCL_ERROR;
    }

  ay_status = ay_otype_register(sfcurve_name,
				sfcurve_createcb,
				sfcurve_deletecb,
				sfcurve_copycb,
				sfcurve_drawcb,
				sfcurve_drawhcb,
				NULL, /* no surface */
				sfcurve_setpropcb,
				sfcurve_getpropcb,
				sfcurve_getpntcb,
				sfcurve_readcb,
				sfcurve_writecb,
				NULL, /* no RIB export */
				sfcurve_bbccb,
				&sfcurve_id);

  ay_status += ay_notify_register(sfcurve_notifycb, sfcurve_id);

  ay_status += ay_convert_register(sfcurve_convertcb, sfcurve_id);

  ay_status += ay_provide_register(sfcurve_providecb, sfcurve_id);

  for(i = 0; i < 3; i++)
    {
      ay_status += ay_tcmd_registergeneric(ops[i], sfcurve_genericopcb,
					   sfcurve_id);
    }

  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Error registering custom object!");
      return TCL_OK;
    }

  /* sfcurve objects may not be associated with materials */
  ay_matt_nomaterial(sfcurve_id);

  /* source sfcurve.tcl, it contains Tcl-code to build
     the SfCurve-Attributes Property GUI */
  if((Tcl_EvalFile(interp, "sfcurve.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname,
		  "Error while sourcing \"sfcurve.tcl\"!");
       return TCL_OK;
     }

  ay_error(AY_EOUTPUT, fname,
	   "Custom object \"SfCurve\" successfully loaded.");

 return TCL_OK;
} /* Sfcurve_Init */
