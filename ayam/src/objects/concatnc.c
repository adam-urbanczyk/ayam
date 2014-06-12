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

/* concatnc.c - concatnc object */

static char *ay_concatnc_name = "ConcatNC";

int ay_concatnc_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* ay_concatnc_createcb:
 *  create callback function of concatnc object
 */
int
ay_concatnc_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtconcatnc";
 ay_concatnc_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_concatnc_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->ftlength = 0.3;
  new->knot_type = 1;

  o->parent = AY_TRUE;

  o->refine = new;

 return AY_OK;
} /* ay_concatnc_createcb */


/* ay_concatnc_deletecb:
 *  delete callback function of concatnc object
 */
int
ay_concatnc_deletecb(void *c)
{
 ay_concatnc_object *concatnc = NULL;

  if(!c)
    return AY_ENULL;

  concatnc = (ay_concatnc_object *)(c);

  if(concatnc->ncurve)
    (void)ay_object_delete(concatnc->ncurve);

  free(concatnc);

 return AY_OK;
} /* ay_concatnc_deletecb */


/* ay_concatnc_copycb:
 *  copy callback function of concatnc object
 */
int
ay_concatnc_copycb(void *src, void **dst)
{
 ay_concatnc_object *concatnc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(concatnc = malloc(sizeof(ay_concatnc_object))))
    return AY_EOMEM;

  memcpy(concatnc, src, sizeof(ay_concatnc_object));

  concatnc->ncurve = NULL;

  *dst = (void *)concatnc;

 return AY_OK;
} /* ay_concatnc_copycb */


/* ay_concatnc_drawcb:
 *  draw (display in an Ayam view window) callback function of concatnc object
 */
int
ay_concatnc_drawcb(struct Togl *togl, ay_object *o)
{
 ay_concatnc_object *concatnc = NULL;

  if(!o)
    return AY_ENULL;

  concatnc = (ay_concatnc_object *)o->refine;

  if(!concatnc)
    return AY_ENULL;

  if(concatnc->ncurve)
    {
      ay_draw_object(togl, concatnc->ncurve, AY_TRUE);
    }

 return AY_OK;
} /* ay_concatnc_drawcb */


/* ay_concatnc_shadecb:
 *  shade (display in an Ayam view window) callback function of concatnc object
 */
int
ay_concatnc_shadecb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_concatnc_shadecb */


/* ay_concatnc_drawacb:
 *  draw annotations (in an Ayam view window) callback function of
 *  concatnc object
 */
int
ay_concatnc_drawacb(struct Togl *togl, ay_object *o)
{
 ay_concatnc_object *concatnc = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  concatnc = (ay_concatnc_object *)o->refine;

  if(!concatnc)
    return AY_ENULL;

  if(concatnc && concatnc->ncurve)
    {
      /* get NURBS curve */
      nc = (ay_nurbcurve_object *)concatnc->ncurve->refine;

      /* draw arrow */
      ay_draw_arrow(togl, &(nc->controlv[nc->length*4-8]),
		    &(nc->controlv[nc->length*4-4]));
    }

 return AY_OK;
} /* ay_concatnc_drawacb */


/* ay_concatnc_drawhcb:
 *  draw handles (in an Ayam view window) callback function of concatnc object
 */
int
ay_concatnc_drawhcb(struct Togl *togl, ay_object *o)
{
 int i = 0, a = 0;
 ay_concatnc_object *concatnc = NULL;
 ay_nurbcurve_object *nc = NULL;
 double *pnts = NULL;

  if(!o)
    return AY_ENULL;

  concatnc = (ay_concatnc_object *)o->refine;

  if(!concatnc)
    return AY_ENULL;

  if(concatnc->ncurve)
    {
      /* get NURBS curve */
      nc = (ay_nurbcurve_object *)concatnc->ncurve->refine;

      pnts = nc->controlv;

      /* draw read only points */
      glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
		(GLfloat)ay_prefs.obb);

      glBegin(GL_POINTS);
       for(i = 0; i < nc->length; i++)
	 {
	   glVertex3dv((GLdouble *)&pnts[a]);
	   a += 4;
	 }
      glEnd();

      glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
		(GLfloat)ay_prefs.seb);
    } /* if */

 return AY_OK;
} /* ay_concatnc_drawhcb */


/* ay_concatnc_getpntcb:
 *  get point (editing and selection) callback function of concatnc object
 */
int
ay_concatnc_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbcurve_object *curve = NULL;
 ay_concatnc_object *concatnc = NULL;

  if(!o)
    return AY_ENULL;

  concatnc = (ay_concatnc_object *)o->refine;

  if(!concatnc)
    return AY_ENULL;

  if(concatnc->ncurve)
    {
      curve = (ay_nurbcurve_object *)concatnc->ncurve->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, curve->length, 4,
			     curve->controlv);
    }

 return AY_ERROR;
} /* ay_concatnc_getpntcb */


/* ay_concatnc_setpropcb:
 *  set property (from Tcl to C context) callback function of concatnc object
 */
int
ay_concatnc_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "ConcatNCAttrData";
 /*char fname[] = "concatnc_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_concatnc_object *concatnc = NULL;

  if(!interp || !o)
    return AY_ENULL;

  concatnc = (ay_concatnc_object *)o->refine;

  if(!concatnc)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(concatnc->closed));

  Tcl_SetStringObj(ton,"FillGaps",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(concatnc->fillgaps));

  Tcl_SetStringObj(ton,"Revert",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(concatnc->revert));

  Tcl_SetStringObj(ton,"Knot-Type",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(concatnc->knot_type));

  Tcl_SetStringObj(ton,"FTLength",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(concatnc->ftlength));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_concatnc_setpropcb */


/* ay_concatnc_getpropcb:
 *  get property (from C to Tcl context) callback function of concatnc object
 */
int
ay_concatnc_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="ConcatNCAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_concatnc_object *concatnc = NULL;

  if(!interp || !o)
    return AY_ENULL;

  concatnc = (ay_concatnc_object *)(o->refine);

  if(!concatnc)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_NewIntObj(concatnc->closed);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"FillGaps",-1);
  to = Tcl_NewIntObj(concatnc->fillgaps);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Revert",-1);
  to = Tcl_NewIntObj(concatnc->revert);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Knot-Type",-1);
  to = Tcl_NewIntObj(concatnc->knot_type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"FTLength",-1);
  to = Tcl_NewDoubleObj(concatnc->ftlength);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getncinfo(interp, n1, concatnc->ncurve);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_concatnc_getpropcb */


/* ay_concatnc_readcb:
 *  read (from scene file) callback function of concatnc object
 */
int
ay_concatnc_readcb(FILE *fileptr, ay_object *o)
{
 ay_concatnc_object *concatnc = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(concatnc = calloc(1, sizeof(ay_concatnc_object))))
    { return AY_EOMEM; }

  concatnc->ftlength = 0.3;

  fscanf(fileptr, "%d\n", &concatnc->closed);
  fscanf(fileptr, "%d\n", &concatnc->fillgaps);
  fscanf(fileptr, "%d\n", &concatnc->revert);

  if(ay_read_version >= 5)
    {
      /* since 1.6 */
      fscanf(fileptr, "%d\n", &concatnc->knot_type);
      fscanf(fileptr, "%lg\n", &concatnc->ftlength);
    }

  if(ay_read_version < 15)
    {
      if(concatnc->knot_type == 1)
	concatnc->fillgaps = AY_TRUE;
    }

  o->refine = concatnc;

 return AY_OK;
} /* ay_concatnc_readcb */


/* ay_concatnc_writecb:
 *  write (to scene file) callback function of concatnc object
 */
int
ay_concatnc_writecb(FILE *fileptr, ay_object *o)
{
 ay_concatnc_object *concatnc = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  concatnc = (ay_concatnc_object *)(o->refine);

  if(!concatnc)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", concatnc->closed);
  fprintf(fileptr, "%d\n", concatnc->fillgaps);
  fprintf(fileptr, "%d\n", concatnc->revert);
  fprintf(fileptr, "%d\n", concatnc->knot_type);
  fprintf(fileptr, "%g\n", concatnc->ftlength);

 return AY_OK;
} /* ay_concatnc_writecb */


/* ay_concatnc_wribcb:
 *  RIB export callback function of concatnc object
 */
int
ay_concatnc_wribcb(char *file, ay_object *o)
{
  /*
 ay_concatnc_object *concatnc = NULL;

  if(!o)
   return AY_ENULL;

  concatnc = (ay_concatnc_object*)o->refine;
  */
 return AY_OK;
} /* ay_concatnc_wribcb */


/* ay_concatnc_bbccb:
 *  bounding box calculation callback function of concatnc object
 */
int
ay_concatnc_bbccb(ay_object *o, double *bbox, int *flags)
{

  if(!o || !bbox || !flags)
    return AY_ENULL;

  /* use the bounding boxes of the children */
  *flags = 2;

 return AY_OK;
} /* ay_concatnc_bbccb */


/* ay_concatnc_notifycb:
 *  notification callback function of concatnc object
 */
int
ay_concatnc_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "concatnc_notifycb";
 ay_concatnc_object *concatnc = NULL;
 ay_object *down = NULL, *ncurve = NULL, *curves = NULL, **next = NULL;
 ay_nurbcurve_object *nc = NULL;
 int numcurves = 0, highest_order = 0;

  if(!o)
    return AY_ENULL;

  concatnc = (ay_concatnc_object *)(o->refine);

  if(!concatnc)
    return AY_ENULL;

  if(concatnc->ncurve)
    (void)ay_object_delete(concatnc->ncurve);
  concatnc->ncurve = NULL;

  /* get child objects */
  down = o->down;
  next = &curves;
  while(down)
    {
      /* copy or provide */
      if(down->type == AY_IDNCURVE)
	{
	  nc = (ay_nurbcurve_object *)(down->refine);
	  if(nc->order > highest_order)
	    highest_order = nc->order;
	  ay_object_copy(down, next);
	  next = &((*next)->next);
	  numcurves++;
	}
      else
	{
	  ay_provide_object(down, AY_IDNCURVE, next);
	  while(*next)
	    {
	      nc = (ay_nurbcurve_object *)((*next)->refine);
	      if(nc->order > highest_order)
		highest_order = nc->order;
	      next = &((*next)->next);
	      numcurves++;
	    }
	}

      down = down->next;
    } /* while */

  if(!curves || (numcurves < 2 && !concatnc->fillgaps))
    {
      goto cleanup;
    }

  ncurve = curves;
  while(ncurve)
    {
      (void)ay_nct_applytrafo(ncurve);
      ay_status = ay_nct_elevate((ay_nurbcurve_object *)(ncurve->refine),
				 highest_order);
      ay_status += ay_nct_clamp((ay_nurbcurve_object *)ncurve->refine, 0);
      if(ay_status)
	goto cleanup;
      ncurve = ncurve->next;
    } /* while */

  if(concatnc->knot_type == 1)
    {
      /* to create the custom knot vector, all parameter curve
	 knots must be defined in the range (0.0-1.0) */
      ncurve = curves;
      while(ncurve)
	{
	  nc = (ay_nurbcurve_object*)ncurve->refine;
	  ay_knots_rescaletorange(nc->length+nc->order, nc->knotv, 0.0, 1.0);
	  ncurve = ncurve->next;
	} /* while */
    }

  /* create fillets? */
  if(concatnc->fillgaps)
    {
      ay_status = ay_nct_fillgaps(concatnc->closed, highest_order,
				  concatnc->ftlength, curves);
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Failed to create fillets!");
	}
    }

  ay_status = ay_nct_concatmultiple(concatnc->closed, concatnc->knot_type,
				    concatnc->fillgaps, curves,
				    &concatnc->ncurve);

  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Failed to concat curves!");
    }
  else
    {
      nc = (ay_nurbcurve_object*)concatnc->ncurve->refine;
      if(concatnc->revert)
	{
	  ay_status = ay_nct_revert(nc);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, "Could not revert curve!");
	    }
	}

      if(concatnc->closed && (!concatnc->fillgaps))
	{
	  ay_status = ay_nct_close(nc);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, "Could not close curve!");
	    }
	}
    } /* if */

cleanup:

  /* free list of temporary curves */
  (void)ay_object_deletemulti(curves, AY_FALSE);

  /* recover selected points */
  if(o->selp)
    {
      if(concatnc->ncurve)
	ay_concatnc_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

 return ay_status;
} /* ay_concatnc_notifycb */


/* ay_concatnc_convertcb:
 *  convert callback function of concatnc object
 */
int
ay_concatnc_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_concatnc_object *concatnc = NULL;
 ay_object *new = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  concatnc = (ay_concatnc_object *) o->refine;

  if(!concatnc)
    return AY_ENULL;

  if(concatnc->ncurve)
    {
      ay_status = ay_object_copy(concatnc->ncurve, &new);
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

	  /* immediately create and show the multiple points */
	  nc->createmp = AY_TRUE;
	  ay_nct_recreatemp(nc);

	  ay_trafo_copy(o, new);

	  if(!in_place)
	    {
	      ay_object_link(new);
	    }
	  else
	    {
	      ay_status = ay_object_replace(new, o);
	    }
	} /* if */
    } /* if */

 return ay_status;
} /* ay_concatnc_convertcb */


/* ay_concatnc_providecb:
 *  provide callback function of concatnc object
 */
int
ay_concatnc_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 ay_concatnc_object *concatnc = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNCURVE)
	return AY_OK;
      else
	return AY_ERROR;
    }

  concatnc = (ay_concatnc_object *) o->refine;

  if(!concatnc)
    return AY_ENULL;

  if(type == AY_IDNCURVE)
    {
      if(concatnc->ncurve)
	{
	  ay_status = ay_object_copy(concatnc->ncurve, result);
	  if(*result)
	    {
	      ay_trafo_copy(o, *result);
	    }
	}
      else
	{
	  return AY_ERROR;
	}
    } /* if */

 return ay_status;
} /* ay_concatnc_providecb */


/* ay_concatnc_init:
 *  initialize the concatnc object module
 */
int
ay_concatnc_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_concatnc_name,
				    ay_concatnc_createcb,
				    ay_concatnc_deletecb,
				    ay_concatnc_copycb,
				    ay_concatnc_drawcb,
				    ay_concatnc_drawhcb,
				    NULL, /* no shading */
				    ay_concatnc_setpropcb,
				    ay_concatnc_getpropcb,
				    ay_concatnc_getpntcb,
				    ay_concatnc_readcb,
				    ay_concatnc_writecb,
				    NULL, /* no RIB export */
				    ay_concatnc_bbccb,
				    AY_IDCONCATNC);

  ay_status += ay_draw_registerdacb(ay_concatnc_drawacb, AY_IDCONCATNC);

  ay_status += ay_notify_register(ay_concatnc_notifycb, AY_IDCONCATNC);

  ay_status += ay_convert_register(ay_concatnc_convertcb, AY_IDCONCATNC);

  ay_status += ay_provide_register(ay_concatnc_providecb, AY_IDCONCATNC);

  /* concatnc objects may not be associated with materials */
  ay_matt_nomaterial(AY_IDCONCATNC);

 return ay_status;
} /* ay_concatnc_init */
