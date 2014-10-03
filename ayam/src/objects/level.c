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

/* level.c - level object */

static char *ay_level_name = "Level";

/* functions: */

/* ay_level_createcb:
 *  create callback function of level object
 */
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
    {
      if(!argv)
	return AY_ENULL;

      level->type = (unsigned int) atoi(argv[2]);
    }

 return AY_OK;
} /* ay_level_createcb */


/* ay_level_deletecb:
 *  delete callback function of level object
 */
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


/* ay_level_copycb:
 *  copy callback function of level object
 */
int
ay_level_copycb(void *src, void **dst)
{
 ay_level_object *level = NULL;

  if(!(level = malloc(sizeof(ay_level_object))))
    return AY_EOMEM;

  memcpy(level, src, sizeof(ay_level_object));

  *dst = level;

 return AY_OK;
} /* ay_level_copycb */


/* ay_level_drawcb:
 *  draw (display in an Ayam view window) callback function of level object
 */
int
ay_level_drawcb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_level_drawcb */


/* ay_level_drawhcb:
 *  draw handles (in an Ayam view window) callback function of level object
 */
int
ay_level_drawhcb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_level_drawhcb */


/* ay_level_shadecb:
 *  shade (display in an Ayam view window) callback function of level object
 */
int
ay_level_shadecb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_level_shadecb */


/* ay_level_getpntcb:
 *  get point (editing and selection) callback function of level object
 */
int
ay_level_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_level_getpntcb */


/* ay_level_setpropcb:
 *  set property (from Tcl to C context) callback function of level object
 */
int
ay_level_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1 = "LevelAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_level_object *level = NULL;

  if(!interp || !o)
    return AY_ENULL;

  level = (ay_level_object *)o->refine;

  if(!level)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("Type",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &level->type);
  level->type++;
  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_level_setpropcb */


/* ay_level_getpropcb:
 *  get property (from C to Tcl context) callback function of level object
 */
int
ay_level_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="LevelAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_level_object *level = NULL;
 int itmp = 0;

  if(!interp || !o)
    return AY_ENULL;

  level = (ay_level_object *)o->refine;

  if(!level)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("Type",-1);
  itmp = level->type-1;
  to = Tcl_NewIntObj(itmp);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_level_getpropcb */


/* ay_level_readcb:
 *  read (from scene file) callback function of level object
 */
int
ay_level_readcb(FILE *fileptr, ay_object *o)
{
 ay_level_object *level = NULL;
 int type = 0, result = 0;

  if(!fileptr || !o)
    return AY_ENULL;

  fscanf(fileptr,"%d\n",&type);

  if(type == AY_LTEND)
    {
      /* the level terminator was already created
	 when the parent object was read, thus, all we
	 need to do here is to go up in the hierarchy */
      result = ay_clevel_gouptcmd(NULL, ay_interp, 0, NULL);
      if(result != TCL_OK)
	return AY_ERROR;
      else
	return AY_EDONOTLINK;
    }

  if(!(level = calloc(1, sizeof(ay_level_object))))
    { return AY_EOMEM; }

  level->type = type;

  o->refine = level;

 return AY_OK;
} /* ay_level_readcb */


/* ay_level_writecb:
 *  write (to scene file) callback function of level object
 */
int
ay_level_writecb(FILE *fileptr, ay_object *o)
{
 ay_level_object *level = NULL;

  if(!fileptr || !o)
    return AY_ENULL;

  level = (ay_level_object *)(o->refine);

  if(!level)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", level->type);

 return AY_OK;
} /* ay_level_writecb */


/* ay_level_wribcb:
 *  RIB export callback function of level object
 *  does nothing, the real work of testing level types
 *  and writing appropriate RiSolids is done in:
 *  aycore/wrib.c/ay_wrib_object()
 *  aycore/instt.c/ay_instt_wribiarchives()
 *  objects/instance.c/ay_instance_wribcb()
 */
int
ay_level_wribcb(char *file, ay_object *o)
{
  /*
  ay_level_object *level = NULL;

  if(!o)
   return AY_ENULL;

  level = (ay_level_object*)o->refine;
  */

 return AY_OK;
} /* ay_level_wribcb */


/* ay_level_bbccb:
 *  bounding box calculation callback function of level object
 */
int
ay_level_bbccb(ay_object *o, double *bbox, int *flags)
{

  if(!o || !bbox || !flags)
    return AY_ENULL;

  /* we have no own bounding box, all that counts are the children */
  *flags = 2;

 return AY_OK;
} /* ay_level_bbccb */


/* ay_level_providecb:
 *  provide callback function of level object
 */
int
ay_level_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 ay_object *t = NULL, *d, **last;
 ay_level_object *l = NULL;
 double m1[16], m2[16], m3[16];

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      /* can we deliver atleast one object of right type? */
      d = o->down;
      while(d && d->next)
	{
	  if(type == d->type)
	    {
	      return AY_OK;
	    }
	  else
	    {
	      if(AY_OK == ay_provide_object(d, type, NULL))
		return AY_OK;
	    }
	  d = d->next;
	} /* while */
      return AY_ERROR;
    } /* if */

  l = (ay_level_object *)o->refine;

  if(!l)
    return AY_ENULL;

  if(l->type == AY_LTEND)
    return AY_OK;

  /* check for presence of any child objects */
  if(!o->down || (o->down && !o->down->next))
    return AY_ERROR;

  d = o->down;
  last = result;
  while(d->next)
    {

      ay_trafo_creatematrix(d, m1);

      *last = NULL;
      if(type == d->type)
	{
	  ay_status = ay_object_copy(d, last);
	  if(*last)
	    {
	      last = &((*last)->next);
	    }
	}
      else
	{
	  ay_status = ay_provide_object(d, type, last);
	  t = *last;

	  if(d->type == AY_IDLEVEL)
	    {
	      while(t)
		{
		  memcpy(m3, m1, 16*sizeof(double));
		  ay_trafo_creatematrix(t, m2);
		  ay_trafo_multmatrix(m3, m2);
		  ay_trafo_decomposematrix(m3, t);
		
		  last = &(t->next);
		  t = t->next;
		} /* while */
	    }
	  else
	    {
	      while(t)
		{
		  last = &(t->next);
		  t = t->next;
		} /* while */
	    } /* if */

	} /* if */
      d = d->next;
    } /* while */

 return ay_status;
} /* ay_level_providecb */


/* ay_level_init:
 *  initialize the level object module
 */
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

  ay_status += ay_provide_register(ay_level_providecb, AY_IDLEVEL);

 return ay_status;
} /* ay_level_init */
