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

/* level.c - level object */

static char *ay_level_name = "Level";

int
ay_level_createcb(int argc, char *argv[], ay_object *o)
{
 ay_level_object *level = NULL;
 char fname[] = "crtlevel";

  if(!o)
    return AY_ENULL;

  if(!(level = calloc(1, sizeof(ay_level_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  o->inherit_trafos = AY_TRUE;
  o->parent = AY_TRUE;
  o->refine = level;

  if(argc > 2)
    level->type = (unsigned int) atoi(argv[2]);

 return AY_OK;
} /* ay_level_createcb */


int
ay_level_deletecb(void *c)
{
 ay_level_object *level = NULL;

  if(!c)
    return AY_ENULL;

  level = (ay_level_object *)(c);

  free(level);

 return AY_OK;
} /* ay_level_deletecb */


int
ay_level_copycb(void *src, void **dst)
{
 ay_level_object *level = NULL;

  if(!(level = calloc(1, sizeof(ay_level_object))))
    return AY_EOMEM; 

  memcpy(level, src, sizeof(ay_level_object)); 

  *dst = level;

 return AY_OK;
} /* ay_level_copycb */


int
ay_level_drawcb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_level_drawcb */


int
ay_level_drawhcb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_level_drawhcb */


int
ay_level_shadecb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_level_shadecb */


int
ay_level_getpntcb(ay_object *o, double *p)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_level_getpntcb */


/* Tcl -> C! */
int
ay_level_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1 = "LevelAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_level_object *level = NULL;

  if(!o)
    return AY_ENULL;

  level = (ay_level_object *)o->refine;
  
  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("Type",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &level->type);
  level->type++;
  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_level_setpropcb */


/* C -> Tcl! */
int
ay_level_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="LevelAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_level_object *level = NULL;
 int itmp = 0;

  if(!o)
    return AY_ENULL;

  level = (ay_level_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("Type",-1);
  itmp = level->type-1;
  to = Tcl_NewIntObj(itmp);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_level_getpropcb */


int
ay_level_readcb(FILE *fileptr, ay_object *o)
{
 ay_level_object *level = NULL;
 int type = 0, result = 0;

  if(!o)
    return AY_ENULL;

  fscanf(fileptr,"%d\n",&type);

  if(type == AY_LTEND)
    {
      result = ay_clevel_gouptcmd(NULL, ay_interp, 0, NULL);
      return AY_EDONOTLINK;
    }

  if(!(level = calloc(1, sizeof(ay_level_object))))
    { return AY_EOMEM; }

  level->type = type;

  o->refine = level;

 return AY_OK;
} /* ay_level_readcb */


int
ay_level_writecb(FILE *fileptr, ay_object *o)
{
 ay_level_object *level = NULL;

  if(!o)
    return AY_ENULL;

  level = (ay_level_object *)(o->refine);
 
  fprintf(fileptr, "%d\n", level->type);

 return AY_OK;
} /* ay_level_writecb */


/* ay_level_wribcb:
 *  does nothing, the real work of testing level types
 *  and writing appropriate RiSolids is done in:
 *  aycore/wrib.c/ay_wrib_object()
 *  aycore/instt.c/ay_instt_wribiarchives()
 *  objects/instance.c/ay_instance_wribcb()
 */
int
ay_level_wribcb(char *file, ay_object *o)
{
 ay_level_object *level = NULL;

  if(!o)
   return AY_ENULL;

  level = (ay_level_object*)o->refine;


 return AY_OK;
} /* ay_level_wribcb */


int
ay_level_bbccb(ay_object *o, double *bbox, int *flags)
{

  if(!o || !bbox)
    return AY_ENULL;

  /* we have no own bounding box, all that counts are the children */
  *flags = 2;

 return AY_OK;
} /* ay_level_bbccb */


int
ay_level_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_level_name,
				    ay_level_createcb,
				    ay_level_deletecb,
				    ay_level_copycb,
				    NULL, /* no drawing! */
				    NULL, /* no handles! */
				    NULL, /* no shading! */
				    ay_level_setpropcb,
				    ay_level_getpropcb,
				    NULL, /* no picking! */
				    ay_level_readcb,
				    ay_level_writecb,
				    ay_level_wribcb,
				    ay_level_bbccb,
				    AY_IDLEVEL);

  Tcl_SetVar(interp,"propertyList","LevelAttr", TCL_APPEND_VALUE |
	     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

 return ay_status;
} /* ay_level_init */
