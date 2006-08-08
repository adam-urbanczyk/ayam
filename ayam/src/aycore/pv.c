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
 ay_tag *tag = NULL;
 char *tagvaltmp = NULL, *pvname, *pvstorage, *pvtype, *pvvalue, *numvals;
 char tok[] = ",";
 unsigned int i, n;
 RtInt *itemp;
 RtFloat *ftemp;
 RtString *stemp;
 RtPoint *ptemp;
 RtColor *ctemp;
 char fname[] = "ay_pv_filltokpar", e1[] = "Missing data value in PV-tag!";
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
				  else
				    {
				      ay_error(AY_EWARN, fname, e1);
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
				  else
				    {
				      ay_error(AY_EWARN, fname, e1);
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
				  else
				    {
				      ay_error(AY_EWARN, fname, e1);
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
				  else
				    {
				      ay_error(AY_EWARN, fname, e1);
				    }
				  if(pvvalue)
				    {
				      sscanf(pvvalue, "%f", &((ptemp[i])[1]));
				      pvvalue = strtok(NULL, tok);
				    }
				  else
				    {
				      ay_error(AY_EWARN, fname, e1);
				    }
				  if(pvvalue)
				    {
				      sscanf(pvvalue, "%f", &((ptemp[i])[2]));
				      pvvalue = strtok(NULL, tok);
				    }
				  else
				    {
				      ay_error(AY_EWARN, fname, e1);
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
				  else
				    {
				      ay_error(AY_EWARN, fname, e1);
				    }
				  if(pvvalue)
				    {
				      sscanf(pvvalue, "%f", &((ctemp[i])[1]));
				      pvvalue = strtok(NULL, tok);
				    }
				  else
				    {
				      ay_error(AY_EWARN, fname, e1);
				    }
				  if(pvvalue)
				    {
				      sscanf(pvvalue, "%f", &((ctemp[i])[2]));
				      pvvalue = strtok(NULL, tok);
				    }
				  else
				    {
				      ay_error(AY_EWARN, fname, e1);
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
			} /* if(pvvalue */

		    } /* if(n > 0 */

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


/* ay_pv_add:
 *  add a PV tag to object <o>
 */
int
ay_pv_add(ay_object *o, char *name, char *detail, int type,
	  int datalen, void *data)
{
 ay_tag *tag = NULL, **nexttag;
 Tcl_DString ds;
 int i;
 char tmp[255];

  if(!o || !name || !detail || !data)
    return AY_ENULL;

  nexttag = &(o->tags);
  tag = o->tags;
  while(tag)
    {
      nexttag = &(tag->next);
      tag = tag->next;
    }
  tag = NULL;
  if(!(tag = calloc(1, sizeof(ay_tag))))
    return AY_EOMEM;

  tag->type = ay_pv_tagtype;

  if(!(tag->name = calloc(3, sizeof(char))))
    return AY_EOMEM;
  strcpy(tag->name, "PV");

  Tcl_DStringInit(&ds);
  Tcl_DStringAppend(&ds, name, -1);
  Tcl_DStringAppend(&ds, ",", -1);
  Tcl_DStringAppend(&ds, detail, -1);
  Tcl_DStringAppend(&ds, ",", -1);

  switch(type)
    {
    case 0:
      Tcl_DStringAppend(&ds, "float", -1);
    default:
      break;
    } /* switch */

  sprintf(tmp, ",%d", datalen);
  Tcl_DStringAppend(&ds, tmp, -1);

  for(i = 0; i < datalen; i++)
    {
      Tcl_DStringAppend(&ds, ",", -1);

      switch(type)
	{
	case 0:
	  sprintf(tmp, "%f", (float)(((double*)data)[i]));
	  /*snprintf(tmp, 254, "%f", (float)(((double*)data)[i]));*/
	  Tcl_DStringAppend(&ds, tmp, -1);
	default:
	  break;
	} /* switch */
    } /* for */

  if(!(tag->val = calloc(strlen(Tcl_DStringValue(&ds))+1,
			 sizeof(char))))
    return AY_EOMEM;
  strcpy(tag->val, Tcl_DStringValue(&ds));

  Tcl_DStringFree(&ds);

  *nexttag = tag;

 return AY_OK;
} /* ay_pv_add */


/* ay_pv_merge:
 *  merge two PV tags (<t1>, <t2>) into one (<mt>)
 *  the elements in <t2> will be appended to the elements in <t1>
 */
int
ay_pv_merge(ay_tag *t1, ay_tag *t2, ay_tag **mt)
{
 int ay_status = AY_OK;
 char *comma1 = NULL, *comma2 = NULL, buf[128];
 int i = 0;
 unsigned int n1, n2;
 ay_tag *nt = NULL;
 Tcl_DString ds;

  if(!t1 || !t2)
    return AY_ENULL;

  if(!(nt = calloc(1, sizeof(ay_tag))))
    { ay_status = AY_EOMEM; goto cleanup; }

  if(!(nt->name = calloc(3, sizeof(char))))
    { ay_status = AY_EOMEM; goto cleanup; }

  strcpy(nt->name, "PV");

  nt->type = ay_pv_tagtype;

  Tcl_DStringInit(&ds);

  /* find the third comma in t1->val */
  comma1 = t1->val;
  while((i < 3) && (comma1 = strchr(comma1, ',')))
    { i++; comma1++; }
  if(!comma1)
    { ay_status = AY_ERROR; goto cleanup; }

  /* copy "name,detail,type," */
  Tcl_DStringAppend(&ds, t1->val, (int)(comma1-(t1->val)));

  sscanf(comma1, "%d", &n1);

  /* find the third comma in t2->val */
  comma2 = t2->val;
  i = 0;
  while((i < 3) && (comma2 = strchr(comma2, ',')))
    { i++; comma2++; }
  if(!comma2)
    { ay_status = AY_ERROR; goto cleanup; }

  sscanf(comma2, "%d", &n2);

  /* calculate and copy new ndata (number of data elements) */
  sprintf(buf, "%d", n1+n2);
  Tcl_DStringAppend(&ds, buf, -1);

  /* copy data elements */
  if(!(comma1 = strchr(comma1, ',')))
    { ay_status = AY_ERROR; goto cleanup; }

  if(!(comma2 = strchr(comma2, ',')))
    { ay_status = AY_ERROR; goto cleanup; }

  Tcl_DStringAppend(&ds, comma1, -1);
  Tcl_DStringAppend(&ds, comma2, -1);

  /* copy collected string to new tag */
  if(!(nt->val = calloc(Tcl_DStringLength(&ds)+1,
			sizeof(char))))
    { ay_status = AY_EOMEM; goto cleanup; }
  strcpy(nt->val, Tcl_DStringValue(&ds));

  /* return result */
  *mt = nt;
  nt = NULL;

cleanup:

  if(nt)
    {
      if(nt->name)
	free(nt->name);
      if(nt->val)
	free(nt->val);
      free(nt);
    } /* if */

  Tcl_DStringFree(&ds);

 return ay_status;
} /* ay_pv_merge */


/* ay_pv_cmpname:
 *  compare the primitive variable names of PV tags <t1> and <t2>
 *  returns AY_TRUE if they are equal, returns AY_FALSE else and on error
 */
int
ay_pv_cmpname(ay_tag *t1, ay_tag *t2)
{
  char *c1 = NULL, *c2 = NULL;

  if(!t1 || !t2)
    return AY_FALSE;

  if((t1->type != ay_pv_tagtype) || (t2->type != ay_pv_tagtype))
    return AY_FALSE;

  c1 = t1->val;
  c2 = t2->val;

  if(!c1 || !c2)
    return AY_FALSE;

  while((*c1 != '\0') && (*c2 != '\0') && (*c1 == *c2) &&
	(*c1 != ',') && (*c2 != ','))
    {
      c1++; c2++;
    }

  if((*c1 != '\0') && (*c2 != '\0') && (*c1 == *c2) && (*c1 == ','))
    return AY_TRUE;

 return AY_FALSE;
} /* ay_pv_cmpname */


/* ay_pv_convert:
 *
 */
int
ay_pv_convert(ay_tag *tag, unsigned int *datalen, void **data)
{
 unsigned int count = 0, i = 0;
 char *c1, *c2, *c3;
 double *da = NULL;

  if(!tag)
    return AY_ENULL;

  if(tag->type != ay_pv_tagtype)
    return AY_ERROR;

  c1 = tag->val;

  /* find the type */
  c1 = strchr(c1, ',');
  if(!c1)
    return AY_ERROR;
  c1++;

  /* find the length */
  c2 = strchr(c1, ',');
  if(!c2)
    return AY_ERROR;
  c2++;
  sscanf(c2, "%d", &count);

  /* find the data */
  c3 = strchr(c2, ',');
  if(!c3)
    return AY_ERROR;
  c3++;
  switch(*c1)
    {
    case 'f':
      /* allocate memory */
      if(!(da = calloc(count, sizeof(double))))
	return AY_EOMEM;
      /* parse data and fill memory */
      do
	{
	  sscanf(c3, ",%lg", &(da[i]));
	  i++;
	}
      while((c3 = strchr(c3, ',')));
      /* prepare result */
      *datalen = count;
      *data = da;
      break;
    default:
      return AY_ERROR;
      break;
    } /* switch */

 return AY_OK;
} /* ay_pv_convert */


/* ay_pv_count:
 *  count PV tags of object <o>
 */
int
ay_pv_count(ay_object *o)
{
 ay_tag *tag = NULL;
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
