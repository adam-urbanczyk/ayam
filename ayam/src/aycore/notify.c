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

/* notify.c - functions for object notification */

/* ay_notify_register:
 *  register the notification callback notcb for
 *  objects of type type_id
 */
int
ay_notify_register(ay_notifycb  *notcb, unsigned int type_id)
{
 int ay_status = AY_OK;
 int overload = ay_prefs.overload;

  /* register notify callback */
  ay_status = ay_table_additem(&ay_notifycbt, (void*)notcb, type_id,
			       overload);

 return ay_status;
} /* ay_notify_register */


/* ay_notify_parent:
 *  call notification callbacks of all parent objects
 *  relative to current level
 */
int
ay_notify_parent(void)
{
 int ay_status = AY_OK;
 char fname[] = "notify_parent";
 ay_list_object *lev = ay_currentlevel;
 ay_object *o = NULL;
 void **arr = NULL;
 ay_notifycb *cb = NULL;

  while(lev)
  {
    if(lev->next)
      if(lev->next->object)
	o = lev->next->object;

    if(o)
      {
	arr = ay_notifycbt.arr;
	cb = (ay_notifycb *)(arr[o->type]);
	if(cb)
	  ay_status = cb(o);

	if(ay_status)
	  {
	    ay_error(AY_ERROR, fname, "notify callback failed");
	    return AY_ERROR;
	  }

      }

    lev = lev->next;
    if(lev)
      lev = lev->next;
  }

 return ay_status;
} /* ay_notify_parent */


/* ay_notify_force:
 *  call notification callback of object o
 */
int
ay_notify_force(ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "notify_force";
 ay_object *od = NULL;
 void **arr = NULL;
 ay_notifycb *cb = NULL;

  /* call notification callbacks of children first */
  od = o->down;
  while(od)
    {
      ay_status = ay_notify_force(od);
      od = od->next;
    }

  /* call the notification callback */
  arr = ay_notifycbt.arr;
  cb = (ay_notifycb *)(arr[o->type]);
  if(cb)
    ay_status = cb(o);

  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "notify callback failed");
      return AY_ERROR;
    }

 return AY_OK;
} /* ay_notify_force */


/* ay_notify_forcetcmd:
 *  force notification of selected objects or all objects
 *  in the scene (if selection is empty)
 */
int
ay_notify_forcetcmd(ClientData clientData, Tcl_Interp * interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "forceNot";
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;


  if((!sel) || (argc >= 2))
    {
      o = ay_root->next;
      while(o)
	{
	  ay_status = ay_notify_force(o);
	  o = o->next;
	}
    }
  else
    {
      while(sel)
	{
	  ay_status = ay_notify_force(sel->object);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, NULL);
	      return TCL_OK;
	    }
	  sel = sel->next;
	}

      ay_notify_parent();

    }

 return TCL_OK;
} /* ay_notify_forcetcmd */

