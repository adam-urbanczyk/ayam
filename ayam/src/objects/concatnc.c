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

/* concatnc.c - concatnc object */

static char *ay_concatnc_name = "ConcatNC";

int
ay_concatnc_createcb(int argc, char *argv[], ay_object *o)
{
 char fname[] = "crtconcatnc";
 ay_concatnc_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_concatnc_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }
 
  o->parent = AY_TRUE;

  o->refine = new;

 return AY_OK;
} /* ay_concatnc_createcb */


int
ay_concatnc_deletecb(void *c)
{
 ay_concatnc_object *concatnc = NULL;

  if(!c)
    return AY_ENULL;    

  concatnc = (ay_concatnc_object *)(c);

  ay_object_delete(concatnc->ncurve);

  free(concatnc);

 return AY_OK;
} /* ay_concatnc_deletecb */


int
ay_concatnc_copycb(void *src, void **dst)
{
 ay_concatnc_object *concatnc = NULL, *concatncsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  concatncsrc = (ay_concatnc_object *)src;

  if(!(concatnc = calloc(1, sizeof(ay_concatnc_object))))
    return AY_EOMEM; 

  memcpy(concatnc, src, sizeof(ay_concatnc_object)); 

  /* copy ncurve */
  ay_object_copy(concatncsrc->ncurve, &(concatnc->ncurve));

  *dst = (void *)concatnc;

 return AY_OK;
} /* ay_concatnc_copycb */


int
ay_concatnc_drawcb(struct Togl *togl, ay_object *o)
{
 ay_concatnc_object *cc = NULL;

  if(!o)
    return AY_ENULL;

  cc = (ay_concatnc_object *)o->refine;

  if(cc->ncurve)
    {
      ay_draw_object(togl, cc->ncurve, AY_TRUE);
    }

 return AY_OK;
} /* ay_concatnc_drawcb */


int
ay_concatnc_shadecb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_concatnc_shadecb */


int
ay_concatnc_drawhcb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_concatnc_drawhcb */


int
ay_concatnc_getpntcb(int mode, ay_object *o, double *p)
{

 return AY_OK;
} /* ay_concatnc_getpntcb */


/* Tcl -> C! */
int
ay_concatnc_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "ConcatNCAttrData";
 /*char fname[] = "concatnc_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_concatnc_object *concatnc = NULL;

  if(!o)
    return AY_ENULL;

  concatnc = (ay_concatnc_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(concatnc->closed));

  Tcl_SetStringObj(ton,"FillGaps",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(concatnc->fillgaps));

  Tcl_SetStringObj(ton,"Revert",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(concatnc->revert));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_status = ay_notify_force(o);

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_concatnc_setpropcb */


/* C -> Tcl! */
int
ay_concatnc_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="ConcatNCAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_concatnc_object *concatnc = NULL;

  if(!o)
    return AY_ENULL;

  concatnc = (ay_concatnc_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);


  Tcl_SetStringObj(ton,"Closed",-1);
  to = Tcl_NewIntObj(concatnc->closed);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"FillGaps",-1);
  to = Tcl_NewIntObj(concatnc->fillgaps);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Revert",-1);
  to = Tcl_NewIntObj(concatnc->revert);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_concatnc_getpropcb */


int
ay_concatnc_readcb(FILE *fileptr, ay_object *o)
{
 ay_concatnc_object *concatnc = NULL;

 if(!o)
   return AY_ENULL;

  if(!(concatnc = calloc(1, sizeof(ay_concatnc_object))))
    { return AY_EOMEM; }

  fscanf(fileptr, "%d\n", &concatnc->closed);
  fscanf(fileptr, "%d\n", &concatnc->fillgaps);
  fscanf(fileptr, "%d\n", &concatnc->revert);

  o->refine = concatnc;

 return AY_OK;
} /* ay_concatnc_readcb */


int
ay_concatnc_writecb(FILE *fileptr, ay_object *o)
{
 ay_concatnc_object *concatnc = NULL;

  if(!o)
    return AY_ENULL;

  concatnc = (ay_concatnc_object *)(o->refine);

  fprintf(fileptr, "%d\n", concatnc->closed);
  fprintf(fileptr, "%d\n", concatnc->fillgaps);
  fprintf(fileptr, "%d\n", concatnc->revert);

 return AY_OK;
} /* ay_concatnc_writecb */


int
ay_concatnc_wribcb(char *file, ay_object *o)
{
 ay_concatnc_object *concatnc = NULL;

  if(!o)
   return AY_ENULL;

  concatnc = (ay_concatnc_object*)o->refine;

 return AY_OK;
} /* ay_concatnc_wribcb */


int
ay_concatnc_bbccb(ay_object *o, double *bbox, int *flags)
{

  if(!o || !flags)
    return AY_ENULL;

  /* use the bounding boxes of the childs */
  *flags = 2;

 return AY_OK;
} /* ay_concatnc_bbccb */


int
ay_concatnc_notifycb(ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "concatnc_notifycb";
 ay_concatnc_object *concatnc = NULL;
 ay_object *down = NULL, *ncurve = NULL, *curves = NULL, **next = NULL;
 ay_nurbcurve_object *nc = NULL;
 int numcurves = 0;

  if(!o)
    return AY_ENULL;    

  concatnc = (ay_concatnc_object *)(o->refine);
  if(concatnc->ncurve)
    ay_object_delete(concatnc->ncurve);
  concatnc->ncurve = NULL;

  /* get child objects */
  down = o->down;
  next = &curves;
  while(down)
    {
      /* */
      if(down->type == AY_IDNCURVE)
	{
	  ay_object_copy(down, next);
	  next = &((*next)->next);
	}
      else
	{
	  ay_provide_object(down, AY_IDNCURVE, next);
	  if(*next)
	    next = &((*next)->next);
	}


      down = down->next;
    } /* while */

  ncurve = curves;
  while(ncurve)
    {
      ay_nct_applytrafo(ncurve);
      ay_nct_clamp((ay_nurbcurve_object *)ncurve->refine);
      ncurve = ncurve->next;
    }

  if(concatnc->fillgaps)
    {
      ay_status = ay_nct_fillgaps(concatnc->closed, curves);
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Failed to create fillets!");
	}
    }

  ncurve = curves;
  while(ncurve)
    {
      numcurves++;
      ncurve = ncurve->next;
    }

  if(numcurves > 1)
    {
      
      ay_status = ay_nct_concatmultiple(curves, &concatnc->ncurve);

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Failed to concat curves!");
	}
      else
	{

	  nc = (ay_nurbcurve_object*)concatnc->ncurve->refine;
	  if(concatnc->revert)
	    {
	      ay_status = ay_nct_revert(nc);
	      if(ay_status)
		{
		  ay_error(ay_status, fname, "Could not revert curve!");
		} /* if */
	    } /* if */

	  if(!concatnc->fillgaps && concatnc->closed)
	    {
	      ay_status = ay_nct_close(nc);
	      if(ay_status)
		{
		  ay_error(ay_status, fname, "Could not close curve!");
		} /* if */
	    } /* if */

	} /* if */
    } /* if */

  /* free list of temporary curves */
  ay_object_deletemulti(curves);

 return AY_OK;
} /* ay_concatnc_notifycb */


int
ay_concatnc_convertcb(ay_object *o)
{
 int ay_status = AY_OK;
 ay_concatnc_object *cc = NULL;
 ay_object *new = NULL;
 ay_nurbcurve_object *nc = NULL;

  if(!o)
    return AY_ENULL;

  cc = (ay_concatnc_object *) o->refine;

  if(cc->ncurve)
    {
      ay_status = ay_object_copy(cc->ncurve, &new);

      /* reset display mode of new curve to "global" */
      nc = (ay_nurbcurve_object *)(new->refine);
      nc->display_mode = 0;

      ay_trafo_copy(o, new);
      ay_status = ay_object_link(new);
    }

 return ay_status;
} /* ay_concatnc_convertcb */


int
ay_concatnc_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 ay_concatnc_object *cc = NULL;

  if(!o || !result)
    return AY_ENULL;

  cc = (ay_concatnc_object *) o->refine;

  if(type == AY_IDNCURVE)
    {
      if(cc->ncurve)
	{
	  ay_status = ay_object_copy(cc->ncurve, result);
	  ay_trafo_copy(o, *result);
	}
    }

 return ay_status;
} /* ay_concatnc_providecb */


int
ay_concatnc_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_concatnc_name,
				    ay_concatnc_createcb,
				    ay_concatnc_deletecb,
				    ay_concatnc_copycb,
				    ay_concatnc_drawcb,
				    NULL, /* no handles */
				    NULL, /* no shading */
				    ay_concatnc_setpropcb,
				    ay_concatnc_getpropcb,
				    ay_concatnc_getpntcb,
				    ay_concatnc_readcb,
				    ay_concatnc_writecb,
				    NULL, /* no RIB export */
				    ay_concatnc_bbccb,
				    AY_IDCONCATNC);

  
  ay_status = ay_notify_register(ay_concatnc_notifycb, AY_IDCONCATNC);

  ay_status = ay_convert_register(ay_concatnc_convertcb, AY_IDCONCATNC);

  ay_status = ay_provide_register(ay_concatnc_providecb, AY_IDCONCATNC);

  ay_matt_nomaterial(AY_IDCONCATNC);

 return ay_status;
} /* ay_concatnc_init */

