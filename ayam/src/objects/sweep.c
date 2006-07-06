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

/* sweep.c - sweep object */

static char *ay_sweep_name = "Sweep";

int
ay_sweep_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtsweep";
 ay_sweep_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_sweep_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->rotate = AY_TRUE;
  new->sections = 5;

  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_sweep_createcb */


int
ay_sweep_deletecb(void *c)
{
 ay_sweep_object *sweep = NULL;

  if(!c)
    return AY_ENULL;    

  sweep = (ay_sweep_object *)(c);

  if(sweep->npatch)
    ay_object_delete(sweep->npatch);

  if(sweep->caps_and_bevels)
    {
      ay_object_deletemulti(sweep->caps_and_bevels);
      sweep->caps_and_bevels = NULL;
    }

  free(sweep);

 return AY_OK;
} /* ay_sweep_deletecb */


int
ay_sweep_copycb(void *src, void **dst)
{
 ay_sweep_object *sweep = NULL, *sweepsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  sweepsrc = (ay_sweep_object *)src;

  if(!(sweep = calloc(1, sizeof(ay_sweep_object))))
    return AY_EOMEM; 

  memcpy(sweep, src, sizeof(ay_sweep_object)); 


  /* copy npatch */
  ay_object_copy(sweepsrc->npatch, &(sweep->npatch));

  sweep->caps_and_bevels = NULL;

  *dst = (void *)sweep;

 return AY_OK;
} /* ay_sweep_copycb */


int
ay_sweep_drawcb(struct Togl *togl, ay_object *o)
{
 ay_sweep_object *sweep = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  sweep = (ay_sweep_object *)o->refine;

  if(!sweep)
    return AY_ENULL;

  if(sweep->npatch)
    ay_draw_object(togl, sweep->npatch, AY_TRUE);

  if(sweep->caps_and_bevels)
    {
      b = sweep->caps_and_bevels;
      while(b)
	{
	  ay_draw_object(togl, b, AY_TRUE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_sweep_drawcb */


int
ay_sweep_shadecb(struct Togl *togl, ay_object *o)
{
 ay_sweep_object *sweep = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  sweep = (ay_sweep_object *)o->refine;

  if(!sweep)
    return AY_ENULL;

  if(sweep->npatch)
    ay_shade_object(togl, sweep->npatch, AY_FALSE);

  if(sweep->caps_and_bevels)
    {
      b = sweep->caps_and_bevels;
      while(b)
	{
	  ay_shade_object(togl, b, AY_FALSE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_sweep_shadecb */


int
ay_sweep_drawhcb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_sweep_drawhcb */


int
ay_sweep_getpntcb(int mode, ay_object *o, double *p)
{

 return AY_OK;
} /* ay_sweep_getpntcb */


/* Tcl -> C! */
int
ay_sweep_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "SweepAttrData";
 /* char fname[] = "sweep_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_sweep_object *sweep = NULL;

  if(!o)
    return AY_ENULL;

  sweep = (ay_sweep_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Rotate",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(sweep->rotate));

  Tcl_SetStringObj(ton,"Interpolate",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(sweep->interpolate));

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(sweep->close));

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(sweep->sections));

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(sweep->has_start_cap));

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(sweep->has_end_cap));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(sweep->glu_display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(sweep->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_force(o);

  o->modified = AY_TRUE;
  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_sweep_setpropcb */


/* C -> Tcl! */
int
ay_sweep_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="SweepAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_sweep_object *sweep = NULL;

  if(!o)
    return AY_ENULL;

  sweep = (ay_sweep_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Rotate",-1);
  to = Tcl_NewIntObj(sweep->rotate);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Interpolate",-1);
  to = Tcl_NewIntObj(sweep->interpolate);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_NewIntObj(sweep->close);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_NewIntObj(sweep->sections);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_NewIntObj(sweep->has_start_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_NewIntObj(sweep->has_end_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(sweep->glu_display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(sweep->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_sweep_getpropcb */


int
ay_sweep_readcb(FILE *fileptr, ay_object *o)
{
 ay_sweep_object *sweep = NULL;

 if(!o)
   return AY_ENULL;

  if(!(sweep = calloc(1, sizeof(ay_sweep_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&sweep->rotate);
  fscanf(fileptr,"%d\n",&sweep->interpolate);
  fscanf(fileptr,"%d\n",&sweep->sections);
  fscanf(fileptr,"%d\n",&sweep->has_start_cap);
  fscanf(fileptr,"%d\n",&sweep->has_end_cap);
  fscanf(fileptr,"%d\n",&sweep->glu_display_mode);
  fscanf(fileptr,"%lg\n",&sweep->glu_sampling_tolerance);

  if(ay_read_version > 3)
    {
      fscanf(fileptr,"%d\n",&sweep->close);
    }

  o->refine = sweep;

 return AY_OK;
} /* ay_sweep_readcb */


int
ay_sweep_writecb(FILE *fileptr, ay_object *o)
{
 ay_sweep_object *sweep = NULL;

  if(!o)
    return AY_ENULL;

  sweep = (ay_sweep_object *)(o->refine);

  fprintf(fileptr, "%d\n", sweep->rotate);
  fprintf(fileptr, "%d\n", sweep->interpolate);
  fprintf(fileptr, "%d\n", sweep->sections);
  fprintf(fileptr, "%d\n", sweep->has_start_cap);
  fprintf(fileptr, "%d\n", sweep->has_end_cap);
  fprintf(fileptr, "%d\n", sweep->glu_display_mode);
  fprintf(fileptr, "%g\n", sweep->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", sweep->close);

 return AY_OK;
} /* ay_sweep_writecb */


int
ay_sweep_wribcb(char *file, ay_object *o)
{
 ay_sweep_object *sweep;
 ay_object *b;

  if(!o)
   return AY_ENULL;

  sweep = (ay_sweep_object*)o->refine;

  if(sweep->npatch)
    ay_wrib_object(file, sweep->npatch);

  if(sweep->caps_and_bevels)
    {
      b = sweep->caps_and_bevels;
      while(b)
	{
	  ay_wrib_object(file, b);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_sweep_wribcb */


int
ay_sweep_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_sweep_object *sweep = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  sweep = (ay_sweep_object *)o->refine; 

  if(sweep->npatch)
    {
      ay_bbc_get(sweep->npatch, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return AY_OK;
} /* ay_sweep_bbccb */


int
ay_sweep_notifycb(ay_object *o)
{
 ay_sweep_object *sweep = NULL;
 ay_object *curve1 = NULL, *curve2 = NULL, *pobject1 = NULL, *pobject2 = NULL;
 ay_object *curve3 = NULL, *pobject3 = NULL;
 ay_object curve4 = {0}, *curve5;
 ay_object *npatch = NULL, *bevel = NULL, *start_cap = NULL, *end_cap = NULL;
 ay_object **nextcb;
 ay_nurbpatch_object *np = NULL;
 int ay_status = AY_OK;
 int got_c1 = AY_FALSE, got_c2 = AY_FALSE, got_c3 = AY_FALSE, mode = 0;
 int has_startb = AY_FALSE, has_endb = AY_FALSE;
 int startb_type, endb_type, startb_sense, endb_sense;
 double startb_radius, endb_radius;
 double tolerance;

  if(!o)
    return AY_ENULL;    

  sweep = (ay_sweep_object *)(o->refine);

  mode = sweep->glu_display_mode;
  tolerance = sweep->glu_sampling_tolerance;

  /* remove old objects */
  if(sweep->npatch)
    ay_object_delete(sweep->npatch);
  sweep->npatch = NULL;

  if(sweep->caps_and_bevels)
    {
      ay_object_deletemulti(sweep->caps_and_bevels);
      sweep->caps_and_bevels = NULL;
    }

  nextcb = &(sweep->caps_and_bevels);

  /* get curves to sweep */
  if(!o->down)
    return AY_OK;

  curve1 = o->down;
  if(curve1->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(curve1, AY_IDNCURVE, &pobject1);
      if(!pobject1)
	{
	  return AY_OK;
	}
      else
	{
	  curve1 = pobject1;
	  got_c1 = AY_TRUE;
	} /* if */
    } /* if */

  if(!o->down->next)
    return AY_OK;
  curve2 = o->down->next;
  if(curve2->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(curve2, AY_IDNCURVE, &pobject2);
      if(!pobject2)
	{
	  return AY_OK;
	}
      else
	{
	  curve2 = pobject2;
	  got_c2 = AY_TRUE;
	} /* if */
    } /* if */

  if(o->down->next->next)
    {
      curve3 = o->down->next->next;
      if(curve3->type != AY_IDNCURVE)
	{
	  ay_status = ay_provide_object(curve3, AY_IDNCURVE, &pobject3);
	  if(pobject3)
	    {
	      curve3 = pobject3;
	      got_c3 = AY_TRUE;
	    }
	  else
	    {
	      curve3 = NULL;
	    } /* if */
	} /* if */
    } /* if */

  /* get bevel parameters */
  ay_npt_getbeveltags(o, 0, &has_startb, &startb_type, &startb_radius,
		      &startb_sense);
  ay_npt_getbeveltags(o, 1, &has_endb, &endb_type, &endb_radius,
		      &endb_sense);

  /* sweep */
  if(!(npatch = calloc(1, sizeof(ay_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  ay_object_defaults(npatch);
  npatch->type = AY_IDNPATCH;

  if(sweep->close < 2)
    {
      /* open or simple closed sweep */
      ay_status = ay_npt_sweep(curve1, curve2, curve3,
			       sweep->sections, sweep->rotate, sweep->close,
			       (ay_nurbpatch_object **)(&(npatch->refine)),
			       has_startb?AY_FALSE:sweep->has_start_cap,
			       &start_cap,
			       has_endb?AY_FALSE:sweep->has_end_cap,
			       &end_cap);
      if(start_cap)
	{
	  *nextcb = start_cap;
	  nextcb = &(start_cap->next);
	}
      if(end_cap)
	{
	  *nextcb = end_cap;
	  nextcb = &(end_cap->next);
	}
    }
  else
    {
      /* periodic sweep */
      ay_status = ay_npt_closedsweep(curve1, curve2, curve3,
				     sweep->sections, sweep->rotate,
				  (ay_nurbpatch_object **)(&(npatch->refine)));
    }

  if(ay_status)
    goto cleanup;

  sweep->npatch = npatch;

  /* copy sampling tolerance/mode over to new objects */
  ((ay_nurbpatch_object *)npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)npatch->refine)->glu_display_mode =
    mode;

  /* create bevels and caps */
  if(!sweep->close && has_startb)
    {
      ay_object_defaults(&curve4);
      curve4.type = AY_IDNCURVE;
      ay_status = ay_npt_extractnc(sweep->npatch, 2, 0.0, AY_FALSE,
		    (ay_nurbcurve_object**)&(curve4.refine));

      if(ay_status)
	goto cleanup;

      ((ay_nurbcurve_object*)curve4.refine)->type =
	((ay_nurbcurve_object*)curve1->refine)->type;

      if(!startb_sense)
	{
	  ay_nct_revert((ay_nurbcurve_object*)(curve4.refine));
	}

      bevel = NULL;
      if(!(bevel = calloc(1, sizeof(ay_object))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      ay_object_defaults(bevel);
      bevel->type = AY_IDNPATCH;
      bevel->parent = AY_TRUE;
      bevel->inherit_trafos = AY_FALSE;
      ay_status = ay_npt_bevel(startb_type, startb_radius, AY_TRUE, &curve4,
			       (ay_nurbpatch_object**)&(bevel->refine));

      ay_nct_destroy((ay_nurbcurve_object*)curve4.refine);
      curve4.refine = NULL;

      if(ay_status)
	goto cleanup;

      *nextcb = bevel;
      nextcb = &(bevel->next);

      /* create cap */
      if(sweep->has_start_cap)
	{
	  if(!(curve5 = calloc(1, sizeof(ay_object))))
	    {
	      ay_status = AY_EOMEM;
	      goto cleanup;
	    }

	  ay_object_defaults(curve5);
	  curve5->type = AY_IDNCURVE;

	  ay_status = ay_npt_extractnc(bevel, 2, 0.0, AY_FALSE,
				    (ay_nurbcurve_object**)&(curve5->refine));

	  if(ay_status)
	    goto cleanup;

	  ay_status = ay_capt_createfromcurve(curve5, nextcb);
      
	  if(ay_status)
	    goto cleanup;

	  nextcb = &((*nextcb)->next);
	} /* if */

    } /* if */

  if(!sweep->close && has_endb)
    {
      memset(&curve4, 0, sizeof(ay_object));
      ay_object_defaults(&curve4);
      curve4.type = AY_IDNCURVE;
      ay_status = ay_npt_extractnc(sweep->npatch, 3, 0.0, AY_FALSE,
		    (ay_nurbcurve_object**)&(curve4.refine));

      if(ay_status)
	goto cleanup;

      ((ay_nurbcurve_object*)curve4.refine)->type =
	((ay_nurbcurve_object*)curve1->refine)->type;

      if(endb_sense)
	{
	  ay_nct_revert((ay_nurbcurve_object*)(curve4.refine));
	}

      bevel = NULL;
      if(!(bevel = calloc(1, sizeof(ay_object))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      ay_object_defaults(bevel);
      bevel->type = AY_IDNPATCH;
      bevel->parent = AY_TRUE;
      bevel->inherit_trafos = AY_FALSE;
      ay_status = ay_npt_bevel(endb_type, endb_radius, AY_TRUE, &curve4,
			       (ay_nurbpatch_object**)&(bevel->refine));

      ay_nct_destroy((ay_nurbcurve_object*)curve4.refine);
      curve4.refine = NULL;

      if(ay_status)
	goto cleanup;

      *nextcb = bevel;
      nextcb = &(bevel->next);

      /* create cap */
      if(sweep->has_end_cap)
	{
	  curve5 = NULL;
	  if(!(curve5 = calloc(1, sizeof(ay_object))))
	    {
	      ay_status = AY_EOMEM;
	      goto cleanup;
	    }

	  ay_object_defaults(curve5);
	  curve5->type = AY_IDNCURVE;

	  ay_status = ay_npt_extractnc(bevel, 3, 0.0, AY_FALSE,
				    (ay_nurbcurve_object**)&(curve5->refine));

	  if(ay_status)
	    goto cleanup;

	  ay_status = ay_capt_createfromcurve(curve5, nextcb);
      
	  if(ay_status)
	    goto cleanup;

	  nextcb = &((*nextcb)->next);
	} /* if */
    } /* if */

  /* copy sampling tolerance/mode attributes to caps and bevels */
  if(sweep->caps_and_bevels)
    {
      bevel = sweep->caps_and_bevels;
      while(bevel)
	{
	  ((ay_nurbpatch_object *)
	   (bevel->refine))->glu_sampling_tolerance = tolerance;
	  ((ay_nurbpatch_object *)
	   (bevel->refine))->glu_display_mode = mode;
	  bevel = bevel->next;
	}
    }

  /* interpolate the swept surface */
  if(sweep->npatch)
    {
      if(sweep->interpolate)
	{
	  np = (ay_nurbpatch_object *)sweep->npatch->refine;
	  ay_status = ay_npt_interpolateu(np, np->uorder);
	}
    }


  npatch = NULL;

cleanup:
  /* remove provided objects */
  if(got_c1)
    {
      ay_object_delete(pobject1);
    }

  if(got_c2)
    {
      ay_object_delete(pobject2);
    }

  if(got_c3)
    {
      ay_object_delete(pobject3);
    }

  if(npatch)
    {
      ay_object_delete(npatch);
    }

 return ay_status;
} /* ay_sweep_notifycb */


int
ay_sweep_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "sweep_providecb";
 ay_sweep_object *s = NULL;
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

  s = (ay_sweep_object *) o->refine;

  if(type == AY_IDNPATCH)
    {
      t = &(new);

      /* copy sweep */
      ay_status = ay_object_copy(s->npatch, t);
      if(ay_status)
	{
	  ay_error(ay_status, fname, NULL);
	  return AY_ERROR;
	}
      ay_trafo_copy(o, *t);
      t = &((*t)->next);

      /* copy caps and bevels */
      p = s->caps_and_bevels;
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

      *result = new;
    } /* if */

 return ay_status;
} /* ay_sweep_providecb */


int
ay_sweep_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_sweep_object *r = NULL;
 ay_object *new = NULL, **next = NULL, *b;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */
  r = (ay_sweep_object *) o->refine;

  if(!r->npatch)
    return AY_OK;

  if(r->caps_and_bevels)
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
	    {
	      (*next)->parent = AY_TRUE;
	      ay_object_crtendlevel(&(*next)->down);
	      next = &((*next)->next);
	    }
	}

      if(r->caps_and_bevels)
	{
	  b = r->caps_and_bevels;
	  while(b)
	    {
	      ay_status = ay_object_copy(b, next);
	      if(*next)
		{
		  next = &((*next)->next);
		}
	      b = b->next;
	    } /* while */
	} /* if */

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
	}
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
} /* ay_sweep_convertcb */


int
ay_sweep_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_sweep_name,
				    ay_sweep_createcb,
				    ay_sweep_deletecb,
				    ay_sweep_copycb,
				    ay_sweep_drawcb,
				    NULL, /* no handles */
				    ay_sweep_shadecb,
				    ay_sweep_setpropcb,
				    ay_sweep_getpropcb,
				    ay_sweep_getpntcb,
				    ay_sweep_readcb,
				    ay_sweep_writecb,
				    ay_sweep_wribcb,
				    ay_sweep_bbccb,
				    AY_IDSWEEP);

  
  ay_status = ay_notify_register(ay_sweep_notifycb, AY_IDSWEEP);

  ay_status = ay_convert_register(ay_sweep_convertcb, AY_IDSWEEP);

  ay_status = ay_provide_register(ay_sweep_providecb, AY_IDSWEEP);

 return ay_status;
} /* ay_sweep_init */

