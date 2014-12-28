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

/* tags.c - functions for tag management */


/* ay_tags_free:
 *  free tag <tag>
 */
void
ay_tags_free(ay_tag *tag)
{

  if(!tag)
    return;

  if(tag->name)
    free(tag->name);

  if(tag->val)
    {
      if(tag->is_binary)
	{
	  if(((ay_btval*)tag->val)->size)
	    {
	      free(((ay_btval*)tag->val)->payload);
	    }
	}
      free(tag->val);
    }

  free(tag);

 return;
} /* ay_tags_free */


/* ay_tags_delall:
 *  remove all tags from object <o>
 */
void
ay_tags_delall(ay_object *o)
{
 ay_tag *t = NULL, *nt = NULL;

  if(!o)
    return;

  t = o->tags;
  while(t)
    {
      nt = t->next;
      ay_tags_free(t);
      t = nt;
    }

  o->tags = NULL;

 return;
} /* ay_tags_delall */


/* ay_tags_copy:
 *  copy a tag from <source> to <dest>
 */
int
ay_tags_copy(ay_tag *source, ay_tag **dest)
{
 int ay_status = AY_OK;
 ay_tag *new = NULL;
 ay_btval *nbt = NULL;

  if(!source || !dest)
    return AY_ENULL;

  if(!source->val)
    return AY_ERROR;

  if(!(new = malloc(sizeof(ay_tag))))
    return AY_EOMEM;

  memcpy(new, source, sizeof(ay_tag));
  /* danger! links point to original hierachy */

  new->next = NULL;

  /* copy name */
  if(source->name)
    {
      if(!(new->name = malloc((strlen(source->name)+1)*sizeof(char))))
	{ free(new); return AY_EOMEM; }
      strcpy(new->name, source->name);
    }

  if(source->is_binary)
    {
      /* copy binary val */
      if(!(new->val = malloc(sizeof(ay_btval))))
	{ free(new->name); free(new); return AY_EOMEM; }
      memcpy(new->val, source->val, sizeof(ay_btval));
      if(((ay_btval*)new->val)->size)
	{
	  nbt = (ay_btval*)new->val;
	  if(!(nbt->payload = malloc(nbt->size)))
	    { free(new->val); free(new->name); free(new); return AY_EOMEM; }
	  memcpy(nbt->payload, ((ay_btval*)source->val)->payload, nbt->size);
	}
    }
  else
    {
      /* copy ASCII val */
      if(!(new->val = malloc((strlen(source->val)+1)*sizeof(char))))
	{ free(new->name); free(new); return AY_EOMEM; }
      strcpy(new->val, source->val);
    }

  *dest = new;

 return ay_status;
} /* ay_tags_copy */


/* ay_tags_copyall:
 *  copy all tags from object <src> to object <dst>
 *  internal and binary tags are _not_ copied
 */
int
ay_tags_copyall(ay_object *src, ay_object *dst)
{
 int ay_status = AY_OK;
 ay_tag *tag = NULL, **newtagptr = NULL;

  if(!src || !dst)
    return AY_ENULL;

  tag = src->tags;
  newtagptr = &(dst->tags);
  while(tag)
    {
      if(!tag->is_intern)
	{
	  ay_status = ay_tags_copy(tag, newtagptr);
	  if(ay_status == AY_OK)
	    {
	      newtagptr = &((*newtagptr)->next);
	      *newtagptr = NULL;
	    }
	  else
	    {
	      ay_tags_delall(dst);
	      return ay_status;
	    }
	}
      tag = tag->next;
    }

 return AY_OK;
} /* ay_tags_copyall */


/* ay_tags_append:
 *  append the tag <tag> to object <o>;
 *  this routine keeps a pointer to the last tag of
 *  object o and is therefore fast for subsequent calls;
 *  the caller has to make sure, that the tags of <o> do not change
 *  between calls, or has to call ay_tags_append(NULL, NULL); to
 *  reset the cache.
 */
int
ay_tags_append(ay_object *o, ay_tag *tag)
{
 int ay_status = AY_OK;
 static ay_object *last_object = NULL;
 static ay_tag **next_tag = NULL;

  if(!o)
    {
      last_object = NULL;
      return AY_OK;
    }

  if(!tag)
    return AY_ENULL;

  if((o != last_object) || (!next_tag))
    {
      next_tag = &(o->tags);
      while(*next_tag)
	{
	  next_tag = &((*next_tag)->next);
	}
    }

  *next_tag = tag;
  next_tag = &(tag->next);

 return ay_status;
} /* ay_tags_append */


/* ay_tags_register:
 *  register a new tag type
 */
int
ay_tags_register(Tcl_Interp *interp, char *name, char **result)
{
 int new_item = 0;
 Tcl_HashEntry *entry = NULL;
 static char *tagcounter = (char *) 1; /* 0 - dummy */
 char fname[] = "tags_register";

  /* check, if type is already registered */
  if((entry = Tcl_FindHashEntry(&ay_tagtypesht, name)))
    {
      ay_error(AY_ERROR, fname, "tag type already registered");
      return AY_ERROR;
    }

  tagcounter++;

  entry = Tcl_CreateHashEntry(&ay_tagtypesht, name, &new_item);
  Tcl_SetHashValue(entry, tagcounter);

  *result = tagcounter;

 return AY_OK;
} /* ay_tags_register */


/** ay_tags_settcmd:
 *  set new tags of selected object(s), after removing all old tags
 *  Implements the \a setTags scripting interface command.
 *  See also the corresponding section in the \ayd{scsettags}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_tags_settcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 int tcl_status = TCL_OK;
 char args[] =
   "[type value]|-index index type value|-delete index";
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_tag *t = NULL, *new = NULL, **next = NULL;
 ay_tag *oldibtags = NULL, **otnext = NULL;
 Tcl_HashEntry *entry = NULL;
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 int index = 0, i = 0, pasteProp = AY_FALSE;

  if(argc < 3)
    {
      ay_error(AY_EARGS, argv[0], args);
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(!strcmp(argv[1],"-index"))
    {
      if(argc < 5)
	{
	  ay_error(AY_EARGS, argv[0], args);
	  return TCL_OK;
	}

      tcl_status = Tcl_GetInt(interp, argv[2], &index);
      AY_CHTCLERRRET(tcl_status, argv[0], interp);

      /* find tag */
      o = sel->object;
      new = o->tags;
      while(new)
	{
	  if(new->is_intern || new->is_binary)
	    {
	      new = new->next;
	    }
	  else
	    {
	      if(i == index)
		break;
	      else
		{
		  i++;
		  new = new->next;
		}
	    }
	} /* while */

      if(!new)
	{
	  ay_error(AY_ERROR, argv[0], "Tag not found!");
	  return TCL_OK;
	}

      if(strcmp(argv[3], ""))
	{
	  if(new->name)
	    {
	      free(new->name);
	      new->name = NULL;
	    }
	  if(new->val)
	    {
	      free(new->val);
	      new->val = NULL;
	    }
	  /* we first try to resolve the tag type */
	  if(!(entry = Tcl_FindHashEntry(&ay_tagtypesht, argv[3])))
	    {
	      if(ay_prefs.wutag)
		ay_error(AY_EWARN, argv[0], "Tag type is not registered!");
	    }
	  if(entry)
	    {
	      new->type = (char *)Tcl_GetHashValue(entry);
	    }
	  /* create new tag */
	  if(!(new->name = malloc((strlen(argv[3])+1) * sizeof(char))))
	    {
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }
	  strcpy(new->name, argv[3]);

	  if(!(new->val = malloc((strlen(argv[4])+1) * sizeof(char))))
	    {
	      ay_error(AY_EOMEM, argv[0], NULL);
	      return TCL_OK;
	    }
	  strcpy(new->val, argv[4]);
	} /* if */

      return TCL_OK; /* early exit! */
    } /* if */

  if(!strcmp(argv[1], "-delete"))
    {
      if(argc < 3)
	{
	  ay_error(AY_EARGS, argv[0], args);
	  return TCL_OK;
	}

      tcl_status = Tcl_GetInt(interp, argv[2], &index);
      AY_CHTCLERRRET(tcl_status, argv[0], interp);

      /* find tag */
      o = sel->object;
      new = o->tags;
      next = &(o->tags);
      while(new)
	{
	  if(new->is_intern || new->is_binary)
	    {
	      next = &(new->next);
	      new = new->next;
	    }
	  else
	    {
	      if(i == index)
		break;
	      else
		{
		  i++;
		  next = &(new->next);
		  new = new->next;
		}
	    }
	} /* while */

      if(!new)
	{
	  ay_error(AY_ERROR, argv[0], "Tag not found!");
	  return TCL_OK;
	}
      else
	{
	  *next = new->next;
	  ay_tags_free(new);
	}

      return TCL_OK; /* early exit! */
    } /* if */

  /* normal setTags functionality */

  /* is a paste property in progress? */
  toa = Tcl_NewStringObj("ay", -1);
  ton = Tcl_NewStringObj("pasteProp", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &pasteProp);

  while(sel)
    {
      o = sel->object;

      if(!pasteProp)
	{
	  /* save old internal and binary tags */
	  t = o->tags;
	  otnext = &(oldibtags);
	  next = &(o->tags);
	  while(t)
	    {
	      if(t->is_intern || t->is_binary)
		{
		  /* link tag t to end of list in oldibtags */
		  *otnext = t;
		  /* unlink tag t from list in o->tags */
		  *next = t->next;
		  /* prepare processing the next tag */
		  t = t->next;
		  /* properly terminate the list in oldibtags */
		  (*otnext)->next = NULL;
		  otnext = &((*otnext)->next);
		}
	      else
		{
		  /* prepare processing the next tag */
		  next = &(t->next);
		  t = t->next;
		}
	    } /* while */

	  /* delete old tags */
	  ay_tags_delall(o);

	  /* now add again the saved internal/binary tags */
	  if(oldibtags)
	    {
	      o->tags = oldibtags;
	      oldibtags = NULL;
	      next = otnext;
	    }
	  else
	    {
	      next = &(o->tags);
	    }
	}
      else
	{
	  /* pasting properties via property clipboard appends... */
	  new = o->tags;
	  next = &(o->tags);
	  while(new)
	    {
	      next = &(new->next);
	      new = new->next;
	    }
	} /* if */

      /* add new tags from args */
      for(index = 1; index < argc-1; index += 2)
	{
	  if(strcmp(argv[index], ""))
	    {
	      if(!(new = calloc(1, sizeof(ay_tag))))
		{
		  ay_error(AY_EOMEM, argv[0], NULL);
		  return TCL_OK;
		}
	      /* we first try to resolve the tag type */
	      if(!(entry = Tcl_FindHashEntry(&ay_tagtypesht, argv[index])))
		{
		  if(ay_prefs.wutag)
		    ay_error(AY_EWARN, argv[0], "Tag type is not registered!");
		}
	      if(entry)
		{
		  new->type = (char *)Tcl_GetHashValue(entry);
		}

	      if(!(new->name = malloc((strlen(argv[index])+1) * sizeof(char))))
		{
		  free(new);
		  ay_error(AY_EOMEM, argv[0], NULL);
		  return TCL_OK;
		}
	      strcpy(new->name, argv[index]);

	      if(!(new->val = malloc((strlen(argv[index+1])+1) * sizeof(char))))
		{
		  free(new->name);
		  free(new);
		  ay_error(AY_EOMEM, argv[0], NULL);
		  return TCL_OK;
		}
	      strcpy(new->val, argv[index+1]);

	      /* link new tag */
	      *next = new;
	      next = &(new->next);
	    } /* if */
	} /* for */

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_tags_settcmd */


/** ay_tags_addtcmd:
 *  add a tag to the selected object(s)
 *  Implements the \a addTag scripting interface command.
 *  See also the corresponding section in the \ayd{scaddtag}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_tags_addtcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_tag *new = NULL, *t = NULL;
 Tcl_HashEntry *entry = NULL;

   if(argc < 3)
     {
       ay_error(AY_EARGS, argv[0], "type value");
       return TCL_OK;
     }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  /* we first try to resolve the tag type */
  if(!(entry = Tcl_FindHashEntry(&ay_tagtypesht, argv[1])))
    {
      if(ay_prefs.wutag)
	ay_error(AY_EWARN, argv[0], "Tag type is not registered!");
    }

  while(sel)
    {
      o = sel->object;

      if(!(new = calloc(1, sizeof(ay_tag))))
	{
	  ay_error(AY_EOMEM, argv[0], NULL);
	  return TCL_OK;
	}
      if(!(new->name = malloc((strlen(argv[1])+1) * sizeof(char))))
	{
	  free(new);
	  ay_error(AY_EOMEM, argv[0], NULL);
	  return TCL_OK;
	}
      strcpy(new->name, argv[1]);

      if(entry)
	new->type = (char *)Tcl_GetHashValue(entry);

      if(!(new->val = malloc((strlen(argv[2])+1) * sizeof(char))))
	{
	  free(new->name); free(new);
	  ay_error(AY_EOMEM, argv[0], NULL);
	  return TCL_OK;
	}
      strcpy(new->val, argv[2]);

      if(!o->tags)
	{
	  o->tags = new;
	}
      else
	{
	  t = o->tags;
	  while(t->next)
	    t = t->next;
	  t->next = new;
	}

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_tags_addtcmd */


/* ay_tags_gettcmd:
 *  return all tags of the (first) selected object
 *  Implements the \a getTags scripting interface command.
 *  See also the corresponding section in the \ayd{scgettags}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_tags_gettcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_tag *tag = NULL;
 int get_single = AY_FALSE;

  if(argv[0][6] != 's')
    get_single = AY_TRUE;

  if(argc < 3)
    {
      if(get_single)
	ay_error(AY_EARGS, argv[0], "tagname vname");
      else
	ay_error(AY_EARGS, argv[0], "vname1 vname2");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(!get_single)
    Tcl_SetVar(interp, argv[1], "", TCL_LEAVE_ERR_MSG);
  Tcl_SetVar(interp, argv[2], "", TCL_LEAVE_ERR_MSG);

  o = sel->object;
  tag = o->tags;
  while(tag)
    {
      if(tag->name && tag->val && !tag->is_intern && !tag->is_binary)
	{
	  if(get_single)
	    {
	      if(!strcmp(argv[1], tag->name))
		{
		  Tcl_SetVar(interp, argv[2], tag->val, TCL_APPEND_VALUE |
			     TCL_LEAVE_ERR_MSG);
		  break;
		}
	    }
	  else
	    {
	      Tcl_SetVar(interp, argv[1], tag->name, TCL_APPEND_VALUE |
			 TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);
	      Tcl_SetVar(interp, argv[2], tag->val, TCL_APPEND_VALUE |
			 TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);
	    }
	} /* if */
      tag = tag->next;
    } /* while */

 return TCL_OK;
} /* ay_tags_gettcmd */


/** ay_tags_hastag:
 *  check for existence of a tag given a tag name or type
 */
int
ay_tags_hastag(const ay_object *o, const char *tagname, const char *tagtype)
{
 ay_tag *tag;

  if(!o)
    return AY_FALSE;

  tag = o->tags;
  if(tagname)
    while(tag)
      {
	if(!ay_comp_strcase(tag->name, tagname))
	  return AY_TRUE;
	tag = tag->next;
      }
  else
    while(tag)
      {
	if(tag->type == tagtype)
	  return AY_TRUE;
	tag = tag->next;
      }

 return AY_FALSE;
} /* ay_tags_hastag */


/** ay_tags_hastcmd:
 *  check for existence of a tag
 *  Implements the \a hasTag scripting interface command.
 *  See also the corresponding section in the \ayd{schastag}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_tags_hastcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_tag *tag = NULL;

  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "name");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;
  tag = o->tags;
  while(tag)
    {
      if(!ay_comp_strcase(tag->name, argv[1]))
	{
	  Tcl_SetResult(interp, "1", TCL_VOLATILE);
	  return TCL_OK;
	}
      tag = tag->next;
    }

  Tcl_SetResult(interp, "0", TCL_VOLATILE);

 return TCL_OK;
} /* ay_tags_hastcmd */


/* ay_tags_deletetcmd:
 *  delete all tags of the selected object(s)
 *  Implements the \a delTags scripting interface command.
 *  See also the corresponding section in the \ayd{scdeltags}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_tags_deletetcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 /*int ay_status = AY_OK;*/
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_tag *tag = NULL, **last = NULL;
 int mode = 1; /* 1 delall, 0 type */

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(argc > 1 && strcmp(argv[1], "all"))
    {
      mode = 0;
    }

  while(sel)
    {
      o = sel->object;

      last = &(o->tags);
      tag = o->tags;
      while(tag)
	{
	  if((!tag->is_binary) && tag->name)
	    {
	      if(mode || (!strcmp(argv[1], tag->name)))
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
	    }
	  else
	    {
	      last = &(tag->next);
	      tag = tag->next;
	    } /* if */
	} /* while */

      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_tags_deletetcmd */


/* ay_tags_parseplist:
 *  parse the string str into a parameterlist suitable for calls
 *  into the RenderMan Interface (e.g. RiDisplayV());
 *  the parameterlist is returned in: argc, tokensr, and valuesr;
 *  if declare is AY_TRUE this routine also makes the necessary
 *  calls to RiDeclare()!
 */
int
ay_tags_parseplist(char *str, int declare, RtInt *argc, RtToken **tokensr,
		   RtPointer **valuesr)
{
 int ay_status = AY_OK;
 char fname[] = "tags_parseplist";
 char e1[] = "Missing value in parameter!";
 char e2[] = "Unknown parameter!";
 char *tmp = NULL, *parname = NULL, *partype = NULL, *parval = NULL;
 char *parval2 = NULL;
 char tok[] = ",";
 int i, valueslen = 0, tokenslen = 0;
 RtInt *itemp;
 RtFloat *ftemp;
 RtPoint *ptemp;
 RtString *stemp;
 RtColor *ctemp;
 RtToken *tokens, *tokenstmp;
 RtPointer *values, *valuestmp;

  if(!str || !argc || !tokensr || !valuesr)
    return AY_ENULL;

  if(*str == '\0')
    return AY_OK;

  /* make a copy of str, so that we may parse it using strtok() */
  if(!(tmp = malloc((strlen(str)+1)*sizeof(char))))
    { return AY_EOMEM; }
  strcpy(tmp, str);

  *argc = 0;
  parval = NULL;
  partype = NULL;
  parname = NULL;
  tokens = NULL;
  values = NULL;

  /* get first name */
  parname = strtok(tmp, tok);

  while(parname)
    {
      /* get type */
      partype = strtok(NULL, tok);

      if(partype)
	{
	  /* get value */
	  parval = strtok(NULL, tok);
	  if(parval)
	    {
	      /* we have all three needed components and thus
		 may allocate memory for the new parameter */
	      if(!(tokenstmp = realloc(tokens, (*argc+1)*sizeof(RtToken))))
		{ ay_status = AY_EOMEM; goto cleanup; }
	      tokens = tokenstmp;
	      if(!(valuestmp = realloc(values, (*argc+1)*sizeof(RtPointer))))
		{ ay_status = AY_EOMEM; goto cleanup; }
	      values = valuestmp;
	      /* copy name */
	      if(!(tokens[*argc] = calloc(strlen(parname)+1, sizeof(char))))
		{ ay_status = AY_EOMEM; goto cleanup; }
	      strcpy(tokens[*argc], parname);
	      tokenslen++;

	      switch(*partype)
		{
		case 'i':
		  if(!(itemp = calloc(1, sizeof(RtInt))))
		    { ay_status = AY_EOMEM; goto cleanup; }
		  values[*argc] = (RtPointer)itemp;
		  sscanf(parval, "%d", itemp);
		  if(declare)
		    { RiDeclare(parname, "integer"); }
		  break;
		case 'j':
		  if(!(itemp = calloc(2, sizeof(RtInt))))
		    { ay_status = AY_EOMEM; goto cleanup; }
		  values[*argc] = (RtPointer)itemp;
		  sscanf(parval, "%d", itemp);
		  parval2 = strtok(NULL, tok);
		  if(parval2)
		    {
		      sscanf(parval2, "%d", &(itemp[1]));
		    }
		  else
		    {
		      ay_error(AY_ERROR, fname, e1);
		    }

		  if(declare)
		    { RiDeclare(parname, "integer[2]"); }
		  break;
		case 'f':
		  if(!(ftemp = calloc(1, sizeof(RtFloat))))
		    { ay_status = AY_EOMEM; goto cleanup; }
		  values[*argc] = (RtPointer)ftemp;
		  sscanf(parval, "%f", ftemp);
		  if(declare)
		    { RiDeclare(parname, "float"); }
		  break;
		case 'g':
		  if(!(ftemp = calloc(2, sizeof(RtFloat))))
		    { ay_status = AY_EOMEM; goto cleanup; }
		  values[*argc] = (RtPointer)ftemp;
		  sscanf(parval, "%f", ftemp);
		  parval2 = strtok(NULL, tok);
		  if(parval2)
		    {
		      sscanf(parval2, "%f", &(ftemp[1]));
		    }
		  else
		    {
		      ay_error(AY_ERROR, fname, e1);
		    }

		  if(declare)
		    { RiDeclare(parname, "float[2]"); }
		  break;
		case 's':
		  if(!(stemp = calloc(1, sizeof(RtString))))
		    { ay_status = AY_EOMEM; goto cleanup; }

		  if(!(*stemp = calloc(strlen(parval)+1, sizeof(char))))
		    { free(stemp); ay_status = AY_EOMEM; goto cleanup; }

		  strcpy(*stemp, parval);
		  values[*argc] = (RtPointer)stemp;
		  if(declare)
		    { RiDeclare(parname, "string"); }
		  break;
		case 'p':
		  if(!(ptemp = calloc(1, sizeof(RtPoint))))
		    { ay_status = AY_EOMEM; goto cleanup; }
		  values[*argc] = (RtPointer)ptemp;
		  sscanf(parval, "%f", &((*ptemp)[0]));
		  parval = strtok(NULL, tok);
		  if(parval)
		    {
		      sscanf(parval, "%f", &((*ptemp)[1]));
		    }
		  else
		    {
		      ay_error(AY_ERROR, fname, e1);
		    }
		  parval = strtok(NULL, tok);
		  if(parval)
		    {
		      sscanf(parval, "%f", &((*ptemp)[2]));
		    }
		  else
		    {
		      ay_error(AY_ERROR, fname, e1);
		    }
		  if(declare)
		    { RiDeclare(parname, "point"); }
		  break;
		case 'c':
		  if(!(ctemp = calloc(1, sizeof(RtColor))))
		    { ay_status = AY_EOMEM; goto cleanup; }
		  values[*argc] = (RtPointer)ctemp;
		  sscanf(parval, "%f", &((*ctemp)[0]));
		  parval = strtok(NULL, tok);
		  if(parval)
		    {
		      sscanf(parval, "%f", &((*ctemp)[1]));
		    }
		  else
		    {
		      ay_error(AY_ERROR, fname, e1);
		    }
		  parval = strtok(NULL, tok);
		  if(parval)
		    {
		      sscanf(parval, "%f", &((*ctemp)[2]));
		    }
		  else
		    {
		      ay_error(AY_ERROR, fname, e1);
		    }
		  if(declare)
		    { RiDeclare(parname, "color"); }
		  break;
		default:
		  ay_error(AY_ERROR, fname, e2);
		  ay_status = AY_ERROR;
		  goto cleanup;
		} /* switch */

	      valueslen++;
	      *argc = *argc+1;

	    } /* if(parval */

	} /* if(partype */

      /* get next name */
      parname = strtok(NULL, tok);
    } /* while(parname */

  *tokensr = tokens;
  *valuesr = values;

  /* prevent cleanup code from doing something harmful */
  tokens = NULL;
  values = NULL;

cleanup:

  if(tokens)
    {
      for(i = 0; i < tokenslen; i++)
	{
	  if(tokens[i])
	    free(tokens[i]);
	}
      free(tokens);
    }

  if(values)
    {
      for(i = 0; i < valueslen; i++)
	{
	  if(values[i])
	    free(values[i]);
	}
      free(values);
    }

  free(tmp);

 return ay_status;
} /* ay_tags_parseplist */


/** ay_tags_reconnect:
 * reconnect already existing tags after late tag type registration
 *
 * \param[in,out] o object hierarchy to process
 * \param[in] tagtype tag type to set
 * \param[in] tagname tag name of tags to process
 */
void
ay_tags_reconnect(ay_object *o, char *tagtype, char *tagname)
{
 ay_tag *tag;

  if(!tagtype || !tagname)
    return;

  while(o)
    {
      if(o->down)
	{
	  ay_tags_reconnect(o->down, tagtype, tagname);
	}

      tag = o->tags;
      while(tag)
	{
	  if(tag->name)
	    {
	      if(!strcmp(tag->name, tagname))
		{
		  tag->type = tagtype;
		}
	    }
	  tag = tag->next;
	} /* while */

      (void)ay_notify_object(o);

      o = o->next;
    } /* while */

 return;
} /* ay_tags_reconnect */


/** ay_tags_addnonm:
 *  add NO/NM tags
 *
 * \param[in,out] o object that gets the nm tag (typically a instance)
 * \param[in,out] m object that gets the no tag (typically the master of o)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_tags_addnonm(ay_object *o, ay_object *m)
{
 int ay_status = AY_OK;
 ay_tag *tag;
 ay_tag *newnotag, *newnmtag;
 int found = AY_FALSE;

  if(!o || !m)
    return AY_ENULL;

  /* see if there is already a NO tag in the master
     that points to us, if there is one, nothing
     needs to be done */
  tag = m->tags;
  while(tag)
    {
      if(tag->type == ay_no_tagtype && tag->is_binary)
	{
	  if(o == ((ay_btval*)tag->val)->payload)
	    {
	      found = AY_TRUE;
	      break;
	    }
	}
      tag = tag->next;
    } /* while */

  if(!found)
    {
      /* first, get memory for the two new binary tags */
      if(!(newnotag = calloc(1, sizeof(ay_tag))))
	{
	  return AY_EOMEM;
	}
      newnotag->type = ay_no_tagtype;
      newnotag->is_intern = AY_TRUE;
      newnotag->is_binary = AY_TRUE;
      if(!(newnotag->val = malloc(sizeof(ay_btval))))
	{
	  free(newnotag);
	  return AY_EOMEM;
	}
      ((ay_btval*)newnotag->val)->size = 0;
      ((ay_btval*)newnotag->val)->payload = o;

      if(!(newnmtag = calloc(1, sizeof(ay_tag))))
	{
	  ay_tags_free(newnotag);
	  return AY_EOMEM;
	}

      newnmtag->type = ay_nm_tagtype;
      newnmtag->is_intern = AY_TRUE;
      newnmtag->is_binary = AY_TRUE;
      if(!(newnmtag->val = malloc(sizeof(ay_btval))))
	{
	  free(newnmtag);
	  ay_tags_free(newnotag);
	  return AY_EOMEM;
	}
      ((ay_btval*)newnmtag->val)->size = 0;
      ((ay_btval*)newnmtag->val)->payload = m;

      /* second, link the new tags */
      newnotag->next = m->tags;
      m->tags = newnotag;

      newnmtag->next = o->tags;
      o->tags = newnmtag;
    } /* if */

 return ay_status;
} /* ay_tags_addnonm */


/** ay_tags_remnonm:
 *  remove NO/NM tags
 *
 * \param[in,out] o object with nm tag (typically a instance)
 * \param[in,out] m object with no tag (typically the master of o)
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_tags_remnonm(ay_object *o, ay_object *m)
{
 int ay_status = AY_OK;
 ay_tag *tag, **lasttag;
 int found = AY_FALSE;

  if(!o || !m)
    return AY_ENULL;

  /* look for and remove the NO tag */
  tag = m->tags;
  lasttag = &(m->tags);
  while(tag)
    {
      if(tag->type == ay_no_tagtype && tag->is_binary)
	{
	  if(o == ((ay_btval*)tag->val)->payload)
	    {
	      *lasttag = tag->next;
	      ay_tags_free(tag);
	      found = AY_TRUE;
	      break;
	    }
	}
      lasttag = &(tag->next);
      tag = tag->next;
    } /* while */

  /* look for and remove corresponding NM tag only if we found the NO tag */
  if(found)
    {
      tag = o->tags;
      lasttag = &(o->tags);
      while(tag)
	{
	  if(tag->type == ay_nm_tagtype && tag->is_binary)
	    {
	      if(m == ((ay_btval*)tag->val)->payload)
		{
		  *lasttag = tag->next;
		  ay_tags_free(tag);
		  break;
		}
	    }
	  lasttag = &(tag->next);
	  tag = tag->next;
	} /* while */
    } /* if */

 return ay_status;
} /* ay_tags_remnonm */


/** ay_tag_copyselected:
 * Copy selected tags from an object to a list of other objects.
 *
 * \param[in] src object that contains the tags to be copied
 * \param[in,out] dst a list of objects that receive the copied tags
 * \param[in] types tag types to copy [typeslen]
 * \param[in] typeslen number of types
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_tag_copyselected(ay_object *src, ay_object *dst,
		    char **types, int typeslen)
{
 int ay_status = AY_OK;
 int i, copy;
 ay_object *o = NULL;
 ay_tag *t = NULL, *s = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!src->tags || !typeslen)
    return AY_OK;

  t = src->tags;

  while(t)
    {
      copy = AY_FALSE;
      for(i = 0; i < typeslen; i++)
	{
	  if(types[i] == t->type)
	    {
	      copy = AY_TRUE;
	      break;
	    }
	}
      if(copy)
	{
	  o = dst;
	  while(o && o != ay_endlevel)
	    {
	      s = o->tags;
	      ay_status = ay_tags_copy(t, &(o->tags));
	      if(ay_status)
		break;
	      o->tags->next = s;
	      o = o->next;
	    } /* while */
	} /* if */
      t = t->next;
    } /* while */

 return ay_status;
} /* ay_tag_copyselected */
