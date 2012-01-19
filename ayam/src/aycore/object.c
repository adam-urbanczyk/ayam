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

/* object.c - general object management*/

/* ay_object_defaults:
 *  reset object attributes of ay_object <o> to safe default settings
 */
void
ay_object_defaults(ay_object *o)
{
  if(!o)
    return;

  o->quat[3] = 1.0;

  o->scalx = 1.0;
  o->scaly = 1.0;
  o->scalz = 1.0;

  o->inherit_trafos = AY_TRUE;

 return;
} /* ay_object_defaults */


/* ay_object_create:
 *  create an object by calling the object type specific create callback
 */
int
ay_object_create(unsigned int index, ay_object **o)
{
 int ay_status = AY_OK;
 char fname[] = "object_create";
 ay_voidfp *arr = NULL;
 ay_createcb *cb = NULL;
 ay_object *new = NULL;

  arr = ay_createcbt.arr;

  if(!(new = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  ay_object_defaults(new);

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


/* ay_object_createargs:
 *  create an object by calling the object type specific create callback
 *  with user provided arguments
 */
int
ay_object_createargs(unsigned int index, int argc, char **argv, ay_object **o)
{
 int ay_status = AY_OK;
 char fname[] = "object_createargs";
 ay_voidfp *arr = NULL;
 ay_createcb *cb = NULL;
 ay_object *new = NULL;

  arr = ay_createcbt.arr;

  if(!(new = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  ay_object_defaults(new);

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


/* ay_object_createtcmd:
 *  Implements the \a crtOb scripting interface command.
 *  See also the corresponding section in the \ayd{sccrtob}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_object_createtcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_voidfp *arr = NULL;
 unsigned int index;
 Tcl_HashEntry *entry = NULL;
 ay_createcb *cb = NULL;
 ay_object *o = NULL;
 char *a = "ay", *n = "sc", *v = "1";

  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "typename [args]!");
      return TCL_OK;
    }

  entry = Tcl_FindHashEntry(&ay_otypesht, argv[1]);
  if(!entry)
    {
      ay_error(AY_ENTYPE, argv[0], argv[1]);
      return TCL_OK;
    }

  index = (unsigned int) Tcl_GetHashValue(entry);

  if(!(o = calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, argv[0], NULL);
      return TCL_OK;
    }

  o->type = index;
  ay_object_defaults(o);

  arr = ay_createcbt.arr;
  cb = (ay_createcb *)(arr[index]);
  if(cb)
    ay_status = cb(argc, argv, o);

  if(ay_status)
    {
      ay_error(ay_status, argv[0], "Create callback failed!");
      free(o);
      return TCL_OK;
    }

  /* for potential parent objects, create endlevel terminator */
  if(o->parent && (!o->down))
    {
      o->down = ay_endlevel;
    }

  /* link object to scene structure */
  ay_object_link(o);

  /* set scene changed flag */
  Tcl_SetVar2(interp, a, n, v, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

 return TCL_OK;
} /* ay_object_createtcmd */


/* ay_object_delete:
 *  delete an object
 *  does not unlink the object!
 */
int
ay_object_delete(ay_object *o)
{
 int ay_status = AY_OK;
 ay_voidfp *arr = NULL;
 ay_deletecb *cb = NULL;
 ay_object *down = NULL, *d = NULL;
 ay_mat_object *mat = NULL;
 unsigned int *refcountptr;
 ay_tag *tag = NULL;

  if(!o)
    return AY_ENULL;

  if(o->refcount > 0)
    return AY_EREF;

  /* never delete the one and only end level object */
  if(o == ay_endlevel)
    return AY_OK;

  /* delete children first */
  if(o->down && (o->down != ay_endlevel))
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

  /* see if other objects point to us via NO tags; remove those tags */
  tag = o->tags;
  while(tag)
    {
      if(tag->type == ay_nm_tagtype)
	{
	  d = (ay_object*)(((ay_btval*)tag->val)->payload);
	  tag = tag->next;
	  ay_tags_remnonm(o, d);
	}
      else
	{
	  tag = tag->next;
	}
    } /* while */

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


/* ay_object_deletemulti:
 *  delete multiple objects connected via their ->next fields
 */
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


/* ay_object_deletetcmd:
 *  Delete selected objects.
 *  Implements the \a delOb scripting interface command.
 *  See also the corresponding section in the \ayd{scdelob}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_object_deletetcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection, *try_again = NULL, **next_try_again, *t;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  next_try_again = &(try_again);

  while(sel)
    {
      o = sel->object;
      if(o == ay_root)
	{
	  ay_error(AY_ERROR, argv[0], "Root object may not be deleted!");
	}
      else
	{
	  ay_status = ay_object_unlink(o);

	  ay_undo_clearobj(o);

	  ay_status = ay_object_delete(o);
	  if(ay_status)
	    {
	      /* could not delete the object, probably due to
		 reference counter not zero; add the object
		 to the try_again list and continue deleting
		 objects in the hope that the user selected _all_
		 instances and we can delete the master anyway
		 in a second attempt */
	      if(!(*next_try_again = calloc(1, sizeof(ay_list_object))))
		{
		  ay_sel_free(AY_FALSE);
		  ay_error(AY_EOMEM, argv[0], NULL);
		  return TCL_OK;
		}
	      (*next_try_again)->object = o;
	      next_try_again = &((*next_try_again)->next);
	    } /* if */

	} /* if */
      sel = sel->next;
    } /* while */

  /* free selection, we _must_ do it now, because otherwise
   * the ay_error() below can lead to the processing of Tcl
   * events and a redraw that includes iterating over the
   * list of selected objects that we just free()d => bummer! */
  ay_sel_free(AY_FALSE);

  while(try_again)
    {
      ay_status = ay_object_delete(try_again->object);
      if(ay_status)
	{
	  ay_error(ay_status, argv[0], NULL);
	  ay_object_link(try_again->object);
	}
      t = try_again->next;
      free(try_again);
      try_again = t;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_object_deletetcmd */


/* ay_object_link:
 *  link object <o> to scene structure
 *  uses ay_next
 *  properly maintains ay_next and ay_currentlevel
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

  /* just in case that we are linking the very first object to
     an empty sub-level (ay_currentlevel points to the end-level object)
     we need to correct ay_currentlevel to point to the new object instead */
  if(ay_currentlevel->object != ay_root)
    {
      ay_clevel_del();
      ay_clevel_add(ay_currentlevel->object->down);
    }

 return ay_status;
} /* ay_object_link */


/* ay_object_unlink:
 *  unlink object o from scene, without deleting it!
 *  properly maintains ay_next and ay_currentlevel
 */
int
ay_object_unlink(ay_object *o)
{
 int ay_status = AY_OK;
 int done;
 ay_list_object *clevel = ay_currentlevel;
 ay_object *clevelobj = NULL, *p1, *p2;

  clevelobj = clevel->object;

  /* unlink first object of current level? */
  if(clevelobj == o)
    { /* yes */
      p1 = clevel->next->object;
      p1->down = o->next;
      if(ay_next == &(o->next))
	{
	  /* repair ay_next */
	  ay_next = &(p1->down);
	}
      ay_clevel_del();
      ay_clevel_add(o->next);
    }
  else
    { /* no */
      p1 = clevelobj;
      p2 = p1;
      p1 = p1->next;
      done = AY_FALSE;
      while(p1 && !done)
	{
	  if(p1 == o)
	    {
	      p2->next = o->next;
	      done = AY_TRUE;
	      if(ay_next == &(o->next))
		{
		  /* repair ay_next */
		  ay_next = &(p2->next);
		}
	    } /* if */
	  p2 = p1;
	  p1 = p1->next;
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
 Tcl_HashEntry *entry = NULL;

  if((entry = Tcl_FindHashEntry(&ay_typenamesht, (char*)index)))
    {
      return Tcl_GetHashValue(entry);
    }
  else
    {
      return NULL;
    }
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

 return name;
} /* ay_object_getname */


/* ay_object_setnametcmd:
 *  Implements the \a nameOb scripting interface command.
 *  See also the corresponding section in the \ayd{scnameob}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_object_setnametcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "name");
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
	  ay_error(AY_EOMEM, argv[0], NULL);
	  return TCL_OK;
	}

      strcpy(o->name, argv[1]);
    } /* if */

 return TCL_OK;
} /* ay_object_setnametcmd */


/* ay_object_copy:
 *  copy object src to dst
 *  this is a deep copy!
 *  tags, material, attributes, and transformations are copied as well
 */
int
ay_object_copy(ay_object *src, ay_object **dst)
{
 int ay_status = AY_OK;
 char fname[] = "object_copy";
 ay_object *sub = NULL, *new = NULL;
 ay_object **next = NULL;
 ay_voidfp *arr = NULL;
 ay_copycb *cb = NULL;
 ay_mat_object *mat = NULL;
 unsigned int *refcountptr;

  if(!src || !dst)
    return AY_ENULL;

  /* refuse to copy the endlevel terminator */
  if(src == ay_endlevel)
    {
      *dst = ay_endlevel;
    }

  /* copy generic object */
  if(!(new = calloc(1, sizeof(ay_object))))
    return AY_EOMEM;

  *dst = new;

  memcpy(new, src, sizeof(ay_object));
  /* danger! links point to original hierarchy */

  new->next = NULL;
  if(src->down != ay_endlevel)
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
  if(src->down && (src->down != ay_endlevel))
    {
      sub = src->down;
      next = &(new->down);
      while(sub)
	{
	  if((ay_status = ay_object_copy(sub, next)))
	    return ay_status;
	  next = &((*next)->next);

	  sub = sub->next;
	} /* while */
    }

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
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  if(!o->down)
    {
      Tcl_SetResult(interp, "0", TCL_VOLATILE);
    }
  else
    {
      if(o->down != ay_endlevel)
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

  /* check args */
  if(argc != 2)
    {
      ay_error(AY_EARGS, argv[0], "varname");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  if(!o)
    {
      ay_error(AY_ENULL, argv[0], NULL);
      return TCL_OK;
    }

  Tcl_SetVar(interp, argv[1], "", TCL_LEAVE_ERR_MSG);

  typename = NULL;
  typename = ay_object_gettypename(o->type);
  if(typename)
    {
      Tcl_SetVar(interp, argv[1], typename, TCL_LEAVE_ERR_MSG);
    }
  else
    {
      ay_error(AY_ENULL, argv[0], NULL);
      return TCL_OK;
    }

 return TCL_OK;
} /* ay_object_gettypetcmd */


/* ay_object_getnametcmd:
 *  Implements the \a getName scripting interface command.
 *  See also the corresponding section in the \ayd{scgetname}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_object_getnametcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 char *name = NULL;

  /* check args */
  if(argc != 2)
    {
      ay_error(AY_EARGS, argv[0], "varname");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  if(!o)
    {
      ay_error(AY_ENULL, argv[0], NULL);
      return TCL_OK;
    }

  Tcl_SetVar(interp, argv[1], "", TCL_LEAVE_ERR_MSG);

  name = NULL;
  name = o->name;
  if(name)
    {
      Tcl_SetVar(interp, argv[1], name, TCL_LEAVE_ERR_MSG);
    }

 return TCL_OK;
} /* ay_object_getnametcmd */


/* ay_object_crtendlevel:
 *
 */
int
ay_object_crtendlevel(ay_object **o)
{
  if(!o)
    return AY_ENULL;
 *o = ay_endlevel;
 return AY_OK;
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
      if(co->down && co->down->next)
	{
	  ay_status = ay_object_deleteinstances(&(co->down));
	} /* if */

      if(co->type == AY_IDINSTANCE)
	{
	  next = co->next;
	  ay_undo_clearobj(co);
	  ay_status = ay_object_delete(co);
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
 ay_voidfp *arr = NULL;
 ay_deletecb *dcb = NULL;
 ay_mat_object *oldmat = NULL;
 ay_object *oldnext = NULL, *d = NULL;
 int oldrefcount = 0;
 ay_tag *tag = NULL;

  if(!src || !dst)
    return AY_ENULL;

  oldmat = dst->mat;
  oldrefcount = dst->refcount;
  oldnext = dst->next;

  if(dst->down && dst->down->next)
    {
      ay_object_deletemulti(dst->down);
    }

  if(dst->tags)
    {
      /* see if other objects point to dst via NO tags; remove those tags */
      tag = dst->tags;
      while(tag)
	{
	  if(tag->type == ay_nm_tagtype)
	    {
	      d = (ay_object*)(((ay_btval*)tag->val)->payload);
	      tag = tag->next;
	      ay_tags_remnonm(dst, d);
	    }
	  else
	    {
	      tag = tag->next;
	    }
	} /* while */

      /* delete all tags */
      ay_tags_delall(dst);
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

  if(ay_status)
    return ay_status;

  memcpy(dst, src, sizeof(ay_object));

  dst->mat = oldmat;
  dst->refcount = oldrefcount;
  dst->next = oldnext;

  free(src);

 return AY_OK;
} /* ay_object_replace */


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


/* ay_object_candelete:
 *  _recursively_ check, whether there are referenced/master objects
 *  in the object hierarchy <p>/<o> and whether any references/instances
 *  of those masters are _not_ in this hierarchy. If this is the case,
 *  <o> must not be deleted.
 */
int
ay_object_candelete(ay_object *p, ay_object *o)
{
 int ay_status = AY_OK;
 unsigned int refs;

  if(!p || !o)
    return AY_ENULL;

  while(o && o->next)
    {
      if(o->down && o->down->next)
	{
	  /* recurse into children */
	  ay_status = ay_object_candelete(p, o->down);

	  /* immediately return a negative result */
	  if(ay_status)
	    return ay_status;
	}

      if(o->refcount)
	{
	  refs = 0;
	  ay_instt_countrefs(p, o, &refs);
	  if(o->refcount > refs)
	    return AY_ERROR;
	}
      o = o->next;
    } /* while */

 return ay_status;
} /* ay_object_candelete */
