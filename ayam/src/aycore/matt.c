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

/* matt.c - material tools - material object helpers */

static Tcl_HashTable ay_matt_ptr_ht;

static Tcl_HashTable ay_matt_blacklist_ht;

static char *ay_matt_mitagtype;

/* ay_matt_registermaterial:
 *  register a new material name
 *  if parameter is NULL: clear material database
 *  else: checks if name is already registered and returns
 *  AY_ERROR if it is already registered
 */
int
ay_matt_registermaterial(char *name, ay_mat_object *mat)
{
 int ay_status = AY_OK;
 int new_item = 0;
 Tcl_HashEntry *entry = NULL;
 Tcl_HashTable *ht = &ay_matt_ptr_ht;


  if(!name)
    {
      Tcl_DeleteHashTable(ht);
      Tcl_InitHashTable(ht, TCL_STRING_KEYS);
      return AY_OK;
    }

  if(!mat)
    return AY_ENULL;

  if((entry = Tcl_FindHashEntry(ht, name)))
    {
      mat->registered = AY_FALSE;
      return AY_ERROR; /* name already registered */
    }
  else
    {
      mat->registered = AY_TRUE;
      /* create new entry */
      entry = Tcl_CreateHashEntry(ht, name, &new_item);
      Tcl_SetHashValue(entry, (char*)mat); 
    }

 return ay_status;
} /* ay_matt_registermaterial */


/* ay_matt_isregistered:
 *  checks whether material name
 *  is registered in the material database
 *  returns AY_TRUE if it is registerd and AY_FALSE if not
 */
int
ay_matt_isregistered(char *name)
{
 Tcl_HashEntry *entry = NULL;
 Tcl_HashTable *ht = &ay_matt_ptr_ht;

  if(!name)
    return AY_FALSE;

  if(!(entry = Tcl_FindHashEntry(ht, name)))
    {
      return AY_FALSE; /* name is not registered */
    }

 return AY_TRUE;
} /* ay_matt_isregistered */


/* ay_matt_deregister:
 *  de-register name from the material database
 */
int
ay_matt_deregister(char *name)
{
 int ay_status = AY_OK;
 Tcl_HashEntry *entry = NULL;
 Tcl_HashTable *ht = &ay_matt_ptr_ht;

  if(!name)
    return AY_ENULL;

  if(!(entry = Tcl_FindHashEntry(ht, name)))
    {
      return AY_ERROR; /* name is not registered */
    }
  else
    {
      Tcl_DeleteHashEntry(entry);
    }
  

 return ay_status;
} /* ay_matt_deregister */


/* ay_matt_getmaterial:
 *  
 */
int
ay_matt_getmaterial(char *name, ay_mat_object **material)
{
 int ay_status = AY_OK;
 Tcl_HashEntry *entry = NULL;
 Tcl_HashTable *ht = &ay_matt_ptr_ht;

  if((!name) || (!material))
    return AY_ENULL;

  if(!(entry = Tcl_FindHashEntry(ht, name)))
    {
      return AY_ERROR; /* name is not registered */
    }
  else
    {
      *material = (ay_mat_object *)Tcl_GetHashValue(entry);
    }

 return ay_status;
} /* ay_matt_getmaterial */



/* ay_matt_removerefs:
 *  
 */
int
ay_matt_removerefs(ay_object *o, ay_mat_object *material)
{
 int ay_status = AY_OK;

 if(!o || !material)
   return AY_OK;

 while(o)
    {
      if(o->mat == material)
	{
	  o->mat = NULL;
	}

      if(o->down)
	{
	  ay_status = ay_matt_removerefs(o->down, material);
	}

      o = o->next;
    }

 return ay_status;
} /* ay_matt_removerefs */


/* ay_matt_removecliprefs:
 *  used by ay_clear_scene(); see there for description
 */
int
ay_matt_removecliprefs(ay_object *o)
{
 int ay_status = AY_OK;

 if(!o)
   return AY_OK;

  while(o->next)
    {
      if(o->type == AY_IDMATERIAL)
	{
	  ay_matt_removerefs(ay_clipboard, (ay_mat_object *)o->refine);

	  ay_matt_deregister(o->name);
	}

      if(o->down)
	{
	  ay_status = ay_matt_removecliprefs(o->down);
	}

      o = o->next;
    }

 return ay_status;
} /* ay_matt_removecliprefs */


/* ay_matt_connect:
 *  connect objects to the appropriate material objects
 *  (using MI tags and the material id hashtable)
 */
int
ay_matt_connect(ay_object *o)
{
 int ay_status = AY_OK;
 int found = AY_FALSE;
 Tcl_HashEntry *entry = NULL;
 ay_tag_object *tag = NULL;
 ay_mat_object *mat = NULL;
 Tcl_HashTable *ht = &ay_matt_ptr_ht;
 unsigned int *refcountptr = NULL;

 if(!o)
   return AY_OK;

  while(o)
    {
      found = AY_FALSE;
      tag = o->tags;
      while(tag && !found)
	{
	  if(tag->type == ay_matt_mitagtype)
	    {
	      if((entry = Tcl_FindHashEntry(ht, tag->val)))
		{
		  mat = (ay_mat_object *)Tcl_GetHashValue(entry);
		  o->mat = mat;
		  if(mat)
		    {
		      refcountptr = mat->refcountptr;
		      (*refcountptr)++;
		    }
		}
	      found = AY_TRUE;
	    } /* if */

	  tag = tag->next;
	} /* while */

      if(o->down)
	ay_status = ay_matt_connect(o->down);

      if(ay_status)
	return ay_status;

      o = o->next;
    } /* while */

 return ay_status;
} /* ay_matt_connect */


/* ay_matt_creatematerialids:
 *  creates material id tags for all objects with material
 *  assumes no MI-tags exist in scene!
 */
int
ay_matt_creatematerialids(ay_object *o)
{
 int ay_status = AY_OK;
 ay_tag_object *newtag = NULL; 
 ay_mat_object *mat = NULL;
 char *mname = NULL, *tname = NULL;;

  while(o)
    {
      if(o->mat)
	{
	  mat =  o->mat;
	  
	  if(!(tname = calloc(3, sizeof(char))))
	    return AY_EOMEM;

	  strcpy(tname, "MI");

	  if(!(newtag = calloc(1, sizeof(ay_tag_object))))
	    return AY_EOMEM;

	  newtag->name = tname;

	  mname = *(mat->nameptr);
	  if(!(newtag->val = calloc(strlen(mname)+1, sizeof(char))))
	    return AY_EOMEM;
	  strcpy(newtag->val, mname);
	  newtag->type = ay_matt_mitagtype;
	  newtag->next = o->tags;
	  o->tags = newtag;
	}

      if(o->down)
	ay_status = ay_matt_creatematerialids(o->down);

      if(ay_status)
	return ay_status;

      o = o->next;
    }

 return ay_status;
} /* ay_matt_creatematerialids */


/* ay_matt_clearmaterialids:
 *  clear all material id tags from scene
 */
int
ay_matt_clearmaterialids(ay_object *o)
{
 int ay_status = AY_OK;
 ay_tag_object *tag = NULL, **last = NULL;

  if(!o)
    return AY_OK;

  while(o)
    {
      if(o->tags)
	{
	  tag = o->tags;
	  last = &(o->tags);
	  tag = o->tags;
	  while(tag)
	    {
	      if(tag->type == ay_matt_mitagtype)
		{
		  *last = tag->next;
		  ay_tags_free(tag);
		  tag = *last;
		}
	      else
		{
		  last = &(tag->next);
		  tag = tag->next;
		} /* if */
	    } /* while */
	
	} /* if */

      if(o->down)
	ay_status = ay_matt_clearmaterialids(o->down);

      if(ay_status)
	return ay_status;

      o = o->next;
    }

 return ay_status;
} /* ay_matt_clearmaterialids */


/* ay_matt_wrib:
 *  write material attributes to RIB
 */
int
ay_matt_wrib(char *file, ay_mat_object *m)
{
 int ay_status = AY_OK;
 RtColor color = {0};
 RtFloat dbound;
 RtInt on = (RtInt)1, itemp;
 char *nonstr = "none", *opstr = "opaque", *shadestr = "shade";
 char *camstr = "camera", *objstr = "object", *shdstr = "shader";
 char *str = NULL;

  if(!m)
   return AY_ENULL;

  /* Color */
  if((m->colr != -1) && (m->colg != -1) && (m->colb != -1))
    {
      color[0] = (RtFloat)m->colr/255.0;
      color[1] = (RtFloat)m->colg/255.0;
      color[2] = (RtFloat)m->colb/255.0;
      RiColor(color);
    }

  /* Opacity */
  if((m->opr != -1) && (m->opg != -1) && (m->opb != -1))
    {
      color[0] = (RtFloat)m->opr/255.0;
      color[1] = (RtFloat)m->opg/255.0;
      color[2] = (RtFloat)m->opb/255.0;
      RiOpacity(color);
    }

  if(m->shading_rate != 1.0)
    RiShadingRate((RtFloat)m->shading_rate);

  if(m->shading_interpolation)
    RiShadingInterpolation(RI_SMOOTH);
  /*  else
      RiShadingInterpolation(RI_CONSTANT);*/


  if(!ay_prefs.ristandard)
    {
      if(m->true_displacement)
	{
	  RiAttribute("render", "truedisplacement", &on, RI_NULL);
	}

      switch(m->dbound)
	{
	case 0:
	  str = objstr;
	  break;
	case 1:
	  str = shdstr;
	  break;
	case 2:
	  str = camstr;
	  break;
	      
	}

      dbound = (RtFloat)m->dbound_val;
      if(dbound != 0.0)
	{
	  RiAttribute("displacementbound", "coordinatesystem",
		      (RtPointer)&str, "sphere", &dbound, RI_NULL);
	}

      if(m->cast_shadows)
	{
	  if(m->cast_shadows == 1)
	    RiAttribute("render", "casts_shadows",
			(RtPointer)&nonstr, RI_NULL);
	  if(m->cast_shadows == 2)
	    RiAttribute("render", "casts_shadows",
			(RtPointer)&opstr, RI_NULL);
	  if(m->cast_shadows == 3)
	    RiAttribute("render", "casts_shadows",
			(RtPointer)&shadestr, RI_NULL);
	} /* if */

      /* visibility */
      if((!m->camera) || (!m->reflection) || (!m->shadow))
	{
	  itemp = 0;

	  if(m->camera)
	    itemp += 1;

	  if(m->reflection)
	    itemp += 2;

	  if(m->shadow)
	    itemp += 4;


	  RiAttribute("render", "visibility", (RtPointer)&itemp, RI_NULL);

	} /* if */
    } /* if !ristandard */

  if(m->sshader)
    {
      ay_status = ay_shader_wrib(m->sshader, AY_STSURFACE, NULL);
    }
  if(m->dshader)
    {
      ay_status = ay_shader_wrib(m->dshader, AY_STDISPLACEMENT, NULL);
    }
  if(m->ishader)
    {
      ay_status = ay_shader_wrib(m->ishader, AY_STINTERIOR, NULL);
    }
  if(m->eshader)
    {
      ay_status = ay_shader_wrib(m->eshader, AY_STEXTERIOR, NULL);
    }

 return AY_OK;
} /* ay_matt_wrib */


/* ay_matt_nomaterial:
 *  arrange for objects of type type not
 *  to be associable with materials
 */
void
ay_matt_nomaterial(unsigned int type)
{
 Tcl_HashEntry *entry = NULL;
 Tcl_HashTable *ht = &ay_matt_blacklist_ht;
 int new_item = 0;
  
 entry = Tcl_CreateHashEntry(ht, (char*)type, &new_item);
 Tcl_SetHashValue(entry, (char*)NULL); 

 return;
} /* ay_matt_nomaterial */


/* ay_matt_mayhavematerial:
 *  returns AY_TRUE, if objects of type type
 *  may have a material, AY_FALSE else
 */
int
ay_matt_mayhavematerial(unsigned int type)
{
  if(Tcl_FindHashEntry(&ay_matt_blacklist_ht, (char *)type) != NULL)
    return AY_FALSE;
  else
    return AY_TRUE;
} /* ay_matt_mayhavematerial */


/* ay_matt_init:
 *  initialize matt module
 */
void
ay_matt_init(Tcl_Interp *interp)
{

  /* hash table for material name -> object pointers */
  Tcl_InitHashTable(&ay_matt_ptr_ht, TCL_STRING_KEYS);

  /* hash table for object types, that may not get materials */
  Tcl_InitHashTable(&ay_matt_blacklist_ht, TCL_ONE_WORD_KEYS);

  /* register Material-ID tag type */
  ay_tags_register(interp, "MI", &ay_matt_mitagtype);

  return;
} /* ay_matt_init */
