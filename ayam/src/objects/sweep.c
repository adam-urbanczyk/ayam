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

  if(sweep->start_cap)
    ay_object_delete(sweep->start_cap);

  if(sweep->end_cap)
    ay_object_delete(sweep->end_cap);

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

  /* copy start cap */
  ay_object_copy(sweepsrc->start_cap, &(sweep->start_cap));

  /* copy end cap */
  ay_object_copy(sweepsrc->end_cap, &(sweep->end_cap));


  *dst = (void *)sweep;

 return AY_OK;
} /* ay_sweep_copycb */


int
ay_sweep_drawcb(struct Togl *togl, ay_object *o)
{
 ay_sweep_object *sweep = NULL;

  if(!o)
    return AY_ENULL;

  sweep = (ay_sweep_object *)o->refine;

  if(!sweep)
    return AY_ENULL;

  if(sweep->npatch)
    ay_draw_object(togl, sweep->npatch, AY_TRUE);

  if(sweep->start_cap)
    ay_draw_object(togl, sweep->start_cap, AY_TRUE);

  if(sweep->end_cap)
    ay_draw_object(togl, sweep->end_cap, AY_TRUE);

 return AY_OK;
} /* ay_sweep_drawcb */


int
ay_sweep_shadecb(struct Togl *togl, ay_object *o)
{
 ay_sweep_object *sweep = NULL;

  if(!o)
    return AY_ENULL;

  sweep = (ay_sweep_object *)o->refine;

  if(!sweep)
    return AY_ENULL;

  if(sweep->npatch)
    ay_shade_object(togl, sweep->npatch, AY_FALSE);

  if(sweep->start_cap)
    ay_shade_object(togl, sweep->start_cap, AY_FALSE);

  if(sweep->end_cap)
    ay_shade_object(togl, sweep->end_cap, AY_FALSE);

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

  Tcl_SetStringObj(ton,"Close",-1);
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

  Tcl_SetStringObj(ton,"Close",-1);
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
 ay_sweep_object *sweep = NULL;

  if(!o)
   return AY_ENULL;

  sweep = (ay_sweep_object*)o->refine;

  if(sweep->npatch)
    ay_wrib_object(file, sweep->npatch);

  if(sweep->start_cap)
    ay_wrib_object(file, sweep->start_cap);

  if(sweep->end_cap)
    ay_wrib_object(file, sweep->end_cap);

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
 ay_object *npatch = NULL;
 ay_nurbpatch_object *np = NULL;
 int ay_status = AY_OK;
 int got_c1 = AY_FALSE, got_c2 = AY_FALSE, got_c3 = AY_FALSE, mode = 0;
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

  if(sweep->start_cap)
    ay_object_delete(sweep->start_cap);
  sweep->start_cap = NULL;

  if(sweep->end_cap)
    ay_object_delete(sweep->end_cap);
  sweep->end_cap = NULL;

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
	}

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
	}

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

  /* sweep */
  if(!(npatch = calloc(1, sizeof(ay_object))))
    {
      return AY_ERROR;
    }

  ay_object_defaults(npatch);
  npatch->type = AY_IDNPATCH;

  ay_status = ay_npt_sweep(curve1, curve2, curve3,
			   sweep->sections, sweep->rotate, sweep->close,
			   (ay_nurbpatch_object **)(&(npatch->refine)),
			   sweep->has_start_cap, &(sweep->start_cap),
			   sweep->has_end_cap, &(sweep->end_cap));

  if(ay_status)
    return ay_status;

  /* copy sampling tolerance/mode over to new objects */
  sweep->npatch = npatch;

  ((ay_nurbpatch_object *)npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)npatch->refine)->glu_display_mode =
    mode;

  if(sweep->start_cap)
    {
      ((ay_nurbpatch_object *)
       (sweep->start_cap->refine))->glu_sampling_tolerance = tolerance;
      ((ay_nurbpatch_object *)
       (sweep->start_cap->refine))->glu_display_mode = mode;
    }

  if(sweep->end_cap)
    {
      ((ay_nurbpatch_object *)
       (sweep->end_cap->refine))->glu_sampling_tolerance = tolerance;
      ((ay_nurbpatch_object *)
       (sweep->end_cap->refine))->glu_display_mode = mode;
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

 return AY_OK;
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

      /* copy caps */
      p = s->start_cap;
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

      p = s->end_cap;
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
 ay_object *new = NULL, **next = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */

  r = (ay_sweep_object *) o->refine;

  if((r->start_cap) || (r->end_cap))
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
	      next = &((*next)->next);
	    }
	}

      if(r->start_cap)
	{
	  ay_status = ay_object_copy(r->start_cap, next);
	  if(*next)
	    {
	      next = &((*next)->next);
	    }
	}

      if(r->end_cap)
	{
	  ay_status = ay_object_copy(r->end_cap, next);
	  if(*next)
	    {
	      next = &((*next)->next);
	    }
	}

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

