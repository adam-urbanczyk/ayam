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

/* extrude.c - extrude object */

static char *ay_extrude_name = "Extrude";

int
ay_extrude_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtextrude";
 ay_extrude_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_extrude_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->height = 1.0;

  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_extrude_createcb */


int
ay_extrude_deletecb(void *c)
{
 ay_extrude_object *extrude = NULL;

  if(!c)
    return AY_ENULL;

  extrude = (ay_extrude_object *)(c);

  if(extrude->npatch)
    ay_object_deletemulti(extrude->npatch);

  if(extrude->caps_and_bevels)
    {
      ay_object_deletemulti(extrude->caps_and_bevels);
      extrude->caps_and_bevels = NULL;
    }

  free(extrude);

 return AY_OK;
} /* ay_extrude_deletecb */


int
ay_extrude_copycb(void *src, void **dst)
{
 ay_extrude_object *extrude = NULL, *extrudesrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  extrudesrc = (ay_extrude_object *)src;

  if(!(extrude = calloc(1, sizeof(ay_extrude_object))))
    return AY_EOMEM;

  memcpy(extrude, src, sizeof(ay_extrude_object));

  extrude->npatch = NULL;

  extrude->caps_and_bevels = NULL;

  *dst = (void *)extrude;

 return AY_OK;
} /* ay_extrude_copycb */


int
ay_extrude_drawcb(struct Togl *togl, ay_object *o)
{
 ay_extrude_object *extrude = NULL;
 ay_object *p = NULL;

  if(!o)
    return AY_ENULL;

  extrude = (ay_extrude_object *)o->refine;

  if(!extrude)
    return AY_ENULL;

  p = extrude->npatch;
  while(p)
    {
      ay_draw_object(togl, p, AY_TRUE);
      p = p->next;
    }

  p = extrude->caps_and_bevels;
  while(p)
    {
      ay_draw_object(togl, p, AY_TRUE);
      p = p->next;
    }

 return AY_OK;
} /* ay_extrude_drawcb */


int
ay_extrude_shadecb(struct Togl *togl, ay_object *o)
{
 ay_extrude_object *extrude = NULL;
 ay_object *p = NULL;

  if(!o)
    return AY_ENULL;

  extrude = (ay_extrude_object *)o->refine;

  if(!extrude)
    return AY_ENULL;

  p = extrude->npatch;
  while(p)
    {
      ay_shade_object(togl, p, AY_FALSE);
      p = p->next;
    }

  p = extrude->caps_and_bevels;
  while(p)
    {
      ay_shade_object(togl, p, AY_FALSE);
      p = p->next;
    }

 return AY_OK;
} /* ay_extrude_shadecb */


int
ay_extrude_drawhcb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_extrude_drawhcb */


int
ay_extrude_getpntcb(int mode, ay_object *o, double *p)
{

 return AY_OK;
} /* ay_extrude_getpntcb */


/* Tcl -> C! */
int
ay_extrude_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "ExtrudeAttrData";
 /* char fname[] = "extrude_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_extrude_object *extrude = NULL;


  if(!o)
    return AY_ENULL;

  extrude = (ay_extrude_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(extrude->height));

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(extrude->has_upper_cap));

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(extrude->has_lower_cap));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(extrude->glu_display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(extrude->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_force(o);

  o->modified = AY_TRUE;
  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_extrude_setpropcb */


/* C -> Tcl! */
int
ay_extrude_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="ExtrudeAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_extrude_object *extrude = NULL;

  if(!o)
    return AY_ENULL;

  extrude = (ay_extrude_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Height",-1);
  to = Tcl_NewDoubleObj(extrude->height);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_NewIntObj(extrude->has_upper_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_NewIntObj(extrude->has_lower_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(extrude->glu_display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(extrude->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_extrude_getpropcb */


int
ay_extrude_readcb(FILE *fileptr, ay_object *o)
{
 ay_extrude_object *extrude = NULL;
 ay_tag tag = {0}, *stag = NULL, *etag = NULL;
 char vbuf[128], nbuf[3] = "BP";
 int has_startb = AY_FALSE, has_startb2 = AY_FALSE;
 int has_endb = AY_FALSE, has_endb2 = AY_FALSE;
 int startb_type, startb_type2, endb_type, startb_sense, endb_sense;
 double startb_radius, startb_radius2, endb_radius;
 
  if(!o)
    return AY_ENULL;

  if(!(extrude = calloc(1, sizeof(ay_extrude_object))))
    { return AY_EOMEM; }

  fscanf(fileptr, "%lg\n", &extrude->height);
  fscanf(fileptr, "%d\n", &extrude->has_upper_cap);
  fscanf(fileptr, "%d\n", &extrude->has_lower_cap);
  fscanf(fileptr, "%d\n", &has_endb2);
  fscanf(fileptr, "%d\n", &has_startb2);
  fscanf(fileptr, "%d\n", &startb_type2);
  fscanf(fileptr, "%lg\n", &startb_radius2);
  fscanf(fileptr, "%d\n", &extrude->glu_display_mode);
  fscanf(fileptr, "%lg\n", &extrude->glu_sampling_tolerance);

  /* get bevel parameters from potentially present BP tags */
  ay_npt_getbeveltags(o, 0, &has_startb, &startb_type, &startb_radius,
		      &startb_sense);
  ay_npt_getbeveltags(o, 1, &has_endb, &endb_type, &endb_radius,
		      &endb_sense);

  tag.name = nbuf;
  tag.type = ay_bp_tagtype;
  tag.val = vbuf;
  if(!has_startb && has_startb2)
    {
      sprintf(vbuf, "0,%d,%g,0", startb_type2, startb_radius2);
      ay_tags_copy(&tag, &stag);
      ay_tags_append(o, stag);
    }

  if(!has_endb && has_endb2)
    {
      sprintf(vbuf, "1,%d,%g,0", startb_type2, startb_radius2);
      ay_tags_copy(&tag, &etag);
      ay_tags_append(o, etag);
    }

  o->refine = extrude;

 return AY_OK;
} /* ay_extrude_readcb */


int
ay_extrude_writecb(FILE *fileptr, ay_object *o)
{
 ay_extrude_object *extrude = NULL;
 int has_startb = AY_FALSE, has_endb = AY_FALSE;
 int startb_type, endb_type, startb_sense, endb_sense;
 double startb_radius, endb_radius;

  if(!o)
    return AY_ENULL;

  extrude = (ay_extrude_object *)(o->refine);

  /* get bevel parameters */
  ay_npt_getbeveltags(o, 0, &has_startb, &startb_type, &startb_radius,
		      &startb_sense);
  ay_npt_getbeveltags(o, 1, &has_endb, &endb_type, &endb_radius,
		      &endb_sense);

  fprintf(fileptr, "%g\n", extrude->height);
  fprintf(fileptr, "%d\n", extrude->has_upper_cap);
  fprintf(fileptr, "%d\n", extrude->has_lower_cap);
  fprintf(fileptr, "%d\n", has_endb);
  fprintf(fileptr, "%d\n", has_startb);
  fprintf(fileptr, "%d\n", startb_type);
  fprintf(fileptr, "%g\n", startb_radius);
  fprintf(fileptr, "%d\n", extrude->glu_display_mode);
  fprintf(fileptr, "%g\n", extrude->glu_sampling_tolerance);

 return AY_OK;
} /* ay_extrude_writecb */


int
ay_extrude_wribcb(char *file, ay_object *o)
{
 ay_extrude_object *extrude = NULL;
 ay_object *p =NULL;

  if(!o)
   return AY_ENULL;

  extrude = (ay_extrude_object*)o->refine;

  p = extrude->npatch;
  while(p)
    {
      ay_wrib_object(file, p);
      p = p->next;
    }

  p = extrude->caps_and_bevels;
  while(p)
    {
      ay_wrib_object(file, p);
      p = p->next;
    }

 return AY_OK;
} /* ay_extrude_wribcb */


int
ay_extrude_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_extrude_object *extrude = NULL;
 ay_object *np = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  extrude = (ay_extrude_object *)o->refine;

  if(extrude->npatch)
    {
      np = extrude->npatch;

      while(np->next)
	{
	  np = np->next;
	}

      ay_bbc_get(np, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return AY_OK;
} /* ay_extrude_bbccb */


int
ay_extrude_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_extrude_object *ext = NULL;
 ay_object *down = NULL, *c = NULL, *cap = NULL, *trim = NULL;
 ay_object *bevel = NULL, *startb = NULL, *endb = NULL, *tloop = NULL;
 ay_object *extrusion = NULL, *ne = NULL, *ot = NULL;
 ay_object *upper_cap = NULL, *lower_cap = NULL;
 ay_object **nextcb;
 ay_nurbcurve_object *curve = NULL;
 ay_nurbpatch_object *patch = NULL;
 int display_mode = 0, got_object = AY_FALSE;
 int has_startb = AY_FALSE, has_endb = AY_FALSE;
 int startb_type, endb_type, startb_sense, endb_sense;
 double startb_radius, endb_radius;
 double uminx, umaxx, uminy, umaxy;
 double lminx, lmaxx, lminy, lmaxy;
 double firstmovex = 0.0, firstmovey = 0.0;
 double trimmx = 0.0, trimmy = 0.0;
 double angle = 0.0, z = 0.0;
 double tolerance = 0.0;

  if(!o)
    return AY_OK;

  ext = o->refine;
  if(!ext)
    return AY_OK;

  /* ok, something changed below, we have to rebuild the extrusion(s) */

  tolerance = ext->glu_sampling_tolerance;
  display_mode = ext->glu_display_mode;

  /* clear old extrusions, caps and bevels */
  extrusion = ext->npatch;
  while(extrusion)
    {
      ne = extrusion->next;
      ay_object_delete(extrusion);
      extrusion = ne;
    }
  ext->npatch = NULL;

  if(ext->caps_and_bevels)
    {
      ay_object_deletemulti(ext->caps_and_bevels); 
    }
  ext->caps_and_bevels = NULL;

  nextcb = &(ext->caps_and_bevels);

  /* create new extrusions, caps and bevels */
  down = o->down;
  while(down)
    {
      got_object = AY_FALSE;
      c = NULL;
      if(down->type != AY_IDNCURVE)
	{
	  ay_status = ay_provide_object(down, AY_IDNCURVE, &c);
	  if(c)
	    got_object = AY_TRUE;
	}
      else
	{
	  c = down;
	}

      /* normally, we do this at the end of the while,
         but this is _so_ far away... */
      down = down->next;

      if(c)
	{
	  /* get bevel parameters from potentially present BP tags */
	  ay_npt_getbeveltags(o, 0, &has_startb, &startb_type, &startb_radius,
			      &startb_sense);
	  ay_npt_getbeveltags(o, 1, &has_endb, &endb_type, &endb_radius,
			      &endb_sense);

	  extrusion = ext->npatch;

	  ay_status = ay_npt_createnpatchobject(&(ext->npatch));

	  ay_npt_extrude(ext->height, c, (ay_nurbpatch_object **)
			 &(ext->npatch->refine));

	  ((ay_nurbpatch_object *)ext->npatch->refine)->
	    glu_display_mode = display_mode;
	  ((ay_nurbpatch_object *)ext->npatch->refine)->
	    glu_sampling_tolerance = tolerance;

	  ext->npatch->next = extrusion;

	  /* create and link bevels */

	  if(has_startb)
	    {
	      ay_status = ay_npt_createnpatchobject(&bevel);
	    
	      ay_status = ay_npt_bevel(startb_type,
				       startb_radius,
				       AY_FALSE,
				       c,
				    (ay_nurbpatch_object **)&(bevel->refine));

	      bevel->scalz = -1;
	      startb = bevel;
	      *nextcb = bevel;
	      nextcb = &(bevel->next);
	    } /* if */

	  if(has_endb)
	    {
	      ay_status = ay_npt_createnpatchobject(&bevel);

	      ay_status = ay_npt_bevel(endb_type,
				       endb_radius,
				       AY_FALSE,
				       c,
				    (ay_nurbpatch_object **)&(bevel->refine));

	      bevel->movz = ext->height;
	      endb = bevel;
	      *nextcb = bevel;
	      nextcb = &(bevel->next);
	    } /* if */
	    
	  /* create caps */
	  if(ext->has_upper_cap)
	    {
	      ay_status = ay_object_copy(c, &trim);
	      if(ay_status)
		return ay_status;

	      /* work around error with rotated curves */
	      ay_nct_applytrafo(trim);

	      curve = (ay_nurbcurve_object *)trim->refine;

	      /* first c? -> trimcurve must be clockwise */
	      if(!upper_cap)
		{
		  firstmovex = trim->movx;
		  firstmovey = trim->movy;

		  z = ext->height;
		  /*
		     if we have bevels, we need a new control vector
		     copied directly from the beveling patch,
		     this is the same vector as the vector from the
		     curve, but displaced by bevel_radius...
		   */
		  if(has_endb)
		    {
		      patch = (ay_nurbpatch_object*)endb->refine;
		      free(curve->controlv);
		      curve->controlv = NULL;
		      if(!(curve->controlv =
			   calloc(4*curve->length, sizeof(double))))
			return AY_EOMEM;
		      memcpy(curve->controlv,&(patch->controlv[
				(patch->width-1)*4*patch->height]),
			     4*curve->length*sizeof(double));
		      z += curve->controlv[2];
		      ay_trafo_defaults(trim);
		    }

		  ay_status = ay_npt_createnpatchobject(&cap);

		  ay_npt_createcap(z,curve,&uminx,&umaxx,&uminy,&umaxy,&angle,
				  (ay_nurbpatch_object **)&(cap->refine));

		  umaxx *= trim->scalx;
		  uminx *= trim->scalx;
		  umaxy *= trim->scaly;
		  uminy *= trim->scaly;

		  if(!has_endb)
		    {
		      ay_trafo_copy(trim, cap);
		    }

		  *nextcb = cap;
		  nextcb = &(cap->next);
		  upper_cap = cap;

		  ay_nct_getorientation((ay_nurbcurve_object *)
					curve, &angle);

		  if(angle < 0.0)
		    ay_nct_revert(trim->refine);

		  trim->scalx /= fabs(umaxx-uminx);
		  trim->scaly /= fabs(umaxy-uminy);
		  trim->movx = -(uminx + (fabs(umaxx-uminx)/2.0))*trim->scalx;
		  trim->movy = -(uminy + (fabs(umaxy-uminy)/2.0))*trim->scaly;

		  trim->movx += 0.5;
		  trim->movy += 0.5;

		}
	      else
		{ /* not first curve  */

		  if(has_endb)
		    {
		      patch = (ay_nurbpatch_object*)endb->refine;
		      free(curve->controlv);
		      curve->controlv = NULL;
		      if(!(curve->controlv =
			   calloc(4*curve->length, sizeof(double))))
			return AY_EOMEM;
		      memcpy(curve->controlv,&(patch->controlv[
					(patch->width-1)*4*patch->height]),
			     4*curve->length*sizeof(double));
		      ay_object_defaults(trim);
		    }

		  ay_nct_getorientation((ay_nurbcurve_object *)
					trim->refine, &angle);

		  if(angle > 0.0)
		    ay_nct_revert(trim->refine);

		  if(!has_endb)
		    {
		      trimmx = trim->movx-firstmovex;
		      trimmy = trim->movy-firstmovey;
		    }
		  else
		    {
		      trimmx = 0.0;
		      trimmy = 0.0;
		    }

		  trim->scalx /= fabs(umaxx-uminx);
		  trim->scaly /= fabs(umaxy-uminy);

		  trim->movx = -(uminx + (fabs(umaxx-uminx)/2.0))*trim->scalx;
		  trim->movy = -(uminy + (fabs(umaxy-uminy)/2.0))*trim->scaly;

		  trim->movx += 0.5+(trimmx/fabs(umaxx-uminx));
		  trim->movy += 0.5+(trimmy/fabs(umaxy-uminy));

		} /* if */

	      ot = upper_cap->down;
	      upper_cap->down = trim;
	      trim->next = ot;

	    } /* if */

	  if(ext->has_lower_cap)
	    {
	      ay_status = ay_object_copy(c, &trim);
	      if(ay_status)
		return ay_status;

	      /* work around error with rotated curves */
	      ay_nct_applytrafo(trim);

	      curve = (ay_nurbcurve_object *)trim->refine;

	      /* first c? -> trimcurve must be cw */
	      if(!lower_cap)
		{
		  if(!ext->has_upper_cap)
		    {
		      firstmovex = trim->movx;
		      firstmovey = trim->movy;
		    }

		  z = 0.0;
		  /*
		     if we have bevels, we need a new control vector
		     copied directly from the beveling patch,
		     this is the same vector as the vector from the
		     curve, but displaced by bevel_radius...
		   */
		  if(has_startb)
		    {
		      patch = (ay_nurbpatch_object*)startb->refine;
		      free(curve->controlv);
		      curve->controlv = NULL;
		      if(!(curve->controlv =
			   calloc(4*curve->length, sizeof(double))))
			return AY_EOMEM;
		      memcpy(curve->controlv,&(patch->controlv[
					(patch->width-1)*4*patch->height]),
			     4*curve->length*sizeof(double));
		      z += curve->controlv[2];
		      ay_trafo_defaults(trim);
		    }

		  ay_status = ay_npt_createnpatchobject(&cap);

		  ay_npt_createcap(z,curve,&lminx,&lmaxx,&lminy,&lmaxy,&angle,
				  (ay_nurbpatch_object **)&(cap->refine));

		  lmaxx *= trim->scalx;
		  lminx *= trim->scalx;
		  lmaxy *= trim->scaly;
		  lminy *= trim->scaly;

		  if(!has_startb)
		    {
		      ay_trafo_copy(trim, cap);
		    }

		  cap->scalz = -1.0;

		  *nextcb = cap;
		  nextcb = &(cap->next);
		  lower_cap = cap;

		  ay_nct_getorientation((ay_nurbcurve_object *)
					  curve, &angle);

		  if(angle < 0.0)
		    ay_nct_revert(trim->refine);

		  trim->scalx /= fabs(lmaxx-lminx);
		  trim->scaly /= fabs(lmaxy-lminy);

		  trim->movx = -(lminx + (fabs(lmaxx-lminx)/2.0))*trim->scalx;
		  trim->movy = -(lminy + (fabs(lmaxy-lminy)/2.0))*trim->scaly;

		  trim->movx += 0.5;
		  trim->movy += 0.5;
		}
	      else
		{ /* not first curve  */
		  if(has_startb)
		    {
		      patch = (ay_nurbpatch_object*)startb->refine;
		      free(curve->controlv);
		      curve->controlv = NULL;
		      if(!(curve->controlv =
			   calloc(4*curve->length, sizeof(double))))
			return AY_EOMEM;
		      memcpy(curve->controlv,&(patch->controlv[
					(patch->width-1)*4*patch->height]),
			     4*curve->length*sizeof(double));
		      ay_object_defaults(trim);
		    }

		  ay_nct_getorientation((ay_nurbcurve_object *)
					trim->refine, &angle);

		  if(angle > 0.0)
		    ay_nct_revert(trim->refine);

		  if(!has_startb)
		    {
		      trimmx = trim->movx-firstmovex;
		      trimmy = trim->movy-firstmovey;
		    }
		  else
		    {
		      trimmx = 0.0;
		      trimmy = 0.0;
		    }

		  trim->scalx /= fabs(lmaxx-lminx);
		  trim->scaly /= fabs(lmaxy-lminy);

		  trim->movx = -(lminx + (fabs(lmaxx-lminx)/2.0))*trim->scalx;
		  trim->movy = -(lminy + (fabs(lmaxy-lminy)/2.0))*trim->scaly;

		  trim->movx += 0.5+(trimmx/fabs(lmaxx-lminx));
		  trim->movy += 0.5+(trimmy/fabs(lmaxy-lminy));

		} /* if */

	      ot = lower_cap->down;
	      lower_cap->down = trim;
	      trim->next = ot;

	    } /* if */

	  if(got_object)
	    ay_object_deletemulti(c);

	} /* if c */

    } /* while down */

  /* properly terminate all levels */
  if(ext)
    {
      bevel = ext->caps_and_bevels;
      while(bevel)
	{  
	  tloop = bevel->down;
	  if(tloop)
	    {
	      while(tloop->next)
		{
		  tloop = tloop->next;
		}
	      ay_status = ay_object_crtendlevel(&(tloop->next));
	    }
	  else
	    {
	      ay_status = ay_object_crtendlevel(&(bevel->down));
	    }

	  ((ay_nurbpatch_object *)bevel->refine)->
	    glu_display_mode = display_mode;
	  ((ay_nurbpatch_object *)bevel->refine)->
	    glu_sampling_tolerance = tolerance;
	  /*
	    ay_status = ay_npt_revertu((ay_nurbpatch_object*)(bevel->refine));
	  */
	  bevel = bevel->next;
	} /* while */
    } /* if */

 return ay_status;
} /* ay_extrude_notifycb */


int
ay_extrude_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "extrude_providecb";
 ay_extrude_object *e = NULL;
 ay_object *new = NULL, **t = NULL, *p = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNPATCH)
	return AY_OK;
      else
	return AY_ERROR;
    }

  e = (ay_extrude_object *) o->refine;

  if(type == AY_IDNPATCH)
    {
      t = &(new);

      /* copy extrusion(s) */
      p = e->npatch;
      while(p)
	{
	  ay_status = ay_object_copy(p, t);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, NULL);
	      return AY_ERROR;
	    }
	  ay_trafo_copy(o, *t);
	  t = &((*t)->next);
	  p = p->next;
	} /* while */

      /* copy caps and bevels */
      p = e->caps_and_bevels;
      while(p)
	{
	  ay_status = ay_object_copy(p, t);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, NULL);
	      return AY_ERROR;
	    }
	  ay_trafo_add(o, *t);
	  t = &((*t)->next);
	  p = p->next;
	} /* while */
#if 0
      /* copy bevels */
      p = e->upper_bevels;
      while(p)
	{
	  ay_status = ay_object_copy(p, t);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, NULL);
	      return AY_ERROR;
	    }
	  ay_trafo_creatematrix(*t, m);
	  np = (ay_nurbpatch_object *)(*t)->refine;
	  cv = np->controlv;
	  for(i = 0; i < np->width; i++)
	    {
	      for(j = 0; j < np->height; j++)
		{
		  ay_trafo_apply4(cv, m);
		  cv += stride;
		}
	    }
	  ay_trafo_defaults(*t);
	  ay_trafo_copy(o, *t);
	  t = &((*t)->next);
	  p = p->next;
	} /* while */

      p = e->lower_bevels;
      while(p)
	{
	  ay_status = ay_object_copy(p, t);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, NULL);
	      return AY_ERROR;
	    }
	  ay_trafo_add(o, *t);
	  t = &((*t)->next);
	  p = p->next;
	} /* while */
#endif

      /* copy eventually present TP tags */
      ay_npt_copytptag(o, new);

      *result = new;
    } /* if */

 return ay_status;
} /* ay_extrude_providecb */


int
ay_extrude_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_extrude_object *r = NULL;
 ay_object *new = NULL, *p = NULL, **next = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */

  r = (ay_extrude_object *) o->refine;

  if(r->caps_and_bevels || (r->npatch && r->npatch->next))
    {
      if(!(new = calloc(1, sizeof(ay_object))))
	{ return AY_EOMEM; }

      ay_object_defaults(new);
      new->type = AY_IDLEVEL;
      new->parent = AY_TRUE;
      new->inherit_trafos = AY_TRUE;
      ay_trafo_copy(o, new);

      if(!(new->refine = calloc(1, sizeof(ay_level_object))))
	{ free(new); return AY_EOMEM; }

      ((ay_level_object *)(new->refine))->type = AY_LTLEVEL;

      next = &(new->down);

      if(r->npatch)
	{
	  ay_status = ay_object_copy(r->npatch, next);

	  if(*next)
	    next = &((*next)->next);

	  p = r->npatch->next;
	  while(p)
	    {
	      ay_status = ay_object_copy(p, next);
	      if(*next)
		{
		  ay_trafo_copy(o, *next);
		  (*next)->parent = AY_TRUE;
		  ay_object_crtendlevel(&(*next)->down);
		  next = &((*next)->next);
		}
	      p = p->next;
	    } /* while */
	} /* if */

      if(r->caps_and_bevels)
	{
	  p = r->caps_and_bevels;
	  while(p)
	    {
	      ay_status = ay_object_copy(p, next);
	      if(*next)
		next = &((*next)->next);

	      p = p->next;
	    } /* while */
	} /* if */

      /* copy eventually present TP tags */
      ay_npt_copytptag(o, new->down);

      ay_object_crtendlevel(next);
    }
  else
    {
       if(r->npatch)
	{
	  ay_status = ay_object_copy(r->npatch, &new);
	  ay_trafo_copy(o, new);
	  new->hide_children = AY_TRUE;
	  new->parent = AY_TRUE;
	  ay_object_crtendlevel(&(new->down));

	  /* copy eventually present TP tags */
	  ay_npt_copytptag(o, new);
	} /* if */
    } /* if */

  /* second, link new objects, or replace old objects with them */

  if(new)
    {
      if(!in_place)
	{
	  ay_status = ay_object_link(new);
	}
      else
	{
	  ay_object_replace(new, o);
	} /* if */
    } /* if */

 return ay_status;
} /* ay_extrude_convertcb */


int
ay_extrude_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_extrude_name,
				    ay_extrude_createcb,
				    ay_extrude_deletecb,
				    ay_extrude_copycb,
				    ay_extrude_drawcb,
				    NULL, /* no handles */
				    ay_extrude_shadecb,
				    ay_extrude_setpropcb,
				    ay_extrude_getpropcb,
				    ay_extrude_getpntcb,
				    ay_extrude_readcb,
				    ay_extrude_writecb,
				    ay_extrude_wribcb,
				    ay_extrude_bbccb,
				    AY_IDEXTRUDE);


  ay_status = ay_notify_register(ay_extrude_notifycb, AY_IDEXTRUDE);

  ay_status = ay_convert_register(ay_extrude_convertcb, AY_IDEXTRUDE);

  ay_status = ay_provide_register(ay_extrude_providecb, AY_IDEXTRUDE);

 return ay_status;
} /* ay_extrude_init */

