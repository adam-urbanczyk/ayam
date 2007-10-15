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

/* select.c - select object */

static char *ay_select_name = "Select";

int
ay_select_createcb(int argc, char *argv[], ay_object *o)
{
 ay_select_object *select = NULL;
 char fname[] = "crtselect";

  if(!o)
    return AY_ENULL;

  if(!(select = calloc(1, sizeof(ay_select_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  o->inherit_trafos = AY_TRUE;
  o->parent = AY_TRUE;
  o->refine = select;

  if(argc > 2)
    select->index = atoi(argv[2]);

 return AY_OK;
} /* ay_select_createcb */


int
ay_select_deletecb(void *c)
{
 ay_select_object *select = NULL;

  if(!c)
    return AY_ENULL;

  select = (ay_select_object *)(c);

  free(select);

 return AY_OK;
} /* ay_select_deletecb */


int
ay_select_copycb(void *src, void **dst)
{
 ay_select_object *select = NULL;

  if(!(select = calloc(1, sizeof(ay_select_object))))
    return AY_EOMEM;

  memcpy(select, src, sizeof(ay_select_object));

  *dst = select;

 return AY_OK;
} /* ay_select_copycb */


int
ay_select_drawcb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_select_drawcb */


int
ay_select_drawhcb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_select_drawhcb */


int
ay_select_shadecb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_select_shadecb */


int
ay_select_getpntcb(int mode, ay_object *o, double *p)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_select_getpntcb */


/* Tcl -> C! */
int
ay_select_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1 = "SelectAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_select_object *select = NULL;

  if(!o)
    return AY_ENULL;

  select = (ay_select_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("Index",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &select->index);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_select_setpropcb */


/* C -> Tcl! */
int
ay_select_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="SelectAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_select_object *select = NULL;

  if(!o)
    return AY_ENULL;

  select = (ay_select_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("Index",-1);
  to = Tcl_NewIntObj(select->index);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_select_getpropcb */


int
ay_select_readcb(FILE *fileptr, ay_object *o)
{
 ay_select_object *select = NULL;

  if(!o)
    return AY_ENULL;

  if(!(select = calloc(1, sizeof(ay_select_object))))
    { return AY_EOMEM; }

  fscanf(fileptr,"%d\n",&select->index);

  o->refine = select;

 return AY_OK;
} /* ay_select_readcb */


int
ay_select_writecb(FILE *fileptr, ay_object *o)
{
 ay_select_object *select = NULL;

  if(!o)
    return AY_ENULL;

  select = (ay_select_object *)(o->refine);

  fprintf(fileptr, "%d\n", select->index);

 return AY_OK;
} /* ay_select_writecb */


int
ay_select_wribcb(char *file, ay_object *o)
{
 ay_select_object *select = NULL;

  if(!o)
   return AY_ENULL;

  select = (ay_select_object*)o->refine;

 return AY_OK;
} /* ay_select_wribcb */


int
ay_select_bbccb(ay_object *o, double *bbox, int *flags)
{

  if(!o || !bbox)
    return AY_ENULL;

  /* we have no own bounding box, all that counts are the children */
  *flags = 2;

 return AY_OK;
} /* ay_select_bbccb */


int
ay_select_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 int i = 0;
 ay_object *t = NULL, *d;
 ay_select_object *sel = NULL;

  if(!o)
    return AY_ENULL;

  sel = (ay_select_object *)o->refine;

  d = o->down;

  if(!result)
    {
      return ay_provide_object(d, type, NULL);
    } /* if */

  ay_status = ay_provide_object(d, type, &t);

  if(!t)
    {
      /* provide failed */
      return AY_ERROR;
    }

  d = t;
  for(i = 0; i < sel->index; i++)
    {
      if(!d)
	{
	  ay_object_deletemulti(t);
	  return AY_ERROR;
	}
      d = d->next;
    }

  ay_status = ay_object_copy(d, result);

  ay_object_deletemulti(t);

 return ay_status;
} /* ay_select_providecb */


int
ay_select_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_select_name,
				    ay_select_createcb,
				    ay_select_deletecb,
				    ay_select_copycb,
				    NULL, /* no drawing! */
				    NULL, /* no handles! */
				    NULL, /* no shading! */
				    ay_select_setpropcb,
				    ay_select_getpropcb,
				    NULL, /* no picking! */
				    ay_select_readcb,
				    ay_select_writecb,
				    ay_select_wribcb,
				    ay_select_bbccb,
				    AY_IDSELECT);

  ay_status = ay_provide_register(ay_select_providecb, AY_IDSELECT);

 return ay_status;
} /* ay_select_init */
