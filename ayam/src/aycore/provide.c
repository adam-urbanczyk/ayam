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

/* provide.c - functions for provide mechanism */

/* ay_provide_register:
 *  register the provide callback <provcb> for
 *  objects of type <type_id>
 */
int
ay_provide_register(ay_providecb *provcb, unsigned int type_id)
{
 int ay_status = AY_OK;

  /* register provide callback */
  ay_status = ay_table_additem(&ay_providecbt, (ay_voidfp)provcb, type_id);

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
 ay_voidfp *arr = NULL;
 ay_providecb *cb = NULL;

  if(!o)
    return AY_ENULL;

  /* call the provide callback */
  arr = ay_providecbt.arr;
  cb = (ay_providecb *)(arr[o->type]);
  if(cb)
    {
      ay_status = cb(o, type, result);

      if(!result)
	{
	  /* caller just wants to test ability to provide
	     => return ay_status without checking */
	  return ay_status;
	}

      if(ay_status)
	{
	  ay_error(AY_ERROR, fname, "provide callback failed");
	  return AY_ERROR;
	}
    }
  else
    {
      if(!result)
	{
	  /* caller just wants to test ability to provide
	     and this object has no provide callback at all... */
	  return AY_ERROR;
	}
        /*
	  else
	  {
	  ay_error(AY_ERROR, fname, "No provide callback registered!");
	  }
	*/
    } /* if */


 return AY_OK;
} /* ay_provide_object */


/* ay_provide_nptoolobj:
 *  helper for tool objects that provide NURBS surfaces with caps/bevels
 */
int
ay_provide_nptoolobj(ay_object *o, unsigned int type,
		     ay_object *npatch, ay_object *cb,
		     ay_object **result)
{
 int ay_status = AY_OK;
 ay_object *new = NULL, **t = NULL, *p = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDNPATCH)
	return AY_OK;
      else
	return AY_ERROR;
    }

  if(type != AY_IDNPATCH)
    return AY_OK;

  if(!npatch)
    return AY_ERROR;

  t = &(new);

  /* copy surface(s) */
  p = npatch;
  while(p)
    {
      ay_status = ay_object_copy(p, t);
      if(ay_status)
	{
	  ay_object_deletemulti(new, AY_FALSE);
	  return AY_ERROR;
	}

      ay_npt_applytrafo(*t);
      ay_trafo_copy(o, *t);

      t = &((*t)->next);
      p = p->next;
    } /* while */

  /* copy caps and bevels */
  p = cb;
  while(p)
    {
      ay_status = ay_object_copy(p, t);
      if(ay_status)
	{
	  ay_object_deletemulti(new, AY_FALSE);
	  return AY_ERROR;
	}

      ay_npt_applytrafo(*t);
      ay_trafo_copy(o, *t);

      t = &((*t)->next);
      p = p->next;
    } /* while */

  /* copy some tags */
  (void)ay_tag_copyselected(o, new, ay_prefs.converttags,
			    ay_prefs.converttagslen);

  *result = new;

 return ay_status;
} /* ay_provide_nptoolobj */
