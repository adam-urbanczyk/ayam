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

/* cap.c - cap object */

static char *ay_cap_name = "Cap";

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

  o->refine = new;
  o->parent = AY_TRUE;

 return AY_OK;
} /* ay_cap_createcb */


int
ay_cap_deletecb(void *c)
{
 ay_cap_object *cap = NULL;

  if(!c)
    return AY_ENULL;    

  cap = (ay_cap_object *)(c);

  ay_object_delete(cap->npatch);

  free(cap);

 return AY_OK;
} /* ay_cap_deletecb */


int
ay_cap_copycb(void *src, void **dst)
{
 ay_cap_object *cap = NULL, *capsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  capsrc = (ay_cap_object *)src;

  if(!(cap = calloc(1, sizeof(ay_cap_object))))
    return AY_EOMEM; 

  memcpy(cap, src, sizeof(ay_cap_object)); 

  cap->npatch = NULL;

  /* copy npatch */
  if(capsrc->npatch)
    ay_object_copy(capsrc->npatch, &(cap->npatch));

  *dst = (void *)cap;

 return AY_OK;
} /* ay_cap_copycb */


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


int
ay_cap_drawhcb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_cap_drawhcb */


int
ay_cap_getpntcb(int mode, ay_object *o, double *p)
{

 return AY_OK;
} /* ay_cap_getpntcb */


/* Tcl -> C! */
int
ay_cap_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "CapAttrData";
 /* char fname[] = "cap_setpropcb";*/
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_cap_object *cap = NULL;

  if(!o)
    return AY_ENULL;

  cap = (ay_cap_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetDoubleFromObj(interp,to, &(cap->glu_sampling_tolerance));

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &(cap->glu_display_mode));

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  if(cap->npatch)
    {
      ((ay_nurbpatch_object *)cap->npatch->refine)->glu_sampling_tolerance =
	cap->glu_sampling_tolerance;
      ((ay_nurbpatch_object *)cap->npatch->refine)->glu_display_mode =
	cap->glu_display_mode;
    }

  /*ay_status = ay_notify_force(o);*/

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_cap_setpropcb */


/* C -> Tcl! */
int
ay_cap_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="CapAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_cap_object *cap = NULL;

  if(!o)
    return AY_ENULL;

  cap = (ay_cap_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Tolerance",-1);
  to = Tcl_NewDoubleObj(cap->glu_sampling_tolerance);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"DisplayMode",-1);
  to = Tcl_NewIntObj(cap->glu_display_mode);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_cap_getpropcb */


int
ay_cap_readcb(FILE *fileptr, ay_object *o)
{
 ay_cap_object *cap = NULL;

 if(!o)
   return AY_ENULL;

  if(!(cap = calloc(1, sizeof(ay_cap_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%lg\n",&cap->glu_sampling_tolerance);
  fscanf(fileptr,"%d\n",&cap->glu_display_mode);

  o->refine = cap;

 return AY_OK;
} /* ay_cap_readcb */


int
ay_cap_writecb(FILE *fileptr, ay_object *o)
{
 ay_cap_object *cap = NULL;

  if(!o)
    return AY_ENULL;

  cap = (ay_cap_object *)(o->refine);

  fprintf(fileptr, "%g\n", cap->glu_sampling_tolerance);
  fprintf(fileptr, "%d\n", cap->glu_display_mode);

 return AY_OK;
} /* ay_cap_writecb */


int
ay_cap_wribcb(char *file, ay_object *o)
{
 ay_cap_object *cap = NULL;
 int ay_status = AY_OK;

  if(!o)
   return AY_ENULL;

  cap = (ay_cap_object*)o->refine;

  ay_status = ay_wrib_object(file, cap->npatch);

 return AY_OK;
} /* ay_cap_wribcb */


int
ay_cap_bbccb(ay_object *o, double *bbox, int *flags)
{
 ay_cap_object *cap = NULL;
 int ay_status = AY_OK;

  if(!o || !bbox)
    return AY_ENULL;

  cap = (ay_cap_object *)o->refine; 

  ay_status = ay_bbc_get(cap->npatch, bbox);

  flags = 0;

 return ay_status;
} /* ay_cap_bbccb */


int
ay_cap_notifycb(ay_object *o)
{
 ay_cap_object *cap = NULL;
 ay_object *down = NULL, *pobject = NULL;
 ay_object **nextcurve = NULL;
 int ay_status = AY_OK;

  if(!o)
    return AY_ENULL;    

  cap = (ay_cap_object *)(o->refine);

  nextcurve = &(pobject);

  /* first, delete old cap */
  if(cap->npatch)
    ay_object_delete(cap->npatch);
  cap->npatch = NULL;

  /* get parameter curve(s) */
  if(!o->down)
    return AY_OK;
  down = o->down;
  while(down->next)
    {
      if(down->type != AY_IDNCURVE)
	{
	  ay_status = ay_provide_object(down, AY_IDNCURVE, nextcurve);
	  if(!nextcurve)
	    {
	      /* XXXX convert NPatch to curve here later */
#if 0
	      if(down->type == AY_IDNPATCH)
		{
		  /* get info which curve from tag */
		  ay_status = ay_nct_createfrompatch(down, 3, nextcurve);
		  if(nextcurve)
		    {
		      nextcurve = &((*nextcurve)->next);
		    } /* if */
		} /* if */
#endif	  
	    }
	  else
	    {
	      nextcurve = &((*nextcurve)->next);
	    } /* if */
	}
      else
	{
	  ay_object_copy(down, nextcurve);
	  nextcurve = &((*nextcurve)->next);
	} /* if */

      down = down->next;
    } /* while */

  if(!pobject)
    {
      return AY_OK;
    } /* if */

  /* the pobject points now to copies of the curves for which
     the cap should be created (and we may mess with it) */

  ay_status = ay_capt_createfromcurve(pobject, &(cap->npatch));

  if(ay_status)
    {
      ay_object_deletemulti(pobject);
      return ay_status;
    }

  if(cap->npatch)
    {
      ((ay_nurbpatch_object *)cap->npatch->refine)->glu_sampling_tolerance =
	cap->glu_sampling_tolerance;
      ((ay_nurbpatch_object *)cap->npatch->refine)->glu_display_mode =
	cap->glu_display_mode;
    }

 return AY_OK;
} /* ay_cap_notifycb */


int
ay_cap_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_cap_object *cap = NULL;
 ay_object *new = NULL;

  if(!o)
    return AY_ENULL;

  cap = (ay_cap_object *) o->refine;

  if(cap->npatch)
    {
      ay_status = ay_object_copy(cap->npatch, &new);

      if(new)
	{
	  ay_trafo_add(o, new);
	  if(!in_place)
	    {
	      ay_status = ay_object_link(new);
	    }
	  else
	    {
	      ay_status = ay_object_replace(new, o);
	    } /* if */
	} /* if */
    } /* if */

 return ay_status;
} /* ay_cap_convertcb */


int
ay_cap_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 ay_cap_object *cap = NULL;

  if(!o || !result)
    return AY_ENULL;

  cap = (ay_cap_object *) o->refine;

  if(type == AY_IDNPATCH)
    {
      if(cap->npatch)
	{
	  ay_status = ay_object_copy(cap->npatch, result);
	  if(*result)
	    {
	      ay_trafo_add(o, *result);
	    } /* if */
	} /* if */
    } /* if */

 return ay_status;
} /* ay_cap_providecb */


int
ay_cap_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_cap_name,
				    ay_cap_createcb,
				    ay_cap_deletecb,
				    ay_cap_copycb,
				    ay_cap_drawcb,
				    NULL, /* No handles! */
				    ay_cap_shadecb,
				    ay_cap_setpropcb,
				    ay_cap_getpropcb,
				    NULL, /* No points! */
				    ay_cap_readcb,
				    ay_cap_writecb,
				    ay_cap_wribcb,
				    ay_cap_bbccb,
				    AY_IDCAP);

  
  ay_status = ay_notify_register(ay_cap_notifycb, AY_IDCAP);

  ay_status = ay_convert_register(ay_cap_convertcb, AY_IDCAP);

  ay_status = ay_provide_register(ay_cap_providecb, AY_IDCAP);

 return ay_status;
} /* ay_cap_init */

