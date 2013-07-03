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

/* convert.c - functions for object conversion */

/* ay_convert_register:
 *  register the conversion callback notcb for
 *  objects of type type_id
 */
int
ay_convert_register(ay_convertcb *convcb, unsigned int type_id)
{
 int ay_status = AY_OK;

  /* register convert callback */
  ay_status = ay_table_additem(&ay_convertcbt, (ay_voidfp)convcb, type_id);

 return ay_status;
} /* ay_convert_register */


/* ay_convert_force:
 *  call conversion callback of object o
 */
int
ay_convert_force(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 char fname[] = "convert";
 ay_voidfp *arr = NULL;
 ay_convertcb *cb = NULL;

  if(!o)
    return AY_ENULL;

  if(in_place && o->down && o->down->next)
    {
      ay_status = ay_object_candelete(o->down, o->down);

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname,
		   "Can not remove children, conversion aborted!");
	  return AY_ERROR;
	}
    }

  /* call the conversion callback */
  arr = ay_convertcbt.arr;
  cb = (ay_convertcb *)(arr[o->type]);
  if(cb)
    {
      ay_status = cb(o, in_place);

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "Convert callback failed!");
	  return AY_ERROR;
	}
    }
  else
    {
     ay_error(AY_ERROR, fname, "No converter available!");
    }

 return AY_OK;
} /* ay_convert_force */


/* ay_convert_forcetcmd:
 *  force conversion of selected object(s)
 *  Implements the \a convOb scripting interface command.
 *  See also the corresponding section in the \ayd{scconvob}.
 *  \returns TCL_OK in any case.
 */
int
ay_convert_forcetcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK, in_place = AY_FALSE, notify_parent = AY_FALSE;
 ay_list_object *sel = ay_selection;

  if(argc > 1)
    {
      if((argv[1][0] == '-') && (argv[1][1] == 'i'))
	{
	  in_place = AY_TRUE;
	}
    }

  while(sel)
    {
      ay_status = ay_convert_force(sel->object, in_place);
      if(!ay_status)
	{
	  sel->object->modified = AY_TRUE;
	  notify_parent = AY_TRUE;
	}
      sel = sel->next;
    }

  if(notify_parent)
    {
      (void)ay_notify_parent();
    }

 return TCL_OK;
} /* ay_convert_forcetcmd */


/* ay_convert_nptoolobj:
 *  helper for tool objects that convert to a NURBS patch
 *  plus caps/bevels
 */
int
ay_convert_nptoolobj(ay_object *o, ay_object *p, ay_object *cb, int in_place)
{
 int ay_status = AY_OK;
 ay_object *new = NULL, **next = NULL;
 ay_object *b;
 ay_nurbpatch_object *np = NULL;

  if(!o || !p)
    return AY_ENULL;

  if(p->next || cb)
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

      b = p;
      while(b)
	{
	  ay_status = ay_object_copy(b, next);
	  if(ay_status)
	    {
	      ay_object_deletemulti(new);
	      return AY_ERROR;
	    }
	  if(*next)
	    {
	      (*next)->hide_children = AY_TRUE;
	      (*next)->parent = AY_TRUE;
	      if(!(*next)->down)
		(*next)->down = ay_endlevel;

	      next = &((*next)->next);
	    }
	  b = b->next;
	} /* while */

      b = cb;
      while(b)
	{
	  ay_status = ay_object_copy(b, next);
	  if(ay_status)
	    {
	      ay_object_deletemulti(new);
	      return AY_ERROR;
	    }
	  if(*next)
	    {
	      (*next)->hide_children = AY_TRUE;
	      (*next)->parent = AY_TRUE;
	      if(!(*next)->down)
		(*next)->down = ay_endlevel;

	      next = &((*next)->next);
	    }
	  b = b->next;
	} /* while */

      /* copy eventually present TP tags */
      ay_npt_copytptag(o, new->down);

      /* terminate the level */
      *next = ay_endlevel;
    }
  else
    {
      ay_status = ay_object_copy(p, &new);
      if(ay_status)
	{
	  ay_object_deletemulti(new);
	  return AY_ERROR;
	}
      if(new)
	{
	  ay_trafo_copy(o, new);
	  new->hide_children = AY_TRUE;
	  new->parent = AY_TRUE;
	  if(!new->down)
	    new->down = ay_endlevel;
	} /* if */
    } /* if */

  if(new)
    {
      if(p->next || cb)
	{
	  b = new->down;
	}
      else
	{
	  b = new;
	}
      while(b)
	{
	  /* reset display mode and sampling tolerance
	     of new patch to "global"? */
	  if(!in_place && ay_prefs.conv_reset_display)
	    {
	      ay_npt_resetdisplay(b);
	    }

	  /* immediately create and show the multiple points */
	  if(b->type == AY_IDNPATCH)
	    {
	      np = (ay_nurbpatch_object *)b->refine;
	      np->createmp = AY_TRUE;
	      ay_npt_recreatemp(np);
	    }
	  b = b->next;
	} /* while */

      /* link the new object(s) to the scene */
      if(!in_place)
	{
	  ay_status = ay_object_link(new);
	}
      else
	{
	  ay_status = ay_object_replace(new, o);
	}
    } /* if */

 return ay_status;
} /* ay_convert_nptoolobj */
