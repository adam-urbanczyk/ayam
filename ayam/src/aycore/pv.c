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

/* pv.c - PV (PrimitiveVariable) tag helpers */

/* ay_pv_filltokpar:
 *  parse all PV tags of object <o> into tokens/parms arrays
 *  ready for a call into the RenderMan Interface
 *  <declare>: if AY_TRUE, make the necessary calls to RiDeclare
 *  <start>: offset in tokens/parms for possibly already filled slots
 *  <added>: this value is increased according to the number of PV tags
 *  processed
 *  <tokens>,<parms>: have to be of the right size and allocated outside!
 *  XXXX leaks lots of memory in low mem situation
 */
int
ay_pv_filltokpar(ay_object *o, int declare, int start,
		 int *added, RtToken tokens[], RtPointer parms[])
{
 ay_tag_object *tag = NULL;
 char *tagvaltmp = NULL, *pvname, *pvstorage, *pvtype, *pvvalue, *numvals;
 char tok[] = ",";
 unsigned int i, n;
 RtInt *itemp;
 RtFloat *ftemp;
 RtString *stemp;
 RtPoint *ptemp;
 RtColor *ctemp;
 char fname[] = "ay_pv_fillparams", e1[] = "Missing value in tag!";
 Tcl_DString ds;

  if(!o)
    return AY_ENULL;

  /* process tags */
  tag = o->tags;
  while(tag)
    {
      if(tag->type == ay_pv_tagtype)
	{
	  if(tagvaltmp)
	    {
	      free(tagvaltmp);
	      tagvaltmp = NULL;
	    }

	  if(!(tagvaltmp = calloc(1, strlen(tag->val)+1)))
	    { return AY_EOMEM; }
	  strcpy(tagvaltmp, tag->val);

	  /* get name of option */
	  pvname = NULL;
	  pvname = strtok(tagvaltmp, tok);

	  /* get parameter(s) */
	  if(pvname)
	    {
	      pvvalue = NULL;
	      pvtype = NULL;
	      pvstorage = NULL;
	      if(!(tokens[start] = calloc(strlen(pvname)+1, sizeof(char))))
		return AY_EOMEM;
	      strcpy(tokens[start], pvname);

	      /* get storage class */
	      pvstorage = strtok(NULL, tok);
	      /* get type */
	      if(pvstorage)
		{
		  pvtype = strtok(NULL, tok);
		}
	      else
		{ 
		  pvtype = NULL;
		}
	      if(pvtype)
		{
		  Tcl_DStringInit(&ds);
		  Tcl_DStringAppend(&ds, pvstorage, -1);
		  Tcl_DStringAppend(&ds, " ", -1);

		  numvals = strtok(NULL, tok);
		  if(numvals)
		    {
		      sscanf(numvals, "%d", &n);
		    }

		  if(n > 0)
		    {
		      /* get value(s) */
		      pvvalue = strtok(NULL, tok);
		      if(pvvalue)
			{
			  switch(*pvtype)
			    {
			    case 'i':
			      Tcl_DStringAppend(&ds, "integer", -1);

			      itemp = NULL;
			      if(!(itemp = calloc(n, sizeof(RtInt))))
				return AY_EOMEM;

			      for(i = 0; i < n; i++)
				{
				  if(pvvalue)
				    {
				      sscanf(pvvalue, "%d", &(itemp[i]));
				      pvvalue = strtok(NULL, tok);
				    }
				}
			      parms[start] = (RtPointer)itemp;
			      break;
			    case 'f':
			      Tcl_DStringAppend(&ds, "float", -1);

			      ftemp = NULL;
			      if(!(ftemp = calloc(n, sizeof(RtFloat))))
				return AY_EOMEM;

			      for(i = 0; i < n; i++)
				{
				  if(pvvalue)
				    {
				      sscanf(pvvalue, "%f", &(ftemp[i]));
				      pvvalue = strtok(NULL, tok);
				    }
				}
			      parms[start] = (RtPointer)ftemp;
			      break;
			    case 's':
			      Tcl_DStringAppend(&ds, "string", -1);

			      ftemp = NULL;
			      if(!(stemp = calloc(n, sizeof(RtString))))
				return AY_EOMEM;

			      for(i = 0; i < n; i++)
				{
				  if(pvvalue)
				    {
				      if(!(stemp[i] = calloc(strlen(pvvalue)+1,
							     sizeof(char))))
					return AY_EOMEM;
				      strcpy(stemp[i], pvvalue);
				      pvvalue = strtok(NULL, tok);
				    }
				}
			      parms[start] = (RtPointer)stemp;
			      break;
			    case 'p':
			      Tcl_DStringAppend(&ds, "point", -1);

			      ptemp = NULL;
			      if(!(ptemp = calloc(n, sizeof(RtPoint))))
				return AY_EOMEM;

			      for(i = 0; i < n; i++)
				{
				  if(pvvalue)
				    {
				      sscanf(pvvalue, "%f", &((ptemp[i])[0]));
				      pvvalue = strtok(NULL, tok);
				    }
				  if(pvvalue)
				    {
				      sscanf(pvvalue, "%f", &((ptemp[i])[1]));
				      pvvalue = strtok(NULL, tok);
				    }
				  if(pvvalue)
				    {
				      sscanf(pvvalue, "%f", &((ptemp[i])[2]));
				      pvvalue = strtok(NULL, tok);
				    }
				} /* for */
			      parms[start] = (RtPointer)ptemp;
			      break;
			    case 'c':
			      Tcl_DStringAppend(&ds, "color", -1);

			      ctemp = NULL;
			      if(!(ctemp = calloc(n, sizeof(RtPoint))))
				return AY_EOMEM;

			      for(i = 0; i < n; i++)
				{
				  if(pvvalue)
				    {
				      sscanf(pvvalue, "%f", &((ctemp[i])[0]));
				      pvvalue = strtok(NULL, tok);
				    }
				  if(pvvalue)
				    {
				      sscanf(pvvalue, "%f", &((ctemp[i])[1]));
				      pvvalue = strtok(NULL, tok);
				    }
				  if(pvvalue)
				    {
				      sscanf(pvvalue, "%f", &((ctemp[i])[2]));
				      pvvalue = strtok(NULL, tok);
				    }
				} /* for */
			      parms[start] = (RtPointer)ctemp;
			      break;
			    default:
			      break;
			    } /* switch */

			  if(declare)
			    {
			      RiDeclare(pvname, Tcl_DStringValue(&ds));
			    }

			  Tcl_DStringFree(&ds);
			  start++;
			  (*added)++;
			} /* if */

		    } /* if(pvvalue */

		} /* if(pvtype */

	    } /* if(pvname */

	} /* if(tagtype== */

      tag = tag->next;
    } /* while */

  if(tagvaltmp)
    {
      free(tagvaltmp);
    }

 return AY_OK;
} /* ay_pv_filltokpar */


/* ay_pv_count:
 *  count PV tags of object <o>
 */
int
ay_pv_count(ay_object *o)
{
 ay_tag_object *tag = NULL;
 int count = 0;

  if(!o)
    return 0;

  tag = o->tags;
  while(tag)
    {
      if(tag->type == ay_pv_tagtype)
	{
	  count++;
	}
      tag = tag->next;
    }

 return count;
} /* ay_pv_count */


/* ay_pv_init:
 *  initialize pv module by registering the PV tag type
 */
void
ay_pv_init(Tcl_Interp *interp)
{

  /* register PV tag type */
  ay_tags_register(interp, "PV", &ay_pv_tagtype);

 return;
} /* ay_pv_init */
