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

/* riattr.c - RiAttribute tag helpers */

/* ay_riattr_wrib:
 *  write all RiAttribute tags from object o to RIB
 */
int
ay_riattr_wrib(ay_object *o)
{
 int ay_status = AY_OK;
 ay_tag *tag = NULL;
 char *tagvaltmp = NULL, *attrname = NULL, *parname = NULL,
   *partype = NULL, *parval = NULL;
 char tok[] = ",";
 int count;
 RtInt itemp, i2temp[2] = {0};
 RtFloat ftemp, f2temp[2] = {0.0f,0.0f};
 RtPoint ptemp = {0.0f,0.0f,0.0f};
 RtColor color = {0.0f,0.0f,0.0f};
 char fname[] = "riattr_wrib", e1[] = "Malformed RiAttribute tag encountered!";

  if(!o)
    return AY_ENULL;

  /* process tags */
  tag = o->tags;
  while(tag)
    {
      if(tag->type == ay_riattr_tagtype)
	{
	  /* muto teleportilis shaelorasti */
	  if(tagvaltmp)
	    {
	      free(tagvaltmp);
	      tagvaltmp = NULL;
	    }

	  if(!(tagvaltmp = calloc(1, strlen(tag->val)+1)))
	    { return AY_EOMEM; }
	  strcpy(tagvaltmp, tag->val);

	  /* get name of attribute */
	  attrname = strtok(tagvaltmp, tok);

	  /* get parameter(s) */
	  if(attrname)
	    {
	      parval = NULL;
	      partype = NULL;
	      parname = NULL;

	      /* get name */
	      parname = strtok(NULL, tok);
	      /* get type */
	      if(parname)
		partype = strtok(NULL, tok);
	      else
		partype = NULL;

	      if(partype)
		{
		  /* get value */
		  parval = strtok(NULL, tok);
		  if(parval)
		    {
		      parval = tag->val;
		      for(count = 0; count < 3; count++)
			parval = strchr(parval, ',')+1;

		      ay_status = AY_OK;
		      switch(*partype)
			{
			case 'i':
			  count = sscanf(parval, "%d", &itemp);
			  if(count == 1)
			    {
			      RiDeclare(parname, "integer");
			      RiAttribute(attrname, parname,
					  (RtPointer)&itemp, RI_NULL);
			    }
			  else
			    {
			      ay_status = AY_ERROR;
			    }
			  break;
			case 'j':
			  count = sscanf(parval, "%d,%d",
					 &(i2temp[0]), &(i2temp[1]));
			  if(count == 2)
			    {
			      RiDeclare(parname, "integer[2]");
			      RiAttribute(attrname, parname,
					  (RtPointer)&i2temp, RI_NULL);
			    }
			  else
			    {
			      ay_status = AY_ERROR;
			    }
			  break;
			case 'f':
			  count = sscanf(parval, "%f", &ftemp);
			  if(count == 1)
			    {
			      RiDeclare(parname, "float");
			      RiAttribute(attrname, parname,
					  (RtPointer)&ftemp, RI_NULL);
			    }
			  else
			    {
			      ay_status = AY_ERROR;
			    }
			  break;
			case 'g':
			  count = sscanf(parval, "%f,%f",
					 &(f2temp[0]), &(f2temp[1]));
			  if(count == 2)
			    {
			      RiDeclare(parname, "float[2]");
			      RiAttribute(attrname, parname,
					  (RtPointer)&f2temp, RI_NULL);
			    }
			  else
			    {
			      ay_status = AY_ERROR;
			    }
			  break;
			case 's':
			  RiDeclare(parname, "string");
			  RiAttribute(attrname, parname,
				      (RtPointer)&parval, RI_NULL);
			  break;
			case 'p':
			  count = sscanf(parval, "%f,%f,%f",
					 &ptemp[0], &ptemp[1], &ptemp[2]);
			  if(count == 3)
			    {
			      RiDeclare(parname, "point");
			      RiAttribute(attrname, parname,
					  (RtPointer)&ptemp, RI_NULL);
			    }
			  else
			    {
			      ay_status = AY_ERROR;
			    }
			  break;
			case 'c':
			  count = sscanf(parval, "%f,%f,%f",
					 &color[0], &color[1], &color[2]);
			  if(count == 3)
			    {
			      RiDeclare(parname, "color");
			      RiAttribute(attrname, parname,
					  (RtPointer)&color, RI_NULL);
			    }
			  else
			    {
			      ay_status = AY_ERROR;
			    }
			  break;
			default:
			  ay_status = AY_ERROR;
			  break;
			} /* switch */
		    }
		  else
		    {
		      ay_status = AY_ERROR;
		    } /* if(parval */
		}
	      else
		{
		  ay_status = AY_ERROR;
		} /* if(partype */
	    }
	  else
	    {
	      ay_status = AY_ERROR;
	    } /* if(attrname */

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, e1);
	      ay_error_reportobject(AY_ERROR, fname, o);
	    }
	} /* if(tagtype== */

      tag = tag->next;
    } /* while */

  if(tagvaltmp)
    {
      free(tagvaltmp);
    }

 return AY_OK;
} /* ay_riattr_wrib */


/* ay_riattr_init:
 *  initialize riattr module by registering the RiAttribute tag type
 */
void
ay_riattr_init(Tcl_Interp *interp)
{

  /* register RiAttribute tag type */
  ay_tags_register(ay_riattr_tagname, &ay_riattr_tagtype);

 return;
} /* ay_riattr_init */
