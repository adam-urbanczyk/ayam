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

/* extrude.c - extrude object */

static char *ay_extrude_name = "Extrude";

int ay_extrude_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_extrude_createcb:
 *  create callback function of extrude object
 */
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


/* ay_extrude_deletecb:
 *  delete callback function of extrude object
 */
int
ay_extrude_deletecb(void *c)
{
 ay_extrude_object *extrude = NULL;

  if(!c)
    return AY_ENULL;

  extrude = (ay_extrude_object *)(c);

  if(extrude->npatch)
    (void)ay_object_deletemulti(extrude->npatch, AY_FALSE);

  if(extrude->caps_and_bevels)
    (void)ay_object_deletemulti(extrude->caps_and_bevels, AY_FALSE);

  free(extrude);

 return AY_OK;
} /* ay_extrude_deletecb */


/* ay_extrude_copycb:
 *  copy callback function of extrude object
 */
int
ay_extrude_copycb(void *src, void **dst)
{
 ay_extrude_object *extrude = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(extrude = malloc(sizeof(ay_extrude_object))))
    return AY_EOMEM;

  memcpy(extrude, src, sizeof(ay_extrude_object));

  extrude->npatch = NULL;
  extrude->caps_and_bevels = NULL;

  *dst = (void *)extrude;

 return AY_OK;
} /* ay_extrude_copycb */


/* ay_extrude_drawcb:
 *  draw (display in an Ayam view window) callback function of extrude object
 */
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


/* ay_extrude_shadecb:
 *  shade (display in an Ayam view window) callback function of extrude object
 */
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


/* ay_extrude_drawhcb:
 *  draw handles (in an Ayam view window) callback function of extrude object
 */
int
ay_extrude_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_extrude_object *extrude;

  if(!o)
    return AY_ENULL;

  extrude = (ay_extrude_object *) o->refine;

  if(!extrude)
    return AY_ENULL;

  if(extrude->npatch)
    {
      ay_npt_drawrohandles((ay_nurbpatch_object *)extrude->npatch);
    }

 return AY_OK;
} /* ay_extrude_drawhcb */


/* ay_extrude_getpntcb:
 *  get point (editing and selection) callback function of extrude object
 */
int
ay_extrude_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbpatch_object *patch = NULL;
 ay_extrude_object *extrude = NULL;

  if(!o)
    return AY_ENULL;

  extrude = (ay_extrude_object *)o->refine;

  if(!extrude)
    return AY_ENULL;

  if(extrude->npatch)
    {
      patch = (ay_nurbpatch_object *)extrude->npatch->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, patch->width*patch->height, 4,
			     patch->controlv);
    }

 return AY_ERROR;
} /* ay_extrude_getpntcb */


/* ay_extrude_setpropcb:
 *  set property (from Tcl to C context) callback function of extrude object
 */
int
ay_extrude_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "ExtrudeAttrData";
 /* char fname[] = "extrude_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_extrude_object *extrude = NULL;

  if(!interp || !o)
    return AY_ENULL;

  extrude = (ay_extrude_object *)o->refine;

  if(!extrude)
    return AY_ENULL;

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
  Tcl_GetIntFromObj(interp,to, &(extrude->display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(extrude->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_notify_object(o);

  o->modified = AY_TRUE;
  ay_notify_parent();

 return AY_OK;
} /* ay_extrude_setpropcb */


/* ay_extrude_getpropcb:
 *  get property (from C to Tcl context) callback function of extrude object
 */
int
ay_extrude_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="ExtrudeAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_extrude_object *extrude = NULL;

  if(!interp || !o)
    return AY_ENULL;

  extrude = (ay_extrude_object *)(o->refine);

  if(!extrude)
    return AY_ENULL;

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
  to = Tcl_NewIntObj(extrude->display_mode);
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


/* ay_extrude_readcb:
 *  read (from scene file) callback function of extrude object
 */
int
ay_extrude_readcb(FILE *fileptr, ay_object *o)
{
 ay_extrude_object *extrude = NULL;
 ay_tag tag = {0}, *stag = NULL, *etag = NULL;
 char vbuf[128], nbuf[3] = "BP", *val;
 int has_startb = AY_FALSE, has_startb2 = AY_FALSE;
 int has_endb = AY_FALSE, has_endb2 = AY_FALSE;
 int startb_type, startb_type2, endb_type, startb_sense, endb_sense;
 double startb_radius, startb_radius2, endb_radius;

  if(!fileptr || !o)
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
  fscanf(fileptr, "%d\n", &extrude->display_mode);
  fscanf(fileptr, "%lg\n", &extrude->glu_sampling_tolerance);

  /* get bevel parameters from potentially present BP tags */
  if(ay_read_version < 16)
    {
      /* before Ayam 1.21 */
      stag = o->tags;
      while(stag)
	{
	  if(stag->type == ay_bp_tagtype && stag->val)
	    {
	      val = (char*)stag->val;
	      if(*val == '0')
		*val = '2';
	      else
		if(*val == '1')
		  *val = '3';
	    } /* if */
	  stag = stag->next;
	} /* while */
    } /* if */

  ay_npt_getbeveltags(o, 2, &has_startb, &startb_type, &startb_radius,
		      &startb_sense);
  ay_npt_getbeveltags(o, 3, &has_endb, &endb_type, &endb_radius,
		      &endb_sense);

  tag.name = nbuf;
  tag.type = ay_bp_tagtype;
  tag.val = vbuf;

  /* need to create BP tag(s) from object parameters? */
  if(!has_startb && has_startb2)
    {
      sprintf(vbuf, "2,%d,%g,0", startb_type2, startb_radius2);
      ay_tags_copy(&tag, &stag);
      ay_tags_append(o, stag);
    }

  if(!has_endb && has_endb2)
    {
      sprintf(vbuf, "3,%d,%g,0", startb_type2, startb_radius2);
      ay_tags_copy(&tag, &etag);
      ay_tags_append(o, etag);
    }

  o->refine = extrude;

 return AY_OK;
} /* ay_extrude_readcb */


/* ay_extrude_writecb:
 *  write (to scene file) callback function of extrude object
 */
int
ay_extrude_writecb(FILE *fileptr, ay_object *o)
{
 ay_extrude_object *extrude = NULL;
 int has_startb = AY_FALSE, has_endb = AY_FALSE;
 int startb_type = 0, endb_type = 0, startb_sense, endb_sense;
 double startb_radius = 1.0, endb_radius = 1.0;

  if(!fileptr || !o)
    return AY_ENULL;

  extrude = (ay_extrude_object *)(o->refine);

  if(!extrude)
    return AY_ENULL;

  /* get bevel parameters */
  ay_npt_getbeveltags(o, 2, &has_startb, &startb_type, &startb_radius,
		      &startb_sense);
  ay_npt_getbeveltags(o, 3, &has_endb, &endb_type, &endb_radius,
		      &endb_sense);

  fprintf(fileptr, "%g\n", extrude->height);
  fprintf(fileptr, "%d\n", extrude->has_upper_cap);
  fprintf(fileptr, "%d\n", extrude->has_lower_cap);
  fprintf(fileptr, "%d\n", has_endb);
  fprintf(fileptr, "%d\n", has_startb);
  fprintf(fileptr, "%d\n", startb_type);
  fprintf(fileptr, "%g\n", startb_radius);
  fprintf(fileptr, "%d\n", extrude->display_mode);
  fprintf(fileptr, "%g\n", extrude->glu_sampling_tolerance);

 return AY_OK;
} /* ay_extrude_writecb */


/* ay_extrude_wribcb:
 *  RIB export callback function of extrude object
 */
int
ay_extrude_wribcb(char *file, ay_object *o)
{
 ay_extrude_object *extrude = NULL;
 ay_object *p = NULL;

  if(!o)
   return AY_ENULL;

  extrude = (ay_extrude_object*)o->refine;

  if(!extrude)
    return AY_ENULL;

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


/* ay_extrude_bbccb:
 *  bounding box calculation callback function of extrude object
 */
int
ay_extrude_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_extrude_object *extrude = NULL;
 ay_object *np = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  extrude = (ay_extrude_object *)o->refine;

  if(!extrude)
    return AY_ENULL;

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


/* ay_extrude_notifycb:
 *  notification callback function of extrude object
 */
int
ay_extrude_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_extrude_object *ext = NULL;
 ay_object *down = NULL, *c = NULL, *cap = NULL, *trim = NULL;
 ay_object *bevel = NULL, *startb = NULL, *endb = NULL, *tloop = NULL;
 ay_object *extrusion = NULL, *upper_cap = NULL, *lower_cap = NULL;
 ay_object **nexttrimu, **nexttriml, **nextcb;
 ay_nurbcurve_object *curve = NULL;
 ay_nurbpatch_object *patch = NULL;
 ay_bparam bparams = {0};
 ay_cparam cparams = {0};
 int bstate, display_mode = 0, is_provided = AY_FALSE;
 int has_startb = AY_FALSE, has_endb = AY_FALSE;
 double uminx, umaxx, uminy, umaxy;
 double lminx, lmaxx, lminy, lmaxy;
 double firstmovex = 0.0, firstmovey = 0.0;
 double trimmx = 0.0, trimmy = 0.0;
 double angle = 0.0, z = 0.0;
 double tolerance = 0.0;

  if(!o)
    return AY_OK;

  ext = (ay_extrude_object*)o->refine;

  if(!ext)
    return AY_ENULL;

  /* ok, something changed below, we have to rebuild the extrusion(s) */

  tolerance = ext->glu_sampling_tolerance;
  display_mode = ext->display_mode;

  /* clear old extrusions, caps and bevels */
  if(ext->npatch)
    {
      ay_object_deletemulti(ext->npatch, AY_FALSE);
    }
  ext->npatch = NULL;

  if(ext->caps_and_bevels)
    {
      ay_object_deletemulti(ext->caps_and_bevels, AY_FALSE);
    }
  ext->caps_and_bevels = NULL;

  nextcb = &(ext->caps_and_bevels);

  /* get bevel parameters from potentially present BP tags */
  if(o->tags)
    {
      ay_bevelt_parsetags(o->tags, &bparams);
      if(bparams.states[2])
	has_startb = AY_TRUE;
      if(bparams.states[3])
	has_endb = AY_TRUE;
      /*ay_capt_parsetags(o->tags, &cparams);*/
    }

  /* create new extrusions, caps and bevels */
  down = o->down;
  while(down)
    {
      is_provided = AY_FALSE;
      c = NULL;
      if(down->type != AY_IDNCURVE)
	{
	  ay_status = ay_provide_object(down, AY_IDNCURVE, &c);
	  if(c)
	    is_provided = AY_TRUE;
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
	  /* create and link the extrusion for curve c */
	  extrusion = ext->npatch;

	  ay_status = ay_npt_createnpatchobject(&(ext->npatch));

	  ay_npt_extrude(ext->height, c, (ay_nurbpatch_object **)(void*)
			 &(ext->npatch->refine));

	  ((ay_nurbpatch_object *)ext->npatch->refine)->
	    display_mode = display_mode;
	  ((ay_nurbpatch_object *)ext->npatch->refine)->
	    glu_sampling_tolerance = tolerance;

	  ext->npatch->next = extrusion;

	  /* create and link upper bevel */
	  if(bparams.states[3])
	    {
	      bstate = bparams.states[2];
	      bparams.states[2] = 0;
	      if(extrusion)
		bparams.dirs[3] = !bparams.dirs[3];
	      ay_bevelt_addbevels(&bparams, &cparams, ext->npatch, nextcb);
	      endb = *nextcb;
	      nextcb = &(endb->next);
	      bparams.states[2] = bstate;
	      if(extrusion)
		bparams.dirs[3] = !bparams.dirs[3];
	    } /* if */

	  /* create and link upper cap */
	  if(ext->has_upper_cap)
	    {
	      ay_status = ay_object_copy(c, &trim);
	      if(ay_status)
		goto cleanup;

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
			{
			  ay_status = AY_EOMEM;
			  goto cleanup;
			}
		      memcpy(curve->controlv,&(patch->controlv[
				(patch->width-1)*4*patch->height]),
			     4*curve->length*sizeof(double));
		      z = curve->controlv[2];
		      ay_trafo_copy(endb, trim);
		      ay_nct_applytrafo(trim);
		    } /* if */

		  ay_status = ay_npt_createnpatchobject(&cap);

		  ay_npt_createcap(z,curve,&uminx,&umaxx,&uminy,&umaxy,
			       (ay_nurbpatch_object **)(void*)&(cap->refine));

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

		  ay_status = ay_nct_getorientation((ay_nurbcurve_object *)
						    curve, 4, 1, 0, &angle);

		  if(angle < 0.0)
		    ay_nct_revert(trim->refine);

		  trim->scalx /= fabs(umaxx-uminx);
		  trim->scaly /= fabs(umaxy-uminy);
		  trim->movx = -(uminx + (fabs(umaxx-uminx)/2.0))*trim->scalx;
		  trim->movy = -(uminy + (fabs(umaxy-uminy)/2.0))*trim->scaly;

		  trim->movx += 0.5;
		  trim->movy += 0.5;
		  upper_cap->down = trim;
		  nexttrimu = &(trim->next);
		}
	      else
		{
		  /* not first curve  */
		  if(has_endb)
		    {
		      patch = (ay_nurbpatch_object*)endb->refine;
		      free(curve->controlv);
		      curve->controlv = NULL;
		      if(!(curve->controlv =
			   calloc(4*curve->length, sizeof(double))))
			{
			  ay_status = AY_EOMEM;
			  goto cleanup;
			}

		      memcpy(curve->controlv,&(patch->controlv[
					(patch->width-1)*4*patch->height]),
			     4*curve->length*sizeof(double));
		      ay_trafo_copy(endb, trim);
		      ay_nct_applytrafo(trim);
		    } /* if */

		  ay_status = ay_nct_getorientation((ay_nurbcurve_object *)
					       trim->refine, 4, 1, 0, &angle);

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

		  trim->next = *nexttrimu;
		  *nexttrimu = trim;
		  nexttrimu = &(trim->next);
		} /* if */
	    } /* if */

	  /* create and link lower bevel */
	  if(bparams.states[2])
	    {
	      bstate = bparams.states[3];
	      bparams.states[3] = 0;
	      if(extrusion)
		bparams.dirs[2] = !bparams.dirs[2];
	      ay_bevelt_addbevels(&bparams, &cparams, ext->npatch, nextcb);
	      startb = *nextcb;
	      nextcb = &(startb->next);
	      bparams.states[3] = bstate;
	      if(extrusion)
		bparams.dirs[2] = !bparams.dirs[2];
	    } /* if */

	  /* create and link lower cap */
	  if(ext->has_lower_cap)
	    {
	      ay_status = ay_object_copy(c, &trim);
	      if(ay_status)
		goto cleanup;

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
			{
			  ay_status = AY_EOMEM;
			  goto cleanup;
			}
		      memcpy(curve->controlv,&(patch->controlv[
					(patch->width-1)*4*patch->height]),
			     4*curve->length*sizeof(double));
		      z = -curve->controlv[2];
		      ay_trafo_copy(startb, trim);
		      ay_nct_applytrafo(trim);
		    } /* if */

		  ay_status = ay_npt_createnpatchobject(&cap);

		  ay_npt_createcap(z,curve,&lminx,&lmaxx,&lminy,&lmaxy,
			       (ay_nurbpatch_object **)(void*)&(cap->refine));

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

		  ay_status = ay_nct_getorientation((ay_nurbcurve_object *)
						   curve, 4, 1, 0, &angle);

		  if(angle < 0.0)
		    ay_nct_revert(trim->refine);

		  trim->scalx /= fabs(lmaxx-lminx);
		  trim->scaly /= fabs(lmaxy-lminy);

		  trim->movx = -(lminx + (fabs(lmaxx-lminx)/2.0))*trim->scalx;
		  trim->movy = -(lminy + (fabs(lmaxy-lminy)/2.0))*trim->scaly;

		  trim->movx += 0.5;
		  trim->movy += 0.5;

		  lower_cap->down = trim;
		  nexttriml = &(trim->next);
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
			{
			  ay_status = AY_EOMEM;
			  goto cleanup;
			}
		      memcpy(curve->controlv,&(patch->controlv[
					(patch->width-1)*4*patch->height]),
			     4*curve->length*sizeof(double));
		      ay_trafo_copy(startb, trim);
		      ay_nct_applytrafo(trim);
		    } /* if */

		  ay_status = ay_nct_getorientation((ay_nurbcurve_object *)
					       trim->refine, 4, 1, 0, &angle);

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

		  trim->next = *nexttriml;
		  *nexttriml = trim;
		  nexttriml = &(trim->next);
		} /* if */
	    } /* if */

	  if(is_provided)
	    {
	      ay_object_deletemulti(c, AY_FALSE);
	    }

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
	      tloop->next = ay_endlevel;
	    }
	  else
	    {
	      bevel->down = ay_endlevel;
	    }

	  ((ay_nurbpatch_object *)bevel->refine)->
	    display_mode = display_mode;
	  ((ay_nurbpatch_object *)bevel->refine)->
	    glu_sampling_tolerance = tolerance;

	  bevel = bevel->next;
	} /* while */
    } /* if */

cleanup:
  /* recover selected points */
  if(o->selp)
    {
      if(ext->npatch)
	ay_extrude_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

 return ay_status;
} /* ay_extrude_notifycb */


/* ay_extrude_providecb:
 *  provide callback function of extrude object
 */
int
ay_extrude_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 ay_extrude_object *e = NULL;

  if(!o)
    return AY_ENULL;

  e = (ay_extrude_object *) o->refine;

  if(!e)
    return AY_ENULL;

 return ay_provide_nptoolobj(o, type, e->npatch, e->caps_and_bevels, result);
} /* ay_extrude_providecb */


/* ay_extrude_convertcb:
 *  convert callback function of extrude object
 */
int
ay_extrude_convertcb(ay_object *o, int in_place)
{
 ay_extrude_object *e = NULL;

  if(!o)
    return AY_ENULL;

  e = (ay_extrude_object *) o->refine;

  if(!e)
    return AY_ENULL;

 return ay_convert_nptoolobj(o, e->npatch, e->caps_and_bevels, in_place);
} /* ay_extrude_convertcb */


/* ay_extrude_init:
 *  initialize the extrude object module
 */
int
ay_extrude_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_extrude_name,
				    ay_extrude_createcb,
				    ay_extrude_deletecb,
				    ay_extrude_copycb,
				    ay_extrude_drawcb,
				    ay_extrude_drawhcb,
				    ay_extrude_shadecb,
				    ay_extrude_setpropcb,
				    ay_extrude_getpropcb,
				    ay_extrude_getpntcb,
				    ay_extrude_readcb,
				    ay_extrude_writecb,
				    ay_extrude_wribcb,
				    ay_extrude_bbccb,
				    AY_IDEXTRUDE);


  ay_status += ay_notify_register(ay_extrude_notifycb, AY_IDEXTRUDE);

  ay_status += ay_convert_register(ay_extrude_convertcb, AY_IDEXTRUDE);

  ay_status += ay_provide_register(ay_extrude_providecb, AY_IDEXTRUDE);

 return ay_status;
} /* ay_extrude_init */
