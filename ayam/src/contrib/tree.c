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

/* tree.c - Ayam Object Hierarchy Tree Widget */

/* this hash table links the image names with the corresponding creation
   functions for drag and drop creation */
Tcl_HashTable ay_creatednd_ht;

/* XXXX nearly all prototypes are missing! */


/* ay_tree_registerdrop:
 *  register the tree drop callback cb for
 *  objects of type type_id
 */
int
ay_tree_registerdrop(ay_treedropcb *cb, unsigned int type_id)
{
 int ay_status = AY_OK;
 int overload = ay_prefs.overload;

  /* register callback */
  ay_status = ay_table_additem(&ay_treedropcbt, (void*)cb, type_id,
			       overload);

 return ay_status;
} /* ay_tree_registerdrop */


/* ay_tree_dellist:
 *  frees a ay_list_object-list
 */
void
ay_tree_dellist(ay_list_object *list)
{
 ay_list_object *next;

  while (list) {
      next = list->next;
      free(list);
      list = next;
  }

 return;
} /* ay_tree_dellist */


/* ay_tree_getclevel:
 *  recreate clevel list from the given node
 */
void
ay_tree_getclevel(char *node)
{
 int ay_status = AY_OK;
 int i = 4, p, n;
 ay_object *o = ay_root;;
 
  ay_status = ay_clevel_delall();

  if (memcmp(node, "root:", 5))
    {
      if(o)
	{
	  while(o->next)
	    {
	      ay_next = &(o->next);
	      o = o->next;
	    }
	}
      return;
    }

  while(node[i] == ':')
    {
      i++;
      p = i;

      while((node[p] >= '0') && (node[p] <= '9'))
	{
	  p++;
	}

      if(node[p] == ':')
	{
	  node[p] = 0;
	  n = strtol(&node[i], NULL, 10);
	  node[p] = ':';
	  i = p;

	  while((o) && (n))
	    {
	      o = o->next; n--;
	    } /* while */

	  if((o) && (o->down))
	    {
	      ay_status = ay_clevel_add(o);
	      ay_status = ay_clevel_add(o->down);
	      o = o->down;
	    }
	  else
	    {
	      return;
	    } /* if */

	}
      else
	{
	  i = p;
	} /* if */

    } /* while */

  if(o)
    {
      while(o->next)
	{
	  ay_next = &(o->next);
	  o = o->next;
	}
    }

 return;
} /* ay_tree_getclevel */


/* ay_tree_getobject:
 *  returns an object pointer to a given node
 */
ay_object *
ay_tree_getobject(char *node)
{
 int i = 4, p, n, c;
 ay_object *o = NULL;
 
  if(memcmp(node, "root:", 5))
    return NULL;

  while(node[i] == ':')
    {
      i++;
      p = i;

      while((node[p] >= '0') && (node[p] <= '9'))
	{
	  p++;
	}

      c = node[p]; node[p] = 0;
      n = strtol(&node[i], NULL, 10);
      node[p] = c; i = p;

      if(o)
	{
	  o = o->down;
	}
      else
	{
	  o = ay_root;
	}

      while((o) && (n))
	{
	  o = o->next; n--;
	}

      if(o == NULL)
	return NULL;
    }

 return o;
} /* ay_tree_getlevel */


/* ay_tree_crtnodename:
 *  creates a Tcl-node string from a list
 *  of level object objects, that lead to this
 *  object; list has to be in order
 * (not like the current level list)
 */
int
ay_tree_crtnodename(ay_object *parent, ay_list_object *list, Tcl_DString *ds)
{
 int ay_status = AY_OK;
 int pos;
 ay_object *o = NULL;
 char buf[64] = "";


  if(list)
    {
      o = parent;

      pos = 0;
      while((o) && (o != list->object))
	{
	  o = o->next;
	  pos++;
	}

      if(o)
	{
	  sprintf(buf, "%d ", pos);
	  Tcl_DStringAppend(ds, buf, -1);
	}
      else
	{
	  return AY_ENULL;
	}

      if ((ay_status = ay_tree_crtnodename(list->object->down,list->next, ds)))
	return ay_status;
    }

 return AY_OK;
} /* ay_tree_crtnodename */


/* ay_tree_crttreestring:
 *  create a tree structure in a Tcl string list
 */
void
ay_tree_crttreestring(Tcl_Interp *interp, ay_object *o,
		      Tcl_DString *ds)
{
 char *name = NULL;
 char *tname = NULL;

  if(!o)
    return;

  /* omit trailing EndLevel-Object! */
  while(o->next)
    {
      name = NULL;
      name = ay_object_getname(o);

      if(o->down)
	{
	  Tcl_DStringAppend(ds, "{ ", -1);
	  
	  Tcl_DStringAppend(ds, name, -1);
	  if((o->name) || (o->type == AY_IDINSTANCE))
	    {
	      if(ay_prefs.list_types)
		{
		  tname = ay_object_gettypename(o->type);
		  Tcl_DStringAppend(ds, "(", -1);
		  Tcl_DStringAppend(ds, tname, -1);
		  Tcl_DStringAppend(ds, ")", -1);
		}
	    }
	  Tcl_DStringAppend(ds, " ", -1);

	  ay_tree_crttreestring(interp, o->down, ds);

	  Tcl_DStringAppend(ds, "} ", -1);
	}
      else
	{
	  Tcl_DStringAppend(ds, name, -1);
	  if((o->name) || (o->type == AY_IDINSTANCE))
	    {
	      if(ay_prefs.list_types)
		{
		  tname = ay_object_gettypename(o->type);
		  Tcl_DStringAppend(ds, "(", -1);
		  Tcl_DStringAppend(ds, tname, -1);
		  Tcl_DStringAppend(ds, ")", -1);
		}
	    }
	  Tcl_DStringAppend(ds, " ", -1);
	} /* if */

      o = o->next;
  } /* while */

 return;
} /* ay_tree_crttreestring */


/* ay_tree_crtstringfromobj:
 * 
 */
int
ay_tree_crtstringfromobj(ay_object *level, ay_object *o, ay_list_object *list,
			 Tcl_DString *ds)
{
 int ay_status = AY_OK;
 ay_list_object *new = NULL, *tmp = NULL, *last = NULL;


  if(!level)
    return AY_ENULL;

  /* omit trailing EndLevel-Object! */
  while(level->next)
    {
      if(level == o)
	{
	  if(!(new = calloc(1,sizeof(ay_list_object))))
	    return AY_EOMEM;

	  tmp = list;
	  while(tmp->next)
	    tmp = tmp->next;
	  tmp->next = new;

	  new->object = level;

	  ay_tree_crtnodename(ay_root, list->next, ds);

	  tmp = list;
	  while(tmp->next)
	    {
	      last = tmp;
	      tmp = tmp->next;
	    }

	  free(last->next);	      
	  last->next = NULL;
	}
      else
	{
	  if(level->down)
	    {

	      new = NULL;
	      if(!(new = calloc(1,sizeof(ay_list_object))))
		return AY_EOMEM;

	      new->object = level;
	      tmp = list;
	      while(tmp->next)
		tmp = tmp->next;
	      tmp->next = new;

	      ay_status = ay_tree_crtstringfromobj(level->down, o, list, ds);

	      tmp = list;
	      while(tmp->next)
		{
		  last = tmp;
		  tmp = tmp->next;
		}

	      free(last->next);	      
	      last->next = NULL;

	    } /* if */
	} /* if */
      level = level->next;
  } /* while */

 return AY_OK;
} /* ay_tree_crtstringfromobj */


/* ay_tree_gettreetcmd:
 *  create a tree structure in a Tcl string list
 */
int
ay_tree_gettreetcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 char fname[] = "treeGetString";
 ay_object *o = NULL;
 Tcl_DString ds;

  if(argc != 3)
    {
      ay_error(AY_EARGS, fname, "varname node");
      return TCL_OK;
    }

  o = ay_tree_getobject(argv[2]);

  if(o)
    {
      o = o->down;
    }
  else
    {
      o = ay_root;
    }

  Tcl_DStringInit(&ds);

  Tcl_UnsetVar(interp, argv[1], TCL_LEAVE_ERR_MSG);
  Tcl_SetVar(interp, argv[1], "", TCL_LEAVE_ERR_MSG);

  ay_tree_crttreestring(interp, o, &ds);
  
  Tcl_SetVar(interp, argv[1], Tcl_DStringValue(&ds), TCL_LEAVE_ERR_MSG);

  Tcl_DStringFree(&ds);

 return TCL_OK;
} /* ay_tree_gettreetcmd */


/* ay_tree_selecttcmd:
 *  select objects
 */
int
ay_tree_selecttcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 /* char fname[] = "treeSelect";*/
 ay_object *o;
 int i;

  /* first, free old selection */
  ay_status = ay_sel_free();

  /* update current level */
  if (argc > 1)
    {
      ay_tree_getclevel(argv[1]);
    }
  else
    {
      return TCL_OK;
    }

  /* now, add selected objects */
  i = 1;
  while (i != argc) {
      o = ay_tree_getobject(argv[i]);
      if(o)
	ay_sel_add(o);
      i++;
  }

 return TCL_OK;
} /* ay_tree_selecttcmd */



#if 0

/* create a new object via drag and drop */
int ayext_createbox(ay_object *o, char *treesel) {
 ay_box_object *box = NULL;
 char fname[] = "crtbox";

  if(!(box = calloc(1,sizeof(ay_box_object)))) {
      return AY_ERROR;
  }

  box->width  = 1.0;
  box->length = 1.0;
  box->height = 1.0;
   
  o->type = AY_OTBOX;
  o->object = box;

  return AY_OK;
}

int ayext_createsphere(ay_object *o, char *treesel) {
 ay_sphere_object *sphere = NULL;
 char fname[] = "crtsphere";

  if(!(sphere = calloc(1,sizeof(ay_sphere_object)))) {
      return TCL_ERROR;
  }

  sphere->closed = AY_TRUE;
  sphere->radius = 1.0;
  sphere->zmin = -1.0;
  sphere->zmax = 1.0;
  sphere->thetamax = 360.0;
  o->type = AY_OTSPHERE;
  o->object = sphere;

  return AY_OK;
}

int aytree_CreateDndObject_tcmd(ClientData clientData, Tcl_Interp * interp,
				  int argc, char *argv[]) {
 int ay_status = AY_OK;
 char *parentnode, c, buf[256];
 ay_object *parent, *o, *n;
 char fname[] = "CreateDndObject";
 int i, p, cnt;
 int failure;
 Tcl_HashEntry *entry = NULL;
 ayext_create *create;

  if(argc != 6) {
      ay_error(interp, AY_ARGS, fname, "varname");
      return TCL_OK;
  }

  /* get the parent of the new object */
  parent = aytree_get_object(argv[2]);

  /* parent points the right parent now, so get the position in this level */
  p = strtol(argv[3], NULL, 10);


  /* allocate memory for the new object */
  if (!(n = calloc(1, sizeof(ay_object)))) {
      ay_error(interp, AY_OUTOFMEM, fname, NULL);
      return TCL_OK;
  }


  entry = Tcl_FindHashEntry(&ay_creatednd_ht, argv[1]);
  if (entry) {
      create = (ayext_create *)Tcl_GetHashValue(entry);
      if (create(n, argv[4]) != AY_OK) {
	  free(n);
	  return TCL_OK;
      }
  }
  else {
      ay_error(interp, AY_ARGS, fname, "varname");
      return TCL_OK;
  }


  i = 0;  /* position where the object was inserted */
  failure = AY_TRUE;

  if (parent) {
      if (parent->down) {
	  if (p == 0) {
	      /* insert object as the first child of parent */
	      n->next = parent->down;
	      parent->down = n;
	      failure = AY_FALSE;
	  }
	  else {
	      /* determine the position of the new object */
	      o = parent->down;
              if (p != -1) {
		  p--;
		  while ((p) && (o)) {
	              o = o->next;
		      p--;
		      i++;
		  }
		  i++;
	      }
	      else {
		  while (o->next) {
		      o = o->next;
		      i++;
		  }
		  i++;
	      }

              if (o) {
		  /* insert child after o */
		  n->next = o->next;
		  o->next = n;
		  failure = AY_FALSE;
	      }
	  }
      }
      else {
	  if (p == 0) {
              /* insert object as the first child of parent */
	      parent->down = n;
	      failure = AY_FALSE;
	  }
      }
  }
  else {
      if (p == 0) {
	  /* should never happen */
	  n->next = ay_root;
	  ay_root = n;
	  failure = AY_FALSE;
      }
      else {
	  /* determine the position of the new object */
	  o = ay_root;
          if (p != -1) {
	      p--;
	      while ((p) && (o)) {
	          o = o->next;
		  p--;
		  i++;
	      }
	      i++;
	  }
	  else {
	      while (o->next) {
		  o = o->next;
		  i++;
	      }
	      i++;
	  }

          if (o) {
	      /* insert child after o */
	      n->next = o->next;
	      o->next = n;
	      failure = AY_FALSE;
	  }
      }
  }

  if (failure) {
      free(n);
      ay_error(interp, AY_ARGS, fname, "varname");
      return TCL_OK;
  }

  strcpy(buf, argv[2]);
  sprintf(&buf[strlen(buf)], ":%d", i);
  Tcl_SetVar(interp, argv[5], buf, TCL_LEAVE_ERR_MSG);

 return TCL_OK;
}

#endif


/* ay_tree_dndtcmd:
 *  move object(s) via drag and drop within the tree
 *  o if object is dropped _onto_ another:
 *    parent: target object
 *    position: -1
 *    - if parent has a drop callback registered, we just call it
 *  o if object is dropped _between_ two other objects:
 *    parent: parent object of level where drop occured
 *    position: the position of an object (drop occured between this
 *              and the next object)
 *  o Warning: tree.tcl/tree_drop prevents calling of this
 *    function with parameters which would make this function
 *    _crash badly_.
 */
int
ay_tree_dndtcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "treeDnd";
 ay_object *p = NULL, *o = NULL, **t = NULL, *open = NULL, *target = NULL;
 ay_list_object *sel = ay_selection, *newlevel = NULL;
 int i = 0, j = 0, instanceerr = AY_FALSE, has_callback = AY_FALSE;
 Tcl_DString ds;
 void **arr = NULL;
 ay_treedropcb *cb = NULL;

  if(argc < 3)
    {
      ay_error(AY_EARGS, fname, "parent position newclevel");
      return TCL_OK;
    }

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  p = ay_tree_getobject(argv[1]);

  Tcl_GetInt(interp, argv[2], &i);

  /* find place to move selected objects to */
  if(i == -1)
    { /* drop to other object */
      /* first check, if object has a drop callback registered */
      arr = ay_treedropcbt.arr;
      cb = (ay_treedropcb *)(arr[p->type]);

      /* call callback */
      if(cb)
	{
	  has_callback = AY_TRUE;
	  ay_status = cb(p);
	}
      
      if((has_callback == AY_FALSE) && (!p->parent))
	{
	  ay_error(AY_ERROR, fname, "Can not place objects here!");
	}

      if((ay_status == AY_EDONOTLINK) || (!p->parent))
	{

	  Tcl_SetVar(interp, argv[3], "no-drop",
		     TCL_LEAVE_ERR_MSG);

	  return TCL_OK;
	}

      t = &(p->down);
      target = p;
      p = p->down;
      /* look for end of level */
      while(p->next)
	{
	  t = &(p->next);
	  p = p->next;
	}
      
    }
  else
    { /* move object between two others */
      /* look for position to insert */
      if(i == 0)
	{
	  t = &(p->down);
	  target = p;
	}
      else
	{
	  
	  if(p)
	    {
	      p = p->down;
	    }
	  else
	    {
	      p = ay_root;
	    }
	  target = p;
	  j = 1;
	  while(p->next)
	    {
	      if(i == j)
		{
		  t = &(p->next);
		}
	      j++;
	      p = p->next;
	    } /* while */
	} /* if */
    } /* if */

  /* instance check */
  while(sel)
    {
      o = sel->object;

      instanceerr = AY_FALSE;

      instanceerr = ay_instt_check(o, target);

      if(instanceerr)
	{
	  ay_error(AY_ERROR, fname, "Recursive instances would result!");
	  return TCL_OK;
	}

      sel = sel->next;
    } /* while instance check */

  /* check if views are to be moved */
  sel = ay_selection;
  /* move objects */
  while(sel)
    {
      o = sel->object;
      if(o->type == AY_IDVIEW)
	{
	  ay_error(AY_ERROR, fname, "View objects may not be moved!");
	  return TCL_OK;
	}

      sel = sel->next;
    }

  sel = ay_selection;
  /* move objects */
  while(sel)
    {
      o = sel->object;

      /* sanity check */
      if((*t != o) && (t != &(o->next)))
	{
	  ay_status = ay_object_unlink(o);
	  o->next = *t;
	  *t = o;
	  t = &(o->next);
	}
      open = o;
      sel = sel->next;
    }

  /* reconstruct current level */
  Tcl_DStringInit(&ds);
  if(!(newlevel = calloc(1, sizeof(ay_list_object))))
    return TCL_OK;
  newlevel->object = ay_root;

  ay_status = ay_tree_crtstringfromobj(ay_root, open, newlevel, &ds);

  free(newlevel);

  Tcl_SetVar(interp, argv[3], Tcl_DStringValue(&ds), TCL_LEAVE_ERR_MSG);

  Tcl_DStringFree(&ds);

 return TCL_OK;
} /* ay_tree_dndtcmd */


/* ay_tree_inittcmd:
 *  initialize this module
 */
int
ay_tree_inittcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
  /*
 char fname[] = "treeInit";
 int new_item;
 Tcl_HashEntry *entry;
  */

  /* create new Tcl commands */
  Tcl_CreateCommand (interp, "treeGetString", ay_tree_gettreetcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand (interp, "treeSelect", ay_tree_selecttcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /*
  Tcl_CreateCommand (interp, "CreateDndObject", aytree_CreateDndObject_tcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  */

  Tcl_CreateCommand (interp, "treeDnd", ay_tree_dndtcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);



  /* Init creation table for drag and drop */
  /*
  Tcl_InitHashTable(&ay_creatednd_ht, TCL_STRING_KEYS);
  entry = Tcl_CreateHashEntry(&ay_creatednd_ht, "boximg", &new_item);
  Tcl_SetHashValue(entry, ayext_createbox);
  entry = Tcl_CreateHashEntry(&ay_creatednd_ht, "sphereimg", &new_item);
  Tcl_SetHashValue(entry, ayext_createsphere);


  if ((Tcl_EvalFile(interp, "ayext.tcl")) != TCL_OK) {
      ay_error(interp, AY_ERROR, fname, "Error while sourcing \\\"ayext.tcl\\\"!");
       return AY_ERROR;
  }

  */

 return TCL_OK;
} /* ay_tree_inittcmd */
