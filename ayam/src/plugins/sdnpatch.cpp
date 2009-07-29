/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2009 by Randolf Schultz
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

  vector<Vertex*> *controlVertices;
  double *controlCoords;

} sdnpatch_object;

extern "C" {
#ifdef WIN32
  __declspec (dllexport)
#endif /* WIN32 */
int Sdnpatch_Init(Tcl_Interp *interp);
}

int sdnpatch_notifycb(ay_object *o);

int sdnpatch_convnp(int mode, ay_object *p, ay_object **result);

int sdnpatch_convnptcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

class AyWriter : public FlatMeshHandler
{
public:
  AyWriter(FILE *filep);

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




class AyConvertor
{
public:
  AyConvertor(MeshFlattener *flattener)
  {
    if(!flattener)
      return;
    m_flattener = flattener;
    m_numVertices = 0;
    m_numFaces = 0;
  };

  void vertexcb(VertexPrecision x,
		VertexPrecision y,
		VertexPrecision z,
		VertexPrecision w);

  void normalcb(VertexPrecision x,
		VertexPrecision y,
		VertexPrecision z);

  void convert(ay_pomesh_object *pomesh);

private:
  MeshFlattener *m_flattener;
  unsigned int m_numVertices;
  unsigned int m_numFaces;
  vector<VertexPrecision> m_vertices;
  vector<VertexPrecision> m_normals;
}; /* AyConvertor */


void
AyConvertor::vertexcb(VertexPrecision x,
		      VertexPrecision y,
		      VertexPrecision z,
		      VertexPrecision w)
{
 static int count = 0;

  if(count == 3)
    {
      m_numFaces++;
      count = 0;
    }
  else
    {
      count++;
    }

  m_numVertices++;
  m_vertices.push_back(x);
  m_vertices.push_back(y);
  m_vertices.push_back(z);
  m_vertices.push_back(w);

 return;
} /* AyConvertor::vertexcb */


void
AyConvertor::normalcb(VertexPrecision x,
		      VertexPrecision y,
		      VertexPrecision z)
{
  m_normals.push_back(x);
  m_normals.push_back(y);
  m_normals.push_back(z);

 return;
} /* AyConvertor::normalcb */


void
AyConvertor::convert(ay_pomesh_object *pomesh)
{
 unsigned int i = 0, j = 0, k = 0, a = 0;

  if(!pomesh)
    return;

  m_flattener->receiveVertices(std::tr1::bind(&AyConvertor::vertexcb,
					      this, _1, _2, _3, _4));
  m_flattener->receiveSmoothNormals(std::tr1::bind(&AyConvertor::normalcb,
						   this, _1, _2, _3));
  m_flattener->flattenFaces();

  if(m_numFaces == 0)
    return;

  pomesh->npolys = m_numFaces;
  pomesh->nloops = (unsigned int *)calloc(m_numFaces, sizeof(unsigned int));
  for(i = 0; i < m_numFaces; i++)
    {
      pomesh->nloops[i] = 1;
    }

  pomesh->nverts = (unsigned int *)calloc(m_numFaces, sizeof(unsigned int));
  for(i = 0; i < m_numFaces; i++)
    {
      pomesh->nverts[i] = 4;
    }

  pomesh->verts = (unsigned int *)calloc(m_numFaces, 4*sizeof(unsigned int));
  j = 0;
  for(i = 0; i < m_numFaces*4; i++)
    {
      pomesh->verts[i] = j;
      j++;
    }

  pomesh->ncontrols = m_numVertices;
  pomesh->has_normals = AY_TRUE;
  pomesh->controlv = (double *)calloc(m_numVertices, 6*sizeof(double));
  j = 0; k = 0;
  for(i = 0; i < m_numVertices; i++)
    {

      pomesh->controlv[a] = m_vertices.at(k);
      pomesh->controlv[a+1] = m_vertices.at(k+1);
      pomesh->controlv[a+2] = m_vertices.at(k+2);

      pomesh->controlv[a+3] = m_normals.at(j);
      pomesh->controlv[a+4] = m_normals.at(j+1);
      pomesh->controlv[a+5] = m_normals.at(j+2);

      a += 6;
      j += 3;
      k += 4;
    }

 return;
} /* AyConvertor::convert */


/* sdnpatch_createcb:
 *  create callback function of sdnpatch object
 */
int
sdnpatch_createcb(int argc, char *argv[], ay_object *o)
{
 sdnpatch_object *sdnpatch = NULL;
 char fname[] = "crtsdnpatch";
 std::vector<Vertex*>::iterator it;
 Vertex *v;
 unsigned int a = 0;

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
  meshBuilder->addToFace(4);
  meshBuilder->addToFace(5);
  meshBuilder->addToFace(1);
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

  sdnpatch->controlVertices = sdnpatch->controlMesh->getVertexPointers();

  if(!(sdnpatch->controlCoords = (double*)calloc(
			sdnpatch->controlVertices->size(), 4*sizeof(double))))
    return AY_EOMEM;

  for(it = (*sdnpatch->controlVertices).begin();
      it != (*sdnpatch->controlVertices).end();
      it++)
    {
      v = *it;
      sdnpatch->controlCoords[a]   = v->getX();
      sdnpatch->controlCoords[a+1] = v->getY();
      sdnpatch->controlCoords[a+2] = v->getZ();
      sdnpatch->controlCoords[a+3] = v->getW();
      a += 4;
    }

  o->refine = sdnpatch;

  ay_notify_force(o);

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

  if(sdnpatch->controlVertices)
    delete sdnpatch->controlVertices;

  if(sdnpatch->controlCoords)
    free(sdnpatch->controlCoords);

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
 std::vector<Vertex*>::iterator it;
 Vertex *v;
 unsigned int a = 0;

  if(!src || !dst)
    return AY_ENULL;

  if(!(sdnpatch = (sdnpatch_object*)calloc(1, sizeof(sdnpatch_object))))
    return AY_EOMEM;

  memcpy(sdnpatch, src, sizeof(sdnpatch_object));

  sdnpatch->controlMesh = NULL;
  sdnpatch->subdivMesh = NULL;
  sdnpatch->controlVertices = NULL;
  sdnpatch->controlCoords = NULL;

  srcsdnpatch = (sdnpatch_object*)src;
  if(srcsdnpatch->controlMesh)
    sdnpatch->controlMesh = new Mesh(*(srcsdnpatch->controlMesh));


  sdnpatch->controlVertices = sdnpatch->controlMesh->getVertexPointers();

  if(!(sdnpatch->controlCoords = (double*)calloc(
			 sdnpatch->controlVertices->size(), 4*sizeof(double))))
    return AY_EOMEM;

  for(it = (*sdnpatch->controlVertices).begin();
      it != (*sdnpatch->controlVertices).end();
      it++)
    {
      v = *it;
      sdnpatch->controlCoords[a]   = v->getX();
      sdnpatch->controlCoords[a+1] = v->getY();
      sdnpatch->controlCoords[a+2] = v->getZ();
      sdnpatch->controlCoords[a+3] = v->getW();
      a += 4;
    }

  if(srcsdnpatch->subdivMesh)
    sdnpatch->subdivMesh = new Mesh(*(srcsdnpatch->subdivMesh));

  *dst = (void *)sdnpatch;

 return AY_OK;
} /* sdnpatch_copycb */


void
sdnpatch_lineloopcb(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
 static int calls = 0;

  glVertex3d(x, y, z);

  if(calls == 3)
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

  glVertex3d(x, y, z);

  if(calls == 3)
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

  if(!sdnpatch->subdivMesh)
    return AY_OK;

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
 std::vector<Vertex*> *vertices = NULL;
 unsigned int i = 0, a = 0;

  if(!o)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)o->refine;

  if(!sdnpatch)
    return AY_ENULL;

  vertices = sdnpatch->controlVertices;

  glBegin(GL_POINTS);
   for(i = 0; i < vertices->size(); i++)
     {
       glVertex3dv(&(sdnpatch->controlCoords[a]));
       a += 4;
     }
  glEnd();

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


  ay_notify_force(o);

  o->modified = AY_TRUE;

  ay_notify_parent();

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

  sdnpatch->controlVertices = sdnpatch->controlMesh->getVertexPointers();

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
// sdnpatch_object *sdnpatch = NULL;
 ay_object *c = NULL;

  if(!o)
    return AY_ENULL;

  //sdnpatch = (sdnpatch_object*)o->refine;

  ay_provide_object(o, AY_IDPOMESH, &c);
  if(!c)
    return AY_ERROR;

  ay_wrib_object(file, c);

  ay_object_deletemulti(c);

 return AY_OK;
} /* sdnpatch_wribcb */


/* sdnpatch_bbccb:
 *  bounding box calculation callback function of sdnpatch object
 */
int
sdnpatch_bbccb(ay_object *o, double *bbox, int *flags)
{
 double xmin, xmax, ymin, ymax, zmin, zmax;
 sdnpatch_object *sdnpatch = NULL;
 std::vector<Vertex*> *vertices = NULL;
 std::vector<Vertex*>::iterator it;
 Vertex *v;

  if(!o || !bbox)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)o->refine;

  if(!sdnpatch || !sdnpatch->controlVertices)
    return AY_ENULL;

  vertices = sdnpatch->controlVertices;
  it = (*vertices).begin();
  v = *it;
  xmin = v->getX();
  xmax = xmin;
  ymin = v->getY();
  ymax = ymin;
  zmin = v->getZ();
  zmax = zmin;
  it++;
  for(; it != (*vertices).end(); it++)
    {
      v = *it;
      if(v->getX() < xmin)
	xmin = v->getX();
      if(v->getX() > xmax)
	xmax = v->getX();

      if(v->getY() < ymin)
	ymin = v->getY();
      if(v->getY() > ymax)
	ymax = v->getY();

      if(v->getZ() < zmin)
	zmin = v->getZ();
      if(v->getZ() > zmax)
	zmax = v->getZ();
    } /* for */

  /* P1 */
  bbox[0] = xmin; bbox[1] = ymax; bbox[2] = zmax;
  /* P2 */
  bbox[3] = xmin; bbox[4] = ymax; bbox[5] = zmin;
  /* P3 */
  bbox[6] = xmax; bbox[7] = ymax; bbox[8] = zmin;
  /* P4 */
  bbox[9] = xmax; bbox[10] = ymax; bbox[11] = zmax;

  /* P5 */
  bbox[12] = xmin; bbox[13] = ymin; bbox[14] = zmax;
  /* P6 */
  bbox[15] = xmin; bbox[16] = ymin; bbox[17] = zmin;
  /* P7 */
  bbox[18] = xmax; bbox[19] = ymin; bbox[20] = zmin;
  /* P8 */
  bbox[21] = xmax; bbox[22] = ymin; bbox[23] = zmax;

 return AY_OK;
} /* sdnpatch_bbccb */


/* sdnpatch_notifycb:
 *  notification callback function of sdnpatch object
 */
int
sdnpatch_notifycb(ay_object *o)
{
 sdnpatch_object *sdnpatch = NULL;
 ay_point *pnt = NULL;
 Vertex *v = NULL;

 int curlev = 0;

  if(!o)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)(o->refine);


  /* manage potentially modified control points */
  if(o->selp)
    {
      pnt = o->selp;

      while(pnt)
	{
	  v = (*(sdnpatch->controlVertices))[pnt->index];

	  v->setX(pnt->point[0]);
	  v->setY(pnt->point[1]);
	  v->setZ(pnt->point[2]);
	  v->setW(pnt->point[3]);

	  pnt = pnt->next;
	}
    }


  /* update subdivision surface */
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


/* sdnpatch_providecb:
 *  provide callback function of sdnpatch object
 */
int
sdnpatch_providecb(ay_object *o, unsigned int type, ay_object **result)
{
 int ay_status = AY_OK;
 sdnpatch_object *sdnpatch = NULL;
 ay_object *newo = NULL;
 ay_pomesh_object *pomesh = NULL;
 MeshFlattener *meshFlattener = NULL;
 AyConvertor *convertor = NULL;

  if(!o)
    return AY_ENULL;

  if(!result)
    {
      if(type == AY_IDPOMESH)
	return AY_OK;
      else
	return AY_ERROR;
    }

  sdnpatch = (sdnpatch_object *) o->refine;

  if(type == AY_IDPOMESH)
    {

      if(sdnpatch->subdivMesh)
	{
	  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
	    return AY_EOMEM;
	  if(!(pomesh = (ay_pomesh_object*)calloc(1,
						  sizeof(ay_pomesh_object))))
	    {free(newo); return AY_EOMEM;}

	  ay_object_defaults(newo);
	  ay_trafo_copy(o, newo);

	  newo->type = AY_IDPOMESH;

	  meshFlattener = MeshFlattener::create(*(sdnpatch->subdivMesh));
	  meshFlattener->setCompatible(true);

	  convertor = new AyConvertor(meshFlattener);

	  convertor->convert(pomesh);

	  newo->refine = pomesh;

	  delete convertor;

	  MeshFlattener::dispose(meshFlattener);

	} /* if */

      *result = newo;
    } /* if */

 return ay_status;
} /* sdnpatch_providecb */


/* sdnpatch_convertcb:
 *  convert callback function of sdnpatch object
 */
int
sdnpatch_convertcb(ay_object *o, int in_place)
{
 int ay_status = AY_OK;
 sdnpatch_object *sdnpatch = NULL;
 ay_object *newo = NULL;
 ay_pomesh_object *pomesh = NULL;
 MeshFlattener *meshFlattener = NULL;
 AyConvertor *convertor = NULL;

  if(!o)
    return AY_ENULL;

  /* first, create new objects */

  sdnpatch = (sdnpatch_object *) o->refine;

  if(sdnpatch->subdivMesh)
    {
      if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
	return AY_EOMEM;
      if(!(pomesh = (ay_pomesh_object*)calloc(1, sizeof(ay_pomesh_object))))
	{free(newo); return AY_EOMEM;}

      ay_object_defaults(newo);
      ay_trafo_copy(o, newo);

      newo->type = AY_IDPOMESH;

      meshFlattener = MeshFlattener::create(*(sdnpatch->subdivMesh));
      meshFlattener->setCompatible(true);

      convertor = new AyConvertor(meshFlattener);

      convertor->convert(pomesh);

      newo->refine = pomesh;

      delete convertor;

      MeshFlattener::dispose(meshFlattener);

    } /* if */

  /* second, link new objects, or replace old objects with them */

  if(newo)
    {
      if(!in_place)
	{
	  ay_status = ay_object_link(newo);
	}
      else
	{
	  ay_object_replace(newo, o);
	} /* if */
    } /* if */

 return ay_status;
} /* sdnpatch_convertcb */


/* sdnpatch_getpntcb:
 *  get point (editing and selection) callback function of sdnpatch object
 */
int
sdnpatch_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 sdnpatch_object *sdnpatch = NULL;
 ay_point *pnt = NULL;
 unsigned int i = 0, a = 0;
 double minDist = ay_prefs.pick_epsilon, curDist = 0.0;
 double **tmpc, *c;
 unsigned int *tmpi;
 std::vector<Vertex*> *vertices = NULL;

  if(!o || !p || !pe)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)(o->refine);

  if(!sdnpatch || !sdnpatch->controlVertices)
    return AY_ENULL;

  vertices = sdnpatch->controlVertices;

  switch(mode)
    {
    case 0:
      /* select all points */
      pe->coords = (double**)calloc(vertices->size(), sizeof(double*));
      pe->indizes = (unsigned int*)calloc(vertices->size(),
					  sizeof(unsigned int));

      for(i = 0; i < vertices->size(); i++)
	{
	  pe->coords[i] = &(sdnpatch->controlCoords[a]);
	  pe->indizes[i] = i;
	  a += 4;
	}

      pe->num = vertices->size();
      break;
    case 1:
      /* select one point */

      if(minDist <= 0.0)
	minDist = DBL_MAX;

      for(i = 0; i < vertices->size(); i++)
	{
	  curDist = AY_VLEN((p[0] - sdnpatch->controlCoords[a]),
			    (p[1] - sdnpatch->controlCoords[a+1]),
			    (p[2] - sdnpatch->controlCoords[a+2]));

	  if(curDist < minDist)
	    {
	      if(!pe->coords)
		pe->coords = (double**)calloc(1, sizeof(double*));
	      if(!pe->indizes)
		pe->indizes = (unsigned int*)calloc(1, sizeof(unsigned int));

	      pe->coords[0] = &(sdnpatch->controlCoords[a]);
	      pe->indizes[0] = i;

	      pe->num = 1;
	      minDist = curDist;
	    } /* if */
	  a += 4;
	} /* for */

      break;
    case 2:
      /* select all points between the planes in p */

      for(i = 0; i < vertices->size(); i++)
	{
	  c = &(sdnpatch->controlCoords[a]);

	  if(((p[0]*c[0] + p[1]*c[1] + p[2]*c[2] + p[3]) < 0.0) &&
	     ((p[4]*c[0] + p[5]*c[1] + p[6]*c[2] + p[7]) < 0.0) &&
	     ((p[8]*c[0] + p[9]*c[1] + p[10]*c[2] + p[11]) < 0.0) &&
	     ((p[12]*c[0] + p[13]*c[1] + p[14]*c[2] + p[15]) < 0.0))
	    {
	      tmpc = NULL;
	      tmpc = (double**)realloc(pe->coords,
			(pe->num+1) * sizeof(double*));
	      if(tmpc)
		pe->coords = tmpc;
	      else
		return AY_EOMEM;

	      pe->coords[pe->num] = c;

	      tmpi = NULL;
	      tmpi = (unsigned int*)realloc(pe->indizes,
			(pe->num+1) * sizeof(unsigned int));
	      if(tmpi)
		pe->indizes = tmpi;
	      else
		return AY_EOMEM;

	      pe->indizes[pe->num] = i;

	      pe->num++;
	    } /* if */

	  a += 4;
	} /* for */

      break;
    case 3:
      /* rebuild from o->selp */

      pnt = o->selp;
      while(pnt)
	{
	  pnt->point = &(sdnpatch->controlCoords[pnt->index*4]);
	  pnt = pnt->next;
	}
      break;
    default:
      break;
    } /* switch */

  pe->homogenous = AY_TRUE;

 return AY_OK;
} /* sdnpatch_getpntcb */

/* sdnpatch_isclosednp:
 *
 */
int
sdnpatch_isclosednp(ay_nurbpatch_object *np, int *closedu, int *closedv)
{
 unsigned int i, a, b;
 int stride = 4;
 double *cv;

  if(!np || !closedu || !closedv)
    return AY_ENULL;

  cv = np->controlv;

  a = 0;
  b = (np->width-1)*np->height*stride;
  *closedu = AY_TRUE;
  for(i = 0; i < np->height; i++)
    {
      if(fabs(cv[a]-cv[b]) > AY_EPSILON ||
	 fabs(cv[a+1]-cv[b+1]) > AY_EPSILON ||
	 fabs(cv[a+2]-cv[b+2]) > AY_EPSILON)
	{
	  *closedu = AY_FALSE;
	  break;
	}
      a += stride;
      b += stride;
    }


  a = 0;
  b = (np->height-1)*stride;
  *closedv = AY_TRUE;
  for(i = 0; i < np->width; i++)
    {
      if(fabs(cv[a]-cv[b]) > AY_EPSILON ||
	 fabs(cv[a+1]-cv[b+1]) > AY_EPSILON ||
	 fabs(cv[a+2]-cv[b+2]) > AY_EPSILON)
	{
	  *closedv = AY_FALSE;
	  break;
	}
      a += (np->height*stride);
      b += (np->height*stride);
    }

 return AY_OK;
} /* sdnpatch_isclosednp */


/* sdnpatch_addfaces:
 * XXXX add dummy faces
 */
void
sdnpatch_addfaces(MeshBuilder *meshBuilder,
		  unsigned int width, unsigned int height)
{
 unsigned int i, j, a, b, d;

  a = 0;
  b = height;

  for(i = 0; i < width-1; i++)
    {

      for(j = 0; j < height-1; j++)
	{
	  meshBuilder->startFace(4);

	  meshBuilder->addToFace(a);
	  meshBuilder->addToFace(a+1);
	  meshBuilder->addToFace(b+1);
	  meshBuilder->addToFace(b);

	  meshBuilder->closeFace();

	  a++;
	  b++;
	} /* for */

      a++;
      b++;
    } /* for */

  /* dummy faces */

 return;
} /* sdnpatch_addfaces */


/* sdnpatch_addfacescu:
 *
 */
void
sdnpatch_addfacescu(MeshBuilder *meshBuilder,
		    unsigned int width, unsigned int height)
{
 unsigned int i, j, a, b, d;

  a = 0;
  b = height;

  for(i = 0; i < width-2; i++)
    {
      for(j = 0; j < height-1; j++)
	{
	  meshBuilder->startFace(4);
	  meshBuilder->addToFace(a);
	  meshBuilder->addToFace(a+1);
	  meshBuilder->addToFace(b+1);
	  meshBuilder->addToFace(b);
	  meshBuilder->closeFace();

	  a++;
	  b++;
	} /* for */

      a++;
      b++;
    } /* for */

  b = 0;
  for(j = 0; j < height-1; j++)
    {
      meshBuilder->startFace(4);
      meshBuilder->addToFace(a);
      meshBuilder->addToFace(a+1);
      meshBuilder->addToFace(b+1);
      meshBuilder->addToFace(b);
      meshBuilder->closeFace();

      a++;
      b++;
    }

  /* dummy faces */
  /* upper */
  a = 0;
  d = (width-1)*height;

  for(i = 0; i < width-2; i++)
    {
      meshBuilder->startFace(4);
      meshBuilder->addToFace(d);
      meshBuilder->addToFace(a);
      a += height;
      meshBuilder->addToFace(a);
      d++;
      meshBuilder->addToFace(d);
      meshBuilder->closeFace();
    }

  meshBuilder->startFace(4);
  meshBuilder->addToFace(d);
  meshBuilder->addToFace(a);
  meshBuilder->addToFace(0);
  meshBuilder->addToFace((width-1)*height);
  meshBuilder->closeFace();
  d++;

  /* lower */
  a = height-1;
  for(i = 0; i < width-2; i++)
    {
      meshBuilder->startFace(4);
      meshBuilder->addToFace(a);
      meshBuilder->addToFace(d);
      d++;
      meshBuilder->addToFace(d);
      a += height;
      meshBuilder->addToFace(a);
      meshBuilder->closeFace();
    }
  meshBuilder->startFace(4);
  meshBuilder->addToFace(a);
  meshBuilder->addToFace(d);
  meshBuilder->addToFace(d-(width-2));
  meshBuilder->addToFace(height-1);
  meshBuilder->closeFace();

 return;
} /* sdnpatch_addfacescu */


/* sdnpatch_addfacescv:
 *
 */
void
sdnpatch_addfacescv(MeshBuilder *meshBuilder,
		    unsigned int width, unsigned int height)
{
 unsigned int i, j, a, b, d;

  a = 0;
  b = height-1;

  for(i = 0; i < width-1; i++)
    {
      for(j = 0; j < height-2; j++)
	{
	  meshBuilder->startFace(4);
	  meshBuilder->addToFace(a);
	  meshBuilder->addToFace(a+1);
	  meshBuilder->addToFace(b+1);
	  meshBuilder->addToFace(b);
	  meshBuilder->closeFace();

	  a++;
	  b++;
	} /* for */

      meshBuilder->startFace(4);
      meshBuilder->addToFace(a);
      meshBuilder->addToFace(a-(height-2));
      meshBuilder->addToFace(b-(height-2));
      meshBuilder->addToFace(b);
      meshBuilder->closeFace();

      a++;
      b++;
    } /* for */

  /* dummy faces */

  /* left side */
  d = width*(height-1);
  a = 0;
  for(j = 0; j < height-2; j++)
    {
      meshBuilder->startFace(4);
      meshBuilder->addToFace(d);
      meshBuilder->addToFace(d+1);
      meshBuilder->addToFace(a+1);
      meshBuilder->addToFace(a);
      meshBuilder->closeFace();
      d++;
      a++;
    }

  meshBuilder->startFace(4);
  meshBuilder->addToFace(d);
  meshBuilder->addToFace(d-(height-2));
  meshBuilder->addToFace(a-(height-2));
  meshBuilder->addToFace(a);
  meshBuilder->closeFace();

  /* right side */
  a = (width-1)*(height-1);
  d++;
  for(j = 0; j < height-2; j++)
    {
      meshBuilder->startFace(4);
      meshBuilder->addToFace(a);
      meshBuilder->addToFace(a+1);
      meshBuilder->addToFace(d+1);
      meshBuilder->addToFace(d);
      meshBuilder->closeFace();
      d++;
      a++;
    }

  meshBuilder->startFace(4);
  meshBuilder->addToFace(a);
  meshBuilder->addToFace(a-(height-2));
  meshBuilder->addToFace(d-(height-2));
  meshBuilder->addToFace(d);
  meshBuilder->closeFace();

 return;
} /* sdnpatch_addfacescv */


/* sdnpatch_addfacescuv:
 *
 */
void
sdnpatch_addfacescuv(MeshBuilder *meshBuilder,
		    unsigned int width, unsigned int height)
{
 unsigned int i, j, a, b;

  a = 0;
  b = height;

  for(i = 0; i < width-2; i++)
    {
      for(j = 0; j < height-1; j++)
	{
	  meshBuilder->startFace(4);

	  meshBuilder->addToFace(a);
	  meshBuilder->addToFace(a+1);
	  meshBuilder->addToFace(b+1);
	  meshBuilder->addToFace(b);

	  meshBuilder->closeFace();

	  a++;
	  b++;
	} /* for */

      a++;
      b++;
    } /* for */

  b = 0;
  for(j = 0; j < height-1; j++)
    {
      meshBuilder->startFace(4);

      meshBuilder->addToFace(a);
      meshBuilder->addToFace(a+1);
      meshBuilder->addToFace(b+1);
      meshBuilder->addToFace(b);

      meshBuilder->closeFace();

      a++;
      b++;
    }

  /* no dummy faces needed */

 return;
} /* sdnpatch_addfacescuv */


/* sdnpatch_convnp:
 *  convert NURBS patches to SDNPatch objects
 */
int
sdnpatch_convnp(int mode, ay_object *p, ay_object **result)
{
 ay_object *newo = NULL;
 ay_nurbpatch_object *np = NULL;
 sdnpatch_object *sdnpatch = NULL;
 double *cv = NULL;
 unsigned int i = 0, endi = 0, j = 0, endj = 0, a = 0;
 int is_closed_u = AY_FALSE, is_closed_v = AY_FALSE;
 std::vector<Vertex*>::iterator it;
 Vertex *v;

  if(!p || !result)
    return AY_ENULL;

  if(p->type != AY_IDNPATCH)
    return AY_ERROR;

  np = (ay_nurbpatch_object *)p->refine;

  sdnpatch_isclosednp(np, &is_closed_u, &is_closed_v);

  if(!(newo = (ay_object*)calloc(1, sizeof(ay_object))))
    {
      return AY_EOMEM;
    }

  ay_object_defaults(newo);
  newo->type = sdnpatch_id;
  ay_trafo_copy(p, newo);

  if(!(sdnpatch = (sdnpatch_object*)calloc(1, sizeof(sdnpatch_object))))
    {
      free(newo);
      return AY_EOMEM;
    }

  newo->refine = sdnpatch;

  /**/
  sdnpatch->subdivDegree = 3;
  sdnpatch->subdivLevel = 2;

  sdnpatch->controlMesh = new Mesh(sdnpatch->subdivDegree);

  MeshBuilder *meshBuilder = MeshBuilder::create(*(sdnpatch->controlMesh));

  endi = (is_closed_u?np->width-1:np->width);
  endj = (is_closed_v?np->height-1:np->height);
  cv = np->controlv;
  for(i = 0; i < endi; i++)
    {
      a = (i*np->height)*4;
      for(j = 0; j < endj; j++)
	{
	  meshBuilder->addVertex(cv[a],cv[a+1],cv[a+2],cv[a+3]);
	  a += 4;
	}
    }
  meshBuilder->finishVertices();

  if(is_closed_u)
    {
      if(is_closed_v)
	{
	  sdnpatch_addfacescuv(meshBuilder, np->width, np->height);
	}
      else
	{
	  sdnpatch_addfacescu(meshBuilder, np->width, np->height);
	}
    }
  else
    {
      if(is_closed_v)
	{
	  sdnpatch_addfacescv(meshBuilder, np->width, np->height);
	}
      else
	{
	  sdnpatch_addfaces(meshBuilder, np->width, np->height);
	}
    }

  meshBuilder->finishFaces();

  meshBuilder->finishKnotIntervals();

  MeshBuilder::dispose(meshBuilder);

  sdnpatch->controlVertices = sdnpatch->controlMesh->getVertexPointers();

  if(!(sdnpatch->controlCoords = (double*)calloc(
			sdnpatch->controlVertices->size(), 4*sizeof(double))))
    return AY_EOMEM;

  a = 0;
  for(it = (*sdnpatch->controlVertices).begin();
      it != (*sdnpatch->controlVertices).end();
      it++)
    {
      v = *it;
      sdnpatch->controlCoords[a]   = v->getX();
      sdnpatch->controlCoords[a+1] = v->getY();
      sdnpatch->controlCoords[a+2] = v->getZ();
      sdnpatch->controlCoords[a+3] = v->getW();
      a += 4;
    }

  newo->modified = AY_TRUE;
  ay_notify_force(newo);

  /* return result */
  *result = newo;

 return AY_OK;
} /* sdnpatch_convnp */


/* sdnpatch_convnptcmd:
 *  Tcl command to convert NURBS patches to SDNPatch objects
 */
int
sdnpatch_convnptcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
 char fname[] = "sdnconvNP";
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL, *p = NULL, *newo = NULL;
 int i = 0;

  /* parse args */
  if(argc > 2)
    {
      while(i+1 < argc)
	{
	  if(!strcmp(argv[i], "-r"))
	    {
	      /*
	      mode = 0;
	      sscanf(argv[i+1], "%lg", &rmin);
	      sscanf(argv[i+2], "%lg", &rmax);
	      */
	    }
	  if(!strcmp(argv[i], "-d"))
	    {
	      /*
	      mode = 1;
	      sscanf(argv[i+1], "%lg", &mindist);
	      */
	    }
	  i += 2;
	} /* while */
    } /* if */

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, fname, NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;

      if(o->type != AY_IDNPATCH)
	{
	  ay_status = ay_provide_object(o, AY_IDNPATCH, &p);
	}
      else
	{
	  ay_status = ay_object_copy(o, &p);
	} /* if */

      if(p)
	{
	  ay_status = sdnpatch_convnp(0, p, &newo);
	  if(newo)
	    {
	      ay_object_link(newo);
	    }

	  ay_object_deletemulti(p);
	}

      sel = sel->next;
    } /* while */

  ay_notify_parent();

 return TCL_OK;
} /* sdnpatch_convnptcmd */


extern "C" {

/* Sdnpatch_Init:
 * initializes the sdnpatch module/plugin by registering a new
 * object type (SDNPatch) and loading the accompanying Tcl script file.
 * Note: This function _must_ be capitalized exactly this way
 * (_S_dnpatch__I_nit, not sdnpatch_init!)
 * regardless of the filename of the shared object (see: man 3tcl load)!
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
				sdnpatch_getpntcb,
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

  ay_status = ay_convert_register(sdnpatch_convertcb, sdnpatch_id);

  ay_status = ay_provide_register(sdnpatch_providecb, sdnpatch_id);


  // create new Tcl commands to interface with the plugin
  Tcl_CreateCommand(interp, "sdnconvertNP", (Tcl_CmdProc*) sdnpatch_convnptcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);


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
