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
 *  XXXX leaks lots of memory in low mem situation
 */
int
ay_pv_filltokpar(ay_object *o, int declare, int start,
		 int *added, RtToken tokens[], RtPointer parms[])
{
 ay_tag *tag = NULL;
 char *tagvaltmp = NULL, *pvname, *pvstorage, *pvtype, *pvvalue, *numvals;
 char tok[] = ",";
 unsigned int i, j, n;
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

			    case 'g':
			      Tcl_DStringAppend(&ds, "float[2]", -1);

			      ftemp = NULL;
			      if(!(ftemp = calloc(n, 2*sizeof(RtFloat))))
				return AY_EOMEM;
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
			      if(!(ctemp = calloc(n, sizeof(RtColor))))
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
	  int datalen, int stride, void *data)
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
    case 1:
      /* float */
      Tcl_DStringAppend(&ds, "f", -1);
      break;
    case 2:
    case 3:
      /* point */
      Tcl_DStringAppend(&ds, "p", -1);
      break;
    case 4:
      /* float[2] */
      Tcl_DStringAppend(&ds, "g", -1);
      break;
    case 5:
      /* color */
      Tcl_DStringAppend(&ds, "c", -1);
      break;

    default:
      break;
    } /* switch */

  sprintf(tmp, ",%d", datalen);
  Tcl_DStringAppend(&ds, tmp, -1);

  switch(type)
    {
    case 0:
      /* float */
      for(i = 0; i < datalen*stride; i += stride)
	{
	  sprintf(tmp, ",%f", (float)(((double*)data)[i]));
	  Tcl_DStringAppend(&ds, tmp, -1);
	}
      break;
    case 1:
      /* float */
      for(i = 0; i < datalen*stride; i += stride)
	{
	  sprintf(tmp, ",%f", ((float*)data)[i]);
	  Tcl_DStringAppend(&ds, tmp, -1);
	}
      break;
    case 2:
      /* point */
      for(i = 0; i < datalen*stride; i += stride)
	{
	  sprintf(tmp, ",%f,%f,%f", (float)(((double*)data)[i]),
		  (float)(((double*)data)[i+1]),
		  (float)(((double*)data)[i+2]));
	  Tcl_DStringAppend(&ds, tmp, -1);
	}
      break;
    case 3:
      /* point */
      for(i = 0; i < datalen*stride; i += stride)
	{
	  sprintf(tmp, ",%f,%f,%f", ((float*)data)[i],
		  ((float*)data)[i+1],
		  ((float*)data)[i+2]);
	  Tcl_DStringAppend(&ds, tmp, -1);
	}
      break;
    case 4:
      /* float[2] */
      for(i = 0; i < datalen*stride; i += stride)
	{
	  sprintf(tmp, ",%f,%f", ((float*)data)[i], ((float*)data)[i+1]);
	  Tcl_DStringAppend(&ds, tmp, -1);
	}
      break;
    case 5:
      /* color */
      for(i = 0; i < datalen*stride; i += stride)
	{
	  sprintf(tmp, ",%f,%f,%f", ((float*)data)[i],
		  ((float*)data)[i+1],
		  ((float*)data)[i+2]);
	  Tcl_DStringAppend(&ds, tmp, -1);
	}
      break;
    default:
      break;
    } /* switch */

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

  for(i = 0; i < 3; i++)
    {
      c1 = strchr(c1, ',');

      if(!c1)
	return AY_FALSE;

      c1++;
    }

  if(!strncmp(t1->val, t2->val, (c1 - t1->val)))
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

  if(strstr(c, name) == c)
    {
      c = strchr(c, ',');
      if(!c)
	return AY_FALSE;

      c++;

      if(strstr(c, detail) == c)
	{
	  c = strchr(c, ',');
	  if(!c)
	    return AY_FALSE;

	  c++;

	  if(strstr(c, type) == c)
	    {
	      return AY_TRUE;
	    }
	}
    }
  
 return AY_FALSE;
} /* ay_pv_checkndt */


/* ay_pv_getdetail:
 *  returns detail from the PV tag <t>
 *  returns -1 on error
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

  if(strcmp(c, "constant"))
    result = 0;

  if(strcmp(c, "uniform"))
    result = 1;

  if(strcmp(c, "vertex"))
    result = 2;

  if(strcmp(c, "varying"))
    result = 3;

  if(detail)
    *detail = c;

 return result;
} /* ay_pv_getdetail */


/* ay_pv_convert:
 *  convert data from PV tag <tag> into an array of doubles (<type> 0)
 *  or floats (<type> 1)
 *  Warning: <datalen> does not contain the array size, but the number
 *  of data elements, e.g. for one color value it is 1 and not 3!
 */
int
ay_pv_convert(ay_tag *tag, int type, unsigned int *datalen, void **data)
{
 unsigned int count = 0, i = 0;
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
  sscanf(c2, "%d", &count);

  /* find the data */
  c3 = strchr(c2, ',');
  if(!c3)
    return AY_ERROR;
  c3++;

  switch(*c1)
    {
    case 'f':
      if(type == 0)
	{
	  /* allocate memory */
	  if(!(da = calloc(count, sizeof(double))))
	    return AY_EOMEM;
	  /* parse data and fill memory */
	  do
	    {
	      sscanf(c3, ",%lg", &(da[i]));
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
	  if(!(fa = calloc(count, sizeof(float))))
	    return AY_EOMEM;
	  /* parse data and fill memory */
	  do
	    {
	      sscanf(c3, ",%f", &(fa[i]));
	      i++;
	      c3++;
	    }
	  while((c3 = strchr(c3, ',')));
	  /* prepare result */
	  *data = fa;
	}
      *datalen = count;
      break;
    case 'g':
      if(type == 0)
	{
	  /* allocate memory */
	  if(!(da = calloc(2*count, sizeof(double))))
	    return AY_EOMEM;
	  /* parse data and fill memory */
	  do
	    {
	      sscanf(c3, ",%lg,%lg", &(da[i]), &(da[i+1]));
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
	  if(!(fa = calloc(2*count, sizeof(float))))
	    return AY_EOMEM;
	  /* parse data and fill memory */
	  do
	    {
	      sscanf(c3, ",%f,%f", &(fa[i]), &(fa[i+1]));
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
      *datalen = count;
      break;
    case 'c':
      if(type == 0)
	{
	  /* allocate memory */
	  if(!(da = calloc(3*count, sizeof(double))))
	    return AY_EOMEM;
	  /* parse data and fill memory */
	  do
	    {
	      sscanf(c3, ",%lg,%lg,%lg", &(da[i]), &(da[i+1]), &(da[i+2]));
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
	  if(!(fa = calloc(3*count, sizeof(float))))
	    return AY_EOMEM;
	  /* parse data and fill memory */
	  do
	    {
	      sscanf(c3, ",%f,%f,%f", &(fa[i]), &(fa[i+1]), &(fa[i+2]));
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
      *datalen = count;
    break;
    default:
      return AY_ERROR;
      break;
    } /* switch */

 return AY_OK;
} /* ay_pv_convert */


/* ay_pv_getst:
 *  get texture coordinates st
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
      ay_pv_convert(stag, 0, &sdalen, (void**)&sda);
      ay_pv_convert(ttag, 0, &tdalen, (void**)&tda);

      if((sdalen == 0) || (tdalen == 0) || (sdalen != tdalen))
	{ ay_status = AY_ERROR; goto cleanup; }

      if(!(st = calloc(2*sdalen, sizeof(float))))
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


/* ay_pv_getvc:
 *  get vertex colors
 */
int
ay_pv_getvc(ay_object *o, int stride, char *myc, void **data)
{
 int ay_status = AY_OK;
 ay_tag *tag = NULL, *ctag = NULL;
 float *cda = NULL;
 float *vc = NULL;
 unsigned int cdalen = 0;
 unsigned int i, j = 0;

  if(!o || !myc || !data)
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
      ay_pv_convert(ctag, 1, &cdalen, (void**)&cda);

      if(cdalen == 0)
	{ ay_status = AY_ERROR; goto cleanup; }

      if(!(vc = calloc(stride*cdalen, sizeof(float))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      for(i = 0; i < cdalen; i++)
	{
	  memcpy(&(vc[i]), &(cda[i]), 3*sizeof(float));
	  /*
	  vc[i] = cda[i];
	  vc[i+1] = cda[i+1];
	  vc[i+2] = cda[i+2];
	  */
	  if(stride > 3)
	    {
	      vc[i+3] = 1.0;
	      /* XXXX
		 vc[i+3] = oda[i+3];
	      */
	    }

	  j += stride;
	} /* for */

      *data = vc;
    } /* if */

cleanup:

  if(cda)
    free(cda);

 return ay_status;
} /* ay_pv_getvc */


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
  ay_tags_register(interp, ay_pv_tagname, &ay_pv_tagtype);

 return;
} /* ay_pv_init */
