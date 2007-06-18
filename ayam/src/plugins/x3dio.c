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

/* global variables: */

char x3dio_version_ma[] = AY_VERSIONSTR;
char x3dio_version_mi[] = AY_VERSIONSTRMI;

static Tcl_HashTable x3dio_write_ht;

static Tcl_HashTable x3dio_defs_ht;


/* current transformation */
x3dio_trafostate *x3dio_ctrafos = NULL;

/* import/export options: */
int x3dio_importcurves = AY_TRUE;
int x3dio_exportcurves = AY_TRUE;
int x3dio_expselected = AY_FALSE;
int x3dio_expobeynoexport = AY_TRUE;
int x3dio_expignorehidden = AY_TRUE;

/* 0: silence, 1: errors, 2: warnings, 3: all */
int x3dio_errorlevel = 1;

/* rescale knots to min dist, if <= 0.0: no scaling */
double x3dio_rescaleknots = 0.0;

/* global scale factor */
double x3dio_scalefactor = 1.0;

char x3dio_stagnamedef[] = "mys";
char *x3dio_stagname = x3dio_stagnamedef;
char x3dio_ttagnamedef[] = "myt";
char *x3dio_ttagname = x3dio_ttagnamedef;

ay_object *x3dio_lrobject = NULL;

/* prototypes of functions local to this module: */
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

int x3dio_linkobject(unsigned int type, void *sobj);

/* 3D */
int x3dio_readbox(scew_element *element);

int x3dio_readsphere(scew_element *element);

int x3dio_readcylinder(scew_element *element);

int x3dio_readcone(scew_element *element);

int x3dio_readindexedfaceset(scew_element *element);

int x3dio_readindexedtriangleset(scew_element *element);

int x3dio_readindexedtrianglestripset(scew_element *element);

int x3dio_readindexedtrianglefanset(scew_element *element);

int x3dio_readindexedlineset(scew_element *element);

int x3dio_readtrianglefanset(scew_element *element);

/* 2D */
int x3dio_readdisk2d(scew_element *element);

int x3dio_readcircle2d(scew_element *element);

int x3dio_readarc2d(scew_element *element);

int x3dio_readarcclose2d(scew_element *element);

int x3dio_readpolyline2d(scew_element *element, int contour);

/* NURBS */
int x3dio_readnurbscurve(scew_element *element, unsigned int dim);

int x3dio_readnurbspatchsurface(scew_element *element, int trimmed);



int x3dio_readtransform(scew_element *element);

int x3dio_readshape(scew_element *element);

int x3dio_readscene(scew_element *element);

int x3dio_adddef(char *name, scew_element *element);

int x3dio_getdef(char *name, scew_element **element);

int x3dio_readelement(scew_element *element);


int x3dio_readtree(scew_tree *tree);

int x3dio_readtcmd(ClientData clientData, Tcl_Interp *interp,
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
 *  get float vector attribute
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
 *  get double vector attribute
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


/* x3dio_linkobject:
 *
 */
int
x3dio_linkobject(unsigned int type, void *sobj)
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

  ay_status = x3dio_linkobject(AY_IDBOX, (void*)&box);

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
  ay_status = x3dio_linkobject(AY_IDSPHERE, (void*)&sphere);

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
      ay_status = x3dio_linkobject(AY_IDCYLINDER, (void*)&cylinder);
    }

  if(has_top && !has_bottom)
    {
      disk.is_simple = AY_TRUE;
      disk.radius = cylinder.radius;
      disk.height = cylinder.zmin;
      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(AY_IDDISK, (void*)&disk);
    }

  if(has_bottom && !has_top)
    {
      disk.is_simple = AY_TRUE;
      disk.radius = cylinder.radius;
      disk.height = cylinder.zmax;
      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(AY_IDDISK, (void*)&disk);
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
      ay_status = x3dio_linkobject(AY_IDCONE, (void*)&cone);
    }

  if(has_bottom && !has_side)
    {
      disk.is_simple = AY_TRUE;
      disk.radius = cone.radius;
      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(AY_IDDISK, (void*)&disk);
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
      ay_status = x3dio_linkobject(AY_IDPOMESH, (void*)&pomesh);

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
      ay_status = x3dio_linkobject(AY_IDPOMESH, (void*)&pomesh);

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
      ay_status = x3dio_linkobject(AY_IDPOMESH, (void*)&pomesh);

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
      ay_status = x3dio_linkobject(AY_IDPOMESH, (void*)&pomesh);

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
 unsigned int i, j, k, l, totalcurves = 0;

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
	  ay_status = x3dio_linkobject(AY_IDNCURVE, (void*)&nc);

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
 unsigned int i, totalverts = 0;

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

      pomesh.npolys = fancountslen;
      /* count vertices */
      for(i = 0; i < fancountslen; i++)
	{
	  if(fancounts[i] >= 3)
	    {
	      totalverts += fancounts[i];
	    }
	  else
	    {
	      pomesh.npolys--;
	    }
	} /* for */

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
	  pomesh.nverts[i] = fancounts[i];;
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
	    }
	}
      else
	{
	  pomesh.controlv = coords;
	  coords = NULL;
	} /* if */

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(AY_IDPOMESH, (void*)&pomesh);

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
      ay_status = x3dio_linkobject(AY_IDDISK, (void*)&disk);
    }
  else
    {
      hyperboloid.p1[0] = iradius;
      hyperboloid.p2[0] = oradius;
      hyperboloid.thetamax = 360.0;

      /* copy object to the Ayam scene */
      ay_status = x3dio_linkobject(AY_IDHYPERBOLOID, (void*)&hyperboloid);
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
  ay_status = x3dio_linkobject(AY_IDNCIRCLE, (void*)&ncircle);

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
  ay_status = x3dio_linkobject(AY_IDNCIRCLE, (void*)&ncircle);

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
 ay_nurbcurve_object cl = {0};
 float radius = 1.0f;
 float sangle = 0.0f;
 float eangle = (float)AY_HALFPI;
 scew_attribute *attr = NULL;
 const XML_Char *str = NULL;
 int ctype = 0, stride = 4;
 double *dummy = NULL;

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
	}
    } /* if */

  /* close arc */
  if(ctype == 0)
    {
      /* PIE */
      cl.length = 3;
      cl.order = 2;

      
    }
  else
    {
      /* CHORD */
      if(!(dummy = realloc(nc.controlv, (nc.length+1)*stride*sizeof(double))))
	return AY_EOMEM;
      nc.controlv = dummy;
      memcpy(&(nc.controlv[nc.length*stride]), nc.controlv,
	     stride*sizeof(double));
      if(!(dummy = realloc(nc.knotv, (nc.length+nc.order+1)*sizeof(double))))
	return AY_EOMEM;
      nc.knotv = dummy;
      nc.knotv[nc.length+nc.order] = nc.knotv[nc.length+nc.order-1] +
	(nc.knotv[nc.length+nc.order-1] - nc.knotv[nc.length-1]);
      nc.length++;
    }

  /* copy object to the Ayam scene */
  ay_status = x3dio_linkobject(AY_IDNCURVE, (void*)&nc);


  if(nc.knotv)
    free(nc.knotv);

  if(nc.controlv)
    free(nc.controlv);

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
      ay_status = x3dio_linkobject(AY_IDNCURVE, (void*)&nc);
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
	    }
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
      ay_status = x3dio_linkobject(AY_IDNCURVE, (void*)&nc);
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
  if(uklen >= width+uorder)
    {
      has_uknots = AY_TRUE;
    }
  ay_status = x3dio_readdoublepoints(element, "vKnot", 1, &vklen, &vknots);
  if(uklen >= width+uorder)
    {
      has_uknots = AY_TRUE;
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
	    } /* if */
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
      ay_status = x3dio_linkobject(AY_IDNPATCH, (void*)&np);

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
	    }
	}
    } /* while */

  ay_object_crtendlevel(ay_next);
  ay_next = old_aynext;
  ay_object_link(o);

  ay_status = x3dio_readshape(element);

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
    }

  /* how many children have been read? */
  if(o->down && o->down->next)
    {
      /* read more than one geometric element */
      /* => keep level */
      ay_object_crtendlevel(ay_next);
      ay_next = old_aynext;
      ay_object_link(o);
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
    }

 return ay_status;
} /* x3dio_readscene */


/* x3dio_adddef:
 *  add a definition with name <name> for element <element>
 *  (process DEF)
 */
int
x3dio_adddef(char *name, scew_element *element)
{
 Tcl_HashEntry *entry = NULL;
 int new_item = 0;

  if(!name || !element)
    return AY_ENULL;

  if((entry = Tcl_FindHashEntry(&x3dio_defs_ht, name)))
    {
      return AY_ERROR; /* name already registered */
    }
  else
    {
      entry = Tcl_CreateHashEntry(&x3dio_defs_ht, name, &new_item);
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
 *  (process USE)
 */
int
x3dio_getdef(char *name, scew_element **element)
{
 Tcl_HashEntry *entry = NULL;

  if(!name || !element)
    return AY_ENULL;

  if((entry = Tcl_FindHashEntry(&x3dio_defs_ht, name)))
    {
      *element = Tcl_GetHashValue(entry);
    }
  else
    {
      return AY_ERROR; /* name not registered */
    }

 return AY_OK;
} /* x3dio_getdef */


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

  if(!element)
    return AY_ENULL;

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
	}
      else
	{
	  ay_error(AY_ERROR, fname, "malformed USE attribute encountered");
	} /* if */
    } /* if */

  element_name = scew_element_name(element);

  /* scene organization */
  if(!strcmp(element_name, "Scene"))
    {
      ay_status = x3dio_readscene(element);
    }
  if(!strcmp(element_name, "Transform"))
    {
      ay_status = x3dio_readtransform(element);
    }
  if(!strcmp(element_name, "Shape"))
    {
      ay_status = x3dio_readshape(element);
    }
  if(!strcmp(element_name, "Appearance"))
    {
      ay_status = x3dio_readappearance(element);
    }
  if(!strcmp(element_name, "Material"))
    {
      ay_status = x3dio_readmaterial(element);
    }

  /* geometric shapes */
  /* 3D */
  if(!strcmp(element_name, "Box"))
    {
      ay_status = x3dio_readbox(element);
    }
  if(!strcmp(element_name, "Sphere"))
    {
      ay_status = x3dio_readsphere(element);
    }
  if(!strcmp(element_name, "Cylinder"))
    {
      ay_status = x3dio_readcylinder(element);
    }
  if(!strcmp(element_name, "Cone"))
    {
      ay_status = x3dio_readcone(element);
    }
  if(!strcmp(element_name, "IndexedFaceSet"))
    {
      ay_status = x3dio_readindexedfaceset(element);
    }
  if(!strcmp(element_name, "IndexedTriangleSet"))
    {
      ay_status = x3dio_readindexedtriangleset(element);
    }
  if(!strcmp(element_name, "IndexedTriangleStripSet"))
    {
      ay_status = x3dio_readindexedtrianglestripset(element);
    }
  if(!strcmp(element_name, "IndexedTriangleFanSet"))
    {
      ay_status = x3dio_readindexedtrianglefanset(element);
    }
  if(!strcmp(element_name, "IndexedLineSet"))
    {
      ay_status = x3dio_readindexedlineset(element);
    }
  if(!strcmp(element_name, "TriangleFanSet"))
    {
      ay_status = x3dio_readtrianglefanset(element);
    }
  /* 2D */
  if(!strcmp(element_name, "Disk2D"))
    {
      ay_status = x3dio_readdisk2d(element);
    }
  if(!strcmp(element_name, "Circle2D"))
    {
      ay_status = x3dio_readcircle2d(element);
    }
  if(!strcmp(element_name, "Arc2D"))
    {
      ay_status = x3dio_readarc2d(element);
    }
  if(!strcmp(element_name, "ArcClose2D"))
    {
      ay_status = x3dio_readarcclose2d(element);
    }
  if(!strcmp(element_name, "Polyline2D"))
    {
      ay_status = x3dio_readpolyline2d(element, AY_FALSE);
    }
  /* NURBS */
  if(!strcmp(element_name, "NurbsCurve"))
    {
      ay_status = x3dio_readnurbscurve(element, 3);
    }
  if(!strcmp(element_name, "NurbsCurve2D"))
    {
      ay_status = x3dio_readnurbscurve(element, 2);
    }
  if(!strcmp(element_name, "NurbsPatchSurface"))
    {
      ay_status = x3dio_readnurbspatchsurface(element, AY_FALSE);
    }
  if(!strcmp(element_name, "NurbsTrimmedSurface"))
    {
      ay_status = x3dio_readnurbspatchsurface(element, AY_TRUE);
    }
  if(!strcmp(element_name, "ContourPolyline2D"))
    {
      ay_status = x3dio_readpolyline2d(element, AY_TRUE);
    }
  if(!strcmp(element_name, "Contour2D"))
    {
      ay_status = x3dio_readshape(element);
    }
  if(!strcmp(element_name, "NurbsSweptSurface"))
    {
      ay_status = x3dio_readnurbssweptsurface(element, AY_FALSE);
    }
  if(!strcmp(element_name, "NurbsSwungSurface"))
    {
      ay_status = x3dio_readnurbssweptsurface(element, AY_TRUE);
    }

  /* non geometric shapes */
  if(!strcmp(element_name, "Group"))
    {
      ay_status = x3dio_readshape(element);
    }


  /* light sources */
  if(!strcmp(element_name, "DirectionalLight"))
    {

    }
  if(!strcmp(element_name, "SpotLight"))
    {

    }
  if(!strcmp(element_name, "PointLight"))
    {

    }

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
 char fname[] = "x3dio_read";
 char errstr[256];
 int i = 2;
 double accuracy = 0.1;
 char arrname[] = "x3dio_options", varname[] = "Progress";
 scew_tree *tree = NULL;
 scew_parser *parser = NULL;
 scew_error errcode;
 enum XML_Error expat_code;

  /* set default import options */
  x3dio_importcurves = AY_TRUE;
  x3dio_rescaleknots = 0.0;
  x3dio_scalefactor = 1.0;

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
      if(!strcmp(argv[i], "-t"))
	{
	  x3dio_stagname = argv[i+1];
	  x3dio_ttagname = argv[i+2];
	  i++;
	}
      i += 2;
    } /* while */

  /* initialize hashtable for DEFs */
  Tcl_InitHashTable(&x3dio_defs_ht, TCL_STRING_KEYS);

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

  /* set progress */
  Tcl_SetVar2(ay_interp, arrname, varname, "50",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  /* convert XML tree to Ayam objects */
  ay_status = x3dio_readtree(tree);

  /* set progress */
  Tcl_SetVar2(ay_interp, arrname, varname, "100",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  /* clean up */
  x3dio_cleartrafo();

  scew_tree_free(tree);

  scew_parser_free(parser);

  Tcl_DeleteHashTable(&x3dio_defs_ht);

  x3dio_stagname = x3dio_stagnamedef;
  x3dio_ttagname = x3dio_ttagnamedef;

 return TCL_OK;
} /* x3dio_readtcmd */


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
 char fname[] = "x3dio_init";
 /* int ay_status = AY_OK; */

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
  /*
  Tcl_CreateCommand(interp, "x3dioWrite", x3dio_writetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  */
  /* init hash table for write callbacks */
  Tcl_InitHashTable(&x3dio_write_ht, TCL_ONE_WORD_KEYS);

  /* fill hash table */


  ay_error(AY_EOUTPUT, fname, "Plugin 'x3dio' successfully loaded.");

 return TCL_OK;
} /* X_Init */
