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

/* clipb.c - functions for the object clipboard */


/* ay_clipb_copytcmd:
 *  copy currently selected objects to the clipboard
 */
int
ay_clipb_copytcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "copOb";
 ay_object *clip = NULL, *o = NULL, *t = NULL;
 ay_list_object *sel = ay_selection;
 
  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
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
      ay_status = ay_object_delete(o);
      if(ay_status)
	{
	  ay_clipboard = t;
	  ay_error(AY_ERROR, fname, "Could not clear clipboard!");
	  ay_error(AY_ERROR, fname,
		   "Maybe there are referenced objects in it?");
	  ay_error(AY_ERROR, fname,
		   "Use menu: \\\"Special/Clipboard/Paste (Move)\\\" first!");
	  return TCL_OK;
	}
      ay_status = ay_undo_clearobj(o);
    }
  ay_clipboard = NULL;

  /* copy object(s) */
  o = sel->object;
  ay_status = ay_object_copy(o, &ay_clipboard);

  if(ay_status)
    {
      ay_error(ay_status, fname, NULL);
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
	  ay_error(ay_status, fname, NULL);
	  return TCL_OK;
	}

      clip = clip->next;
      sel = sel->next;
    } /* while */

 return TCL_OK;
} /* ay_clipb_copytcmd */


/* ay_clipb_cuttcmd:
 *  move currently selected objects to the clipboard
 */
int
ay_clipb_cuttcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "cutOb";
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL, *clip = NULL, *t = NULL;

  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  /* do not cut root object or views! */
  while(sel)
    {
      if(sel->object == ay_root)
	{
	  ay_error(AY_ERROR, fname, "Can not cut root object!");
	  return TCL_OK;
	}
      if(sel->object->type == AY_IDVIEW)
	{
	  ay_error(AY_ERROR, fname, "Can not cut view object!");
	  return TCL_OK;
	}
      sel = sel->next;
    } /* while */

  sel = ay_selection;

  /* clear all cached pointers to scene hierarchy */
  ay_object_ccp(NULL);

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
      ay_status = ay_object_delete(o);	  
      if(ay_status)
	{
	  ay_clipboard = t;
	  ay_error(AY_ERROR, fname, "Could not clear clipboard!");
	  ay_error(AY_ERROR, fname,
		   "Maybe there are referenced objects in it?");
	  ay_error(AY_ERROR, fname,
		   "Use menu: \\\"Special/Paste (Move)\\\" first!");
	  return TCL_OK;
	} /* if */
      ay_status = ay_undo_clearobj(o);
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

  /* free point selection */
  if(ay_point_edit_coords)
    free(ay_point_edit_coords);

  ay_point_edit_coords = NULL;

  /* free selection */
  ay_status = ay_sel_free(AY_TRUE);

  ay_notify_parent();

 return TCL_OK;
} /* ay_clipb_cuttcmd */


/* ay_clipb_pastetcmd:
 *  paste objects from the clipboard to the current level
 */
int
ay_clipb_pastetcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "pasOb";
 ay_object *ins = NULL, *clip = ay_clipboard;
 int instanceerr = AY_FALSE;

  /* first, check whether we are moving instances into their masters */
  while(clip)
    {
      instanceerr = AY_FALSE;

      instanceerr = ay_instt_check(clip, ay_currentlevel->object);

      if(instanceerr)
	{
	  ay_error(AY_ERROR, fname, "Recursive instances would result!");
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
	  ay_error(ay_status, fname, NULL);
	  return TCL_ERROR;
	}

      ay_status = ay_object_link(ins);
      if(ay_status)
	{
	  /* oops, could not link object */
	  ay_error(ay_status, fname, NULL);
	  ay_status = ay_object_delete(ins);
	  return TCL_OK;
	} /* if */
	    
      clip = clip->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* ay_clipb_pastetcmd */


/* ay_clipb_movetcmd:
 *  move objects from the clipboard to the current level
 */
int
ay_clipb_movetcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "cmovOb";
 ay_object *next = NULL, *clip = ay_clipboard;
 int instanceerr = AY_FALSE;

  /* first, check whether we are moving instances into their masters */
  while(clip)
    {
      instanceerr = AY_FALSE;

      instanceerr = ay_instt_check(clip, ay_currentlevel->object);

      if(instanceerr)
	{
	  ay_error(AY_ERROR, fname, "Recursive instances would result!");
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
	  ay_error(ay_status, fname, NULL);
	  return TCL_OK;
	}
	    
      clip = next;
    } /* while */

  ay_clipboard = NULL;

  ay_notify_parent();

 return TCL_OK;
} /* ay_clipb_movetcmd */


/* ay_clipb_replacetcmd:
 *  replace objects from the current selection with the clipboard content
 */
int
ay_clipb_replacetcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "repOb";
 ay_object *clip = ay_clipboard, *clipend = NULL;
 ay_object **presel, *selend;
 int instanceerr = AY_FALSE;
 ay_list_object *sel = ay_selection;
 
  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  /* do not replace root object or views! */
  while(sel)
    {
      if(sel->object == ay_root)
	{
	  ay_error(AY_ERROR, fname, "Can not replace root object!");
	  return TCL_OK;
	}
      if(sel->object->type == AY_IDVIEW)
	{
	  ay_error(AY_ERROR, fname, "Can not replace view object!");
	  return TCL_OK;
	}
      sel = sel->next;
    } /* while */

  sel = ay_selection;

  if(!clip)
    {
      ay_error(AY_ERROR, fname, "Clipboard is empty!");
      return TCL_OK;
    }

  /* first, check whether we would be moving instances into their masters */
  while(clip)
    {
      instanceerr = AY_FALSE;

      instanceerr = ay_instt_check(clip, ay_currentlevel->object);

      if(instanceerr)
	{
	  ay_error(AY_ERROR, fname, "Recursive instances would result!");
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
      ay_error(AY_ERROR, fname, NULL);
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
  ay_status = ay_sel_free(AY_TRUE);

  ay_notify_parent();

 return TCL_OK;
} /* ay_clipb_replacetcmd */
