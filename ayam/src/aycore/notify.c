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
 ay_tag_object *tag = NULL;

  while(lev)
    {
      o = NULL;

      if(lev->next)
	{
	  if(lev->next->object)
	    {
	      o = lev->next->object;
	    } /* if */
	} /* if */

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
	    } /* if */

	  /* search for and execute all NS tag(s) */
	  tag = o->tags;
	  while(tag)
	    {
	      if(tag->type == ay_ns_tagtype)
		ay_ns_execute(o, tag->val);
	      tag = tag->next;
	    }

	} /* if */

      lev = lev->next;
      if(lev)
	{
	  lev = lev->next;
	}
    } /* while */

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
 ay_tag_object *tag = NULL;

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

  /* search for and execute all NS tag(s) */
  tag = o->tags;
  while(tag)
    {
      if(tag->type == ay_ns_tagtype)
	ay_ns_execute(o, tag->val);
      tag = tag->next;
    }

 return AY_OK;
} /* ay_notify_force */


/* ay_notify_forceparent:
 *  call notification callback of parents of object o
 */
int
ay_notify_forceparent(ay_object *o, int silent)
{
 int ay_status = AY_OK;
 char fname[] = "notify_forceparent";
 ay_list_object *oldclevel = ay_currentlevel, *lev;
 int found = AY_FALSE;

  if(!o)
    {
      return AY_ENULL;
    }
  
  if(!ay_root->next)
    {
      return AY_OK;
    }

  oldclevel = ay_currentlevel;

  ay_currentlevel = NULL;
  ay_clevel_add(ay_root);

  ay_status = ay_clevel_find(ay_root->next, o, &found);

  if(!found)
    {
      if(!silent)
	{
	  ay_error(AY_ERROR, fname, "object not found in scene");
	} /* if */
      return AY_OK; /* XXXX early exit! */
    } /* if */

  ay_status = ay_notify_parent();

  while(ay_currentlevel)
    {
      lev = ay_currentlevel->next;
      free(ay_currentlevel);
      ay_currentlevel = lev;
    }

  ay_currentlevel = oldclevel;

 return AY_OK;
} /* ay_notify_forceparent */


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
	} /* while */
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
	    } /* if */
	  sel = sel->next;
	} /* while */

      ay_notify_parent();

    } /* if */

 return TCL_OK;
} /* ay_notify_forcetcmd */
