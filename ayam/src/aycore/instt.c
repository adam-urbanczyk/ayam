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

/* instt.c - instancing tools - instance object helpers */

static Tcl_HashTable ay_instt_oidptr_ht;

/* ay_instt_createoidht:
 *  create object id hashtable, which links
 *  object ids to object pointers
 */
int
ay_instt_createoidht(ay_object *o)
{
 int ay_status = AY_OK;
 int new_item = 0;
 int found = AY_FALSE;
 Tcl_HashEntry *entry = NULL;
 ay_tag_object *tag = NULL;
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
	      if(tag->type == ay_instt_oitagtype)
		{
		  if((entry = Tcl_FindHashEntry(ht, tag->val)))
		    return AY_ERROR; /* OID already registered? */

		  entry = Tcl_CreateHashEntry(ht,
					      tag->val, &new_item);
		  Tcl_SetHashValue(entry, (char*)o); 

		  found = AY_TRUE;
		}

	      tag = tag->next;
	    }


	}

      if(o->down)
	ay_status = ay_instt_createoidht(o->down);

      if(ay_status)
	return ay_status;

      o = o->next;
    }

 return ay_status;
} /* ay_instt_createoidht */


/* ay_instt_connect:
 *  connect instance objects to the appropriate
 *  master objects (using OI tags and the object
 *  id hashtable)
 */
int
ay_instt_connect(ay_object *o, ay_object **last)
{
 int ay_status = AY_OK;
 char fname[] = "instt_connect";
 int found = AY_FALSE, removed = AY_FALSE;
 Tcl_HashEntry *entry = NULL;
 ay_tag_object *tag = NULL;
 ay_object *target = NULL;
 Tcl_HashTable *ht = &ay_instt_oidptr_ht;

 if(!o)
   return AY_OK;

  while(o)
    {
      removed = AY_FALSE;
      if((o->type == AY_IDINSTANCE) && (!o->refine))
	{
	  found = AY_FALSE;
	  tag = o->tags;
	  while(tag && !found)
	    {
	      if(tag->type == ay_instt_oitagtype)
		{
		  if(!(entry = Tcl_FindHashEntry(ht, tag->val)))
		    { /* OID not registered? */
		      ay_error(AY_ERROR, fname,
                    "Could not connect instance, removing instance with OID:");
		      ay_error(AY_ERROR, fname, tag->val);
		      *last = o->next;
		      ay_status = ay_object_delete(o);
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
	    return AY_ERROR; /* this should never happen */

	} /* if */

      if(!removed)
	{
	  if(o->down)
	    ay_status = ay_instt_connect(o->down, &(o->down));

	  if(ay_status)
	    return ay_status;

	  last = &(o->next);
	  o = o->next;
	} /* if */
    } /* while */

 return ay_status;
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
      counter=0;
      return AY_OK;
    }
  else
    counter++;

  /*
    TeaMan, 24.3.1999 on #AmigaGER (IRCNet):
    "32 chars sind genug"
    "es geht ja wohl um Praxisanwendung und nicht
    um akademische Betrachtungen" ;)
  */

  if(!(*dest = calloc(32, sizeof(char))))
    return AY_EOMEM;

  sprintf(*dest,"%u",counter);

 return AY_OK;
} /* ay_instt_createoid */


/* ay_instt_createorigids:
 *  create object id tags for all original or master or referenced
 *  objects
 */
int
ay_instt_createorigids(ay_object *o)
{
 int ay_status = AY_OK;
 char *tname = NULL, *tval = NULL;
 int found = AY_FALSE;
 ay_tag_object *tag = NULL, *newtag = NULL;

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
	      if(tag->type == ay_instt_oitagtype)
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
		return AY_EOMEM;

	      strcpy(tname,"OI");
	      if(!(newtag = calloc(1,sizeof(ay_tag_object))))
		return AY_EOMEM;

	      newtag->name = tname;
	      newtag->val = tval;
	      newtag->type = ay_instt_oitagtype;
	      newtag->next = o->tags;
	      o->tags = newtag;
	    }

	}

      if(o->down)
	ay_status = ay_instt_createorigids(o->down);

      if(ay_status)
	return ay_status;

      o = o->next;
    }

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
 ay_tag_object *tag = NULL, *origtag = NULL; 
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
	      if(tag->type == ay_instt_oitagtype)
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
	      if(tag->type == ay_instt_oitagtype)
		{ 
		  /* copy val from origtag to oitag of instance object */
		  free(tag->val);
		  if(!(tag->val=calloc(strlen(origtag->val)+1,sizeof(char))))
		    return AY_EOMEM;
		  strcpy(tag->val, origtag->val);

		  found = AY_TRUE;
		}
	      tag = tag->next;
	    }

	  if(!found)
	    {
	      /* need to create a new tag object */
	      ay_tags_copy(origtag, &tag);
	      tag->next = o->tags;
	      o->tags = tag;
	    }

	}

      if(o->down)
	ay_status = ay_instt_createinstanceids(o->down);

      if(ay_status)
	return ay_status;

      o = o->next;
    }

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
 ay_object *down = NULL;
 ay_tag_object *tag = NULL;
 char *iafilename = NULL;
 void **arr = NULL;
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
	      if(tag->type == ay_instt_oitagtype)
		{

		  /* create filename */
		  if(!file)
		    {
		      RiBegin(tag->val);
		    }
		  else
		    {
		      if(!(iafilename = calloc(1,
			   strlen(tag->val)+strlen(file)+2)))
			return AY_EOMEM;
		      strcpy(iafilename,file);
		      iafilename[strlen(file)] = '-';
		      strcpy(&(iafilename[strlen(file)+1]),tag->val);

		      RiBegin(iafilename);

		      free(iafilename);
		    }

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

				}
			    }
			}
		      else
			{
			  if(o->name)
			    {
			      RiArchiveRecord(RI_COMMENT, o->name);
			    }
			}
	
		      ay_status = cb(file, o);
		    }
		  
		  if(o->down)
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
			      RiSolidBegin(RI_PRIMITIVE);
			      break;
			    default:
			      break;
			    }
			}

		      while(down->next)
			{
			  ay_status = ay_wrib_object(file, down);
			  down = down->next;
			}
		    }

		  if(l)
		    {
		      if(l->type > 1)
			{
			  RiSolidEnd();
			}
		    }

		  RiAttributeEnd();
		  RiEnd();

		  found = AY_TRUE;
		}
	      tag = tag->next;
	    }

	  if(!found)
	    return AY_ERROR; /* This should never happen! */

	}

      if(o->down)
	ay_status = ay_instt_wribiarchives(file, o->down);

      if(ay_status)
	return ay_status;

      o = o->next;
    }

 return ay_status;
} /* ay_instt_wribiarchives */


/* ay_instt_clearoidtags:
 *  clear all object id tags from scene
 */
int
ay_instt_clearoidtags(ay_object *o)
{
 int ay_status = AY_OK;
 ay_tag_object *tag = NULL, **last = NULL;

  if(!o)
    return AY_OK;

  while(o)
    {
      if(o->tags)
	{
	  tag = o->tags;
	  last = &(o->tags);
	  tag = o->tags;
	  while(tag)
	    {
	      if(tag->type == ay_instt_oitagtype)
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
    }

 return ay_status;
} /* ay_instt_clearoidtags */


/* ay_instt_findinstance:
 *  check objects pointed to by "o" for instance objects that
 *  are instances of object "r"; returns AY_ERROR imediately
 *  if a single instance has been found 
 */
int
ay_instt_findinstance(ay_object *r, ay_object *o)
{
 int ay_status = AY_OK;

  if(!o)
    return AY_OK;

  while(o)
    {
      if(o->type == AY_IDINSTANCE)
	{
	  if(((ay_object *)o->refine) == r)
	    return AY_ERROR;
	}

      if(o->down)
	ay_status = ay_instt_findinstance(r, o->down);

      if(ay_status)
	return ay_status;

      o = o->next;
    }

 return ay_status;
} /* ay_instt_findinstance */


/* ay_instt_checkclipboard:
 *  used by ay_clear_scene(); see there for description
 */
int
ay_instt_checkclipboard(ay_object *o)
{
 int ay_status = AY_OK;

  if(!o)
    return AY_OK;

  while(o)
    {
      if(o->refcount>0 && o->type != AY_IDMATERIAL)
	{
	  ay_status = ay_instt_findinstance(o, ay_clipboard);
	  if(ay_status)
	    return ay_status;
	}

      if(o->down)
	ay_status = ay_instt_checkclipboard(o->down);

      if(ay_status)
	return ay_status;

      o = o->next;
    }

 return ay_status;
} /* ay_instt_checkclipboard */


/* ay_instt_resolve:
 *  resolve instance i by copying the master object
 *  onto i
 */
int
ay_instt_resolve(ay_object *i)
{
 int ay_status = AY_OK;
 char *iname = NULL;
 double movx, movy, movz;
 double rotx, roty, rotz;
 double scalx, scaly, scalz;
 double quat[4];
 ay_object *orig = NULL, *temp = NULL;
 ay_object *inext = NULL;

  movx = i->movx;
  movy = i->movy;
  movz = i->movz;

  rotx = i->rotx;
  roty = i->roty;
  rotz = i->rotz;

  scalx = i->scalx;
  scaly = i->scaly;
  scalz = i->scalz;

  quat[0] = i->quat[0];
  quat[1] = i->quat[1];
  quat[2] = i->quat[2];
  quat[3] = i->quat[3];

  if(i->type != AY_IDINSTANCE)
    return AY_ERROR;

  orig = (ay_object *)i->refine;
  if(!orig)
    return AY_ERROR;

  inext = i->next;
  iname = i->name;

  ay_status = ay_object_copy(orig, &temp);
  if(ay_status)
    return ay_status;

  memcpy(i, temp, sizeof(ay_object));
  if(i->name) free(i->name);
  i->name = iname;

  i->next = inext;

  i->movx = movx;
  i->movy = movy;
  i->movz = movz;

  i->rotx = rotx;
  i->roty = roty;
  i->rotz = rotz;

  i->scalx = scalx;
  i->scaly = scaly;
  i->scalz = scalz;

  i->quat[0] = quat[0];
  i->quat[1] = quat[1];
  i->quat[2] = quat[2];
  i->quat[3] = quat[3];

  orig->refcount--;

 return ay_status;
} /* ay_instt_resolve */


/* ay_instt_resolvetcmd:
 *  resolve selected instance object by copying its master object
 */
int
ay_instt_resolvetcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 char fname[] = "resolveIn";

  if(!sel)
    {
      ay_error(AY_ENOSEL,fname,NULL);
      return TCL_OK;
    }

  if(!(o = sel->object))
    {
      ay_error(AY_ERROR,fname,NULL);
      return TCL_OK;
    }

  if(!o->type == AY_IDINSTANCE)
    {
      ay_error(AY_ERROR, fname, "Object is not of type Instance!");
      return TCL_OK;
    }

  ay_status = ay_instt_resolve(o);

  if(ay_status)
    {
      ay_error(ay_status, fname, NULL);
      return TCL_OK;
    }

 return TCL_OK;
} /* ay_instt_resolvetcmd */


/* ay_instt_checkinstance:
 *  Recursive! Check, whether master object of "instance"
 *  is a (even remote) parent of "target"
 *  (to prevent recursive instances).
 *  Returns AY_TRUE, if recursive instances would result from
 *  a DnD/paste operation of "instance" to "target".
 */
int
ay_instt_checkinstance(ay_object *o, ay_object *target,
		       ay_object *instance, int *check_this_tree)
{
 int res = AY_FALSE;
 int check = AY_FALSE;

 while(o->next)
    {
      if(o->down)
	{
	  res = ay_instt_checkinstance(o->down, target, instance, &check);
	  *check_this_tree = check;

	  if(res)
	    {
	      return res;
	    } /* if */

	  if(check)
	    {
	      if(o == ((ay_object *)instance->refine))
		{
		  return AY_TRUE;
		} /* if */
	    } /* if */
	} /* if */

      if(o == target)
	{
	  *check_this_tree = AY_TRUE;

	  if((o == ((ay_object *)instance->refine)) && o->down)
	    {
	      return AY_TRUE;
	    }
	  else
	    {
	      return AY_FALSE;
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
	  res = AY_FALSE;
	  if(down->down)
	    {
	      res = ay_instt_check(down, target);

	      /* immediately return positive result */
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
	      res = ay_instt_checkinstance(ay_root, target, down, &check);

	      /* immediately return positive result */
	      if(res == AY_TRUE)
		return res;
	    } /* if */

	  down = down->next;
	} /* while */
    } /* if */

 
  if(o->type == AY_IDINSTANCE)
    {
      res = ay_instt_checkinstance(ay_root, target, o, &check);

      /* immediately return positive result */
      if(res == AY_TRUE)
	return res;
    } /* if */

 return AY_FALSE;
} /* ay_instt_check */


/* ay_instt_getmaster:
 *  Recursively browse through the scene and find the master
 *  object of the instance object o.
 */
int
ay_instt_getmaster(ay_object *o, ay_object *i, ay_object **r)
{
 int result = AY_FALSE;

  while(o->next)
    {
      if(o->down)
	{
	  result = ay_instt_getmaster(o->down, i, r);
	  
	  if(result == AY_TRUE)
	    {
	      ay_clevel_add(o);
	      return AY_TRUE;
	    }
	}

      if(i->refine == o)
	{
	  *r = o;
	  ay_clevel_add(o);
	  return AY_TRUE;
	}

      o = o->next;
    } /* while */

 return AY_FALSE;
} /* ay_instt_getmaster */


/* ay_instt_getmastertcmd:
 *  find the master object of the (currently selected) instance object
 */
int
ay_instt_getmastertcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_list_object *clevel = ay_currentlevel;
 ay_object *o = NULL, *master = NULL;
 char fname[] = "getMaster";
 int result = AY_FALSE;
 Tcl_DString ds;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  if(!(o = sel->object))
    {
      ay_error(AY_ERROR, fname, NULL);
      return TCL_OK;
    }

  if(o->type != AY_IDINSTANCE)
    {
      ay_error(AY_ERROR, fname, "Object is not of type Instance!");
      return TCL_OK;
    }

  ay_currentlevel = NULL;

  result = ay_instt_getmaster(ay_root, o, &master);

  if(result == AY_FALSE)
    {
      ay_error(ay_status, fname, "Could not find Master object in scene!");
      return TCL_OK;
    }

  Tcl_DStringInit(&ds);

  if(ay_currentlevel->object == ay_root)
    ay_clevel_del();

  ay_tree_crtnodename(ay_root, ay_currentlevel, &ds);
  
  Tcl_SetVar(interp, argv[1], Tcl_DStringValue(&ds), TCL_LEAVE_ERR_MSG);

  Tcl_DStringFree(&ds);
  
  ay_clevel_delall();
  free(ay_currentlevel);
  ay_currentlevel = clevel;

 return TCL_OK;
} /* ay_instt_getmastertcmd */


/* ay_instt_init:
 *  initialize instt module
 */
void
ay_instt_init(Tcl_Interp *interp)
{
  /* register ObjectID tag type */
  ay_tags_register(interp, "OI", &ay_instt_oitagtype);

  /* hash table for id -> original object pointers */
  Tcl_InitHashTable(&ay_instt_oidptr_ht, TCL_STRING_KEYS);

  return;
} /* ay_instt_init */
