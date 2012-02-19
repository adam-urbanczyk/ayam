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

  new->knot_type = AY_KTNURB;
  new->ftlength = 1.0;

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

  ay_object_delete(concatnp->npatch);

  if(concatnp->uv_select)
    free(concatnp->uv_select);

  free(concatnp);

 return AY_OK;
} /* ay_concatnp_deletecb */


/* ay_concatnp_copycb:
 *  copy callback function of concatnp object
 */
int
ay_concatnp_copycb(void *src, void **dst)
{
 ay_concatnp_object *concatnp = NULL, *concatnpsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  concatnpsrc = (ay_concatnp_object *)src;

  if(!(concatnp = calloc(1, sizeof(ay_concatnp_object))))
    return AY_EOMEM;

  memcpy(concatnp, src, sizeof(ay_concatnp_object));

  if(concatnpsrc->uv_select)
    {
      if(!(concatnp->uv_select = calloc(strlen(concatnpsrc->uv_select)+1,
					sizeof(char))))
	{ free(concatnp); return AY_EOMEM; }

      strcpy(concatnp->uv_select, concatnpsrc->uv_select);
    }

  /* copy npatch */
  ay_object_copy(concatnpsrc->npatch, &(concatnp->npatch));

  *dst = (void *)concatnp;

 return AY_OK;
} /* ay_concatnp_copycb */


/* ay_concatnp_drawcb:
 *  draw (display in an Ayam view window) callback function of concatnp object
 */
int
ay_concatnp_drawcb(struct Togl *togl, ay_object *o)
{
 ay_concatnp_object *cc = NULL;

  if(!o)
    return AY_ENULL;

  cc = (ay_concatnp_object *)o->refine;

  if(cc->npatch)
    {
      ay_draw_object(togl, cc->npatch, AY_TRUE);
    }

 return AY_OK;
} /* ay_concatnp_drawcb */


/* ay_concatnp_shadecb:
 *  shade (display in an Ayam view window) callback function of concatnp object
 */
int
ay_concatnp_shadecb(struct Togl *togl, ay_object *o)
{
 ay_concatnp_object *cc = NULL;

  if(!o)
    return AY_ENULL;

  cc = (ay_concatnp_object *)o->refine;

  if(cc->npatch)
    {
      ay_shade_object(togl, cc->npatch, AY_FALSE);
    }

 return AY_OK;
} /* ay_concatnp_shadecb */


/* ay_concatnp_drawhcb:
 *  draw handles (in an Ayam view window) callback function of concatnp object
 */
int
ay_concatnp_drawhcb(struct Togl *togl, ay_object *o)
{
 int i = 0, a = 0;
 ay_concatnp_object *concatnp = NULL;
 double *pnts = NULL;
 double point_size = ay_prefs.handle_size;
 ay_nurbpatch_object *patch = NULL;

  if(!o)
    return AY_ENULL;

  concatnp = (ay_concatnp_object *) o->refine;

  if(concatnp->npatch)
    {
      patch = (ay_nurbpatch_object *)concatnp->npatch->refine;
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

  if(!o)
    return AY_ENULL;

  concatnp = (ay_concatnp_object *)o->refine;

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

  Tcl_SetStringObj(ton,"FillGaps",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(concatnp->fillgaps));

  Tcl_SetStringObj(ton,"FTLength",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(concatnp->ftlength));

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
      o->modified = AY_TRUE;
    }

  if(stringlen > 0)
    {
      if(!(concatnp->uv_select = calloc(stringlen+1, sizeof(char))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  goto cleanup;
	}
      strcpy(concatnp->uv_select, string);

      o->modified = AY_TRUE;
    }

cleanup:

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_notify_object(o);

  o->modified = AY_TRUE;
  ay_notify_parent();

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

  if(!o)
    return AY_ENULL;

  concatnp = (ay_concatnp_object *)(o->refine);

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
 int read = 0;
 ay_concatnp_object *concatnp = NULL;

  if(!o)
    return AY_ENULL;

  if(!(concatnp = calloc(1, sizeof(ay_concatnp_object))))
    { return AY_EOMEM; }

  concatnp->ftlength = 0.3;

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
      read = fgetc(fileptr);
      if(read == '\r')
	fgetc(fileptr);

      ay_status = ay_read_string(fileptr, &(concatnp->uv_select));
      if(ay_status)
	{
	  free(concatnp);
	  return ay_status;
	}
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

  if(!o)
    return AY_ENULL;

  concatnp = (ay_concatnp_object *)(o->refine);

  fprintf(fileptr, "%d\n", concatnp->type);
  fprintf(fileptr, "%d\n", concatnp->revert);
  fprintf(fileptr, "%d\n", concatnp->knot_type);

  fprintf(fileptr, "%d\n", concatnp->fillgaps);
  fprintf(fileptr, "%g\n", concatnp->ftlength);

  fprintf(fileptr, "%d\n", concatnp->order);
  if(concatnp->uv_select)
    fprintf(fileptr, "%s", concatnp->uv_select);

  fprintf(fileptr, "\n");


 return AY_OK;
} /* ay_concatnp_writecb */


/* ay_concatnp_wribcb:
 *  RIB export callback function of concatnp object
 */
int
ay_concatnp_wribcb(char *file, ay_object *o)
{
 ay_concatnp_object *concatnp = NULL;

  if(!o)
   return AY_ENULL;

  concatnp = (ay_concatnp_object*)o->refine;

  if(concatnp->npatch)
    ay_wrib_toolobject(file, concatnp->npatch, o);

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

  if(!o)
    return AY_ENULL;

  concatnp = (ay_concatnp_object *)(o->refine);
  if(concatnp->npatch)
    ay_object_delete(concatnp->npatch);
  concatnp->npatch = NULL;

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
      return AY_OK;
    }

  ay_status = ay_npt_concat(patches, concatnp->type, concatnp->order,
			    concatnp->knot_type,
			    concatnp->fillgaps, concatnp->ftlength,
			    concatnp->uv_select, &concatnp->npatch);

  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Failed to concat!");
    }
  else
    {
      np = (ay_nurbpatch_object*)concatnp->npatch->refine;

      np->glu_sampling_tolerance =
	((ay_nurbpatch_object*)patches->refine)->glu_sampling_tolerance;
      np->display_mode =
	((ay_nurbpatch_object*)patches->refine)->display_mode;

      if(concatnp->revert)
	{
	  ay_status = ay_npt_revertu(np);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, "Failed to revert!");
	    } /* if */
	} /* if */
    } /* if */

cleanup:

  /* free list of temporary curves */
  if(patches)
    ay_object_deletemulti(patches);

  /* recover selected points */
  if(o->selp)
    {
      ay_concatnp_getpntcb(3, o, NULL, NULL);
    }

 return AY_OK;
} /* ay_concatnp_notifycb */


/* ay_concatnp_convertcb:
 *  convert callback function of concatnp object
 */
int
ay_concatnp_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_concatnp_object *cc = NULL;
 ay_object *new = NULL;
 ay_nurbpatch_object *np = NULL;

  if(!o)
    return AY_ENULL;

  cc = (ay_concatnp_object *) o->refine;

  if(cc->npatch)
    {
      ay_status = ay_object_copy(cc->npatch, &new);
      if(new)
	{
	  /* reset display mode and sampling tolerance
	     of new patch to "global"? */
	  if(!in_place && ay_prefs.conv_reset_display)
	    {
	      ay_npt_resetdisplay(new);
	    }

	  /* immediately create and show the multiple points */
	  np = (ay_nurbpatch_object *)new->refine;
	  np->createmp = AY_TRUE;
	  ay_npt_recreatemp(np);

	  ay_trafo_copy(o, new);

	  if(!in_place)
	    {
	      ay_status = ay_object_link(new);
	    }
	  else
	    {
	      ay_status = ay_object_replace(new, o);
	    }
	}
    } /* if */

 return ay_status;
} /* ay_concatnp_convertcb */


/* ay_concatnp_providecb:
 *  provide callback function of concatnp object
 */
int
ay_concatnp_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char *fname = "concatnp_providecb";
 ay_concatnp_object *cc = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNPATCH)
	return AY_OK;
      else
	return AY_ERROR;
    }

  cc = (ay_concatnp_object *) o->refine;

  if(type == AY_IDNPATCH)
    {
      if(cc->npatch)
	{
	  ay_status = ay_object_copy(cc->npatch, result);
	  if(ay_status)
	    {
	      ay_error(ay_status, fname, NULL);
	      return AY_ERROR;
	    }
	  ay_trafo_copy(o, *result);
	}
      else
	{
	  return AY_ERROR;
	}
    }

 return ay_status;
} /* ay_concatnp_providecb */


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


  ay_status = ay_notify_register(ay_concatnp_notifycb, AY_IDCONCATNP);

  ay_status = ay_convert_register(ay_concatnp_convertcb, AY_IDCONCATNP);

  ay_status = ay_provide_register(ay_concatnp_providecb, AY_IDCONCATNP);

 return ay_status;
} /* ay_concatnp_init */

