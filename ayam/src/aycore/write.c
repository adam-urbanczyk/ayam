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

/* write.c - write scenes */

/* ay_write_header:
 *
 */
int
ay_write_header(FILE *fileptr)
{
 int ay_status = AY_OK;

 fprintf(fileptr,"Ayam\n");
 fprintf(fileptr,"%s\n", AY_VERSIONSTR);

 return ay_status;
} /* ay_write_header */


/* ay_write_attributes:
 *
 */
int 
ay_write_attributes(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;

  if(!o)
    return AY_ENULL;

  if((o->movx != 0.0) || (o->movy != 0.0) || (o->movz != 0.0) ||
     (o->rotx != 0.0) || (o->roty != 0.0) || (o->rotz != 0.0) ||
     (o->scalx != 1.0) || (o->scaly != 1.0) || (o->scalz != 1.0) ||
     (o->quat[0] != 0.0) || (o->quat[1] != 0.0) || (o->quat[2] != 0.0) || 
     (o->quat[3] != 1.0))
    {
      fprintf(fileptr,"1\n");
      fprintf(fileptr,"%g\n",o->movx);
      fprintf(fileptr,"%g\n",o->movy);
      fprintf(fileptr,"%g\n",o->movz);

      fprintf(fileptr,"%g\n",o->rotx);
      fprintf(fileptr,"%g\n",o->roty);
      fprintf(fileptr,"%g\n",o->rotz);

      fprintf(fileptr,"%g\n",o->quat[0]);
      fprintf(fileptr,"%g\n",o->quat[1]);
      fprintf(fileptr,"%g\n",o->quat[2]);
      fprintf(fileptr,"%g\n",o->quat[3]);

      fprintf(fileptr,"%g\n",o->scalx);
      fprintf(fileptr,"%g\n",o->scaly);
      fprintf(fileptr,"%g\n",o->scalz);
    }
  else
    {
      fprintf(fileptr,"0\n");
    }

  fprintf(fileptr,"%d\n", o->parent);
  fprintf(fileptr,"%d\n", o->inherit_trafos);
  fprintf(fileptr,"%d\n", o->hide);
  fprintf(fileptr,"%d\n", o->hide_children);

  if(o->name)
    {
      fprintf(fileptr,"%s\n", o->name);
    }
  else
    {
      fprintf(fileptr,"\n");
    }

 return ay_status;
} /* ay_write_attributes */


/* ay_write_tags:
 *
 */
int 
ay_write_tags(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_tag_object *tag = NULL;
 int tcount = 0;
 int temp = 0;

  if(!o)
    return AY_ENULL;

  /* count tags */
  tag = o->tags;
  while(tag)
    {
      ay_status = ay_tags_temp(ay_interp, tag->name, 0, &temp);
      if(temp == AY_FALSE)
	tcount++;
      tag = tag->next;
    }

  fprintf(fileptr,"%d\n",tcount);

  /* write tags */
  tag = o->tags;
  while(tag)
    {
      ay_status = ay_tags_temp(ay_interp, tag->name, 0, &temp);
      if(temp == AY_FALSE)
	{
	  fprintf(fileptr,"%s\n",tag->name);
	  fprintf(fileptr,"%s\n",tag->val);
	}
      tag = tag->next;
    }

 return ay_status;
} /* ay_write_tags */


/* ay_write_shader:
 *
 */
int 
ay_write_shader(FILE *fileptr, ay_shader *shader)
{
 int ay_status = AY_OK;
 int argcount = 0, i;
 ay_shader_arg *sarg;

  if(!shader)
    return AY_ENULL;

  fprintf(fileptr,"%s\n",shader->name);
  fprintf(fileptr,"%d\n",shader->type);

  /* count args */
  argcount = 0;
  sarg = shader->arg;
  while(sarg)
    {
      argcount++;
      sarg = sarg->next;
    }
  sarg = shader->arg;

  fprintf(fileptr,"%d\n",argcount);

  while(sarg)
    {
      fprintf(fileptr,"%s\n",sarg->name);
      fprintf(fileptr,"%d\n",sarg->type);

      switch(sarg->type)
	{
	case AY_SACOLOR:
	  fprintf(fileptr,"%g\n",sarg->val.color[0]);
	  fprintf(fileptr,"%g\n",sarg->val.color[1]);
	  fprintf(fileptr,"%g\n",sarg->val.color[2]);
	  break;
	case AY_SAVECTOR:
	case AY_SANORMAL:
	case AY_SAPOINT:
	  fprintf(fileptr,"%g\n",sarg->val.point[0]);
	  fprintf(fileptr,"%g\n",sarg->val.point[1]);
	  fprintf(fileptr,"%g\n",sarg->val.point[2]);
	  break;
	case AY_SASCALAR:
	  fprintf(fileptr,"%g\n",sarg->val.scalar);
	  break;
	case AY_SASTRING:
	  if(sarg->val.string)
	    fprintf(fileptr,"%s\n",sarg->val.string);
	  else
	    fprintf(fileptr,"\n");
	  break;
	case AY_SAMATRIX:
	  for(i = 0; i < 16; i++)
	    {
	      fprintf(fileptr,"%g ",sarg->val.matrix[i]);
	    }
	  fprintf(fileptr,"\n");
	  break;
	}
      sarg = sarg->next;
    }

 return ay_status;
} /* ay_write_shader */


/* ay_write_object:
 *
 */
int
ay_write_object(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 char fname[] = "write_object";
 void **arr = NULL;
 ay_writecb *cb = NULL;
 ay_object *down = NULL;

  if(!o)
    return AY_OK;

  /* mark start of new object */
  putc('\a', fileptr);

  if(o->type < AY_IDLAST)
    {
      fprintf(fileptr,"0\n%d\n", o->type);
    }
  else
    {
      fprintf(fileptr,"1\n%s\n", ay_object_gettypename(o->type));
    }

  if(o->down)
    {
      fprintf(fileptr,"1\n");
    }
  else
    {
      fprintf(fileptr,"0\n");
    }

  ay_status = ay_write_attributes(fileptr, o);

  if(ay_status)
    {
      ay_error(ay_status, fname, "error saving attributes");
      return AY_ERROR;
    }

  ay_status = ay_write_tags(fileptr, o);

  if(ay_status)
    {
      ay_error(ay_status, fname, "error saving tags");
      return AY_ERROR;
    }

  arr = ay_writecbt.arr;
  cb = (ay_writecb *)(arr[o->type]);
  if(cb)
    ay_status = cb(fileptr, o);

  if(ay_status)
    {
      ay_error(ay_status, fname, "write callback failed");
      return AY_ERROR;
    }

  /* write children */
  if(o->down)
    {
      down = o->down;
      while(down)
	{
	  ay_status = ay_write_object(fileptr, down);
	  if(ay_status)
	    {
	      return ay_status;
	    }
	  down = down->next;
	}
    }

 return ay_status;
} /* ay_write_object */


/* ay_write_scene:
 *
 */
int
ay_write_scene(char *fname, int selected)
{
 int ay_status = AY_OK;
 ay_object *o = ay_root;
 FILE *fileptr = NULL;
 char funcname[] = "ay_write_scene";

  if(selected)
    {
      o = ay_currentlevel->object; 
    }

  if(!o)
    return AY_ENULL;

  if(!fname)
    return AY_ENULL;

  if(!(fileptr = fopen(fname, "wb")))
    {
      ay_error(AY_EOPENFILE, funcname, fname);
      return AY_ERROR;
    }

  clearerr(fileptr);

  /* clear oid tags from scene */
  ay_status = ay_instt_clearoidtags(ay_root);
  /* reset oid counter for saving of Instance objects */
  ay_status = ay_instt_createoid(NULL);
  /* create oid tags for master objects */
  ay_status = ay_instt_createorigids(o);
  /* create oid tags for instance objects */
  ay_status = ay_instt_createinstanceids(o);

  /* clear Material ID tags from scene */
  ay_status = ay_matt_clearmaterialids(ay_root);
  /* create new Material ID tags */
  ay_status = ay_matt_creatematerialids(ay_root);
  
  /* write header information */
  ay_write_header(fileptr);

  /* omit EndLevel-object in top level! */
  while(o->next)
    {
      if(selected)
	{
	  if(o->selected)
	    {
	      ay_status = ay_write_object(fileptr, o);
	    }
	}
      else
	{
	  ay_status = ay_write_object(fileptr, o);
	}

      if(ay_status)
	{ fclose(fileptr); return ay_status; }
      o = o->next;
    }
  if(ferror(fileptr) || errno != 0)
    {
      ay_error(AY_ERROR, fname, strerror(errno));
    }

  if(fclose(fileptr))
    {
      ay_error(AY_ERROR, fname, strerror(errno));
    }

 return ay_status;
} /* ay_write_scene */


/* ay_write_scenetcmd:
 *
 */
int
ay_write_scenetcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "saveScene";
 int selected = AY_FALSE;

  /* check args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "filename [1|0]");
      return TCL_OK;
    }

  if(argc > 2)
    selected = atoi(argv[2]);

  ay_status = ay_write_scene(argv[1], selected);

 return TCL_OK;
} /* ay_write_scenetcmd */

