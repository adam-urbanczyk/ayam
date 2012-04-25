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
#include <ctype.h>

/* script.c - script object */

/*
 * ToDo:
 *  option to automatically add "NoExport" tags to children to be modified
 */

static char *ay_script_name = "Script";

static char *ay_script_sp = "SP"; /* Save (Individual) Parameters */

static char *ay_script_sa = "array:"; /* Save Array */

static char *ay_script_ul = "use:"; /* Use (Language) */

int ay_script_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe);

int ay_script_notifycb(ay_object *o);

int ay_script_hasnptrafo(ay_object *o);

/* functions: */

/* ay_script_createcb:
 *  create callback function of script object
 */
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


/* ay_script_deletecb:
 *  delete callback function of script object
 */
int
ay_script_deletecb(void *c)
{
 int result = TCL_OK;
 ay_script_object *sc = NULL;
 int i = 0;
 Tcl_Interp *interp = NULL;

  if(!c)
    return AY_ENULL;

  sc = (ay_script_object *)(c);

  /* free compiled script */
  if(sc->cscript)
    {
      if(sc->cb)
	{

#ifdef AYNOSAFEINTERP
	  interp = ay_interp;
#else
	  interp = ay_safeinterp;
#endif

	  /* JavaScript/Lua/... */
	  result = sc->cb(interp, NULL, AY_TRUE, &(sc->cscript));
	}

      /* if there is no callback or it did not clean up properly... */
      if(sc->cscript)
	{
	  Tcl_DecrRefCount(sc->cscript);
	  sc->cscript = NULL;
	}
    }

  /* free the script string */
  if(sc->script)
    free(sc->script);

  if(sc->cm_objects)
    ay_object_deletemulti(sc->cm_objects);

  /* free saved parameters */
  if(sc->params)
    {
      for(i = 0; i < sc->paramslen; i++)
	{
	  Tcl_DecrRefCount(sc->params[i]);
	}
      free(sc->params);
    }

  /* free read only points */
  if(sc->pnts)
    free(sc->pnts);

  free(sc);

 return AY_OK;
} /* ay_script_deletecb */


/* ay_script_copycb:
 *  copy callback function of script object
 */
int
ay_script_copycb(void *src, void **dst)
{
 ay_script_object *scdst = NULL, *scsrc = NULL;
 int i = 0;

  if(!src || !dst)
    return AY_ENULL;

  scsrc = (ay_script_object *)src;

  if(!(scdst = calloc(1, sizeof(ay_script_object))))
    return AY_EOMEM;

  memcpy(scdst, scsrc, sizeof(ay_script_object));

  scdst->pnts = NULL;
  scdst->pntslen = 0;

  /* copy saved parameters */
  scdst->params = NULL;
  if(scdst->paramslen)
    {
      if(!(scdst->params = calloc(scdst->paramslen, sizeof(Tcl_Obj*))))
	{
	  free(scdst);
	  return AY_EOMEM;
	}
      for(i = 0; i < scdst->paramslen; i++)
	{
	  scdst->params[i] = Tcl_DuplicateObj(scsrc->params[i]);
	  Tcl_IncrRefCount(scdst->params[i]);
	}
    }

  /* copy script string */
  if(scsrc->script)
    {
      if(!(scdst->script = calloc(strlen(scsrc->script)+1, sizeof(char))))
	{
	  if(scdst->params)
	    free(scdst->params);
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


/* ay_script_drawcb:
 *  draw (display in an Ayam view window) callback function of script object
 */
int
ay_script_drawcb(struct Togl *togl, ay_object *o)
{
 ay_object *mo = NULL;
 ay_script_object *sc = NULL;

  if(!o)
    return AY_ENULL;

  /* ignore own transformation attributes? */
  if(!o->tags || !ay_script_hasnptrafo(o))
    {
      glLoadIdentity();
    }

  sc = (ay_script_object *)o->refine;

  switch(sc->type)
    {
    case 1:
    case 2:
      if(sc->cm_objects)
	{
	  mo = sc->cm_objects;
	  while(mo && mo->next)
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


/* ay_script_shadecb:
 *  shade (display in an Ayam view window) callback function of script object
 */
int
ay_script_shadecb(struct Togl *togl, ay_object *o)
{
 ay_object *mo = NULL;
 ay_script_object *sc = NULL;

  if(!o)
    return AY_ENULL;

  /* ignore own transformation attributes? */
  if(!o->tags || !ay_script_hasnptrafo(o))
    {
      glLoadIdentity();
    }

  sc = (ay_script_object *)o->refine;

  switch(sc->type)
    {
    case 1:
    case 2:
      if(sc->cm_objects)
	{
	  mo = sc->cm_objects;
	  while(mo && mo->next)
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


/* ay_script_drawhcb:
 *  draw handles (in an Ayam view window) callback function of script object
 */
int
ay_script_drawhcb(struct Togl *togl, ay_object *o)
{
 ay_script_object *sc = NULL;
 double point_size = ay_prefs.handle_size;
 unsigned int i = 0, a = 0;

  if(!o)
    return AY_ENULL;

  /* ignore own transformation attributes? */
  if(!o->tags || !ay_script_hasnptrafo(o))
    {
      glLoadIdentity();
    }

  sc = (ay_script_object *)o->refine;

  if(!sc->pnts)
    {
      sc->pntslen = 1;
      ay_script_notifycb(o);
    }

  if(sc->pnts)
    {
      glColor3f((GLfloat)ay_prefs.obr, (GLfloat)ay_prefs.obg,
		(GLfloat)ay_prefs.obb);

      glPointSize((GLfloat)point_size);

      glBegin(GL_POINTS);
      for(i = 0; i < sc->pntslen; i++)
	{
	  glVertex3dv((GLdouble *)&sc->pnts[a]);
	  a += 4;
	}
      glEnd();

      glColor3f((GLfloat)ay_prefs.ser, (GLfloat)ay_prefs.seg,
		(GLfloat)ay_prefs.seb);
    }

 return AY_OK;
} /* ay_script_drawhcb */


/* ay_script_getpntcb:
 *  get point (editing and selection) callback function of script object
 */
int
ay_script_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 ay_script_object *sc = NULL;

  if(!o)
    return AY_ENULL;

  sc = (ay_script_object *)o->refine;

  if(!sc->pnts)
    {
      sc->pntslen = 1;
      ay_script_notifycb(o);
    }

 return ay_selp_getpnts(mode, o, p, pe, 1, sc->pntslen, 4, sc->pnts);
} /* ay_script_getpntcb */


/* ay_script_getsp:
 *  helper for setpropcb; manages the saved parameters
 */
int
ay_script_getsp(Tcl_Interp *interp, ay_script_object *sc)
{
 int ay_status = AY_OK;
 char *arrname = NULL;
 char *lineend = NULL, *arrnameend = NULL;
 Tcl_Obj *arrmemberlist = NULL, *arrmember;
 int arrmembers = 0, i;
 Tcl_Obj *toa = NULL, *ton = NULL;


  /* handle script parameters */
  if(sc->script)
    {
      lineend = strchr(sc->script, '\n');
      if(lineend)
	*lineend = '\0';
      else
	return ay_status;

      if(!(arrname = strstr(sc->script, ay_script_sa)))
	{
	  *lineend = '\n';
	  return ay_status;
	}

      arrname = strchr(arrname, ':');

      arrname++;
      while(arrname[0] == ' ' || arrname[0] == '\t')
	arrname++;

      arrnameend = arrname;
      while(isgraph(arrnameend[0]) && (arrnameend[0] != ',') &&
	    (arrnameend[0] != ';'))
	arrnameend++;

      if(sc->params)
	{
	  /* remove old saved parameters */
	  for(i = 0; i < sc->paramslen; i++)
	    {
	      Tcl_DecrRefCount(sc->params[i]);
	    }
	  free(sc->params);
	  sc->params = NULL;
	  sc->paramslen = 0;
	}

      toa = Tcl_NewStringObj(arrname, arrnameend - arrname);
      ton = Tcl_NewStringObj(ay_script_sp, -1);

      arrmemberlist = Tcl_ObjGetVar2(interp, toa, ton, TCL_GLOBAL_ONLY);

      if(arrmemberlist)
	{

	  Tcl_ListObjLength(interp, arrmemberlist, &arrmembers);

	  if(arrmembers > 0)
	    {
	      if(!(sc->params = calloc(arrmembers, sizeof(Tcl_Obj*))))
		{
		  ay_status = AY_EOMEM;
		}
	      else
		{
		  sc->paramslen = arrmembers;
		  for(i = 0; i < arrmembers; i++)
		    {
		      arrmember = NULL;
		      Tcl_ListObjIndex(interp, arrmemberlist, i, &arrmember);
		      if(arrmember)
			{
			  sc->params[i] =
			Tcl_DuplicateObj(Tcl_ObjGetVar2(interp, toa, arrmember,
						    TCL_GLOBAL_ONLY));
			  Tcl_IncrRefCount(sc->params[i]);
			  /**/
#ifndef AYNOSAFEINTERP
			  Tcl_ObjSetVar2(ay_safeinterp, toa, arrmember,
		      Tcl_ObjGetVar2(interp, toa, arrmember, TCL_GLOBAL_ONLY),
					 TCL_GLOBAL_ONLY);
#endif
			} /* if */
		    } /* for */
		} /* if */
	    } /* if */
	} /* if */

      if(lineend)
	*lineend = '\n';

      Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
    } /* if */

  return ay_status;
} /* ay_script_getsp */


/* ay_script_setpropcb:
 *  set property (from Tcl to C context) callback function of script object
 */
int
ay_script_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 /*int ay_status = AY_OK;*/
 char *n1 = "ScriptAttrData";
 char fname[] = "script_setpropcb";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_script_object *sc = NULL;
 char *string;
 int stringlen, newscript = AY_FALSE;

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
      return AY_ERROR;
    }
  if(!sc->script)
    {
      newscript = AY_TRUE;
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
	      return AY_ERROR;
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

  if(!newscript)
    ay_script_getsp(interp, sc);

  sc->modified = AY_TRUE;

  ay_notify_object(o);

  o->modified = AY_TRUE;

  ay_notify_parent();

  if(newscript)
    ay_script_getsp(interp, sc);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_script_setpropcb */


/* ay_script_getpropcb:
 *  get property (from C to Tcl context) callback function of script object
 */
int
ay_script_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 char *n1="ScriptAttrData", *empty = "";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 ay_script_object *sc = NULL;
 char *arrname = NULL;
 char *lineend = NULL, *arrnameend = NULL;
 Tcl_Obj *arrmemberlist = NULL, *arrmember;
 int arrmembers = 0, i;

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

  /* handle script parameters */
  if(sc->params && sc->script)
    {
      lineend = strchr(sc->script, '\n');
      if(lineend)
	{
	  *lineend = '\0';
	}
      else
	{
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

      if(!(arrname = strstr(sc->script, ay_script_sa)))
	{
	  goto cleanup;
	}

      arrname = strchr(arrname, ':');

      arrname++;
      while(arrname[0] == ' ' || arrname[0] == '\t')
	arrname++;

      arrnameend = arrname;
      while(isgraph(arrnameend[0]) && (arrnameend[0] != ',') &&
	    (arrnameend[0] != ';'))
	arrnameend++;

      Tcl_SetStringObj(toa, arrname, arrnameend - arrname);
      Tcl_SetStringObj(ton, ay_script_sp, -1);

      arrmemberlist = Tcl_ObjGetVar2(interp, toa, ton, TCL_GLOBAL_ONLY);

      if(arrmemberlist)
	{
	  Tcl_ListObjLength(interp, arrmemberlist, &arrmembers);

	  for(i = 0; i < arrmembers; i++)
	    {
	      arrmember = NULL;
	      Tcl_ListObjIndex(interp, arrmemberlist, i, &arrmember);
	      if(arrmember)
		{
		  Tcl_ObjSetVar2(interp, toa, arrmember, sc->params[i],
				 TCL_GLOBAL_ONLY);
		} /* if */
	    } /* for */
	} /* if */

    } /* if */

cleanup:

  if(lineend)
    *lineend = '\n';

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* ay_script_getpropcb */


/* ay_script_readcb:
 *  read (from scene file) callback function of script object
 */
int
ay_script_readcb(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_script_object *sc = NULL;
 int i;
 unsigned int j, len;
 int readc;
 Tcl_Interp *interp = NULL;
 Tcl_Obj *toa = NULL, *ton = NULL;
 char *arrname = NULL, *membername = NULL, *memberval = NULL;
 char *arrnameend = NULL, *lineend = NULL;
 int arrmembers = 0;
#ifdef AYNOSAFEINTERP
 int deactivate = 0;
 char script_disable_cmd[] = "script_disable";
 char a1[] = "ay", n1[] = "scriptdisable";
 Tcl_Obj *to = NULL;
#endif

  if(!o)
    return AY_ENULL;

#ifdef AYNOSAFEINTERP
  interp = ay_interp;
#else
  interp = ay_safeinterp;
#endif

  if(!(sc = calloc(1, sizeof(ay_script_object))))
    { return AY_EOMEM; }

  /* old versions may be written without parent flag set, correct */
  o->parent = AY_TRUE;

  fscanf(fileptr, "%d\n", &sc->active);
  fscanf(fileptr, "%d\n", &sc->type);
  fscanf(fileptr, "%u\n", &len);

  if(len > 0)
    {
      if(!(sc->script = calloc(len+1, sizeof(char))))
	{
	  free(sc);
	  return AY_EOMEM;
	}

      for(j = 0; j < len; j++)
	{
	  readc = getc(fileptr);
	  if(readc == EOF)
	    return AY_EUEOF;
	  (sc->script)[j] = (char)readc;
	} /* for */
    } /* if */

  if(ay_read_version >= 8)
    {
      if(sc->script)
	{
	  fscanf(fileptr, "%d\n", &arrmembers);

	  if(arrmembers > 0)
	    {
	      lineend = strchr(sc->script, '\n');
	      if(lineend)
		*lineend = '\0';

	      arrname = strstr(sc->script, ay_script_sa);
	      if(!arrname)
		{
		  if(lineend)
		    *lineend = '\n';
		  goto cleanup;
		}
	      arrname = strchr(arrname, ':');

	      arrname++;
	      while(arrname[0] == ' ' || arrname[0] == '\t')
		arrname++;

	      arrnameend = arrname;
	      while(isgraph(arrnameend[0]) && (arrnameend[0] != ',') &&
		    (arrnameend[0] != ';'))
		arrnameend++;

	      if(!(sc->params = calloc(arrmembers-1, sizeof(Tcl_Obj*))))
		{
		  ay_status = AY_EOMEM;
		  goto cleanup;
		}
	      sc->paramslen = arrmembers-1;

	      toa = Tcl_NewStringObj(arrname, arrnameend - arrname);
	      ton = Tcl_NewStringObj("", -1);
	      j = 0;
	      for(i = 0; i < arrmembers; i++)
		{
		  ay_read_string(fileptr, &membername);

		  ay_read_string(fileptr, &memberval);

		  Tcl_SetVar2(interp, Tcl_GetString(toa),
			      membername, memberval,
			      TCL_GLOBAL_ONLY);

		  /* do not put the SP list into the object! */
		  if(strcmp(membername, ay_script_sp) != 0)
		    {
		      Tcl_SetStringObj(ton, membername, -1);
		      sc->params[j] =
			Tcl_DuplicateObj(Tcl_ObjGetVar2(interp, toa, ton,
							TCL_GLOBAL_ONLY));
		      Tcl_IncrRefCount(sc->params[j]);
		      j++;
		    }

		  free(membername);
		  membername = NULL;
		  free(memberval);
		  memberval = NULL;
		} /* for */

	      Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
	      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

	      if(lineend)
		*lineend = '\n';

	    } /* if */
	} /* if */
    } /* if */

cleanup:

  o->refine = sc;

  /* clean transformation attributes if no NP Transformations
     tag is present */
  if(!o->tags || !ay_script_hasnptrafo(o))
    {
      ay_trafo_defaults(o);
    }

#ifdef AYNOSAFEINTERP
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
#endif

 return AY_OK;
} /* ay_script_readcb */


/* ay_script_writecb:
 *  write (to scene file) callback function of script object
 */
int
ay_script_writecb(FILE *fileptr, ay_object *o)
{
 ay_script_object *sc = NULL;
 char *lineend = NULL;
 char *arrname = NULL, *membername = NULL;
 char *arrnameend = NULL, *memberval = NULL;
 Tcl_Obj *arrmemberlist = NULL, *arrmember;
 int arrmembers = 0, i, slen, tlen;
 unsigned int len = 0;
 Tcl_Obj *toa = NULL, *ton = NULL;
 Tcl_Interp *interp = NULL;

  if(!o)
    return AY_ENULL;

#ifdef AYNOSAFEINTERP
  interp = ay_interp;
#else
  interp = ay_safeinterp;
#endif

  sc = (ay_script_object *)(o->refine);

  fprintf(fileptr, "%d\n", sc->active);
  fprintf(fileptr, "%d\n", sc->type);
  if(sc->script)
    {
      len = strlen(sc->script);
      fprintf(fileptr, "%u\n", len);
      fprintf(fileptr, "%s\n", sc->script);
    }
  else
    {
      fprintf(fileptr, "0\n");
    } /* if */

  if(sc->script)
    {
      lineend = strchr(sc->script, '\n');
      if(lineend)
	*lineend = '\0';

      arrname = strstr(sc->script, ay_script_sa);
      if(!arrname)
	{
	  if(lineend)
	    *lineend = '\n';
	  return AY_OK;
	}
      arrname = strchr(arrname, ':');

      arrname++;
      while(arrname[0] == ' ' || arrname[0] == '\t')
	arrname++;

      arrnameend = arrname;
      while(isgraph(arrnameend[0]) && (arrnameend[0] != ',') &&
	    (arrnameend[0] != ';'))
	arrnameend++;

      /* get "SP" array variable */
      toa = Tcl_NewStringObj(arrname,  arrnameend - arrname);
      ton = Tcl_NewStringObj(ay_script_sp, -1);

      arrmemberlist = Tcl_ObjGetVar2(interp, toa, ton, TCL_GLOBAL_ONLY);

      if(arrmemberlist)
	Tcl_ListObjLength(interp, arrmemberlist, &arrmembers);

      if(arrmembers > 0)
	{
	  fprintf(fileptr, "%d\n", arrmembers+1);
	  fprintf(fileptr, "%s\n%s\n", ay_script_sp,
		  Tcl_GetStringFromObj(arrmemberlist, &tlen));

	  for(i = 0; i < arrmembers; i++)
	    {
	      arrmember = NULL;
	      Tcl_ListObjIndex(interp, arrmemberlist, i, &arrmember);
	      if(arrmember)
		{
		  membername = Tcl_GetStringFromObj(arrmember, &slen);
		  if(membername)
		    {
		      if(sc->params && sc->params[i])
			memberval = Tcl_GetStringFromObj(sc->params[i], &tlen);
		      else
			memberval = NULL;
		      if(memberval)
			fprintf(fileptr, "%s\n%s\n", membername, memberval);
		    } /* if */
		} /* if */
	    } /* for */
	}
      else
	{
	  fprintf(fileptr, "0\n");
	} /* if */

      if(lineend)
	*lineend = '\n';

      Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

    } /* if */

 return AY_OK;
} /* ay_script_writecb */


/* ay_script_wribcb:
 *  RIB export callback function of script object
 */
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
	  while(mo->next)
	    {
	      ay_wrib_toolobject(file, mo, o);
	      mo = mo->next;
	    } /* while */
	} /* if */
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_script_wribcb */


/* ay_script_bbccb:
 *  bounding box calculation callback function of script object
 */
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

  if(!o || !bbox || !flags)
    return AY_ENULL;

  sc = (ay_script_object *)(o->refine);

  switch(sc->type)
    {
    case 1:
    case 2:
      if(sc->cm_objects)
	{
	  mo = sc->cm_objects;
	  while(mo->next)
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


/* ay_script_getlanguage:
 *  helper for notifycb; get language (process use:)
 */
int
ay_script_getlanguage(ay_script_object *sc, char **lang)
{
 int ay_status = AY_OK;
 char *lineend = NULL;
 char *langname = NULL, *langend = NULL;

  if(sc->script)
    {
      lineend = strchr(sc->script, '\n');
      if(lineend)
	*lineend = '\0';
      else
	return ay_status;

      if(!(langname = strstr(sc->script, ay_script_ul)))
	{
	  *lineend = '\n';
	  return ay_status;
	}

      langname = strchr(langname, ':');

      langname++;
      while(langname[0] == ' ' || langname[0] == '\t')
	langname++;

      langend = langname;
      while(isgraph(langend[0]) && (langend[0] != ',') && (langend[0] != ';'))
	langend++;

      if(!(*lang = calloc(langend - langname + 1, sizeof(char))))
	{
	  *lineend = '\n';
	  return AY_EOMEM;
	}
      strncpy(*lang, langname, langend - langname);

      *lineend = '\n';
    } /* if */

 return ay_status;
} /* ay_script_getlanguage */


/* ay_script_notifycb:
 *  notification callback function of script object
 */
int
ay_script_notifycb(ay_object *o)
{
 static int lock = 0;
 int ay_status = AY_OK, result = TCL_OK;
 char fname[] = "script_notifycb";
 char buf[256], *l1 = NULL, *l2 = NULL, *language = NULL;
 double *p1, *p2, dummy[3], m[16];
 unsigned int j = 0, a = 0;
 ay_object *down = NULL, **nexto = NULL, **old_aynext, *ccm_objects;
 ay_object *ddown = NULL, *last = NULL;
 ay_list_object *l = NULL, *old_sel = NULL, *sel = NULL;
 ay_list_object *old_currentlevel;
 ay_object *old_clipboard = NULL;
 ay_script_object *sc = NULL, *dsc = NULL;
 ay_pointedit pe = {0};
 ay_voidfp *arr = NULL;
 int i = 0;
 int old_rdmode = 0;
 ClientData old_restrictcd;
 Tcl_DString ds;
 Tcl_Interp *interp = NULL;
 Tcl_HashTable *ht = &ay_languagesht;
 Tcl_HashEntry *entry = NULL;

  /* this lock protects ourselves from running in an endless
     recursive loop should the script modify our child objects
     (leading to another round of notification) */
  if(lock)
    {
      /*
      ay_error(AY_ERROR, fname,
	       "Recursive call to Script object notification blocked!");
      */
      return AY_OK;
    }
  else
    {
      lock = 1;
    } /* if */

  /* check arguments */
  if(!o)
    {
      lock = 0;
      return AY_ENULL;
    } /* if */

  sc = (ay_script_object *)(o->refine);

  /* always clear the old read only points */
  if(sc->pnts)
    {
      free(sc->pnts);
      sc->pnts = NULL;
    }

  /* before we do anything, check whether we have a runnable script at all
     and check, whether we are active */
  if((sc->active == 0) || (!sc->script) || (sc->script && !strlen(sc->script)))
    {
      /* script is inactive or empty, bail out... */
      goto cleanup;
    } /* if */

#ifdef AYNOSAFEINTERP
  interp = ay_interp;
#else
  interp = ay_safeinterp;
#endif

  /* set up a clean environment */
  old_clipboard = ay_clipboard;
  ay_clipboard = NULL;

  old_currentlevel = ay_currentlevel;
  ay_currentlevel = NULL;
  ay_status = ay_clevel_add(ay_root);
  ay_status = ay_clevel_add(o);
  ay_status = ay_clevel_add(o->down);
  old_aynext = ay_next;
  ay_next = &(o->down);

  if(ay_currentview)
    {
      old_rdmode = ay_currentview->redraw;
      ay_currentview->redraw = AY_FALSE;
    }

  /* clean transformation attributes if no NP Transformations
     tag is present */
  if(!o->tags || !ay_script_hasnptrafo(o))
    {
      ay_trafo_defaults(o);
    }

  /* copy cached/saved individual parameters to global Tcl array */
  if(/*!sc->modified && */sc->params)
    {
      ay_script_getpropcb(interp, 0, NULL, o);
    }

  /* prepare compiling/compile the script? */
  if(sc->modified || (!sc->cscript))
    {
      sc->cb = NULL;

      /* get language (process use:) */
      ay_script_getlanguage(sc, &language);
      if(language)
	{
	  if((entry = Tcl_FindHashEntry(ht, language)))
	    {
	      arr = ay_sevalcbt.arr;
	      sc->cb = (ay_sevalcb *)(arr[(int)Tcl_GetHashValue(entry)]);
	    }
	  else
	    {
	      ay_error(AY_ERROR, fname, "unknown language");
	      ay_status = AY_ERROR;
	      goto resenv;
	    }
	}

      if(sc->cb)
	{
	  /* JavaScript/Lua/... */
	  result = sc->cb(interp, sc->script, AY_TRUE, &(sc->cscript));
	  if(result == TCL_ERROR)
	    goto resenv;
	}
      else
	{
	  /* Tcl */

	  /* clear old compiled script */
	  if(sc->cscript)
	    {
	      Tcl_DecrRefCount(sc->cscript);
	      sc->cscript = NULL;
	    }

	  sc->cscript = Tcl_NewStringObj(sc->script, -1);
	  Tcl_IncrRefCount(sc->cscript);
	  sc->modified = AY_FALSE;
	}
    } /* if */

  Tk_RestrictEvents(ay_ns_restrictall, NULL, &old_restrictcd);

  if(sc->type == 0)
    {
      /* Just Run */

      /* evaluate (execute) script */
      if(sc->cb)
	{
	  /* JavaScript/Lua/... */
	  result = sc->cb(interp, sc->script, AY_FALSE, &(sc->cscript));
	}
      else
	{
	  /* Tcl */
	  result = Tcl_EvalObjEx(interp, sc->cscript, TCL_EVAL_GLOBAL);
	}
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

      /* evaluate (execute) script */
      if(sc->cb)
	{
	  /* JavaScript/Lua/... */
	  result = sc->cb(interp, sc->script, AY_FALSE, &(sc->cscript));
	}
      else
	{
	  /* Tcl */
	  result = Tcl_EvalObjEx(interp, sc->cscript, TCL_EVAL_GLOBAL);
	}

      /* move newly created objects to script object */
      if(o->down && o->down->next)
	{
	  sc->cm_objects = o->down;
	  down = o->down;
	  while(down && down->next)
	    {
	      last = down;
	      down = down->next;
	    }
	  o->down = down;
	  last->next = ay_endlevel;
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

      /* Do we have atleast one child? */
      if(o->down && o->down->next)
	{ /* Yes */

	  old_sel = ay_selection;
	  ay_selection = NULL;

	  /* move original children into safety (sc->cm_objects);
	     create working copies of them in o->down and
	     select them while copying */
	  sc->cm_objects = o->down;
	  o->down = NULL;
	  down = sc->cm_objects;
	  nexto = &(o->down);
	  while(down)
	    {
	      if(down->type != AY_IDSCRIPT)
		{
		  ay_status = ay_object_copy(down, nexto);
		  if(ay_status)
		    {
		      ay_error(AY_ERROR, fname, "object copy failed");
		      break;
		    }

		  ay_notify_object(*nexto);

		  if(down->next)
		    ay_sel_add(*nexto);

		  nexto = &((*nexto)->next);
		}
	      else
		{
		  /* special case for script objects that are
		     children (spare the user the convOb -inplace) */
		  dsc = (ay_script_object*)down->refine;
		  if(dsc->cm_objects)
		    {
		      ddown = dsc->cm_objects;
		      while(ddown && ddown->next)
			{
			  ay_status = ay_object_copy(ddown, nexto);
			  if(ay_status)
			    {
			      ay_error(AY_ERROR, fname, "object copy failed");
			      break;
			    }

			  ay_notify_object(*nexto);

			  ay_sel_add(*nexto);

			  nexto = &((*nexto)->next);
			  ddown = ddown->next;
			} /* while */
		    } /* if */
		} /* if */

	      down = down->next;
	    } /* while */

	  ay_clevel_del();
	  ay_status = ay_clevel_add(o->down);

	  if(!ay_status)
	    {
	      /* evaluate (execute) script */
	      if(sc->cb)
		{
		  /* JavaScript/Lua/... */
		  result = sc->cb(interp, sc->script, AY_FALSE,
				  &(sc->cscript));
		}
	      else
		{
		  /* Tcl */
		  result = Tcl_EvalObjEx(interp, sc->cscript, TCL_EVAL_GLOBAL);
		}
	    }

	  /* call notification of modified objects */
	  sel = ay_selection;
	  while(sel)
	    {
	      if(sel->object->modified)
		{
		  ay_notify_object(sel->object);
		}
	      sel = sel->next;
	    }

	  /* restore old selection */
	  while(ay_selection)
	    {
	      l = ay_selection->next;
	      free(ay_selection);
	      ay_selection = l;
	    } /* while */
	  ay_selection = old_sel;

	  /* exchange modified objects with saved originals */
	  ccm_objects = sc->cm_objects;
	  sc->cm_objects = o->down;
	  o->down = ccm_objects;
	} /* if */
    } /* if */

  Tk_RestrictEvents(NULL, NULL, &old_restrictcd);

resenv:

  if(ay_currentview)
    {
      ay_currentview->redraw = old_rdmode;
    }

  /* restore the environment */
  ay_clevel_delall();
  free(ay_currentlevel);
  ay_currentlevel = old_currentlevel;

  if(ay_clipboard)
    {
      ay_object_deletemulti(ay_clipboard);
    }

  ay_clipboard = old_clipboard;

  ay_next = old_aynext;

  /* report failed scripts to the user */
  if(result == TCL_ERROR)
    {
      i = interp->errorLine;
      sprintf(buf, "Script failed in line: %d", i);
      ay_error(AY_ERROR, fname, buf);

      /* paint the error line red */
      Tcl_DStringInit(&ds);
      Tcl_DStringAppend(&ds, "set _w $ay(pca).fScriptAttr.tScript;", -1);
      Tcl_DStringAppend(&ds, "$_w tag add errtag ", -1);
      sprintf(buf, "%d", i);
      Tcl_DStringAppend(&ds, buf, -1);
      Tcl_DStringAppend(&ds, ".0 ", -1);
      Tcl_DStringAppend(&ds, buf, -1);
      Tcl_DStringAppend(&ds, ".end;", -1);
      Tcl_DStringAppend(&ds, "$_w tag configure errtag -background red;", -1);
      Tcl_Eval(ay_interp, Tcl_DStringValue(&ds));
      Tcl_DStringFree(&ds);

      /* get and output the error line
	 for more exact error location */
      l1 = sc->script;
      i--;
      while(l1 && i)
	{
	  l1 = strchr(l1,'\n')+1;
	  i--;
	}
      if(l1)
	{
	  l2 = strchr(l1,'\n');
	  if(l2)
	    {
	      *l2 = '\0';
	      ay_error(AY_ERROR, fname, l1);
	      *l2 = '\n';
	    }
	  else
	    {
	      ay_error(AY_ERROR, fname, l1);
	    }
	} /* if */
    } /* if */

  /* manage read only points */
  if(sc->pntslen)
    {
      if(sc->pntslen && sc->cm_objects)
	{
	  down = sc->cm_objects;
	  sc->pntslen = 0;
	  /* iterate over all cm_objects and transform/copy
	     their points according to the respective trafos
	     into a big points vector (built up dynamically
	     using realloc()) */
	  while(down)
	    {
	      ay_status = ay_pact_getpoint(0, down, dummy, &pe);

	      if(!ay_status && pe.num)
		{

		  sc->pntslen += pe.num;

		  p1 = realloc(sc->pnts,
			       sc->pntslen*4*sizeof(double));

		  if(p1)
		    {
		      sc->pnts = p1;

		      ay_trafo_creatematrix(down, m);
		      if(pe.rational)
			{
			  for(j = 0; j < pe.num; j++)
			    {
			      p1 = &(sc->pnts[a]);
			      p2 = pe.coords[j];
			      AY_APTRAN3(p1, p2, m);
			      p1[3] = pe.coords[j][3];
			      a += 4;
			    } /* for */
			}
		      else
			{
			  for(j = 0; j < pe.num; j++)
			    {
			      p1 = &(sc->pnts[a]);
			      p2 = pe.coords[j];
			      AY_APTRAN3(p1, p2, m);
			      p1[3] = 1.0;
			      a += 4;
			    } /* for */
			} /* if */
		    }
		  else
		    {
		      /* realloc() failed! */
		      ay_pact_clearpointedit(&pe);
		      free(sc->pnts);
		      sc->pnts = NULL;
		      break;
		    } /* if */

		} /* if */

	      ay_pact_clearpointedit(&pe);

	      down = down->next;
	    } /* while */
	} /* if */
    } /* if */

cleanup:

  /* correct any inconsistent values of pnts and pntslen */
  if(sc->pntslen && !sc->pnts)
    {
      sc->pntslen = 0;
    }

  /* recover selected points */
  if(o->selp)
    {
      ay_script_getpntcb(3, o, NULL, NULL);
    }

  lock = 0;

 return ay_status;
} /* ay_script_notifycb */


/* ay_script_convertcb:
 *  convert callback function of script object
 */
int
ay_script_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 ay_tag *tag = NULL;
 ay_script_object *sc = NULL;
 ay_level_object *newl = NULL;
 ay_object *cmo = NULL, *new = NULL;

  if(!o)
    return AY_ENULL;

  sc = (ay_script_object *)(o->refine);

  if(sc->cm_objects)
    ay_sel_clearselflag(sc->cm_objects);

  switch(sc->type)
    {
    case 1:
    case 2:
      if(sc->cm_objects)
	{
	  if(in_place)
	    {
	      if(sc->cm_objects->next && sc->cm_objects->next->next)
		{
		  /* have multiple objects */
		  if(!(newl = calloc(1, sizeof(ay_level_object))))
		    return AY_EOMEM;
		  o->type = AY_IDLEVEL;
		  newl->type = AY_LTLEVEL;

		  /* remove current children (of the script object) */
		  ay_status = ay_object_deletemulti(o->down);

		  /* XXXX check and report errors? */

		  /* link created/modified objects as new
		     children to the new level object */
		  o->down = sc->cm_objects;
		  sc->cm_objects = NULL;

		  /* now free the script object */
		  ay_script_deletecb(sc);

		  /* complete transmogrification of o */
		  o->refine = newl;
		}
	      else
		{
		  /* have just one object */
		  cmo = sc->cm_objects;
		  o->type = cmo->type;
		  
		  sc->cm_objects = NULL;

		  /* now free the script object */
		  ay_script_deletecb(sc);

		  /* complete transmogrification of o */
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
	      while(cmo && cmo->next)
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


/* ay_script_providecb:
 *  provide callback function of script object
 */
int
ay_script_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 ay_script_object *sc = NULL;
 ay_object *cmo = NULL, *po = NULL, **npo = NULL;

  if(!o)
    return AY_ENULL;

  sc = (ay_script_object *)(o->refine);

  if(!result)
    {
      switch(sc->type)
	{
	case 1:
	case 2:
	  if(sc->cm_objects)
	    {
	      cmo = sc->cm_objects;
	      while(cmo && cmo->next)
		{
		  if(cmo->type != type)
		    {
		      ay_status = ay_provide_object(cmo, type, NULL);

		      if(ay_status == AY_OK)
			{
			  return AY_OK;
			}
		      /* it is sufficient if we can provide atleast
			 one object of wanted type, thus we do not
			 give up here immediately... */
		    }
		  else
		    {
		      return AY_OK;
		    }

		  cmo = cmo->next;
		} /* while */
	    } /* if */
	  break;
	default:
	  break;
	} /* switch */

      return AY_ERROR;
    } /* if */

  if(sc->cm_objects)
    ay_sel_clearselflag(sc->cm_objects);

  switch(sc->type)
    {
    case 1:
    case 2:
      if(sc->cm_objects)
	{
	  npo = &po;
	  cmo = sc->cm_objects;
	  while(cmo && cmo->next)
	    {
	      if(cmo->type != type)
		{
		  ay_status = ay_provide_object(cmo, type, npo);
		}
	      else
		{
		  ay_status = ay_object_copy(cmo, npo);
		}

	      if(!ay_status && *npo)
		{
		  while(*npo)
		    npo = &((*npo)->next);
		}
	      cmo = cmo->next;
	    } /* while */

	  if(result)
	    *result = po;
	} /* if */
      break;
    default:
      break;
    } /* switch */

 return AY_OK;
} /* ay_script_providecb */


/* ay_script_hasnptrafo:
 *  has object <o> a NP tag for the transformations property?
 */
int
ay_script_hasnptrafo(ay_object *o)
{
 ay_tag *t = NULL;

  if(!o)
    return AY_FALSE;

  /* find NP tag */
  t = o->tags;
  while(t)
    {
      if(t->type == ay_np_tagtype)
	{
	  if(t->val && strstr(t->val, "Transformations"))
	    {
	      return AY_TRUE;
	    }
	}
      t = t->next;
    }

 return AY_FALSE;
} /* ay_script_hasnptrafo */


/* ay_script_init:
 *  initialize the script object module
 */
int
ay_script_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;

  ay_status = ay_otype_registercore(ay_script_name,
				    ay_script_createcb,
				    ay_script_deletecb,
				    ay_script_copycb,
				    ay_script_drawcb,
				    ay_script_drawhcb,
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

  /* script objects may not be associated with materials */
  ay_matt_nomaterial(AY_IDSCRIPT);

 return ay_status;
} /* ay_script_init */
