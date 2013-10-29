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

/* read.c - read scenes */

/* ay_read_string:
 *
 */
int
ay_read_string(FILE *fileptr, char **result)
{
 char readchar, *str;
 int read;
 Tcl_DString ds;

  Tcl_DStringInit(&ds);

  do
    {
      read = getc(fileptr);

      if(read == EOF)
	{Tcl_DStringFree(&ds); return AY_EUEOF;}

      readchar = (char)read;

      Tcl_DStringAppend(&ds, &readchar, 1);
    }
  while((char)read != '\n');

  if(Tcl_DStringLength(&ds) <= 1)
    {Tcl_DStringFree(&ds); return AY_OK;}

  str = Tcl_DStringValue(&ds);
  str[Tcl_DStringLength(&ds)-1] = '\0';

  if(!(*result = malloc((Tcl_DStringLength(&ds)+1)*sizeof(char))))
    {Tcl_DStringFree(&ds); return AY_EOMEM;}

  strcpy(*result, str);

  Tcl_DStringFree(&ds);

 return AY_OK;
} /* ay_read_string */


/* ay_read_unistring:
 *
 */
int
ay_read_unistring(FILE *fileptr, Tcl_UniChar **result)
{
 int ay_status = AY_OK;
 char readchar;
 int read, uc = 0, i = 0;
 Tcl_DString ds;
 Tcl_UniChar *unistring = NULL, *unistringtmp;

  Tcl_DStringInit(&ds);
  read = getc(fileptr);

  if(read == EOF)
    {Tcl_DStringFree(&ds); return AY_EUEOF;}

  if((char)read == '\n')
    {Tcl_DStringFree(&ds); return AY_OK;}

  while((char)read != '\n')
    {
      readchar = (char)read;

      Tcl_DStringAppend(&ds, &readchar, 1);

      if(read == ' ')
	{
	  sscanf(Tcl_DStringValue(&ds), "%d", &uc);

	  if(!(unistringtmp = realloc(unistring, (i+1)*sizeof(Tcl_UniChar))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  unistring = unistringtmp;

	  unistring[i] = (Tcl_UniChar)uc;
	  i++;
	  Tcl_DStringFree(&ds);
	  Tcl_DStringInit(&ds);
	}

      read = getc(fileptr);

      if(read == EOF)
	{ ay_status = AY_EUEOF; goto cleanup; }

    } /* while */

  /* terminate unistring */
  if(!(unistringtmp = realloc(unistring, (i+1)*sizeof(Tcl_UniChar))))
    { ay_status = AY_EOMEM; goto cleanup; }

  unistring = unistringtmp;
  unistring[i] = (Tcl_UniChar)0;
  *result = unistring;
  unistring = NULL;

cleanup:

  if(unistring)
    free(unistring);
  Tcl_DStringFree(&ds);

 return ay_status;
} /* ay_read_unistring */


/* ay_read_skip:
 *  skip to next entity, marked by a single bell, or two bell's!
 */
int
ay_read_skip(FILE *fileptr)
{
 int read;

  do
    {
      read = getc(fileptr);
      if(read == EOF)
	return AY_EEOF;
    }
  while((char)read != '\a');

 return AY_OK;
} /* ay_read_skip */


/* ay_read_header:
 *
 */
int
ay_read_header(FILE *fileptr)
{
 int ay_status = AY_OK;
 char *version = NULL, *nbuffer = NULL;
 int version_unknown = AY_TRUE;
 char fname[] = "read_header";

  ay_status = ay_read_string(fileptr, &nbuffer);
  if(ay_status)
    return ay_status;

  if(!strstr(nbuffer, "Ayam"))
    {
      free(nbuffer);
      return AY_EFORMAT;
    }

  ay_status = ay_read_string(fileptr, &version);

  if(ay_status)
    {
      free(nbuffer);
      return ay_status;
    }

  ay_read_version = 1;

  if(!strcmp(version,"1.0"))
    {
      ay_read_version = 0;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.1"))
    {
      ay_read_version = 1;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.2"))
    {
      ay_read_version = 2;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.3"))
    {
      ay_read_version = 3;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.4"))
    {
      ay_read_version = 4;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.5"))
    {
      ay_read_version = 4;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.6"))
    {
      ay_read_version = 5;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.7"))
    {
      ay_read_version = 6;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.8"))
    {
      ay_read_version = 7;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.9"))
    {
      ay_read_version = 8;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.10"))
    {
      ay_read_version = 9;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.11"))
    {
      ay_read_version = 9;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.12"))
    {
      ay_read_version = 9;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.13"))
    {
      ay_read_version = 10;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.14"))
    {
      ay_read_version = 11;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.15"))
    {
      ay_read_version = 12;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.16"))
    {
      ay_read_version = 13;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.17"))
    {
      ay_read_version = 13;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.18"))
    {
      ay_read_version = 13;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.19"))
    {
      ay_read_version = 14;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.20"))
    {
      ay_read_version = 15;
      version_unknown = AY_FALSE;
    }

  if(!strcmp(version,"1.21"))
    {
      ay_read_version = 16;
      version_unknown = AY_FALSE;
    }

  if(version_unknown)
    {
      ay_error(AY_EWARN, fname, "Unknown file version, reading as V1.0!");
    }

  free(version);
  free(nbuffer);

 return ay_status;
} /* ay_read_header */


/* ay_read_attributes:
 *
 */
int
ay_read_attributes(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 int trafos = 0, read = 0;

  if(!o)
    return AY_ENULL;

  fscanf(fileptr,"%d\n", &trafos);

  if(trafos)
    {
      fscanf(fileptr,"%lg\n",&o->movx);
      fscanf(fileptr,"%lg\n",&o->movy);
      fscanf(fileptr,"%lg\n",&o->movz);

      fscanf(fileptr,"%lg\n",&o->rotx);
      fscanf(fileptr,"%lg\n",&o->roty);
      fscanf(fileptr,"%lg\n",&o->rotz);

      fscanf(fileptr,"%lg\n",&o->quat[0]);
      fscanf(fileptr,"%lg\n",&o->quat[1]);
      fscanf(fileptr,"%lg\n",&o->quat[2]);
      fscanf(fileptr,"%lg\n",&o->quat[3]);

      fscanf(fileptr,"%lg\n",&o->scalx);
      fscanf(fileptr,"%lg\n",&o->scaly);
      fscanf(fileptr,"%lg\n",&o->scalz);

      ay_quat_norm(o->quat);
    }

  fscanf(fileptr,"%d\n",&o->parent);
  fscanf(fileptr,"%d\n",&o->inherit_trafos);
  fscanf(fileptr,"%d\n",&o->hide);
  fscanf(fileptr,"%d",&o->hide_children);
  read = fgetc(fileptr);
  if(read == EOF)
    return AY_ERROR;

  ay_status = ay_read_string(fileptr, &(o->name));

 return ay_status;
} /* ay_read_attributes */


/* ay_read_tags:
 *
 */
int
ay_read_tags(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_tag *tag = NULL, **next = NULL;
 Tcl_HashEntry *entry = NULL;
 int tcount = 0, i = 0;
 char fname[] = "read_tags";
#ifdef AYNOSAFEINTERP
 int deactivate = 0;
 char *tc = NULL;
 char a1[] = "ay", n1[] = "scriptdisable";
 char script_disable_cmd[] = "script_disable";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
#endif

  if(!o)
    return AY_ENULL;

  fscanf(fileptr, "%d\n", &tcount);

  next = &(o->tags);

  for(i = 0; i < tcount; i++)
    {
      if(!(tag = calloc(1, sizeof(ay_tag))))
	return AY_EOMEM;

      ay_status = ay_read_string(fileptr, &(tag->name));

      if(ay_status)
	{ free(tag); return ay_status; }

      if(!(entry = Tcl_FindHashEntry(&ay_tagtypesht, tag->name)))
	{
	  if(ay_prefs.wutag)
	    ay_error(AY_EWARN, fname, "Tag type is not registered!");
	}
      else
	{
	  tag->type = (char *)Tcl_GetHashValue(entry);
	}

     ay_status = ay_read_string(fileptr, (char**)(void*)&(tag->val));

      if(ay_status)
	{ free(tag->name); free(tag); return ay_status; }

      /* avoid null value */
      if(!tag->val)
	{
	  if(!(tag->val = calloc(1, sizeof(char))))
	    { free(tag->name); free(tag); return AY_EOMEM; }
	}

      *next = tag;
      next = &(tag->next);

#ifdef AYNOSAFEINTERP
     /* if there is no safe interpreter, disable all ANS/BNS tags
	by manipulating their type */
     if(tag->type == ay_bns_tagtype)
       {
	 Tcl_Eval(ay_interp, script_disable_cmd);
	 toa = Tcl_NewStringObj(a1, -1);
	 ton = Tcl_NewStringObj(n1, -1);
	 to = Tcl_ObjGetVar2(ay_interp, toa, ton,
			     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	 Tcl_GetIntFromObj(ay_interp, to, &(deactivate));

	 if(deactivate)
	   {
	     tag->type = ay_dbns_tagtype;
	     tc = NULL;
	     tc = realloc(tag->name, (strlen(ay_dbns_tagname)+1)*sizeof(char));
	     if(tc)
	       {
		 tag->name = tc;
		 strcpy(tag->name, ay_dbns_tagname);
	       }
	   }

	 Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
	 Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
       } /* if */
     if(tag->type == ay_ans_tagtype)
       {
	 Tcl_Eval(ay_interp, script_disable_cmd);
	 toa = Tcl_NewStringObj(a1, -1);
	 ton = Tcl_NewStringObj(n1, -1);
	 to = Tcl_ObjGetVar2(ay_interp, toa, ton,
			     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	 Tcl_GetIntFromObj(ay_interp, to, &(deactivate));

	 if(deactivate)
	   {
	     tag->type = ay_dans_tagtype;
	     tc = NULL;
	     tc = realloc(tag->name, (strlen(ay_dans_tagname)+1)*sizeof(char));
	     if(tc)
	       {
		 tag->name = tc;
		 strcpy(tag->name, ay_dans_tagname);
	       }
	   }

	 Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
	 Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
       } /* if */
#endif /* AYNOSAFEINTERP */
    } /* for */

 return ay_status;
} /* ay_read_tags */


/* ay_read_shader:
 *
 */
int
ay_read_shader(FILE *fileptr, ay_shader **result)
{
 int ay_status = AY_OK;
 int argcount = 0, i = 0, j = 0;
 ay_shader *shader = NULL;
 ay_shader_arg *sarg = NULL, **prevargptr = NULL;
 int read;

  if(!result)
    return AY_ENULL;

  if(!(shader = calloc(1, sizeof(ay_shader))))
   return AY_EOMEM;

  ay_status = ay_read_string(fileptr, &(shader->name));

  if(ay_status)
    { free(shader); return ay_status; }

  fscanf(fileptr,"%d\n", &(shader->type));
  fscanf(fileptr,"%d\n", &argcount);

  prevargptr = &(shader->arg);

  for(j = 0; j < argcount; j++)
    {
      if(!(sarg = calloc(1, sizeof(ay_shader_arg))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      ay_status = ay_read_string(fileptr, &(sarg->name));
      if(ay_status)
	{ free(sarg); goto cleanup;}

      fscanf(fileptr,"%d", &(sarg->type));
      read = fgetc(fileptr);
      if(read == EOF)
	{ free(sarg->name); free(sarg); ay_status = AY_ERROR; goto cleanup; }

      switch(sarg->type)
	{
	case AY_SACOLOR:
	  fscanf(fileptr,"%g\n", &((sarg->val).color[0]));
	  fscanf(fileptr,"%g\n", &((sarg->val).color[1]));
	  fscanf(fileptr,"%g\n", &((sarg->val).color[2]));
	  break;
	case AY_SAVECTOR:
	case AY_SANORMAL:
	case AY_SAPOINT:
	  fscanf(fileptr,"%g\n", &((sarg->val).point[0]));
	  fscanf(fileptr,"%g\n", &((sarg->val).point[1]));
	  fscanf(fileptr,"%g\n", &((sarg->val).point[2]));
	  break;
	case AY_SASCALAR:
	  fscanf(fileptr,"%g\n", &((sarg->val).scalar));
	  break;
	case AY_SASTRING:
	  ay_status = ay_read_string(fileptr, &((sarg->val).string));
	  if(ay_status)
	    { free(sarg->name); free(sarg); goto cleanup; }
	  break;
	case AY_SAMATRIX:
	  for(i = 0; i < 15; i++)
	    {
	      fscanf(fileptr,"%g ", &((sarg->val).matrix[i]));
	    }
	  fscanf(fileptr,"%g\n", &((sarg->val).matrix[15]));
	  break;
	} /* switch */

      *prevargptr = sarg;
      prevargptr = &(sarg->next);
    } /* for */

  *result = shader;

  shader = NULL;

cleanup:

  if(shader)
    ay_shader_free(shader);

 return ay_status;
} /* ay_read_shader */


/* ay_read_object:
 *
 */
int
ay_read_object(FILE *fileptr)
{
 char fname[] = "read_object";
 int ay_status = AY_OK;
 int type = 0, has_typename = 0, has_child = 0, read = 0;
 ay_object *o = NULL;
 ay_tag tag = {0};
 ay_voidfp *arr = NULL;
 ay_readcb *cb = NULL;
 char *typename = NULL;
 Tcl_HashEntry *entry = NULL;
 Tcl_DString ds;

  if(feof(fileptr))
    return AY_EEOF;

  /* skip to next object */
  ay_status = ay_read_skip(fileptr);

  if(ay_status)
    return ay_status;

  if(!(o = calloc(1, sizeof(ay_object))))
    return AY_EOMEM;

  ay_object_defaults(o);

  /* get type of object */
  fscanf(fileptr, "%d", &has_typename);
  read = fgetc(fileptr);
  if(read == EOF)
    return AY_ERROR;

  if(has_typename)
    {
      /* get type name */
      ay_status = ay_read_string(fileptr, &(typename));
      if(ay_status)
	{ ay_object_delete(o); return ay_status; }

      /* is the type name registered? */
      if((entry = Tcl_FindHashEntry(&ay_otypesht, typename)))
	{
	  /* yes */
	  /* get type id of name */
	  type = (int) Tcl_GetHashValue(entry);
	}
      else
	{
	  /* no */
	  /* try to autoload the custom object/plugin */
	  Tcl_DStringInit(&ds);
	  Tcl_DStringAppend(&ds, "loadPlugin ", -1);
	  Tcl_DStringAppend(&ds, typename, -1);
	  Tcl_Eval(ay_interp, Tcl_DStringValue(&ds));
	  Tcl_DStringFree(&ds);

	  /* check again whether type name is registered */
	  if((entry = Tcl_FindHashEntry(&ay_otypesht, typename)))
	    {
	      /* yes */
	      /* get type id of name */
	      type = (int) Tcl_GetHashValue(entry);
	    }
	  else
	    {
	      ay_error(AY_ENTYPE, fname, typename);
	      free(typename);
	      ay_object_delete(o);
	      return AY_OK;
	    } /* if */
	} /* if */
      free(typename);
    }
  else
    {
      fscanf(fileptr, "%d\n", &type);
    } /* if */

  o->type = type;

  if(ay_read_version > 0)
    {
      fscanf(fileptr, "%d\n", &has_child);
    }

  ay_status = ay_read_attributes(fileptr, o);
  if(ay_status)
    { ay_object_delete(o); return ay_status; }

  ay_status = ay_read_tags(fileptr, o);
  if(ay_status)
    { ay_object_delete(o); return ay_status; }

  /* inform object to read that there follow children;
     this, currently, is only interesting for views */
  if(has_child)
    {
      tag.type = ay_hc_tagtype;
      tag.next = o->tags;
      o->tags = &tag;
    }

  /* get and execute read callback */
  arr = ay_readcbt.arr;
  cb = (ay_readcb *)(arr[type]);
  if(cb)
    ay_status = cb(fileptr, o);

  /* restore tags */
  if(has_child)
    {
      o->tags = tag.next;
    }

  if(ay_status)
    {
      if(ay_status == AY_EDONOTLINK)
	{
	  ay_object_delete(o);
	  return AY_OK;
	}
      else
	{
	  ay_error(ay_status, fname, "read callback failed");
	  ay_object_delete(o);
	  return ay_status;
	} /* if */
    } /* if */

  if(o->parent && (!o->down))
    {
      o->down = ay_endlevel;
    }

  ay_object_link(o);

  if(ay_read_version == 0)
    {
      fscanf(fileptr, "%d\n", &has_child);
    }

  if(has_child)
    {
      /* go down */
      ay_clevel_add(o);
      ay_clevel_add(o->down);
      ay_next = &(o->down);
    }

 return ay_status;
} /* ay_read_object */


/* ay_read_scene:
 *
 */
int
ay_read_scene(Tcl_Interp *interp, char *filename, int insert)
{
 int ay_status = AY_OK;
 FILE *fileptr = NULL;
 char *fname = "read_scene";
 ay_object *o = NULL;
 int old_save_rv = ay_prefs.save_rootviews;

  if(!insert)
    {
      ay_read_viewnum = 0;
    }
  else
    {
      ay_read_viewnum = 3;
    }

  if(insert)
    {
      ay_instt_clearoidtags(ay_root->next);
    }

  /* reading a root object will set the save_rootviews state to AY_TRUE */
  ay_prefs.save_rootviews = AY_FALSE;

  if(!(fileptr = fopen(filename,"rb")))
    {
      ay_error(AY_EOPENFILE, fname, filename);
      ay_prefs.save_rootviews = old_save_rv;
      return AY_ERROR;
    }

  ay_status = ay_read_header(fileptr);

  if(ay_status)
    {
      fclose(fileptr);
      ay_error(ay_status, fname, filename);
      ay_prefs.save_rootviews = old_save_rv;
      return AY_ERROR;
    }

  while(!ay_status)
    {
      ay_status = ay_read_object(fileptr);
      if(ay_status)
	{
	  if(ay_status != AY_EEOF)
	    {
	      /* issue errmsg */
	      ay_error(ay_status, fname, NULL);
	      if(ay_prefs.onerror)
		{
		  ay_status = AY_OK;
		} /* if */
	    } /* if */
	} /* if */

    } /* while */

  if(ay_status == AY_EEOF)
    ay_status = AY_OK;

  fclose(fileptr);

  /* link instance objects to their originals */
  /* clear hashtable of oid ptrs */
  ay_instt_createoidht(NULL);
  /* build new hashtable */
  ay_instt_createoidht(ay_root->next);
  /* link instance objects to their originals */
  ay_instt_connect(ay_root->next, &(ay_root->next));

  /* connect objects to their materials */
  ay_matt_connect(ay_root);

  /* clear Material ID tags from scene */
  ay_matt_clearmaterialids(ay_root);

  /* force rebuild of all objects, that rely on children */
  o = ay_root;
  while(o)
    {
      ay_notify_object(o);
      o = o->next;
    }

  /* inserting files does not change the save_rootviews state */
  if(insert)
    {
      ay_prefs.save_rootviews = old_save_rv;
    }

 return ay_status;
} /* ay_read_scene */


/* ay_read_replacetcmd:
 *
 */
int
ay_read_replacetcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;

  /* check args */
  if(argc != 2)
    {
      ay_error(AY_EARGS, argv[0], "filename");
      return TCL_OK;
    }

  /* first, we completely clear the old scene */
  ay_status = ay_clear_scene();
  if(ay_status)
    {
      ay_error(ay_status, argv[0], "Could not clear scene!");
      return TCL_OK;
    }

  /* then, we read the new scene */
  ay_status = ay_read_scene(interp, argv[1], AY_FALSE);

  if(ay_status)
    {
      ay_error(ay_status, argv[0], "Error reading file!");
      return TCL_OK;
    }

 return TCL_OK;
} /* ay_read_replacetcmd */


/* ay_read_inserttcmd:
 *
 */
int
ay_read_inserttcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;

  /* check args */
  if(argc != 2)
    {
      ay_error(AY_EARGS, argv[0], "filename");
      return TCL_OK;
    }

  ay_status = ay_read_scene(interp, argv[1], AY_TRUE);

  if(ay_status)
    {
      ay_error(ay_status, argv[0], "Error reading file!");
      return TCL_OK;
    }

 return TCL_OK;
} /* ay_read_inserttcmd */
