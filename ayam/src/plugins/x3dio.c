/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2007 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

/* x3dio.cpp - Ayam X3D Import/Export Plugin based on Expat/SCEW */

/* Ayam includes: */
#include "ayam.h"

/* SCEW includes: */
#include "scew.h"

#include <stdio.h>

/* local types: */

typedef struct x3dio_trafostate_s {
  struct x3dio_trafostate_s *next;

  double m[16];

} x3dio_trafostate;

typedef int (x3dio_writecb) (scew_element *element, ay_object *o);


/* global variables: */

char x3dio_version_ma[] = AY_VERSIONSTR;
char x3dio_version_mi[] = AY_VERSIONSTRMI;

static Tcl_HashTable x3dio_write_ht;

static Tcl_HashTable *x3dio_defs_ht = NULL;


/* current transformation */
x3dio_trafostate *x3dio_ctrafos = NULL;

/* import/export options: */
int x3dio_importcurves = AY_TRUE;
int x3dio_exportcurves = AY_TRUE;
int x3dio_expselected = AY_FALSE;
int x3dio_expobeynoexport = AY_TRUE;
int x3dio_expignorehidden = AY_TRUE;
int x3dio_mergeinlinedefs = AY_TRUE;


int x3dio_tesspomesh = AY_FALSE;
int x3dio_writecurves = AY_TRUE;

unsigned int x3dio_allobjcnt = 0;
unsigned int x3dio_curobjcnt = 0;


static double tm[16] = {0}; /* current transformation matrix */


/* 0: silence, 1: errors, 2: warnings, 3: all */
int x3dio_errorlevel = 1;

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

char x3dio_stagnamedef[] = "mys";
char *x3dio_stagname = x3dio_stagnamedef;
char x3dio_ttagnamedef[] = "myt";
char *x3dio_ttagname = x3dio_ttagnamedef;

ay_object *x3dio_lrobject = NULL;

/* prototypes of functions local to this module: */

/* low-level import support functions */
void x3dio_pushtrafo(void);

void x3dio_poptrafo(void);

void x3dio_cleartrafo(void);

void x3dio_trafotoobject(ay_object *o, double *transform);

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

int x3dio_readindex(scew_element *element, char *attrname,
		    unsigned int *len, int **res);

int x3dio_readcoords(scew_element *element, unsigned int *len, double **res);

int x3dio_readnormals(scew_element *element, unsigned int *len, double **res);

int x3dio_readcolors(scew_element *element, unsigned int *len, double **res);

int x3dio_readname(scew_element *element, ay_object *obj);

int x3dio_linkobject(scew_element *element, unsigned int type, void *sobj);

/* 3D */
int x3dio_readbox(scew_element *element);

int x3dio_readsphere(scew_element *element);

int x3dio_readcylinder(scew_element *element);

int x3dio_readcone(scew_element *element);

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

int x3dio_readextrusion(scew_element *element);


/* 2D */
int x3dio_readdisk2d(scew_element *element);

int x3dio_readcircle2d(scew_element *element);

int x3dio_readarc2d(scew_element *element);

int x3dio_readarcclose2d(scew_element *element);

int x3dio_readpolyline2d(scew_element *element, int contour);

/* NURBS */
int x3dio_readnurbscurve(scew_element *element, unsigned int dim);

int x3dio_readnurbspatchsurface(scew_element *element, int trimmed);

int x3dio_readnurbssweptsurface(scew_element *element, int is_swung);

int x3dio_readnurbsset(scew_element *element);

/* Lights */
int x3dio_readlight(scew_element *element, int type);


/* non-geometric/scene structure */

int x3dio_readinline(scew_element *element);

int x3dio_readtransform(scew_element *element);

int x3dio_readshape(scew_element *element);

int x3dio_readscene(scew_element *element);

int x3dio_adddef(char *name, scew_element *element);

int x3dio_getdef(char *name, scew_element **element);

int x3dio_countelements(scew_element *element, unsigned int *counter);

int x3dio_readelement(scew_element *element);

int x3dio_readtree(scew_tree *tree);

int x3dio_readtcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[]);

/********************/

/* export */
int x3dio_writenpconvertible(FILE *fileptr, ay_object *o, double *m);

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


/* x3dio_trafotoobject:
 *  decompose transformation matrix in <transform> and set transformation
 *  states in <o> accordingly
 *  Matrix Decomposition Code borrowed from Graphics Gems II unmatrix.c.
 */
void
x3dio_trafotoobject(ay_object *o, double *transform)
{
 double v1[3], v2[3], v3[3], v4[3];
 double sx, sy, sz;
 double rx, ry, rz;
 int i;
 double axis[3], quat[4] = {0};
 char fname[] = "x3dio_trafotoobject";

  o->scalx = 1.0;
  o->scaly = 1.0;
  o->scalz = 1.0;
  o->quat[0] = 0.0;
  o->quat[1] = 0.0;
  o->quat[2] = 0.0;
  o->quat[3] = 1.0;
  o->rotx = 0.0;
  o->roty = 0.0;
  o->rotz = 0.0;

  quat[3] = 1.0;

  if(fabs(transform[15]) <= AY_EPSILON )
    return;

  /* normalize matrix */
  for(i = 0; i < 16; i++)
      transform[i] /= transform[15];

  /* decompose matrix */

  /* get translation */
  o->movx = (double)transform[12];
  o->movy = (double)transform[13];
  o->movz = (double)transform[14];

  /* get row vectors containing scale&rotation */
  v1[0] = (double)transform[0];
  v1[1] = (double)transform[1];
  v1[2] = (double)transform[2];

  v2[0] = (double)transform[4];
  v2[1] = (double)transform[5];
  v2[2] = (double)transform[6];

  v3[0] = (double)transform[8];
  v3[1] = (double)transform[9];
  v3[2] = (double)transform[10];

  /* get scale */
  sx = AY_V3LEN(v1);
  sy = AY_V3LEN(v2);
  sz = AY_V3LEN(v3);

  /* normalize row vectors */
  if(fabs(sx) > AY_EPSILON)
    {
      o->scalx *= sx;
      AY_V3SCAL(v1, 1.0/sx);
    }
  if(fabs(sy) > AY_EPSILON)
    {
      o->scaly *= sy;
      AY_V3SCAL(v2, 1.0/sy);
    }
  if(fabs(sz) > AY_EPSILON)
    {
      o->scalz *= sz;
      AY_V3SCAL(v3, 1.0/sz);
    }

  /*
   * Check for a coordinate system flip.  If the determinant
   * is -1, then negate the matrix and the scaling factors.
   */
  AY_V3CROSS(v4, v2, v3)
  if(AY_V3DOT(v1, v4) < 0)
    {
      ay_error(AY_EWARN, fname, "Coordinate system flip detected!");

      o->scalx *= -1.0;
      o->scaly *= -1.0;
      o->scalz *= -1.0;

      for ( i = 0; i < 3; i++ )
	{
	  v1[i] *= -1;
	}
      for ( i = 0; i < 3; i++ )
	{
	  v2[i] *= -1;
	}
      for ( i = 0; i < 3; i++ )
	{
	  v3[i] *= -1;
	}
    }

  /* now get rotation */
  ry = asin(-v1[2]);
  if(cos(ry) != 0)
    {
      rx = atan2(v2[2], v3[2]);
      rz = atan2(v1[1], v1[0]);
    }
  else
    {
      rx = atan2(v2[0], v2[1]);
      rz = 0;
    }

  if(fabs(rx) > AY_EPSILON)
    {
      axis[0] = 1.0;
      axis[1] = 0.0;
      axis[2] = 0.0;
      quat[0] = 0.0;
      quat[1] = 0.0;
      quat[2] = 0.0;
      quat[3] = 1.0;
      ay_quat_axistoquat(axis, -rx, quat);
      ay_quat_add(quat, o->quat, o->quat);
      o->rotx = AY_R2D(rx);
    }

  if(fabs(ry) > AY_EPSILON)
    {
      axis[0] = 0.0;
      axis[1] = 1.0;
      axis[2] = 0.0;
      quat[0] = 0.0;
      quat[1] = 0.0;
      quat[2] = 0.0;
      quat[3] = 1.0;
      ay_quat_axistoquat(axis, -ry, quat);
      ay_quat_add(quat, o->quat, o->quat);
      o->roty = AY_R2D(ry);
    }

  if(fabs(rz) > AY_EPSILON)
    {
      axis[0] = 0.0;
      axis[1] = 0.0;
      axis[2] = 1.0;
      quat[0] = 0.0;
      quat[1] = 0.0;
      quat[2] = 0.0;
      quat[3] = 1.0;
      ay_quat_axistoquat(axis, -rz, quat);
      ay_quat_add(quat, o->quat, o->quat);
      o->rotz = AY_R2D(rz);
    }

 return;
} /* x3dio_trafotoobject */


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
	  if(!strcmp(str, "true"))
	    {
	      *res = 1;
	    }
	  else
	    {
	      if(!strcmp(str, "false"))
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
	      sscanf(str, "%d %d %d %d",  &(res[0]), &(res[1]),
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
	      sscanf(str, "%f %f %f %f",  &(res[0]), &(res[1]),
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
 float *dummy = NULL, *fp;
 unsigned int i;

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
	  while(*p != '\0')
	    {
	      if(!(dummy = realloc(*res, (*len+1)*dim*sizeof(float))))
		{
		  /* XXXX early exit, memory leak? */
		  return AY_EOMEM;
		}
	      *res = dummy;
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
 double *dummy = NULL, *fp;
 unsigned int i;

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
	  while(*p != '\0')
	    {
	      if(!(dummy = realloc(*res, (*len+1)*dim*sizeof(double))))
		{
		  /* XXXX early exit, memory leak? */
		  return AY_EOMEM;
		}
	      *res = dummy;
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
		unsigned int *len, int **res)
{
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL, *p;
 int *dummy = NULL, *ip;

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
	  while(*p != '\0')
	    {
	      if(!(dummy = realloc(*res, (*len+1)*sizeof(int))))
		{
		  /* XXXX early exit, memory leak? */
		  return AY_EOMEM;
		}
	      *res = dummy;
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
} /* x3dio_readindex */


/* x3dio_readcoords:
 *  look through all children of <element> for a "Coordinate" or
 *  "CoordinateDouble" element and read the coordinates into <len> and <res>
 *  XXXX process USE!
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
	  ay_status = x3dio_readdoublepoints(child, "point", 3, len, res);
	  return AY_OK; /* XXXX early exit! */
	}
    } /* while */

  return AY_OK;
} /* x3dio_readcoords */


/* x3dio_readnormals:
 *  look through all children of <element> for a "Normal" element and
 *  read the normals therein into <len> and <res>
 *  XXXX process USE!
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


/* x3dio_readname:
 *
 */
int
x3dio_readname(scew_element *element, ay_object *obj)
{
 int ay_status = AY_OK;
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;
 size_t len = 0;
 char *c;

  if(!element || !obj)
    return AY_ENULL;

  /* set name from DEF */
  attr = scew_attribute_by_name(element, "DEF");
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
  x3dio_trafotoobject(new, x3dio_ctrafos->m);

  /* set name from DEF */
  ay_status = x3dio_readname(element, new);

  /* link the object to the scene */
  ay_status = ay_object_link(new);

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
 double m[16];

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
  memcpy(m, x3dio_ctrafos->m, 16*sizeof(double));

  ay_trafo_rotatematrix(-90.0, 1.0, 0.0, 0.0, x3dio_ctrafos->m);

  /* copy object to the Ayam scene */
  ay_status = x3dio_linkobject(element, AY_IDSPHERE, (void*)&sphere);

  memcpy(x3dio_ctrafos->m, m, 16*sizeof(double));

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
 double m[16];

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
  memcpy(m, x3dio_ctrafos->m, 16*sizeof(double));

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

  memcpy(x3dio_ctrafos->m, m, 16*sizeof(double));

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
 double translate_y = 0.0, m[16];

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

  /* add an additional rotation to fix the orientation */
  memcpy(m, x3dio_ctrafos->m, 16*sizeof(double));

  ay_trafo_rotatematrix(-90.0, 1.0, 0.0, 0.0, x3dio_ctrafos->m);

  /* accomodate for height/position difference */
  ay_trafo_translatematrix(0.0, translate_y, 0.0, x3dio_ctrafos->m);

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

  memcpy(x3dio_ctrafos->m, m, 16*sizeof(double));

 return ay_status;
} /* x3dio_readcone */


/* x3dio_readindexedfaceset:
 *
 */
int
x3dio_readindexedfaceset(scew_element *element)
{
 int ay_status = AY_OK;
 ay_pomesh_object pomesh = {0};
 unsigned int coordlen = 0, normallen = 0, coordilen = 0, normalilen = 0;
 int *coordi = NULL, *normali = NULL;
 int normalPerVertex = AY_FALSE;
 double *coords = NULL, *normals = NULL;
 unsigned int i, j, k, totalverts = 0;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "coordIndex", &coordilen, &coordi);

  if(coordilen > 0)
    {
      /* get normals */
      ay_status = x3dio_readnormals(element, &normallen, &normals);

      ay_status = x3dio_readindex(element, "normalIndex", &normalilen,
				  &normali);

      ay_status = x3dio_readbool(element, "normalPerVertex", &normalPerVertex);

      /* get colors */

      /* get texture coordinates */

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
      if(normalPerVertex)
	{
	  if(normalilen > 0)
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
	    }
	}
      else
	{
	  pomesh.controlv = coords;
	  coords = NULL;
	} /* if */

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDPOMESH, (void*)&pomesh);

    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(normals)
    free(normals);

  if(coordi)
    free(coordi);

  if(normali)
    free(normali);

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
 unsigned int coordlen = 0, normallen = 0, coordilen = 0;
 int *coordi = NULL;
 int normalPerVertex = AY_FALSE;
 double *coords = NULL, *normals = NULL;
 unsigned int i, totalverts = 0;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "index", &coordilen, &coordi);

  if(coordilen > 0)
    {
      /* get normals */
      ay_status = x3dio_readnormals(element, &normallen, &normals);

      ay_status = x3dio_readbool(element, "normalPerVertex", &normalPerVertex);

      /* get colors */

      /* get texture coordinates */

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
	    }
	}
      else
	{
	  pomesh.controlv = coords;
	  coords = NULL;
	} /* if */

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDPOMESH, (void*)&pomesh);

    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(normals)
    free(normals);

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
 unsigned int coordlen = 0, normallen = 0, coordilen = 0;
 int *coordi = NULL;
 int normalPerVertex = AY_FALSE;
 double *coords = NULL, *normals = NULL;
 unsigned int i, totalverts = 0;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "index", &coordilen, &coordi);

  if(coordilen > 0)
    {
      /* get normals */
      ay_status = x3dio_readnormals(element, &normallen, &normals);

      ay_status = x3dio_readbool(element, "normalPerVertex", &normalPerVertex);

      /* get colors */

      /* get texture coordinates */

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
	    }
	}
      else
	{
	  pomesh.controlv = coords;
	  coords = NULL;
	} /* if */

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDPOMESH, (void*)&pomesh);

    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(normals)
    free(normals);

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
 unsigned int coordlen = 0, normallen = 0, coordilen = 0;
 int *coordi = NULL;
 int normalPerVertex = AY_FALSE;
 double *coords = NULL, *normals = NULL;
 unsigned int i, j, k, totalverts = 0;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "index", &coordilen, &coordi);

  if(coordilen > 0)
    {
      /* get normals */
      ay_status = x3dio_readnormals(element, &normallen, &normals);

      ay_status = x3dio_readbool(element, "normalPerVertex", &normalPerVertex);

      /* get colors */

      /* get texture coordinates */

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
	      totalverts++;
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
      j = 0; k = 0;
      for(i = 0; i < pomesh.npolys; i++)
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
	    }
	}
      else
	{
	  pomesh.controlv = coords;
	  coords = NULL;
	} /* if */

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDPOMESH, (void*)&pomesh);

    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(normals)
    free(normals);

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
 unsigned int coordlen = 0, normallen = 0, coordilen = 0;
 int *coordi = NULL;
 int normalPerVertex = AY_FALSE;
 double *coords = NULL, *normals = NULL;
 unsigned int i, j, k, totalverts = 0;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "index", &coordilen, &coordi);

  if(coordilen > 0)
    {
      /* get normals */
      ay_status = x3dio_readnormals(element, &normallen, &normals);

      ay_status = x3dio_readbool(element, "normalPerVertex", &normalPerVertex);

      /* get colors */

      /* get texture coordinates */

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
	      totalverts++;
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
      j = 0; k = 0;
      for(i = 0; i < pomesh.npolys; i++)
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
	    }
	}
      else
	{
	  pomesh.controlv = coords;
	  coords = NULL;
	} /* if */

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(element, AY_IDPOMESH, (void*)&pomesh);

    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(normals)
    free(normals);

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

  ay_status = x3dio_readindex(element, "coordIndex", &coordilen, &coordi);

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

  ay_status = x3dio_readindex(element, "vertexCount",
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
 ay_pomesh_object pomesh = {0};
 unsigned int coordlen = 0, normallen = 0, fancountslen = 0;
 int *fancounts = NULL;
 int normalPerVertex = AY_FALSE;
 double *coords = NULL, *normals = NULL;
 unsigned int i, j, k, l, totalverts = 0;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "fanCount", &fancountslen, &fancounts);

  if(fancountslen > 0)
    {
      /* get normals */
      ay_status = x3dio_readnormals(element, &normallen, &normals);

      ay_status = x3dio_readbool(element, "normalPerVertex", &normalPerVertex);

      /* get colors */

      /* get texture coordinates */

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

    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(normals)
    free(normals);

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
 ay_pomesh_object pomesh = {0};
 unsigned int coordlen = 0, normallen = 0, stripcountslen = 0;
 int *stripcounts = NULL;
 int normalPerVertex = AY_FALSE;
 double *coords = NULL, *normals = NULL;
 unsigned int i, j, k, l, totalverts = 0;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readcoords(element, &coordlen, &coords);

  if(coordlen == 0)
    {
      return AY_OK;
    }

  ay_status = x3dio_readindex(element, "stripCount",
			      &stripcountslen, &stripcounts);

  if(stripcountslen > 0)
    {
      /* get normals */
      ay_status = x3dio_readnormals(element, &normallen, &normals);

      ay_status = x3dio_readbool(element, "normalPerVertex", &normalPerVertex);

      /* get colors */

      /* get texture coordinates */

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

    } /* if */

cleanup:
  if(coords)
    free(coords);

  if(normals)
    free(normals);

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
 ay_pomesh_object pomesh = {0};
 unsigned int coordlen = 0, normallen = 0;
 int normalPerVertex = AY_FALSE;
 double *coords = NULL, *normals = NULL;
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

  /* get texture coordinates */

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

cleanup:
  if(coords)
    free(coords);

  if(normals)
    free(normals);

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
 ay_pomesh_object pomesh = {0};
 unsigned int coordlen = 0, normallen = 0;
 int normalPerVertex = AY_FALSE;
 double *coords = NULL, *normals = NULL;
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

  /* get texture coordinates */

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

cleanup:
  if(coords)
    free(coords);

  if(normals)
    free(normals);

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
    {
      return AY_ERROR;
    }

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

  /* immediately create NURBS patch representation */
  ay_status = ay_pmt_tonpatch(&pamesh, &(pamesh.npatch));

  /* copy object to the Ayam scene */
  ay_status = x3dio_linkobject(element, AY_IDPAMESH, (void*)&pamesh);

cleanup:

  if(heights)
    free(heights);

  if(pamesh.controlv)
    free(pamesh.controlv);

  if(pamesh.npatch)
    ay_object_delete(pamesh.npatch);

 return ay_status;
} /* x3dio_readelevationgrid */


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
 int has_startcap = AY_TRUE, has_endcap = AY_TRUE;
 ay_pomesh_object pomesh = {0};
 unsigned int totalverts = 0, i, j, a = 0, b = 0;
 /* double rotx = 0.0, roty = 0.0, rotz = 0.0;*/

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readfloatpoints(element, "crossSection", 2,
				    &cslen, &cs);

  if(cslen == 0)
    {
      cs = csd;
      cslen = 5;
    }

  ay_status = x3dio_readfloatpoints(element, "spine", 3,
				    &splen, &sp);

  if(splen == 0)
    {
      sp = spd;
      splen = 2;
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

  pomesh.npolys = (cslen-1) * (splen-1) * 2 +
    (has_startcap?1:0) + (has_endcap?1:0);
  totalverts = pomesh.npolys * 3 +
    (has_startcap?cslen:0) + (has_endcap?cslen:0);

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

  /* allocate and fill controlv */
  pomesh.ncontrols = cslen * splen;

  if(!(pomesh.controlv = calloc(3*pomesh.ncontrols, sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }

  a = 0;
  for(i = 0; i < splen; i++)
    {
      /* calculate rotation angles */
      if(i == 0)
	{
	  /* first spine point */

	}
      if((i > 0) && (i < splen-1))
	{
	  /* middle spine point */


	}
      if(i == splen-1)
	{
	  /* last spine point */
	}

      for(j = 0; j < cslen; j++)
	{
	  /* take cross section */
	  pomesh.controlv[a]   = cs[j*2];
	  pomesh.controlv[a+1] = 0.0;
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

	  /* move to spine */
	  pomesh.controlv[a]   += sp[i*3];
	  pomesh.controlv[a+1] += sp[i*3+1];
	  pomesh.controlv[a+1] += sp[i*3+2];

	  /* apply rotation */

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
      ay_status = x3dio_readfloatpoints(element, "point", 2, &len, &cv2d);
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

  ay_status = x3dio_readdoublepoints(element, "weight", 1, &wlen, &w);
  if(wlen >= len)
    {
      has_weights = AY_TRUE;
    }

  ay_status = x3dio_readdoublepoints(element, "knot", 1, &klen, &knots);
  if(klen >= len+order)
    {
      has_knots = AY_TRUE;
    }

  if(len == 0)
    {
      ay_status = x3dio_readcoords(element, &len, &dcv);
      is_double = AY_TRUE;
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
	  nc.knot_type = AY_KTCUSTOM;
	  nc.knotv = knots;
	  knots = NULL;
	}
      else
	{
	  nc.knot_type = AY_KTBSPLINE;
	  ay_status = ay_knots_createnc(&nc);
	}

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


/* x3dio_readnurbspatchsurface:
 *
 */
int
x3dio_readnurbspatchsurface(scew_element *element, int trimmed)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object np = {0};
 ay_object *o, **old_aynext;
 float *cv = NULL;
 double *dcv = NULL, *w = NULL, *uknots = NULL, *vknots = NULL;
 unsigned int i, len = 0, wlen = 0, uklen = 0, vklen = 0;
 int width = 0, height = 0, uorder = 3, vorder = 3, stride = 4;
 int has_weights = AY_FALSE, has_uknots = AY_FALSE, has_vknots = AY_FALSE;
 int is_double = AY_FALSE;
 scew_element *child = NULL;
 const char *element_name = NULL;

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readint(element, "uOrder", &uorder);
  ay_status = x3dio_readint(element, "vOrder", &vorder);
  ay_status = x3dio_readint(element, "uDimension", &width);
  ay_status = x3dio_readint(element, "vDimension", &height);

  ay_status = x3dio_readfloatpoints(element, "controlPoint", 3, &len, &cv);

  ay_status = x3dio_readdoublepoints(element, "weight", 1, &wlen, &w);
  if(wlen >= len)
    {
      has_weights = AY_TRUE;
    }

  ay_status = x3dio_readdoublepoints(element, "uKnot", 1, &uklen, &uknots);
  if(uklen >= (unsigned int)width+uorder)
    {
      has_uknots = AY_TRUE;
    }
  ay_status = x3dio_readdoublepoints(element, "vKnot", 1, &vklen, &vknots);
  if(vklen >= (unsigned int)height+vorder)
    {
      has_vknots = AY_TRUE;
    }

  if(len == 0)
    {
      ay_status = x3dio_readcoords(element, &len, &dcv);
      is_double = AY_TRUE;
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
	  np.uknot_type = AY_KTCUSTOM;
	  np.uknotv = uknots;
	  uknots = NULL;
	}
      else
	{
	  np.uknot_type = AY_KTBSPLINE;
	}

      if(has_vknots)
	{
	  np.vknot_type = AY_KTCUSTOM;
	  np.vknotv = vknots;
	  vknots = NULL;
	}
      else
	{
	  np.vknot_type = AY_KTBSPLINE;
	}

      if(!has_uknots || !has_uknots)
	{
	  ay_status = ay_knots_createnp(&np);
	}

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

      /* read trim curves? */
      if(trimmed)
	{
	  old_aynext = ay_next;
	  ay_next = &(x3dio_lrobject->down);
	  o = x3dio_lrobject;
	  x3dio_lrobject = NULL;

	  while((child = scew_element_next(element, child)) != NULL)
	    {
	      element_name = scew_element_name(element);
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

	  /* create endlevel object */
	  if(x3dio_lrobject)
	    ay_object_crtendlevel(&(x3dio_lrobject->next));
	  else
	    ay_object_crtendlevel(&(o->down));
	  ay_next = old_aynext;
	}
      else
	{
	  /* just create endlevel object */
	  ay_object_crtendlevel(&(x3dio_lrobject->down));
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
x3dio_readnurbssweptsurface(scew_element *element, int is_swung)
{
 int ay_status = AY_OK;
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;
 scew_element *child;
 ay_object *o = NULL, **old_aynext;
 ay_sweep_object *sweep = NULL;
 const char *cs_name = NULL, *tr_name = "trajectoryCurve";

  if(!element)
    return AY_ENULL;

  if(is_swung)
    {
      cs_name = "profileCurve";
    }
  else
    {
      cs_name = "crossSectionCurve";
    }

  if(!(o = calloc(1, sizeof(ay_object))))
    {
      return AY_EOMEM;
    }

  if(!(sweep = calloc(1, sizeof(ay_sweep_object))))
    {
      free(o); return AY_EOMEM;
    }

  sweep->rotate = AY_TRUE;
  sweep->sections = 10;

  o->refine = sweep;

  ay_status = ay_object_defaults(o);

  o->type = AY_IDSWEEP;
  o->parent = AY_TRUE;

  old_aynext = ay_next;
  ay_next = &(o->down);

  /* read children to get the cross section and the trajectory */
  child = NULL;
  while((child = scew_element_next(element, child)) != NULL)
    {
      attr = scew_attribute_by_name(element, "containerField");
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
      attr = scew_attribute_by_name(element, "containerField");
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

  ay_object_crtendlevel(ay_next);
  ay_next = old_aynext;
  ay_object_link(o);

  ay_status = x3dio_readname(element, o);

cleanup:

 return ay_status;
} /* x3dio_readnurbssweptsurface */


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
  light.colr = (int)color[0]*255;
  light.colg = (int)color[1]*255;
  light.colb = (int)color[2]*255;

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

  if(!element)
    return AY_ENULL;

  ay_status = x3dio_readbool(element, "load", &load);

  if(load)
    {
      attr = scew_attribute_by_name(element, "url");
      if(attr)
	{
	  str = scew_attribute_value(attr);

	  ay_error(AY_EOUTPUT, fname, "Inlining file:");
	  ay_error(AY_EOUTPUT, fname, str);

	  /* initialize XML parser */
	  parser = scew_parser_create();

	  scew_parser_ignore_whitespaces(parser, 1);

	  /* load an XML (X3D) file */
	  if(!scew_parser_load_file(parser, str))
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
  /*int ay_status = AY_OK;*/
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;

  if(!element)
    return AY_ENULL;

  attr = scew_attribute_by_name(element, "DEF");
  if(attr)
    {
      str = scew_attribute_value(attr);
    }

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
      ay_trafo_rotatematrix(rotation[3], rotation[0], rotation[1], rotation[2],
			    x3dio_ctrafos->m);
    }

  if(fabs(scaleorient[3]) > AY_EPSILON)
    {
      ay_trafo_rotatematrix(scaleorient[3], scaleorient[0],
			    scaleorient[1], scaleorient[2],
			    x3dio_ctrafos->m);
    }
  ay_trafo_scalematrix(scale[0], scale[1], scale[2],
		       x3dio_ctrafos->m);
  if(fabs(scaleorient[3]) > AY_EPSILON)
    {
      ay_trafo_rotatematrix(-scaleorient[3], scaleorient[0],
			    scaleorient[1], scaleorient[2],
			    x3dio_ctrafos->m);
    }

  ay_trafo_translatematrix(-center[0], -center[1], -center[2],
			   x3dio_ctrafos->m);

  /* read children */
  while((child = scew_element_next(element, child)) != NULL)
    {
      ay_status = x3dio_readelement(child);
      if(ay_status == AY_EDONOTLINK)
	break;
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

  ay_status = ay_object_defaults(o);

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
      ay_object_crtendlevel(ay_next);
      ay_next = old_aynext;
      ay_object_link(o);
      /* read shape name from DEF */
      ay_status = x3dio_readname(element, o);
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
	      ay_status = x3dio_readname(element, o->down);
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


/* x3dio_countelements:
 *  _recursively_ counts the child elements/nodes of <element>
 *  increases <counter> for each child
 */
int
x3dio_countelements(scew_element *element, unsigned int *counter)
{
 int ay_status = AY_OK;
 scew_element *child = NULL;

  while((child = scew_element_next(element, child)) != NULL)
    {
      (*counter)++;
      ay_status = x3dio_countelements(child, counter);
    }

 return AY_OK;
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
      if(!strcmp(element_name, "Cylinder"))
	{
	  ay_status = x3dio_readcylinder(element);
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
	  handled_elements = 0;
	}
      if(!strcmp(element_name, "Contour2D"))
	{
	  ay_status = x3dio_readshape(element);
	  handled_elements = 0;
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
	  ay_status = x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "IndexedTriangleSet"))
	{
	  ay_status = x3dio_readindexedtriangleset(element);
	  ay_status = x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "IndexedTriangleStripSet"))
	{
	  ay_status = x3dio_readindexedtrianglestripset(element);
	  ay_status = x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "IndexedTriangleFanSet"))
	{
	  ay_status = x3dio_readindexedtrianglefanset(element);
	  ay_status = x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "IndexedLineSet"))
	{
	  ay_status = x3dio_readindexedlineset(element);
	  ay_status = x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "IndexedQuadSet"))
	{
	  ay_status = x3dio_readindexedquadset(element);
	  ay_status = x3dio_countelements(element, &handled_elements);
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
	  ay_status = x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "NurbsCurve2D"))
	{
	  ay_status = x3dio_readnurbscurve(element, 2);
	  ay_status = x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "NurbsPatchSurface"))
	{
	  ay_status = x3dio_readnurbspatchsurface(element, AY_FALSE);
	  ay_status = x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "NurbsTrimmedSurface"))
	{
	  ay_status = x3dio_readnurbspatchsurface(element, AY_TRUE);
	  ay_status = x3dio_countelements(element, &handled_elements);
	}
      if(!strcmp(element_name, "NurbsSweptSurface"))
	{
	  ay_status = x3dio_readnurbssweptsurface(element, AY_FALSE);
	  handled_elements = 1;
	}
      if(!strcmp(element_name, "NurbsSwungSurface"))
	{
	  ay_status = x3dio_readnurbssweptsurface(element, AY_TRUE);
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
    case 'V':
      break;
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
      ay_status = x3dio_countelements(element, &handled_elements);
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
      to = Tcl_NewIntObj(15);
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
 char fname[] = "x3dioRead";
 char errstr[256];
 int i = 2;
 double accuracy = 0.1;
 char arrname[] = "x3dio_options", varname[] = "Progress";
 scew_tree *tree = NULL;
 scew_parser *parser = NULL;
 scew_error errcode;
 enum XML_Error expat_code;
 scew_element *element = NULL, *child = NULL;

  /* set default import options and reset global counters */
  x3dio_importcurves = AY_TRUE;
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
      ay_error(AY_EARGS, fname, "filename");
      return TCL_OK;
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
      if(!strcmp(argv[i], "-e"))
	{
	  sscanf(argv[i+1], "%d", &x3dio_errorlevel);
	}
      else
      if(!strcmp(argv[i], "-r"))
	{
	  sscanf(argv[i+1], "%lg", &x3dio_rescaleknots);
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
      if(!strcmp(argv[i], "-t"))
	{
	  x3dio_stagname = argv[i+1];
	  x3dio_ttagname = argv[i+2];
	  i++;
	}
      i += 2;
    } /* while */

  /* create and initialize hashtable for DEFs */
  if(!(x3dio_defs_ht = calloc(1, sizeof(Tcl_HashTable))))
    return TCL_OK;
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
      return TCL_OK;
    } /* if */

  tree = scew_parser_tree(parser);

  /* count elements */
  element = scew_tree_root(tree);
  while((child = scew_element_next(element, child)) != NULL)
    {
      x3dio_totalelements++;
      ay_status = x3dio_countelements(child, &x3dio_totalelements);
    }

  /* set progress */
  Tcl_SetVar2(ay_interp, arrname, varname, "50",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  /* convert XML tree to Ayam objects */
  ay_status = x3dio_readtree(tree);
  if(ay_status == AY_EDONOTLINK)
    {
      if(x3dio_errorlevel > 1)
	{
	  ay_error(AY_EOUTPUT, fname,
		   "Import cancelled! Not all objects may have been read!");
	}
    }

  /* set progress */
  Tcl_SetVar2(ay_interp, arrname, varname, "100",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  /* clean up */
  x3dio_cleartrafo();

  scew_tree_free(tree);

  scew_parser_free(parser);

  Tcl_DeleteHashTable(x3dio_defs_ht);

  x3dio_stagname = x3dio_stagnamedef;
  x3dio_ttagname = x3dio_ttagnamedef;

 return TCL_OK;
} /* x3dio_readtcmd */


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

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
 *
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

#if 0

/* x3dio_writevertices:
 *  write <n> <stride>D-texturevertices from array <v[n*stride]> to
 *  file <fileptr>
 */
int
x3dio_writevertices(scew_element *element, unsigned int n, int stride, double *v)
{
 unsigned int i, j = 0;

  switch(stride)
    {
    case 2:
      for(i = 0; i < n; i++)
	{
	  fprintf(fileptr, "v %g %g\n", v[j], v[j+1]);
	  j += stride;
	}
      break;
    case 3:
      for(i = 0; i < n; i++)
	{
	  fprintf(fileptr, "v %g %g %g\n", v[j], v[j+1], v[j+2]);
	  j += stride;
	}
      break;
    case 4:
      for(i = 0; i < n; i++)
	{
	  fprintf(fileptr, "v %g %g %g %g\n", v[j], v[j+1], v[j+2],
		  v[j+3]);
	  j += stride;
	}
      break;
    default:
      return AY_ERROR;
      break;
    } /* switch */

 return AY_OK;
} /* x3dio_writevertices */


/* x3dio_writetvertices:
 *  write <n> <stride>D-vertices from array <v[n*stride]> to file <fileptr>
 */
int
x3dio_writetvertices(scew_element *element, unsigned int n, int stride, double *v)
{
 unsigned int i, j = 0;

  switch(stride)
    {
    case 2:
      for(i = 0; i < n; i++)
	{
	  fprintf(fileptr, "vt %g %g\n", v[j], v[j+1]);
	  j += stride;
	}
      break;
    case 3:
      for(i = 0; i < n; i++)
	{
	  fprintf(fileptr, "vt %g %g %g\n", v[j], v[j+1], v[j+2]);
	  j += stride;
	}
      break;
    default:
      return AY_ERROR;
      break;
    } /* switch */

 return AY_OK;
} /* x3dio_writetvertices */


/* x3dio_writencurve:
 *
 */
int
x3dio_writencurve(scew_element *element, ay_object *o, double *m)
{
 ay_nurbcurve_object *nc;
 double *v = NULL, *p1, *p2, pw[3], umin, umax;
 int stride = 4, i;

  if(!x3dio_writecurves)
    return AY_OK;

  if(!o)
    return AY_ENULL;

  nc = (ay_nurbcurve_object *)o->refine;

  /* get all vertices and transform them to world space */
  if(!(v = calloc(nc->length * (nc->is_rat?4:3), sizeof(double))))
    return AY_EOMEM;

  p1 = v;
  p2 = nc->controlv;
  for(i = 0; i < nc->length; i++)
    {
      if(nc->is_rat)
	{
	  pw[0] = p2[0]/p2[3];
	  pw[1] = p2[1]/p2[3];
	  pw[2] = p2[2]/p2[3];
	  AY_APTRAN3(p1,pw,m)
	  p1[3] = p2[3];
	  p1 += stride;
	}
      else
	{
	  AY_APTRAN3(p1,p2,m)
	  p1 += 3;
	} /* if */
      p2 += 4;
    } /* for */

  /* write all vertices */
  x3dio_writevertices(fileptr, (unsigned int)nc->length,
			 nc->is_rat?4:3, v);

  /* write bspline curve */
  if(nc->is_rat)
    fprintf(fileptr, "cstype rat bspline\n");
  else
    fprintf(fileptr, "cstype bspline\n");

  fprintf(fileptr, "deg %d\n", nc->order-1);

  ay_knots_getuminmax(o, nc->order, nc->length+nc->order, nc->knotv,
		      &umin, &umax);

  fprintf(fileptr, "curv %g %g", umin, umax);

  for(i = nc->length; i > 0; i--)
    {
      fprintf(fileptr, " -%d", i);
    }
  fprintf(fileptr, "\n");

  /* write knot vector */
  fprintf(fileptr, "parm u");
  for(i = 0; i < (nc->length + nc->order); i++)
    {
      fprintf(fileptr, " %g", nc->knotv[i]);
    }
  fprintf(fileptr, "\n");

  free(v);

 return AY_OK;
} /* x3dio_writencurve */


/* x3dio_writetcurve:
 *  write a single trim curve
 */
int
x3dio_writetcurve(scew_element *element, ay_object *o, double *m)
{
 ay_nurbcurve_object *nc;
 double v[3] = {0}, *p1, pw[3] = {0}, ma[16] = {0}, mn[16] = {0};
 int stride = 4, i;

  if(!o)
    return AY_ENULL;

  nc = (ay_nurbcurve_object *)o->refine;

  /* create proper transformation matrix */
  ay_trafo_creatematrix(o, mn);
  memcpy(ma, m, 16*sizeof(double));
  ay_trafo_multmatrix4(ma, mn);

  /* get all vertices, transform them and write them out */

  p1 = nc->controlv;
  for(i = 0; i < nc->length; i++)
    {
      if(nc->is_rat)
	{
	  pw[0] = p1[0]/p1[3];
	  pw[1] = p1[1]/p1[3];
	  AY_APTRAN3(v,pw,ma)
	    v[2] = p1[3];
	  fprintf(fileptr, "vp %g %g %g\n", v[0], v[1], v[2]);
	}
      else
	{
	  AY_APTRAN3(v,p1,ma)
	  fprintf(fileptr, "vp %g %g\n", v[0], v[1]);
	}
      p1 += stride;
    }

  /* write 2D bspline curve */
  if(nc->is_rat)
    fprintf(fileptr, "cstype rat bspline\n");
  else
    fprintf(fileptr, "cstype bspline\n");

  fprintf(fileptr, "deg %d\n", nc->order-1);
  fprintf(fileptr, "curv2 ");

  for(i = nc->length; i > 0; i--)
    {
      fprintf(fileptr, " -%d", i);
    }
  fprintf(fileptr, "\n");

  /* write knot vector */
  fprintf(fileptr, "parm u");
  for(i = 0; i < (nc->length + nc->order); i++)
    {
      fprintf(fileptr, " %g", nc->knotv[i]);
    }
  fprintf(fileptr, "\nend\n");

 return AY_OK;
} /* x3dio_writetcurve */


/* x3dio_writetrim:
 *  write all trim curves pointed to by <o>
 */
int
x3dio_writetrim(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 double mi[16] = {0};
 ay_object *down = NULL, *pnc = NULL;

  ay_trafo_identitymatrix(mi);

  while(o->next)
    {
      switch(o->type)
	{
	case AY_IDNCURVE:
	  x3dio_writetcurve(fileptr, o, mi);
	  break;
	case AY_IDLEVEL:
	  if((o->down) && (o->down->next))
	    {
	      ay_trafo_creatematrix(o, mi);
	      down = o->down;
	      while(down->next)
		{
		  if(down->type == AY_IDNCURVE)
		    {
		      x3dio_writetcurve(fileptr, down, mi);
		    }
		  down = down->next;
		} /* while */
	      ay_trafo_identitymatrix(mi);
	    } /* if */
	  break;
	default:
	  pnc = NULL;
	  ay_status = ay_provide_object(o, AY_IDNCURVE, &pnc);
	  down = pnc;
	  while(down)
	    {
	      x3dio_writetcurve(fileptr, pnc, mi);
	      down = down->next;
	    }
	  if(pnc)
	    {
	      ay_object_deletemulti(pnc);
	    }
	  break;
	} /* switch */

      o = o->next;
    } /* while */

 return AY_OK;
} /* x3dio_writetrim */


/* x3dio_writetrimids:
 *  write ids of all trim curves pointed to by <o>
 */
int
x3dio_writetrimids(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *o2 = o, *down = NULL, *pnc = NULL, *cnc = NULL;
 ay_nurbcurve_object *nc = NULL;
 double umin, umax, orient;
 int tc = 0, hole;

  /* first, count trim curves */
  while(o->next)
    {
      switch(o->type)
	{
	case AY_IDNCURVE:
	  tc++;
	  break;
	case AY_IDLEVEL:
	  if((o->down) && (o->down->next))
	    {
	      down = o->down;
	      while(down->next)
		{
		  if(down->type == AY_IDNCURVE)
		    {
		      tc++;
		    }
		  down = down->next;
		} /* while */
	    } /* if */
	default:
	  pnc = NULL;
	  ay_status = ay_provide_object(o, AY_IDNCURVE, &pnc);
	  down = pnc;
	  while(down)
	    {
	      tc++;
	      down = down->next;
	    }
	  if(pnc)
	    {
	      ay_object_deletemulti(pnc);
	    }
	  break;
	} /* switch */

      o = o->next;
    } /* while */

  /* now write the ids */
  o = o2;
  while(o->next)
    {
      switch(o->type)
	{
	case AY_IDNCURVE:
	  nc = (ay_nurbcurve_object *)o->refine;
	  ay_status = ay_nct_getorientation(nc, &orient);
	  if(orient < 0.0)
	    hole = AY_TRUE;
	  else
	    hole = AY_FALSE;

	  ay_knots_getuminmax(o, nc->order, nc->length+nc->order, nc->knotv,
			      &umin, &umax);

	  if(hole)
	    fprintf(fileptr, "hole %g %g -%d\n", umin, umax, tc);
	  else
	    fprintf(fileptr, "trim %g %g -%d\n", umin, umax, tc);
	  tc--;
	  break;
	case AY_IDLEVEL:
	  if((o->down) && (o->down->next))
	    {
	      down = o->down;
	      if(down->type == AY_IDNCURVE)
		{
		  nc = (ay_nurbcurve_object *)down->refine;
		  ay_status = ay_nct_getorientation(nc, &orient);
		  if(ay_status)
		    {
		      /* failed to detect orientation, maybe the curve
		       * is too simple, concat all curves and retry... */
		      ay_status = ay_nct_concatmultiple(AY_FALSE, 0, AY_FALSE,
							o->down, &cnc);
		      if(cnc)
			{
			  nc = (ay_nurbcurve_object *)cnc->refine;
			  ay_status = ay_nct_getorientation(nc, &orient);
			  ay_object_delete(cnc);
			  cnc = NULL;
			}
		    } /* if */

		  if(orient < 0.0)
		    fprintf(fileptr, "hole ");
		  else
		    fprintf(fileptr, "trim ");
		} /* if */

	      while(down->next)
		{
		  if(down->type == AY_IDNCURVE)
		    {
		      nc = (ay_nurbcurve_object *)down->refine;

		      ay_knots_getuminmax(o, nc->order, nc->length+nc->order,
					  nc->knotv,
					  &umin, &umax);

		      fprintf(fileptr, " %g %g -%d", umin, umax, tc);

		      tc--;
		    }
		  down = down->next;
		} /* while */
	      fprintf(fileptr, "\n");
	    } /* if */
	  break;
	default:
	  pnc = NULL;
	  ay_status = ay_provide_object(o, AY_IDNCURVE, &pnc);
	  down = pnc;

	  if(down->type == AY_IDNCURVE)
	    {
	      nc = (ay_nurbcurve_object *)down->refine;
	      ay_status = ay_nct_getorientation(nc, &orient);
	      if(ay_status)
		{
		  /* failed to detect orientation, maybe the curve
		   * is too simple, concat all curves and retry... */
		  ay_status = ay_nct_concatmultiple(AY_FALSE, 0, AY_FALSE,
						    down, &cnc);
		  if(cnc)
		    {
		      nc = (ay_nurbcurve_object *)cnc->refine;
		      ay_status = ay_nct_getorientation(nc, &orient);
		      ay_object_delete(cnc);
		      cnc = NULL;
		    }
		} /* if */

	      if(orient < 0.0)
		fprintf(fileptr, "hole ");
	      else
		fprintf(fileptr, "trim ");
	    } /* if */

	  while(down->next)
	    {
	      if(down->type == AY_IDNCURVE)
		{
		  nc = (ay_nurbcurve_object *)down->refine;

		  ay_knots_getuminmax(o, nc->order, nc->length+nc->order,
				      nc->knotv,
				      &umin, &umax);

		  fprintf(fileptr, " %g %g -%d", umin, umax, tc);

		  tc--;
		} /* if */
	      down = down->next;
	    } /* while */
	  fprintf(fileptr, "\n");

	  if(pnc)
	    {
	      ay_object_deletemulti(pnc);
	    }
	  break;
	} /* switch */
      o = o->next;
    } /* while */

 return AY_OK;
} /* x3dio_writetrimids */


/* x3dio_writenpatch:
 *
 */
int
x3dio_writenpatch(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *np;
 double *v = NULL, *p1, *p2, pw[3];
 double umin, umax, vmin, vmax;
 int stride = 4, i, j;
 int have_mys = AY_FALSE, have_myt = AY_FALSE;
 unsigned int myslen = 0, mytlen = 0, mystlen = 0, ui, uj;
 double *mysarr = NULL, *mytarr = NULL, *mystarr = NULL;
 ay_tag mystag = {NULL, 0, NULL};
 ay_tag myttag = {NULL, 0, NULL};
 ay_tag *tag;

  if(!o)
    return AY_ENULL;

  mystag.type = ay_pv_tagtype;
  myttag.type = ay_pv_tagtype;

  /* first, check for and write the trim curves */
  if(o->down && o->down->next)
    {
      ay_status = x3dio_writetrim(fileptr, o->down);
    }

  np = (ay_nurbpatch_object *)o->refine;

  /* get all vertices and transform them to world space,
     also adapting row/column major order in the process */
  if(!(v = calloc(np->width * np->height * (np->is_rat?4:3), sizeof(double))))
    return AY_EOMEM;

  p1 = v;
  for(i = 0; i < np->height; i++)
    {
      p2 = &(np->controlv[i*stride]);
      for(j = 0; j < np->width; j++)
	{
	  if(np->is_rat)
	    {
	      pw[0] = p2[0]/p2[3];
	      pw[1] = p2[1]/p2[3];
	      pw[2] = p2[2]/p2[3];
	      AY_APTRAN3(p1,pw,m)
		p1[3] = p2[3];
	      p1 += 4;
	    }
	  else
	    {
	      AY_APTRAN3(p1,p2,m)
	      p1 += 3;
	    } /* if */
	  p2 += np->height*stride;
	} /* for */
    } /* for */

  /* write all vertices */
  x3dio_writevertices(fileptr, (unsigned int)(np->width * np->height),
			 (np->is_rat?4:3), v);

  /* write texture coordinates from potentially present PV tags */
  if(o->tags)
    {
      if(!(mystag.val = calloc(strlen(x3dio_stagname)+2,sizeof(char))))
	return AY_EOMEM;
      if(!(myttag.val = calloc(strlen(x3dio_ttagname)+2,sizeof(char))))
	return AY_EOMEM;
      strcpy(mystag.val, x3dio_stagname);
      mystag.val[strlen(x3dio_stagname)] = ',';
      strcpy(myttag.val, x3dio_ttagname);
      myttag.val[strlen(x3dio_ttagname)] = ',';
      tag = o->tags;
      while(tag)
	{
	  if((tag->type == ay_pv_tagtype) && ay_pv_cmpname(tag, &mystag))
	    {
	      have_mys = AY_TRUE;

	      ay_status = ay_pv_convert(tag, &myslen, (void**)&mysarr);
	    }
	  if((tag->type == ay_pv_tagtype) && ay_pv_cmpname(tag, &myttag))
	    {
	      have_myt = AY_TRUE;

	      ay_status = ay_pv_convert(tag, &mytlen, (void**)&mytarr);
	    }
	  tag = tag->next;
	} /* while */
      free(mystag.val);
      free(myttag.val);
    } /* if */

  /* merge and write the texture vertices */
  if(have_mys)
    mystlen = 2*myslen;
  else
    if(have_myt)
      mystlen = 2*mytlen;

  if(mystlen > 0)
    {
      if(!(mystarr = calloc(mystlen, sizeof(double))))
	{
	  if(v)
	    free(v);
	  if(mysarr)
	    free(mysarr);
	  if(mytarr)
	    free(mytarr);
	  return AY_EOMEM;
	} /* if */
      /* i am C/C++ line 111111 in Ayam :) */
      uj = 0;
      for(ui = 0; ui < mystlen; ui++)
	{
	  if(have_mys)
	    mystarr[uj]   = mysarr[ui];
	  if(have_myt)
	    mystarr[uj+1] = mytarr[ui];
	  uj += 2;
	} /* for */

      x3dio_writetvertices(fileptr, mystlen, 2, mystarr);

      if(mysarr)
	free(mysarr);
      if(mytarr)
	free(mytarr);
      free(mystarr);
      mystarr = NULL;
    } /* if */

  /* write bspline surface */
  if(np->is_rat)
    fprintf(fileptr, "cstype rat bspline\n");
  else
    fprintf(fileptr, "cstype bspline\n");

  fprintf(fileptr, "deg %d %d\n", np->uorder-1, np->vorder-1);

  ay_knots_getuminmax(o, np->uorder, np->width+np->uorder, np->uknotv,
		      &umin, &umax);
  ay_knots_getvminmax(o, np->vorder, np->height+np->vorder, np->vknotv,
		      &vmin, &vmax);

  fprintf(fileptr, "surf %g %g %g %g", umin, umax, vmin, vmax);

  for(i = np->width*np->height; i > 0; i--)
    {
      if(have_mys || have_myt)
	{
	  fprintf(fileptr, " -%d/-%d", i, i);
	}
      else
	{
	  fprintf(fileptr, " -%d", i);
	}
    } /* for */
  fprintf(fileptr, "\n");

  /* write knot vector (u) */
  fprintf(fileptr, "parm u");
  for(i = 0; i < (np->width + np->uorder); i++)
    {
      fprintf(fileptr, " %g", np->uknotv[i]);
    }
  fprintf(fileptr, "\n");

  /* write knot vector (v) */
  fprintf(fileptr, "parm v");
  for(i = 0; i < (np->height + np->vorder); i++)
    {
      fprintf(fileptr, " %g", np->vknotv[i]);
    }
  fprintf(fileptr, "\n");

  /* write pointers to trim curves (if any) */
  if(o->down && o->down->next)
    {
      x3dio_writetrimids(fileptr, o->down);
    } /* if */

  free(v);

 return AY_OK;
} /* x3dio_writenpatch */


/* x3dio_writelevel:
 *
 */
int
x3dio_writelevel(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *down = NULL;
 ay_level_object *lev;
 double m1[16] = {0};

  if(!o)
    return AY_ENULL;

  lev = (ay_level_object *)o->refine;
  if(o->down && o->down->next)
    {
      memcpy(m1, tm, 16*sizeof(double));
      memcpy(tm, m, 16*sizeof(double));
      down = o->down;
      while(down->next && down->next->next)
	{
	  ay_status = x3dio_writeobject(fileptr, down, AY_TRUE, AY_TRUE);
	  down = down->next;
	}

      if(down)
	{
	  ay_status = x3dio_writeobject(fileptr, down, AY_FALSE, AY_TRUE);
	}

      memcpy(tm, m1, 16*sizeof(double));
    } /* if */

 return AY_OK;
} /* x3dio_writelevel */


/* x3dio_writencconvertible:
 *
 */
int
x3dio_writencconvertible(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *c = NULL, *t;

  if(!x3dio_writecurves)
    return AY_OK;

  if(!o)
   return AY_ENULL;

  ay_status = ay_provide_object(o, AY_IDNCURVE, &c);
  if(!c)
    return AY_ERROR;
  t = c;
  while(t->next)
    {
      if(t->type == AY_IDNCURVE)
	{
	  ay_status = x3dio_writeobject(fileptr, t, AY_TRUE, AY_FALSE);
	}

      t = t->next;
    } /* while */

  if(t->type == AY_IDNCURVE)
    {
      ay_status = x3dio_writeobject(fileptr, t, AY_FALSE, AY_FALSE);
    }

  ay_status = ay_object_deletemulti(c);

 return ay_status;
} /* x3dio_writencconvertible */


/* x3dio_writenpconvertible:
 *
 */
int
x3dio_writenpconvertible(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *p = NULL, *t;

  if(!o)
   return AY_ENULL;

  ay_status = ay_provide_object(o, AY_IDNPATCH, &p);
  if(!p)
    return AY_ERROR;
  t = p;
  while(t->next)
    {
      if(t->type == AY_IDNPATCH)
	{
	  ay_status = x3dio_writeobject(fileptr, t, AY_TRUE, AY_FALSE);
	}

      t = t->next;
    } /* while */

  if(t->type == AY_IDNPATCH)
    {
      ay_status = x3dio_writeobject(fileptr, t, AY_FALSE, AY_FALSE);
    }

  ay_status = ay_object_deletemulti(p);

 return ay_status;
} /* x3dio_writenpconvertible */


/* x3dio_writebox:
 *
 */
int
x3dio_writebox(scew_element *element, ay_object *o)
{
 ay_box_object *box;
 double v[24] = {0}, wh, hh, lh;
 int i;

  if(!o)
   return AY_ENULL;

  box = (ay_box_object *)o->refine;

  wh = (GLdouble)(box->width  * 0.5);
  lh = (GLdouble)(box->length * 0.5);
  hh = (GLdouble)(box->height * 0.5);

  v[0] = -wh;
  v[1] = -hh;
  v[2] = -lh;

  v[3] = -wh;
  v[4] = hh;
  v[5] = -lh;

  v[6] = -wh;
  v[7] = hh;
  v[8] = lh;

  v[9] = -wh;
  v[10] = -hh;
  v[11] = lh;


  v[12] = wh;
  v[13] = -hh;
  v[14] = -lh;

  v[15] = wh;
  v[16] = hh;
  v[17] = -lh;

  v[18] = wh;
  v[19] = hh;
  v[20] = lh;

  v[21] = wh;
  v[22] = -hh;
  v[23] = lh;


  for(i = 0; i < 8; i++)
    {
      ay_trafo_apply3(&(v[i*3]), m);
    }

  /* write all vertices */
  x3dio_writevertices(fileptr, 8, 3, v);

  /* write faces */
  fprintf(fileptr,"f -8 -7 -6 -5\n");
  fprintf(fileptr,"f -4 -3 -2 -1\n");
  fprintf(fileptr,"f -8 -7 -4 -3\n");
  fprintf(fileptr,"f -6 -5 -2 -1\n");
  fprintf(fileptr,"f -8 -6 -4 -2\n");
  fprintf(fileptr,"f -7 -5 -3 -1\n");

 return AY_OK;
} /* x3dio_writebox */


/* x3dio_writepomesh:
 *
 */
int
x3dio_writepomesh(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 /*char fname[] = "x3dio_writepomesh";*/
 ay_object *to = NULL;
 ay_list_object *li = NULL, **nextli = NULL, *lihead = NULL;
 ay_pomesh_object *po;
 double v[3], *p1;
 int stride;
 unsigned int i, j, k, p = 0, q = 0, r = 0;
 int have_mys = AY_FALSE, have_myt = AY_FALSE;
 unsigned int myslen = 0, mytlen = 0, mystlen = 0;
 double *mysarr = NULL, *mytarr = NULL, *mystarr = NULL;
 ay_tag mystag = {NULL, 0, NULL};
 ay_tag myttag = {NULL, 0, NULL};
 ay_tag *tag;

  if(!o)
   return AY_ENULL;

  mystag.type = ay_pv_tagtype;
  mystag.name = x3dio_stagname;
  myttag.type = ay_pv_tagtype;
  myttag.name = x3dio_ttagname;

  po = (ay_pomesh_object *)o->refine;

  if(po->has_normals)
    stride = 6;
  else
    stride = 3;

  /* get all vertices, transform them to world space and write them */
  p1 = po->controlv;
  for(i = 0; i < po->ncontrols; i++)
    {
      AY_APTRAN3(v,p1,m)
      fprintf(fileptr, "v %g %g %g\n", v[0], v[1], v[2]);
      p1 += stride;
    }

  /* write normals */
  if(po->has_normals)
    {
      p1 = &(po->controlv[3]);
      for(i = 0; i < po->ncontrols; i++)
	{
	  fprintf(fileptr, "vn %g %g %g\n", p1[0], p1[1], p1[2]);
	  p1 += 6;
	}
    }

  /* write texture coordinates from potentially present PV tags */
  if(o->tags)
    {
      if(!(mystag.val = calloc(strlen(x3dio_stagname)+2,sizeof(char))))
	return AY_EOMEM;
      if(!(myttag.val = calloc(strlen(x3dio_ttagname)+2,sizeof(char))))
	return AY_EOMEM;
      strcpy(mystag.val, x3dio_stagname);
      mystag.val[strlen(x3dio_stagname)] = ',';
      strcpy(myttag.val, x3dio_ttagname);
      myttag.val[strlen(x3dio_ttagname)] = ',';
      tag = o->tags;
      while(tag)
	{
	  if((tag->type == ay_pv_tagtype) && ay_pv_cmpname(tag, &mystag))
	    {
	      have_mys = AY_TRUE;

	      ay_status = ay_pv_convert(tag, &myslen, (void**)&mysarr);
	    }
	  if((tag->type == ay_pv_tagtype) && ay_pv_cmpname(tag, &myttag))
	    {
	      have_myt = AY_TRUE;

	      ay_status = ay_pv_convert(tag, &mytlen, (void**)&mytarr);
	    }
	  tag = tag->next;
	} /* while */
      free(mystag.val);
      free(myttag.val);
    } /* if */

  /* merge and write the texture vertices */
  if(have_mys)
    mystlen = 2*myslen;
  else
    if(have_myt)
      mystlen = 2*mytlen;

  if(mystlen > 0)
    {
      if(!(mystarr = calloc(mystlen, sizeof(double))))
	{
	  if(v)
	    free(v);
	  if(mysarr)
	    free(mysarr);
	  if(mytarr)
	    free(mytarr);
	  return AY_EOMEM;
	} /* if */

      j = 0;
      for(i = 0; i < mystlen; i++)
	{
	  if(have_mys)
	    mystarr[j]   = mysarr[i];
	  if(have_myt)
	    mystarr[j+1] = mytarr[i];
	  j += 2;
	} /* for */

      x3dio_writetvertices(fileptr, mystlen, 2, mystarr);

      if(mysarr)
	free(mysarr);
      if(mytarr)
	free(mytarr);
      free(mystarr);
      mystarr = NULL;
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
		  /* this is a triangle */
		  fprintf(fileptr, "f");

		  if(po->has_normals)
		    {
		      for(k = 0; k < po->nverts[q]; k++)
			{
			  if(have_mys || have_myt)
			    {
			      fprintf(fileptr, " -%d/-%d/-%d",
				      po->ncontrols-po->verts[r],
				      po->ncontrols-po->verts[r],
				      po->ncontrols-po->verts[r]);
			    }
			  else
			    {
			      fprintf(fileptr, " -%d//-%d",
				      po->ncontrols-po->verts[r],
				      po->ncontrols-po->verts[r]);
			    }
			  r++;
			} /* for */
		    }
		  else
		    {
		      for(k = 0; k < po->nverts[q]; k++)
			{
			  if(have_mys || have_myt)
			    {
			      fprintf(fileptr, " -%d/-%d",
				      po->ncontrols-po->verts[r],
				      po->ncontrols-po->verts[r]);
			    }
			  else
			    {
			      fprintf(fileptr, " -%d",
				      po->ncontrols-po->verts[r]);
			    }
			  r++;
			} /* for */
		    } /* if */

		  fprintf(fileptr, "\n");
		}
	      else
		{
		  /* this is not a triangle => tesselate it */

		  /* create new object (for the tesselated face) */
		  li = NULL;
		  if(!(li = calloc(1, sizeof(ay_list_object))))
		    return AY_EOMEM;
		  to = NULL;
		  if(!(to = calloc(1, sizeof(ay_object))))
		    return AY_EOMEM;
		  li->object = to;

		  ay_object_defaults(to);

		  to->type = AY_IDPOMESH;

		  ay_status = ay_tess_pomeshf(po, i, q, r, AY_FALSE,
					  (ay_pomesh_object **)&(to->refine));

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
	  li = NULL;
	  if(!(li = calloc(1, sizeof(ay_list_object))))
	    return AY_EOMEM;
	  to = NULL;
	  if(!(to = calloc(1, sizeof(ay_object))))
	    return AY_EOMEM;
	  li->object = to;

	  ay_object_defaults(to);

	  to->type = AY_IDPOMESH;

	  ay_status = ay_tess_pomeshf(po, i, q, r, AY_FALSE,
				      (ay_pomesh_object **)&(to->refine));

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
	  x3dio_writepomesh(fileptr, to, m);
	  ay_object_delete(to);
	}
    }
  else
    {
      if(lihead)
	x3dio_writepomesh(fileptr, lihead->object, m);
    } /* if */

  while(lihead)
    {
      ay_object_delete(lihead->object);
      li = lihead->next;
      free(lihead);
      lihead = li;
    } /* while */

 return AY_OK;
} /* x3dio_writepomesh */


/* x3dio_writeclone:
 *
 */
int
x3dio_writeclone(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_clone_object *cl;
 ay_object *clone;

  if(!o)
   return AY_ENULL;

  cl = (ay_clone_object *)o->refine;

  clone = cl->clones;

  while(clone)
    {
      ay_status = x3dio_writeobject(element, clone, AY_FALSE);

      clone = clone->next;
    }

 return ay_status;
} /* x3dio_writeclone */


/* x3dio_writeinstance:
 *
 */
int
x3dio_writeinstance(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *orig, tmp = {0};

  if(!o || !o->refine)
   return AY_ENULL;

  orig = (ay_object *)o->refine;

  ay_trafo_copy(orig, &tmp);
  ay_trafo_copy(o, orig);
  ay_status = x3dio_writeobject(element, orig, AY_FALSE);
  ay_trafo_copy(&tmp, orig);

 return ay_status;
} /* x3dio_writeinstance */


/* x3dio_writescript:
 *
 */
int
x3dio_writescript(scew_element *element, ay_object *o)
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
      while(cmo && cmo->next)
	{
	  ay_status = x3dio_writeobject(element, cmo, AY_FALSE);
	  cmo = cmo->next;
	}
      if(cmo)
	{
	  ay_status = x3dio_writeobject(element, cmo, AY_FALSE);
	}
    } /* if */

 return ay_status;
} /* x3dio_writescript */

#endif /* 0 */


/* x3dio_writetransform:
 *
 */
int
x3dio_writetransform(scew_element *element, ay_object *o, 
		     scew_element **transform_element)
{
 char buffer[256];
 double axis[3] = {0}, angle = 0.0;

  if(!element || !o || !transform_element)
    return AY_ENULL;

  *transform_element = scew_element_add(element, "Transform");

  if((o->movx != 0.0) || (o->movy != 0.0) || (o->movz != 0.0) ||
     (o->rotx != 0.0) || (o->roty != 0.0) || (o->rotz != 0.0) ||
     (o->scalx != 1.0) || (o->scaly != 1.0) || (o->scalz != 1.0) ||
     (o->quat[0] != 0.0) || (o->quat[1] != 0.0) ||
     (o->quat[2] != 0.0) || (o->quat[3] != 1.0))
    {
      /* process translation */
      if((o->movx != 0.0) || (o->movy != 0.0) || (o->movz != 0.0))
	{
	  sprintf(buffer, "%g %g %g", o->movx, o->movy, o->movz);
	  scew_element_add_attr_pair(*transform_element, "translation",
				     buffer);
	}
      /* process scale */
      if((o->scalx != 0.0) || (o->scaly != 0.0) || (o->scalz != 0.0))
	{
	  sprintf(buffer, "%g %g %g", o->scalx, o->scaly, o->scalz);
	  scew_element_add_attr_pair(*transform_element, "scale",
				     buffer);
	}
      /* process rotation */
      if((o->quat[0] != 0.0) || (o->quat[1] != 0.0) ||
	 (o->quat[2] != 0.0) || (o->quat[3] != 1.0))
	{
	  memcpy(axis, o->quat, 3*sizeof(double));
	  AY_V3NORM(axis);
	  angle = 2 * acos(o->quat[3]);

	  sprintf(buffer, "%g %g %g %g", axis[0], axis[1], axis[2], angle);
	  scew_element_add_attr_pair(*transform_element, "rotation",
				     buffer);
	}
    } /* if */

 return AY_OK;
} /* x3dio_writetransform */


/* x3dio_writename:
 *
 */
int
x3dio_writename(scew_element *element, ay_object *o)
{
 char buffer[256];
 static unsigned int count = 0;

  if(!element || !o)
    {
      count = 0;
      return AY_ENULL;
    }

  /* write name as DEF */
  if(o->name && (strlen(o->name)>1))
   {
     scew_element_add_attr_pair(element, "DEF", o->name);
   }
  else
    {
      if(o->refcount)
	{
	  count++;
	  sprintf(buffer, "%u", count);
	  scew_element_add_attr_pair(element, "DEF", buffer);
	}
    }

 return AY_OK;
} /* x3dio_writename */


/* x3dio_writesphere:
 *
 */
int
x3dio_writesphere(scew_element *element, ay_object *o)
{
 int ay_status = AY_OK;
 ay_sphere_object *sphere;
 scew_element *transform_element = NULL;
 scew_element *shape_element = NULL;
 scew_element *sphere_element = NULL;

  if(!element || !o)
    return AY_ENULL;

  /* write transform */
  ay_status = x3dio_writetransform(element, o, &transform_element);

  /* write shape */
  shape_element = scew_element_add(transform_element, "Shape"); 

  /* write name to shape element */
  ay_status = x3dio_writename(shape_element, o);

  /* now write the sphere */
  sphere_element = scew_element_add(shape_element, "Sphere"); 

  /* sphere parameters? */


 return AY_OK;
} /* x3dio_writesphere */


#if 0
/* x3dio_writencurve:
 *
 */
int
x3dio_writencurve(scew_element *element, ay_object *o)
{
 ay_nurbcurve_object *nc;

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

  if(!o)
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
		  ay_error(AY_EWARN, fname,
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
	      ay_object_deletemulti(c);
	      i = -1;
	      break;
	    }
	} /* for */

      if(i == -1)
	{
	  sprintf(err, "Cannot export objects of type: %s.",
		  ay_object_gettypename(o->type));
	  ay_error(AY_EWARN, fname, err);
	}
    } /* if */

 return AY_OK;
} /* x3dio_writeobject */


/* x3dio_writescene:
 *
 */
int
x3dio_writescene(char *filename, int selected)
{
 int ay_status = AY_OK;
 char fname[] = "x3dio_writescene";
 ay_object *o = ay_root->next;
 ay_list_object *sel = NULL;
 scew_tree *tree = NULL;
 scew_element *root = NULL;
 scew_element *scene_element = NULL;
 scew_attribute *attribute = NULL;

  if(selected)
    {
      o = ay_currentlevel->object;
    }

  if(!o)
    return AY_ENULL;

  if(!filename)
    return AY_ENULL;

  /* create in-memory XML tree */
  tree = scew_tree_create();

  scew_tree_set_xml_preamble(tree, "X3D PUBLIC \"ISO//Web3D//DTD X3D 3.0//EN\"   \"http://www.web3d.org/specifications/x3d-3.0.dtd\"");

  root = scew_tree_add_root(tree, "X3D");

  attribute = scew_attribute_create("version", "3.0");
  scew_element_add_attr(root, attribute);

  scene_element = scew_element_add(root, "Scene");

  ay_trafo_identitymatrix(tm);

  if(x3dio_scalefactor != 1.0)
    {
      tm[0]  *= x3dio_scalefactor;
      tm[5]  *= x3dio_scalefactor;
      tm[10] *= x3dio_scalefactor;
    }

  /* count objects to be exported */
  if(!selected)
    {
      x3dio_allobjcnt = x3dio_count(ay_root->next);
    }
  else
    {
      sel = ay_selection;
      while(sel)
	{
	  x3dio_allobjcnt++;
	  if(sel->object->down && sel->object->down->next)
	    x3dio_allobjcnt += x3dio_count(sel->object->down);
	  sel = sel->next;
	}
    } /* if */

  x3dio_curobjcnt = 0;

  /* omit EndLevel-object in top level! */
  while(o->next)
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

      if(ay_status)
	{
	  /* user cancelled export? */
	  if(ay_status == AY_EDONOTLINK)
	    ay_status = AY_OK;
	  break;
	}

      o = o->next;
    } /* while */

  /* write out the in-memory XML tree */
  if(!scew_writer_tree_file(tree, filename))
    {
      ay_error(AY_EOPENFILE, fname, filename);
      ay_status = AY_EOPENFILE;
    }

  /* free the SCEW tree */
  scew_tree_free(tree);

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
 char fname[] = "x3dioWrite";
 int selected = AY_FALSE, i = 2;

  /* check args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "filename");
      return TCL_OK;
    }

  x3dio_tesspomesh = AY_FALSE;
  x3dio_writecurves = AY_TRUE;

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
      if(!strcmp(argv[i], "-p"))
	{
	  sscanf(argv[i+1], "%d", &x3dio_tesspomesh);
	}
      else
      if(!strcmp(argv[i], "-t"))
	{
	  x3dio_stagname = argv[i+1];
	  x3dio_ttagname = argv[i+2];
	  i++;
	}
      else
      if(!strcmp(argv[i], "-f"))
	{
	  sscanf(argv[i+1], "%lg", &x3dio_scalefactor);
	}
      i += 2;
    } /* while */

  ay_status = x3dio_writescene(argv[1], selected);

  x3dio_stagname = x3dio_stagnamedef;
  x3dio_ttagname = x3dio_ttagnamedef;

  x3dio_scalefactor = 1.0;

 return TCL_OK;
} /* x3dio_writetcmd */


/* X_Init:
 *  initialize the x3dio plugin
 *  note: this function _must_ be named and capitalized exactly this way
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
  if(strcmp(ay_version_ma, x3dio_version_ma))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "It is unsafe to continue! Bailing out...");
      return TCL_OK;
    }

  if(strcmp(ay_version_mi, x3dio_version_mi))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "However, it is probably safe to continue...");
    }

#ifndef AYX3DIOWRAPPED
  /* source x3dio.tcl, it contains vital Tcl-code */
  if((Tcl_EvalFile(interp, "x3dio.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname,
		  "Error while sourcing \\\"x3dio.tcl\\\"!");
       return TCL_OK;
     }
#endif /* !AYX3DIOWRAPPED */

  /* create new Tcl commands to interface with the plugin */
  Tcl_CreateCommand(interp, "x3dioRead", x3dio_readtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  
  Tcl_CreateCommand(interp, "x3dioWrite", x3dio_writetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  
  /* init hash table for write callbacks */
  Tcl_InitHashTable(&x3dio_write_ht, TCL_ONE_WORD_KEYS);

  /* fill hash table */
#if 0
  ay_status = x3dio_registerwritecb((char *)(AY_IDNPATCH),
				       x3dio_writenpatch);
  ay_status = x3dio_registerwritecb((char *)(AY_IDNCURVE),
				       x3dio_writencurve);
  ay_status = x3dio_registerwritecb((char *)(AY_IDLEVEL),
				       x3dio_writelevel);
  ay_status = x3dio_registerwritecb((char *)(AY_IDCLONE),
				       x3dio_writeclone);
  ay_status = x3dio_registerwritecb((char *)(AY_IDINSTANCE),
				       x3dio_writeinstance);
  ay_status = x3dio_registerwritecb((char *)(AY_IDSCRIPT),
				       x3dio_writescript);

  ay_status = x3dio_registerwritecb((char *)(AY_IDICURVE),
				       x3dio_writencconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDCONCATNC),
				       x3dio_writencconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDEXTRNC),
				       x3dio_writencconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDNCIRCLE),
				       x3dio_writencconvertible);

  ay_status = x3dio_registerwritecb((char *)(AY_IDEXTRUDE),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDREVOLVE),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDSWEEP),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDSKIN),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDCAP),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDPAMESH),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDBPATCH),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDGORDON),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDBIRAIL1),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDBIRAIL2),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDTEXT),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDBEVEL),
				       x3dio_writenpconvertible);

  ay_status = x3dio_registerwritecb((char *)(AY_IDSPHERE),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDDISK),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDCYLINDER),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDCONE),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDHYPERBOLOID),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDPARABOLOID),
				       x3dio_writenpconvertible);
  ay_status = x3dio_registerwritecb((char *)(AY_IDTORUS),
				       x3dio_writenpconvertible);

  ay_status = x3dio_registerwritecb((char *)(AY_IDPOMESH),
				       x3dio_writepomesh);

  ay_status = x3dio_registerwritecb((char *)(AY_IDBOX),
				       x3dio_writebox);

#endif

  ay_status = x3dio_registerwritecb((char *)(AY_IDSPHERE),
				       x3dio_writesphere);

  ay_error(AY_EOUTPUT, fname, "Plugin 'x3dio' successfully loaded.");

 return TCL_OK;
} /* X_Init */
