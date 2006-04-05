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

/* script.c - script object */

/*
 * ToDo:
 *  option to automatically add "NoExport" tags to children to be modified
 */

static char *ay_script_name = "Script";

int
ay_script_createcb(int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char fname[] = "crtscript";
 ay_script_object *new = NULL;

  if(!o)
    return AY_ENULL;

  if(!(new = calloc(1, sizeof(ay_script_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  o->inherit_trafos = AY_TRUE;
  o->parent = AY_TRUE;
  o->refine = new;

 return AY_OK;
} /* ay_script_createcb */


int
ay_script_deletecb(void *c)
{
 ay_script_object *sc = NULL;

  if(!c)
    return AY_ENULL;

  sc = (ay_script_object *)(c);

  /* free the script string */
  if(sc->script)
    free(sc->script);

  if(sc->cm_objects)
    ay_object_deletemulti(sc->cm_objects);

  /* free compiled script */
  if(sc->cscript)
    {
      Tcl_DecrRefCount(sc->cscript);
      sc->cscript = NULL;
    }

  free(sc);

 return AY_OK;
} /* ay_script_deletecb */


int
ay_script_copycb(void *src, void **dst)
{
 ay_script_object *scdst = NULL, *scsrc = NULL;

  if(!src || !dst)
    return AY_ENULL;

  scsrc = (ay_script_object *)src;

  if(!(scdst = calloc(1, sizeof(ay_script_object))))
    return AY_EOMEM;

  memcpy(scdst, scsrc, sizeof(ay_script_object));

  /* copy script string */
  if(scsrc->script)
    {
      if(!(scdst->script = calloc(strlen(scsrc->script)+1, sizeof(char))))
	{
	  free(scdst);
	  return AY_EOMEM;
	}
      strcpy(scdst->script, scsrc->script);
    } /* if */

  scdst->cm_objects = NULL;

  scdst->cscript = NULL;

  *dst = (void *)scdst;

 return AY_OK;
} /* ay_script_copycb */


int
ay_script_drawcb(struct Togl *togl, ay_object *o)
{
 ay_object *mo = NULL;
 ay_script_object *sc = NULL;

  if(!o)
    return AY_ENULL;

  /* ignore own transformation attributes */
  glLoadIdentity();

  sc = (ay_script_object *)o->refine;

  switch(sc->type)
    {
    case 1:
    case 2:
      if(sc->cm_objects)
	{
	  mo = sc->cm_objects;
	  while(mo)
	    {
	      ay_draw_object(togl, mo, AY_TRUE);
	      mo = mo->next;
	    } /* while */
	} /* if */
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_script_drawcb */


int
ay_script_shadecb(struct Togl *togl, ay_object *o)
{
 ay_object *mo = NULL;
 ay_script_object *sc = NULL;

  if(!o)
    return AY_ENULL;

  /* ignore own transformation attributes */
  glLoadIdentity();

  sc = (ay_script_object *)o->refine;

  switch(sc->type)
    {
    case 1:
    case 2:
      if(sc->cm_objects)
	{
	  mo = sc->cm_objects;
	  while(mo)
	    {
	      ay_shade_object(togl, mo, AY_FALSE);
	      mo = mo->next;
	    } /* while */
	} /* if */
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_script_shadecb */


int
ay_script_drawhcb(struct Togl *togl, ay_object *o)
{

 return AY_OK;
} /* ay_script_drawhcb */


int
ay_script_getpntcb(int mode, ay_object *o, double *p)
{

 return AY_OK;
} /* ay_script_getpntcb */


/* Tcl -> C! */
int
ay_script_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1 = "ScriptAttrData";
 char fname[] = "script_setpropcb";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_script_object *sc = NULL;
 char *string;
 int stringlen;

  if(!o)
    return AY_ENULL;

  sc = (ay_script_object *)o->refine;

  toa = Tcl_NewStringObj(n1, -1);
  ton = Tcl_NewStringObj(n1, -1);

  Tcl_SetStringObj(ton, "Active", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(sc->active));

  Tcl_SetStringObj(ton, "Type", -1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &(sc->type));

  switch(sc->type)
    {
    case 0:
      o->hide_children = AY_FALSE;
      o->parent = AY_TRUE;
      break;
    case 1:
      o->hide_children = AY_TRUE;
      o->parent = AY_FALSE;
      break;
    case 2:
      o->hide_children = AY_TRUE;
      o->parent = AY_TRUE;
      break;
    default:
      o->hide_children = AY_FALSE;
      o->parent = AY_TRUE;
      break;
    } /* switch */

  Tcl_SetStringObj(ton,"Script",-1);
  to = Tcl_ObjGetVar2(interp, toa, ton, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  string = Tcl_GetStringFromObj(to, &stringlen);
  if(!string)
    {
      ay_error(AY_ENULL, fname, NULL);
      return TCL_OK;
    }
  if(stringlen > 0)
    {
      if(!sc->script || (sc->script && strcmp(sc->script, string)))
	{
	  if(sc->script)
	    free(sc->script);
	  sc->script = NULL;
	  if(!(sc->script = calloc(stringlen+1, sizeof(char))))
	    {
	      ay_error(AY_EOMEM, fname, NULL);
	      return TCL_OK;
	    }
	  strcpy(sc->script, string);
	} /* if */
    }
  else
    {
      if(sc->script)
	free(sc->script);
      sc->script = NULL;
    } /* if */

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

  sc->modified = AY_TRUE;

  ay_status = ay_notify_force(o);

  ay_status = ay_notify_parent();

 return AY_OK;
} /* ay_script_setpropcb */


/* C -> Tcl! */
int
ay_script_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="ScriptAttrData", *empty = "";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_script_object *sc = NULL;

  if(!o)
    return AY_ENULL;

  sc = (ay_script_object *)(o->refine);

  toa = Tcl_NewStringObj(n1, -1);

  ton = Tcl_NewStringObj(n1, -1);

  Tcl_SetStringObj(ton, "Active", -1);
  to = Tcl_NewIntObj(sc->active);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Type", -1);
  to = Tcl_NewIntObj(sc->type);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton, "Script", -1);
  if(sc->script)
    to = Tcl_NewStringObj(sc->script, -1);
  else
    to = Tcl_NewStringObj(empty, -1);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_script_getpropcb */


int
ay_script_readcb(FILE *fileptr, ay_object *o)
{
 ay_script_object *sc = NULL;
 int i, len;
 int readc, deactivate = 0;
 char script_disable_cmd[] = "script_disable";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 char a1[] = "ay", n1[] = "scriptdisable";
 char *arrname = NULL, *membername = NULL, *memberval = NULL;
 int arrmembers = 0;
 Tcl_Interp *interp = ay_interp;

  if(!o)
    return AY_ENULL;

  if(!(sc = calloc(1, sizeof(ay_script_object))))
    { return AY_EOMEM; }

  fscanf(fileptr, "%d\n", &sc->active);
  fscanf(fileptr, "%d\n", &sc->type);
  fscanf(fileptr, "%d\n", &len);

  if(len > 0)
    {
      if(!(sc->script = calloc(len+1, sizeof(char))))
	{
	  free(sc);
	  return AY_EOMEM;
	}

      for(i = 0; i < len; i++)
	{
	  readc = getc(fileptr);
	  if(readc == EOF)
	    return AY_EUEOF;
	  (sc->script)[i] = (char)readc;
	} /* for */
    } /* if */

  if(ay_read_version >= 8)
    {
      if(strstr(sc->script, "# Ayam, save array:"))
	{
	  arrname = strchr(sc->script, ':');
	  if(arrname[1] == ' ')
	    arrname++;

	  fscanf(fileptr, "%d\n", &arrmembers);

	  for(i = 0; i < arrmembers; i++)
	    {
	      ay_read_string(fileptr, &membername);
	      ay_read_string(fileptr, &memberval);

	      Tcl_SetVar2(interp, arrname, membername, memberval,
			  TCL_GLOBAL_ONLY);

	      free(membername);
	      membername = NULL;
	      free(memberval);
	      memberval = NULL;
	    } /* for */
	} /* if */
    } /* if */

  o->refine = sc;

  ay_trafo_defaults(o);

  if(sc->active)
    {
      Tcl_Eval(ay_interp, script_disable_cmd);
      toa = Tcl_NewStringObj(a1, -1);
      ton = Tcl_NewStringObj(n1, -1);
      to = Tcl_ObjGetVar2(ay_interp, toa, ton,
			  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      Tcl_GetIntFromObj(ay_interp, to, &(deactivate));

      if(deactivate)
	{
	  sc->active = AY_FALSE;
	}

      Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
    } /* if */

 return AY_OK;
} /* ay_script_readcb */


int
ay_script_writecb(FILE *fileptr, ay_object *o)
{
 ay_script_object *sc = NULL;
 char *arrname = NULL, *membername = NULL, *memberval = NULL;
 char *arrnameend = NULL;
 Tcl_Obj *arrmemberlist = NULL, *arrmember;
 int arrmembers = 0, i, slen;
 Tcl_Interp *interp = ay_interp;

  if(!o)
    return AY_ENULL;

  sc = (ay_script_object *)(o->refine);

  fprintf(fileptr, "%d\n", sc->active);
  fprintf(fileptr, "%d\n", sc->type);
  if(sc->script)
    {
      fprintf(fileptr, "%d\n", strlen(sc->script));
      fprintf(fileptr, "%s\n", sc->script);
    }
  else
    {
      fprintf(fileptr, "0\n");
    } /* if */

  if(strstr(sc->script, "# Ayam, save array:"))
    {
      arrname = strchr(sc->script, ':');
      arrname++;
      while(arrname[0] == ' ')
	arrname++;

      arrnameend = strchr(arrname, '\n');

      if(arrnameend)
	*arrnameend = '\0';

      Tcl_VarEval(interp, "global ", arrname, ";",
		  "array names ", arrname, ";", (char*)NULL);

      arrmemberlist = Tcl_GetObjResult(interp);

      Tcl_ListObjLength(interp, arrmemberlist, &arrmembers);

      fprintf(fileptr, "%d", arrmembers);

      for(i = 0; i < arrmembers; i++)
	{
	  arrmember = NULL;
	  Tcl_ListObjIndex(interp, arrmemberlist, i, &arrmember);
	  if(arrmember)
	    {
	      membername = Tcl_GetStringFromObj(arrmember, &slen);
	      if(membername)
		{
		  memberval = Tcl_GetVar2(interp, arrname, membername,
					  TCL_GLOBAL_ONLY);
		  if(memberval)
		    {
		      fprintf(fileptr, "%s\n%s\n", membername, memberval);
		    } /* if */
		} /* if */
	    } /* if */
	} /* for */

      if(arrnameend)
	*arrnameend = '\n';

    } /* if */

 return AY_OK;
} /* ay_script_writecb */


int
ay_script_wribcb(char *file, ay_object *o)
{
 ay_script_object *sc = NULL;
 ay_object *mo = NULL;

  if(!o)
   return AY_ENULL;

  sc = (ay_script_object*)o->refine;

  switch(sc->type)
    {
    case 1:
    case 2:
      if(sc->cm_objects)
	{
	  mo = sc->cm_objects;
	  while(mo)
	    {
	      ay_wrib_object(file, mo);
	      mo = mo->next;
	    } /* while */
	} /* if */
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_script_wribcb */


int
ay_script_bbccb(ay_object *o, double *bbox, int *flags)
{
 int ay_status = AY_OK;
 int i, a;
 double bbt[24] = {0};
 double xmin = DBL_MAX, xmax = -DBL_MAX, ymin = DBL_MAX;
 double ymax = -DBL_MAX, zmin = DBL_MAX, zmax = -DBL_MAX;
 ay_script_object *sc = NULL;
 ay_object *mo = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  sc = (ay_script_object *)(o->refine);

  switch(sc->type)
    {
    case 1:
    case 2:
      if(sc->cm_objects)
	{
	  mo = sc->cm_objects;
	  while(mo)
	    {
	      ay_status = ay_bbc_get(mo, bbt);

	      if(!ay_status)
		{
		  /* bounding box of object mo is valid => merge it in */
		  a = 0;
		  for(i = 0; i < 8; i++)
		    {
		      if(bbt[a] < xmin)
			xmin = bbt[a];
		      if(bbt[a] > xmax)
			xmax = bbt[a];
		      a += 3;
		    }

		  a = 1;
		  for(i = 0; i < 8; i++)
		    {
		      if(bbt[a] < ymin)
			ymin = bbt[a];
		      if(bbt[a] > ymax)
			ymax = bbt[a];
		      a += 3;
		    }

		  a = 2;
		  for(i = 0; i < 8; i++)
		    {
		      if(bbt[a] < zmin)
			zmin = bbt[a];
		      if(bbt[a] > zmax)
			zmax = bbt[a];
		      a += 3;
		    }
		} /* if */

	      mo = mo->next;
	    } /* while */

	  /* P1 */
	  bbox[0] = xmin; bbox[1] = ymax; bbox[2] = zmax;
	  /* P2 */
	  bbox[3] = xmin; bbox[4] = ymax; bbox[5] = zmin;
	  /* P3 */
	  bbox[6] = xmax; bbox[7] = ymax; bbox[8] = zmin;
	  /* P4 */
	  bbox[9] = xmax; bbox[10] = ymax; bbox[11] = zmax;

	  /* P5 */
	  bbox[12] = xmin; bbox[13] = ymin; bbox[14] = zmax;
	  /* P6 */
	  bbox[15] = xmin; bbox[16] = ymin; bbox[17] = zmin;
	  /* P7 */
	  bbox[18] = xmax; bbox[19] = ymin; bbox[20] = zmin;
	  /* P8 */
	  bbox[21] = xmax; bbox[22] = ymin; bbox[23] = zmax;
	}
      else
	{
	  /* we have no own bounding box, all that counts are the children */
	  *flags = 2;
	} /* if */
      break;
    default:
      /* we have no own bounding box, all that counts are the children */
      *flags = 2;
      break;
    }

 return AY_OK;
} /* ay_script_bbccb */


int
ay_script_notifycb(ay_object *o)
{
 int ay_status = AY_OK, result = TCL_OK;
 char fname[] = "script_notifycb";
 ay_object *down = NULL, **nexto = NULL, **old_aynext, *ccm_objects;
 ay_list_object *l = NULL, *old_sel = NULL;
 ay_script_object *sc = NULL, *csc = NULL;
 static int sema = 0;
 int old_rdmode = 0;
 ClientData old_restrictcd;

  /* this semaphor protects ourselves from running in an endless
     recursive loop should the script modify our child objects */
  if(sema)
    {
      return AY_OK;
    }
  else
    {
      sema = 1;
    } /* if */

  if(!o)
    {
      sema = 0;
      return AY_ENULL;
    } /* if */

  ay_trafo_defaults(o);

  sc = (ay_script_object *)(o->refine);

  if((sc->active == 0) || (!sc->script) || (sc->script && !strlen(sc->script)))
    {
      /* script is inactive or empty, bail out... */
      sema = 0;
      return AY_OK;
    } /* if */

  /* prepare compiling the script? */
  if(sc->modified || (!sc->cscript))
    {
      if(sc->cscript)
	{
	  Tcl_DecrRefCount(sc->cscript);
	  sc->cscript = NULL;
	}
      sc->cscript = Tcl_NewStringObj(sc->script, -1);
      Tcl_IncrRefCount(sc->cscript);
      sc->modified = AY_FALSE;
    }

  if(sc->type == 0)
    {
      /* Just Run */
      result = Tcl_EvalObjEx(ay_interp, sc->cscript, TCL_EVAL_GLOBAL);
      /*result = Tcl_GlobalEval(ay_interp, sc->script);*/
    } /* if */

  if(sc->type == 1)
    {
      /* Create Children */

      if(sc->cm_objects)
	{
	  ay_status = ay_object_deletemulti(sc->cm_objects);
	  sc->cm_objects = NULL;
	}

      old_sel = ay_selection;
      ay_selection = NULL;

      /* remove old children */
      down = o->down;
      while(down && down->next)
	{
	  o->down = down->next;
	  free(down);
	  down = o->down;
	} /* while */

      old_aynext = ay_next;

      /* evaluate (execute) script string */
      result = Tcl_EvalObjEx(ay_interp, sc->cscript, TCL_EVAL_GLOBAL);
      /*result = Tcl_GlobalEval(ay_interp, sc->script);*/

      /* move newly created objects to script object */
      if(old_aynext != ay_next)
	{
	  sc->cm_objects = *old_aynext;
	  *old_aynext = *ay_next;
	  *ay_next = NULL;
	  ay_next = old_aynext;
	}

      /* restore old selection */
      while(ay_selection)
	{
	  l = ay_selection->next;
	  if(ay_selection->object)
	    ay_selection->object->selected = AY_FALSE;
	  free(ay_selection);
	  ay_selection = l;
	} /* while */

      ay_selection = old_sel;

    } /* if */

  if(sc->type == 2)
    {
      /* Modify Children */

      if(sc->cm_objects)
	{
	  ay_status = ay_object_deletemulti(sc->cm_objects);
	  sc->cm_objects = NULL;
	}

      /* Do we have a (real) child? */
      if(o->down && o->down->next)
	{ /* Yes */

	  /* copy child(ren) and fake selection */
	  old_sel = ay_selection;
	  ay_selection = NULL;
	  down = o->down;
	  nexto = &(sc->cm_objects);

	  while(down && down->next)
	    {
	      if(down->type != AY_IDSCRIPT)
		{
		  ay_status = ay_object_copy(down, nexto);
		  ay_notify_force(*nexto);
		  ay_sel_add(*nexto);
		  nexto = &((*nexto)->next);
		}
	      else
		{
		  csc = down->refine;
		  ccm_objects = csc->cm_objects;
		  while(ccm_objects)
		    {
		      ay_status = ay_object_copy(ccm_objects, nexto);
		      ay_notify_force(*nexto);
		      ay_sel_add(*nexto);
		      nexto = &((*nexto)->next);
		      ccm_objects = ccm_objects->next;
		    } /* while */
		} /* if */
	      down = down->next;
	    } /* while */

	  /* evaluate (execute) script string */
	  if(ay_currentview)
	    {
	      old_rdmode = ay_currentview->redraw;
	      ay_currentview->redraw = AY_FALSE;
	    }

	  Tk_RestrictEvents(ay_ns_restrictall, NULL, &old_restrictcd);
	  result = Tcl_EvalObjEx(ay_interp, sc->cscript, TCL_EVAL_GLOBAL);
	  /*result = Tcl_GlobalEval(ay_interp, sc->script);*/
	  Tk_RestrictEvents(NULL, NULL, &old_restrictcd);

	  if(ay_currentview)
	    {
	      ay_currentview->redraw = old_rdmode;
	    }

	  /* restore old selection */
	  while(ay_selection)
	    {
	      l = ay_selection->next;
	      free(ay_selection);
	      ay_selection = l;
	    } /* while */
	  ay_selection = old_sel;
	} /* if */
    } /* if */

  if(result == TCL_ERROR)
    {
      ay_error(AY_ERROR, fname, "Script failed!");
    }

  sema = 0;

 return AY_OK;
} /* ay_script_notifycb */


int
ay_script_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_tag_object *tag = NULL;
 ay_script_object *sc = NULL;
 ay_level_object *newl = NULL;
 ay_object *cmo = NULL, *new = NULL;

  if(!o)
    return AY_ENULL;

  sc = (ay_script_object *)(o->refine);

  switch(sc->type)
    {
    case 1:
    case 2:
      if(sc->cm_objects)
	{
	  if(in_place)
	    {
	      if(sc->cm_objects->next)
		{
		  /* have multiple objects */
		  if(!(newl = calloc(1, sizeof(ay_level_object))))
		    return AY_EOMEM;
		  o->type = AY_IDLEVEL;
		  o->down = sc->cm_objects;
		  cmo = sc->cm_objects;
		  while(cmo->next)
		    {
		      cmo = cmo->next;
		    }
		  cmo->next = o->down;
		  if(sc->script)
		    free(sc->script);
		  free(o->refine);
		  newl->type = AY_LTLEVEL;
		  o->refine = newl;
		}
	      else
		{
		  /* have just one object */
		  cmo = sc->cm_objects;
		  o->type = cmo->type;
		  if(sc->script)
		    free(sc->script);
		  free(o->refine);
		  o->refine = cmo->refine;
		  ay_trafo_copy(cmo, o);
		  o->parent = cmo->parent;
		  o->hide_children = cmo->hide_children;
		  o->inherit_trafos = cmo->inherit_trafos;
		  o->hide = cmo->hide;
		  o->selp = cmo->selp;
		  if(o->tags)
		    {
		      tag = o->tags;
		      while(tag->next)
			{
			  tag = tag->next;
			} /* while */
		      tag->next = cmo->tags;
		    }
		  else
		    {
		      o->tags = cmo->tags;
		    } /* if */
		  o->mat = cmo->mat;
		} /* if */
	    }
	  else
	    {
	      cmo = sc->cm_objects;
	      while(cmo)
		{
		  new = NULL;
		  ay_status = ay_object_copy(cmo, &new);
		  if(ay_status == AY_OK && new)
		    {
		      ay_status = ay_object_link(new);
		    } /* if */
		  cmo = cmo->next;
		} /* while */
	    } /* if */
	} /* if */
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_script_convertcb */


int
ay_script_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 ay_script_object *sc = NULL;
 ay_object *cmo = NULL, *po = NULL, **npo = NULL;

  if(!o)
    return AY_ENULL;

  sc = (ay_script_object *)(o->refine);

  switch(sc->type)
    {
    case 1:
    case 2:
      if(sc->cm_objects)
	{
	  npo = &po;
	  cmo = sc->cm_objects;
	  while(cmo)
	    {
	      
	      if(cmo->type != type)
		{
		  ay_provide_object(cmo, type, npo);
		}
	      else
		{
		  ay_status = ay_object_copy(cmo, npo);
		}

	      if(*npo)
		npo = &((*npo)->next);

	      cmo = cmo->next;
	    } /* while */
	  *result = po;
	} /* if */
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_script_providecb */


int
ay_script_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_script_name,
				    ay_script_createcb,
				    ay_script_deletecb,
				    ay_script_copycb,
				    ay_script_drawcb,
				    ay_script_drawhcb, /* no handles? */
				    ay_script_shadecb,
				    ay_script_setpropcb,
				    ay_script_getpropcb,
				    ay_script_getpntcb,
				    ay_script_readcb,
				    ay_script_writecb,
				    ay_script_wribcb,
				    ay_script_bbccb,
				    AY_IDSCRIPT);


  ay_status = ay_notify_register(ay_script_notifycb, AY_IDSCRIPT);

  ay_status = ay_convert_register(ay_script_convertcb, AY_IDSCRIPT);

  ay_status = ay_provide_register(ay_script_providecb, AY_IDSCRIPT);

  ay_matt_nomaterial(AY_IDSCRIPT);

 return ay_status;
} /* ay_script_init */
