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

#include "ayam.h"

/* mfio.c - 3DMF Import/Export Plugin */

#include <mf3d.h>
#include <mferrors.h>

/* global variables and structs: */
static ay_object *ay_mfio_lastreadobject;

static MF3DErr ay_mfio_mf3d_errno;

static MF3DDataFormatEnum ay_mfio_mf3d_data_format;


static Tcl_HashTable ay_mfio_read_ht;

static Tcl_HashTable ay_mfio_write_ht;


char ay_mfio_version_ma[] = AYVERSIONSTR;
char ay_mfio_version_mi[] = AYVERSIONSTRMI;

typedef int (ay_mfio_readcb) (MF3DVoidObjPtr object);

typedef int (ay_mfio_writecb) (MF3D_FilePtr fileptr, ay_object *o);


/* prototypes of functions local to this module: */
int ay_mfio_registerreadcb(char *type, ay_mfio_readcb *cb);

int ay_mfio_readnurbcurve(MF3DVoidObjPtr object);

int ay_mfio_readnurbpatch(MF3DVoidObjPtr object);

int ay_mfio_readpolyline(MF3DVoidObjPtr object);

int ay_mfio_readgeneralpolygon(MF3DVoidObjPtr object);

int ay_mfio_readpolygon(MF3DVoidObjPtr object);

int ay_mfio_readtriangle(MF3DVoidObjPtr object);

int ay_mfio_readtrigrid(MF3DVoidObjPtr object);

int ay_mfio_readcntr(MF3DVoidObjPtr object);

int ay_mfio_readecntr(MF3DVoidObjPtr object);

int ay_mfio_readscal(MF3DVoidObjPtr object);

int ay_mfio_readtran(MF3DVoidObjPtr object);

int ay_mfio_readrot(MF3DVoidObjPtr object);

int ay_mfio_readdcol(MF3DVoidObjPtr object);

int ay_mfio_readtcol(MF3DVoidObjPtr object);

int ay_mfio_readobject(MF3DVoidObjPtr object);

int ay_mfio_readscene(Tcl_Interp *interp, char *filename);

int ay_mfio_writenurbcurve(MF3D_FilePtr fileptr, ay_object *o);

int ay_mfio_writetrimcurve(MF3D_FilePtr fileptr, ay_object *o);

int ay_mfio_writenurbpatch(MF3D_FilePtr fileptr, ay_object *o);

int ay_mfio_writesphere(MF3D_FilePtr fileptr, ay_object *o);

int ay_mfio_writeattributes(MF3D_FilePtr fileptr, ay_object *o);

int ay_mfio_writelevel(MF3D_FilePtr fileptr, ay_object *o);

int ay_mfio_writeinstance(MF3D_FilePtr fileptr, ay_object *o);

int ay_mfio_writecntr(MF3D_FilePtr fileptr);

int ay_mfio_writeecntr(MF3D_FilePtr fileptr);

int ay_mfio_writeobject(MF3D_FilePtr fileptr, ay_object *object);

int ay_mfio_writescene(Tcl_Interp *interp, char *filename);

int ay_mfio_registerwritecb(char *name, ay_mfio_writecb *cb);

int ay_mfio_importscenetcmd(ClientData clientData, Tcl_Interp * interp,
				int argc, char *argv[]);

int ay_mfio_exportscenetcmd(ClientData clientData, Tcl_Interp *interp,
			      int argc, char *argv[]);

void ay_mfio_printerr(MF3DErr errcode);

int Mfio_Init(Tcl_Interp *interp);


/* functions: */
/* ay_mfio_registerreadcb:
 *
 */
int
ay_mfio_registerreadcb(char *type, ay_mfio_readcb *cb)
{
 int ay_status = AY_OK;
 int new_item = 0;
 Tcl_HashEntry *entry = NULL;
 Tcl_HashTable *ht = &ay_mfio_read_ht;

  if(!cb)
    return AY_ENULL;

  if((entry = Tcl_FindHashEntry(ht, type)))
    {
      return AY_ERROR; /* name already registered */
    }
  else
    {
      /* create new entry */
      entry = Tcl_CreateHashEntry(ht, type, &new_item);
      Tcl_SetHashValue(entry, (char*)cb); 
    }

 return ay_status;
} /* ay_mfio_registerreadcb */


/* ay_mfio_readnurbpatch:
 *
 */
int
ay_mfio_readnurbpatch(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 int width, height, i, j, a, b;
 double *controlv = NULL;
 double *uknotv = NULL, *vknotv = NULL;
 MF3DNurbPatchObjPtr o = (MF3DNurbPatchObjPtr) object;
 ay_nurbpatch_object *patch = NULL;
 ay_object *newo = NULL;

  /* get some info about the patch */
  width = o->numColumns;
  height = o->numRows;

  /* alloc new (safe) memory */
  if(!(controlv = calloc(width*height*4, sizeof(double))))
    return AY_EOMEM; 
  if(!(uknotv = calloc(width+o->uOrder, sizeof(double))))
    { free(controlv); return AY_EOMEM; }
  if(!(vknotv = calloc(height+o->vOrder, sizeof(double))))
    { free(controlv); free(uknotv); return AY_EOMEM; }

  /* copy data into new (safe) memory */
  a = 0; b = 0;
  for(i=0;i<width;i++)
    for(j=0;j<height;j++)
      {
	controlv[b++] = (o->points)[a].x;
	controlv[b++] = (o->points)[a].y;
	controlv[b++] = (o->points)[a].z;
	controlv[b++] = (o->points)[a].w;
	a++;
      }
  a = 0;
  for(i=0;i<width+o->uOrder;i++)
    {
      uknotv[a] = (o->uKnots)[a];
      a++;
    }
  a = 0;
  for(i=0;i<height+o->vOrder;i++)
    {
      vknotv[a] = (o->vKnots)[a];
      a++;
    }

  /* now create a NURBPatch */
  ay_status = ay_npt_create(o->uOrder, o->vOrder, width, height,
			    AY_KTCUSTOM, AY_KTCUSTOM,
			    controlv, uknotv, vknotv,
			    &patch);

  if(ay_status)
    { free(controlv); free(uknotv); free(vknotv); return ay_status; }

  /* link the new patch into the hierarchy */
  if(!(newo = calloc(1, sizeof(ay_object))))
    { free(controlv); free(uknotv); free(vknotv); return AY_EOMEM; }

  newo->type = AY_IDNPATCH;
  newo->refine = patch;

  ay_status = ay_object_link(newo);

  if(ay_status)
    ay_status = ay_object_delete(newo);

  ay_mfio_lastreadobject = newo;

  ay_status = ay_object_defaults(newo);

  newo->parent = AY_TRUE;
  newo->hidechilds = AY_TRUE;
  newo->inherit_trafos = AY_FALSE;

 return ay_status;
} /* ay_mfio_readnurbpatch */


/* ay_mfio_readnurbcurve:
 *
 */
int
ay_mfio_readnurbcurve(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 int length, i, a, b;
 double *controlv = NULL;
 double *knotv = NULL;
 MF3DNURBCurveObjPtr o = (MF3DNURBCurveObjPtr) object;
 ay_nurbcurve_object *curve = NULL;
 ay_object *newo = NULL;

  /* get some info about the curve */
  length = o->nPoints;

  /* alloc new (safe) memory */
  if(!(controlv = calloc(length*4, sizeof(double))))
    return AY_EOMEM; 
  if(!(knotv = calloc(length+o->order, sizeof(double))))
    { free(controlv); return AY_EOMEM; }

  /* copy data into new (safe) memory */
  a = 0; b = 0;
  for(i=0;i<length;i++)
    {
      controlv[b++] = (o->points)[a].x;
      controlv[b++] = (o->points)[a].y;
      controlv[b++] = (o->points)[a].z;
      controlv[b++] = (o->points)[a].w;
      a++;
    }
  a = 0;
  for(i=0;i<length+o->order;i++)
    {
      knotv[a] = (o->knots)[a];
      a++;
    }

  /* now create a NURBCurve */
  ay_status = ay_nct_create(o->order, length,
			    AY_KTCUSTOM, controlv, knotv,
			    &curve);

  if(ay_status)
    { free(controlv); free(knotv); return ay_status; }  

  /* link the new curve into the hierarchy */
  if(!(newo = calloc(1, sizeof(ay_object))))
    { free(controlv); free(knotv); return AY_EOMEM; }

  newo->type = AY_IDNCURVE;
  newo->refine = curve;

  ay_status = ay_object_link(newo);

  if(ay_status)
    {
      ay_object_delete(newo);
      return ay_status;
    }

  ay_mfio_lastreadobject = newo;

  ay_status = ay_object_defaults(newo);

 return ay_status;
} /* ay_mfio_readnurbcurve */


/* ay_mfio_readpolyline:
 *
 */
int
ay_mfio_readpolyline(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 int length, i, a, b;
 double *controlv = NULL;
 MF3DPolyLineObjPtr o = (MF3DPolyLineObjPtr) object;
 ay_nurbcurve_object *curve = NULL;
 ay_object *newo = NULL;
 char fname[] = "mfio_readpolyline";

  /* get some info about the polyline */
  if(o->nVertices > INT_MAX)
    {
      ay_error(AY_ERROR, fname, "polyline is too long");
      return AY_OK;
    }

  length = (int) o->nVertices;

  /* alloc new (safe) memory */
  if(!(controlv = calloc(length*4, sizeof(double))))
    return AY_EOMEM; 

  /* copy data into new (safe) memory */
  a = 0; b = 0;
  for(i = 0; i < length; i++)
    {
      controlv[b++] = (o->vertices)[a].x;
      controlv[b++] = (o->vertices)[a].y;
      controlv[b++] = (o->vertices)[a].z;
      controlv[b++] = 1.0;
      a++;
    }

  /* now create a NURBCurve */
  ay_status = ay_nct_create(2, length,
			    AY_KTNURB, controlv, NULL,
			    &curve);

  if(ay_status)
    { free(controlv); return ay_status; }  

  /* link the new curve into the hierarchy */
  if(!(newo = calloc(1, sizeof(ay_object))))
    { free(controlv); return AY_EOMEM; }

  newo->type = AY_IDNCURVE;
  newo->refine = curve;

  ay_status = ay_object_link(newo);

  if(ay_status)
    {
      ay_object_delete(newo);
      return ay_status;
    }

  ay_mfio_lastreadobject = newo;

  ay_status = ay_object_defaults(newo);

 return ay_status;
} /* ay_mfio_readpolyline */


/* ay_mfio_readgeneralpolygon:
 *
 */
int
ay_mfio_readgeneralpolygon(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 MF3DGeneralPolygonObjPtr o = (MF3DGeneralPolygonObjPtr) object;
 MF3DPolygonDataPtr p;
 ay_pomesh_object *pomesh = NULL;
 ay_object *newo = NULL;
 unsigned int i = 0, j = 0, a = 0, total_verts = 0;

  if(!(pomesh = calloc(1, sizeof(ay_pomesh_object))))
    return AY_EOMEM;

  p = o->polygons;
  
  pomesh->npolys = 1;

  if(!(pomesh->nloops = calloc(1, sizeof(unsigned int))))
    { free(pomesh); return AY_EOMEM; }
  pomesh->nloops[0] = o->nContours;

  if(!(pomesh->nverts = calloc(o->nContours, sizeof(unsigned int))))
    { free(pomesh->nloops); free(pomesh); return AY_EOMEM; }

  for(i = 0; i < o->nContours; i++)
    {
      pomesh->nverts[i] = p->nVertices;
      total_verts += p->nVertices;
      p++;
    } /* for */

  if(!(pomesh->verts = calloc(total_verts, sizeof(unsigned int))))
    {
      free(pomesh->nverts); free(pomesh->nloops); free(pomesh);
      return AY_EOMEM;
    }

  for(i = 0; i < total_verts; i++)
    {
      pomesh->verts[i] = i;
    } /* for */

  if(!(pomesh->controlv = calloc(total_verts * 3, sizeof(double))))
    {
      free(pomesh->verts); free(pomesh->nverts); free(pomesh->nloops);
      free(pomesh); return AY_EOMEM;
    }

  p = o->polygons;
  a = 0;
  for(i = 0; i < o->nContours; i++)
    {
      for(j = 0; j < p->nVertices; j++)
	{
	  pomesh->controlv[a]   = (double)(p->vertices[i].x);
	  pomesh->controlv[a+1] = (double)(p->vertices[i].y);
	  pomesh->controlv[a+2] = (double)(p->vertices[i].z);
	  a += 3;
	} /* for */
      p++;
    } /* for */


  if(!(newo = calloc(1, sizeof(ay_object))))
    { 
      free(pomesh->controlv); free(pomesh->verts); free(pomesh->nverts);
      free(pomesh->nloops); free(pomesh); return AY_EOMEM;
    }

  ay_object_defaults(newo);
  newo->type = AY_IDPOMESH;
  newo->refine = pomesh;

  ay_status = ay_object_link(newo);

  if(ay_status)
    {
      ay_object_delete(newo);
      return ay_status;
    }

  ay_mfio_lastreadobject = newo;

 return ay_status;
} /* ay_mfio_readgeneralpolygon */


/* ay_mfio_readpolygon:
 *
 */
int
ay_mfio_readpolygon(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 MF3DPolygonObjPtr o = (MF3DPolygonObjPtr) object;
 ay_pomesh_object *pomesh = NULL;
 ay_object *newo = NULL;
 unsigned int i = 0, a = 0;

  if(!(pomesh = calloc(1, sizeof(ay_pomesh_object))))
    return AY_EOMEM;

  pomesh->npolys = 1;

  if(!(pomesh->nloops = calloc(1, sizeof(unsigned int))))
    return AY_EOMEM;
  pomesh->nloops[0] = 1;

  if(!(pomesh->nverts = calloc(1, sizeof(unsigned int))))
    return AY_EOMEM;
  pomesh->nverts[0] = o->nVertices;

  if(!(pomesh->verts = calloc(o->nVertices, sizeof(unsigned int))))
    return AY_EOMEM;

  for(i = 0; i < o->nVertices; i++)
    {
      pomesh->verts[i] = i;
    } /* for */

  if(!(pomesh->controlv = calloc(o->nVertices * 3, sizeof(double))))
    return AY_EOMEM;

  a = 0;
  for(i = 0; i < o->nVertices; i++)
    {
      pomesh->controlv[a]   = (double)(o->vertices[i].x);
      pomesh->controlv[a+1] = (double)(o->vertices[i].y);
      pomesh->controlv[a+2] = (double)(o->vertices[i].z);
      a += 3;
    } /* for */


  if(!(newo = calloc(1, sizeof(ay_object))))
    { 
      free(pomesh->controlv); free(pomesh->verts); free(pomesh->nverts);
      free(pomesh->nloops); free(pomesh); return AY_EOMEM;
    }

  ay_object_defaults(newo);
  newo->type = AY_IDPOMESH;
  newo->refine = pomesh;

  ay_status = ay_object_link(newo);

  if(ay_status)
    {
      ay_object_delete(newo);
      return ay_status;
    }

  ay_mfio_lastreadobject = newo;

 return ay_status;
} /* ay_mfio_readpolygon */


/* ay_mfio_readtriangle:
 *
 */
int
ay_mfio_readtriangle(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 MF3DTriangleObjPtr o = (MF3DTriangleObjPtr) object;
 ay_pomesh_object *pomesh = NULL;
 ay_object *newo = NULL;

  if(!(pomesh = calloc(1, sizeof(ay_pomesh_object))))
    return AY_EOMEM;

  pomesh->npolys = 1;

  if(!(pomesh->nloops = calloc(1, sizeof(unsigned int))))
    { free(pomesh); return AY_EOMEM; }
  pomesh->nloops[0] = 1;

  if(!(pomesh->nverts = calloc(1, sizeof(unsigned int))))
    { free(pomesh->nloops); free(pomesh); return AY_EOMEM; }
  pomesh->nverts[0] = 1;

  if(!(pomesh->verts = calloc(3, sizeof(unsigned int))))
    {
      free(pomesh->nverts); free(pomesh->nloops); free(pomesh);
      return AY_EOMEM;
    }
  pomesh->verts[0] = 0;
  pomesh->verts[1] = 1;
  pomesh->verts[2] = 2;

  if(!(pomesh->controlv = calloc(3 * 3, sizeof(double))))
    {
      free(pomesh->verts); free(pomesh->nverts); free(pomesh->nloops);
      free(pomesh); return AY_EOMEM;
    }

  pomesh->controlv[0] = (double)(o->vertex1.x);
  pomesh->controlv[1] = (double)(o->vertex1.y);
  pomesh->controlv[2] = (double)(o->vertex1.z);

  pomesh->controlv[3] = (double)(o->vertex2.x);
  pomesh->controlv[4] = (double)(o->vertex2.y);
  pomesh->controlv[5] = (double)(o->vertex2.z);

  pomesh->controlv[6] = (double)(o->vertex3.x);
  pomesh->controlv[7] = (double)(o->vertex3.y);
  pomesh->controlv[8] = (double)(o->vertex3.z);

  if(!(newo = calloc(1, sizeof(ay_object))))
    { free(pomesh); return AY_EOMEM; }

  ay_object_defaults(newo);
  newo->type = AY_IDPOMESH;
  newo->refine = pomesh;

  ay_status = ay_object_link(newo);

  if(ay_status)
    {
      ay_object_delete(newo);
      return ay_status;
    }

  ay_mfio_lastreadobject = newo;

 return ay_status;
} /* ay_mfio_readtriangle */


/* ay_mfio_readtrigrid:
 *
 */
int
ay_mfio_readtrigrid(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 MF3DTriGridObjPtr o = (MF3DTriGridObjPtr) object;
 ay_pomesh_object *pomesh = NULL;
 ay_object *newo = NULL;
 unsigned int i = 0, j = 0, k = 0, l = 0, a = 0;
 unsigned int u, v;
 int uquadtype = AY_FALSE, vquadtype = AY_FALSE;

  if(!(pomesh = calloc(1, sizeof(ay_pomesh_object))))
    return AY_EOMEM;

  u = o->numRows;
  v = o->numColumns;
  
  pomesh->npolys = u * v;

  if(!(pomesh->nloops = calloc(u * v, sizeof(unsigned int))))
    { free(pomesh); return AY_EOMEM; }
  for(i = 0; i < (u * v); i++)
    {
      pomesh->nloops[i] = 1;
    }

  if(!(pomesh->nverts = calloc( u * v, sizeof(unsigned int))))
    { free(pomesh->nloops); free(pomesh); return AY_EOMEM; }
  for(i = 0; i < (u * v); i++)
    {
      pomesh->nverts[i] = 3;
    } /* for */

  if(!(pomesh->verts = calloc(u * v, sizeof(unsigned int))))
    {
      free(pomesh->nverts); free(pomesh->nloops); free(pomesh);
      return AY_EOMEM;
    }

  vquadtype = AY_TRUE;
  a = 0;
  for(i = 0; i < v; i++)
    {
      j = i*u;
      k = i*u + u;
      uquadtype = vquadtype;
      for(l = 0; l < u; l++)
	{
	  /* always work on two triangles (a quadrilateral) at once;
	     the quadtype (how are the triangles orientated in the quad)
	     alternates in u _and_ v direction... */
	  if(uquadtype)
	    {
	      pomesh->verts[a]   = j;
	      pomesh->verts[a+1] = k+1;
	      pomesh->verts[a+2] = k;
	      pomesh->verts[a+3] = j;
	      pomesh->verts[a+4] = j+1;
	      pomesh->verts[a+5] = k+1;
	    }
	  else
	    {
	      pomesh->verts[a]   = j;
	      pomesh->verts[a+1] = j+1;
	      pomesh->verts[a+2] = k;
	      pomesh->verts[a+3] = k;
	      pomesh->verts[a+4] = j+1;
	      pomesh->verts[a+5] = k+1;
	    }
	  a += 6;

	  /* toggle uquadtype */
	  if(uquadtype)
	    uquadtype = AY_FALSE;
	  else
	    uquadtype = AY_TRUE;
	} /* for */

      /* toggle vquadtype */
      if(vquadtype)
	vquadtype = AY_FALSE;
      else
	vquadtype = AY_TRUE;
    } /* for */

  if(!(pomesh->controlv = calloc(u * v * 3, sizeof(double))))
    {
      free(pomesh->verts); free(pomesh->nverts); free(pomesh->nloops);
      free(pomesh); return AY_EOMEM;
    }

  a = 0;
  for(i = 0; i < (u * v); i++)
    {
      pomesh->controlv[a]   = (double)(o->vertices[i].x);
      pomesh->controlv[a+1] = (double)(o->vertices[i].y);
      pomesh->controlv[a+2] = (double)(o->vertices[i].z);
      a += 3;
    } /* for */


  if(!(newo = calloc(1, sizeof(ay_object))))
    {
      free(pomesh->controlv); free(pomesh->verts); free(pomesh->nverts);
      free(pomesh->nloops); free(pomesh); return AY_EOMEM;
    }

  ay_object_defaults(newo);
  newo->type = AY_IDPOMESH;
  newo->refine = pomesh;

  ay_status = ay_object_link(newo);

  if(ay_status)
    {
      ay_object_delete(newo);
      return ay_status;
    }

  ay_mfio_lastreadobject = newo;

 return ay_status;
} /* ay_mfio_readtrigrid */


/* ay_mfio_readbox:
 *
 */
int
ay_mfio_readbox(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 MF3DBoxObjPtr o = (MF3DBoxObjPtr) object;
 MF3DBoxDataPtr ed;
 ay_box_object *box = NULL;
 ay_object *newo = NULL;

  if(!(box = calloc(1,sizeof(ay_box_object))))
    return AY_EOMEM;

  box->width = 1.0;
  box->height = 1.0;
  box->length = 1.0;

  if(!(newo = calloc(1, sizeof(ay_object))))
    { free(box); return AY_EOMEM; }

  newo->type = AY_IDBOX;
  newo->refine = box;

  ay_status = ay_object_link(newo);

  if(ay_status)
    {
      ay_object_delete(newo);
      return ay_status;
    }

  ay_mfio_lastreadobject = newo;

  ay_status = ay_object_defaults(newo);

  newo->movx = 0.5;
  newo->movy = 0.5;
  newo->movz = 0.5;

  if(o->box)
    {
      ed = o->box;
      newo->movx += ed->origin.x;
      newo->movy += ed->origin.y;
      newo->movz += ed->origin.z;

      /* XXXX minor & major Axis? */

    }

 return ay_status;
} /* ay_mfio_readbox */


/* ay_mfio_readellipsoid:
 *
 */
int
ay_mfio_readellipsoid(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 MF3DEllipsoidObjPtr o = (MF3DEllipsoidObjPtr) object;
 MF3DEllipsoidDataPtr ed;
 ay_sphere_object *sphere = NULL;
 ay_object *newo = NULL;

  if(!(sphere = calloc(1,sizeof(ay_sphere_object))))
    return AY_EOMEM;

  sphere->closed = AY_FALSE;
  sphere->radius = 1.0;
  sphere->zmin = -1.0;
  sphere->zmax = 1.0;
  sphere->thetamax = 360.0;

  if(!(newo = calloc(1, sizeof(ay_object))))
    { free(sphere); return AY_EOMEM; }

  newo->type = AY_IDSPHERE;
  newo->refine = sphere;

  ay_status = ay_object_link(newo);

  if(ay_status)
    {
      ay_object_delete(newo);
      return ay_status;
    }

  ay_mfio_lastreadobject = newo;

  ay_status = ay_object_defaults(newo);

  if(o->ellipsoid)
    {
      ed = o->ellipsoid;
      newo->movx = ed->origin.x;
      newo->movy = ed->origin.y;
      newo->movz = ed->origin.z;

      /* XXXX minor & major Radius? */

    }

 return ay_status;
} /* ay_mfio_readellipsoid */


/* ay_mfio_readcylinder:
 *
 */
int
ay_mfio_readcylinder(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 MF3DCylinderObjPtr o = (MF3DCylinderObjPtr) object;
 MF3DCylinderDataPtr cd;
 ay_cylinder_object *cylinder = NULL;
 ay_object *newo = NULL;

  if(!(cylinder = calloc(1,sizeof(ay_cylinder_object))))
    return AY_EOMEM;

  cylinder->closed = AY_FALSE;
  cylinder->radius = 1.0;
  cylinder->zmin = -1.0;
  cylinder->zmax = 1.0;
  cylinder->thetamax = 360.0;

  if(!(newo = calloc(1, sizeof(ay_object))))
    { free(cylinder); return AY_EOMEM; }

  newo->type = AY_IDCYLINDER;
  newo->refine = cylinder;

  ay_status = ay_object_link(newo);

  if(ay_status)
    {
      ay_object_delete(newo);
      return ay_status;
    }

  ay_mfio_lastreadobject = newo;

  ay_status = ay_object_defaults(newo);

  if(o->cylinder)
    {
      cd = o->cylinder;
      newo->movx = cd->origin.x;
      newo->movy = cd->origin.y;
      newo->movz = cd->origin.z;

      /* XXXX orientation & minor & major Radius? */

    }

 return ay_status;
} /* ay_mfio_readcylinder */


/* ay_mfio_readcone:
 *
 */
int
ay_mfio_readcone(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 MF3DConeObjPtr o = (MF3DConeObjPtr) object;
 MF3DConeDataPtr cd;
 ay_cone_object *cone = NULL;
 ay_object *newo = NULL;

  if(!(cone = calloc(1,sizeof(ay_cone_object))))
    return AY_EOMEM;

  cone->closed = AY_FALSE;
  cone->radius = 1.0;
  cone->height = 1.0;
  cone->thetamax = 360.0;

  if(!(newo = calloc(1, sizeof(ay_object))))
    { free(cone); return AY_EOMEM; }

  newo->type = AY_IDCONE;
  newo->refine = cone;

  ay_status = ay_object_link(newo);

  if(ay_status)
    {
      ay_object_delete(newo);
      return ay_status;
    }

  ay_mfio_lastreadobject = newo;

  ay_status = ay_object_defaults(newo);

  if(o->cone)
    {
      cd = o->cone;
      newo->movx = cd->origin.x;
      newo->movy = cd->origin.y;
      newo->movz = cd->origin.z;

      /* XXXX orientation & minor & major Radius? */

    }

 return ay_status;
} /* ay_mfio_readcone */


/* ay_mfio_readdisk:
 *
 */
int
ay_mfio_readdisk(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 MF3DDiskObjPtr o = (MF3DDiskObjPtr) object;
 MF3DDiskDataPtr cd;
 ay_disk_object *disk = NULL;
 ay_object *newo = NULL;

  if(!(disk = calloc(1,sizeof(ay_disk_object))))
    return AY_EOMEM;

  disk->radius = 1.0;
  disk->height = 1.0;
  disk->thetamax = 360.0;

  if(!(newo = calloc(1, sizeof(ay_object))))
    { free(disk); return AY_EOMEM; }

  newo->type = AY_IDDISK;
  newo->refine = disk;

  ay_status = ay_object_link(newo);

  if(ay_status)
    {
      ay_object_delete(newo);
      return ay_status;
    }

  ay_mfio_lastreadobject = newo;

  ay_status = ay_object_defaults(newo);

  if(o->disk)
    {
      cd = o->disk;
      newo->movx = cd->origin.x;
      newo->movy = cd->origin.y;
      newo->movz = cd->origin.z;

      /* XXXX orientation & minor & major Radius? */

    }

 return ay_status;
} /* ay_mfio_readdisk */


/* ay_mfio_readtorus:
 *
 */
int
ay_mfio_readtorus(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 MF3DTorusObjPtr o = (MF3DTorusObjPtr) object;
 MF3DTorusDataPtr cd;
 ay_torus_object *torus = NULL;
 ay_object *newo = NULL;

  if(!(torus = calloc(1,sizeof(ay_torus_object))))
    return AY_EOMEM;

  torus->closed = AY_FALSE;
  torus->majorrad = 0.75;
  torus->minorrad = 0.25;
  torus->phimin = 0.0;
  torus->phimax = 360.0;
  torus->thetamax = 360.0;

  if(!(newo = calloc(1, sizeof(ay_object))))
    { free(torus); return AY_EOMEM; }

  newo->type = AY_IDTORUS;
  newo->refine = torus;

  ay_status = ay_object_link(newo);

  if(ay_status)
    {
      ay_object_delete(newo);
      return ay_status;
    }

  ay_mfio_lastreadobject = newo;

  ay_status = ay_object_defaults(newo);

  if(o->torus)
    {
      cd = o->torus;
      newo->movx = cd->origin.x;
      newo->movy = cd->origin.y;
      newo->movz = cd->origin.z;

      /* XXXX orientation & minor & major Radius? */

    }

 return ay_status;
} /* ay_mfio_readtorus */


/* ay_mfio_readscal:
 *
 */
int
ay_mfio_readscal(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 ay_object *o = ay_currentlevel->object;
 MF3DScaleObjPtr t = (MF3DScaleObjPtr) object;

 if(!o)
   return AY_OK;

 o->scalx *= t->scale.x;
 o->scaly *= t->scale.y;
 o->scalz *= t->scale.z;

 return ay_status;
} /* ay_mfio_readscal */


/* ay_mfio_readtran:
 *
 */
int
ay_mfio_readtran(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 ay_object *o = ay_currentlevel->object;
 MF3DTranslateObjPtr t = (MF3DTranslateObjPtr) object;

 if(!o)
   return AY_OK;

 o->movx += t->translate.x;
 o->movy += t->translate.y;
 o->movz += t->translate.z;

 return ay_status;
} /* ay_mfio_readtran */


/* ay_mfio_readrot:
 *
 */
int
ay_mfio_readrot(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 ay_object *o = ay_currentlevel->object;
 MF3DRotateObjPtr t = (MF3DRotateObjPtr) object;
 double xaxis[3] = {1.0, 0.0, 0.0};
 double yaxis[3] = {0.0, 1.0, 0.0};
 double zaxis[3] = {0.0, 0.0, 1.0};
 double quat[4] = {0}, drot;

 if(!o)
   return AY_OK;

 drot = AY_R2D(t->radians);

 if(drot != 0.0)
   { 
     if(t->axis == MF3DAxisX)
       {
	 o->rotx += drot;
	 ay_quat_axistoquat(xaxis, t->radians, quat);
	 ay_quat_add(quat, o->quat, o->quat);
       }
     else
     if(t->axis == MF3DAxisY)
       {
	 o->roty += drot;
	 ay_quat_axistoquat(yaxis, t->radians,quat);
	 ay_quat_add(quat, o->quat, o->quat);
       }
     else
     if(t->axis == MF3DAxisZ)
       {
	 o->rotz += drot;
	 ay_quat_axistoquat(zaxis, t->radians, quat);
	 ay_quat_add(quat, o->quat, o->quat);
       }
   }

 return ay_status;
} /* ay_mfio_readrot */


/* ay_mfio_readquat:
 *
 */
int
ay_mfio_readquat(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 ay_object *o = ay_currentlevel->object;
 MF3DQuaternionObjPtr q = (MF3DQuaternionObjPtr) object;
 double quat[4] = {0}, euler[3] = {0};

  if(!o)
    return AY_OK;

  quat[0] = q->quaternion.w;
  quat[1] = q->quaternion.x;
  quat[2] = q->quaternion.y;
  quat[3] = q->quaternion.z;

  ay_quat_add(quat, o->quat, o->quat);
  ay_quat_toeuler(o->quat, euler);
  o->rotx = euler[2];
  o->roty = euler[1];
  o->rotz = euler[0];

 return ay_status;
} /* ay_mfio_readquat */


/* ay_mfio_readrotaaxis:
 * XXXX this is only valid for axes located in the origin!
 */
int
ay_mfio_readrotaaxis(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 ay_object *o = ay_currentlevel->object;
 MF3DRotateAboutAxisObjPtr t = (MF3DRotateAboutAxisObjPtr) object;
 double axis[3] = {0};
 double quat[4] = {0}, euler[3] = {0}, drot;

 if(!o)
   return AY_OK;

 drot = AY_R2D(t->radians);

 if(drot != 0.0)
   {
     axis[0] = t->orientation.x - t->origin.x;
     axis[1] = t->orientation.y - t->origin.y;
     axis[2] = t->orientation.z - t->origin.z;

     ay_quat_axistoquat(axis, t->radians, quat);
     ay_quat_add(quat, o->quat, o->quat);
     ay_quat_toeuler(o->quat, euler);
     o->rotx = euler[2];
     o->roty = euler[1];
     o->rotz = euler[0];
   }

 return ay_status;
} /* ay_mfio_readrotaaxis */


/* ay_mfio_readdcol:
 *
 */
int
ay_mfio_readdcol(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 ay_object *o = ay_currentlevel->object;
 /* MF3DDiffuseColorObjPtr c = (MF3DDiffuseColorObjPtr) object;*/

 if(!o)
   return AY_OK;

 /*
 o->colr = c->diffuseColor.red;
 o->colg = c->diffuseColor.green;
 o->colb = c->diffuseColor.blue;
 */

 return ay_status;
} /* ay_mfio_readdcol */


/* ay_mfio_readtcol:
 *
 */
int
ay_mfio_readtcol(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 ay_object *o = ay_currentlevel->object;
 /*MF3DTransparencyColorObjPtr c = (MF3DTransparencyColorObjPtr) object;*/

 if(!o)
   return AY_OK;

 /*
 o->opr = c->transparency.red;
 o->opg = c->transparency.green;
 o->opb = c->transparency.blue;
 */

 return ay_status;
} /* ay_mfio_readtcol */


/* ay_mfio_readcntr:
 *
 */
int
ay_mfio_readcntr(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 ay_level_object *newl = NULL;
 ay_object *newo = NULL;

  if(!(newl = calloc(1, sizeof(ay_level_object))))
    { return AY_EOMEM; }
 
  if(!(newo = calloc(1, sizeof(ay_object))))
    { free(newl); return AY_EOMEM; }
 
  newo->refine = newl;

  newo->type = AY_IDLEVEL;

  ay_mfio_lastreadobject = newo;

  ay_status = ay_object_link(newo);

  ay_clevel_add(newo);

  ay_next = &(newo->down);

  ay_status = ay_object_defaults(newo);

 return ay_status;
} /* ay_mfio_readcntr */


/* ay_mfio_readecntr:
 *
 */
int
ay_mfio_readecntr(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 char fname[] = "mfio_readecntr";

 ay_status = ay_object_crtendlevel(ay_next);

 if(ay_status)
   {
     ay_error(AY_ERROR, fname,
	      "Could not create EndLevel object, scene is corrupt now!");
   }

 ay_mfio_lastreadobject = ay_currentlevel->object;

 ay_clevel_del();

 return ay_status;
} /* ay_mfio_readecntr */


/* ay_mfio_readobject:
 *
 */
int
ay_mfio_readobject(MF3DVoidObjPtr object)
{
 int ay_status = AY_OK;
 char fname[] = "mfio_readobject";
 Tcl_HashTable *ht = &ay_mfio_read_ht;
 Tcl_HashEntry *entry = NULL;
 ay_mfio_readcb *cb = NULL;
 char err[255];

  if((entry = Tcl_FindHashEntry(ht, (char *)(object->objectType))))
    {
      cb = (ay_mfio_readcb*)Tcl_GetHashValue(entry);
      if(cb)
	{
	  ay_status = cb(object);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Error importing object.");
	      ay_status = AY_OK;
	    }
	}
    }
  else
    {
      sprintf(err,"No callback registered for this type: %d.",
	      object->objectType);
      ay_error(AY_EWARN, fname, err);
      ay_status = AY_OK;
    }
 
 return ay_status;
} /* ay_mfio_readobject */


/* ay_mfio_readscene:
 *
 */
int
ay_mfio_readscene(Tcl_Interp *interp, char *filename)
{
 MF3D_FilePtr metafilePtr;	/* MF3D internal file pointer   */
 MF3DVoidObjPtr object;	        /* object returned by read      */
 MF3DVoidObjPtr extStorageObj = NULL;
 MF3DBoolean nextObjIsRoot;	/* next obj read will be root   */
 MF3DErr status = kMF3DNoErr;	/* temporary result code        */
 int ay_status = AY_OK;

  /* Open the metafile */
  status = MF3DOpenInputStdCFile (filename, &metafilePtr);
  if(status != kMF3DNoErr)
    {
      if(status > kMF3DTopError)
	{
	  ay_mfio_printerr(status);
	  status = kMF3DNoErr;
	}
      else
	{
	  ay_mfio_mf3d_errno = status;
	  return AY_ERROR;
	}
    }

  nextObjIsRoot = kMF3DBooleanFalse;

  while (status == kMF3DNoErr)
    {				
      /* Read an object from the metafile */
      status = MF3DReadAnObject (metafilePtr, &object);

      if (status != kMF3DNoErr)
	{
	  if (status != kMF3DNoMoreObjects)
	    { 
	      ay_mfio_mf3d_errno = status;
	      return AY_ERROR;
	    }
	}

      if ((status == kMF3DNoErr) && object->objectType == kMF3DObjReference)
	{	/* If we have just read a Reference object, resolve it.
		 * There are two types of references: local and external.
		 * An external reference will be the root object of a container
		 * and will have a subobject specifying the path
		 * to an external file.
		 */
	  extStorageObj = NULL;
	  if (nextObjIsRoot == kMF3DBooleanTrue)
	    {	/* We are in a container which must mean we have an
		 * external reference.
		 */
	      status = MF3DReadAnObject (metafilePtr, &extStorageObj);

	      if (status != kMF3DNoErr)
		{ 
		  ay_mfio_mf3d_errno = status;
		  
		  return AY_ERROR;
		}

	    }

	  /* Resolve the reference. This causes metafilePtr to point
	   * to the referenced object, so that the next call to
	   * MF3DReadAnObject will read the object which is being
	   * referenced. MF3DReadAnObject will then restore the correct
	   * file position after the referenced object has been read.
	   *
	   * If the reference is local, then extStorageObj is NULL.
	   * If the reference is external, then the file specified by
	   * extStorageObj will be opened automatically (and closed
	   * automatically after the reference has been resolved).
	   */
	  if(status == kMF3DNoErr)
	    status = MF3DResolveReference (metafilePtr,
					 (MF3DReferenceObjPtr) object,
					 (MF3DStorageObjPtr) extStorageObj);

	  /* Dispose the external storage object, if there is one.
	   * The Reference object will be disposed later.
	   */
	  MF3DDisposeObject (extStorageObj);
	}

      /* If the object we just read is a BeginContainer object, the next
       * object read will be the root object of the container.
       */
      if (status == kMF3DNoErr && object->objectType == kMF3DObjContainer)
	nextObjIsRoot = kMF3DBooleanTrue;
      else
	nextObjIsRoot = kMF3DBooleanFalse;

      if (status == kMF3DNoErr)
	{
	  ay_status = ay_mfio_readobject(object);
	  if(ay_status)
	    return ay_status;
	}
      /* Dispose the object */
      if (status == kMF3DNoErr)
	MF3DDisposeObject(object);

    }

  /* Close the file */
  status = MF3DClose (metafilePtr);
  if (status != kMF3DNoErr)
    return AY_ECLOSEFILE;

  return AY_OK;
} /* ay_mfio_readscene */


/* ay_mfio_writetrimcurve:
 *
 */
int 
ay_mfio_writetrimcurve(MF3D_FilePtr fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 int i = 0, a = 0, b = 0;
 MF3DNURBCurve2DObj mf3do = {0};
 MF3DErr status = kMF3DNoErr;	/* temporary result code */  
 ay_nurbcurve_object *curve = (ay_nurbcurve_object*)(o->refine);
 GLdouble m[16] = {0};
 GLdouble mr[16];
 double x,y,z,w;

  /* we fill the MF3D-object now */
  mf3do.objectType = kMF3DObjNURBCurve2D;
  mf3do.nPoints = curve->length;
  mf3do.order = curve->order;

  if(!(mf3do.points = calloc(curve->length, 
			    sizeof(MF3DRationalPoint3D))))
    return AY_EOMEM;
  if(!(mf3do.knots = calloc(curve->length+curve->order,
			    sizeof(MF3DKnot))))
    { free(mf3do.points); return AY_EOMEM; }

  /* get curves transformation-matrix */
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity ();
  glTranslated(o->movx, o->movy, o->movz);

  ay_quat_torotmatrix(o->quat, mr);
  glMultMatrixd(mr);

  glScaled (o->scalx, o->scaly, o->scalz);
  glGetDoublev(GL_MODELVIEW_MATRIX, m);
  glPopMatrix();

  a = 0; b = 0;
  for(i=0; i<curve->length; i++)
    {
      x = curve->controlv[b];
      b++;
      y = curve->controlv[b];
      b++;
      z = curve->controlv[b];
      b++;
      w = curve->controlv[b];
      b++;


      (mf3do.points)[a].x = m[0]*x + m[4]*y + m[8]*z + m[12]*w;
      (mf3do.points)[a].y = m[1]*x + m[5]*y + m[9]*z + m[13]*w;
      (mf3do.points)[a].w = w;


      a++;
    }

  a = 0;
  for(i=0; i<curve->length+curve->order; i++)
    {
      (mf3do.knots)[a] = curve->knotv[a]; 
      a++;
    }

  /* we write out the curve now */
  status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&mf3do);
  if(status != kMF3DNoErr)
    { free(mf3do.points); free(mf3do.knots); ay_mfio_mf3d_errno = status; 
      return AY_ERROR; }

  /* delete the object */
  free(mf3do.points);
  free(mf3do.knots);

 return ay_status;
} /* ay_mfio_writetrimcurve */


/* ay_mfio_writenurbpatch:
 *
 */
int 
ay_mfio_writenurbpatch(MF3D_FilePtr fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 int i = 0, j = 0, a = 0, b = 0;
 MF3DNurbPatchObj mf3do = {0};
 MF3DTrimCurvesObj tco = {0};
 MF3DErr status = kMF3DNoErr;	/* temporary result code */  
 ay_nurbpatch_object *patch = (ay_nurbpatch_object*)(o->refine);
 ay_object *trim;

  /* write enclosing container */
  ay_status = ay_mfio_writecntr(fileptr);
  if(ay_status)
    { return ay_status; }
 
  /* we fill the MF3D-object now */
  mf3do.objectType = kMF3DObjNURBPatch;
  mf3do.numColumns = patch->width;
  mf3do.numRows = patch->height;
  mf3do.uOrder = patch->uorder;
  mf3do.vOrder = patch->vorder;

  if(!(mf3do.points = calloc(patch->width*patch->height, 
			    sizeof(MF3DRationalPoint4D))))
    return AY_EOMEM;
  if(!(mf3do.uKnots = calloc(patch->width+patch->uorder,
			    sizeof(MF3DKnot))))
    { free(mf3do.points); return AY_EOMEM; }
  if(!(mf3do.vKnots = calloc(patch->height+patch->vorder,
			    sizeof(MF3DKnot))))
    { free(mf3do.points); free(mf3do.vKnots); return AY_EOMEM; }

  a = 0; b = 0;
  for(i=0; i<patch->width; i++)
    for(j=0; j<patch->height; j++)
      {
	(mf3do.points)[a].x = patch->controlv[b++];
	(mf3do.points)[a].y = patch->controlv[b++];
	(mf3do.points)[a].z = patch->controlv[b++];
	(mf3do.points)[a].w = patch->controlv[b++];

	a++;
      }
  a = 0;
  for(i=0; i<patch->width+patch->uorder; i++)
    {
      (mf3do.uKnots)[a] = patch->uknotv[a]; 
      a++;
    }
  a = 0;
  for(i=0; i<patch->height+patch->vorder; i++)
    {
      (mf3do.vKnots)[a] = patch->vknotv[a];
      a++;
    }

  /* we write out the patch now */
  status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&mf3do);
  if(status != kMF3DNoErr)
    { free(mf3do.points); free(mf3do.vKnots); free(mf3do.uKnots);
      ay_mfio_mf3d_errno = status; return AY_ERROR; }

  /* write attributes */
  ay_status = ay_mfio_writeattributes(fileptr, o);
  if(ay_status)
    { free(mf3do.points); free(mf3do.vKnots); free(mf3do.uKnots);
      return ay_status; }

  /* write trimcurves */
  trim = o->down;
  if(trim)
    {

      /* write enclosing container */
      ay_status = ay_mfio_writecntr(fileptr);
      if(ay_status)
	{ free(mf3do.points); free(mf3do.vKnots); free(mf3do.uKnots);
          return ay_status; }

      tco.objectType = kMF3DObjTrimCurves;
      /* we write out the trimcurve-object now */
      status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&tco);
      if(status != kMF3DNoErr)
	{ free(mf3do.points); free(mf3do.vKnots); free(mf3do.uKnots);
	ay_mfio_mf3d_errno = status; return AY_ERROR; }

      if(trim->type == AY_IDNCURVE)       /* single trimcurve? */
	{
	  ay_status = ay_mfio_writetrimcurve(fileptr, trim);
	  if(ay_status)
	    { free(mf3do.points); free(mf3do.vKnots); free(mf3do.uKnots);
	      return ay_status; }
	}
      else if(trim->type == AY_IDLEVEL)      /* trimloop? */
	{
	  trim = trim->down;
	  while(trim)
	    {
	      ay_status = ay_mfio_writetrimcurve(fileptr, trim);
	      if(ay_status)
		{ free(mf3do.points); free(mf3do.vKnots); free(mf3do.uKnots);
		return ay_status; }

	      trim = trim->next;
	    } /* while */

	} /* if */

      /* close container */
      ay_status = ay_mfio_writeecntr(fileptr);
      if(ay_status)
	{ free(mf3do.points); free(mf3do.vKnots); free(mf3do.uKnots);
	  return ay_status; }

    } /* if */
    
  /* delete the object */
  free(mf3do.points);
  free(mf3do.vKnots);
  free(mf3do.uKnots);

  /* close container */
  ay_status = ay_mfio_writeecntr(fileptr);

 return ay_status;
} /* ay_mfio_writenurbpatch */


/* ay_mfio_writenurbcurve:
 *
 */
int 
ay_mfio_writenurbcurve(MF3D_FilePtr fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 int i = 0, a = 0, b = 0;
 MF3DNURBCurveObj mf3do = {0};
 MF3DErr status = kMF3DNoErr;	/* temporary result code */  
 ay_nurbcurve_object *curve = (ay_nurbcurve_object*)(o->refine);

  /* write enclosing container */
  ay_status = ay_mfio_writecntr(fileptr);
  if(ay_status)
    { return ay_status; }

 
  /* we fill the MF3D-object now */
  mf3do.objectType = kMF3DObjNURBCurve;
  mf3do.nPoints = curve->length;
  mf3do.order = curve->order;

  if(!(mf3do.points = calloc(curve->length, 
			    sizeof(MF3DRationalPoint4D))))
    return AY_EOMEM;
  if(!(mf3do.knots = calloc(curve->length+curve->order,
			    sizeof(MF3DKnot))))
    { free(mf3do.points); return AY_EOMEM; }

  a = 0; b = 0;
  for(i=0; i<curve->length; i++)
    {
	(mf3do.points)[a].x = curve->controlv[b++];
	(mf3do.points)[a].y = curve->controlv[b++];
	(mf3do.points)[a].z = curve->controlv[b++];
	(mf3do.points)[a].w = curve->controlv[b++];

	a++;
      }
  a = 0;
  for(i=0; i<curve->length+curve->order; i++)
    {
      (mf3do.knots)[a] = curve->knotv[a]; 
      a++;
    }

  /* we write out the curve now */
  status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&mf3do);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

  /* write attributes */
  ay_status = ay_mfio_writeattributes(fileptr, o);
  if(ay_status)
    { free(mf3do.points); free(mf3do.knots); return ay_status; }

  /* delete the object */
  free(mf3do.points);
  free(mf3do.knots);

  /* close container */
  ay_status = ay_mfio_writeecntr(fileptr);

 return ay_status;
} /* ay_mfio_writenurbcurve */


/* ay_mfio_writebox:
 *
 */
int 
ay_mfio_writebox(MF3D_FilePtr fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 MF3DBoxObj mf3do = {0};
 MF3DBoxData bd;
 MF3DErr status = kMF3DNoErr;	/* temporary result code */  
 ay_box_object *box = (ay_box_object*)(o->refine);

  /* write enclosing container */
  ay_status = ay_mfio_writecntr(fileptr);
  if(ay_status)
    { return ay_status; }

  /* we fill the MF3D-object now */
  bd.orientation.x = 0.0;
  bd.orientation.y = 0.0;
  bd.orientation.z = box->length;

  bd.majorAxis.x = 0.0;
  bd.majorAxis.y = box->height;
  bd.majorAxis.z = 0.0;

  bd.minorAxis.x = box->width;
  bd.minorAxis.y = 0.0;
  bd.minorAxis.z = 0.0;

  bd.origin.x = -box->width/2.0;
  bd.origin.y = -box->height/2.0;
  bd.origin.z = -box->length/2.0;

  mf3do.objectType = kMF3DObjBox;
  mf3do.box = &bd;

  /* we write out the box now */
  status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&mf3do);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

  /* write attributes */
  ay_status = ay_mfio_writeattributes(fileptr, o);
  if(ay_status)
    return ay_status;

  /* close container */
  ay_status = ay_mfio_writeecntr(fileptr);

 return ay_status;
} /* ay_mfio_writebox */


/* ay_mfio_writesphere:
 *
 */
int 
ay_mfio_writesphere(MF3D_FilePtr fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 MF3DEllipsoidObj mf3do = {0};
 MF3DEllipsoidData ed;
 MF3DErr status = kMF3DNoErr;	/* temporary result code */  
 ay_sphere_object *sphere = (ay_sphere_object*)(o->refine);

  /* write enclosing container */
  ay_status = ay_mfio_writecntr(fileptr);
  if(ay_status)
    { return ay_status; }

  /* we fill the MF3D-object now */
  ed.orientation.x = 0.0;
  ed.orientation.y = 0.0;
  ed.orientation.z = sphere->radius;

  ed.majorRadius.x = 0.0;
  ed.majorRadius.y = sphere->radius;
  ed.majorRadius.z = 0.0;

  ed.minorRadius.x = sphere->radius;
  ed.minorRadius.y = 0.0;
  ed.minorRadius.z = 0.0;

  ed.origin.x = 0.0;
  ed.origin.y = 0.0;
  ed.origin.z = 0.0;

  mf3do.objectType = kMF3DObjEllipsoid;
  mf3do.ellipsoid = &ed;

  /* we write out the sphere now */
  status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&mf3do);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

  /* write attributes */
  ay_status = ay_mfio_writeattributes(fileptr, o);
  if(ay_status)
    return ay_status;

  /* close container */
  ay_status = ay_mfio_writeecntr(fileptr);

 return ay_status;
} /* ay_mfio_writesphere */


/* ay_mfio_writecylinder:
 *
 */
int 
ay_mfio_writecylinder(MF3D_FilePtr fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 MF3DCylinderObj mf3do = {0};
 MF3DCylinderData cd;
 MF3DCapsObj cp;
 MF3DErr status = kMF3DNoErr;	/* temporary result code */  
 ay_cylinder_object *cylinder = (ay_cylinder_object*)(o->refine);

  /* write enclosing container */
  ay_status = ay_mfio_writecntr(fileptr);
  if(ay_status)
    { return ay_status; }

  /* we fill the MF3D-object now */
  cd.orientation.x = 0.0;
  cd.orientation.y = 0.0;
  cd.orientation.z = cylinder->zmax - cylinder->zmin;

  cd.majorAxis.x = 0.0;
  cd.majorAxis.y = cylinder->radius;
  cd.majorAxis.z = 0.0;

  cd.minorAxis.x = cylinder->radius;
  cd.minorAxis.y = 0.0;
  cd.minorAxis.z = 0.0;

  cd.origin.x = 0.0;
  cd.origin.y = 0.0;
  cd.origin.z = cylinder->zmin;

  mf3do.objectType = kMF3DObjCylinder;
  mf3do.cylinder = &cd;

  /* we write out the cylinder now */
  status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&mf3do);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

  /* closed? */
  if(cylinder->closed)
    {
      cp.objectType = kMF3DObjCaps;
      cp.caps = kCapsBottom | kCapsTop;

      status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&cp);
      if(status != kMF3DNoErr)
	{ ay_mfio_mf3d_errno = status; return AY_ERROR; }
    }

  /* write attributes */
  ay_status = ay_mfio_writeattributes(fileptr, o);
  if(ay_status)
    return ay_status;

  /* close container */
  ay_status = ay_mfio_writeecntr(fileptr);

 return ay_status;
} /* ay_mfio_writecylinder */


/* ay_mfio_writecone:
 *
 */
int 
ay_mfio_writecone(MF3D_FilePtr fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 MF3DConeObj mf3do = {0};
 MF3DConeData cd;
 MF3DCapsObj cp;
 MF3DErr status = kMF3DNoErr;	/* temporary result code */  
 ay_cone_object *cone = (ay_cone_object*)(o->refine);

  /* write enclosing container */
  ay_status = ay_mfio_writecntr(fileptr);
  if(ay_status)
    { return ay_status; }

  /* we fill the MF3D-object now */
  cd.orientation.x = 0.0;
  cd.orientation.y = 0.0;
  cd.orientation.z = cone->height;

  cd.majorAxis.x = 0.0;
  cd.majorAxis.y = cone->radius;
  cd.majorAxis.z = 0.0;

  cd.minorAxis.x = cone->radius;
  cd.minorAxis.y = 0.0;
  cd.minorAxis.z = 0.0;

  cd.origin.x = 0.0;
  cd.origin.y = 0.0;
  cd.origin.z = 0.0;

  mf3do.objectType = kMF3DObjCone;
  mf3do.cone = &cd;

  /* we write out the cone now */
  status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&mf3do);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

  /* closed? */
  if(cone->closed)
    {
      cp.objectType = kMF3DObjCaps;
      cp.caps = kCapsBottom;

      status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&cp);
      if(status != kMF3DNoErr)
	{ ay_mfio_mf3d_errno = status; return AY_ERROR; }
    }

  /* write attributes */
  ay_status = ay_mfio_writeattributes(fileptr, o);
  if(ay_status)
    return ay_status;

  /* close container */
  ay_status = ay_mfio_writeecntr(fileptr);

 return ay_status;
} /* ay_mfio_writecone */


/* ay_mfio_writedisk:
 *
 */
int 
ay_mfio_writedisk(MF3D_FilePtr fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 MF3DDiskObj mf3do = {0};
 MF3DDiskData dd;
 MF3DErr status = kMF3DNoErr;	/* temporary result code */  
 ay_disk_object *disk = (ay_disk_object*)(o->refine);

  /* write enclosing container */
  ay_status = ay_mfio_writecntr(fileptr);
  if(ay_status)
    { return ay_status; }

  /* we fill the MF3D-object now */
  dd.majorRadius.x = 0.0;
  dd.majorRadius.y = disk->radius;
  dd.majorRadius.z = 0.0;

  dd.minorRadius.x = disk->radius;
  dd.minorRadius.y = 0.0;
  dd.minorRadius.z = 0.0;

  dd.origin.x = 0.0;
  dd.origin.y = 0.0;
  dd.origin.z = disk->height;

  mf3do.objectType = kMF3DObjDisk;
  mf3do.disk = &dd;

  /* we write out the disk now */
  status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&mf3do);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

  /* write attributes */
  ay_status = ay_mfio_writeattributes(fileptr, o);
  if(ay_status)
    return ay_status;

  /* close container */
  ay_status = ay_mfio_writeecntr(fileptr);

 return ay_status;
} /* ay_mfio_writedisk */


/* ay_mfio_writetorus:
 *
 */
int 
ay_mfio_writetorus(MF3D_FilePtr fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 MF3DTorusObj mf3do = {0};
 MF3DTorusData td;
 MF3DErr status = kMF3DNoErr;	/* temporary result code */  
 ay_torus_object *torus = (ay_torus_object*)(o->refine);

  /* write enclosing container */
  ay_status = ay_mfio_writecntr(fileptr);
  if(ay_status)
    { return ay_status; }

  /* we fill the MF3D-object now */
  td.orientation.x = 0.0;
  td.orientation.y = 0.0;
  td.orientation.z = torus->minorrad;

  td.majorAxis.x = 0.0;
  td.majorAxis.y = torus->majorrad;
  td.majorAxis.z = 0.0;

  td.minorAxis.x = torus->majorrad;
  td.minorAxis.y = 0.0;
  td.minorAxis.z = 0.0;

  td.ratio = 1.0;

  td.origin.x = 0.0;
  td.origin.y = 0.0;
  td.origin.z = 0.0;

  mf3do.objectType = kMF3DObjTorus;
  mf3do.torus = &td;

  /* we write out the torus now */
  status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&mf3do);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

  /* write attributes */
  ay_status = ay_mfio_writeattributes(fileptr, o);
  if(ay_status)
    return ay_status;

  /* close container */
  ay_status = ay_mfio_writeecntr(fileptr);

 return ay_status;
} /* ay_mfio_writetorus */


/* ay_mfio_writeattributes:
 *
 */
int 
ay_mfio_writeattributes(MF3D_FilePtr fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 double euler[3] = {0};
 MF3DTranslateObj to = {0};
 MF3DScaleObj so = {0};
 MF3DRotateObj ro = {0};
 /* MF3DDiffuseColorObj dco = {0};
 MF3DTransparencyColorObj tco = {0};*/
 MF3DErr status = kMF3DNoErr;	/* temporary result code */  

  to.objectType = kMF3DObjTranslateTransform;
  to.translate.x = (MF3DFloat32)o->movx;
  to.translate.y = (MF3DFloat32)o->movy;
  to.translate.z = (MF3DFloat32)o->movz;

  if((o->movx != 0.0) || (o->movy != 0.0) || (o->movz != 0.0))
    status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&to);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

  ay_quat_toeuler(o->quat,euler);

  ro.objectType = kMF3DObjRotateTransform;
  ro.axis = MF3DAxisZ;
  ro.radians = -euler[0];
  if(euler[0] != 0.0)
    status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&ro);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

  ro.axis = MF3DAxisY;
  ro.radians = -euler[1];
  if(euler[1] != 0.0)
    status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&ro);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

  ro.axis = MF3DAxisX;
  ro.radians = -euler[2];
  if(euler[2] != 0.0)
    status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&ro);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

  so.objectType = kMF3DObjScaleTransform;
  so.scale.x = o->scalx;
  so.scale.y = o->scaly;
  so.scale.z = o->scalz;

  if((o->scalx != 1.0) || (o->scaly != 1.0) || (o->scalz != 1.0))
    status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&so);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

  /*
    XXXX get color from material
  dco.objectType = kMF3DObjDiffuseColor;
  dco.diffuseColor.red = o->colr;
  dco.diffuseColor.green = o->colg;
  dco.diffuseColor.blue = o->colb;
  status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&dco);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

  tco.objectType = kMF3DObjTransparencyColor;
  tco.transparency.red = o->opr;
  tco.transparency.green = o->opg;
  tco.transparency.blue = o->opb;
  status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&tco);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }
  */

 return ay_status;
} /* ay_mfio_writeattributes */


/* ay_mfio_writelevel:
 *
 */
int 
ay_mfio_writelevel(MF3D_FilePtr fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *level = o;

  if(!o)
    return AY_OK;
  
  o = o->down;
  if(o)
    {
      ay_status = ay_mfio_writecntr(fileptr);
      ay_status = ay_mfio_writeattributes(fileptr, level);
      while(o)
	{
	  ay_status = ay_mfio_writeobject(fileptr, o);
	  o = o->next;
	}

      ay_status = ay_mfio_writeecntr(fileptr);
    }

 return ay_status;
} /* ay_mfio_writelevel */


/* ay_mfio_writeinstance:
 *
 */
int 
ay_mfio_writeinstance(MF3D_FilePtr fileptr, ay_object *o)
{
 int ay_status = AY_OK;
 ay_object *orig = NULL;
 double quat[4] = {0}, rotx, roty, rotz, scalx, scaly, scalz;
 double movx, movy, movz;

  if(!o)
    return AY_OK;
  
  orig = (ay_object *)o->refine;

  memcpy(quat, orig->quat, 4*(sizeof(double)));
  rotx = orig->rotx;
  roty = orig->roty;
  rotz = orig->rotz;

  scalx = orig->scalx;
  scaly = orig->scaly;
  scalz = orig->scalz;

  movx = orig->movx;
  movy = orig->movy;
  movz = orig->movz;

  ay_status = ay_mfio_writeobject(fileptr, orig);

  memcpy(orig->quat, quat, 4*(sizeof(double)));

  orig->rotx = rotx;
  orig->roty = roty;
  orig->rotz = rotz;

  orig->scalx = scalx;
  orig->scaly = scaly;
  orig->scalz = scalz;

  orig->movx = movx;
  orig->movy = movy;
  orig->movz = movz;

 return ay_status;
} /* ay_mfio_writeinstance */


/* ay_mfio_writecntr:
 *
 */
int 
ay_mfio_writecntr(MF3D_FilePtr fileptr)
{
 int ay_status = AY_OK;
 MF3DContainerObj mf3do = {0};
 MF3DErr status = kMF3DNoErr;	/* temporary result code */  
  
  mf3do.objectType = kMF3DObjContainer;

  /* we write out a container now */
  status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&mf3do);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

 return ay_status;
} /* ay_mfio_writecntr */


/* ay_mfio_writeecntr:
 *
 */
int 
ay_mfio_writeecntr(MF3D_FilePtr fileptr)
{
 int ay_status = AY_OK;
 MF3DEndContainerObj mf3do = {0};
 MF3DErr status = kMF3DNoErr;	/* temporary result code */  
  
  mf3do.objectType = kMF3DObjEndContainer;
  status = MF3DWriteAnObject (fileptr, (MF3DVoidObjPtr)&mf3do);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

 return ay_status;
} /* ay_mfio_writeecntr */


/* ay_mfio_writeobject:
 *
 */
int 
ay_mfio_writeobject(MF3D_FilePtr fileptr, ay_object *object)
{
 int ay_status = AY_OK;
 char fname[] = "mfio_readobject";
 Tcl_HashTable *ht = &ay_mfio_write_ht;
 Tcl_HashEntry *entry = NULL;
 char err[255];
 ay_mfio_writecb *cb = NULL;

  if((entry = Tcl_FindHashEntry(ht, (char *)(object->type))))
    {
      cb = (ay_mfio_writecb*)Tcl_GetHashValue(entry);
      if(cb)
	{
	  ay_status = cb(fileptr, object);
	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Error exporting object.");
	      ay_status = AY_OK;
	    }
	}
    }
  else
    {
      sprintf(err,"No callback registered for this type: %d.",
	      object->type);
      ay_error(AY_EWARN, fname, err);
    }
 
 return ay_status;
} /* ay_mfio_writeobject */


/* ay_mfio_writescene:
 *
 */
int
ay_mfio_writescene(Tcl_Interp *interp, char *filename)
{
 int ay_status = AY_OK;
 ay_object *co = ay_root->next;
 MF3D_FilePtr metafilePtr;	/* MF3D internal file pointer   */
 MF3DErr status = kMF3DNoErr;	/* temporary result code        */
 MF3DMetafileObj mfo = {0};

  mfo.objectType = kMF3DObjMetafile;
  mfo.dataFormat = ay_mfio_mf3d_data_format;
  mfo.majorVersion = (MF3DUns16)1;
  mfo.minorVersion = (MF3DUns16)1;
  mfo.flags = kMF3DFormatStream;
  mfo.refSeed = kMF3DMaximumRefSeed;
  mfo.typeSeed = kMF3DMinimumTypeSeed;

  /* Open the metafile */
  status = MF3DOpenOutputStdCFile (ay_mfio_mf3d_data_format, filename,
				   &metafilePtr);
  if (status != kMF3DNoErr)
    return AY_EOPENFILE;

  /* write the metafile-object */
  status = MF3DWriteAnObject (metafilePtr, (MF3DVoidObjPtr)&mfo);
  if(status != kMF3DNoErr)
    { ay_mfio_mf3d_errno = status; return AY_ERROR; }

  while(co)
    {
      ay_status = ay_mfio_writeobject(metafilePtr, co);
      if (ay_status)
	return ay_status;

      co = co->next;
    }

  /* Close the file */
  status = MF3DClose (metafilePtr);
  if (status != kMF3DNoErr)
    return AY_ECLOSEFILE;

 return AY_OK;
} /* ay_mfio_writescene */


/* ay_mfio_registerwritecb:
 *
 */
int
ay_mfio_registerwritecb(char *name, ay_mfio_writecb *cb)
{
 int ay_status = AY_OK;
 int new_item = 0;
 Tcl_HashEntry *entry = NULL;
 Tcl_HashTable *ht = &ay_mfio_write_ht;

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
} /* ay_mfio_registerwritecb */


/* ay_mfio_importscenetcmd:
 *
 */
int
ay_mfio_importscenetcmd(ClientData clientData, Tcl_Interp * interp,
			int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "3dmf_import";

 /* check args */
 if(argc != 2)
   {
     ay_error(AY_EARGS, fname, "filename");
     return TCL_OK;
   }

 ay_mfio_lastreadobject = NULL;

 ay_status = ay_mfio_readscene(interp, argv[1]);

  if(ay_status)
    {
      ay_mfio_printerr(ay_mfio_mf3d_errno);
      ay_error(AY_ERROR, fname, "Error while importing from:");
      ay_error(AY_ERROR, fname, argv[1]);
    }
  else
    {
      ay_error(AY_EOUTPUT, fname, "Done importing scene from:");
      ay_error(AY_EOUTPUT, fname, argv[1]);
    }

 return TCL_OK;
} /* ay_mfio_importscenetcmd */


/* ay_mfio_exportscenetcmd:
 *
 */
int
ay_mfio_exportscenetcmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "3dmf_export";

  /* check args */
  if(argc != 2)
    {
      ay_error(AY_EARGS, fname, "filename");
      return TCL_OK;
    }

  ay_status = ay_mfio_writescene(interp, argv[1]);
  
  if(ay_status)
    {
      ay_mfio_printerr(ay_mfio_mf3d_errno);
      ay_error(AY_ERROR, fname, "Error while exporting to:");
      ay_error(AY_ERROR, fname, argv[1]);
    }
  else
    {
      ay_error(AY_EOUTPUT, fname, "Done exporting scene to:");
      ay_error(AY_EOUTPUT, fname, argv[1]);
    }

 return TCL_OK;
} /* ay_mfio_exportscenetcmd */


/* ay_mfio_printerr:
 *
 */
void
ay_mfio_printerr(MF3DErr errcode)
{
 char fname[] = "MFIO";

  switch(errcode)
    {
    case kMF3DErrInvalidParameter:
      ay_error(AY_ERROR, fname, "Invalid Parameter");
      break;
    case kMF3DErrOutOfMemory:
      ay_error(AY_EOMEM, fname, NULL);
      break;
    case kMF3DErrNoObjectsFound:
      ay_error(AY_ERROR, fname, "No Objects Found");
      break;
    case kMF3DErrCantParse:
      ay_error(AY_ERROR, fname, "Cant Parse");
      break;
    case kMF3DErrDidntReadEntireObj:
      ay_error(AY_ERROR, fname, "Didnt Read Entire Obj");
      break;
    case kMF3DErrObjHasTooManyLabels:
      ay_error(AY_ERROR, fname, "Obj Has Too Many Labels");
      break;
    case kMF3DErrIllegalObjName:
      ay_error(AY_ERROR, fname, "Illegal Obj Name");
      break;
    case kMF3DErrUnquotedString:
      ay_error(AY_ERROR, fname, "Unquoted String");
      break;
    case kMF3DErrRawDataOddNumberOfHexChars:
      ay_error(AY_ERROR, fname, "Raw Data Odd Number Of Hex Chars");
      break;
    case kMF3DErrIllegalEnum:
      ay_error(AY_ERROR, fname, "Illegal Enum");
      break;
    case kMF3DErrIllegalDataType:
      ay_error(AY_ERROR, fname, "Illegal Data Type");
      break;
    case kMF3DErrWrongObjType:
      ay_error(AY_ERROR, fname, "Wrong Obj Type");
      break;
    case kMF3DErrNotAReferenceObj:
      ay_error(AY_ERROR, fname, "Not A Reference Obj");
      break;
    case kMF3DErrReferenceNotFound:
      ay_error(AY_ERROR, fname, "Reference Not Found");
      break;

    case kMF3DErrTooManyStorageObjects:
      ay_error(AY_ERROR, fname, "Too Many Storage Objects");
      break;
    case kMF3DErrIllegalUserObjectType:
      ay_error(AY_ERROR, fname, "Illegal User Object Type");
      break;
    case kMF3DErrNIndicesLessThanZero:
      ay_error(AY_ERROR, fname, "NIndices Less Than Zero");
      break;
    case kMF3DErrNIndicesGreaterThanNObjects:
      ay_error(AY_ERROR, fname, "NIndices Greater Than NObjects");
      break;
    case kMF3DErrNumCornersIsZero:
      ay_error(AY_ERROR, fname, "Num Corners Is Zero");
      break;
    case kMF3DErrNumEdgesIsZero:
      ay_error(AY_ERROR, fname, "Num Edges Is Zero");
      break;
    case kMF3DErrNumFacesIsZero:
      ay_error(AY_ERROR, fname, "Num Faces Is Zero");
      break;
    case kMF3DErrOrderTooSmall:
      ay_error(AY_ERROR, fname, "Order Too Small");
      break;
    case kMF3DErrTooFewPoints:
      ay_error(AY_ERROR, fname, "Too Few Points");
      break;

    case kMF3DErrNumContoursIsZero:
      ay_error(AY_ERROR, fname, "Num Contours Is Zero");
      break;

    case kMF3DErrTooFewVertices:
      ay_error(AY_ERROR, fname, "Too Few Vertices");
      break;

    case kMF3DErrWidthOrHeightIsZero:
      ay_error(AY_ERROR, fname, "Width Or Height Is Zero");
      break;

    case kMF3DErrNotEnoughEndGroups:
      ay_error(AY_ERROR, fname, "Not Enough End Groups");
      break;
    case kMF3DErrTooManyEndGroups:
      ay_error(AY_ERROR, fname, "Too Many End Groups");
      break;
    case kMF3DErrTooManyEndContainers:
      ay_error(AY_ERROR, fname, "Too Many End Containers");
      break;
    case kMF3DErrContainedObjTooBig:
      ay_error(AY_ERROR, fname, "Contained Obj Too Big");
      break;
    case kMF3DErrCantWrite:
      ay_error(AY_ERROR, fname, "Cant Write");
      break;
    case kMF3DErrIllegalRefID:
      ay_error(AY_ERROR, fname, "Illegal Ref ID");
      break;
    case kMF3DErrTwoObjectsWithSameRefID:
      ay_error(AY_ERROR, fname, "Two Objects With Same Ref ID");
      break;
    case kMF3DErrTwoObjectsWithSameRefName:
      ay_error(AY_ERROR, fname, "Two Objects With Same Ref Name");
      break;
    case kMF3DErrGroupTypeNotSpecified:
      ay_error(AY_ERROR, fname, "Group Type Not Specified");
      break;

    case kMF3DErrFailedToOpen:
      ay_error(AY_ERROR, fname, "Failed To Open");
      break;
    case kMF3DErrReadFailedEOF:
      ay_error(AY_ERROR, fname, "ReadFailedEOF");
      break;
    case kMF3DErrReadFailed:
      ay_error(AY_ERROR, fname, "ReadFailed");
      break;
    case kMF3DErrReadPartialEOF:
      ay_error(AY_ERROR, fname, "Read Partial EOF");
      break;
    case kMF3DErrReadPartial:
      ay_error(AY_ERROR, fname, "Read Partial");
      break;

    case kMF3DErrWriteFailedEOF:
      ay_error(AY_ERROR, fname, "Write Failed EOF");
      break;
    case kMF3DErrWriteFailed:
      ay_error(AY_ERROR, fname, "Write Failed");
      break;
    case kMF3DErrWritePartialEOF:
      ay_error(AY_ERROR, fname, "Write Partial EOF");
      break;
    case kMF3DErrWritePartial:
      ay_error(AY_ERROR, fname, "Write Partial");
      break;

    case kMF3DErrTellFailed:
      ay_error(AY_ERROR, fname, "Tell Failed");
      break;
    case kMF3DErrSeekFailed:
      ay_error(AY_ERROR, fname, "Seek Failed");
      break;
    case kMF3DErrSeekOutOfRange:
      ay_error(AY_ERROR, fname, "Seek Out Of Range");
      break;

    case kMF3DErrCloseEOF:
      ay_error(AY_ERROR, fname, "Close EOF");
      break;

    case kMF3DErrUnsupportedUns64:
      ay_error(AY_ERROR, fname, "Unsupported Uns 64");
      break;
    case kMF3DErrUnsupportedInt64:
      ay_error(AY_ERROR, fname, "Unsupported Int 64");
      break;
    case kMF3DErrUnsupportedMetafileFormat:
      ay_error(AY_ERROR, fname, "Unsupported Metafile Format");
      break;

    case kMF3DWarnNewerMetafileFormat:
      ay_error(AY_EWARN, fname, "Newer Metafile Format Detected");
      break;
    case kMF3DWarnWrongUnknownTypeFormat:
      ay_error(AY_EWARN, fname, "Wrong Unknown Type Format Detected");
      break;

    default:
      ay_error(AY_ERROR, fname, "Unknown Error Code, Internal Error");
      printf("MFIO-Error: %u\n",errcode);
      break;
    } /* switch */

  return;
} /* ay_mfio_printerr */


/* Mfio_Init:
 *
 */
int
Mfio_Init(Tcl_Interp *interp)
{
 char fname[] = "mfio_init";
 int ay_status = AY_OK;


  /* first, check versions */
  if(strcmp(ay_version_ma, ay_mfio_version_ma))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "It is unsafe to continue! Bailing out...");
      return TCL_OK;
    }

  if(strcmp(ay_version_mi, ay_mfio_version_mi))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "However, it is probably safe to continue...");
    }


  /* Initialization: */
  ay_mfio_mf3d_data_format = kMF3DFormatText /*kMF3DFormatBinary*/;

  /* init hash table for read callbacks */
  Tcl_InitHashTable(&ay_mfio_read_ht, TCL_ONE_WORD_KEYS);

  /* fill hash table */
  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjNURBPatch),
				       ay_mfio_readnurbpatch);

  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjNURBCurve),
				       ay_mfio_readnurbcurve);

  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjPolyLine),
				       ay_mfio_readpolyline);

  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjGeneralPolygon),
				     ay_mfio_readgeneralpolygon);

  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjPolygon),
				     ay_mfio_readpolygon);

  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjTriangle),
				     ay_mfio_readtriangle);

  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjTriGrid),
				     ay_mfio_readtrigrid);

  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjBox),
				       ay_mfio_readbox);

  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjContainer),
				       ay_mfio_readcntr);

  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjEndContainer),
				       ay_mfio_readecntr);

  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjTranslateTransform),
				       ay_mfio_readtran);

  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjScaleTransform),
				       ay_mfio_readscal);

  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjRotateTransform),
				       ay_mfio_readrot);

  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjQuaternionTransform),
				       ay_mfio_readquat);

  ay_status = ay_mfio_registerreadcb(
				  (char *)(kMF3DObjRotateAboutAxisTransform),
				  ay_mfio_readrotaaxis);

  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjDiffuseColor),
				       ay_mfio_readdcol);

  ay_status = ay_mfio_registerreadcb((char *)(kMF3DObjTransparencyColor),
				       ay_mfio_readtcol);


  /* init hash table for write callbacks */
  Tcl_InitHashTable(&ay_mfio_write_ht, TCL_ONE_WORD_KEYS);

  /* fill hash table */
  ay_status = ay_mfio_registerwritecb((char *)(AY_IDNPATCH),
				       ay_mfio_writenurbpatch);
  ay_status = ay_mfio_registerwritecb((char *)(AY_IDNCURVE),
				       ay_mfio_writenurbcurve);
  ay_status = ay_mfio_registerwritecb((char *)(AY_IDLEVEL),
				       ay_mfio_writelevel);
  ay_status = ay_mfio_registerwritecb((char *)(AY_IDBOX),
				       ay_mfio_writebox);
  ay_status = ay_mfio_registerwritecb((char *)(AY_IDSPHERE),
				       ay_mfio_writesphere);
  ay_status = ay_mfio_registerwritecb((char *)(AY_IDDISK),
				       ay_mfio_writedisk);
  ay_status = ay_mfio_registerwritecb((char *)(AY_IDCONE),
				       ay_mfio_writecone);
  ay_status = ay_mfio_registerwritecb((char *)(AY_IDCYLINDER),
				       ay_mfio_writecylinder);
  ay_status = ay_mfio_registerwritecb((char *)(AY_IDTORUS),
				       ay_mfio_writetorus);
  ay_status = ay_mfio_registerwritecb((char *)(AY_IDINSTANCE),
				       ay_mfio_writeinstance);


  /* register some C-functions as Tcl-Commands */
  Tcl_CreateCommand (interp, "ay_mfio_import",
		     ay_mfio_importscenetcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand (interp, "ay_mfio_export",
		     ay_mfio_exportscenetcmd,
		     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  /* source mfio.tcl, it contains Tcl-code for menu entries */
  if((Tcl_EvalFile(interp, "mfio.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname,
		  "Error while sourcing \\\"mfio.tcl\\\"!");
       return TCL_OK;
     }


  ay_error(AY_EOUTPUT, fname,
	   "3DMF import/export plugin successfully loaded.");


 return TCL_OK;
} /* Mfio_Init */
