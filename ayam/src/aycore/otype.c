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

/* otype.c - object type management */


/* ay_otype_register:
 *  register a new object type, filling pointers
 *  to the callbacks into various tables
 *  In:
 *   overload: allow overloading?
 *   name: typename
 *   crtcb: create callback
 *  Out:
 *   type_index: type identificator
 */
int
ay_otype_register(char *name,
		  ay_createcb  *crtcb,
		  ay_deletecb  *delcb,
		  ay_copycb    *copycb,
		  ay_drawcb    *drawcb,
		  ay_drawcb    *drawhcb,
		  ay_drawcb    *shadecb,
		  ay_propcb    *setpropcb,
		  ay_propcb    *getpropcb,
		  ay_getpntcb  *getpntcb,
		  ay_readcb    *readcb,
		  ay_writecb   *writecb,
		  ay_wribcb    *wribcb,
		  ay_bbccb     *bbccb,
		  unsigned int *type_index)
{
 int ay_status = AY_OK;
 static unsigned int tc = AY_IDLAST; /* type counter and index into tables */
 int increase_tc = AY_TRUE;
 int overload = ay_prefs.overload;
 int i = 0, new_item = 0;
 Tcl_HashEntry *entry = NULL;

 /* check, whether type is already registered */
  if((entry = Tcl_FindHashEntry(&ay_otypesht, name)))
    { /* yes, we already registered this type */

      /* check, whether we may overload the callbacks */
      if(!overload)
	{
	  return AY_ETYPE;
	}
      increase_tc = AY_FALSE;
      i = (unsigned int) Tcl_GetHashValue(entry);
    }
  else
    { /* no, this is a new type */

      entry = Tcl_CreateHashEntry(&ay_otypesht, name, &new_item);
      Tcl_SetHashValue(entry, tc);
      i = tc;
    }

  /* register typename */
  if((ay_status = ay_table_additem(&ay_typenamest, (void*)name, i,
				   overload)))
    return ay_status;

  /* register create callback */
  if((ay_status = ay_table_additem(&ay_createcbt, (void*)crtcb, i,
				   overload)))
    return ay_status;

  /* register delete callback */
  if((ay_status = ay_table_additem(&ay_deletecbt, (void*)delcb, i,
				   overload)))
    return ay_status;

  /* register copy callback */
  if((ay_status = ay_table_additem(&ay_copycbt, (void*)copycb, i,
				   overload)))
    return ay_status;

  /* register draw callback */
  if((ay_status = ay_table_additem(&ay_drawcbt, (void*)drawcb, i,
				   overload)))
    return ay_status;

  /* register draw points callback */
  if((ay_status = ay_table_additem(&ay_drawhcbt, (void*)drawhcb,
				   i, overload)))
    return ay_status;

  /* register shade callback */
  if((ay_status = ay_table_additem(&ay_shadecbt, (void*)shadecb, i,
				   overload)))
    return ay_status;

  /* register setprop callback */
  if((ay_status = ay_table_additem(&ay_setpropcbt, (void*)setpropcb, i,
				   overload)))
    return ay_status;

  /* register getprop callback */
  if((ay_status = ay_table_additem(&ay_getpropcbt, (void*)getpropcb, i,
				   overload)))
    return ay_status;

  /* register pickpnt callback */
  if((ay_status = ay_table_additem(&ay_getpntcbt, (void*)getpntcb, i,
				   overload)))
    return ay_status;

  /* register read callback */
  if((ay_status = ay_table_additem(&ay_readcbt, (void*)readcb, i,
				   overload)))
    return ay_status;

  /* register write callback */
  if((ay_status = ay_table_additem(&ay_writecbt, (void*)writecb, i,
				   overload)))
    return ay_status;

  /* register wrib callback */
  if((ay_status = ay_table_additem(&ay_wribcbt, (void*)wribcb, i,
				   overload)))
    return ay_status;

  /* register bbc callback */
  if((ay_status = ay_table_additem(&ay_bbccbt, (void*)bbccb, i,
				   AY_TRUE)))
    return ay_status;

  *type_index = i;

  if(increase_tc)
    tc++;

 return ay_status;
} /* ay_otype_register */


/* ay_otype_registercore:
 *  register a new core object type, filling pointers
 *  to the callbacks into various tables
 *  In:
 *   name: typename
 *   crtcb: create callback
 *   type_index: type identificator
 */
int
ay_otype_registercore(char *name,
			   ay_createcb  *crtcb,
			   ay_deletecb  *delcb,
			   ay_copycb    *copycb,
			   ay_drawcb    *drawcb,
			   ay_drawcb    *drawhcb,
			   ay_drawcb    *shadecb,
			   ay_propcb    *setpropcb,
			   ay_propcb    *getpropcb,
			   ay_getpntcb  *getpntcb,
			   ay_readcb    *readcb,
			   ay_writecb   *writecb,
			   ay_wribcb    *wribcb,
			   ay_bbccb     *bbccb,
			   unsigned int type_index)
{
 int ay_status = AY_OK;
 /* char fname[] = "ay_otype_registercore";*/
 int new_item = 0;
 Tcl_HashEntry *entry = NULL;
 
  /* check, whether type is already registered */
  /* this should not happen, btw. ... */
  if((entry = Tcl_FindHashEntry(&ay_otypesht, name)))
    { /* yes, we already have registered this type */
      return AY_ETYPE;
    }

  /* no, this is a new type */
  entry = Tcl_CreateHashEntry(&ay_otypesht, name, &new_item);
  Tcl_SetHashValue(entry, type_index);

  /* register typename */
  if((ay_status = ay_table_additem(&ay_typenamest, (void*)name, type_index,
				   AY_TRUE)))
    return ay_status;

  /* register create callback */
  if((ay_status = ay_table_additem(&ay_createcbt, (void*)crtcb, type_index,
				   AY_TRUE)))
    return ay_status;

  /* register delete callback */
  if((ay_status = ay_table_additem(&ay_deletecbt, (void*)delcb, type_index,
				   AY_TRUE)))
    return ay_status;

  /* register copy callback */
  if((ay_status = ay_table_additem(&ay_copycbt, (void*)copycb, type_index,
				   AY_TRUE)))
    return ay_status;

  /* register draw callback */
  if((ay_status = ay_table_additem(&ay_drawcbt, (void*)drawcb, type_index,
				   AY_TRUE)))
    return ay_status;

  /* register draw points callback */
  if((ay_status = ay_table_additem(&ay_drawhcbt, (void*)drawhcb,
				   type_index, AY_TRUE)))
    return ay_status;

  /* register shade callback */
  if((ay_status = ay_table_additem(&ay_shadecbt, (void*)shadecb, type_index,
				   AY_TRUE)))
    return ay_status;

  /* register setprop callback */
  if((ay_status = ay_table_additem(&ay_setpropcbt, (void*)setpropcb,
				   type_index, AY_TRUE)))
    return ay_status;

  /* register getprop callback */
  if((ay_status = ay_table_additem(&ay_getpropcbt, (void*)getpropcb,
				   type_index, AY_TRUE)))
    return ay_status;

  /* register pickpnt callback */
  if((ay_status = ay_table_additem(&ay_getpntcbt, (void*)getpntcb,
				   type_index, AY_TRUE)))
    return ay_status;

  /* register read callback */
  if((ay_status = ay_table_additem(&ay_readcbt, (void*)readcb, type_index,
				   AY_TRUE)))
    return ay_status;

  /* register write callback */
  if((ay_status = ay_table_additem(&ay_writecbt, (void*)writecb, type_index,
				   AY_TRUE)))
    return ay_status;

  /* register wrib callback */
  if((ay_status = ay_table_additem(&ay_wribcbt, (void*)wribcb, type_index,
				   AY_TRUE)))
    return ay_status;

  /* register bbc callback */
  if((ay_status = ay_table_additem(&ay_bbccbt, (void*)bbccb, type_index,
				   AY_TRUE)))
    return ay_status;

 return ay_status;
} /* ay_otype_registercore */

