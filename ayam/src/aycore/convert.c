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

/* convert.c - functions for object conversion */

/* ay_convert_register:
 *  register the conversion callback notcb for
 *  objects of type type_id
 */
int
ay_convert_register(ay_convertcb *convcb, unsigned int type_id)
{
 int ay_status = AY_OK;
 int overload = ay_prefs.overload;

  /* register convert callback */
  ay_status = ay_table_additem(&ay_convertcbt, (void*)convcb, type_id,
			       overload);

 return ay_status;
} /* ay_convert_register */


/* ay_convert_force:
 *  call conversion callback of object o
 */
int
ay_convert_force(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 char fname[] = "convert";
 void **arr = NULL;
 ay_convertcb *cb = NULL;


  /* call the conversion callback */
  arr = ay_convertcbt.arr;
  cb = (ay_convertcb *)(arr[o->type]);
  if(cb)
    {
      ay_status = cb(o, in_place);

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "convert callback failed");
	  return AY_ERROR;
	}
    }
  else
    {
     ay_error(AY_ERROR, fname, "No converter available!"); 
    }

 return AY_OK;
} /* ay_convert_force */


/* ay_convert_forcetcmd:
 *  force conversion of selected object(s)
 */
int
ay_convert_forcetcmd(ClientData clientData, Tcl_Interp * interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "convOb";
 ay_list_object *sel = ay_selection;
 int in_place = AY_FALSE;

  if(argc > 1)
    {
      if(!strcmp(argv[1], "-inplace"))
	{
	  in_place = AY_TRUE;
	}
    }

  while(sel)
    {
      ay_status = ay_convert_force(sel->object, in_place);
      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, NULL);
	}
      sel = sel->next;
    }

  if(ay_selection)
    {
      ay_status = ay_notify_parent();
    }

  /* clear all cached pointers to scene hierarchy */
  ay_status = ay_object_ccp(NULL);

 return TCL_OK;
} /* ay_convert_forcetcmd */
