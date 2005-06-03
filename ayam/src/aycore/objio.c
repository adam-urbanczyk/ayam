/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2002 by Randolf Schultz
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

static Tcl_HashTable ay_objio_write_ht; /* write callbacks */

static int objio_tesspomesh = AY_FALSE;

static int objio_omitcurves = AY_FALSE;

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
 *  write <n> <stride>D-vertices from array <v[n*stride]> to file <fileptr>
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
    }

 return AY_OK;
} /* ay_objio_writevertices */


/* ay_objio_writencurve:
 *
 */
int
ay_objio_writencurve(FILE *fileptr, ay_object *o, double *m)
{
 ay_nurbcurve_object *nc;
 double *v = NULL, *p1, *p2, pw[3];
 int stride = 4, i;

  if(objio_omitcurves)
    return AY_OK;

  if(!o)
    return AY_ENULL;

  nc = (ay_nurbcurve_object *)o->refine;

  /* get all vertices and transform them to world space */
  if(!(v = calloc(nc->length * stride, sizeof(double))))
    return AY_EOMEM;

  p1 = v;
  p2 = nc->controlv;
  for(i = 0; i < nc->length; i++)
    {
      pw[0] = p2[0]/p2[3];
      pw[1] = p2[1]/p2[3];
      pw[2] = p2[2]/p2[3];
      AY_APTRAN3(p1,pw,m)
      p1[3] = p2[3];
      p1 += stride;
      p2 += stride;
    }

  /* write all vertices */
  ay_objio_writevertices(fileptr, (unsigned int)nc->length, stride, v);

  /* write rational bspline curve */
  fprintf(fileptr, "cstype rat bspline\n");
  fprintf(fileptr, "deg %d\n", nc->order-1);
  fprintf(fileptr, "curv %g %g", nc->knotv[0],
	  nc->knotv[nc->length+nc->order-1]);

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
      pw[0] = p1[0]/p1[3];
      pw[1] = p1[1]/p1[3];
      AY_APTRAN3(v,pw,ma)
      v[2] = p1[3];
      fprintf(fileptr, "vp %g %g %g\n", v[0], v[1], v[2]);
      p1 += stride;
    }

  /* write 2D rational bspline curve */
  fprintf(fileptr, "cstype rat bspline\n");
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
	    }
	  ay_trafo_identitymatrix(mi);
	}

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
	    }
	}

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
	  umin = nc->knotv[0];
	  umax = nc->knotv[nc->length+nc->order-1];
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
		  umin = nc->knotv[0];
		  umax = nc->knotv[nc->length+nc->order-1];

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
 int stride = 4, i, j;

  if(!o)
    return AY_ENULL;

  /* first, check for and write the trim curves */
  if(o->down && o->down->next)
    {
      ay_status = ay_objio_writetrim(fileptr, o->down);
    }

  np = (ay_nurbpatch_object *)o->refine;

  /* get all vertices and transform them to world space,
     also adapting row/column major order in the process */
  if(!(v = calloc(np->width * np->height * stride, sizeof(double))))
    return AY_EOMEM;

  p1 = v;
  for(i = 0; i < np->height; i++)
    {
      p2 = &(np->controlv[i*stride]);
      for(j = 0; j < np->width; j++)
	{
	  pw[0] = p2[0]/p2[3];
	  pw[1] = p2[1]/p2[3];
	  pw[2] = p2[2]/p2[3];
	  AY_APTRAN3(p1,pw,m)
	  p1[3] = p2[3];
	  p1 += stride;
	  p2 += np->height*stride;
	}
    }

  /* write all vertices */
  ay_objio_writevertices(fileptr, (unsigned int)(np->width * np->height),
			 stride, v);

  /* write bspline surface */
  fprintf(fileptr, "cstype rat bspline\n");
  fprintf(fileptr, "deg %d %d\n", np->uorder-1, np->vorder-1);
  fprintf(fileptr, "surf %g %g %g %g", np->uknotv[0],
	  np->uknotv[np->width + np->uorder-1], np->vknotv[0],
	  np->vknotv[np->height + np->vorder-1]);

  for(i = np->width*np->height; i > 0; i--)
    {
      fprintf(fileptr, " -%d", i);
    }
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

  if(objio_omitcurves)
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
			  fprintf(fileptr, " -%d//-%d",
				  po->ncontrols-po->verts[r],
				  po->ncontrols-po->verts[r]);
			  r++;
			}
		    }
		  else
		    {
		      for(k = 0; k < po->nverts[q]; k++)
			{
			  fprintf(fileptr, " -%d",
				  po->ncontrols-po->verts[r]);
			  r++;
			}
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
      ay_status = ay_pomesht_merge(lihead, &to);
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
    }

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
    }

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
 int selected = AY_FALSE;

  /* check args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "filename [1|0] | [1|0] [1|0]");
      return TCL_OK;
    }

  objio_tesspomesh = AY_FALSE;
  objio_omitcurves = AY_FALSE;

  if(argc > 2)
    selected = atoi(argv[2]);

  if(argc > 3)
    objio_tesspomesh = atoi(argv[3]);

  if(argc > 4)
    objio_omitcurves = atoi(argv[4]);

  ay_status = ay_objio_writescene(argv[1], selected);

 return TCL_OK;
} /* ay_objio_writescenetcmd */


/****************************************************************/

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

int objio_cstype;
int objio_csrat;
int objio_degu;
int objio_degv;

double *objio_uknotv;
double *objio_vknotv;

int objio_curvtrimsurf; /* 0 - unset, 1 - curve, 2 - trim, 3 - surface */

ay_nurbcurve_object objio_ncurve;
ay_nurbpatch_object objio_npatch;

int ay_objio_addvertex(int type, double *v);

int ay_objio_getvertex(int type, unsigned int index, double **v);

int ay_objio_freevertices(void);

int ay_objio_readvertex(char *str);

int ay_objio_readvindex(char *c, int *gvindex, int *tvindex, int *nvindex);

int ay_objio_readface(char *str);

int ay_objio_readend(void);

int ay_objio_readline(FILE *fileptr);

int ay_objio_readscene(char *filename, int selected);

int ay_objio_readscenetcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);


/* ay_objio_addvertex:
 *
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
      memcpy(nv->v, v, 4*sizeof(double));
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
      memcpy(nv->v, v, 4*sizeof(double));
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
 *
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

  *pl = l;
  *v = &(l->v[0]);

 return AY_OK;
} /* ay_objio_getvertex */


/* ay_objio_freevertices:
 *
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

  /* clear cached "current" pointers in getvertex() */
  ay_status = ay_objio_getvertex(0, 0, NULL);

 return AY_OK;
} /* ay_objio_freevertices */


/* ay_objio_readvertex:
 *
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
      sscanf(&(str[2])," %lg %lg %lg", &(v[0]), &(v[1]), &(v[2]));
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
      ay_status = ay_objio_addvertex(1, v);
    } /* if */

 return ay_status;
} /* ay_objio_readvertex */


/* ay_objio_readvindex:
 *
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
 */
int
ay_objio_readskip(char **b)
{
 char *c = *b;

  if(!b)
    return AY_ENULL;

  while((isspace(*c)) && (*c != '\0'))
    c++;
  if(*c == '\0')
    return AY_OK;
  while(!(isspace(*c)) && (*c != '\0'))
    c++;

  /* return result */
  *b = c;

 return AY_OK;
} /* ay_objio_readskip */


/* ay_objio_readface:
 *
 */
int
ay_objio_readface(char *str)
{
 int ay_status = AY_OK;
 char *c = NULL;
 int gvindex = 0, tvindex = 0, nvindex = 0, stride = 0, i;
 int last_stride = 0;
 double *gv, *nv/*, *tv*/;
 double *newcontrolv = NULL;
 ay_pomesh_object po = {0};
 ay_object t = {0}, *o = NULL;
 unsigned int nloops = 1, nverts = 0;

  if(!str)
    return AY_ENULL;

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
      /*tv = NULL;*/

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
	      if(objio_gverts_tail)
		{
		  ay_status = ay_objio_getvertex(2,
					   objio_nverts_tail->index + nvindex,
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
	  /* get texture vertex data and add it to the pomesh */
	  /* XXXX to be done */
	}

      stride = 3;
      if(nv)
	stride += 3;

      /*if(tv)
	stride += 2;*/

      if((last_stride > 0) && (stride != last_stride))
	{ay_status = AY_ERROR; goto cleanup;}

      last_stride = stride;

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

      ay_status = ay_object_copy(&t, &o);
      if(ay_status)
	goto cleanup;

      ay_status = ay_object_link(o);

    } /* if */

cleanup:
  if(po.controlv)
    free(po.controlv);

  if(po.verts)
    free(po.verts);

 return ay_status;
} /* ay_objio_readface */


/* ay_objio_readcstype:
 *
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
 *
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
 *
 */
int
ay_objio_readcurv(char *str)
{
 int ay_status = AY_OK;
 char *c = str;
 double umin, umax;
 int gvindex = 0, tvindex = 0, nvindex = 0, stride = 4;
 double *gv, *controlv;

  if(!str)
    return AY_ENULL;

  while(!(isspace(*c)) && (*c != '\0'))
    c++;

  /* read umin/umax */
  sscanf(c, " %lg %lg", &umin, &umax);
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

  objio_curvtrimsurf = 1;

cleanup:

 return ay_status;
} /* ay_objio_readcurv */


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
    }
  else
    {
      if(objio_vknotv)
	free(objio_vknotv);
      objio_vknotv = knotv;
    } /* if */

 return ay_status;
} /* ay_objio_readparm */


/* ay_objio_readend:
 *  realise curve or surface
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
      if(!(newo = calloc(1, sizeof(ay_object))))
	return AY_EOMEM;
      ay_object_defaults(newo);
      newo->type = AY_IDNCURVE;
      newo->refine = &objio_ncurve;
      objio_ncurve.knotv = objio_uknotv;
      objio_ncurve.order = objio_degu;
      objio_ncurve.knot_type = AY_KTCUSTOM;
      ay_status = ay_object_copy(newo, &o);
      if(ay_status)
	goto cleanup;
      ay_status = ay_object_link(o);
      free(objio_uknotv);
      objio_uknotv = NULL;
      free(objio_ncurve.controlv);
      objio_ncurve.controlv = NULL;
      break;
    case 2:
      /* read a 2D (trim) curve */
      /* XXXX to be done */
      free(objio_uknotv);
      objio_uknotv = NULL;
      free(objio_ncurve.controlv);
      objio_ncurve.controlv = NULL;
      break;
    case 3:
      /* read a surface */
      if(!(newo = calloc(1, sizeof(ay_object))))
	return AY_EOMEM;
      ay_object_defaults(newo);
      newo->type = AY_IDNPATCH;
      newo->refine = &objio_npatch;
      objio_npatch.uknotv = objio_uknotv;
      objio_npatch.vknotv = objio_vknotv;
      objio_npatch.uorder = objio_degu;
      objio_npatch.vorder = objio_degv;
      objio_npatch.uknot_type = AY_KTCUSTOM;
      objio_npatch.vknot_type = AY_KTCUSTOM;
      ay_status = ay_object_copy(newo, &o);
      if(ay_status)
	goto cleanup;
      ay_status = ay_object_link(o);

      free(objio_uknotv);
      objio_uknotv = NULL;
      free(objio_vknotv);
      objio_vknotv = NULL;
      free(objio_npatch.controlv);
      objio_npatch.controlv = NULL;
    default:
      /* read a superfluous end? */
      break;
    }

  switch(objio_cstype)
    {
    case 0:
      break;
    default:
      break;
    } /* switch */

cleanup:

  if(newo)
    free(newo);

 return ay_status;
} /* ay_objio_readend */


/* ay_objio_readline:
 *
 */
int
ay_objio_readline(FILE *fileptr)
{
 int ay_status = AY_OK;
 int read;
 char readchar, *str;
 Tcl_DString ds;

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
    case 'v':
      ay_status = ay_objio_readvertex(str);
      break;
    case 'f':
      ay_status = ay_objio_readface(str);
      break;
    case 'e':
      ay_status = ay_objio_readend();
      break;
    case 'p':
      ay_status = ay_objio_readparm(str);
      break;
    default:
      break;
    } /* switch */

  Tcl_DStringFree(&ds);

 return ay_status;
} /* ay_objio_readline */


/* ay_objio_readscene:
 *
 */
int
ay_objio_readscene(char *filename, int selected)
{
 int ay_status = AY_OK;
 ay_object *o = ay_root->next;
 FILE *fileptr = NULL;
 char fname[] = "objio_readscene";

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
  objio_vknotv = NULL;
  objio_curvtrimsurf = 0;
  memset(&objio_ncurve, 0, sizeof(ay_nurbcurve_object));
  memset(&objio_npatch, 0, sizeof(ay_nurbpatch_object));

  while(!feof(fileptr))
    {
      if((ay_status = ay_objio_readline(fileptr)))
	break;
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
 int selected = AY_FALSE;

  /* check args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "filename [1|0] | [1|0] [1|0]");
      return TCL_OK;
    }

  objio_tesspomesh = AY_FALSE;
  objio_omitcurves = AY_FALSE;

  if(argc > 2)
    selected = atoi(argv[2]);

  if(argc > 3)
    objio_tesspomesh = atoi(argv[3]);

  if(argc > 4)
    objio_omitcurves = atoi(argv[4]);

  ay_status = ay_objio_readscene(argv[1], selected);

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
