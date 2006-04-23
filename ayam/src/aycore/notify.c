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


/* global variables for this module: */
static char *ay_nc_tagtype = NULL;

static char* ay_nc_tagname = "NC";

/* functions: */

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
 ay_list_object *lev = ay_currentlevel, *sel = ay_selection;
 ay_object *o = NULL;
 void **arr = NULL;
 ay_notifycb *cb = NULL;
 ay_tag_object *tag = NULL;

  if(ay_prefs.completenotify == 1)
    {
      while(sel)
	{
	  o = sel->object;
	  if(o && o->modified)
	    {
	      ay_status = ay_notify_complete(o);
	    }
	  sel = sel->next;
	}
      return ay_status;
    } /* while */

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
	  if(ay_prefs.completenotify)
	    {
	      ay_status = ay_notify_complete(sel->object);
	      if(ay_status)
		{
		  ay_error(AY_ERROR, fname, NULL);
		  return TCL_OK;
		} /* if */
	    } /* if */
	  sel = sel->next;
	} /* while */

      if(!ay_prefs.completenotify)
	{
	  ay_notify_parent();
	}

    } /* if */

 return TCL_OK;
} /* ay_notify_forcetcmd */


/* ay_notify_findparents:
 *
 */
int
ay_notify_findparents(ay_object *o, ay_object *r, ay_list_object **parents)
{
 ay_object *down;
 ay_tag_object *newt = NULL;
 ay_list_object *newl = NULL;
 int dfound = AY_FALSE, found = AY_FALSE;

  if(!o || !r || !parents)
    return 0;

  if(o->down)
    {
      down = o->down;

      while(down && down->next)
	{
	  dfound = AY_FALSE;
	  if(down->down && down->down->next)
	    {
	      dfound = ay_notify_findparents(down, r, parents);
	    }
	  if(dfound)
	    {
	      found = AY_TRUE;
	    }
	  if((down == r) || (down->refine == r))
	    {
	      found = AY_TRUE;
	    }
	  down = down->next;
	} /* while */

      if(found)
	{
	  if(!(newl = calloc(1, sizeof(ay_list_object))))
	    {
	      return 0;
	    }
	  if(!(newt = calloc(1, sizeof(ay_tag_object))))
	    {
	      return 0;
	    }
	  newl->object = o;
	  o->modified = AY_FALSE;
	  newl->next = *parents;
	  *parents = newl;

	  newt->next = o->tags;
	  newt->type = ay_nc_tagtype;
	  o->tags = newt;

	} /* if */
    } /* if */

 return found;
} /* ay_notify_findparents */


/* ay_notify_complete:
 *
 */
int
ay_notify_complete(ay_object *r)
{
 int propagate = AY_TRUE;
 ay_object *o;
 ay_tag_object *tag;
 ay_list_object *l = NULL, *s, *t, *u;

  if(!r)
   return AY_ENULL;

  o = ay_root->next;

  while(o)
    {
      ay_notify_findparents(o, r, &l);
      o = o->next;
    } /* while */

  u = NULL;
  while(propagate)
    {
      propagate = AY_FALSE;
      t = l;
      s = l;
      while(s && (s != u))
	{
	  if(s->object && (s->object->refcount > 0))
	    {
	      o = ay_root->next;
	      while(o)
		{
		  ay_notify_findparents(o, s->object, &l);
		  o = o->next;
		} /* while */
	    } /* if */
	  s = s->next;
	} /* while */

      if(l != t)
	{
	  /* added some objects, need to continue propagating... */
	  propagate = AY_TRUE;
	}
      /* for consecutive iterations, arrange to stop at old list, because
	 we processed that already */
      u = t;

    } /* while */

  /* revert list */
  s = NULL;
  while(l)
    {
      t = l;
      l = l->next;
      o = t->object;
      if(o && o->tags && (o->tags->type == ay_nc_tagtype))
	{
	  ((int)o->tags->val)++;
	}
      t->next = s;
      s = t;
    } /* while */

  while(s)
    {
      o = s->object;
      if(o && o->tags && (o->tags->type == ay_nc_tagtype))
	{
	  ((int)o->tags->val)--;
	  if(o->tags->val == 0)
	    {
	      ay_notify_force(o);
	      tag = o->tags;
	      o->tags = tag->next;
	      free(tag);
	    }
	}
      t = s->next;
      free(s);
      s = t;
    } /* while */

 return AY_OK;
} /* ay_notify_complete */


int
ay_notify_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  /* register NC tag type */
  ay_status = ay_tags_register(interp, ay_nc_tagname, &ay_nc_tagtype);
  if(ay_status)
    return ay_status;

  return AY_OK;
} /* ay_notify_init */
