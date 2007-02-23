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

// dxfio.cpp - Ayam DXF Import/Export Plugin

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
#include <dime/entities/Line.h>
#include <dime/entities/LWPolyline.h>
#include <dime/entities/Polyline.h>
#include <dime/entities/Solid.h>
#include <dime/entities/Spline.h>
#include <dime/entities/Trace.h>
#include <dime/entities/UnknownEntity.h>
#include <dime/entities/Vertex.h>
#include <dime/util/Linear.h>
#include <stdio.h>

// local types


// global variables

char dxfio_version_ma[] = AY_VERSIONSTR;
char dxfio_version_mi[] = AY_VERSIONSTRMI;

static Tcl_HashTable dxfio_write_ht;

ay_object *dxfio_lrobject = NULL;

static double tm[16] = {0}; // current transformation matrix

int dxfio_importcurves = AY_TRUE;
int dxfio_exportcurves = AY_TRUE;
int dxfio_expselected = AY_FALSE;
int dxfio_expobeynoexport = AY_TRUE;
int dxfio_expignorehidden = AY_TRUE;
int dxfio_exptoplevellayers = AY_TRUE;

int dxfio_currentlayer = 0;

double dxfio_rescaleknots = 0.0;

double dxfio_accuracy = 1.0e-12;

double dxfio_scalefactor = 1.0;

char dxfio_stagnamedef[] = "mys";
char *dxfio_stagname = dxfio_stagnamedef;
char dxfio_ttagnamedef[] = "myt";
char *dxfio_ttagname = dxfio_ttagnamedef;

// prototypes of functions local to this module
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

int dxfio_read3dface(const class dimeState *state,
		     class dime3DFace *tdface,
		     void *clientdata);

int dxfio_readarc(const class dimeState *state,
		  class dimeArc *arc,
		  void *clientdata);

int dxfio_readblock(const class dimeState *state,
		    class dimeBlock *block,
		    void *clientdata);

int dxfio_readcircle(const class dimeState *state,
		     class dimeCircle *circle,
		     void *clientdata);

int dxfio_readellipse(const class dimeState *state,
		      class dimeEllipse *ellipse,
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

// implementation of functions

// dxfio_extrudebpatch:
//
int
dxfio_extrudebpatch(class dimeExtrusionEntity *entity, ay_object *o)
{
 int ay_status = AY_OK;
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
	      if(AY_COMP3DP(p1, p2))
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
 ay_bpatch_object *newbp = NULL;
 dimeVec3f v0, v1, v2, v3;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  if(!(newbp = (ay_bpatch_object*)calloc(1, sizeof(ay_bpatch_object))))
    { free(newo); return AY_EOMEM; }

  tdface->getVertices(v0, v1, v2, v3);

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

  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDBPATCH;
  newo->refine = newbp;

  // link the new bpatch/3dface into the scene hierarchy
  ay_status = ay_object_link(newo);

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

  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDNCIRCLE;
  newo->refine = newc;
  arc->getCenter(center);
  newo->movx = center[0];
  newo->movy = center[1];
  newo->movz = center[2];

  ay_status = ay_notify_force(newo);

  // link the new arc into the scene hierarchy
  ay_status = ay_object_link(newo);

 return ay_status;
} // dxfio_readarc


// dxfio_readblock:
//
int
dxfio_readblock(const class dimeState *state,
		class dimeBlock *block,
		void *clientdata)
{
 int ay_status = AY_OK;
 ay_object *newo = NULL;
 ay_level_object *newl = NULL;

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    { return AY_EOMEM; }

  if(!(newl = (ay_level_object*)calloc(1, sizeof(ay_level_object))))
    { free(newo); return AY_EOMEM; }

  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDLEVEL;
  newo->refine = newl;

  // link the new block into the scene hierarchy
  ay_status = ay_object_link(newo);

 return ay_status;
} // dxfio_readblock


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
  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDNCIRCLE;
  newo->refine = newc;

  newo->movx = circle->getCenter()[0];
  newo->movy = circle->getCenter()[1];
  newo->movz = circle->getCenter()[2];

  ay_status = ay_notify_force(newo);

  // link the new circle into the scene hierarchy
  ay_status = ay_object_link(newo);

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

  ay_status = ay_object_defaults(newo);

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

  ay_status = ay_notify_force(newo);

  // link the new circle/ellipse into the scene hierarchy
  ay_status = ay_object_link(newo);

 return ay_status;
} // dxfio_readellipse


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
		(ay_nurbcurve_object**)&(newo->refine));

  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDNCURVE;

  // link the new curve/line into the scene hierarchy
  ay_status = ay_object_link(newo);

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
		(ay_nurbcurve_object**)&(newo->refine));

  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDNCURVE;

  // XXXX process bulges



  // link the new curve/lwpolyline into the scene hierarchy
  ay_status = ay_object_link(newo);

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
	} // if
      newcv[a+3] = 1.0;
      a += stride;
    } // for

  ay_nct_create(2, len, AY_KTNURB, newcv, NULL,
		(ay_nurbcurve_object**)&(newo->refine));
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
 unsigned int a, i, j, numfaces = 0, numverts = 0,m,n;
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
  a = 0;
  j = 0;
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
      pomesh->verts[a+2] = i+n;
      pomesh->verts[a+3] = i+n+1;
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
      /*
      if(polyline->getFlags() & dimePolyline::IS_POLYMESH_3D)
      {*/
	  pomesh->controlv[a+2] = cv[2];
	  /*
	}
      else
	{
	  cv = polyline->getElevation();
	  pomesh->controlv[a+2] = cv[2];
	  }*/
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
 int w, h, uorder, vorder, uknott, vknott, numcv;
 int i, j, a, b, c;
 double *controlv = NULL;
 bool closem = false, closen = false;
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
      closem = true;
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
      closen = true;
    }

  // copy coordinate values
  numcv = w*h;
  if(!(controlv = (double*)calloc(numcv*4, sizeof(double))))
    { return AY_EOMEM; }

  a = 0;
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
		(ay_nurbpatch_object**)&(newo->refine));

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
 unsigned int a, i, j, numfaces = 0, numverts = 0;
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
      ay_status = ay_object_link(newo);
      newo = NULL;
    }
  /*
cleanup:
  */
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

  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDBPATCH;
  newo->refine = newbp;

  // link the new bpatch/solid into the scene hierarchy
  ay_status = ay_object_link(newo);

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
		(ay_nurbcurve_object**)&(newo->refine));

  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDNCURVE;

  // link the new curve/spline into the scene hierarchy
  ay_status = ay_object_link(newo);

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

  ay_status = ay_object_defaults(newo);

  newo->type = AY_IDBPATCH;
  newo->refine = newbp;

  // link the new bpatch/trace into the scene hierarchy
  ay_status = ay_object_link(newo);

  // XXXX extrude trace

 return ay_status;
} // dxfio_readtrace


// dxfio_readentitydcb:
//  Dime entity traversal callback
bool
dxfio_readentitydcb(const class dimeState *state,
		    class dimeEntity *entity,
		    void *clientdata)
{
 char fname[] = "dxfio_readentity";

  if(!state || ! entity)
    return true;

  switch(entity->typeId())
    {
    case dimeBase::dime3DFaceType:
      dxfio_read3dface(state, (dime3DFace*)entity, clientdata);
      break;
    case dimeBase::dimeArcType:
      dxfio_readarc(state, (dimeArc*)entity, clientdata);
      break;
    case dimeBase::dimeCircleType:
      dxfio_readcircle(state, (dimeCircle*)entity, clientdata);
      break;
    case dimeBase::dimeEllipseType:
      dxfio_readellipse(state, (dimeEllipse*)entity, clientdata);
      break;
    case dimeBase::dimeLineType:
      dxfio_readline(state, (dimeLine*)entity, clientdata);
      break;
    case dimeBase::dimeLWPolylineType:
      dxfio_readlwpolyline(state, (dimeLWPolyline*)entity, clientdata);
      break;
    case dimeBase::dimePolylineType:
      dxfio_readpolyline(state, (dimePolyline*)entity, clientdata);
      break;
    case dimeBase::dimeSolidType:
      dxfio_readsolid(state, (dimeSolid*)entity, clientdata);
      break;
    case dimeBase::dimeSplineType:
      dxfio_readspline(state, (dimeSpline*)entity, clientdata);
      break;
    case dimeBase::dimeTraceType:
      dxfio_readtrace(state, (dimeTrace*)entity, clientdata);
      break;
    case dimeBase::dimeUnknownEntityType:
      //if(dxfio_errorlevel > 1)
      ay_error(AY_EWARN, fname, "Skipping entity of unknown type.");
      break;
    default:
      break;
    } // switch

 return true;
} // dxfio_readentitydcb


// dxfio_readprogressdcb:
//
int
dxfio_readprogressdcb(float progress, void *clientdata)
{
 static float oldprogress = 0.0f;
 char progressstr[32];
 char arrname[] = "dxfio_options", varname[] = "Progress";

  if(clientdata != NULL)
    {
      oldprogress = 0.0f;
      return 1;
    }

  if(progress-oldprogress > 0.05)
    {
      sprintf(progressstr, "%d", (int)(progress*50.0f));

      Tcl_SetVar2(ay_interp, arrname, varname, progressstr,
		  TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
      while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

      oldprogress = progress;
    } // if

  return 1;
} // dxfio_readprogressdcb


// dxfio_readtcmd:
//  Tcl command to read DXF files using Dime
int
dxfio_readtcmd(ClientData clientData, Tcl_Interp *interp,
	       int argc, char *argv[])
{
  //int ay_status = AY_OK;
 char fname[] = "dxfio_read";
 char *minus, lineerrstr[64];
 int i = 2, sframe = -1, eframe = -1;
 double accuracy = 0.1;
 char arrname[] = "dxfio_options", varname[] = "Progress";

  dxfio_importcurves = AY_TRUE;
  dxfio_rescaleknots = 0.0;
  dxfio_scalefactor = 1.0;
  // reset internal progress counter
  dxfio_readprogressdcb(0.0f, (void*)1);

  // check args
  if(argc < 2)
    {
      ay_error(AY_EARGS, fname, "filename");
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
      if(!strcmp(argv[i], "-t"))
	{
	  dxfio_stagname = argv[i+1];
	  dxfio_ttagname = argv[i+2];
	  i++;
	}
      else
      if(!strcmp(argv[i], "-l"))
	{
	  if(argv[i+1])
	    {
	      if(*argv[i+1] != '-')
		{
		  sscanf(argv[i+1], "%d", &sframe);
		  eframe = sframe;
		  if((strlen(argv[i+1]) > 3) &&
		     (minus = strchr(/*(const char*)*/(&(argv[i+1][1])), '-')))
		    {
		      minus++;
		      if(*minus != '\0')
			{
			  sscanf(minus, "%d", &eframe);
			}
		      else
			{
			  ay_error(AY_ERROR, fname,
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
      ay_error(AY_EOPENFILE, fname, argv[1]);
      return TCL_OK;
    }

  in.setCallback(&dxfio_readprogressdcb, NULL);

  // try reading the file
  dimeModel model;

  if(!model.read(&in))
    {
      int line = in.getFilePosition();
      sprintf(lineerrstr, "DXF read error in line: %d", line);
      ay_error(AY_ERROR, fname, lineerrstr);
      return TCL_OK;
    }

  // set progress
  Tcl_SetVar2(ay_interp, arrname, varname, "50",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  // get all entities
  dxfio_lrobject = NULL;
  model.traverseEntities(&dxfio_readentitydcb, NULL,
			 false, true, false);

  // set progress
  Tcl_SetVar2(ay_interp, arrname, varname, "100",
	      TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  while(Tcl_DoOneEvent(TCL_DONT_WAIT)){};

  dxfio_stagname = dxfio_stagnamedef;
  dxfio_ttagname = dxfio_ttagnamedef;

 return TCL_OK;
} // dxfio_readtcmd


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
 // int ay_status = AY_OK;

  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }

  /* first, check versions */
  if(strcmp(ay_version_ma, dxfio_version_ma))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "It is unsafe to continue! Bailing out...");
      return TCL_OK;
    }

  if(strcmp(ay_version_mi, dxfio_version_mi))
    {
      ay_error(AY_ERROR, fname,
	       "Plugin has been compiled for a different Ayam version!");
      ay_error(AY_ERROR, fname, "However, it is probably safe to continue...");
    }

#ifndef AYDXFIOWRAPPED
  // source dxfio.tcl, it contains vital Tcl-code
  if((Tcl_EvalFile(interp, "dxfio.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname,
		  "Error while sourcing \\\"dxfio.tcl\\\"!");
       return TCL_OK;
     }
#endif // !AYDXFIOWRAPPED

  // create new Tcl commands to interface with the plugin
  Tcl_CreateCommand(interp, "dxfioRead", dxfio_readtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  /*
  Tcl_CreateCommand(interp, "dxfioWrite", dxfio_writetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
  */
  /* init hash table for write callbacks */
  Tcl_InitHashTable(&dxfio_write_ht, TCL_ONE_WORD_KEYS);

  /* fill hash table */


  ay_error(AY_EOUTPUT, fname, "Plugin 'dxfio' successfully loaded.");

 return TCL_OK;
} /* Dxfio_Init */

} // extern "C"
