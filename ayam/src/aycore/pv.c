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

/* pv.c - PV (PrimitiveVariable) tag helpers */

/* ay_pv_filltokpar:
 *  parse all PV tags of object <o> into tokens/parms arrays
 *  ready for a call into the RenderMan Interface
 *  <declare>: if AY_TRUE, make the necessary calls to RiDeclare
 *  <start>: offset in tokens/parms for possibly already filled slots
 *  <added>: this value is increased according to the number of PV tags
 *  processed
 *  <tokens>,<parms>: have to be of the right size and allocated outside!
 */
int
ay_pv_filltokpar(ay_object *o, int declare, int start,
		 int *added, RtToken tokens[], RtPointer parms[])
{
 int ay_status = AY_OK;
 ay_tag *tag = NULL;
 char *tagvaltmp = NULL, *pvname, *pvstorage, *pvtype, *pvvalue, *numvals;
 char tok[] = ",";
 unsigned int i, j, n;
 RtFloat *ftemp = NULL;
 RtString *stemp = NULL;
 RtPoint *ptemp = NULL;
 RtColor *ctemp = NULL, *otemp = NULL;
 char fname[] = "pv_filltokpar", e1[] = "Missing data value in PV-tag!";
 Tcl_DString ds, dso;

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
	    }

	  if(!(tagvaltmp = malloc((strlen(tag->val)+1)*sizeof(char))))
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
	      if(!(tokens[start] = malloc((strlen(pvname)+1)*sizeof(char))))
		{ ay_status = AY_EOMEM; goto cleanup; }

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
		  n = 0;
		  if(numvals)
		    {
		      sscanf(numvals, "%u", &n);
		    }

		  if(n > 0)
		    {
		      /* get value(s) */
		      pvvalue = strtok(NULL, tok);
		      if(pvvalue)
			{
			  switch(*pvtype)
			    {
			    case 'f':
			      Tcl_DStringAppend(&ds, "float", -1);

			      if(!(ftemp = malloc(n*sizeof(RtFloat))))
				{ ay_status = AY_EOMEM; goto cleanup; }

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

			    case 'g':
			      Tcl_DStringAppend(&ds, "float[2]", -1);

			      if(!(ftemp = malloc(n*2*sizeof(RtFloat))))
				{ ay_status = AY_EOMEM; goto cleanup; }
			      j = 0;
			      for(i = 0; i < n; i++)
				{
				  if(pvvalue)
				    {
				      sscanf(pvvalue, "%f", &(ftemp[j]));
				      pvvalue = strtok(NULL, tok);
				    }
				  else
				    {
				      ay_error(AY_EWARN, fname, e1);
				    }
				  j++;
				  if(pvvalue)
				    {
				      sscanf(pvvalue, "%f", &(ftemp[j]));
				      pvvalue = strtok(NULL, tok);
				    }
				  else
				    {
				      ay_error(AY_EWARN, fname, e1);
				    }
				  j++;
				}
			      parms[start] = (RtPointer)ftemp;
			      break;

			    case 's':
			      Tcl_DStringAppend(&ds, "string", -1);

			      if(!(stemp = malloc(n*sizeof(RtString))))
				{ ay_status = AY_EOMEM; goto cleanup; }

			      for(i = 0; i < n; i++)
				{
				  if(pvvalue)
				    {
				      if(!(stemp[i] = malloc(
					 (strlen(pvvalue)+1)*sizeof(char))))
					{ ay_status = AY_EOMEM; goto cleanup; }
				      strcpy(stemp[i], pvvalue);
				      pvvalue = strtok(NULL, tok);
				    }
				  else
				    {
				      ay_error(AY_EWARN, fname, e1);
				    }
				}
			      parms[start] = (RtPointer)stemp;
			      stemp = NULL;
			      break;

			    case 'n':
			    case 'p':
			    case 'v':
			      switch(*pvtype)
				{
				case 'n':
				  Tcl_DStringAppend(&ds, "normal", -1);
				  break;
				case 'p':
				  Tcl_DStringAppend(&ds, "point", -1);
				  break;
				case 'v':
				  Tcl_DStringAppend(&ds, "vector", -1);
				  break;
				default:
				  break;
				}

			      if(!(ptemp = malloc(n*sizeof(RtPoint))))
				{ ay_status = AY_EOMEM; goto cleanup; }

			      for(i = 0; i < n; i++)
				{
				  for(j = 0; j < 3; j++)
				    {
				      if(pvvalue)
					{
				      sscanf(pvvalue, "%f", &((ptemp[i])[j]));
				      pvvalue = strtok(NULL, tok);
					}
				      else
					{
					  ay_error(AY_EWARN, fname, e1);
					}
				    } /* for j */
				} /* for i */
			      parms[start] = (RtPointer)ptemp;
			      break;

			    case 'c':
			      Tcl_DStringAppend(&ds, "color", -1);

			      if(!(ctemp = malloc(n*sizeof(RtColor))))
				{ ay_status = AY_EOMEM; goto cleanup; }

			      for(i = 0; i < n; i++)
				{
				  for(j = 0; j < 3; j++)
				    {
				      if(pvvalue)
					{
				      sscanf(pvvalue, "%f", &((ctemp[i])[j]));
				      pvvalue = strtok(NULL, tok);
					}
				      else
					{
					  ay_error(AY_EWARN, fname, e1);
					}
				    } /* for j */
				} /* for i */
			      parms[start] = (RtPointer)ctemp;
			      ctemp = NULL;
			      break;

			    case 'd':
			      Tcl_DStringAppend(&ds, "color", -1);

			      if(!(ctemp = malloc(n*sizeof(RtColor))))
				{ ay_status = AY_EOMEM; goto cleanup; }

			      if(!(otemp = malloc(n*sizeof(RtColor))))
				{ ay_status = AY_EOMEM; goto cleanup; }

			      for(i = 0; i < n; i++)
				{
				  for(j = 0; j < 3; j++)
				    {
				      if(pvvalue)
					{
				      sscanf(pvvalue, "%f", &((ctemp[i])[j]));
				      pvvalue = strtok(NULL, tok);
					}
				      else
					{
					  ay_error(AY_EWARN, fname, e1);
					}
				    } /* for j */
				  /* read the opacity */
				  if(pvvalue)
				    {
				      sscanf(pvvalue, "%f", &((otemp[i])[0]));
				      /* expand to RenderMan opacity style */
				      otemp[i][1] = otemp[i][0];
				      otemp[i][2] = otemp[i][0];
				      pvvalue = strtok(NULL, tok);
				    }
				  else
				    {
				      ay_error(AY_EWARN, fname, e1);
				    }
				} /* for */
			      parms[start] = (RtPointer)ctemp;
			      ctemp = NULL;

			      /* sneak in the opacity */
			      start++;
			      (*added)++;
			      if(declare)
				{
				  Tcl_DStringInit(&dso);
				  Tcl_DStringAppend(&dso, pvstorage, -1);
				  Tcl_DStringAppend(&dso, " color", -1);
				  RiDeclare(ay_prefs.opacityname,
					    Tcl_DStringValue(&dso));
				  Tcl_DStringFree(&dso);
				}
			      if(!(tokens[start] = calloc(strlen(
				      ay_prefs.opacityname)+1, sizeof(char))))
				{ ay_status = AY_EOMEM; goto cleanup; }

			      strcpy(tokens[start], pvname);
			      parms[start] = (RtPointer)otemp;
			      otemp = NULL;
			      break;

			    default:
			      /* XXXX issue error message? */
			      /* ...unsupported type encountered */
			      break;
			    } /* switch */

			  if(declare)
			    {
			      RiDeclare(pvname, Tcl_DStringValue(&ds));
			    }

			  start++;
			  (*added)++;
			} /* if(pvvalue */

		    } /* if(n > 0 */

		  Tcl_DStringFree(&ds);
		} /* if(pvtype */

	    } /* if(pvname */

	} /* if(tagtype== */

      tag = tag->next;
    } /* while */

cleanup:

  if(tagvaltmp)
    free(tagvaltmp);

  if(stemp)
    free(stemp);

  if(ctemp)
    free(ctemp);

  if(otemp)
    free(otemp);

 return ay_status;
} /* ay_pv_filltokpar */


/* ay_pv_add:
 *  add a PV tag to object <o>
 */
int
ay_pv_add(ay_object *o,
	  const char *name, const char *detail, const char *type,
	  int datalen, int stride, void *data)
{
 ay_tag *tag = NULL, **nexttag;
 Tcl_DString ds;
 int i, len;
 char tmp[255];

  if(!o || !name || !detail || !type || !data)
    return AY_ENULL;

  nexttag = &(o->tags);
  tag = o->tags;
  while(tag)
    {
      nexttag = &(tag->next);
      tag = tag->next;
    }

  if(!(tag = calloc(1, sizeof(ay_tag))))
    return AY_EOMEM;

  tag->type = ay_pv_tagtype;

  if(!(tag->name = calloc(3, sizeof(char))))
    { free(tag); return AY_EOMEM; }
  strcpy(tag->name, "PV");

  Tcl_DStringInit(&ds);
  Tcl_DStringAppend(&ds, name, -1);
  Tcl_DStringAppend(&ds, ",", 1);
  Tcl_DStringAppend(&ds, detail, -1);
  Tcl_DStringAppend(&ds, ",", 1);
  Tcl_DStringAppend(&ds, type, 1);

  sprintf(tmp, ",%d", datalen);
  Tcl_DStringAppend(&ds, tmp, -1);

  switch(type[0])
    {
    case 'f':
      /* float */
      for(i = 0; i < datalen*stride; i += stride)
	{
	  len = sprintf(tmp, ",%f", ((float*)data)[i]);
	  Tcl_DStringAppend(&ds, tmp, len);
	}
      break;
    case 'g':
      /* float[2] */
      for(i = 0; i < datalen*stride; i += stride)
	{
	  len = sprintf(tmp, ",%f,%f", ((float*)data)[i], ((float*)data)[i+1]);
	  Tcl_DStringAppend(&ds, tmp, len);
	}
      break;
    case 'p':
    case 'v':
    case 'n':
    case 'c':
      /* point/vector/normal/color */
      for(i = 0; i < datalen*stride; i += stride)
	{
	  len = sprintf(tmp, ",%f,%f,%f", (float)(((double*)data)[i]),
		  (float)(((double*)data)[i+1]),
		  (float)(((double*)data)[i+2]));
	  Tcl_DStringAppend(&ds, tmp, len);
	}
      break;
    case 'd':
      /* color+opacity */
      for(i = 0; i < datalen*stride; i += stride)
	{
	  len = sprintf(tmp, ",%f,%f,%f,%f", ((float*)data)[i],
		  ((float*)data)[i+1],
		  ((float*)data)[i+2],
		  ((float*)data)[i+3]);
	  Tcl_DStringAppend(&ds, tmp, len);
	}
      break;
    default:
      break;
    } /* switch */

  if(!(tag->val = malloc((Tcl_DStringLength(&ds)+1)*sizeof(char))))
    { free(tag->name); free(tag); return AY_EOMEM; }
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
  Tcl_DStringAppend(&ds, (char*)t1->val, (int)(comma1-((char*)t1->val)));

  sscanf(comma1, "%u", &n1);

  /* find the third comma in t2->val */
  comma2 = t2->val;
  i = 0;
  while((i < 3) && (comma2 = strchr(comma2, ',')))
    { i++; comma2++; }
  if(!comma2)
    { ay_status = AY_ERROR; goto cleanup; }

  sscanf(comma2, "%u", &n2);

  /* calculate and copy new ndata (number of data elements) */
  sprintf(buf, "%u", n1+n2);
  Tcl_DStringAppend(&ds, buf, -1);

  /* copy data elements */
  if(!(comma1 = strchr(comma1, ',')))
    { ay_status = AY_ERROR; goto cleanup; }

  if(!(comma2 = strchr(comma2, ',')))
    { ay_status = AY_ERROR; goto cleanup; }

  Tcl_DStringAppend(&ds, comma1, -1);
  Tcl_DStringAppend(&ds, comma2, -1);

  /* copy collected string to new tag */
  if(!(nt->val = malloc((Tcl_DStringLength(&ds)+1)*sizeof(char))))
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


/* ay_pv_cmpndt:
 *  compare the primitive variable names, details, and data types
 *  of the PV tags <t1> and <t2>
 *  returns AY_TRUE if they are equal, returns AY_FALSE else and on error
 *  does not check the tag names, but does check the tag types
 */
int
ay_pv_cmpndt(ay_tag *t1, ay_tag *t2)
{
 char *c1 = NULL, *c2 = NULL;
 int i;

  if(!t1 || !t2)
    return AY_FALSE;

  if((t1->type != ay_pv_tagtype) || (t2->type != ay_pv_tagtype))
    return AY_FALSE;

  c1 = t1->val;
  c2 = t2->val;

  if(!c1 || !c2)
    return AY_FALSE;

  /* get the third comma in t1->val */
  for(i = 0; i < 3; i++)
    {
      c1 = strchr(c1, ',');

      if(!c1)
	return AY_FALSE;

      c1++;
    }

  /* now compare the strings up to the third comma in t1->val,
     i.e. compare their names, details, and types */
  if(!strncmp((char*)t1->val, (char*)t2->val, (c1 - (char*)t1->val)))
    {
      return AY_TRUE;
    }

 return AY_FALSE;
} /* ay_pv_cmpndt */


/* ay_pv_checkndt:
 *  check, whether the tag <t> is a PV tag, and check its name,
 *  detail, and data type
 *  returns AY_TRUE on full match and correct PV syntax, AY_FALSE else
 */
int
ay_pv_checkndt(ay_tag *t, const char *name, const char *detail,
	       const char *type)
{
 char *c = NULL;

  if(!t)
    return AY_FALSE;

  if((t->type != ay_pv_tagtype))
    return AY_FALSE;

  c = t->val;

  if(!c)
    return AY_FALSE;

  if(!strncmp(c, name, strlen(name)))
    {
      c = strchr(c, ',');
      if(!c)
	return AY_FALSE;

      c++;

      if(!strncmp(c, detail, strlen(detail)))
	{
	  c = strchr(c, ',');
	  if(!c)
	    return AY_FALSE;

	  c++;

	  if(!strncmp(c, type, strlen(type)))
	    return AY_TRUE;
	} /* if */
    } /* if */

 return AY_FALSE;
} /* ay_pv_checkndt */


/** ay_pv_getdetail:
 *  returns detail from a PV tag
 *
 *  \param[in] t tag to analyze
 *  \param[in,out] detail pointer to detail string found, may be NULL
 *
 *  \returns -1 on error, the detail encoded in an integer else
 */
int
ay_pv_getdetail(ay_tag *t, char **detail)
{
 char *c;
 int result = -1;

  if(!t || !t->type || !t->val || (t->type != ay_pv_tagtype))
    return result;

  c = strchr(t->val, ',');

  if(!c)
    return result;

  c++;

  if(!strncmp(c, "constant", 8))
    result = 0;
  else
   if(!strncmp(c, "uniform", 7))
    result = 1;
  else
   if(!strncmp(c, "vertex", 6))
    result = 2;
  else
   if(!strncmp(c, "varying", 7))
    result = 3;

  if(detail)
    *detail = c;

 return result;
} /* ay_pv_getdetail */


/** ay_pv_gettype:
 *  returns data type from a PV tag
 *
 *  \param[in] t tag to analyze
 *
 *  \returns -1 on error, the data type encoded in an integer else
 */
int
ay_pv_gettype(ay_tag *t)
{
 char *c;
 int result = -1;

  if(!t || !t->type || !t->val || (t->type != ay_pv_tagtype))
    return result;

  c = strchr(t->val, ',');

  if(!c)
    return result;

  c++;

  c = strchr(t->val, ',');

  c++;

  switch(*c)
    {
    case 'f':
      result = 0;
      break;
    case 'g':
      result = 1;
      break;
    case 'c':
      result = 2;
      break;
    case 'p':
      result = 3;
      break;
    case 'n':
      result = 4;
      break;
    case 'v':
      result = 5;
      break;
    case 'd':
      result = 6;
      break;
    default:
      break;
    } /* switch */

 return result;
} /* ay_pv_gettype */


/* ay_pv_convert:
 *  convert data from PV tag <tag> into an array of doubles (<type> 0)
 *  or floats (<type> 1)
 *  Warning: <datalen> does not contain the array size, but the number
 *  of data elements, e.g. for one color value it is 1 and not 3!
 */
int
ay_pv_convert(ay_tag *tag, int type, unsigned int *datalen, void **data)
{
 int ay_status = AY_OK;
 unsigned int count = 0, i = 0;
 int cc = 0;
 char *c1, *c2, *c3;
 double *da = NULL;
 float *fa = NULL;

  if(!tag)
    return AY_ENULL;

  if(tag->type != ay_pv_tagtype)
    return AY_ERROR;

  c1 = tag->val;

  /* find the type */
  c1 = strchr(c1, ',');
  c1++;
  c1 = strchr(c1, ',');
  if(!c1)
    return AY_ERROR;
  c1++;

  /* find the length */
  c2 = strchr(c1, ',');
  if(!c2)
    return AY_ERROR;
  c2++;
  sscanf(c2, "%u", &count);

  /* find the data */
  c3 = strchr(c2, ',');
  if(!c3)
    return AY_ERROR;
  /*c3++;*/

  switch(*c1)
    {
    case 'f':
      if(type == 0)
	{
	  /* allocate memory */
	  if(!(da = malloc(count*sizeof(double))))
	    return AY_EOMEM;
	  /* parse data and fill memory */
	  do
	    {
	      cc = sscanf(c3, ",%lg", &(da[i]));
	      if(cc != 1)
		{ay_status = AY_ERROR; goto cleanup;}
	      i++;
	      c3++;
	    }
	  while((c3 = strchr(c3, ',')));
	  /* prepare result */
	  *data = da;
	}
      if(type == 1)
	{
	  /* allocate memory */
	  if(!(fa = malloc(count*sizeof(float))))
	    return AY_EOMEM;
	  /* parse data and fill memory */
	  do
	    {
	      cc = sscanf(c3, ",%f", &(fa[i]));
	      if(cc != 1)
		{ay_status = AY_ERROR; goto cleanup;}
	      i++;
	      c3++;
	    }
	  while((c3 = strchr(c3, ',')));
	  /* prepare result */
	  *data = fa;
	}
      if(datalen)
	*datalen = count;
      break;

    case 'g':
      if(type == 0)
	{
	  /* allocate memory */
	  if(!(da = malloc(2*count*sizeof(double))))
	    return AY_EOMEM;
	  /* parse data and fill memory */
	  do
	    {
	      cc = sscanf(c3, ",%lg,%lg", &(da[i]), &(da[i+1]));
	      if(cc != 2)
		{ay_status = AY_ERROR; goto cleanup;}
	      i+=2;
	      c3++;
	      if(!(c3 = strchr(c3, ',')))
		break;
	      c3++;
	    }
	  while((c3 = strchr(c3, ',')));
	  /* prepare result */
	  *data = da;
	}
      if(type == 1)
	{
	  /* allocate memory */
	  if(!(fa = malloc(2*count*sizeof(float))))
	    return AY_EOMEM;
	  /* parse data and fill memory */
	  do
	    {
	      cc = sscanf(c3, ",%f,%f", &(fa[i]), &(fa[i+1]));
	      if(cc != 2)
		{ay_status = AY_ERROR; goto cleanup;}
	      i+=2;
	      c3++;
	      if(!(c3 = strchr(c3, ',')))
		break;
	      c3++;
	    }
	  while((c3 = strchr(c3, ',')));
	  /* prepare result */
	  *data = fa;
	}
      if(datalen)
	*datalen = count;
      break;

    case 'c':
    case 'p':
    case 'n':
    case 'v':
      if(type == 0)
	{
	  /* allocate memory */
	  if(!(da = malloc(3*count*sizeof(double))))
	    return AY_EOMEM;
	  /* parse data and fill memory */
	  do
	    {
	      cc = sscanf(c3, ",%lg,%lg,%lg",
			  &(da[i]), &(da[i+1]), &(da[i+2]));
	      if(cc != 3)
		{ay_status = AY_ERROR; goto cleanup;}
	      i+=3;
	      c3++;
	      if(!(c3 = strchr(c3, ',')))
		break;
	      c3++;
	      if(!(c3 = strchr(c3, ',')))
		break;
	      c3++;
	    }
	  while((c3 = strchr(c3, ',')));
	  /* prepare result */
	  *data = da;
	}
      if(type == 1)
	{
	  /* allocate memory */
	  if(!(fa = malloc(3*count*sizeof(float))))
	    return AY_EOMEM;
	  /* parse data and fill memory */
	  do
	    {
	      cc = sscanf(c3, ",%f,%f,%f", &(fa[i]), &(fa[i+1]), &(fa[i+2]));
	      if(cc != 3)
		{ay_status = AY_ERROR; goto cleanup;}
	      i+=3;
	      c3++;
	      if(!(c3 = strchr(c3, ',')))
		break;
	      c3++;
	      if(!(c3 = strchr(c3, ',')))
		break;
	      c3++;
	    }
	  while((c3 = strchr(c3, ',')));
	  /* prepare result */
	  *data = fa;
	}
      if(datalen)
	*datalen = count;
    break;

    case 'd':
      if(type == 0)
	{
	  /* allocate memory */
	  if(!(da = malloc(4*count*sizeof(double))))
	    return AY_EOMEM;
	  /* parse data and fill memory */
	  do
	    {
	      cc = sscanf(c3, ",%lg,%lg,%lg,%lg",
			  &(da[i]), &(da[i+1]), &(da[i+2]), &(da[i+3]));
	      if(cc != 4)
		{ay_status = AY_ERROR; goto cleanup;}
	      i+=4;
	      c3++;
	      if(!(c3 = strchr(c3, ',')))
		break;
	      c3++;
	      if(!(c3 = strchr(c3, ',')))
		break;
	      c3++;
	      if(!(c3 = strchr(c3, ',')))
		break;
	      c3++;
	    }
	  while((c3 = strchr(c3, ',')));
	  /* prepare result */
	  *data = da;
	}
      if(type == 1)
	{
	  /* allocate memory */
	  if(!(fa = malloc(4*count*sizeof(float))))
	    return AY_EOMEM;
	  /* parse data and fill memory */
	  do
	    {
	      cc = sscanf(c3, ",%f,%f,%f,%f",
			  &(fa[i]), &(fa[i+1]), &(fa[i+2]), &(fa[i+3]));
	      if(cc != 4)
		{ay_status = AY_ERROR; goto cleanup;}
	      i += 4;
	      c3++;
	      if(!(c3 = strchr(c3, ',')))
		break;
	      c3++;
	      if(!(c3 = strchr(c3, ',')))
		break;
	      c3++;
	      if(!(c3 = strchr(c3, ',')))
		break;
	      c3++;
	    }
	  while((c3 = strchr(c3, ',')));
	  /* prepare result */
	  *data = fa;
	}
      if(datalen)
	*datalen = count;
    break;

    default:
      ay_status = AY_ERROR;
      break;
    } /* switch */

  /* prevent cleanup code from doing something harmful */
  da = NULL;
  fa = NULL;

cleanup:
  if(fa)
    free(fa);

  if(da)
    free(da);

 return ay_status;
} /* ay_pv_convert */


/** ay_pv_getst:
 *  get texture coordinates (st) from PV tags
 *
 * \param[in] o object to get the texture coordinates from
 * \param[in] mys name of PV tag that contains the s component
 * \param[in] myt name of PV tag that contains the t component
 * \param[in,out] data texture coordinates
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_pv_getst(ay_object *o, char *mys, char *myt, void **data)
{
 int ay_status = AY_OK;
 ay_tag *tag = NULL, *stag = NULL, *ttag = NULL;
 double *sda = NULL, *tda = NULL;
 float *st = NULL;
 unsigned int sdalen = 0, tdalen = 0;
 unsigned int i, j;

  if(!o || !mys || !myt || !data)
    return AY_ENULL;

  tag = o->tags;
  while(tag)
    {
      if(tag->type == ay_pv_tagtype)
	{
	  if(!strncmp(tag->val, mys, strlen(mys)))
	    {
	      stag = tag;
	      if(ttag)
		break;
	    }
	  if(!strncmp(tag->val, myt, strlen(myt)))
	    {
	      ttag = tag;
	      if(stag)
		break;
	    }
	}
      tag = tag->next;
    }

  if(stag && ttag)
    {
      ay_pv_convert(stag, 0, &sdalen, (void**)(void*)&sda);
      ay_pv_convert(ttag, 0, &tdalen, (void**)(void*)&tda);

      if((sdalen == 0) || (tdalen == 0) || (sdalen != tdalen) ||
	 !sda || !tda)
	{ ay_status = AY_ERROR; goto cleanup; }

      if(!(st = malloc(2*sdalen*sizeof(float))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      j = 0;
      for(i = 0; i < sdalen; i++)
	{
	  st[j] = (float)sda[i];
	  j += 2;
	}
      j = 1;
      for(i = 0; i < sdalen; i++)
	{
	  st[j] = (float)tda[i];
	  j += 2;
	}

      *data = st;
    } /* if */

cleanup:

  if(sda)
    free(sda);

  if(tda)
    free(tda);

 return ay_status;
} /* ay_pv_getst */


/** ay_pv_getvc:
 *  get vertex colors from PV tag
 *  is able to work with PV tags of type 'c' and 'd'
 *  and expands/reduces the data accordingly
 *
 * \param[in] o object to get the vertex colors from
 * \param[in] myc name of PV tag
 * \param[in] stride desired output stride
 * \param[in,out] datalen length of data (in tuples)
 * \param[in,out] data vertex colors
 *
 * \returns AY_OK on success, error code otherwise.
 */
int
ay_pv_getvc(ay_object *o, char *myc, int stride, unsigned int *datalen,
	    void **data)
{
 int ay_status = AY_OK;
 ay_tag *tag = NULL, *ctag = NULL;
 float *cda = NULL;
 float *vc = NULL;
 int pvtype = -1;
 unsigned int cdalen = 0;
 unsigned int cdastride = 3, i, j = 0, k = 0;

  if(!o || !myc || !datalen || !data)
    return AY_ENULL;

  tag = o->tags;
  while(tag)
    {
      if(tag->type == ay_pv_tagtype)
	{
	  if(!strncmp(tag->val, myc, strlen(myc)))
	    {
	      ctag = tag;
		break;
	    }
	}
      tag = tag->next;
    }

  if(ctag)
    {
      pvtype = ay_pv_gettype(ctag);
      if(pvtype != 2 && pvtype != 6)
	{
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

      /* see if we can convert the tag data directly */
      if((pvtype == 6) && (stride == 4))
	{
	  /* yes */
	  ay_status = ay_pv_convert(ctag, 1, datalen, data);
	  goto cleanup;
	}

      if((pvtype == 2) && (stride == 3))
	{
	  /* yes */
	  ay_status = ay_pv_convert(ctag, 1, datalen, data);
	  goto cleanup;
	}

      /* can not convert directly, need to expand/reduce */
      ay_pv_convert(ctag, 1, &cdalen, (void**)(void*)&cda);

      if(cdalen == 0)
	{ ay_status = AY_ERROR; goto cleanup; }

      if(!(vc = malloc(stride*cdalen*sizeof(float))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      if(pvtype == 6)
	{
	  cdastride = 4;
	}

      for(i = 0; i < cdalen; i++)
	{
	  memcpy(&(vc[k]), &(cda[j]), 3*sizeof(float));

	  if(stride > 3)
	    {
	      vc[k+3] = 1.0;
	    }

	  k += stride;
	  j += cdastride;
	} /* for */

      /* return result */
      *data = vc;
      *datalen = cdalen;
    } /* if */

cleanup:

  if(cda)
    free(cda);

 return ay_status;
} /* ay_pv_getvc */


/** ay_pv_count:
 *  count PV tags of an object
 *
 * \param[in] o object to process
 *
 * \returns number of PV tags
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
	  /* 'd' counts twice because we expand it into Cs and Os
	     upon RIB export */
	  if(ay_pv_gettype(tag) == 6)
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
  ay_tags_register(interp, ay_pv_tagname, &ay_pv_tagtype);

 return;
} /* ay_pv_init */
