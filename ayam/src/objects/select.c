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
#include <ctype.h>

/* select.c - select object */

static char *ay_select_name = "Select";

/* functions: */

/* ay_select_createcb:
 *  create callback function of select object
 */
int
ay_select_createcb(int argc, char *argv[], ay_object *o)
{
 ay_select_object *select = NULL;
 char fname[] = "crtselect";

  if(!o)
    return AY_ENULL;

  if(!(select = calloc(1, sizeof(ay_select_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  o->inherit_trafos = AY_TRUE;
  o->parent = AY_TRUE;
  o->refine = select;
  /*
  if(argc > 2)
    select->index = atoi(argv[2]);
  */

 return AY_OK;
} /* ay_select_createcb */


/* ay_select_deletecb:
 *  delete callback function of select object
 */
int
ay_select_deletecb(void *c)
{
 ay_select_object *select = NULL;

  if(!c)
    return AY_ENULL;

  select = (ay_select_object *)(c);

  if(select->indices)
    free(select->indices);

  if(select->seli)
    free(select->seli);

  free(select);

 return AY_OK;
} /* ay_select_deletecb */


/* ay_select_copycb:
 *  copy callback function of select object
 */
int
ay_select_copycb(void *src, void **dst)
{
 ay_select_object *select = NULL, *selectsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  selectsrc = (ay_select_object *)src;

  if(!(select = malloc(sizeof(ay_select_object))))
    return AY_EOMEM;

  memcpy(select, src, sizeof(ay_select_object));

  if(selectsrc->indices)
    {
      if(!(select->indices = malloc((strlen(selectsrc->indices)+1) *
				    sizeof(char))))
	{ free(select); return AY_EOMEM; }

      strcpy(select->indices, selectsrc->indices);
    }

  select->seli = NULL;

  *dst = select;

 return AY_OK;
} /* ay_select_copycb */


/* ay_select_drawcb:
 *  draw (display in an Ayam view window) callback function of select object
 */
int
ay_select_drawcb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_select_drawcb */


/* ay_select_drawhcb:
 *  draw handles (in an Ayam view window) callback function of select object
 */
int
ay_select_drawhcb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_select_drawhcb */


/* ay_select_shadecb:
 *  shade (display in an Ayam view window) callback function of select object
 */
int
ay_select_shadecb(struct Togl *togl, ay_object *o)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_select_shadecb */


/* ay_select_getpntcb:
 *  get point (editing and selection) callback function of select object
 */
int
ay_select_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
  if(!o)
    return AY_ENULL;

 return AY_OK;
} /* ay_select_getpntcb */


/* ay_select_setpropcb:
 *  set property (from Tcl to C context) callback function of select object
 */
int
ay_select_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char fname[] = "setprop";
 char *n1 = "SelectAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_select_object *select = NULL;
 char *string = NULL;
 int stringlen;

  if(!o)
    return AY_ENULL;

  select = (ay_select_object *)o->refine;

  if(!select)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("Indices",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  string = Tcl_GetStringFromObj(to, &stringlen);
  if(!string)
    {
      ay_error(AY_ENULL, fname, NULL);
      goto cleanup;
    }

  if(select->indices)
    {
      free(select->indices);
      select->indices = NULL;
      o->modified = AY_TRUE;
    }

  if(stringlen > 0)
    {
      if(!(select->indices = calloc(stringlen+1, sizeof(char))))
	{
	  ay_error(AY_EOMEM, fname, NULL);
	  goto cleanup;
	}
      strcpy(select->indices, string);

      if(select->seli)
	free(select->seli);
      select->seli = NULL;
      o->modified = AY_TRUE;
    }

cleanup:

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  ay_notify_parent();

 return AY_OK;
} /* ay_select_setpropcb */


/* ay_select_getpropcb:
 *  get property (from C to Tcl context) callback function of select object
 */
int
ay_select_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="SelectAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_select_object *select = NULL;

  if(!o)
    return AY_ENULL;

  select = (ay_select_object *)o->refine;

  if(!select)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj("Indices",-1);
  to = Tcl_NewStringObj(select->indices,-1);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_select_getpropcb */


/* ay_select_readcb:
 *  read (from scene file) callback function of select object
 */
int
ay_select_readcb(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_select_object *select = NULL;
 int tmpi;
 char *buf = NULL;

  if(!o)
    return AY_ENULL;

  if(!(select = calloc(1, sizeof(ay_select_object))))
    { return AY_EOMEM; }

  if(ay_read_version >= 13)
    {
      /* since 1.16 */
      fscanf(fileptr, "%d", &tmpi);
      fgetc(fileptr);

      ay_status = ay_read_string(fileptr, &(select->indices));
      if(ay_status)
	{
	  free(select);
	  return ay_status;
	}
    }
  else
    {
      fscanf(fileptr, "%d\n", &tmpi);

      if(!(buf = calloc(64, sizeof(char))))
	{ free(select); return AY_EOMEM; }
      sprintf(buf,"%d",tmpi);
      select->indices = buf;
    }

  o->refine = select;

 return AY_OK;
} /* ay_select_readcb */


/* ay_select_writecb:
 *  write (to scene file) callback function of select object
 */
int
ay_select_writecb(FILE *fileptr, ay_object *o)
{
 ay_select_object *select = NULL;

  if(!o)
    return AY_ENULL;

  select = (ay_select_object *)(o->refine);

  if(!select)
    return AY_ENULL;

  if(select->indices)
    fprintf(fileptr, "0\n%s\n", select->indices);
  else
    fprintf(fileptr, "0\n\n");

 return AY_OK;
} /* ay_select_writecb */


/* ay_select_wribcb:
 *  RIB export callback function of select object
 */
int
ay_select_wribcb(char *file, ay_object *o)
{
  /*
  ay_select_object *select = NULL;

  if(!o)
   return AY_ENULL;

  select = (ay_select_object*)o->refine;
  */
 return AY_OK;
} /* ay_select_wribcb */


/* ay_select_bbccb:
 *  bounding box calculation callback function of select object
 */
int
ay_select_bbccb(ay_object *o, double *bbox, int *flags)
{

  if(!o || !bbox || !flags)
    return AY_ENULL;

  /* we have no own bounding box, all that counts are the children */

  /* XXXX since we represent one or more of the childrens provided object(s)
     we should rather calculate the bounding box of _those_ instead */

  *flags = 2;

 return AY_OK;
} /* ay_select_bbccb */


/* ay_select_notifycb:
 *  notify callback function of select object
 */
int
ay_select_notifycb(ay_object *o)
{
 ay_select_object *select = NULL;

  if(!o)
    return AY_ENULL;

  select = (ay_select_object *)o->refine;

  if(!select)
    return AY_ENULL;

  /* if something changed below, the cached indices may be invalid now */
  if(select->seli)
    {
      free(select->seli);
      select->seli = NULL;
    }

 return AY_OK;
} /* ay_select_notifycb */


/* ay_select_providecb:
 *  provide callback function of select object
 */
int
ay_select_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 int i = 0, j = 0, *tmp = NULL;
 int index, startindex, endindex, lastindex;
 char *buf = NULL, *tok, *min;
 const char delim = ',';
 ay_object *down, *allprovided = NULL, **next;
 ay_object *selected = NULL;
 ay_select_object *sel = NULL;

  if(!o)
    return AY_ENULL;

  down = o->down;

  if(!result)
    {
      /* check, whether we can provide atleast one object
	 of the wanted type */
      while(down->next)
	{
	  ay_status = ay_provide_object(down, type, NULL);
	  if(ay_status == AY_OK)
	    return AY_OK;
	  down = down->next;
	}

      return AY_ERROR;
    } /* if */

  sel = (ay_select_object *)o->refine;

  if(!sel)
    return AY_ENULL;

  if(!sel->indices)
    return AY_OK;

  if(strlen(sel->indices) == 0)
    return AY_OK;

  /* get and count all provided objects first;
     we need to do this first, to know what 'e' means
     when interpreting the indices string
  */
  if(!o->down || !o->down->next)
    {
      return AY_OK;
    }
  next = &(allprovided);
  lastindex = 0;
  down = o->down;
  while(down && down->next)
    {
      ay_status = ay_provide_object(down, type, next);

      while(*next)
	{
	  lastindex++;
	  next = &((*next)->next);
	}

      down = down->next;
    }

  /* no provided objects => no need to parse the indices string at all */
  if(lastindex == 0)
    return AY_OK;

  /* parse indices string only, if there is no cached index array */
  if(!sel->seli)
    {
      /* (re-)parse indices */

      if(!(buf = calloc(strlen(sel->indices)+1, sizeof(char))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      strcpy(buf, sel->indices);

      if(!(sel->seli = calloc(64, sizeof(int))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      tok = strtok(buf, &delim);
      while(tok)
	{
	  /* is there a '-' character in the token? */
	  min = strchr(tok, '-');
	  if(min)
	    {
	      /* parse range */

	      /* get start index of range */
	      /* jump over leading white-space */
	      while(isspace(*tok))
		{
		  tok++;
		}
	      /* sanity check for half empty ranges: ",-end,..." or ", -3,...",
		 empty ranges "1,-,..." _and_
		 premature end of string "1,2-" ... */
	      if((*tok == '\0') || (*tok == delim) || (*tok == '-'))
		{
		  /* ignore this token, process next token */
		  tok = strtok(NULL, &delim);
		}
	      if(*tok == 'e')
		{
		  startindex = lastindex;
		}
	      else
		{
		  startindex = atoi(tok);
		}

	      /* get end index of range */
	      /* jump over the '-' */
	      min++;
	      /* jump over leading white-space */
	      while(isspace(*min))
		{
		  min++;
		}
	      /* sanity check for half empty ranges: ",1-,..." and
		 premature end of string "1,2-"... */
	      if((*min == '\0') || (*min == delim))
		{
		  /* ignore this token, process next token */
		  tok = strtok(NULL, &delim);
		  continue;
		}
	      if(*min == 'e')
		{
		  endindex = lastindex;
		}
	      else
		{
		  endindex = atoi(min);
		}

	      /* add range to index array */
	      tmp = NULL;
	      if(!(tmp = realloc(sel->seli,
				 (i+(endindex-startindex))*sizeof(int))))
		{ ay_status = AY_EOMEM; goto cleanup; }
	      sel->seli = tmp;
	      /* increasing range */
	      if(endindex > startindex)
		{
		  for(j = 0; j < (endindex-startindex); j++)
		    {
		      sel->seli[i+j] = startindex+j;
		    }
		}
	      /* decreasing range */
	      if(endindex < startindex)
		{
		  for(j = 0; j < (endindex-startindex); j--)
		    {
		      sel->seli[i+j] = endindex-j;
		    }
		}
	      /* no(n)sense range */
	      if(endindex == startindex)
		{
		  tmp = NULL;
		  if(!(tmp = realloc(sel->seli, (i+1)*sizeof(int))))
		    { ay_status = AY_EOMEM; goto cleanup; }
		  sel->seli = tmp;
		  sel->seli[i] = startindex;
		  i++;
		}

	      i++;
	    }
	  else
	    {
	      /* parse single index */

	      /* jump over leading white-space */
	      while(isspace(*tok))
		{
		  tok++;
		}
	      /* sanity check for empty ranges: "1,,2,..." and
		 premature end of string "1,"... */
	      if((*tok == '\0') || (*tok == delim))
		{
		  /* ignore this token, process next token */
		  tok = strtok(NULL, &delim);
		  continue;
		}
	      if(*tok == 'e')
		{
		  index = lastindex;
		}
	      else
		{
		  index = atoi(tok);
		}

	      /* add single index to index array */
	      tmp = NULL;
	      if(!(tmp = realloc(sel->seli, (i+1)*sizeof(int))))
		{ ay_status = AY_EOMEM; goto cleanup; }
	      sel->seli = tmp;
	      sel->seli[i] = index;
	      i++;
	    } /* if */

	  /* process next token */
	  tok = strtok(NULL, &delim);
	} /* while */

      sel->length = i;
    } /* if */

  next = &selected;
  lastindex = 0;
  down = allprovided;
  for(i = 0; i < sel->length; i++)
    {
      index = sel->seli[i];
      if(index >= lastindex)
	{
	  for(j = 0; j < index-lastindex; j++)
	    {
	      down = down->next;
	      /* sanity check, index too high? */
	      if(!down)
		break;
	    }
	}
      else
	{
	  down = allprovided;
	  for(j = 0; j < index; j++)
	    {
	      down = down->next;
	      /* sanity check, index too high? */
	      if(!down)
		break;
	    }
	} /* if */

      if(down)
	{
	  /* XXXX make this smarter by not copying _every_ object */
	  ay_status = ay_object_copy(down, next);
	  next = &((*next)->next);
	  lastindex = index;
	}
      else
	{
	  /* we run over the end of the list of provided objects for
	     this index, maybe we have more luck with the next one... */
	  down = allprovided;
	  lastindex = 0;
	}
    } /* for */

  *result = selected;

cleanup:

  if(buf)
    free(buf);

  if(allprovided)
    (void)ay_object_deletemulti(allprovided, AY_FALSE);

 return ay_status;
} /* ay_select_providecb */


/* ay_select_init:
 *  initialize the select object module
 */
int
ay_select_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_select_name,
				    ay_select_createcb,
				    ay_select_deletecb,
				    ay_select_copycb,
				    NULL, /* no drawing! */
				    NULL, /* no handles! */
				    NULL, /* no shading! */
				    ay_select_setpropcb,
				    ay_select_getpropcb,
				    NULL, /* no picking! */
				    ay_select_readcb,
				    ay_select_writecb,
				    ay_select_wribcb,
				    ay_select_bbccb,
				    AY_IDSELECT);

  ay_status += ay_provide_register(ay_select_providecb, AY_IDSELECT);

  ay_status += ay_notify_register(ay_select_notifycb, AY_IDSELECT);

 return ay_status;
} /* ay_select_init */
