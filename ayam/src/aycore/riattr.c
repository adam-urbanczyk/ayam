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
 RtColor color = {0};
 char *tagvaltmp = NULL, *attrname = NULL, *parname = NULL,
   *partype = NULL, *parval = NULL;
 char tok[] = ",";
 RtInt itemp;
 RtFloat ftemp;
 RtPoint ptemp = {0};

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
			  RiDeclare(parname, "int");
			  sscanf(parval, "%d", &itemp);
			  RiAttribute(attrname, parname,
				      (RtPointer)&itemp, RI_NULL);
			  break;
			case 'f':
			  RiDeclare(parname, "float");
			  sscanf(parval, "%f", &ftemp);
			  RiAttribute(attrname, parname,
				      (RtPointer)&ftemp, RI_NULL);
			  break;
			case 's':
			  RiDeclare(parname, "string");
			  RiAttribute(attrname, parname,
				      (RtPointer)&parval, RI_NULL);
			break;
			case 'p':
			  RiDeclare(parname, "point");
			  sscanf(parval, "%f,%f,%f", &ptemp[0], &ptemp[1],
				 &ptemp[2]);
			  RiAttribute(attrname, parname,
				      (RtPointer)&ptemp, RI_NULL);
			  break;
			case 'c':
			  RiDeclare(parname, "color");
			  sscanf(parval, "%f,%f,%f", &color[0], &color[1],
				 &color[2]);
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
