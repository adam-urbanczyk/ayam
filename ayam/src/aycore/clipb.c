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

/* clipb.c - functions for the object clipboard */


/* ay_clipb_copytcmd:
 *  Implements the \a copOb scripting interface command.
 *  See also the corresponding section in the \ayd{sccopob}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_clipb_copytcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_object *clip = NULL, *o = NULL, *t = NULL;
 ay_list_object *sel = ay_selection;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  /* clear old clipboard */
  /* first, delete all instance objects */
  ay_object_deleteinstances(&ay_clipboard);
  clip = ay_clipboard;
  /* now delete all normal objects */
  while(clip)
    {
      t = ay_clipboard;
      o = clip;
      clip = clip->next;
      ay_clipboard = clip;
      ay_undo_clearobj(o);
      ay_status = ay_object_delete(o);
      if(ay_status)
	{
	  ay_clipboard = t;
	  ay_error(AY_ERROR, argv[0], "Could not clear clipboard!");
	  ay_error(AY_ERROR, argv[0],
		   "Maybe there are referenced objects in it?");
	  ay_error(AY_ERROR, argv[0],
		   "Use menu: \"Special/Clipboard/Paste (Move)\" first!");
	  return TCL_OK;
	}
    }
  ay_clipboard = NULL;

  /* copy object(s) */
  o = sel->object;
  ay_status = ay_object_copy(o, &ay_clipboard);

  if(ay_status)
    {
      ay_error(ay_status, argv[0], NULL);
      ay_clipboard = NULL;
      return TCL_OK;
    }

  clip = ay_clipboard;

  sel = sel->next;

  while(sel)
    {
      o = sel->object;

      ay_status = ay_object_copy(o, &(clip->next));

      if(ay_status)
	{
	  ay_error(ay_status, argv[0], NULL);
	  return TCL_OK;
	}

      clip = clip->next;
      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_clipb_copytcmd */


/* ay_clipb_cuttcmd:
 *  move currently selected objects to the clipboard
 *  Implements the \a movOb scripting interface command.
 *  See also the corresponding section in the \ayd{scmovob}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_clipb_cuttcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL, *clip = NULL, *t = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  /* do not cut root object or views! */
  while(sel)
    {
      if(sel->object == ay_root)
	{
	  ay_error(AY_ERROR, argv[0], "Can not cut root object!");
	  return TCL_OK;
	}
      if(sel->object->type == AY_IDVIEW)
	{
	  ay_error(AY_ERROR, argv[0], "Can not cut view object!");
	  return TCL_OK;
	}
      sel = sel->next;
    } /* while */

  sel = ay_selection;

  /* clear old clipboard */

  /* first, delete all instance objects */
  ay_object_deleteinstances(&ay_clipboard);
  clip = ay_clipboard;

  /* now delete all normal objects */
  while(clip)
    {
      t = ay_clipboard;
      o = clip;
      clip = clip->next;
      ay_clipboard = clip;
      ay_undo_clearobj(o);
      ay_status = ay_object_delete(o);
      if(ay_status)
	{
	  ay_clipboard = t;
	  ay_error(AY_ERROR, argv[0], "Could not clear clipboard!");
	  ay_error(AY_ERROR, argv[0],
		   "Maybe there are referenced objects in it?");
	  ay_error(AY_ERROR, argv[0],
		   "Use menu: \"Special/Clipboard/Paste (Move)\" first!");
	  return TCL_OK;
	} /* if */
    } /* while */
  ay_clipboard = NULL;

  /* cut objects to clipboard */
  if(sel)
    ay_clipboard = sel->object;

  o = NULL;
  while(sel)
    {
      ay_status = ay_object_unlink(sel->object);
      if(o)
	o->next = sel->object;
      o = sel->object;

      sel = sel->next;
    } /* while */

  /* terminate clipboard hierarchy properly */
  if(o)
    o->next = NULL;

  /* free selection */
  ay_sel_free(AY_TRUE);

  /* notify parent object about changes */
  if(ay_currentlevel->next && ay_currentlevel->next->object)
    {
      ay_currentlevel->next->object->modified = AY_TRUE;
    }
  ay_notify_parent();

 return TCL_OK;
} /* ay_clipb_cuttcmd */


/* ay_clipb_pastetcmd:
 *  paste objects from the clipboard to the current level
 *  Implements the \a pasOb scripting interface command.
 *  See also the corresponding section in the \ayd{scpasob}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_clipb_pastetcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_object *ins = NULL, *clip = ay_clipboard;
 int instanceerr = AY_FALSE;

  /* first, check whether we are moving instances into their masters */
  while(clip)
    {
      instanceerr = ay_instt_check(clip, ay_currentlevel->object);

      if(instanceerr)
	{
	  ay_error(AY_ERROR, argv[0], "Recursive instances would result!");
	  return TCL_OK;
	}

      clip = clip->next;
    } /* while */

  clip = ay_clipboard;
  while(clip)
    {
      ay_status = ay_object_copy(clip, &ins);
      if(ay_status)
	{
	  ay_error(ay_status, argv[0], NULL);
	  return TCL_OK;
	}

      ay_status = ay_object_link(ins);
      if(ay_status)
	{
	  /* oops, could not link object */
	  ay_error(ay_status, argv[0], NULL);
	  ay_status = ay_object_delete(ins);
	  return TCL_OK;
	} /* if */

      clip = clip->next;
    } /* while */

  /* notify parent object about changes */
  if(ay_currentlevel->next && ay_currentlevel->next->object)
    {
      ay_currentlevel->next->object->modified = AY_TRUE;
    }
  ay_notify_parent();

 return TCL_OK;
} /* ay_clipb_pastetcmd */


/* ay_clipb_movetcmd:
 *  move objects from the clipboard to the current level
 *  Implements the \a pasmovOb scripting interface command.
 *  See also the corresponding section in the \ayd{scpasmovob}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_clipb_movetcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_object *next = NULL, *clip = ay_clipboard;
 int instanceerr = AY_FALSE;

  /* first, check whether we are moving instances into their masters */
  while(clip)
    {
      instanceerr = ay_instt_check(clip, ay_currentlevel->object);

      if(instanceerr)
	{
	  ay_error(AY_ERROR, argv[0], "Recursive instances would result!");
	  return TCL_OK;
	}
      clip = clip->next;
    } /* while */

  clip = ay_clipboard;
  while(clip)
    {
      next = clip->next;
      ay_status = ay_object_link(clip);
      if(ay_status)
	{
	  /* oops, could not link object */
	  ay_error(ay_status, argv[0], NULL);
	  return TCL_OK;
	}
      clip = next;
    } /* while */

  ay_clipboard = NULL;

  /* notify parent object about changes */
  if(ay_currentlevel->next && ay_currentlevel->next->object)
    {
      ay_currentlevel->next->object->modified = AY_TRUE;
    }
  ay_notify_parent();

 return TCL_OK;
} /* ay_clipb_movetcmd */


/* ay_clipb_replacetcmd:
 *  replace objects from the current selection with the clipboard content
 *  Implements the \a repOb scripting interface command.
 *  See also the corresponding section in the \ayd{screpob}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_clipb_replacetcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 ay_object *clip = ay_clipboard, *clipend = NULL;
 ay_object **presel, *selend;
 int instanceerr = AY_FALSE;
 ay_list_object *sel = ay_selection;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  /* do not replace root object or views! */
  while(sel)
    {
      if(sel->object == ay_root)
	{
	  ay_error(AY_ERROR, argv[0], "Can not replace root object!");
	  return TCL_OK;
	}
      if(sel->object->type == AY_IDVIEW)
	{
	  ay_error(AY_ERROR, argv[0], "Can not replace view object!");
	  return TCL_OK;
	}
      sel = sel->next;
    } /* while */

  sel = ay_selection;

  if(!clip)
    {
      ay_error(AY_ERROR, argv[0], "Clipboard is empty!");
      return TCL_OK;
    }

  /* first, check whether we would be moving instances into their masters */
  while(clip)
    {
      instanceerr = ay_instt_check(clip, ay_currentlevel->object);

      if(instanceerr)
	{
	  ay_error(AY_ERROR, argv[0], "Recursive instances would result!");
	  return TCL_OK;
	}

      clipend = clip;
      clip = clip->next;
    } /* while */

  /* find pointer to first selected object */
  if(ay_currentlevel->next->object)
    {
      presel = &(ay_currentlevel->next->object->down);
    }
  else
    {
      presel = &(ay_root->next);
    }

  while((*presel) && ((*presel) != sel->object))
    {
      presel = &((*presel)->next);
    }

  if(!(*presel))
    {
      /* internal error, scene structure broken? */
      ay_error(AY_ERROR, argv[0], NULL);
      return TCL_OK;
    }

  /* find last object in first (contiguous) selected region of objects */
  selend = sel->object;
  while(selend->next && selend->next->selected)
    {
      selend = selend->next;
    }

  /* replace first (contiguous) region of selection with clipboard content */

  /* head */
  clip = *presel;
  *presel = ay_clipboard;
  ay_clipboard = clip;

  /* tail */
  clipend->next = selend->next;
  selend->next = NULL;

  /* clear selected flags of replaced objects */
  clip = ay_clipboard;
  while(clip)
    {
      clip->selected = AY_FALSE;
      clip = clip->next;
    } /* while */

  /* free selection */
  ay_sel_free(AY_TRUE);

  /* notify parent object about changes */
  if(ay_currentlevel->next && ay_currentlevel->next->object)
    {
      ay_currentlevel->next->object->modified = AY_TRUE;
    }
  ay_notify_parent();

 return TCL_OK;
} /* ay_clipb_replacetcmd */


/* ay_clipb_hmovtcmd:
 *  Implements the \a upOb scripting interface command.
 *  Also implements the \a downOb scripting interface command.
 *  See also the corresponding section in the \ayd{scupob}.
 *  See also the corresponding section in the \ayd{scdownob}.
 *
 *  \returns TCL_OK in any case.
 */
int
ay_clipb_hmovtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 ay_list_object *sel = ay_selection;
 ay_object *s1, *s2, *l, *t, **before;

  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  if(argv[0][0] == 'u')
    {
      /* move object upwards */
      while(sel)
	{
	  s1 = sel->object;

	  if(s1 == ay_root)
	    break;

	  if(ay_currentlevel->object == ay_root)
	    before = &(ay_root->next);
	  else
	    before = &(ay_currentlevel->next->object->down);

	  if(*before == s1)
	    break;

	  l = ay_currentlevel->object;
	  while(l)
	    {
	      if(l->next == s1)
		{
		  s2 = s1;
		  while(l->next && sel->next && sel->next->object == l->next)
		    {
		      s2 = s2->next;
		      l = l->next;
		      sel = sel->next;
		    }

		  /* swap l->next and l (move up s1-s2 one object)*/
		  *before = s1;
		  t = s2->next;
		  s2->next = l;
		  l->next = t;
		  if(l == ay_currentlevel->object)
		    ay_currentlevel->object = s1;
		  break;
		}
	      before = &(l->next);
	      l = l->next;
	    } /* while */
	  sel = sel->next;
	} /* while */
    }
  else
    {
      /* move object downwards */
      while(sel->next)
	{
	  sel = sel->next;
	}
      l = ay_currentlevel->object;
      while(l->next != ay_endlevel)
	{
	  l = l->next;
	}

      if(sel->object == l)
	return TCL_OK;

      sel = ay_selection;
      while(sel)
	{
	  s1 = sel->object;

	  if(s1 == ay_root)
	    break;

	  if(ay_currentlevel->object == ay_root)
	    before = &(ay_root->next);
	  else
	    before = &(ay_currentlevel->next->object->down);

	  l = ay_currentlevel->object;
	  while(l && (l->next != ay_endlevel))
	    {
	      if(l == s1 && l->next)
		{
		  s2 = s1;
		  while(l->next && sel->next && sel->next->object == l->next)
		    {
		      s2 = s2->next;
		      l = l->next;
		      sel = sel->next;
		    }
		  if(l->next == ay_endlevel)
		    break;
		  /* swap l->next and l (move down s1-s2 one object)*/
		  if(s1 == ay_currentlevel->object)
		    {ay_currentlevel->object = s2->next;}
		  *before = l->next;
		  t = l->next->next;
		  l->next->next = s1;
		  s2->next = t;
		  break;
		}
	      before = &(l->next);
	      l = l->next;
	    } /* while */
	  sel = sel->next;
	} /* while */
    } /* if */

  l = ay_currentlevel->object;
  while(l && l->next)
    {
      ay_next = &(l->next);
      l = l->next;
    }

 return TCL_OK;
} /* ay_clipb_hmovtcmd */
