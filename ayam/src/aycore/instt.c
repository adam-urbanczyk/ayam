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

/* instt.c - instancing tools - instance object helpers */

static Tcl_HashTable ay_instt_oidptr_ht;

/* ay_instt_createoidht:
 *  _recursively_ create object id hashtable, which links
 *  object ids to object pointers
 *  if called with NULL, clears the object id hashtable
 *  returns AY_OK on success, AY_ERROR on error (OID already exists)
 */
int
ay_instt_createoidht(ay_object *o)
{
 int ay_status = AY_OK;
 int new_item = 0;
 int found = AY_FALSE;
 Tcl_HashEntry *entry = NULL;
 ay_tag *tag = NULL;
 Tcl_HashTable *ht = &ay_instt_oidptr_ht;

  if(!o)
    {
      Tcl_DeleteHashTable(ht);
      Tcl_InitHashTable(ht, TCL_STRING_KEYS);
      return AY_OK;
    }

  while(o)
    {
      if(o->type != AY_IDINSTANCE)
	{

	  found = AY_FALSE;
	  tag = o->tags;
	  while(tag && !found)
	    {
	      if(tag->type == ay_oi_tagtype)
		{
		  if((entry = Tcl_FindHashEntry(ht, tag->val)))
		    return AY_ERROR; /* Oops, OID already registered? */

		  entry = Tcl_CreateHashEntry(ht, tag->val, &new_item);
		  Tcl_SetHashValue(entry, (char*)o);

		  found = AY_TRUE;
		}

	      tag = tag->next;
	    } /* while */
	} /* if */

      /* recurse into children of o */
      if(o->down)
	{
	  ay_status = ay_instt_createoidht(o->down);
	  if(ay_status)
	    return ay_status;
	}

      o = o->next;
    } /* while */

 return ay_status;
} /* ay_instt_createoidht */


/* ay_instt_connect:
 *  _recursively_ connect instance objects to the appropriate
 *  master objects (using OI tags and the object
 *  id hashtable created by ay_instt_createoidht() above)
 *  returns AY_OK on success, AY_ERROR on error (master not found)
 *  to avoid crashes, unconnected instances will be removed
 *  immediately
 */
void
ay_instt_connect(ay_object *o, ay_object **last)
{
 char fname[] = "instt_connect", errmsg[] = "Could not re-connect instance!";
 int found = AY_FALSE, removed = AY_FALSE;
 Tcl_HashEntry *entry = NULL;
 ay_tag *tag = NULL;
 ay_object *target = NULL;
 Tcl_HashTable *ht = &ay_instt_oidptr_ht;

  if(!o)
    return;

  while(o)
    {
      removed = AY_FALSE;
      if((o->type == AY_IDINSTANCE) && (!o->refine))
	{
	  found = AY_FALSE;
	  tag = o->tags;
	  while(tag && !found)
	    {
	      if(tag->type == ay_oi_tagtype)
		{
		  if(!(entry = Tcl_FindHashEntry(ht, tag->val)))
		    {
		      /* OID not registered? */
		      ay_error(AY_ERROR, fname, errmsg);
		      *last = o->next;
		      (void)ay_object_delete(o);
		      removed = AY_TRUE;
		      o = *last;
		    }
		  else
		    {
		      target = (ay_object *)Tcl_GetHashValue(entry);
		      o->refine = target;
		      if(target)
			target->refcount++;
		    } /* if */
		  found = AY_TRUE;
		} /* if */

	      tag = tag->next;
	    } /* while */

	  if(!found)
	    {
	      /* instance without OID tag? */
	      ay_error(AY_ERROR, fname, errmsg);
	      *last = o->next;
	      (void)ay_object_delete(o);
	      removed = AY_TRUE;
	      o = *last;
	    }
	} /* if */

      if(!removed)
	{
	  if(o->down && o->down->next)
	    {
	      ay_instt_connect(o->down, &(o->down));
	    }
	  last = &(o->next);
	  o = o->next;
	} /* if */
    } /* while */

 return;
} /* ay_instt_connect */


/* ay_instt_createoid:
 *  create an object id string for use within
 *  OI tags
 *  resets counter if parameter dest is NULL
 */
int
ay_instt_createoid(char **dest)
{
 static unsigned int counter = 0;

  if(!dest)
    {
      counter = 0;
      return AY_OK;
    }
  else
    {
      counter++;
    }

  /*
    TeaMan, 24.3.1999 on #AmigaGER (IRCNet):
    "32 chars sind genug"
    "es geht ja wohl um Praxisanwendung und nicht
    um akademische Betrachtungen" ;)
  */

  if(!(*dest = calloc(32, sizeof(char))))
    return AY_EOMEM;

  sprintf(*dest, "%u", counter);

 return AY_OK;
} /* ay_instt_createoid */


/* ay_instt_createorigids:
 *  _recursively_ create object id tags for all original or
 *  master or referenced objects
 */
int
ay_instt_createorigids(ay_object *o)
{
 int ay_status = AY_OK;
 char *tname = NULL, *tval = NULL;
 int found = AY_FALSE;
 ay_tag *tag = NULL, *newtag = NULL;

  while(o)
    {
      if((o->refcount) && (o->type != AY_IDMATERIAL))
	{
	  tname = NULL;
	  tval = NULL;

	  ay_status = ay_instt_createoid(&tval);

	  if(ay_status)
	    return ay_status;

	  found = AY_FALSE;
	  tag = o->tags;
	  while(tag && !found)
	    {
	      if(tag->type == ay_oi_tagtype)
		{
		  free(tag->val);
		  tag->val = tval;
		  found = AY_TRUE;
		}
	      tag = tag->next;
	    }

	  if(!found)
	    {
	      if(!(tname = calloc(3, sizeof(char))))
		{ free(tval); return AY_EOMEM; }

	      strcpy(tname,"OI");
	      if(!(newtag = calloc(1, sizeof(ay_tag))))
		{ free(tval); free(tname); return AY_EOMEM; }

	      newtag->name = tname;
	      newtag->val = tval;
	      newtag->type = ay_oi_tagtype;
	      newtag->next = o->tags;
	      o->tags = newtag;
	    }
	} /* if */

      if(o->down)
	ay_status = ay_instt_createorigids(o->down);

      if(ay_status)
	return ay_status;

      o = o->next;
    } /* while */

 return ay_status;
} /* ay_instt_createorigids */


/* ay_instt_createinstanceids:
 *  create object id tags for all instance objects
 */
int
ay_instt_createinstanceids(ay_object *o)
{
 int ay_status = AY_OK;
 int found = AY_FALSE;
 ay_tag *tag = NULL, *origtag = NULL;
 ay_object *orig = NULL;

  while(o)
    {
      if(o->type == AY_IDINSTANCE)
	{
	  orig = (ay_object *) o->refine;

	  found = AY_FALSE;
	  tag = orig->tags;
	  while(tag && !found)
	    {
	      if(tag->type == ay_oi_tagtype)
		{
		  origtag = tag;
		  found = AY_TRUE;
		}
	      tag = tag->next;
	    }

	  if(!found)
	    return AY_ERROR; /* This should never happen! */

	  found = AY_FALSE;
	  tag = o->tags;
	  while(tag && !found)
	    {
	      if(tag->type == ay_oi_tagtype)
		{
		  /* copy val from origtag to oitag of instance object */
		  free(tag->val);
		  if(!(tag->val=malloc((strlen(origtag->val)+1)*sizeof(char))))
		    return AY_EOMEM;
		  strcpy(tag->val, origtag->val);

		  found = AY_TRUE;
		}
	      tag = tag->next;
	    }

	  if(!found)
	    {
	      /* need to create a new tag object */
	      ay_status = ay_tags_copy(origtag, &tag);

	      if(ay_status)
		return ay_status;

	      tag->next = o->tags;
	      o->tags = tag;
	    }
	} /* if instance */

      if(o->down)
	{
	  ay_status = ay_instt_createinstanceids(o->down);

	  if(ay_status)
	    return ay_status;
	}
      o = o->next;
    } /* while */

 return ay_status;
} /* ay_instt_createinstanceids */


/* ay_instt_wribiarchives:
 *  wrib instance archives
 */
int
ay_instt_wribiarchives(char *file, ay_object *o)
{
 int ay_status = AY_OK;
 int found = AY_FALSE;
 int down_is_prim = AY_FALSE;
 ay_object *down = NULL;
 ay_tag *tag = NULL;
 char *iafilename = NULL, *ext = NULL;
 ay_voidfp *arr = NULL;
 ay_wribcb *cb = NULL;
 ay_level_object *l = NULL;
 char *parname = "name";

  while(o->next)
    {
      arr = ay_wribcbt.arr;
      cb = (ay_wribcb *)(arr[o->type]);

      if((o->refcount) && (o->type != AY_IDMATERIAL) && (cb))
	{
	  found = AY_FALSE;
	  tag = o->tags;
	  while(tag && !found)
	    {
	      if(tag->type == ay_oi_tagtype)
		{

		  /* create filename */
		  if(!file)
		    {
		      RiBegin(tag->val);
		    }
		  else
		    {
		      if(!(iafilename = malloc((strlen(tag->val)+
						strlen(file)+2)*sizeof(char))))
			return AY_EOMEM;
		      strcpy(iafilename, file);
		      ext = strrchr(iafilename, '.');
		      if(ext)
			{
			  *ext = '-';
			  ext++;
			  strcpy(ext, tag->val);
			  ext += strlen(tag->val);
			  strcpy(ext, ".rib");
			}
		      else
			{
			  free(iafilename);
			  return AY_ERROR;
			}

		      RiBegin(iafilename);

		      free(iafilename);
		    } /* if */

		  RiAttributeBegin();
		  /* write material information */
		  if(o->mat)
		    {
		      ay_status = ay_matt_wrib(file, o->mat);
		    }

		  /* XXXX write tags */
		  /*
		  arr = ay_wribcbt.arr;
		  cb = (ay_wribcb *)(arr[o->type]);
		  */
		  if(cb)
		    {
		      /* write name */
		      if(ay_prefs.writeident)
			{
			  if(o->name)
			    {
			      if(!ay_prefs.ristandard)
				{
				  RiDeclare(parname, "string");
				  RiAttribute("identifier", parname,
					      (RtPointer)&o->name, RI_NULL);

				} /* if */
			    } /* if */
			}
		      else
			{
			  if(o->name)
			    {
			      RiArchiveRecord(RI_COMMENT, o->name);
			    } /* if */
			} /* if */

		      ay_status = cb(file, o);
		    } /* if */

		  l = NULL;

		  if(o->down && o->down->next)
		    {
		      down = o->down;

		      if(o->type == AY_IDLEVEL)
			{
			  l = (ay_level_object*)o->refine;
			  switch(l->type)
			    {
			    case AY_LTUNION:
			      RiSolidBegin(RI_UNION);
			      break;
			    case AY_LTDIFF:
			      RiSolidBegin(RI_DIFFERENCE);
			      break;
			    case AY_LTINT:
			      RiSolidBegin(RI_INTERSECTION);
			      break;
			    case AY_LTPRIM:
			      if(!ay_current_primlevel)
				{
				  RiSolidBegin(RI_PRIMITIVE);
				}
			      ay_current_primlevel++;
			      break;
			    default:
			      break;
			    } /* switch */
			} /* if */

		      while(down->next)
			{
			  down_is_prim = AY_FALSE;
			  if(ay_wrib_isprimitive(down))
			    {
			      down_is_prim = AY_TRUE;

			      if(l && ((l->type == AY_LTUNION) ||
				       (l->type == AY_LTDIFF) ||
				       (l->type == AY_LTINT)))
				{
				  if(!ay_current_primlevel)
				    {
				      RiSolidBegin(RI_PRIMITIVE);
				    }
				  ay_current_primlevel++;
				} /* if */
			    } /* if */

			  ay_status = ay_wrib_object(file, down);

			  if(down_is_prim)
			    {
			      if(l && ((l->type == AY_LTUNION) ||
				       (l->type == AY_LTDIFF) ||
				       (l->type == AY_LTINT)))
				{
				  ay_current_primlevel--;
				  if(!ay_current_primlevel)
				    {
				      RiSolidEnd();
				    }
				} /* if */
			    } /* if */

			  down = down->next;
			} /* while */
		    } /* if */

		  if(l)
		    {
		      if(l->type > 1)
			{
			  if(l->type == AY_LTPRIM)
			    {
			      ay_current_primlevel--;
			      if(!ay_current_primlevel)
				{
				  RiSolidEnd();
				}
			    }
			  else
			    {
			      RiSolidEnd();
			    } /* if */
			} /* if */
		    } /* if */

		  RiAttributeEnd();
		  RiEnd();

		  found = AY_TRUE;
		} /* if */
	      tag = tag->next;
	    } /* while */

	  if(!found)
	    return AY_ERROR; /* This should never happen! */

	} /* if */

      if(o->down && o->down->next)
	{
	  if(o->type == AY_IDLEVEL)
	    {
	      l = (ay_level_object*)o->refine;
	      if(l->type != AY_LTLEVEL)
		{
		  ay_current_primlevel++;
		}

	    } /* if */

	  ay_status = ay_instt_wribiarchives(file, o->down);

	  if(o->type == AY_IDLEVEL)
	    {
	      l = (ay_level_object*)o->refine;
	      if(l->type != AY_LTLEVEL)
		{
		  ay_current_primlevel--;
		} /* if */

	    } /* if */

	} /* if */

      if(ay_status)
	return ay_status;

      o = o->next;
    } /* while */

 return ay_status;
} /* ay_instt_wribiarchives */


/* ay_instt_clearoidtags:
 *  clear all object id tags from scene
 */
int
ay_instt_clearoidtags(ay_object *o)
{
 int ay_status = AY_OK;
 ay_tag *tag = NULL, **last = NULL;

  if(!o)
    return AY_OK;

  while(o)
    {
      if(o->tags)
	{
	  last = &(o->tags);
	  tag = o->tags;
	  while(tag)
	    {
	      if(tag->type == ay_oi_tagtype)
		{
		  *last = tag->next;
		  ay_tags_free(tag);
		  tag = *last;
		}
	      else
		{
		  last = &(tag->next);
		  tag = tag->next;
		} /* if */
	    } /* while */
	} /* if */

      if(o->down)
	ay_status = ay_instt_clearoidtags(o->down);

      if(ay_status)
	return ay_status;

      o = o->next;
    } /* while */

 return ay_status;
} /* ay_instt_clearoidtags */


/* ay_instt_findinstance:
 *  _recursively_ check objects pointed to by "o" for instance objects
 *  whose master is "m"; returns AY_ERROR immediately
 *  if a single such instance has been found
 */
int
ay_instt_findinstance(ay_object *m, ay_object *o)
{
 int ay_status = AY_OK;

  if(!o)
    return AY_OK;

  while(o)
    {
      if(o->type == AY_IDINSTANCE)
	{
	  if(((ay_object *)o->refine) == m)
	    return AY_ERROR;
	}

      if(o->down)
	{
	  ay_status = ay_instt_findinstance(m, o->down);

	  if(ay_status)
	    return ay_status;
	}
      o = o->next;
    }

 return ay_status;
} /* ay_instt_findinstance */


/** ay_instt_removeinstances:
 *  Remove all instance objects from objects pointed to by \a **o,
 *  which will eventually be modified (if \a *o is an instance);
 *  is able to work with multiple and nested objects!
 *
 * \param[in,out] o object hierarchy to check
 * \param[in] m if != NULL only instances of this object are removed,
 *  otherwise all instances will be removed
 */
void
ay_instt_removeinstances(ay_object **o, ay_object *m)
{
 ay_object **last = NULL, *next = NULL, *co = NULL;

  if(!o || !*o)
    return;

  co = *o;
  last = o;
  while(co)
    {
      if(co->down && co->down->next)
	{
	  ay_instt_removeinstances(&(co->down), m);
	} /* if */

      if((co->type == AY_IDINSTANCE) && (!m || (co->refine == m)))
	{
	  next = co->next;
	  ay_undo_clearobj(co);
	  (void)ay_object_delete(co);
	  (*last) = next;
	  co = next;
	}
      else
	{
	  last = &(co->next);
	  co = co->next;
	} /* if */
    } /* while */

 return;
} /* ay_instt_removeinstances */


/** ay_instt_clearclipboard:
 *  Check if there are instances of any master from the object
 *  hierarchy \a o in the clipboard and remove/delete them from
 *  the clipboard. The hierarchy can "change" because refcounts
 *  can be decreased in the progress.
 *
 * \param[in,out] o object hierarchy to check
 */
void
ay_instt_clearclipboard(ay_object *o)
{

  if(!o)
    return;

  while(o)
    {
      if((o->refcount > 0) && (o->type != AY_IDMATERIAL))
	{
	  ay_instt_removeinstances(&(ay_clipboard), o);
	}

      if(o->down && o->down->next)
	{
	  ay_instt_clearclipboard(o->down);
	}
      o = o->next;
    } /* while */

 return;
} /* ay_instt_clearclipboard */


/* ay_instt_resolvetcmd:
 *  resolve selected instance object by copying its master object
 *  Implements the \a resolveIn scripting interface command.
 *  See also the corresponding section in the \ayd{scresolvein}.
 *  \returns TCL_OK in any case.
 */
int
ay_instt_resolvetcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_voidfp *arr;
 ay_convertcb *cb;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  arr = ay_convertcbt.arr;
  cb = (ay_convertcb *)(arr[AY_IDINSTANCE]);
  if(!cb)
    return TCL_OK;

  while(sel)
    {
      o = sel->object;
      /* so that we may use continue */
      sel = sel->next;

      if(o->type != AY_IDINSTANCE)
	{
	  ay_error(AY_ERROR, argv[0], "Object is not of type Instance!");
	  continue;
	}
      /* convert in place */
      ay_status = cb(o, AY_TRUE);

      if(ay_status)
	{
	  ay_error(ay_status, argv[0], NULL);
	  continue;
	}
    }

 return TCL_OK;
} /* ay_instt_resolvetcmd */


/* ay_instt_checkinstance:
 *  Recursively check, whether master object of "instance"
 *  is a direct or remote parent of "target" to prevent
 *  recursive instances (instance is child of master).
 *  Returns AY_TRUE, if recursive instances would result from
 *  a DnD/paste operation of "instance" to "target",
 *  else returns AY_FALSE.
 *  This function first recursively browses the scene searching
 *  for the target, and when found, upon unwinding the recursive
 *  invocations, checks all (intermediate) parents from target to o.
 */
int
ay_instt_checkinstance(ay_object *o, ay_object *target,
		       ay_object *instance, int *check_this_tree)
{
 int res = AY_FALSE;
 int check = AY_FALSE;

  while(o->next)
    {
      if(o == target)
	{
	  /* inform all parent invocations that they need to check */
	  *check_this_tree = AY_TRUE;
	  return AY_FALSE;
	} /* if */

      if(o->down && o->down->next)
	{
	  res = ay_instt_checkinstance(o->down, target, instance, &check);

	  /* immediately return a positive result */
	  if(res == AY_TRUE)
	    {
	      return res;
	    } /* if */

	  /* inform parent invocation whether it needs to check */
	  *check_this_tree = check;

	  if(check)
	    {
	      if(o == ((ay_object *)instance->refine))
		{
		  return AY_TRUE;
		}
	      else
		{
		  return AY_FALSE;
		} /* if */
	    } /* if */
	} /* if */

      o = o->next;
    } /* while */

 return AY_FALSE;
} /* ay_instt_checkinstance */


/* ay_instt_check:
 *  Recursively browse through children of "o"
 *  (i.e. to be dropped/pasted) objects and call
 *  ay_instt_checkinstance() on them (if they are instances)
 *  Also check o (if it is a instance).
 */
int
ay_instt_check(ay_object *o, ay_object *target)
{
 int res = AY_FALSE;
 int check = AY_FALSE;
 ay_object *down = NULL;

  if(o->down)
    {
      down = o->down;
      while(down->next)
	{
	  if(down->down)
	    {
	      res = ay_instt_check(down, target);

	      /* immediately return a positive result */
	      if(res == AY_TRUE)
		return res;
	    } /* if */

	  if(down->type == AY_IDINSTANCE)
	    {
	      /* first, do easy check */
	      if(((ay_object*)down->refine) == target)
		{
		  return AY_TRUE;
		} /* if */

	      /* recursive check */
	      check = AY_FALSE;
	      res = ay_instt_checkinstance(ay_root, target, down, &check);

	      /* immediately return a positive result */
	      if(res == AY_TRUE)
		return res;
	    } /* if */

	  down = down->next;
	} /* while */
    } /* if */

  if(o->type == AY_IDINSTANCE)
    {
      /* first, do easy check */
      if(((ay_object*)o->refine) == target)
	{
	  return AY_TRUE;
	} /* if */

      check = AY_FALSE;
      res = ay_instt_checkinstance(ay_root, target, o, &check);

      /* immediately return a positive result */
      if(res == AY_TRUE)
	return res;
    } /* if */

 return AY_FALSE;
} /* ay_instt_check */


/* ay_instt_getmastertcmd:
 *  find the master object of the (currently selected) instance object
 *  Implements the \a getMaster scripting interface command.
 *  See also the corresponding section in the \ayd{scgetmaster}.
 *  \returns TCL_OK in any case.
 */
int
ay_instt_getmastertcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 int found = AY_FALSE;
 char *buf = NULL, *node = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "varname");
      return TCL_OK;
    }

  o = sel->object;

  if(o->type != AY_IDINSTANCE)
    {
      ay_error(AY_EWTYPE, argv[0], "Instance");
      return TCL_OK;
    }

  ay_status = ay_tree_crtnodefromobj(o->refine, ay_root, 1,
				     &buf, &node, &found);

  if(found == AY_FALSE)
    {
      ay_error(ay_status, argv[0], "Could not find Master object in scene!");
      return TCL_OK;
    }

  Tcl_SetVar(interp, argv[1], node, TCL_LEAVE_ERR_MSG);

  if(buf)
    free(buf);

 return TCL_OK;
} /* ay_instt_getmastertcmd */


/* ay_instt_countrefs:
 *  _recursively_ search for instances of master <m>
 *  in the list of objects <o> (and all their children)
 *  and count all found references in <refs>
 */
void
ay_instt_countrefs(ay_object *o, ay_object *m, unsigned int *refs)
{

  if(!m || !refs)
    return;

  while(o)
    {
      if(o->down && o->down->next)
	{
	  ay_instt_countrefs(o->down, m, refs);
	}
      if(o->type == AY_IDINSTANCE)
	{
	  if(o->refine == m)
	    {
	      *refs = *refs + 1;
	    }
	}
      o = o->next;
    } /* while */

 return;
} /* ay_instt_countrefs */


/* ay_instt_countrefslist:
 *  search for instances of master <m>
 *  in the list of objects <l> (and all their children)
 *  and count all found references in <refs>
 */
void
ay_instt_countrefslist(ay_list_object *l, ay_object *m, unsigned int *refs)
{
 ay_object *o = NULL;

  if(!m || !refs)
    return;

  while(l)
    {
      o = l->object;
      if(o->down && o->down->next)
	{
	  ay_instt_countrefs(o->down, m, refs);
	}
      if(o->type == AY_IDINSTANCE)
	{
	  if(o->refine == m)
	    {
	      *refs = *refs + 1;
	    }
	}
      l = l->next;
    } /* while */

 return;
} /* ay_instt_countrefslist */


/* ay_instt_init:
 *  initialize instt module
 */
void
ay_instt_init(Tcl_Interp *interp)
{
  /* register ObjectID tag type */
  ay_tags_register(interp, ay_oi_tagname, &ay_oi_tagtype);

  /* hash table for id -> master object pointers */
  Tcl_InitHashTable(&ay_instt_oidptr_ht, TCL_STRING_KEYS);

 return;
} /* ay_instt_init */
