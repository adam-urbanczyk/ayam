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

/* table.c - functions for function/callback tables */

int
ay_table_init(ay_ftable *table)
{
  if(!(table->arr = calloc(64, sizeof(ay_voidfp))))
    return AY_EOMEM;
  table->size = 64;

 return AY_OK;
} /* ay_table_init */

int
ay_table_additem(ay_ftable *table, ay_voidfp item, unsigned int index)
{
 ay_voidfp *arr = NULL, *tmp = NULL;

  arr = table->arr;
  if(!arr)
    return AY_ERROR;

  if(index > table->size)
    {
      if(!(tmp = realloc(arr, table->size*sizeof(ay_voidfp) +
			 (64*sizeof(ay_voidfp)))))
	return AY_EOMEM;
      arr = tmp;

      /* clear new mem */
      memset(&(arr[table->size]), 0, 64*sizeof(ay_voidfp));

      /* update table */
      table->size += 64;
      table->arr = arr;
    }

  arr[index] = item;

 return AY_OK;
} /* ay_table_additem */
