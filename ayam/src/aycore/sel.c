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

/* sel.c - functions for the selection */


/* ay_sel_free:
 *  frees the entire list of selected objects;
 *  additionally clears the selected flag from
 *  previously selected objects, if clear_selflag is AY_TRUE
 */
void
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

 return;
} /* ay_sel_free */


/* ay_sel_add:
 *  add object o to the list of selected objects;
 *  additionally marks object o as selected
 */
int
ay_sel_add(ay_object *o, int set_selflag)
{
 ay_list_object *new_sel = NULL;
 static ay_list_object *last_sel = NULL;

  if(!(new_sel = calloc(1, sizeof(ay_list_object))))
    return AY_EOMEM;

  new_sel->object = o;
  if(set_selflag)
    o->selected = AY_TRUE;

  if(ay_selection && last_sel)
    last_sel->next = new_sel;
  else
    ay_selection = new_sel;

  last_sel = new_sel;

 return AY_OK;
} /* ay_sel_add */


/* ay_sel_selobtcmd:
 *  set the selection (selOb command)
 *  I: argv[] contains a sorted and split list of object indices
 *  in the current level
 */
int
ay_sel_selobtcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
 int ay_status = AY_OK;
 int lbmode = AY_FALSE;
 ay_list_object *oldsel, *newsel, *t;
 ay_object *o = ay_currentlevel->object;
 int i = 0, j = 0, argvi = 0, start = 1, need_redraw = AY_TRUE;
 char vname[] = "ay(need_redraw)", yes[] = "1", no[] = "0";

  /* clear selected flags from currently selected objects */
  t = ay_selection;
  while(t)
    {
      t->object->selected = AY_FALSE;
      t = t->next;
    }

  if(argc > 1)
    {
      if(argv[1][0] == '-' && argv[1][1] == '1')
	{
	  return ay_sel_hsltcmd(clientData, interp, 1, argv);
	}
    }

  /* save old selection for later comparison with new */
  oldsel = ay_selection;
  ay_selection = NULL;

  /* establish new selection from arguments */
  if(argc > 1)
    {
      /* work with listbox indices? */
      if(!strcmp(argv[1], "-lb"))
	{
	  if(argc < 3)
	    goto cleanup;
	  lbmode = AY_TRUE;
	  start++;
	  j++;
	}

      if(lbmode && (atoi(argv[start]) == 0))
	{
	  if(o == ay_root)
	    {
	      ay_status = ay_sel_add(o, AY_TRUE);
	      if(ay_status)
		{
		  ay_error(ay_status, argv[0], NULL);
		  return TCL_OK;
		}
	    }
	  start++;
	}

      if(lbmode && (o == ay_root))
	if(o->next)
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
		goto cleanup;

	      /* no reset of o for next iteration, because we believe
		 that the arguments are sorted! */
	    } /* while */

	  /* found a selected object -> add to the list */
	  if(o && o->next)
	    {
	      ay_status = ay_sel_add(o, AY_TRUE);
	      if(ay_status)
		{
		  ay_error(ay_status, argv[0], NULL);
		  return TCL_OK;
		}
	    } /* if */
	} /* for */
    } /* if */

cleanup:
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

  /* if we are going to redraw, align the local views first */
  if(need_redraw)
    {
      ay_viewt_alignlocal();
    }

 return TCL_OK;
} /* ay_sel_selobtcmd */


/* ay_sel_getseltcmd:
 *  get the current selection
 */
int
ay_sel_getseltcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int i = 0;
 ay_object *o = ay_currentlevel->object;
 Tcl_Obj *to = NULL, *toa = NULL;

  /* check args */
  if(argc < 1)
    {
      ay_error(AY_EARGS, argv[0], "varname");
      return TCL_OK;
    }

  toa = Tcl_NewStringObj(argv[1], -1);

  while(o)
    {
      if(o->selected)
	{
	  to = Tcl_NewIntObj(i);
	  Tcl_ObjSetVar2(interp,toa,NULL,to,TCL_APPEND_VALUE |
			 TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);
	}
      i++;
      o = o->next;
    } /* while */

 return TCL_OK;
} /* ay_sel_getseltcmd */


/* ay_sel_hsltcmd:
 *  hidden select last; allow selection of the last object(s) in the current
 *  level without any feedback in the GUI and without redraw
 *  I: argv[1] may contain the number of objects to select, defaults to 1
 */
int
ay_sel_hsltcmd(ClientData clientData, Tcl_Interp *interp,
	       int argc, char *argv[])
{
 int tcl_status = TCL_OK;
 ay_list_object *cl = ay_currentlevel;
 ay_object *l, *o = NULL;
 int num = 1, tnum;

  if(argc > 1)
    {
      if(argv[1])
	{
	  tcl_status = Tcl_GetInt(interp, argv[1], &num);
	  AY_CHTCLERRRET(tcl_status, argv[0], interp);
	}
      if(num < 1)
	{
	  ay_error(AY_ERROR, argv[0], "argument should be >= 1");
	  num = 1;
	} /* if */
    } /* if */

  if(cl)
    l = cl->object;
  else
    return TCL_OK;

  if(num == 1)
    {
      while(l && l->next)
	{
	  o = l;
	  l = l->next;
	} /* while */

      if(o)
	{
	  ay_sel_free(AY_FALSE);
	  ay_sel_add(o, AY_TRUE);
	} /* while */
    }
  else
    {
      tnum = 0;
      ay_sel_free(AY_FALSE);

      l = ay_currentlevel->object;
      while(l && l->next)
	{
	  tnum++;
	  l = l->next;
	} /* while */
      tnum -= num;
      l = ay_currentlevel->object;
      while(l && l->next)
	{
	  tnum--;
	  if(tnum <= 0)
	    {
	      ay_sel_add(l, AY_TRUE);
	    }
	  l = l->next;
	} /* while */
    } /* if */

 return TCL_OK;
} /* ay_sel_hsltcmd */


/* ay_sel_clearselflag:
 *  helper to _recursively_ clear the selected
 *  flag from object hierarchy pointed to by <o>
 */
void
ay_sel_clearselflag(ay_object *o)
{

  if(!o)
    return;

  while(o)
    {
      if(o->down)
	ay_sel_clearselflag(o->down);
      o->selected = AY_FALSE;
      o = o->next;
    }

 return;
} /* ay_sel_clearselflag */

