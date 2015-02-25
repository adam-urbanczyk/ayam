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

/* ncircle.c - ncircle object */

static char *ay_ncircle_name = "NCircle";

int ay_ncircle_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_ncircle_createcb:
 *  create callback function of ncircle object
 */
int
ay_ncircle_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 int tcl_status = TCL_OK;
 char fname[] = "crtncircle";
 char option_handled = AY_FALSE;
 int optnum = 0, i = 2;
 double radius = 1.0, tmin = 0.0, tmax = 360.0;
 ay_ncircle_object *ncircle = NULL;

  if(!o)
    return AY_ENULL;

  /* parse args */
  while(i+1 < argc)
    {
      if(i+1 >= argc)
	{
	  ay_error(AY_EOPT, fname, argv[i]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

      tcl_status = TCL_OK;
      option_handled = AY_FALSE;
      optnum = i;
      if(argv[i] && argv[i][0] != '\0')
	{
	  switch(argv[i][1])
	    {
	    case 'r':
	      /* -radius */
	      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &radius);
	      option_handled = AY_TRUE;
	      break;
	    case 't':
	      if(argv[i][2] != '\0' && argv[i][3] != '\0')
		{
		  switch(argv[i][3])
		    {
		    case 'a':
		      /* -tmax */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &tmax);
		      option_handled = AY_TRUE;
		      break;
		    case 'i':
		      /* -tmin */
		      tcl_status = Tcl_GetDouble(ay_interp, argv[i+1], &tmin);
		      option_handled = AY_TRUE;
		      break;
		    default:
		      break;
		    } /* switch */
		} /* if */
	    default:
	      break;
	    } /* switch */

	  if(option_handled && (tcl_status != TCL_OK))
	    {
	      ay_error(AY_EOPT, fname, argv[i]);
	      ay_status = AY_ERROR;
	      goto cleanup;
	    }

	  i += 2;
	}
      else
	{
	  i++;
	} /* if */

      if(!option_handled)
	{
	  ay_error(AY_EUOPT, fname, argv[optnum]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

    } /* while */

  if(!(ncircle = calloc(1, sizeof(ay_ncircle_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  ncircle->radius = radius;
  ncircle->tmin = tmin;
  ncircle->tmax = tmax;

  o->refine = ncircle;

  ay_notify_object(o);

cleanup:

 return ay_status;
} /* ay_ncircle_createcb */


/* ay_ncircle_deletecb:
 *  delete callback function of ncircle object
 */
int
ay_ncircle_deletecb(void *c)
{
 ay_ncircle_object *ncircle = NULL;

  if(!c)
    return AY_ENULL;

  ncircle = (ay_ncircle_object *)(c);

  if(ncircle->ncurve)
    (void)ay_object_delete(ncircle->ncurve);

  ncircle->ncurve = NULL;

  free(ncircle);

 return AY_OK;
} /* ay_ncircle_deletecb */


/* ay_ncircle_copycb:
 *  copy callback function of ncircle object
 */
int
ay_ncircle_copycb(void *src, void **dst)
{
 ay_ncircle_object *ncircle = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(ncircle = malloc(sizeof(ay_ncircle_object))))
    return AY_EOMEM;

  memcpy(ncircle, src, sizeof(ay_ncircle_object));

  ncircle->ncurve = NULL;

  *dst = (void *)ncircle;

 return AY_OK;
} /* ay_ncircle_copycb */


/* ay_ncircle_drawcb:
 *  draw (display in an Ayam view window) callback function of ncircle object
 */
int
ay_ncircle_drawcb(struct Togl *togl, ay_object *o)
{
 ay_ncircle_object *ncircle = NULL;

  if(!o)
    return AY_ENULL;

  ncircle = (ay_ncircle_object *)o->refine;

  if(!ncircle)
    return AY_ENULL;

  if(ncircle->ncurve)
    ay_draw_object(togl, ncircle->ncurve, AY_TRUE);

 return AY_OK;
} /* ay_ncircle_drawcb */


/* ay_ncircle_shadecb:
 *  shade (display in an Ayam view window) callback function of ncircle object
 */
int
ay_ncircle_shadecb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_ncircle_shadecb */


/* ay_ncircle_drawacb:
 *  draw annotations (in an Ayam view window) callback function of
 *  ncircle object
 */
int
ay_ncircle_drawacb(struct Togl *togl, ay_object *o)
{
 ay_ncircle_object *ncircle = NULL;
 ay_nurbcurve_object *curve = NULL;

  if(!o)
    return AY_ENULL;

  ncircle = (ay_ncircle_object *) o->refine;

  if(!ncircle)
    return AY_ENULL;

  if(ncircle && ncircle->ncurve)
    {
      /* get NURBS curve */
      curve = (ay_nurbcurve_object *)ncircle->ncurve->refine;

      /* draw arrow */
      ay_draw_arrow(togl, &(curve->controlv[curve->length*4-8]),
		    &(curve->controlv[curve->length*4-4]));
    }

 return AY_OK;
} /* ay_ncircle_drawacb */


/* ay_ncircle_drawhcb:
 *  draw handles (in an Ayam view window) callback function of ncircle object
 */
int
ay_ncircle_drawhcb(struct Togl *togl, ay_object *o)
{
 int i;
 double *pnts;
 ay_ncircle_object *ncircle;
 ay_nurbcurve_object *curve;

  if(!o)
    return AY_ENULL;

  ncircle = (ay_ncircle_object *) o->refine;

  if(!ncircle)
    return AY_ENULL;

  if(ncircle->ncurve)
    {
      /* get NURBS curve */
      curve = (ay_nurbcurve_object *)ncircle->ncurve->refine;
      pnts = curve->controlv;

      /* draw read only points */
      glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
		(GLfloat)ay_prefs.obb);

      glBegin(GL_POINTS);
       if(ay_prefs.rationalpoints)
	 {
	   for(i = 0; i < curve->length; i++)
	     {
	       glVertex3d((GLdouble)pnts[0]*pnts[3],
			  (GLdouble)pnts[1]*pnts[3],
			  (GLdouble)pnts[2]*pnts[3]);
	       pnts += 4;
	     }
	 }
       else
	 {
	   for(i = 0; i < curve->length; i++)
	     {
	       glVertex3dv((GLdouble *)pnts);
	       pnts += 4;
	     }
	 }
      glEnd();

      glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
		(GLfloat)ay_prefs.seb);
    } /* if */

 return AY_OK;
} /* ay_ncircle_drawhcb */


/* ay_ncircle_getpntcb:
 *  get point (editing and selection) callback function of ncircle object
 */
int
ay_ncircle_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbcurve_object *curve = NULL;
 ay_ncircle_object *ncircle = NULL;

  if(!o)
    return AY_ENULL;

  ncircle = (ay_ncircle_object *)o->refine;

  if(ncircle->ncurve)
    {
      curve = (ay_nurbcurve_object *)ncircle->ncurve->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, curve->length, 4,
			     curve->controlv);
    }

 return AY_ERROR;
} /* ay_ncircle_getpntcb */


/* ay_ncircle_setpropcb:
 *  set property (from Tcl to C context) callback function of ncircle object
 */
int
ay_ncircle_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "NCircleAttrData";
 /*char fname[] = "ncircle_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_ncircle_object *ncircle = NULL;

  if(!interp || !o)
    return AY_ENULL;

  ncircle = (ay_ncircle_object *)o->refine;

  if(!ncircle)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Radius",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(ncircle->radius));

  Tcl_SetStringObj(ton,"TMin",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(ncircle->tmin));

  Tcl_SetStringObj(ton,"TMax",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(ncircle->tmax));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(ncircle->glu_sampling_tolerance));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(ncircle->display_mode));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_ncircle_setpropcb */


/* ay_ncircle_getpropcb:
 *  get property (from C to Tcl context) callback function of ncircle object
 */
int
ay_ncircle_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="NCircleAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_ncircle_object *ncircle = NULL;

  if(!interp || !o)
    return AY_ENULL;

  ncircle = (ay_ncircle_object *)(o->refine);

  if(!ncircle)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Radius",-1);
  to = Tcl_NewDoubleObj(ncircle->radius);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"TMin",-1);
  to = Tcl_NewDoubleObj(ncircle->tmin);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"TMax",-1);
  to = Tcl_NewDoubleObj(ncircle->tmax);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(ncircle->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(ncircle->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getncinfo(interp, n1, ncircle->ncurve);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_ncircle_getpropcb */


/* ay_ncircle_readcb:
 *  read (from scene file) callback function of ncircle object
 */
int
ay_ncircle_readcb(FILE *fileptr, ay_object *o)
{
 ay_ncircle_object *ncircle = NULL;

 if(!fileptr || !o)
   return AY_ENULL;

  if(!(ncircle = calloc(1, sizeof(ay_ncircle_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%lg\n",&ncircle->radius);
  fscanf(fileptr,"%lg\n",&ncircle->tmin);
  fscanf(fileptr,"%lg\n",&ncircle->tmax);
  fscanf(fileptr,"%lg\n",&ncircle->glu_sampling_tolerance);
  fscanf(fileptr,"%d\n",&ncircle->display_mode);

  o->refine = ncircle;

 return AY_OK;
} /* ay_ncircle_readcb */


/* ay_ncircle_writecb:
 *  write (to scene file) callback function of ncircle object
 */
int
ay_ncircle_writecb(FILE *fileptr, ay_object *o)
{
 ay_ncircle_object *ncircle = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  ncircle = (ay_ncircle_object *)(o->refine);

  if(!ncircle)
    return AY_ENULL;

  fprintf(fileptr, "%g\n", ncircle->radius);
  fprintf(fileptr, "%g\n", ncircle->tmin);
  fprintf(fileptr, "%g\n", ncircle->tmax);
  fprintf(fileptr, "%g\n", ncircle->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", ncircle->display_mode);

 return AY_OK;
} /* ay_ncircle_writecb */


/* ay_ncircle_wribcb:
 *  RIB export callback function of ncircle object
 */
int
ay_ncircle_wribcb(char *file, ay_object *o)
{
  /*
  ay_ncircle_object *ncircle = NULL;

  if(!o)
   return AY_ENULL;

  ncircle = (ay_ncircle_object*)o->refine;
  */
 return AY_OK;
} /* ay_ncircle_wribcb */


/* ay_ncircle_bbccb:
 *  bounding box calculation callback function of ncircle object
 */
int
ay_ncircle_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_ncircle_object *ncircle = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  ncircle = (ay_ncircle_object *)o->refine;

  if(!ncircle)
    return AY_ENULL;

  if(ncircle->ncurve)
    {
      ay_bbc_get(ncircle->ncurve, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    } /* if */

 return AY_OK;
} /* ay_ncircle_bbccb */


/* ay_ncircle_notifycb:
 *  notification callback function of ncircle object
 */
int
ay_ncircle_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_ncircle_object *ncircle = NULL;
 ay_nurbcurve_object *nc = NULL;
 ay_object *ncurve = NULL;
 int revert = AY_FALSE;

  if(!o)
    return AY_ENULL;

  ncircle = (ay_ncircle_object *)(o->refine);

  if(!ncircle)
    return AY_ENULL;

  if(ncircle->ncurve)
    (void)ay_object_delete(ncircle->ncurve);

  ncircle->ncurve = NULL;

  if(!(nc = calloc(1, sizeof(ay_nurbcurve_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  if(ncircle->tmin > ncircle->tmax)
    {
      revert = AY_TRUE;
      ay_status = ay_nb_CreateNurbsCircleArc(ncircle->radius,
					 ncircle->tmax, ncircle->tmin,
			         &(nc->length), &(nc->knotv), &(nc->controlv));
    }
  else
    {
      ay_status = ay_nb_CreateNurbsCircleArc(ncircle->radius,
					 ncircle->tmin, ncircle->tmax,
			         &(nc->length), &(nc->knotv), &(nc->controlv));
    }

  if(ay_status)
    goto cleanup;

  nc->order = 3;
  nc->knot_type = AY_KTCUSTOM;
  nc->is_rat = AY_TRUE;
  nc->display_mode = ncircle->display_mode;
  nc->glu_sampling_tolerance = ncircle->glu_sampling_tolerance;

  if(revert)
    ay_nct_revert(nc);

  ay_nct_settype(nc);

  if(!(ncurve = calloc(1, sizeof(ay_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  ncurve->type = AY_IDNCURVE;
  ay_object_defaults(ncurve);
  ncurve->refine = nc;
  nc = NULL;

  ncircle->ncurve = ncurve;

cleanup:

  if(nc)
    ay_nct_destroy(nc);

  /* recover selected points */
  if(o->selp)
    {
      if(ncircle->ncurve)
	ay_ncircle_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

 return ay_status;
} /* ay_ncircle_notifycb */


/* ay_ncircle_convertcb:
 *  convert callback function of ncircle object
 */
int
ay_ncircle_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_ncircle_object *ncircle = NULL;
 ay_object *new = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  ncircle = (ay_ncircle_object *) o->refine;

  if(!ncircle)
    return AY_ENULL;

  if(ncircle->ncurve)
    {
      ay_status = ay_object_copy(ncircle->ncurve, &new);

      if(new)
	{
	  /* reset display mode and sampling tolerance
	     of new curve to "global"? */
	  if(!in_place && ay_prefs.conv_reset_display)
	    {
	      nc = (ay_nurbcurve_object *)(new->refine);
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
} /* ay_ncircle_convertcb */


/* ay_ncircle_providecb:
 *  provide callback function of ncircle object
 */
int
ay_ncircle_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 ay_ncircle_object *ncircle = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNCURVE)
	return AY_OK;
      else
	return AY_ERROR;
    }

  ncircle = (ay_ncircle_object *) o->refine;

  if(!ncircle)
    return AY_ENULL;

  if(type == AY_IDNCURVE)
    {
      if(ncircle->ncurve)
	{
	  ay_status = ay_object_copy(ncircle->ncurve, result);
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
} /* ay_ncircle_providecb */


/* ay_ncircle_init:
 *  initialize the ncircle object module
 */
int
ay_ncircle_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_ncircle_name,
				    ay_ncircle_createcb,
				    ay_ncircle_deletecb,
				    ay_ncircle_copycb,
				    ay_ncircle_drawcb,
				    ay_ncircle_drawhcb,
				    NULL, /* no shading */
				    ay_ncircle_setpropcb,
				    ay_ncircle_getpropcb,
				    ay_ncircle_getpntcb,
				    ay_ncircle_readcb,
				    ay_ncircle_writecb,
				    NULL, /* no RIB export */
				    ay_ncircle_bbccb,
				    AY_IDNCIRCLE);


  ay_status += ay_draw_registerdacb(ay_ncircle_drawacb, AY_IDNCIRCLE);

  ay_status += ay_notify_register(ay_ncircle_notifycb, AY_IDNCIRCLE);

  ay_status += ay_convert_register(ay_ncircle_convertcb, AY_IDNCIRCLE);

  ay_status += ay_provide_register(ay_ncircle_providecb, AY_IDNCIRCLE);

  /* ncircles may not be associated with materials */
  ay_matt_nomaterial(AY_IDNCIRCLE);

 return ay_status;
} /* ay_ncircle_init */
