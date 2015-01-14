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

// dxfio.cpp - Ayam DXF Import/Export Plugin based on Dime

// includes:
#include "ayam.h"

// Dime includes
#include <dime/Base.h>
#include <dime/Basic.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/Model.h>
#include <dime/State.h>
#include <dime/entities/Entity.h>
#include <dime/entities/ExtrusionEntity.h>
#include <dime/entities/3DFace.h>
#include <dime/entities/Arc.h>
#include <dime/entities/Block.h>
#include <dime/entities/Circle.h>
#include <dime/entities/Ellipse.h>
#include <dime/entities/Insert.h>
#include <dime/entities/Line.h>
#include <dime/entities/LWPolyline.h>
#include <dime/entities/Polyline.h>
#include <dime/entities/Solid.h>
#include <dime/entities/Spline.h>
#include <dime/entities/Trace.h>
#include <dime/entities/UnknownEntity.h>
#include <dime/entities/Vertex.h>
#include <dime/sections/BlocksSection.h>
#include <dime/sections/EntitiesSection.h>
#include <dime/sections/TablesSection.h>
#include <dime/tables/LayerTable.h>
#include <dime/tables/Table.h>
#include <dime/util/Linear.h>

#include <stdio.h>

// local types:

typedef struct dxfio_block_s {
  struct dxfio_block_s *next;
  dimeBlock *block;
  ay_object *object;
} dxfio_block;

// global variables:

char dxfio_version_ma[] = AY_VERSIONSTR;
char dxfio_version_mi[] = AY_VERSIONSTRMI;

static Tcl_HashTable dxfio_write_ht;

// last read object
ay_object *dxfio_lrobject = NULL;
// list of master objects of inserted blocks from the DXF file
dxfio_block *dxfio_blocks = NULL;

// current transformation matrix
static double tm[16] = {0};

// total number of entities in DXF file
int dxfio_totalents = 0;

// import/export options:
int dxfio_importcurves = AY_TRUE;
int dxfio_exportcurves = AY_TRUE;
int dxfio_expselected = AY_FALSE;
int dxfio_expobeynoexport = AY_TRUE;
int dxfio_expignorehidden = AY_TRUE;
int dxfio_exptoplevellayers = AY_FALSE;

// first layer to read; -1: read all layers
int dxfio_slayer = -1;
// last layer to read; elayer==slayer: read one or all
int dxfio_elayer = -1;

int dxfio_currentlayer = 0;

// rescale knots to min dist, if <= 0.0: no scaling
double dxfio_rescaleknots = 0.0;

// global scale factor
double dxfio_scalefactor = 1.0;

// cancelled flag
int dxfio_cancelled = AY_FALSE;


// prototypes of functions local to this module:

int dxfio_countsubentities(dimeInsert *entity);

int dxfio_countentities(dimeModel *model);

int dxfio_extrudebpatch(class dimeExtrusionEntity *entity, ay_object *o);

int dxfio_getpolyfacemesh(const class dimeState *state,
			  class dimePolyline *polyline,
			  void *clientdata, ay_object *newo);

int dxfio_getpolyline(const class dimeState *state,
		      class dimePolyline *polyline,
		      void *clientdata, ay_object *newo);

int dxfio_getpolymesh(const class dimeState *state,
		      class dimePolyline *polyline,
		      void *clientdata, ay_object *newo);

int dxfio_getsmoothsurface(const class dimeState *state,
			   class dimePolyline *polyline,
			   void *clientdata, ay_object *newo);

int dxfio_linkobject(ay_object *o);

int dxfio_read3dface(const class dimeState *state,
		     class dime3DFace *tdface,
		     void *clientdata);

int dxfio_readarc(const class dimeState *state,
		  class dimeArc *arc,
		  void *clientdata);

int dxfio_readcircle(const class dimeState *state,
		     class dimeCircle *circle,
		     void *clientdata);

int dxfio_readellipse(const class dimeState *state,
		      class dimeEllipse *ellipse,
		      void *clientdata);

int dxfio_readinsert(const class dimeState *state,
		   class dimeInsert *line,
		   void *clientdata);

int dxfio_readline(const class dimeState *state,
		   class dimeLine *line,
		   void *clientdata);

int dxfio_readlwpolyline(const class dimeState *state,
			 class dimeLWPolyline *lwpolyline,
			 void *clientdata);

int dxfio_readpolyline(const class dimeState *state,
		       class dimePolyline *polyline,
		       void *clientdata);

int dxfio_readsolid(const class dimeState *state,
		    class dimeSolid *solid,
		    void *clientdata);

int dxfio_readspline(const class dimeState *state,
		     class dimeSpline *spline,
		     void *clientdata);

int dxfio_readtrace(const class dimeState *state,
		    class dimeTrace *trace,
		    void *clientdata);

int dxfio_readprogressdcb(float progress, void *clientdata);

bool dxfio_readentitydcb(const class dimeState *state,
			 class dimeEntity *entity,
			 void *clientdata);

int dxfio_readtcmd(ClientData clientData, Tcl_Interp *interp,
		   int argc, char *argv[]);

extern "C" {
#ifdef WIN32
  __declspec( dllexport )
#endif
  int Dxfio_Init(Tcl_Interp *interp);
} // extern "C"

// implementation of functions:

// dxfio_extrudebpatch:
//
int
dxfio_extrudebpatch(class dimeExtrusionEntity *entity, ay_object *o)
{
 int ay_status = AY_OK;
#if 0
 int i, j, n = 4;
 double v[12], *p1, *p2;
 ay_object *newo = NULL;
 ay_bpatch_object *bp = NULL;
 dimeVec3f ex;
 dxfdouble th;

 //entity->getExtrusionDir(ex);
 //th = entity->getThickness();

  if(th <= 0.0)
    return AY_OK;

  bp = (ay_bpatch_object *)o->refine;


  // get number of unique points
  for(i = 0; i < 4; i++)
    {
      if(i == 0)
	memcpy(&(v[0]), bp->p1, 3*sizeof(double));
      if(i == 1)
	memcpy(&(v[3]), bp->p2, 3*sizeof(double));
      if(i == 2)
	memcpy(&(v[6]), bp->p3, 3*sizeof(double));
      if(i == 3)
	memcpy(&(v[9]), bp->p4, 3*sizeof(double));
    }
  for(i = 0; i < 4; i++)
    {
      for(j = i; j < 4; j++)
	{
	  if(i != j)
	    {
	      p1 = &(v[i*3]);
	      p2 = &(v[j*3]);
	      if(AY_V3COMP(p1, p2))
		{
		  n--;
		}
	    }
	} // for
    } // for

  // create new object(s)
  switch(n)
    {
    case 0:
      break;
    default:
      break;
    }
#endif
 return ay_status;
} // dxfio_extrudebpatch


// dxfio_read3dface:
//
int
dxfio_read3dface(const class dimeState *state,
		 class dime3DFace *tdface,
		 void *clientdata)
{
 int ay_status = AY_OK;
 ay_object *newo = NULL;
 ay_pomesh_object *pomesh = NULL;
 ay_bpatch_object *bpatch = NULL;
 dimeVec3f v0, v1, v2, v3;
 dimeVec3f d1, d2, n;
 double d, dp;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  ay_object_defaults(newo);

  tdface->getVertices(v0, v1, v2, v3);

  // is this face a triangle?
  if(AY_V3COMP(v2,v3))
    {
      // yes it is a triangle
      if(!(pomesh = (ay_pomesh_object*)calloc(1, sizeof(ay_pomesh_object))))
	{ return AY_EOMEM; }

      pomesh->npolys = 1;

      if(!(pomesh->nloops = (unsigned int*)calloc(1, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      pomesh->nloops[0] = 1;

      if(!(pomesh->nverts = (unsigned int*)calloc(1, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      pomesh->nverts[0] = 3;

      if(!(pomesh->verts = (unsigned int*)calloc(3, sizeof(unsigned int))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      pomesh->verts[0] = 0;
      pomesh->verts[1] = 1;
      pomesh->verts[2] = 2;

      pomesh->ncontrols = 3;
      if(!(pomesh->controlv = (double*)calloc(pomesh->ncontrols*3,
					      sizeof(double))))
	{ ay_status = AY_EOMEM; goto cleanup; }
      pomesh->controlv[0] = v0[0];
      pomesh->controlv[1] = v0[1];
      pomesh->controlv[2] = v0[2];

      pomesh->controlv[3] = v1[0];
      pomesh->controlv[4] = v1[1];
      pomesh->controlv[5] = v1[2];

      pomesh->controlv[6] = v2[0];
      pomesh->controlv[7] = v2[1];
      pomesh->controlv[8] = v2[2];

      newo->type = AY_IDPOMESH;
      newo->refine = pomesh;
      pomesh = NULL;
    }
  else
    {
      // no it is a quad

      // is this quad planar?

      // get plane equation from first three points
      AY_V3SUB(d1,v1,v0);
      AY_V3SUB(d2,v2,v0);
      AY_V3CROSS(n,d1,d2);
      d = AY_V3DOT(n,v0);
      // insert fourth point into plane equation
      dp = n[0]*v3[0]+n[1]*v3[1]+n[2]*v3[2]-d;

      if(dp < AY_EPSILON)
	{
	  // quad is planar
	  if(!(pomesh = (ay_pomesh_object*)calloc(1, sizeof(ay_pomesh_object))))
	    { return AY_EOMEM; }

	  pomesh->npolys = 1;

	  if(!(pomesh->nloops = (unsigned int*)calloc(1, sizeof(unsigned int))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  pomesh->nloops[0] = 1;

	  if(!(pomesh->nverts = (unsigned int*)calloc(1, sizeof(unsigned int))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  pomesh->nverts[0] = 4;

	  if(!(pomesh->verts = (unsigned int*)calloc(4, sizeof(unsigned int))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  pomesh->verts[0] = 0;
	  pomesh->verts[1] = 1;
	  pomesh->verts[2] = 2;
	  pomesh->verts[2] = 3;

	  pomesh->ncontrols = 4;
	  if(!(pomesh->controlv = (double*)calloc(pomesh->ncontrols*3,
						  sizeof(double))))
	    { ay_status = AY_EOMEM; goto cleanup; }
	  pomesh->controlv[0] = v0[0];
	  pomesh->controlv[1] = v0[1];
	  pomesh->controlv[2] = v0[2];

	  pomesh->controlv[3] = v1[0];
	  pomesh->controlv[4] = v1[1];
	  pomesh->controlv[5] = v1[2];

	  pomesh->controlv[6] = v2[0];
	  pomesh->controlv[7] = v2[1];
	  pomesh->controlv[8] = v2[2];

	  pomesh->controlv[9]  = v3[0];
	  pomesh->controlv[10] = v3[1];
	  pomesh->controlv[11] = v3[2];

	  newo->type = AY_IDPOMESH;
	  newo->refine = pomesh;
	  pomesh = NULL;
	}
      else
	{
	  // quad is not planar
	  if(!(bpatch = (ay_bpatch_object*)calloc(1, sizeof(ay_bpatch_object))))
	    { free(newo); return AY_EOMEM; }

	  bpatch->p1[0] = v0[0];
	  bpatch->p1[1] = v0[1];
	  bpatch->p1[2] = v0[2];

	  bpatch->p2[0] = v1[0];
	  bpatch->p2[1] = v1[1];
	  bpatch->p2[2] = v1[2];

	  bpatch->p3[0] = v2[0];
	  bpatch->p3[1] = v2[1];
	  bpatch->p3[2] = v2[2];

	  bpatch->p4[0] = v3[0];
	  bpatch->p4[1] = v3[1];
	  bpatch->p4[2] = v3[2];

	  newo->type = AY_IDBPATCH;
	  newo->refine = bpatch;
	  bpatch = NULL;
	} // if
    } // if

  // link the new 3dface into the scene hierarchy
  ay_status = dxfio_linkobject(newo);

cleanup:

  if(pomesh)
    {
      if(pomesh->nloops)
	free(pomesh->nloops);
      if(pomesh->nverts)
	free(pomesh->nverts);
      if(pomesh->verts)
	free(pomesh->verts);
      if(pomesh->controlv)
	free(pomesh->controlv);
      free(pomesh);
    }

  if(bpatch)
    free(bpatch);

 return ay_status;
} // dxfio_read3dface


// dxfio_readarc:
//
int
dxfio_readarc(const class dimeState *state,
	      class dimeArc *arc,
	      void *clientdata)
{
 int ay_status = AY_OK;
 ay_object *newo = NULL;
 ay_ncircle_object *newc = NULL;
 dimeVec3f center;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  if(!(newc = (ay_ncircle_object*)calloc(1, sizeof(ay_ncircle_object))))
    { free(newo); return AY_EOMEM; }

  newc->radius = arc->getRadius();
  newc->tmin = arc->getStartAngle();
  newc->tmax = arc->getEndAngle();

  ay_object_defaults(newo);

  newo->type = AY_IDNCIRCLE;
  newo->refine = newc;
  arc->getCenter(center);
  newo->movx = center[0];
  newo->movy = center[1];
  newo->movz = center[2];

  (void)ay_notify_object(newo);

  // link the new arc into the scene hierarchy
  ay_status = dxfio_linkobject(newo);

 return ay_status;
} // dxfio_readarc


// dxfio_readcircle:
//
int
dxfio_readcircle(const class dimeState *state,
		 class dimeCircle *circle,
		 void *clientdata)
{
 int ay_status = AY_OK;
 ay_object *newo = NULL;
 ay_ncircle_object *newc = NULL;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  if(!(newc = (ay_ncircle_object*)calloc(1, sizeof(ay_ncircle_object))))
    { free(newo); return AY_EOMEM; }

  newc->radius = circle->getRadius();
  newc->tmax = 360.0;
  ay_object_defaults(newo);

  newo->type = AY_IDNCIRCLE;
  newo->refine = newc;

  newo->movx = circle->getCenter()[0];
  newo->movy = circle->getCenter()[1];
  newo->movz = circle->getCenter()[2];

  (void)ay_notify_object(newo);

  // link the new circle into the scene hierarchy
  ay_status = dxfio_linkobject(newo);

 return ay_status;
} // dxfio_readcircle


// dxfio_readellipse:
//
int
dxfio_readellipse(const class dimeState *state,
		  class dimeEllipse *ellipse,
		  void *clientdata)
{
 int ay_status = AY_OK;
 double zaxis[3] = {0.0, 0.0, 1.0}, maja[2];
 ay_object *newo = NULL;
 ay_ncircle_object *newc = NULL;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  if(!(newc = (ay_ncircle_object*)calloc(1, sizeof(ay_ncircle_object))))
    { free(newo); return AY_EOMEM; }

  maja[0] = ellipse->getMajorAxisEndpoint()[0];
  maja[1] = ellipse->getMajorAxisEndpoint()[1];

  newc->radius = AY_V2LEN(maja);
  newc->tmin = AY_R2D(ellipse->getStartParam());
  newc->tmax = AY_R2D(ellipse->getEndParam());

  ay_object_defaults(newo);

  newo->type = AY_IDNCIRCLE;
  newo->refine = newc;

  newo->movx = ellipse->getCenter()[0];
  newo->movy = ellipse->getCenter()[1];
  newo->movz = ellipse->getCenter()[2];

  newo->scalx = ellipse->getMinorMajorRatio();

  if(fabs(maja[0]) > AY_EPSILON)
    {
      newo->rotz = AY_R2D(atan(maja[1]/maja[0]));
      ay_quat_axistoquat(zaxis, atan(maja[1]/maja[0]), newo->quat);
    }
  else
    {
      if(maja[1] < 0.0)
	{
	  newo->rotz = 90.0;
	  ay_quat_axistoquat(zaxis, AY_HALFPI, newo->quat);
	}
      else
	{
	  newo->rotz = 270.0;
	  ay_quat_axistoquat(zaxis, AY_PI+AY_HALFPI, newo->quat);
	}
    } // if

  (void)ay_notify_object(newo);

  // link the new circle/ellipse into the scene hierarchy
  ay_status = dxfio_linkobject(newo);

 return ay_status;
} // dxfio_readellipse


// dxfio_readinsert:
//
int
dxfio_readinsert(const class dimeState *state,
		 class dimeInsert *insert,
		 void *clientdata)
{
 int ay_status = AY_OK;
 //char fname[] = "dxfio_readinsert";
 ay_object *newo = NULL, *master = NULL, **old_aynext = NULL;
 ay_level_object *newl = NULL;
 int i;
 dimeVec3f v;
 double angle = 0.0, zaxis[3] = {0.0, 0.0, 1.0};
 dxfio_block *block;
 bool inserted = false;
 dimeBlock *dblock;
 dimeEntity *entity = NULL;
 dxfio_block *newb = NULL;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  ay_object_defaults(newo);

  // move instance
  v = insert->getInsertionPoint();
  newo->movx = v[0];
  newo->movy = v[1];
  newo->movz = v[2];

  // scale instance
  v = insert->getScale();
  newo->scalx = v[0];
  newo->scaly = v[1];
  newo->scalz = v[2];
  // sanity checks...
  if(fabs(newo->scalx) < AY_EPSILON)
    newo->scalx = 1.0;
  if(fabs(newo->scaly) < AY_EPSILON)
    newo->scaly = 1.0;
  if(fabs(newo->scalz) < AY_EPSILON)
    newo->scalz = 1.0;

  // rotate instance
  angle = insert->getRotAngle();
  newo->rotz = angle;
  if(fabs(angle) > AY_EPSILON)
    ay_quat_axistoquat(zaxis, AY_D2R(angle), newo->quat);

  newo->type = AY_IDINSTANCE;

  block = dxfio_blocks;
  while(block)
    {
      if(block->block == insert->getBlock())
	{
	  master = block->object;
	  inserted = true;
	}
      block = block->next;
    } // while

  if(inserted)
    {
      // block has been inserted already, just create an instance
      master->refcount++;
      newo->refine = master;
    }
  else
    {
      // block has not been inserted before, create master
      dblock = insert->getBlock();
      if(dblock && dblock->getNumEntities() > 0)
	{
	  if(!(newb = (dxfio_block*)calloc(1, sizeof(dxfio_block))))
	    { return AY_EOMEM; }
	  newb->next = dxfio_blocks;
	  dxfio_blocks = newb;
	  newb->block = dblock;

	  if(dblock->getNumEntities() > 1)
	    {
	      if(!(newl = (ay_level_object*)calloc(1,
					      sizeof(ay_level_object))))
		{ free(newo); return AY_EOMEM; }

	      //ay_object_defaults(newo);

	      newo->type = AY_IDLEVEL;
	      newo->refine = newl;

	      old_aynext = ay_next;
	      ay_next = &(newo->down);
	      for(i = 0; i < dblock->getNumEntities(); i++)
		{
		  entity = dblock->getEntity(i);
		  ay_status = dxfio_readentitydcb(state, entity, clientdata);
		} // for
	      // terminate level hierarchy
	      *ay_next = ay_endlevel;
	      // reset ay_next
	      ay_next = old_aynext;
	      newb->object = newo;
	    }
	  else
	    {
	      entity = dblock->getEntity(0);
	      dxfio_lrobject = NULL;
	      ay_status = dxfio_readentitydcb(state, entity, clientdata);
	      // did we actually read/convert an entity (layer check)?
	      if(dxfio_lrobject)
		{
		  newb->object = dxfio_lrobject;
		  ay_trafo_copy(newo, dxfio_lrobject);
		}
	      free(newo);
	      newo = NULL;
	    } // if
	} // if
    } // if

  // link the new instance/master into the scene hierarchy
  if(newo)
    ay_status = dxfio_linkobject(newo);

 return ay_status;
} // dxfio_readinsert


// dxfio_readline:
//
int
dxfio_readline(const class dimeState *state,
	       class dimeLine *line,
	       void *clientdata)
{
 int ay_status = AY_OK;
 double *newcv = NULL;
 ay_object *newo = NULL;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  if(!(newcv = (double*)calloc(2*4, sizeof(double))))
    { free(newo); return AY_EOMEM; }

  newcv[0] = (line->getCoords(0))[0];
  newcv[1] = (line->getCoords(0))[1];
  newcv[2] = (line->getCoords(0))[2];
  newcv[3] = 1.0;

  newcv[4] = (line->getCoords(1))[0];
  newcv[5] = (line->getCoords(1))[1];
  newcv[6] = (line->getCoords(1))[2];
  newcv[7] = 1.0;

  ay_nct_create(2, 2, AY_KTNURB, newcv, NULL,
		(ay_nurbcurve_object**)(void*)&(newo->refine));

  ay_object_defaults(newo);

  newo->type = AY_IDNCURVE;

  // link the new curve/line into the scene hierarchy
  ay_status = dxfio_linkobject(newo);

 return ay_status;
} // dxfio_readline


// dxfio_readlwpolyline:
//
int
dxfio_readlwpolyline(const class dimeState *state,
		     class dimeLWPolyline *lwpolyline,
		     void *clientdata)
{
 int ay_status = AY_OK;
 int len = 0, i = 0, a = 0, stride = 4;
 double *newcv = NULL;
 const dxfdouble *xverts, *yverts;
 ay_object *newo = NULL;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  len = lwpolyline->getNumVertices();

  if(!(newcv = (double*)calloc(len*stride, sizeof(double))))
    { free(newo); return AY_EOMEM; }

  xverts = lwpolyline->getXCoords();
  yverts = lwpolyline->getYCoords();

  for(i = 0; i < len; i++)
    {
      newcv[a]   = xverts[i];
      newcv[a+1] = yverts[i];
      newcv[a+2] = lwpolyline->getElevation();
      newcv[a+3] = 1.0;
      a += stride;
    } // for

  ay_nct_create(2, len, AY_KTNURB, newcv, NULL,
		(ay_nurbcurve_object**)(void*)&(newo->refine));

  ay_object_defaults(newo);

  newo->type = AY_IDNCURVE;

  // XXXX process bulges


  // link the new curve/lwpolyline into the scene hierarchy
  ay_status = dxfio_linkobject(newo);

 return ay_status;
} // dxfio_readlwpolyline


// dxfio_getpolyline:
//
int
dxfio_getpolyline(const class dimeState *state,
		  class dimePolyline *polyline,
		  void *clientdata, ay_object *newo)
{
 int ay_status = AY_OK;
 int len = 0, i = 0, a = 0, stride = 4;
 double *newcv = NULL;
 dimeVertex *vert = NULL;
 dimeVec3f cv;

  len = polyline->getNumCoordVertices();

  if(len == 0)
    return AY_ERROR;

  if(polyline->getFlags() & dimePolyline::CLOSED)
    {
      len++;
    }

  if(!(newcv = (double*)calloc(len*stride, sizeof(double))))
    { free(newo); return AY_EOMEM; }

  for(i = 0; i < len; i++)
    {
      vert = NULL;
      vert = polyline->getCoordVertex(i);
      if(vert /* && (getFlags() & dimeVertex::POLYLINE_3D_VERTEX)*/)
	{
	  cv = vert->getCoords();
	  newcv[a]   = cv[0];
	  newcv[a+1] = cv[1];
	  if(polyline->getFlags() & dimePolyline::IS_POLYLINE_3D)
	    {
	      newcv[a+2] = cv[2];
	    }
	  else
	    {
	      cv = polyline->getElevation();
	      newcv[a+2] = cv[2];
	    }
	} // if vert
      newcv[a+3] = 1.0;
      a += stride;
    } // for

  if(polyline->getFlags() & dimePolyline::CLOSED)
    {
      memcpy(&(newcv[(len-2)*stride]), newcv, stride*sizeof(double));
    }

  ay_nct_create(2, len, AY_KTNURB, newcv, NULL,
		(ay_nurbcurve_object**)(void*)&(newo->refine));
  newo->type = AY_IDNCURVE;

  // XXXX process bulges?

 return ay_status;
} // dxfio_getpolyline


// dxfio_getpolymesh:
//
int
dxfio_getpolymesh(const class dimeState *state,
		  class dimePolyline *polyline,
		  void *clientdata, ay_object *newo)
{
 int ay_status = AY_OK;
 unsigned int a, i, numfaces = 0, numverts = 0,m,n;
 ay_pomesh_object *pomesh = NULL;
 dimeVertex *v;
 dimeVec3f cv;

  if(polyline->getSurfaceType() > 0)
    {
      return dxfio_getsmoothsurface(state, polyline, clientdata, newo);
    }

  if(!(pomesh = (ay_pomesh_object*)calloc(1, sizeof(ay_pomesh_object))))
    { return AY_EOMEM; }

  m = polyline->getPolymeshCountM();
  n = polyline->getPolymeshCountN();

  numverts = polyline->getNumCoordVertices();

  // sanity check
  if(m*n != numverts)
    { ay_status = AY_ERROR; goto cleanup; }


  // copy/create face/loop/vertex info
  numfaces = (m-1)*(n-1);

  // XXXX add faces for closed meshes
  /*
  if(polyline->getFlags & dimePolyline::POLYMESH_CLOSED_M)
    numfaces += n;
  if(polyline->getFlags & dimePolyline::POLYMESH_CLOSED_N)
    numfaces += m;
  pomesh->npolys = numfaces;
  */
  if(!(pomesh->nloops = (unsigned int*)calloc(numfaces, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(pomesh->nverts = (unsigned int*)calloc(numfaces, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(pomesh->verts = (unsigned int*)calloc(numfaces*4, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  a = 0;
  for(i = 0; i < numfaces; i++)
    {
      // every polymesh polygon/face has just one loop (no holes)
      pomesh->nloops[i] = 1;
      // all faces are quads
      pomesh->nverts[i] = 4;
      // generate indices
      pomesh->verts[a]   = i;
      pomesh->verts[a+1] = i+1;
      // coordinates are ordered width-first, so we get to the next row by +m?
      pomesh->verts[a+2] = i+m;
      pomesh->verts[a+3] = i+m+1;
      a += 4;
    } // for

  // XXXX add faces for closed meshes


  // copy coordinate values
  pomesh->ncontrols = polyline->getNumCoordVertices();
  // since there are no normals, every control point is 3 doubles
  if(!(pomesh->controlv = (double*)calloc(pomesh->ncontrols*3,
					  sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  a = 0;
  for(i = 0; i < pomesh->ncontrols; i++)
    {
      v = polyline->getCoordVertex((int)i);
      cv = v->getCoords();
      pomesh->controlv[a]   = cv[0];
      pomesh->controlv[a+1] = cv[1];
      if(polyline->getFlags() & dimePolyline::IS_POLYMESH_3D)
	{
	  pomesh->controlv[a+2] = cv[2];
	}
      else
	{
	  cv = polyline->getElevation();
	  pomesh->controlv[a+2] = cv[2];
	}
      a += 3;
    } // for

  // return result
  newo->refine = pomesh;
  newo->type = AY_IDPOMESH;
  pomesh = NULL;

cleanup:

  if(pomesh)
    {
      if(pomesh->nloops)
	free(pomesh->nloops);

      if(pomesh->nverts)
	free(pomesh->nverts);

      if(pomesh->verts)
	free(pomesh->verts);

      if(pomesh->controlv)
	free(pomesh->controlv);

      free(pomesh);
    } // if

 return ay_status;
} // dxfio_getpolymesh


// dxfio_getsmoothsurface:
//
int
dxfio_getsmoothsurface(const class dimeState *state,
		       class dimePolyline *polyline,
		       void *clientdata, ay_object *newo)
{
 int ay_status = AY_OK;
 int w, h, uorder = 0, vorder = 0, uknott = 0, vknott = 0, numcv;
 int i, j, a, b, c;
 double *controlv = NULL;
 // bool closem = false, closen = false;
 dimeVertex *v;
 dimeVec3f cv;

  // get NURBS surface parameters
  w = polyline->getPolymeshCountM();
  h = polyline->getPolymeshCountN();

  switch(polyline->getSurfaceType())
    {
    case dimePolyline::QUADRIC_BSPLINE:
      uorder = 3;
      vorder = 3;
      uknott = AY_KTBSPLINE;
      vknott = AY_KTBSPLINE;
      break;
    case dimePolyline::CUBIC_BSPLINE:
      uorder = 4;
      vorder = 4;
      uknott = AY_KTBSPLINE;
      vknott = AY_KTBSPLINE;
      break;
    case dimePolyline::BEZIER:
      uorder = w;
      vorder = h;
      uknott = AY_KTBEZIER;
      vknott = AY_KTBEZIER;
      break;
    default:
      break;
    } // switch

  // correct parameters for closed cases
  if(polyline->getFlags() & dimePolyline::POLYMESH_CLOSED_M)
    {
      if(polyline->getSurfaceType() != dimePolyline::BEZIER)
	{
	  // B-Spline
	  w += (uorder-1);
	}
      else
	{
	  // Bezier
	  w++;
	  uorder++;
	}
      //closem = true;
    }
  if(polyline->getFlags() & dimePolyline::POLYMESH_CLOSED_N)
    {
      if(polyline->getSurfaceType() != dimePolyline::BEZIER)
	{
	  // B-Spline
	  h += (vorder-1);
	}
      else
	{
	  // Bezier
	  h++;
	  vorder++;
	}
      //closen = true;
    }

  // copy coordinate values
  numcv = w*h;
  if(!(controlv = (double*)calloc(numcv*4, sizeof(double))))
    { return AY_EOMEM; }

  b = 0;
  c = 0;
  for(i = 0; i < w; i++)
    {
      if(i >= polyline->getPolymeshCountM())
	{
	  // if we get here, the surface is closed in M/U direction
	  // and we need to reset <a> to the start of the patch
	  // (wrap around)
	  a = c * polyline->getPolymeshCountN();
	  c++;
	}
      else
	{
	  // note well: we need to _set_ <a> here for every iteration
	  // because the inner loop may reset <a> to the start of the
	  // patch, detecting a closed surface (then a++ is not correct)
	  a = i * polyline->getPolymeshCountN();
	}
      for(j = 0; j < h; j++)
	{
	  if(j == polyline->getPolymeshCountN())
	    {
	      // if we get here, the surface is closed in N/V direction
	      // and we need to reset <a> to the start of the patch
	      // (wrap around)
	      a -= polyline->getPolymeshCountN();
	    }
	  v = polyline->getSplineFrameControlPoint(a);
	  cv = v->getCoords();
	  controlv[b]   = cv[0];
	  controlv[b+1] = cv[1];
	  controlv[b+2] = cv[2];
	  controlv[b+3] = 1.0;
	  b += 4;
	  a++;
	} // for
    } // for

  ay_npt_create(uorder, vorder, w, h, uknott, vknott,
		controlv, NULL, NULL,
		(ay_nurbpatch_object**)(void*)&(newo->refine));

  /*
  if(closem)
    ((ay_nurbpatch_object*)newo->refine)->closedu = AY_TRUE;
  if(closen)
    ((ay_nurbpatch_object*)newo->refine)->closedv = AY_TRUE;
  */

  // return result
  newo->type = AY_IDNPATCH;

 return ay_status;
} // dxfio_getsmoothsurface


// dxfio_getpolyfacemesh:
//
int
dxfio_getpolyfacemesh(const class dimeState *state,
		      class dimePolyline *polyline,
		      void *clientdata, ay_object *newo)
{
 int ay_status = AY_OK;
 unsigned int a, i, numfaces = 0, numverts = 0;
 int j;
 ay_pomesh_object *pomesh = NULL;
 dimeVertex *v;
 dimeVec3f cv;

  if(!(pomesh = (ay_pomesh_object*)calloc(1, sizeof(ay_pomesh_object))))
    { return AY_EOMEM; }

  // copy/create face/loop info
  numfaces = polyline->getNumIndexVertices();
  if(numfaces == 0)
    { ay_status = AY_ERROR; goto cleanup; }

  pomesh->npolys = numfaces;

  if(!(pomesh->nloops = (unsigned int*)calloc(numfaces, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  if(!(pomesh->nverts = (unsigned int*)calloc(numfaces, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }

  for(i = 0; i < numfaces; i++)
    {
      // every polyfacemesh polygon/face has just one loop (no holes)
      pomesh->nloops[i] = 1;
      // but is it a quad or triangle?
      v = polyline->getIndexVertex((int)i);
      pomesh->nverts[i] = v->numIndices();
      numverts += pomesh->nverts[i];
    } // for

  // copy vertice indices
  if(!(pomesh->verts = (unsigned int*)calloc(numverts, sizeof(unsigned int))))
    { ay_status = AY_EOMEM; goto cleanup; }
  a = 0;
  for(i = 0; i < numfaces; i++)
    {
      v = polyline->getIndexVertex((int)i);
      for(j = 0; j < v->numIndices(); j++)
	{
	  if(v->getIndex((int)j) >= 0)
	    {
	      pomesh->verts[a] = v->getIndex((int)j);
	    }
	  else
	    {
	      // negative indizes mean hidden edges, just remove the sign...
	      pomesh->verts[a] = -v->getIndex((int)j);
	    }
	  pomesh->verts[a] -= 1;
	  a++;
	} // for
    } // for

  // copy coordinate values
  pomesh->ncontrols = polyline->getNumCoordVertices();
  // since there are no normals, every control point is 3 doubles
  if(!(pomesh->controlv = (double*)calloc(pomesh->ncontrols*3,
					  sizeof(double))))
    { ay_status = AY_EOMEM; goto cleanup; }
  a = 0;
  for(i = 0; i < pomesh->ncontrols; i++)
    {
      v = polyline->getCoordVertex((int)i);
      cv = v->getCoords();
      pomesh->controlv[a]   = cv[0];
      pomesh->controlv[a+1] = cv[1];
      pomesh->controlv[a+2] = cv[2];
      a += 3;
    } // for

  // return result
  newo->refine = pomesh;
  newo->type = AY_IDPOMESH;
  pomesh = NULL;

cleanup:

  if(pomesh)
    {
      if(pomesh->nloops)
	free(pomesh->nloops);

      if(pomesh->nverts)
	free(pomesh->nverts);

      if(pomesh->verts)
	free(pomesh->verts);

      if(pomesh->controlv)
	free(pomesh->controlv);

      free(pomesh);
    } // if

 return ay_status;
} // dxfio_getpolyfacemesh


// dxfio_readpolyline:
//
int
dxfio_readpolyline(const class dimeState *state,
		   class dimePolyline *polyline,
		   void *clientdata)
{
 int ay_status = AY_OK;
 ay_object *newo = NULL;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  ay_object_defaults(newo);

  switch(polyline->getType())
    {
    case dimePolyline::POLYLINE:
      ay_status = dxfio_getpolyline(state, polyline, clientdata, newo);
      break;
    case dimePolyline::POLYFACE_MESH:
      ay_status = dxfio_getpolyfacemesh(state, polyline, clientdata, newo);
      break;
    case dimePolyline::POLYGON_MESH:
      ay_status = dxfio_getpolymesh(state, polyline, clientdata, newo);
      break;
    default:
      break;
    } // switch

  // link the new curve/polyline into the scene hierarchy
  if(newo && newo->refine)
    {
      ay_status = dxfio_linkobject(newo);
      newo = NULL;
    }

  // cleanup
  if(newo)
    free(newo);

 return ay_status;
} // dxfio_readpolyline


// dxfio_readsolid:
//
int
dxfio_readsolid(const class dimeState *state,
		class dimeSolid *solid,
		void *clientdata)
{
 int ay_status = AY_OK;
 ay_object *newo = NULL;
 ay_bpatch_object *newbp = NULL;
 dimeVec3f v0, v1, v2, v3;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  if(!(newbp = (ay_bpatch_object*)calloc(1, sizeof(ay_bpatch_object))))
    { free(newo); return AY_EOMEM; }

  solid->getVertices(v0, v1, v2, v3);

  newbp->p1[0] = v0[0];
  newbp->p1[1] = v0[1];
  newbp->p1[2] = v0[2];

  newbp->p2[0] = v1[0];
  newbp->p2[1] = v1[1];
  newbp->p2[2] = v1[2];

  newbp->p3[0] = v2[0];
  newbp->p3[1] = v2[1];
  newbp->p3[2] = v2[2];

  newbp->p4[0] = v3[0];
  newbp->p4[1] = v3[1];
  newbp->p4[2] = v3[2];

  ay_object_defaults(newo);

  newo->type = AY_IDBPATCH;
  newo->refine = newbp;

  // link the new bpatch/solid into the scene hierarchy
  ay_status = dxfio_linkobject(newo);

  // XXXX extrude solid

  //dxfio_extrudebpatch(solid, newo);

 return ay_status;
} // dxfio_readsolid


// dxfio_readspline:
//
int
dxfio_readspline(const class dimeState *state,
		 class dimeSpline *spline,
		 void *clientdata)
{
 int ay_status = AY_OK;
 int len = 0, order = 0, i = 0, a = 0, stride = 4;
 double *newcv = NULL, *newkv = NULL;
 dimeVec3f cv;
 ay_object *newo = NULL;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  len = spline->getNumControlPoints();
  order = spline->getDegree() + 1;

  if(!(newcv = (double*)calloc(len*stride, sizeof(double))))
    { free(newo); return AY_EOMEM; }

  if(!(newkv = (double*)calloc(len+order, sizeof(double))))
    { free(newcv); free(newo); return AY_EOMEM; }

  // copy control points
  for(i = 0; i < len; i++)
    {
      cv = spline->getControlPoint(i);
      newcv[a]   = cv[0];
      newcv[a+1] = cv[1];
      newcv[a+2] = cv[2];
      a += stride;
    } // for

  // copy/create weights
  a = 3;
  if(spline->hasWeights() && (spline->getNumWeights() > 0))
    {
      for(i = 0; i < len; i++)
	{
	  newcv[a] = spline->getWeight(i);
	  if(fabs(newcv[a]) < AY_EPSILON)
	    newcv[a] = 1.0;
	  a += stride;
	}
    }
  else
    {
      for(i = 0; i < len; i++)
	{
	  newcv[a] = 1.0;
	  a += stride;
	}
    } // if

  // copy knots
  for(i = 0; i < len+order; i++)
    {
      newkv[i] = spline->getKnotValue(i);
    }

  // rescale knots to safe distance?
  if(dxfio_rescaleknots != 0.0)
    {
      ay_knots_rescaletomindist(len+order, newkv, dxfio_rescaleknots);
    }

  // now create a NURBCurve object
  ay_nct_create(order, len, AY_KTCUSTOM, newcv, newkv,
		(ay_nurbcurve_object**)(void*)&(newo->refine));

  ay_object_defaults(newo);

  newo->type = AY_IDNCURVE;

  // link the new curve/spline into the scene hierarchy
  ay_status = dxfio_linkobject(newo);

 return ay_status;
} // dxfio_readspline


// dxfio_readtrace:
//
int
dxfio_readtrace(const class dimeState *state,
		class dimeTrace *trace,
		void *clientdata)
{
 int ay_status = AY_OK;
 ay_object *newo = NULL;
 ay_bpatch_object *newbp = NULL;
 dimeVec3f v0, v1, v2, v3;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  if(!(newbp = (ay_bpatch_object*)calloc(1, sizeof(ay_bpatch_object))))
    { free(newo); return AY_EOMEM; }

  trace->getVertices(v0, v1, v2, v3);

  newbp->p1[0] = v0[0];
  newbp->p1[1] = v0[1];
  newbp->p1[2] = v0[2];

  newbp->p2[0] = v1[0];
  newbp->p2[1] = v1[1];
  newbp->p2[2] = v1[2];

  newbp->p3[0] = v2[0];
  newbp->p3[1] = v2[1];
  newbp->p3[2] = v2[2];

  newbp->p4[0] = v3[0];
  newbp->p4[1] = v3[1];
  newbp->p4[2] = v3[2];

  ay_object_defaults(newo);

  newo->type = AY_IDBPATCH;
  newo->refine = newbp;

  // link the new bpatch/trace into the scene hierarchy
  ay_status = dxfio_linkobject(newo);

  // XXXX extrude trace

 return ay_status;
} // dxfio_readtrace


// dxfio_linkobject
//  apply global scale factor, then link object <o> to the scene
int
dxfio_linkobject(ay_object *o)
{
  if(!o)
    return AY_ENULL;

  // apply global scale factor
  if(dxfio_scalefactor != 1.0)
    {
      o->scalx *= dxfio_scalefactor;
      o->scaly *= dxfio_scalefactor;
      o->scalz *= dxfio_scalefactor;
    }

  if(dxfio_importcurves == AY_FALSE)
    {
      if(o->type == AY_IDNCURVE || o->type == AY_IDNCIRCLE)
	{
	  ay_object_delete(o);
	  return AY_OK;
	}
    }

  dxfio_lrobject = o;

  ay_object_link(o);

  return AY_OK;
} // dxfio_linkobject


// dxfio_readentitydcb:
//  Dime entity traversal callback
bool
dxfio_readentitydcb(const class dimeState *state,
		    class dimeEntity *entity,
		    void *clientdata)
{
 int ay_status = AY_OK;
 char fname[] = "dxfio_readentity";
 static float oldprogress = 0.0f;
 float progress;
 static int entitynum = 0;
 char progressstr[32];
 char arrname[] = "dxfio_options", varname1[] = "Progress";
 char varname2[] = "Cancel";
 const char *val = NULL;

  if(!state || !entity)
    {
      oldprogress = 0.0f;
      entitynum = 0;
      return true;
    }

  // check layer
  if(dxfio_slayer != -1)
    {
      const dimeLayer *layer = entity->getLayer();
      if(layer && (layer->getLayerNum() < dxfio_slayer ||
		   layer->getLayerNum() > dxfio_elayer))
	return true;
    }

  // use matching callback to convert the entity
  switch(entity->typeId())
    {
    case dimeBase::dime3DFaceType:
      ay_status = dxfio_read3dface(state, (dime3DFace*)entity, clientdata);
      break;
    case dimeBase::dimeArcType:
      ay_status = dxfio_readarc(state, (dimeArc*)entity, clientdata);
      break;
    case dimeBase::dimeCircleType:
      ay_status = dxfio_readcircle(state, (dimeCircle*)entity, clientdata);
      break;
    case dimeBase::dimeEllipseType:
      ay_status = dxfio_readellipse(state, (dimeEllipse*)entity, clientdata);
      break;
    case dimeBase::dimeInsertType:
      ay_status = dxfio_readinsert(state, (dimeInsert*)entity, clientdata);
      break;
    case dimeBase::dimeLineType:
      ay_status = dxfio_readline(state, (dimeLine*)entity, clientdata);
      break;
    case dimeBase::dimeLWPolylineType:
      ay_status = dxfio_readlwpolyline(state, (dimeLWPolyline*)entity,
				       clientdata);
      break;
    case dimeBase::dimePolylineType:
      ay_status = dxfio_readpolyline(state, (dimePolyline*)entity, clientdata);
      break;
    case dimeBase::dimeSolidType:
      ay_status = dxfio_readsolid(state, (dimeSolid*)entity, clientdata);
      break;
    case dimeBase::dimeSplineType:
      ay_status = dxfio_readspline(state, (dimeSpline*)entity, clientdata);
      break;
    case dimeBase::dimeTraceType:
      ay_status = dxfio_readtrace(state, (dimeTrace*)entity, clientdata);
      break;
    case dimeBase::dimeUnknownEntityType:
      ay_error(AY_EWARN, fname, "Skipping entity of unknown type.");
      break;
    default:
      break;
    } // switch

  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Error converting entity.");
    }

  // calculate/set progress
  if(entity->typeId() != dimeBase::dimeVertexType)
    {
      entitynum++;
      progress = (float)entitynum/(float)dxfio_totalents;

      if(progress-oldprogress > 0.05)
	{
	  sprintf(progressstr, "%d", (int)(50.0+progress*50.0f));
	  Tcl_SetVar2(ay_interp, arrname, varname1, progressstr,
		      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

	  oldprogress = progress;
	} // if

      // also, check for cancel button
      val = Tcl_GetVar2(ay_interp, arrname, varname2,
			TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      if(val && val[0] == '1')
	{
	  ay_error(AY_EOUTPUT, fname,
		   "Import cancelled! Not all objects may have been read!");
	  return false;
	} // if
    } // if

 return true;
} // dxfio_readentitydcb


// dxfio_readprogressdcb:
//
int
dxfio_readprogressdcb(float progress, void *clientdata)
{
 static float oldprogress = 0.0f;
 char progressstr[32];
 char fname[] = "dxfio_readfile";
 char arrname[] = "dxfio_options", varname1[] = "Progress";
 char varname2[] = "Cancel";
 const char *val = NULL;

  if(clientdata != NULL)
    {
      oldprogress = 0.0f;
      return 1;
    }

  // set progress counter in the GUI
  if(progress-oldprogress > 0.05)
    {
      sprintf(progressstr, "%d", (int)(progress*50.0f));

      Tcl_SetVar2(ay_interp, arrname, varname1, progressstr,
		  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

      oldprogress = progress;
    } // if

  // also, check for cancel button (at a somewhat higher frequency)
  if(progress-oldprogress > 0.01)
    {
      val = Tcl_GetVar2(ay_interp, arrname, varname2,
			TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      if(val && val[0] == '1')
	{
	  ay_error(AY_EOUTPUT, fname,
		   "Import cancelled! Not all objects may have been read!");
	  return 0;
	}
    } // if

  return 1;
} // dxfio_readprogressdcb


// dxfio_countsubentities:
//
int
dxfio_countsubentities(dimeInsert *insert)
{
 dimeBlock *block = insert->getBlock();

  if(block)
    {
      for(int i = 0; i < block->getNumEntities(); i++)
	{
	  dimeEntity *entity = block->getEntity(i);
	  if(entity)
	    {
	      dxfio_totalents++;
	      if(entity->typeId() == dimeBase::dimeInsertType)
		{
		  dxfio_countsubentities((dimeInsert*)entity);
		}
	    } // if
	} // for
    } // if

 return AY_OK;
} // dxfio_countsubentities


// dxfio_countentities:
//
int
dxfio_countentities(dimeModel *model)
{
 int i;
 const dimeSection *section;
 dimeEntitiesSection *esection = NULL;
 dimeEntity *entity = NULL;

  if(!model)
    return AY_ENULL;

  section = model->findSection("ENTITIES");

  if(section)
    {
      esection = (dimeEntitiesSection*)section;
      for(i = 0; i < esection->getNumEntities(); i++)
	{
	  entity = esection->getEntity(i);
	  if(!(entity->typeId() == dimeBase::dimeVertexType))
	    {
	      dxfio_totalents++;
	      if(entity->typeId() == dimeBase::dimeInsertType)
		{
		  dxfio_countsubentities((dimeInsert*)entity);
		} // if
	    } // if
	} // for
    } // if

  char fname[] = "dxfio_countentities";
  char message[64];
  sprintf(message, "%d entities to convert.\n", dxfio_totalents);
  ay_error(AY_EOUTPUT, fname, message);

 return AY_OK;
} // dxfio_countentities


// dxfio_readtcmd:
//  Tcl command to read DXF files using Dime
int
dxfio_readtcmd(ClientData clientData, Tcl_Interp *interp,
	       int argc, char *argv[])
{
  //int ay_status = AY_OK;
 char *minus, lineerrstr[64];
 int i = 2;
 double accuracy = 0.1;
 char arrname[] = "dxfio_options", varname[] = "Progress";

  dxfio_importcurves = AY_TRUE;
  dxfio_rescaleknots = 0.0;
  dxfio_scalefactor = 1.0;
  dxfio_slayer = -1;
  dxfio_elayer = -1;

  // reset internal progress counter
  dxfio_readprogressdcb(0.0f, (void*)1);

  // check args
  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "filename");
      return TCL_OK;
    }

  // parse args
  const char *filename = argv[1];

  while(i+1 < argc)
    {
      if(!strcmp(argv[i], "-a"))
	{
	  sscanf(argv[i+1], "%lg", &accuracy);
	}
      else
      if(!strcmp(argv[i], "-c"))
	{
	  sscanf(argv[i+1], "%d", &dxfio_importcurves);
	}
      else
      if(!strcmp(argv[i], "-r"))
	{
	  sscanf(argv[i+1], "%lg", &dxfio_rescaleknots);
	}
      else
      if(!strcmp(argv[i], "-f"))
	{
	  sscanf(argv[i+1], "%lg", &dxfio_scalefactor);
	}
      else
      if(!strcmp(argv[i], "-l"))
	{
	  if(argv[i+1])
	    {
	      if(*argv[i+1] != '-')
		{
		  sscanf(argv[i+1], "%d", &dxfio_slayer);
		  dxfio_elayer = dxfio_slayer;
		  if((strlen(argv[i+1]) > 3) &&
		     (minus = strchr(/*(const char*)*/(&(argv[i+1][1])), '-')))
		    {
		      minus++;
		      if(*minus != '\0')
			{
			  sscanf(minus, "%d", &dxfio_elayer);
			}
		      else
			{
			  ay_error(AY_ERROR, argv[0],
	    "could not parse layer range, specify it as: startindex-endindex");
			  return TCL_OK;
			} // if
		    } // if
		} // if
	    } // if
	} // if
      i += 2;
    } // while


  // open file for reading
  dimeInput in;

  if(!in.setFile(filename))
    {
      ay_error(AY_EOPENFILE, argv[0], argv[1]);
      return TCL_OK;
    }

  in.setCallback(&dxfio_readprogressdcb, NULL);

  // try reading the file
  dimeModel model;

  if(!model.read(&in))
    {
      int line = in.getFilePosition();
      sprintf(lineerrstr, "DXF read error in line: %d", line);
      ay_error(AY_ERROR, argv[0], lineerrstr);
      return TCL_OK;
    }

  // set progress
  Tcl_SetVar2(ay_interp, arrname, varname, "50",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  // count convertable entities
  dxfio_totalents = 0;
  dxfio_countentities(&model);

  // convert all entities to Ayam objects
  dxfio_lrobject = NULL;
  model.traverseEntities(&dxfio_readentitydcb, NULL,
			 false, false, false);

  // set progress
  Tcl_SetVar2(ay_interp, arrname, varname, "100",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  // clean up
  dxfio_readentitydcb(NULL, NULL, NULL);

  while(dxfio_blocks)
    {
      dxfio_block *t = dxfio_blocks->next;
      free(dxfio_blocks);
      dxfio_blocks = t;
    }

 return TCL_OK;
} // dxfio_readtcmd

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

typedef int (dxfio_writecb) (ay_object *o, dimeModel *dm, double *m);

int dxfio_writepomesh(ay_object *o, dimeModel *dm, double *m);

int dxfio_writenpatch(ay_object *o, dimeModel *dm, double *m);

int dxfio_writencurve(ay_object *o, dimeModel *dm, double *m);

int dxfio_writelevel(ay_object *o, dimeModel *dm, double *m);

int dxfio_writeclone(ay_object *o, dimeModel *dm, double *m);

int dxfio_writeinstance(ay_object *o, dimeModel *dm, double *m);

int dxfio_writescript(ay_object *o, dimeModel *dm, double *m);

int dxfio_writeobject(ay_object *o, dimeModel *dm);

int dxfio_writetcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[]);

int dxfio_registerwritecb(char *name, dxfio_writecb *cb);

int dxfio_writeprogressdcb(float progress, void *clientdata);


// dxfio_writepomesh:
//
int
dxfio_writepomesh(ay_object *o, dimeModel *dm, double *m)
{
 int ay_status = AY_OK;
 ay_pomesh_object *pm, *trpm = NULL;
 double m1[16] = {0}, m2[16] = {0};
 int stride = 3, iverts = 0;
 bool needtess = false;
 unsigned int a, f = 0, i, j, p = 0, q = 0, r = 0;
 dimePolyline *pl = NULL;
 dimeVertex **cverticesarr = NULL, **iverticesarr = NULL;

  if(!o || !dm || !m)
    return AY_ENULL;

  pm = (ay_pomesh_object *)(o->refine);

  memcpy(m2, m, 16*sizeof(double));
  ay_trafo_creatematrix(o, m1);
  ay_trafo_multmatrix(m2, m1);

  for(i = 0; i < pm->npolys; i++)
    {
      if(pm->nloops[i] == 1)
	{
	  for(j = 0; j < pm->nloops[p]; j++)
	    {
	      if(pm->nverts[q] > 4)
		{
		  needtess = true;
		  break;
		}
	    } // for
	}
      else
	{
	  needtess = true;
	  break;
	} // if
      if(needtess == true)
	break;
    } // for

  if(needtess)
    {
      ay_status = ay_tess_pomesh(pm, AY_TRUE, NULL, &trpm);
      if(trpm)
	{
	  pm = trpm;
	}
      else
	{
	  return AY_ERROR;
	}
    } // if

  pl = new dimePolyline;

  if(!pl)
    { ay_status = AY_EOMEM; goto cleanup; }

  pl->setFlags(dimePolyline::IS_POLYFACE_MESH);

  // set vertex coordinates and normals
  cverticesarr = (dimeVertex**)calloc((int)pm->ncontrols, sizeof(dimeVertex*));

  if(!cverticesarr)
    { ay_status = AY_EOMEM; goto cleanup; }

  a = 0;
  if(pm->has_normals)
    stride += 3;

  for(i = 0; i < pm->ncontrols; i++)
    {
      double v3[3];
      dimeVec3f v;
      dimeVertex *cvert = new dimeVertex;
      if(!cvert)
	{ ay_status = AY_EOMEM; goto cleanup; }

      memcpy(v3, &(pm->controlv[a]), 3*sizeof(double));
      ay_trafo_apply3(v3, m2);
      v.setValue(v3[0], v3[1], v3[2]);
      cvert->setCoords(v);
      cvert->setFlags(dimeVertex::POLYFACE_MESH_VERTEX);
      cverticesarr[i] = cvert;
      a += stride;
    } // for

  pl->setCoordVertices(cverticesarr, (int)pm->ncontrols, NULL);

  for(i = 0; i < pm->npolys; i++)
    {
      for(j = 0; j < pm->nloops[p]; j++)
	{
	  if((pm->nverts[q] == 3) || (pm->nverts[q] == 4))
	     iverts++;
	  q++;
	}
      p++;
    }

  iverticesarr = (dimeVertex**)calloc(iverts, sizeof(dimeVertex*));

  if(!iverticesarr)
    { ay_status = AY_EOMEM; goto cleanup; }

  // set faces
  p = 0;
  q = 0;
  for(i = 0; i < pm->npolys; i++)
    {
      for(j = 0; j < pm->nloops[p]; j++)
	{

	  if(pm->nverts[q] == 3)
	    {
	      // this is a triangle
	      dimeVertex *ivert = new dimeVertex;
	      if(!ivert)
		{ ay_status = AY_EOMEM; goto cleanup; }

	      ivert->setIndex(0, pm->verts[r]+1);
	      ivert->setIndex(1, pm->verts[r+1]+1);
	      ivert->setIndex(2, pm->verts[r+2]+1);
	      // XXXX unneeded?
	      ivert->setFlags(dimeVertex::POLYFACE_MESH_VERTEX);
	      iverticesarr[f] = ivert;

	      f++;
	      r += 3;
	    } // if
	  if(pm->nverts[q] == 4)
	    {
	      // this is a quad
	      dimeVertex *ivert = new dimeVertex;
	      if(!ivert)
		{ ay_status = AY_EOMEM; goto cleanup; }

	      ivert->setIndex(0, pm->verts[r]+1);
	      ivert->setIndex(1, pm->verts[r+1]+1);
	      ivert->setIndex(2, pm->verts[r+2]+1);
	      ivert->setIndex(3, pm->verts[r+3]+1);
	      // XXXX unneeded?
	      ivert->setFlags(dimeVertex::POLYFACE_MESH_VERTEX);
	      iverticesarr[f] = ivert;

	      f++;
	      r += 4;
	    } // if
	  q++;
	} // for
      p++;
    } // for

  pl->setIndexVertices(iverticesarr, iverts, NULL);

  // set to current layer
  pl->setLayer(dm->getLayer(dxfio_currentlayer));

  // append to entities table
  dm->addEntity(pl);

  // prevent cleanup code to remove the verts
  free(cverticesarr);
  cverticesarr = NULL;

  free(iverticesarr);
  iverticesarr = NULL;

cleanup:

  if(trpm)
    {
      if(trpm->nloops)
	free(trpm->nloops);
      if(trpm->nverts)
	free(trpm->nverts);
      if(trpm->verts)
	free(trpm->verts);

      if(trpm->controlv)
	free(trpm->controlv);

      free(trpm);
    }

  if(cverticesarr)
    {
      i = 0;
      while(cverticesarr[i])
	{
	  delete cverticesarr[i];
	  i++;
	}
      free(cverticesarr);
    }

  if(iverticesarr)
    {
      i = 0;
      while(iverticesarr[i])
	{
	  delete iverticesarr[i];
	  i++;
	}
      free(iverticesarr);
    }

 return ay_status;
} // dxfio_writepomesh


// dxfio_writenpatch:
//
int
dxfio_writenpatch(ay_object *o, dimeModel *dm, double *m)
{
 int ay_status = AY_OK;
 ay_object *p = NULL, *t = NULL;

  ay_status = ay_provide_object(o, AY_IDPOMESH, &p);
  if(p)
    {
      t = p;
      while(t)
	{
	  if(t->type == AY_IDPOMESH)
	    {

	      ay_status = dxfio_writeobject(t, dm);

	    } // if
	  t = t->next;
	} // while

      ay_status = ay_object_deletemulti(p, AY_FALSE);

      return AY_OK;
    } // if

 return ay_status;
} // dxfio_writenpatch


// dxfio_writencurve:
//
int
dxfio_writencurve(ay_object *o, dimeModel *dm, double *m)
{
 int ay_status = AY_OK;
 ay_nurbcurve_object *nc = NULL;
 double m1[16] = {0}, m2[16] = {0};
 int stride = 4, a = 0;
 bool has_weights = false;
 dimeSpline *sp = NULL;
 dimeVec3f *dv = NULL;
 dxfdouble *dk = NULL;

  if(!o || !dm || !m)
    return AY_ENULL;

  if(!dxfio_exportcurves)
    return AY_OK;

  nc = (ay_nurbcurve_object *)(o->refine);

  memcpy(m2, m, 16*sizeof(double));
  ay_trafo_creatematrix(o, m1);
  ay_trafo_multmatrix(m2, m1);

  if(nc->is_rat)
    {
      has_weights = true;
    }

  sp = new dimeSpline;
  if(!sp)
    { ay_status = AY_EOMEM; goto cleanup; }

  sp->setFlags(0);

  if(has_weights)
    {
      sp->setFlags(dimeSpline::RATIONAL);
    }

  sp->setDegree(nc->order-1);

  dk = (dxfdouble*)calloc(nc->length+nc->order, sizeof(dxfdouble));
  if(!dk)
    { ay_status = AY_EOMEM; goto cleanup; }

  for(int i = 0; i < nc->length+nc->order; i++)
    {
      dk[i] = (dxfdouble)nc->knotv[i];
    }
  sp->setKnotValues(dk, nc->length+nc->order, NULL);
  dk = NULL;

  dv = (dimeVec3f*)calloc(nc->length, sizeof(dimeVec3f));
  if(!dv)
    { free(dk); ay_status = AY_EOMEM; goto cleanup; }

  for(int i = 0; i < nc->length; i++)
    {
      double v[3];
      memcpy(v, &(nc->controlv[a]), 3*sizeof(double));
      ay_trafo_apply3(v, m2);
      dv[i].setValue((dxfdouble)v[0], (dxfdouble)v[1], (dxfdouble)v[2]);
      a += stride;
    }

  sp->setControlPoints(dv, nc->length, NULL);

  if(has_weights)
    {
      a = 3;
      for(int i = 0; i < nc->length; i++)
	{
	  sp->setWeight(i, (dxfdouble)nc->controlv[a], NULL);
	  a += stride;
	}
    }

  // set to current layer
  sp->setLayer(dm->getLayer(dxfio_currentlayer));

  // append to entities table
  dm->addEntity(sp);

  // prevent cleanup code from doing anything harmful
  sp = NULL;

cleanup:

  if(sp)
    {
      delete sp;
    }

 return ay_status;
} // dxfio_writencurve


// dxfio_writelevel:
//
int
dxfio_writelevel(ay_object *o, dimeModel *dm, double *m)
{
 int ay_status = AY_OK;
 ay_object *down = NULL;
 ay_level_object *l = NULL;
 double m1[16] = {0};

  if(!o || !dm || !m)
    return AY_ENULL;

  l = (ay_level_object *)o->refine;

  if(l->type == AY_LTEND)
    return AY_OK;

  if(o->down && o->down->next)
    {
      memcpy(m1, tm, 16*sizeof(double));
      memcpy(tm, m, 16*sizeof(double));

      down = o->down;
      while(down->next)
	{
	  ay_status = dxfio_writeobject(down, dm);
	  down = down->next;
	}

      memcpy(tm, m1, 16*sizeof(double));
    } // if

 return ay_status;
} // dxfio_writelevel


// dxfio_writeclone:
//
int
dxfio_writeclone(ay_object *o, dimeModel *dm, double *m)
{
 int ay_status = AY_OK;
 ay_clone_object *cl = NULL;
 ay_object *clone;

  if(!o || !dm || !m)
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
	  ay_status = dxfio_writeobject(clone, dm);
	  clone = clone->next;
	}
      clone = cl->clones;
    }

  while(clone)
    {
      ay_status = dxfio_writeobject(clone, dm);
      clone = clone->next;
    } // while

 return ay_status;
} // dxfio_writeclone


// dxfio_writeinstance:
//
int
dxfio_writeinstance(ay_object *o, dimeModel *dm, double *m)
{
 int ay_status = AY_OK;
 ay_object *orig, tmp = {0};

  if(!o || !dm || !m)
    return AY_ENULL;

  orig = (ay_object *)o->refine;

  ay_trafo_copy(orig, &tmp);
  ay_trafo_copy(o, orig);
  ay_status = dxfio_writeobject(orig, dm);
  ay_trafo_copy(&tmp, orig);

 return ay_status;
} // dxfio_writeinstance


// dxfio_writescript:
//
int
dxfio_writescript(ay_object *o, dimeModel *dm, double *m)
{
 int ay_status = AY_OK;
 ay_script_object *sc = NULL;
 ay_object *cm;

  if(!o || !dm || !m)
    return AY_ENULL;

  sc = (ay_script_object *)o->refine;

  if(((sc->type == 1) || (sc->type == 2)) && (sc->cm_objects))
    {
      cm = sc->cm_objects;
      while(cm)
	{
	  ay_status = dxfio_writeobject(cm, dm);

	  cm = cm->next;
	} // while
    } // if

 return ay_status;
} // dxfio_writescript


// dxfio_writeobject:
//
int
dxfio_writeobject(ay_object *o, dimeModel *dm)
{
 int ay_status = AY_OK;
 char fname[] = "dxfio_writeobject";
 Tcl_HashTable *ht = &dxfio_write_ht;
 Tcl_HashEntry *entry = NULL;
 char err[255];
 dxfio_writecb *cb = NULL;
 double m1[16] = {0}, m2[16];
 ay_tag *t = NULL;
 ay_object *c = NULL, *to = NULL;
 int i, numconvs = 3, conversions[3] = {AY_IDNPATCH, AY_IDNCURVE, AY_IDPOMESH};
 char arrname[] = "dxfio_options";
 char varname2[] = "Cancel";
 const char *val = NULL;

  if(!o || !dm)
    return AY_ENULL;

  if(dxfio_expselected && !o->selected)
    return AY_OK;

  if(dxfio_expobeynoexport && o->tags)
    {
      t = o->tags;
      while(t)
	{
	  if(t->type == ay_noexport_tagtype)
	    {
	      return AY_OK;
	    } // if
	  t = t->next;
	} // while
    } // if

  if(dxfio_expignorehidden && o->hide)
    return AY_OK;

  if((entry = Tcl_FindHashEntry(ht, (char *)(o->type))))
    {
      cb = (dxfio_writecb*)Tcl_GetHashValue(entry);
      if(cb)
	{
	  if((o->movx != 0.0) || (o->movy != 0.0) || (o->movz != 0.0) ||
	     (o->rotx != 0.0) || (o->roty != 0.0) || (o->rotz != 0.0) ||
	     (o->scalx != 1.0) || (o->scaly != 1.0) || (o->scalz != 1.0) ||
	     (o->quat[0] != 0.0) || (o->quat[1] != 0.0) ||
	     (o->quat[2] != 0.0) || (o->quat[3] != 1.0))
	    {
	      ay_trafo_creatematrix(o, m1);
	      memcpy(m2, tm, 16*sizeof(double));
	      ay_trafo_multmatrix(m2, m1);
	      ay_status = cb(o, dm, m2);
	    }
	  else
	    {
	      ay_status = cb(o, dm, tm);
	    } // if

	  if(ay_status)
	    {
	      ay_error(AY_ERROR, fname, "Error exporting object.");
	      ay_status = AY_OK;
	    } // if
	} // if
    }
  else
    {
      // can not export directly => try to convert object
      for(i = 0; i < numconvs; i++)
	{
	  c = NULL;
	  ay_status = ay_provide_object(o, conversions[i], &c);
	  to = c;
	  while(to)
	    {
	      ay_status = dxfio_writeobject(to, dm);
	      to = to->next;
	    }

	  if(c)
	    {
	      ay_object_deletemulti(c, AY_FALSE);
	      i = -1;
	      break;
	    }
	} // for
      if(i != -1)
	{
	  sprintf(err, "Cannot export objects of type: %s.",
		  ay_object_gettypename(o->type));
	  ay_error(AY_EWARN, fname, err);
	}
    } // if

  // also, check for cancel button
  val = Tcl_GetVar2(ay_interp, arrname, varname2,
		    TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  if(val && val[0] == '1')
    {
      ay_error(AY_EOUTPUT, fname,
	       "Export cancelled! Not all objects may have been written!");
      dxfio_cancelled = AY_TRUE;
      return AY_ERROR;
    } // if

 return ay_status;
} // dxfio_writeobject


// dxfio_fixlayer:
//
int
dxfio_fixlayer(dimeLayerTable *layer)
{

  if(!layer)
    return AY_ENULL;

  // need to set some extra records so that AutoCAD will stop
  // complaining
  dimeParam param;
  param.string_data = "CONTINUOUS";
  layer->setRecord(6, param);
  param.int16_data = 64;
  layer->setRecord(70, param);

 return AY_OK;
} // dxfio_fixlayer


// dxfio_writetcmd:
//
int
dxfio_writetcmd(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[])
{
 int ay_status = AY_OK;
 int i = 2, li;
 ay_object *o;
 char aname[] = "dxfio_options", vname1[] = "Progress";

  // set default parameters
  dxfio_scalefactor = 1.0;
  ay_trafo_identitymatrix(tm);

  dxfio_exportcurves = AY_TRUE;
  dxfio_expselected = AY_FALSE;
  dxfio_expobeynoexport = AY_TRUE;
  dxfio_expignorehidden = AY_TRUE;
  dxfio_exptoplevellayers = AY_FALSE;

  // reset internal progress counter
  dxfio_writeprogressdcb(0.0f, (void*)1);

  dxfio_cancelled = AY_FALSE;

  // check args
  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "filename");
      return TCL_OK;
    }

  // parse args
  while(i+1 < argc)
    {
      if(!strcmp(argv[i], "-c"))
	{
	  sscanf(argv[i+1], "%d", &dxfio_exportcurves);
	}
      else
      if(!strcmp(argv[i], "-s"))
	{
	  sscanf(argv[i+1], "%d", &dxfio_expselected);
	}
      else
      if(!strcmp(argv[i], "-o"))
	{
	  sscanf(argv[i+1], "%d", &dxfio_expobeynoexport);
	}
      else
      if(!strcmp(argv[i], "-i"))
	{
	  sscanf(argv[i+1], "%d", &dxfio_expignorehidden);
	}
      else
      if(!strcmp(argv[i], "-l"))
	{
	  sscanf(argv[i+1], "%d", &dxfio_exptoplevellayers);
	}
      else
      if(!strcmp(argv[i], "-f"))
	{
	  sscanf(argv[i+1], "%lg", &dxfio_scalefactor);
	}

      i += 2;
    } // while

  // initialize and open output file
  dimeOutput out;
  if(!out.setFilename(argv[1]))
    {
      ay_error(AY_EOPENFILE, argv[0], argv[1]);

      return TCL_OK;
    }

  dimeModel dm;

  // add tables section (needed for layers).
  dimeTablesSection *tables = new dimeTablesSection;
  dm.insertSection(tables);

  // set up a layer table to store our layers
  dimeTable *layers = new dimeTable(NULL);

  dimeLayerTable *layer = new dimeLayerTable;
  layer->setLayerName("Default", NULL);

  dxfio_fixlayer(layer);

  // important, register layer in model
  layer->registerLayer(&dm);
  layers->insertTableEntry(layer);

  // create layers from top level objects?
  if(dxfio_exptoplevellayers)
    {
      li = 0;
      o = ay_root->next;
      while(o)
	{
	  if((o->type == AY_IDLEVEL) && o->next)
	    {
	      if(li >= 1)
		{
		  dimeLayerTable *layer = new dimeLayerTable;
		  if(o->name && strlen(o->name))
		    layer->setLayerName(o->name, NULL);
		  else
		    layer->setLayerName("Unnamed", NULL);

		  // the color numbers are defined in dime/Layer.cpp.
		  //layer->setColorNumber(colnum);

		  dxfio_fixlayer(layer);

		  // important, register layer in model
		  layer->registerLayer(&dm);
		  layers->insertTableEntry(layer);
		}
	      li++;
	    } // if
	  o = o->next;
	} // while
    }

  // insert the layer in the table
  tables->insertTable(layers);

  // add the entities section.
  dimeEntitiesSection *entities = new dimeEntitiesSection;
  dm.insertSection(entities);

  // fill entities section
  li = 0;
  o = ay_root->next;
  while(o)
    {
      ay_trafo_identitymatrix(tm);

      if(dxfio_scalefactor != 1.0)
	{
	  ay_trafo_scalematrix(dxfio_scalefactor, dxfio_scalefactor,
			       dxfio_scalefactor, tm);
	}

      dxfio_currentlayer = 0;

      if(dxfio_exptoplevellayers && (o->type == AY_IDLEVEL) && o->next)
	{
	  dxfio_currentlayer = li;
	  li++;
	}

      ay_status = dxfio_writeobject(o, &dm);

      if(dxfio_cancelled)
	break;

      o = o->next;
    } // while

  if(dxfio_cancelled)
    goto cleanup;

  // set progress
  Tcl_SetVar2(ay_interp, aname, vname1, "50",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  // arrange to be called back when Dime writes the DXF file out
  out.setCallback(dm.countRecords(), &dxfio_writeprogressdcb, NULL);

  // write the model to the output file
  if(!dm.write(&out))
    {
      if(!dxfio_cancelled)
	{
	  ay_error(AY_ERROR, argv[0], "Error writing file!");
	}
    }

  // set progress
  Tcl_SetVar2(ay_interp, aname, vname1, "100",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

cleanup:

  // nothing to do; the destructor of <dm> should clean all sections properly

 return TCL_OK;
} // dxfio_writetcmd


// dxfio_registerwritecb:
//
int
dxfio_registerwritecb(char *name, dxfio_writecb *cb)
{
 int ay_status = AY_OK;
 int new_item = 0;
 Tcl_HashEntry *entry = NULL;
 Tcl_HashTable *ht = &dxfio_write_ht;

  if(!cb)
    return AY_ENULL;

  if((entry = Tcl_FindHashEntry(ht, name)))
    {
      return AY_ERROR; // name already registered
    }
  else
    {
      // create new entry
      entry = Tcl_CreateHashEntry(ht, name, &new_item);
      Tcl_SetHashValue(entry, (char*)cb);
    }

 return ay_status;
} // dxfio_registerwritecb


// dxfio_writeprogressdcb:
//
int
dxfio_writeprogressdcb(float progress, void *clientdata)
{
 static float oldprogress = 0.0f;
 char progressstr[32];
 char fname[] = "dxfio_writefile";
 char arrname[] = "dxfio_options", varname1[] = "Progress";
 char varname2[] = "Cancel";
 const char *val = NULL;

  if(clientdata != NULL)
    {
      oldprogress = 0.0f;
      return 1;
    }

  // set progress counter in the GUI
  if(progress-oldprogress > 0.05)
    {
      sprintf(progressstr, "%d", (int)(50.0f+(progress*50.0f)));

      Tcl_SetVar2(ay_interp, arrname, varname1, progressstr,
		  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

      oldprogress = progress;
    } // if

  // also, check for cancel button (at a somewhat higher frequency)
  if(progress-oldprogress > 0.01)
    {
      val = Tcl_GetVar2(ay_interp, arrname, varname2,
			TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      if(val && val[0] == '1')
	{
	  ay_error(AY_EOUTPUT, fname,
		   "Export cancelled! Not all objects may have been written!");
	  dxfio_cancelled = AY_TRUE;
	  return 0;
	}
    } // if

  return 1;
} // dxfio_writeprogressdcb

extern "C" {

/* note: this function _must_ be capitalized exactly this way
 * regardless of the filename of the shared object (see: man n load)!
 */
#ifdef WIN32
__declspec( dllexport ) int
Dxfio_Init(Tcl_Interp *interp)
#else
int
Dxfio_Init(Tcl_Interp *interp)
#endif // WIN32
{
 char fname[] = "dxfio_init";
 int ay_status = AY_OK;

  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }

  /* first, check versions */
  if(ay_checkversion(fname, dxfio_version_ma, dxfio_version_mi))
    {
      return TCL_ERROR;
    }

  // init hash table for write callbacks
  Tcl_InitHashTable(&dxfio_write_ht, TCL_ONE_WORD_KEYS);

  // fill hash table
  ay_status += dxfio_registerwritecb((char *)(AY_IDLEVEL),
				     dxfio_writelevel);

  ay_status += dxfio_registerwritecb((char *)(AY_IDCLONE),
				     dxfio_writeclone);

  ay_status += dxfio_registerwritecb((char *)(AY_IDMIRROR),
				     dxfio_writeclone);

  ay_status += dxfio_registerwritecb((char *)(AY_IDINSTANCE),
				     dxfio_writeinstance);

  ay_status += dxfio_registerwritecb((char *)(AY_IDSCRIPT),
				     dxfio_writescript);

  ay_status += dxfio_registerwritecb((char *)(AY_IDPOMESH),
				     dxfio_writepomesh);

  ay_status += dxfio_registerwritecb((char *)(AY_IDNPATCH),
				     dxfio_writenpatch);

  ay_status += dxfio_registerwritecb((char *)(AY_IDNCURVE),
				     dxfio_writencurve);

  if(ay_status)
    return TCL_ERROR;

  // source dxfio.tcl, it contains vital Tcl-code
  if((Tcl_EvalFile(interp, "dxfio.tcl")) != TCL_OK)
    {
      ay_error(AY_ERROR, fname, "Error while sourcing \"dxfio.tcl\"!");
      return TCL_ERROR;
    }

  // create new Tcl commands to interface with the plugin
  Tcl_CreateCommand(interp, "dxfioRead", (Tcl_CmdProc*) dxfio_readtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "dxfioWrite", (Tcl_CmdProc*) dxfio_writetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  ay_error(AY_EOUTPUT, fname, "Plugin 'dxfio' successfully loaded.");

 return TCL_OK;
} /* Dxfio_Init */

} // extern "C"

/*! \file dxfio.cpp \brief DXF Import/Export Plugin */
