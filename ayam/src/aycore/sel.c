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
 *  previously selected objects
 */
int
ay_sel_free(void)
{
 ay_list_object *sel = ay_selection, *seln = NULL;

  while(sel)
    {
      seln = sel->next;
      sel->object->selected = AY_FALSE;
      free(sel);
      sel = seln;
    }

  ay_selection = NULL;

  /* XXXX does this belong here? */
  if(ay_point_edit_coords)
    free(ay_point_edit_coords);

  ay_point_edit_coords = NULL;

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
 int i = 0, j = 1, argvi = 0, start = 1;
 ay_object *o = ay_currentlevel->object;
 char fname[] = "sel_setfromlb";

  /* first, clear old selection */
  if(ay_selection)
    ay_sel_free();

  /* nothing selected? -> bail out */
  if(argc == 1)
    return TCL_OK; 

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

