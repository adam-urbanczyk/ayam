/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2004 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* birail2.c - birail2 object */

static char *ay_birail2_name = "Birail2";

int
ay_birail2_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtbirail2";
 ay_birail2_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_birail2_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->sections = 5;

  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_birail2_createcb */


int
ay_birail2_deletecb(void *c)
{
 ay_birail2_object *birail2 = NULL;

  if(!c)
    return AY_ENULL;    

  birail2 = (ay_birail2_object *)(c);

  if(birail2->npatch)
    ay_object_delete(birail2->npatch);

  if(birail2->start_cap)
    ay_object_delete(birail2->start_cap);

  if(birail2->end_cap)
    ay_object_delete(birail2->end_cap);

  free(birail2);

 return AY_OK;
} /* ay_birail2_deletecb */


int
ay_birail2_copycb(void *src, void **dst)
{
 ay_birail2_object *birail2 = NULL, *birail2src = NULL;

  if(!src || !dst)
    return AY_ENULL;

  birail2src = (ay_birail2_object *)src;

  if(!(birail2 = calloc(1, sizeof(ay_birail2_object))))
    return AY_EOMEM; 

  memcpy(birail2, src, sizeof(ay_birail2_object)); 


  /* copy npatch */
  ay_object_copy(birail2src->npatch, &(birail2->npatch));

  /* copy start cap */
  ay_object_copy(birail2src->start_cap, &(birail2->start_cap));

  /* copy end cap */
  ay_object_copy(birail2src->end_cap, &(birail2->end_cap));


  *dst = (void *)birail2;

 return AY_OK;
} /* ay_birail2_copycb */


int
ay_birail2_drawcb(struct Togl *togl, ay_object *o)
{
 ay_birail2_object *birail2 = NULL;

  if(!o)
    return AY_ENULL;

  birail2 = (ay_birail2_object *)o->refine;

  if(!birail2)
    return AY_ENULL;

  if(birail2->npatch)
    ay_draw_object(togl, birail2->npatch, AY_TRUE);

  if(birail2->start_cap)
    ay_draw_object(togl, birail2->start_cap, AY_TRUE);

  if(birail2->end_cap)
    ay_draw_object(togl, birail2->end_cap, AY_TRUE);

 return AY_OK;
} /* ay_birail2_drawcb */


int
ay_birail2_shadecb(struct Togl *togl, ay_object *o)
{
 ay_birail2_object *birail2 = NULL;

  if(!o)
    return AY_ENULL;

  birail2 = (ay_birail2_object *)o->refine;

  if(!birail2)
    return AY_ENULL;

  if(birail2->npatch)
    ay_shade_object(togl, birail2->npatch, AY_FALSE);

  if(birail2->start_cap)
    ay_shade_object(togl, birail2->start_cap, AY_FALSE);

  if(birail2->end_cap)
    ay_shade_object(togl, birail2->end_cap, AY_FALSE);

 return AY_OK;
} /* ay_birail2_shadecb */


int
ay_birail2_drawhcb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_birail2_drawhcb */


int
ay_birail2_getpntcb(int mode, ay_object *o, double *p)
{

 return AY_OK;
} /* ay_birail2_getpntcb */


/* Tcl -> C! */
int
ay_birail2_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "Birail2AttrData";
 /* char fname[] = "birail2_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_birail2_object *birail2 = NULL;


  if(!o)
    return AY_ENULL;

  birail2 = (ay_birail2_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Close",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail2->close));

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail2->sections));

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail2->has_start_cap));

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail2->has_end_cap));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail2->glu_display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(birail2->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_force(o);

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_birail2_setpropcb */


/* C -> Tcl! */
int
ay_birail2_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="Birail2AttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_birail2_object *birail2 = NULL;

  if(!o)
    return AY_ENULL;

  birail2 = (ay_birail2_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Close",-1);
  to = Tcl_NewIntObj(birail2->close);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_NewIntObj(birail2->sections);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_NewIntObj(birail2->has_start_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_NewIntObj(birail2->has_end_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(birail2->glu_display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(birail2->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_birail2_getpropcb */


int
ay_birail2_readcb(FILE *fileptr, ay_object *o)
{
 ay_birail2_object *birail2 = NULL;

 if(!o)
   return AY_ENULL;

  if(!(birail2 = calloc(1, sizeof(ay_birail2_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&birail2->close);
  fscanf(fileptr,"%d\n",&birail2->sections);
  fscanf(fileptr,"%d\n",&birail2->has_start_cap);
  fscanf(fileptr,"%d\n",&birail2->has_end_cap);
  fscanf(fileptr,"%d\n",&birail2->glu_display_mode);
  fscanf(fileptr,"%lg\n",&birail2->glu_sampling_tolerance);

  o->refine = birail2;

 return AY_OK;
} /* ay_birail2_readcb */


int
ay_birail2_writecb(FILE *fileptr, ay_object *o)
{
 ay_birail2_object *birail2 = NULL;

  if(!o)
    return AY_ENULL;

  birail2 = (ay_birail2_object *)(o->refine);

  fprintf(fileptr, "%d\n", birail2->close);
  fprintf(fileptr, "%d\n", birail2->sections);
  fprintf(fileptr, "%d\n", birail2->has_start_cap);
  fprintf(fileptr, "%d\n", birail2->has_end_cap);
  fprintf(fileptr, "%d\n", birail2->glu_display_mode);
  fprintf(fileptr, "%g\n", birail2->glu_sampling_tolerance);

 return AY_OK;
} /* ay_birail2_writecb */


int
ay_birail2_wribcb(char *file, ay_object *o)
{
 ay_birail2_object *birail2 = NULL;

  if(!o)
   return AY_ENULL;

  birail2 = (ay_birail2_object*)o->refine;

  if(birail2->npatch)
    ay_wrib_object(file, birail2->npatch);

  if(birail2->start_cap)
    ay_wrib_object(file, birail2->start_cap);

  if(birail2->end_cap)
    ay_wrib_object(file, birail2->end_cap);

 return AY_OK;
} /* ay_birail2_wribcb */


int
ay_birail2_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_birail2_object *birail2 = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  birail2 = (ay_birail2_object *)o->refine; 

  if(birail2->npatch)
    {
      ay_bbc_get(birail2->npatch, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return AY_OK;
} /* ay_birail2_bbccb */


int
ay_birail2_notifycb(ay_object *o)
{
 ay_birail2_object *birail2 = NULL;
 ay_object *curve1 = NULL, *curve2 = NULL, *pobject1 = NULL, *pobject2 = NULL;
 ay_object *curve3 = NULL, *curve4 = NULL, *pobject3 = NULL, *pobject4 = NULL;
 ay_object *npatch = NULL;
 int ay_status = AY_OK;
 int got_c1 = AY_FALSE, got_c2 = AY_FALSE;
 int got_c3 = AY_FALSE, got_c4 = AY_FALSE;
 int mode = 0;
 double tolerance;

  if(!o)
    return AY_ENULL;    

  birail2 = (ay_birail2_object *)(o->refine);

  mode = birail2->glu_display_mode;
  tolerance = birail2->glu_sampling_tolerance;

  /* remove old objects */
  if(birail2->npatch)
    ay_object_delete(birail2->npatch);
  birail2->npatch = NULL;

  if(birail2->start_cap)
    ay_object_delete(birail2->start_cap);
  birail2->start_cap = NULL;

  if(birail2->end_cap)
    ay_object_delete(birail2->end_cap);
  birail2->end_cap = NULL;

  /* get curves to birail2 */
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
    goto cleanup;

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

  if(!o->down->next->next)
    goto cleanup;

  curve3 = o->down->next->next;
  if(curve3->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(curve3, AY_IDNCURVE, &pobject3);
      if(!pobject3)
	{
	  return AY_OK;
	}
      else
	{
	  curve3 = pobject3;
	  got_c3 = AY_TRUE;
	} /* if */
    } /* if */

  if(!o->down->next->next->next)
    goto cleanup;

  curve4 = o->down->next->next->next;
  if(curve4->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(curve4, AY_IDNCURVE, &pobject4);
      if(!pobject4)
	{
	  return AY_OK;
	}
      else
	{
	  curve4 = pobject4;
	  got_c4 = AY_TRUE;
	} /* if */
    } /* if */

  /* birail2 */
  if(!(npatch = calloc(1, sizeof(ay_object))))
    {
      return AY_ERROR;
    }

  ay_object_defaults(npatch);
  npatch->type = AY_IDNPATCH;

  ay_status = ay_npt_birail2(curve1, curve2, curve3, curve4,
			   birail2->sections, AY_FALSE/*birail2->close*/,
			   (ay_nurbpatch_object **)(&(npatch->refine)),
			   birail2->has_start_cap, &(birail2->start_cap),
			   birail2->has_end_cap, &(birail2->end_cap));

  if(ay_status)
    return ay_status;

  /* copy sampling tolerance/mode over to new objects */
  birail2->npatch = npatch;

  ((ay_nurbpatch_object *)npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)npatch->refine)->glu_display_mode =
    mode;

  /* create caps */
  if(birail2->start_cap)
    {
      ((ay_nurbpatch_object *)
       (birail2->start_cap->refine))->glu_sampling_tolerance = tolerance;
      ((ay_nurbpatch_object *)
       (birail2->start_cap->refine))->glu_display_mode = mode;
    }

  if(birail2->end_cap)
    {
      ((ay_nurbpatch_object *)
       (birail2->end_cap->refine))->glu_sampling_tolerance = tolerance;
      ((ay_nurbpatch_object *)
       (birail2->end_cap->refine))->glu_display_mode = mode;
    }

  /* remove provided objects */
cleanup:
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

  if(got_c4)
    {
      ay_object_delete(pobject4);
    }

 return AY_OK;
} /* ay_birail2_notifycb */


int
ay_birail2_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "birail2_providecb";
 ay_birail2_object *s = NULL;
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

  s = (ay_birail2_object *) o->refine;

  if(type == AY_IDNPATCH)
    {
      t = &(new);

      /* copy birail2 */
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
} /* ay_birail2_providecb */


int
ay_birail2_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_birail2_object *r = NULL;
 ay_object *new = NULL, **next = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */

  r = (ay_birail2_object *) o->refine;

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
} /* ay_birail2_convertcb */


int
ay_birail2_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_birail2_name,
				    ay_birail2_createcb,
				    ay_birail2_deletecb,
				    ay_birail2_copycb,
				    ay_birail2_drawcb,
				    NULL, /* no handles */
				    ay_birail2_shadecb,
				    ay_birail2_setpropcb,
				    ay_birail2_getpropcb,
				    ay_birail2_getpntcb,
				    ay_birail2_readcb,
				    ay_birail2_writecb,
				    ay_birail2_wribcb,
				    ay_birail2_bbccb,
				    AY_IDBIRAIL2);

  
  ay_status = ay_notify_register(ay_birail2_notifycb, AY_IDBIRAIL2);

  ay_status = ay_convert_register(ay_birail2_convertcb, AY_IDBIRAIL2);

  ay_status = ay_provide_register(ay_birail2_providecb, AY_IDBIRAIL2);

 return ay_status;
} /* ay_birail2_init */

