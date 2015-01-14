/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2007 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

/** \file x3dio.c Ayam X3D Import/Export Plugin based on Expat/SCEW */

/* Ayam includes: */
#include "ayam.h"

/* SCEW includes (also pull in Expat includes): */
#include "scew.h"

#include <stdio.h>


/* local types: */

/** transformation stack */
typedef struct x3dio_trafostate_s {
  struct x3dio_trafostate_s *next; /**< next state in stack */

  double m[16]; /**< 4x4 transformation matrix */

} x3dio_trafostate;

/** X3D export callback type */
typedef int (x3dio_writecb) (scew_element *element, ay_object *o);


/* global variables: */

char x3dio_version_ma[] = AY_VERSIONSTR;
char x3dio_version_mi[] = AY_VERSIONSTRMI;

static Tcl_HashTable x3dio_write_ht;

static Tcl_HashTable *x3dio_defs_ht = NULL;

/* MN tags are used to temporarily store x3dio generated Master/DEF names */
char *x3dio_mn_tagtype = NULL;
char *x3dio_mn_tagname = "mn";


char *x3dio_truestring = "true";
char *x3dio_falsestring = "false";

/* current transformation */
x3dio_trafostate *x3dio_ctrafos = NULL;

/* import/export options: */
int x3dio_importcurves = AY_TRUE;
int x3dio_readstrim = AY_TRUE;
int x3dio_importviews = 0;
int x3dio_exportcurves = AY_TRUE;
int x3dio_expselected = AY_FALSE;
int x3dio_expobeynoexport = AY_TRUE;
int x3dio_expignorehidden = AY_TRUE;
int x3dio_mergeinlinedefs = AY_TRUE;

int x3dio_tesspomesh = AY_FALSE;
int x3dio_writecurves = AY_TRUE;
int x3dio_writeviews = AY_TRUE;
int x3dio_writeparam = AY_FALSE;
int x3dio_writemat = AY_FALSE;
int x3dio_writex3dom = AY_FALSE;
int x3dio_resolveinstances = AY_FALSE;

char *x3dio_x3domtemplate = "x3dom-template.xhtml";
int x3dio_x3domwidth = 0;
int x3dio_x3domheight = 0;

unsigned int x3dio_allobjcnt = 0;
unsigned int x3dio_curobjcnt = 0;

/* current transformation matrix */
static double tm[16] = {0};

/* rescale knots to min dist, if <= 0.0: no scaling */
double x3dio_rescaleknots = 0.0;

/* global scale factor */
double x3dio_scalefactor = 1.0;

/* total number of elements */
unsigned int x3dio_totalelements = 0;

/* number of read elements */
unsigned int x3dio_handledelements = 0;

/* progress counter (x3dio_handledelements/x3dio_totalelements) */
float x3dio_progress = 0.0f;

/* counter for nested USE attributes */
unsigned int x3dio_inuse = 0;

/* pointer to last read object */
ay_object *x3dio_lrobject = NULL;

/* prototypes of functions local to this module: */

/* low-level import support functions */
void x3dio_pushtrafo(void);

void x3dio_poptrafo(void);

void x3dio_cleartrafo(void);

int x3dio_readbool(scew_element *element, char *attrname, int *res);

int x3dio_readint(scew_element *element, char *attrname, int *res);

int x3dio_readintvec(scew_element *element, char *attrname,
		     unsigned int dim, int *res);

int x3dio_readfloat(scew_element *element, char *attrname, float *res);

int x3dio_readfloatvec(scew_element *element, char *attrname,
		       unsigned int dim, float *res);

int x3dio_readfloatpoints(scew_element *element, char *attrname,
			  unsigned int dim, unsigned int *len, float **res);

int x3dio_readdoublepoints(scew_element *element, char *attrname,
			   unsigned int dim, unsigned int *len, double **res);

int x3dio_readindex(scew_element *element, char *attrname, int clean,
		    unsigned int *len, int **res);

int x3dio_readboolvec(scew_element *element, char *attrname,
		      unsigned int *len, int **res);

int x3dio_processuse(scew_element **element);

int x3dio_readcoords(scew_element *element, unsigned int *len, double **res);

int x3dio_readtexcoords(scew_element *element, unsigned int *len, float **res);

int x3dio_readnormals(scew_element *element, unsigned int *len, double **res);

int x3dio_readcolors(scew_element *element, int *stride, unsigned int *len,
		     float **res);

int x3dio_readname(scew_element *element, const XML_Char *attn,
		   ay_object *obj);

int x3dio_linkobject(scew_element *element, unsigned int type, void *sobj);


/* 3D */
int x3dio_readbox(scew_element *element);

int x3dio_readsphere(scew_element *element);

int x3dio_readcylinder(scew_element *element);

int x3dio_readcone(scew_element *element);

void x3dio_readnct(scew_element *element, ay_object *o,
		   unsigned int totalverts);

int x3dio_readindexedfaceset(scew_element *element);

int x3dio_readindexedtriangleset(scew_element *element);

int x3dio_readindexedquadset(scew_element *element);

int x3dio_readindexedtrianglestripset(scew_element *element);

int x3dio_readindexedtrianglefanset(scew_element *element);

int x3dio_readindexedlineset(scew_element *element);

int x3dio_readlineset(scew_element *element);

int x3dio_readtrianglefanset(scew_element *element);

int x3dio_readtrianglestripset(scew_element *element);

int x3dio_readtriangleset(scew_element *element);

int x3dio_readquadset(scew_element *element);

int x3dio_readelevationgrid(scew_element *element);

int x3dio_getquatfromvec(double *v, double *q);

int x3dio_fixcrosssection(unsigned int cslen, float *cs,
			  unsigned int *newcslen, float **newcs);

int x3dio_getdiffspinepoint(unsigned int splen, float *sp, unsigned int sindex,
			    int stride, unsigned int *index);

int x3dio_getspinerots(unsigned int splen, float *sp, int sp_closed,
		   unsigned int orlen, float *or,
		   double **rots);

int x3dio_readextrusion(scew_element *element);


/* 2D */
int x3dio_readdisk2d(scew_element *element);

int x3dio_readcircle2d(scew_element *element);

int x3dio_readarc2d(scew_element *element);

int x3dio_readarcclose2d(scew_element *element);

int x3dio_readpolyline2d(scew_element *element, int contour);

/* NURBS */
int x3dio_readnurbscurve(scew_element *element, unsigned int dim);

int x3dio_fixnpatch(ay_nurbpatch_object *np);

int x3dio_readnurbspatchsurface(scew_element *element, int is_trimmed);

int x3dio_readnurbssweptsurface(scew_element *element);

int x3dio_readnurbsswungsurface(scew_element *element);

int x3dio_readnurbsset(scew_element *element);

/* Lights */
int x3dio_readlight(scew_element *element, int type);


/* non-geometric/scene structure */
int x3dio_readviewpoint(scew_element *element);

int x3dio_readcadelement(scew_element *element, int type);

int x3dio_readinline(scew_element *element);

int x3dio_readtransform(scew_element *element);

int x3dio_readshape(scew_element *element);

int x3dio_readscene(scew_element *element);

int x3dio_adddef(char *name, scew_element *element);

int x3dio_getdef(char *name, scew_element **element);

void x3dio_removedefs(scew_element *element);

void x3dio_countelements(scew_element *element, unsigned int *counter);

int x3dio_readelement(scew_element *element);

int x3dio_readtree(scew_tree *tree);

/* Tcl interface */
int x3dio_readtcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[]);

/****************************************************************************/

/* low-level export support functions */
scew_element* x3dio_findelement(scew_element const* parent,
				XML_Char const* name);

unsigned int x3dio_count(ay_object *o);

int x3dio_registerwritecb(char *name, x3dio_writecb *cb);

int x3dio_copypv(ay_tag *src, char **dst);

void x3dio_writetransform(scew_element *element, ay_object *o,
			  scew_element **transform_element);

void x3dio_clearmntags(ay_object *o);

int x3dio_writename(scew_element *element, ay_object *o, int trafo);

int x3dio_writeintattrib(scew_element *element, char *name, int *value);

int x3dio_writedoubleattrib(scew_element *element, char *name, double *value);

int x3dio_writedoublevecattrib(scew_element *element, char *name,
			       unsigned int dim, double *value);

int x3dio_writedoublepoints(scew_element *element, char *name,
			    unsigned int dim, unsigned int length,
			    unsigned int stride, double *value);

int x3dio_writeweights(scew_element *element, char *name,
		       unsigned int length, double *value);

int x3dio_writeknots(scew_element *element, char *name,
		     unsigned int length, double *value);

int x3dio_writencurve(scew_element *element, int dim, ay_nurbcurve_object *c);

/* export callback functions */
int x3dio_writencurveobj(scew_element *element, ay_object *o);

int x3dio_writencconvertibleobj(scew_element *element, ay_object *o);

int x3dio_writetrimcurve(scew_element *element, ay_object *o);

int x3dio_writetrimloop(scew_element *element, ay_object *o);

int x3dio_writenpatchobj(scew_element *element, ay_object *o);

int x3dio_writenpwire(scew_element *element, ay_object *o);

int x3dio_writetrimmednpwire(scew_element *element, ay_object *o);

int x3dio_writetrimwire(scew_element *element, ay_nurbpatch_object *np,
			ay_stess_uvp *p1, ay_stess_uvp *p2);

int x3dio_writenpconvertibleobj(scew_element *element, ay_object *o);

int x3dio_writelevelobj(scew_element *element, ay_object *o);

int x3dio_writecloneobj(scew_element *element, ay_object *o);

int x3dio_writeinstanceobj(scew_element *element, ay_object *o);

int x3dio_writescriptobj(scew_element *element, ay_object *o);

int x3dio_writeboxobj(scew_element *element, ay_object *o);

int x3dio_writesphereobj(scew_element *element, ay_object *o);

int x3dio_writecylinderobj(scew_element *element, ay_object *o);

int x3dio_writeconeobj(scew_element *element, ay_object *o);

int x3dio_writepomeshobj(scew_element *element, ay_object *o);

int x3dio_writepomeshwire(scew_element *element, ay_object *o);

int x3dio_writeview(scew_element *element, ay_object *o);

int x3dio_writelight(scew_element *element, ay_object *o);

int x3dio_writematerial(scew_element *shape_element, ay_object *o);

int x3dio_writerevolveobj(scew_element *element, ay_object *o);

int x3dio_writesweepobj(scew_element *element, ay_object *o);

int x3dio_writeswingobj(scew_element *element, ay_object *o);

int x3dio_writeextrudeobj(scew_element *element, ay_object *o);

/* export */

int x3dio_writeobject(scew_element *element, ay_object *o, int count);


int x3dio_writescene(char *filename, int selected, int toplevellayers);

/* Tcl interface */
int x3dio_writetcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[]);

/* functions: */

/* x3dio_pushtrafo:
 *  add a new state to the top of the transformation stack
 *  and initialize the new state with the identity transformation
 */
void
x3dio_pushtrafo(void)
{
 x3dio_trafostate *newstate = NULL;
 char fname[] = "x3dio_pushtrafo";

  if(!(newstate = calloc(1, sizeof(x3dio_trafostate))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return;
    }

  newstate->m[0] = 1.0;
  newstate->m[5] = 1.0;
  newstate->m[10] = 1.0;
  newstate->m[15] = 1.0;

  /* link new state to stack */
  newstate->next = x3dio_ctrafos;
  x3dio_ctrafos = newstate;

 return;
} /* x3dio_pushtrafo */


/* x3dio_poptrafo:
 *  remove topmost state from the transformation stack
 */
void
x3dio_poptrafo(void)
{
 x3dio_trafostate *nextstate = NULL;
 char fname[] = "x3dio_poptrafos";

  if(!x3dio_ctrafos)
    {
      ay_error(AY_ERROR, fname, "No states left!");
      return;
    }

  nextstate = x3dio_ctrafos->next;

  /* free toplevel state */
  free(x3dio_ctrafos);

  x3dio_ctrafos = nextstate;

 return;
} /* x3dio_poptrafo */


/* x3dio_cleartrafo:
 *  completely clear the transformation stack
 */
void
x3dio_cleartrafo(void)
{
 x3dio_trafostate *nextstate = NULL;

  while(x3dio_ctrafos)
    {
      nextstate = x3dio_ctrafos->next;
      free(x3dio_ctrafos);
      x3dio_ctrafos = nextstate;
    } /* while */

 return;
} /* x3dio_cleartrafo */


/* x3dio_readbool:
 *  get boolean attribute
 */
int
x3dio_readbool(scew_element *element, char *attrname, int *res)
{
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;

  if(!element || !attrname || !res)
    return AY_ENULL;

  attr = scew_attribute_by_name(element, attrname);
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  if(!strcmp(str, x3dio_truestring))
	    {
	      *res = 1;
	    }
	  else
	    {
	      if(!strcmp(str, x3dio_falsestring))
		{
		  *res = 0;
		}
	      else
		{
		  return AY_ERROR;
		} /* if */
	    } /* if */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
    }
  else
    {
      return AY_EWARN;
    } /* if */

 return AY_OK;
} /* x3dio_readbool */


/* x3dio_readint:
 *  get single integer value attribute
 */
int
x3dio_readint(scew_element *element, char *attrname, int *res)
{
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;

  if(!element || !attrname || !res)
    return AY_ENULL;

  attr = scew_attribute_by_name(element, attrname);
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  sscanf(str, "%d", res);
	}
      else
	{
	  return AY_ERROR;
	} /* if */
    }
  else
    {
      return AY_EWARN;
    } /* if */

 return AY_OK;
} /* x3dio_readint */


/* x3dio_readintvec:
 *  get int vector attribute
 */
int
x3dio_readintvec(scew_element *element, char *attrname,
		 unsigned int dim, int *res)
{
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;

  if(!element || !attrname || !res)
    return AY_ENULL;

  attr = scew_attribute_by_name(element, attrname);
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  switch(dim)
	    {
	    case 1:
	      sscanf(str, "%d", res);
	      break;
	    case 2:
	      sscanf(str, "%d %d", &(res[0]), &(res[1]));
	      break;
	    case 3:
	      sscanf(str, "%d %d %d", &(res[0]), &(res[1]), &(res[2]));
	      break;
	    case 4:
	      sscanf(str, "%d %d %d %d", &(res[0]), &(res[1]),
		     &(res[2]), &(res[3]));
	      break;
	    default:
	      return AY_ERROR;
	    } /* switch */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
    }
  else
    {
      return AY_EWARN;
    } /* if */

 return AY_OK;
} /* x3dio_readintvec */


/* x3dio_readfloat:
 *  get single float value attribute
 */
int
x3dio_readfloat(scew_element *element, char *attrname, float *res)
{
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;

  if(!element || !attrname || !res)
    return AY_ENULL;

  attr = scew_attribute_by_name(element, attrname);
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  sscanf(str, "%f", res);
	}
      else
	{
	  return AY_ERROR;
	} /* if */
    }
  else
    {
      return AY_EWARN;
    } /* if */

 return AY_OK;
} /* x3dio_readfloat */


/* x3dio_readfloatvec:
 *  get float vector attribute
 */
int
x3dio_readfloatvec(scew_element *element, char *attrname,
		   unsigned int dim, float *res)
{
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;

  if(!element || !attrname || !res)
    return AY_ENULL;

  attr = scew_attribute_by_name(element, attrname);
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  switch(dim)
	    {
	    case 1:
	      sscanf(str, "%f", res);
	      break;
	    case 2:
	      sscanf(str, "%f %f", &(res[0]), &(res[1]));
	      break;
	    case 3:
	      sscanf(str, "%f %f %f", &(res[0]), &(res[1]), &(res[2]));
	      break;
	    case 4:
	      sscanf(str, "%f %f %f %f", &(res[0]), &(res[1]),
		     &(res[2]), &(res[3]));
	      break;
	    default:
	      return AY_ERROR;
	    } /* switch */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
    }
  else
    {
      return AY_EWARN;
    } /* if */

 return AY_OK;
} /* x3dio_readfloatvec */


/* x3dio_readfloatpoints:
 *  get float point vector attribute
 */
int
x3dio_readfloatpoints(scew_element *element, char *attrname,
		      unsigned int dim, unsigned int *len, float **res)
{
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL, *p;
 float *tmp = NULL, *fp;
 unsigned int i, j = 0;

  if(!element || !attrname || !len || !res)
    return AY_ENULL;

  *len = 0;
  *res = NULL;

  attr = scew_attribute_by_name(element, attrname);
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  p = str;
	  /* count words */
	  while(*p != '\0')
	    {
	      if(*p == ' ' || *p == ',')
		j++;
	      p++;
	    }
	  if(!(*res = malloc(j*sizeof(float))))
	    return AY_EOMEM;
	  p = str;
	  while(*p != '\0')
	    {
	      if(j < (*len+1)*dim)
		{
		  if(!(tmp = realloc(*res, (*len+1)*dim*sizeof(float))))
		    {
		      if(*res)
			free(*res);
		      *res = NULL;
		      *len = 0;
		      return AY_EOMEM;
		    }
		  *res = tmp;
		}
	      fp = &((*res)[(*len)*dim]);
	      (*len)++;

	      for(i = 0; i < dim; i++)
		{
		  /* forward p to next float */
		  /* jump over leading whitespace */
		  while(isspace(*p) && (*p != '\0'))
		    {
		      p++;
		    }

		  /* check for (premature) end of string */
		  if(*p == '\0')
		    {
		      /* if we did not read a complete point, but already
			 encounter the end of the string, we need to
			 correct the number of (complete) points read */
		      (*len)--;
		      break;
		    }

		  sscanf(p, "%f", fp);

		  fp++;

		  /* jump over the float we just read */
		  while(!isspace(*p) && (*p != '\0'))
		    {
		      p++;
		    }
		} /* for */
	    } /* while */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
    }
  else
    {
      return AY_EWARN;
    } /* if */

 return AY_OK;
} /* x3dio_readfloatpoints */


/* x3dio_readdoublepoints:
 *  get double point vector attribute
 */
int
x3dio_readdoublepoints(scew_element *element, char *attrname,
		       unsigned int dim, unsigned int *len, double **res)
{
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL, *p;
 double *tmp = NULL, *fp;
 unsigned int i, j = 0;

  if(!element || !attrname || !len || !res)
    return AY_ENULL;

  *len = 0;
  *res = NULL;

  attr = scew_attribute_by_name(element, attrname);
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  p = str;
	  /* count words */
	  while(*p != '\0')
	    {
	      if(*p == ' ' || *p == ',')
		j++;
	      p++;
	    }
	  if(!(*res = malloc(j*sizeof(double))))
	    return AY_EOMEM;
	  p = str;
	  while(*p != '\0')
	    {
	      if(j < (*len+1)*dim)
		{
		  if(!(tmp = realloc(*res, (*len+1)*dim*sizeof(double))))
		    {
		      if(*res)
			free(*res);
		      *res = NULL;
		      *len = 0;
		      return AY_EOMEM;
		    }
		  *res = tmp;
		}
	      fp = &((*res)[(*len)*dim]);
	      (*len)++;

	      for(i = 0; i < dim; i++)
		{
		  /* forward p to next double */
		  /* jump over leading whitespace */
		  while(isspace(*p) && (*p != '\0'))
		    {
		      p++;
		    }

		  /* check for (premature) end of string */
		  if(*p == '\0')
		    {
		      /* if we did not read a complete point, but already
			 encounter the end of the string, we need to
			 correct the number of (complete) points read */
		      (*len)--;
		      break;
		    }

		  sscanf(p, "%lg", fp);

		  fp++;

		  /* jump over the double we just read */
		  while(!isspace(*p) && (*p != '\0'))
		    {
		      p++;
		    }
		} /* for */
	    } /* while */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
    }
  else
    {
      return AY_EWARN;
    } /* if */

 return AY_OK;
} /* x3dio_readdoublepoints */


/* x3dio_readindex:
 *  read a vector of integer values (e.g. for the coordinateIndex
 *  of an IndexedFaceSet)
 */
int
x3dio_readindex(scew_element *element, char *attrname,
		int clean, unsigned int *len, int **res)
{
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL, *p;
 int *tmp = NULL, *ip;
 unsigned int j = 0;

  if(!element || !attrname || !len || !res)
    return AY_ENULL;

  *len = 0;
  *res = NULL;

  attr = scew_attribute_by_name(element, attrname);
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  p = str;
	  /* count words */
	  while(*p != '\0')
	    {
	      if(*p == ' ' || *p == ',')
		j++;
	      p++;
	    }
	  if(!(*res = malloc(j*sizeof(int))))
	    return AY_EOMEM;
	  p = str;
	  while(*p != '\0')
	    {
	      if(j < (*len+1))
		{
		  if(!(tmp = realloc(*res, (*len+1)*sizeof(int))))
		    {
		      if(*res)
			free(*res);
		      *res = NULL;
		      *len = 0;
		      return AY_EOMEM;
		    }
		  *res = tmp;
		}
	      ip = &((*res)[(*len)]);
	      (*len)++;

	      /* forward p to next integer */
	      /* jump over leading whitespace */
	      while(isspace(*p) && (*p != '\0'))
		{
		  p++;
		}

	      /* check for (premature) end of string */
	      if(*p == '\0')
		{
		  (*len)--;
		  break;
		}

	      sscanf(p, "%d", ip);

	      if(clean)
		{
		  if(*ip == -1)
		    (*len)--;
		}

	      /*ip++;*/

	      /* jump over the integer we just read */
	      while(!isspace(*p) && (*p != '\0'))
		{
		  p++;
		}
	    } /* while */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
    }
  else
    {
      return AY_EWARN;
    } /* if */

 return AY_OK;
} /* x3dio_readindex */


/* x3dio_readboolvec:
 *  read a vector of boolean values
 */
int
x3dio_readboolvec(scew_element *element, char *attrname,
		  unsigned int *len, int **res)
{
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL, *p;
 int *tmp = NULL, *ip;
 unsigned int j = 0;

  if(!element || !attrname || !len || !res)
    return AY_ENULL;

  *len = 0;
  *res = NULL;

  attr = scew_attribute_by_name(element, attrname);
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  p = str;
	  /* count words */
	  while(*p != '\0')
	    {
	      if(*p == ' ' || *p == ',')
		j++;
	      p++;
	    }
	  if(!(*res = malloc(j*sizeof(int))))
	    return AY_EOMEM;
	  p = str;
	  while(*p != '\0')
	    {
	      if(j < (*len+1))
		{
		  if(!(tmp = realloc(*res, (*len+1)*sizeof(int))))
		    {
		      if(*res)
			free(*res);
		      *res = NULL;
		      *len = 0;
		      return AY_EOMEM;
		    }
		  *res = tmp;
		}
	      ip = &((*res)[(*len)]);
	      (*len)++;

	      /* forward p to next integer */
	      /* jump over leading whitespace */
	      while(isspace(*p) && (*p != '\0'))
		{
		  p++;
		}

	      /* check for (premature) end of string */
	      if(*p == '\0')
		{
		  (*len)--;
		  break;
		}

	      if(strcmp(x3dio_truestring, p))
		{
		  *ip = AY_FALSE;
		}
	      else
		{
		  *ip = AY_TRUE;
		}

	      ip++;

	      /* jump over the integer we just read */
	      while(!isspace(*p) && (*p != '\0'))
		{
		  p++;
		}
	    } /* while */
	}
      else
	{
	  return AY_ERROR;
	} /* if */
    }
  else
    {
      return AY_EWARN;
    } /* if */

 return AY_OK;
} /* x3dio_readboolvec */


/* x3dio_processuse:
 *
 */
int
x3dio_processuse(scew_element **element)
{
 int ay_status = AY_OK;
 char fname[] = "x3dio_processuse", *errstr = NULL;
 const char *errfmt = "could not find element: %s";
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;

  if(!element || !*element)
    return AY_ENULL;

  attr = scew_attribute_by_name(*element, "USE");
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  ay_status = x3dio_getdef((char*)str, element);
	  if(ay_status)
	    {
	      if(!(errstr = calloc(strlen(errfmt) + strlen(str) + 2,
				   sizeof(char))))
		{
		  ay_error(AY_ERROR, fname, NULL);
		}
	      else
		{
		  sprintf(errstr, errfmt, str);
		  ay_error(AY_ERROR, fname, errstr);
		  free(errstr);
		}
	      return AY_ERROR;
	    } /* if */
	}
      else
	{
	  ay_error(AY_ERROR, fname, "malformed USE attribute encountered");
	} /* if str */
    } /* if attr */

 return AY_OK;
} /* x3dio_processuse */


/* x3dio_readcoords:
 *  look through all children of <element> for a "Coordinate" or
 *  "CoordinateDouble" element and read the coordinates into <len> and <res>
 */
int
x3dio_readcoords(scew_element *element, unsigned int *len, double **res)
{
 int ay_status = AY_OK;
 scew_element *child = NULL;
 const char *element_name = NULL;
 float *cv = NULL;
 unsigned int i;

  if(!element || !len || !res)
    return AY_ENULL;

  while((child = scew_element_next(element, child)) != NULL)
    {
      element_name = scew_element_name(child);
      if(!strcmp(element_name, "Coordinate"))
	{
	  /* process USE attribute */
	  ay_status = x3dio_processuse(&child);

	  if(ay_status)
	    return AY_ERROR;

	  /* read data */
	  ay_status = x3dio_readfloatpoints(child, "point", 3, len, &cv);
	  if(*len)
	    {
	      if(!(*res = calloc((*len)*3, sizeof(double))))
		{
		  if(cv)
		    free(cv);
		  return AY_EOMEM;
		}
	      for(i = 0; i < (*len)*3; i++)
		{
		  (*res)[i] = (double)cv[i];
		}

	      if(cv)
		free(cv);
	    } /* if */
	  return AY_OK; /* XXXX early exit! */
	}
      if(!strcmp(element_name, "CoordinateDouble"))
	{
	  /* process USE attribute */
	  ay_status = x3dio_processuse(&child);

	  if(ay_status)
	    return AY_ERROR;

	  /* read data */
	  ay_status = x3dio_readdoublepoints(child, "point", 3, len, res);

	  return ay_status; /* XXXX early exit! */
	}
    } /* while */

 return AY_OK;
} /* x3dio_readcoords */


/* x3dio_readtexcoords:
 *  look through all children of <element> for a "TextureCoordinate"
 *  element and read the coordinates into <len> and <res>
 */
int
x3dio_readtexcoords(scew_element *element, unsigned int *len, float **res)
{
 int ay_status = AY_OK;
 scew_element *child = NULL;
 const char *element_name = NULL;

  if(!element || !len || !res)
    return AY_ENULL;

  while((child = scew_element_next(element, child)) != NULL)
    {
      element_name = scew_element_name(child);
      if(!strcmp(element_name, "TextureCoordinate"))
	{
	  /* process USE attribute */
	  ay_status = x3dio_processuse(&child);

	  if(ay_status)
	    return AY_ERROR;

	  /* read data */
	  ay_status = x3dio_readfloatpoints(child, "point", 2, len, res);

	  return ay_status; /* XXXX early exit! */
	}
    } /* while */

 return AY_OK;
} /* x3dio_readtexcoords */


/* x3dio_readnormals:
 *  look through all children of <element> for a "Normal" element and
 *  read the normals therein into <len> and <res>
 */
int
x3dio_readnormals(scew_element *element, unsigned int *len, double **res)
{
 int ay_status = AY_OK;
 scew_element *child = NULL;
 const char *element_name = NULL;
 float *cv = NULL;
 unsigned int i;

  if(!element || !len || !res)
    return AY_ENULL;

  while((child = scew_element_next(element, child)) != NULL)
    {
      element_name = scew_element_name(child);
      if(!strcmp(element_name, "Normal"))
	{
	  /* process USE attribute */
	  ay_status = x3dio_processuse(&child);

	  if(ay_status)
	    return AY_ERROR;

	  /* read data */
	  ay_status = x3dio_readfloatpoints(child, "vector", 3, len, &cv);
	  if(*len)
	    {
	      if(!(*res = calloc((*len)*3, sizeof(double))))
		{
		  if(cv)
		    free(cv);
		  return AY_EOMEM;
		}
	      for(i = 0; i < *len; i++)
		{
		  (*res)[i] = (double)cv[i];
		}

	      if(cv)
		free(cv);
	    } /* if */
	  return AY_OK; /* XXXX early exit! */
	} /* if */
    } /* while */

  return AY_OK;
} /* x3dio_readnormals */


/* x3dio_readcolors:
 *  look through all children of <element> for a "Color" element and
 *  read the colors therein into <len> and <res>
 */
int
x3dio_readcolors(scew_element *element, int *stride, unsigned int *len, float **res)
{
 int ay_status = AY_OK;
 scew_element *child = NULL;
 const char *element_name = NULL;

  if(!element || !len || !res)
    return AY_ENULL;

  while((child = scew_element_next(element, child)) != NULL)
    {
      element_name = scew_element_name(child);
      if(!strcmp(element_name, "Color"))
	{
	  /* process USE attribute */
	  ay_status = x3dio_processuse(&child);

	  if(ay_status)
	    return AY_ERROR;

	  /* read data */
	  ay_status = x3dio_readfloatpoints(child, "color", 3, len, res);

	  *stride = 3;

	  goto cleanup;
	} /* if */

      if(!strcmp(element_name, "ColorRGBA"))
	{
	  /* process USE attribute */
	  ay_status = x3dio_processuse(&child);

	  if(ay_status)
	    return AY_ERROR;

	  /* read data */
	  ay_status = x3dio_readfloatpoints(child, "color", 4, len, res);

	  *stride = 4;

	  goto cleanup;
	} /* if */
    } /* while */

cleanup:

 return ay_status;
} /* x3dio_readcolors */


/* x3dio_readname:
 *
 */
int
x3dio_readname(scew_element *element, const XML_Char *attn, ay_object *obj)
{
 int ay_status = AY_OK;
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;
 size_t len = 0;
 char *c;

  if(!element || !attn || !obj)
    return AY_ENULL;

  /* set name from attn attribute */
  attr = scew_attribute_by_name(element, attn);
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  len = strlen(str)+1;
	  if((obj->name = calloc(len, sizeof(char))))
	    {
	      strcpy(obj->name, str);
	      /* remove whitespace */
	      c = obj->name;
	      while(c && *c != '\0')
		{
		  if(*c == ' ')
		    *c = '_';
		  c++;
		} /* while */
	    } /* if */
	} /* if */
    } /* if */

 return ay_status;
} /* x3dio_readname */


/* x3dio_linkobject:
 *
 */
int
x3dio_linkobject(scew_element *element, unsigned int type, void *sobj)
{
 int ay_status = AY_OK;
 ay_object obj = {0}, *new = NULL;

  if(!sobj)
    return AY_ENULL;

  ay_object_defaults(&obj);

  obj.type = type;
  obj.refine = sobj;

  /* make a copy to be linked to the scene */
  ay_status = ay_object_copy(&obj, &new);

  if(ay_status || !new)
    return ay_status;

  /* set transformation attributes */
  ay_trafo_decomposematrix(x3dio_ctrafos->m, new);

  /* set name from DEF */
  ay_status = x3dio_readname(element, "DEF", new);

  /* link the object to the scene */
  ay_object_link(new);

  if(type == AY_IDPAMESH)
    {
      (void)ay_notify_object(new);
    }

  x3dio_lrobject = new;

 return ay_status;
} /* x3dio_linkobject */


/* x3dio_readbox:
 *
 */
int
x3dio_readbox(scew_element *element)
{
 int ay_status = AY_OK;
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;
 ay_box_object box = {0};
 float w = 0.0f, h = 0.0f, l = 0.0f;

  if(!element)
    return AY_ENULL;

  attr = scew_attribute_by_name(element, "size");
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  sscanf(str, "%f %f %f", &w, &h, &l);
	  box.width  = (double)w;
	  box.height = (double)h;
	  box.length = (double)l;
	}
      else
	{
	  return AY_ERROR;
	} /* if */
    }
  else
    {
      box.width  = 2.0;
      box.height = 2.0;
      box.length = 2.0;
    } /* if */

  ay_status = x3dio_linkobject(element, AY_IDBOX, (void*)&box);

 return ay_status;
} /* x3dio_readbox */


/* x3dio_readsphere:
 *
 */
int
x3dio_readsphere(scew_element *element)
{
 int ay_status = AY_OK;
 ay_sphere_object sphere = {0};
 float radius = 1.0f;

  if(!element)
    return AY_ENULL;

  sphere.radius = 1.0;
  x3dio_readfloat(element, "radius", &radius);
  if(radius > 0.0f)
    {
      sphere.radius = (double)radius;
    }
  else
    {
      return AY_ERROR;
    }

  sphere.zmin = sphere.radius;
  sphere.zmax = sphere.radius;
  sphere.thetamax = 360.0;
  sphere.is_simple = AY_TRUE;

  /* add an additional rotation to fix the orientation */

  x3dio_pushtrafo();

  ay_trafo_rotatematrix(-90.0, 1.0, 0.0, 0.0, x3dio_ctrafos->m);

  /* copy object to the Ayam scene */
  ay_status = x3dio_linkobject(element, AY_IDSPHERE, (void*)&sphere);

  x3dio_poptrafo();

 return ay_status;
} /* x3dio_readsphere */


/* x3dio_readcylinder:
 *
 */
int
x3dio_readcylinder(scew_element *element)
{
 int ay_status = AY_OK;
 ay_cylinder_object cylinder = {0};
 ay_disk_object disk = {0};
 float radius = 1.0f;
 float height = 2.0f;
 int has_side = 1, has_top = 1, has_bottom = 1;

  if(!element)
    return AY_ENULL;

  cylinder.radius = 1.0;
  x3dio_readfloat(element, "radius", &radius);
  if(radius > 0.0f)
    {
      cylinder.radius = (double)radius;
    }
  else
    {
      return AY_ERROR;
    }

  cylinder.zmin = -1.0;
  cylinder.zmax = 1.0;
  x3dio_readfloat(element, "height", &height);
  if(height > 0.0f)
    {
      cylinder.zmin = (double)-height/2.0f;
      cylinder.zmax = (double)height/2.0f;
    }
  else
    {
      return AY_ERROR;
    }

  cylinder.thetamax = 360.0;
  cylinder.is_simple = AY_TRUE;

  ay_status = x3dio_readbool(element, "side", &has_side);

  ay_status = x3dio_readbool(element, "top", &has_top);

  ay_status = x3dio_readbool(element, "bottom", &has_bottom);


  /* add an additional rotation to fix the orientation */
  x3dio_pushtrafo();

  ay_trafo_rotatematrix(-90.0, 1.0, 0.0, 0.0, x3dio_ctrafos->m);

  if(has_side)
    {
      if(has_top && has_bottom)
	{
	  cylinder.closed = AY_TRUE;
	}
      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDCYLINDER, (void*)&cylinder);
    }

  if(has_top && !has_bottom)
    {
      disk.is_simple = AY_TRUE;
      disk.radius = cylinder.radius;
      disk.height = cylinder.zmin;
      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDDISK, (void*)&disk);
    }

  if(has_bottom && !has_top)
    {
      disk.is_simple = AY_TRUE;
      disk.radius = cylinder.radius;
      disk.height = cylinder.zmax;
      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDDISK, (void*)&disk);
    }

  x3dio_poptrafo();

 return ay_status;
} /* x3dio_readcylinder */


/* x3dio_readcone:
 *
 */
int
x3dio_readcone(scew_element *element)
{
 int ay_status = AY_OK;
 ay_cone_object cone = {0};
 ay_disk_object disk = {0};
 float radius = 1.0f;
 float height = 2.0f;
 int has_side = 1, has_bottom = 1;
 double translate_y = 0.0;

  if(!element)
    return AY_ENULL;

  cone.radius = 1.0;
  x3dio_readfloat(element, "bottomRadius", &radius);
  if(radius > 0.0f)
    {
      cone.radius = (double)radius;
    }
  else
    {
      return AY_ERROR;
    }

  cone.height = 2.0;
  translate_y = -1.0;
  x3dio_readfloat(element, "height", &height);
  if(height > 0.0f)
    {
      cone.height = (double)height;
      translate_y = (double)-height/2.0f;
    }
  else
    {
      return AY_ERROR;
    }

  cone.thetamax = 360.0;
  cone.is_simple = AY_TRUE;

  ay_status = x3dio_readbool(element, "side", &has_side);

  ay_status = x3dio_readbool(element, "bottom", &has_bottom);


  x3dio_pushtrafo();

  /* accomodate for height/position difference */
  ay_trafo_translatematrix(0.0, translate_y, 0.0, x3dio_ctrafos->m);

  /* add an additional rotation to fix the orientation */
  ay_trafo_rotatematrix(-90.0, 1.0, 0.0, 0.0, x3dio_ctrafos->m);

  if(has_side)
    {
      if(has_bottom)
	{
	  cone.closed = AY_TRUE;
	}
      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDCONE, (void*)&cone);
    }

  if(has_bottom && !has_side)
    {
      disk.is_simple = AY_TRUE;
      disk.radius = cone.radius;
      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDDISK, (void*)&disk);
    }

  x3dio_poptrafo();

 return ay_status;
} /* x3dio_readcone */


/* x3dio_readnct:
 *  read normals, colors, and texture coordinates
 *  helper for PolyMesh creating elements
 *  (IndexedFaceSet, IndexedTriangleSet etc.)
 */
void
x3dio_readnct(scew_element *element, ay_object *o, unsigned int totalverts)
{
 int ay_status = AY_OK;
 char *tcname = ay_prefs.texcoordname;
 char *nname = ay_prefs.normalname;
 char *cname = ay_prefs.colorname;
 ay_pomesh_object *pomesh = NULL;
 int colorstride = 0, vertnormal = AY_TRUE, vertcolor = AY_TRUE;
 unsigned int normallen = 0, normalilen = 0;
 unsigned int colorlen = 0, colorilen = 0;
 unsigned int texcoordlen = 0, texcoordilen = 0;
 double *normals = NULL;
 float *texcoords = NULL, *colors = NULL;
 int *normali = NULL, *colori = NULL, *texcoordi = NULL;
 int expandcolors = AY_FALSE;
 unsigned int i, stride = 3;
 double *expandedcontrols = NULL;
 double *expandednormals = NULL;
 float *expandedtexcoords = NULL;
 float *expandedcolors = NULL;

  if(!o || (o->type != AY_IDPOMESH))
    return;

  pomesh = (ay_pomesh_object *)(o->refine);

  /* get data from element */

  /* get normals */
  ay_status = x3dio_readnormals(element, &normallen, &normals);
  if(normallen > 0)
    {
      ay_status = x3dio_readindex(element, "normalIndex", AY_TRUE,
				  &normalilen, &normali);

      ay_status = x3dio_readbool(element, "normalPerVertex",
				 &vertnormal);
    }

  /* get colors */
  ay_status = x3dio_readcolors(element, &colorstride, &colorlen, &colors);
  if(colorlen > 0)
    {
      ay_status = x3dio_readindex(element, "colorIndex", AY_TRUE,
				  &colorilen, &colori);

      ay_status = x3dio_readbool(element, "colorPerVertex",
				 &vertcolor);
    }

  /* get texture coordinates */
  ay_status = x3dio_readtexcoords(element, &texcoordlen, &texcoords);
  if(texcoordlen > 0)
    {
      ay_status = x3dio_readindex(element, "texCoordIndex", AY_TRUE,
				  &texcoordilen, &texcoordi);

    }

  /* process data */

  /* first check, whether we need to expand the vertex coordinate data */
  if((vertnormal && (normalilen > 0)) ||
     (vertcolor && (colorilen > 0)) ||
     (texcoordilen > 0))
    {
      /* colors may not need to be expanded if defined per face */
      if(vertcolor)
	expandcolors = AY_TRUE;

      /* expand verts and auxiliary data that needs to be expanded */
      if((normallen > 0) && vertnormal)
	{
	  pomesh->has_normals = AY_TRUE;
	  stride = 6;
	}
      else
	{
	  if(normallen > 0)
	    {
	      /* add face normals */
	      if(normalilen > 0)
		{
		  /*
		    we have a normal index,
		    better rewrite/expand the normals
		    so that every face gets its own distinct value
		  */
		  if(!(expandednormals = malloc(3*pomesh->npolys*
						sizeof(double))))
		    { goto cleanup; }
		  for(i = 0; i < pomesh->npolys; i++)
		    {
		      memcpy(&(expandednormals[i*3]), &(normals[normali[i]*3]),
			     3*sizeof(double));
		    }

		  ay_pv_add(o, nname, "uniform", "n",
			    pomesh->npolys, 3, expandednormals);

		  free(expandednormals);
		}
	      else
		{
		  ay_pv_add(o, nname, "uniform", "n",
			    pomesh->npolys, 3, normals);
		}
	    }
	}

      /* process control points and mix in the vertex normals */
      if(!(expandedcontrols = malloc(stride*totalverts*sizeof(double))))
	{ goto cleanup; }

      for(i = 0; i < totalverts; i++)
	{
	  memcpy(&(expandedcontrols[i*stride]),
		 &(pomesh->controlv[pomesh->verts[i]*3]),
		 3*sizeof(double));
	  if(pomesh->has_normals)
	    {
	      if(normalilen > 0)
		memcpy(&(expandedcontrols[i*stride+3]),
		       &(normals[normali[i]*3]),
		       3*sizeof(double));
	      else
		memcpy(&(expandedcontrols[i*stride+3]),
		       &(normals[pomesh->verts[i]*3]),
		       3*sizeof(double));
	    }
	}

      /* process colors */
      if(colorlen > 0)
	{
	  if(expandcolors)
	    {
	      /* vertex colors */
	      if(!(expandedcolors = malloc(colorstride*totalverts*
					   sizeof(float))))
		{ goto cleanup; }
	      if(colorilen > 0)
		{
		  for(i = 0; i < totalverts; i++)
		    {
		      memcpy(&(expandedcolors[i*colorstride]),
			     &(colors[pomesh->verts[i]*colorstride]),
			     colorstride*sizeof(float));
		    }
		}
	      else
		{
		  for(i = 0; i < totalverts; i++)
		    {
		      memcpy(&(expandedcolors[i*colorstride]),
			     &(colors[colori[i]*colorstride]),
			     colorstride*sizeof(float));
		    }
		}
	      if(colorstride == 3)
		ay_pv_add(o, cname, "varying", "c", totalverts, 3,
			  expandedcolors);
	      else
		ay_pv_add(o, cname, "varying", "d", totalverts, 4,
			  expandedcolors);

	      free(expandedcolors);
	    }
	  else
	    {
	      /* face colors */
	      if(colorstride == 3)
		ay_pv_add(o, cname, "uniform", "c", pomesh->npolys, 3,
			  colors);
	      else
		ay_pv_add(o, cname, "uniform", "d", pomesh->npolys, 4,
			  colors);
	    }
	} /* if */

      /* process texcoords */
      if(texcoordlen > 0)
	{
	  if(!(expandedtexcoords = malloc(2*totalverts*sizeof(float))))
	    { goto cleanup; }

	  if(texcoordilen > 0)
	    {
	      for(i = 0; i < totalverts; i++)
		{
		  memcpy(&(expandedtexcoords[i*2]),
			 &(texcoords[texcoordi[i]*2]),
			 2*sizeof(float));

		}
	    }
	  else
	    {
	      for(i = 0; i < totalverts; i++)
		{
		  memcpy(&(expandedtexcoords[i*2]),
			 &(texcoords[pomesh->verts[i]*2]),
			 2*sizeof(float));

		}
	    }

	  ay_pv_add(o, tcname, "varying", "g",
		    totalverts, 2, expandedtexcoords);

	  free(expandedtexcoords);
	} /* if */

      for(i = 0; i < totalverts; i++)
	{
	  pomesh->verts[i] = i;
	}
      free(pomesh->controlv);
      pomesh->controlv = expandedcontrols;
      expandedcontrols = NULL;
      pomesh->ncontrols = totalverts;
    }
  else
    {
      if(normallen > 0)
	{
	  if(vertnormal)
	    {
	      /* vertex normals */
	      /* no need to check for an index, we ruled that out already */
	      if(!(expandedcontrols = malloc(6*pomesh->ncontrols*
					     sizeof(double))))
		{ goto cleanup; }
	      for(i = 0; i < pomesh->npolys; i++)
		{
		  memcpy(&(expandedcontrols[i*6]), &(pomesh->controlv[i*3]),
			 3*sizeof(double));
		  memcpy(&(expandedcontrols[i*6+3]), &(normals[i*3]),
			 3*sizeof(double));
		}
	      free(pomesh->controlv);
	      pomesh->controlv = expandedcontrols;
	      expandedcontrols = NULL;
	      pomesh->has_normals = AY_TRUE;
	    }
	  else
	    {
	      /* face normals */
	      if(normalilen > 0)
		{
		  /*
		    we have a normal index,
		    better rewrite/expand the normals
		    so that every face gets its own distinct value
		  */
		  if(!(expandednormals = malloc(3*pomesh->npolys*
						sizeof(double))))
		    { goto cleanup; }
		  for(i = 0; i < pomesh->npolys; i++)
		    {
		      memcpy(&(expandednormals[i*3]),
			     &(normals[normali[i]*3]),
			     3*sizeof(double));
		    }

		  ay_pv_add(o, nname, "uniform", "n",
			    pomesh->npolys, 3, expandednormals);

		  free(expandednormals);
		}
	      else
		{
		  /* no normal index */
		  ay_pv_add(o, nname, "uniform", "n",
			    pomesh->npolys, 3, normals);
		}
	    }
	} /* if */

      if(colorlen > 0)
	{
	  if(vertcolor)
	    {
	      /* vertex colors */
	      /* no need to check for an index, we ruled that out already */
	      if(colorstride == 3)
		ay_pv_add(o, cname, "varying", "c",
			  pomesh->ncontrols, 3, colors);
	      else
		ay_pv_add(o, cname, "varying", "d",
			  pomesh->ncontrols, 4, colors);
	    }
	  else
	    {
	      /* face colors */
	      if(colorilen > 0)
		{
		  /*
		    we have a normal index,
		    better rewrite/expand the colors
		    so that every face gets its own distinct value
		  */
		  if(!(expandedcolors = malloc(colorstride*pomesh->npolys*
					       sizeof(float))))
		    { goto cleanup; }
		  for(i = 0; i < pomesh->npolys; i++)
		    {
		      memcpy(&(expandedcolors[i*colorstride]),
			     &(colors[colori[i]*colorstride]),
			     colorstride*sizeof(float));
		    }
		  if(colorstride == 3)
		    ay_pv_add(o, cname, "uniform", "c",
			      pomesh->npolys, 3, expandedcolors);
		  else
		    ay_pv_add(o, cname, "uniform", "d",
			      pomesh->npolys, 4, expandedcolors);

		  free(expandedcolors);
		}
	      else
		{
		  /* no color index */
		  if(colorstride == 3)
		    ay_pv_add(o, cname, "uniform", "c", colorlen, 3, colors);
		  else
		    ay_pv_add(o, cname, "uniform", "d", colorlen, 4, colors);
		}
	    }
	} /* if */

      if(texcoordlen > 0)
	{
	  /* no need to check for an index, we ruled that out already */
	  ay_pv_add(o, tcname, "varying", "g",
		    pomesh->ncontrols, 2, texcoords);
	} /* if */
    } /* if */

cleanup:

  if(normali)
    free(normali);
  if(normals)
    free(normals);
  if(colori)
    free(colori);
  if(colors)
    free(colors);
  if(texcoordi)
    free(texcoordi);
  if(texcoords)
    free(texcoords);

 return;
} /* x3dio_readnct */


/* x3dio_readindexedfaceset:
 *
 */
int
x3dio_readindexedfaceset(scew_element *element)
{
 int ay_status = AY_OK;
 ay_pomesh_object pomesh = {0};
 unsigned int coordlen = 0, coordilen = 0;
 int *coordi = NULL;
 double *coords = NULL;
 unsigned int i, j, k, totalverts = 0;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "coordIndex", AY_FALSE,
			      &coordilen, &coordi);

  if(coordilen > 0)
    {
      /* count faces */
      for(i = 0; i < coordilen; i++)
	{
	  if(coordi[i] == -1)
	    {
	      pomesh.npolys++;
	    }
	  else
	    {
	      totalverts++;
	    }
	}
      if(coordi[coordilen-1] != -1)
	{
	  pomesh.npolys++;
	}

      /* allocate polymesh index arrays */
      if(!(pomesh.nloops = calloc(pomesh.npolys, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      if(!(pomesh.nverts = calloc(pomesh.npolys, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      if(!(pomesh.verts = calloc(totalverts, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      /* fill polymesh index arrays */
      for(i = 0; i < pomesh.npolys; i++)
	{
	  pomesh.nloops[i] = 1;
	}
      j = 0;
      k = 0;
      for(i = 0; i < coordilen; i++)
	{
	  if(coordi[i] != -1)
	    {
	      (pomesh.nverts[j])++;
	      (pomesh.verts[k]) = coordi[i];
	      k++;
	    }
	  else
	    {
	      j++;
	    }
	} /* for */

      /* copy coordinate values and normals */
      pomesh.ncontrols = coordlen;
      pomesh.controlv = coords;
      coords = NULL;

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDPOMESH, (void*)&pomesh);

      if(ay_status)
	goto cleanup;

      /* read/process normals, colors, and texture coordinates */
      x3dio_readnct(element, x3dio_lrobject, totalverts);

    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(coordi)
    free(coordi);

  if(pomesh.nloops)
    free(pomesh.nloops);

  if(pomesh.nverts)
    free(pomesh.nverts);

  if(pomesh.verts)
    free(pomesh.verts);

  if(pomesh.controlv)
    free(pomesh.controlv);

 return ay_status;
} /* x3dio_readindexedfaceset */


/* x3dio_readindexedtriangleset:
 *
 */
int
x3dio_readindexedtriangleset(scew_element *element)
{
 int ay_status = AY_OK;
 ay_pomesh_object pomesh = {0};
 unsigned int coordlen = 0, coordilen = 0;
 int *coordi = NULL;
 double *coords = NULL;
 unsigned int i, totalverts = 0;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "coordIndex", AY_FALSE,
			      &coordilen, &coordi);

  if(coordilen > 0)
    {
      /* calculate number of triangles */
      pomesh.npolys = coordilen/3;
      totalverts = pomesh.npolys*3;

      /* allocate polymesh index arrays */
      if(!(pomesh.nloops = calloc(pomesh.npolys, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      if(!(pomesh.nverts = calloc(pomesh.npolys, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      if(!(pomesh.verts = calloc(totalverts, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      /* fill polymesh index arrays */
      for(i = 0; i < pomesh.npolys; i++)
	{
	  pomesh.nloops[i] = 1;
	}
      for(i = 0; i < pomesh.npolys; i++)
	{
	  pomesh.nverts[i] = 3;
	} /* for */
      for(i = 0; i < totalverts; i++)
	{
	  pomesh.verts[i] = coordi[i];
	} /* for */

      /* copy coordinate values and normals */
      pomesh.ncontrols = coordlen;
      pomesh.controlv = coords;
      coords = NULL;

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDPOMESH, (void*)&pomesh);

      if(ay_status)
	goto cleanup;

      /* read/process normals, colors, and texture coordinates */
      x3dio_readnct(element, x3dio_lrobject, totalverts);

    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(coordi)
    free(coordi);

  if(pomesh.nloops)
    free(pomesh.nloops);

  if(pomesh.nverts)
    free(pomesh.nverts);

  if(pomesh.verts)
    free(pomesh.verts);

  if(pomesh.controlv)
    free(pomesh.controlv);

 return ay_status;
} /* x3dio_readindexedtriangleset */


/* x3dio_readindexedquadset:
 *
 */
int
x3dio_readindexedquadset(scew_element *element)
{
 int ay_status = AY_OK;
 ay_pomesh_object pomesh = {0};
 unsigned int coordlen = 0, coordilen = 0;
 int *coordi = NULL;
 double *coords = NULL;
 unsigned int i, totalverts = 0;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "index", AY_FALSE,
			      &coordilen, &coordi);

  if(coordilen > 0)
    {
      /* calculate number of quads */
      pomesh.npolys = coordilen/4;
      totalverts = pomesh.npolys*4;

      /* allocate polymesh index arrays */
      if(!(pomesh.nloops = calloc(pomesh.npolys, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      if(!(pomesh.nverts = calloc(pomesh.npolys, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      if(!(pomesh.verts = calloc(totalverts, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      /* fill polymesh index arrays */
      for(i = 0; i < pomesh.npolys; i++)
	{
	  pomesh.nloops[i] = 1;
	}
      for(i = 0; i < pomesh.npolys; i++)
	{
	  pomesh.nverts[i] = 4;
	} /* for */
      for(i = 0; i < totalverts; i++)
	{
	  pomesh.verts[i] = coordi[i];
	} /* for */

      /* copy coordinate values and normals */
      pomesh.ncontrols = coordlen;
      pomesh.controlv = coords;
      coords = NULL;

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDPOMESH, (void*)&pomesh);

      if(ay_status)
	goto cleanup;

      /* read/process normals, colors, and texture coordinates */
      x3dio_readnct(element, x3dio_lrobject, totalverts);

    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(coordi)
    free(coordi);

  if(pomesh.nloops)
    free(pomesh.nloops);

  if(pomesh.nverts)
    free(pomesh.nverts);

  if(pomesh.verts)
    free(pomesh.verts);

  if(pomesh.controlv)
    free(pomesh.controlv);

 return ay_status;
} /* x3dio_readindexedquadset */


/* x3dio_readindexedtrianglestripset:
 *
 */
int
x3dio_readindexedtrianglestripset(scew_element *element)
{
 int ay_status = AY_OK;
 ay_pomesh_object pomesh = {0};
 unsigned int coordlen = 0, coordilen = 0;
 int *coordi = NULL;
 double *coords = NULL;
 unsigned int i, k;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "index", AY_FALSE,
			      &coordilen, &coordi);

  if(coordilen > 0)
    {
      /* count faces */
      for(i = 0; i < coordilen; i++)
	{
	  if(coordi[i] == -1)
	    {
	      pomesh.npolys -= 2;
	    }
	  else
	    {
	      pomesh.npolys++;
	    }
	} /* for */
      if(coordi[coordilen-1] != -1)
	{
	  pomesh.npolys -= 2;
	}

      /* allocate polymesh index arrays */
      if(!(pomesh.nloops = calloc(pomesh.npolys, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      if(!(pomesh.nverts = calloc(pomesh.npolys, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      if(!(pomesh.verts = calloc(pomesh.npolys*3, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      /* fill polymesh index arrays */
      for(i = 0; i < pomesh.npolys; i++)
	{
	  pomesh.nloops[i] = 1;
	}
      for(i = 0; i < pomesh.npolys; i++)
	{
	  pomesh.nverts[i] = 3;
	} /* for */
      k = 0;
      for(i = 0; i < pomesh.npolys-1; i++)
	{
	  pomesh.verts[i*3]   = coordi[k];
	  pomesh.verts[i*3+1] = coordi[k+1];
	  pomesh.verts[i*3+2] = coordi[k+2];
	  if(coordi[k+3] == -1)
	    {
	      k += 4;
	    }
	  else
	    {
	      k++;
	    }
	} /* for */

      pomesh.verts[i*3]   = coordi[k];
      pomesh.verts[i*3+1] = coordi[k+1];
      pomesh.verts[i*3+2] = coordi[k+2];

      /* copy coordinate values and normals */
      pomesh.ncontrols = coordlen;
      pomesh.controlv = coords;
      coords = NULL;

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDPOMESH, (void*)&pomesh);

      if(ay_status)
	goto cleanup;

      /* read/process normals, colors, and texture coordinates */
      x3dio_readnct(element, x3dio_lrobject, pomesh.npolys*3);

    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(coordi)
    free(coordi);

  if(pomesh.nloops)
    free(pomesh.nloops);

  if(pomesh.nverts)
    free(pomesh.nverts);

  if(pomesh.verts)
    free(pomesh.verts);

  if(pomesh.controlv)
    free(pomesh.controlv);

 return ay_status;
} /* x3dio_readindexedtrianglestripset */


/* x3dio_readindexedtrianglefanset:
 *
 */
int
x3dio_readindexedtrianglefanset(scew_element *element)
{
 int ay_status = AY_OK;
 ay_pomesh_object pomesh = {0};
 unsigned int coordlen = 0, coordilen = 0;
 int *coordi = NULL;
 double *coords = NULL;
 unsigned int i, j, k;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "index", AY_FALSE,
			      &coordilen, &coordi);

  if(coordilen > 0)
    {

      /* count faces */
      for(i = 0; i < coordilen; i++)
	{
	  if(coordi[i] == -1)
	    {
	      pomesh.npolys -= 2;
	    }
	  else
	    {
	      pomesh.npolys++;
	    }
	} /* for */
      if(coordi[coordilen-1] != -1)
	{
	  pomesh.npolys -= 2;
	}

      /* allocate polymesh index arrays */
      if(!(pomesh.nloops = calloc(pomesh.npolys, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      if(!(pomesh.nverts = calloc(pomesh.npolys, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      if(!(pomesh.verts = calloc(pomesh.npolys*3, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      /* fill polymesh index arrays */
      for(i = 0; i < pomesh.npolys; i++)
	{
	  pomesh.nloops[i] = 1;
	}
      for(i = 0; i < pomesh.npolys; i++)
	{
	  pomesh.nverts[i] = 3;
	} /* for */
      j = 0; k = 0;
      for(i = 0; i < pomesh.npolys-1; i++)
	{
	  pomesh.verts[i*3]   = coordi[j];
	  pomesh.verts[i*3+1] = coordi[k+1];
	  pomesh.verts[i*3+2] = coordi[k+2];
	  if(coordi[k+3] == -1)
	    {
	      k += 4;
	      j = k;
	    }
	  else
	    {
	      k++;
	    }
	} /* for */

      pomesh.verts[i*3]   = coordi[j];
      pomesh.verts[i*3+1] = coordi[k+1];
      pomesh.verts[i*3+2] = coordi[k+2];

      /* copy coordinate values and normals */
      pomesh.ncontrols = coordlen;
      pomesh.controlv = coords;
      coords = NULL;

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDPOMESH, (void*)&pomesh);

      if(ay_status)
	goto cleanup;

      /* read/process normals, colors, and texture coordinates */
      x3dio_readnct(element, x3dio_lrobject, pomesh.npolys*3);

    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(coordi)
    free(coordi);

  if(pomesh.nloops)
    free(pomesh.nloops);

  if(pomesh.nverts)
    free(pomesh.nverts);

  if(pomesh.verts)
    free(pomesh.verts);

  if(pomesh.controlv)
    free(pomesh.controlv);

 return ay_status;
} /* x3dio_readindexedtrianglefanset */


/* x3dio_readindexedlineset:
 *
 */
int
x3dio_readindexedlineset(scew_element *element)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object nc = {0};
 unsigned int coordlen = 0, coordilen = 0;
 int *coordi = NULL, stride = 4;
 double *coords = NULL;
 unsigned int i, j = 0, k, totalcurves = 0;
 int l;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "coordIndex", AY_FALSE,
			      &coordilen, &coordi);

  if(coordilen > 0)
    {

      /* get colors */

      /* count curves */
      for(i = 0; i < coordilen; i++)
	{
	  if(coordi[i] == -1)
	    {
	      totalcurves++;
	    }
	}
      if(coordi[coordilen-1] != -1)
	{
	  totalcurves++;
	}

      /* create curves */
      for(i = 0; i < totalcurves; i++)
	{
	  k = j;
	  while((k < coordilen) && (coordi[k] != -1))
	    {
	      nc.length++;
	      k++;
	    }

	  if(!(nc.controlv = calloc(stride*nc.length, sizeof(double))))
	    { ay_status = AY_EOMEM; goto cleanup; }

	  for(l = 0; l < nc.length; l++)
	    {
	      memcpy(&(nc.controlv[l*stride]), &(coords[coordi[j]*3]),
		     3*sizeof(double));
	      nc.controlv[l*stride+3] = 1.0;
	      j++;
	    }

	  nc.order = 2;
	  nc.knot_type = AY_KTNURB;

	  ay_status = ay_knots_createnc(&nc);

	  if(ay_status)
	    goto cleanup;

	  /* copy object to the Ayam scene */
	  ay_status = x3dio_linkobject(element, AY_IDNCURVE, (void*)&nc);

	  /* clean up curve object */
	  if(nc.knotv)
	    free(nc.knotv);

	  if(nc.controlv)
	    free(nc.controlv);

	  memset(&nc, 0, sizeof(ay_nurbcurve_object));
	  j++;
	} /* for */

    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(coordi)
    free(coordi);

  if(nc.knotv)
    free(nc.knotv);

  if(nc.controlv)
    free(nc.controlv);

 return ay_status;
} /* x3dio_readindexedlineset */


/* x3dio_readlineset:
 *
 */
int
x3dio_readlineset(scew_element *element)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object nc = {0};
 unsigned int coordlen = 0, vertexcountslen = 0;
 int *vertexcounts = NULL, stride = 4;
 double *coords = NULL;
 unsigned int i, j = 0;
 int l;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "vertexCount", AY_FALSE,
			      &vertexcountslen, &vertexcounts);

  if(vertexcountslen > 0)
    {

      /* get colors */

      /* XXXX check, whether sum of vertexcounts == coordlen? */

      /* create curves */
      for(i = 0; i < vertexcountslen; i++)
	{
	  if(vertexcounts[i] >= 2)
	    {
	      nc.length = vertexcounts[i];

	      if(!(nc.controlv = calloc(stride*nc.length, sizeof(double))))
		{ ay_status = AY_EOMEM; goto cleanup; }

	      for(l = 0; l < nc.length; l++)
		{
		  memcpy(&(nc.controlv[l*stride]), &(coords[j]),
			 3*sizeof(double));
		  nc.controlv[l*stride+3] = 1.0;
		  j += 3;
		}

	      nc.order = 2;
	      nc.knot_type = AY_KTNURB;

	      ay_status = ay_knots_createnc(&nc);

	      if(ay_status)
		goto cleanup;

	      /* copy object to the Ayam scene */
	      ay_status = x3dio_linkobject(element, AY_IDNCURVE, (void*)&nc);

	      /* clean up curve object */
	      if(nc.knotv)
		free(nc.knotv);

	      if(nc.controlv)
		free(nc.controlv);

	      memset(&nc, 0, sizeof(ay_nurbcurve_object));
	    } /* if */
	} /* for */
    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(vertexcounts)
    free(vertexcounts);

  if(nc.knotv)
    free(nc.knotv);

  if(nc.controlv)
    free(nc.controlv);

 return ay_status;
} /* x3dio_readlineset */


/* x3dio_readtrianglefanset:
 *
 */
int
x3dio_readtrianglefanset(scew_element *element)
{
 int ay_status = AY_OK;
 char *tcname = ay_prefs.texcoordname;
 char *cname = ay_prefs.colorname;
 ay_pomesh_object pomesh = {0};
 unsigned int coordlen = 0, normallen = 0, colorlen = 0, texcoordlen = 0;
 unsigned int fancountslen = 0;
 int colorstride, *fancounts = NULL;
 int normalPerVertex = AY_FALSE, colorPerVertex = AY_FALSE;
 double *coords = NULL, *normals = NULL;
 float *texcoords = NULL, *colors = NULL;
 unsigned int i, j, k, l, totalverts = 0;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "fanCount", AY_FALSE,
			      &fancountslen, &fancounts);

  if(fancountslen > 0)
    {
      /* get normals */
      ay_status = x3dio_readnormals(element, &normallen, &normals);

      ay_status = x3dio_readbool(element, "normalPerVertex", &normalPerVertex);

      /* get colors */
      ay_status = x3dio_readcolors(element, &colorstride, &colorlen, &colors);

      ay_status = x3dio_readbool(element, "colorPerVertex", &colorPerVertex);

      /* get texture coordinates */
      ay_status = x3dio_readtexcoords(element, &texcoordlen, &texcoords);

      /* count vertices and polygons */
      for(i = 0; i < fancountslen; i++)
	{
	  if(fancounts[i] >= 3)
	    {
	      pomesh.npolys += fancounts[i]-2;
	    }
	} /* for */
      totalverts = pomesh.npolys*3;

      /* allocate polymesh index arrays */
      if(!(pomesh.nloops = calloc(pomesh.npolys, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      if(!(pomesh.nverts = calloc(pomesh.npolys, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      if(!(pomesh.verts = calloc(totalverts, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      /* fill polymesh index arrays */
      for(i = 0; i < pomesh.npolys; i++)
	{
	  pomesh.nloops[i] = 1;
	}
      for(i = 0; i < pomesh.npolys; i++)
	{
	  pomesh.nverts[i] = 3;
	} /* for */
      k = 0; l = 0;
      for(i = 0; i < fancountslen; i++)
	{
	  for(j = 1; j < (unsigned int)fancounts[i]-1; j++)
	    {
	      pomesh.verts[k] = l;
	      k++;
	      pomesh.verts[k] = l+j;
	      k++;
	      pomesh.verts[k] = l+j+1;
	      k++;
	    } /* for */
	  l += fancounts[i];
	} /* for */

      /* copy coordinate values and normals */
      pomesh.ncontrols = coordlen;
      if(normalPerVertex)
	{
	  if(normallen > 0)
	    {
	      pomesh.has_normals = AY_TRUE;
	      if(!(pomesh.controlv = calloc(6*coordlen, sizeof(double))))
		{ ay_status = AY_EOMEM; goto cleanup; }
	      for(i = 0; i < coordlen; i++)
		{
		  memcpy(&(pomesh.controlv[i*6]), &(coords[i*3]),
			 3*sizeof(double));
		  memcpy(&(pomesh.controlv[i*6+3]), &(normals[i*3]),
			 3*sizeof(double));
		}
	    }
	  else
	    {
	      pomesh.controlv = coords;
	      coords = NULL;
	    } /* if */
	}
      else
	{
	  pomesh.controlv = coords;
	  coords = NULL;
	} /* if */

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDPOMESH, (void*)&pomesh);

      if(ay_status)
	goto cleanup;

      /* process colors */
      if(colorlen > 0)
	{
	  if(colorPerVertex)
	    if(colorstride == 3)
	      ay_pv_add(x3dio_lrobject, cname, "varying", "c",
			pomesh.ncontrols, 3, colors);
	    else
	      ay_pv_add(x3dio_lrobject, cname, "varying", "d",
			pomesh.ncontrols, 4, colors);
	  else
	    if(colorstride == 3)
	      ay_pv_add(x3dio_lrobject, cname, "uniform", "c",
			pomesh.ncontrols, 3, colors);
	    else
	      ay_pv_add(x3dio_lrobject, cname, "uniform", "d",
			pomesh.ncontrols, 4, colors);
	}

      /* process texcoords */
      if(texcoordlen > 0)
	{
	  ay_pv_add(x3dio_lrobject, tcname, "varying", "g",
		    pomesh.ncontrols, 2, texcoords);
	}

    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(normals)
    free(normals);

  if(colors)
    free(colors);

  if(texcoords)
    free(texcoords);

  if(fancounts)
    free(fancounts);

  if(pomesh.nloops)
    free(pomesh.nloops);

  if(pomesh.nverts)
    free(pomesh.nverts);

  if(pomesh.verts)
    free(pomesh.verts);

  if(pomesh.controlv)
    free(pomesh.controlv);

 return ay_status;
} /* x3dio_readtrianglefanset */


/* x3dio_readtrianglestripset:
 *
 */
int
x3dio_readtrianglestripset(scew_element *element)
{
 int ay_status = AY_OK;
 char *tcname = ay_prefs.texcoordname;
 char *cname = ay_prefs.colorname;
 ay_pomesh_object pomesh = {0};
 unsigned int coordlen = 0, normallen = 0, colorlen = 0, texcoordlen = 0;
 unsigned int stripcountslen = 0;
 int *stripcounts = NULL;
 int colorstride = 0, colorPerVertex = AY_FALSE, normalPerVertex = AY_FALSE;
 double *coords = NULL, *normals = NULL;
 float *texcoords = NULL, *colors = NULL;
 unsigned int i, j, k, l, totalverts = 0;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "stripCount", AY_FALSE,
			      &stripcountslen, &stripcounts);

  if(stripcountslen > 0)
    {
      /* get normals */
      ay_status = x3dio_readnormals(element, &normallen, &normals);

      ay_status = x3dio_readbool(element, "normalPerVertex", &normalPerVertex);

      /* get colors */
      ay_status = x3dio_readcolors(element, &colorstride, &colorlen, &colors);

      ay_status = x3dio_readbool(element, "colorPerVertex", &colorPerVertex);

      /* get texture coordinates */
      ay_status = x3dio_readtexcoords(element, &texcoordlen, &texcoords);

      /* count vertices and polygons */
      for(i = 0; i < stripcountslen; i++)
	{
	  if(stripcounts[i] >= 3)
	    {
	      pomesh.npolys += stripcounts[i]-2;
	    }
	} /* for */
      totalverts = pomesh.npolys*3;

      /* allocate polymesh index arrays */
      if(!(pomesh.nloops = calloc(pomesh.npolys, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      if(!(pomesh.nverts = calloc(pomesh.npolys, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      if(!(pomesh.verts = calloc(totalverts, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      /* fill polymesh index arrays */
      for(i = 0; i < pomesh.npolys; i++)
	{
	  pomesh.nloops[i] = 1;
	}
      for(i = 0; i < pomesh.npolys; i++)
	{
	  pomesh.nverts[i] = 3;
	} /* for */
      k = 0; l = 0;
      for(i = 0; i < stripcountslen; i++)
	{
	  for(j = 0; j < (unsigned int)stripcounts[i]-2; j++)
	    {
	      pomesh.verts[k] = l+j;
	      k++;
	      pomesh.verts[k] = l+j+1;
	      k++;
	      pomesh.verts[k] = l+j+2;
	      k++;
	    } /* for */
	  l += stripcounts[i];
	} /* for */

      /* copy coordinate values and normals */
      pomesh.ncontrols = coordlen;
      if(normalPerVertex)
	{
	  if(normallen > 0)
	    {
	      pomesh.has_normals = AY_TRUE;
	      if(!(pomesh.controlv = calloc(6*coordlen, sizeof(double))))
		{ ay_status = AY_EOMEM; goto cleanup; }
	      for(i = 0; i < coordlen; i++)
		{
		  memcpy(&(pomesh.controlv[i*6]), &(coords[i*3]),
			 3*sizeof(double));
		  memcpy(&(pomesh.controlv[i*6+3]), &(normals[i*3]),
			 3*sizeof(double));
		}
	    }
	  else
	    {
	      pomesh.controlv = coords;
	      coords = NULL;
	    } /* if */
	}
      else
	{
	  pomesh.controlv = coords;
	  coords = NULL;
	} /* if */

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDPOMESH, (void*)&pomesh);

      if(ay_status)
	goto cleanup;

      /* process colors */
      if(colorlen > 0)
	{
	  if(colorPerVertex)
	    if(colorstride == 3)
	      ay_pv_add(x3dio_lrobject, cname, "varying", "c",
			pomesh.ncontrols, 3, colors);
	    else
	      ay_pv_add(x3dio_lrobject, cname, "varying", "d",
			pomesh.ncontrols, 4, colors);
	  else
	    if(colorstride == 3)
	      ay_pv_add(x3dio_lrobject, cname, "uniform", "c",
			pomesh.ncontrols, 3, colors);
	    else
	      ay_pv_add(x3dio_lrobject, cname, "uniform", "d",
			pomesh.ncontrols, 4, colors);
	}

      /* process texcoords */
      if(texcoordlen > 0)
	{
	  ay_pv_add(x3dio_lrobject, tcname, "varying", "g",
		    pomesh.ncontrols, 2, texcoords);
	}

    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(normals)
    free(normals);

  if(colors)
    free(colors);

  if(texcoords)
    free(texcoords);

  if(stripcounts)
    free(stripcounts);

  if(pomesh.nloops)
    free(pomesh.nloops);

  if(pomesh.nverts)
    free(pomesh.nverts);

  if(pomesh.verts)
    free(pomesh.verts);

  if(pomesh.controlv)
    free(pomesh.controlv);

 return ay_status;
} /* x3dio_readtrianglestripset */


/* x3dio_readtriangleset:
 *
 */
int
x3dio_readtriangleset(scew_element *element)
{
 int ay_status = AY_OK;
 char *tcname = ay_prefs.texcoordname;
 char *cname = ay_prefs.colorname;
 ay_pomesh_object pomesh = {0};
 unsigned int coordlen = 0, normallen = 0, colorlen = 0, texcoordlen = 0;
 int colorstride = 0, colorPerVertex = AY_FALSE, normalPerVertex = AY_FALSE;
 double *coords = NULL, *normals = NULL;
 float *texcoords = NULL, *colors = NULL;
 unsigned int i, totalverts = 0;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  /* get normals */
  ay_status = x3dio_readnormals(element, &normallen, &normals);

  ay_status = x3dio_readbool(element, "normalPerVertex", &normalPerVertex);

  /* get colors */
  ay_status = x3dio_readcolors(element, &colorstride, &colorlen, &colors);

  ay_status = x3dio_readbool(element, "colorPerVertex", &colorPerVertex);

  /* get texture coordinates */
  ay_status = x3dio_readtexcoords(element, &texcoordlen, &texcoords);

  pomesh.npolys = coordlen/3;
  totalverts = pomesh.npolys*3;

  /* allocate polymesh index arrays */
  if(!(pomesh.nloops = calloc(pomesh.npolys, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(pomesh.nverts = calloc(pomesh.npolys, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(pomesh.verts = calloc(totalverts, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* fill polymesh index arrays */
  for(i = 0; i < pomesh.npolys; i++)
    {
      pomesh.nloops[i] = 1;
    }
  for(i = 0; i < pomesh.npolys; i++)
    {
      pomesh.nverts[i] = 3;
    } /* for */
  for(i = 0; i < totalverts; i++)
    {
      pomesh.verts[i] = i;
    } /* for */

  /* copy coordinate values and normals */
  pomesh.ncontrols = coordlen;
  if(normalPerVertex)
    {
      if(normallen > 0)
	{
	  pomesh.has_normals = AY_TRUE;
	  if(!(pomesh.controlv = calloc(6*coordlen, sizeof(double))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  for(i = 0; i < coordlen; i++)
	    {
	      memcpy(&(pomesh.controlv[i*6]), &(coords[i*3]),
		     3*sizeof(double));
	      memcpy(&(pomesh.controlv[i*6+3]), &(normals[i*3]),
		     3*sizeof(double));
	    }
	}
      else
	{
	  pomesh.controlv = coords;
	  coords = NULL;
	} /* if */
    }
  else
    {
      pomesh.controlv = coords;
      coords = NULL;
    } /* if */

  /* copy object to the Ayam scene */
  ay_status = x3dio_linkobject(element, AY_IDPOMESH, (void*)&pomesh);

  if(ay_status)
    goto cleanup;

  /* process colors */
  if(colorlen > 0)
    {
      if(colorPerVertex)
	if(colorstride == 3)
	  ay_pv_add(x3dio_lrobject, cname, "varying", "c",
		    pomesh.ncontrols, 3, colors);
	else
	  ay_pv_add(x3dio_lrobject, cname, "varying", "d",
		    pomesh.ncontrols, 4, colors);
      else
	if(colorstride == 3)
	  ay_pv_add(x3dio_lrobject, cname, "uniform", "c",
		    pomesh.ncontrols, 3, colors);
	else
	  ay_pv_add(x3dio_lrobject, cname, "uniform", "d",
		    pomesh.ncontrols, 4, colors);
    }

  /* process texcoords */
  if(texcoordlen > 0)
    {
      ay_pv_add(x3dio_lrobject, tcname, "varying", "g",
		pomesh.ncontrols, 2, texcoords);
    }


cleanup:
  if(coords)
    free(coords);

  if(normals)
    free(normals);

  if(colors)
    free(colors);

  if(texcoords)
    free(texcoords);

  if(pomesh.nloops)
    free(pomesh.nloops);

  if(pomesh.nverts)
    free(pomesh.nverts);

  if(pomesh.verts)
    free(pomesh.verts);

  if(pomesh.controlv)
    free(pomesh.controlv);

 return ay_status;
} /* x3dio_readtriangleset */


/* x3dio_readquadset:
 *
 */
int
x3dio_readquadset(scew_element *element)
{
 int ay_status = AY_OK;
 char *tcname = ay_prefs.texcoordname;
 char *cname = ay_prefs.colorname;
 ay_pomesh_object pomesh = {0};
 unsigned int coordlen = 0, normallen = 0, colorlen = 0, texcoordlen = 0;
 int colorstride = 0, colorPerVertex = AY_FALSE, normalPerVertex = AY_FALSE;
 double *coords = NULL, *normals = NULL;
 float *texcoords = NULL, *colors = NULL;
 unsigned int i, totalverts = 0;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  /* get normals */
  ay_status = x3dio_readnormals(element, &normallen, &normals);

  ay_status = x3dio_readbool(element, "normalPerVertex", &normalPerVertex);

  /* get colors */
  ay_status = x3dio_readcolors(element, &colorstride, &colorlen, &colors);

  ay_status = x3dio_readbool(element, "colorPerVertex", &colorPerVertex);

  /* get texture coordinates */
  ay_status = x3dio_readtexcoords(element, &texcoordlen, &texcoords);

  pomesh.npolys = coordlen/4;
  totalverts = pomesh.npolys*4;

  /* allocate polymesh index arrays */
  if(!(pomesh.nloops = calloc(pomesh.npolys, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(pomesh.nverts = calloc(pomesh.npolys, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(pomesh.verts = calloc(totalverts, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* fill polymesh index arrays */
  for(i = 0; i < pomesh.npolys; i++)
    {
      pomesh.nloops[i] = 1;
    }
  for(i = 0; i < pomesh.npolys; i++)
    {
      pomesh.nverts[i] = 4;
    } /* for */
  for(i = 0; i < totalverts; i++)
    {
      pomesh.verts[i] = i;
    } /* for */

  /* copy coordinate values and normals */
  pomesh.ncontrols = coordlen;
  if(normalPerVertex)
    {
      if(normallen > 0)
	{
	  pomesh.has_normals = AY_TRUE;
	  if(!(pomesh.controlv = calloc(6*coordlen, sizeof(double))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  for(i = 0; i < coordlen; i++)
	    {
	      memcpy(&(pomesh.controlv[i*6]), &(coords[i*3]),
		     3*sizeof(double));
	      memcpy(&(pomesh.controlv[i*6+3]), &(normals[i*3]),
		     3*sizeof(double));
	    }
	}
      else
	{
	  pomesh.controlv = coords;
	  coords = NULL;
	} /* if */
    }
  else
    {
      pomesh.controlv = coords;
      coords = NULL;
    } /* if */

  /* copy object to the Ayam scene */
  ay_status = x3dio_linkobject(element, AY_IDPOMESH, (void*)&pomesh);

  if(ay_status)
    goto cleanup;

  /* process colors */
  if(colorlen > 0)
    {
      if(colorPerVertex)
	if(colorstride == 3)
	  ay_pv_add(x3dio_lrobject, cname, "varying", "c",
		    pomesh.ncontrols, 3, colors);
	else
	  ay_pv_add(x3dio_lrobject, cname, "varying", "d",
		    pomesh.ncontrols, 4, colors);
      else
	if(colorstride == 3)
	  ay_pv_add(x3dio_lrobject, cname, "uniform", "c",
		    pomesh.ncontrols, 3, colors);
	else
	  ay_pv_add(x3dio_lrobject, cname, "uniform", "d",
		    pomesh.ncontrols, 4, colors);
    }

  /* process texcoords */
  if(texcoordlen > 0)
    {
      ay_pv_add(x3dio_lrobject, tcname, "varying", "g",
		pomesh.ncontrols, 2, texcoords);
    }

cleanup:
  if(coords)
    free(coords);

  if(normals)
    free(normals);

  if(colors)
    free(colors);

  if(texcoords)
    free(texcoords);

  if(pomesh.nloops)
    free(pomesh.nloops);

  if(pomesh.nverts)
    free(pomesh.nverts);

  if(pomesh.verts)
    free(pomesh.verts);

  if(pomesh.controlv)
    free(pomesh.controlv);

 return ay_status;
} /* x3dio_readquadset */


/* x3dio_readelevationgrid:
 *
 */
int
x3dio_readelevationgrid(scew_element *element)
{
 int ay_status = AY_OK;
 ay_pamesh_object pamesh = {0};
 int xDim = 0, zDim = 0;
 float *heights = NULL, xSpac = 1.0f, zSpac = 1.0f;
 unsigned int i, j, a = 0, b = 0, heightslen = 0;

  if(!element)
    return AY_ENULL;

  pamesh.type = AY_PTBILINEAR;

  ay_status = x3dio_readint(element, "xDimension", &xDim);
  ay_status = x3dio_readint(element, "zDimension", &zDim);

  if((xDim == 0) || (zDim == 0))
    {
      return AY_OK;
    }

  pamesh.width = xDim;
  pamesh.height = zDim;

  ay_status = x3dio_readfloat(element, "xSpacing", &xSpac);
  ay_status = x3dio_readfloat(element, "zSpacing", &zSpac);

  ay_status = x3dio_readfloatpoints(element, "height", 1,
				    &heightslen, &heights);

  if(heightslen < (unsigned int)xDim*zDim)
    { ay_status = AY_ERROR; goto cleanup; }

  /* get normals */

  /* get colors */

  /* get texture coordinates */

  /* copy coordinate values */
  if(!(pamesh.controlv = calloc(4*xDim*zDim, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  for(i = 0; i < (unsigned int)xDim; i++)
    {
      for(j = 0; j < (unsigned int)zDim; j++)
	{
	  pamesh.controlv[a] = xSpac * i;
	  pamesh.controlv[a+1] = heights[b];
	  pamesh.controlv[a+2] = zSpac * j;
	  pamesh.controlv[a+3] = 1.0;
	  a += 4;
	  b++;
	} /* for */
    } /* for */

  /* copy object to the Ayam scene */
  ay_status = x3dio_linkobject(element, AY_IDPAMESH, (void*)&pamesh);

cleanup:

  if(heights)
    free(heights);

  if(pamesh.controlv)
    free(pamesh.controlv);

 return ay_status;
} /* x3dio_readelevationgrid */


/* x3dio_getquatfromvec:
 *
 */
int
x3dio_getquatfromvec(double *v, double *q)
{
 double tmp = 0.0;
 double xylen = 0.0, zylen = 0.0, x = 0.0, z = 0.0;
 double q1[4] = {0}, q2[4] = {0}, quat[4] = {0};
 double xaxis[3] = {1.0, 0.0, 0.0}, zaxis[3] = {0.0, 0.0, 1.0};
 double len;

  if(!v || !q)
   return AY_ENULL;

  /* XXXX check v? */

  len = AY_V3LEN(v);
  AY_V3SCAL(v, 1.0/len);

  /*printf("vx:%g, vy:%g, vz:%g\n",v[0], v[1], v[2]);*/

  if(fabs(v[0]) > AY_EPSILON ||
     fabs(v[1]) > AY_EPSILON ||
     fabs(v[2]) > AY_EPSILON)
    {
      xylen = sqrt(v[0] * v[0] + v[1] * v[1]);

      if(fabs(xylen) < AY_EPSILON)
	{
	  z = (v[2] < 0.0) ? AY_PI : 0.0;
	}
      else
	{
	  tmp = v[1]/xylen;
	  z = acos((fabs(tmp)<=1.0?tmp:(tmp<-1.0?-1.0:1.0)));
	}

      if(fabs(v[2]) > AY_EPSILON)
	{
	  zylen = sqrt(xylen * xylen + v[1] * v[1]);

	  if(fabs(zylen) > AY_EPSILON)
	    {
	      x = acos(xylen/zylen);
	    }
	  else
	    {
	      x = 0.0;
	    }

	  if(v[1] > 0.0)
	    x = -x;
	}

      if(v[0] < 0.0)
	z = -z;

      /*printf("x:%g, z:%g\n",AY_R2D(x),AY_R2D(z));*/

      if((fabs(x) > AY_EPSILON) || (fabs(z) > AY_EPSILON))
	{
	  if(fabs(x) > AY_EPSILON)
	    {
	      ay_quat_axistoquat(xaxis, x, q1);
	      if(fabs(z) > AY_EPSILON)
		{
		  ay_quat_axistoquat(zaxis, z, q2);
		  ay_quat_add(q1, q2, quat);
		}
	      else
		{
		  memcpy(quat, q1, 4*sizeof(double));
		}
	    }
	  else
	    {
	      ay_quat_axistoquat(zaxis, z, quat);
	    } /* if */
	  ay_quat_norm(quat);
	}
      else
	{
	  quat[3] = 1.0;
	} /* if */
    }
  else
    {
      /* oops, all components zero => spine is not oriented
	 at all => gracefully assume y-aligned spine */
      quat[3] = 1.0;
    } /* if */

  memcpy(q, quat, 4*sizeof(double));

 return AY_OK;
} /* x3dio_getquatfromvec */


/* x3dio_fixcrosssection:
 *  find equal consecutive points in <cs> and remove them
 */
int
x3dio_fixcrosssection(unsigned int cslen, float *cs,
		      unsigned int *newcslen, float **newcs)
{
 float *tcs = NULL;
 unsigned int i, j, tcslen;

  if(!cs)
   return AY_ENULL;

  if(!(tcs = calloc(cslen*2, sizeof(float))))
    return AY_EOMEM;

  memcpy(tcs, cs, 2*sizeof(float));
  tcslen = 1;
  j = 0;
  for(i = 1; i < cslen; i++)
    {
      if(fabs(tcs[j*2]   - cs[i*2])   > AY_EPSILON ||
	 fabs(tcs[j*2+1] - cs[i*2+1]) > AY_EPSILON)
	{
	  j++;
	  memcpy(&(tcs[j*2]), &(cs[i*2]), 2*sizeof(float));
	  tcslen++;
	}
    } /* for */

  *newcslen = tcslen;
  *newcs = tcs;

 return AY_ERROR;
} /* x3dio_fixcrosssection */


/* x3dio_getdiffspinepoint:
 *
 */
int
x3dio_getdiffspinepoint(unsigned int splen, float *sp, unsigned int sindex,
			int stride, unsigned int *index)
{

  if(!sp || !index)
    return AY_ENULL;

  if((stride != -1) && (stride != 1))
    return AY_ERROR;

  if(sindex > splen)
    return AY_ERROR;

  *index = sindex + stride;

  while((*index > 0) && (*index < splen))
    {
      if(fabs(sp[sindex*3]   - sp[*index*3])   > AY_EPSILON ||
	 fabs(sp[sindex*3+1] - sp[*index*3+1]) > AY_EPSILON ||
	 fabs(sp[sindex*3+2] - sp[*index*3+2]) > AY_EPSILON)
	return AY_OK;

      (*index) += stride;
    }

 return AY_ERROR;
} /* x3dio_getdiffspinepoint */


/* x3dio_getspinerots:
 *
 */
int
x3dio_getspinerots(unsigned int splen, float *sp, int sp_closed,
		   unsigned int orlen, float *or,
		   double **rots)
{
 double /*scpx[3],*/ scpy[3], /*scpz[3],*/ t1[3]/*, t2[3]*/;
 /*double prevscpz[3];*/
 double q1[4] = {0}, q2[4] = {0}, quat[4] = {0};
 double *quats = NULL;
 unsigned int i, next;

  if(!sp || !rots)
   return AY_ENULL;

  if(!(quats = calloc(splen*4,sizeof(double))))
    return AY_EOMEM;

  if(splen > 2)
    {
      for(i = 0; i < splen; i++)
	{
	  /* calculate rotation angles */
	  if(i == 0)
	    {
	      /* first spine point */
	      if(!sp_closed)
		{
		  x3dio_getdiffspinepoint(splen, sp, 0, 1, &next);
		  scpy[0] = sp[next*3] - sp[0];
		  scpy[1] = sp[next*3+1] - sp[1];
		  scpy[2] = sp[next*3+2] - sp[2];
		  /*
		  t1[0] = sp[2*3]   - sp[3];
		  t1[1] = sp[2*3+1] - sp[4];
		  t1[2] = sp[2*3+2] - sp[5];

		  t2[0] = sp[0] - sp[3];
		  t2[1] = sp[1] - sp[4];
		  t2[2] = sp[2] - sp[5];
		  */
		}
	      else
		{
		  scpy[0] = sp[3] - sp[(splen-2)*3];
		  scpy[1] = sp[4] - sp[(splen-2)*3+1];
		  scpy[2] = sp[5] - sp[(splen-2)*3+2];
		  /*
		  t1[0] = sp[3] - sp[0];
		  t1[1] = sp[4] - sp[1];
		  t1[2] = sp[5] - sp[2];

		  t1[0] = sp[(splen-2)*3]   - sp[0];
		  t1[0] = sp[(splen-2)*3+1] - sp[1];
		  t1[0] = sp[(splen-2)*3+2] - sp[2];
		  */
		} /* if */
	    } /* if */
	  if((i > 0) && (i < splen-1))
	    {
	      /* middle spine point */
	      x3dio_getdiffspinepoint(splen, sp, i, 1, &next);
	      scpy[0] = sp[next*3] - sp[i*3];
	      scpy[1] = sp[next*3+1] - sp[i*3+1];
	      scpy[2] = sp[next*3+2] - sp[i*3+2];
	      /*
	      t1[0] = sp[next*3] - sp[i*3];
	      t1[1] = sp[next*3+1] - sp[i*3+1];
	      t1[2] = sp[next*3+2] - sp[i*3+2];

	      t2[0] = sp[(i-1)*3] - sp[i*3];
	      t2[1] = sp[(i-1)*3+1] - sp[i*3+1];
	      t2[2] = sp[(i-1)*3+2] - sp[i*3+2];
	      */
	    } /* if */
	  if(i == splen-1)
	    {
	      /* last spine point */
	      if(!sp_closed)
		{
		  x3dio_getdiffspinepoint(splen, sp, i, -1, &next);
		  scpy[0] = sp[i*3]   - sp[next*3];
		  scpy[1] = sp[i*3+1] - sp[next*3+1];
		  scpy[2] = sp[i*3+2] - sp[next*3+2];
		  /*
		  t1[0] = sp[i*3]   - sp[(splen-2)*3];
		  t1[1] = sp[i*3+1] - sp[(splen-2)*3+1];
		  t1[2] = sp[i*3+2] - sp[(splen-2)*3+2];

		  t2[0] = sp[(splen-3)*3]   - sp[(splen-2)*3];
		  t2[1] = sp[(splen-3)*3+1] - sp[(splen-2)*3+1];
		  t2[2] = sp[(splen-3)*3+2] - sp[(splen-2)*3+2];
		  */
		}
	      else
		{
		  scpy[0] = sp[3] - sp[(splen-2)*3];
		  scpy[1] = sp[4] - sp[(splen-2)*3+1];
		  scpy[2] = sp[5] - sp[(splen-2)*3+2];
		  /*
		  t1[0] = sp[3] - sp[0];
		  t1[1] = sp[4] - sp[1];
		  t1[2] = sp[5] - sp[2];

		  t1[0] = sp[(splen-2)*3]   - sp[0];
		  t1[0] = sp[(splen-2)*3+1] - sp[1];
		  t1[0] = sp[(splen-2)*3+2] - sp[2];
		  */
		} /* if */
	    } /* if */
#if 0
	  AY_V3CROSS(scpz, t1, t2);

	  /* flip scpz? */
	  if(i > 0)
	    {
	      if(AY_V3DOT(scpz, prevscpz) < 0.0)
		{
		  AY_V3SCAL(scpz, -1.0);
		}
	    }
	  memcpy(prevscpz, scpz, 3*sizeof(double));

	  AY_V3CROSS(scpx, scpy, scpz);
#endif
	  memset(quat, 0, 3*sizeof(double));
	  quat[3] = 1.0;

	  x3dio_getquatfromvec(scpy, quat);

	  /* add orientation */
	  if(fabs(or[i*4+3]) > AY_EPSILON)
	    {
	      /*quats[i*4] = 1.0;*/

	      q2[0] = or[i*4];
	      q2[1] = or[i*4+1];
	      q2[2] = or[i*4+2];
	      q2[3] = or[i*4+3];

	      ay_quat_axistoquat(q2, q2[3], q1);
	      ay_quat_add(quat, q1, &(quats[i*4]));
	    }
	  else
	    {
	      memcpy(&(quats[i*4]), quat, 4*sizeof(double));
	    } /* if */
	} /* for */
    }
  else
    {
      /* simple 2 point (straight) spine */
      t1[0] = sp[3] - sp[0];
      t1[1] = sp[4] - sp[1];
      t1[2] = sp[5] - sp[2];

      if(fabs(t1[0]) < AY_EPSILON &&
	 fabs(t1[2]) < AY_EPSILON)
	{
	  /* spine is already y-axis-aligned */
	  quat[3] = 1.0;
	}
      else
	{
	  x3dio_getquatfromvec(t1, quat);
	} /* if */

      /* add orientation of spine start point */
      if(fabs(or[3]) > AY_EPSILON)
	{
	  q2[0] = or[0];
	  q2[1] = or[1];
	  q2[2] = or[2];
	  q2[3] = or[3];

	  ay_quat_axistoquat(q2, q2[3], q1);
	  ay_quat_add(q1, quat, quats);
	}
      else
	{
	  memcpy(quats, quat, 4*sizeof(double));
	} /* if */
      /* add orientation of spine end point */
      if(fabs(or[7]) > AY_EPSILON)
	{
	  q2[0] = or[4];
	  q2[1] = or[5];
	  q2[2] = or[6];
	  q2[3] = or[7];

	  ay_quat_axistoquat(q2, q2[3], q1);
	  ay_quat_add(q1, quat, &(quats[4]));
	}
      else
	{
	  memcpy(&(quats[4]), quat, 4*sizeof(double));
	} /* if */

    } /* if */

  *rots = quats;

 return AY_OK;
} /* x3dio_getspinerots */


/* x3dio_readextrusion:
 *
 */
int
x3dio_readextrusion(scew_element *element)
{
 int ay_status = AY_OK;
 unsigned int cslen = 0, splen = 0, scalelen = 0, orientlen = 0;
 float *cs = NULL, csd[10] = {1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
			     -1.0f, 1.0f, 1.0f, 1.0f};
 float *sp = NULL, spd[6] = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
 float *scale = NULL, scaled[2] = {1.0f, 1.0f};
 float *orient = NULL, orientd[4] = {0.0f, 0.0f, 1.0f, 0.0f};
 float *tcs = NULL;
 unsigned int tcslen = 0;
 int has_sides = AY_TRUE, has_startcap = AY_TRUE, has_endcap = AY_TRUE;
 int sp_closed = AY_FALSE;
 ay_pomesh_object pomesh = {0};
 unsigned int totalverts = 0, i, j, a = 0, b = 0;
 double *rots = NULL, rotmat[16];

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readfloatpoints(element, "crossSection", 2,
				    &cslen, &cs);

  if(cslen == 0)
    {
      cs = csd;
      cslen = 5;
    }
  else
    {
      ay_status = x3dio_fixcrosssection(cslen, cs, &tcslen, &tcs);
      if(tcs)
	{
	  free(cs);
	  cslen = tcslen;
	  cs = tcs;
	}
    } /* if */

  ay_status = x3dio_readfloatpoints(element, "spine", 3,
				    &splen, &sp);

  if(splen == 0)
    {
      sp = spd;
      splen = 2;
    }

  if((fabs(sp[0]-sp[(splen-1)*3]) < AY_EPSILON) &&
     (fabs(sp[1]-sp[(splen-1)*3+1]) < AY_EPSILON) &&
     (fabs(sp[2]-sp[(splen-1)*3+2]) < AY_EPSILON))
    {
      sp_closed = AY_TRUE;
    }

  ay_status = x3dio_readfloatpoints(element, "scale", 2,
				    &scalelen, &scale);

  if(scalelen == 0)
    {
      scale = scaled;
      scalelen = 1;
    }
  else
    {
      if((scalelen > 1) && (scalelen < splen))
	{
	  ay_status = AY_ERROR;
	  goto cleanup;
	}
    } /* if */

  ay_status = x3dio_readfloatpoints(element, "orientation", 4,
				    &orientlen, &orient);

  if(orientlen == 0)
    {
      orient = orientd;
      orientlen = 1;
    }
  else
    {
      if((orientlen > 1) && (orientlen < splen))
	{
	  ay_status = AY_ERROR;
	  goto cleanup;
	}
    } /* if */

  ay_status = x3dio_readbool(element, "beginCap", &has_startcap);

  ay_status = x3dio_readbool(element, "endCap", &has_endcap);

  if(cslen <= 2)
    {
      has_startcap = AY_FALSE;
      has_endcap = AY_FALSE;
    }

  ay_status = x3dio_readbool(element, "sides", &has_sides);

  pomesh.npolys = (has_sides?(cslen-1) * (splen-1) * 2:0) +
    (has_startcap?1:0) + (has_endcap?1:0);
  totalverts = (has_sides?pomesh.npolys * 3:0) +
    (has_startcap?cslen:0) + (has_endcap?cslen:0);

  if(pomesh.npolys == 0)
    { ay_status = AY_ERROR; goto cleanup; }

  /* allocate polymesh index arrays */
  if(!(pomesh.nloops = calloc(pomesh.npolys, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(pomesh.nverts = calloc(pomesh.npolys, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(pomesh.verts = calloc(totalverts, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }

  /* fill polymesh index arrays */
  for(i = 0; i < pomesh.npolys; i++)
    {
      pomesh.nloops[i] = 1;
    }
  for(i = 0; i < pomesh.npolys; i++)
    {
      pomesh.nverts[i] = 3;
    } /* for */

  if(has_sides)
    {
      for(i = 0; i < splen-1; i++)
	{
	  for(j = 0; j < cslen-1; j++)
	    {
	      pomesh.verts[a]   = b;
	      pomesh.verts[a+1] = b+1;
	      pomesh.verts[a+2] = b+cslen;
	      a += 3;
	      pomesh.verts[a]   = b+1;
	      pomesh.verts[a+1] = b+cslen+1;
	      pomesh.verts[a+2] = b+cslen;
	      a += 3;
	      b++;
	    } /* for */
	  b++;
	} /* for */
    } /* if */

  /* create caps */
  if(has_startcap && has_endcap)
    {
      pomesh.nverts[pomesh.npolys-2] = cslen;
      pomesh.nverts[pomesh.npolys-1] = cslen;

      for(j = 0; j < cslen; j++)
	{
	  pomesh.verts[a] = j;
	  a++;
	} /* for */
      b = splen * cslen - cslen;
      for(j = 0; j < cslen; j++)
	{
	  pomesh.verts[a] = b+j;
	  a++;
	} /* for */
    }
  else
    {
      if(has_startcap)
	{
	  pomesh.nverts[pomesh.npolys-1] = cslen;
	  for(j = 0; j < cslen; j++)
	    {
	      pomesh.verts[a] = j;
	      a++;
	    } /* for */
	} /* if */
      if(has_endcap)
	{
	  pomesh.nverts[pomesh.npolys-1] = cslen;
	  b = splen * cslen - cslen;
	  for(j = 0; j < cslen; j++)
	    {
	      pomesh.verts[a] = b+j;
	      a++;
	    } /* for */
	} /* if */
    } /* if */

  /* calculate rotation angles */
  ay_status = x3dio_getspinerots(splen, sp, sp_closed, orientlen, orient,
				 &rots);
  if(!rots)
    { goto cleanup; }

  /* allocate and fill controlv */
  pomesh.ncontrols = cslen * splen;

  if(!(pomesh.controlv = calloc(3*pomesh.ncontrols, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  a = 0;
  for(i = 0; i < splen; i++)
    {
      ay_quat_torotmatrix(&(rots[i*4]), rotmat);

      for(j = 0; j < cslen; j++)
	{
	  /* take cross section */
	  pomesh.controlv[a]   = cs[j*2];
	  /*pomesh.controlv[a+1] = 0.0;*/
	  pomesh.controlv[a+2] = cs[j*2+1];

	  /* apply scale */
	  if(scalelen > 1)
	    {
	      pomesh.controlv[a]   *= scale[i*2];
	      pomesh.controlv[a+2] *= scale[i*2+1];
	    }
	  else
	    {
	      pomesh.controlv[a]   *= scale[0];
	      pomesh.controlv[a+2] *= scale[1];
	    }

	  /* apply rotation */
	  ay_trafo_apply3(&(pomesh.controlv[a]), rotmat);

	  /* move to spine */
	  pomesh.controlv[a]   += sp[i*3];
	  pomesh.controlv[a+1] += sp[i*3+1];
	  pomesh.controlv[a+2] += sp[i*3+2];

	  a += 3;
	} /* for */
    } /* for */

  /* copy object to the Ayam scene */
  ay_status = x3dio_linkobject(element, AY_IDPOMESH, (void*)&pomesh);

cleanup:

  if(cs && (cs != csd))
    free(cs);

  if(sp && (sp != spd))
    free(sp);

  if(scale && (scale != scaled))
    free(scale);

  if(orient && (orient != orientd))
    free(orient);

  if(rots)
    free(rots);

 return ay_status;
} /* x3dio_readextrusion */


/* x3dio_readdisk2d:
 *
 */
int
x3dio_readdisk2d(scew_element *element)
{
 int ay_status = AY_OK;
 ay_disk_object disk = {0};
 ay_hyperboloid_object hyperboloid = {0};
 float iradius = 0.0f;
 float oradius = 0.0f;

  if(!element)
    return AY_ENULL;

  x3dio_readfloat(element, "innerRadius", &iradius);
  if(iradius < 0.0f)
    {
      return AY_ERROR;
    }
  disk.radius = 1.0;
  x3dio_readfloat(element, "outerRadius", &oradius);
  if(oradius > 0.0f)
    {
      disk.radius = oradius;
    }
  else
    {
      return AY_ERROR;
    }

  if(fabs(iradius) < AY_EPSILON)
    {
      disk.thetamax = 360.0;
      disk.is_simple = AY_TRUE;

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDDISK, (void*)&disk);
    }
  else
    {
      hyperboloid.p1[0] = iradius;
      hyperboloid.p2[0] = oradius;
      hyperboloid.thetamax = 360.0;

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDHYPERBOLOID,
				   (void*)&hyperboloid);
    } /* if */

 return ay_status;
} /* x3dio_readdisk2d */


/* x3dio_readcircle2d:
 *
 */
int
x3dio_readcircle2d(scew_element *element)
{
 int ay_status = AY_OK;
 ay_ncircle_object ncircle = {0};
 float radius = 1.0f;

  if(!element)
    return AY_ENULL;

  ncircle.radius = 1.0;
  x3dio_readfloat(element, "radius", &radius);
  if(radius > 0.0f)
    {
      ncircle.radius = radius;
    }
  else
    {
      return AY_ERROR;
    }

  ncircle.tmax = 360.0;

  /* copy object to the Ayam scene */
  ay_status = x3dio_linkobject(element, AY_IDNCIRCLE, (void*)&ncircle);

  if(ay_status == AY_OK)
    ay_notify_object(x3dio_lrobject);

 return ay_status;
} /* x3dio_readcircle2d */


/* x3dio_readarc2d:
 *
 */
int
x3dio_readarc2d(scew_element *element)
{
 int ay_status = AY_OK;
 ay_ncircle_object ncircle = {0};
 float radius = 1.0f;
 float sangle = 0.0f;
 float eangle = (float)AY_HALFPI;

  if(!element)
    return AY_ENULL;

  ncircle.radius = 1.0;
  x3dio_readfloat(element, "radius", &radius);
  if(radius > 0.0f)
    {
      ncircle.radius = radius;
    }
  else
    {
      return AY_ERROR;
    }

  ncircle.tmin = 0.0;
  x3dio_readfloat(element, "startAngle", &sangle);
  ncircle.tmin = AY_R2D((double)sangle);

  ncircle.tmax = 90.0;
  x3dio_readfloat(element, "endAngle", &eangle);
  ncircle.tmax = AY_R2D((double)eangle);

  /* copy object to the Ayam scene */
  ay_status = x3dio_linkobject(element, AY_IDNCIRCLE, (void*)&ncircle);

  if(ay_status == AY_OK)
    ay_notify_object(x3dio_lrobject);

 return ay_status;
} /* x3dio_readarc2d */


/* x3dio_readarcclose2d:
 *
 */
int
x3dio_readarcclose2d(scew_element *element)
{
 int ay_status = AY_OK;
 ay_ncircle_object ncircle = {0};
 ay_nurbcurve_object nc = {0};
 ay_nurbcurve_object *cl = NULL;
 ay_object onc = {0}, ocl = {0}, *o = NULL;
 float radius = 1.0f;
 float sangle = 0.0f;
 float eangle = (float)AY_HALFPI;
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;
 int ctype = 0, stride = 4;
 double *clcv = NULL;

  if(!element)
    return AY_ENULL;

  ncircle.radius = 1.0;
  x3dio_readfloat(element, "radius", &radius);
  if(radius > 0.0f)
    {
      ncircle.radius = radius;
    }
  else
    {
      return AY_ERROR;
    }

  ncircle.tmin = 0.0;
  x3dio_readfloat(element, "startAngle", &sangle);
  ncircle.tmin = AY_R2D((double)sangle);

  ncircle.tmax = 90.0;
  x3dio_readfloat(element, "endAngle", &eangle);
  ncircle.tmax = AY_R2D((double)eangle);

  ay_status = ay_nb_CreateNurbsCircleArc(ncircle.radius,
					 ncircle.tmin, ncircle.tmax,
			         &(nc.length), &(nc.knotv), &(nc.controlv));

  if(ay_status)
    return AY_ERROR;

  nc.order = 3;
  nc.knot_type = AY_KTCUSTOM;
  nc.is_rat = AY_TRUE;

  attr = scew_attribute_by_name(element, "closureType");
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  if(!strcmp(str, "PIE"))
	    {
	      ctype = 0;
	    }
	  if(!strcmp(str, "CHORD"))
	    {
	      ctype = 1;
	    }
	} /* if */
    } /* if */

  /* close arc */
  if(ctype == 0)
    {
      /* PIE */
      if(!(clcv = calloc(3*stride, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      memcpy(clcv, &(nc.controlv[(nc.length-1)*stride]),
	     stride*sizeof(double));
      /* weight of middle point */
      clcv[stride+3] = 1.0;
      memcpy(&(clcv[stride*2]), nc.controlv,
	     stride*sizeof(double));

      ay_status = ay_nct_create(2, 3, AY_KTNURB, clcv, NULL, &cl);
      if(ay_status)
	{goto cleanup;}
    }
  else
    {
      /* CHORD */
      if(!(clcv = calloc(2*stride, sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      memcpy(clcv, &(nc.controlv[(nc.length-1)*stride]),
	     stride*sizeof(double));
      memcpy(&(clcv[stride]), nc.controlv,
	     stride*sizeof(double));

      ay_status = ay_nct_create(2, 2, AY_KTNURB, clcv, NULL, &cl);
      if(ay_status)
	{goto cleanup;}
    } /* if */

  /* concatenate arc and closing curve */
  ay_status = ay_nct_elevate(cl, 3);

  if(ay_status)
    {goto cleanup;}

  onc.next = &ocl;
  ay_object_defaults(&onc);
  ay_object_defaults(&ocl);

  onc.refine = &nc;
  onc.type = AY_IDNCURVE;
  ocl.refine = cl;
  ocl.type = AY_IDNCURVE;

  ay_status = ay_nct_concatmultiple(AY_TRUE, 1, 0, &onc, &o);

  if(!o || !o->refine)
    {ay_status = AY_ERROR; goto cleanup;}

  /* copy object to the Ayam scene */
  ay_status = x3dio_linkobject(element, AY_IDNCURVE, o->refine);

cleanup:

  if(nc.knotv)
    free(nc.knotv);

  if(nc.controlv)
    free(nc.controlv);

  if(cl)
    {
      if(cl->controlv)
	free(cl->controlv);

      if(cl->knotv)
	free(cl->knotv);

      free(cl);
    }

  if(o)
    {
      ay_object_delete(o);
    }

 return ay_status;
} /* x3dio_readarcclose2d */


/* x3dio_readpolyline2d:
 *
 */
int
x3dio_readpolyline2d(scew_element *element, int contour)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object nc = {0};
 float *cv2d = NULL;
 unsigned int i, len = 0, stride = 4;

  if(!element)
    return AY_ENULL;

  if(contour)
    {
      ay_status = x3dio_readfloatpoints(element, "controlPoint", 2,
					&len, &cv2d);
    }
  else
    {
      ay_status = x3dio_readfloatpoints(element, "lineSegments", 2,
					&len, &cv2d);
    }

  if(len > 1)
    {
      nc.length = len;
      nc.order = 2;
      nc.knot_type = AY_KTNURB;

      if(!(nc.controlv = calloc(len, stride*sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}

      for(i = 0; i < len; i++)
	{
	  nc.controlv[i*stride]   = cv2d[i*2];
	  nc.controlv[i*stride+1] = cv2d[i*2+1];
	  nc.controlv[i*stride+3] = 1.0;
	}

      ay_status = ay_knots_createnc(&nc);

      if(ay_status)
	goto cleanup;

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDNCURVE, (void*)&nc);
    } /* if */

cleanup:

  if(cv2d)
    free(cv2d);

  if(nc.knotv)
    free(nc.knotv);

  if(nc.controlv)
    free(nc.controlv);

 return ay_status;
} /* x3dio_readpolyline2d */


/* x3dio_readnurbscurve:
 *
 */
int
x3dio_readnurbscurve(scew_element *element, unsigned int dim)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object nc = {0};
 float *cv = NULL;
 double *dcv = NULL, *w = NULL, *knots = NULL;
 unsigned int i, len = 0, wlen = 0, klen = 0, stride = 4;
 int order = 3;
 int is_double = AY_FALSE, has_weights = AY_FALSE, has_knots = AY_FALSE;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readint(element, "order", &order);

  ay_status = x3dio_readfloatpoints(element, "controlPoint", dim, &len, &cv);

  if(len == 0)
    {
      ay_status = x3dio_readcoords(element, &len, &dcv);
      is_double = AY_TRUE;
    }

  ay_status = x3dio_readdoublepoints(element, "weight", 1, &wlen, &w);
  if(wlen >= len)
    {
      has_weights = AY_TRUE;
    }

  ay_status = x3dio_readdoublepoints(element, "knot", 1, &klen, &knots);
  if(klen >= len+order)
    {
      has_knots = AY_TRUE;
      /* rescale knots to safe distance? */
      if(x3dio_rescaleknots != 0.0)
	{
	  ay_knots_rescaletomindist(klen, knots, x3dio_rescaleknots);
	}
    }

  if(len > 1)
    {
      nc.length = len;
      nc.order = order;
      nc.knot_type = AY_KTNURB;

      if(!(nc.controlv = calloc(len, stride*sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      if(!is_double)
	{
	  for(i = 0; i < len; i++)
	    {
	      nc.controlv[i*stride]   = (double)cv[i*dim];
	      nc.controlv[i*stride+1] = (double)cv[i*dim+1];
	      if(dim > 2)
		{
		  nc.controlv[i*stride+2] = (double)cv[i*dim+2];
		}
	      if(has_weights)
		{
		  nc.controlv[i*stride+3] = w[i];
		}
	      else
		{
		  nc.controlv[i*stride+3] = 1.0;
		}
	    } /* for */
	}
      else
	{
	  for(i = 0; i < len; i++)
	    {
	      memcpy(&(nc.controlv[i*stride]), &(dcv[i*3]), 3*sizeof(double));
	      if(has_weights)
		{
		  nc.controlv[i*stride+3] = w[i];
		}
	      else
		{
		  nc.controlv[i*stride+3] = 1.0;
		}
	    } /* for */
	} /* if */
      if(has_knots)
	{
	  nc.knot_type = ay_knots_classify(nc.order, knots, nc.length+nc.order,
					   AY_EPSILON);
	  nc.knotv = knots;
	  knots = NULL;
	}
      else
	{
	  nc.knot_type = AY_KTBSPLINE;
	  ay_status = ay_knots_createnc(&nc);
	  if(ay_status)
	    goto cleanup;
	}

      nc.is_rat = ay_nct_israt(&nc);

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDNCURVE, (void*)&nc);
    } /* if */

cleanup:

  if(cv)
    free(cv);

  if(dcv)
    free(dcv);

  if(w)
    free(w);

  if(knots)
    free(knots);

  if(nc.controlv)
    free(nc.controlv);

 return ay_status;
} /* x3dio_readnurbscurve */


/* x3dio_fixnpatch:
 *  fix row/column major order in np controlv (from X3D to Ayam style);
 */
int
x3dio_fixnpatch(ay_nurbpatch_object *np)
{
 int ay_status = AY_OK;
 int i, j, stride = 4;
 double *v = NULL, *p1, *p2;

  if(!np)
    return AY_ENULL;

  if(!(v = calloc(np->width * np->height * stride, sizeof(double))))
    return AY_EOMEM;

  p1 = v;
  for(i = 0; i < np->width; i++)
    {
      p2 = &(np->controlv[i*stride]);
      for(j = 0; j < np->height; j++)
	{
	  memcpy(p1, p2, stride * sizeof(double));

	  p1 += stride;
	  p2 += np->width*stride;
	} /* for */
    } /* for */

  free(np->controlv);
  np->controlv = v;

  np->is_rat = ay_npt_israt(np);

 return ay_status;
} /* x3dio_fixnpatch */


/* x3dio_readnurbspatchsurface:
 *
 */
int
x3dio_readnurbspatchsurface(scew_element *element, int is_trimmed)
{
 int ay_status = AY_OK;
 char *tcname = ay_prefs.texcoordname;
 ay_nurbpatch_object np = {0};
 ay_object *o, **old_aynext;
 float *cv = NULL, *tc = NULL;
 double *dcv = NULL, *w = NULL, *uknots = NULL, *vknots = NULL;
 double oldmin, oldmax;
 unsigned int i, len = 0, wlen = 0, uklen = 0, vklen = 0, tclen = 0;
 int width = 0, height = 0, uorder = 3, vorder = 3, stride = 4;
 int has_weights = AY_FALSE, has_uknots = AY_FALSE, has_vknots = AY_FALSE;
 int is_double = AY_FALSE, is_bound = AY_FALSE;
 scew_element *child = NULL;
 const char *element_name = NULL;
 x3dio_trafostate *old_state, notrafos;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readint(element, "uOrder", &uorder);
  ay_status = x3dio_readint(element, "vOrder", &vorder);
  ay_status = x3dio_readint(element, "uDimension", &width);
  ay_status = x3dio_readint(element, "vDimension", &height);

  ay_status = x3dio_readfloatpoints(element, "controlPoint", 3, &len, &cv);

  if(len == 0)
    {
      ay_status = x3dio_readcoords(element, &len, &dcv);
      is_double = AY_TRUE;
    }

  ay_status = x3dio_readfloatpoints(element, "texCoord", 2, &tclen, &tc);

  if(tclen == 0)
    {
      ay_status = x3dio_readtexcoords(element, &tclen, &tc);
    }

  ay_status = x3dio_readdoublepoints(element, "weight", 1, &wlen, &w);

  if(wlen >= len)
    {
      has_weights = AY_TRUE;
    }

  ay_status = x3dio_readdoublepoints(element, "uKnot", 1, &uklen, &uknots);
  if(uklen >= (unsigned int)width+uorder)
    {
      has_uknots = AY_TRUE;
      /* rescale knots to safe distance? */
      if(!is_trimmed && (x3dio_rescaleknots != 0.0))
	{
	  ay_knots_rescaletomindist(uklen, uknots, x3dio_rescaleknots);
	}
    }
  ay_status = x3dio_readdoublepoints(element, "vKnot", 1, &vklen, &vknots);
  if(vklen >= (unsigned int)height+vorder)
    {
      has_vknots = AY_TRUE;
      /* rescale knots to safe distance? */
      if(!is_trimmed && (x3dio_rescaleknots != 0.0))
	{
	  ay_knots_rescaletomindist(vklen, vknots, x3dio_rescaleknots);
	}
    }

  if(len > 1)
    {
      np.width = width;
      np.height = height;
      np.uorder = uorder;
      np.vorder = vorder;
      np.uknot_type = AY_KTNURB;
      np.vknot_type = AY_KTNURB;

      if(!(np.controlv = calloc(len, stride*sizeof(double))))
	{
	  ay_status = AY_EOMEM;
	  goto cleanup;
	}
      if(!is_double)
	{
	  for(i = 0; i < len; i++)
	    {
	      np.controlv[i*stride]   = cv[i*2];
	      np.controlv[i*stride+1] = cv[i*2+1];
	      np.controlv[i*stride+2] = cv[i*2+2];
	      if(has_weights)
		{
		  np.controlv[i*stride+3] = w[i];
		}
	      else
		{
		  np.controlv[i*stride+3] = 1.0;
		}
	    } /* for */
	}
      else
	{
	  for(i = 0; i < len; i++)
	    {
	      memcpy(&(np.controlv[i*stride]), &(dcv[i*3]), 3*sizeof(double));
	      if(has_weights)
		{
		  np.controlv[i*stride+3] = w[i];
		}
	      else
		{
		  np.controlv[i*stride+3] = 1.0;
		}
	    } /* for */
	} /* if */

      if(has_uknots)
	{
	  np.uknot_type = ay_knots_classify(np.uorder, uknots,
					    np.width+np.uorder,
					    AY_EPSILON);
	  np.uknotv = uknots;
	  uknots = NULL;
	}
      else
	{
	  np.uknot_type = AY_KTBSPLINE;
	}

      if(has_vknots)
	{
	  np.vknot_type = ay_knots_classify(np.vorder, vknots,
					    np.height+np.vorder,
					    AY_EPSILON);
	  np.vknotv = vknots;
	  vknots = NULL;
	}
      else
	{
	  np.vknot_type = AY_KTBSPLINE;
	}

      if(!has_uknots || !has_vknots)
	{
	  ay_status = ay_knots_createnp(&np);
	  if(ay_status)
	    goto cleanup;
	}

      /* fix row/column major order */
      ay_status = x3dio_fixnpatch(&np);

      /* copy object to the Ayam scene */
      x3dio_lrobject = NULL;
      ay_status = x3dio_linkobject(element, AY_IDNPATCH, (void*)&np);

      if(!x3dio_lrobject)
	{
	  ay_status = AY_ENULL;
	  goto cleanup;
	}

      /* set correct NURBS patch flags */
      x3dio_lrobject->parent = AY_TRUE;
      x3dio_lrobject->hide_children = AY_TRUE;
      x3dio_lrobject->inherit_trafos = AY_FALSE;

      /* add texture coordinates as PV tags */
      if(tclen > 0)
	{
	  ay_status = ay_pv_add(x3dio_lrobject, tcname, "varying", "g",
				tclen, 2, tc);
	} /* if */

      /* read trim curves? */
      if(is_trimmed)
	{
	  old_aynext = ay_next;
	  ay_next = &(x3dio_lrobject->down);
	  o = x3dio_lrobject;
	  x3dio_lrobject = NULL;

	  /* fake a clean (non transforming) transformation state */
	  old_state = x3dio_ctrafos;
	  x3dio_ctrafos = &(notrafos);
	  ay_trafo_identitymatrix(notrafos.m);

	  /* read all child elements */
	  while((child = scew_element_next(element, child)) != NULL)
	    {
	      element_name = scew_element_name(child);
	      if(!strcmp(element_name, "NurbsCurve") ||
		 !strcmp(element_name, "NurbsCurve2D") ||
		 !strcmp(element_name, "Contour2D") ||
		 !strcmp(element_name, "ContourPolyline2D"))
		{
		  ay_status = x3dio_readelement(child);
		  if(ay_status == AY_EDONOTLINK)
		    goto cleanup;
		}
	    } /* while */

	  /* reset the transformation state */
	  x3dio_ctrafos = old_state;

	  /* check for simple trim, if it is the only trim */
	  if((!x3dio_readstrim) && (o->down->next))
	    {
	      ay_status = ay_npt_isboundcurve(o->down,
					      np.uknotv[np.uorder-1],
				      np.uknotv[np.width],
					      np.vknotv[np.vorder-1],
				      np.vknotv[np.height],
					      &is_bound);
	      /* discard simple trim */
	      if(is_bound)
		{
		  (void)ay_object_deletemulti(o->down, AY_FALSE);
		  x3dio_lrobject = NULL;
		}
	    } /* if */

	  /* create endlevel object */
	  if(x3dio_lrobject)
	    {
	      x3dio_lrobject->next = ay_endlevel;
	    }
	  else
	    {
	      o->down = ay_endlevel;
	    }

	  /* rescale knots to safe distance? */
	  if(x3dio_rescaleknots > 0.0)
	    {
	      /* save old knot range */
	      oldmin = np.uknotv[0];
	      oldmax = np.uknotv[np.width+np.uorder-1];

	      /* rescale knots */
	      ay_knots_rescaletomindist(np.width + np.uorder, np.uknotv,
					x3dio_rescaleknots);

	      /* scale trim curves */
	      if(o->down && o->down->next)
		{
		  ay_status = ay_npt_rescaletrims(o->down, 0, oldmin, oldmax,
						  np.uknotv[0],
					  np.uknotv[np.width+np.uorder-1]);
		}

	      /* save old knot range */
	      oldmin = np.vknotv[0];
	      oldmax = np.vknotv[np.height+np.vorder-1];

	      /* rescale knots */
	      ay_knots_rescaletomindist(np.height + np.vorder, np.vknotv,
					x3dio_rescaleknots);

	      /* scale trim curves */
	      if(o->down && o->down->next)
		{
		  ay_status = ay_npt_rescaletrims(o->down, 1, oldmin, oldmax,
						  np.vknotv[0],
				 np.vknotv[np.height+np.vorder-1]);
		}

	    } /* if */

	  ay_next = old_aynext;
	}
      else
	{
	  x3dio_lrobject->down = ay_endlevel;
	} /* if */
    } /* if */

cleanup:

  if(cv)
    free(cv);

  if(dcv)
    free(dcv);

  if(w)
    free(w);

  if(uknots)
    free(uknots);

  if(vknots)
    free(vknots);

  if(np.controlv)
    free(np.controlv);

 return ay_status;
} /* x3dio_readnurbspatchsurface */


/* x3dio_readnurbssweptsurface:
 *
 */
int
x3dio_readnurbssweptsurface(scew_element *element)
{
 int ay_status = AY_OK;
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;
 scew_element *child;
 ay_object *o = NULL, **old_aynext, *cs = NULL;
 ay_sweep_object *sweep = NULL;
 const char *cs_name = "crossSectionCurve", *tr_name = "trajectoryCurve";
 double yaxis[3] = {0.0,1.0,0.0};
 x3dio_trafostate *old_state, notrafos;

  if(!element)
    return AY_ENULL;

  if(!(o = calloc(1, sizeof(ay_object))))
    {
      return AY_EOMEM;
    }

  if(!(sweep = calloc(1, sizeof(ay_sweep_object))))
    {
      free(o); return AY_EOMEM;
    }

  sweep->rotate = AY_TRUE;
  sweep->sections = 0;

  o->refine = sweep;

  ay_object_defaults(o);

  o->type = AY_IDSWEEP;

  o->parent = AY_TRUE;

  old_aynext = ay_next;
  ay_next = &(o->down);

  /* fake a clean (non transforming) transformation state */
  old_state = x3dio_ctrafos;
  x3dio_ctrafos = &(notrafos);
  ay_trafo_identitymatrix(notrafos.m);

  /* read children to get the cross section and the trajectory */
  child = NULL;
  while((child = scew_element_next(element, child)) != NULL)
    {
      attr = scew_attribute_by_name(child, "containerField");
      if(attr)
	{
	  str = scew_attribute_value(attr);
	  if(!strcmp(str, cs_name))
	    {
	      ay_status = x3dio_readelement(child);
	      if(ay_status == AY_EDONOTLINK)
		goto cleanup;
	      /* rotate cross section from XY to ZY plane */
	      cs = x3dio_lrobject;
	      if(cs)
		{
		  cs->roty = 90.0;
		  ay_quat_axistoquat(yaxis, AY_D2R(90.0), cs->quat);
		}
	    }
	}
    } /* while */

  child = NULL;
  while((child = scew_element_next(element, child)) != NULL)
    {
      attr = scew_attribute_by_name(child, "containerField");
      if(attr)
	{
	  str = scew_attribute_value(attr);
	  if(!strcmp(str, tr_name))
	    {
	      ay_status = x3dio_readelement(child);
	      if(ay_status == AY_EDONOTLINK)
		goto cleanup;
	    }
	}
    } /* while */

  /* reset the transformation state */
  x3dio_ctrafos = old_state;

  *ay_next = ay_endlevel;

  /* continue import in current level */
  ay_next = old_aynext;
  ay_object_link(o);

  ay_notify_object(o);

  ay_status = x3dio_readname(element, "DEF", o);

cleanup:

 return ay_status;
} /* x3dio_readnurbssweptsurface */


/* x3dio_readnurbsswungsurface:
 *
 */
int
x3dio_readnurbsswungsurface(scew_element *element)
{
 int ay_status = AY_OK;
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;
 scew_element *child;
 ay_object *o = NULL, **old_aynext;
 ay_swing_object *swing = NULL;
 const char *cs_name = "profileCurve", *tr_name = "trajectoryCurve";
 x3dio_trafostate *old_state, notrafos;

  if(!element)
    return AY_ENULL;

  if(!(o = calloc(1, sizeof(ay_object))))
    {
      return AY_EOMEM;
    }

  if(!(swing = calloc(1, sizeof(ay_swing_object))))
    {
      free(o); return AY_EOMEM;
    }

  o->refine = swing;

  ay_object_defaults(o);

  o->type = AY_IDSWING;

  o->parent = AY_TRUE;

  old_aynext = ay_next;
  ay_next = &(o->down);

  /* fake a clean (non transforming) transformation state */
  old_state = x3dio_ctrafos;
  x3dio_ctrafos = &(notrafos);
  ay_trafo_identitymatrix(notrafos.m);

  /* read children to get the cross section and the trajectory */
  child = NULL;
  while((child = scew_element_next(element, child)) != NULL)
    {
      attr = scew_attribute_by_name(child, "containerField");
      if(attr)
	{
	  str = scew_attribute_value(attr);
	  if(!strcmp(str, cs_name))
	    {
	      ay_status = x3dio_readelement(child);
	      if(ay_status == AY_EDONOTLINK)
		goto cleanup;
	    }
	}
    } /* while */

  child = NULL;
  while((child = scew_element_next(element, child)) != NULL)
    {
      attr = scew_attribute_by_name(child, "containerField");
      if(attr)
	{
	  str = scew_attribute_value(attr);
	  if(!strcmp(str, tr_name))
	    {
	      ay_status = x3dio_readelement(child);
	      if(ay_status == AY_EDONOTLINK)
		goto cleanup;
	    }
	}
    } /* while */

  /* reset the transformation state */
  x3dio_ctrafos = old_state;

  *ay_next = ay_endlevel;

  /* continue import in current level */
  ay_next = old_aynext;
  ay_object_link(o);

  ay_notify_object(o);

  ay_status = x3dio_readname(element, "DEF", o);

cleanup:

 return ay_status;
} /* x3dio_readnurbsswungsurface */


/* x3dio_readnurbsset:
 *
 */
int
x3dio_readnurbsset(scew_element *element)
{
 int ay_status = AY_OK;

  if(!element)
    return AY_ENULL;

  /**/

  ay_status = x3dio_readshape(element);

 return ay_status;
} /* x3dio_readnurbsset */


/* x3dio_readlight:
 *
 */
int
x3dio_readlight(scew_element *element, int type)
{
 int ay_status = AY_OK;
 ay_light_object light = {0};
 float intensity = 1.0f, color[3] = {1.0f, 1.0f, 1.0f};
 float ftemp = 0.0f, fvtemp[3] = {0};

  if(!element)
    return AY_ENULL;

  light.on = AY_TRUE;
  ay_status = x3dio_readbool(element, "on", &(light.on));

  ay_status = x3dio_readfloat(element, "intensity", &intensity);
  light.intensity = intensity;

  ay_status = x3dio_readfloatvec(element, "color", 3, color);
  light.colr = (int)(color[0]*255);
  light.colg = (int)(color[1]*255);
  light.colb = (int)(color[2]*255);

  switch(type)
    {
    case 0:
      /* directional light */
      light.type = AY_LITDISTANT;
      light.local = AY_TRUE;

      fvtemp[0] = 0;
      fvtemp[1] = 0;
      fvtemp[2] = -1;
      ay_status = x3dio_readfloatvec(element, "direction", 3, fvtemp);
      light.tto[0] = (double)fvtemp[0];
      light.tto[1] = (double)fvtemp[1];
      light.tto[2] = (double)fvtemp[2];

      break;
    case 1:
      /* point light */
      light.type = AY_LITPOINT;

      fvtemp[0] = 0;
      fvtemp[1] = 0;
      fvtemp[2] = 0;
      ay_status = x3dio_readfloatvec(element, "location", 3, fvtemp);
      light.tfrom[0] = (double)fvtemp[0];
      light.tfrom[1] = (double)fvtemp[1];
      light.tfrom[2] = (double)fvtemp[2];

      break;
    case 2:
      /* spot light */
      light.type = AY_LITSPOT;
      ftemp = (float)AY_HALFPI/2.0;
      ay_status = x3dio_readfloat(element, "cutOffAngle", &(ftemp));
      light.cone_angle = (double)ftemp;
      ftemp = (float)AY_HALFPI;
      ay_status = x3dio_readfloat(element, "beamWidth", &(ftemp));
      if((light.cone_angle - ftemp) > AY_EPSILON)
	{
	  light.cone_delta_angle = light.cone_angle - ftemp;
	}
      light.beam_distribution = 1.0;

      fvtemp[0] = 0;
      fvtemp[1] = 0;
      fvtemp[2] = 0;
      ay_status = x3dio_readfloatvec(element, "location", 3, fvtemp);
      light.tfrom[0] = (double)fvtemp[0];
      light.tfrom[1] = (double)fvtemp[1];
      light.tfrom[2] = (double)fvtemp[2];

      fvtemp[0] = 0;
      fvtemp[1] = 0;
      fvtemp[2] = -1;
      ay_status = x3dio_readfloatvec(element, "direction", 3, fvtemp);
      light.tto[0] = (double)fvtemp[0];
      light.tto[1] = (double)fvtemp[1];
      light.tto[2] = (double)fvtemp[2];
      break;
    default:
      return AY_OK;
      break;
    }

  /* copy object to the Ayam scene */
  ay_status = x3dio_linkobject(element, AY_IDLIGHT, (void*)&light);

 return ay_status;
} /* x3dio_readlight */


/* x3dio_readviewpoint:
 *
 */
int
x3dio_readviewpoint(scew_element *element)
{
 int ay_status = AY_OK;
 int width = 400, height = 300;
 float position[3] = {0.0f, 0.0f, 10.0f};
 float orient[4] = {0.0f, 0.0f, 1.0f, 0.0f};
 float fov = (float)AY_PI/4.0f;
 double m[16];
 double from[3], to[3], up[3] = {0.0, 1.0, 0.0};
 char command[255] = {0}, update_cmd[] = "update";
 ay_object *root = ay_root, *down, *last;
 ay_view_object *v = NULL;
 ay_camera_object c;
 x3dio_trafostate *trafo = NULL;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readfloatvec(element, "position", 3, position);

  from[0] = (double)position[0];
  from[1] = (double)position[1];
  from[2] = (double)position[2];

  ay_trafo_identitymatrix(m);

  trafo = x3dio_ctrafos;
  while(trafo)
    {
      ay_trafo_multmatrix(m, trafo->m);
      trafo = trafo->next;
    }
  ay_trafo_apply3(from, m);

  to[0] = from[0];
  to[1] = from[1];
  to[2] = from[2]-10.0;

  ay_status = x3dio_readfloatvec(element, "orientation", 4, orient);

  if(fabs(orient[3]) > AY_EPSILON)
    {
      ay_trafo_identitymatrix(m);
      ay_trafo_translatematrix(from[0], from[1], from[2], m);
      ay_trafo_rotatematrix(AY_R2D(orient[3]), orient[0], orient[1], orient[2],
			    m);
      ay_trafo_translatematrix(-from[0], -from[1], -from[2], m);
      ay_trafo_apply3(to, m);

      /* up is relative, need no translation component */
      ay_trafo_identitymatrix(m);
      ay_trafo_rotatematrix(AY_R2D(orient[3]), orient[0], orient[1], orient[2],
			    m);
      ay_trafo_apply3(up, m);
    }

  ay_status = x3dio_readfloat(element, "fieldOfView", &fov);

  if(x3dio_importviews == 1)
    {
      /* open a new view window */
      sprintf(command,"viewOpen %d %d 0\n", width, height);
      Tcl_Eval(ay_interp, command);

      Tcl_Eval(ay_interp, update_cmd);

      /* configure the view */
      down = root->down;
      last = root->down;
      while(down->next)
	{
	  last = down;
	  down = down->next;
	}

      v = (ay_view_object *)last->refine;
      memcpy(v->from, from, 3*sizeof(double));
      memcpy(v->to, to, 3*sizeof(double));
      memcpy(v->up, up, 3*sizeof(double));

      v->type = AY_VTPERSP;

      v->zoom = fabs(tan((double)fov/2.0));

      /* notify also includes reshape() and additionally loads the BGImage */
      ay_notify_object(last);

      /* set window title */
      sprintf(command,
	      "global ay;viewTitle [lindex $ay(views) end] Persp -");
      Tcl_Eval(ay_interp, command);

      Tcl_Eval(ay_interp, update_cmd);

      /* establish keyboard shortcuts and mouse bindings */
      sprintf(command,"global ay;viewBind [lindex $ay(views) end]");
      Tcl_Eval(ay_interp, command);

      Tcl_Eval(ay_interp, update_cmd);
    }
  else
    {
      memset(&c, 0, sizeof(ay_camera_object));

      memcpy(c.from, from, 3*sizeof(double));
      memcpy(c.to, to, 3*sizeof(double));
      memcpy(c.up, up, 3*sizeof(double));

      c.zoom = fabs(tan((double)fov/2.0));

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDCAMERA, (void*)&c);
    } /* if */

 return ay_status;
} /* x3dio_readviewpoint */


/* x3dio_readcadelement:
 *  type:
 *        0 - CADAssembly
 *        1 - CADFace
 *        2 - CADLayer
 *        3 - CADPart
 */
int x3dio_readcadelement(scew_element *element, int type)
{
 int ay_status = AY_OK;
 scew_element *child = NULL;
 ay_object *o = NULL, **old_aynext;
 unsigned int vislen = 0, i;
 int *vis = NULL;
 float scale[3] = {1.0f, 1.0f, 1.0f};
 float center[3] = {0.0f, 0.0f, 0.0f};
 float translation[3] = {0.0f, 0.0f, 0.0f};
 float rotation[4] = {0.0f, 0.0f, 1.0f, 0.0f};
 float scaleorient[4] = {0.0f, 0.0f, 1.0f, 0.0f};

  if(!element)
    return AY_ENULL;

  if(!(o = calloc(1, sizeof(ay_object))))
    {
      return AY_EOMEM;
    }

  if(!(o->refine = calloc(1, sizeof(ay_level_object))))
    {
      free(o); return AY_EOMEM;
    }

  ay_object_defaults(o);

  o->type = AY_IDLEVEL;
  o->parent = AY_TRUE;

  old_aynext = ay_next;
  ay_next = &(o->down);

  if(type == 3)
    {
      /* push transformation stack */
      x3dio_pushtrafo();

      /* get transformation parameters/attributes */
      (void)x3dio_readfloatvec(element, "scale", 3, scale);
      (void)x3dio_readfloatvec(element, "center", 3, center);
      (void)x3dio_readfloatvec(element, "translation", 3, translation);
      (void)x3dio_readfloatvec(element, "rotation", 4, rotation);
      (void)x3dio_readfloatvec(element, "scaleOrientation", 4,
			       scaleorient);

      /* apply trafos to current transformation stack matrix */
      ay_trafo_translatematrix(translation[0], translation[1], translation[2],
			       x3dio_ctrafos->m);

      ay_trafo_translatematrix(center[0], center[1], center[2],
			       x3dio_ctrafos->m);

      if(fabs(rotation[3]) > AY_EPSILON)
	{
	  ay_trafo_rotatematrix(AY_R2D(rotation[3]),
				rotation[0], rotation[1], rotation[2],
				x3dio_ctrafos->m);
	}

      if(fabs(scaleorient[3]) > AY_EPSILON)
	{
	  ay_trafo_rotatematrix(AY_R2D(scaleorient[3]), scaleorient[0],
				scaleorient[1], scaleorient[2],
				x3dio_ctrafos->m);
	}

      ay_trafo_scalematrix(scale[0], scale[1], scale[2],
			   x3dio_ctrafos->m);

      if(fabs(scaleorient[3]) > AY_EPSILON)
	{
	  ay_trafo_rotatematrix(-AY_R2D(scaleorient[3]), scaleorient[0],
				scaleorient[1], scaleorient[2],
				x3dio_ctrafos->m);
	}

      ay_trafo_translatematrix(-center[0], -center[1], -center[2],
			       x3dio_ctrafos->m);
    } /* if */

  /* read child elements */
  while((child = scew_element_next(element, child)) != NULL)
    {
      ay_status = x3dio_readelement(child);
      if(ay_status == AY_EDONOTLINK)
	break;
    }
  *ay_next = ay_endlevel;
  ay_next = old_aynext;
  ay_object_link(o);

  if(type == 3)
    {
      /* pop transformation stack */
      x3dio_poptrafo();
    }

  /* read name */
  ay_status = x3dio_readname(element, "name", o);

  if(type == 2)
    {
      /* read visible attribute */
      ay_status = x3dio_readboolvec(element, "visible", &vislen, &vis);

      /* apply visible attribute */
      o = o->down;
      for(i = 0; i < vislen; i++)
	{
	  if(!vis[i])
	    {
	      o->hide = AY_TRUE;
	    }
	  o = o->next;
	}

      if(vis)
	{
	  free(vis);
	}
    } /* if */

 return ay_status;
} /* x3dio_readcadelement */


/* x3dio_readinline:
 *
 */
int
x3dio_readinline(scew_element *element)
{
 int ay_status = AY_OK;
 char fname[] = "x3dio_readinline";
 char errstr[256];
 /*
 char arrname[] = "x3dio_options", varname[] = "IProgress";
 */
 scew_tree *tree = NULL;
 scew_parser *parser = NULL;
 scew_error errcode;
 enum XML_Error expat_code;
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;
 int load = AY_TRUE;
 Tcl_HashTable *old_x3dio_defs_ht = NULL;
 const char *filename;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readbool(element, "load", &load);

  if(load)
    {
      attr = scew_attribute_by_name(element, "url");
      if(attr)
	{
	  str = scew_attribute_value(attr);

	  filename = strchr(str, '"');

	  if(!strncmp("file:", str, 5))
	    {
	      /* "file://host.name/path/filename" */
	      filename = strchr(str, ':');
	      filename += 3;
	      if(*filename != '/')
		{
		  filename = strchr(filename, '/');
		}
	    }

	  ay_error(AY_EOUTPUT, fname, "Inlining file:");
	  ay_error(AY_EOUTPUT, fname, filename);

	  /* initialize XML parser */
	  parser = scew_parser_create();

	  scew_parser_ignore_whitespaces(parser, 1);

	  /* load an XML (X3D) file */
	  if(!scew_parser_load_file(parser, filename))
	    {
	      errcode = scew_error_code();
	      sprintf(errstr, "Unable to load file (error #%d: %s)\n",
		      errcode,
		      scew_error_string(errcode));
	      ay_error(AY_ERROR, fname, errstr);
	      if(errcode == scew_error_expat)
		{
		  expat_code = scew_error_expat_code(parser);
		  sprintf(errstr, "Expat error #%d (line %d, column %d): %s\n",
			  expat_code,
			  scew_error_expat_line(parser),
			  scew_error_expat_column(parser),
			  scew_error_expat_string(expat_code));
		  ay_error(AY_ERROR, fname, errstr);
		}
	      ay_status = AY_ERROR;
	      goto cleanup;
	    } /* if */

	  tree = scew_parser_tree(parser);

	  /* set progress */
	  /*
	  Tcl_SetVar2(ay_interp, arrname, varname, "50",
		      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};
	  */

	  if(!x3dio_mergeinlinedefs)
	    {
	      /* save old DEF hash-table */
	      old_x3dio_defs_ht = x3dio_defs_ht;
	      if(!(x3dio_defs_ht = calloc(1, sizeof(Tcl_HashTable))))
		{
		  ay_status = AY_EOMEM;
		  goto cleanup;
		}
	      Tcl_InitHashTable(x3dio_defs_ht, TCL_STRING_KEYS);
	    } /* if */

	  /* prevent advancing the main progress counter */
	  x3dio_inuse++;

	  /* convert XML tree to Ayam objects */
	  ay_status = x3dio_readtree(tree);

	  /* allow advancing the main progress counter */
	  x3dio_inuse--;

	  /* set progress */
	  /*
	  Tcl_SetVar2(ay_interp, arrname, varname, "100",
		      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};
	  */

	  if(!x3dio_mergeinlinedefs)
	    {
	      Tcl_DeleteHashTable(x3dio_defs_ht);
	      free(x3dio_defs_ht);
	      x3dio_defs_ht = old_x3dio_defs_ht;
	    }
	  else
	    {
	      /* to avoid crashes the DEF table must be cleaned
		 from references to the inlined XML tree*/
	      x3dio_removedefs(scew_tree_root(tree));
	    } /* if */

	  /* cleanup */
cleanup:
	  if(tree)
	    scew_tree_free(tree);

	  if(parser)
	    scew_parser_free(parser);

	} /* if */
    } /* if */

 return ay_status;
} /* x3dio_readinline */


/* x3dio_readappearance:
 *
 */
int
x3dio_readappearance(scew_element *element)
{

  if(!element)
    return AY_ENULL;

 return AY_OK;
} /* x3dio_readappearance */


/* x3dio_readtransform:
 *
 */
int
x3dio_readtransform(scew_element *element)
{
 int ay_status = AY_OK;
 scew_element *child = NULL;
 ay_object *o = NULL, **old_aynext = NULL;
 const char *element_name = NULL;
 int need_level = AY_FALSE;
 float scale[3] = {1.0f, 1.0f, 1.0f};
 float center[3] = {0.0f, 0.0f, 0.0f};
 float translation[3] = {0.0f, 0.0f, 0.0f};
 float rotation[4] = {0.0f, 0.0f, 1.0f, 0.0f};
 float scaleorient[4] = {0.0f, 0.0f, 1.0f, 0.0f};

  if(!element)
    return AY_ENULL;

  /* push transformation stack */
  x3dio_pushtrafo();

  /* get transformation parameters/attributes */
  ay_status = x3dio_readfloatvec(element, "scale", 3, scale);
  ay_status = x3dio_readfloatvec(element, "center", 3, center);
  ay_status = x3dio_readfloatvec(element, "translation", 3, translation);
  ay_status = x3dio_readfloatvec(element, "rotation", 4, rotation);
  ay_status = x3dio_readfloatvec(element, "scaleOrientation", 4, scaleorient);

  /* apply trafos to current transformation stack matrix */
  ay_trafo_translatematrix(translation[0], translation[1], translation[2],
			   x3dio_ctrafos->m);

  ay_trafo_translatematrix(center[0], center[1], center[2],
			   x3dio_ctrafos->m);

  if(fabs(rotation[3]) > AY_EPSILON)
    {
      ay_trafo_rotatematrix(AY_R2D(rotation[3]),
			    rotation[0], rotation[1], rotation[2],
			    x3dio_ctrafos->m);
    }

  if(fabs(scaleorient[3]) > AY_EPSILON)
    {
      ay_trafo_rotatematrix(AY_R2D(scaleorient[3]), scaleorient[0],
			    scaleorient[1], scaleorient[2],
			    x3dio_ctrafos->m);
    }
  ay_trafo_scalematrix(scale[0], scale[1], scale[2],
		       x3dio_ctrafos->m);
  if(fabs(scaleorient[3]) > AY_EPSILON)
    {
      ay_trafo_rotatematrix(-AY_R2D(scaleorient[3]), scaleorient[0],
			    scaleorient[1], scaleorient[2],
			    x3dio_ctrafos->m);
    }

  ay_trafo_translatematrix(-center[0], -center[1], -center[2],
			   x3dio_ctrafos->m);

  /* check children, if there are other transform elements,
     we need to create a level object with the current trafos */
  while((child = scew_element_next(element, child)) != NULL)
    {
      element_name = scew_element_name(child);
      if(!strcmp(element_name, "Transform"))
	{
	  need_level = AY_TRUE;
	  break;
	}
    } /* while */

  if(need_level)
    {
      if(!(o = calloc(1, sizeof(ay_object))))
	{
	  return AY_EOMEM;
	}

      if(!(o->refine = calloc(1, sizeof(ay_level_object))))
	{
	  free(o); return AY_EOMEM;
	}

      ay_object_defaults(o);

      o->type = AY_IDLEVEL;
      o->parent = AY_TRUE;

      /* set transformation attributes */
      ay_trafo_decomposematrix(x3dio_ctrafos->m, o);

      old_aynext = ay_next;
      ay_next = &(o->down);
    } /* if */

  /* read children */
  child = NULL;
  while((child = scew_element_next(element, child)) != NULL)
    {
      ay_status = x3dio_readelement(child);
      if(ay_status == AY_EDONOTLINK)
	break;
    }

  /* properly close level */
  if(need_level)
    {
      *ay_next = ay_endlevel;
      ay_next = old_aynext;
      ay_object_link(o);
      /* read shape name from DEF */
      ay_status = x3dio_readname(element, "DEF", o);
    }

  /* pop transformation stack */
  x3dio_poptrafo();

 return AY_OK;
} /* x3dio_readtransform */


/* x3dio_readshape:
 *
 */
int
x3dio_readshape(scew_element *element)
{
 int ay_status = AY_OK;
 scew_element *child = NULL;
 ay_object *o = NULL, **old_aynext;

  if(!element)
    return AY_ENULL;

  if(!(o = calloc(1, sizeof(ay_object))))
    {
      return AY_EOMEM;
    }

  if(!(o->refine = calloc(1, sizeof(ay_level_object))))
    {
      free(o); return AY_EOMEM;
    }

  ay_object_defaults(o);

  o->type = AY_IDLEVEL;
  o->parent = AY_TRUE;

  old_aynext = ay_next;
  ay_next = &(o->down);

  /* read child elements */
  while((child = scew_element_next(element, child)) != NULL)
    {
      ay_status = x3dio_readelement(child);
      if(ay_status == AY_EDONOTLINK)
	break;
    }

  /* how many children have been read? */
  if(o->down && o->down->next)
    {
      /* read more than one geometric element */
      /* => keep level */
      *ay_next = ay_endlevel;
      ay_next = old_aynext;
      ay_object_link(o);
      /* read shape name from DEF */
      ay_status = x3dio_readname(element, "DEF", o);
    }
  else
    {
      /* read one (or none) geometric element(s) */
      /* => remove level */
      ay_next = old_aynext;
      if(o->down)
	{
	  /* XXXX trafos of level object? */
	  ay_object_link(o->down);
	  /* if the object has no name already... */
	  if(!o->down->name)
	    {
	      /* ...read shape name from DEF */
	      ay_status = x3dio_readname(element, "DEF", o->down);
	    }
	}
      o->down = NULL;
      ay_object_delete(o);
    } /* if */

 return ay_status;
} /* x3dio_readshape */


/* x3dio_readmaterial:
 *
 */
int
x3dio_readmaterial(scew_element *element)
{
  /*
 int ay_status = AY_OK;
 scew_attribute *name_attr = NULL;
 const XML_Char *name_str = NULL;

  if(!element)
    return AY_ENULL;

  name_attr = scew_attribute_by_name(element, "diffuseColor");
  if(name_attr)
    {
      name_str = scew_attribute_value(name_attr);
    }
  */
 return AY_OK;
} /* x3dio_readmaterial */


/* x3dio_readscene:
 *
 */
int
x3dio_readscene(scew_element *element)
{
 int ay_status = AY_OK;
 scew_element *child = NULL;

  if(!element)
    return AY_ENULL;

  while((child = scew_element_next(element, child)) != NULL)
    {
      ay_status = x3dio_readelement(child);
      if(ay_status == AY_EDONOTLINK)
	break;
    }

 return ay_status;
} /* x3dio_readscene */


/* x3dio_adddef:
 *  add a definition with name <name> for element <element>
 *  (processes the DEF attribute)
 */
int
x3dio_adddef(char *name, scew_element *element)
{
 Tcl_HashEntry *entry = NULL;
 int new_item = 0;

  if(!name || !element)
    return AY_ENULL;

  if((entry = Tcl_FindHashEntry(x3dio_defs_ht, name)))
    {
      return AY_ERROR; /* name already registered */
    }
  else
    {
      entry = Tcl_CreateHashEntry(x3dio_defs_ht, name, &new_item);
      if(entry)
	{
	  Tcl_SetHashValue(entry, element);
	}
      else
	{
	  return AY_ERROR; /* ? */
	}
    } /* if */

 return AY_OK;
} /* x3dio_adddef */


/* x3dio_getdef:
 *  get the definition for <name> and put a pointer to the element
 *  into <element>
 *  (processes the USE attribute)
 */
int
x3dio_getdef(char *name, scew_element **element)
{
 Tcl_HashEntry *entry = NULL;

  if(!name || !element)
    return AY_ENULL;

  if((entry = Tcl_FindHashEntry(x3dio_defs_ht, name)))
    {
      *element = (scew_element*)Tcl_GetHashValue(entry);
    }
  else
    {
      return AY_ERROR; /* name not registered */
    }

 return AY_OK;
} /* x3dio_getdef */


/* x3dio_removedefs:
 *  _recursively_ remove all definitions from the DEF hashtable
 */
void
x3dio_removedefs(scew_element *element)
{
 scew_element *child = NULL;
 const XML_Char *str = NULL;
 Tcl_HashEntry *entry = NULL;
 scew_attribute *attr = NULL;

  if(!element)
    return;

  attr = scew_attribute_by_name(element, "DEF");
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  if((entry = Tcl_FindHashEntry(x3dio_defs_ht, str)))
	    {
	      Tcl_DeleteHashEntry(entry);
	    }
	}
    }

  while((child = scew_element_next(element, child)) != NULL)
    {
      x3dio_removedefs(child);
    }

 return;
} /* x3dio_removedefs */


/* x3dio_countelements:
 *  _recursively_ counts the child elements/nodes of <element>
 *  increases <counter> for each child
 */
void
x3dio_countelements(scew_element *element, unsigned int *counter)
{
 scew_element *child = NULL;

  while((child = scew_element_next(element, child)) != NULL)
    {
      (*counter)++;
      x3dio_countelements(child, counter);
    }

 return;
} /* x3dio_countelements */


/* x3dio_readelement:
 *
 */
int
x3dio_readelement(scew_element *element)
{
 int ay_status = AY_OK;
 char fname[] = "x3dio_readelement", *errstr = NULL;
 const char *element_name = NULL, *errfmt = "could not find element: %s";
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;
 unsigned int handled_elements = 0;
 int is_use = AY_FALSE;
 float progress;
 char progressstr[32];
 char arrname[] = "x3dio_options", varname1[] = "Progress";
 char varname2[] = "Cancel", *val = NULL;
 Tcl_Obj *to = NULL, *ton = NULL;

  if(!element)
    {
      return AY_ENULL;
    }

  /* handle DEF/USE attributes */
  attr = scew_attribute_by_name(element, "DEF");
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  ay_status = x3dio_adddef((char*)str, element);
	}
      else
	{
	  ay_error(AY_ERROR, fname, "malformed DEF attribute encountered");
	}
    }

  attr = scew_attribute_by_name(element, "USE");
  if(attr)
    {
      str = scew_attribute_value(attr);
      if(str)
	{
	  ay_status = x3dio_getdef((char*)str, &element);
	  if(ay_status)
	    {
	      if(!(errstr = calloc(strlen(errfmt) + strlen(str) + 2,
				   sizeof(char))))
		{
		  ay_error(AY_ERROR, fname, NULL);
		}
	      else
		{
		  sprintf(errstr, errfmt, str);
		  ay_error(AY_ERROR, fname, errstr);
		}
	      return AY_ERROR;
	    } /* if */
	  is_use = AY_TRUE;
	  x3dio_inuse++;
	}
      else
	{
	  ay_error(AY_ERROR, fname, "malformed USE attribute encountered");
	} /* if */
    } /* if */

  element_name = scew_element_name(element);

  switch((int)(element_name[0]))
    {
    case 'A':
      if(!strcmp(element_name, "Appearance"))
	{
	  ay_status = x3dio_readappearance(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "Arc2D"))
	{
	  ay_status = x3dio_readarc2d(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "ArcClose2D"))
	{
	  ay_status = x3dio_readarcclose2d(element);
	  handled_elements = 1;
	}
      break;
    case 'B':
      if(!strcmp(element_name, "Box"))
	{
	  ay_status = x3dio_readbox(element);
	  handled_elements = 1;
	}
      break;
    case 'C':
      if(!strcmp(element_name, "CADAssembly"))
	{
	  ay_status = x3dio_readcadelement(element, 0);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "CADFace"))
	{
	  ay_status = x3dio_readcadelement(element, 1);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "CADLayer"))
	{
	  ay_status = x3dio_readcadelement(element, 2);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "CADPart"))
	{
	  ay_status = x3dio_readcadelement(element, 3);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "Cylinder"))
	{
	  ay_status = x3dio_readcylinder(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "Collision"))
	{
	  ay_status = x3dio_readshape(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "Cone"))
	{
	  ay_status = x3dio_readcone(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "Circle2D"))
	{
	  ay_status = x3dio_readcircle2d(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "ContourPolyline2D"))
	{
	  ay_status = x3dio_readpolyline2d(element, AY_TRUE);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "Contour2D"))
	{
	  ay_status = x3dio_readshape(element);
	  handled_elements = 1;
	}
      break;
    case 'D':
      if(!strcmp(element_name, "Disk2D"))
	{
	  ay_status = x3dio_readdisk2d(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "DirectionalLight"))
	{
	  ay_status = x3dio_readlight(element, 0);
	  handled_elements = 1;
	}
      break;
    case 'E':
      if(!strcmp(element_name, "ElevationGrid"))
	{
	  ay_status = x3dio_readelevationgrid(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "Extrusion"))
	{
	  ay_status = x3dio_readextrusion(element);
	  handled_elements = 1;
	}
      break;
      /*
    case 'F':
      break;
      */
    case 'G':
      if(!strcmp(element_name, "Group"))
	{
	  ay_status = x3dio_readshape(element);
	  handled_elements = 1;
	}
      break;
      /*
    case 'H':
      break;
      */
    case 'I':
      if(!strcmp(element_name, "IndexedFaceSet"))
	{
	  ay_status = x3dio_readindexedfaceset(element);
	  x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "IndexedTriangleSet"))
	{
	  ay_status = x3dio_readindexedtriangleset(element);
	  x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "IndexedTriangleStripSet"))
	{
	  ay_status = x3dio_readindexedtrianglestripset(element);
	  x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "IndexedTriangleFanSet"))
	{
	  ay_status = x3dio_readindexedtrianglefanset(element);
	  x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "IndexedLineSet"))
	{
	  ay_status = x3dio_readindexedlineset(element);
	  x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "IndexedQuadSet"))
	{
	  ay_status = x3dio_readindexedquadset(element);
	  x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "Inline"))
	{
	  ay_status = x3dio_readinline(element);
	  handled_elements = 1;
	}
      break;
      /*
    case 'J':
      break;
    case 'K':
      break;
      */
    case 'L':
      if(!strcmp(element_name, "LineSet"))
	{
	  ay_status = x3dio_readlineset(element);
	  handled_elements = 1;
	}
      break;
    case 'M':
      if(!strcmp(element_name, "Material"))
	{
	  ay_status = x3dio_readmaterial(element);
	  handled_elements = 1;
	}
      break;
    case 'N':
      if(!strcmp(element_name, "NurbsCurve"))
	{
	  ay_status = x3dio_readnurbscurve(element, 3);
	  x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "NurbsCurve2D"))
	{
	  ay_status = x3dio_readnurbscurve(element, 2);
	  x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "NurbsPatchSurface"))
	{
	  ay_status = x3dio_readnurbspatchsurface(element, AY_FALSE);
	  x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "NurbsTrimmedSurface"))
	{
	  ay_status = x3dio_readnurbspatchsurface(element, AY_TRUE);
	  x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "NurbsSet"))
	{
	  ay_status = x3dio_readnurbsset(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "NurbsSweptSurface"))
	{
	  ay_status = x3dio_readnurbssweptsurface(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "NurbsSwungSurface"))
	{
	  ay_status = x3dio_readnurbsswungsurface(element);
	  handled_elements = 1;
	}
      break;
      /*
    case 'O':
      break;
      */
    case 'P':
      if(!strcmp(element_name, "Polyline2D"))
	{
	  ay_status = x3dio_readpolyline2d(element, AY_FALSE);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "PointLight"))
	{
	  ay_status = x3dio_readlight(element, 1);
	  handled_elements = 1;
	}
      break;
    case 'Q':
      if(!strcmp(element_name, "QuadSet"))
	{
	  ay_status = x3dio_readquadset(element);
	  handled_elements = 1;
	}
      break;
      /*
    case 'R':
      break;
      */
    case 'S':
      if(!strcmp(element_name, "Scene"))
	{
	  ay_status = x3dio_readscene(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "Shape"))
	{
	  ay_status = x3dio_readshape(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "Sphere"))
	{
	  ay_status = x3dio_readsphere(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "SpotLight"))
	{
	  ay_status = x3dio_readlight(element, 2);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "StaticGroup"))
	{
	  ay_status = x3dio_readshape(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "Switch"))
	{
	  ay_status = x3dio_readscene(element);
	  handled_elements = 1;
	}
      break;
    case 'T':
      if(!strcmp(element_name, "Transform"))
	{
	  ay_status = x3dio_readtransform(element);
	  handled_elements = 1; /* XXXX ? */
	}
      if(!strcmp(element_name, "TriangleFanSet"))
	{
	  ay_status = x3dio_readtrianglefanset(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "TriangleStripSet"))
	{
	  ay_status = x3dio_readtrianglestripset(element);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "TriangleSet"))
	{
	  ay_status = x3dio_readtriangleset(element);
	  handled_elements = 1;
	}
      break;
      /*
    case 'U':
      break;
      */
    case 'V':
      if(!strcmp(element_name, "Viewpoint"))
	{
	  if(x3dio_importviews)
	    {
	      ay_status = x3dio_readviewpoint(element);
	    }
	  handled_elements = 1;
	}
      break;
      /*
    case 'W':
      break;
    case 'X':
      break;
    case 'Y':
      break;
    case 'Z':
      break;
      */
    default:
      x3dio_countelements(element, &handled_elements);
      break;
    } /* switch */

  if(is_use)
    {
      x3dio_inuse--;
      if(x3dio_inuse == 0)
	{
	  handled_elements = 1;
	}
      else
	{
	  handled_elements = 0;
	}
    } /* if */

  /* calculate & report progress */
  x3dio_handledelements += handled_elements;
  progress = (float)x3dio_handledelements/(float)x3dio_totalelements;

  if(progress-x3dio_progress > 0.05)
    {
      sprintf(progressstr, "%d", (int)(50.0+progress*50.0f));
      Tcl_SetVar2(ay_interp, arrname, varname1, progressstr,
		  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

      x3dio_progress = progress;
    } /* if */

  /* also, check for cancel button */
  val = Tcl_GetVar2(ay_interp, arrname, varname2,
		    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(val && val[0] == '1')
    {
      ton = Tcl_NewStringObj("ay_error", -1);
      to = Tcl_NewIntObj(AY_EDONOTLINK);
      Tcl_ObjSetVar2(ay_interp, ton, NULL, to, TCL_LEAVE_ERR_MSG |
		     TCL_GLOBAL_ONLY);
      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
      return AY_EDONOTLINK;
    } /* if */

 return AY_OK;
} /* x3dio_readelement */


/* x3dio_readtree:
 *
 */
int
x3dio_readtree(scew_tree *tree)
{
 int ay_status = AY_OK;
 scew_element *element = NULL, *child = NULL;

  element = scew_tree_root(tree);

  while((child = scew_element_next(element, child)) != NULL)
    {
      ay_status = x3dio_readelement(child);
      if(ay_status == AY_EDONOTLINK)
	break;
    }

 return ay_status;
} /* x3dio_readtree */


/* x3dio_readtcmd:
 *  Tcl command to read X3D files
 */
int
x3dio_readtcmd(ClientData clientData, Tcl_Interp *interp,
	       int argc, char *argv[])
{
 int ay_status = AY_OK;
 char errstr[256];
 char *minus;
 int i = 2, slayer = -1, elayer = -1;
 double accuracy = 0.1;
 char arrname[] = "x3dio_options", varname[] = "Progress";
 scew_tree *tree = NULL;
 scew_parser *parser = NULL;
 scew_error errcode;
 enum XML_Error expat_code;
 scew_element *element = NULL, *child = NULL;


  /* set default import options and reset global counters */
  x3dio_importcurves = AY_TRUE;
  x3dio_readstrim = AY_TRUE;
  x3dio_importviews = 0;
  x3dio_rescaleknots = 0.0;
  x3dio_scalefactor = 1.0;
  x3dio_mergeinlinedefs = AY_FALSE;
  x3dio_totalelements = 0;
  x3dio_handledelements = 0;
  x3dio_progress = 0.0f;
  x3dio_inuse = 0;

  /* check args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "filename");
      goto cleanup;
    }

  /* parse args */
  while(i+1 < argc)
    {
      if(!strcmp(argv[i], "-a"))
	{
	  sscanf(argv[i+1], "%lg", &accuracy);
	}
      else
      if(!strcmp(argv[i], "-c"))
	{
	  sscanf(argv[i+1], "%d", &x3dio_importcurves);
	}
      else
      if(!strcmp(argv[i], "-v"))
	{
	  sscanf(argv[i+1], "%d", &x3dio_importviews);
	}
      else
      if(!strcmp(argv[i], "-r"))
	{
	  sscanf(argv[i+1], "%lg", &x3dio_rescaleknots);
	}
      else
      if(!strcmp(argv[i], "-s"))
	{
	  sscanf(argv[i+1], "%d", &x3dio_readstrim);
	}
      else
      if(!strcmp(argv[i], "-f"))
	{
	  sscanf(argv[i+1], "%lg", &x3dio_scalefactor);
	}
      else
      if(!strcmp(argv[i], "-m"))
	{
	  sscanf(argv[i+1], "%d", &x3dio_mergeinlinedefs);
	}
      else
      if(!strcmp(argv[i], "-l"))
	{
	  if(argv[i+1])
	    {
	      if(*argv[i+1] != '-')
		{
		  sscanf(argv[i+1], "%d", &slayer);
		  elayer = slayer;
		  if((strlen(argv[i+1]) > 3) &&
		     (minus = strchr(/*(const char*)*/(&(argv[i+1][1])), '-')))
		    {
		      minus++;
		      if(*minus != '\0')
			{
			  sscanf(minus, "%d", &elayer);
			}
		      else
			{
			  ay_error(AY_ERROR, argv[0],
	    "could not parse layer range, specify it as: startindex-endindex");
			  goto cleanup;
			} /* if */
		    } /* if */
		} /* if */
	    } /* if */
	} /* if */
      i += 2;
    } /* while */

  /* create and initialize hashtable for DEFs */
  if(!(x3dio_defs_ht = calloc(1, sizeof(Tcl_HashTable))))
    goto cleanup;
  Tcl_InitHashTable(x3dio_defs_ht, TCL_STRING_KEYS);

  /* initialize transformation stack */
  x3dio_pushtrafo();

  /* initialize XML parser */
  parser = scew_parser_create();

  scew_parser_ignore_whitespaces(parser, 1);

  /* load an XML (X3D) file */
  if(!scew_parser_load_file(parser, argv[1]))
    {
      errcode = scew_error_code();
      sprintf(errstr, "Unable to load file (error #%d: %s)\n",
	      errcode,
	      scew_error_string(errcode));
      ay_error(AY_ERROR, argv[0], errstr);
      if(errcode == scew_error_expat)
        {
	  expat_code = scew_error_expat_code(parser);
	  sprintf(errstr, "Expat error #%d (line %d, column %d): %s\n",
		  expat_code,
		  scew_error_expat_line(parser),
		  scew_error_expat_column(parser),
		  scew_error_expat_string(expat_code));
	  ay_error(AY_ERROR, argv[0], errstr);
        }
      goto cleanup;
    } /* if */

  tree = scew_parser_tree(parser);

  /* count elements */
  element = scew_tree_root(tree);
  while((child = scew_element_next(element, child)) != NULL)
    {
      x3dio_totalelements++;
      x3dio_countelements(child, &x3dio_totalelements);
    }

  /* set progress */
  Tcl_SetVar2(ay_interp, arrname, varname, "50",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  /* convert XML tree to Ayam objects */
  ay_status = x3dio_readtree(tree);
  if(ay_status == AY_EDONOTLINK)
    {
      ay_error(AY_EOUTPUT, argv[0],
	       "Import cancelled! Not all objects may have been read!");
    }

  /* set progress */
  Tcl_SetVar2(ay_interp, arrname, varname, "100",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  /* clean up */
cleanup:

  x3dio_cleartrafo();

  if(tree)
    scew_tree_free(tree);

  if(parser)
    scew_parser_free(parser);

  if(x3dio_defs_ht)
    Tcl_DeleteHashTable(x3dio_defs_ht);

 return TCL_OK;
} /* x3dio_readtcmd */


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

/* x3dio_count:
 *  _recursively_ count the objects to be exported
 */
unsigned int
x3dio_count(ay_object *o)
{
 unsigned int lcount = 0;
 int lasttype = -1;
 Tcl_HashTable *ht = &x3dio_write_ht;
 Tcl_HashEntry *entry = NULL;
 x3dio_writecb *cb = NULL;

  if(!o)
    return 0;

  while(o->next)
    {
      if(lasttype != (int)o->type)
	{
	  entry = NULL;
	  if((entry = Tcl_FindHashEntry(ht, (char *)(o->type))))
	    {
	      cb = (x3dio_writecb*)Tcl_GetHashValue(entry);
	    }
	  else
	    {
	      cb = NULL;
	    }
	  lasttype = o->type;
	} /* if */

      if(o->down && o->down->next /*&& (cb != x3dio_writenpconvertible)*/)
	lcount += x3dio_count(o->down);

      if(cb != NULL)
	lcount++;

      o = o->next;
    } /* while */

 return lcount;
} /* x3dio_count */


/* x3dio_registerwritecb:
 *  manage the write callback hash table
 */
int
x3dio_registerwritecb(char *name, x3dio_writecb *cb)
{
 int ay_status = AY_OK;
 int new_item = 0;
 Tcl_HashEntry *entry = NULL;
 Tcl_HashTable *ht = &x3dio_write_ht;

  if(!cb)
    return AY_ENULL;

  if((entry = Tcl_FindHashEntry(ht, name)))
    {
      return AY_ERROR; /* name already registered */
    }
  else
    {
      /* create new entry */
      entry = Tcl_CreateHashEntry(ht, name, &new_item);
      Tcl_SetHashValue(entry, (char*)cb);
    }

 return ay_status;
} /* x3dio_registerwritecb */


/* x3dio_writetransform:
 *
 */
void
x3dio_writetransform(scew_element *element, ay_object *o,
		     scew_element **transform_element)
{
 char buffer[256];
 double axis[3] = {0}, angle = 0.0, len = 0.0;

  if(!element || !o || !transform_element)
    return;

  *transform_element = scew_element_add(element, "Transform");

  if(AY_ISTRAFO(o))
    {

      /* process translation */
      if((o->movx != 0.0) || (o->movy != 0.0) || (o->movz != 0.0))
	{
	  sprintf(buffer, "%g %g %g", o->movx, o->movy, o->movz);
	  scew_element_add_attr_pair(*transform_element, "translation",
				     buffer);
	}

      /* process rotation */
      if((o->quat[0] != 0.0) || (o->quat[1] != 0.0) ||
	 (o->quat[2] != 0.0) || (o->quat[3] != 1.0))
	{
	  memcpy(axis, o->quat, 3*sizeof(double));

	  if(fabs(axis[0]) > AY_EPSILON ||
	     fabs(axis[1]) > AY_EPSILON ||
	     fabs(axis[2]) > AY_EPSILON)
	    {
	      len = AY_V3LEN(axis);
	      AY_V3SCAL(axis, 1.0/len);

	      angle = -2.0 * acos(o->quat[3]);

	      sprintf(buffer, "%g %g %g %g", axis[0], axis[1], axis[2], angle);
	      scew_element_add_attr_pair(*transform_element, "rotation",
					 buffer);
	    }
	}

      /* process scale */
      if((o->scalx != 1.0) || (o->scaly != 1.0) || (o->scalz != 1.0))
	{
	  sprintf(buffer, "%g %g %g", o->scalx, o->scaly, o->scalz);
	  scew_element_add_attr_pair(*transform_element, "scale",
				     buffer);
	}

    } /* if */

 return;
} /* x3dio_writetransform */


/* x3dio_clearmntags:
 * _recursively_ clear all MN tags from <o> its siblings and children
 */
void
x3dio_clearmntags(ay_object *o)
{
 ay_tag *tag = NULL, **last = NULL;

  if(!o)
    return;

  while(o)
    {
      if(o->tags)
	{
	  tag = o->tags;
	  last = &(o->tags);
	  tag = o->tags;
	  while(tag)
	    {
	      if(tag->type == x3dio_mn_tagtype)
		{
		  *last = tag->next;
		  tag->name = NULL;
		  ay_tags_free(tag);
		  tag = *last;
		}
	      else
		{
		  last = &(tag->next);
		  tag = tag->next;
		} /* if */
	    } /* while */
	} /* if */

      if(o->down)
	x3dio_clearmntags(o->down);

      o = o->next;
    } /* while */

 return;
} /* x3dio_clearmntags */


/* x3dio_writename:
 *
 */
int
x3dio_writename(scew_element *element, ay_object *o, int trafo)
{
 int ay_status = AY_OK;
 char *newname = NULL, *number = NULL;
 unsigned int len, i;
 static unsigned int count = 0;
 ay_tag *tag = NULL;

  if(!element || !o)
    {
      count = 0;
      return AY_ENULL;
    }

  /* write name as DEF */
  if(o->name && (o->name[0] != '\0'))
    {
      /* check, whether we already use this DEF */
      if(AY_ERROR == x3dio_adddef(o->name, element))
	{
	  /* DEF is already used, create a new name */
	  len = strlen(o->name);
	  if(!(newname = malloc((len+65)*sizeof(char))))
	    return AY_EOMEM;

	  memcpy(newname, o->name, len);
	  number = newname+len;
	  i = 2;
	  do
	    {
	      sprintf(number, "_%u", i++);
	    }
	  while(AY_ERROR == x3dio_adddef(newname, element));

	  scew_element_add_attr_pair(element, "DEF", newname);
	}
      else
	{
	  scew_element_add_attr_pair(element, "DEF", o->name);
	} /* if */
    }
  else
    {
      if(!x3dio_resolveinstances && o->refcount)
	{
	  count++;
	  if(!(newname = malloc(65*sizeof(char))))
	    return AY_EOMEM;

	  sprintf(newname, "Master_%u", count);

	  /* check, whether we already use this DEF */
	  if(AY_ERROR == x3dio_adddef(newname, element))
	    {
	      /* DEF is already used, create a new name */
	      do
		{
		  count++;
		  sprintf(newname, "Master_%u", count);
		}
	      while(AY_ERROR == x3dio_adddef(newname, element));
	    }

	  scew_element_add_attr_pair(element, "DEF", newname);
	} /* if */
    } /* if */

  if(!x3dio_resolveinstances && o->refcount)
    {
      /*  add a MN (MasterName) tag */
      if(!(tag = calloc(1, sizeof(ay_tag))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      if(newname)
	{
	  len = strlen(newname)+1;
	  if(!(tag->val = malloc(len*sizeof(char))))
	    { free(tag); ay_status = AY_EOMEM; goto cleanup; }
	  memcpy(tag->val, newname, len*sizeof(char));
	}
      else
	{
	  len = strlen(o->name)+1;
	  if(!(tag->val = malloc(len*sizeof(char))))
	    { free(tag); ay_status = AY_EOMEM; goto cleanup; }
	  memcpy(tag->val, o->name, len*sizeof(char));
	}

      tag->type = x3dio_mn_tagtype;
      tag->name = (char*)trafo;

      /* link tag to object o */
      tag->next = o->tags;
      o->tags = tag;
    }

cleanup:

  if(newname)
    free(newname);

 return ay_status;
} /* x3dio_writename */


/* x3dio_writeintattrib:
 *
 */
int
x3dio_writeintattrib(scew_element *element, char *name, int *value)
{
 char buf[64];

  if(!element || !name || !value)
    return AY_ENULL;

  sprintf(buf, "%d", *value);

  scew_element_add_attr_pair(element, name, buf);

 return AY_OK;
} /* x3dio_writeintattrib */


/* x3dio_writedoubleattrib:
 *
 */
int
x3dio_writedoubleattrib(scew_element *element, char *name, double *value)
{
 char buf[64];

  if(!element || !name || !value)
    return AY_ENULL;

  sprintf(buf, "%g", *value);

  scew_element_add_attr_pair(element, name, buf);

 return AY_OK;
} /* x3dio_writedoubleattrib */


/* x3dio_writedoublevecattrib:
 *
 */
int
x3dio_writedoublevecattrib(scew_element *element, char *name, unsigned int dim,
			   double *value)
{
 char buf[256];

  if(!element || !name || !value)
    return AY_ENULL;

  switch(dim)
    {
    case 1:
      sprintf(buf, "%g", *value);
      break;
    case 2:
      sprintf(buf, "%g %g", value[0], value[1]);
      break;
    case 3:
      sprintf(buf, "%g %g %g", value[0], value[1], value[2]);
      break;
    case 4:
      sprintf(buf, "%g %g %g %g", value[0], value[1], value[2], value[3]);
      break;
    default:
      return AY_ERROR;
      break;
    } /* switch */

  scew_element_add_attr_pair(element, name, buf);

 return AY_OK;
} /* x3dio_writedoublevecattrib */


/* x3dio_writedoublepoints:
 * TODO: use TCL_DOUBLE_SPACE for better initial estimation of allocation
 */
int
x3dio_writedoublepoints(scew_element *element, char *name, unsigned int dim,
		      unsigned int length, unsigned int stride, double *value)
{
 char buf[256];
 char *attr = NULL, *tmp;
 size_t buflen = 0, totalbuflen = 0;
 unsigned int i, a = 0;

  if(!element || !name || !value)
    return AY_ENULL;

  if(length == 0)
    return AY_ERROR;

  for(i = 0; i < length; i++)
    {
      switch(dim)
	{
	case 1:
	  buflen = sprintf(buf, "%g ", value[a]);
	  break;
	case 2:
	  buflen = sprintf(buf, "%g %g, ", value[a], value[a+1]);
	  break;
	case 3:
	  buflen = sprintf(buf, "%g %g %g, ", value[a], value[a+1],
			   value[a+2]);
	  break;
	case 4:
	  buflen = sprintf(buf, "%g %g %g %g, ", value[a], value[a+1],
			   value[a+2], value[a+3]);
	  break;
	default:
	  return AY_ERROR;
	  break;
	} /* switch */

      if(!(tmp = realloc(attr, (totalbuflen+buflen+1)*sizeof(char))))
	{
	  if(attr)
	    free(attr);
	  return AY_EOMEM;
	}
      attr = tmp;
      memcpy(&(attr[totalbuflen]), buf, buflen*sizeof(char));
      totalbuflen += buflen;
      a += stride;
    } /* for */

  attr[totalbuflen-1] = '\0';

  scew_element_add_attr_pair(element, name, attr);

 return AY_OK;
} /* x3dio_writedoublepoints */


/* x3dio_writeweights:
 *
 */
int
x3dio_writeweights(scew_element *element, char *name,
		   unsigned int length, double *value)
{
 char buf[256];
 char *attr = NULL, *tmp;
 size_t buflen = 0, totalbuflen = 0;
 unsigned int i, a = 0;
 unsigned int stride = 4;

  if(!element || !name || !value)
    return AY_ENULL;

  if(length == 0)
    return AY_ERROR;

  for(i = 0; i < length; i++)
    {
      buflen = sprintf(buf, "%g ", value[a+3]);

      if(!(tmp = realloc(attr, (totalbuflen+buflen+1)*sizeof(char))))
	{
	  if(attr)
	    free(attr);
	  return AY_EOMEM;
	}
      attr = tmp;
      memcpy(&(attr[totalbuflen]), buf, buflen*sizeof(char));
      totalbuflen += buflen;
      a += stride;
    } /* for */

  attr[totalbuflen-1] = '\0';

  scew_element_add_attr_pair(element, name, attr);

 return AY_OK;
} /* x3dio_writeweights */


/* x3dio_writeknots:
 *
 */
int
x3dio_writeknots(scew_element *element, char *name,
		 unsigned int length, double *value)
{
 char buf[256];
 char *attr = NULL, *tmp;
 size_t buflen = 0, totalbuflen = 0;
 unsigned int i;

  if(!element || !name || !value)
    return AY_ENULL;

  if(length == 0)
    return AY_ERROR;

  for(i = 0; i < length; i++)
    {
      buflen = sprintf(buf, "%g ", value[i]);

      if(!(tmp = realloc(attr, (totalbuflen+buflen+1)*sizeof(char))))
	{
	  if(attr)
	    free(attr);
	  return AY_EOMEM;
	}
      attr = tmp;
      memcpy(&(attr[totalbuflen]), buf, buflen*sizeof(char));
      totalbuflen += buflen;
    } /* for */

  attr[totalbuflen-1] = '\0';

  scew_element_add_attr_pair(element, name, attr);

 return AY_OK;
} /* x3dio_writeknots */


/* x3dio_writencurve:
 *
 */
int
x3dio_writencurve(scew_element *element, int dim, ay_nurbcurve_object *c)
{
 scew_element *curve_element = NULL;
 scew_element *coord_element = NULL;
 char buf[64];

  if(!c)
    return AY_ENULL;

  if(c->order == 2)
    {
      /* write the curve as PolyLine2D/LineSet */
      if(dim == 2)
	{
	  curve_element = scew_element_add(element, "Polyline2D");

	  x3dio_writedoublepoints(curve_element, "lineSegments", dim,
				  c->length, 4, c->controlv);
	}
      else
	{
	  curve_element = scew_element_add(element, "LineSet");

	  sprintf(buf, "%d", c->length);

	  scew_element_add_attr_pair(curve_element, "vertexCount", buf);

	  coord_element = scew_element_add(curve_element, "Coordinate");

	  x3dio_writedoublepoints(coord_element, "point", dim, c->length,
				  4, c->controlv);
	}
    }
  else
    {
      /* write the curve as NurbsCurve */
      if(dim == 2)
	curve_element = scew_element_add(element, "NurbsCurve2D");
      else
	curve_element = scew_element_add(element, "NurbsCurve");

      x3dio_writeintattrib(curve_element, "order", &c->order);

      x3dio_writeknots(curve_element, "knot", c->length+c->order, c->knotv);

      if(c->is_rat)
	{
	  x3dio_writeweights(curve_element, "weight", c->length, c->controlv);
	}

      coord_element = scew_element_add(curve_element, "Coordinate");

      x3dio_writedoublepoints(coord_element, "point", dim, c->length,
			      4, c->controlv);
    }

 return AY_OK;
} /* x3dio_writencurve */


/* x3dio_writencurveobj:
 *
 */
int
x3dio_writencurveobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *c;
 scew_element *transform_element = NULL;
 scew_element *shape_element = NULL;

  if(!element || !o || !o->refine)
    return AY_ENULL;

  c = (ay_nurbcurve_object *)o->refine;

  /* write transform */
  x3dio_writetransform(element, o, &transform_element);

  /* write shape */
  shape_element = scew_element_add(transform_element, "Shape");

  /* write name to shape element */
  ay_status = x3dio_writename(shape_element, o, AY_FALSE);

  if(ay_status)
    return ay_status;

  ay_status = x3dio_writencurve(shape_element, 3, c);

 return ay_status;
} /* x3dio_writencurveobj */


/* x3dio_writencconvertibleobj:
 *
 */
int
x3dio_writencconvertibleobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *c = NULL, *t;
 scew_element *transform_element = NULL;

  if(!x3dio_writecurves)
    return AY_OK;

  if(!o)
    return AY_ENULL;

  ay_status = ay_provide_object(o, AY_IDNCURVE, &c);
  if(!c)
    return AY_ERROR;

  /* write transform */
  if(!x3dio_resolveinstances && (o->refcount > 0))
    {
      /* write an extra outer transform level with our transformations
       * so that our instances may connect to the inner transform level
       * (without our transformations)
       */
      transform_element = scew_element_add(element, "Transform");
    }
  else
    {
      transform_element = element;
    }

  /* write name */
  ay_status = x3dio_writename(transform_element, o, AY_TRUE);

  t = c;
  while(t)
    {
      if(t->type == AY_IDNCURVE)
	{
	  ay_status = x3dio_writeobject(transform_element, t, AY_FALSE);
	}

      t = t->next;
    } /* while */

  (void)ay_object_deletemulti(c, AY_FALSE);

 return ay_status;
} /* x3dio_writencconvertibleobj */


/* x3dio_writetrimcurve:
 *
 */
int
x3dio_writetrimcurve(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *c = NULL;
 ay_nurbcurve_object *nc;
 int i, a = 0, stride = 4;
 double m[16] = {0};
 scew_element *curve_element = NULL;
 scew_element *coord_element = NULL;

  if(!element || !o || !o->refine)
    return AY_ENULL;

  if(o->type != AY_IDNCURVE)
    {
      ay_status = ay_provide_object(o, AY_IDNCURVE, &c);
      if(c && c->next)
	{
	  ay_status = x3dio_writetrimloop(element, c);
	  goto cleanup;
	}
    }
  else
    {
      ay_object_copy(o, &c);
    }

  if(!c)
    return AY_ERROR;

  nc = (ay_nurbcurve_object *)c->refine;

  /* apply trafos */
  ay_trafo_creatematrix(c, m);
  for(i = 0; i < nc->length; i++)
    {
      ay_trafo_apply4(&(nc->controlv[a]), m);
      a += stride;
    }

  /* now write the curve */
  if(nc->order == 2)
    {
      curve_element = scew_element_add(element, "ContourPolyline2D");

      x3dio_writedoublepoints(curve_element, "controlPoint", 2,
			      nc->length, 4, nc->controlv);
    }
  else
    {
      curve_element = scew_element_add(element, "NurbsCurve2D");

      x3dio_writeintattrib(curve_element, "order", &nc->order);

      x3dio_writeknots(curve_element, "knot", nc->length+nc->order, nc->knotv);

      if(nc->is_rat)
	{
	  x3dio_writeweights(curve_element, "weight", nc->length, nc->controlv);
	}

      coord_element = scew_element_add(curve_element, "Coordinate");
      x3dio_writedoublepoints(coord_element, "point", 2, nc->length, 4,
			      nc->controlv);
    }

cleanup:

  if(c)
    {
      (void)ay_object_deletemulti(c, AY_FALSE);
    }

 return ay_status;
} /* x3dio_writetrimcurve */


/* x3dio_writetrimloop:
 *
 */
int
x3dio_writetrimloop(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *down = NULL;

  if(!element || !o || !o->refine)
    return AY_ENULL;

  if(o->down && o->down->next)
    {
      down = o->down;
      while(down->next)
	{
	  ay_status = x3dio_writetrimcurve(element, down);

	  down = down->next;
	} /* while */
    } /* if */

 return ay_status;
} /* x3dio_writetrimloop */


/* x3dio_writenpatchobj:
 *
 */
int
x3dio_writenpatchobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 char *tcname = ay_prefs.texcoordname;
 ay_object *down = NULL;
 ay_nurbpatch_object *np;
 unsigned int i, j, stride = 4, copystride = 3;
 int have_texcoords = AY_FALSE;
 char *texcoordstring = NULL;
 double *v = NULL, *p1, *p2;
 ay_tag *tag;
 scew_element *transform_element = NULL;
 scew_element *shape_element = NULL;
 scew_element *patch_element = NULL;
 scew_element *coord_element = NULL;
 scew_element *texcoord_element = NULL;
 scew_element *contour_element = NULL;

  if(!element || !o || !o->refine)
    return AY_ENULL;

  /* write this patch as wire frame? */
  if(ay_tags_hastag(o, NULL, ay_aswire_tagtype))
    {
      /* yes */
      return x3dio_writenpwire(element, o);
    } /* if wire */

  /* decode potentially present PV tags */
  if(o->tags)
    {
      tag = o->tags;
      while(tag)
	{
	  if(ay_pv_checkndt(tag, tcname, "varying", "g"))
	    {
	      ay_status = x3dio_copypv(tag, &texcoordstring);
	      if(ay_status == AY_OK)
		{
		  have_texcoords = AY_TRUE;
		}
	      else
		{
		  /* XXXX report error (PV format error)? */
		}
	    }
	  tag = tag->next;
	} /* while */
    } /* if */

  np = (ay_nurbpatch_object *)o->refine;

  /* write transform */
  x3dio_writetransform(element, o, &transform_element);

  /* write shape */
  shape_element = scew_element_add(transform_element, "Shape");

  /* write name to shape element */
  ay_status = x3dio_writename(shape_element, o, AY_FALSE);

  if(x3dio_writemat)
    x3dio_writematerial(shape_element, o);

  /* now write the patch */
  if(o->down && o->down->next)
    {
      patch_element = scew_element_add(shape_element, "NurbsTrimmedSurface");
    }
  else
    {
      patch_element = scew_element_add(shape_element, "NurbsPatchSurface");
    }

  x3dio_writeintattrib(patch_element, "uOrder", &np->uorder);
  x3dio_writeintattrib(patch_element, "vOrder", &np->vorder);
  x3dio_writeintattrib(patch_element, "uDimension", &np->width);
  x3dio_writeintattrib(patch_element, "vDimension", &np->height);

  x3dio_writeknots(patch_element, "uKnot", np->width+np->uorder, np->uknotv);
  x3dio_writeknots(patch_element, "vKnot", np->height+np->vorder, np->vknotv);

  /* fix row/column major order */
  if(np->is_rat)
    {
      copystride = 4;
    }
  if(!(v = calloc(np->width * np->height * copystride, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  p1 = v;
  for(i = 0; i < (unsigned)np->height; i++)
    {
      p2 = &(np->controlv[i*stride]);
      for(j = 0; j < (unsigned)np->width; j++)
	{
	  memcpy(p1, p2, copystride*sizeof(double));
	  p1 += copystride;
	  p2 += np->height*stride;
	} /* for */
    } /* for */

  if(np->is_rat)
    {
      x3dio_writeweights(patch_element, "weight", np->width*np->height, v);
    }

  /* write coordinates */
  coord_element = scew_element_add(patch_element, "Coordinate");
  x3dio_writedoublepoints(coord_element, "point", 3, np->width*np->height,
			  copystride, v);

  /* write texture coordinates */
  if(have_texcoords)
    {
      texcoord_element = scew_element_add(patch_element, "TextureCoordinate");
      scew_element_add_attr_pair(texcoord_element, "point", texcoordstring);
    } /* if */

  /* write trim curves*/
  if(o->down && o->down->next)
    {
      down = o->down;
      while(down->next)
	{
	  contour_element = scew_element_add(patch_element, "Contour2D");
	  if(down->type == AY_IDLEVEL)
	    {
	      ay_status = x3dio_writetrimloop(contour_element, down);
	    }
	  else
	    {
	      ay_status = x3dio_writetrimcurve(contour_element, down);
	    }
	  down = down->next;
	} /* while */
    } /* if */

  /* write the caps and bevels */
  down = np->caps_and_bevels;
  while(down)
    {
      x3dio_writenpatchobj(transform_element, down);
      down = down->next;
    }

cleanup:

  if(v)
    free(v);

  if(texcoordstring)
    free(texcoordstring);

 return ay_status;
} /* x3dio_writenpatchobj */


/* x3dio_writewiremat:
 *
 */
int
x3dio_writewiremat(scew_element *shape_element)
{
 char buf[128];
 scew_element *app_element = NULL;
 scew_element *mat_element = NULL;

  if(!shape_element)
    return AY_ENULL;

  app_element = scew_element_add(shape_element, "Appearance");

  mat_element = scew_element_add(app_element, "Material");

  sprintf(buf, "%g %g %g", ay_prefs.obr, ay_prefs.obg, ay_prefs.obb);

  scew_element_add_attr_pair(mat_element, "diffuseColor", buf);

 return AY_OK;
} /* x3dio_writewiremat */


/* x3dio_writenpwire:
 *
 */
int
x3dio_writenpwire(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *c;
 ay_nurbpatch_object *np;
 char buf[64], *attr = NULL, *tmp;
 int a, b, i, j, k, m, n, p, q;
 int ulines, vlines, fulines, fvlines/*, spanu, spanv*/;
 /* int *spanus, *spanvs, *fspanus, *fspanvs;*/
 int idxsize = 0;
 double *P, *U, *V, *fd1, *fd2, *Ct;
 double l, u, v, ud, fud, vd, fvd;
 double N[3] = {0}, fder[9] = {0};
 double offset = 0.005;
 scew_element *transform_element = NULL;
 scew_element *shape_element = NULL;
 scew_element *line_element = NULL;
 scew_element *coord_element = NULL;

  if(!element || !o || !o->refine)
    return AY_ENULL;

  np = (ay_nurbpatch_object *)o->refine;

  if(ay_npt_istrimmed(o, 0))
    {
      return x3dio_writetrimmednpwire(element, o);
    }

  /* write transform */
  x3dio_writetransform(element, o, &transform_element);

  P = np->controlv;
  U = np->uknotv;
  V = np->vknotv;
  n = np->width;
  m = np->height;
  p = np->uorder-1;
  q = np->vorder-1;

  ulines = 1;
  for(a = p; a < n; a++)
    {
      if(U[a] != U[a+1])
	ulines++;
    }
  fulines = ulines*4;

  vlines = 1;
  for(a = q; a < m; a++)
    {
      if(V[a] != V[a+1])
	vlines++;
    }
  fvlines = vlines*4;

  ud = (U[n] - U[p]) / (ulines - 1);
  fud = (U[n] - U[p]) / (fulines - 1);
  vd = (V[m] - V[q]) / (vlines - 1);
  fvd = (V[m] - V[q]) / (fvlines - 1);

  if(!(Ct = malloc((fulines>fvlines?fulines:fvlines)*3*2*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

#if 0
  if(!(spanus = malloc((ulines+vlines)*sizeof(int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  spanvs = spanus + ulines;
  if(!(fspanus = malloc((fulines+fvlines)*sizeof(int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  fspanvs = fspanus + fulines;
  if(!(Nu = calloc(p+1+q+1, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  Nv = Nu + (p+1);

  /* employ linear variants of FindSpan() as they are much faster
     than a binary search; especially, since we calculate
     spans for all parameters in order */

  /* coarse lines */
  u = U[p];
  spanu = p;
  for(a = 0; a < ulines-1; a++)
    {
      if(u > U[p+1])
	{
	  while(u > U[spanu+1])
	    {
	      spanu++;
	    }
	}
      spanus[a] = spanu;

      u += ud;
    }
  spanus[a] = spanus[a-1];

  v = V[q];
  spanv = q;
  for(a = 0; a < vlines-1; a++)
    {
      if(v > V[q+1])
	{
	  while(v > V[spanv+1])
	    {
	      spanv++;
	    }
	}
      spanvs[a] = spanv;
      v += vd;
    }
  spanvs[a] = spanvs[a-1];

  /* fine lines */
  u = U[p];
  spanu = p;
  for(a = 0; a < fulines-1; a++)
    {
      if(u > U[p+1])
	{
	  while(u > U[spanu+1])
	    {
	      spanu++;
	    }
	}
      fspanus[a] = spanu;

      u += fud;
    }
  fspanus[a] = fspanus[a-1];

  v = V[q];
  spanv = q;
  for(a = 0; a < fvlines-1; a++)
    {
      if(v > V[q+1])
	{
	  while(v > V[spanv+1])
	    {
	      spanv++;
	    }
	}
      fspanvs[a] = spanv;
      v += fvd;
    }
  fspanvs[a] = fspanvs[a-1];
#endif

  /* v-lines */

  /* estimate memory needed to store the indices */
  idxsize = sprintf(buf, " %d", (fvlines*2));
  if(!(attr = malloc((idxsize*fvlines*2+10)*sizeof(char))))
    { ay_status = AY_EOMEM; goto cleanup; }
  tmp = attr;
  for(a = 0; a < fvlines; a++)
    {
      tmp += sprintf(tmp, " %d", a);
    } /* for */
  tmp += sprintf(tmp, " -1");
  for(a = fvlines; a < fvlines*2; a++)
    {
      tmp += sprintf(tmp, " %d", a);
    } /* for */
  memcpy(tmp, " -1", 4*sizeof(char));

  u = U[p];
  i = p;
  for(a = 0; a < ulines; a++)
    {
      /*spanu = spanus[a];
      ay_nb_DersBasisFuns(spanu, u, p, 1, U, Nu);*/

      v = V[q];
      j = 0;
      k = fvlines*3;
      for(b = 0; b < fvlines; b++)
	{
	  /*spanv = fspanvs[b];
	  ay_nb_DersBasisFuns(spanv, v, q, 1, V, Nv);*/

	  /* calculate point and normal */
	  if(np->is_rat)
	    ay_nb_CompFirstDerSurf4D(n-1, m-1, p, q, U, V, P, u, v, fder);
	  else
	    ay_nb_CompFirstDerSurf3D(n-1, m-1, p, q, U, V, P, u, v, fder);
	  fd1 = &(fder[3]);
	  fd2 = &(fder[6]);
	  AY_V3CROSS(N, fd2, fd1);
	  l = AY_V3LEN(N);
	  if(fabs(l) > AY_EPSILON)
	    AY_V3SCAL(N, 1.0/l);
	  /* offset point along normal */
	  AY_V3SCAL(N, offset);
	  Ct[j]   = fder[0]+N[0];
	  Ct[j+1] = fder[1]+N[1];
	  Ct[j+2] = fder[2]+N[2];
	  Ct[k]   = fder[0]-N[0];
	  Ct[k+1] = fder[1]-N[1];
	  Ct[k+2] = fder[2]-N[2];
	  j += 3;
	  k += 3;
	  v += fvd;
	} /* for */

      /* write all the data */
      shape_element = scew_element_add(transform_element, "Shape");

      x3dio_writewiremat(shape_element);

      line_element = scew_element_add(shape_element, "IndexedLineSet");

      scew_element_add_attr_pair(line_element, "coordIndex", attr);

      coord_element = scew_element_add(line_element, "Coordinate");

      x3dio_writedoublepoints(coord_element, "point", 3, fvlines*2, 3, Ct);

      /* skip multiple knots */
      do
	{
	  i++;
	}
      while((U[i] == U[i+1]) && i < n);
      u = U[i];
    } /* for */

  if(attr)
    free(attr);

  /* u-lines */

  /* estimate memory needed to store the indices */
  idxsize = sprintf(buf, " %d", (fulines*2));
  if(!(attr = malloc((idxsize*fulines*2+10)*sizeof(char))))
    { ay_status = AY_EOMEM; goto cleanup; }
  tmp = attr;
  for(a = 0; a < fulines; a++)
    {
      tmp += sprintf(tmp, " %d", a);
    } /* for */
  tmp += sprintf(tmp, " -1");
  for(a = fulines; a < fulines*2; a++)
    {
      tmp += sprintf(tmp, " %d", a);
    } /* for */
  memcpy(tmp, " -1", 4*sizeof(char));

  v = V[q];
  i = q;
  for(a = 0; a < vlines; a++)
    {
      /*spanv = spanvs[a];
	ay_nb_DersBasisFuns(spanv, v, q, 1, V, Nv);*/

      u = U[p];
      j = 0;
      k = fulines*3;
      for(b = 0; b < fulines; b++)
	{
	  /*spanu = fspanus[b];
	    ay_nb_DersBasisFuns(spanu, u, p, 1, U, Nu);*/

	  /* calculate point and normal */
	  if(np->is_rat)
	    ay_nb_CompFirstDerSurf4D(n-1, m-1, p, q, U, V, P, u, v, fder);
	  else
	    ay_nb_CompFirstDerSurf3D(n-1, m-1, p, q, U, V, P, u, v, fder);
	  fd1 = &(fder[3]);
	  fd2 = &(fder[6]);
	  AY_V3CROSS(N, fd2, fd1);
	  l = AY_V3LEN(N);
	  if(fabs(l) > AY_EPSILON)
	    AY_V3SCAL(N, 1.0/l);
	  /* offset point along normal */
	  AY_V3SCAL(N, offset);
	  Ct[j]   = fder[0]+N[0];
	  Ct[j+1] = fder[1]+N[1];
	  Ct[j+2] = fder[2]+N[2];
	  Ct[k]   = fder[0]-N[0];
	  Ct[k+1] = fder[1]-N[1];
	  Ct[k+2] = fder[2]-N[2];
	  j += 3;
	  k += 3;
	  u += fud;
	} /* for */

      /* write all the data */
      shape_element = scew_element_add(transform_element, "Shape");

      x3dio_writewiremat(shape_element);

      line_element = scew_element_add(shape_element, "IndexedLineSet");

      scew_element_add_attr_pair(line_element, "coordIndex", attr);

      coord_element = scew_element_add(line_element, "Coordinate");

      x3dio_writedoublepoints(coord_element, "point", 3, fulines*2, 3, Ct);

      /* skip multiple knots */
      do
	{
	  i++;
	}
      while((V[i] == V[i+1]) && i < m);
      v = V[i];
    } /* for */

  /* write the caps and bevels */
  c = np->caps_and_bevels;
  while(c)
    {
      x3dio_writenpwire(transform_element, c);
      c = c->next;
    }

cleanup:

  if(Ct)
    free(Ct);
  /*
  if(spanus)
    free(spanus);
  if(fspanus)
    free(fspanus);
  if(Nu)
    free(Nu);
  */
  if(attr)
    free(attr);

 return ay_status;
} /* x3dio_writenpwire */


/* x3dio_writetrimwire:
 *
 */
int
x3dio_writetrimwire(scew_element *element, ay_nurbpatch_object *np,
		    ay_stess_uvp *p1, ay_stess_uvp *p2)
{
 int ay_status = AY_OK;
 char buf[64], *attr = NULL, *tmp;
 int a, b, i, j, k, m, n, p, q;
 int idxsize = 0;
 double *P, *U, *V, *fd1, *fd2, *Ct = NULL;
 double l, N[3] = {0}, fder[9] = {0};
 double offset = 0.005;
 scew_element *shape_element = NULL;
 scew_element *line_element = NULL;
 scew_element *coord_element = NULL;
 ay_stess_uvp *uvp;

  if(!element || !np || !p1 || !p2)
    return AY_ENULL;

  if(p1 == p2)
    return AY_OK;

  P = np->controlv;
  U = np->uknotv;
  V = np->vknotv;
  n = np->width;
  m = np->height;
  p = np->uorder-1;
  q = np->vorder-1;

  /* estimate memory needed to store the indices */
  /* calculate total number of points/indices */
  a = 1;
  uvp = p1;
  while(uvp && uvp != p2)
    {
      a++;
      uvp = uvp->next;
    }

  b = a;
  idxsize = sprintf(buf, " %d", b*2);

  /* allocate and fill indices */
  if(!(attr = malloc((b*2*idxsize+10)*sizeof(char))))
    { ay_status = AY_EOMEM; goto cleanup; }
  tmp = attr;
  a = 0;
  for(i = 0; i < b; i++)
    {
      tmp += sprintf(tmp, " %d", a);
      a++;
    }
  tmp += sprintf(tmp, " -1");
  for(i = a; i < b*2; i++)
    {
      tmp += sprintf(tmp, " %d", a);
      a++;
    }
  tmp += sprintf(tmp, " -1");

  /* allocate and fill coordinates */
  if(!(Ct = malloc(b*2*3*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  j = 0;
  k = b*3;
  uvp = p1;
  for(i = 0; i < b; i++)
    {
      if(np->is_rat)
	ay_nb_CompFirstDerSurf4D(n-1, m-1, p, q, U, V, P,
				 uvp->u, uvp->v,
				 fder);
      else
	ay_nb_CompFirstDerSurf3D(n-1, m-1, p, q, U, V, P,
				 uvp->u, uvp->v,
				 fder);

      fd1 = &(fder[3]);
      fd2 = &(fder[6]);
      AY_V3CROSS(N, fd2, fd1);
      l = AY_V3LEN(N);
      if(fabs(l) > AY_EPSILON)
	AY_V3SCAL(N, 1.0/l);
      /* offset point along normal */
      AY_V3SCAL(N, offset);
      Ct[j]   = fder[0]+N[0];
      Ct[j+1] = fder[1]+N[1];
      Ct[j+2] = fder[2]+N[2];
      Ct[k]   = fder[0]-N[0];
      Ct[k+1] = fder[1]-N[1];
      Ct[k+2] = fder[2]-N[2];
      j += 3;
      k += 3;

      uvp = uvp->next;
    } /* for */

  /* write out all the data */
  shape_element = scew_element_add(element, "Shape");

  x3dio_writewiremat(shape_element);

  line_element = scew_element_add(shape_element, "IndexedLineSet");

  scew_element_add_attr_pair(line_element, "coordIndex", attr);

  coord_element = scew_element_add(line_element, "Coordinate");

  x3dio_writedoublepoints(coord_element, "point", 3, b*2, 3, Ct);

cleanup:

  if(Ct)
    free(Ct);

  if(attr)
    free(attr);

 return AY_OK;
} /* x3dio_writetrimwire */


/* x3dio_writetrimmednpwire:
 * Special version of x3dio_writetrimwire() for trimmed NURBS patches.
 *
 * As the UV-lines may be interrupted by the trim curves, we use the
 * STESS tesselation and instead of drawing line strips with OpenGL we
 * write out IndexedLineSet elements using x3dio_writetrimwire() above.
 *
 * We also tesselate and write out the trim curves themselves as a
 * single IndexedLineSet element.
 */
int
x3dio_writetrimmednpwire(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *c;
 ay_nurbpatch_object *np;
 char buf[64], *attr = NULL, *tmp;
 int a, b, i, j, k, m, n, p, q, i0, ii, jj;
 int idxsize = 0;
 double *P, *U, *V, *fd1, *fd2, *Ct = NULL;
 double l, N[3] = {0}, fder[9] = {0};
 double qf, offset = 0.005;
 scew_element *transform_element = NULL;
 scew_element *shape_element = NULL;
 scew_element *line_element = NULL;
 scew_element *coord_element = NULL;
 int clear_stess = AY_FALSE, out, tcslen = 0;
 ay_stess *stess = NULL;
 int *tcslens = NULL;
 double **tcs = NULL;
 ay_stess_uvp *uvpptr, *p1, *p2;

  if(!element || !o || !o->refine)
    return AY_ENULL;

  np = (ay_nurbpatch_object *)o->refine;

  /* write transform */
  x3dio_writetransform(element, o, &transform_element);

  if(!np->stess)
    {
      qf = ay_stess_GetQF(ay_prefs.glu_sampling_tolerance);

      ay_status = ay_stess_TessTrimmedNP(o, qf);
      if(ay_status)
	return ay_status;

      clear_stess = AY_TRUE;
    }

  stess = np->stess;

  /* write iso-u lines */
  out = 0;
  for(i = 0; i < stess->upslen; i++)
    {
      uvpptr = stess->ups[i];
      p1 = uvpptr;
      if(uvpptr && uvpptr->next)
	{
	  if(stess->ft_cw)
	    {
	      out = 0;
	      p1 = uvpptr;
	      /*
		glBegin(GL_LINE_STRIP);
	      */
	    }
	  else
	    out = 1;

	  while(uvpptr)
	    {
	      if(uvpptr->type > 0)
		{
		  if(out)
		    {
		      p1 = uvpptr;
		      /*
			glBegin(GL_LINE_STRIP);
			glVertex3dv((GLdouble*)(uvpptr->C));
		      */
		      out = 0;
		    }
		  else
		    {
		      p2 = uvpptr;
		      x3dio_writetrimwire(transform_element, np, p1, p2);
		      /*
			glVertex3dv((GLdouble*)(uvpptr->C));
			glEnd();
		      */
		      out = 1;
		    } /* if */
		}

	      uvpptr = uvpptr->next;
	    } /* while */

	  if(!out)
	    {
	      p2 = uvpptr;
	      x3dio_writetrimwire(transform_element, np, p1, p2);
	      /*
		glEnd();
	      */
	    }
	} /* if line has atleast two points */
    } /* for all u lines */

  /* write iso-v lines */
  out = 0;
  for(i = 0; i < stess->vpslen; i++)
    {
      uvpptr = stess->vps[i];
      p1 = uvpptr;
      if(uvpptr && uvpptr->next)
	{
	  if(stess->ft_cw)
	    {
	      out = 0;
	      p1 = uvpptr;
	      /*
		glBegin(GL_LINE_STRIP);
		glVertex3dv((GLdouble*)(uvpptr->C));
	      */
	    }
	  else
	    out = 1;

	  while(uvpptr)
	    {
	      if(uvpptr->type > 0)
		{
		  if(out)
		    {
		      p1 = uvpptr;
		      /*
			glBegin(GL_LINE_STRIP);
			glVertex3dv((GLdouble*)(uvpptr->C));
		      */
		      out = 0;
		    }
		  else
		    {
		      p2 = uvpptr;
		      x3dio_writetrimwire(transform_element, np, p1, p2);
		      /*
			glVertex3dv((GLdouble*)(uvpptr->C));
			glEnd();
		      */
		      out = 1;
		    } /* if */
		}

	      uvpptr = uvpptr->next;
	    } /* while */

	  if(!out)
	    {
	      p2 = uvpptr;
	      x3dio_writetrimwire(transform_element, np, p1, p2);
	      /*
		glEnd();
	      */
	    }
	} /* if line has atleast two points */
    } /* for all v lines */

  /* write trim curves */
  P = np->controlv;
  U = np->uknotv;
  V = np->vknotv;
  n = np->width;
  m = np->height;
  p = np->uorder-1;
  q = np->vorder-1;

  /* skip the outermost trim, if it lies on the boundary */
  ii = AY_FALSE;
  ay_npt_isboundcurve(o->down, U[p], U[n], V[q], V[m], &ii);
  if(ii == AY_TRUE)
    i0 = 1;
  else
    i0 = 0;

  qf = ay_stess_GetQF(ay_prefs.glu_sampling_tolerance);

  ay_status = ay_stess_TessTrimCurves(o, qf,
				      &tcslen, &tcs,
				      &tcslens, NULL);
  if(ay_status)
    goto cleanup;

  ay_status = ay_stess_ReTessTrimCurves(o, qf,
					tcslen, tcs,
					tcslens, NULL);
  if(ay_status)
    goto cleanup;

  /* estimate memory needed to store the indices */
  /* calculate total number of points/indices */
  a = 0;
  for(ii = i0; ii < tcslen; ii++)
    {
      a += tcslens[ii];
    }
  idxsize = sprintf(buf, " %d", a*2);
  /* allocate and fill indices */
  if(!(attr = malloc((a*2*idxsize+10)*sizeof(char))))
    { ay_status = AY_EOMEM; goto cleanup; }
  tmp = attr;
  a = 0;
  for(ii = i0; ii < tcslen; ii++)
    {
      for(jj = 0; jj < tcslens[ii]; jj++)
	{
	  tmp += sprintf(tmp, " %d", a);
	  a++;
	}
      tmp += sprintf(tmp, " -1");
    }
  b = a;
  for(ii = i0; ii < tcslen; ii++)
    {
      for(jj = 0; jj < tcslens[ii]; jj++)
	{
	  tmp += sprintf(tmp, " %d", a);
	  a++;
	}
      tmp += sprintf(tmp, " -1");
    }
  /* allocate and fill coordinates */
  if(!(Ct = malloc(a*3*2*sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  j = 0; k = b*3;
  for(ii = i0; ii < tcslen; ii++)
    {
      for(jj = 0; jj < tcslens[ii]; jj++)
	{
	  if(np->is_rat)
	    ay_nb_CompFirstDerSurf4D(n-1, m-1, p, q, U, V, P,
				     tcs[ii][jj*2], tcs[ii][jj*2+1],
				     fder);
	  else
	    ay_nb_CompFirstDerSurf3D(n-1, m-1, p, q, U, V, P,
				     tcs[ii][jj*2], tcs[ii][jj*2+1],
				     fder);

	  fd1 = &(fder[3]);
	  fd2 = &(fder[6]);
	  AY_V3CROSS(N, fd2, fd1);
	  l = AY_V3LEN(N);
	  if(fabs(l) > AY_EPSILON)
	    AY_V3SCAL(N, 1.0/l);
	  /* offset point along normal */
	  AY_V3SCAL(N, offset);
	  Ct[j]   = fder[0]+N[0];
	  Ct[j+1] = fder[1]+N[1];
	  Ct[j+2] = fder[2]+N[2];
	  Ct[k]   = fder[0]-N[0];
	  Ct[k+1] = fder[1]-N[1];
	  Ct[k+2] = fder[2]-N[2];
	  j += 3;
	  k += 3;
	} /* for */
    } /* for */

  /* write out all the data */
  shape_element = scew_element_add(transform_element, "Shape");

  x3dio_writewiremat(shape_element);

  line_element = scew_element_add(shape_element, "IndexedLineSet");

  scew_element_add_attr_pair(line_element, "coordIndex", attr);

  coord_element = scew_element_add(line_element, "Coordinate");

  x3dio_writedoublepoints(coord_element, "point", 3, b*2, 3, Ct);


  /* write the caps and bevels */
  c = np->caps_and_bevels;
  while(c)
    {
      x3dio_writenpwire(transform_element, c);
      c = c->next;
    }

cleanup:
  if(clear_stess)
    ay_stess_destroy(np);

  if(Ct)
    free(Ct);

  if(attr)
    free(attr);

  if(tcs)
    {
      for(i = 0; i < tcslen; i++)
	{
	  free(tcs[i]);
	}
      free(tcs);
    }

  if(tcslens)
    free(tcslens);

 return ay_status;
} /* x3dio_writetrimmednpwire */


/* x3dio_writenpconvertibleobj:
 *
 */
int
x3dio_writenpconvertibleobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *c = NULL, *t;
 scew_element *transform_element = NULL;

  if(!o)
   return AY_ENULL;

  ay_status = ay_provide_object(o, AY_IDNPATCH, &c);
  if(ay_status || !c)
    return AY_ERROR;

  /* write transform */
  if(!x3dio_resolveinstances && (o->refcount > 0))
    {
      /* write an extra outer transform level with our transformations
       * so that our instances may connect to the inner transform level
       * (without our transformations)
       */
      transform_element = scew_element_add(element, "Transform");
    }
  else
    {
      transform_element = element;
    }

  /* write name */
  ay_status = x3dio_writename(transform_element, o, AY_TRUE);

  t = c;
  while(t)
    {
      if(t->type == AY_IDNPATCH)
	{
	  ay_status = x3dio_writeobject(transform_element, t, AY_FALSE);
	}

      t = t->next;
    } /* while */

  (void)ay_object_deletemulti(c, AY_FALSE);

 return ay_status;
} /* x3dio_writenpconvertibleobj */


/* x3dio_writelevelobj:
 *
 */
int
x3dio_writelevelobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *down = NULL;
 scew_element *transform_element = NULL;
 scew_element *ot_element = NULL;

  if(!element || !o || !o->refine)
    return AY_ENULL;

  if(o == ay_endlevel)
    return AY_OK;

  if(o->down && o->down->next)
    {
      if(!x3dio_resolveinstances && (o->refcount > 0))
	{
	  /* write an extra outer transform level with our transformations
	   * so that our instances may connect to the inner transform level
	   * (without our transformations)
	   */
	  x3dio_writetransform(element, o, &ot_element);
	  transform_element = scew_element_add(ot_element, "Transform");
	}
      else
	{
	  /* write transform */
	  x3dio_writetransform(element, o, &transform_element);
	}

      /* write name */
      ay_status = x3dio_writename(transform_element, o, AY_TRUE);

      if(ay_status)
	return ay_status;

      /* write children */
      down = o->down;
      while(down->next)
	{
	  ay_status = x3dio_writeobject(transform_element, down, AY_TRUE);
	  down = down->next;
	}
    } /* if */

 return AY_OK;
} /* x3dio_writelevelobj */


/* x3dio_writecloneobj:
 *
 */
int
x3dio_writecloneobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_clone_object *cl;
 ay_object *clone, *firstclone = NULL, *down = NULL;
 scew_element *transform_element = NULL;
 scew_element *ot_element = NULL;

  if(!element || !o || !o->refine)
   return AY_ENULL;

  cl = (ay_clone_object *)o->refine;

  clone = cl->clones;

  /* clone object without clones? */
  if(!clone)
    return AY_OK;

  /* write transform */
  if(!x3dio_resolveinstances && (o->refcount > 0))
    {
      /* write an extra outer transform level with our transformations
       * so that our instances may connect to the inner transform level
       * (without our transformations)
       */
      x3dio_writetransform(element, o, &ot_element);
      transform_element = scew_element_add(ot_element, "Transform");
    }
  else
    {
      /* write transform */
      x3dio_writetransform(element, o, &transform_element);
    }

  /* write name */
  ay_status = x3dio_writename(transform_element, o, AY_TRUE);

  if(ay_status)
    return ay_status;

  if(!x3dio_resolveinstances && o->type != AY_IDMIRROR)
    {
      /* if the first child is not an instance, write the first clone
       * as normal object, otherwise the master to the clone instance
       * will never appear in the output file;
       * if the first child is an instance, its master is believed to
       * exist outside the Clone and its appearance in the output is
       * controlled by the user
       */
      down = o->down;
      if(down && down->next)
	{
	  if((down->type != AY_IDINSTANCE) && clone)
	    {
	      if(!(firstclone = calloc(1, sizeof(ay_object))))
		return AY_EOMEM;
	      memcpy(firstclone, down, sizeof(ay_object));
	      firstclone->refcount++;
	      ay_trafo_copy(clone, firstclone);

	      ay_status = x3dio_writeobject(transform_element, firstclone,
					    AY_FALSE);

	      if((firstclone->refcount == 1) && firstclone->tags &&
		 firstclone->tags->type == x3dio_mn_tagtype)
		{
		  firstclone->tags->next = down->tags;
		  down->tags = firstclone->tags;
		}

	      free(firstclone);

	      clone = clone->next;
	    } /* if */
	} /* if */
    } /* if */

  if(o->type == AY_IDMIRROR)
    {
      down = o->down;
      while(down && down->next)
	{
	  down->refcount++;
	  ay_status = x3dio_writeobject(transform_element, down, AY_TRUE);
	  down = down->next;
	}
    }

  while(clone)
    {
      ay_status = x3dio_writeobject(transform_element, clone, AY_FALSE);

      clone = clone->next;
    }

  if(o->type == AY_IDMIRROR)
    {
      down = o->down;
      while(down && down->next)
	{
	  down->refcount--;
	  down = down->next;
	}
    }

 return ay_status;
} /* x3dio_writecloneobj */


/* x3dio_writeinstanceobj:
 *
 */
int
x3dio_writeinstanceobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 char *masterdef = NULL;
 ay_object *master, tmp = {0};
 scew_element *transform_element = NULL, *itransform_element = NULL;
 scew_element *shape_element = NULL;

  if(!element || !o || !o->refine)
   return AY_ENULL;

  master = (ay_object *)o->refine;

  if(x3dio_resolveinstances)
    {
      memcpy(&tmp, master, 1*sizeof(ay_object));
      ay_trafo_defaults(&tmp);
      tmp.refcount = 0;

      /* write transform */
      x3dio_writetransform(element, o, &transform_element);

      /* write master object */
      ay_status = x3dio_writeobject(transform_element, &tmp, AY_FALSE);
    }
  else
    {
      if(master->tags && (master->tags->type == x3dio_mn_tagtype))
	{
	  masterdef = master->tags->val;
	}
      else
	{
	  return AY_ERROR;
	}

      /* write transform */
      x3dio_writetransform(element, o, &transform_element);

      if(!master->tags->name)
	{
	  /*
	   * write USE to shape element as the corresponding DEF
	   * is also in a shape element
	   */
	  shape_element = scew_element_add(transform_element, "Shape");

	  /* write USE to shape element */
	  scew_element_add_attr_pair(shape_element, "USE", masterdef);
	}
      else
	{
	  /*
	   * write USE to transform element as the corresponding DEF
	   * is also in a transform element
	   */
	  itransform_element = scew_element_add(transform_element,
						"Transform");

	  /* write USE to transform element */
	  scew_element_add_attr_pair(itransform_element, "USE", masterdef);
	}
    } /* if */

 return ay_status;
} /* x3dio_writeinstanceobj */


/* x3dio_writescriptobj:
 *
 */
int
x3dio_writescriptobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *cmo = NULL;
 ay_script_object *sc = NULL;

  if(!element || !o || !o->refine)
   return AY_ENULL;

  sc = (ay_script_object *)o->refine;

  if(((sc->type == 1) || (sc->type == 2)) && (sc->cm_objects))
    {
      cmo = sc->cm_objects;
      while(cmo)
	{
	  ay_status = x3dio_writeobject(element, cmo, AY_FALSE);
	  cmo = cmo->next;
	}
    } /* if */

 return ay_status;
} /* x3dio_writescriptobj */


/* x3dio_writeboxobj:
 *
 */
int
x3dio_writeboxobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_box_object *box;
 scew_element *transform_element = NULL;
 scew_element *shape_element = NULL;
 scew_element *box_element = NULL;
 double v[3] = {0};

  if(!element || !o || !o->refine)
   return AY_ENULL;

  box = (ay_box_object *)o->refine;

  /* write transform */
  x3dio_writetransform(element, o, &transform_element);

  /* write shape */
  shape_element = scew_element_add(transform_element, "Shape");

  /* write name to shape element */
  ay_status = x3dio_writename(shape_element, o, AY_FALSE);

  if(ay_status)
    return ay_status;

  if(x3dio_writemat)
    x3dio_writematerial(shape_element, o);

  /* now write the box */
  box_element = scew_element_add(shape_element, "Box");

  /* box parameters */
  v[0] = box->width;
  v[1] = box->length;
  v[2] = box->height;

  x3dio_writedoublevecattrib(box_element, "size", 3, v);

 return AY_OK;
} /* x3dio_writeboxobj */


/* x3dio_writesphereobj:
 *
 */
int
x3dio_writesphereobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_sphere_object *sphere;
 scew_element *transform_element = NULL, *itransform_element = NULL;
 scew_element *shape_element = NULL;
 scew_element *sphere_element = NULL;
 char buffer[256];

  if(!element || !o || !o->refine)
    return AY_ENULL;

  sphere = (ay_sphere_object *)o->refine;

  if(fabs(sphere->zmin) >= sphere->radius &&
     fabs(sphere->zmax) >= sphere->radius &&
     sphere->thetamax == 360.0)
    {
      /* write transform */
      x3dio_writetransform(element, o, &transform_element);

      itransform_element = scew_element_add(transform_element, "Transform");

      sprintf(buffer, "1.0 0.0 0.0 %g", AY_HALFPI);
      scew_element_add_attr_pair(itransform_element, "rotation",
				 buffer);

      /* write shape */
      shape_element = scew_element_add(itransform_element, "Shape");

      /* write name to shape element */
      ay_status = x3dio_writename(shape_element, o, AY_FALSE);

      if(ay_status)
	return ay_status;

      if(x3dio_writemat)
	x3dio_writematerial(shape_element, o);

      /* now write the sphere */
      sphere_element = scew_element_add(shape_element, "Sphere");

      /* sphere parameters */
      if(fabs(sphere->radius))
	{
	  x3dio_writedoubleattrib(sphere_element, "radius", &sphere->radius);
	}
    }
  else
    {
      ay_status = x3dio_writenpconvertibleobj(element, o);
    }

 return ay_status;
} /* x3dio_writesphereobj */


/* x3dio_writecylinderobj:
 *
 */
int
x3dio_writecylinderobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_cylinder_object *cylinder = NULL;
 double height = 0.0;
 scew_element *transform_element = NULL, *itransform_element = NULL;
 scew_element *shape_element = NULL;
 scew_element *cylinder_element = NULL;
 char buffer[256];

  if(!element || !o || !o->refine)
    return AY_ENULL;

  cylinder = (ay_cylinder_object *)o->refine;

  if(cylinder->is_simple)
    {
      height = cylinder->zmax-cylinder->zmin;

      /* write transform */
      x3dio_writetransform(element, o, &transform_element);

      itransform_element = scew_element_add(transform_element, "Transform");

      sprintf(buffer, "0.0 0.0 %g",
	      cylinder->zmin+(height/2.0));

      scew_element_add_attr_pair(itransform_element, "translation",
				 buffer);

      sprintf(buffer, "1.0 0.0 0.0 %g", AY_HALFPI);
      scew_element_add_attr_pair(itransform_element, "rotation",
				 buffer);

      /* write shape */
      shape_element = scew_element_add(itransform_element, "Shape");

      /* write name to shape element */
      ay_status = x3dio_writename(shape_element, o, AY_FALSE);

      if(ay_status)
	return ay_status;

      if(x3dio_writemat)
	x3dio_writematerial(shape_element, o);

      /* now write the cylinder */
      cylinder_element = scew_element_add(shape_element, "Cylinder");

      /* cylinder parameters */

      x3dio_writedoubleattrib(cylinder_element, "radius",
			      &cylinder->radius);

      x3dio_writedoubleattrib(cylinder_element, "height",
			      &height);
      if(!cylinder->closed)
	{
	  scew_element_add_attr_pair(cylinder_element, "bottom",
				     x3dio_falsestring);
	  scew_element_add_attr_pair(cylinder_element, "top",
				     x3dio_falsestring);
	}
    }
  else
    {
      ay_status = x3dio_writenpconvertibleobj(element, o);
    }

 return ay_status;
} /* x3dio_writecylinderobj */


/* x3dio_writeconeobj:
 *
 */
int
x3dio_writeconeobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_cone_object *cone = NULL;
 scew_element *transform_element = NULL, *itransform_element = NULL;
 scew_element *shape_element = NULL;
 scew_element *cone_element = NULL;
 char buffer[256];

  if(!element || !o || !o->refine)
    return AY_ENULL;

  cone = (ay_cone_object *)o->refine;

  if(cone->is_simple)
    {
      /* write transform */
      x3dio_writetransform(element, o, &transform_element);

      itransform_element = scew_element_add(transform_element, "Transform");

      sprintf(buffer, "0.0 0.0 %g", cone->height/2.0);
      scew_element_add_attr_pair(itransform_element, "translation",
				 buffer);

      sprintf(buffer, "1.0 0.0 0.0 %g", AY_HALFPI);
      scew_element_add_attr_pair(itransform_element, "rotation",
				 buffer);

      /* write shape */
      shape_element = scew_element_add(itransform_element, "Shape");

      /* write name to shape element */
      ay_status = x3dio_writename(shape_element, o, AY_FALSE);

      if(ay_status)
	return ay_status;

      if(x3dio_writemat)
	x3dio_writematerial(shape_element, o);

      /* now write the cone */
      cone_element = scew_element_add(shape_element, "Cone");

      /* cone parameters */
      x3dio_writedoubleattrib(cone_element, "bottomRadius",
			      &cone->radius);

      x3dio_writedoubleattrib(cone_element, "height",
			      &cone->height);
      if(!cone->closed)
	{
	  scew_element_add_attr_pair(cone_element, "bottom",
				     x3dio_falsestring);
	}
    }
  else
    {
      ay_status = x3dio_writenpconvertibleobj(element, o);
    }

 return ay_status;
} /* x3dio_writeconeobj */


/* x3dio_copypv:
 *  create a copy of the string in <src>,
 *  but replace all comma "," by space " "
 *  caller must free the memory in *dst if no longer needed
 */
int
x3dio_copypv(ay_tag *src, char **dst)
{
 char *tmp = NULL, *srcptr, *dstptr;
 unsigned int len = 0, i = 0;

  if(!src || !dst)
    return AY_ENULL;

  /* find the data portion of the PV tags value string */
  srcptr = src->val;
  for(i = 0; i < 4; i++)
    {
      if(!srcptr)
	{
	  /* probably PV format error */
	  return AY_ERROR;
	}
      else
	{
	  srcptr = strchr(srcptr, ',');
	  srcptr++;
	}
    }

  len = strlen(srcptr);
  if(!(tmp = calloc(len+1, sizeof(char))))
    {
      return AY_EOMEM;
    }

  dstptr = tmp;
  for(i = 0; i < len; i++)
    {
      /* replace comma by space, otherwise just copy the data */
      if(*srcptr == ',')
	*dstptr = ' ';
      else
	*dstptr = *srcptr;
      srcptr++;
      dstptr++;
    }

  /* return result */
  if(*dst)
    free(*dst);
  *dst = tmp;

 return AY_OK;
} /* x3dio_copypv */


/* x3dio_writepomeshobj:
 *
 */
int
x3dio_writepomeshobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 /*char fname[] = "x3dio_writepomesh";*/
 ay_object *to = NULL;
 ay_list_object *li = NULL, **nextli = NULL, *lihead = NULL;
 ay_pomesh_object *po;
 unsigned int stride;
 unsigned int i, j, k, p = 0, q = 0, r = 0;
 int have_texcoords = AY_FALSE, have_facenormals = AY_FALSE;
 int have_vcolors = AY_FALSE, have_fcolors = AY_FALSE;
 int have_alpha = AY_FALSE;
 int idxsize = 0;
 char *texcoordstring = NULL, *normalstring = NULL;
 char *colorstring = NULL;
 ay_tag *tag;
 scew_element *transform_element = NULL;
 scew_element *shape_element = NULL;
 scew_element *ifs_element = NULL;
 scew_element *coord_element = NULL;
 scew_element *normal_element = NULL;
 scew_element *color_element = NULL;
 scew_element *texcoord_element = NULL;
 char buf[256];
 char *attr = NULL, *nattr = NULL, *tmp;
 size_t buflen = 0, totalbuflen = 0;

  if(!element || !o)
   return AY_ENULL;

  if(ay_tags_hastag(o, NULL, ay_aswire_tagtype))
    return x3dio_writepomeshwire(element, o);

  po = (ay_pomesh_object *)o->refine;

  /* write transform */
  x3dio_writetransform(element, o, &transform_element);

  /* write shape */
  shape_element = scew_element_add(transform_element, "Shape");

  /* write name to shape element */
  ay_status = x3dio_writename(shape_element, o, AY_FALSE);

  if(x3dio_writemat)
    x3dio_writematerial(shape_element, o);

  /* write the IndexedFaceSet element */
  ifs_element = scew_element_add(shape_element, "IndexedFaceSet");

  /* decode potentially present PV tags */
  if(o->tags)
    {
      tag = o->tags;
      while(tag)
	{
	  if(tag->type == ay_pv_tagtype)
	    {
	      if(ay_pv_checkndt(tag, ay_prefs.texcoordname, "varying", "g"))
		{
		  ay_status = x3dio_copypv(tag, &texcoordstring);
		  if(ay_status == AY_OK)
		    {
		      have_texcoords = AY_TRUE;
		    }
		}
	      else
	      if((ay_pv_checkndt(tag, ay_prefs.colorname, "varying", "c")) ||
		 (ay_pv_checkndt(tag, ay_prefs.colorname, "varying", "d")))
		{
		  ay_status = x3dio_copypv(tag, &colorstring);
		  if(ay_status == AY_OK)
		    {
		      have_vcolors = AY_TRUE;
		      have_fcolors = AY_FALSE;
		      if(ay_pv_checkndt(tag, ay_prefs.colorname,
					"varying", "d"))
			have_alpha = AY_TRUE;
		      else
			have_alpha = AY_FALSE;
		    }
		}
	      else
	      if((ay_pv_checkndt(tag, ay_prefs.colorname, "uniform", "c")) ||
		 (ay_pv_checkndt(tag, ay_prefs.colorname, "uniform", "d")))
		{
		  ay_status = x3dio_copypv(tag, &colorstring);
		  if(ay_status == AY_OK)
		    {
		      have_vcolors = AY_FALSE;
		      have_fcolors = AY_TRUE;
		      if(ay_pv_checkndt(tag, ay_prefs.colorname,
					"uniform", "d"))
			have_alpha = AY_TRUE;
		      else
			have_alpha = AY_FALSE;
		    }
		}
	      else
	      if(ay_pv_checkndt(tag, ay_prefs.normalname, "uniform", "n"))
		{
		  ay_status = x3dio_copypv(tag, &normalstring);
		  if(ay_status == AY_OK)
		    {
		      have_facenormals = AY_TRUE;
		    }
		}
	    } /* if */

	  tag = tag->next;
	} /* while */
    } /* if */

  /* write faces */
  for(i = 0; i < po->npolys; i++)
    {
      if(po->nloops[i] == 1)
	{
	  /* this face has just one loop (no hole) */

	  /* XXXX this "for" unneeded? */
	  for(j = 0; j < po->nloops[p]; j++)
	    {
	      if(!x3dio_tesspomesh ||
		 (x3dio_tesspomesh && (po->nverts[q] == 3)))
		{

		  for(k = 0; k < po->nverts[q]; k++)
		    {
		      buflen = sprintf(buf, " %u", po->verts[r]);

		      if(!(tmp = realloc(attr,
					 (totalbuflen+buflen+1)*sizeof(char))))
			{ ay_status = AY_EOMEM; goto cleanup; }
		      attr = tmp;
		      memcpy(&(attr[totalbuflen]), buf,
			     buflen*sizeof(char));
		      totalbuflen += buflen;
		      r++;
		    } /* for */

		  if(!(tmp = realloc(attr, (totalbuflen+4)*sizeof(char))))
		    { ay_status = AY_EOMEM; goto cleanup; }
		  attr = tmp;
		  memcpy(&(attr[totalbuflen]), " -1", 4*sizeof(char));
		  totalbuflen += 3;
		}
	      else
		{
		  /* user requested triangles only => tesselate it */

		  /* create new object (for the tesselated face) */
		  if(!(li = calloc(1, sizeof(ay_list_object))))
		    { ay_status = AY_EOMEM; goto cleanup; }
		  if(!(to = calloc(1, sizeof(ay_object))))
		    { ay_status = AY_EOMEM; goto cleanup; }
		  li->object = to;

		  ay_object_defaults(to);

		  to->type = AY_IDPOMESH;

		  ay_status = ay_tess_pomeshf(po, i, q, r, AY_FALSE,
				  (ay_pomesh_object **)(void*)&(to->refine));

		  /* temporarily save the tesselated face */
		  if(nextli)
		    {
		      *nextli = li;
		    }
		  else
		    {
		      lihead = li;
		    }
		  nextli = &(li->next);

		  /* advance index r */
		  for(k = 0; k < po->nverts[q]; k++)
		    {
		      r++;
		    }
		} /* if */
	      q++;
	    } /* for */
	}
      else
	{
	  /* this face has more than one loop (hole(s)) => tesselate it */

	  /* create new object (for the tesselated face) */
	  if(!(li = calloc(1, sizeof(ay_list_object))))
	    { ay_status = AY_EOMEM; goto cleanup; }

	  if(!(to = calloc(1, sizeof(ay_object))))
	    { ay_status = AY_EOMEM; goto cleanup; }

	  li->object = to;

	  ay_object_defaults(to);

	  to->type = AY_IDPOMESH;

	  ay_status = ay_tess_pomeshf(po, i, q, r, AY_FALSE,
				   (ay_pomesh_object **)(void*)&(to->refine));

	  /* temporarily save the tesselated face */
	  if(nextli)
	    {
	      *nextli = li;
	    }
	  else
	    {
	      lihead = li;
	    }
	  nextli = &(li->next);

	  /* advance indices r and q */
	  for(j = 0; j < po->nloops[p]; j++)
	    {
	      for(k = 0; k < po->nverts[q]; k++)
		{
		  r++;
		}
	      q++;
	    } /* for */
	} /* if */
      p++;
    } /* for */

  if(attr)
    {
      scew_element_add_attr_pair(ifs_element, "coordIndex", attr);

      /* XXXX make this configurable? */
      scew_element_add_attr_pair(ifs_element, "ccw", x3dio_falsestring);
      scew_element_add_attr_pair(ifs_element, "solid", x3dio_falsestring);

      /* write control points */
      if(po->has_normals)
	stride = 6;
      else
	stride = 3;

      coord_element = scew_element_add(ifs_element, "Coordinate");
      x3dio_writedoublepoints(coord_element, "point", 3, po->ncontrols,
			      stride, po->controlv);

      /* write normals */
      if(po->has_normals)
	{
	  normal_element = scew_element_add(ifs_element, "Normal");
	  x3dio_writedoublepoints(normal_element, "vector", 3, po->ncontrols,
				  6, &(po->controlv[3]));
	}
      else
	{
	  /* face normals from PV tag */
	  if(have_facenormals)
	    {
	      scew_element_add_attr_pair(ifs_element, "normalPerVertex",
					 x3dio_falsestring);

	      idxsize = sprintf(buf, " %u", po->npolys);
	      if(!(nattr = malloc((idxsize*po->npolys+10)*sizeof(char))))
		{ ay_status = AY_EOMEM; goto cleanup; }
	      tmp = nattr;
	      for(i = 0; i < po->npolys; i++)
		{
		  tmp += sprintf(tmp, " %u", i);
		}

	      scew_element_add_attr_pair(ifs_element, "normalIndex", nattr);

	      normal_element = scew_element_add(ifs_element, "Normal");

	      scew_element_add_attr_pair(normal_element, "vector",
					 normalstring);
	    } /* if */
	} /* if */

      /* write colors */
      if(have_vcolors || have_fcolors)
	{
	  if(have_alpha)
	    color_element = scew_element_add(ifs_element, "ColorRGBA");
	  else
	    color_element = scew_element_add(ifs_element, "Color");

	  scew_element_add_attr_pair(color_element, "color",
				     colorstring);

	  if(have_vcolors)
	    scew_element_add_attr_pair(ifs_element, "colorPerVertex",
				       x3dio_truestring);
	  else
	    scew_element_add_attr_pair(ifs_element, "colorPerVertex",
				       x3dio_falsestring);
	}

      /* write texture coordinates */
      if(have_texcoords)
	{
	  texcoord_element = scew_element_add(ifs_element,
					      "TextureCoordinate");

	  scew_element_add_attr_pair(texcoord_element, "point",
				     texcoordstring);
	}

    } /* if */

  /* write tesselated face(s) */
  if(lihead && lihead->next)
    {
      to = NULL;
      ay_status = ay_pomesht_merge(AY_FALSE, lihead, &to);
      if(to)
	{
	  ay_status = ay_pomesht_optimizecoords(to->refine, AY_FALSE);
	  ay_object_defaults(to);
	  to->type = AY_IDPOMESH;
	  /*ay_trafo_copy(o, to);*/
	  x3dio_writepomeshobj(element, to);
	  ay_object_delete(to);
	}
    }
  else
    {
      if(lihead)
	{
	  x3dio_writepomeshobj(element, lihead->object);
	}
    } /* if */

  /* cleanup */

cleanup:
  while(lihead)
    {
      ay_object_delete(lihead->object);
      li = lihead->next;
      free(lihead);
      lihead = li;
    } /* while */

  if(attr)
    free(attr);

  if(nattr)
    free(nattr);

  if(texcoordstring)
    free(texcoordstring);

  if(colorstring)
    free(colorstring);

  if(normalstring)
    free(normalstring);

 return ay_status;
} /* x3dio_writepomeshobj */


/* x3dio_writepomeshwire:
 *
 */
int
x3dio_writepomeshwire(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_pomesh_object *po;
 unsigned int a, b, i, j, k, l = 0, m = 0, n = 0;
 scew_element *transform_element = NULL;
 scew_element *shape_element = NULL;
 scew_element *line_element = NULL;
 scew_element *coord_element = NULL;
 char buf[256];
 char *attr = NULL, *tmp;
 unsigned int idxsize = 0;
 double *centroids = NULL, cd[3], cw;
 double offset = 0.005, len, *v1, *normal;
 double *fn = NULL, *weighted_normals = NULL, *offset_cv = NULL;

  if(!element || !o)
   return AY_ENULL;

  po = (ay_pomesh_object *)o->refine;

  if(!po->has_normals)
    {
      if(!(centroids = calloc(po->npolys*3, sizeof(double))))
	return AY_EOMEM;

      if(!(weighted_normals = calloc(po->ncontrols*3, sizeof(double))))
	{ay_status = AY_EOMEM; goto cleanup;}

      for(i = 0; i < po->npolys; i++)
	{
	  if(po->nloops[l] > 0)
	    {
	      for(k = 0; k < po->nverts[m]; k++)
		{
		  a = po->verts[n++]*3;
		  centroids[i*3]   += (po->controlv[a]   / po->nverts[m]);
		  centroids[i*3+1] += (po->controlv[a+1] / po->nverts[m]);
		  centroids[i*3+2] += (po->controlv[a+2] / po->nverts[m]);
		}
	      m++;
	      for(j = 1; j < po->nloops[l]; j++)
		{
		  n += po->nverts[m];
		  m++;
		}
	    } /* if */
	  l++;
	} /* for */

      fn = po->face_normals;
      if(!fn)
	{
	  if((ay_status = ay_pomesht_genfacenormals(po, &fn)))
	    goto cleanup;
	}

      l = 0; m = 0; n = 0;
      for(i = 0; i < po->npolys; i++)
	{
	  if(po->nloops[l] > 0)
	    {
	      normal = &(fn[i*3]);
	      /* calc/update weighted normal of outer loops vertices */
	      for(k = 0; k < po->nverts[m]; k++)
		{
		  a = po->verts[n++]*3;

		  cd[0] = po->controlv[a]   - centroids[i*3];
		  cd[1] = po->controlv[a+1] - centroids[i*3+1];
		  cd[2] = po->controlv[a+2] - centroids[i*3+2];

		  len = AY_V3LEN(cd);
		  if(len > AY_EPSILON)
		    cw = 1.0/(len*len);
		  else
		    cw = 0.0;
		  weighted_normals[a]   += normal[0]*cw;
		  weighted_normals[a+1] += normal[1]*cw;
		  weighted_normals[a+2] += normal[2]*cw;
		}
	      m++;
	      /* the vertices of the inner loops just get the
		 face normal */
	      for(j = 1; j < po->nloops[l]; j++)
		{
		  for(k = 0; k < po->nverts[m]; k++)
		    {
		      a = po->verts[n++]*3;
		      memcpy(&(weighted_normals[a]),normal,3*sizeof(double));
		    }
		  m++;
		}
	    } /* if */
	  l++;
	} /* for */

      if(!po->face_normals)
	free(fn);

      if(!(offset_cv = calloc(po->ncontrols*6, sizeof(double))))
	{ay_status = AY_EOMEM; goto cleanup;}

      a = 0;
      b = 0;
      for(i = 0; i < po->ncontrols; i++)
	{
	  v1 = &(weighted_normals[b]);

	  /* scale mean normal to offset length */
	  len = AY_V3LEN(v1);
	  if(fabs(len) > AY_EPSILON)
	    {
	      AY_V3SCAL(v1, 1.0/len*offset);
	    }

	  /* offset control points */
	  offset_cv[a]   = po->controlv[b]   + weighted_normals[b];
	  offset_cv[a+1] = po->controlv[b+1] + weighted_normals[b+1];
	  offset_cv[a+2] = po->controlv[b+2] + weighted_normals[b+2];

	  offset_cv[a+3] = po->controlv[b]   - weighted_normals[b];
	  offset_cv[a+4] = po->controlv[b+1] - weighted_normals[b+1];
	  offset_cv[a+5] = po->controlv[b+2] - weighted_normals[b+2];

	  a += 6;
	  b += 3;
	} /* for */
    }
  else
    {
      /* just use the already present vertex normals */
      weighted_normals = po->controlv;

      if(!(offset_cv = calloc(po->ncontrols*6, sizeof(double))))
	{ay_status = AY_EOMEM; goto cleanup;}

      a = 0;
      b = 3;
      for(i = 0; i < po->ncontrols; i++)
	{
	  v1 = &(weighted_normals[b]);
	  len = AY_V3LEN(v1);
	  if(fabs(len) > AY_EPSILON)
	    {
	      memcpy(cd, v1, 3*sizeof(double));
	      if((fabs(len) > 1.0+AY_EPSILON) || (fabs(len) < 1.0-AY_EPSILON))
		{
		  /* need to normalize */
		  AY_V3SCAL(cd, 1.0/len*offset);
		}
	      else
		{
		  AY_V3SCAL(cd, offset);
		}

	      /* offset control points */
	      offset_cv[a]   = po->controlv[a]   + cd[0];
	      offset_cv[a+1] = po->controlv[a+1] + cd[1];
	      offset_cv[a+2] = po->controlv[a+2] + cd[2];

	      offset_cv[a+3] = po->controlv[a]   - cd[0];
	      offset_cv[a+4] = po->controlv[a+1] - cd[1];
	      offset_cv[a+5] = po->controlv[a+2] - cd[2];
	    } /* if */

	  a += 6;
	  b += 6;
	} /* for */

    } /* if */

  /* write transform */
  x3dio_writetransform(element, o, &transform_element);

  /* write shape */
  shape_element = scew_element_add(transform_element, "Shape");

  x3dio_writewiremat(shape_element);

  line_element = scew_element_add(shape_element, "IndexedLineSet");

  /* calculate size of index */
  l = 0; m = 0; n = 0;
  for(i = 0; i < po->npolys; i++)
    {
      for(j = 0; j < po->nloops[l]; j++)
	{
	  for(k = 0; k < po->nverts[m]; k++)
	    {
	      idxsize += sprintf(buf, " %u", po->verts[n++]);
	    }
	  idxsize += sprintf(buf, " %u", po->verts[n-k]);
	  /* add " -1" */
	  idxsize += 3;
	  m++;
	}
      l++;
    } /* for */

  /* generate index */
  if(!(attr = malloc((idxsize+10)*sizeof(char))))
    { ay_status = AY_EOMEM; goto cleanup; }
  tmp = attr;
  l = 0; m = 0; n = 0;
  for(i = 0; i < po->npolys; i++)
    {
      for(j = 0; j < po->nloops[l]; j++)
	{
	  for(k = 0; k < po->nverts[m]; k++)
	    {
	      tmp += sprintf(tmp, " %u", po->verts[n++]);
	    }
	  tmp += sprintf(tmp, " %u", po->verts[n-k]);
	  tmp += sprintf(tmp, " -1");
	  m++;
	}
      l++;
    } /* for */

  scew_element_add_attr_pair(line_element, "coordIndex", attr);

  /* write coordinates */
  coord_element = scew_element_add(line_element, "Coordinate");
  x3dio_writedoublepoints(coord_element, "point", 3, po->ncontrols,
			  6, offset_cv);

  /* write another shape */
  shape_element = scew_element_add(transform_element, "Shape");

  x3dio_writewiremat(shape_element);

  line_element = scew_element_add(shape_element, "IndexedLineSet");

  scew_element_add_attr_pair(line_element, "coordIndex", attr);

  /* write coordinates */
  coord_element = scew_element_add(line_element, "Coordinate");
  x3dio_writedoublepoints(coord_element, "point", 3, po->ncontrols,
			  6, &(offset_cv[3]));

cleanup:

  if(po->has_normals)
    weighted_normals = NULL;

  if(centroids)
    free(centroids);

  if(weighted_normals)
    free(weighted_normals);

  if(offset_cv)
    free(offset_cv);

 return ay_status;
} /* x3dio_writepomeshwire */


/* x3dio_writeview:
 *
 */
int
x3dio_writeview(scew_element *element, ay_object *o)
{
 struct Togl *togl;
 ay_view_object *view;
 /*ay_camera_object *camera;*/
 char buffer[128];
 double tmp = 0.0, v[3] = {0};
 double xzlen = 0.0, yzlen = 0.0, x = 0.0, y = 0.0;
 double q1[4] = {0}, q2[4] = {0}, quat[4] = {0};
 double xaxis[3] = {1.0, 0.0, 0.0}, yaxis[3] = {0.0, 1.0, 0.0};
 scew_element *vp_element = NULL;
 static int vnum = 1;

  if(!element || !o)
    {
      vnum = 1;
      return AY_ENULL;
    }

  if(o->type == AY_IDVIEW)
    {
      view = (ay_view_object*)o->refine;

      vp_element = scew_element_add(element, "Viewpoint");

      x3dio_writedoublevecattrib(vp_element, "position", 3, view->from);

      v[0] = view->to[0] - view->from[0];
      v[1] = view->to[1] - view->from[1];
      v[2] = view->to[2] - view->from[2];

      if(fabs(v[0]) > AY_EPSILON ||
	 fabs(v[1]) > AY_EPSILON ||
	 fabs(v[2]) > AY_EPSILON)
	{
	  xzlen = sqrt(v[0] * v[0] + v[2] * v[2]);

	  if(fabs(xzlen) < AY_EPSILON)
	    {
	      y = (v[1] < 0.0) ? AY_PI : 0.0;
	    }
	  else
	    {
	      tmp = -v[2]/xzlen;
	      y = acos((fabs(tmp)<=1.0?tmp:(tmp<-1.0?-1.0:1.0)));
	    }

	  yzlen = sqrt(v[1] * v[1] + xzlen * xzlen);

	  if(fabs(yzlen) > AY_EPSILON)
	    {
	      x = acos(xzlen/yzlen);
	    }
	  else
	    {
	      x = 0.0;
	    }

	  if(v[1] < 0.0)
	    x = -x;

	  if(v[0] > 0.0)
	    y = -y;

	  if((fabs(x) > AY_EPSILON) || (fabs(y) > AY_EPSILON))
	    {
	      if(fabs(x) > AY_EPSILON)
		{
		  ay_quat_axistoquat(xaxis, x, q1);
		  if(fabs(y) > AY_EPSILON)
		    {
		      ay_quat_axistoquat(yaxis, y, q2);
		      ay_quat_add(q1, q2, quat);
		    }
		  else
		    {
		      memcpy(quat, q1, 4*sizeof(double));
		    }
		}
	      else
		{
		  ay_quat_axistoquat(yaxis, y, quat);
		} /* if */
	      ay_quat_norm(quat);
	      tmp = sqrt(1.0-quat[3]*quat[3]);
	      if(tmp > AY_EPSILON)
		{
		  quat[0] /= tmp;
		  quat[1] /= tmp;
		  quat[2] /= tmp;
		}
	      else
		{
		  quat[0] = 1.0;
		  quat[1] = 0.0;
		  quat[2] = 0.0;
		}
	      quat[3] = 2 * acos(quat[3]);

	      x3dio_writedoublevecattrib(vp_element, "orientation", 4, quat);
	    } /* if */

	  if(fabs(view->to[0]) > AY_EPSILON ||
	     fabs(view->to[1]) > AY_EPSILON ||
	     fabs(view->to[2]) > AY_EPSILON)
	    {
	      x3dio_writedoublevecattrib(vp_element, "centerOfRotation",
					 3, view->to);
	    }
	} /* if */
      /* XXXX issue error/warning? */
      /*
	else
	{
	  ay_error();
	}
      */

      /* compose describing string */
      switch(view->type)
	{
	case AY_VTFRONT:
	  sprintf(buffer, "View%d-Front", vnum);
	  break;
	case AY_VTSIDE:
	  sprintf(buffer, "View%d-Side", vnum);
	  break;
	case AY_VTTOP:
	  sprintf(buffer, "View%d-Top", vnum);
	  break;
	case AY_VTPERSP:
	  sprintf(buffer, "View%d-Persp", vnum);
	  break;
	case AY_VTTRIM:
	  sprintf(buffer, "View%d-Trim", vnum);
	  break;
	default:
	  sprintf(buffer, "View%d", vnum);
	  break;
	}
      scew_element_add_attr_pair(vp_element, "description",
				 buffer);

      /* field of view */
      tmp = atan(view->zoom)*2.0;
      x3dio_writedoubleattrib(vp_element, "fieldOfView", &tmp);

      /* remember view window dimensions for later
	 inclusion in x3dom file */
      togl = view->togl;
      x3dio_x3domwidth = Togl_Width(togl);
      x3dio_x3domheight =  Togl_Height(togl);
   } /* if */

 /*
 if(o->type == AY_IDCAMERA)
   {
     camera = (ay_camera_object*)o->refine;
   }

  if(o->name && (strlen(o->name)>1))
   {

   }
 */
  vnum++;

 return AY_OK;
} /* x3dio_writeview */


/* x3dio_writelight:
 *
 */
int
x3dio_writelight(scew_element *element, ay_object *o)
{
 ay_light_object *light;
 ay_shader *shader = NULL;
 ay_shader_arg *sarg = NULL;
 scew_element *light_element = NULL;
 double dir[3] = {0}, col[3] = {0};
 double from[3] = {0}, to[3] = {0};
 double coneAngle = 0.0, beamWidth = 0.0;
 int has_from = AY_FALSE, has_to = AY_FALSE, has_angle = AY_FALSE;

  if(!element || !o)
    return AY_ENULL;

  light = (ay_light_object*)o->refine;

  switch(light->type)
    {
    case AY_LITPOINT:
      light_element = scew_element_add(element, "PointLight");
      x3dio_writedoublevecattrib(light_element, "location", 3, light->tfrom);
      break;
    case AY_LITDISTANT:
      light_element = scew_element_add(element, "DirectionalLight");
      dir[0] = light->tto[0] - light->tfrom[0];
      dir[1] = light->tto[1] - light->tfrom[1];
      dir[2] = light->tto[2] - light->tfrom[2];
      x3dio_writedoublevecattrib(light_element, "direction", 3, dir);
      break;
    case AY_LITSPOT:
      light_element = scew_element_add(element, "SpotLight");
      dir[0] = light->tto[0] - light->tfrom[0];
      dir[1] = light->tto[1] - light->tfrom[1];
      dir[2] = light->tto[2] - light->tfrom[2];
      x3dio_writedoublevecattrib(light_element, "direction", 3, dir);

      x3dio_writedoubleattrib(light_element, "cutOffAngle",
			      &light->cone_angle);

      beamWidth = light->cone_angle - light->cone_delta_angle;
      x3dio_writedoubleattrib(light_element, "beamWidth",
			      &beamWidth);
      break;
    case AY_LITCUSTOM:
      if(light->lshader)
	{
	  shader = light->lshader;

	  sarg = shader->arg;
	  while(sarg)
	    {
	      if((!ay_comp_strcase(sarg->name, "from")) &&
		 (sarg->type == AY_SAPOINT))
		{
		  has_from = AY_TRUE;
		  from[0] = sarg->val.point[0];
		  from[1] = sarg->val.point[1];
		  from[2] = sarg->val.point[2];
		}
	      if((!ay_comp_strcase(sarg->name, "to")) &&
		 (sarg->type == AY_SAPOINT))
		{
		  has_to = AY_TRUE;
		  to[0] = sarg->val.point[0];
		  to[1] = sarg->val.point[1];
		  to[2] = sarg->val.point[2];
		}
	      if((!ay_comp_strcase(sarg->name, "coneangle")) &&
		 (sarg->type == AY_SASCALAR))
		{
		  has_to = AY_TRUE;
		  coneAngle = sarg->val.scalar;
		}
	      sarg = sarg->next;
	    } /* while */

	  if(has_from && has_to && has_angle)
	    {
	      light_element = scew_element_add(element, "SpotLight");
	      dir[0] = to[0] - from[0];
	      dir[1] = to[1] - from[1];
	      dir[2] = to[2] - from[2];
	      x3dio_writedoublevecattrib(light_element, "direction", 3, dir);

	      x3dio_writedoubleattrib(light_element, "cutOffAngle",
				      &coneAngle);
	      /*
	      beamWidth = light->cone_angle - light->cone_delta_angle;
	      x3dio_writedoubleattrib(light_element, "beamWidth",
				      &beamWidth);
	      */
	    }
	  else
	    {
	      if(has_from && has_to)
		{
		  light_element = scew_element_add(element,
						   "DirectionalLight");
		  dir[0] = to[0] - from[0];
		  dir[1] = to[1] - from[1];
		  dir[2] = to[2] - from[2];
		  x3dio_writedoublevecattrib(light_element, "direction",
					     3, dir);
		}
	      else
		{
		  if(has_from)
		    {
		      light_element = scew_element_add(element, "PointLight");
		      x3dio_writedoublevecattrib(light_element, "location",
						 3, from);
		    } /* if */
		} /* if */
	    } /* if */
	} /* if */
      break;
    default:
      break;
    } /* switch */

  /* add some standard parameters */
  if(light_element)
    {
      /* intensity */
      x3dio_writedoubleattrib(light_element, "intensity", &light->intensity);

      /* color */
      col[0] = light->colr/255.0;
      col[1] = light->colg/255.0;
      col[2] = light->colb/255.0;
      x3dio_writedoublevecattrib(light_element, "color", 3, col);

      /* local/global state */
      if(light->local)
	{
	  scew_element_add_attr_pair(light_element, "global",
				     x3dio_falsestring);
	}

      /* is_on state */
      if(!light->on)
	{
	  scew_element_add_attr_pair(light_element, "on", x3dio_falsestring);
	}
    } /* if */

 return AY_OK;
} /* x3dio_writelight */


/* x3dio_writematerial:
 *
 */
int
x3dio_writematerial(scew_element *shape_element, ay_object *o)
{
 scew_element *appearance_element = NULL;
 scew_element *material_element = NULL;
 char buf[128];

  if(!shape_element || !o)
    return AY_ENULL;

  /* write appearance */
  appearance_element = scew_element_add(shape_element, "Appearance");

  /* write material */
  material_element = scew_element_add(appearance_element, "Material");

  if(o->mat)
    {
      sprintf(buf, "%g %g %g",
	      o->mat->colr/255.0, o->mat->colg/255.0, o->mat->colb/255.0);
      scew_element_add_attr_pair(material_element, "diffuseColor", buf);
    }
  else
    {
      scew_element_add_attr_pair(material_element, "diffuseColor", "1 1 1");
    }

 return AY_OK;
} /* x3dio_writematerial */


/* x3dio_writerevolveobj:
 *
 */
int
x3dio_writerevolveobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_revolve_object *rev;
 scew_element *transform_element = NULL;
 scew_element *shape_element = NULL;
 scew_element *swing_element = NULL;
 scew_element *curve_element = NULL;
 ay_object *c = NULL, *e;
 ay_nurbcurve_object *cs, *circle = NULL;
 double radius = 1.0;

  if(!element || !o)
    return AY_ENULL;

  if(!o->down || !o->down->next)
    return AY_ERROR;

  rev = (ay_revolve_object*)o->refine;

  if(!x3dio_writeparam || (rev->sections > 0))
    return x3dio_writenpconvertibleobj(element, o);

  /* write transform */
  x3dio_writetransform(element, o, &transform_element);

  /* write shape */
  shape_element = scew_element_add(transform_element, "Shape");

  /* write name to shape element */
  ay_status = x3dio_writename(shape_element, o, AY_FALSE);

  if(ay_status)
    return ay_status;

  if(x3dio_writemat)
    x3dio_writematerial(shape_element, o);

  /* write swing element */
  swing_element = scew_element_add(shape_element, "NurbsSwungSurface");

  /* get cross section curve */
  if(o->down->type != AY_IDNCURVE)
    {
      ay_provide_object(o->down, AY_IDNCURVE, &c);
    }
  else
    {
      ay_object_copy(o->down, &c);
    }

  if(!c)
    return AY_ERROR;

  ay_nct_applytrafo(c);

  cs = (ay_nurbcurve_object*)c->refine;

  /* write cross section curve */
  x3dio_writencurve(swing_element, 2, cs);

  /* get the curve element we just wrote */
  curve_element = scew_element_next(swing_element, NULL);

  /* and add a containerField attribute */
  scew_element_add_attr_pair(curve_element, "containerField", "profileCurve");

  /* create and write a trajectory curve (a circle/circular arc) */
  if(rev->thetamax == 360.0)
    {
      ay_status = ay_nct_crtncircle(radius, &circle);
    }
  else
    {
      ay_status = ay_nct_crtncirclearc(radius, rev->thetamax, &circle);
    }

  if(!circle)
    {
      (void)ay_object_deletemulti(c, AY_FALSE);
      return AY_ERROR;
    }

  x3dio_writencurve(swing_element, 2, circle);

  /* get the curve element we just wrote */
  curve_element = scew_element_next(swing_element, NULL);
  curve_element = scew_element_next(swing_element, curve_element);

  /* and add a containerField attribute */
  scew_element_add_attr_pair(curve_element, "containerField",
			     "trajectoryCurve");

  /* write the caps and bevels */
  e = rev->caps_and_bevels;
  while(e)
    {
      x3dio_writenpatchobj(transform_element, e);
      e = e->next;
    }

  /* cleanup */
  ay_nct_destroy(circle);

  (void)ay_object_deletemulti(c, AY_FALSE);

 return AY_OK;
} /* x3dio_writerevolveobj */


/* x3dio_writesweepobj:
 *
 */
int
x3dio_writesweepobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_sweep_object *swp;
 scew_element *transform_element = NULL;
 scew_element *shape_element = NULL;
 scew_element *sweep_element = NULL;
 scew_element *curve_element = NULL;
 ay_object *c = NULL, *d = NULL, *e = NULL;
 ay_nurbcurve_object *cs;

  if(!element || !o)
    return AY_ENULL;

  if(!o->down || !o->down->next)
    return AY_ERROR;

  if(!x3dio_writeparam)
    return x3dio_writenpconvertibleobj(element, o);

  swp = (ay_sweep_object*)o->refine;

  /* write transform */
  x3dio_writetransform(element, o, &transform_element);

  /* write shape */
  shape_element = scew_element_add(transform_element, "Shape");

  /* write name to shape element */
  ay_status = x3dio_writename(shape_element, o, AY_FALSE);

  if(ay_status)
    return ay_status;

  if(x3dio_writemat)
    x3dio_writematerial(shape_element, o);

  /* write sweep element */
  sweep_element = scew_element_add(shape_element, "NurbsSweptSurface");

  /* get cross section curve */
  if(o->down->type != AY_IDNCURVE)
    {
      ay_provide_object(o->down, AY_IDNCURVE, &c);
    }
  else
    {
      ay_object_copy(o->down, &c);
    }

  if(!c)
    return AY_ERROR;

  ay_nct_applytrafo(c);

  cs = (ay_nurbcurve_object*)c->refine;

  /* TODO rotate cross section to XY for proper 2D export */

  /* write cross section curve */
  x3dio_writencurve(sweep_element, 2, cs);

  /* get the curve element we just wrote */
  curve_element = scew_element_next(sweep_element, NULL);

  /* and add a containerField attribute */
  scew_element_add_attr_pair(curve_element, "containerField",
			     "crossSectionCurve");

  /* write trajectory curve */
  if(o->down->next->type != AY_IDNCURVE)
    {
      ay_provide_object(o->down->next, AY_IDNCURVE, &d);
    }
  else
    {
      ay_object_copy(o->down->next, &d);
    }

  if(!d)
    {
      (void)ay_object_deletemulti(c, AY_FALSE);
      return AY_ERROR;
    }

  ay_nct_applytrafo(d);

  cs = (ay_nurbcurve_object*)d->refine;

  x3dio_writencurve(sweep_element, 3, cs);

  /* get the curve element we just wrote */
  curve_element = scew_element_next(sweep_element, NULL);
  curve_element = scew_element_next(sweep_element, curve_element);

  /* and add a containerField attribute */
  scew_element_add_attr_pair(curve_element, "containerField",
			     "trajectoryCurve");

  /* write the caps and bevels */
  e = swp->caps_and_bevels;
  while(e)
    {
      x3dio_writenpatchobj(transform_element, e);
      e = e->next;
    }

  /* cleanup */
  (void)ay_object_deletemulti(c, AY_FALSE);
  (void)ay_object_deletemulti(d, AY_FALSE);

 return AY_OK;
} /* x3dio_writesweepobj */


/* x3dio_writeswingobj:
 *
 */
int
x3dio_writeswingobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_swing_object *swing;
 scew_element *transform_element = NULL;
 scew_element *shape_element = NULL;
 scew_element *swing_element = NULL;
 scew_element *curve_element = NULL;
 ay_object *c = NULL, *d = NULL, *e = NULL;
 ay_nurbcurve_object *cs;

  if(!element || !o)
    return AY_ENULL;

  if(!o->down || !o->down->next)
    return AY_ERROR;

  if(!x3dio_writeparam)
    return x3dio_writenpconvertibleobj(element, o);

  swing = (ay_swing_object*)o->refine;

  /* write transform */
  x3dio_writetransform(element, o, &transform_element);

  /* write shape */
  shape_element = scew_element_add(transform_element, "Shape");

  /* write name to shape element */
  ay_status = x3dio_writename(shape_element, o, AY_FALSE);

  if(ay_status)
    return ay_status;

  if(x3dio_writemat)
    x3dio_writematerial(shape_element, o);

  /* write swing element */
  swing_element = scew_element_add(shape_element, "NurbsSwungSurface");

  /* get cross section curve */
  if(o->down->type != AY_IDNCURVE)
    {
      ay_provide_object(o->down, AY_IDNCURVE, &c);
    }
  else
    {
      ay_object_copy(o->down, &c);
    }

  if(!c)
    return AY_ERROR;

  ay_nct_applytrafo(c);

  cs = (ay_nurbcurve_object*)c->refine;

  /* TODO rotate cross section to XY for proper 2D export */

  /* write cross section curve */
  x3dio_writencurve(swing_element, 2, cs);

  /* get the curve element we just wrote */
  curve_element = scew_element_next(swing_element, NULL);

  /* and add a containerField attribute */
  scew_element_add_attr_pair(curve_element, "containerField", "profileCurve");

  /* get trajectory curve */
  if(o->down->next->type != AY_IDNCURVE)
    {
      ay_provide_object(o->down->next, AY_IDNCURVE, &d);
    }
  else
    {
      ay_object_copy(o->down->next, &d);
    }

  if(!d)
    {
      (void)ay_object_deletemulti(c, AY_FALSE);
      return AY_ERROR;
    }

  ay_nct_applytrafo(d);

  cs = (ay_nurbcurve_object*)d->refine;

  /* write trajectory curve */
  x3dio_writencurve(swing_element, 2, cs);

  /* get the curve element we just wrote */
  curve_element = scew_element_next(swing_element, NULL);
  curve_element = scew_element_next(swing_element, curve_element);

  /* and add a containerField attribute */
  scew_element_add_attr_pair(curve_element, "containerField",
			     "trajectoryCurve");

  /* write the caps and bevels */
  e = swing->caps_and_bevels;
  while(e)
    {
      x3dio_writenpatchobj(transform_element, e);
      e = e->next;
    }

  /* cleanup */
  (void)ay_object_deletemulti(c, AY_FALSE);
  (void)ay_object_deletemulti(d, AY_FALSE);

 return AY_OK;
} /* x3dio_writeswingobj */


/* x3dio_writeextrudeobj:
 *
 */
int
x3dio_writeextrudeobj(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_extrude_object *ext;
 scew_element *transform_element = NULL;
 scew_element *shape_element = NULL;
 scew_element *sweep_element = NULL;
 scew_element *curve_element = NULL;
 ay_object *c = NULL, *d = NULL, *e = NULL;
 ay_nurbcurve_object *cs, *tr;
 double *dtmp = NULL, controlv[8] = {0.0,0.0,0.0,1.0, 0.0,0.0,0.0,1.0};

  if(!element || !o)
    return AY_ENULL;

  if(!o->down || !o->down->next)
    return AY_ERROR;

  if(!x3dio_writeparam)
    return x3dio_writenpconvertibleobj(element, o);

  ext = (ay_extrude_object*)o->refine;

  /* create trajectory curve */
  tr = NULL;
  controlv[6] = ext->height;

  if(!(dtmp = calloc(8, sizeof(double))))
    return AY_EOMEM;
  memcpy(dtmp, controlv, 8*sizeof(double));

  ay_nct_create(2, 2, AY_KTNURB, dtmp, NULL, &tr);

  if(!tr)
    {
      if(dtmp)
	free(dtmp);
      return AY_ERROR;
    }

  /* write transform */
  x3dio_writetransform(element, o, &transform_element);

  /* write shape */
  shape_element = scew_element_add(transform_element, "Shape");

  /* write name to shape element */
  ay_status = x3dio_writename(shape_element, o, AY_FALSE);

  if(ay_status)
    return ay_status;

  if(x3dio_writemat)
    x3dio_writematerial(shape_element, o);

  d = o->down;
  while(d->next)
    {
      /* for each outline/hole write a sweep element */
      sweep_element = scew_element_add(shape_element, "NurbsSweptSurface");

      /* get cross section curve */
      c = NULL;

      if(d->type != AY_IDNCURVE)
	{
	  ay_provide_object(d, AY_IDNCURVE, &c);
	}
      else
	{
	  ay_object_copy(d, &c);
	}

      if(!c)
	{
	  if(dtmp)
	    free(dtmp);
	  ay_nct_destroy(tr);
	  return AY_ERROR;
	}

      ay_nct_applytrafo(c);

      cs = (ay_nurbcurve_object*)c->refine;

      /* write cross section curve */
      x3dio_writencurve(sweep_element, 2, cs);

      /* get the curve element we just wrote */
      curve_element = scew_element_next(sweep_element, NULL);

      /* and add a containerField attribute */
      scew_element_add_attr_pair(curve_element, "containerField",
				 "profileCurve");

      /* write trajectory curve */
      x3dio_writencurve(sweep_element, 3, tr);

      /* get the curve element we just wrote */
      curve_element = scew_element_next(sweep_element, NULL);
      curve_element = scew_element_next(sweep_element, curve_element);

      /* and add a containerField attribute */
      scew_element_add_attr_pair(curve_element, "containerField",
				 "trajectoryCurve");

      /* cleanup */
      (void)ay_object_deletemulti(c, AY_FALSE);

      d = d->next;
    } /* while */

  /* write the caps and bevels */
  e = ext->caps_and_bevels;
  while(e)
    {
      x3dio_writenpatchobj(transform_element, e);
      e = e->next;
    }

  /* cleanup */
  ay_nct_destroy(tr);

 return AY_OK;
} /* x3dio_writeextrudeobj */


#if 0
/* x3dio_writencurve:
 *
 */
int
x3dio_writencurve(scew_element *element, ay_object *o)
{
 ay_nurbcurve_object *nc;

 if(!element || !o)
   return AY_ENULL;



  if(o->name && (strlen(o->name)>1))
   {

   }

 return AY_OK;
} /* x3dio_writencurve */
#endif


/* x3dio_writeobject:
 *
 */
int
x3dio_writeobject(scew_element *element, ay_object *o, int count)
{
 int ay_status = AY_OK;
 char fname[] = "x3dio_writeobject";
 Tcl_HashTable *ht = &x3dio_write_ht;
 Tcl_HashEntry *entry = NULL;
 char err[255];
 x3dio_writecb *cb = NULL;
 ay_object *t, *c = NULL;
 int curprog = 0;
 char aname[] = "x3dio_options", vname1[] = "Progress";
 char vname2[] = "Cancel", *val = NULL;
 char pbuffer[64];
 int i, numconvs = 3, conversions[3] = {AY_IDNPATCH, AY_IDNCURVE, AY_IDPOMESH};

  if(!element || !o)
    return AY_ENULL;

  if((entry = Tcl_FindHashEntry(ht, (char *)(o->type))))
    {
      cb = (x3dio_writecb*)Tcl_GetHashValue(entry);

      if(cb)
	{
	  ay_status = cb(element, o);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Error exporting object.");
	      ay_status = AY_OK;
	    }

	  if(count)
	    {
	      x3dio_curobjcnt++;

	      /* calculate new progress value in percent */
	      curprog = (int)(x3dio_curobjcnt*100.0/x3dio_allobjcnt);

	      sprintf(pbuffer, "%d", curprog);
	      Tcl_SetVar2(ay_interp, aname, vname1, pbuffer,
			  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

	      /* process all events (update the GUI) for every object */
	      /*
		if(!fmod(x3dio_curobjcnt, 5.0))
		{
	      */
	      while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

	      /* also, check for cancel button */
	      val = Tcl_GetVar2(ay_interp, aname, vname2,
				TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      if(val && val[0] == '1')
		{
		  ay_error(AY_EOUTPUT, fname,
		   "Export cancelled! Not all objects may have been written!");
		  return AY_EDONOTLINK;
		}
	      /*
		}
	      */
	    } /* if count */
	} /* if cb */
    }
  else
    {
      /* can not export directly => try to convert object */
      for(i = 0; i < numconvs; i++)
	{
	  c = NULL;
	  ay_status = ay_provide_object(o, conversions[i], &c);
	  t = c;
	  while(t)
	    {
	      ay_status = x3dio_writeobject(element, t, AY_FALSE);
	      t = t->next;
	    }

	  if(c)
	    {
	      (void)ay_object_deletemulti(c, AY_FALSE);
	      i = -1;
	      break;
	    }
	} /* for */

      if(i != -1)
	{
	  sprintf(err, "Cannot export objects of type: %s.",
		  ay_object_gettypename(o->type));
	  ay_error(AY_EWARN, fname, err);
	}
    } /* if */

 return AY_OK;
} /* x3dio_writeobject */


/* x3dio_findelement:
 * _Recursively_ find the element <name> in the XML tree below <parent>.
 */
scew_element*
x3dio_findelement(scew_element const* parent, XML_Char const* name)
{
 scew_element *next_element = NULL, *result;

  if(name == NULL)
    {
      return NULL;
    }

  next_element = scew_element_next(parent, 0);
  while (next_element)
    {
      if(!strcmp(scew_element_name(next_element), name))
	return next_element;

      if((result = x3dio_findelement(next_element, name)))
	return result;

      next_element = scew_element_next(parent, next_element);
    }

 return NULL;
} /* x3dio_findelement */


/* x3dio_writescene:
 *
 */
int
x3dio_writescene(char *filename, int selected, int toplevellayers)
{
 int ay_status = AY_OK;
 char fname[] = "x3dio_writescene";
 char buf[256];
 ay_object *o = ay_root->next, *d = NULL;
 ay_list_object *sel = NULL;
 scew_error errcode;
 scew_parser *parser = NULL;
 scew_tree *tree = NULL;
 scew_element *root = NULL;
 scew_element *scene_element = NULL;
 scew_element *cadlayer_element = NULL;
 scew_element *x3d_element = NULL;
 scew_attribute *attribute = NULL;
 enum XML_Error expat_code;

  if(!filename)
    return AY_ENULL;

  /* create and initialize hashtable for DEFs */
  if(!(x3dio_defs_ht = calloc(1, sizeof(Tcl_HashTable))))
    return AY_EOMEM;
  Tcl_InitHashTable(x3dio_defs_ht, TCL_STRING_KEYS);

  /* clear potentially present MN tags from scene */
  x3dio_clearmntags(ay_root);

  /* reset object number counter */
  (void)x3dio_writename(NULL, NULL, 0);

  if(!x3dio_writex3dom)
    {
      /* create in-memory XML tree */
      tree = scew_tree_create();

      scew_tree_set_xml_preamble(tree, "DOCTYPE X3D PUBLIC \"ISO//Web3D//DTD X3D 3.0//EN\"   \"http://www.web3d.org/specifications/x3d-3.0.dtd\"");

      root = scew_tree_add_root(tree, "X3D");

      attribute = scew_attribute_create("profile", "Full");
      scew_element_add_attr(root, attribute);
      attribute = scew_attribute_create("version", "3.0");
      scew_element_add_attr(root, attribute);

      scene_element = scew_element_add(root, "Scene");
    }
  else
    {
      /* write x3dom scene i.e. augment an existig template file
	 with the current scene */

      /* initialize XML parser */
      parser = scew_parser_create();

      scew_parser_ignore_whitespaces(parser, 1);

      /* load an XML (X3D) file */
      if(!scew_parser_load_file(parser, x3dio_x3domtemplate))
	{
	  errcode = scew_error_code();
	  sprintf(buf, "Unable to load file (error #%d: %s)\n",
		  errcode,
		  scew_error_string(errcode));
	  ay_error(AY_ERROR, fname, buf);
	  if(errcode == scew_error_expat)
	    {
	      expat_code = scew_error_expat_code(parser);
	      sprintf(buf, "Expat error #%d (line %d, column %d): %s\n",
		      expat_code,
		      scew_error_expat_line(parser),
		      scew_error_expat_column(parser),
		      scew_error_expat_string(expat_code));
	      ay_error(AY_ERROR, fname, buf);
	    }
	  ay_status = AY_ERROR;
	  goto cleanup;
	} /* if */

      tree = scew_parser_tree(parser);
      root = scew_tree_root(tree);
      scene_element = x3dio_findelement(root, "Scene");
      if(!scene_element)
	{
	  ay_error(AY_ERROR, fname, "Could not find the <Scene> element.");
	  ay_status = AY_ERROR;
	  goto cleanup;
	}
    } /* if */

  ay_trafo_identitymatrix(tm);

  if(x3dio_scalefactor != 1.0)
    {
      sprintf(buf, "%g %g %g", x3dio_scalefactor, x3dio_scalefactor,
	      x3dio_scalefactor);
      scene_element = scew_element_add(scene_element, "Transform");
      scew_element_add_attr_pair(scene_element, "scale", buf);
    }

  /* export the views */
  if(x3dio_writeviews)
    {
      /* reset view number */
      (void)x3dio_writeview(NULL, NULL);
      /* loop through the view level */
      o = ay_root->down;
      while(o)
	{
	  if(o->type == AY_IDVIEW)
	    {
	      ay_status = x3dio_writeview(scene_element, o);
	    }
	  o = o->next;
	}
      o = ay_root->next;
    } /* if */

  if(selected)
    {
      o = ay_currentlevel->object;
    }

  if(!o)
    return AY_ENULL;

  /* count objects to be exported */
  if(!selected)
    {
      x3dio_allobjcnt = x3dio_count(ay_root->next);
    }
  else
    {
      x3dio_allobjcnt = 0;
      sel = ay_selection;
      while(sel)
	{
	  x3dio_allobjcnt++;
	  if(sel->object->down && sel->object->down->next)
	    {
	      x3dio_allobjcnt += x3dio_count(sel->object->down);
	    }
	  sel = sel->next;
	}
    } /* if */

  x3dio_curobjcnt = 0;

  /* omit EndLevel-object in top level! */
  while(o->next)
    {
      if((o->type == AY_IDLEVEL) && (toplevellayers))
	{
	  cadlayer_element = scew_element_add(scene_element, "CADLayer");

	  /* write name to cad layer element */
	  ay_status = x3dio_writename(cadlayer_element, o, AY_FALSE);

	  d = o->down;
	  while(d->next)
	    {
	      if(selected)
		{
		  if(d->selected)
		    {
		      ay_status = x3dio_writeobject(cadlayer_element, d,
						    AY_TRUE);

		      if(ay_status)
			{
			  break;
			}
		    }
		}
	      else
		{
		  ay_status = x3dio_writeobject(cadlayer_element, d, AY_TRUE);

		  if(ay_status)
		    {
		      break;
		    }
		} /* if */

	      d = d->next;
	    } /* while */
	}
      else
	{
	  if(selected)
	    {
	      if(o->selected)
		{
		  ay_status = x3dio_writeobject(scene_element, o, AY_TRUE);
		}
	    }
	  else
	    {
	      ay_status = x3dio_writeobject(scene_element, o, AY_TRUE);
	    }
	} /* if */

      if(ay_status)
	{
	  /* user cancelled export? */
	  if(ay_status == AY_EDONOTLINK)
	    ay_status = AY_OK;
	  break;
	}

      o = o->next;
    } /* while */

  /* write width/height from last view to X3D element (for x3dom) */
  if(x3dio_writex3dom)
    {
      root = scew_tree_root(tree);
      x3d_element = x3dio_findelement(root, "X3D");
      if(x3d_element)
	{
	  sprintf(buf, "%d", x3dio_x3domwidth);
	  scew_element_add_attr_pair(x3d_element, "width", buf);
	  sprintf(buf, "%d", x3dio_x3domheight);
	  scew_element_add_attr_pair(x3d_element, "height", buf);
	}
      else
	{
	  ay_error(AY_ERROR, fname, "Could not find the <X3D> element.");
	}
    }

  /* write out the in-memory XML tree */
  if(!scew_writer_tree_file(tree, filename))
    {
      ay_error(AY_EOPENFILE, fname, filename);
      ay_status = AY_EOPENFILE;
    }

cleanup:

  /* free the in-memory XML tree */
  scew_tree_free(tree);

  Tcl_DeleteHashTable(x3dio_defs_ht);

  /* clear potentially present MN tags from scene */
  x3dio_clearmntags(ay_root);

  if(x3dio_writex3dom)
    {
      scew_parser_free(parser);
    }

 return ay_status;
} /* x3dio_writescene */


/* x3dio_writetcmd:
 *
 */
int
x3dio_writetcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 int ay_status = AY_OK;
 int selected = AY_FALSE, toplevellayers = AY_FALSE, i = 2;

  /* check args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "filename");
      return TCL_OK;
    }

  /* set default options */
  x3dio_tesspomesh = AY_FALSE;
  x3dio_writecurves = AY_TRUE;
  x3dio_writeviews = AY_TRUE;
  x3dio_writeparam = AY_FALSE;
  x3dio_writemat = AY_FALSE;
  x3dio_writex3dom = AY_FALSE;
  x3dio_resolveinstances = AY_FALSE;
  x3dio_scalefactor = 1.0;
  x3dio_x3domwidth = 0;
  x3dio_x3domheight = 0;

  while(i+1 < argc)
    {
      if(!strcmp(argv[i], "-c"))
	{
	  sscanf(argv[i+1], "%d", &x3dio_writecurves);
	}
      else
      if(!strcmp(argv[i], "-s"))
	{
	  sscanf(argv[i+1], "%d", &selected);
	}
      else
      if(!strcmp(argv[i], "-l"))
	{
	  sscanf(argv[i+1], "%d", &toplevellayers);
	}
      else
      if(!strcmp(argv[i], "-t"))
	{
	  sscanf(argv[i+1], "%d", &x3dio_tesspomesh);
	}
      else
      if(!strcmp(argv[i], "-f"))
	{
	  sscanf(argv[i+1], "%lg", &x3dio_scalefactor);
	}
      else
      if(!strcmp(argv[i], "-v"))
	{
	  sscanf(argv[i+1], "%d", &x3dio_writeviews);
	}
      else
      if(!strcmp(argv[i], "-p"))
	{
	  sscanf(argv[i+1], "%d", &x3dio_writeparam);
	}
      else
      if(!strcmp(argv[i], "-m"))
	{
	  sscanf(argv[i+1], "%d", &x3dio_writemat);
	}
      else
      if(!strcmp(argv[i], "-r"))
	{
	  sscanf(argv[i+1], "%d", &x3dio_resolveinstances);
	}
      else
      if(!strcmp(argv[i], "-x"))
	{
	  sscanf(argv[i+1], "%d", &x3dio_writex3dom);
	}
      i += 2;
    } /* while */

  ay_status = x3dio_writescene(argv[1], selected, toplevellayers);

 return TCL_OK;
} /* x3dio_writetcmd */


/** X_Init:
 *  Initialize the x3dio plugin.\ \n
 *  Note: this function _must_ be named and capitalized exactly this way
 *  regardless of the filename of the shared object (see: man n load)!
 */
#ifdef WIN32
__declspec( dllexport ) int
X_Init(Tcl_Interp *interp)
#else
int
X_Init(Tcl_Interp *interp)
#endif /* WIN32 */
{
 int ay_status = AY_OK;
 char fname[] = "x3dio_init";

  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }

  /* first, check versions */
  if(ay_checkversion(fname, x3dio_version_ma, x3dio_version_mi))
    {
      return TCL_ERROR;
    }

  /* register MN tag type */
  ay_status = ay_tags_register(interp, x3dio_mn_tagname, &x3dio_mn_tagtype);

  if(ay_status)
    return TCL_ERROR;

  /* init hash table for write callbacks */
  Tcl_InitHashTable(&x3dio_write_ht, TCL_ONE_WORD_KEYS);

  /* fill hash table */
  ay_status += x3dio_registerwritecb((char *)(AY_IDLEVEL),
				     x3dio_writelevelobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDCLONE),
				     x3dio_writecloneobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDMIRROR),
				     x3dio_writecloneobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDINSTANCE),
				     x3dio_writeinstanceobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDSCRIPT),
				     x3dio_writescriptobj);

  ay_status += x3dio_registerwritecb((char *)(AY_IDLIGHT),
				     x3dio_writelight);

  ay_status += x3dio_registerwritecb((char *)(AY_IDPOMESH),
				     x3dio_writepomeshobj);

  ay_status += x3dio_registerwritecb((char *)(AY_IDBOX),
				     x3dio_writeboxobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDSPHERE),
				     x3dio_writesphereobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDCYLINDER),
				     x3dio_writecylinderobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDCONE),
				     x3dio_writeconeobj);

  ay_status += x3dio_registerwritecb((char *)(AY_IDNCURVE),
				     x3dio_writencurveobj);

  ay_status += x3dio_registerwritecb((char *)(AY_IDICURVE),
				     x3dio_writencconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDCONCATNC),
				     x3dio_writencconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDEXTRNC),
				     x3dio_writencconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDNCIRCLE),
				     x3dio_writencconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDOFFNC),
				     x3dio_writencconvertibleobj);

  ay_status += x3dio_registerwritecb((char *)(AY_IDNPATCH),
				     x3dio_writenpatchobj);

  ay_status += x3dio_registerwritecb((char *)(AY_IDDISK),
				     x3dio_writenpconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDHYPERBOLOID),
				     x3dio_writenpconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDPARABOLOID),
				     x3dio_writenpconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDTORUS),
				     x3dio_writenpconvertibleobj);

  ay_status += x3dio_registerwritecb((char *)(AY_IDEXTRUDE),
				     x3dio_writeextrudeobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDREVOLVE),
				     x3dio_writerevolveobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDSWEEP),
				     x3dio_writesweepobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDSWING),
				     x3dio_writeswingobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDSKIN),
				     x3dio_writenpconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDCAP),
				     x3dio_writenpconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDPAMESH),
				     x3dio_writenpconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDBPATCH),
				     x3dio_writenpconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDGORDON),
				     x3dio_writenpconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDBIRAIL1),
				     x3dio_writenpconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDBIRAIL2),
				     x3dio_writenpconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDTEXT),
				     x3dio_writenpconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDBEVEL),
				     x3dio_writenpconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDEXTRNP),
				     x3dio_writenpconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDOFFNP),
				     x3dio_writenpconvertibleobj);
  ay_status += x3dio_registerwritecb((char *)(AY_IDIPATCH),
				     x3dio_writenpconvertibleobj);

  if(ay_status)
    return TCL_ERROR;

  /* source x3dio.tcl, it contains vital Tcl-code */
  if((Tcl_EvalFile(interp, "x3dio.tcl")) != TCL_OK)
    {
      ay_error(AY_ERROR, fname, "Error while sourcing \"x3dio.tcl\"!");
      return TCL_ERROR;
    }

  /* create new Tcl commands to interface with the plugin */
  Tcl_CreateCommand(interp, "x3dioRead", x3dio_readtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "x3dioWrite", x3dio_writetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  ay_error(AY_EOUTPUT, fname, "Plugin 'x3dio' successfully loaded.");

 return TCL_OK;
} /* X_Init */
