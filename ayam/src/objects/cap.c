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

/* cap.c - cap object */

static char *ay_cap_name = "Cap";

int ay_cap_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_cap_createcb:
 *  create callback function of cap object
 */
int
ay_cap_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtcap";
 ay_cap_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_cap_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->fraction = 0.5;

  o->refine = new;
  o->parent = AY_TRUE;

 return AY_OK;
} /* ay_cap_createcb */


/* ay_cap_deletecb:
 *  delete callback function of cap object
 */
int
ay_cap_deletecb(void *c)
{
 ay_cap_object *cap = NULL;

  if(!c)
    return AY_ENULL;

  cap = (ay_cap_object *)(c);

  (void)ay_object_delete(cap->npatch);

  free(cap);

 return AY_OK;
} /* ay_cap_deletecb */


/* ay_cap_copycb:
 *  copy callback function of cap object
 */
int
ay_cap_copycb(void *src, void **dst)
{
 ay_cap_object *cap = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(cap = malloc(sizeof(ay_cap_object))))
    return AY_EOMEM;

  memcpy(cap, src, sizeof(ay_cap_object));

  cap->npatch = NULL;

  *dst = (void *)cap;

 return AY_OK;
} /* ay_cap_copycb */


/* ay_cap_drawcb:
 *  draw (display in an Ayam view window) callback function of cap object
 */
int
ay_cap_drawcb(struct Togl *togl, ay_object *o)
{
 ay_cap_object *cap = NULL;

  if(!o)
    return AY_ENULL;

  cap = (ay_cap_object *)o->refine;

  if(!cap)
    return AY_ENULL;

  if(cap->npatch)
    ay_draw_object(togl, cap->npatch, AY_TRUE);

 return AY_OK;
} /* ay_cap_drawcb */


/* ay_cap_shadecb:
 *  shade (display in an Ayam view window) callback function of cap object
 */
int
ay_cap_shadecb(struct Togl *togl, ay_object *o)
{
 ay_cap_object *cap = NULL;

  if(!o)
    return AY_ENULL;

  cap = (ay_cap_object *)o->refine;

  if(!cap)
    return AY_ENULL;

  if(cap->npatch)
    ay_shade_object(togl, cap->npatch, AY_FALSE);

 return AY_OK;
} /* ay_cap_shadecb */


/* ay_cap_drawhcb:
 *  draw handles (in an Ayam view window) callback function of cap object
 */
int
ay_cap_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_cap_object *cap = NULL;

  if(!o)
    return AY_ENULL;

  cap = (ay_cap_object *) o->refine;

  if(!cap)
    return AY_ENULL;

  if(cap->npatch)
    {
      ay_npt_drawrohandles((ay_nurbpatch_object *)cap->npatch);
    }

 return AY_OK;
} /* ay_cap_drawhcb */


/* ay_cap_getpntcb:
 *  get point (editing and selection) callback function of cap object
 */
int
ay_cap_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbpatch_object *patch = NULL;
 ay_cap_object *cap = NULL;

  if(!o)
    return AY_ENULL;

  cap = (ay_cap_object *)o->refine;

  if(!cap)
    return AY_ENULL;

  if(cap->npatch)
    {
      patch = (ay_nurbpatch_object *)cap->npatch->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, patch->width*patch->height, 4,
			     patch->controlv);
    }

 return AY_ERROR;
} /* ay_cap_getpntcb */


/* ay_cap_setpropcb:
 *  set property (from Tcl to C context) callback function of cap object
 */
int
ay_cap_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "CapAttrData";
 /* char fname[] = "cap_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_cap_object *cap = NULL;

  if(!interp || !o)
    return AY_ENULL;

  cap = (ay_cap_object *)o->refine;

  if(!cap)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(cap->type));

  Tcl_SetStringObj(ton,"Fraction",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(cap->fraction));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(cap->glu_sampling_tolerance));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(cap->display_mode));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  if(cap->npatch)
    {
      ((ay_nurbpatch_object *)cap->npatch->refine)->glu_sampling_tolerance =
	cap->glu_sampling_tolerance;
      ((ay_nurbpatch_object *)cap->npatch->refine)->display_mode =
	cap->display_mode;
    }

  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

 return AY_OK;
} /* ay_cap_setpropcb */


/* ay_cap_getpropcb:
 *  get property (from C to Tcl context) callback function of cap object
 */
int
ay_cap_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="CapAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_cap_object *cap = NULL;

  if(!interp || !o)
    return AY_ENULL;

  cap = (ay_cap_object *)(o->refine);

  if(!cap)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Type",-1);
  to = Tcl_NewIntObj(cap->type);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Fraction",-1);
  to = Tcl_NewDoubleObj(cap->fraction);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(cap->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(cap->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getnpinfo(interp, n1, cap->npatch);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_cap_getpropcb */


/* ay_cap_readcb:
 *  read (from scene file) callback function of cap object
 */
int
ay_cap_readcb(FILE *fileptr, ay_object *o)
{
 ay_cap_object *cap = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  if(!(cap = calloc(1, sizeof(ay_cap_object))))
    { return AY_EOMEM; }

  cap->fraction = 0.5;

  fscanf(fileptr, "%lg\n", &cap->glu_sampling_tolerance);
  fscanf(fileptr, "%d\n", &cap->display_mode);

  if(ay_read_version >= 10)
    {
      /* since 1.13 */
      fscanf(fileptr, "%d\n", &cap->type);
    }
  if(ay_read_version >= 16)
    {
      /* since 1.21 */
      fscanf(fileptr, "%lg\n", &cap->fraction);
    }
  o->refine = cap;

 return AY_OK;
} /* ay_cap_readcb */


/* ay_cap_writecb:
 *  write (to scene file) callback function of cap object
 */
int
ay_cap_writecb(FILE *fileptr, ay_object *o)
{
 ay_cap_object *cap = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  cap = (ay_cap_object *)(o->refine);

  if(!cap)
    return AY_ENULL;

  fprintf(fileptr, "%g\n", cap->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", cap->display_mode);
  fprintf(fileptr, "%d\n", cap->type);
  fprintf(fileptr, "%g\n", cap->fraction);

 return AY_OK;
} /* ay_cap_writecb */


/* ay_cap_wribcb:
 *  RIB export callback function of cap object
 */
int
ay_cap_wribcb(char *file, ay_object *o)
{
 ay_cap_object *cap = NULL;
 int ay_status = AY_OK;

  if(!o)
   return AY_ENULL;

  cap = (ay_cap_object*)o->refine;

  if(!cap)
    return AY_ENULL;

  if(cap->npatch)
    ay_status = ay_wrib_toolobject(file, cap->npatch, o);

 return ay_status;
} /* ay_cap_wribcb */


/* ay_cap_bbccb:
 *  bounding box calculation callback function of cap object
 */
int
ay_cap_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_cap_object *cap = NULL;
 int ay_status = AY_OK;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  cap = (ay_cap_object *)o->refine;

  if(!cap)
    return AY_ENULL;

  if(cap->npatch)
    {
      ay_status = ay_bbc_get(cap->npatch, bbox);
      /* exclusive bounding box (discard children bbox) */
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return ay_status;
} /* ay_cap_bbccb */


/* ay_cap_notifycb:
 *  notification callback function of cap object
 */
int
ay_cap_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_cap_object *cap = NULL;
 ay_object *down = NULL, *pobject = NULL;
 ay_object **nextcurve = NULL;
 ay_tag *tag = NULL;
 double v[3] = {0}, *mp = NULL;

  if(!o)
    return AY_ENULL;

  cap = (ay_cap_object *)(o->refine);

  if(!cap)
    return AY_ENULL;

  nextcurve = &(pobject);

  /* first, delete old cap */
  if(cap->npatch)
    (void)ay_object_delete(cap->npatch);
  cap->npatch = NULL;

  /* get parameter curve(s) */
  if(!o->down)
    goto cleanup;
  down = o->down;
  while(down->next)
    {
      if(down->type != AY_IDNCURVE)
	{
	  (void)ay_provide_object(down, AY_IDNCURVE, nextcurve);
	  if(*nextcurve)
	    {
	      ay_nct_applytrafo(*nextcurve);

	      nextcurve = &((*nextcurve)->next);
	    } /* if */
	}
      else
	{
	  ay_object_copy(down, nextcurve);

	  ay_nct_applytrafo(*nextcurve);

	  nextcurve = &((*nextcurve)->next);
	} /* if */

      down = down->next;
    } /* while */

  if(!pobject)
    {
      goto cleanup;
    } /* if */

  tag = o->tags;
  while(tag)
    {
      if(tag->type == ay_mp_tagtype)
	{
	  if(tag->val)
	    {
	      sscanf(tag->val, "%lg,%lg,%lg", &(v[0]), &(v[1]), &(v[2]));
	      mp = v;
	    }
	  break;
	}
      tag = tag->next;
    }

  /* the pobject points now to copies of the curves for which
     the cap should be created (and we may mess with it) */
  switch(cap->type)
    {
    case 0:
      ay_status = ay_capt_crttrimcap(pobject, &(cap->npatch));
      break;
    case 1:
      ay_status = ay_capt_crtgordoncap(pobject, &(cap->npatch));
      break;
    case 2:
      ay_status = ay_capt_crtsimplecap(pobject, 0, cap->fraction, mp,
				       &(cap->npatch));
      break;
    case 3:
      ay_status = ay_capt_crtsimplecap(pobject, 1, cap->fraction, mp,
				       &(cap->npatch));
      break;
    default:
      ay_status = AY_ERROR;
      break;
    } /* switch */

  if(ay_status)
    {
      (void)ay_object_deletemulti(pobject, AY_FALSE);
      goto cleanup;
    }

  if(cap->npatch)
    {
      ((ay_nurbpatch_object *)cap->npatch->refine)->glu_sampling_tolerance =
	cap->glu_sampling_tolerance;
      ((ay_nurbpatch_object *)cap->npatch->refine)->display_mode =
	cap->display_mode;
    }

cleanup:
  /* recover selected points */
  if(o->selp)
    {
      if(cap->npatch)
	ay_cap_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

 return AY_OK;
} /* ay_cap_notifycb */


/* ay_cap_convertcb:
 *  convert callback function of cap object
 */
int
ay_cap_convertcb(ay_object *o, int in_place)
{
 ay_cap_object *c = NULL;

  if(!o)
    return AY_ENULL;

  c = (ay_cap_object *) o->refine;

  if(!c)
    return AY_ENULL;

 return ay_convert_nptoolobj(o, c->npatch, NULL, in_place);
} /* ay_cap_convertcb */


/* ay_cap_providecb:
 *  provide callback function of cap object
 */
int
ay_cap_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 ay_cap_object *c = NULL;

  if(!o)
    return AY_ENULL;

  c = (ay_cap_object *) o->refine;

  if(!c)
    return AY_ENULL;

 return ay_provide_nptoolobj(o, type, c->npatch, NULL, result);
} /* ay_cap_providecb */


/* ay_cap_init:
 *  initialize the cap object module
 */
int
ay_cap_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_cap_name,
				    ay_cap_createcb,
				    ay_cap_deletecb,
				    ay_cap_copycb,
				    ay_cap_drawcb,
				    ay_cap_drawhcb,
				    ay_cap_shadecb,
				    ay_cap_setpropcb,
				    ay_cap_getpropcb,
				    ay_cap_getpntcb,
				    ay_cap_readcb,
				    ay_cap_writecb,
				    ay_cap_wribcb,
				    ay_cap_bbccb,
				    AY_IDCAP);


  ay_status += ay_notify_register(ay_cap_notifycb, AY_IDCAP);

  ay_status += ay_convert_register(ay_cap_convertcb, AY_IDCAP);

  ay_status += ay_provide_register(ay_cap_providecb, AY_IDCAP);

 return ay_status;
} /* ay_cap_init */
