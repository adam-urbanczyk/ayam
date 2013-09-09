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

/* extrnp.c - ExtrNP (extract NURBS patch) object */

static char *ay_extrnp_name = "ExtrNP";

int ay_extrnp_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

/* functions: */

/* ay_extrnp_createcb:
 *  create callback function of extrnp object
 */
int
ay_extrnp_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtextrnp";
 ay_extrnp_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_extrnp_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->umax = 1.0;
  new->vmax = 1.0;
  new->pnum = 1;

  o->hide_children = AY_TRUE;
  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_extrnp_createcb */


/* ay_extrnp_deletecb:
 *  delete callback function of extrnp object
 */
int
ay_extrnp_deletecb(void *c)
{
 ay_extrnp_object *extrnp = NULL;

  if(!c)
    return AY_ENULL;

  extrnp = (ay_extrnp_object *)(c);

  if(extrnp->npatch)
    (void)ay_object_delete(extrnp->npatch);

  free(extrnp);

 return AY_OK;
} /* ay_extrnp_deletecb */


/* ay_extrnp_copycb:
 *  copy callback function of extrnp object
 */
int
ay_extrnp_copycb(void *src, void **dst)
{
 ay_extrnp_object *extrnp = NULL, *extrnpsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  extrnpsrc = (ay_extrnp_object *)src;

  if(!(extrnp = malloc(sizeof(ay_extrnp_object))))
    return AY_EOMEM;

  memcpy(extrnp, src, sizeof(ay_extrnp_object));

  extrnp->npatch = NULL;

  *dst = (void *)extrnp;

 return AY_OK;
} /* ay_extrnp_copycb */


/* ay_extrnp_drawcb:
 *  draw (display in an Ayam view window) callback function of extrnp object
 */
int
ay_extrnp_drawcb(struct Togl *togl, ay_object *o)
{
 ay_extrnp_object *extrnp = NULL;

  if(!o)
    return AY_ENULL;

  extrnp = (ay_extrnp_object *)o->refine;

  if(!extrnp)
    return AY_ENULL;

  if(extrnp->npatch)
    ay_draw_object(togl, extrnp->npatch, AY_TRUE);

 return AY_OK;
} /* ay_extrnp_drawcb */


/* ay_extrnp_shadecb:
 *  shade (display in an Ayam view window) callback function of extrnp object
 */
int
ay_extrnp_shadecb(struct Togl *togl, ay_object *o)
{
 ay_extrnp_object *extrnp = NULL;

  if(!o)
    return AY_ENULL;

  extrnp = (ay_extrnp_object *)o->refine;

  if(!extrnp)
    return AY_ENULL;

  if(extrnp->npatch)
    ay_shade_object(togl, extrnp->npatch, AY_FALSE);

 return AY_OK;
} /* ay_extrnp_shadecb */


/* ay_extrnp_drawacb:
 *  draw annotations (in an Ayam view window) callback function of extrnp object
 */
int
ay_extrnp_drawacb(struct Togl *togl, ay_object *o)
{
 ay_extrnp_object *extrnp = NULL;
 ay_nurbpatch_object *np = NULL;

  if(!o)
    return AY_ENULL;

  extrnp = (ay_extrnp_object *) o->refine;

  if(extrnp->npatch)
    {
      np = (ay_nurbpatch_object *)extrnp->npatch->refine;

      /* draw arrow */
      ay_draw_arrow(togl, &(np->controlv[np->width*np->height*4-8]),
		    &(np->controlv[np->width*np->height*4-4]));
    }

  return AY_OK;
} /* ay_extrnp_drawacb */


/* ay_extrnp_drawhcb:
 *  draw handles (in an Ayam view window) callback function of extrnp object
 */
int
ay_extrnp_drawhcb(struct Togl *togl, ay_object *o)
{
 int i;
 double *pnts;
 ay_extrnp_object *extrnp;
 ay_nurbpatch_object *np;

  if(!o)
    return AY_ENULL;

  extrnp = (ay_extrnp_object *) o->refine;

  if(extrnp->npatch)
    {
      np = (ay_nurbpatch_object *)extrnp->npatch->refine;

      /* draw read only points */
      pnts = np->controlv;

      glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
		(GLfloat)ay_prefs.obb);

      glBegin(GL_POINTS);
       for(i = 0; i < np->width*np->height; i++)
	 {
	   glVertex3dv((GLdouble *)pnts);
	   pnts += 4;
	 }
      glEnd();

      glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
		(GLfloat)ay_prefs.seb);
    }

 return AY_OK;
} /* ay_extrnp_drawhcb */


/* ay_extrnp_getpntcb:
 *  get point (editing and selection) callback function of extrnp object
 */
int
ay_extrnp_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_nurbpatch_object *patch = NULL;
 ay_extrnp_object *extrnp = NULL;

  if(!o)
    return AY_ENULL;

  extrnp = (ay_extrnp_object *)o->refine;

  if(extrnp->npatch)
    {
      patch = (ay_nurbpatch_object *)extrnp->npatch->refine;
      return ay_selp_getpnts(mode, o, p, pe, 1, patch->width*patch->height, 4,
			     patch->controlv);
    }

 return AY_ERROR;
} /* ay_extrnp_getpntcb */


/* ay_extrnp_setpropcb:
 *  set property (from Tcl to C context) callback function of extrnp object
 */
int
ay_extrnp_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "ExtrNPAttrData";
 /* char fname[] = "extrnp_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_extrnp_object *extrnp = NULL;

  if(!o)
    return AY_ENULL;

  extrnp = (ay_extrnp_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"UMin",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(extrnp->umin));

  Tcl_SetStringObj(ton,"UMax",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(extrnp->umax));

  Tcl_SetStringObj(ton,"VMin",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(extrnp->vmin));

  Tcl_SetStringObj(ton,"VMax",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(extrnp->vmax));

  Tcl_SetStringObj(ton,"PatchNum",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(extrnp->pnum));

  Tcl_SetStringObj(ton,"Relative",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(extrnp->relative));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(extrnp->display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(extrnp->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_notify_object(o);

  o->modified = AY_TRUE;
  ay_notify_parent();

 return AY_OK;
} /* ay_extrnp_setpropcb */


/* ay_extrnp_getpropcb:
 *  get property (from C to Tcl context) callback function of extrnp object
 */
int
ay_extrnp_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="ExtrNPAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_extrnp_object *extrnp = NULL;

  if(!o)
    return AY_ENULL;

  extrnp = (ay_extrnp_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"UMin",-1);
  to = Tcl_NewDoubleObj(extrnp->umin);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"UMax",-1);
  to = Tcl_NewDoubleObj(extrnp->umax);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"VMin",-1);
  to = Tcl_NewDoubleObj(extrnp->vmin);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"VMax",-1);
  to = Tcl_NewDoubleObj(extrnp->vmax);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Relative",-1);
  to = Tcl_NewIntObj(extrnp->relative);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"PatchNum",-1);
  to = Tcl_NewIntObj(extrnp->pnum);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(extrnp->display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(extrnp->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  ay_prop_getnpinfo(interp, n1, extrnp->npatch);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_extrnp_getpropcb */


/* ay_extrnp_readcb:
 *  read (from scene file) callback function of extrnp object
 */
int
ay_extrnp_readcb(FILE *fileptr, ay_object *o)
{
 ay_extrnp_object *extrnp = NULL;

 if(!o)
   return AY_ENULL;

  if(!(extrnp = calloc(1, sizeof(ay_extrnp_object))))
    { return AY_EOMEM; }

  fscanf(fileptr, "%d\n", &extrnp->pnum);
  fscanf(fileptr, "%lg\n", &extrnp->umin);
  fscanf(fileptr, "%lg\n", &extrnp->umax);
  fscanf(fileptr, "%lg\n", &extrnp->vmin);
  fscanf(fileptr, "%lg\n", &extrnp->vmax);

  fscanf(fileptr, "%d\n", &extrnp->display_mode);
  fscanf(fileptr, "%lg\n", &extrnp->glu_sampling_tolerance);

  if(ay_read_version >= 12)
    {
      /* since 1.15 */
      fscanf(fileptr, "%d\n", &extrnp->relative);
    }

  o->refine = extrnp;

 return AY_OK;
} /* ay_extrnp_readcb */


/* ay_extrnp_writecb:
 *  write (to scene file) callback function of extrnp object
 */
int
ay_extrnp_writecb(FILE *fileptr, ay_object *o)
{
 ay_extrnp_object *extrnp = NULL;

  if(!o)
    return AY_ENULL;

  extrnp = (ay_extrnp_object *)(o->refine);

  fprintf(fileptr, "%d\n", extrnp->pnum);
  fprintf(fileptr, "%g\n", extrnp->umin);
  fprintf(fileptr, "%g\n", extrnp->umax);
  fprintf(fileptr, "%g\n", extrnp->vmin);
  fprintf(fileptr, "%g\n", extrnp->vmax);
  fprintf(fileptr, "%d\n", extrnp->display_mode);
  fprintf(fileptr, "%g\n", extrnp->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", extrnp->relative);

 return AY_OK;
} /* ay_extrnp_writecb */


/* ay_extrnp_wribcb:
 *  RIB export callback function of extrnp object
 */
int
ay_extrnp_wribcb(char *file, ay_object *o)
{
 ay_extrnp_object *extrnp = NULL;

  if(!o)
   return AY_ENULL;

  extrnp = (ay_extrnp_object*)o->refine;

  if(extrnp->npatch)
    ay_wrib_toolobject(file, extrnp->npatch, o);

 return AY_OK;
} /* ay_extrnp_wribcb */


/* ay_extrnp_bbccb:
 *  bounding box calculation callback function of extrnp object
 */
int
ay_extrnp_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_extrnp_object *extrnp = NULL;

  if(!o || !bbox || !flags)
    return AY_ENULL;

  extrnp = (ay_extrnp_object *)o->refine;

  if(extrnp->npatch)
    {
      ay_bbc_get(extrnp->npatch, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    } /* if */

 return AY_OK;
} /* ay_extrnp_bbccb */


/* ay_extrnp_notifycb:
 *  notification callback function of extrnp object
 */
int
ay_extrnp_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_extrnp_object *extrnp = NULL;
 ay_object *n = NULL, *pobject = NULL;
 ay_object *npatch = NULL;
 int mode, pnum, provided = AY_FALSE;
 double tolerance;

  if(!o)
    return AY_ENULL;

  extrnp = (ay_extrnp_object *)(o->refine);

  pnum = extrnp->pnum - 1;
  mode = extrnp->display_mode;
  tolerance = extrnp->glu_sampling_tolerance;

  /* remove old objects */
  if(extrnp->npatch)
    (void)ay_object_delete(extrnp->npatch);
  extrnp->npatch = NULL;

  /* get patch to extract the npatch from */
  if(!o->down)
    goto cleanup;
  if(!o->down->next)
    goto cleanup;

  n = o->down;
  if(n->type != AY_IDNPATCH)
    {
      ay_status = ay_provide_object(n, AY_IDNPATCH, &pobject);
      if(!pobject)
	{
	  goto cleanup;
	}
      else
	{
	  if(pnum > 0)
	    {
	      while(pobject && pnum)
		{
		  pobject = pobject->next;
		  pnum--;
		}
	      if(pobject)
		{
		  n = pobject;
		}
	      else
		{
		  /* not enough patches for pnum! */
		  ay_status = AY_ERROR;
		  goto cleanup;
		}
	    }
	  else
	    {
	      n = pobject;
	    }
	  provided = AY_TRUE;
	} /* if */
    } /* if */

  /* extract the patch */
  ay_status = ay_npt_extractnp(n, extrnp->umin, extrnp->umax,
			       extrnp->vmin, extrnp->vmax,
			       extrnp->relative, &npatch);

  if(ay_status || !npatch)
    {
      goto cleanup;
    }

  ay_trafo_defaults(npatch);

  extrnp->npatch = npatch;

  /* copy transformation attributes over to extrnp object
     (the extracted patch is always at the same place as
     the original surface) */
  ay_trafo_copy(n, o);

  /* copy sampling tolerance/mode over to new object */
  ((ay_nurbpatch_object *)npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)npatch->refine)->display_mode =
    mode;

cleanup:

  /* remove provided object(s) */
  if(provided)
    {
      (void)ay_object_deletemulti(pobject);
    }

  /* recover selected points */
  if(o->selp)
    {
      if(extrnp->npatch)
	ay_extrnp_getpntcb(3, o, NULL, NULL);
      else
	ay_selp_clear(o);
    }

 return ay_status;
} /* ay_extrnp_notifycb */


/* ay_extrnp_providecb:
 *  provide callback function of extrnp object
 */
int
ay_extrnp_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 ay_extrnp_object *e = NULL;

  if(!o)
    return AY_ENULL;

  e = (ay_extrnp_object *) o->refine;

 return ay_provide_nptoolobj(o, type, e->npatch, NULL, result);
} /* ay_extrnp_providecb */


/* ay_extrnp_convertcb:
 *  convert callback function of extrnp object
 */
int
ay_extrnp_convertcb(ay_object *o, int in_place)
{
 ay_extrnp_object *e = NULL;

  if(!o)
    return AY_ENULL;

  e = (ay_extrnp_object *) o->refine;

 return ay_convert_nptoolobj(o, e->npatch, NULL, in_place);
} /* ay_extrnp_convertcb */


/* ay_extrnp_init:
 *  initialize the extrnp object module
 */
int
ay_extrnp_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_extrnp_name,
				    ay_extrnp_createcb,
				    ay_extrnp_deletecb,
				    ay_extrnp_copycb,
				    ay_extrnp_drawcb,
				    ay_extrnp_drawhcb,
				    ay_extrnp_shadecb,
				    ay_extrnp_setpropcb,
				    ay_extrnp_getpropcb,
				    ay_extrnp_getpntcb,
				    ay_extrnp_readcb,
				    ay_extrnp_writecb,
				    ay_extrnp_wribcb,
				    ay_extrnp_bbccb,
				    AY_IDEXTRNP);

  ay_status += ay_draw_registerdacb(ay_extrnp_drawacb, AY_IDEXTRNP);

  ay_status += ay_notify_register(ay_extrnp_notifycb, AY_IDEXTRNP);

  ay_status += ay_convert_register(ay_extrnp_convertcb, AY_IDEXTRNP);

  ay_status += ay_provide_register(ay_extrnp_providecb, AY_IDEXTRNP);

 return ay_status;
} /* ay_extrnp_init */
