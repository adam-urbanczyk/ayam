/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2001 by Randolf Schultz
 * (randolf.schultz@gmail.com) and others.
 *
 * All rights reserved.
 *
 * See the file License for details.
 *
 */

/* sdnpatch.cpp - sdnpatch custom object (Subdivision NURBS) */

#include "ayam.h"

#include "snurbs.h"

using namespace std;
using namespace std::tr1::placeholders;
using namespace snurbs;

static char *sdnpatch_name = "SDNPatch";

static unsigned int sdnpatch_id;

typedef struct sdnpatch_object_s
{
  unsigned char subdivLevel;
  unsigned char subdivDegree;
  Mesh *controlMesh;
  GLuint controlObject;

  Mesh *subdivMesh;
  GLuint subdivObject;

  Mesh *lowDegreeMesh;
  GLuint isoparamTexture;
  bool texFirstFace;
  GLfloat                firstS;
  GLfloat                firstT;
  float                  max[3];
  float                  min[3];
  float                  median[3];
  float                  boundingCube;
  int                    nVerts;
  GLuint                 selectedFace;
  GLuint                 selectedEdge;

  double                 selectCloseLoop[4];
  bool                   selectFirstVert;
  snurbs::KnotInterval  *selectedKnot;
  snurbs::KnotInterval  *lowDegreeKnot;
} sdnpatch_object;

extern "C" {
#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int Sdnpatch_Init(Tcl_Interp *interp);
}

int sdnpatch_notifycb(ay_object *o);


class AyWriter : public FlatMeshHandler
{
public:
  AyWriter(FILE *filep);
  //~AyWriter(void);
  void addVertex(VertexPrecision x,
		 VertexPrecision y,
		 VertexPrecision z,
		 VertexPrecision w);
  void startFace(unsigned int numEdges);
  void addToFace(unsigned int vertNum);
  void closeFace(void);
  void addKnotInterval(unsigned int vertex1,
		       unsigned int vertex2,
		       KnotPrecision interval);
  void finishKnotIntervals(void);

private:
  FILE *m_fp;

  unsigned int m_numVertices;
  vector<VertexPrecision> m_vertices;
  unsigned int m_numFaces;
  vector<unsigned int> m_numEdges;
  vector<unsigned int> m_faces;
  unsigned int m_numKnots;
  vector<unsigned int> m_v1;
  vector<unsigned int> m_v2;
  vector<KnotPrecision> m_intervals;

};


AyWriter::AyWriter(FILE *filep)
{
  m_fp = filep;
  m_numVertices = 0;
  m_numFaces = 0;
  m_numKnots = 0;
} /* AyWriter::AyWriter */


void
AyWriter::addVertex(VertexPrecision x,
		    VertexPrecision y,
		    VertexPrecision z,
		    VertexPrecision w)
{
  m_numVertices++;
  m_vertices.push_back(x);
  m_vertices.push_back(y);
  m_vertices.push_back(z);
  m_vertices.push_back(w);
} /* AyWriter::addVertex */


void
AyWriter::startFace(unsigned int numEdges)
{
    m_numEdges.push_back(numEdges);
} /* AyWriter::startFace */


void
AyWriter::addToFace(unsigned int vertNum)
{
  m_faces.push_back(vertNum);
} /* AyWriter::addToFace */

void
AyWriter::closeFace(void)
{
  m_numFaces++;
} /* AyWriter::closeFace */

void
AyWriter::addKnotInterval(unsigned int vertex1,
			  unsigned int vertex2,
			  KnotPrecision interval)
{
  m_v1.push_back(vertex1);
  m_v2.push_back(vertex1);
  m_intervals.push_back(interval);
  m_numKnots++;
} /* AyWriter::addKnotInterval */

void
AyWriter::finishKnotIntervals(void)
{
  VertexPrecision x, y, z, w;
  vector<VertexPrecision>::iterator vi = m_vertices.begin();
  vector<unsigned int>::iterator ei = m_numEdges.begin();
  vector<unsigned int>::iterator fi = m_faces.begin();
  vector<unsigned int>::iterator v1i = m_v1.begin();
  vector<unsigned int>::iterator v2i = m_v2.begin();
  vector<KnotPrecision>::iterator ki = m_intervals.begin();
  unsigned int i, j, e, f;

  fprintf(m_fp, "%u\n", m_numVertices);

  for(i = 0; i < m_numVertices; i++)
    {
      x = *vi;
      vi++;
      y = *vi;
      vi++;
      z = *vi;
      vi++;
      w = *vi;
      vi++;
      fprintf(m_fp, "%g %g %g %g\n", x, y, z, w);
    }
  if(m_numFaces > 0)
    {
      fprintf(m_fp, "%u", m_numFaces);

      for(i = 0; i < m_numFaces; i++)
	{
	  e = *ei;
	  fprintf(m_fp, "%u", e);
	  for(j = 0; j < e; j++)
	    {
	      f = *fi;
	      fi++;
	      fprintf(m_fp, " %u", f);
	    }
	  fprintf(m_fp, "\n");
	}
    }
  else
    {
      fprintf(m_fp, "0\n");
    }

  fprintf(m_fp, "%u\n", m_numKnots);

  for(i = 0; i < m_numKnots; i++)
    {
      fprintf(m_fp, "%u %u %f\n", *v1i, *v2i, *ki);
      v1i++;
      v2i++;
      ki++;
    }

 return;
} /* AyWriter::finishKnotIntervals */


/* sdnpatch_createcb:
 *  create callback function of sdnpatch object
 */
int
sdnpatch_createcb(int argc, char *argv[], ay_object *o)
{
 sdnpatch_object *sdnpatch = NULL;
 char fname[] = "crtsdnpatch";

  if(!o)
    return AY_ENULL;

  if(!(sdnpatch = (sdnpatch_object*)calloc(1, sizeof(sdnpatch_object))))
    {
      ay_error(AY_EOMEM, fname, NULL);
      return AY_ERROR;
    }

  /*sdnpatch->lowDegreeMesh = new snurbs::Mesh;*/
  sdnpatch->subdivDegree = 3;
  sdnpatch->subdivLevel = 2;

  sdnpatch->controlMesh = new Mesh(sdnpatch->subdivDegree);

  MeshBuilder *meshBuilder = MeshBuilder::create(*(sdnpatch->controlMesh));

  meshBuilder->addVertex(0,0,0,1);
  meshBuilder->addVertex(1,0,0,1);
  meshBuilder->addVertex(1,1,0,1);
  meshBuilder->addVertex(0,1,0,1);

  meshBuilder->addVertex(0,0,1,1);
  meshBuilder->addVertex(1,0,1,1);
  meshBuilder->addVertex(1,1,1,1);
  meshBuilder->addVertex(0,1,1,1);

  meshBuilder->finishVertices();

  meshBuilder->startFace(4);
  meshBuilder->addToFace(0);
  meshBuilder->addToFace(1);
  meshBuilder->addToFace(2);
  meshBuilder->addToFace(3);
  meshBuilder->closeFace();

  meshBuilder->startFace(4);
  meshBuilder->addToFace(1);
  meshBuilder->addToFace(5);
  meshBuilder->addToFace(6);
  meshBuilder->addToFace(2);
  meshBuilder->closeFace();

  meshBuilder->startFace(4);
  meshBuilder->addToFace(5);
  meshBuilder->addToFace(4);
  meshBuilder->addToFace(7);
  meshBuilder->addToFace(6);
  meshBuilder->closeFace();

  meshBuilder->startFace(4);
  meshBuilder->addToFace(4);
  meshBuilder->addToFace(0);
  meshBuilder->addToFace(3);
  meshBuilder->addToFace(7);
  meshBuilder->closeFace();

  meshBuilder->startFace(4);
  meshBuilder->addToFace(0);
  meshBuilder->addToFace(1);
  meshBuilder->addToFace(5);
  meshBuilder->addToFace(4);
  meshBuilder->closeFace();

  meshBuilder->startFace(4);
  meshBuilder->addToFace(3);
  meshBuilder->addToFace(2);
  meshBuilder->addToFace(6);
  meshBuilder->addToFace(7);
  meshBuilder->closeFace();

  meshBuilder->finishFaces();

  meshBuilder->finishKnotIntervals();

  MeshBuilder::dispose(meshBuilder);

  o->refine = sdnpatch;

 return AY_OK;
} /* sdnpatch_createcb */


/* sdnpatch_deletecb:
 *  delete callback function of sdnpatch object
 */
int
sdnpatch_deletecb(void *c)
{
 sdnpatch_object *sdnpatch = NULL;

  if(!c)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)(c);

  if(sdnpatch->controlMesh)
    delete sdnpatch->controlMesh;

  if(sdnpatch->subdivMesh)
    delete sdnpatch->subdivMesh;

  free(sdnpatch);

 return AY_OK;
} /* sdnpatch_deletecb */


/* sdnpatch_copycb:
 *  copy callback function of sdnpatch object
 */
int
sdnpatch_copycb(void *src, void **dst)
{
 sdnpatch_object *sdnpatch = NULL;
 sdnpatch_object *srcsdnpatch = NULL;

  if(!src || !dst)
    return AY_ENULL;

  if(!(sdnpatch = (sdnpatch_object*)calloc(1, sizeof(sdnpatch_object))))
    return AY_EOMEM;

  memcpy(sdnpatch, src, sizeof(sdnpatch_object));

  sdnpatch->controlMesh = NULL;
  sdnpatch->subdivMesh = NULL;

  srcsdnpatch = (sdnpatch_object*)src;
  if(srcsdnpatch->controlMesh)
    sdnpatch->controlMesh = new Mesh(*(srcsdnpatch->controlMesh));

  if(srcsdnpatch->subdivMesh)
    sdnpatch->subdivMesh = new Mesh(*(srcsdnpatch->subdivMesh));

  *dst = (void *)sdnpatch;

 return AY_OK;
} /* sdnpatch_copycb */


void
sdnpatch_lineloopcb(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
 static int calls = 0;

  glVertex4d(x, y, z, w);

  if(calls > 3)
    {
      glEnd();
      glBegin(GL_LINE_LOOP);
      calls = 0;
    }
  else
    {
      calls++;
    }

 return;
} /* sdnpatch_lineloopcb */


/* sdnpatch_drawcb:
 *  draw (display in an Ayam view window) callback function of sdnpatch object
 */
int
sdnpatch_drawcb(struct Togl *togl, ay_object *o)
{
 sdnpatch_object *sdnpatch = NULL;
 MeshFlattener *meshFlattener = NULL;

  if(!o)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)o->refine;

  if(!sdnpatch)
    return AY_ENULL;

  meshFlattener = MeshFlattener::create(*(sdnpatch->controlMesh));
  meshFlattener->setCompatible(true);

  glBegin(GL_LINE_LOOP);
   meshFlattener->receiveVertices(std::tr1::bind(&sdnpatch_lineloopcb,
						 _1, _2, _3, _4));
   meshFlattener->flattenFaces();
  glEnd();

  MeshFlattener::dispose(meshFlattener);

 return AY_OK;
} /* sdnpatch_drawcb */


void
sdnpatch_quadcb(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
 static int calls = 0;

  glVertex4d(x, y, z, w);

  if(calls > 3)
    {
      glEnd();
      glBegin(GL_QUADS);
      calls = 0;
    }
  else
    {
      calls++;
    }

 return;
} /* sdnpatch_quadcb */


/* sdnpatch_shadecb:
 *  shade (display in an Ayam view window) callback function of sdnpatch object
 */
int
sdnpatch_shadecb(struct Togl *togl, ay_object *o)
{
 sdnpatch_object *sdnpatch = NULL;
 MeshFlattener *meshFlattener = NULL;

  if(!o)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)o->refine;

  if(!sdnpatch)
    return AY_ENULL;

  meshFlattener = MeshFlattener::create(*(sdnpatch->subdivMesh));
  meshFlattener->setCompatible(true);

  glBegin(GL_QUADS);
   meshFlattener->receiveVertices(std::tr1::bind(&sdnpatch_quadcb,
						 _1, _2, _3, _4));
   meshFlattener->receiveSmoothNormals(&glNormal3d);
   meshFlattener->flattenFaces();
  glEnd();

  MeshFlattener::dispose(meshFlattener);

 return AY_OK;
} /* sdnpatch_shadecb */


/* sdnpatch_drawhcb:
 *  draw handles (in an Ayam view window) callback function of sdnpatch object
 */
int
sdnpatch_drawhcb(struct Togl *togl, ay_object *o)
{
 sdnpatch_object *sdnpatch = NULL;
 MeshFlattener *meshFlattener = NULL;

  if(!o)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)o->refine;

  if(!sdnpatch)
    return AY_ENULL;

  meshFlattener = MeshFlattener::create(*(sdnpatch->controlMesh));
  meshFlattener->setCompatible(true);

  glBegin(GL_POINTS);
   meshFlattener->receiveVertices(&glVertex4d);
   meshFlattener->flattenFaces();
  glEnd();

  MeshFlattener::dispose(meshFlattener);

 return AY_OK;
} /* sdnpatch_drawhcb */


/* sdnpatch_setpropcb:
 *  set property (from Tcl to C context) callback function of sdnpatch object
 */
int
sdnpatch_setpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1 = "SDNPatchAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 sdnpatch_object *sdnpatch = NULL;
 int itemp = 0;

  if(!o)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)o->refine;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Degree",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &itemp);
  sdnpatch->subdivDegree = itemp;

  Tcl_SetStringObj(ton,"Level",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &itemp);
  sdnpatch->subdivLevel = itemp;

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* sdnpatch_setpropcb */


/* sdnpatch_getpropcb:
 *  get property (from C to Tcl context) callback function of sdnpatch object
 */
int
sdnpatch_getpropcb(Tcl_Interp *interp, int argc, char *argv[], ay_object *o)
{
 char *n1="SDNPatchAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 sdnpatch_object *sdnpatch = NULL;

  if(!o)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)(o->refine);

  toa = Tcl_NewStringObj(n1,-1);

  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Degree",-1);
  to = Tcl_NewIntObj(sdnpatch->subdivDegree);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Level",-1);
  to = Tcl_NewIntObj(sdnpatch->subdivLevel);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);

 return AY_OK;
} /* sdnpatch_getpropcb */


/* sdnpatch_readcb:
 *  read (from scene file) callback function of sdnpatch object
 */
int
sdnpatch_readcb(FILE *fileptr, ay_object *o)
{
 sdnpatch_object *sdnpatch = NULL;
 int itemp;
 unsigned int i, j, numVerts, numFaces, numKnots;
 unsigned int numEdges, e, v1, v2;
 VertexPrecision x, y, z, w;
 KnotPrecision k;
 MeshBuilder *meshBuilder;

  if(!o)
    return AY_ENULL;

  if(!(sdnpatch = (sdnpatch_object*)calloc(1, sizeof(sdnpatch_object))))
    { return AY_EOMEM; }

  fscanf(fileptr, "%d\n", &itemp);
  sdnpatch->subdivDegree = itemp;
  fscanf(fileptr, "%d\n", &itemp);
  sdnpatch->subdivLevel = itemp;

  sdnpatch->controlMesh = new Mesh(sdnpatch->subdivDegree);
  meshBuilder = MeshBuilder::create(*(sdnpatch->controlMesh));

  fscanf(fileptr, "%u\n", &numVerts);

  for(i = 0; i < numVerts; i++)
    {
      fscanf(fileptr, "%lg %lg %lg %lg\n", &x, &y, &z, &w);
      meshBuilder->addVertex(x, y, z, w);
    }
  meshBuilder->finishVertices();

  fscanf(fileptr, "%u\n", &numFaces);

  for(i = 0; i < numFaces; i++)
    {
      fscanf(fileptr, "%u", &numEdges);
      meshBuilder->startFace(numEdges);

      for(j = 0; j < numEdges; j++)
	{
	  fscanf(fileptr, "%u", &e);
	  meshBuilder->addToFace(e);
	}
      meshBuilder->closeFace();
    }
  meshBuilder->finishFaces();

  fscanf(fileptr, "%u\n", &numKnots);

  for(i = 0; i < numKnots; i++)
    {
      fscanf(fileptr, "%u %u %f\n", &v1, &v2, &k);

      meshBuilder->addKnotInterval(v1, v2, k);
    }
  meshBuilder->finishKnotIntervals();

  MeshBuilder::dispose(meshBuilder);

  o->refine = sdnpatch;

 return AY_OK;
} /* sdnpatch_readcb */


/* sdnpatch_writecb:
 *  write (to scene file) callback function of sdnpatch object
 */
int
sdnpatch_writecb(FILE *fileptr, ay_object *o)
{
 sdnpatch_object *sdnpatch = NULL;
 Mesh *mesh = NULL;
 MeshFlattener *meshFlattener = NULL;
 FlatMeshHandler *handler = NULL;

  if(!o)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)(o->refine);

  fprintf(fileptr, "%d\n", sdnpatch->subdivDegree);

  fprintf(fileptr, "%d\n", sdnpatch->subdivLevel);

  /* write the control mesh using the AyWriter MeshFlattener handler */
  mesh = sdnpatch->controlMesh;
  meshFlattener = MeshFlattener::create(*(sdnpatch->controlMesh));
  meshFlattener->setCompatible(true);
  handler = new AyWriter(fileptr);
  meshFlattener->flatten(*handler);
  delete handler;
  MeshFlattener::dispose(meshFlattener);

 return AY_OK;
} /* sdnpatch_writecb */


/* sdnpatch_wribcb:
 *  RIB export callback function of sdnpatch object
 */
int
sdnpatch_wribcb(char *file, ay_object *o)
{
 sdnpatch_object *sdnpatch = NULL;


  if(!o)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object*)o->refine;

 return AY_OK;
} /* sdnpatch_wribcb */


/* sdnpatch_bbccb:
 *  bounding box calculation callback function of sdnpatch object
 */
int
sdnpatch_bbccb(ay_object *o, double *bbox, int *flags)
{
 sdnpatch_object *sdnpatch = NULL;

  if(!o || !bbox)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)o->refine;

 return AY_OK;
} /* sdnpatch_bbccb */


/* sdnpatch_notifycb:
 *  notification callback function of sdnpatch object
 */
int
sdnpatch_notifycb(ay_object *o)
{
 sdnpatch_object *sdnpatch = NULL;
 int curlev = 0;

  if(!o)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)(o->refine);

  if(sdnpatch->subdivMesh)
    {
      delete sdnpatch->subdivMesh;
    }

  sdnpatch->subdivMesh = new Mesh(*(sdnpatch->controlMesh));

  while(curlev < sdnpatch->subdivLevel)
    {
      sdnpatch->subdivMesh->subdivide();
      curlev++;
    }

 return AY_OK;
} /* sdnpatch_notifycb */


extern "C" {

/* Sdnpatch_Init:
 * initializes the sdnpatch module/plugin by registering a new
 * object type (SDNPatch) and loading the accompanying Tcl script file.
 * Note: This function _must_ be capitalized exactly this way
 * (_S_dnpatch__I_nit, not sdnpatch_init!)
 * regardless of the filename of the shared object (see: man n load)!
 */
#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int
Sdnpatch_Init(Tcl_Interp *interp)
{
 int ay_status = AY_OK;
 char fname[] = "sdnpatch_init";

#ifdef WIN32
  if(Tcl_InitStubs(interp, "8.2", 0) == NULL)
    {
      return TCL_ERROR;
    }
#endif /* WIN32 */

  ay_status = ay_otype_register(sdnpatch_name,
				sdnpatch_createcb,
				sdnpatch_deletecb,
				sdnpatch_copycb,
				sdnpatch_drawcb,
				sdnpatch_drawhcb,
				sdnpatch_shadecb,
				sdnpatch_setpropcb,
				sdnpatch_getpropcb,
				NULL, /* No Picking! */
				sdnpatch_readcb,
				sdnpatch_writecb,
				sdnpatch_wribcb,
				sdnpatch_bbccb,
				&sdnpatch_id);

  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Error registering custom object!");
      return TCL_OK;
    }


  ay_status = ay_notify_register(sdnpatch_notifycb, sdnpatch_id);


  /* source sdnpatch.tcl, it contains Tcl-code to build
     the Sdnpatch-Attributes Property GUI */
  if((Tcl_EvalFile(interp, "sdnpatch.tcl")) != TCL_OK)
     {
       ay_error(AY_ERROR, fname,
		  "Error while sourcing \"sdnpatch.tcl\"!");
       return TCL_OK;
     }

  ay_error(AY_EOUTPUT, fname,
	   "Custom object \"SDNPatch\" successfully loaded.");

 return TCL_OK;
} /* Sdnpatch_Init */


} // extern "C"
