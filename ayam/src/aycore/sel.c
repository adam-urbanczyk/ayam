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


/** ay_sel_free:
 * Frees the entire list of selected objects;
 * additionally the selected flag can be cleared from
 * all previously selected objects.
 *
 * \param[in] clear_selflag if AY_TRUE, clears the selected flag
 */
void
ay_sel_free(int clear_selflag)
{
 ay_list_object *sel = ay_selection, *seln = NULL;

  while(sel)
    {
      seln = sel->next;
      if(clear_selflag)
	{
	  sel->object->selected = AY_FALSE;
	}
      free(sel);
      sel = seln;
    }

  ay_selection = NULL;

 return;
} /* ay_sel_free */


/** ay_sel_add:
 * Add object \a o to the list of selected objects;
 * additionally the object can be marked as selected.
 * There is no check, whether \a o is already selected.
 *
 * \param[in,out] o the object to add to the selection
 * \param[in] set_selflag if AY_TRUE, the object will be marked as selected
 *
 * \returns AY_OK on success, error code otherwise
 */
int
ay_sel_add(ay_object *o, int set_selflag)
{
 ay_list_object *new_sel = NULL;
 static ay_list_object *last_sel = NULL;

  if(o == ay_endlevel)
    {
      return AY_ERROR;
    }

  if(!(new_sel = calloc(1, sizeof(ay_list_object))))
    {
      return AY_EOMEM;
    }

  new_sel->object = o;
  if(set_selflag)
    {
      o->selected = AY_TRUE;
    }

  if(ay_selection && last_sel)
    {
      last_sel->next = new_sel;
    }
  else
    {
      ay_selection = new_sel;
    }

  last_sel = new_sel;

 return AY_OK;
} /* ay_sel_add */


/** ay_sel_selobtcmd:
 * Set a new selection or clear the selection.
 * Implements the \a selOb scripting interface command.
 * See also the corresponding section in the \ayd{scselob}.
 *
 * \returns TCL_OK in any case.
 */
int
ay_sel_selobtcmd(ClientData clientData, Tcl_Interp *interp,
		 int argc, char *argv[])
{
 int ay_status = AY_OK;
 int lbmode = AY_FALSE;
 ay_list_object *oldsel, *newsel, *t;
 ay_object *o = ay_currentlevel->object, *e;
 int i = 0, j = 0, firstarg = 1, start, end, need_redraw = AY_TRUE;
 char vname[] = "ay(need_redraw)", yes[] = "1", no[] = "0", *endptr;
 long int argvi = 0;

  /* clear selected flags from currently selected objects */
  t = ay_selection;
  while(t)
    {
      t->object->selected = AY_FALSE;
      t = t->next;
    }

  if(argc > 1)
    {
      if((argv[1][0] == '-' && argv[1][1] == '1') ||
	 (argv[1][0] == '-' && argv[1][1] == 'e'))
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
      /* is -clear? */
      if(argv[1][0] == '-' && argv[1][1] == 'c')
	goto cleanup;

      /* work with listbox indices? */
      if(!strcmp(argv[1], "-lb"))
	{
	  if(argc < 3)
	    goto cleanup;
	  lbmode = AY_TRUE;
	  firstarg++;
	  j++;
	}

      if(lbmode && (atoi(argv[firstarg]) == 0))
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
	  firstarg++;
	}

      if(lbmode && (o == ay_root))
	{
	  if(o->next)
	    {
	      o = o->next;
	    }
	}

      /* iterate through arguments and select appropriate objects */
      for(i = firstarg; i < argc; i++)
	{
	  /*argvi = atoi(argv[i]);*/

	  if(argv[i][0] == 'e')
	    {
	      e = o;
	      start = j;
	      while(e && e->next)
		{
		  start++;
		  e = e->next;
		}
	      start--;
	      end = start;
	    }
	  else
	    {
	      argvi = strtol(argv[i], &endptr, 10);

	      if(argv[i] == endptr)
		{
		  ay_error(AY_ERROR, argv[0], "could not parse index");
		  goto error;
		}

	      start = (int)argvi;
	      end = start;

	      if(*endptr != '\0')
		{
		  endptr++;
		  if(*endptr == 'e')
		    end = -1;
		  else
		    {
		      end = atoi(endptr);
		      if(end < start)
			{
			  ay_error(AY_ERROR, argv[0], "could not parse range");
			  goto error;
			}
		    }
		} /* if */
	    } /* if index is e */

	  while(j != start)
	    {
	      j++;

	      if(o)
		{
		  o = o->next;
		}

	      if(!o)
		{
		  goto cleanup;
		}

	      /* no reset of o for next iteration, because we believe
		 that the arguments are sorted! */
	    } /* while */

	  /* found a selected object -> add to the list */
	  if(start == end)
	    {
	      /* select single object */
	      if(o && o->next)
		{
		  ay_status = ay_sel_add(o, AY_TRUE);
		  if(ay_status)
		    {
		      ay_error(ay_status, argv[0], NULL);
		      goto error;
		    }
		} /* if */
	    }
	  else
	    {
	      /* select range of objects */
	      while(o && o->next)
		{
		  ay_status = ay_sel_add(o, AY_TRUE);
		  if(ay_status)
		    {
		      ay_error(ay_status, argv[0], NULL);
		      goto error;
		    }

		  o = o->next;

		  if(end != -1)
		    {
		      end--;
		      if(end < start)
			break;
		    }
		} /* while */
	    } /* if */
	} /* for */
    } /* if have args */

cleanup:
  newsel = ay_selection;

  /* never redraw in script object scripts or in notify tag scripts */
  if(interp != ay_safeinterp)
    {
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
    }
  else
    {
      need_redraw = AY_FALSE;
    }

error:
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


/** ay_sel_getseltcmd:
 * Get the current selection as ordered list of zero based indices
 * of selected object in the current level.
 *
 * Implements the \a getSel scripting interface command.
 * See also the corresponding section in the \ayd{scgetsel}.
 *
 * \returns TCL_OK in any case.
 */
int
ay_sel_getseltcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int tcl_status;
 int i = 0;
 int return_result = AY_FALSE;
 ay_object *o = ay_currentlevel->object;
 Tcl_Obj *to = NULL, *toa = NULL, *tol = NULL;

  if(argc < 2)
    {
      return_result = AY_TRUE;
    }

  if(!return_result)
    {
      /* always clear result variable */
      Tcl_SetVar(interp, argv[1], "", TCL_LEAVE_ERR_MSG);
      toa = Tcl_NewStringObj(argv[1], -1);
    }

  while(o)
    {
      if(o->selected)
	{
	  to = Tcl_NewIntObj(i);
	  if(!return_result)
	    {
	      Tcl_ObjSetVar2(interp, toa, NULL, to, TCL_APPEND_VALUE |
			     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);
	    }
	  else
	    {
	      if(ay_selection->next)
		{
		  if(!tol)
		    {
		      tol = Tcl_NewListObj(0, NULL);
		    }
		  tcl_status = Tcl_ListObjAppendElement(interp, tol, to);
		  if(tcl_status == TCL_ERROR)
		    {
		      goto cleanup;
		    }
		}
	      else
		{
		  Tcl_SetObjResult(interp, to);
		}
	    } /* if */
	} /* if selected */
      i++;
      o = o->next;
    } /* while */

  if(return_result && tol)
    {
      Tcl_SetObjResult(interp, tol);
    }

cleanup:
  if(toa)
    {
      Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
    }

 return TCL_OK;
} /* ay_sel_getseltcmd */


/** ay_sel_hsltcmd:
 * Select the last object(s) in the current level, without any feedback
 * in the GUI and without redraw.
 *
 * Implements the \a hSL scripting interface command.
 * See also the corresponding section in the \ayd{schsl}.
 *
 * \returns TCL_OK in any case.
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
    {
      l = cl->object;
    }
  else
    {
      return TCL_OK;
    }

  if(num == 1)
    {
      while(l && l->next)
	{
	  o = l;
	  l = l->next;
	}

      if(o)
	{
	  ay_sel_free(/*clear_selflag=*/AY_TRUE);
	  ay_sel_add(o, /*set_selflag=*/AY_TRUE);
	}
    }
  else
    {
      tnum = 0;
      ay_sel_free(/*clear_selflag=*/AY_TRUE);

      l = ay_currentlevel->object;
      while(l && l->next)
	{
	  tnum++;
	  l = l->next;
	}
      tnum -= num;
      l = ay_currentlevel->object;
      while(l && l->next)
	{
	  tnum--;
	  if(tnum <= 0)
	    {
	      ay_sel_add(l, /*set_selflag=*/AY_TRUE);
	    }
	  l = l->next;
	} /* while */
    } /* if */

 return TCL_OK;
} /* ay_sel_hsltcmd */


/** ay_sel_clearselflag:
 * helper to _recursively_ clear the selected
 * flag from the object hierarchy pointed to by \a o
 *
 * \param[in,out] o object hierarchy to process
 */
void
ay_sel_clearselflag(ay_object *o)
{

  if(!o)
    {
      return;
    }

  while(o)
    {
      if(o->down)
	{
	  ay_sel_clearselflag(o->down);
	}
      o->selected = AY_FALSE;
      o = o->next;
    }

 return;
} /* ay_sel_clearselflag */


/** ay_sel_clean:
 *  remove all objects from the selection that can not be reached
 *  via \a ay_root (are removed or cut to the clipboard)
 */
void
ay_sel_clean()
{
 ay_list_object *sel = ay_selection, **selp = &ay_selection, *seln = NULL;

  while(sel)
    {
      seln = sel->next;
      if(!ay_object_find(sel->object, ay_root))
	{
	  if(selp == &ay_selection)
	    {
	      ay_selection = seln;
	    }
	  else
	    {
	      *selp = seln;
	    }
	  free(sel);
	}
      else
	{
	  selp = &(sel->next);
	}
      sel = seln;
    } /* while */

 return;
} /* ay_sel_clean */
