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


/* ay_object_placemark:
 *  move object to the mark
 */
void
ay_object_placemark(ay_object *o)
{
 ay_list_object tsel = {0}, *osel;

  if(!o)
    return;

  /* move object to the mark? */
  if(ay_prefs.createatmark && ay_currentview && ay_currentview->drawmark)
    {
      /* fake single object selection for snaptomarkcb() */
      osel = ay_selection;
      tsel.object = o;
      ay_selection = &tsel;

      ay_pact_snaptomarkcb(ay_currentview->togl, -1, NULL);
      ay_selection = osel;
    }

 return;
} /* ay_object_placemark */


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
      ay_error(AY_EARGS, argv[0], "typename [args]");
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

  if(!o->parent)
    {
      ay_object_placemark(o);
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
 *  _recursively_ delete the children of an object and then the object;
 *  fails if there are objects/children with refcount > 0;
 *  does not unlink the object!
 */
int
ay_object_delete(ay_object *o)
{
 int ay_status = AY_OK;
 ay_voidfp *arr = NULL;
 ay_deletecb *cb = NULL;
 ay_object *d = NULL;
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
      while(o->down)
	{
	  d = o->down;
	  /* unlink the first child and try to delete it */
	  o->down = d->next;
	  ay_status = ay_object_delete(d);
	  if(ay_status)
	    {
	      /* delete failed, re-link the child */
	      o->down = d;
	      return ay_status;
	    } /* if */
	} /* while */
    } /* if */

  if(o->refine)
    {
      arr = ay_deletecbt.arr;
      cb = (ay_deletecb *)(arr[o->type]);
      if(cb)
	ay_status = cb(o->refine);

      if(ay_status)
	return ay_status;
    }

  /* delete selected points */
  if(o->selp)
    {
      ay_selp_clear(o);
    }

  /* see if other objects point to us via NO tags; remove those tags */
  tag = o->tags;
  while(tag)
    {
      if(tag->type == ay_nm_tagtype && tag->is_binary)
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
 *
 *  If the \a force argument is AY_FALSE (0) and the hierarchy contains
 *  undeletable objects (e.g. due to references), the removal process
 *  will be stopped and the object hierarchy will be in unknown state.
 *  This variant should only be used, when there are no referenced
 *  objects in \a o, otherwise memory may leak.
 *
 *  If the \a force argument is AY_TRUE (1), the removal of master objects
 *  will be enforced by setting the reference counts to 0 before deletion.
 *  Furthermore, all internal references will be removed beforehand via
 *  instt_removeinstances() and matt_removeallrefs().
 *  This variant should only be used after a successful call to
 *  object_candelete(),
 *  otherwise access to freed memory/crashes can occur later (via the
 *  references)!
 *
 *  If the \a force argument is 2, the removal of master objects will be
 *  enforced by setting the reference counts to 0 before deletion.
 *  There will be _no_ calls to instt_removeinstances() or
 *  matt_removeallrefs()!
 *  The object hierarchy should either have no references and no objects
 *  with materials (i.e. already have been cleaned by instt_removeinstances()
 *  and matt_removeallrefs()) or the objects must be sorted in a way that
 *  all references are deleted before the respective master/material objects.
 *  Otherwise crashes can occur while removing (use after free()).
 *  This variant should only be used after a successful call to
 *  object_candelete(),
 *  otherwise access to freed memory/crashes can occur later (via the
 *  references)!
 */
int
ay_object_deletemulti(ay_object *o, int force)
{
 int ay_status = AY_OK;
 ay_object *next = NULL, *d = NULL;

  if(!o)
    return AY_ENULL;

  d = o;

  if(force == AY_TRUE)
    {
      ay_instt_removeinstances(&(d), NULL);
      ay_matt_removeallrefs(d);
      force = 2;
    }

  while(d)
    {
      if(force)
	{
	  if(d->down && d->down->next)
	    {
	      (void)ay_object_deletemulti(d->down, force);
	      d->down = NULL;
	    }
	  d->refcount = 0;
	}

      next = d->next;
      ay_status = ay_object_delete(d);
      if(!force && ay_status)
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
 ay_object *o = NULL, *oldnext;
 ay_list_object *sel, *try_again = NULL, **next_try_again, *t;
 ay_mat_object *m = NULL;

  if(!ay_selection)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  /* remove all references of the objects to be removed
     in order to avoid illegal memory accesses; should e.g.
     a material be removed first and later an object of that
     material decreases the reference counter of the removed
     material object while being deleted, an illegal memory
     access ensues */
  sel = ay_selection;
  while(sel)
    {
      o = sel->object;
      if(o->down && o->down->next)
	{
	  ay_instt_removeinstances(&(o->down), NULL);
	  ay_matt_removeallrefs(o->down);
	}
      if(o->mat)
	{
	  m = o->mat;
	  o->mat = NULL;
	  if(m->refcountptr)
	    (*(m->refcountptr))--;
	}
      sel = sel->next;
    }

  /* check for referenced objects that can not be deleted */
  sel = ay_selection;
  if(ay_object_candeletelist(sel, NULL) != AY_OK)
    {
      while(sel)
	{
	  /* temporarily isolate the object so that clearclipboard()
	     does not browse through too much of the scene */
	  oldnext = sel->object->next;
	  sel->object->next = NULL;
	  ay_instt_clearclipboard(sel->object);
	  sel->object->next = oldnext;
	  sel = sel->next;
	}
      sel = ay_selection;
      if(ay_object_candeletelist(sel, NULL) != AY_OK)
	{
	  while(sel)
	    {
	      ay_instt_removeinstances(&(sel->object->down), NULL);
	      sel = sel->next;
	    }
	}
    }

  next_try_again = &(try_again);
  sel = ay_selection;
  while(sel)
    {
      o = sel->object;
      if(o == ay_root)
	{
	  ay_error(AY_ERROR, argv[0], "Root object may not be deleted!");
	}
      else
	{
	  ay_object_unlink(o);

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
		  while(try_again)
		    {
		      t = try_again->next;
		      free(try_again);
		      try_again = t;
		    } /* while */
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
 *  link single object <o> to scene structure;
 *  uses ay_next;
 *  properly maintains ay_next and ay_currentlevel
 */
void
ay_object_link(ay_object *o)
{

  if(ay_next)
    {
      o->next = *ay_next;

      *ay_next = o;

      ay_next = &(o->next);
    }

  /* just in case that we are linking the very first object to
     an empty sub-level (ay_currentlevel points to the end-level object)
     we need to correct ay_currentlevel to point to the new object instead */
  if(ay_currentlevel && ay_currentlevel->object != ay_root)
    {
      if(ay_currentlevel->next && ay_currentlevel->next->object)
	ay_clevel_set(ay_currentlevel->next->object->down);
    }

 return;
} /* ay_object_link */


/* ay_object_unlink:
 *  unlink object \a o from scene, without deleting it!
 *  \a o must be in the current level;
 *  properly maintains ay_next and ay_currentlevel
 */
void
ay_object_unlink(ay_object *o)
{
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
      ay_clevel_set(o->next);
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

 return;
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

  if((o == ay_root) || (o->type == AY_IDVIEW) || (o->type == AY_IDMATERIAL))
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

      if(!(o->name = malloc(strlen((argv[1])+1) * sizeof(char))))
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

  /* silently avoid to really copy the endlevel terminator object;
     instead, just copy the reference */
  if(src == ay_endlevel)
    {
      *dst = ay_endlevel;
      return AY_OK;
    }

  /* copy generic object */
  if(!(new = malloc(sizeof(ay_object))))
    return AY_EOMEM;

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
      if(!(new->name = malloc((strlen(src->name)+1) * sizeof(char))))
	{
	  free(new);
	  return AY_EOMEM;
	}

      strcpy(new->name, src->name);
    }

  /* copy tags */
  ay_status = ay_tags_copyall(src, new);

  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "copy tags failed");
      if(new->name)
	free(new->name);
      free(new);
      return AY_ERROR;
    }

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
	    {
	      ay_object_delete(new);
	      return ay_status;
	    }
	  next = &((*next)->next);

	  sub = sub->next;
	} /* while */
    }

  new->modified = AY_TRUE;
  new->selected = AY_FALSE;

  *dst = new;

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
      if(ay_status || !(*dst))
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
 char *res, no[] = "0", yes[] = "1";

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;

      if(o->down && o->down != ay_endlevel)
	res = yes;
      else
	res = no;

      if(ay_selection->next)
	Tcl_AppendElement(interp, res);
      else
	Tcl_SetResult(interp, res, TCL_VOLATILE);

      sel = sel->next;
    }

 return TCL_OK;
} /* ay_object_haschildtcmd */


/* ay_object_gettypeornametcmd:
 *  Implements the \a getType scripting interface command.
 *  Also implements the \a getName scripting interface command.
 *  See also the corresponding section in the \ayd{scgettype}.
 *  See also the corresponding section in the \ayd{scgetname}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_object_gettypeornametcmd(ClientData clientData, Tcl_Interp *interp,
			    int argc, char *argv[])
{
 ay_object *o = NULL;
 ay_list_object *sel = ay_selection;
 char *typeorname = NULL;

  /* check args */
  if(argc != 2)
    {
      ay_error(AY_EARGS, argv[0], "varname");
      return TCL_OK;
    }

  Tcl_SetVar(interp, argv[1], "", TCL_LEAVE_ERR_MSG);

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;

      if(!o)
	{
	  ay_error(AY_ENULL, argv[0], NULL);
	  return TCL_OK;
	}

      if(argv[0][3] == 'N')
	{
	  typeorname = o->name;
	}
      else
	{
	  typeorname = ay_object_gettypename(o->type);
	}

      if(typeorname)
	{
	  Tcl_SetVar(interp, argv[1], typeorname, TCL_APPEND_VALUE |
		     TCL_LIST_ELEMENT);
	}
      else
	{
	  if(argv[0][3] == 'N')
	    {
	      ay_error(AY_EWARN, argv[0], "Object has no name.");
	    }
	  else
	    {
	      ay_error(AY_ENULL, argv[0], NULL);
	      return TCL_OK;
	    }
	}
      sel = sel->next;
    }

 return TCL_OK;
} /* ay_object_gettypeornametcmd */


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
 char fname[] = "replace";
 ay_voidfp *arr = NULL;
 ay_deletecb *dcb = NULL;
 ay_mat_object *oldmat = NULL;
 ay_object *oldnext = NULL, *d = NULL;
 ay_tag *tag = NULL;
 int oldrefcount = 0;

  if(!src || !dst)
    return AY_ENULL;

  oldmat = dst->mat;
  oldrefcount = dst->refcount;
  oldnext = dst->next;

  if(dst->down && dst->down->next)
    {
      ay_status = ay_object_candelete(dst->down, dst->down);
      if(ay_status != AY_OK)
	{
	  ay_clipb_prepend(dst->down, fname);
	}
      else
	{
	  (void)ay_object_deletemulti(dst->down, AY_TRUE);
	}
    }

  if(dst->tags)
    {
      /* see if other objects point to dst via NO tags; remove those tags */
      tag = dst->tags;
      while(tag)
	{
	  if(tag->type == ay_nm_tagtype && tag->is_binary)
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
 *  The terminating endlevel objects are not included.
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
 *  in the hierarchy pointed to by <o> and whether any references/instances
 *  of those masters are _not_ in the hierarchy pointed to by <h>.
 *  If this is the case, <o> must not be deleted.
 */
int
ay_object_candelete(ay_object *h, ay_object *o)
{
 int ay_status = AY_OK;
 unsigned int refs;

  if(!h || !o)
    return AY_ENULL;

  while(o && o->next)
    {
      if(o->down && o->down->next)
	{
	  /* recurse into children */
	  ay_status = ay_object_candelete(h, o->down);

	  /* immediately return a negative result */
	  if(ay_status)
	    return ay_status;
	}

      if(o->refcount)
	{
	  refs = 0;
	  ay_instt_countrefs(h, o, &refs);
	  if(o->refcount > refs)
	    return AY_ERROR;
	}
      o = o->next;
    } /* while */

 return ay_status;
} /* ay_object_candelete */


/* ay_object_candeletelist:
 *  _recursively_ check, whether there are referenced/master objects
 *  in the list/hierarchy pointed to by <l> and whether any
 *  references/instances of those masters are _not_ in the hierarchy
 *  pointed to by <l>.
 *  If this is the case, <l> must not be deleted.
 *  For the initial invocation o must be NULL.
 */
int
ay_object_candeletelist(ay_list_object *l, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *m;
 unsigned int refs;

  if(!l)
    return AY_ENULL;

  if(o)
    {
      /* worker invocation, check all objects pointed to by o */
      while(o && o->next)
	{
	  if(o->down && o->down->next)
	    {
	      /* recurse into children */
	      ay_status = ay_object_candeletelist(l, o->down);

	      /* immediately return a negative result */
	      if(ay_status)
		return ay_status;
	    }

	  if(o->refcount)
	    {
	      refs = 0;
	      ay_instt_countrefslist(l, o, &refs);
	      if(o->refcount > refs)
		return AY_ERROR;
	    }

	  o = o->next;
	} /* while */
    }
  else
    {
      /* top level invocation, just iterate the list, check
	 the objects directly pointed to by it, and if those
	 objects have children, fire off a recursive worker */
      while(l)
	{
	  m = l->object;

	  if(m->refcount)
	    {
	      refs = 0;
	      ay_instt_countrefslist(l, m, &refs);
	      if(m->refcount > refs)
		return AY_ERROR;
	    }

	  if(m->down && m->down->next)
	    {
	      ay_status = ay_object_candeletelist(l, m->down);

	      /* immediately return a negative result */
	      if(ay_status)
		return ay_status;
	    }

	  l = l->next;
	} /* while */
    } /* if */

 return ay_status;
} /* ay_object_candeletelist */
