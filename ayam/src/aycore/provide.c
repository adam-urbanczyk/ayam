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

/* provide.c - functions for object notification */

/* ay_provide_register:
 *  register the conversion callback notcb for
 *  objects of type type_id
 */
int
ay_provide_register(ay_providecb *provcb, unsigned int type_id)
{
 int ay_status = AY_OK;
 int overload = ay_prefs.overload;

  /* register provide callback */
  ay_status = ay_table_additem(&ay_providecbt, (void*)provcb, type_id,
			       overload);

 return ay_status;
} /* ay_provide_register */


/* ay_provide_object:
 *  call conversion callback of object o
 */
int
ay_provide_object(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 char fname[] = "provide";
 void **arr = NULL;
 ay_providecb *cb = NULL;


  /* call the provide callback */
  arr = ay_providecbt.arr;
  cb = (ay_providecb *)(arr[o->type]);
  if(cb)
    {
      ay_status = cb(o, type, result);

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "provide callback failed");
	  return AY_ERROR;
	}
    }
  /*
  else
    {
     ay_error(AY_ERROR, fname, "No provide callback registered!"); 
    }
  */

 return AY_OK;
} /* ay_provide_object */
