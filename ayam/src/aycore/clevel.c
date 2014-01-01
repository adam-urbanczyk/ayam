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

#include <ctype.h>

/* clevel.c - functions for current level management */

/* ay_clevel_set:
 *  set list object on top of current level stack
 *  pointing to <o>
 */
void
ay_clevel_set(ay_object *o)
{

  if(ay_currentlevel)
    ay_currentlevel->object = o;

 return;
} /* ay_clevel_set */


/* ay_clevel_find:
 *  _recursively_ search through all objects beneath and below <c>
 *  for object <o> and build a stack of list objects pointing from the
 *  ay_root to the object <o> in <ay_currentlevel>
 */
int
ay_clevel_find(ay_object *c, ay_object *o, int *found)
{
 int ay_status = AY_OK;

 /**found = AY_FALSE;*/
  while(c->next)
    {
      if(c != o)
	{
	  if(c->down)
	    {
	      ay_status = ay_clevel_find(c->down, o, found);
	      if(ay_status)
		break;
	      if(*found)
		{
		  ay_status = ay_clevel_add(c);
		  if(ay_status)
		    break;
		  ay_status = ay_clevel_add(c->down);
		  if(ay_status)
		    break;
		  return AY_OK;
		}
	    } /* if */
	}
      else
	{
	  *found = AY_TRUE;
	  return AY_OK;
	} /* if */
      c = c->next;
    } /* while */

 return ay_status;
} /* ay_clevel_find */


/* ay_clevel_add:
 *  put list object pointing to <o> to top of current level stack
 */
int
ay_clevel_add(ay_object *o)
{
 ay_list_object *lev = NULL;

  if(!(lev = malloc(sizeof(ay_list_object))))
    return AY_EOMEM;

  lev->object = o;

  if(ay_currentlevel)
    lev->next = ay_currentlevel;
  else
    lev->next = NULL;

  ay_currentlevel = lev;

 return AY_OK;
} /* ay_clevel_add */


/* ay_clevel_del:
 *  remove topmost list object from current level stack
 */
void
ay_clevel_del(void)
{
 ay_list_object *lev = ay_currentlevel;

  if(lev && lev->next)
    {
      ay_currentlevel = lev->next;
      free(lev);
    }

 return;
} /* ay_clevel_del */


/* ay_clevel_delall:
 *  deletes all clevel objects but not the first
 *  two that point to NULL and to ay_root
 */
void
ay_clevel_delall(void)
{
 ay_list_object *lev = ay_currentlevel;

  while(lev && lev->next && lev->next->next)
    {
      ay_currentlevel = lev->next;
      free(lev);
      lev = ay_currentlevel;
    }

 return;
} /* ay_clevel_delall */


/* ay_clevel_gotop:
 *  change the current level to the top level;
 *  additionally, reset ay_next to the end of the top level
 */
void
ay_clevel_gotop()
{
 ay_object *o = ay_root;

  ay_clevel_delall();

  if(o)
    {
      while(o->next)
	{
	  ay_next = &(o->next);
	  o = o->next;
	}
    }

 return;
} /* ay_clevel_gotop */


/* ay_clevel_gotoptcmd:
 *  Implements the \a goTop scripting interface command.
 *  See also the corresponding section in the \ayd{scgotop}.
 *  \returns TCL_OK in any case.
 */
int
ay_clevel_gotoptcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 /* char varName[20], newValue[10];*/

  ay_clevel_gotop();

#if 0
  /* Synchronize the value of ay(CurrentLevel) */
  sprintf(varName, "ay(CurrentLevel)");
  sprintf(newValue, "root");
  Tcl_SetVar(interp, varName, newValue,
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
#endif
 return TCL_OK;
} /* ay_clevel_gotoptcmd */


/* ay_clevel_gouptcmd:
 *  Implements the \a goUp scripting interface command.
 *  See also the corresponding section in the \ayd{scgoup}.
 *  \returns TCL_OK in any case.
 */
int
ay_clevel_gouptcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 /*int ay_status = AY_OK;*/
 ay_object *o = NULL;
 /*
 char fname[] = "goUp";
 char *part1 = "ay", *part2 = "CurrentLevel";
 char *tmp = NULL, *string = NULL;
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL, *no = NULL;
 int length = 0;
 */

  if(ay_currentlevel->object != ay_root)
    {
      ay_clevel_del();
      ay_clevel_del();

      o = ay_currentlevel->object;
      if(o)
	{
	  while(o->next)
	    {
	      ay_next = &(o->next);
	      o = o->next;
	    }
	}
    }

#if 0
  /* Synchronize the value of ay(CurrentLevel) */
  toa = Tcl_NewStringObj(part1, -1);
  ton = Tcl_NewStringObj(part2, -1);

  to = Tcl_ObjGetVar2(interp, toa, ton,
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  /* Get ay(CurrentLevel) value */
  string = Tcl_GetStringFromObj(to, &length);

  /* Duplicate the value ay(CurrentLevel) since the original string
     must not be modified */
  tmp = malloc(length * sizeof(char));
  if(!tmp)
    {
      ay_error(AY_EOMEM, fname, NULL);
      Tcl_IncrRefCount(toa); Tcl_DecrRefCount(toa);
      Tcl_IncrRefCount(ton); Tcl_DecrRefCount(ton);
      return TCL_OK;
    }
  sprintf(tmp, "%s", string);

  /* Remove the last component */
  while ((length > 0) && (tmp[length] != ':'))
    {
      length--;
    }
  if (length > 0)
    {
      tmp[length] = 0;
    }

  /* Replace the old value */
  /*  Tcl_SetStringObj(to, tmp, -1);*/

  no = Tcl_NewStringObj(tmp, -1);
  Tcl_ObjSetVar2(interp, toa, ton, no,
		 TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  if(tmp)
    free(tmp);

  Tcl_IncrRefCount(toa); Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton); Tcl_DecrRefCount(ton);
#endif
 return TCL_OK;
} /* ay_clevel_gouptcmd */


/* ay_clevel_godowntcmd:
 *  Implements the \a goDown scripting interface command.
 *  See also the corresponding section in the \ayd{scgodown}.
 *  \returns TCL_OK in any case.
 */
int
ay_clevel_godowntcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 int found = AY_FALSE, j = 0, argvi = 0;
 ay_object *o = NULL;
 /*
 char *part1 = "ay", *part2 = "CurrentLevel";
 char tmp[256];
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL, *no = NULL;
 */

  /* check args */
  if(argc != 2)
    {
      /* ay_error(AY_EARGS, argv[0], "index"); */
      return TCL_OK;
    }

  o = ay_currentlevel->object;

  if(!o)
    return TCL_OK;

  argvi = atoi(argv[1]);

  if(argvi == -1)
    {
      /* user specified "-1" => select last object */
      while(o->next && o->next->next)
	{
	  o = o->next;
	}
      found = AY_TRUE;
    }
  else
    {
      /* user specified an index => find and select appropriate object */

      /* are we in a sublevel (not root)? */
      if(ay_root != o)
	{
	  /* Yes */
	  if(argvi == 0)
	    {
	      /* ".." selected? -> go up instead! */
	      return ay_clevel_gouptcmd(clientData, interp, argc, argv);
	    }
	  j = 1;
	}

      while(o->next)
	{
	  if(argvi == j)
	    {
	      /* found the selected object */
	      found = AY_TRUE;
	      break;
	    } /* if */
	  j++;
	  o = o->next;
	} /* while */
    } /* if */

  if(found && o && o->parent)
    {
      ay_status = ay_clevel_add(o);
      if(ay_status)
	{
	  ay_error(ay_status, argv[0], NULL);
	  return TCL_OK;
	}

      ay_status = ay_clevel_add(o->down);
      if(ay_status)
	{
	  ay_clevel_del();
	  ay_error(ay_status, argv[0], NULL);
	  return TCL_OK;
	}

      if(!o->down)
	{
	  ay_error(AY_ENULL, argv[0], NULL);
	  return TCL_OK;
	}

      /* if there is just a single object in this level
	 (this is then an EndLevel object) we add the next
	 object to the down link */
      if(!o->down->next)
	{
	  ay_next = &(o->down);
	}
      else
	{
	  o = o->down;
	  while(o->next)
	    {
	      ay_next = &(o->next);
	      o = o->next;
	    } /* while */
	} /* if */

#if 0
      /* Synchronize the value of ay(CurrentLevel) */
      toa = Tcl_NewStringObj(part1, -1);
      ton = Tcl_NewStringObj(part2, -1);

      /* Get ay(CurrentLevel) value */
      to = Tcl_ObjGetVar2(interp, toa, ton,
			  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

      no = Tcl_NewStringObj(Tcl_GetStringFromObj(to, NULL), -1);

      /* Append the new component of the level */
      if(strcmp(Tcl_GetStringFromObj(to, NULL), "root"))
	{
	  sprintf(tmp, ":%d", argvi-1);
	} else {
	  sprintf(tmp, ":%d", argvi);
	}

      Tcl_AppendStringsToObj(no, tmp, NULL);
      Tcl_ObjSetVar2(interp, toa, ton, no,
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

      Tcl_IncrRefCount(toa); Tcl_DecrRefCount(toa);
      Tcl_IncrRefCount(ton); Tcl_DecrRefCount(ton);
#endif

    } /* if */

 return TCL_OK;
} /* ay_clevel_godowntcmd */


/* ay_clevel_gettcmd:
 *  Implements the \a getLevel scripting interface command.
 *  See also the corresponding section in the \ayd{scgetlevel}.
 *  \returns TCL_OK in any case.
 */
int
ay_clevel_gettcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 ay_object *o = ay_currentlevel->object;
 char *name;
 char *typename;
 Tcl_DString ds;
 int count = 0;
 Tcl_Obj *to = NULL, *toa = NULL;

  /* check args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "[-l] varname [varname]");
      return TCL_OK;
    }

  if(!o)
    {
      ay_error(AY_ENULL, argv[0], NULL);
      return TCL_OK;
    }

  if(argv[1][0] == '-' && argv[1][1] == 'l')
    {
      /* just count the objects in the current level */
      while(o->next)
	{
	  count++;
	  o = o->next;
	}

      toa = Tcl_NewStringObj(argv[2], -1);
      to = Tcl_NewIntObj(count);
      Tcl_ObjSetVar2(interp, toa, NULL, to, TCL_LEAVE_ERR_MSG);

      return TCL_OK;
    }

  Tcl_SetVar(interp, argv[1], "", TCL_LEAVE_ERR_MSG);
  if(argc > 1)
    Tcl_SetVar(interp, argv[2], "", TCL_LEAVE_ERR_MSG);

  if(ay_currentlevel->object != ay_root)
    {
      Tcl_SetVar(interp, argv[1], "..",
		 TCL_APPEND_VALUE | TCL_LEAVE_ERR_MSG);
      if(argc > 1)
	Tcl_SetVar(interp, argv[2], "..",
		   TCL_APPEND_VALUE | TCL_LEAVE_ERR_MSG);
    }

  /* o->next because we silently discard the ever present EndLevel object */
  while(o->next)
    {
      name = ay_object_getname(o);

      if(name)
	{
	  if((o->name) || (o->type == AY_IDINSTANCE))
	    {
	      if(ay_prefs.list_types)
		{
		  typename = ay_object_gettypename(o->type);
		  Tcl_DStringInit(&ds);
		  if(ay_prefs.mark_hidden && o->hide)
		    {
		      Tcl_DStringAppend(&ds, "!", -1);
		    }
		  Tcl_DStringAppend(&ds, name, -1);
		  Tcl_DStringAppend(&ds, "(", -1);
		  Tcl_DStringAppend(&ds, typename, -1);
		  Tcl_DStringAppend(&ds, ")", -1);

		  Tcl_SetVar(interp, argv[1], Tcl_DStringValue(&ds),
		     TCL_APPEND_VALUE | TCL_LIST_ELEMENT |
		     TCL_LEAVE_ERR_MSG);

		  Tcl_DStringFree(&ds);

		}
	      else
		{
		  if(ay_prefs.mark_hidden && o->hide)
		    {
		      Tcl_DStringInit(&ds);
		      Tcl_DStringAppend(&ds, "!", -1);
		      Tcl_DStringAppend(&ds, name, -1);
		      Tcl_SetVar(interp, argv[1], Tcl_DStringValue(&ds),
				 TCL_APPEND_VALUE | TCL_LIST_ELEMENT |
				 TCL_LEAVE_ERR_MSG);

		      Tcl_DStringFree(&ds);
		    }
		  else
		    {
		      Tcl_SetVar(interp, argv[1], name, TCL_APPEND_VALUE |
				 TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);
		    } /* if */
		} /* if */
	    }
	  else
	    {
	      if(ay_prefs.mark_hidden && o->hide)
		{
		  Tcl_DStringInit(&ds);
		  Tcl_DStringAppend(&ds, "!", -1);
		  Tcl_DStringAppend(&ds, name, -1);
		  Tcl_SetVar(interp, argv[1], Tcl_DStringValue(&ds),
			     TCL_APPEND_VALUE | TCL_LIST_ELEMENT |
			     TCL_LEAVE_ERR_MSG);

		  Tcl_DStringFree(&ds);
		}
	      else
		{
		  Tcl_SetVar(interp, argv[1], name, TCL_APPEND_VALUE |
			     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);
		}
	    } /* if */
	}
      else
	{
	  ay_error(AY_ENULL, argv[0], NULL);
	  return TCL_OK;
	} /* if */

      if(argc > 1)
	{
	  typename = ay_object_gettypename(o->type);
	  if(typename)
	    {
	      Tcl_SetVar(interp, argv[2], typename, TCL_APPEND_VALUE |
			 TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);
	    }
	  else
	    {
	      ay_error(AY_ENULL, argv[0], NULL);
	      return TCL_OK;
	    }
	}

      o = o->next;
    } /* while */

 return TCL_OK;
} /* ay_clevel_gettcmd */


/* ay_clevel_dup:
 *  _recursively_ duplicates a current level list (<src>) in <ay_currentlevel>
 */
int
ay_clevel_dup(ay_list_object *src)
{
 int ay_status = AY_OK;

  if(src->next)
    {
      if((ay_status = ay_clevel_dup(src->next)))
	return ay_status;
    }

  ay_status = ay_clevel_add(src->object);

 return ay_status;
} /* ay_clevel_dup */


/* ay_clevel_cltcmd:
 *  Tcl command to set the current level (cl - change level)
 */
int
ay_clevel_cltcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
 int ay_status = AY_OK;
 static ay_list_object *ocl = NULL;
 static ay_object **oay_next = NULL;
 ay_list_object *tcl;
 ay_object **tay_next = NULL;
 ay_object *o = ay_root, *t;
 int i, lindex = 0;
 char *lc = NULL; /* level component */

  /* check args */
  if(argc != 2)
    {
      ay_error(AY_EARGS, argv[0], "( - | level )");
      return TCL_OK;
    }

  if(*argv[1] == '-')
    {
      /* change back to last saved level */
      if(ocl)
	{
	  tcl = ay_currentlevel;
	  ay_currentlevel = ocl;
	  ocl = tcl;

	  tay_next = ay_next;
	  ay_next = oay_next;
	  oay_next = tay_next;
	}
      else
	{
	  /* report error */
	  ay_error(AY_ERROR, argv[0], "no level saved");
	  return TCL_OK;
	}
    }
  else
    {
      /* first, save current level, for a potential "cl -" */
      while(ocl)
	{
	  tcl = ocl->next;
	  free(ocl);
	  ocl = tcl;
	}
      oay_next = ay_next;
      ocl = ay_currentlevel;
      ay_currentlevel = NULL;
      ay_status = ay_clevel_dup(ocl);
      if(ay_status)
	{
	  ay_clevel_delall();
	  if(ay_currentlevel)
	    {
	      if(ay_currentlevel->next)
		free(ay_currentlevel->next);
	      free(ay_currentlevel);
	    }
	  ay_currentlevel = ocl;
	  ay_next = oay_next;
	  ay_error(AY_ERROR, argv[0], "Could not save current level.");
	  return TCL_OK;
	}

      /* now, change to new level */
      lc = argv[1];

      /* is the new level expressed in a relative way? */
      if(*argv[1] == '.')
	{
	  /* yes, relative level */
	  while(lc && (strlen(lc) > 2) && !(strcmp("..", lc)))
	    {
	      ay_clevel_del();
	      ay_clevel_del();
	      lc += 3;
	    } /* while */
	  o = ay_currentlevel->object;
	  t = o;
	}
      else
	{
	  /* no, absolute level */
	  ay_clevel_delall();
	  t = ay_root;
	} /* if */

      ay_next = &(t->next);
      while(t->next)
	{
	  ay_next = &(t->next);
	  t = t->next;
	}

      /* get first number */
      while(*lc != '\0' && !isdigit(*lc))
	{
	  lc++;
	}
      while(*lc != '\0')
	{
	  lindex = 0;
	  sscanf(lc, "%d", &lindex);

	  /* find and add object */
	  i = 0;
	  while(o && o->next && (i < lindex))
	    {
	      i++;
	      o = o->next;
	    }
	  if(o && o->next && o->parent)
	    {
	      /* all is well => descend for next level index */
	      ay_status = ay_clevel_add(o);
	      if(ay_status)
		{
		  ay_error(AY_ERROR, argv[0], NULL);
		  return TCL_OK;
		}
	      ay_next = &(o->down);
	      o = o->down;
	      ay_status = ay_clevel_add(o);
	      if(ay_status)
		{
		  ay_clevel_del();
		  ay_error(AY_ERROR, argv[0], NULL);
		  return TCL_OK;
		}

	      t = o;
	      while(t->next)
		{
		  ay_next = &(t->next);
		  t = t->next;
		}
	    }
	  else
	    {
	      /* error, we run over end of level => index is wrong */
	      /* cleanup */
	      ay_clevel_delall();
	      if(ay_currentlevel)
		{
		  if(ay_currentlevel->next)
		    free(ay_currentlevel->next);
		  free(ay_currentlevel);
		}
	      ay_currentlevel = ocl;
	      ocl = NULL;
	      ay_next = oay_next;
	      oay_next = NULL;
	      /* report error */
	      ay_error(AY_ERROR, argv[0], "could not find level");
	      return TCL_OK;
	    } /* if */

	  /* jump over the index number that we just processed */
	  while(*lc != '\0' && isdigit(*lc))
	    {
	      lc++;
	    }
	  /* jump over separator */
	  if(*lc != '\0')
	    {
	      lc++;
	    }
	} /* while */

    } /* if */

 return TCL_OK;
} /* ay_clevel_cltcmd */
