/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2009 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* concatnp.c - concatnp object */

static char *ay_concatnp_name = "ConcatNP";

int ay_concatnp_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_concatnp_createcb:
 *  create callback function of concatnp object
 */
int
ay_concatnp_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtconcatnp";
 ay_concatnp_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_concatnp_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->knot_type = AY_KTCUSTOM;
  new->ftlength = -0.3;

  o->hide_children = AY_TRUE;
  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_concatnp_createcb */


/* ay_concatnp_deletecb:
 *  delete callback function of concatnp object
 */
int
ay_concatnp_deletecb(void *c)
{
 ay_concatnp_object *concatnp = NULL;

  if(!c)
    return AY_ENULL;

  concatnp = (ay_concatnp_object *)(c);

  if(concatnp->uv_select)
    free(concatnp->uv_select);

  if(concatnp->npatch)
    (void)ay_object_delete(concatnp->npatch);

  if(concatnp->caps_and_bevels)
    (void)ay_object_deletemulti(concatnp->caps_and_bevels, AY_FALSE);

  free(concatnp);

 return AY_OK;
} /* ay_concatnp_deletecb */


/* ay_concatnp_copycb:
 *  copy callback function of concatnp object
 */
int
ay_concatnp_copycb(void *src, void **dst)
{
 ay_concatnp_object *concatnp = NULL, *concatnpsrc;

  if(!src || !dst)
    return AY_ENULL;

  concatnpsrc = (ay_concatnp_object*)src;

  if(!(concatnp = malloc(sizeof(ay_concatnp_object))))
    return AY_EOMEM;

  memcpy(concatnp, src, sizeof(ay_concatnp_object));

  if(concatnpsrc->uv_select)
    {
      if(!(concatnp->uv_select = malloc((strlen(concatnpsrc->uv_select)+1)*
					sizeof(char))))
	{ free(concatnp); return AY_EOMEM; }

      strcpy(concatnp->uv_select, concatnpsrc->uv_select);
    }

  concatnp->npatch = NULL;
  concatnp->caps_and_bevels = NULL;

  *dst = (void *)concatnp;

 return AY_OK;
} /* ay_concatnp_copycb */


/* ay_concatnp_drawcb:
 *  draw (display in an Ayam view window) callback function of concatnp object
 */
int
ay_concatnp_drawcb(struct Togl *togl, ay_object *o)
{
 ay_concatnp_object *concatnp = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  concatnp = (ay_concatnp_object *)o->refine;

  if(!concatnp)
    return AY_ENULL;

  if(concatnp->npatch)
    {
      ay_draw_object(togl, concatnp->npatch, AY_TRUE);
    }

  b = concatnp->caps_and_bevels;
  while(b)
    {
      ay_draw_object(togl, b, AY_TRUE);
      b = b->next;
    }

 return AY_OK;
} /* ay_concatnp_drawcb */


/* ay_concatnp_shadecb:
 *  shade (display in an Ayam view window) callback function of concatnp object
 */
int
ay_concatnp_shadecb(struct Togl *togl, ay_object *o)
{
 ay_concatnp_object *concatnp = NULL;
 ay_object *b;

  if(!o)
    return AY_ENULL;

  concatnp = (ay_concatnp_object *)o->refine;

  if(!concatnp)
    return AY_ENULL;

  if(concatnp->npatch)
    {
      ay_shade_object(togl, concatnp->npatch, AY_FALSE);
    }

  b = concatnp->caps_and_bevels;
  while(b)
    {
      ay_shade_object(togl, b, AY_FALSE);
      b = b->next;
    }

 return AY_OK;
} /* ay_concatnp_shadecb */


/* ay_concatnp_drawacb:
 *  draw annotations (in an Ayam view window) callback function of
 *  concatnp object
 */
int
ay_concatnp_drawacb(struct Togl *togl, ay_object *o)
{
 ay_concatnp_object *concatnp = NULL;
 ay_nurbpatch_object *np = NULL;

  if(!o)
    return AY_ENULL;

  concatnp = (ay_concatnp_object *) o->refine;

  if(!concatnp)
    return AY_ENULL;

  if(concatnp->npatch)
    {
      np = (ay_nurbpatch_object *)concatnp->npatch->refine;

      /* draw arrow */
      ay_draw_arrow(togl, &(np->controlv[np->width*np->height*4-8]),
		    &(np->controlv[np->width*np->height*4-4]));
    }

 return AY_OK;
} /* ay_concatnp_drawacb */


/* ay_concatnp_drawhcb:
 *  draw handles (in an Ayam view window) callback function of concatnp object
 */
int
ay_concatnp_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_concatnp_object *concatnp = NULL;

  if(!o)
    return AY_ENULL;

  concatnp = (ay_concatnp_object *) o->refine;

  if(!concatnp)
    return AY_ENULL;

  if(concatnp->npatch)
    {
      ay_npt_drawrohandles((ay_nurbpatch_object *)concatnp->npatch);
    }

 return AY_OK;
} /* ay_concatnp_drawhcb */


/* ay_concatnp_getpntcb:
 *  get point (editing and selection) callback function of concatnp object
 */
int
ay_concatnp_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbpatch_object *patch = NULL;
 ay_concatnp_object *concatnp = NULL;

  if(!o)
    return AY_ENULL;

  concatnp = (ay_concatnp_object *)o->refine;

  if(!concatnp)
    return AY_ENULL;

  if(concatnp->npatch)
    {
      patch = (ay_nurbpatch_object *)concatnp->npatch->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, patch->width*patch->height, 4,
			     patch->controlv);
    }

 return AY_ERROR;
} /* ay_concatnp_getpntcb */


/* ay_concatnp_setpropcb:
 *  set property (from Tcl to C context) callback function of concatnp object
 */
int
ay_concatnp_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "ConcatNPAttrData";
 char fname[] = "concatnp_setpropcb";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_concatnp_object *concatnp = NULL;
 char *string = NULL;
 int stringlen, newknottype = 0;

  if(!interp || !o)
    return AY_ENULL;

  concatnp = (ay_concatnp_object *)o->refine;

  if(!concatnp)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(concatnp->type));

  Tcl_SetStringObj(ton,"Order",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(concatnp->order));

  Tcl_SetStringObj(ton,"Revert",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(concatnp->revert));

  Tcl_SetStringObj(ton,"Knot-Type",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(newknottype));
  newknottype++;
  if(newknottype < AY_KTCUSTOM)
    concatnp->knot_type = newknottype;
  else
    concatnp->knot_type = newknottype + 1;

  if(newknottype == 5)
    concatnp->knot_type = AY_KTCUSTOM;

  Tcl_SetStringObj(ton,"FillGaps",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(concatnp->fillgaps));

  Tcl_SetStringObj(ton,"FTLength",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(concatnp->ftlength));

  Tcl_SetStringObj(ton,"Compatible",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(concatnp->compat));

  Tcl_SetStringObj(ton,"UVSelect",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  string = Tcl_GetStringFromObj(to, &stringlen);
  if(!string)
    {
      ay_error(AY_ENULL, fname, NULL);
      goto cleanup;
    }

  if(concatnp->uv_select)
    {
      free(concatnp->uv_select);
      concatnp->uv_select = NULL;
    }

  if(stringlen > 0)
    {
      if(!(concatnp->uv_select = malloc((stringlen+1)*sizeof(char))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  goto cleanup;
	}
      strcpy(concatnp->uv_select, string);
    }

cleanup:

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_concatnp_setpropcb */


/* ay_concatnp_getpropcb:
 *  get property (from C to Tcl context) callback function of concatnp object
 */
int
ay_concatnp_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="ConcatNPAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_concatnp_object *concatnp = NULL;

  if(!interp || !o)
    return AY_ENULL;

  concatnp = (ay_concatnp_object *)(o->refine);

  if(!concatnp)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_NewIntObj(concatnp->type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order",-1);
  to = Tcl_NewIntObj(concatnp->order);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Revert",-1);
  to = Tcl_NewIntObj(concatnp->revert);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Knot-Type",-1);
  if(concatnp->knot_type < AY_KTCUSTOM)
    to = Tcl_NewIntObj(concatnp->knot_type-1);
  else
    if(concatnp->knot_type == AY_KTCUSTOM)
      to = Tcl_NewIntObj(5);
    else
      to = Tcl_NewIntObj(concatnp->knot_type-2);

  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"FillGaps",-1);
  to = Tcl_NewIntObj(concatnp->fillgaps);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"FTLength",-1);
  to = Tcl_NewDoubleObj(concatnp->ftlength);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ton = Tcl_NewStringObj("UVSelect",-1);
  to = Tcl_NewStringObj(concatnp->uv_select,-1);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Compatible",-1);
  to = Tcl_NewIntObj(concatnp->compat);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getnpinfo(interp, n1, concatnp->npatch);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_concatnp_getpropcb */


/* ay_concatnp_readcb:
 *  read (from scene file) callback function of concatnp object
 */
int
ay_concatnp_readcb(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_concatnp_object *concatnp = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(concatnp = calloc(1, sizeof(ay_concatnp_object))))
    { return AY_EOMEM; }

  concatnp->ftlength = -0.3;

  fscanf(fileptr, "%d\n", &concatnp->type);
  fscanf(fileptr, "%d\n", &concatnp->revert);
  fscanf(fileptr, "%d\n", &concatnp->knot_type);

  fscanf(fileptr, "%d\n", &concatnp->fillgaps);
  if(ay_read_version <= 14)
    {
      fscanf(fileptr, "%lg\n", &concatnp->ftlength);
    }
  else
    {
      /* Since Ayam 1.20: */
      fscanf(fileptr, "%d\n", &concatnp->order);
      fscanf(fileptr, "%lg", &concatnp->ftlength);
      (void)fgetc(fileptr);

      ay_status = ay_read_string(fileptr, &(concatnp->uv_select));
      if(ay_status)
	{
	  free(concatnp);
	  return ay_status;
	}
    }

  if(ay_read_version >= 15)
    {
      /* Since Ayam 1.21: */
      fscanf(fileptr, "%d\n", &concatnp->compat);
    }

  o->refine = concatnp;

 return AY_OK;
} /* ay_concatnp_readcb */


/* ay_concatnp_writecb:
 *  write (to scene file) callback function of concatnp object
 */
int
ay_concatnp_writecb(FILE *fileptr, ay_object *o)
{
 ay_concatnp_object *concatnp = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  concatnp = (ay_concatnp_object *)(o->refine);

  if(!concatnp)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", concatnp->type);
  fprintf(fileptr, "%d\n", concatnp->revert);
  fprintf(fileptr, "%d\n", concatnp->knot_type);

  fprintf(fileptr, "%d\n", concatnp->fillgaps);
  fprintf(fileptr, "%d\n", concatnp->order);
  fprintf(fileptr, "%g\n", concatnp->ftlength);

  if(concatnp->uv_select)
    fprintf(fileptr, "%s", concatnp->uv_select);

  fprintf(fileptr, "\n");

  fprintf(fileptr, "%d\n", concatnp->compat);

 return AY_OK;
} /* ay_concatnp_writecb */


/* ay_concatnp_wribcb:
 *  RIB export callback function of concatnp object
 */
int
ay_concatnp_wribcb(char *file, ay_object *o)
{
 ay_concatnp_object *concatnp = NULL;
 ay_object *b;

  if(!o)
   return AY_ENULL;

  concatnp = (ay_concatnp_object*)o->refine;

  if(!concatnp)
    return AY_ENULL;

  if(concatnp->npatch)
    ay_wrib_toolobject(file, concatnp->npatch, o);

  b = concatnp->caps_and_bevels;
  while(b)
    {
      ay_wrib_object(file, b);
      b = b->next;
    }

 return AY_OK;
} /* ay_concatnp_wribcb */


/* ay_concatnp_bbccb:
 *  bounding box calculation callback function of concatnp object
 */
int
ay_concatnp_bbccb(ay_object *o, double *bbox, int *flags)
{

  if(!o || !bbox || !flags)
    return AY_ENULL;

  /* use the bounding boxes of the children */
  *flags = 2;

 return AY_OK;
} /* ay_concatnp_bbccb */


/* ay_concatnp_notifycb:
 *  notification callback function of concatnp object
 */
int
ay_concatnp_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "concatnp_notifycb";
 ay_concatnp_object *concatnp = NULL;
 ay_object *down = NULL, *patches = NULL, **next = NULL;
 ay_nurbpatch_object *np = NULL;
 ay_object *bevel = NULL, **nextcb;
 ay_bparam bparams = {0};
 ay_cparam cparams = {0};
 int mode;
 double tolerance;

  if(!o)
    return AY_ENULL;

  concatnp = (ay_concatnp_object *)(o->refine);

  if(!concatnp)
    return AY_ENULL;

  mode = concatnp->display_mode;
  tolerance = concatnp->glu_sampling_tolerance;

  nextcb = &(concatnp->caps_and_bevels);

  /* remove old objects */
  if(concatnp->npatch)
    (void)ay_object_delete(concatnp->npatch);
  concatnp->npatch = NULL;

  if(concatnp->caps_and_bevels)
    {
      (void)ay_object_deletemulti(concatnp->caps_and_bevels, AY_FALSE);
      concatnp->caps_and_bevels = NULL;
    }

  /* get child objects */
  down = o->down;
  next = &patches;
  while(down)
    {
      /* copy or provide */
      if(down->type == AY_IDNPATCH)
	{
	  ay_status = ay_object_copy(down, next);
	  if(ay_status)
	    goto cleanup;
	  next = &((*next)->next);
	}
      else
	{
	  if(ay_provide_object(down, AY_IDNPATCH, NULL) == AY_OK)
	    {
	      ay_provide_object(down, AY_IDNPATCH, next);
	      while(*next)
		{
		  next = &((*next)->next);
		}
	    }
	  else
	    {
	      if(down->type == AY_IDNCURVE)
		{
		  ay_status = ay_object_copy(down, next);
		  if(ay_status)
		    goto cleanup;
		  next = &((*next)->next);
		}
	      else
		{
		  ay_provide_object(down, AY_IDNCURVE, next);
		  while(*next)
		    {
		      next = &((*next)->next);
		    }
		} /* if */
	    } /* if */
	} /* if */

      down = down->next;
    } /* while */

  /* gracefully exit, if there are no
     (or not enough?) patches to concat */
  if(!patches)
    {
      goto cleanup;
    }

  ay_status = ay_npt_concat(patches, concatnp->type, concatnp->order,
			    concatnp->knot_type,
			    concatnp->fillgaps, concatnp->ftlength,
			    concatnp->compat,
			    concatnp->uv_select, &concatnp->npatch);

  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Failed to concat!");
    }
  else
    {
      np = (ay_nurbpatch_object*)concatnp->npatch->refine;

      np->glu_sampling_tolerance = tolerance;
      np->display_mode = mode;

      if(concatnp->revert)
	{
	  ay_status = ay_npt_revertu(np);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, "Failed to revert!");
	    } /* if */
	} /* if */
    } /* if */

  /* get bevel and cap parameters */
  if(o->tags)
    {
      ay_bevelt_parsetags(o->tags, &bparams);
      ay_capt_parsetags(o->tags, &cparams);
    }

  /* create/add caps */
  if(cparams.has_caps)
    {
      ay_status = ay_capt_addcaps(&cparams, &bparams, concatnp->npatch, nextcb);
      if(ay_status)
	goto cleanup;

      while(*nextcb)
	nextcb = &((*nextcb)->next);
    }

  /* create/add bevels */
  if(bparams.has_bevels)
    {
      bparams.dirs[1] = !bparams.dirs[1];
      bparams.dirs[2] = !bparams.dirs[2];
      bparams.radii[2] = -bparams.radii[2];

      ay_status = ay_bevelt_addbevels(&bparams, &cparams,
				      concatnp->npatch, nextcb);
      if(ay_status)
	goto cleanup;
    }

  if(concatnp->caps_and_bevels)
    {
      bevel = concatnp->caps_and_bevels;
      while(bevel)
	{
	  ((ay_nurbpatch_object *)
	   (bevel->refine))->glu_sampling_tolerance = tolerance;
	  ((ay_nurbpatch_object *)
	   (bevel->refine))->display_mode = mode;
	  bevel = bevel->next;
	}
    }

cleanup:

  /* free list of temporary patches/curves */
  if(patches)
    (void)ay_object_deletemulti(patches, AY_FALSE);

  /* recover selected points */
  if(o->selp)
    {
      if(concatnp->npatch)
	ay_concatnp_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

 return ay_status;
} /* ay_concatnp_notifycb */


/* ay_concatnp_providecb:
 *  provide callback function of concatnp object
 */
int
ay_concatnp_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 ay_concatnp_object *c = NULL;

  if(!o)
    return AY_ENULL;

  c = (ay_concatnp_object *) o->refine;

  if(!c)
    return AY_ENULL;

 return ay_provide_nptoolobj(o, type, c->npatch, c->caps_and_bevels, result);
} /* ay_concatnp_providecb */


/* ay_concatnp_convertcb:
 *  convert callback function of concatnp object
 */
int
ay_concatnp_convertcb(ay_object *o, int in_place)
{
 ay_concatnp_object *c = NULL;

  if(!o)
    return AY_ENULL;

  c = (ay_concatnp_object *) o->refine;

  if(!c)
    return AY_ENULL;

 return ay_convert_nptoolobj(o, c->npatch, c->caps_and_bevels, in_place);
} /* ay_concatnp_convertcb */


/* ay_concatnp_init:
 *  initialize the concatnp object module
 */
int
ay_concatnp_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_concatnp_name,
				    ay_concatnp_createcb,
				    ay_concatnp_deletecb,
				    ay_concatnp_copycb,
				    ay_concatnp_drawcb,
				    ay_concatnp_drawhcb,
				    ay_concatnp_shadecb,
				    ay_concatnp_setpropcb,
				    ay_concatnp_getpropcb,
				    ay_concatnp_getpntcb,
				    ay_concatnp_readcb,
				    ay_concatnp_writecb,
				    ay_concatnp_wribcb,
				    ay_concatnp_bbccb,
				    AY_IDCONCATNP);


  ay_status += ay_draw_registerdacb(ay_concatnp_drawacb, AY_IDCONCATNP);

  ay_status += ay_notify_register(ay_concatnp_notifycb, AY_IDCONCATNP);

  ay_status += ay_convert_register(ay_concatnp_convertcb, AY_IDCONCATNP);

  ay_status += ay_provide_register(ay_concatnp_providecb, AY_IDCONCATNP);

 return ay_status;
} /* ay_concatnp_init */

