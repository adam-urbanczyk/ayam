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

 return TCL_OK;
} /* ay_objio_init */
