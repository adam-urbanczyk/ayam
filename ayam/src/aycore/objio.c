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

int ay_objio_writenpatch(FILE *fileptr, ay_object *o, double *m);

int ay_objio_writelevel(FILE *fileptr, ay_object *o, double *m);

int ay_objio_writencconvertible(FILE *fileptr, ay_object *o, double *m);

int ay_objio_writenpconvertible(FILE *fileptr, ay_object *o, double *m);

int ay_objio_writeobject(FILE *fileptr, ay_object *o);

int ay_objio_writescenetcmd(ClientData clientData, Tcl_Interp *interp,
			    int argc, char *argv[]);


/* global variables */

static double tm[16] = {0}; /* current transformation matrix */

static Tcl_HashTable ay_objio_write_ht; /* write callbacks */


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
	  fprintf(fileptr, "v %lg %lg\n", v[j], v[j+1]);
	  j += stride;
	}
      break;
    case 3:
      for(i = 0; i < n; i++)
	{
	  fprintf(fileptr, "v %lg %lg %lg\n", v[j], v[j+1], v[j+2]);
	  j += stride;
	}
      break;
    case 4:
      for(i = 0; i < n; i++)
	{
	  fprintf(fileptr, "v %lg %lg %lg %lg\n", v[j], v[j+1], v[j+2],
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
 double *v = NULL, *p1, *p2;
 int stride = 4, i;

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
      AY_APTRAN3(p1,p2,m)
      p1 += stride;
      p2 += stride;
    }

  /* write all vertices */
  ay_objio_writevertices(fileptr, (unsigned int)nc->length, stride, v);

  /* write bspline curve */
  fprintf(fileptr, "cstype rat bspline\n");
  fprintf(fileptr, "deg %d\n", nc->order-1);
  fprintf(fileptr, "curv %lg %lg", nc->knotv[0],
	  nc->knotv[nc->length+nc->order-1]);

  for(i = nc->length; i > 0; i--)
    {
      fprintf(fileptr, " -%d", i);
    }
  fprintf(fileptr, "\n");

  /* write knot vector */
  fprintf(fileptr, "parm u");
  for(i = 0; i < (nc->length+nc->order-1); i++)
    {
      fprintf(fileptr, " %lg", nc->knotv[i]);
    }
  fprintf(fileptr, "\n");

  free(v);

 return AY_OK;
} /* ay_objio_writencurve */


/* ay_objio_writenpatch:
 *
 */
int
ay_objio_writenpatch(FILE *fileptr, ay_object *o, double *m)
{
 ay_nurbpatch_object *np;
 double *v = NULL, *p1, *p2;
 int stride = 4, i, j;

  if(!o)
    return AY_ENULL;

  np = (ay_nurbpatch_object *)o->refine;

  /* get all vertices and transform them to world space */
  if(!(v = calloc(np->width * np->height * stride, sizeof(double))))
    return AY_EOMEM;

  p1 = v;
  p2 = np->controlv;
  for(i = 0; i < np->width; i++)
    {
      for(j = 0; j < np->height; j++)
	{
	  AY_APTRAN3(p1,p2,m)
	  p1 += stride;
	  p2 += stride;
	}
    }

  /* write all vertices */
  ay_objio_writevertices(fileptr, (unsigned int)(np->width * np->height),
			 stride, v);

  /* write bspline curve */
  fprintf(fileptr, "cstype rat bspline\n");
  fprintf(fileptr, "deg %d %d\n", np->uorder-1, np->vorder-1);
  fprintf(fileptr, "surf %lg %lg %lg %lg", np->uknotv[0],
	  np->uknotv[np->width + np->uorder-1], np->vknotv[0],
	  np->vknotv[np->height + np->vorder-1]);

  for(i = np->width*np->height; i > 0; i--)
    {
      fprintf(fileptr, " -%d", i);
    }
  fprintf(fileptr, "\n");

  /* write knot vector (u) */
  fprintf(fileptr, "parm u");
  for(i = 0; i < (np->width + np->uorder - 1); i++)
    {
      fprintf(fileptr, " %lg", np->uknotv[i]);
    }
  fprintf(fileptr, "\n");

  /* write knot vector (v) */
  fprintf(fileptr, "parm v");
  for(i = 0; i < (np->height + np->vorder - 1); i++)
    {
      fprintf(fileptr, " %lg", np->vknotv[i]);
    }
  fprintf(fileptr, "\n");

  /* write trim curves (if any) */

  free(v);

 return AY_OK;
} /* ay_objio_writenpatch */


/* ay_objio_writencurve:
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
	  ay_status = ay_objio_writeobject(fileptr, down);
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

  if(!o)
   return AY_ENULL;

  ay_status = ay_provide_object(o, AY_IDNCURVE, &c);
  t = c;
  while(t)
    {
      if(t->type == AY_IDNCURVE)
	{
	  ay_status = ay_objio_writeobject(fileptr, t);
	}

      t = t->next;
    } /* while */

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
  while(t)
    {
      if(t->type == AY_IDNPATCH)
	{
	  ay_status = ay_objio_writeobject(fileptr, t);
	}

      t = t->next;
    } /* while */
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
 char fname[] = "objio_writepomesh";
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

  /* get all vertices, transform them to world space and write them*/
  p1 = po->controlv;
  for(i = 0; i < po->ncontrols; i++)
    {
      AY_APTRAN3(v,p1,m)
      fprintf(fileptr, "v %lg %lg %lg\n", v[0], v[1], v[2]);
      p1 += stride;
    }

  /* write normals */
  if(po->has_normals)
    {
      p1 = po->controlv;
      for(i = 0; i < po->ncontrols; i++)
	{
	  fprintf(fileptr, "vn %lg %lg %lg\n", p1[0], p1[1], p1[2]);
	  p1 += 6;
	}
    }

  /* write faces */
  for(i = 0; i < po->npolys; i++)
    {
      if(po->nloops[i] == 1)
	{
	  fprintf(fileptr, "f");
	  for(j = 0; j < po->nloops[p]; j++)
	    {
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
		}
	      q++;
	    } /* for */
	  fprintf(fileptr, "\n");
	}
      else
	{
	  ay_error(AY_EWARN, fname, "Skipping polygon with hole(s).");
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


 return AY_OK;
} /* ay_objio_writepomesh */


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
ay_objio_writeobject(FILE *fileptr, ay_object *o)
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
	  fprintf(fileptr, "end\n");
	}
    }
  else
    {
      sprintf(err, "No callback registered for this type: %d.", o->type);
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
 ay_object *o = ay_root;
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
	      ay_status = ay_objio_writeobject(fileptr, o);
	    }
	}
      else
	{
	  ay_status = ay_objio_writeobject(fileptr, o);
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
      ay_error(AY_EARGS, fname, "filename [1|0]");
      return TCL_OK;
    }

  if(argc > 2)
    selected = atoi(argv[2]);

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

  ay_status = ay_objio_registerwritecb((char *)(AY_IDICURVE),
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

  ay_status = ay_objio_registerwritecb((char *)(AY_IDPOMESH),
				       ay_objio_writepomesh);

  Tcl_CreateCommand(interp, "ay_objio_write", ay_objio_writescenetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

 return TCL_OK;
} /* ay_objio_init */
