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

/* skin.c - skin object */

static char *ay_skin_name = "Skin";

int
ay_skin_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtskin";
 ay_skin_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_skin_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_skin_createcb */


int
ay_skin_deletecb(void *c)
{
 ay_skin_object *skin = NULL;

  if(!c)
    return AY_ENULL;    

  skin = (ay_skin_object *)(c);

  if(skin->npatch)
    ay_object_delete(skin->npatch);

  if(skin->start_cap)
    ay_object_delete(skin->start_cap);

  if(skin->end_cap)
    ay_object_delete(skin->end_cap);

  free(skin);

 return AY_OK;
} /* ay_skin_deletecb */


int
ay_skin_copycb(void *src, void **dst)
{
 ay_skin_object *skin = NULL, *skinsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  skinsrc = (ay_skin_object *)src;

  if(!(skin = calloc(1, sizeof(ay_skin_object))))
    return AY_EOMEM; 

  memcpy(skin, src, sizeof(ay_skin_object)); 


  /* copy npatch */
  ay_object_copy(skinsrc->npatch, &(skin->npatch));

  /* copy start cap */
  ay_object_copy(skinsrc->start_cap, &(skin->start_cap));

  /* copy end cap */
  ay_object_copy(skinsrc->end_cap, &(skin->end_cap));


  *dst = (void *)skin;

 return AY_OK;
} /* ay_skin_copycb */


int
ay_skin_drawcb(struct Togl *togl, ay_object *o)
{
 ay_skin_object *skin = NULL;

  if(!o)
    return AY_ENULL;

  skin = (ay_skin_object *)o->refine;

  if(!skin)
    return AY_ENULL;

  if(skin->npatch)
    ay_draw_object(togl, skin->npatch, AY_TRUE);

  if(skin->start_cap)
    ay_draw_object(togl, skin->start_cap, AY_TRUE);

  if(skin->end_cap)
    ay_draw_object(togl, skin->end_cap, AY_TRUE);

 return AY_OK;
} /* ay_skin_drawcb */


int
ay_skin_shadecb(struct Togl *togl, ay_object *o)
{
 ay_skin_object *skin = NULL;

  if(!o)
    return AY_ENULL;

  skin = (ay_skin_object *)o->refine;

  if(!skin)
    return AY_ENULL;

  if(skin->npatch)
    ay_shade_object(togl, skin->npatch, AY_FALSE);

  if(skin->start_cap)
    ay_shade_object(togl, skin->start_cap, AY_FALSE);

  if(skin->end_cap)
    ay_shade_object(togl, skin->end_cap, AY_FALSE);

 return AY_OK;
} /* ay_skin_shadecb */


int
ay_skin_drawhcb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_skin_drawhcb */


int
ay_skin_getpntcb(int mode, ay_object *o, double *p)
{

 return AY_OK;
} /* ay_skin_getpntcb */


/* Tcl -> C! */
int
ay_skin_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "SkinAttrData";
 /* char fname[] = "skin_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_skin_object *skin = NULL;


  if(!o)
    return AY_ENULL;

  skin = (ay_skin_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Interpolate",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(skin->interpolate));

  Tcl_SetStringObj(ton,"Order_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(skin->uorder));

  Tcl_SetStringObj(ton,"Knot-Type_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(skin->uknot_type));

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(skin->has_start_cap));

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(skin->has_end_cap));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(skin->glu_display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(skin->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_force(o);

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_skin_setpropcb */


/* C -> Tcl! */
int
ay_skin_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="SkinAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_skin_object *skin = NULL;

  if(!o)
    return AY_ENULL;

  skin = (ay_skin_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Interpolate",-1);
  to = Tcl_NewIntObj(skin->interpolate);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order_U",-1);
  to = Tcl_NewIntObj(skin->uorder);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Knot-Type_U",-1);
  to = Tcl_NewIntObj(skin->uknot_type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"StartCap",-1);
  to = Tcl_NewIntObj(skin->has_start_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"EndCap",-1);
  to = Tcl_NewIntObj(skin->has_end_cap);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(skin->glu_display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(skin->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_skin_getpropcb */


int
ay_skin_readcb(FILE *fileptr, ay_object *o)
{
 ay_skin_object *skin = NULL;

 if(!o)
   return AY_ENULL;

  if(!(skin = calloc(1, sizeof(ay_skin_object))))
    { return AY_EOMEM; }


  fscanf(fileptr,"%d\n",&skin->interpolate);
  fscanf(fileptr,"%d\n",&skin->uorder);
  fscanf(fileptr,"%d\n",&skin->uknot_type);
  fscanf(fileptr,"%d\n",&skin->has_start_cap);
  fscanf(fileptr,"%d\n",&skin->has_end_cap);
  fscanf(fileptr,"%d\n",&skin->glu_display_mode);
  fscanf(fileptr,"%lg\n",&skin->glu_sampling_tolerance);

  o->refine = skin;

 return AY_OK;
} /* ay_skin_readcb */


int
ay_skin_writecb(FILE *fileptr, ay_object *o)
{
 ay_skin_object *skin = NULL;

  if(!o)
    return AY_ENULL;

  skin = (ay_skin_object *)(o->refine);

  fprintf(fileptr, "%d\n", skin->interpolate);
  fprintf(fileptr, "%d\n", skin->uorder);
  fprintf(fileptr, "%d\n", skin->uknot_type);
  fprintf(fileptr, "%d\n", skin->has_start_cap);
  fprintf(fileptr, "%d\n", skin->has_end_cap);
  fprintf(fileptr, "%d\n", skin->glu_display_mode);
  fprintf(fileptr, "%g\n", skin->glu_sampling_tolerance);

 return AY_OK;
} /* ay_skin_writecb */


int
ay_skin_wribcb(char *file, ay_object *o)
{
 ay_skin_object *skin = NULL;

  if(!o)
   return AY_ENULL;

  skin = (ay_skin_object*)o->refine;

  if(skin->npatch)
    ay_wrib_object(file, skin->npatch);

  if(skin->start_cap)
    ay_wrib_object(file, skin->start_cap);

  if(skin->end_cap)
    ay_wrib_object(file, skin->end_cap);

 return AY_OK;
} /* ay_skin_wribcb */


int
ay_skin_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_skin_object *skin = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  skin = (ay_skin_object *)o->refine; 

  if(skin->npatch)
    {
      ay_bbc_get(skin->npatch, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return AY_OK;
} /* ay_skin_bbccb */


int
ay_skin_notifycb(ay_object *o)
{
 ay_skin_object *skin = NULL;
 ay_object *down = NULL, *c = NULL, *last = NULL, *all_curves = NULL;
 ay_object *npatch = NULL;
 /*
 ay_nurbcurve_object *nc = NULL;
 ay_nurbpatch_object *np = NULL;
 */
 int ay_status = AY_OK;
 int mode = 0, count = 0;
 double tolerance;

  if(!o)
    return AY_ENULL;    

  skin = (ay_skin_object *)(o->refine);

  mode = skin->glu_display_mode;
  tolerance = skin->glu_sampling_tolerance;

  /* remove old objects */
  if(skin->npatch)
    ay_object_delete(skin->npatch);
  skin->npatch = NULL;

  if(skin->start_cap)
    ay_object_delete(skin->start_cap);
  skin->start_cap = NULL;

  if(skin->end_cap)
    ay_object_delete(skin->end_cap);
  skin->end_cap = NULL;

  /* get curves to skin */
  if(!o->down)
    return AY_OK;
  down = o->down;

  while(down)
    {
      c = NULL;

      if(down->type == AY_IDNCURVE)
	{
	  ay_object_copy(down, &c);
	}
      else
	{
	  ay_status = ay_provide_object(down, AY_IDNCURVE, &c);
	} /* if */

      if(c == NULL)
	{
	  /*	  ay_error(AY_EWARN, fname,
		     "object is not a NURBCurve");*/
	}
      else
	{
	  if(last)
	    last->next = c;
	  else
	    all_curves = c;

	  last = c;

	  count++;
	} /* if */

      down = down->next;
    } /* while */

  /* No curves to skin? */
  if(count <= 1)
    return AY_OK;

  /* skin */
  if(!(npatch = calloc(1, sizeof(ay_object))))
    {
      return AY_ERROR;
    }

  ay_object_defaults(npatch);
  npatch->type = AY_IDNPATCH;

  /* create caps */
  if(skin->has_start_cap)
    {
      c = NULL;
      ay_status = ay_object_copy(all_curves, &c);
      ay_trafo_defaults(c);
      ay_status = ay_capt_createfromcurve(c, &(skin->start_cap));
      ay_trafo_copy(all_curves, skin->start_cap);
    } /* if */

  if(skin->has_end_cap)
    {
      c = NULL;
      ay_status = ay_object_copy(last, &c);
      ay_trafo_defaults(c);
      ay_status = ay_capt_createfromcurve(c, &(skin->end_cap));
      ay_trafo_copy(last, skin->end_cap);
    } /* if */

  ay_status = ay_npt_skin(all_curves, skin->uorder, skin->uknot_type,
			  skin->interpolate,
			  (ay_nurbpatch_object **)(&(npatch->refine)));

  if(ay_status)
    return ay_status;

  /* copy sampling tolerance/mode over to new objects */
  skin->npatch = npatch;

  ((ay_nurbpatch_object *)npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)npatch->refine)->glu_display_mode =
    mode;

  if(skin->start_cap)
    {
      ((ay_nurbpatch_object *)
       (skin->start_cap->refine))->glu_sampling_tolerance = tolerance;
      ((ay_nurbpatch_object *)
       (skin->start_cap->refine))->glu_display_mode = mode;
    }

  if(skin->end_cap)
    {
      ((ay_nurbpatch_object *)
       (skin->end_cap->refine))->glu_sampling_tolerance = tolerance;
      ((ay_nurbpatch_object *)
       (skin->end_cap->refine))->glu_display_mode = mode;
    }

  /* remove temporary curves */
  while(all_curves)
    {
      c = all_curves->next;
      ay_object_delete(all_curves);
      all_curves = c;
    }

 return AY_OK;
} /* ay_skin_notifycb */


int
ay_skin_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "skin_providecb";
 ay_skin_object *s = NULL;
 ay_object *new = NULL, **t = NULL, *p = NULL;

  if(!o || !result)
    return AY_ENULL;

  s = (ay_skin_object *) o->refine;

  if(type == AY_IDNPATCH)
    {
      t = &(new);

      /* copy skin */
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
} /* ay_skin_providecb */


int
ay_skin_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_skin_object *r = NULL;
 ay_object *new = NULL, **next = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */

  r = (ay_skin_object *) o->refine;

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
	    next = &((*next)->next);
	}

      if(r->start_cap)
	{
	  ay_status = ay_object_copy(r->start_cap, next);
	  if(*next)
	    next = &((*next)->next);
	}

      if(r->end_cap)
	{
	  ay_status = ay_object_copy(r->end_cap, next);
	  if(*next)
	    next = &((*next)->next);
	}

      ay_object_crtendlevel(next);
    }
  else
    {
       if(r->npatch)
	{
	  ay_status = ay_object_copy(r->npatch, &new);
	  ay_trafo_copy(o, new);
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
} /* ay_skin_convertcb */


int
ay_skin_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_skin_name,
				    ay_skin_createcb,
				    ay_skin_deletecb,
				    ay_skin_copycb,
				    ay_skin_drawcb,
				    NULL, /* no handles */
				    ay_skin_shadecb,
				    ay_skin_setpropcb,
				    ay_skin_getpropcb,
				    ay_skin_getpntcb,
				    ay_skin_readcb,
				    ay_skin_writecb,
				    ay_skin_wribcb,
				    ay_skin_bbccb,
				    AY_IDSKIN);

  
  ay_status = ay_notify_register(ay_skin_notifycb, AY_IDSKIN);

  ay_status = ay_convert_register(ay_skin_convertcb, AY_IDSKIN);

  ay_status = ay_provide_register(ay_skin_providecb, AY_IDSKIN);

 return ay_status;
} /* ay_skin_init */

