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

/** ay_table_init:
 * Initialize a callback table by allocating memory and setting the
 * size attribute accordingly.
 * 
 * \param[in,out] table callback table to initialize
 * 
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_table_init(ay_ftable *table)
{
  if(!(table->arr = calloc(64, sizeof(ay_voidfp))))
    return AY_EOMEM;
  table->size = 64;

 return AY_OK;
} /* ay_table_init */


/** ay_table_additem:
 * Add an item (function pointer) to a callback table.
 * If the supplied index is larger than the current size of the
 * table, the table will grow automatically to accomodate for the
 * new index.
 *
 * \param table callback table to manipulate
 * \param item function pointer to set
 * \param index designates the slot in the table to manipulate
 * 
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_table_additem(ay_ftable *table, ay_voidfp item, unsigned int index)
{
 ay_voidfp *arr = NULL, *tmp = NULL;

  arr = table->arr;
  if(!arr)
    return AY_ERROR;

  while(index >= table->size)
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
