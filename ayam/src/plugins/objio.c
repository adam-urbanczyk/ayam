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

/* \file objio.c Wavefront OBJ Input/Output plugin */

#include "ayam.h"
#include <ctype.h>


/* types local to this module */

typedef int (objio_writecb) (FILE *fileptr, ay_object *o, double *m);


/* prototypes of functions local to this module */

unsigned int objio_count(ay_object *o);

int objio_registerwritecb(char *name, objio_writecb *cb);

int objio_writevertices(FILE *fileptr, unsigned int n, int stride,
			   double *v);

int objio_writetvertices(FILE *fileptr, unsigned int n, int stride,
			    double *v);

int objio_writencurve(FILE *fileptr, ay_object *o, double *m);

int objio_writetcurve(FILE *fileptr, ay_object *o);

int objio_writetrims(FILE *fileptr, ay_object *o);

int objio_writetrimids(FILE *fileptr, ay_object *o);

int objio_writenpatch(FILE *fileptr, ay_object *o, double *m);

int objio_writelevel(FILE *fileptr, ay_object *o, double *m);

int objio_writencconvertible(FILE *fileptr, ay_object *o, double *m);

int objio_writenpconvertible(FILE *fileptr, ay_object *o, double *m);

int objio_writebox(FILE *fileptr, ay_object *o, double *m);

int objio_writepomesh(FILE *fileptr, ay_object *o, double *m);

int objio_writeclone(FILE *fileptr, ay_object *o, double *m);

int objio_writeinstance(FILE *fileptr, ay_object *o, double *m);

int objio_writescript(FILE *fileptr, ay_object *o, double *m);

int objio_writeobject(FILE *fileptr, ay_object *o, int writeend, int count);

int objio_writescene(char *filename, int selected);

int objio_writescenetcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[]);

#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int Objio_Init(Tcl_Interp *interp);


/* global variables */

static char objio_version_ma[] = AY_VERSIONSTR;
static char objio_version_mi[] = AY_VERSIONSTRMI;

static double tm[16] = {0}; /* current transformation matrix */

static double objio_scalefactor = 1.0;

static Tcl_HashTable objio_write_ht; /* write callbacks */

static int objio_tesspomesh = AY_FALSE;

static int objio_writecurves = AY_TRUE;

static int objio_writedmw = AY_FALSE; /** < de-multiply weights? */

static unsigned int objio_allobjcnt = 0;
static unsigned int objio_curobjcnt = 0;


/* functions */

/* objio_count:
 * _recursively_ count the objects to be exported
 *
 */
unsigned int
objio_count(ay_object *o)
{
 unsigned int lcount = 0;
 int lasttype = -1;
 Tcl_HashTable *ht = &objio_write_ht;
 Tcl_HashEntry *entry = NULL;
 objio_writecb *cb = NULL;

  if(!o)
    return 0;

  while(o->next)
    {
      if(lasttype != (int)o->type)
	{
	  entry = NULL;
	  if((entry = Tcl_FindHashEntry(ht, (char *)(o->type))))
	    {
	      cb = (objio_writecb*)Tcl_GetHashValue(entry);
	    }
	  else
	    {
	      cb = NULL;
	    }
	  lasttype = o->type;
	} /* if */

      if(o->down && o->down->next && (cb != objio_writenpconvertible))
	lcount += objio_count(o->down);

      if(cb != NULL)
	lcount++;

      o = o->next;
    } /* while */

 return lcount;
} /* objio_count */


/* objio_registerwritecb:
 *
 */
int
objio_registerwritecb(char *name, objio_writecb *cb)
{
 int ay_status = AY_OK;
 int new_item = 0;
 Tcl_HashEntry *entry = NULL;
 Tcl_HashTable *ht = &objio_write_ht;

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
} /* objio_registerwritecb */


/* objio_writevertices:
 *  write <n> <stride>D geometric vertices from array <v[n*stride]> to
 *  file <fileptr>
 */
int
objio_writevertices(FILE *fileptr, unsigned int n, int stride, double *v)
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
} /* objio_writevertices */


/* objio_writetvertices:
 *  write <n> <stride>D texture vertices from array <v[n*stride]>
 *  to file <fileptr>
 */
int
objio_writetvertices(FILE *fileptr, unsigned int n, int stride, double *v)
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
} /* objio_writetvertices */


/* objio_writencurve:
 *
 */
int
objio_writencurve(FILE *fileptr, ay_object *o, double *m)
{
 ay_nurbcurve_object *nc;
 double *v = NULL, *p1, *p2, pw[3], umin, umax;
 int stride = 4, i;

  if(!objio_writecurves)
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
	  pw[0] = p2[0];
	  pw[1] = p2[1];
	  pw[2] = p2[2];
	  AY_APTRAN3(p1,pw,m)
	  p1[3] = p2[3];
	  p1 += stride;
	}
      else
	{
	  AY_APTRAN3(p1,p2,m)
	  p1 += 3;
	} /* if */
      p2 += stride;
    } /* for */

  /* write all vertices */
  objio_writevertices(fileptr, (unsigned int)nc->length,
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
} /* objio_writencurve */


/* objio_writetcurve:
 *  write a single trim curve
 */
int
objio_writetcurve(FILE *fileptr, ay_object *o)
{
 ay_nurbcurve_object *nc;
 double v[3] = {0}, *p1, pw[3] = {0}, m[16] = {0};
 int stride = 4, i;

  if(!o)
    return AY_ENULL;

  nc = (ay_nurbcurve_object *)o->refine;

  /* create proper transformation matrix */
  ay_trafo_creatematrix(o, m);

  /* get all vertices, transform them and write them out */
  p1 = nc->controlv;
  for(i = 0; i < nc->length; i++)
    {
      if(nc->is_rat)
	{
	  pw[0] = p1[0];
	  pw[1] = p1[1];
	  AY_APTRAN3(v,pw,m)
	  v[2] = p1[3];
	  fprintf(fileptr, "vp %g %g %g\n", v[0], v[1], v[2]);
	}
      else
	{
	  AY_APTRAN3(v,p1,m)
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
} /* objio_writetcurve */


/* objio_writetrims:
 *  write all trim curves pointed to by <o>
 */
int
objio_writetrims(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *down = NULL, *pnc = NULL, *p;

  while(o->next)
    {
      switch(o->type)
	{
	case AY_IDNCURVE:
	  objio_writetcurve(fileptr, o);
	  break;
	case AY_IDLEVEL:
	  if((o->down) && (o->down->next))
	    {
	      down = o->down;
	      while(down->next)
		{
		  if(down->type == AY_IDNCURVE)
		    {
		      objio_writetcurve(fileptr, down);
		    }
		  else
		    {
		      p = NULL;
		      ay_status = ay_provide_object(o, AY_IDNCURVE, &p);
		      pnc = p;
		      while(pnc)
			{
			  objio_writetcurve(fileptr, pnc);
			  pnc = pnc->next;
			}
		      (void)ay_object_deletemulti(p, AY_FALSE);
		    }
		  down = down->next;
		} /* while */
	    } /* if */
	  break;
	default:
	  p = NULL;
	  ay_status = ay_provide_object(o, AY_IDNCURVE, &p);
	  pnc = p;
	  while(pnc)
	    {
	      objio_writetcurve(fileptr, pnc);
	      pnc = pnc->next;
	    }
	  (void)ay_object_deletemulti(p, AY_FALSE);
	  break;
	} /* switch */

      o = o->next;
    } /* while */

 return AY_OK;
} /* objio_writetrims */


/* objio_writetrimids:
 *  write ids of all trim curves pointed to by <o>
 */
int
objio_writetrimids(FILE *fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *o2 = o, *down = NULL, *pnc = NULL, *cnc = NULL, *p = NULL;
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
		  else
		    {
		      p = NULL;
		      ay_status = ay_provide_object(down, AY_IDNCURVE, &p);
		      pnc = p;
		      while(pnc)
			{
			  tc++;
			  pnc = pnc->next;
			}
		      (void)ay_object_deletemulti(p, AY_FALSE);
		    }
		  down = down->next;
		} /* while */
	    } /* if */
	default:
	  p = NULL;
	  ay_status = ay_provide_object(o, AY_IDNCURVE, &p);
	  pnc = p;
	  while(pnc)
	    {
	      tc++;
	      pnc = pnc->next;
	    }
	  (void)ay_object_deletemulti(p, AY_FALSE);
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
	  ay_status = ay_nct_getorientation(nc, 4, 1, 0, &orient);
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
	      ay_nct_concatobjs(down, &cnc);

	      if(cnc)
		{
		  nc = (ay_nurbcurve_object *)cnc->refine;
		  ay_status = ay_nct_getorientation(nc, 4, 1, 0, &orient);
		  ay_object_delete(cnc);
		  cnc = NULL;

		  if(orient < 0.0)
		    fprintf(fileptr, "hole ");
		  else
		    fprintf(fileptr, "trim ");

		  while(down->next)
		    {
		      if(down->type == AY_IDNCURVE)
			{
			  nc = (ay_nurbcurve_object *)down->refine;
			  ay_knots_getuminmax(down, nc->order,
					      nc->length+nc->order, nc->knotv,
					      &umin, &umax);
			  fprintf(fileptr, " %g %g -%d", umin, umax, tc);
			  tc--;
			}
		      else
			{
			  p = NULL;
			  ay_status = ay_provide_object(down, AY_IDNCURVE, &p);
			  pnc = p;
			  while(pnc)
			    {
			      nc = (ay_nurbcurve_object *)pnc->refine;
			      ay_knots_getuminmax(pnc, nc->order,
						  nc->length+nc->order,
						  nc->knotv,
						  &umin, &umax);
			      fprintf(fileptr, " %g %g -%d", umin, umax, tc);
			      tc--;
			      pnc = pnc->next;
			    }
			  (void)ay_object_deletemulti(p, AY_FALSE);
			}
		      down = down->next;
		    } /* while */

		  fprintf(fileptr, "\n");
		} /* if */
	    } /* if */
	  break;
	default:
	  p = NULL;
	  ay_status = ay_provide_object(o, AY_IDNCURVE, &p);
	  pnc = p;

	  while(pnc)
	    {
	      nc = (ay_nurbcurve_object *)pnc->refine;
	      ay_status = ay_nct_getorientation(nc, 4, 0, 0, &orient);

	      if(orient < 0.0)
		fprintf(fileptr, "hole ");
	      else
		fprintf(fileptr, "trim ");

	      ay_knots_getuminmax(pnc, nc->order, nc->length+nc->order,
				  nc->knotv,
				  &umin, &umax);

	      fprintf(fileptr, " %g %g -%d\n", umin, umax, tc);
	      tc--;

	      pnc = pnc->next;
	    } /* while */

	  (void)ay_object_deletemulti(p, AY_FALSE);
	  break;
	} /* switch */
      o = o->next;
    } /* while */

 return AY_OK;
} /* objio_writetrimids */


/* objio_writenpatch:
 *
 */
int
objio_writenpatch(FILE *fileptr, ay_object *o, double *m)
{
 int ay_status = AY_OK;
 char *tcname = ay_prefs.texcoordname;
 ay_object *c;
 ay_nurbpatch_object *np;
 double *v = NULL, *p1, *p2, pw[3];
 double umin, umax, vmin, vmax;
 int stride = 4, i, j;
 int have_texcoords = AY_FALSE;
 unsigned int mystlen = 0;
 double *mystarr = NULL;
 ay_tag *tag;

  if(!o)
    return AY_ENULL;

  /* first, check for and write the trim curves */
  if(o->down && o->down->next)
    {
      ay_status = objio_writetrims(fileptr, o->down);
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
	      pw[0] = p2[0];
	      pw[1] = p2[1];
	      pw[2] = p2[2];
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
  objio_writevertices(fileptr, (unsigned int)(np->width * np->height),
			 (np->is_rat?4:3), v);

  /* get texture coordinates from potentially present PV tags */
  if(o->tags)
    {
      tag = o->tags;
      while(tag)
	{
	  if(ay_pv_checkndt(tag, tcname, "varying", "g"))
	    {
	      have_texcoords = AY_TRUE;

	      ay_status = ay_pv_convert(tag, 0,
					&mystlen, (void**)(void*)&mystarr);
	      if(ay_status)
		goto cleanup;
	      break;
	    }

	  tag = tag->next;
	} /* while */
    } /* if */

  /* write the texture vertices */
  if(have_texcoords)
    {
      objio_writetvertices(fileptr, mystlen, 2, mystarr);

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
      if(have_texcoords)
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
      objio_writetrimids(fileptr, o->down);
    } /* if */

  /* write the caps and bevels */
  c = np->caps_and_bevels;
  while(c)
    {
      objio_writenpatch(fileptr, c, m);
      c = c->next;
    }

cleanup:

  free(v);

 return ay_status;
} /* objio_writenpatch */


/* objio_writelevel:
 *
 */
int
objio_writelevel(FILE *fileptr, ay_object *o, double *m)
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
	  ay_status = objio_writeobject(fileptr, down, AY_TRUE, AY_TRUE);
	  down = down->next;
	}

      if(down)
	{
	  ay_status = objio_writeobject(fileptr, down, AY_FALSE, AY_TRUE);
	}

      memcpy(tm, m1, 16*sizeof(double));
    } /* if */

 return AY_OK;
} /* objio_writelevel */


/* objio_writencconvertible:
 *
 */
int
objio_writencconvertible(FILE *fileptr, ay_object *o, double *m)
{
 int ay_status = AY_OK;
 ay_object *c = NULL, *t;

  if(!objio_writecurves)
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
	  ay_status = objio_writeobject(fileptr, t, AY_TRUE, AY_FALSE);
	}

      t = t->next;
    } /* while */

  if(t->type == AY_IDNCURVE)
    {
      ay_status = objio_writeobject(fileptr, t, AY_FALSE, AY_FALSE);
    }

  (void)ay_object_deletemulti(c, AY_FALSE);

 return ay_status;
} /* objio_writencconvertible */


/* objio_writenpconvertible:
 *
 */
int
objio_writenpconvertible(FILE *fileptr, ay_object *o, double *m)
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
	  ay_status = objio_writeobject(fileptr, t, AY_TRUE, AY_FALSE);
	}

      t = t->next;
    } /* while */

  if(t->type == AY_IDNPATCH)
    {
      ay_status = objio_writeobject(fileptr, t, AY_FALSE, AY_FALSE);
    }

  (void)ay_object_deletemulti(p, AY_FALSE);

 return ay_status;
} /* objio_writenpconvertible */


/* objio_writebox:
 *
 */
int
objio_writebox(FILE *fileptr, ay_object *o, double *m)
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
  objio_writevertices(fileptr, 8, 3, v);

  /* write faces */
  fprintf(fileptr,"f -8 -7 -6 -5\n");
  fprintf(fileptr,"f -4 -3 -2 -1\n");
  fprintf(fileptr,"f -8 -7 -4 -3\n");
  fprintf(fileptr,"f -6 -5 -2 -1\n");
  fprintf(fileptr,"f -8 -6 -4 -2\n");
  fprintf(fileptr,"f -7 -5 -3 -1\n");

 return AY_OK;
} /* objio_writebox */


/* objio_writepomesh:
 *
 */
int
objio_writepomesh(FILE *fileptr, ay_object *o, double *m)
{
 int ay_status = AY_OK;
 /*char fname[] = "objio_writepomesh";*/
 char *tcname = ay_prefs.texcoordname;
 ay_object *to = NULL;
 ay_list_object *li = NULL, **nextli = NULL, *lihead = NULL;
 ay_pomesh_object *po;
 double v[3], *p1;
 int stride;
 unsigned int i, j, k, p = 0, q = 0, r = 0;
 int have_texcoords = AY_FALSE;
 unsigned int mystlen = 0;
 double *mystarr = NULL;
 ay_tag *tag;

  if(!o)
   return AY_ENULL;

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

  /* get texture coordinates from potentially present PV tags */
  if(o->tags)
    {
      tag = o->tags;
      while(tag)
	{
	  if(ay_pv_checkndt(tag, tcname, "varying", "g"))
	    {
	      have_texcoords = AY_TRUE;

	      ay_status = ay_pv_convert(tag, 0,
					&mystlen, (void**)(void*)&mystarr);
	      if(ay_status)
		return AY_ERROR;
	      break;
	    }
	  tag = tag->next;
	} /* while */
    } /* if */

  /* write the texture vertices */
  if(have_texcoords)
    {
      objio_writetvertices(fileptr, mystlen, 2, mystarr);

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
	      if(!objio_tesspomesh ||
		 (objio_tesspomesh && (po->nverts[q] == 3)))
		{
		  /* this is a triangle */
		  fprintf(fileptr, "f");

		  if(po->has_normals)
		    {
		      for(k = 0; k < po->nverts[q]; k++)
			{
			  if(have_texcoords)
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
			  if(have_texcoords)
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
		  if(!(li = calloc(1, sizeof(ay_list_object))))
		    {ay_status = AY_EOMEM; goto cleanup;}

		  if(!(to = calloc(1, sizeof(ay_object))))
		    {free(li); ay_status = AY_EOMEM; goto cleanup;}

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
	    {ay_status = AY_EOMEM; goto cleanup;}

	  if(!(to = calloc(1, sizeof(ay_object))))
	    {free(li); ay_status = AY_EOMEM; goto cleanup;}
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
	  objio_writepomesh(fileptr, to, m);
	  ay_object_delete(to);
	}
    }
  else
    {
      if(lihead)
	objio_writepomesh(fileptr, lihead->object, m);
    } /* if */

cleanup:

  while(lihead)
    {
      ay_object_delete(lihead->object);
      li = lihead->next;
      free(lihead);
      lihead = li;
    } /* while */

 return ay_status;
} /* objio_writepomesh */


/* objio_writeclone:
 *
 */
int
objio_writeclone(FILE *fileptr, ay_object *o, double *m)
{
 int ay_status = AY_OK;
 ay_clone_object *cl;
 ay_object *clone;

  if(!o)
   return AY_ENULL;

  cl = (ay_clone_object *)o->refine;

  clone = cl->clones;

  if(!clone)
    return AY_OK;

  if(o->type == AY_IDMIRROR)
    {
      clone = o->down;
      while(clone && clone->next)
	{
	  ay_status = objio_writeobject(fileptr, clone, AY_TRUE, AY_TRUE);
	  clone = clone->next;
	}
      clone = cl->clones;
    }

  while(clone && clone->next)
    {
      ay_status = objio_writeobject(fileptr, clone, AY_TRUE, AY_FALSE);

      clone = clone->next;
    }

  if(clone)
    {
      ay_status = objio_writeobject(fileptr, clone, AY_FALSE, AY_FALSE);
    }

 return ay_status;
} /* objio_writeclone */


/* objio_writeinstance:
 *
 */
int
objio_writeinstance(FILE *fileptr, ay_object *o, double *m)
{
 int ay_status = AY_OK;
 ay_object *orig, tmp = {0};

  if(!o || !o->refine)
   return AY_ENULL;

  orig = (ay_object *)o->refine;

  ay_trafo_copy(orig, &tmp);
  ay_trafo_copy(o, orig);
  ay_status = objio_writeobject(fileptr, orig, AY_FALSE, AY_FALSE);
  ay_trafo_copy(&tmp, orig);

 return ay_status;
} /* objio_writeinstance */


/* objio_writescript:
 *
 */
int
objio_writescript(FILE *fileptr, ay_object *o, double *m)
{
 int ay_status = AY_OK;
 ay_object *cmo = NULL;
 ay_script_object *sc = NULL;

  if(!o || !o->refine)
   return AY_ENULL;

  sc = (ay_script_object *)o->refine;

  if(((sc->type == 1) || (sc->type == 2)) && (sc->cm_objects))
    {
      cmo = sc->cm_objects;
      while(cmo && cmo->next)
	{
	  ay_status = objio_writeobject(fileptr, cmo, AY_TRUE, AY_FALSE);
	  cmo = cmo->next;
	}
      if(cmo)
	{
	  ay_status = objio_writeobject(fileptr, cmo, AY_FALSE, AY_FALSE);
	}
    } /* if */

 return ay_status;
} /* objio_writescript */


#if 0
/* objio_writencurve:
 *
 */
int
objio_writencurve(FILE *fileptr, ay_object *o, double *m)
{
 ay_nurbcurve_object *nc;
 return AY_OK;
} /* objio_writencurve */
#endif


/* objio_writeobject:
 *
 */
int
objio_writeobject(FILE *fileptr, ay_object *o, int writeend, int count)
{
 int ay_status = AY_OK;
 char fname[] = "objio_writeobject";
 Tcl_HashTable *ht = &objio_write_ht;
 Tcl_HashEntry *entry = NULL;
 double m1[16] = {0}, m2[16];
 char err[255];
 objio_writecb *cb = NULL;
 ay_object *t, *c = NULL;
 int curprog = 0;
 char aname[] = "objio_options", vname1[] = "Progress";
 char vname2[] = "Cancel", *val = NULL;
 char pbuffer[64];
 int i, numconvs = 3, conversions[3] = {AY_IDNPATCH, AY_IDNCURVE, AY_IDPOMESH};

  if(!o)
    return AY_ENULL;

  if((entry = Tcl_FindHashEntry(ht, (char *)(o->type))))
    {
      cb = (objio_writecb*)Tcl_GetHashValue(entry);

      if(cb)
	{
	  if(o->name && (strlen(o->name)>1))
	    fprintf(fileptr, "o %s\n", o->name);

	  if(AY_ISTRAFO(o))
	    {
	      ay_trafo_creatematrix(o, m1);
	      memcpy(m2, tm, 16*sizeof(double));
	      ay_trafo_multmatrix(m2, m1);

	      ay_status = cb(fileptr, o, m2);
	    }
	  else
	    {
	      ay_status = cb(fileptr, o, tm);
	    }

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Error exporting object.");
	      ay_status = AY_OK;
	    }

	  if(count)
	    {
	      objio_curobjcnt++;

	      /* calculate new progress value in percent */
	      curprog = (int)(objio_curobjcnt*100.0/objio_allobjcnt);

	      sprintf(pbuffer, "%d", curprog);
	      Tcl_SetVar2(ay_interp, aname, vname1, pbuffer,
			  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

	      /* process all events (update the GUI) for every object */
	      /*
		if(!fmod(objio_curobjcnt, 5.0))
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

	  if(writeend)
	    fprintf(fileptr, "end\n");
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
	      ay_status = objio_writeobject(fileptr, t, AY_TRUE, AY_FALSE);
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
} /* objio_writeobject */


/* objio_writescene:
 *
 */
int
objio_writescene(char *filename, int selected)
{
 int ay_status = AY_OK;
 ay_object *o = ay_root->next;
 ay_list_object *sel = NULL;
 FILE *fileptr = NULL;
 char fname[] = "objio_writescene";

  if(selected)
    {
      o = ay_currentlevel->object;
    }

  if(!o)
    return AY_ENULL;

  if(!filename)
    return AY_ENULL;

  if(!(fileptr = fopen(filename, "wb")))
    {
      ay_error(AY_EOPENFILE, fname, filename);
      return AY_ERROR;
    }

  clearerr(fileptr);

  ay_trafo_identitymatrix(tm);

  if(objio_scalefactor != 1.0)
    {
      tm[0]  *= objio_scalefactor;
      tm[5]  *= objio_scalefactor;
      tm[10] *= objio_scalefactor;
    }

  /* count objects to be exported */
  if(!selected)
    {
      objio_allobjcnt = objio_count(ay_root->next);
    }
  else
    {
      sel = ay_selection;
      while(sel)
	{
	  objio_allobjcnt++;
	  if(sel->object->down && sel->object->down->next)
	    objio_allobjcnt += objio_count(sel->object->down);
	  sel = sel->next;
	}
    } /* if */
  objio_curobjcnt = 0;

  /* write header information */
  /*  objio_writeheader(fileptr);*/

  /* omit EndLevel-object in top level! */
  while(o->next)
    {
      if(selected)
	{
	  if(o->selected)
	    {
	      ay_status = objio_writeobject(fileptr, o, AY_TRUE, AY_TRUE);
	    }
	}
      else
	{
	  ay_status = objio_writeobject(fileptr, o, AY_TRUE, AY_TRUE);
	}

      if(ay_status)
	{
	  /* user cancelled export? */
	  if(ay_status == AY_EDONOTLINK)
	    ay_status = AY_OK;
	  fclose(fileptr);
	  return ay_status;
	}

      o = o->next;
    } /* while */

  if(ferror(fileptr))
    {
      ay_error(AY_ERROR, fname, strerror(errno));
    }

  if(fclose(fileptr))
    {
      ay_error(AY_ERROR, fname, strerror(errno));
    }

 return ay_status;
} /* objio_writescene */


/* objio_writescenetcmd:
 *
 */
int
objio_writescenetcmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[])
{
 int ay_status = AY_OK;
 int selected = AY_FALSE, i = 2;

  /* check args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "filename");
      return TCL_OK;
    }

  objio_tesspomesh = AY_FALSE;
  objio_writecurves = AY_TRUE;

  while(i+1 < argc)
    {
      if(!strcmp(argv[i], "-c"))
	{
	  sscanf(argv[i+1], "%d", &objio_writecurves);
	}
      else
      if(!strcmp(argv[i], "-s"))
	{
	  sscanf(argv[i+1], "%d", &selected);
	}
      else
      if(!strcmp(argv[i], "-p"))
	{
	  sscanf(argv[i+1], "%d", &objio_tesspomesh);
	}
      else
      if(!strcmp(argv[i], "-f"))
	{
	  sscanf(argv[i+1], "%lg", &objio_scalefactor);
	}
      i += 2;
    } /* while */

  ay_status = objio_writescene(argv[1], selected);

  objio_scalefactor = 1.0;

 return TCL_OK;
} /* objio_writescenetcmd */


/****************************************************************/

static int objio_readcurves;

static int objio_readstrim;

static int objio_mergecfaces;

static int objio_mergepvtags;

static double objio_rescaleknots;

static int objio_checkdegen;

typedef struct objio_vertex_s {
  struct objio_vertex_s *next;
  struct objio_vertex_s *prev;

  unsigned int index;

  double v[4];

} objio_vertex;

static objio_vertex *objio_gverts_head = NULL;
static objio_vertex *objio_gverts_tail = NULL;

static objio_vertex *objio_nverts_head = NULL;
static objio_vertex *objio_nverts_tail = NULL;

static objio_vertex *objio_pverts_head = NULL;
static objio_vertex *objio_pverts_tail = NULL;

static objio_vertex *objio_tverts_head = NULL;
static objio_vertex *objio_tverts_tail = NULL;

static int objio_cstype; /* -1 - unset, 0 - bmatrix, 1 - bezier, 2 - bspline,
			    3 - cardinal, 4 - taylor */
static int objio_csrat;
static int objio_degu;
static int objio_degv;

static double *objio_uknotv;
static int objio_uknotvlen;
static double *objio_vknotv;
static int objio_vknotvlen;

static int objio_curvtrimsurf; /* 0 - unset, 1 - curve, 2 - trim, 3 - surface */

static ay_nurbcurve_object objio_ncurve;
static ay_nurbpatch_object objio_npatch;

static double *objio_texturev;
static int objio_texturevlen;

static double objio_umin;
static double objio_umax;
static double objio_vmin;
static double objio_vmax;

typedef struct objio_trim_s {
  struct objio_trim_s *next;
  struct objio_trim_s *prev;

  unsigned int index;

  ay_object *c;

} objio_trim;

static objio_trim *objio_trims_head = NULL;
static objio_trim *objio_trims_tail = NULL;

static ay_object *objio_trims;
static ay_object **objio_nexttrim;

static ay_object *objio_lastface;


int objio_addvertex(int type, double *v);

int objio_getvertex(int type, unsigned int index, double **v);

int objio_freevertices(void);

int objio_readvertex(char *str);

int objio_readvindex(char *c, int *gvindex, int *tvindex, int *nvindex);

void objio_readskip(char **b);

int objio_readface(char *str, int lastlinewasface);

int objio_readpolyline(char *str);

int objio_readcstype(char *str);

int objio_readdeg(char *str);

int objio_readcurv(char *str);

int objio_readsurf(char *str);

int objio_readparm(char *str);

int objio_addtrim(ay_object *o);

int objio_gettrim(unsigned int index, ay_object **t);

void objio_freetrims(void);

int objio_readtrim(char *str, int hole);

int objio_fixnpatch(ay_nurbpatch_object *np);

int objio_fixncurve(ay_nurbcurve_object *nc);

int objio_readend(void);

int objio_readline(FILE *fileptr);

int objio_readscene(char *filenam);

int objio_readscenetcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);


/* objio_addvertex:
 *  add a vertex to a vertex buffer
 */
int
objio_addvertex(int type, double *v)
{
 objio_vertex *nv = NULL;

  if(!v)
    return AY_ENULL;

  if(!(nv = calloc(1, sizeof(objio_vertex))))
    return AY_EOMEM;
  nv->index = 1;

  switch(type)
    {
    case 1:
      /* geometric vertex */
      memcpy(nv->v, v, 4*sizeof(double));
      if(objio_gverts_tail)
	{
	  nv->index = objio_gverts_tail->index + 1;
	  nv->prev = objio_gverts_tail;
	  objio_gverts_tail->next = nv;
	}
      objio_gverts_tail = nv;
      if(!objio_gverts_head)
	{
	  objio_gverts_head = nv;
	}
      break;
    case 2:
      /* normal vertex */
      memcpy(nv->v, v, 3*sizeof(double));
      if(objio_nverts_tail)
	{
	  nv->index = objio_nverts_tail->index + 1;
	  nv->prev = objio_nverts_tail;
	  objio_nverts_tail->next = nv;
	}
      objio_nverts_tail = nv;
      if(!objio_nverts_head)
	{
	  objio_nverts_head = nv;
	}
      break;
    case 3:
      /* parametric vertex */
      memcpy(nv->v, v, 4*sizeof(double));
      if(objio_pverts_tail)
	{
	  nv->index = objio_pverts_tail->index + 1;
	  nv->prev = objio_pverts_tail;
	  objio_pverts_tail->next = nv;
	}
      objio_pverts_tail = nv;
      if(!objio_pverts_head)
	{
	  objio_pverts_head = nv;
	}
      break;
    case 4:
      /* texture vertex */
      memcpy(nv->v, v, 3*sizeof(double));
      if(objio_tverts_tail)
	{
	  nv->index = objio_tverts_tail->index + 1;
	  nv->prev = objio_tverts_tail;
	  objio_tverts_tail->next = nv;
	}
      objio_tverts_tail = nv;
      if(!objio_tverts_head)
	{
	  objio_tverts_head = nv;
	}
      break;
    default:
      free(nv);
      break;
    } /* switch */

 return AY_OK;
} /* objio_addvertex */


/* objio_getvertex:
 *  get a vertex from a vertex buffer
 */
int
objio_getvertex(int type, unsigned int index, double **v)
{
 static objio_vertex *objio_gverts_cur = NULL;
 static objio_vertex *objio_nverts_cur = NULL;
 static objio_vertex *objio_pverts_cur = NULL;
 static objio_vertex *objio_tverts_cur = NULL;
 objio_vertex *l = NULL, **pl;
 unsigned int i;

  if(!v)
    {
      objio_gverts_cur = NULL;
      objio_nverts_cur = NULL;
      objio_pverts_cur = NULL;
      objio_tverts_cur = NULL;
      return AY_ENULL;
    } /* if */

  if(index == 0)
    return AY_ERROR;

  switch(type)
    {
    case 1:
      /* vertex buffer empty? */
      if(!objio_gverts_tail)
	return AY_ERROR;

      /* index out of range? */
      if(index > objio_gverts_tail->index)
	return AY_ERROR;

      if(objio_gverts_cur)
	l = objio_gverts_cur;
      else
	l = objio_gverts_tail;

      pl = &objio_gverts_cur;
      break;
    case 2:
      /* vertex buffer empty? */
      if(!objio_nverts_tail)
	return AY_ERROR;

      /* index out of range? */
      if(index > objio_nverts_tail->index)
	return AY_ERROR;

      if(objio_nverts_cur)
	l = objio_nverts_cur;
      else
	l = objio_nverts_tail;

      pl = &objio_nverts_cur;
      break;
    case 3:
      /* vertex buffer empty? */
      if(!objio_pverts_tail)
	return AY_ERROR;

      /* index out of range? */
      if(index > objio_pverts_tail->index)
	return AY_ERROR;

      if(objio_pverts_cur)
	l = objio_pverts_cur;
      else
	l = objio_pverts_tail;

      pl = &objio_pverts_cur;
      break;
    case 4:
      /* vertex buffer empty? */
      if(!objio_tverts_tail)
	return AY_ERROR;

      /* index out of range? */
      if(index > objio_tverts_tail->index)
	return AY_ERROR;

      if(objio_tverts_cur)
	l = objio_tverts_cur;
      else
	l = objio_tverts_tail;

      pl = &objio_tverts_cur;
      break;
    default:
      return AY_ERROR;
      break;
    }

  /* check, whether the same vertex was requested last time */
  if(index == l->index)
    {
      /* Yes, we may immediately return the result. */
      *v = &(l->v[0]);
      return AY_OK;
    }

  if(index < l->index)
    {
      /* rewind... */
      for(i = l->index; i > index; i--)
	{
	  l = l->prev;
	}
    }
  else
    {
      /* forward... */
      for(i = l->index; i < index; i++)
	{
	  l = l->next;
	}
    } /* if */

  /* remember pointer to vertex in static variable */
  *pl = l;

  /* return result */
  *v = &(l->v[0]);

 return AY_OK;
} /* objio_getvertex */


/* objio_freevertices:
 *  free all vertex buffers
 */
int
objio_freevertices(void)
{
 int ay_status = AY_OK;
 objio_vertex *v = NULL, *vt = NULL;

  if(objio_gverts_tail)
    {
      objio_gverts_head = NULL;
      vt = objio_gverts_tail;
      while(vt)
	{
	  v = vt->prev;
	  free(vt);
	  vt = v;
	} /* while */
      objio_gverts_tail = NULL;
    } /* if */

  if(objio_nverts_tail)
    {
      objio_nverts_head = NULL;
      vt = objio_nverts_tail;
      while(vt)
	{
	  v = vt->prev;
	  free(vt);
	  vt = v;
	} /* while */
      objio_nverts_tail = NULL;
    } /* if */

  if(objio_pverts_tail)
    {
      objio_pverts_head = NULL;
      vt = objio_pverts_tail;
      while(vt)
	{
	  v = vt->prev;
	  free(vt);
	  vt = v;
	} /* while */
      objio_pverts_tail = NULL;
    } /* if */

  if(objio_tverts_tail)
    {
      objio_tverts_head = NULL;
      vt = objio_tverts_tail;
      while(vt)
	{
	  v = vt->prev;
	  free(vt);
	  vt = v;
	} /* while */
      objio_tverts_tail = NULL;
    } /* if */

  /* clear cached "current" pointers in objio_getvertex() */
  ay_status = objio_getvertex(0, 0, NULL);

 return AY_OK;
} /* objio_freevertices */


/* objio_readvertex:
 *  read a single vertex and add it to the appropriate vertex buffer
 */
int
objio_readvertex(char *str)
{
 int ay_status = AY_OK;
 double v[4] = {0};

  if(strlen(str) < 2)
    return AY_ERROR;

  if(str[1] == 'n')
    {
      sscanf(&(str[2])," %lg %lg %lg", &(v[0]), &(v[1]), &(v[2]));
      ay_status = objio_addvertex(2, v);
    }
  else
  if(str[1] == 'p')
    {
      if(sscanf(&(str[2])," %lg %lg %lg", &(v[0]), &(v[1]), &(v[3])) < 3)
	{
	  v[3] = 1.0;
	}
      ay_status = objio_addvertex(3, v);
    }
  else
  if(str[1] == 't')
    {
      sscanf(&(str[2])," %lg %lg %lg", &(v[0]), &(v[1]), &(v[2]));
      ay_status = objio_addvertex(4, v);
    }
  else
    {
      if(sscanf(&(str[1])," %lg %lg %lg %lg",
		&(v[0]), &(v[1]), &(v[2]), &(v[3])) < 4)
	{
	  v[3] = 1.0;
	}
      if(objio_scalefactor != 1.0)
	AY_V3SCAL(v, objio_scalefactor)
      ay_status = objio_addvertex(1, v);
    } /* if */

 return ay_status;
} /* objio_readvertex */


/* objio_readvindex:
 *  read a single vertex index of the form "g/t/n", where g is the
 *  index of the geometric vertex, t the texture vertex, and n the
 *  corresponding normal; returns results in gvindex, tvindex, and nvindex
 */
int
objio_readvindex(char *c, int *gvindex, int *tvindex, int *nvindex)
{
 int ay_status = AY_OK;

  if(!c || !gvindex || !tvindex || !nvindex)
    return AY_ENULL;

  /* parse geometric vertex index */
  sscanf(c, "%d", gvindex);
  /* forward to next index, /, or end */
  while((isspace(*c)))
    c++;
  if(*c == '\0')
    return AY_OK;
  while((isdigit(*c) || (*c == '-')) && (*c != '\0'))
    c++;
  if(*c == '\0')
    return AY_OK;

  /* parse texture vertex index? */
  if(*c == '/')
    {
      /* maybe; check if next char is not a / too (which would mean:
	 no texture vertex index present) */
      if(c[1] != '/')
	{
	  /* next char is not a /, parse texture vertex index */
	  c++;
	  if(*c == '\0')
	    return AY_OK;
	  sscanf(c, "%d", tvindex);

	  /* forward to next index or / */
	  while((isdigit(*c) || (*c == '-')) && (*c != '\0'))
	    c++;
	  if(*c == '\0')
	    return AY_OK;
	  if(*c == '/')
	    c++;
	  if(*c == '\0')
	    return AY_OK;
	}
      else
	{
	  /* skip over // combination */
	  c += 2;
	  if(*c == '\0')
	    return AY_OK;
	} /* if */

      /* parse normal vertex index */
      if(isdigit(*c) || (*c == '-'))
	{
	  sscanf(c, "%d", nvindex);
	} /* if */
    } /* if */

 return ay_status;
} /* objio_readvindex */


/* objio_readskip:
 *  skip over a number
 *  !Modifies argument <b>!
 */
void
objio_readskip(char **b)
{
 char *c = *b, *d;

  while((isspace(*c)) && (*c != '\0'))
    c++;
  if(*c == '\0')
    return;
  while(!(isspace(*c)) && (*c != '\0'))
    c++;

  /* skip over trailing whitespace */
  if(*c != '\0')
    {
      d = c;
      while((isspace(*d)) && (*d != '\0'))
	d++;
      if(*d == '\0')
	{
	  *b = d;
	  return;
	} /* if */
    } /* if */

  /* return result */
  *b = c;

 return;
} /* objio_readskip */


/* objio_readface:
 *  read a face statement (polygonal face)
 */
int
objio_readface(char *str, int lastlinewasface)
{
 int ay_status = AY_OK;
 char fname[] = "objio_readface";
 char *tcname = ay_prefs.texcoordname;
 char *c = NULL;
 int gvindex = 0, tvindex = 0, nvindex = 0, stride = 0, degen = AY_FALSE;
 int last_stride = 0;
 double *gv, *nv, *tv, *oldpnt;
 double *newcontrolv = NULL, *texv = NULL, *tmpv = NULL;
 int texvlen = 0;
 ay_pomesh_object po = {0}, *temppo;
 ay_tag *temptags;
 ay_object t = {0}, *o = NULL, *m = NULL;
 unsigned int nloops = 1, nverts = 0, i;
 static ay_list_object *storedfaces = NULL, **laststoredface = &storedfaces;
 static unsigned int nfaces = 0;
 ay_list_object *tl = NULL;
 /*double angle, v1[3] = {0}, v2[3] = {0}, *v;*/

  if(!str && (lastlinewasface != -1))
    return AY_ENULL;

  if(lastlinewasface == -1)
    {
      /*printf("Merging %d faces...\n",nfaces);*/

      /* merge */
      m = NULL;
      if(storedfaces && storedfaces->next)
	ay_status = ay_pomesht_merge(objio_mergepvtags, storedfaces, &m);

      /*printf("Done merging faces...\n");*/

      if(ay_status)
	goto cleanup;

      nfaces = 0;
      if(m)
	{
	  /* replace objio_lastface with m */
	  temppo = objio_lastface->refine;
	  objio_lastface->refine = m->refine;
	  m->refine = temppo;

	  temptags = objio_lastface->tags;
	  objio_lastface->tags = m->tags;
	  m->tags = temptags;

	  /* remove temporary object that now carries the old poly mesh */
	  ay_status = ay_object_delete(m);
	} /* if */

      /* clean up */
      while(storedfaces)
	{
	  tl = storedfaces->next;
	  ay_object_delete(storedfaces->object);
	  free(storedfaces);
	  storedfaces = tl;
	} /* while */

      laststoredface = &storedfaces;
      return AY_OK;
    } /* if */

  if((str[0] == '\0') || (str[1] == '\0') || (str[2] == '\0'))
    return AY_ERROR;

  po.npolys = 1;
  po.nloops = &nloops;

  c = &(str[1]);

  while(*c != '\0')
    {
      gvindex = 0; tvindex = 0; nvindex = 0;
      ay_status = objio_readvindex(c, &gvindex, &tvindex, &nvindex);

      gv = NULL;
      nv = NULL;
      tv = NULL;

      /* get geometric vertex data and add it to the pomesh */
      if(gvindex < 0)
	{
	  if(objio_gverts_tail)
	    {
	      ay_status = objio_getvertex(1,
					objio_gverts_tail->index + gvindex + 1,
					     &gv);
	    }
	  else
	    {
	      ay_status = AY_ENULL;
	      goto cleanup;
	    } /* if */
	}
      else
	{
	  ay_status = objio_getvertex(1, gvindex, &gv);
	} /* if */
      if(ay_status)
	goto cleanup;

      if(nvindex != 0)
	{
	  /* get normal vertex data and add it to the pomesh */
	  if(nvindex < 0)
	    {
	      if(objio_nverts_tail)
		{
		  ay_status = objio_getvertex(2,
				        objio_nverts_tail->index + nvindex + 1,
					     &nv);
		}
	      else
		{
		  ay_status = AY_ENULL;
		  goto cleanup;
		} /* if */
	    }
	  else
	    {
	      ay_status = objio_getvertex(2, nvindex, &nv);
	    } /* if */
	  if(ay_status)
	    goto cleanup;
	} /* if */

      if(tvindex != 0)
	{
	  /* get texture vertex data and cache it in texv */

	  tv = NULL;
	  if(tvindex < 0)
	    {
	      if(objio_tverts_tail)
		{
		  ay_status = objio_getvertex(4,
					objio_tverts_tail->index + tvindex + 1,
						 &tv);
		}
	      else
		{
		  ay_status = AY_ENULL;
		  goto cleanup;
		} /* if */
	    }
	  else
	    {
	      ay_status = objio_getvertex(4, tvindex, &tv);
	    } /* if */

	  if(tv)
	    {
	      if(!(tmpv = realloc(texv, (texvlen + 1) * 2 * sizeof(double))))
		{ ay_status = AY_EOMEM; goto cleanup; }
	      texv = tmpv;
	      memcpy(&(texv[texvlen*2]), tv, 2*sizeof(double));
	      texvlen++;
	    } /* if */

	} /* if */

      stride = 3;
      if(nv)
	stride += 3;

      /*if(tv)
	stride += 2;*/

      if((last_stride > 0) && (stride != last_stride))
	{ ay_status = AY_ERROR; goto cleanup; }

      last_stride = stride;

      /* check for degenerated faces */
      if(objio_checkdegen)
	{
	  degen = AY_FALSE;
	  for(i = 0; i < nverts; i++)
	    {
	      oldpnt = &(po.controlv[i*stride]);
	      if(AY_V3COMP(oldpnt, gv))
		{
		  ay_error(AY_EWARN, fname, "Degenerated face encountered!");
		  /*ay_error(AY_EOUTPUT, fname, str);*/
		  degen = AY_TRUE;
		  break;
		}
	    } /* for */
	  if(degen)
	    {
	      /* skip to next vindex */
	      objio_readskip(&c);
	      break;
	    } /* if */
	} /* if */

      /* add vertex to PolyMesh object */
      newcontrolv = NULL;
      if(!(newcontrolv = realloc(po.controlv, (nverts + 1) * stride *
				 sizeof(double))))
	{ay_status = AY_EOMEM; goto cleanup;}
      po.controlv = newcontrolv;

      i = nverts * stride;
      if(gv)
	memcpy(&(po.controlv[i]), gv, 3*sizeof(double));
      if(nv)
	memcpy(&(po.controlv[i+3]), nv, 3*sizeof(double));
      /*if(tv)
	memcpy(&(po.controlv[i+6]), tv, 2*sizeof(double));*/

      nverts++;

      /* skip to next vindex */
      objio_readskip(&c);
    } /* while */

  if(nverts >= 3 && !degen)
    {
      if(nvindex != 0)
	{
	  po.has_normals = AY_TRUE;
	}
      /* XXXX replace with working degeneracy test
      if(nverts == 4)
	{
	  v = po.controlv;
	  v1[0] = v[stride] - v[0];
	  v1[1] = v[stride+1] - v[1];
	  v1[2] = v[stride+2] - v[2];
	  v2[0] = v[3*stride] - v[0];
	  v2[1] = v[3*stride+1] - v[1];
	  v2[2] = v[3*stride+2] - v[2];
	  angle = acos(AY_V3DOT(v1,v2)/(AY_V3LEN(v1)*AY_V3LEN(v2)));

	  v1[0] = v[2*stride] - v[stride];
	  v1[1] = v[2*stride+1] - v[stride+1];
	  v1[2] = v[2*stride+2] - v[stride+2];
	  v2[0] = v[0] - v[stride];
	  v2[1] = v[1] - v[stride+1];
	  v2[2] = v[2] - v[stride+2];

	  angle += acos(AY_V3DOT(v1,v2)/(AY_V3LEN(v1)*AY_V3LEN(v2)));

	  v1[0] = v[3*stride] - v[2*stride];
	  v1[1] = v[3*stride+1] - v[2*stride+1];
	  v1[2] = v[3*stride+2] - v[2*stride+2];
	  v2[0] = v[stride] - v[2*stride];
	  v2[1] = v[stride+1] - v[2*stride+1];
	  v2[2] = v[stride+2] - v[2*stride+2];

	  angle += acos(AY_V3DOT(v1,v2)/(AY_V3LEN(v1)*AY_V3LEN(v2)));

	  v1[0] = v[0] - v[3*stride];
	  v1[1] = v[1] - v[3*stride+1];
	  v1[2] = v[2] - v[3*stride+2];
	  v2[0] = v[2*stride] - v[3*stride];
	  v2[1] = v[2*stride+1] - v[3*stride+1];
	  v2[2] = v[2*stride+2] - v[3*stride+2];

	  angle += acos(AY_V3DOT(v1,v2)/(AY_V3LEN(v1)*AY_V3LEN(v2)));
	  if(fabs(AY_R2D(angle) - 360.0)>179)
	    {
	      printf("%g\n",AY_R2D(angle));
	      memcpy(v1, v, stride*sizeof(double));
	      memcpy(v, &(v[stride]), stride*sizeof(double));
	      memcpy(&(v[stride]), v1, stride*sizeof(double));
	    }

	}
      */

      po.nverts = &nverts;
      po.ncontrols = nverts;

      if(!(po.verts = calloc(nverts, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }

      for(i = 0; i < nverts; i++)
	{
	  po.verts[i] = i;
	}

      ay_object_defaults(&t);
      t.type = AY_IDPOMESH;
      t.refine = (void*)(&po);

      if(texv)
	{
	  ay_status = ay_pv_add(&t, tcname, "varying", "g",
				texvlen, 2, texv);
	}

      if(objio_mergecfaces)
	{
	  /* store */
	  if(!(*laststoredface = calloc(1, sizeof(ay_list_object))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  ay_status = ay_object_copy(&t, &((*laststoredface)->object));
	  laststoredface = &((*laststoredface)->next);
	  nfaces++;
	} /* if */

      if(!objio_mergecfaces || !lastlinewasface)
	{
	  /* link new polymesh into the scene */
	  ay_status = ay_object_copy(&t, &o);
	  if(ay_status)
	    goto cleanup;

	  ay_object_link(o);

	  objio_lastface = o;
	} /* if */
    } /* if */

cleanup:
  if(po.controlv)
    free(po.controlv);

  if(po.verts)
    free(po.verts);

  if(texv)
    free(texv);

  if(t.tags)
    ay_tags_delall(&t);

  if(degen)
    {
      return AY_ERROR;
    }

 return ay_status;
} /* objio_readface */


/* objio_readpolyline:
 *  read a line statement (polygon)
 */
int
objio_readpolyline(char *str)
{
 int ay_status = AY_OK;
 char *c = NULL;
 int gvindex = 0, tvindex = 0, dummy, length = 0, stride = 4, texvlen = 0;
 double *gv, *tv, *controlv = NULL, *tmp, *texv = NULL;
 ay_object *o = NULL;

  if(!str)
    return AY_ENULL;

  if((str[0] == '\0') || (str[1] == '\0') || (str[2] == '\0'))
    return AY_ERROR;

  c = &(str[1]);

  while(*c != '\0')
    {
      gvindex = 0; tvindex = 0;
      ay_status = objio_readvindex(c, &gvindex, &tvindex, &dummy);

      /* XXXX check status */

      gv = NULL;
      tv = NULL;

      /* get geometric vertex data and add it to the curve */
      if(gvindex < 0)
	{
	  if(objio_gverts_tail)
	    {
	      ay_status = objio_getvertex(1,
					objio_gverts_tail->index + gvindex + 1,
					  &gv);
	    }
	  else
	    {
	      ay_status = AY_ENULL;
	      goto cleanup;
	    } /* if */
	}
      else
	{
	  ay_status = objio_getvertex(1, gvindex, &gv);
	} /* if */
      if(ay_status)
	goto cleanup;

      if(gv)
	{
	  if(!(tmp = realloc(controlv, (length + 1) * stride *
			     sizeof(double))))
	    {ay_status = AY_EOMEM; goto cleanup;}
	  controlv = tmp;
	  memcpy(&(controlv[length * stride]), gv, stride * sizeof(double));
	  length++;
	} /* if */

      if(tvindex != 0)
	{
	  /* get texture vertex data and cache it in texv */

	  tv = NULL;
	  if(tvindex < 0)
	    {
	      if(objio_tverts_tail)
		{
		  ay_status = objio_getvertex(4,
					objio_tverts_tail->index + tvindex + 1,
					      &tv);
		}
	      else
		{
		  ay_status = AY_ENULL;
		  goto cleanup;
		} /* if */
	    }
	  else
	    {
	      ay_status = objio_getvertex(4, tvindex, &tv);
	    } /* if */

	  if(tv)
	    {
	      if(!(tmp = realloc(texv, (texvlen + 1) * 2 * sizeof(double))))
		{ ay_status = AY_EOMEM; goto cleanup; }
	      texv = tmp;
	      memcpy(&(texv[texvlen*2]), tv, 2*sizeof(double));
	      texvlen++;
	    } /* if */
	} /* if */

      /* skip to next vindex */
      objio_readskip(&c);
    } /* while */

  if(length < 2)
    goto cleanup;

  /* create/link the curve object */
  if(!(o = calloc(1, sizeof(ay_object))))
    {ay_status = AY_EOMEM; goto cleanup;}
  ay_object_defaults(o);
  o->type = AY_IDNCURVE;
  ay_status = ay_nct_create(2, length, AY_KTNURB, controlv, NULL,
			    ((ay_nurbcurve_object **)(void*)&(o->refine)));

  ay_object_link(o);

  /* prevent cleanup code from doing something harmful */
  controlv = NULL;

cleanup:
  if(controlv)
    free(controlv);

  if(texv)
    free(texv);

 return ay_status;
} /* objio_readpolyline */


/* objio_readcstype:
 *  read a cstype statement signifying the type of the next curve/surface
 */
int
objio_readcstype(char *str)
{
 int ay_status = AY_OK;

  if(!str)
    return AY_ENULL;

  if(strstr(str, "rat"))
    {
      objio_csrat = AY_TRUE;
    }
  else
    {
      objio_csrat = AY_FALSE;
    }

  if(strstr(str, "bmatrix"))
    {
      objio_cstype = 0;
    }

  if(strstr(str, "bezier"))
    {
      objio_cstype = 1;
    }

  if(strstr(str, "bspline"))
    {
      objio_cstype = 2;
    }

  if(strstr(str, "cardinal"))
    {
      objio_cstype = 3;
    }

  if(strstr(str, "taylor"))
    {
      objio_cstype = 4;
    }

 return ay_status;
} /* objio_readcstype */


/* objio_readdeg:
 *  read a deg (degree) statement
 */
int
objio_readdeg(char *str)
{
 int ay_status = AY_OK;
 char *c = str;

  if(!str)
    return AY_ENULL;

  while(!(isspace(*c)) && (*c != '\0'))
    c++;

  sscanf(c," %d %d", &objio_degu, &objio_degv);

 return ay_status;
} /* objio_readdeg */


/* objio_readcurv:
 *  read a curv (freeform curve) statement
 */
int
objio_readcurv(char *str)
{
 int ay_status = AY_OK;
 char *c = str;
 int gvindex = 0, tvindex = 0, nvindex = 0, stride = 4;
 double *gv, *controlv;
 int vtype = 0;

  if(!str)
    return AY_ENULL;

  if(str[4] == '2' /*strstr(str, "curv2")*/)
    {
      objio_curvtrimsurf = 2;
      vtype = 3; /* parametric vertex */
    }
  else
    {
      objio_curvtrimsurf = 1;
      vtype = 1; /* geometric vertex */
    } /* if */

  while(!(isspace(*c)) && (*c != '\0'))
    c++;

  /* read umin/umax */
  if(vtype == 1)
    {
      sscanf(c, " %lg %lg", &objio_umin, &objio_umax);
      objio_readskip(&c);
      objio_readskip(&c);
    }

  /* read (and resolve) control point indices */
  while(*c != '\0')
    {
      gvindex = 0; tvindex = 0; nvindex = 0;
      ay_status = objio_readvindex(c, &gvindex, &tvindex, &nvindex);
      gv = NULL;

      if(gvindex < 0)
	{
	  if(vtype == 1)
	    {
	      if(objio_gverts_tail)
		{
		  ay_status = objio_getvertex(vtype,
					objio_gverts_tail->index + gvindex + 1,
						 &gv);
		}
	      else
		{
		  ay_status = AY_ENULL;
		  goto cleanup;
		} /* if */
	    }
	  else
	    {
	      /* vtype == 3, get parametric vertex... */
	      if(objio_pverts_tail)
		{
		  ay_status = objio_getvertex(vtype,
					objio_pverts_tail->index + gvindex + 1,
						 &gv);
		}
	      else
		{
		  ay_status = AY_ENULL;
		  goto cleanup;
		} /* if */
	    } /* if */
	}
      else
	{
	  ay_status = objio_getvertex(vtype, gvindex, &gv);
	} /* if */

      if(gv)
	{
	  if(!(controlv = realloc(objio_ncurve.controlv,
			 (objio_ncurve.length + 1) * stride * sizeof(double))))
	    { return AY_EOMEM; }
	  objio_ncurve.controlv = controlv;
	  memcpy(&(controlv[objio_ncurve.length * stride]), gv,
		 stride * sizeof(double));
	  objio_ncurve.length++;
	} /* if */

      /* skip to next vindex */
      objio_readskip(&c);
    } /* while */

cleanup:

 return ay_status;
} /* objio_readcurv */


/* objio_readsurf:
 *  read a surf (freeform surface) statement
 */
int
objio_readsurf(char *str)
{
 int ay_status = AY_OK;
 char *c = str;
 int gvindex = 0, tvindex = 0, nvindex = 0, stride = 4;
 int glength = 0, tlength = 0;
 double *gv, *controlv = NULL, *tv, *texturev = NULL;

  if(!str)
    return AY_ENULL;

  while(!(isspace(*c)) && (*c != '\0'))
    c++;

  /* read umin/umax */
  sscanf(c, " %lg %lg", &objio_umin, &objio_umax);
  objio_readskip(&c);
  objio_readskip(&c);

  /* read vmin/vmax */
  sscanf(c, " %lg %lg", &objio_vmin, &objio_vmax);
  objio_readskip(&c);
  objio_readskip(&c);

  /* read (and resolve) control point indices */
  while(*c != '\0')
    {
      gvindex = 0; tvindex = 0; nvindex = 0;
      ay_status = objio_readvindex(c, &gvindex, &tvindex, &nvindex);

      gv = NULL;
      if(gvindex < 0)
	{
	  if(objio_gverts_tail)
	    {
	      ay_status = objio_getvertex(1,
					objio_gverts_tail->index + gvindex + 1,
					     &gv);
	    }
	  else
	    {
	      ay_status = AY_ENULL;
	      goto cleanup;
	    } /* if */
	}
      else
	{
	  ay_status = objio_getvertex(1, gvindex, &gv);
	} /* if */

      if(gv)
	{
	  if(!(controlv = realloc(objio_npatch.controlv,
			 (glength + 1) * stride * sizeof(double))))
	    { return AY_EOMEM; }
	  objio_npatch.controlv = controlv;
	  memcpy(&(controlv[glength * stride]), gv,
		 stride * sizeof(double));
	  glength++;
	} /* if */

      if(tvindex != 0)
	{
	  tv = NULL;
	  if(tvindex < 0)
	    {
	      if(objio_tverts_tail)
		{
		  ay_status = objio_getvertex(4,
					objio_tverts_tail->index + tvindex + 1,
						 &tv);
		}
	      else
		{
		  ay_status = AY_ENULL;
		  goto cleanup;
		} /* if */
	    }
	  else
	    {
	      ay_status = objio_getvertex(4, tvindex, &tv);
	    } /* if */

	  if(tv)
	    {

	      if(!(texturev = realloc(objio_texturev,
				       (tlength + 1) * 2 * sizeof(double))))
		{ return AY_EOMEM; }
	      objio_texturev = texturev;
	      memcpy(&(texturev[tlength*2]), tv, 2*sizeof(double));
	      tlength++;
	    } /* if */
	} /* if */

      /* skip to next vindex */
      objio_readskip(&c);
    } /* while */

  objio_curvtrimsurf = 3;

  objio_texturevlen = tlength;

cleanup:

 return ay_status;
} /* objio_readsurf */


/* objio_readparm:
 *  read a knot vector
 */
int
objio_readparm(char *str)
{
 int ay_status = AY_OK;
 double knot, *knotv = NULL, *t = NULL;
 int readu = AY_FALSE, knots = 0;
 char *c = str;

  if(!str)
    return AY_ENULL;

  while(!(isspace(*c)) && (*c != '\0'))
    c++;

  if(c[1] == 'u')
    {
      readu = AY_TRUE;
    }

  objio_readskip(&c);

  while(*c != '\0')
    {
      if(sscanf(c, "%lg", &knot))
	{
	  t = NULL;
	  if(!(t = realloc(knotv, (knots + 1) * sizeof(double))))
	    { if(knotv){free(knotv);} return AY_EOMEM; }
	  knotv = t;
	  knotv[knots] = knot;
	  knots++;
	} /* if */

      /* skip to next knot */
      objio_readskip(&c);
    } /* while */

  if(readu)
    {
      if(objio_uknotv)
	free(objio_uknotv);
      objio_uknotv = knotv;
      objio_uknotvlen = knots;
    }
  else
    {
      if(objio_vknotv)
	free(objio_vknotv);
      objio_vknotv = knotv;
      objio_vknotvlen = knots;
    } /* if */

 return ay_status;
} /* objio_readparm */


/* objio_addtrim:
 *  add trim curve <o> to the trim curve buffer
 */
int
objio_addtrim(ay_object *o)
{
 objio_trim *nt = NULL;

  if(!o)
    return AY_ENULL;

  if(!(nt = calloc(1, sizeof(objio_trim))))
    return AY_EOMEM;
  nt->index = 1;

  nt->c = o;

  if(objio_trims_tail)
    {
      nt->index = objio_trims_tail->index + 1;
      nt->prev = objio_trims_tail;
      objio_trims_tail->next = nt;
    }
  objio_trims_tail = nt;
  if(!objio_trims_head)
    {
      objio_trims_head = nt;
    }

 return AY_OK;
} /* objio_addtrim */


/* objio_gettrim:
 *  get a trim curve object from the trim curve buffer
 */
int
objio_gettrim(unsigned int index, ay_object **t)
{
 static objio_trim *objio_trims_cur = NULL;
 objio_trim *l = NULL, **pl;
 unsigned int i;

  if(!t)
    {
      objio_trims_cur = NULL;
      return AY_ENULL;
    } /* if */

  if(index == 0)
    return AY_ERROR;

  /* trim buffer empty? */
  if(!objio_trims_tail)
    return AY_ERROR;

  /* index out of range? */
  if(index > objio_trims_tail->index)
    return AY_ERROR;

  if(objio_trims_cur)
    l = objio_trims_cur;
  else
    l = objio_trims_tail;

  pl = &objio_trims_cur;

  /* check, whether the same trim was requested last time */
  if(index == l->index)
    {
      /* Yes, we may immediately return the result. */
      *t = l->c;
      return AY_OK;
    }

  if(index < l->index)
    {
      /* rewind... */
      for(i = l->index; i > index; i--)
	{
	  l = l->prev;
	}
    }
  else
    {
      /* forward... */
      for(i = l->index; i < index; i++)
	{
	  l = l->next;
	}
    } /* if */

  *pl = l;
  *t = l->c;

 return AY_OK;
} /* objio_gettrim */


/* objio_freetrims:
 *  free trim curve buffer
 */
void
objio_freetrims(void)
{
 int ay_status = AY_OK;
 objio_trim *t = NULL, *tt = NULL;

  if(objio_trims_tail)
    {
      objio_trims_head = NULL;
      tt = objio_trims_tail;
      while(tt)
	{
	  t = tt->prev;
	  (void)ay_object_delete(tt->c);
	  free(tt);
	  tt = t;
	} /* while */
      objio_trims_tail = NULL;
    } /* if */

  /* clear cached "current" pointers in objio_gettrim() */
  ay_status = objio_gettrim(0, NULL);

 return;
} /* objio_freetrims */


/* objio_readtrim:
 *  read a trim statement (trim loop specification),
 *  get the corresponding trim curves from the trim curve
 *  buffer and copy them for connection to the surface
 *  object later on
 */
int
objio_readtrim(char *str, int hole)
{
 int ay_status = AY_OK;
 int tcount = 0, tindex = 0;
 char *c = str;
 ay_level_object *lev = NULL;
 ay_object *t = NULL, *l = NULL, **orig_nexttrim = objio_nexttrim;
 ay_nurbcurve_object *nc = NULL;

  if(!str)
    return AY_ENULL;

  while(!(isspace(*c)) && (*c != '\0'))
    c++;

  while(*c != '\0')
    {
      /* read umin/umax */
      sscanf(c, " %lg %lg", &objio_umin, &objio_umax);
      objio_readskip(&c);
      objio_readskip(&c);

      tindex = 0;
      if(sscanf(c, "%d", &tindex))
	{
	  t = NULL;
	  if(tindex < 0)
	    {
	      if(objio_trims_tail)
		{
		  ay_status = objio_gettrim(
					objio_trims_tail->index + tindex + 1,
					     &t);
		}
	      else
		{
		  ay_status = AY_ENULL;
		  goto cleanup;
		} /* if */
	    }
	  else
	    {
	      ay_status = objio_gettrim(tindex, &t);
	    } /* if */

	  if(t)
	    {
	      if(tcount == 1)
		{
		  /* second trim => it is a loop => create level */
		  if(!(lev = calloc(1, sizeof(ay_level_object))))
		    { ay_status = AY_EOMEM; goto cleanup; }
		  lev->type = AY_LTLEVEL;
		  if(!(l = calloc(1, sizeof(ay_object))))
		    { free(lev); ay_status = AY_EOMEM; goto cleanup; }
		  ay_object_defaults(l);
		  l->type = AY_IDLEVEL;
		  l->parent = AY_TRUE;
		  l->refine = lev;
		  l->down = *orig_nexttrim;
		  *orig_nexttrim = l;
		} /* if */

	      ay_status = ay_object_copy(t, objio_nexttrim);
	      if(ay_status)
		goto cleanup;

	      nc = (ay_nurbcurve_object *)((*objio_nexttrim)->refine);

	      if((objio_umin > nc->knotv[nc->order]) ||
		 (objio_umax < nc->knotv[nc->length]))
		ay_knots_setuminmax(*objio_nexttrim, objio_umin, objio_umax);

	      objio_nexttrim = &((*objio_nexttrim)->next);

	      tcount++;
	    } /* if */
	} /* if */

      /* skip to next trim */
      objio_readskip(&c);
    } /* while */

  /* repair nexttrim pointer? */
  if(tcount > 1)
    {
      *objio_nexttrim = ay_endlevel;
      objio_nexttrim = &(l->next);
    }

cleanup:

 return ay_status;
} /* objio_readtrim */


/* objio_fixnpatch:
 *  fix row/column major order in np controlv (from Wavefront to Ayam style);
 *  additionally, set the is_rat attribute
 *  (optionally) de-multiplying the weights for rational vertices,
 *  (optionally) rescaling the knot vector to safe distances, and
 *  trying to detect the knot type
 */
int
objio_fixnpatch(ay_nurbpatch_object *np)
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

	  if(objio_writedmw && (p1[3] != 1.0))
	    {
	      p1[0] /= p1[3];
	      p1[1] /= p1[3];
	      p1[2] /= p1[3];
	    }

	  p1 += stride;
	  p2 += np->width*stride;
	} /* for */
    } /* for */

  free(np->controlv);
  np->controlv = v;

  np->is_rat = ay_npt_israt(np);

  if(objio_rescaleknots != 0.0)
    {
      ay_knots_rescaletomindist(np->width+np->uorder, np->uknotv,
				objio_rescaleknots);

      ay_knots_rescaletomindist(np->height+np->vorder, np->vknotv,
				objio_rescaleknots);

    }

  np->uknot_type = ay_knots_classify(np->uorder, np->uknotv,
				     np->width+np->uorder,
				     AY_EPSILON);

  np->vknot_type = ay_knots_classify(np->vorder, np->vknotv,
				     np->height+np->vorder,
				     AY_EPSILON);

 return ay_status;
} /* objio_fixnpatch */


/* objio_fixncurve:
 *  fix up a Wavefront NURBS curve by
 *  setting the is_rat attribute
 *  (optionally) de-multiplying the weights
 *  (optionally) rescaling the knot vector to safe distances, and
 *  trying to detect the knot type
 */
int
objio_fixncurve(ay_nurbcurve_object *nc)
{
 int ay_status = AY_OK;
 int i, stride = 4;
 double *p = NULL;

  if(!nc)
    return AY_ENULL;

  p = nc->controlv;

  nc->is_rat = ay_nct_israt(nc);

  if(nc->is_rat && objio_writedmw)
    {
      for(i = 0; i < nc->length; i++)
	{
	  if(p[3] != 1.0)
	    {
	      p[0] /= p[3];
	      p[1] /= p[3];
	      p[2] /= p[3];
	    } /* if */
	  p += stride;
	} /* for */
    }

  if(objio_rescaleknots != 0.0)
    {
      ay_knots_rescaletomindist(nc->length+nc->order, nc->knotv,
				objio_rescaleknots);
    }

  nc->knot_type = ay_knots_classify(nc->order, nc->knotv,
				    nc->length+nc->order,
				    AY_EPSILON);

 return ay_status;
} /* objio_fixncurve */


/* objio_readend:
 *  read an end statement (realise curve or surface)
 */
int
objio_readend(void)
{
 int ay_status = AY_OK;
 char *tcname = ay_prefs.texcoordname;
 ay_object *newo = NULL, *o = NULL;
 ay_nurbpatch_object *np = NULL;
 double oldmin, oldmax;
 int is_bound = AY_FALSE;

  switch(objio_curvtrimsurf)
    {
    case 1:
      /* read a normal 3D curve */
      if(!objio_readcurves)
	goto cleanup;

      /* discard unspecified, bmatrix, cardinal, and taylor splines */
      if((objio_cstype == -1) || (objio_cstype == 0) ||
	 (objio_cstype == 3) || (objio_cstype == 4))
	break;

      if(!(newo = calloc(1, sizeof(ay_object))))
	return AY_EOMEM;
      ay_object_defaults(newo);
      newo->type = AY_IDNCURVE;
      newo->refine = &objio_ncurve;
      objio_ncurve.knotv = objio_uknotv;
      objio_ncurve.order = objio_degu + 1;
      objio_ncurve.knot_type = AY_KTCUSTOM;

      ay_status = objio_fixncurve(&objio_ncurve);

      ay_status = ay_object_copy(newo, &o);
      if(ay_status)
	goto cleanup;

      if((objio_umin > objio_uknotv[objio_degu+1]) ||
	 (objio_umax < objio_uknotv[objio_ncurve.length]))
	ay_knots_setuminmax(o, objio_umin, objio_umax);

      ay_object_link(o);

      break;
    case 2:
      /* read a 2D (trim) curve */

      /* discard unspecified, bmatrix, cardinal, and taylor splines */
      if((objio_cstype == -1) || (objio_cstype == 0) ||
	 (objio_cstype == 3) || (objio_cstype == 4))
	break;

      if(!(newo = calloc(1, sizeof(ay_object))))
	return AY_EOMEM;
      ay_object_defaults(newo);
      newo->type = AY_IDNCURVE;
      newo->refine = &objio_ncurve;
      objio_ncurve.knotv = objio_uknotv;
      objio_ncurve.order = objio_degu + 1;
      objio_ncurve.knot_type = AY_KTCUSTOM;

      ay_status = objio_fixncurve(&objio_ncurve);

      ay_status = ay_object_copy(newo, &o);
      if(ay_status)
	goto cleanup;

      ay_status = objio_addtrim(o);

      break;
    case 3:
      /* read a surface */

      /* discard unspecified, bmatrix, cardinal, and taylor splines */
      if((objio_cstype == -1) || (objio_cstype == 0) ||
	 (objio_cstype == 3) || (objio_cstype == 4))
	break;

      if(!(newo = calloc(1, sizeof(ay_object))))
	return AY_EOMEM;
      ay_object_defaults(newo);
      newo->type = AY_IDNPATCH;
      newo->parent = AY_TRUE;
      newo->inherit_trafos = AY_FALSE;
      newo->hide_children = AY_TRUE;

      newo->refine = &objio_npatch;

      objio_npatch.uknotv = objio_uknotv;
      objio_npatch.vknotv = objio_vknotv;
      objio_npatch.uorder = objio_degu + 1;
      objio_npatch.vorder = objio_degv + 1;

      objio_npatch.width = objio_uknotvlen - objio_degu - 1;
      objio_npatch.height = objio_vknotvlen - objio_degv - 1;

      objio_npatch.uknot_type = AY_KTCUSTOM;
      objio_npatch.vknot_type = AY_KTCUSTOM;

      ay_status = objio_fixnpatch(&objio_npatch);

      ay_status = ay_object_copy(newo, &o);
      if(ay_status)
	goto cleanup;

      /* add trim curves */
      if(objio_trims)
	{
	  /* properly terminate the list of trims */
	  *objio_nexttrim = ay_endlevel;
	  o->down = objio_trims;

	  /* check for simple trim, if it is the only one */
	  if((!objio_readstrim) && (objio_trims->next->next))
	    {
	      ay_status = ay_npt_isboundcurve(o->down,
				   objio_npatch.uknotv[objio_npatch.uorder-1],
				   objio_npatch.uknotv[objio_npatch.width],
				   objio_npatch.vknotv[objio_npatch.vorder-1],
				   objio_npatch.vknotv[objio_npatch.height],
					      &is_bound);
	      /* discard simple trim */
	      if(is_bound)
		{
		  (void)ay_object_deletemulti(objio_trims, AY_FALSE);
		  o->down = ay_endlevel;
		}
	    } /* if */
	  objio_trims = NULL;
	  objio_nexttrim = &(objio_trims);
	}
      else
	{
	  o->down = ay_endlevel;
	} /* if */

      /* rescale knot vectors */
      if(objio_rescaleknots > 0.0)
	{
	  np = (ay_nurbpatch_object *)o->refine;

	  /* save old knot range */
	  oldmin = np->uknotv[0];
	  oldmax = np->uknotv[np->width+np->uorder-1];

	  /* rescale knots */
	  ay_knots_rescaletomindist(np->width+np->uorder, np->uknotv,
				    objio_rescaleknots);
	  /* scale trim curves */
	  if(o->down && o->down->next)
	    {
	      ay_status = ay_npt_rescaletrims(o->down, 0, oldmin, oldmax,
					      np->uknotv[0],
				 np->uknotv[np->width+np->uorder-1]);
	    }

	  /* save old knot range */
	  oldmin = np->vknotv[0];
	  oldmax = np->vknotv[np->height+np->vorder-1];

	  /* rescale knots */
	  ay_knots_rescaletomindist(np->height+np->vorder, np->vknotv,
				    objio_rescaleknots);

	  /* scale trim curves */
	  if(o->down && o->down->next)
	    {
	      ay_status = ay_npt_rescaletrims(o->down, 1, oldmin, oldmax,
					      np->vknotv[0],
				 np->vknotv[np->height+np->vorder-1]);
	    }

	} /* if*/

      /* add texture coordinates (as PV tags) */
      if(objio_texturev)
	{
	  ay_status = ay_pv_add(o, tcname, "varying", "g",
				objio_texturevlen, 1, objio_texturev);
	}

      /* add umin/umax/vmin/vmax tags */
      if((objio_umin > objio_uknotv[objio_degu+1]) ||
	 (objio_umax < objio_uknotv[objio_npatch.width]))
	ay_knots_setuminmax(o, objio_umin, objio_umax);

      if((objio_vmin > objio_vknotv[objio_degv+1]) ||
	 (objio_vmax < objio_vknotv[objio_npatch.height]))
	ay_knots_setvminmax(o, objio_vmin, objio_vmax);

      ay_object_link(o);

    default:
      /* read a superfluous end? */

      /* just cleanup */
      goto cleanup;
      break;
    } /* switch */

cleanup:

  if(newo)
    free(newo);
  if(objio_uknotv)
    free(objio_uknotv);
  objio_uknotv = NULL;
  if(objio_vknotv)
    free(objio_vknotv);
  objio_vknotv = NULL;
  if(objio_ncurve.controlv)
    free(objio_ncurve.controlv);
  objio_ncurve.controlv = NULL;
  objio_ncurve.length = 0;
  if(objio_npatch.controlv)
    free(objio_npatch.controlv);
  objio_npatch.controlv = NULL;
  if(objio_texturev)
    free(objio_texturev);
  objio_texturev = NULL;

  objio_curvtrimsurf = 0;

 return ay_status;
} /* objio_readend */


/* objio_readline:
 *  read a single line from a Wavefront OBJ file
 */
int
objio_readline(FILE *fileptr)
{
 int ay_status = AY_OK;
 int read, peek, continuation = AY_FALSE;
 static int lastlinewasface = AY_FALSE;
 char readchar, *str;
 Tcl_DString ds;

  if(!fileptr)
    {
      if(lastlinewasface)
	ay_status = objio_readface(NULL, -1);
      lastlinewasface = AY_FALSE;
      return AY_ENULL;
    }

  Tcl_DStringInit(&ds);

  read = getc(fileptr);

  if(read == EOF)
    {Tcl_DStringFree(&ds); return AY_EUEOF;}

  if((char)read == '\n' || (char)read == '\r')
    {Tcl_DStringFree(&ds); return AY_OK;}

  /* read characters from the file until a line end is hit
     (and no line continuation hint ('\') was encountered before
      on that line) */
  while(1)
    {
      /* put read character to the buffer string
	 (but omit line end and continuation characters) */
      if((char)read != '\n' && (char)read != '\r' && (char)read != '\\')
	{
	  readchar = (char)read;
	  Tcl_DStringAppend(&ds, &readchar, 1);
	}

      read = getc(fileptr);

      if(read == EOF)
	{
	  break;
	}

      if((char)read == '\\')
	{
	  peek = getc(fileptr);

	  if(peek == EOF)
	    {
	      break;
	    }

	  if((char)peek == '\n' || (char)peek == '\r')
	    {
	      continuation = AY_TRUE;
	    }

	  ungetc(peek, fileptr);
	}

      if((char)read == '\n' || (char)read == '\r')
	{
	  if(continuation)
	    {
	      continuation = AY_FALSE;
	    }
	  else
	    {
	      break;
	    }
	}
    } /* while */

  /* add some white space to the string, just in case we
     left the loop prematurely and the string is still empty */
  Tcl_DStringAppend(&ds, " ", -1);

  str = Tcl_DStringValue(&ds);

  switch(str[0])
    {
    case '#':
      break;
    case 'c':
      if(str[1] == 's')
	ay_status = objio_readcstype(str);
      if(str[1] == 'u')
	ay_status = objio_readcurv(str);
      break;
    case 'd':
      ay_status = objio_readdeg(str);
      break;
    case 'e':
      ay_status = objio_readend();
      break;
    case 'f':
      ay_status = objio_readface(str, lastlinewasface);
      break;
    case 'h':
      ay_status = objio_readtrim(str, AY_TRUE);
      break;
    case 'l':
      ay_status = objio_readpolyline(str);
      break;
    case 'p':
      ay_status = objio_readparm(str);
      break;
    case 's':
      ay_status = objio_readsurf(str);
      break;
    case 't':
      ay_status = objio_readtrim(str, AY_FALSE);
      break;
    case 'v':
      ay_status = objio_readvertex(str);
      break;
    default:
      break;
    } /* switch */

  if(str[0] == 'f')
    {
      if(ay_status == AY_OK)
	{
	  lastlinewasface = AY_TRUE;
	}
      else
	{
	  if(ay_status == AY_ERROR)
	    {
	      ay_status = AY_OK;
	    }
	}
    }
  else
    {
      if(str[0] == 'u')
	{
	  /* handle ignored usemtl statements */
	  if(lastlinewasface)
	    lastlinewasface = AY_TRUE;
	}
      else
	{
	  if(lastlinewasface)
	    ay_status = objio_readface(NULL, -1);
	  lastlinewasface = AY_FALSE;
	}
    }

  Tcl_DStringFree(&ds);

 return ay_status;
} /* objio_readline */


/* objio_readscene:
 *
 */
int
objio_readscene(char *filename)
{
 int ay_status = AY_OK;
 ay_object *o = ay_root->next;
 FILE *fileptr = NULL;
 char fname[] = "objio_readscene";
 char aname[] = "objio_options", vname1[] = "Progress";
 char vname2[] = "Cancel", *val = NULL;
 char pbuffer[64];
 int lastprog = 0, curprog = 0;
 long fsize;
 unsigned int lineno = 0, lines = 0;

  if(!o)
    return AY_ENULL;

  if(!filename)
    return AY_ENULL;

  if(!(fileptr = fopen(filename, "rb")))
    {
      ay_error(AY_EOPENFILE, fname, filename);
      return AY_ERROR;
    }

  clearerr(fileptr);

  /* prepare a bunch of global variables */

  objio_cstype = -1;
  objio_csrat = AY_FALSE;
  objio_degu = 0;
  objio_degv = 0;
  objio_uknotv = NULL;
  objio_uknotvlen = 0;
  objio_vknotv = NULL;
  objio_vknotvlen = 0;
  objio_curvtrimsurf = 0;
  memset(&objio_ncurve, 0, sizeof(ay_nurbcurve_object));
  memset(&objio_npatch, 0, sizeof(ay_nurbpatch_object));
  objio_texturev = NULL;
  objio_texturevlen = 0;

  objio_trims = NULL;
  objio_nexttrim = &(objio_trims);

  fseek(fileptr, 0L, SEEK_END);
  fsize = ftell(fileptr);
  if(fsize < 0)
    fsize = LONG_MAX;
  rewind(fileptr);

  /* estimate number of lines (the average Wavefront OBJ line has 28 bytes) */
  lines = fsize/28;

  while(!feof(fileptr))
    {
      if((ay_status = objio_readline(fileptr)))
	break;

      lineno++;

      /* calculate new progress value in percent */
      curprog = (int)(lineno*100.0/lines);

      if(curprog > lastprog)
	{
	  sprintf(pbuffer, "%d", curprog);
	  Tcl_SetVar2(ay_interp, aname, vname1, pbuffer,
		     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	  lastprog = curprog;
	}

      /* process all events (update the GUI) every 500 lines */
      if(!fmod(lineno, 500.0))
	{
	  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

	  /* also, check for cancel button */
	  val = Tcl_GetVar2(ay_interp, aname, vname2,
			    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	  if(val && val[0] == '1')
	    {
	      ay_error(AY_EOUTPUT, fname,
	             "Import cancelled! Not all objects may have been read!");
	      break;
	    }
	}

    } /* while */

  if(ferror(fileptr) || (errno != 0))
    {
      ay_error(AY_ERROR, fname, strerror(errno));
    }

  if(fclose(fileptr))
    {
      ay_error(AY_ERROR, fname, strerror(errno));
    }

  /* clean up all vertex buffers */
  ay_status = objio_freevertices();

  /* clean up */
  if(objio_uknotv)
    free(objio_uknotv);

  if(objio_vknotv)
    free(objio_vknotv);

  if(objio_ncurve.controlv)
    free(objio_ncurve.controlv);

  if(objio_npatch.controlv)
    free(objio_npatch.controlv);

  if(objio_texturev)
    free(objio_texturev);
  objio_texturev = NULL;

  if(objio_trims)
    (void)ay_object_deletemulti(objio_trims, AY_FALSE);

  objio_trims = NULL;
  objio_nexttrim = &(objio_trims);

  /* clean up trims buffer */
  objio_freetrims();

  /* reset lastlinewasface state in objio_readline() */
  objio_readline(NULL);

 return ay_status;
} /* objio_readscene */


/* objio_readscenetcmd:
 *
 */
int
objio_readscenetcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
 int i = 2;

  /* check args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "filename");
      return TCL_OK;
    }

  objio_mergecfaces = AY_TRUE;
  objio_mergepvtags = AY_TRUE;
  objio_readcurves = AY_TRUE;
  objio_rescaleknots = 0.0;
  objio_checkdegen = AY_TRUE;
  objio_readstrim = AY_TRUE;

  while(i+1 < argc)
    {
      if(!strcmp(argv[i], "-m"))
	{
	  sscanf(argv[i+1], "%d", &objio_mergecfaces);
	}
      else
      if(!strcmp(argv[i], "-c"))
	{
	  sscanf(argv[i+1], "%d", &objio_readcurves);
	}
      else
      if(!strcmp(argv[i], "-p"))
	{
	  sscanf(argv[i+1], "%d", &objio_mergepvtags);
	}
      else
      if(!strcmp(argv[i], "-r"))
	{
	  sscanf(argv[i+1], "%lg", &objio_rescaleknots);
	}
      else
      if(!strcmp(argv[i], "-f"))
	{
	  sscanf(argv[i+1], "%lg", &objio_scalefactor);
	}
      else
      if(!strcmp(argv[i], "-s"))
	{
	  sscanf(argv[i+1], "%d", &objio_readstrim);
	}
      else
      if(!strcmp(argv[i], "-d"))
	{
	  sscanf(argv[i+1], "%d", &objio_checkdegen);
	}

      i += 2;
    } /* while */

  ay_status = objio_readscene(argv[1]);

  objio_scalefactor = 1.0;

 return TCL_OK;
} /* objio_readscenetcmd */


/* Objio_Init:
 *  initialize Wavefront OBJ Import/Export plugin
 *  note: this function _must_ be capitalized exactly this way
 *  regardless of the filename of the shared object (see: man n load)!
 */
#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int
Objio_Init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;
 char fname[] = "objio_init";

  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }

  if(ay_checkversion(fname, objio_version_ma, objio_version_mi))
    {
      return TCL_ERROR;
    }

  /* init hash table for write callbacks */
  Tcl_InitHashTable(&objio_write_ht, TCL_ONE_WORD_KEYS);

  /* fill hash table */
  ay_status = objio_registerwritecb((char *)(AY_IDNPATCH),
				       objio_writenpatch);
  ay_status = objio_registerwritecb((char *)(AY_IDNCURVE),
				       objio_writencurve);
  ay_status = objio_registerwritecb((char *)(AY_IDLEVEL),
				       objio_writelevel);
  ay_status = objio_registerwritecb((char *)(AY_IDCLONE),
				       objio_writeclone);
  ay_status = objio_registerwritecb((char *)(AY_IDMIRROR),
				       objio_writeclone);
  ay_status = objio_registerwritecb((char *)(AY_IDINSTANCE),
				       objio_writeinstance);
  ay_status = objio_registerwritecb((char *)(AY_IDSCRIPT),
				       objio_writescript);

  ay_status = objio_registerwritecb((char *)(AY_IDACURVE),
				       objio_writencconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDICURVE),
				       objio_writencconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDCONCATNC),
				       objio_writencconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDEXTRNC),
				       objio_writencconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDNCIRCLE),
				       objio_writencconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDOFFNC),
				       objio_writencconvertible);

  ay_status = objio_registerwritecb((char *)(AY_IDEXTRUDE),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDREVOLVE),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDSWEEP),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDSWING),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDSKIN),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDCAP),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDPAMESH),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDBPATCH),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDGORDON),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDBIRAIL1),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDBIRAIL2),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDTEXT),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDBEVEL),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDEXTRNP),
				       objio_writenpconvertible);

  ay_status = objio_registerwritecb((char *)(AY_IDSPHERE),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDDISK),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDCYLINDER),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDCONE),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDHYPERBOLOID),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDPARABOLOID),
				       objio_writenpconvertible);
  ay_status = objio_registerwritecb((char *)(AY_IDTORUS),
				       objio_writenpconvertible);

  ay_status = objio_registerwritecb((char *)(AY_IDPOMESH),
				       objio_writepomesh);

  ay_status = objio_registerwritecb((char *)(AY_IDBOX),
				       objio_writebox);

  Tcl_CreateCommand(interp, "objioWrite", objio_writescenetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "objioRead", objio_readscenetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* source objio.tcl, it contains vital Tcl-code */
  if((Tcl_EvalFile(interp, "objio.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname, "Error while sourcing \"objio.tcl\"!");
       return TCL_OK;
     }

  ay_error(AY_EOUTPUT, fname, "Plugin 'objio' successfully loaded.");

 return TCL_OK;
} /* Objio_Init */
