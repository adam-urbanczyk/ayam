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

/* clevel.c - functions for current level management */


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
	      ay_clevel_add(c);
	      ay_clevel_add(c->down);
	      ay_status = ay_clevel_find(c->down, o, found);
	      if(!(*found))
		{
		  ay_clevel_del();
		  ay_clevel_del();
		}
	    }

	}
      else
	{
	  *found = AY_TRUE;
	  return AY_OK;
	}
      c = c->next;
    }


 return AY_OK;
} /* ay_clevel_find */


int
ay_clevel_add(ay_object *o)
{
 ay_list_object *lev = NULL;

  if(!(lev = calloc(1, sizeof(ay_list_object))))
    return AY_EOMEM;

  lev->object = o;

  if(ay_currentlevel)
    lev->next = ay_currentlevel;

  ay_currentlevel = lev;  

 return AY_OK;
} /* ay_clevel_add */


int
ay_clevel_del(void)
{
 ay_list_object *lev = ay_currentlevel;

  if(lev->next)
  {
    ay_currentlevel = lev->next;
    free(lev);
  }

 return AY_OK;
} /* ay_clevel_del */


/* actually deletes all clevel objects but not the first
   that points to ay_root */
int
ay_clevel_delall(void)
{
 ay_list_object *lev = ay_currentlevel;

  if(lev)
    while(lev->next)
      {
	ay_currentlevel = lev->next;
	free(lev);
	lev = ay_currentlevel;
      }

  ay_clevel_add(ay_root);

 return AY_OK;
} /* ay_clevel_delall */


int
ay_clevel_gotoptcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_object *o = ay_root;
 char varName[20], newValue[10];

  ay_status = ay_clevel_delall();

  if(o)
    {
      while(o->next)
	{
	  ay_next = &(o->next);
	  o = o->next;
	}
    }

  /* Synchronize the value of ay(CurrentLevel) */
  sprintf(varName, "ay(CurrentLevel)");
  sprintf(newValue, "root");
  Tcl_SetVar(interp, varName, newValue,
		TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
    
  return TCL_OK;
} /* ay_clevel_gotoptcmd */


int
ay_clevel_gouptcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_object *o = NULL;
 char fname[] = "clevel_goup";
 char *part1 = "ay", *part2 = "CurrentLevel";
 char *tmp = NULL, *string = NULL;
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 int length = 0;

 if(ay_currentlevel->object != ay_root)
   {
     ay_status = ay_clevel_del();
     ay_status = ay_clevel_del();

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
  if (!tmp)
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
  Tcl_SetStringObj(to, tmp, -1);
  
  free(tmp);
  Tcl_IncrRefCount(toa); Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton); Tcl_DecrRefCount(ton);

 return TCL_OK;
} /* ay_clevel_gouptcmd */


int
ay_clevel_godowntcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 int j = 0, argvi = 0;
 ay_object *o = ay_currentlevel->object;
 char fname[] = "clevel_godown";
 char *part1 = "ay", *part2 = "CurrentLevel";
 char tmp[256];
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;

  /* check args */
  if(argc != 2)
    {
      /* ay_error(AY_EARGS, fname, "index"); */
      return TCL_OK;
    }

  argvi = atoi(argv[1]);

  /* sublevel? */
  if(ay_root != o)
    {
      /* ".." selected? -> go up instead! */
      if(argvi == 0)
	{
	  return(ay_clevel_gouptcmd(clientData, interp, argc, argv));
	}
      j = 1;
    }


  while(o)
    {
      if(argvi == j)
	{ /* found the selected object */
	  if(o->parent)
	    {
	      ay_status = ay_clevel_add(o);
	      if(ay_status)
		{
		  ay_error(ay_status, fname, NULL);
		  return TCL_OK;
		}

	      ay_status = ay_clevel_add(o->down);
	      if(ay_status)
		{
		  ay_error(ay_status, fname, NULL);
		  return TCL_OK;
		}

	      if(!o->down)
		{
		  ay_error(AY_ENULL, fname, NULL);
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
		    }
		}
	    }

	  /* Synchronize the value of ay(CurrentLevel) */
	  toa = Tcl_NewStringObj(part1, -1);
	  ton = Tcl_NewStringObj(part2, -1);

	  /* Get ay(CurrentLevel) value */
	  to = Tcl_ObjGetVar2(interp, toa, ton,
			TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	  
	  /* Append the new component of the level */
	  if (strcmp(Tcl_GetStringFromObj(to, NULL), "root"))
	    {
	      sprintf(tmp, ":%d", argvi-1);
	    } else {
	      sprintf(tmp, ":%d", argvi);
	    }
	  Tcl_AppendStringsToObj(to, tmp, NULL);

	  Tcl_IncrRefCount(toa); Tcl_DecrRefCount(toa);
	  Tcl_IncrRefCount(ton); Tcl_DecrRefCount(ton);

	  return TCL_OK;
	}

      j++;
      o = o->next;
    }
    
 return TCL_OK;
} /* ay_clevel_godowntcmd */


int
ay_clevel_gettcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 ay_object *o = ay_currentlevel->object;
 char *name = NULL;
 char *typename = NULL;
 char fname[] = "clevel_get";
 Tcl_DString ds;

  /* check args */
  if(argc != 3)
    {
      ay_error(AY_EARGS, fname, "varname varname");
      return TCL_OK;
    }

  Tcl_SetVar(interp, argv[1], "", TCL_LEAVE_ERR_MSG);
  Tcl_SetVar(interp, argv[2], "", TCL_LEAVE_ERR_MSG);

  if(ay_currentlevel->object != ay_root)
    {
      Tcl_SetVar(interp, argv[1], "..",
		 TCL_APPEND_VALUE | TCL_LEAVE_ERR_MSG);
      Tcl_SetVar(interp, argv[2], "..",
		 TCL_APPEND_VALUE | TCL_LEAVE_ERR_MSG);
    }

  /* o->next because we silently discard the ever present EndLevel object */
  while(o->next)
    {
      name = NULL;
      name = ay_object_getname(o);

      if(name)
	{
	  if((o->name) || (o->type == AY_IDINSTANCE))
	    {
	      if(ay_prefs.list_types)
		{
		  typename = ay_object_gettypename(o->type);
		  Tcl_DStringInit(&ds);
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
		  Tcl_SetVar(interp, argv[1], name, TCL_APPEND_VALUE |
			     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);
		}
	    }
	  else
	    {
	      Tcl_SetVar(interp, argv[1], name, TCL_APPEND_VALUE |
			 TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);
	    }
	}
      else
	{
	  ay_error(AY_ENULL, fname, NULL);
	  return TCL_OK;
	}

      typename = NULL;
      typename = ay_object_gettypename(o->type);
      if(typename)
	{
	  Tcl_SetVar(interp, argv[2], typename, TCL_APPEND_VALUE |
		     TCL_LIST_ELEMENT | TCL_LEAVE_ERR_MSG);
	}
      else
	{
	  ay_error(AY_ENULL, fname, NULL);
	  return TCL_OK;
	}

      o = o->next;  
    }


 return TCL_OK;
} /* ay_clevel_gettcmd */
