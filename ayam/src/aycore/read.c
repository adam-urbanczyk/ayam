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

/* read.c - read scenes */

/* ay_read_string:
 *
 */
int
ay_read_string(FILE *fileptr, char **result)
{
 int ay_status = AY_OK;
 char readchar, *str;
 int read;
 Tcl_DString ds;

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
      read = getc(fileptr);

      if(read == EOF)
	{Tcl_DStringFree(&ds); return AY_EUEOF;}

    }

  Tcl_DStringAppend(&ds,"\0", 1);

  str = Tcl_DStringValue(&ds);

  if(str[strlen(str)-1] == '\r')
    {
      str[strlen(str)-1] = '\0';
    }

  if(!(*result = calloc(strlen(str)+1,sizeof(char))))
    {Tcl_DStringFree(&ds); return AY_EOMEM;}

  strcpy(*result,str);

  Tcl_DStringFree(&ds);

 return ay_status;
} /* ay_read_string */


/* ay_read_skip:
 *  skip to next entity, marked by a single bell, or two bell's!
 */
int
ay_read_skip(FILE *fileptr)
{
 int ay_status = AY_OK;
 int read;

  read = getc(fileptr);

  if(read == EOF)
    {return AY_EEOF;}

  if((char)read == '\a')
    {return AY_OK;}

  while((char)read != '\a')
    {
      read = getc(fileptr);

      if(read == EOF)
	return AY_EEOF;
    }

 return ay_status;
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
 char fname[] = "ay_read_header";

  ay_status = ay_read_string(fileptr, &nbuffer);
  if(ay_status)
    return ay_status;

  if(!strstr(nbuffer, "Ayam"))
    {
      free(nbuffer);
      return AY_EFORMAT;
    }

  ay_read_string(fileptr, &version);

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
    }

  fscanf(fileptr,"%d\n",&o->parent);
  fscanf(fileptr,"%d\n",&o->inherit_trafos);
  fscanf(fileptr,"%d\n",&o->hide);
  fscanf(fileptr,"%d",&o->hide_children);
  read = fgetc(fileptr);
  ay_read_string(fileptr, &(o->name));

 return ay_status;
} /* ay_read_attributes */


/* ay_read_tags:
 *  
 */
int 
ay_read_tags(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_tag_object *tag = NULL, *last = NULL;
 Tcl_HashEntry *entry = NULL;
 int tcount = 0, i = 0;
 char fname[] = "ay_read_tags";
 
  if(!o)
    return AY_ENULL;

 fscanf(fileptr,"%d\n",&tcount);

 for(i=0;i<tcount;i++)
   {
     tag = NULL;
     if(!(tag = calloc(1, sizeof(ay_tag_object))))
       return AY_EOMEM;

     ay_read_string(fileptr,&(tag->name));

     if(!(entry = Tcl_FindHashEntry(&ay_tagtypesht, tag->name)))
       {
	 ay_error(AY_EWARN, fname, "Tag type is not registered!");
       }

     if(entry)
       tag->type = (char *)Tcl_GetHashValue(entry);

     ay_read_string(fileptr, &(tag->val));

     /* do not create tags with NULL pointers */
     if(!tag->val)
       {
	 if(!(tag->val = calloc(1, sizeof(char))))
	   return AY_EOMEM;
	 *(tag->val) = '\0';
       }

     if(!o->tags)
       {
	 o->tags = tag;
       }
     else
       {
	 last->next = tag;
       }

     last = tag;
   } /* for */

 return ay_status;
} /* ay_read_tags */


/* ay_read_shader:
 *  XXXX leaks memory in low-mem situation
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

  shader = NULL;
  if(!(shader = calloc(1, sizeof(ay_shader))))
   return AY_EOMEM;

  ay_read_string(fileptr,&(shader->name));

  fscanf(fileptr,"%d\n", &(shader->type));
  fscanf(fileptr,"%d\n", &argcount);

  prevargptr = &(shader->arg);

  for(j=0; j < argcount; j++)
    {
      if(!(sarg = calloc(1, sizeof(ay_shader_arg))))
	return AY_EOMEM; 

      ay_read_string(fileptr, &(sarg->name));
      fscanf(fileptr,"%d", &(sarg->type));
      read = fgetc(fileptr);
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
	  ay_read_string(fileptr, &((sarg->val).string));
	  if(!((sarg->val).string))
	    {
	      if(!((sarg->val).string = calloc(1, sizeof(char))))
		return AY_EOMEM;
	    }
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
 int type = 0, has_name = 0, has_child = 0, read = 0;
 ay_object *o = NULL;
 void **arr = NULL;
 ay_readcb *cb = NULL;
 char *type_name = NULL, err[255], autoload[] = "io_lcAuto";
 Tcl_HashEntry *entry = NULL;

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
  fscanf(fileptr,"%d", &has_name);
  read = fgetc(fileptr);
  if(has_name)
    {
      /* get name */
      ay_read_string(fileptr, &(type_name));

      /* get type_id of name */
      if((entry = Tcl_FindHashEntry(&ay_otypesht, type_name)))
	{
	  type = (int) Tcl_GetHashValue(entry);
	}
      else
	{

	  /* try to autoload custom object */
	  Tcl_SetVar2(ay_interp, "ay", "autoload", type_name,
		      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

	  Tcl_Eval(ay_interp, autoload);


	  if((entry = Tcl_FindHashEntry(&ay_otypesht, type_name)))
	    {
	      type = (int) Tcl_GetHashValue(entry);
	    }
	  else
	    {
	      sprintf(err, "Unknown object type: %s", type_name);
	      ay_error(AY_ERROR, fname, err);
	      free(type_name);
	      free(o);
	      return AY_OK;
	    } /* if */
	} /* if */
      free(type_name);
    }
  else
    {
      fscanf(fileptr,"%d\n", &type);
    } /* if */


  o->type = type;

  if(ay_read_version > 0)
    {
      fscanf(fileptr, "%d\n", &has_child);
    }

  ay_status = ay_read_attributes(fileptr, o);

  ay_status = ay_read_tags(fileptr, o);


  arr = ay_readcbt.arr;
  cb = (ay_readcb *)(arr[type]);
  if(cb)
    ay_status = cb(fileptr, o);

  if(ay_status)
    {
      if(ay_status == AY_EDONOTLINK)
	{
	  if(o->name)
	    free(o->name);
	  if(o->tags)
	    ay_tags_delall(o);
	  free(o); o = NULL;
	  return AY_OK;
	}
      else
	{
	  ay_error(ay_status, fname, "read callback failed");
	  if(o->name)
	    free(o->name);
	  if(o->tags)
	    ay_tags_delall(o);
	  free(o); o = NULL;
	  return ay_status;
	} /* if */
    } /* if */

  if(o)
    {
      if(o->parent && (!o->down))
	{
	  ay_status = ay_object_crtendlevel(&(o->down));
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname,
           "Could not create terminating level object, scene is corrupt now!");
	    }
	}

      ay_status = ay_object_link(o);

    } /* if */

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

 if(insert)
   ay_instt_clearoidtags(ay_root->next);

  if(!(fileptr = fopen(filename,"rb")))
    {
      ay_error(AY_EOPENFILE, fname, filename);
      return AY_ERROR;
    }

  ay_status = ay_read_header(fileptr);

  if(ay_status)
    { 
      fclose(fileptr);
      ay_error(ay_status, fname, filename);
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
  /* build it */
  ay_instt_createoidht(ay_root->next);
  /* link instance objects to their originals */
  ay_instt_connect(ay_root->next, &(ay_root->next));
  /* force rebuild of all objects, that rely on children */
  o = ay_root->next;
  while(o)
    {
      ay_notify_force(o);
      o = o->next;
    }

  /* connect objects to their materials */
  ay_status = ay_matt_connect(ay_root);

  /* clear Material ID tags from scene */
  ay_status = ay_matt_clearmaterialids(ay_root);

 return ay_status;
} /* ay_read_scene */


/* ay_read_replacetcmd:
 *  
 */
int
ay_read_replacetcmd(ClientData clientData, Tcl_Interp * interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "replaceScene";

  /* check args */
  if(argc != 2)
    {
      ay_error(AY_EARGS, fname, "filename");
      return TCL_OK;
    }

  /* first, we completely clear the old scene */
  ay_status = ay_clear_scene();
  if(ay_status)
    {
      ay_error(ay_status, fname, "Could not clear scene!");
      return TCL_OK;
    }

  /* then, we read the new scene */
  ay_status = ay_read_scene(interp, argv[1], AY_FALSE);
  
  if(ay_status)
    {
      ay_error(ay_status, fname, "Error reading file!");
      return TCL_OK;
    }

 return TCL_OK;
} /* ay_read_replacetcmd */


/* ay_read_inserttcmd:
 *  
 */
int
ay_read_inserttcmd(ClientData clientData, Tcl_Interp * interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "insertScene";

  /* check args */
  if(argc != 2)
    {
      ay_error(AY_EARGS, fname, "filename");
      return TCL_OK;
    }

  ay_status = ay_read_scene(interp, argv[1], AY_TRUE);

  if(ay_status)
    {
      ay_error(ay_status, fname, "Error reading file!");
      return TCL_OK;
    }

 return TCL_OK;
} /* ay_read_inserttcmd */
