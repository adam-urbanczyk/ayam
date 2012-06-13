/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2004 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* birail1.c - birail1 object */

static char *ay_birail1_name = "Birail1";

int ay_birail1_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_birail1_createcb:
 *  create callback function of birail1 object
 */
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

  new->sections = 0;

  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_birail1_createcb */


/* ay_birail1_deletecb:
 *  delete callback function of birail1 object
 */
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


/* ay_birail1_copycb:
 *  copy callback function of birail1 object
 */
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

  if(birail1src->caps_and_bevels)
    ay_object_copymulti(birail1src->caps_and_bevels,
			&(birail1->caps_and_bevels));

  *dst = (void *)birail1;

 return AY_OK;
} /* ay_birail1_copycb */


/* ay_birail1_drawcb:
 *  draw (display in an Ayam view window) callback function of birail1 object
 */
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


/* ay_birail1_shadecb:
 *  shade (display in an Ayam view window) callback function of birail1 object
 */
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


/* ay_birail1_drawhcb:
 *  draw handles (in an Ayam view window) callback function of birail1 object
 */
int
ay_birail1_drawhcb(struct Togl *togl, ay_object *o)
{
 int i = 0, a = 0;
 ay_birail1_object *birail1 = NULL;
 double *pnts = NULL;
 double point_size = ay_prefs.handle_size;
 ay_nurbpatch_object *patch = NULL;

  if(!o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *) o->refine;

  if(birail1->npatch)
    {
      patch = (ay_nurbpatch_object *)birail1->npatch->refine;
      pnts = patch->controlv;
      glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
		(GLfloat)ay_prefs.obb);

      glPointSize((GLfloat)point_size);

      glBegin(GL_POINTS);
      for(i = 0; i < patch->width*patch->height; i++)
	{
	  glVertex3dv((GLdouble *)&pnts[a]);
	  a += 4;
	}
      glEnd();

      glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
		(GLfloat)ay_prefs.seb);
    }

 return AY_OK;
} /* ay_birail1_drawhcb */


/* ay_birail1_getpntcb:
 *  get point (editing and selection) callback function of birail1 object
 */
int
ay_birail1_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbpatch_object *patch = NULL;
 ay_birail1_object *birail1 = NULL;

  if(!o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)o->refine;

  if(birail1->npatch)
    {
      patch = (ay_nurbpatch_object *)birail1->npatch->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, patch->width*patch->height, 4,
			     patch->controlv);
    }

 return AY_ERROR;
} /* ay_birail1_getpntcb */


/* ay_birail1_setpropcb:
 *  set property (from Tcl to C context) callback function of birail1 object
 */
int
ay_birail1_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "Birail1AttrData";
 /* char fname[] = "birail1_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_birail1_object *birail1 = NULL;

  if(!o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(birail1->type));

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
  Tcl_GetIntFromObj(interp,to, &(birail1->display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(birail1->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_notify_object(o);

  o->modified = AY_TRUE;
  ay_notify_parent();

 return AY_OK;
} /* ay_birail1_setpropcb */


/* ay_birail1_getpropcb:
 *  get property (from C to Tcl context) callback function of birail1 object
 */
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

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_NewIntObj(birail1->type);
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
  to = Tcl_NewIntObj(birail1->display_mode);
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


/* ay_birail1_readcb:
 *  read (from scene file) callback function of birail1 object
 */
int
ay_birail1_readcb(FILE *fileptr, ay_object *o)
{
 ay_birail1_object *birail1 = NULL;

 if(!o)
   return AY_ENULL;

  if(!(birail1 = calloc(1, sizeof(ay_birail1_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&birail1->type);
  fscanf(fileptr,"%d\n",&birail1->sections);
  fscanf(fileptr,"%d\n",&birail1->has_start_cap);
  fscanf(fileptr,"%d\n",&birail1->has_end_cap);
  fscanf(fileptr,"%d\n",&birail1->display_mode);
  fscanf(fileptr,"%lg\n",&birail1->glu_sampling_tolerance);

  o->refine = birail1;

 return AY_OK;
} /* ay_birail1_readcb */


/* ay_birail1_writecb:
 *  write (to scene file) callback function of birail1 object
 */
int
ay_birail1_writecb(FILE *fileptr, ay_object *o)
{
 ay_birail1_object *birail1 = NULL;

  if(!o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)(o->refine);

  fprintf(fileptr, "%d\n", birail1->type);
  fprintf(fileptr, "%d\n", birail1->sections);
  fprintf(fileptr, "%d\n", birail1->has_start_cap);
  fprintf(fileptr, "%d\n", birail1->has_end_cap);
  fprintf(fileptr, "%d\n", birail1->display_mode);
  fprintf(fileptr, "%g\n", birail1->glu_sampling_tolerance);

 return AY_OK;
} /* ay_birail1_writecb */


/* ay_birail1_wribcb:
 *  RIB export callback function of birail1 object
 */
int
ay_birail1_wribcb(char *file, ay_object *o)
{
 ay_birail1_object *birail1;
 ay_object *b;

  if(!o)
   return AY_ENULL;

  birail1 = (ay_birail1_object*)o->refine;

  if(birail1->npatch)
    ay_wrib_toolobject(file, birail1->npatch, o);

  if(birail1->caps_and_bevels)
    {
      b = birail1->caps_and_bevels;
      while(b)
	{
	  ay_wrib_toolobject(file, b, o);
	  b = b->next;
	}
    }

 return AY_OK;
} /* ay_birail1_wribcb */


/* ay_birail1_bbccb:
 *  bounding box calculation callback function of birail1 object
 */
int
ay_birail1_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_birail1_object *birail1 = NULL;

  if(!o || !bbox || !flags)
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


/* ay_birail1_notifycb:
 *  notification callback function of birail1 object
 */
int
ay_birail1_notifycb(ay_object *o)
{
 ay_birail1_object *birail1 = NULL;
 int caps[4] = {0};
 ay_object *curve1 = NULL, *curve2 = NULL, *pobject1 = NULL, *pobject2 = NULL;
 ay_object *curve3 = NULL, *pobject3 = NULL;
 ay_object *npatch = NULL, **nextcb, *start_cap = NULL, *end_cap = NULL;
 ay_object *bevel = NULL;
 ay_bparam bparams;
 int ay_status = AY_OK;
 int is_provided[3] = {0}, mode = 0;
 double tolerance;

  if(!o)
    return AY_ENULL;

  birail1 = (ay_birail1_object *)(o->refine);

  mode = birail1->display_mode;
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
	  is_provided[0] = AY_TRUE;
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
	  is_provided[1] = AY_TRUE;
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
	  is_provided[2] = AY_TRUE;
	} /* if */
    } /* if */

  /* get bevel parameters */
  memset(&bparams, 0, sizeof(ay_bparam));
  if(o->tags)
    {
      ay_bevelt_parsetags(o->tags, &bparams);
    }

  /* do the birail */
  ay_status = ay_npt_createnpatchobject(&npatch);
  if(ay_status)
    {
      goto cleanup;
    }

  if(birail1->type < 2)
    {
      ay_status = ay_npt_birail1(curve1, curve2, curve3,
			   birail1->sections, birail1->type,
			   (ay_nurbpatch_object **)(void*)&(npatch->refine),
			   bparams.states[0]?AY_FALSE:birail1->has_start_cap,
			   &start_cap,
			   bparams.states[1]?AY_FALSE:birail1->has_end_cap,
			   &end_cap);
    }
  else
    {
      ay_status = ay_npt_birail1periodic(curve1, curve2, curve3,
					 birail1->sections,
			   (ay_nurbpatch_object **)(void*)&(npatch->refine));
    }

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

  /* prevent cleanup code from doing something harmful */
  npatch = NULL;

  /* create/add bevels */
  if((birail1->type < 2) && o->tags)
    {
      caps[0] = birail1->has_start_cap;
      caps[1] = birail1->has_end_cap;

      bparams.dirs[0] = !bparams.dirs[0];
      bparams.extrncsides[0] = 2;
      bparams.extrncsides[1] = 3;

      ay_status = ay_bevelt_addbevels(&bparams, caps, birail1->npatch,
				      nextcb);
    }

  /* copy sampling tolerance/mode attributes over to birail */
  ((ay_nurbpatch_object *)birail1->npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)birail1->npatch->refine)->display_mode =
    mode;

  /* copy sampling tolerance/mode attributes to caps and bevels */
  if(birail1->caps_and_bevels)
    {
      bevel = birail1->caps_and_bevels;
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
  /* remove provided objects */
  if(is_provided[0])
    {
      ay_object_deletemulti(pobject1);
    }

  if(is_provided[1])
    {
      ay_object_deletemulti(pobject2);
    }

  if(is_provided[2])
    {
      ay_object_deletemulti(pobject3);
    }

  if(npatch)
    {
      ay_object_delete(npatch);
    }

  /* recover selected points */
  if(o->selp)
    {
      ay_birail1_getpntcb(3, o, NULL, NULL);
    }

 return ay_status;
} /* ay_birail1_notifycb */


/* ay_birail1_providecb:
 *  provide callback function of birail1 object
 */
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

      if(!s->npatch)
	return AY_ERROR;

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

	  ay_npt_applytrafo(*t);
	  ay_trafo_copy(o, *t);

	  t = &((*t)->next);
	  p = p->next;
	} /* while */

      /* copy eventually present TP tags */
      ay_npt_copytptag(o, new);

      *result = new;
    } /* if */

 return ay_status;
} /* ay_birail1_providecb */


/* ay_birail1_convertcb:
 *  convert callback function of birail1 object
 */
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
	      /* reset display mode and sampling tolerance
		 of new patch to "global"? */
	      if(!in_place && ay_prefs.conv_reset_display)
		{
		  ay_npt_resetdisplay(*next);
		}

	      (*next)->parent = AY_TRUE;
	      (*next)->down = ay_endlevel;
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
		  /* reset display mode and sampling tolerance
		     of new patch to "global"? */
		  if(!in_place && ay_prefs.conv_reset_display)
		    {
		      ay_npt_resetdisplay(*next);
		    }

		  next = &((*next)->next);
		}
	      b = b->next;
	    } /* while */
	} /* if */

      /* copy eventually present TP tags */
      ay_npt_copytptag(o, new->down);
      *next = ay_endlevel;
    }
  else
    {
       if(birail1->npatch)
	{
	  ay_status = ay_object_copy(birail1->npatch, &new);
	  if(new)
	    {
	      /* reset display mode and sampling tolerance
		 of new patch to "global"? */
	      if(!in_place && ay_prefs.conv_reset_display)
		{
		  ay_npt_resetdisplay(new);
		}

	      ay_trafo_copy(o, new);

	      /* copy eventually present TP tags */
	      ay_npt_copytptag(o, new);
	    } /* if */
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
} /* ay_birail1_convertcb */


/* ay_birail1_init:
 *  initialize the birail1 object module
 */
int
ay_birail1_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_birail1_name,
				    ay_birail1_createcb,
				    ay_birail1_deletecb,
				    ay_birail1_copycb,
				    ay_birail1_drawcb,
				    ay_birail1_drawhcb,
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

