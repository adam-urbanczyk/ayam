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

/* table.c - functions for function/callback tables */

int
ay_table_init(ay_table *table)
{

  table->size = 256;
  if(!(table->arr = calloc(256, sizeof(void *))))
    return AY_EOMEM;

 return AY_OK;
} /* ay_table_init */


int
ay_table_additem(ay_table *table, void *newitem,
		 unsigned int index, int policy)
{
 void **arr = NULL;

  arr = table->arr;
  if(!arr)
    return AY_ERROR;

  if(index > table->size)
    {
      if(!(arr = realloc(table, sizeof(arr)+(256*sizeof(void *)))))
	return AY_EOMEM;

      /* clear new mem */
      memset(&(arr[table->size+256]), 0, 256*sizeof(void *));

      /* update table */
      table->size += 256;
      table->arr = arr;
    }

  switch(policy)
    {
    case 0: /* do not overwrite */
      /* XXXX Bug: if old content is NULL, it may be overwritten,
         regardless of policy. Does not matter _that_ much,
         because ay_object_registertype() checks with typename
         hashtable first, whether an object is already registered. */
      if(arr[index])
	return AY_ERROR;
      else
	arr[index] = newitem;
      break;
    case 1: /* overwrite */
      arr[index] = newitem;
      break;
    default:
      break;
    }

 return AY_OK;
} /* ay_table_additem */
