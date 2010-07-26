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
		   "Can not remove children, conversion failed!");
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
 */
int
ay_convert_forcetcmd(ClientData clientData, Tcl_Interp * interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 int in_place = AY_FALSE, notify_parent = AY_FALSE;

  if(argc > 1)
    {
      if(!strcmp(argv[1], "-inplace"))
	{
	  in_place = AY_TRUE;
	}
    }

  while(sel)
    {
      ay_status = ay_convert_force(sel->object, in_place);
      if(ay_status)
	{
	  ay_error(AY_ERROR, argv[0], NULL);
	}
      else
	{
	  sel->object->modified = AY_TRUE;
	  notify_parent = AY_TRUE;
	}
      sel = sel->next;
    }

  if(notify_parent)
    {
      ay_status = ay_notify_parent();
    }

 return TCL_OK;
} /* ay_convert_forcetcmd */
