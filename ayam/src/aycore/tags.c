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

/* tags.c - functions for tag management */


/* ay_tags_free:
 *  free a tag
 */
int
ay_tags_free(ay_tag_object *tag)
{
  int ay_status = AY_OK;

  if(!tag)
    return AY_OK;

  if(tag->name)
    free(tag->name);

  if(tag->val)
    free(tag->val);

  free(tag);

 return ay_status;
} /* ay_tags_free */


/* ay_tags_delall:
 *  remove all tags from object *o
 */
int
ay_tags_delall(ay_object *o)
{
 ay_tag_object *t = NULL, *nt = NULL;

  if(!o)
    return AY_OK;

  t = o->tags;
  while(t)
    {
      nt = t->next;
      ay_tags_free(t);
      t = nt;
    }

  o->tags = NULL;

 return AY_OK;
} /* ay_tags_delall */


/* ay_tags_copy:
 *  copy a tag
 */
int
ay_tags_copy(ay_tag_object *source,
	     ay_tag_object **dest)
{
 int ay_status = AY_OK;
 ay_tag_object *new = NULL;

  if(!(new = calloc(1,sizeof(ay_tag_object))))
    return AY_EOMEM; 

  memcpy(new, source, sizeof(ay_tag_object)); 
  /* danger! links point to original hierachy */

  new->next = NULL;

  /* copy name */
  if(!(new->name = calloc(1, strlen(source->name)+1)))
    { free(new); return AY_EOMEM; }
  strcpy(new->name, source->name);

  /* copy val */
  if(!(new->val = calloc(1, strlen(source->val)+1)))
    { free(new); free(new->name); return AY_EOMEM; }
  strcpy(new->val, source->val);

  *dest = new;

 return ay_status;
} /* ay_tags_copy */


/* ay_tags_copyall:
 *  copy all tags from object src to object dst
 */
int
ay_tags_copyall(ay_object *src, ay_object *dst)
{
 int ay_status = AY_OK;
 ay_tag_object *tag = NULL, **newtagptr = NULL;

  tag = src->tags;
  newtagptr = &(dst->tags);
  while(tag)
    {
      ay_status = ay_tags_copy(tag, newtagptr);
      newtagptr = &((*newtagptr)->next);
      *newtagptr = NULL;
      tag = tag->next;
    }

 return AY_OK;
} /* ay_tags_copyall */


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
      return TCL_OK;
    }

  tagcounter++;

  entry = Tcl_CreateHashEntry(&ay_tagtypesht, name, &new_item);
  Tcl_SetHashValue(entry, tagcounter);

  *result = tagcounter;

 return TCL_OK;
} /* ay_tags_register */


/* ay_tags_temp:
 *  if set == 1: mark tag type "name" temporary, it will not be saved
 *  if set == 0: set result to 1 if tag type "name" is temporary
 *               else set result to 0
 */
int
ay_tags_temp(Tcl_Interp *interp, char *name, int set, int *result)
{
 int new_item = 0;
 Tcl_HashEntry *entry = NULL;
 char fname[] = "tags_temp";

  if(set == 1)
    { /* set */
      if((entry = Tcl_FindHashEntry(&ay_temptagtypesht, name)))
	{
	  ay_error(AY_ERROR, fname, "tag type already marked temporary");
	  return TCL_OK;
	}

      entry = Tcl_CreateHashEntry(&ay_temptagtypesht, name, &new_item);
      Tcl_SetHashValue(entry, 1);
    }
  else
    { /* query */
      if((entry = Tcl_FindHashEntry(&ay_temptagtypesht, name)))
	{
	  *result = AY_TRUE;
	}
      else
	{
	  *result = AY_FALSE;
	}
    } /* if */

 return TCL_OK;
} /* ay_tags_temp */


/* ay_tags_istemptcmd:
 *  
 */
int
ay_tags_istemptcmd(ClientData clientData, Tcl_Interp * interp,
		   int argc, char *argv[])
{
 char fname[] = "tagIsTemp";
 int temp;
 char yes[] = "1", no[] = "0";

  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "tagname");
    }

  ay_tags_temp(interp, argv[1], 0, &temp);

  if(temp)
    {
      Tcl_SetResult(interp, yes, TCL_VOLATILE);     
    }
  else
    {
      Tcl_SetResult(interp, no, TCL_VOLATILE);
    }

 return TCL_OK;
} /* ay_tags_istemptcmd */


/* ay_tags_settcmd:
 *  
 */
int
ay_tags_settcmd(ClientData clientData, Tcl_Interp * interp,
		int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_tag_object *new = NULL, **next = NULL;
 Tcl_HashEntry *entry = NULL;
 int index = 0, i;
 char fname[] = "setTags";

  if(argc < 3)
    {
      ay_error(AY_EARGS, fname, "\\[type value\\]|-index index type value");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  if(!sel->object)
    return TCL_OK;

  if(!strcmp(argv[1],"-index"))
    {
      if(argc < 5)
	{
	  ay_error(AY_EARGS, fname,
		   "\\[type value\\]|-index index type value");
	  return TCL_OK;
	}

      Tcl_GetInt(ay_interp, argv[2], &index);

      /* find tag */
      o = sel->object;
      new = o->tags;
      for(i = 0; i < index; i++)
	{
	  if(!new)
	    {
	      ay_error(AY_ERROR, fname, "Tag not found!");
	      return TCL_OK;
	    }
	  new = new->next;
	} /* for */

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
	      ay_error(AY_EWARN, fname, "Tag type is not registered!");
	    }
	  if(entry)
	    {
	      new->type = (char *)Tcl_GetHashValue(entry);
	    }

	  if(!(new->name = calloc(strlen(argv[3])+1, sizeof(char))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  strcpy(new->name, argv[3]);


	  if(!(new->val = calloc(strlen(argv[4])+1, sizeof(char))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  strcpy(new->val, argv[4]);
	  
	} /* if */

      return TCL_OK; /* early exit! */
    } /* if */

  while(sel)
    {
      o = sel->object;

      /* delete old tags */
      ay_tags_delall(o);
      next = &(o->tags);
      for(index = 1; index < argc-1; index += 2)
	{
	  if(strcmp(argv[index], ""))
	    {

	      if(!(new = calloc(1,sizeof(ay_tag_object))))
		{
		  ay_error(AY_EOMEM, fname, NULL);
		  return TCL_OK;
		}

	      /* we first try to resolve the tag type */
	      if(!(entry = Tcl_FindHashEntry(&ay_tagtypesht, argv[index])))
		{
		  ay_error(AY_EWARN, fname, "Tag type is not registered!");
		}
	      if(entry)
		{
		  new->type = (char *)Tcl_GetHashValue(entry);
		}

	      if(!(new->name = calloc(strlen(argv[index])+1, sizeof(char))))
		{
		  ay_error(AY_EOMEM, fname, NULL);
		  return TCL_OK;
		}
	      strcpy(new->name, argv[index]);

	      if(!(new->val = calloc(strlen(argv[index+1])+1, sizeof(char))))
		{
		  ay_error(AY_EOMEM, fname, NULL);
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


/* ay_tags_addtcmd:
 *  
 */
int
ay_tags_addtcmd(ClientData clientData, Tcl_Interp * interp,
		int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_tag_object *new = NULL, *t = NULL;
 Tcl_HashEntry *entry = NULL;
 char fname[] = "addTag";

   if(argc < 3)
     {
       ay_error(AY_EARGS, fname, "type value");
       return TCL_OK;
     }
 
  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  if(!sel->object)
    return TCL_OK;

  while(sel)
    {
      o = sel->object;

      /* we first try to resolve the tag type */
      if(!(entry = Tcl_FindHashEntry(&ay_tagtypesht, argv[1])))
	{
	  ay_error(AY_EWARN, fname, "Tag type is not registered!");
	}
      if(!(new = calloc(1,sizeof(ay_tag_object))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return TCL_OK;
	}
      if(!(new->name = calloc(strlen(argv[1])+1, sizeof(char))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return TCL_OK;
	}
      strcpy(new->name,argv[1]);

      if(entry)
	new->type = (char *)Tcl_GetHashValue(entry);

      if(!(new->val = calloc(strlen(argv[2])+1, sizeof(char))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  return TCL_OK;
	}
      strcpy(new->val,argv[2]);

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
 *  
 */
int
ay_tags_gettcmd(ClientData clientData, Tcl_Interp * interp,
		   int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_tag_object *tag = NULL;
 char fname[] = "setTags";

  if(argc < 3)
    {
      ay_error(AY_EARGS, fname, "varname varname2");
      return TCL_OK;
    }

  if(!sel)
    return TCL_OK;

  if(!sel->object)
    return TCL_OK;

  Tcl_SetVar(interp,argv[1], "", TCL_LEAVE_ERR_MSG);
  Tcl_SetVar(interp,argv[2], "", TCL_LEAVE_ERR_MSG);

  o = sel->object;
  if(o)
    {
      tag = o->tags;
      while(tag)
	{
	  Tcl_SetVar(interp,argv[1],tag->name, TCL_APPEND_VALUE |\
		     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);
	  Tcl_SetVar(interp,argv[2],tag->val, TCL_APPEND_VALUE |\
		     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);

	  tag = tag->next;
	} /* while */
    } /* if */
 
  return TCL_OK;
} /* ay_tags_gettcmd */


/* ay_tags_deletetcmd:
 *  
 */
int
ay_tags_deletetcmd(ClientData clientData, Tcl_Interp * interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 ay_tag_object *tag = NULL, **last = NULL;
 char fname[] = "delTags";
 int mode = 0; /* 0 delall, 1 type */

  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "type\\|all");
      return TCL_OK;
    }

  if(!sel)
    return TCL_OK;

  if(strcmp(argv[1], "all"))
     mode = 1;

  while(sel)
    {
      o = sel->object;
      last = &(o->tags);
      if(mode)
	{
	  tag = o->tags;
	  while(tag)
	    {
	      if(!strcmp(argv[1], tag->name))
		{
		  *last = tag->next;
		  ay_tags_free(tag);
		  tag = *last;
		}
	      else
		{
		  last = &(tag->next);
		  tag = tag->next;
		}
	    } /* while */
	}
      else
	{
	  ay_status = ay_tags_delall(o);
	}
      sel = sel->next;
    } /* while */
 
  return TCL_OK;
} /* ay_tags_deletetcmd */

