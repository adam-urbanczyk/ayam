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

/* sel.c - functions for the selection */


/* ay_sel_free:
 *  frees entire list of selected objects;
 *  additionally clears the selected flag from
 *  previously selected objects, if clear_selflag is AY_TRUE
 */
int
ay_sel_free(int clear_selflag)
{
 ay_list_object *sel = ay_selection, *seln = NULL;

  while(sel)
    {
      seln = sel->next;
      if(clear_selflag)
	sel->object->selected = AY_FALSE;
      free(sel);
      sel = seln;
    }

  ay_selection = NULL;

 return AY_OK;
} /* ay_sel_free */


/* ay_sel_add:
 *  add object o to the list of selected objects;
 *  additionally marks object o as selected
 */
int
ay_sel_add(ay_object *o)
{
 ay_list_object *new_sel = NULL;
 static ay_list_object *last_sel = NULL;

  if(!(new_sel = calloc(1, sizeof(ay_list_object))))
    return AY_EOMEM;

  new_sel->object = o;
  o->selected = AY_TRUE;

  if(ay_selection)
    last_sel->next = new_sel;
  else
    ay_selection = new_sel; 

  last_sel = new_sel;

 return AY_OK;
} /* ay_sel_add */


/* ay_sel_setfromlbtcmd:
 *  set the selection from listbox
 *  I: argv[] contains the sorted and split list of selected elements
 *     of the hierarchy listbox
 *
 */
int
ay_sel_setfromlbtcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "selOb";
 ay_list_object *oldsel, *newsel, *t;
 ay_object *o = ay_currentlevel->object;
 int i = 0, j = 1, argvi = 0, start = 1, need_redraw = AY_TRUE;
 char vname[] = "ay(need_redraw)", yes[] = "1", no[] = "0";

  /* clear selected flags from currently selected objects */
  t = ay_selection;
  while(t)
    {
      t->object->selected = AY_FALSE;
      t = t->next;
    }

  /* save old selection for later comparison with new */
  oldsel = ay_selection;
  ay_selection = NULL;

  /* nothing selected? -> bail out */
  if(argc > 1)
    {
      /* first item in clevel selected? (this is root or "..") */
      if(atoi(argv[1]) == 0)
	{
	  start++;

	  /* root selected? */
	  if(o == ay_root)
	    {
	      ay_status = ay_sel_add(o);
	      if(ay_status)
		{
		  ay_error(ay_status, fname, NULL);
		  return TCL_OK;
		}

	      if(o->next)
		o = o->next;
	    }
	}

      if(o == ay_root)
	o = o->next;

      /* iterate through arguments and select appropriate objects */
      for(i = start; i < argc; i++)
	{
	  argvi = atoi(argv[i]);

	  while(j != argvi)
	    {
	      j++;

	      if(o)
		o = o->next;	

	      if(!o)
		break;

	      /* no reset of o for next iteration, because we believe
		 that the arguments are sorted! */
	    } /* while */

	  /* found a selected object -> add to the list */
	  if(o)
	    {
	      ay_status = ay_sel_add(o);
	      if(ay_status)
		{
		  ay_error(ay_status, fname, NULL);
		  return TCL_OK;
		}
	    } /* if */

	} /* for */
    } /* if */

  newsel = ay_selection;
  /* do we need a complete redraw ? */
  ay_draw_needredraw(oldsel, newsel, &need_redraw);

  if(need_redraw)
    {
      Tcl_SetVar(interp, vname, yes, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY); 
    }
  else
    {
      Tcl_SetVar(interp, vname, no, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    }

  /* now, free old selection */
  ay_selection = oldsel;
  while(ay_selection)
    {
      t = ay_selection;
      ay_selection = t->next;
      free(t);
    }
  ay_selection = newsel;

  if(need_redraw)
    {
      ay_viewt_alignlocal();
    }

 return TCL_OK;
} /* ay_sel_setfromlbtcmd */


/* ay_sel_getseltcmd:
 *  get the current selection
 *  I: argv[] contains the sorted and split list of selected elements
 *     of the hierarchy listbox
 *
 */
int
ay_sel_getseltcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int i = 0;
 ay_object *o = ay_currentlevel->object;
 char fname[] = "getSel";
 Tcl_Obj *to = NULL, *toa = NULL;

  /*  */
  if(argc < 1)
    {
      ay_error(AY_EARGS, fname, "varname");
      return TCL_OK; 
    }

  toa = Tcl_NewStringObj(argv[1],-1);

  while(o)
    {
      if(o->selected)
	{
	  to = Tcl_NewIntObj(i);
	  Tcl_ObjSetVar2(interp,toa,NULL,to,TCL_APPEND_VALUE | 
			 TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG |
			 TCL_GLOBAL_ONLY);
	}
      i++;
      o = o->next;
    }


 return TCL_OK;
} /* ay_sel_getseltcmd */

