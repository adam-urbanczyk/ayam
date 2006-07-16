/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2006 by Randolf Schultz
 * (rschultz@informatik.uni-rostock.de) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

/* objio.c - Wavefront OBJ Input/Output module */

#include "ayam.h"
#include <ctype.h>

/* types local to this module */

typedef int (ay_objio_writecb) (FILE *fileptr, ay_object *o, double *m);


/* prototypes of functions local to this module */

int ay_objio_registerwritecb(char *name, ay_objio_writecb *cb);

int ay_objio_writevertices(FILE *fileptr, unsigned int n, int stride,
			   double *v);

int ay_objio_writetvertices(FILE *fileptr, unsigned int n, int stride,
			    double *v);

int ay_objio_writencurve(FILE *fileptr, ay_object *o, double *m);

int ay_objio_writetcurve(FILE *fileptr, ay_object *o, double *m);

int ay_objio_writetrim(FILE *fileptr, ay_object *o);

int ay_objio_writetrimids(FILE *fileptr, ay_object *o);

int ay_objio_writenpatch(FILE *fileptr, ay_object *o, double *m);

int ay_objio_writelevel(FILE *fileptr, ay_object *o, double *m);

int ay_objio_writencconvertible(FILE *fileptr, ay_object *o, double *m);

int ay_objio_writenpconvertible(FILE *fileptr, ay_object *o, double *m);

int ay_objio_writebox(FILE *fileptr, ay_object *o, double *m);

int ay_objio_writepomesh(FILE *fileptr, ay_object *o, double *m);

int ay_objio_writeclone(FILE *fileptr, ay_object *o, double *m);

int ay_objio_writeinstance(FILE *fileptr, ay_object *o, double *m);

int ay_objio_writescript(FILE *fileptr, ay_object *o, double *m);

int ay_objio_writeobject(FILE *fileptr, ay_object *o, int writeend);

int ay_objio_writescenetcmd(ClientData clientData, Tcl_Interp *interp,
			    int argc, char *argv[]);


/* global variables */

static double tm[16] = {0}; /* current transformation matrix */

static double objio_scalefactor = 1.0;

static Tcl_HashTable ay_objio_write_ht; /* write callbacks */

static int objio_tesspomesh = AY_FALSE;

static int objio_writecurves = AY_TRUE;

char objio_stagnamedef[] = "mys";
char *objio_stagname = objio_stagnamedef;
char objio_ttagnamedef[] = "myt";
char *objio_ttagname = objio_ttagnamedef;


/* functions */

/* ay_objio_registerwritecb:
 *
 */
int
ay_objio_registerwritecb(char *name, ay_objio_writecb *cb)
{
 int ay_status = AY_OK;
 int new_item = 0;
 Tcl_HashEntry *entry = NULL;
 Tcl_HashTable *ht = &ay_objio_write_ht;

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
} /* ay_objio_registerwritecb */


/* ay_objio_writevertices:
 *  write <n> <stride>D-texturevertices from array <v[n*stride]> to
 *  file <fileptr>
 */
int
ay_objio_writevertices(FILE *fileptr, unsigned int n, int stride, double *v)
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
} /* ay_objio_writevertices */


/* ay_objio_writetvertices:
 *  write <n> <stride>D-vertices from array <v[n*stride]> to file <fileptr>
 */
int
ay_objio_writetvertices(FILE *fileptr, unsigned int n, int stride, double *v)
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
} /* ay_objio_writetvertices */


/* ay_objio_writencurve:
 *
 */
int
ay_objio_writencurve(FILE *fileptr, ay_object *o, double *m)
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
  ay_objio_writevertices(fileptr, (unsigned int)nc->length,
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
} /* ay_objio_writencurve */


/* ay_objio_writetcurve:
 *  write a single trim curve
 */
int
ay_objio_writetcurve(FILE *fileptr, ay_object *o, double *m)
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
} /* ay_objio_writetcurve */


/* ay_objio_writetrim:
 *  write all trim curves pointed to by <o>
 */
int
ay_objio_writetrim(FILE *fileptr, ay_object *o)
{
 double mi[16] = {0};
 ay_object *down = NULL;

  ay_trafo_identitymatrix(mi);

  while(o->next)
    {
      if(o->type == AY_IDNCURVE)
	{
	  ay_objio_writetcurve(fileptr, o, mi);
	}

      if((o->type == AY_IDLEVEL) && (o->down) && (o->down->next))
	{
	  ay_trafo_creatematrix(o, mi);
	  down = o->down;
	  while(down->next)
	    {
	      if(down->type == AY_IDNCURVE)
		{
		  ay_objio_writetcurve(fileptr, down, mi);
		}
	      down = down->next;
	    } /* while */
	  ay_trafo_identitymatrix(mi);
	} /* if */

      o = o->next;
    } /* while */

 return AY_OK;
} /* ay_objio_writetrim */


/* ay_objio_writetrimids:
 *  write ids of all trim curves pointed to by <o>
 */
int
ay_objio_writetrimids(FILE *fileptr, ay_object *o)
{
 ay_object *o2 = o, *down = NULL;
 ay_nurbcurve_object *nc = NULL;
 double umin, umax, orient;
 int tc = 0, hole;

  /* first, count trim curves */
  while(o->next)
    {
      if(o->type == AY_IDNCURVE)
	{
	  tc++;
	}

      if((o->type == AY_IDLEVEL) && (o->down) && (o->down->next))
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

      o = o->next;
    } /* while */

  /* now write the ids */
  o = o2;
  while(o->next)
    {
      if(o->type == AY_IDNCURVE)
	{

	  nc = (ay_nurbcurve_object *)o->refine;
	  ay_nct_getorientation(nc, &orient);
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
	}

      if((o->type == AY_IDLEVEL) && (o->down) && (o->down->next))
	{
	  down = o->down;
	  if(down->type == AY_IDNCURVE)
	    {
	      nc = (ay_nurbcurve_object *)down->refine;
	      ay_nct_getorientation(nc, &orient);
	      if(orient < 0.0)
		fprintf(fileptr, "hole ");
	      else
		fprintf(fileptr, "trim ");
	    }

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
	    }
	  fprintf(fileptr, "\n");

	} /* if */

      o = o->next;
    } /* while */

 return AY_OK;
} /* ay_objio_writetrimids */


/* ay_objio_writenpatch:
 *
 */
int
ay_objio_writenpatch(FILE *fileptr, ay_object *o, double *m)
{
 int ay_status = AY_OK;
 ay_nurbpatch_object *np;
 double *v = NULL, *p1, *p2, pw[3];
 double umin, umax, vmin, vmax;
 int stride = 4, i, j;
 int have_mys = AY_FALSE, have_myt = AY_FALSE;
 unsigned int myslen = 0, mytlen = 0, mystlen = 0, ui, uj;
 double *mysarr = NULL, *mytarr = NULL, *mystarr = NULL;
 ay_tag_object mystag = {NULL, 0, NULL};
 ay_tag_object myttag = {NULL, 0, NULL};
 ay_tag_object *tag;

  if(!o)
    return AY_ENULL;

  mystag.type = ay_pv_tagtype;
  myttag.type = ay_pv_tagtype;

  /* first, check for and write the trim curves */
  if(o->down && o->down->next)
    {
      ay_status = ay_objio_writetrim(fileptr, o->down);
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
  ay_objio_writevertices(fileptr, (unsigned int)(np->width * np->height),
			 (np->is_rat?4:3), v);

  /* write texture coordinates from potentially present PV tags */
  if(o->tags)
    {
      if(!(mystag.val = calloc(strlen(objio_stagname)+2,sizeof(char))))
	return AY_EOMEM;
      if(!(myttag.val = calloc(strlen(objio_ttagname)+2,sizeof(char))))
	return AY_EOMEM;
      strcpy(mystag.val, objio_stagname);
      mystag.val[strlen(objio_stagname)] = ',';
      strcpy(myttag.val, objio_ttagname);
      myttag.val[strlen(objio_ttagname)] = ',';
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

      ay_objio_writetvertices(fileptr, mystlen, 2, mystarr);

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
      ay_objio_writetrimids(fileptr, o->down);
    } /* if */

  free(v);

 return AY_OK;
} /* ay_objio_writenpatch */


/* ay_objio_writelevel:
 *
 */
int
ay_objio_writelevel(FILE *fileptr, ay_object *o, double *m)
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
      while(down->next)
	{
	  ay_status = ay_objio_writeobject(fileptr, down, AY_TRUE);
	  down = down->next;
	}
      memcpy(tm, m1, 16*sizeof(double));
    }

 return AY_OK;
} /* ay_objio_writelevel */


/* ay_objio_writencconvertible:
 *
 */
int
ay_objio_writencconvertible(FILE *fileptr, ay_object *o, double *m)
{
 int ay_status = AY_OK;
 ay_object *c = NULL, *t;

  if(!objio_writecurves)
    return AY_OK;

  if(!o)
   return AY_ENULL;

  ay_status = ay_provide_object(o, AY_IDNCURVE, &c);
  t = c;
  while(t->next)
    {
      if(t->type == AY_IDNCURVE)
	{
	  ay_status = ay_objio_writeobject(fileptr, t, AY_TRUE);
	}

      t = t->next;
    } /* while */

  if(t->type == AY_IDNCURVE)
    {
      ay_status = ay_objio_writeobject(fileptr, t, AY_FALSE);
    }

  ay_status = ay_object_deletemulti(c);

 return ay_status;
} /* ay_objio_writencconvertible */


/* ay_objio_writenpconvertible:
 *
 */
int
ay_objio_writenpconvertible(FILE *fileptr, ay_object *o, double *m)
{
 int ay_status = AY_OK;
 ay_object *p = NULL, *t;

  if(!o)
   return AY_ENULL;

  ay_status = ay_provide_object(o, AY_IDNPATCH, &p);
  t = p;
  while(t->next)
    {
      if(t->type == AY_IDNPATCH)
	{
	  ay_status = ay_objio_writeobject(fileptr, t, AY_TRUE);
	}

      t = t->next;
    } /* while */

  if(t->type == AY_IDNPATCH)
    {
      ay_status = ay_objio_writeobject(fileptr, t, AY_FALSE);
    }

  ay_status = ay_object_deletemulti(p);

 return ay_status;
} /* ay_objio_writenpconvertible */


/* ay_objio_writebox:
 *
 */
int
ay_objio_writebox(FILE *fileptr, ay_object *o, double *m)
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
  ay_objio_writevertices(fileptr, 8, 3, v);

  /* write faces */
  fprintf(fileptr,"f -8 -7 -6 -5\n");
  fprintf(fileptr,"f -4 -3 -2 -1\n");
  fprintf(fileptr,"f -8 -7 -4 -3\n");
  fprintf(fileptr,"f -6 -5 -2 -1\n");
  fprintf(fileptr,"f -8 -6 -4 -2\n");
  fprintf(fileptr,"f -7 -5 -3 -1\n");

 return AY_OK;
} /* ay_objio_writebox */


/* ay_objio_writepomesh:
 *
 */
int
ay_objio_writepomesh(FILE *fileptr, ay_object *o, double *m)
{
 int ay_status = AY_OK;
 /*char fname[] = "objio_writepomesh";*/
 ay_object *to = NULL;
 ay_list_object *li = NULL, **nextli = NULL, *lihead = NULL;
 ay_pomesh_object *po;
 double v[3], *p1;
 int stride;
 unsigned int i, j, k, p = 0, q = 0, r = 0;
 int have_mys = AY_FALSE, have_myt = AY_FALSE;
 unsigned int myslen = 0, mytlen = 0, mystlen = 0;
 double *mysarr = NULL, *mytarr = NULL, *mystarr = NULL;
 ay_tag_object mystag = {NULL, 0, NULL};
 ay_tag_object myttag = {NULL, 0, NULL};
 ay_tag_object *tag;

  if(!o)
   return AY_ENULL;

  mystag.type = ay_pv_tagtype;
  mystag.name = objio_stagname;
  myttag.type = ay_pv_tagtype;
  myttag.name = objio_ttagname;

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
      if(!(mystag.val = calloc(strlen(objio_stagname)+2,sizeof(char))))
	return AY_EOMEM;
      if(!(myttag.val = calloc(strlen(objio_ttagname)+2,sizeof(char))))
	return AY_EOMEM;
      strcpy(mystag.val, objio_stagname);
      mystag.val[strlen(objio_stagname)] = ',';
      strcpy(myttag.val, objio_ttagname);
      myttag.val[strlen(objio_ttagname)] = ',';
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

      ay_objio_writetvertices(fileptr, mystlen, 2, mystarr);

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
	      if(!objio_tesspomesh ||
		 (objio_tesspomesh && (po->nverts[q] == 3)))
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
	  ay_objio_writepomesh(fileptr, to, m);
	  ay_object_delete(to);
	}
    }
  else
    {
      if(lihead)
	ay_objio_writepomesh(fileptr, lihead->object, m);
    } /* if */

  while(lihead)
    {
      ay_object_delete(lihead->object);
      li = lihead->next;
      free(lihead);
      lihead = li;
    } /* while */

 return AY_OK;
} /* ay_objio_writepomesh */


/* ay_objio_writeclone:
 *
 */
int
ay_objio_writeclone(FILE *fileptr, ay_object *o, double *m)
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
      ay_status = ay_objio_writeobject(fileptr, clone, AY_TRUE);

      clone = clone->next;
    }

 return ay_status;
} /* ay_objio_writeclone */


/* ay_objio_writeinstance:
 *
 */
int
ay_objio_writeinstance(FILE *fileptr, ay_object *o, double *m)
{
 int ay_status = AY_OK;
 ay_object *orig, tmp = {0};

  if(!o || !o->refine)
   return AY_ENULL;

  orig = (ay_object *)o->refine;

  ay_trafo_copy(orig, &tmp);
  ay_trafo_copy(o, orig);
  ay_status = ay_objio_writeobject(fileptr, orig, AY_FALSE);
  ay_trafo_copy(&tmp, orig);

 return ay_status;
} /* ay_objio_writeinstance */


/* ay_objio_writescript:
 *
 */
int
ay_objio_writescript(FILE *fileptr, ay_object *o, double *m)
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
      while(cmo)
	{
	  ay_status = ay_objio_writeobject(fileptr, cmo, AY_TRUE);
	  cmo = cmo->next;
	}
    } /* if */

 return ay_status;
} /* ay_objio_writescript */


#if 0
/* ay_objio_writencurve:
 *
 */
int
ay_objio_writencurve(FILE *fileptr, ay_object *o, double *m)
{
 ay_nurbcurve_object *nc;
 return AY_OK;
} /* ay_objio_writencurve */
#endif


/* ay_objio_writeobject:
 *
 */
int
ay_objio_writeobject(FILE *fileptr, ay_object *o, int writeend)
{
 int ay_status = AY_OK;
 char fname[] = "objio_writeobject";
 Tcl_HashTable *ht = &ay_objio_write_ht;
 Tcl_HashEntry *entry = NULL;
 double m1[16] = {0}, m2[16];
 char err[255];
 ay_objio_writecb *cb = NULL;

  if(!o)
    return AY_ENULL;

  if((entry = Tcl_FindHashEntry(ht, (char *)(o->type))))
    {
      cb = (ay_objio_writecb*)Tcl_GetHashValue(entry);
      if(cb)
	{
	  if(o->name && (strlen(o->name)>1))
	    fprintf(fileptr, "o %s\n", o->name);

	  if((o->movx != 0.0) || (o->movy != 0.0) || (o->movz != 0.0) ||
	     (o->rotx != 0.0) || (o->roty != 0.0) || (o->rotz != 0.0) ||
	     (o->scalx != 1.0) || (o->scaly != 1.0) || (o->scalz != 1.0) ||
	     (o->quat[0] != 0.0) || (o->quat[1] != 0.0) ||
	     (o->quat[2] != 0.0) || (o->quat[3] != 1.0))
	    {
	      ay_trafo_creatematrix(o, m1);
	      memcpy(m2, tm, 16*sizeof(double));
	      ay_trafo_multmatrix4(m2, m1);

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
	  if(writeend)
	    fprintf(fileptr, "end\n");
	}
    }
  else
    {
      sprintf(err, "Cannot export objects of type: %s.",
	      ay_object_gettypename(o->type));
      ay_error(AY_EWARN, fname, err);
    } /* if */

 return AY_OK;
} /* ay_objio_writeobject */


/* ay_objio_writescene:
 *
 */
int
ay_objio_writescene(char *filename, int selected)
{
 int ay_status = AY_OK;
 ay_object *o = ay_root->next;
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

  /* write header information */
  /*  ay_objio_writeheader(fileptr);*/

  /* omit EndLevel-object in top level! */
  while(o->next)
    {
      if(selected)
	{
	  if(o->selected)
	    {
	      ay_status = ay_objio_writeobject(fileptr, o, AY_TRUE);
	    }
	}
      else
	{
	  ay_status = ay_objio_writeobject(fileptr, o, AY_TRUE);
	}

      if(ay_status)
	{ fclose(fileptr); return ay_status; }

      o = o->next;
    } /* while */

  if(ferror(fileptr) || errno != 0)
    {
      ay_error(AY_ERROR, fname, strerror(errno));
    }

  if(fclose(fileptr))
    {
      ay_error(AY_ERROR, fname, strerror(errno));
    }

 return ay_status;
} /* ay_objio_writescene */


/* ay_objio_writescenetcmd:
 *
 */
int
ay_objio_writescenetcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "ay_objio_write";
 int selected = AY_FALSE, i = 2;

  /* check args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "filename");
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
      if(!strcmp(argv[i], "-t"))
	{
	  objio_stagname = argv[i+1];
	  objio_ttagname = argv[i+2];
	  i++;
	}
      else
      if(!strcmp(argv[i], "-f"))
	{
	  sscanf(argv[i+1], "%lg", &objio_scalefactor);
	}
      i += 2;
    } /* while */

  ay_status = ay_objio_writescene(argv[1], selected);

  objio_stagname = objio_stagnamedef;
  objio_ttagname = objio_ttagnamedef;

  objio_scalefactor = 1.0;

 return TCL_OK;
} /* ay_objio_writescenetcmd */


/****************************************************************/

static int objio_readcurves;

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

objio_vertex *objio_gverts_head = NULL;
objio_vertex *objio_gverts_tail = NULL;

objio_vertex *objio_nverts_head = NULL;
objio_vertex *objio_nverts_tail = NULL;

objio_vertex *objio_pverts_head = NULL;
objio_vertex *objio_pverts_tail = NULL;

objio_vertex *objio_tverts_head = NULL;
objio_vertex *objio_tverts_tail = NULL;

int objio_cstype; /* -1 - unset, 0 - bmatrix, 1 - bezier, 2 - bspline,
		     3 - cardinal, 4 - taylor */
int objio_csrat;
int objio_degu;
int objio_degv;

double *objio_uknotv;
int objio_uknotvlen;
double *objio_vknotv;
int objio_vknotvlen;

int objio_curvtrimsurf; /* 0 - unset, 1 - curve, 2 - trim, 3 - surface */

ay_nurbcurve_object objio_ncurve;
ay_nurbpatch_object objio_npatch;

double *objio_texturesv;
double *objio_texturetv;
int objio_texturesvlen;
int objio_texturetvlen;

double objio_umin;
double objio_umax;
double objio_vmin;
double objio_vmax;

typedef struct objio_trim_s {
  struct objio_trim_s *next;
  struct objio_trim_s *prev;

  unsigned int index;

  ay_object *c;

} objio_trim;

objio_trim *objio_trims_head = NULL;
objio_trim *objio_trims_tail = NULL;

ay_object *objio_trims;
ay_object **objio_nexttrim;

ay_object *objio_lastface;


int ay_objio_addvertex(int type, double *v);

int ay_objio_getvertex(int type, unsigned int index, double **v);

int ay_objio_freevertices(void);

int ay_objio_readvertex(char *str);

int ay_objio_readvindex(char *c, int *gvindex, int *tvindex, int *nvindex);

int ay_objio_readskip(char **b);

int ay_objio_readface(char *str, int lastlinewasface);

int ay_objio_readcstype(char *str);

int ay_objio_readdeg(char *str);

int ay_objio_readcurv(char *str);

int ay_objio_readsurf(char *str);

int ay_objio_readparm(char *str);

int ay_objio_addtrim(ay_object *o);

int ay_objio_gettrim(unsigned int index, ay_object **t);

int ay_objio_freetrims(void);

int ay_objio_readtrim(char *str, int hole);

int ay_objio_fixnpatch(ay_nurbpatch_object *np);

int ay_objio_fixncurve(ay_nurbcurve_object *nc);

int ay_objio_readend(void);

int ay_objio_readline(FILE *fileptr);

int ay_objio_readscene(char *filenam);

int ay_objio_readscenetcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);


/* ay_objio_addvertex:
 *  add a vertex to a vertex buffer
 */
int
ay_objio_addvertex(int type, double *v)
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
} /* ay_objio_addvertex */


/* ay_objio_getvertex:
 *  get a vertex from a vertex buffer
 */
int
ay_objio_getvertex(int type, unsigned int index, double **v)
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
} /* ay_objio_getvertex */


/* ay_objio_freevertices:
 *  free all vertex buffers
 */
int
ay_objio_freevertices(void)
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

  /* clear cached "current" pointers in ay_objio_getvertex() */
  ay_status = ay_objio_getvertex(0, 0, NULL);

 return AY_OK;
} /* ay_objio_freevertices */


/* ay_objio_readvertex:
 *  read a single vertex and add it to the appropriate vertex buffer
 */
int
ay_objio_readvertex(char *str)
{
 int ay_status = AY_OK;
 double v[4] = {0};

  if(strlen(str) < 2)
    return AY_ERROR;

  if(str[1] == 'n')
    {
      sscanf(&(str[2])," %lg %lg %lg", &(v[0]), &(v[1]), &(v[2]));
      ay_status = ay_objio_addvertex(2, v);
    }
  else
  if(str[1] == 'p')
    {
      if(sscanf(&(str[2])," %lg %lg %lg", &(v[0]), &(v[1]), &(v[3])) < 3)
	{
	  v[3] = 1.0;
	}
      ay_status = ay_objio_addvertex(3, v);
    }
  else
  if(str[1] == 't')
    {
      sscanf(&(str[2])," %lg %lg %lg", &(v[0]), &(v[1]), &(v[2]));
      ay_status = ay_objio_addvertex(4, v);
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
      ay_status = ay_objio_addvertex(1, v);
    } /* if */

 return ay_status;
} /* ay_objio_readvertex */


/* ay_objio_readvindex:
 *  read a single vertex index of the form "g/t/n", where g is the
 *  index of the geometric vertex, t the texture vertex, and n the
 *  corresponding normal; returns results in gvindex, tvindex, and nvindex
 */
int
ay_objio_readvindex(char *c, int *gvindex, int *tvindex, int *nvindex)
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
} /* ay_objio_readvindex */


/* ay_objio_readskip:
 *  skip over a number
 *  !Modifies argument <b>!
 */
int
ay_objio_readskip(char **b)
{
 char *c = *b, *d;

  if(!b)
    return AY_ENULL;

  while((isspace(*c)) && (*c != '\0'))
    c++;
  if(*c == '\0')
    return AY_OK;
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
	  return AY_OK;
	} /* if */
    } /* if */

  /* return result */
  *b = c;

 return AY_OK;
} /* ay_objio_readskip */


/* ay_objio_readface:
 *  read a face statement (polygonal face)
 */
int
ay_objio_readface(char *str, int lastlinewasface)
{
 int ay_status = AY_OK;
 char fname[] = "objio_readface";
 char *c = NULL;
 int gvindex = 0, tvindex = 0, nvindex = 0, stride = 0, degen = AY_FALSE;
 int last_stride = 0;
 double *gv, *nv, *tv, *oldpnt;
 double *newcontrolv = NULL, *texsv = NULL, *textv = NULL, *tmpv = NULL;
 int texsvlen = 0, textvlen = 0;
 ay_pomesh_object po = {0}, *temppo;
 ay_tag_object *temptags;
 ay_object t = {0}, *o = NULL, *m = NULL;
 unsigned int nloops = 1, nverts = 0, i;
 static ay_list_object *storedfaces = NULL, **laststoredface = &storedfaces;
 static unsigned int nfaces = 0;
 ay_list_object *tl = NULL;
 double angle, v1[3] = {0}, v2[3] = {0}, *v;

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
      ay_status = ay_objio_readvindex(c, &gvindex, &tvindex, &nvindex);

      gv = NULL;
      nv = NULL;
      tv = NULL;

      /* get geometric vertex data and add it to the pomesh */
      if(gvindex < 0)
	{
	  if(objio_gverts_tail)
	    {
	      ay_status = ay_objio_getvertex(1,
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
	  ay_status = ay_objio_getvertex(1, gvindex, &gv);
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
		  ay_status = ay_objio_getvertex(2,
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
	      ay_status = ay_objio_getvertex(2, nvindex, &nv);
	    } /* if */
	  if(ay_status)
	    goto cleanup;
	} /* if */

      if(tvindex != 0)
	{
	  /* get texture vertex data and cach it in texsv/textv */

	  tv = NULL;
	  if(tvindex < 0)
	    {
	      if(objio_tverts_tail)
		{
		  ay_status = ay_objio_getvertex(4,
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
	      ay_status = ay_objio_getvertex(4, tvindex, &tv);
	    } /* if */

	  if(tv)
	    {
	      if(!(tmpv = realloc(texsv, (texsvlen + 1) * sizeof(double))))
		{ ay_status = AY_EOMEM; goto cleanup; }
	      texsv = tmpv;
	      memcpy(&(texsv[texsvlen]), tv, sizeof(double));
	      texsvlen++;

	      if(!(tmpv = realloc(textv, (textvlen + 1) * sizeof(double))))
		{ ay_status = AY_EOMEM; goto cleanup; }
	      textv = tmpv;
	      memcpy(&(textv[textvlen]), &(tv[1]), sizeof(double));
	      textvlen++;
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
	      if(AY_COMP3DP(oldpnt,gv))
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
	      ay_status = ay_objio_readskip(&c);
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
      ay_status = ay_objio_readskip(&c);
    } /* while */

  if(nverts >= 3)
    {
      if(nvindex != 0)
	{
	  po.has_normals = AY_TRUE;
	}
      /*
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

      if(texsv)
	{
	  ay_status = ay_pv_add(&t, objio_stagname, "varying", 0,
				texsvlen, texsv);
	}

      if(textv)
	{
	  ay_status = ay_pv_add(&t, objio_ttagname, "varying", 0,
				textvlen, textv);
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

	  ay_status = ay_object_link(o);

	  objio_lastface = o;
	} /* if */
    } /* if */

cleanup:
  if(po.controlv)
    free(po.controlv);

  if(po.verts)
    free(po.verts);

  if(texsv)
    free(texsv);

  if(textv)
    free(textv);

  if(t.tags)
    ay_status = ay_tags_delall(&t);

  /*
  if(degen)
    {
      ay_objio_readface(NULL, -1);
      return AY_ERROR;
    }
  */
 return ay_status;
} /* ay_objio_readface */


/* ay_objio_readcstype:
 *  read a cstype statement signifying the type of the next curve/surface
 */
int
ay_objio_readcstype(char *str)
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
} /* ay_objio_readcstype */


/* ay_objio_readdeg:
 *  read a deg (degree) statement
 */
int
ay_objio_readdeg(char *str)
{
 int ay_status = AY_OK;
 char *c = str;

  if(!str)
    return AY_ENULL;

  while(!(isspace(*c)) && (*c != '\0'))
    c++;

  sscanf(c," %d %d", &objio_degu, &objio_degv);

 return ay_status;
} /* ay_objio_readdeg */


/* ay_objio_readcurv:
 *  read a curv (freeform curve) statement
 */
int
ay_objio_readcurv(char *str)
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
      ay_status = ay_objio_readskip(&c);
      ay_status = ay_objio_readskip(&c);
    }

  /* read (and resolve) control point indices */
  while(*c != '\0')
    {
      gvindex = 0; tvindex = 0; nvindex = 0;
      ay_status = ay_objio_readvindex(c, &gvindex, &tvindex, &nvindex);
      gv = NULL;

      if(gvindex < 0)
	{
	  if(vtype == 1)
	    {
	      if(objio_gverts_tail)
		{
		  ay_status = ay_objio_getvertex(vtype,
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
		  ay_status = ay_objio_getvertex(vtype,
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
	  ay_status = ay_objio_getvertex(vtype, gvindex, &gv);
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
      ay_status = ay_objio_readskip(&c);
    } /* while */

cleanup:

 return ay_status;
} /* ay_objio_readcurv */


/* ay_objio_readsurf:
 *  read a surf (freeform surface) statement
 */
int
ay_objio_readsurf(char *str)
{
 int ay_status = AY_OK;
 char *c = str;
 int gvindex = 0, tvindex = 0, nvindex = 0, stride = 4;
 int glength = 0, tlength = 0;
 double *gv, *controlv = NULL, *tv, *texturesv = NULL, *texturetv = NULL;

  if(!str)
    return AY_ENULL;

  while(!(isspace(*c)) && (*c != '\0'))
    c++;

  /* read umin/umax */
  sscanf(c, " %lg %lg", &objio_umin, &objio_umax);
  ay_status = ay_objio_readskip(&c);
  ay_status = ay_objio_readskip(&c);

  /* read vmin/vmax */
  sscanf(c, " %lg %lg", &objio_vmin, &objio_vmax);
  ay_status = ay_objio_readskip(&c);
  ay_status = ay_objio_readskip(&c);

  /* read (and resolve) control point indices */
  while(*c != '\0')
    {
      gvindex = 0; tvindex = 0; nvindex = 0;
      ay_status = ay_objio_readvindex(c, &gvindex, &tvindex, &nvindex);

      gv = NULL;
      if(gvindex < 0)
	{
	  if(objio_gverts_tail)
	    {
	      ay_status = ay_objio_getvertex(1,
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
	  ay_status = ay_objio_getvertex(1, gvindex, &gv);
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
		  ay_status = ay_objio_getvertex(4,
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
	      ay_status = ay_objio_getvertex(4, tvindex, &tv);
	    } /* if */

	  if(tv)
	    {

	      if(!(texturesv = realloc(objio_texturesv,
				       (tlength + 1) * sizeof(double))))
		{ return AY_EOMEM; }
	      objio_texturesv = texturesv;
	      memcpy(&(texturesv[tlength]), tv, sizeof(double));
	      if(!(texturetv = realloc(objio_texturetv,
				       (tlength + 1) * sizeof(double))))
		{ return AY_EOMEM; }
	      objio_texturetv = texturetv;
	      memcpy(&(texturetv[tlength]), &(tv[1]), sizeof(double));
	      tlength++;
	    } /* if */
	} /* if */

      /* skip to next vindex */
      ay_status = ay_objio_readskip(&c);
    } /* while */

  objio_curvtrimsurf = 3;

  objio_texturesvlen = tlength;
  objio_texturetvlen = tlength;

cleanup:

 return ay_status;
} /* ay_objio_readsurf */


/* ay_objio_readparm:
 *  read a knot vector
 */
int
ay_objio_readparm(char *str)
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

  ay_status = ay_objio_readskip(&c);

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
      ay_status = ay_objio_readskip(&c);
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
} /* ay_objio_readparm */


/* ay_objio_addtrim:
 *  add trim curve <o> to the trim curve buffer
 */
int
ay_objio_addtrim(ay_object *o)
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
} /* ay_objio_addtrim */


/* ay_objio_gettrim:
 *  get a trim curve object from the trim curve buffer
 */
int
ay_objio_gettrim(unsigned int index, ay_object **t)
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
} /* ay_objio_gettrim */


/* ay_objio_freetrims:
 *  free trim curve buffer
 */
int
ay_objio_freetrims(void)
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
	  ay_object_delete(tt->c);
	  free(tt);
	  tt = t;
	} /* while */
      objio_trims_tail = NULL;
    } /* if */

  /* clear cached "current" pointers in ay_objio_gettrim() */
  ay_status = ay_objio_gettrim(0, NULL);

 return AY_OK;
} /* ay_objio_freetrims */


/* ay_objio_readtrim:
 *  read a trim statement (trim loop specification),
 *  get the corresponding trim curves from the trim curve
 *  buffer and copy them for connection to the surface
 *  object later on
 */
int
ay_objio_readtrim(char *str, int hole)
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
      ay_status = ay_objio_readskip(&c);
      ay_status = ay_objio_readskip(&c);

      tindex = 0;
      if(sscanf(c, "%d", &tindex))
	{
	  t = NULL;
	  if(tindex < 0)
	    {
	      if(objio_trims_tail)
		{
		  ay_status = ay_objio_gettrim(
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
	      ay_status = ay_objio_gettrim(tindex, &t);
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
		    { ay_status = AY_EOMEM; goto cleanup; }
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
      ay_status = ay_objio_readskip(&c);

    } /* while */

  /* repair nexttrim pointer? */
  if(tcount > 1)
    {
      ay_object_crtendlevel(objio_nexttrim);
      objio_nexttrim = &(l->next);
    }

cleanup:

 return ay_status;
} /* ay_objio_readtrim */


/* ay_objio_fixnpatch:
 *  fix row/column major order in np controlv (from Wavefront to Ayam style);
 *  additionally, multiply the weights in for rational vertices
 *  XXXX to be done: improve the knot vector (type, GLU compat)
 */
int
ay_objio_fixnpatch(ay_nurbpatch_object *np)
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

	  if(p1[3] != 1.0)
	    {
	      p1[0] *= p1[3];
	      p1[1] *= p1[3];
	      p1[2] *= p1[3];
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

 return ay_status;
} /* ay_objio_fixnpatch */


/* ay_objio_fixncurve:
 *  fix a Wavefront NURBS curve by
 *  multiplying the weights in for rational vertices
 *  XXXX to be done: improve the knot vector (type, GLU compat)
 */
int
ay_objio_fixncurve(ay_nurbcurve_object *nc)
{
 int ay_status = AY_OK;
 int i, stride = 4;
 double *p = NULL;

  if(!nc)
    return AY_ENULL;

  p = nc->controlv;

  for(i = 0; i < nc->length; i++)
    {
      if(p[3] != 1.0)
	{
	  p[0] *= p[3];
	  p[1] *= p[3];
	  p[2] *= p[3];
	} /* if */
      p += stride;
    } /* for */

  nc->is_rat = ay_nct_israt(nc);

  if(objio_rescaleknots != 0.0)
    {
      ay_knots_rescaletomindist(nc->length+nc->order, nc->knotv,
				objio_rescaleknots);
    }

 return ay_status;
} /* ay_objio_fixncurve */


/* ay_objio_readend:
 *  read an end statement (realise curve or surface)
 */
int
ay_objio_readend(void)
{
 int ay_status = AY_OK;
 ay_object *newo = NULL, *o = NULL;

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

      ay_status = ay_objio_fixncurve(&objio_ncurve);

      ay_status = ay_object_copy(newo, &o);
      if(ay_status)
	goto cleanup;

      if((objio_umin > objio_uknotv[objio_degu+1]) ||
	 (objio_umax < objio_uknotv[objio_ncurve.length]))
	ay_knots_setuminmax(o, objio_umin, objio_umax);

      ay_status = ay_object_link(o);

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

      ay_status = ay_objio_fixncurve(&objio_ncurve);

      ay_status = ay_object_copy(newo, &o);
      if(ay_status)
	goto cleanup;

      ay_status = ay_objio_addtrim(o);

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

      ay_status = ay_objio_fixnpatch(&objio_npatch);

      ay_status = ay_object_copy(newo, &o);
      if(ay_status)
	goto cleanup;

      /* add trim curves */
      if(objio_trims)
	{
	  ay_status = ay_object_crtendlevel(objio_nexttrim);
	  if(ay_status)
	    goto cleanup;
	  o->down = objio_trims;
	  objio_trims = NULL;
	  objio_nexttrim = &(objio_trims);
	}
      else
	{
	  ay_status = ay_object_crtendlevel(&(o->down));
	} /* if */

      /* add texture coordinates (as PV tags) */
      if(objio_texturesv)
	{
	  ay_status = ay_pv_add(o, objio_stagname, "varying", 0,
				objio_texturesvlen, objio_texturesv);
	}

      if(objio_texturetv)
	{
	  ay_status = ay_pv_add(o, objio_ttagname, "varying", 0,
				objio_texturetvlen, objio_texturetv);
	}

      /* add umin/umax/vmin/vmax tags */
      if((objio_umin > objio_uknotv[objio_degu+1]) ||
	 (objio_umax < objio_uknotv[objio_npatch.width]))
	ay_knots_setuminmax(o, objio_umin, objio_umax);

      if((objio_vmin > objio_vknotv[objio_degv+1]) ||
	 (objio_vmax < objio_vknotv[objio_npatch.height]))
	ay_knots_setvminmax(o, objio_vmin, objio_vmax);

      ay_status = ay_object_link(o);

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
  if(objio_texturesv)
    free(objio_texturesv);
  objio_texturesv = NULL;
  if(objio_texturetv)
    free(objio_texturetv);
  objio_texturetv = NULL;

  objio_curvtrimsurf = 0;

 return ay_status;
} /* ay_objio_readend */


/* ay_objio_readline:
 *  read a single line from a Wavefront OBJ file
 */
int
ay_objio_readline(FILE *fileptr)
{
 int ay_status = AY_OK;
 int read;
 static int lastlinewasface = AY_FALSE;
 char readchar, *str;
 Tcl_DString ds;

  if(!fileptr)
    {
      if(lastlinewasface)
	ay_status = ay_objio_readface(NULL, -1);
      lastlinewasface = AY_FALSE;
      return AY_ENULL;
    }

  Tcl_DStringInit(&ds);
  read = getc(fileptr);

  if(read == EOF)
    {Tcl_DStringFree(&ds); return AY_EUEOF;}

  if((char)read == '\n')
    {Tcl_DStringFree(&ds); return AY_OK;}

  while((char)read != '\n')
    {
      readchar = (char)read;

      Tcl_DStringAppend(&ds, &readchar, 1);
      read = getc(fileptr);

      if(read == EOF)
	{break;}

    } /* while */


  Tcl_DStringAppend(&ds, "\0", 1);

  str = Tcl_DStringValue(&ds);

  if(str[strlen(str)-1] == '\r')
    {
      str[strlen(str)-1] = '\0';
    }

  switch(str[0])
    {
    case '#':
      break;
    case 'c':
      if(str[1] == 's')
	ay_status = ay_objio_readcstype(str);
      if(str[1] == 'u')
	ay_status = ay_objio_readcurv(str);
      break;
    case 'd':
      ay_status = ay_objio_readdeg(str);
      break;
    case 'e':
      ay_status = ay_objio_readend();
      break;
    case 'f':
      ay_status = ay_objio_readface(str, lastlinewasface);
      break;
    case 'h':
      ay_status = ay_objio_readtrim(str, AY_TRUE);
      break;
    case 'p':
      ay_status = ay_objio_readparm(str);
      break;
    case 's':
      ay_status = ay_objio_readsurf(str);
      break;
    case 't':
      ay_status = ay_objio_readtrim(str, AY_FALSE);
      break;
    case 'v':
      ay_status = ay_objio_readvertex(str);
      break;
    default:
      break;
    } /* switch */

  if(str[0] == 'f')
    {
      lastlinewasface = AY_TRUE;
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
	    ay_status = ay_objio_readface(NULL, -1);
	  lastlinewasface = AY_FALSE;
	}
    }

  Tcl_DStringFree(&ds);

 return ay_status;
} /* ay_objio_readline */


/* ay_objio_readscene:
 *
 */
int
ay_objio_readscene(char *filename)
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
  objio_texturesv = NULL;
  objio_texturesvlen = 0;
  objio_texturetv = NULL;
  objio_texturetvlen = 0;

  objio_trims = NULL;
  objio_nexttrim = &(objio_trims);

  fseek(fileptr, 0L, SEEK_END);
  fsize = ftell(fileptr);
  if(fsize < 0)
    fsize = LONG_MAX;
  rewind(fileptr);

  /* estimate number of lines (the average Wavefront OBJ line has 34 bytes) */
  lines = fsize/34;

  while(!feof(fileptr))
    {
      if((ay_status = ay_objio_readline(fileptr)))
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
	      ay_error(AY_EWARN, fname,
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
  ay_status = ay_objio_freevertices();

  /* clean up */
  if(objio_uknotv)
    free(objio_uknotv);
  if(objio_vknotv)
    free(objio_vknotv);

  if(objio_ncurve.controlv)
    free(objio_ncurve.controlv);

  if(objio_npatch.controlv)
    free(objio_npatch.controlv);

  if(objio_texturesv)
    free(objio_texturesv);
  if(objio_texturetv)
    free(objio_texturetv);

  if(objio_trims)
    ay_object_deletemulti(objio_trims);

  objio_trims = NULL;
  objio_nexttrim = &(objio_trims);

  /* clean up trims buffer */
  ay_status = ay_objio_freetrims();

  /* reset lastlinewasface state in ay_objio_readline() */
  ay_objio_readline(NULL);

 return ay_status;
} /* ay_objio_readscene */


/* ay_objio_readscenetcmd:
 *
 */
int
ay_objio_readscenetcmd(ClientData clientData, Tcl_Interp *interp,
		       int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "ay_objio_read";
 int i = 2;

  /* check args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "filename");
      return TCL_OK;
    }

  objio_mergecfaces = AY_TRUE;
  objio_mergepvtags = AY_TRUE;
  objio_readcurves = AY_TRUE;
  objio_rescaleknots = 0.0;
  objio_checkdegen = AY_TRUE;

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
      if(!strcmp(argv[i], "-t"))
	{
	  objio_stagname = argv[i+1];
	  objio_ttagname = argv[i+2];
	  i++;
	}
      else
      if(!strcmp(argv[i], "-f"))
	{
	  sscanf(argv[i+1], "%lg", &objio_scalefactor);
	}
      else
      if(!strcmp(argv[i], "-d"))
	{
	  sscanf(argv[i+1], "%d", &objio_checkdegen);
	}

      i += 2;
    } /* while */

  ay_status = ay_objio_readscene(argv[1]);

  objio_stagname = objio_stagnamedef;
  objio_ttagname = objio_ttagnamedef;

  objio_scalefactor = 1.0;

 return TCL_OK;
} /* ay_objio_readscenetcmd */


/* ay_objio_init:
 *
 */
int
ay_objio_init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;
 /*char fname[] = "objio_init";*/

  /* init hash table for write callbacks */
  Tcl_InitHashTable(&ay_objio_write_ht, TCL_ONE_WORD_KEYS);

  /* fill hash table */
  ay_status = ay_objio_registerwritecb((char *)(AY_IDNPATCH),
				       ay_objio_writenpatch);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDNCURVE),
				       ay_objio_writencurve);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDLEVEL),
				       ay_objio_writelevel);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDCLONE),
				       ay_objio_writeclone);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDINSTANCE),
				       ay_objio_writeinstance);

  ay_status = ay_objio_registerwritecb((char *)(AY_IDICURVE),
				       ay_objio_writencconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDCONCATNC),
				       ay_objio_writencconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDEXTRNC),
				       ay_objio_writencconvertible);

  ay_status = ay_objio_registerwritecb((char *)(AY_IDEXTRUDE),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDREVOLVE),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDSWEEP),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDSKIN),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDCAP),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDPAMESH),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDBPATCH),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDGORDON),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDBIRAIL1),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDBIRAIL2),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDTEXT),
				       ay_objio_writenpconvertible);

  ay_status = ay_objio_registerwritecb((char *)(AY_IDSPHERE),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDDISK),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDCYLINDER),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDCONE),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDHYPERBOLOID),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDPARABOLOID),
				       ay_objio_writenpconvertible);
  ay_status = ay_objio_registerwritecb((char *)(AY_IDTORUS),
				       ay_objio_writenpconvertible);

  ay_status = ay_objio_registerwritecb((char *)(AY_IDPOMESH),
				       ay_objio_writepomesh);

  ay_status = ay_objio_registerwritecb((char *)(AY_IDBOX),
				       ay_objio_writebox);

  Tcl_CreateCommand(interp, "ay_objio_write", ay_objio_writescenetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "ay_objio_read", ay_objio_readscenetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

 return TCL_OK;
} /* ay_objio_init */
