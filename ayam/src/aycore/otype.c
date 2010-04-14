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

/* otype.c - object type management */


/* ay_otype_register:
 *  register a new object type, filling pointers
 *  to the callbacks into various tables
 *  In:
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
 int i = 0, new_item = 0;
 Tcl_HashEntry *entry = NULL;

 /* check, whether type is already registered */
  if((entry = Tcl_FindHashEntry(&ay_otypesht, name)))
    {
      /* yes, we already registered this type */
      increase_tc = AY_FALSE;
      i = (unsigned int) Tcl_GetHashValue(entry);
    }
  else
    {
      /* no, this is a new type */
      entry = Tcl_CreateHashEntry(&ay_otypesht, name, &new_item);
      Tcl_SetHashValue(entry, tc);
      i = tc;
    }

  /* register typename */
  if(!(entry = Tcl_FindHashEntry(&ay_typenamesht, (char*)i)))
    {
      entry = Tcl_CreateHashEntry(&ay_typenamesht, (char*)i, &new_item);
    }
  Tcl_SetHashValue(entry, (void*)name);

  /* register typename */
  /*
  if((ay_status = ay_table_additemo(&ay_typenamest, (void*)name, i)))
    return ay_status;
  */
  /* register create callback */
  if((ay_status = ay_table_additem(&ay_createcbt, (ay_voidfp)crtcb, i)))
    return ay_status;

  /* register delete callback */
  if((ay_status = ay_table_additem(&ay_deletecbt, (ay_voidfp)delcb, i)))
    return ay_status;

  /* register copy callback */
  if((ay_status = ay_table_additem(&ay_copycbt, (ay_voidfp)copycb, i)))
    return ay_status;

  /* register draw callback */
  if((ay_status = ay_table_additem(&ay_drawcbt, (ay_voidfp)drawcb, i)))
    return ay_status;

  /* register draw points callback */
  if((ay_status = ay_table_additem(&ay_drawhcbt, (ay_voidfp)drawhcb, i)))
    return ay_status;

  /* register shade callback */
  if((ay_status = ay_table_additem(&ay_shadecbt, (ay_voidfp)shadecb, i)))
    return ay_status;

  /* register setprop callback */
  if((ay_status = ay_table_additem(&ay_setpropcbt, (ay_voidfp)setpropcb, i)))
    return ay_status;

  /* register getprop callback */
  if((ay_status = ay_table_additem(&ay_getpropcbt, (ay_voidfp)getpropcb, i)))
    return ay_status;

  /* register pickpnt callback */
  if((ay_status = ay_table_additem(&ay_getpntcbt, (ay_voidfp)getpntcb, i)))
    return ay_status;

  /* register read callback */
  if((ay_status = ay_table_additem(&ay_readcbt, (ay_voidfp)readcb, i)))
    return ay_status;

  /* register write callback */
  if((ay_status = ay_table_additem(&ay_writecbt, (ay_voidfp)writecb, i)))
    return ay_status;

  /* register wrib callback */
  if((ay_status = ay_table_additem(&ay_wribcbt, (ay_voidfp)wribcb, i)))
    return ay_status;

  /* register bbc callback */
  if((ay_status = ay_table_additem(&ay_bbccbt, (ay_voidfp)bbccb, i)))
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
  entry = Tcl_CreateHashEntry(&ay_typenamesht, (char*)type_index, &new_item);
  Tcl_SetHashValue(entry, (void*)name);

  /*
  if((ay_status = ay_table_additemo(&ay_typenamest, (ay_voidfp)name,
				    type_index)))
    return ay_status;
  */
  /* register create callback */
  if((ay_status = ay_table_additem(&ay_createcbt, (ay_voidfp)crtcb,
				   type_index)))
    return ay_status;

  /* register delete callback */
  if((ay_status = ay_table_additem(&ay_deletecbt, (ay_voidfp)delcb,
				   type_index)))
    return ay_status;

  /* register copy callback */
  if((ay_status = ay_table_additem(&ay_copycbt, (ay_voidfp)copycb,
				   type_index)))
    return ay_status;

  /* register draw callback */
  if((ay_status = ay_table_additem(&ay_drawcbt, (ay_voidfp)drawcb,
				   type_index)))
    return ay_status;

  /* register draw points callback */
  if((ay_status = ay_table_additem(&ay_drawhcbt, (ay_voidfp)drawhcb,
				   type_index)))
    return ay_status;

  /* register shade callback */
  if((ay_status = ay_table_additem(&ay_shadecbt, (ay_voidfp)shadecb,
				   type_index)))
    return ay_status;

  /* register setprop callback */
  if((ay_status = ay_table_additem(&ay_setpropcbt, (ay_voidfp)setpropcb,
				   type_index)))
    return ay_status;

  /* register getprop callback */
  if((ay_status = ay_table_additem(&ay_getpropcbt, (ay_voidfp)getpropcb,
				   type_index)))
    return ay_status;

  /* register pickpnt callback */
  if((ay_status = ay_table_additem(&ay_getpntcbt, (ay_voidfp)getpntcb,
				   type_index)))
    return ay_status;

  /* register read callback */
  if((ay_status = ay_table_additem(&ay_readcbt, (ay_voidfp)readcb,
				   type_index)))
    return ay_status;

  /* register write callback */
  if((ay_status = ay_table_additem(&ay_writecbt, (ay_voidfp)writecb,
				   type_index)))
    return ay_status;

  /* register wrib callback */
  if((ay_status = ay_table_additem(&ay_wribcbt, (ay_voidfp)wribcb,
				   type_index)))
    return ay_status;

  /* register bbc callback */
  if((ay_status = ay_table_additem(&ay_bbccbt, (ay_voidfp)bbccb,
				   type_index)))
    return ay_status;

 return ay_status;
} /* ay_otype_registercore */

