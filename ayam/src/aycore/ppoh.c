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

#include "ppoh.h"

/* ppoh.c - pretty print object hierarchies (for debugging purposes) */

/* ay_ppoh_prtype:
 *  ppoh callback that prints the type of an object
 */
int
ay_ppoh_prtype(ay_object *o, FILE *fileptr, char *prefix)
{
 int ay_status = AY_OK;
 ay_level_object *level = NULL;

  if(prefix)
    {
      fprintf(fileptr, "%s", prefix);
    }

  fprintf(fileptr, "%s", ay_object_gettypename(o->type));

  if(o->type == AY_IDLEVEL)
    {
      level = (ay_level_object*)o->refine;
      switch(level->type)
	{
	case AY_LTEND:
	  fprintf(fileptr, ", End");
	  break;
	case AY_LTLEVEL:
	  fprintf(fileptr, ", Level");
	  break;
	case AY_LTUNION:
	  fprintf(fileptr, ", Union");
	  break;
	case AY_LTDIFF:
	  fprintf(fileptr, ", Difference");
	  break;
	case AY_LTINT:
	  fprintf(fileptr, ", Intersection");
	  break;
	case AY_LTPRIM:
	  fprintf(fileptr, ", Primitive");
	  break;
	default:
	  break;
	} /* switch */
    } /* if */
  fprintf(fileptr, "\n");

 return ay_status;
} /* ay_ppoh_prtype */


/* ay_ppoh_prtrafos:
 *  ppoh callback that prints the transformation attributes of an object
 */
int
ay_ppoh_prtrafos(ay_object *o, FILE *fileptr, char *prefix)
{
 int ay_status = AY_OK;
 double euler[3];

  if(prefix)
    {
      fprintf(fileptr, "%s", prefix);
    }

  if((fabs(o->movx) < AY_EPSILON) &&
     (fabs(o->movy) < AY_EPSILON) &&
     (fabs(o->movz) < AY_EPSILON) &&
     (o->scalx == 1.0) &&
     (o->scaly == 1.0) &&
     (o->scalz == 1.0) &&
     (o->quat[0] == 0.0) && 
     (o->quat[1] == 0.0) &&
     (o->quat[2] == 0.0) &&
     (o->quat[3] == 1.0))
    {
      fprintf(fileptr, "Default Transformations");
    }
  else
    {
      fprintf(fileptr, "Trans: %g %g %g\n", o->movx, o->movy, o->movz);
      if(prefix)
	{
	  fprintf(fileptr, "%s", prefix);
	}
      fprintf(fileptr, "Scale: %g %g %g\n", o->scalx, o->scaly, o->scalz);
      ay_quat_toeuler(o->quat, euler);
      if(prefix)
	{
	  fprintf(fileptr, "%s", prefix);
	}
      fprintf(fileptr, "Rot: %g %g %g\n", euler[0], euler[1], euler[2]);
      if(prefix)
	{
	  fprintf(fileptr, "%s", prefix);
	}
      fprintf(fileptr, "Quat: %g %g %g %g", o->quat[0], o->quat[1], o->quat[2],
	      o->quat[3]);
    } /* if */
  fprintf(fileptr, "\n");

 return ay_status;
} /* ay_ppoh_prtrafos */


/* ay_ppoh_prflags:
 *  ppoh callback that prints miscellaneous attributes of an object
 */
int
ay_ppoh_prflags(ay_object *o, FILE *fileptr, char *prefix)
{
 int ay_status = AY_OK;
 ay_mat_object *mat = NULL;

  if(prefix)
    {
      fprintf(fileptr, "%s", prefix);
    }

  fprintf(fileptr, "Refcount: %d", o->refcount);

  if(o->selected)
    fprintf(fileptr, ", Selected");

  if(o->modified)
    fprintf(fileptr, ", Modified");
  
  if(o->parent)
    fprintf(fileptr, ", Parent");

  if(o->inherit_trafos)
    fprintf(fileptr, ", InhTrafos");

  if(o->hide)
    fprintf(fileptr, ", Hidden");

  if(o->hide_children)
    fprintf(fileptr, ", HideChld");

  if(o->tags)
    fprintf(fileptr, ", HasTags");

  if(o->mat)
    {
      mat = (ay_mat_object*)(o->mat);
      fprintf(fileptr, ", Material: %s", *(mat->nameptr));
    }

  fprintf(fileptr, "\n");

 return ay_status;
} /* ay_ppoh_prflags */


/* ay_ppoh_prtags:
 *  ppoh callback that prints the tags of an object
 */
int
ay_ppoh_prtags(ay_object *o, FILE *fileptr, char *prefix)
{
 int ay_status = AY_OK;
 ay_tag_object *tag = NULL;

  if(!o->tags)
    return ay_status;

  tag = o->tags;
  while(tag)
    {
      if(prefix)
	{
	  fprintf(fileptr, "%s", prefix);
	}

      if(tag->name)
	{
	  fprintf(fileptr, "%s", tag->name);
	}
      else
	{
	  fprintf(fileptr, "(NULL!)");
	}

      if(tag->val)
	{
	  fprintf(fileptr, ", %s", tag->val);
	}
      else
	{
	  fprintf(fileptr, "(NULL!)");
	}

      tag = tag->next;
    } /* while */

  fprintf(fileptr, "\n");

 return ay_status;
} /* ay_ppoh_prtags */


/* ay_ppoh_print:
 *
 */
int
ay_ppoh_print(ay_object *o, FILE *fileptr, int indent, ay_printcb **cbv)
{
 int ay_status = AY_OK;
 int i = 0;
 char fillc = ' ';
 char *prefix = NULL;

  if(!o || !cbv)
    return AY_ENULL;

  if(indent > 0)
    {
      if(!(prefix = calloc(indent+1, sizeof(char))))
	{
	  return AY_EOMEM;
	}
      memset(prefix, (int)fillc, indent*sizeof(char));
    }

  while(o)
    {

      if(prefix)
	{
	  fprintf(fileptr, "%s----\n", prefix);
	}
      else
	{
	  fprintf(fileptr, "----\n");
	}

      i = 0;
      while(cbv[i])
	{
	  ay_status = cbv[i](o, fileptr, prefix);
	  i++;
	}

      if(o->down)
	{
	  ay_status = ay_ppoh_print(o->down, fileptr, indent+4, cbv);
	}

      o = o->next;
    } /* while */

  if(prefix)
    free(prefix);

 return ay_status;
} /* ay_ppoh_print */


/* ay_ppoh_printtcmd:
 *
 */
int
ay_ppoh_printtcmd(ClientData clientData, Tcl_Interp *interp,
		  int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "ppoh_print";
 char *filename = NULL;
 FILE *fileptr = NULL;
 ay_printcb *cbv[4];

  cbv[0] = &ay_ppoh_prtype;
  cbv[1] = &ay_ppoh_prtrafos;
  cbv[2] = &ay_ppoh_prflags;
  cbv[3] = NULL;

  if(argc > 1)
    {
      filename = argv[1];
    }
  else
    {
      ay_error(AY_EARGS, fname, "filename");
      return TCL_OK;
    }

  if(!(fileptr = fopen(filename, "wb")))
    {
      ay_error(AY_EOPENFILE, fname, filename);
      return TCL_OK;
    }

  ay_status = ay_ppoh_print(ay_root, fileptr, 0, cbv);

  fclose(fileptr);

 return TCL_OK;
} /* ay_ppoh_printtcmd */


/* ay_ppoh_init:
 *  initialize ppoh module
 */
int
ay_ppoh_init(Tcl_Interp *interp)
{
  /*char fname[] = "ppoh_init";*/
 static int initialized = AY_FALSE;
 /* int ay_status = AY_OK;*/

  if(!initialized)
    {
      /* register some C-functions as Tcl-Commands */
      Tcl_CreateCommand(interp, "ppoh", ay_ppoh_printtcmd,
			(ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
      initialized = AY_TRUE;
    }

 return TCL_OK;
} /* ay_ppoh_init */
