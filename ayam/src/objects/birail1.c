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

/* birail1.c - birail1 object */

static char *ay_birail1_name = "Birail1";

int
ay_birail1_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtbirail1";
 ay_birail1_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_birail1_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->sections = 5;

  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_birail1_createcb */


int
ay_birail1_deletecb(void *c)
{
 ay_birail1_object *birail1 = NULL;

  if(!c)
    return AY_ENULL;    

  birail1 = (ay_birail1_object *)(c);

  if(birail1->npatch)
    ay_object_delete(birail1->npatch);

  if(birail1->caps_and_bevels)
    {
      ay_object_deletemulti(birail1->caps_and_bevels);
      birail1->caps_and_bevels = NULL;
    }

  free(birail1);

 return AY_OK;
} /* ay_birail1_deletecb */


int
ay_birail1_copycb(void *src, void **dst)
{
 ay_birail1_object *birail1 = NULL, *birail1src = NULL;

  if(!src || !dst)
    return AY_ENULL;

  birail1src = (ay_birail1_object *)src;

  if(!(birail1 = calloc(1, sizeof(ay_birail1_object))))
    return AY_EOMEM; 

  memcpy(birail1, src, sizeof(ay_birail1_object)); 

  /* copy npatch */
  ay_object_copy(birail1src->npatch, &(birail1->npatch));

  birail1->caps_and_bevels = NULL;

  *dst = (void *)birail1;

 return AY_OK;
} /* ay_birail1_copycb */


int
ay_birail1_drawcb(struct Togl *togl, ay_object *o)
{
 ay_birail1_object *birail1;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)o->refine;

  if(!birail1)
    return AY_ENULL;

  if(birail1->npatch)
    ay_draw_object(togl, birail1->npatch, AY_TRUE);

  if(birail1->caps_and_bevels)
    {
      b = birail1->caps_and_bevels;
      while(b)
	{
	  ay_draw_object(togl, b, AY_TRUE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_birail1_drawcb */


int
ay_birail1_shadecb(struct Togl *togl, ay_object *o)
{
 ay_birail1_object *birail1;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)o->refine;

  if(!birail1)
    return AY_ENULL;

  if(birail1->npatch)
    ay_shade_object(togl, birail1->npatch, AY_FALSE);

  if(birail1->caps_and_bevels)
    {
      b = birail1->caps_and_bevels;
      while(b)
	{
	  ay_shade_object(togl, b, AY_FALSE);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_birail1_shadecb */


int
ay_birail1_drawhcb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_birail1_drawhcb */


int
ay_birail1_getpntcb(int mode, ay_object *o, double *p)
{

 return AY_OK;
} /* ay_birail1_getpntcb */


/* Tcl -> C! */
int
ay_birail1_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "Birail1AttrData";
 /* char fname[] = "birail1_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_birail1_object *birail1 = NULL;

  if(!o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Close",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail1->close));

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail1->sections));

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail1->has_start_cap));

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail1->has_end_cap));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail1->glu_display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(birail1->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_force(o);

  o->modified = AY_TRUE;
  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_birail1_setpropcb */


/* C -> Tcl! */
int
ay_birail1_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="Birail1AttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_birail1_object *birail1 = NULL;

  if(!o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Close",-1);
  to = Tcl_NewIntObj(birail1->close);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Sections",-1);
  to = Tcl_NewIntObj(birail1->sections);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_NewIntObj(birail1->has_start_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_NewIntObj(birail1->has_end_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(birail1->glu_display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(birail1->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getnpinfo(interp, n1, birail1->npatch);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_birail1_getpropcb */


int
ay_birail1_readcb(FILE *fileptr, ay_object *o)
{
 ay_birail1_object *birail1 = NULL;

 if(!o)
   return AY_ENULL;

  if(!(birail1 = calloc(1, sizeof(ay_birail1_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&birail1->close);
  fscanf(fileptr,"%d\n",&birail1->sections);
  fscanf(fileptr,"%d\n",&birail1->has_start_cap);
  fscanf(fileptr,"%d\n",&birail1->has_end_cap);
  fscanf(fileptr,"%d\n",&birail1->glu_display_mode);
  fscanf(fileptr,"%lg\n",&birail1->glu_sampling_tolerance);

  o->refine = birail1;

 return AY_OK;
} /* ay_birail1_readcb */


int
ay_birail1_writecb(FILE *fileptr, ay_object *o)
{
 ay_birail1_object *birail1 = NULL;

  if(!o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)(o->refine);

  fprintf(fileptr, "%d\n", birail1->close);
  fprintf(fileptr, "%d\n", birail1->sections);
  fprintf(fileptr, "%d\n", birail1->has_start_cap);
  fprintf(fileptr, "%d\n", birail1->has_end_cap);
  fprintf(fileptr, "%d\n", birail1->glu_display_mode);
  fprintf(fileptr, "%g\n", birail1->glu_sampling_tolerance);

 return AY_OK;
} /* ay_birail1_writecb */


int
ay_birail1_wribcb(char *file, ay_object *o)
{
 ay_birail1_object *birail1;
 ay_object *b;

  if(!o)
   return AY_ENULL;

  birail1 = (ay_birail1_object*)o->refine;

  if(birail1->npatch)
    ay_wrib_object(file, birail1->npatch);

  if(birail1->caps_and_bevels)
    {
      b = birail1->caps_and_bevels;
      while(b)
	{
	  ay_wrib_object(file, b);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_birail1_wribcb */


int
ay_birail1_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_birail1_object *birail1 = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)o->refine; 

  if(birail1->npatch)
    {
      ay_bbc_get(birail1->npatch, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return AY_OK;
} /* ay_birail1_bbccb */


int
ay_birail1_notifycb(ay_object *o)
{
 ay_birail1_object *birail1 = NULL;
 ay_object *curve1 = NULL, *curve2 = NULL, *pobject1 = NULL, *pobject2 = NULL;
 ay_object *curve3 = NULL, *pobject3 = NULL;
 ay_object curve4 = {0}, *curve5;
 ay_object *npatch = NULL, **nextcb, *start_cap = NULL, *end_cap = NULL;
 ay_object *bevel = NULL;
 int ay_status = AY_OK;
 int got_c1 = AY_FALSE, got_c2 = AY_FALSE, got_c3 = AY_FALSE, mode = 0;
 int has_startb = AY_FALSE, has_endb = AY_FALSE;
 int startb_type, endb_type, startb_sense, endb_sense;
 double tolerance, startb_radius, endb_radius;

  if(!o)
    return AY_ENULL;    

  birail1 = (ay_birail1_object *)(o->refine);

  mode = birail1->glu_display_mode;
  tolerance = birail1->glu_sampling_tolerance;

  nextcb = &(birail1->caps_and_bevels);

  /* remove old objects */
  if(birail1->npatch)
    ay_object_delete(birail1->npatch);
  birail1->npatch = NULL;

  if(birail1->caps_and_bevels)
    {
      ay_object_deletemulti(birail1->caps_and_bevels);
      birail1->caps_and_bevels = NULL;
    }

  /* get curves to birail1 */
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
	  goto cleanup;
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
	  goto cleanup;
	}
      else
	{
	  curve3 = pobject3;
	  got_c3 = AY_TRUE;
	} /* if */
    } /* if */

  /* get bevel parameters */
  ay_npt_getbeveltags(o, 0, &has_startb, &startb_type, &startb_radius,
		      &startb_sense);
  ay_npt_getbeveltags(o, 1, &has_endb, &endb_type, &endb_radius,
		      &endb_sense);

  /* birail1 */
  if(!(npatch = calloc(1, sizeof(ay_object))))
    {
      ay_status = AY_EOMEM; goto cleanup;
    }

  ay_object_defaults(npatch);
  npatch->type = AY_IDNPATCH;

  ay_status = ay_npt_birail1(curve1, curve2, curve3,
			   birail1->sections, AY_FALSE/*birail1->close*/,
			   (ay_nurbpatch_object **)(&(npatch->refine)),
			   has_startb?AY_FALSE:birail1->has_start_cap,
			   &start_cap,
			   has_endb?AY_FALSE:birail1->has_end_cap,
			   &end_cap);
    
  if(ay_status)
    goto cleanup;

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

  birail1->npatch = npatch;

  /* copy sampling tolerance/mode attributes over to birail */
  ((ay_nurbpatch_object *)birail1->npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)birail1->npatch->refine)->glu_display_mode =
    mode;

  /* create bevels and caps */
  if(/*!birail1->close &&*/ has_startb)
    {
      ay_object_defaults(&curve4);
      curve4.type = AY_IDNCURVE;
      ay_status = ay_npt_extractnc(npatch, 3, 0.0, AY_FALSE,
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
      if(birail1->has_start_cap)
	{
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

  if(/*!birail1->close &&*/ has_endb)
    {
      memset(&curve4, 0, sizeof(ay_object));
      ay_object_defaults(&curve4);
      curve4.type = AY_IDNCURVE;
      ay_status = ay_npt_extractnc(npatch, 2, 0.0, AY_FALSE,
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
      if(birail1->has_end_cap)
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
  if(birail1->caps_and_bevels)
    {
      bevel = birail1->caps_and_bevels;
      while(bevel)
	{
	  ((ay_nurbpatch_object *)
	   (bevel->refine))->glu_sampling_tolerance = tolerance;
	  ((ay_nurbpatch_object *)
	   (bevel->refine))->glu_display_mode = mode;
	  bevel = bevel->next;
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
    free(npatch);

 return ay_status;
} /* ay_birail1_notifycb */


int
ay_birail1_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "birail1_providecb";
 ay_birail1_object *s = NULL;
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

  s = (ay_birail1_object *) o->refine;

  if(type == AY_IDNPATCH)
    {
      t = &(new);

      /* copy birail1 */
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

      /* copy eventually present TP tags */
      ay_npt_copytptag(o, new);

      *result = new;
    } /* if */

 return ay_status;
} /* ay_birail1_providecb */


int
ay_birail1_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_birail1_object *birail1 = NULL;
 ay_object *new = NULL, **next = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */

  birail1 = (ay_birail1_object *) o->refine;

  if(birail1->caps_and_bevels)
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

      if(birail1->npatch)
	{
	  ay_status = ay_object_copy(birail1->npatch, next);
	  if(*next)
	    {
	      (*next)->parent = AY_TRUE;
	      ay_object_crtendlevel(&(*next)->down);
	      next = &((*next)->next);
	    }
	}

      if(birail1->caps_and_bevels)
	{
	  b = birail1->caps_and_bevels;
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
      
      /* copy eventually present TP tags */
      ay_npt_copytptag(o, new->down);

      ay_object_crtendlevel(next);
    }
  else
    {
       if(birail1->npatch)
	{
	  ay_status = ay_object_copy(birail1->npatch, &new);
	  ay_trafo_copy(o, new);
	  new->hide_children = AY_TRUE;
	  new->parent = AY_TRUE;
	  ay_object_crtendlevel(&(new->down));

	  /* copy eventually present TP tags */
	  ay_npt_copytptag(o, new);
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
} /* ay_birail1_convertcb */


int
ay_birail1_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_birail1_name,
				    ay_birail1_createcb,
				    ay_birail1_deletecb,
				    ay_birail1_copycb,
				    ay_birail1_drawcb,
				    NULL, /* no handles */
				    ay_birail1_shadecb,
				    ay_birail1_setpropcb,
				    ay_birail1_getpropcb,
				    ay_birail1_getpntcb,
				    ay_birail1_readcb,
				    ay_birail1_writecb,
				    ay_birail1_wribcb,
				    ay_birail1_bbccb,
				    AY_IDBIRAIL1);

  
  ay_status = ay_notify_register(ay_birail1_notifycb, AY_IDBIRAIL1);

  ay_status = ay_convert_register(ay_birail1_convertcb, AY_IDBIRAIL1);

  ay_status = ay_provide_register(ay_birail1_providecb, AY_IDBIRAIL1);

 return ay_status;
} /* ay_birail1_init */

