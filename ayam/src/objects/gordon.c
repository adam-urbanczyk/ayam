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

/* gordon.c - gordon surface object */

static char *ay_gordon_name = "Gordon";

int
ay_gordon_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtgordon";
 ay_gordon_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_gordon_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  new->wcc = AY_TRUE;
  new->uorder = 4;
  new->vorder = 4;

  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_gordon_createcb */


int
ay_gordon_deletecb(void *c)
{
 ay_gordon_object *gordon = NULL;

  if(!c)
    return AY_ENULL;    

  gordon = (ay_gordon_object *)(c);

  if(gordon->npatch)
    ay_object_delete(gordon->npatch);

  free(gordon);

 return AY_OK;
} /* ay_gordon_deletecb */


int
ay_gordon_copycb(void *src, void **dst)
{
 ay_gordon_object *gordon = NULL, *gordonsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  gordonsrc = (ay_gordon_object *)src;

  if(!(gordon = calloc(1, sizeof(ay_gordon_object))))
    return AY_EOMEM; 

  memcpy(gordon, src, sizeof(ay_gordon_object)); 


  /* copy npatch */
  ay_object_copy(gordonsrc->npatch, &(gordon->npatch));

  *dst = (void *)gordon;

 return AY_OK;
} /* ay_gordon_copycb */


int
ay_gordon_drawcb(struct Togl *togl, ay_object *o)
{
 ay_gordon_object *gordon = NULL;

  if(!o)
    return AY_ENULL;

  gordon = (ay_gordon_object *)o->refine;

  if(!gordon)
    return AY_ENULL;

  if(gordon->npatch)
    ay_draw_object(togl, gordon->npatch, AY_TRUE);

 return AY_OK;
} /* ay_gordon_drawcb */


int
ay_gordon_shadecb(struct Togl *togl, ay_object *o)
{
 ay_gordon_object *gordon = NULL;

  if(!o)
    return AY_ENULL;

  gordon = (ay_gordon_object *)o->refine;

  if(!gordon)
    return AY_ENULL;

  if(gordon->npatch)
    ay_shade_object(togl, gordon->npatch, AY_FALSE);

 return AY_OK;
} /* ay_gordon_shadecb */


int
ay_gordon_drawhcb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_gordon_drawhcb */


int
ay_gordon_getpntcb(int mode, ay_object *o, double *p)
{

 return AY_OK;
} /* ay_gordon_getpntcb */


/* Tcl -> C! */
int
ay_gordon_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "GordonAttrData";
 /* char fname[] = "gordon_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_gordon_object *gordon = NULL;


  if(!o)
    return AY_ENULL;

  gordon = (ay_gordon_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"WatchCurves",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(gordon->wcc));

  Tcl_SetStringObj(ton,"Order_U",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(gordon->uorder));

  Tcl_SetStringObj(ton,"Order_V",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(gordon->vorder));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(gordon->glu_display_mode));

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(gordon->glu_sampling_tolerance));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_force(o);

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_gordon_setpropcb */


/* C -> Tcl! */
int
ay_gordon_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="GordonAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_gordon_object *gordon = NULL;

  if(!o)
    return AY_ENULL;

  gordon = (ay_gordon_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"WatchCurves",-1);
  to = Tcl_NewIntObj(gordon->wcc);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order_U",-1);
  to = Tcl_NewIntObj(gordon->uorder);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Order_V",-1);
  to = Tcl_NewIntObj(gordon->vorder);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(gordon->glu_display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(gordon->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_gordon_getpropcb */


int
ay_gordon_readcb(FILE *fileptr, ay_object *o)
{
 ay_gordon_object *gordon = NULL;

 if(!o)
   return AY_ENULL;

  if(!(gordon = calloc(1, sizeof(ay_gordon_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&gordon->wcc);
  fscanf(fileptr,"%d\n",&gordon->uorder);
  fscanf(fileptr,"%d\n",&gordon->vorder);
  fscanf(fileptr,"%d\n",&gordon->glu_display_mode);
  fscanf(fileptr,"%lg\n",&gordon->glu_sampling_tolerance);

  o->refine = gordon;

 return AY_OK;
} /* ay_gordon_readcb */


int
ay_gordon_writecb(FILE *fileptr, ay_object *o)
{
 ay_gordon_object *gordon = NULL;

  if(!o)
    return AY_ENULL;

  gordon = (ay_gordon_object *)(o->refine);

  fprintf(fileptr, "%d\n", gordon->wcc);
  fprintf(fileptr, "%d\n", gordon->uorder);
  fprintf(fileptr, "%d\n", gordon->vorder);
  fprintf(fileptr, "%d\n", gordon->glu_display_mode);
  fprintf(fileptr, "%g\n", gordon->glu_sampling_tolerance);

 return AY_OK;
} /* ay_gordon_writecb */


int
ay_gordon_wribcb(char *file, ay_object *o)
{
 ay_gordon_object *gordon = NULL;

  if(!o)
   return AY_ENULL;

  gordon = (ay_gordon_object*)o->refine;

  if(gordon->npatch)
    ay_wrib_object(file, gordon->npatch);

 return AY_OK;
} /* ay_gordon_wribcb */


int
ay_gordon_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_gordon_object *gordon = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  gordon = (ay_gordon_object *)o->refine; 

  if(gordon->npatch)
    {
      ay_bbc_get(gordon->npatch, bbox);
      *flags = 1;
    }
  else
    {
      /* invalid/nonexisting bbox */
      *flags = 2;
    }

 return AY_OK;
} /* ay_gordon_bbccb */


int
ay_gordon_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *curve = NULL;
 ay_nurbpatch_object *patch = NULL;
 ay_gordon_object *gordon = NULL;
 ay_object *down = NULL, *c = NULL, *p = NULL, *last = NULL;
 ay_object *hcurves = NULL, *vcurves = NULL, *inpatch = NULL;
 ay_object *npatch = NULL;
 int getvcurves = AY_FALSE, getinpatch = AY_FALSE, hcount = 0, vcount = 0;
 int mode = 0, a, i;
 double tolerance, m[16] = {0};

  if(!o)
    return AY_ENULL;    

  gordon = (ay_gordon_object *)(o->refine);

  mode = gordon->glu_display_mode;
  tolerance = gordon->glu_sampling_tolerance;

  /* remove old objects */
  if(gordon->npatch)
    ay_object_delete(gordon->npatch);
  gordon->npatch = NULL;

  /* get parameter curves */
  if(!o->down)
    return AY_OK;

  if(gordon->wcc)
    ay_status = ay_npt_gordonwc(o);

  down = o->down;

  while(down->next)
    {
      c = NULL;
      if(down->type == AY_IDLEVEL)
	{
	  if(!getvcurves)
	    {
	      getvcurves = AY_TRUE;
	    }
	  else
	    {
	      getinpatch = AY_TRUE; 
	    }
	  last = NULL;
	}
      if(!getinpatch)
	{
	  if(down->type == AY_IDNCURVE)
	    {
	      ay_object_copy(down, &c);
	    }
	  else
	    {
	      ay_status = ay_provide_object(down, AY_IDNCURVE, &c);
	    } /* if */

	  if(c)
	    {
	      /* immediately apply transformation attributes to
		 control points */
	      if((c->movx != 0.0) || (c->movy != 0.0) || (c->movz != 0.0) ||
		 (c->rotx != 0.0) || (c->roty != 0.0) || (c->rotz != 0.0) ||
		 (c->scalx != 1.0) || (c->scaly != 1.0) || (c->scalz != 1.0) ||
		 (c->quat[0] != 0.0) || (c->quat[1] != 0.0) ||
		 (c->quat[2] != 0.0) || (c->quat[3] != 1.0))
		{
		  ay_trafo_creatematrix(c, m);
		  curve = (ay_nurbcurve_object *)c->refine;
		  a = 0;
		  for(i = 0; i < curve->length; i++)
		    {
		      ay_trafo_apply4(&(curve->controlv[a]), m);
		      a += 4;
		    }
		} /* if */

	      /* link curve to list */
	      if(getvcurves)
		{
		  hcount++;
		  if(last)
		    last->next = c;
		  else
		    vcurves = c;
		}
	      else
		{
		  vcount++;
		  if(last)
		    last->next = c;
		  else
		    hcurves = c;
		}

	      last = c;
	    } /* if */
	}
      else
	{
	  if(!inpatch)
	    {
	      if(down->type == AY_IDNPATCH)
		{
		  ay_object_copy(down, &inpatch);
		}
	      else
		{
		  ay_status = ay_provide_object(down, AY_IDNPATCH, &inpatch);
		} /* if */

	      if(inpatch)
		{
		  p = inpatch;
		  if((p->movx != 0.0) || (p->movy != 0.0) ||
		     (p->movz != 0.0) ||
		     (p->rotx != 0.0) || (p->roty != 0.0) ||
		     (p->rotz != 0.0) ||
		     (p->scalx != 1.0) || (p->scaly != 1.0) ||
		     (p->scalz != 1.0) ||
		     (p->quat[0] != 0.0) || (p->quat[1] != 0.0) ||
		     (p->quat[2] != 0.0) || (p->quat[3] != 1.0))
		    {
		      ay_trafo_creatematrix(c, m);
		      patch = (ay_nurbpatch_object *)p->refine;
		      a = 0;
		      for(i = 0; i < patch->width*patch->height; i++)
			{
			  ay_trafo_apply4(&(patch->controlv[a]), m);
			  a += 4;
			}
		    } /* if */
		} /* if */
	    } /* if */
	} /* if */

      down = down->next;
    } /* while */

  /* enough curves to process? */
  if((hcount < 2) || (vcount < 2))
    {
      ay_status = AY_OK;
      goto cleanup;
    }

  /* create Gordon surface */
  if(!(npatch = calloc(1, sizeof(ay_object))))
    {
      ay_status =  AY_EOMEM;
      goto cleanup;
    }

  ay_object_defaults(npatch);
  npatch->type = AY_IDNPATCH;

  ay_status = ay_npt_gordon(hcurves, vcurves, inpatch,
			    gordon->uorder, gordon->vorder,
			    (ay_nurbpatch_object **)(&(npatch->refine)));

  if(ay_status)
    {
      free(npatch);
      goto cleanup;
    }

  /* copy sampling tolerance/mode over to new object */
  gordon->npatch = npatch;

  ((ay_nurbpatch_object *)npatch->refine)->glu_sampling_tolerance =
    tolerance;
  ((ay_nurbpatch_object *)npatch->refine)->glu_display_mode =
    mode;

cleanup:
  /* remove temporary curves and intersection patch */
  while(hcurves)
    {
      c = hcurves->next;
      ay_object_delete(hcurves);
      hcurves = c;
    }

  while(vcurves)
    {
      c = vcurves->next;
      ay_object_delete(vcurves);
      vcurves = c;
    }

  if(inpatch)
    ay_object_delete(inpatch);

 return ay_status;
} /* ay_gordon_notifycb */


int
ay_gordon_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "gordon_providecb";
 ay_gordon_object *s = NULL;
 ay_object *new = NULL, **t = NULL;

  if(!o || !result)
    return AY_ENULL;

  s = (ay_gordon_object *) o->refine;

  if(type == AY_IDNPATCH)
    {
      t = &(new);

      /* copy gordon */
      ay_status = ay_object_copy(s->npatch, t);
      if(ay_status)
	{
	  ay_error(ay_status, fname, NULL);
	  return AY_ERROR;
	}
      ay_trafo_copy(o, *t);
      t = &((*t)->next);

      *result = new;
    } /* if */

 return ay_status;
} /* ay_gordon_providecb */


int
ay_gordon_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_gordon_object *r = NULL;
 ay_object *new = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */

  r = (ay_gordon_object *) o->refine;
  if(r->npatch)
    {
      ay_status = ay_object_copy(r->npatch, &new);
      ay_trafo_copy(o, new);
      new->hide_children = AY_TRUE;
      new->parent = AY_TRUE;
      ay_object_crtendlevel(&(new->down));
    }

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
} /* ay_gordon_convertcb */


int
ay_gordon_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_gordon_name,
				    ay_gordon_createcb,
				    ay_gordon_deletecb,
				    ay_gordon_copycb,
				    ay_gordon_drawcb,
				    NULL, /* no handles */
				    ay_gordon_shadecb,
				    ay_gordon_setpropcb,
				    ay_gordon_getpropcb,
				    ay_gordon_getpntcb,
				    ay_gordon_readcb,
				    ay_gordon_writecb,
				    ay_gordon_wribcb,
				    ay_gordon_bbccb,
				    AY_IDGORDON);

  
  ay_status = ay_notify_register(ay_gordon_notifycb, AY_IDGORDON);

  ay_status = ay_convert_register(ay_gordon_convertcb, AY_IDGORDON);

  ay_status = ay_provide_register(ay_gordon_providecb, AY_IDGORDON);

 return ay_status;
} /* ay_gordon_init */

