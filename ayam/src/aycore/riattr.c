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

/* riattr.c - RiAttribute tag helpers */

/* ay_riattr_wrib:
 *  write all RiAttribute tags from object o to RIB
 */
int
ay_riattr_wrib(ay_object *o)
{
 ay_tag_object *tag = NULL;
 RtColor color = {0.0f,0.0f,0.0f};
 char *tagvaltmp = NULL, *attrname = NULL, *parname = NULL,
   *partype = NULL, *parval = NULL, *parval2 = NULL;
 char tok[] = ",";
 RtInt itemp, i2temp[2] = {0};
 RtFloat ftemp, f2temp[2] = {0.0f,0.0f};
 RtPoint ptemp = {0.0f,0.0f,0.0f};
 char fname[] = "ay_riattr_wrib", e1[] = "Missing value in RiAttribute tag!";

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
	      parname = strtok(NULL,tok);
	      /* get type */
	      if(parname)
		partype = strtok(NULL,tok);
	      else 
		partype = NULL;

	      if(partype)
		{
		  /* get value */
		  parval = strtok(NULL,tok);
		  if(parval)
		    {
		      switch(*partype)
			{
			case 'i':
			  RiDeclare(parname, "integer");
			  sscanf(parval, "%d", &itemp);
			  RiAttribute(attrname, parname,
				      (RtPointer)&itemp, RI_NULL);
			  break;
			case 'j':
			  RiDeclare(parname, "integer[2]");
			  
			  sscanf(parval, "%d", &(i2temp[0]));
			  parval2 = strtok(NULL, tok);
			  if(parval2)
			    {
			      sscanf(parval2, "%d", &(i2temp[1]));
			    }
			  else
			    {
			      ay_error(AY_ERROR, fname, e1);
			    }
			  RiAttribute(attrname, parname,
				      (RtPointer)&i2temp,
				      RI_NULL);
			  break;
			case 'f':
			  RiDeclare(parname, "float");
			  sscanf(parval, "%f", &ftemp);
			  RiAttribute(attrname, parname,
				      (RtPointer)&ftemp, RI_NULL);
			  break;
			case 'g':
			  RiDeclare(parname, "float[2]");
			  
			  sscanf(parval, "%f", &(f2temp[0]));
			  parval2 = strtok(NULL, tok);
			  if(parval2)
			    {
			      sscanf(parval2, "%f", &(f2temp[1]));
			    }
			  else
			    {
			      ay_error(AY_ERROR, fname, e1);
			    }
			  RiAttribute(attrname, parname,
				      (RtPointer)&f2temp,
				      RI_NULL);
			  break;
			case 's':
			  RiDeclare(parname, "string");
			  RiAttribute(attrname, parname,
				      (RtPointer)&parval, RI_NULL);
			break;
			case 'p':
			  RiDeclare(parname, "point");
			  sscanf(parval, "%f", &ptemp[0]);
			  parval = strtok(NULL, tok);
			  if(parval)
			    {
			      sscanf(parval, "%f", &ptemp[1]);
			    }
			  else
			    {
			      ay_error(AY_ERROR, fname, e1);
			    }
			  parval = strtok(NULL, tok);
			  if(parval)
			    {
			      sscanf(parval, "%f", &ptemp[2]);
			    }
			  else
			    {
			      ay_error(AY_ERROR, fname, e1);
			    }
			  RiAttribute(attrname, parname,
				      (RtPointer)&ptemp, RI_NULL);
			  break;
			case 'c':
			  RiDeclare(parname, "color");
			  sscanf(parval, "%f", &color[0]);
			  parval = strtok(NULL, tok);
			  if(parval)
			    {
			      sscanf(parval, "%f", &color[1]);
			    }
			  else
			    {
			      ay_error(AY_ERROR, fname, e1);
			    }
			  parval = strtok(NULL, tok);
			  if(parval)
			    {
			      sscanf(parval, "%f", &color[2]);
			    }
			  else
			    {
			      ay_error(AY_ERROR, fname, e1);
			    }
			  RiAttribute(attrname, parname,
				      (RtPointer)&color, RI_NULL);
			  break;
			default:
			  break;
			} /* switch */

		    } /* if(parval */

		} /* if(partype */

	    } /* if(attrname */

	} /* if(tagtype== */

      tag = tag->next;
    } /* while */

 return AY_OK;
} /* ay_riattr_wrib */


/* ay_riattr_init:
 *  initialize riattr module by registering the RiAttribute tag type
 */
void
ay_riattr_init(Tcl_Interp *interp)
{

  /* register RiAttribute tag type */
  ay_tags_register(interp, "RiAttribute", &ay_riattr_tagtype);

 return;
} /* ay_riattr_init */
