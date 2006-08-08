/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2004 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

#include "ayam.h"

/* object.c - general object management*/

/* ay_object_defaults:
 *  reset object attributes of ay_object *o to default settings
 */
int
ay_object_defaults(ay_object *o)
{
  if(!o)
    return AY_ENULL;

  o->quat[3] = 1.0;

  o->scalx = 1.0;
  o->scaly = 1.0;
  o->scalz = 1.0;

  o->inherit_trafos = AY_TRUE;

 return AY_OK;
} /* ay_object_defaults */


int
ay_object_create(unsigned int index, ay_object **o)
{
 int ay_status = AY_OK;
 char fname[] = "object_create";
 void **arr = NULL;
 ay_createcb *cb = NULL;
 ay_object *new = NULL;


  arr = ay_createcbt.arr;

  if(!(new = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  ay_status = ay_object_defaults(new);

  new->type = index;

  cb = (ay_createcb *)(arr[index]);

  ay_status = cb(0, NULL, new);

  if(ay_status)
    {
      ay_error(ay_status, fname, NULL);
      free(new);
      return AY_ERROR;
    }

  *o = new;

 return ay_status;
} /* ay_object_create */


int
ay_object_createargs(unsigned int index, int argc, char **argv, ay_object **o)
{
 int ay_status = AY_OK;
 char fname[] = "object_createargs";
 void **arr = NULL;
 ay_createcb *cb = NULL;
 ay_object *new = NULL;


  arr = ay_createcbt.arr;

  if(!(new = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  ay_status = ay_object_defaults(new);

  new->type = index;

  cb = (ay_createcb *)(arr[index]);

  ay_status = cb(argc, argv, new);

  if(ay_status)
    {
      ay_error(ay_status, fname, NULL);
      free(new);
      return AY_ERROR;
    }

  *o = new;

 return ay_status;
} /* ay_object_createargs */


int
ay_object_createtcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "crtOb";
 void **arr = NULL;
 unsigned int index;
 Tcl_HashEntry *entry = NULL;
 ay_createcb *cb = NULL;
 ay_object *o = NULL;
 char *a = "ay", *n = "sc", *v = "1";

  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "typename \\[args\\]!");
      return TCL_OK;
    }

  entry = Tcl_FindHashEntry(&ay_otypesht, argv[1]);
  if(!entry)
    {
      ay_error(AY_ENTYPE, fname, argv[1]);
      return TCL_OK;
    }

  index = (unsigned int) Tcl_GetHashValue(entry);

  if(!(o = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return TCL_OK;
    }

  o->type = index;
  ay_status = ay_object_defaults(o);

  arr = ay_createcbt.arr;
  cb = (ay_createcb *)(arr[index]);
  if(cb)
    ay_status = cb(argc, argv, o);

  if(ay_status)
    {
      ay_error(ay_status, fname, " Create callback failed!");
      free(o);
      return TCL_OK;
    }

  /* for objects that may have childs that do not have
     a child already create an EndLevel object */
  if(o->parent && (!o->down))
    {
      ay_status = ay_object_create(AY_IDLEVEL, &(o->down));
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname,
           "Could not create terminating level object, scene is corrupt now!");
	}
    }


  ay_object_link(o);

  /* set scene changed flag */
  Tcl_SetVar2(interp, a, n, v, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

 return TCL_OK;
} /* ay_object_createtcmd */


int
ay_object_delete(ay_object *o)
{
 int ay_status = AY_OK;
 void **arr = NULL;
 ay_deletecb *cb = NULL;
 ay_object *down = NULL, *d = NULL;
 ay_mat_object *mat = NULL;
 unsigned int *refcountptr;

 if(!o)
   return AY_ENULL;

 if(o->refcount > 0)
   return AY_EREF;

  /* delete children first */
  if(o->down)
    {
      down = o->down;
      while(down)
	{
	  d = down;
	  down = down->next;
	  ay_status = AY_OK;
	  ay_status = ay_object_delete(d);
	  if(ay_status)
	    {
	      return ay_status;
	    } /* if */
	} /* while */
    } /* if */

  arr = ay_deletecbt.arr;
  cb = (ay_deletecb *)(arr[o->type]);
  if(cb)
    ay_status = cb(o->refine);

  if(ay_status)
    return ay_status;

  /* delete selected points */
  if(o->selp)
    ay_selp_clear(o);

  /* delete tags */
  if(o->tags)
    ay_tags_delall(o);

  /* remove reference to material */
  if(o->mat)
    {
      mat = o->mat;
      refcountptr = mat->refcountptr;
      (*refcountptr)--;
    }

  /* free name */
  if(o->name)
    {
      free(o->name);
      o->name = NULL;
    }

  /* finally, delete the object */
  free(o);

 return AY_OK;
} /* ay_object_delete */


int
ay_object_deletemulti(ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *next = NULL, *d = NULL;

  if(!o)
    return AY_ENULL;

  d = o;
  while(d)
    {
      next = d->next;
      ay_status = ay_object_delete(d);
      if(ay_status)
	{
	  return ay_status;
	}
      d = next;
    }

 return AY_OK;
} /* ay_object_deletemulti */


int
ay_object_deletetcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "delOb";
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection, *try_again = NULL, **next_try_again, *t;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  next_try_again = &(try_again);

  while(sel)
    {
      o = sel->object;
      if(o == ay_root)
	{
	  ay_error(AY_ERROR, fname, "Root object may not be deleted!");
	}
      else
	{
	  ay_object_unlink(o);

	  ay_status = ay_object_delete(o);
	  if(ay_status)
	    {
	      /*
	      ay_error(ay_status, fname, NULL);
	      ay_object_link(o);
	      return TCL_OK;
	      */
	      if(!(*next_try_again = calloc(1, sizeof(ay_list_object))))
		{
		  ay_error(AY_EOMEM, fname, NULL);
		  return TCL_OK;
		}
	      (*next_try_again)->object = o;
	      next_try_again = &((*next_try_again)->next);
	    } /* if */

	  ay_status = ay_undo_clearobj(o);

	} /* if */
      sel = sel->next;
    } /* while */

  while(try_again)
    {
      ay_status = ay_object_delete(try_again->object);
      if(ay_status)
	{
	  ay_error(ay_status, fname, NULL);
	  ay_object_link(o);
	}
      t = try_again->next;
      free(try_again);
      try_again = t;
    } /* while */

  /* clear all cached pointers to scene hierarchy */
  ay_status = ay_object_ccp(NULL);

  /* free point selection */
  if(ay_point_edit_coords)
    free(ay_point_edit_coords);

  ay_point_edit_coords = NULL;

  /* free selection */
  ay_sel_free(AY_FALSE);

  ay_notify_parent();

 return TCL_OK;
} /* ay_object_deletetcmd */



/* ay_object_link:
 *  link object o into scene
 */
int
ay_object_link(ay_object *o)
{
 int ay_status = AY_OK;

  if(ay_next)
    {
      o->next = *ay_next;

      *ay_next = o;

      ay_next = &(o->next);
    }

  if(ay_currentlevel->object != ay_root)
    {
      ay_clevel_del();
      ay_clevel_add(ay_currentlevel->object->down);
    }

 return ay_status;
} /* ay_object_link */


/* ay_object_unlink:
 *  unlink object o from scene, without deleting it!
 */
int
ay_object_unlink(ay_object *o)
{
 int ay_status = AY_OK;
 int done;
 ay_list_object *lev = ay_currentlevel;
 ay_object *clevel = NULL, *p, *p2;

  clevel = lev->object;

  /* unlink first object of current level? */
  if(clevel == o)
    { /* yes */
      p = lev->next->object;
      p->down = o->next;
      if(ay_next == &(o->next))
	{ /* repair ay_next */
	  ay_next = &(p->down);
	}
      ay_clevel_del();
      ay_clevel_add(o->next);
    }
  else
    { /* no */
      p = clevel;
      p2 = p;
      p = p->next;
      done = AY_FALSE;
      while(p && !done)
	{
	  if(p == o)
	    {
	      p2->next = o->next;
	      done = AY_TRUE;
	      if(ay_next == &(o->next))
		{ /* repair ay_next */
		  ay_next = &(p2->next);
		}

	    } /* if */
	  p2 = p;
	  p = p->next;
	} /* while */
    } /* if */

 return ay_status;
} /* ay_object_unlink */


/* ay_object_gettypename:
 *  return type name that has been registered for
 *  object type index
 */
char *
ay_object_gettypename(unsigned int index)
{
 void **arr = NULL;

  if(index > ay_typenamest.size)
    return NULL;

  arr = ay_typenamest.arr;
  if(!arr)
    return NULL;

 return((char*)arr[index]);
} /* ay_object_gettypename */


/* ay_object_getname:
 *  return object name or (if object is unnamed) name that has
 *  been registered for the object type
 */
char *
ay_object_getname(ay_object *o)
{
 char *name = NULL;

  if(o->type == AY_IDINSTANCE)
    {
      return ay_object_getname((ay_object *)(o->refine));
    }

  if(o->name)
    name = o->name;
  else
    name = ay_object_gettypename(o->type);

 return(name);
} /* ay_object_getname */


/* ay_object_setnametcmd:
 *
 */
int
ay_object_setnametcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 char fname[] = "nameOb";
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "name");
      return TCL_OK;
    }

  o = sel->object;

  if((o == ay_root) || (o->type == AY_IDVIEW))
    return TCL_OK;

  if(o)
    {
      if(o->name)
	free(o->name);
      o->name = NULL;

      /* clear the current name if argument is "" */
      if(strlen(argv[1]) == 0)
	{
	  return TCL_OK;
	}

      if(!(o->name = calloc(strlen(argv[1])+1, sizeof(char))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return TCL_OK;
	}

      strcpy(o->name, argv[1]);
    } /* if */

 return TCL_OK;
} /* ay_object_setnametcmd */


/* ay_object_copy:
 *  copy object src to dst
 */
int
ay_object_copy(ay_object *src, ay_object **dst)
{
 int ay_status = AY_OK;
 char fname[] = "object_copy";
 ay_object *sub = NULL, *sub2 = NULL, *temp = NULL, *new = NULL;
 void **arr = NULL;
 ay_copycb *cb = NULL;
 ay_mat_object *mat = NULL;
 unsigned int *refcountptr;

  if(!src || !dst)
    return AY_ENULL;

  /* copy generic object */
  if(!(new = calloc(1, sizeof(ay_object))))
    return AY_EOMEM;

  *dst = new;

  memcpy(new, src, sizeof(ay_object));
  /* danger! links point to original hierarchy */

  new->next = NULL;
  new->down = NULL;
  new->selp = NULL;
  new->tags = NULL;

  new->refcount = 0;

  /* copy type specific part */
  arr = ay_copycbt.arr;
  cb = (ay_copycb*)(arr[src->type]);
  if(cb)
    ay_status = cb(src->refine, &(new->refine));

  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "copy callback failed");
      free(new);
      return AY_ERROR;
    }

  /* copy name */
  if(src->name)
    {
      if(!(new->name = calloc(strlen(src->name)+1, sizeof(char))))
	{
	  free(new);
	  return AY_EOMEM;
	}

      strcpy(new->name, src->name);
    }

  /* copy tags */
  ay_status = ay_tags_copyall(src, new);

  /* increase material objects refcount */
  if(new->mat)
    {
      mat = new->mat;
      refcountptr = mat->refcountptr;
      (*refcountptr)++;
    }

  /* copy children */
  if(src->down)
    {
      sub = src->down;

      if((ay_status = ay_object_copy(sub, &new->down)))
	return ay_status;
      temp = new->down;
      sub = sub->next;

      while(sub)
	{
	  sub2 = sub->next;
	  if((ay_status = ay_object_copy(sub, &temp->next)))
	    return ay_status;
	  temp = temp->next;

	  sub = sub2;
	} /* while */

    } /* if */

  new->modified = AY_TRUE;
  new->selected = AY_FALSE;

 return AY_OK;
} /* ay_object_copy */


/* ay_object_copymulti:
 *  copy multiple objects (linked via ->next) from src to dst
 */
int
ay_object_copymulti(ay_object *src, ay_object **dst)
{
 int ay_status = AY_OK;

 if(!src || !dst)
   return AY_ENULL;

 while(src)
   {
     ay_status = ay_object_copy(src, dst);
     if(ay_status|| !(*dst))
       {
	 return ay_status;
       }
     else
       {
	 dst = &((*dst)->next);
       }
     src = src->next;
   } /* while */

 return AY_OK;
} /* ay_object_copymulti */


/* ay_object_haschildtcmd:
 *  returns 1 if selected object has a regular (other than
 *  endlevel) child object
 */
int
ay_object_haschildtcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 char fname[] = "hasChild";
 ay_object *o = NULL, *d = NULL;
 ay_list_object *sel = ay_selection;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  o = sel->object;

  if(!o->down)
    {
      Tcl_SetResult(interp, "0", TCL_VOLATILE);
    }
  else
    {
      d = o->down;
      if(d->next)
	{
	  Tcl_SetResult(interp, "1", TCL_VOLATILE);
	}
      else
	{
	  Tcl_SetResult(interp, "0", TCL_VOLATILE);
	}
    }

 return TCL_OK;
} /* ay_object_haschildtcmd */


int
ay_object_gettypetcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 char *typename = NULL;
 char fname[] = "getType";

  /* check args */
  if(argc != 2)
    {
      ay_error(AY_EARGS, fname, "varname");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  o = sel->object;

  if(!o)
    {
      ay_error(AY_ENULL, fname, NULL);
      return TCL_OK;
    }

  Tcl_SetVar(interp, argv[1], "", TCL_LEAVE_ERR_MSG);

  typename = NULL;
  typename = ay_object_gettypename(o->type);
  if(typename)
    {
      Tcl_SetVar(interp, argv[1], typename, TCL_LEAVE_ERR_MSG );
    }
  else
    {
      ay_error(AY_ENULL, fname, NULL);
      return TCL_OK;
    }

 return TCL_OK;
} /* ay_object_gettypetcmd */


int
ay_object_getnametcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 char *name = NULL;
 char fname[] = "getName";

  /* check args */
  if(argc != 2)
    {
      ay_error(AY_EARGS, fname, "varname");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  o = sel->object;

  if(!o)
    {
      ay_error(AY_ENULL, fname, NULL);
      return TCL_OK;
    }

  Tcl_SetVar(interp, argv[1], "", TCL_LEAVE_ERR_MSG);

  name = NULL;
  name = o->name;
  if(name)
    {
      Tcl_SetVar(interp, argv[1], name, TCL_LEAVE_ERR_MSG );
    }

 return TCL_OK;
} /* ay_object_getnametcmd */


/* ay_object_crtendlevel:
 *
 */
int
ay_object_crtendlevel(ay_object **o)
{
 int ay_status = AY_OK;
 char fname[] = "object_crtendlevel";
 ay_object *new = NULL;
 ay_level_object *l = NULL;

  if(!(new = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  ay_status = ay_object_defaults(new);

  new->type = AY_IDLEVEL;

  if(!(l = calloc(1, sizeof(ay_level_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  l->type = AY_LTEND;

  new->refine = l;

  *o = new;

 return ay_status;
} /* ay_object_crtendlevel */


/* ay_object_deleteinstances:
 *  remove all instance objects from objects pointed to by **o,
 *  which will eventually be modified (if *o is an instance);
 *  is able to work with multiple and nested objects!
 */
int
ay_object_deleteinstances(ay_object **o)
{
 int ay_status = AY_OK;
 ay_object **last = NULL, *next = NULL, *co = NULL;

  if(!o || !*o)
    return AY_OK;

  co = *o;
  last = o;
  while(co)
    {
      if(co->down)
	{
	  ay_status = ay_object_deleteinstances(&(co->down));
	} /* if */

      if(co->type == AY_IDINSTANCE)
	{
	  next = co->next;
	  ay_status = ay_object_delete(co);
	  ay_status = ay_undo_clearobj(co);
	  (*last) = next;
	  co = next;
	}
      else
	{
	  last = &(co->next);
	  co = co->next;
	} /* if */
    } /* while */

 return ay_status;
} /* ay_object_deleteinstances */


/* ay_object_replace:
 *  replace object dst with the one pointed to by src;
 *  type specific contents of dst are lost afterwards;
 *  object src should not be accessed via src afterwards
 *  because it is free()d here!
 */
int
ay_object_replace(ay_object *src, ay_object *dst)
{
 int ay_status = AY_OK;
 void **arr = NULL;
 ay_deletecb *dcb = NULL;
 ay_mat_object *oldmat = NULL;
 ay_object *oldnext = NULL;
 int oldrefcount = 0;

  if(!src || !dst)
    return AY_ENULL;

  oldmat = dst->mat;
  oldrefcount = dst->refcount;
  oldnext = dst->next;

  if(dst->down)
    {
      ay_object_deletemulti(dst->down);
    }

  if(dst->tags)
    {
      ay_status = ay_tags_delall(dst);
    }

  if(dst->name)
    {
      free(dst->name);
    }

  if(dst->selp)
    {
      ay_selp_clear(dst);
    }

  arr = ay_deletecbt.arr;
  dcb = (ay_deletecb*)(arr[dst->type]);
  if(dcb)
    ay_status = dcb(dst->refine);

  memcpy(dst, src, sizeof(ay_object));

  dst->mat = oldmat;
  dst->refcount = oldrefcount;
  dst->next = oldnext;

  free(src);

 return AY_OK;
} /* ay_object_replace */


/* ay_object_ccp:
 *  this function calls all functions that cleanup the caches of
 *  pointers to the scene hierarchy held by other parts of the core
 */
int
ay_object_ccp(ay_object *o)
{
 int ay_status = AY_OK;

  /* in case direct point editing is active:
   *  clear the point cache of the direct point edit action
   */
  ay_pact_pedclear(o);

 return ay_status;
} /* ay_object_ccp */


/* ay_object_count:
 *  this function counts all objects pointed to by o (including all siblings
 *  and all children of o and its siblings) in a recursive manner
 */
unsigned int
ay_object_count(ay_object *o)
{
 unsigned int lcount = 0;

  if(!o)
    return 0;

  while(o->next)
    {
      if(o->down && o->down->next)
	lcount += ay_object_count(o->down);
      lcount++;
      o = o->next;
    }

 return lcount;
} /* ay_object_count */
