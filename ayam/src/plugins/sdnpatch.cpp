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

/** \file sdnpatch.cpp SDNPatch custom object (Subdivision NURBS) */

#include "snurbs.h"
#include "knotinterval.h"

#include "ayam.h"

using namespace std;
using namespace std::tr1::placeholders;
using namespace snurbs;

static char sdnpatch_version_ma[] = AY_VERSIONSTR;
static char sdnpatch_version_mi[] = AY_VERSIONSTRMI;

static char *sdnpatch_name = "SDNPatch";

static unsigned int sdnpatch_id;

typedef struct sdnpatch_object_s
{
  unsigned char subdivLevel;
  unsigned char subdivDegree;

  Mesh *controlMesh;
  /*
  GLuint controlWireList;
  GLuint controlFaceList;
  */
  Mesh *subdivMesh;
  /*
  GLuint subdivObject;
  */
  vector<Vertex*> *controlVertices;
  double *controlCoords;

} sdnpatch_object;

/* prototypes of functions local to this module: */

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

int sdnpatch_impplytcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

int sdnpatch_expplytcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

int sdnpatch_extrudefacetcmd(ClientData clientData, Tcl_Interp *interp,
			     int argc, char *argv[]);

int sdnpatch_mergefacetcmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);

int sdnpatch_removefacetcmd(ClientData clientData, Tcl_Interp *interp,
			    int argc, char *argv[]);

int sdnpatch_reverttcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[]);

int sdnpatch_mergepatchtcmd(ClientData clientData, Tcl_Interp *interp,
			    int argc, char *argv[]);

int sdnpatch_getcontrolvertices(sdnpatch_object *sdnpatch);

/* assorted helper classes: */

/**
 * Revertor: Helper class to revert all edges of a sdnpatch
 *
 * ToDo: add support for texture coordinates
 */
class Revertor : public FlatMeshHandler
{

public:

  Revertor(sdnpatch_object *sdnpatch);
  ~Revertor();
  void addVertex(VertexPrecision x,
		 VertexPrecision y,
		 VertexPrecision z,
		 VertexPrecision w,
		 unsigned int id);
  void finishVertices(void);
  void startFace(unsigned int numEdges);
  void addToFace(unsigned int vertNum);
  /*
  void addTexCoords(KnotPrecision u,
		    KnotPrecision v);
  */
  void closeFace(void);
  void finishFaces(void);
  void addKnotInterval(unsigned int vertex1,
		       unsigned int vertex2,
		       KnotPrecision interval);
  void finishKnotIntervals(void);

  Mesh *m_newMesh;

private:

  MeshBuilder *m_meshBuilder;
  sdnpatch_object *m_sdnpatch;
  unsigned int m_id;
  vector<unsigned int> m_vertices;
};


Revertor::Revertor(sdnpatch_object *sdnpatch)
{
  m_sdnpatch = sdnpatch;
  m_newMesh = new Mesh(m_sdnpatch->subdivDegree);
  m_meshBuilder = MeshBuilder::create(*m_newMesh);
  m_id = 0;
} /* Revertor::Revertor */


Revertor::~Revertor()
{
  MeshBuilder::dispose(m_meshBuilder);
} /* FaceConnector::~FaceConnector */


void
Revertor::addVertex(VertexPrecision x,
		    VertexPrecision y,
		    VertexPrecision z,
		    VertexPrecision w,
		    unsigned int id)
{
  m_meshBuilder->addVertex(x,y,z,w,m_id);
  m_id++;
 return;
} /* Revertor::addVertex */


void
Revertor::finishVertices(void)
{
  m_meshBuilder->finishVertices();
 return;
} /* Revertor::finishVertices */


void
Revertor::startFace(unsigned int numEdges)
{
  m_meshBuilder->startFace(numEdges);
  m_vertices.reserve(numEdges);
 return;
} /* Revertor::startFace */


void
Revertor::addToFace(unsigned int vertNum)
{
  m_vertices.push_back(vertNum);
 return;
} /* Revertor::addToFace */

#if 0
void
Revertor::addTexCoords(KnotPrecision u,
		       KnotPrecision v)
{
 return;
} /* Revertor::addTexCoords */
#endif


void
Revertor::closeFace(void)
{
  unsigned int i;

  for(i = m_vertices.size(); i > 0; i--)
    {
      m_meshBuilder->addToFace(m_vertices[i-1]);
    }
  m_vertices.clear();

  m_meshBuilder->closeFace();
 return;
} /* Revertor::closeFace */


void
Revertor::finishFaces(void)
{
  m_meshBuilder->finishFaces();
 return;
} /* Revertor::finishFaces */


void
Revertor::addKnotInterval(unsigned int vertex1,
			  unsigned int vertex2,
			  KnotPrecision interval)
{
  m_meshBuilder->addKnotInterval(vertex2, vertex1, interval);
 return;
} /* Revertor::addKnotInterval */


void
Revertor::finishKnotIntervals(void)
{
  m_meshBuilder->finishKnotIntervals();
 return;
} /* Revertor::finishKnotIntervals */


/**
 * PatchMerger: Helper class to merge multiple sdnpatches
 *
 * In contrast to the other helper classes, where the complete
 * operation runs in the flattening stage, the PatchMerger must
 * be used like this:
 * patchMerger->addPatch();
 * meshFlattener->flatten(&patchMerger);
 * patchMerger->addPatch();
 * meshFlattener->flatten(&patchMerger);
 * newMesh = patchMerger->buildMesh();
 *
 * ToDo: add support for texture coordinates
 */
class PatchMerger : public FlatMeshHandler
{

public:

  PatchMerger(void);

  void addPatch(void);
  void addVertex(VertexPrecision x,
		 VertexPrecision y,
		 VertexPrecision z,
		 VertexPrecision w,
		 unsigned int id);
  void finishVertices(void);
  void startFace(unsigned int numEdges);
  void addToFace(unsigned int vertNum);
  /*
  void addTexCoords(KnotPrecision u,
		    KnotPrecision v);
  */
  void closeFace(void);
  void finishFaces(void);
  void addKnotInterval(unsigned int vertex1,
		       unsigned int vertex2,
		       KnotPrecision interval);
  void finishKnotIntervals(void);
  Mesh *buildMesh(unsigned int degree);

private:

  unsigned int m_offset;
  unsigned int m_dummyoffset;
  unsigned int m_id;
  vector<VertexPrecision> m_newVerts;
  vector<unsigned int> m_newVertIDs;
  unsigned int m_curVertsNum;
  unsigned int m_maxDummyIndex;
  unsigned int m_numEdges;
  bool m_faceIsDummy;
  vector<unsigned int> m_newFace;
  unsigned int m_newVertsNum;
  unsigned int m_newFacesNum;

  vector<unsigned int> m_newFaces;
  unsigned int m_newDummyFacesNum;
  vector<unsigned int> m_newDummyFaces;
  unsigned int m_newKnotIntervalsNum;
  vector<unsigned int> m_newKnotIndices;
  vector<KnotPrecision> m_newKnotIntervals;
  unsigned int m_newDummyKnotIntervalsNum;
  vector<unsigned int> m_newDummyKnotIndices;
  vector<KnotPrecision> m_newDummyKnotIntervals;
};


PatchMerger::PatchMerger()
{
  m_offset = 0;
  m_dummyoffset = 0;
  m_id = 0;
  m_curVertsNum = 0;
  m_newVertsNum = 0;
  m_newFacesNum = 0;
  m_newKnotIntervalsNum = 0;
  m_newDummyFacesNum = 0;
  m_newDummyKnotIntervalsNum = 0;
} /* PatchMerger::PatchMerger */


void
PatchMerger::addPatch()
{
  m_curVertsNum = 0;
  m_maxDummyIndex = 0;
 return;
} /* PatchMerger::addPatch */


void
PatchMerger::addVertex(VertexPrecision x,
		       VertexPrecision y,
		       VertexPrecision z,
		       VertexPrecision w,
		       unsigned int id)
{
  m_newVerts.push_back(x);
  m_newVerts.push_back(y);
  m_newVerts.push_back(z);
  m_newVerts.push_back(w);
  m_newVertIDs.push_back(m_id);
  m_id++;
  m_newVertsNum++;
  m_curVertsNum++;
 return;
} /* PatchMerger::addVertex */


void
PatchMerger::finishVertices(void)
{
 return;
} /* PatchMerger::finishVertices */


void
PatchMerger::startFace(unsigned int numEdges)
{
  m_numEdges = numEdges;
  m_faceIsDummy = false;
 return;
} /* PatchMerger::startFace */


void
PatchMerger::addToFace(unsigned int vertNum)
{
  if(vertNum >= m_curVertsNum)
    {
      m_faceIsDummy = true;
    }
  m_newFace.push_back(vertNum);
 return;
} /* PatchMerger::addToFace */

#if 0
void
PatchMerger::addTexCoords(KnotPrecision u,
			  KnotPrecision v)
{
 return;
} /* PatchMerger::addTexCoords */
#endif

void
PatchMerger::closeFace(void)
{
 unsigned int i;

  if(!m_faceIsDummy)
    {
      // process real face
      m_newFaces.push_back(m_numEdges);
      for(i = 0; i < m_numEdges; i++)
	{
	  m_newFaces.push_back(m_newFace[i] + m_offset);
	}
      m_newFacesNum++;
    }
  else
    {
      m_newDummyFaces.push_back(m_numEdges);
      for(i = 0; i < m_numEdges; i++)
	{
	  // is this vertex dummy?
	  if(m_newFace[i] < m_curVertsNum)
	    {
	      // no, process real vertex in dummy face
	      m_newDummyFaces.push_back(0);
	      m_newDummyFaces.push_back(m_newFace[i] + m_offset);
	    }
	  else
	    {
	      // yes, process dummy vertex
	      m_newDummyFaces.push_back(1);
	      // - m_curVertsNum shifts the indices to the (0-x)
	      // range and
	      // + m_dummyoffset accounts for dummy vertices from
	      // multiple patches (like m_offset);
	      // the (now zero based) indices will be offset again in
	      // buildMesh() to get behind all real vertices, we can not
	      // do it here since we do not know how many real vertices
	      // we will get in total yet
	      m_newDummyFaces.push_back(m_newFace[i] - m_curVertsNum +
					m_dummyoffset);
	      // calculate maximum dummy index; this is used to count
	      // the number of dummy vertices in this patch; and this is
	      // then used to calculate the m_dummyoffset for the next
	      // patch(es)
	      if(m_newFace[i] > m_maxDummyIndex)
		m_maxDummyIndex = m_newFace[i];
	    } // if
	} // for
      m_newDummyFacesNum++;
    } // if
  m_newFace.clear();
 return;
} /* PatchMerger::closeFace */


void
PatchMerger::finishFaces(void)
{
 return;
} /* PatchMerger::finishFaces */


void
PatchMerger::addKnotInterval(unsigned int vertex1,
			     unsigned int vertex2,
			     KnotPrecision interval)
{
  if((vertex1 < m_curVertsNum) && (vertex2 < m_curVertsNum))
    {
      // process real knot
      m_newKnotIndices.push_back(vertex1 + m_offset);
      m_newKnotIndices.push_back(vertex2 + m_offset);
      m_newKnotIntervals.push_back(interval);
      m_newKnotIntervalsNum++;
    }
  else
    {
      // process dummy knot
      // using the same offsetting scheme as for dummy faces above
      if(vertex1 < m_curVertsNum)
	{
	  m_newDummyKnotIndices.push_back(0);
	  m_newDummyKnotIndices.push_back(vertex1 + m_offset);
	}
      else
	{
	  m_newDummyKnotIndices.push_back(1);
	  m_newDummyKnotIndices.push_back(vertex1 - m_curVertsNum +
					  m_dummyoffset);
	}
      if(vertex2 < m_curVertsNum)
	{
	  m_newDummyKnotIndices.push_back(0);
	  m_newDummyKnotIndices.push_back(vertex2 + m_offset);
	}
      else
	{
	  m_newDummyKnotIndices.push_back(1);
	  m_newDummyKnotIndices.push_back(vertex2 - m_curVertsNum +
					  m_dummyoffset);
	}
      m_newDummyKnotIntervals.push_back(interval);
      m_newDummyKnotIntervalsNum++;
    } // if
 return;
} /* PatchMerger::addKnotInterval */


void
PatchMerger::finishKnotIntervals(void)
{
  // calculate offsets for indices of the next patch(es)
  m_offset += m_curVertsNum;
  m_dummyoffset += (m_maxDummyIndex + 1 - m_curVertsNum);
 return;
} /* PatchMerger::finishKnotIntervals */


Mesh *
PatchMerger::buildMesh(unsigned int degree)
{
 MeshBuilder *meshBuilder = NULL;
 Mesh *newMesh = NULL;
 vector<unsigned int>::iterator fi;
 unsigned int numVerts;
 unsigned int i, j, k, v1, v2;

  newMesh = new Mesh(degree);
  meshBuilder = MeshBuilder::create(*newMesh);

  j = 0;
  k = 0;
  for(i = 0; i < m_newVertsNum; i++)
    {
      meshBuilder->addVertex(m_newVerts[j],
			     m_newVerts[j+1],
			     m_newVerts[j+2],
			     m_newVerts[j+3],
			     m_newVertIDs[k]);
      j += 4;
      k++;
    }

  meshBuilder->finishVertices();

  k = 0;
  for(i = 0; i < m_newFacesNum; i++)
    {
      numVerts = m_newFaces[k];
      meshBuilder->startFace(numVerts);
      k++;
      for(j = 0; j < numVerts; j++)
	{
	  meshBuilder->addToFace(m_newFaces[k]);
	  k++;
	}

      meshBuilder->closeFace();
    }

  k = 0;
  for(i = 0; i < m_newDummyFacesNum; i++)
    {
      numVerts = m_newDummyFaces[k];
      meshBuilder->startFace(numVerts);
      k++;
      for(j = 0; j < numVerts; j++)
	{
	  if(m_newDummyFaces[k] == 0)
	    {
	      meshBuilder->addToFace(m_newDummyFaces[k+1]);
	    }
	  else
	    {
	      // + m_newVertsNum is the said second offset to shift
	      // all dummy vertices behind all real vertices
	      // (see closeFace() above)
	      meshBuilder->addToFace(m_newDummyFaces[k+1] + m_newVertsNum);
	    }
	  k += 2;
	}
      meshBuilder->closeFace();
    }

  meshBuilder->finishFaces();

  if(m_newKnotIntervalsNum > 0)
    {
      j = 0;
      for(i = 0; i < m_newKnotIntervalsNum; i++)
	{
	  meshBuilder->addKnotInterval(m_newKnotIndices[j],
				       m_newKnotIndices[j+1],
				       m_newKnotIntervals[i]);

	  j += 2;
	}
    }

  if(m_newDummyKnotIntervalsNum > 0)
    {
      j = 0;
      for(i = 0; i < m_newDummyKnotIntervalsNum; i++)
	{
	  if(m_newDummyKnotIndices[j] == 0)
	    {
	      v1 = m_newDummyKnotIndices[j+1];
	    }
	  else
	    {
	      v1 = m_newDummyKnotIndices[j+1] + m_newVertsNum;
	    }
	  if(m_newDummyKnotIndices[j+2] == 0)
	    {
	      v2 = m_newDummyKnotIndices[j+3];
	    }
	  else
	    {
	      v2 = m_newDummyKnotIndices[j+3] + m_newVertsNum;
	    }

	  meshBuilder->addKnotInterval(v1, v2, m_newDummyKnotIntervals[i]);

	  j += 4;
	} // for
    } // if

  meshBuilder->finishKnotIntervals();

  MeshBuilder::dispose(meshBuilder);

 return newMesh;
} /* PatchMerger::buildMesh */


/**
 * AyWriter: Helper class to write a mesh to an Ayam scene file
 *
 * ToDo: add support for texture coordinates
 */
class AyWriter : public FlatMeshHandler
{

public:

  AyWriter(FILE *filep);

  void addVertex(VertexPrecision x,
		 VertexPrecision y,
		 VertexPrecision z,
		 VertexPrecision w,
		 unsigned int id);
  void startFace(unsigned int numEdges);
  void addToFace(unsigned int vertNum);
  /*
  void addTexCoords(KnotPrecision u,
		    KnotPrecision v);
  */
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
  /*
  unsigned int m_numTexCoords;
  vector<KnotPrecision> m_texcoords;
  */
};


AyWriter::AyWriter(FILE *filep)
{
  m_fp = filep;
  m_numVertices = 0;
  m_numFaces = 0;
  m_numKnots = 0;
  /*
  m_numTexCoords = 0;
  */
} /* AyWriter::AyWriter */


void
AyWriter::addVertex(VertexPrecision x,
		    VertexPrecision y,
		    VertexPrecision z,
		    VertexPrecision w,
		    unsigned int id)
{
  m_numVertices++;
  m_vertices.push_back(x);
  m_vertices.push_back(y);
  m_vertices.push_back(z);
  m_vertices.push_back(w);
 return;
} /* AyWriter::addVertex */


void
AyWriter::startFace(unsigned int numEdges)
{
  m_numEdges.push_back(numEdges);
 return;
} /* AyWriter::startFace */


void
AyWriter::addToFace(unsigned int vertNum)
{
  m_faces.push_back(vertNum);
 return;
} /* AyWriter::addToFace */

#if 0
void
AyWriter::addTexCoords(KnotPrecision u,
		       KnotPrecision v)
{
  m_texcoords.push_back(u);
  m_texcoords.push_back(v);
  m_numTexCoords++;
  // XXXX remember current vertex index?
 return;
} /* AyWriter::addTexCoords */
#endif

void
AyWriter::closeFace(void)
{
  m_numFaces++;
 return;
} /* AyWriter::closeFace */

void
AyWriter::addKnotInterval(unsigned int vertex1,
			  unsigned int vertex2,
			  KnotPrecision interval)
{
  m_v1.push_back(vertex1);
  m_v2.push_back(vertex2);
  m_intervals.push_back(interval);
  m_numKnots++;
 return;
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
      fprintf(m_fp, "%u\n", m_numFaces);

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


/**
 * AyConvertor: helper class to convert a Mesh to
 * a Ayam PolyMesh object
 *
 * ToDo: add support for texture coordinates
 */
class AyConvertor
{

public:

  AyConvertor(MeshFlattener *flattener);

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


AyConvertor::AyConvertor(MeshFlattener *flattener)
{
  if(!flattener)
    return;
  m_flattener = flattener;
  m_numVertices = 0;
  m_numFaces = 0;
};


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


/**
 * FaceExtruder: Helper class to extrude a face from a mesh
 *
 * ToDo: add support for texture coordinates
 */
class FaceExtruder : public FlatMeshHandler
{
public:
  FaceExtruder(sdnpatch_object *sdnpatch, ay_point *pnts,
	       double length, double scale);

  void addVertex(VertexPrecision x,
		 VertexPrecision y,
		 VertexPrecision z,
		 VertexPrecision w,
		 unsigned int id);
  void addToFace(unsigned int vertNum);
  void closeFace(void);
  void addKnotInterval(unsigned int vertex1,
		       unsigned int vertex2,
		       KnotPrecision interval);
  void finishKnotIntervals(void);

  Mesh *m_newMesh;

  // collect new vertex selection
  vector<unsigned int> m_newSelection;

private:

  sdnpatch_object *m_sdnpatch;
  ay_point *m_pnts;

  double m_length;
  double m_scale;

  unsigned int m_oldVertsNum;
  unsigned int m_newVertsNum;

  vector<unsigned int> m_faceVerts;

  // collect new vertices
  vector<VertexPrecision> m_newVerts;
  vector<unsigned int> m_newVertexIDs;

  // collect new faces
  unsigned int m_newFacesNum;
  vector<unsigned int> m_newFaces;

  // collect new dummy faces (we collect them separately, because their
  // vertice indices need to be increased by the number of extra
  // vertices created by the face extrusion
  unsigned int m_newDummyFacesNum;
  vector<unsigned int> m_newDummyFaces;

  // collect knot intervals
  unsigned int m_newKnotIntervalsNum;
  vector<unsigned int> m_newKnotIntervals;
  vector<KnotPrecision> m_newKnots;
};


FaceExtruder::FaceExtruder(sdnpatch_object *sdnpatch, ay_point *pnts,
			   double length, double scale)
{
  m_sdnpatch = sdnpatch;
  m_pnts = pnts;

  m_length = length;
  m_scale = scale;

  m_oldVertsNum = sdnpatch->controlVertices->size();
  m_newVertsNum = 0;
  m_newFacesNum = 0;
  m_newDummyFacesNum = 0;
  m_newKnotIntervalsNum = 0;

  /*
    XXXX ToDo: reserve memory for various vectors
  */

} /* FaceExtruder::FaceExtruder */


void
FaceExtruder::addVertex(VertexPrecision x,
			VertexPrecision y,
			VertexPrecision z,
			VertexPrecision w,
			unsigned int id)
{
  m_newVerts.push_back(x);
  m_newVerts.push_back(y);
  m_newVerts.push_back(z);
  m_newVerts.push_back(w);
  m_newVertexIDs.push_back(id);
 return;
} /* FaceExtruder::addVertex */


void
FaceExtruder::addToFace(unsigned int vertNum)
{
  m_faceVerts.push_back(vertNum);
 return;
} /* FaceExtruder::addToFace */


void
FaceExtruder::closeFace(void)
{
 vector<unsigned int>::iterator fi;
 bool found = false, isSelected = true, isDummy = false;
 unsigned int i = 0, j = 0;
 ay_point *pnt = NULL;
 double *cv = NULL, *p = NULL, N[3] = {1.0,0.0,0.0};
 vector<unsigned int> newVerts;
 vector<unsigned int>::iterator nfi;

  /* is this face selected? */
  fi = m_faceVerts.begin();
  for(i = 0; i < m_faceVerts.size(); i++)
    {
      // rule out dummy faces
      if(*fi >= m_newVertexIDs.size())
	{
	  isSelected = false;
	  break;
	}

      pnt = m_pnts;
      found = false;
      while(pnt)
	{
	  if(pnt->index == m_newVertexIDs[*fi])
	    {
	      found = true;
	      break;
	    }
	  pnt = pnt->next;
	}

      if(!found)
	{
	  isSelected = false;
	  break;
	}

      fi++;
    } // for

  if(isSelected)
    {
      /* this face is selected => discard original face and
	 create new faces */

      /* calculate the extrusion direction (use normal of selected face) */

      if(!(cv = (double*)calloc(m_faceVerts.size()*3, sizeof(double))))
	return;

      p = cv;
      fi = m_faceVerts.begin();
      for(i = 0; i < m_faceVerts.size(); i++)
	{
	  j = (*fi)*4;

	  p[0] = m_newVerts[j];
	  j++;
	  p[1] = m_newVerts[j];
	  j++;
	  p[2] = m_newVerts[j];

	  p += 3;
	  fi++;
	}

      if((fabs(cv[0]-cv[3]) > AY_EPSILON||
	  fabs(cv[1]-cv[4]) > AY_EPSILON||
	  fabs(cv[2]-cv[5]) > AY_EPSILON) &&
	 (fabs(cv[3]-cv[6]) > AY_EPSILON||
	  fabs(cv[4]-cv[7]) > AY_EPSILON||
	  fabs(cv[5]-cv[8]) > AY_EPSILON))
	{
	  ay_geom_calcnfrom3(&(cv[0]), &(cv[3]), &(cv[6]), N);
	}

      /* apply scale factor */
      ay_trafo_scalecog(m_scale, cv, m_faceVerts.size(), 3);

      /* create the new vertices */

      newVerts.reserve(m_faceVerts.size());

      p = cv;
      fi = m_faceVerts.begin();
      for(i = 0; i < m_faceVerts.size(); i++)
	{
	  j = (*fi)*4+3;

	  m_newVerts.push_back(p[0]-(N[0]*m_length));
	  m_newVerts.push_back(p[1]-(N[1]*m_length));
	  m_newVerts.push_back(p[2]-(N[2]*m_length));
	  m_newVerts.push_back(m_newVerts[j]);

	  newVerts.push_back(m_oldVertsNum+m_newVertsNum);

	  m_newSelection.push_back(m_oldVertsNum+m_newVertsNum);

	  m_newVertsNum++;

	  p += 3;
	  fi++;
	}

      /* create the new faces */

      for(i = 0; i < m_faceVerts.size()-1; i++)
	{
	  m_newFaces.push_back(4);
	  m_newFaces.push_back(m_faceVerts[i]);
	  m_newFaces.push_back(m_faceVerts[i+1]);
	  m_newFaces.push_back(newVerts[i+1]);
	  m_newFaces.push_back(newVerts[i]);

	  m_newFacesNum++;
	}

      m_newFaces.push_back(4);
      m_newFaces.push_back(m_faceVerts.back());
      m_newFaces.push_back(m_faceVerts.front());
      m_newFaces.push_back(newVerts.front());
      m_newFaces.push_back(newVerts.back());
      m_newFacesNum++;

      nfi = newVerts.begin();
      m_newFaces.push_back(newVerts.size());
      for(i = 0; i < newVerts.size(); i++)
	{
	  m_newFaces.push_back(*nfi);
	  nfi++;
	}
      m_newFacesNum++;
    }
  else
    {
      /* face is not selected => just copy original face */

      isDummy = false;
      fi = m_faceVerts.begin();
      for(i = 0; i < m_faceVerts.size(); i++)
	{
	  if(*fi > m_oldVertsNum)
	    {
	      isDummy = true;
	      break;
	    }
	  fi++;
	}

      if(!isDummy)
	{
	  m_newFaces.push_back(m_faceVerts.size());
	  fi = m_faceVerts.begin();
	  for(i = 0; i < m_faceVerts.size(); i++)
	    {
	      m_newFaces.push_back(*fi);
	      fi++;
	    }
	  m_newFacesNum++;
	}
      else
	{
	  m_newDummyFaces.push_back(m_faceVerts.size());
	  fi = m_faceVerts.begin();
	  for(i = 0; i < m_faceVerts.size(); i++)
	    {
	      m_newDummyFaces.push_back(*fi);
	      fi++;
	    }
	  m_newDummyFacesNum++;
	}
    } // if

  m_faceVerts.clear();

 return;
} /* FaceExtruder::closeFace */


void
FaceExtruder::addKnotInterval(unsigned int vertex1,
			      unsigned int vertex2,
			      KnotPrecision interval)
{
  m_newKnotIntervals.push_back(vertex1);
  m_newKnotIntervals.push_back(vertex2);
  m_newKnots.push_back(interval);
  m_newKnotIntervalsNum++;
 return;
} /* FaceExtruder::addKnotInterval */


void
FaceExtruder::finishKnotIntervals(void)
{
 MeshBuilder *meshBuilder = NULL;
 vector<unsigned int>::iterator fi;
 unsigned int numVerts;
 unsigned int i, j, ki1, ki2;

  m_newMesh = new Mesh(m_sdnpatch->subdivDegree);
  meshBuilder = MeshBuilder::create(*m_newMesh);

  j = 0;
  for(i = 0; i < m_oldVertsNum+m_newVertsNum; i++)
    {
      meshBuilder->addVertex(m_newVerts[j],
			     m_newVerts[j+1],
			     m_newVerts[j+2],
			     m_newVerts[j+3],
			     i);
      j += 4;
    }

  meshBuilder->finishVertices();

  fi = m_newFaces.begin();
  for(i = 0; i < m_newFacesNum; i++)
    {
      numVerts = *fi;
      meshBuilder->startFace(numVerts);

      fi++;
      for(j = 0; j < numVerts; j++)
	{
	  meshBuilder->addToFace(*fi);
	  fi++;
	}

      meshBuilder->closeFace();
    }

  if(m_newDummyFacesNum > 0)
    {
      fi = m_newDummyFaces.begin();
      for(i = 0; i < m_newDummyFacesNum; i++)
	{
	  numVerts = *fi;
	  meshBuilder->startFace(numVerts);

	  fi++;
	  for(j = 0; j < numVerts; j++)
	    {

	      if(*fi>= m_oldVertsNum)
		{
		  meshBuilder->addToFace(*fi+m_newVertsNum);
		}
	      else
		{
		  meshBuilder->addToFace(*fi);
		}
	      fi++;
	    }
	  meshBuilder->closeFace();
	}
    }

  meshBuilder->finishFaces();

  if(m_newKnotIntervalsNum > 0)
    {
      j = 0;
      for(i = 0; i < m_newKnotIntervalsNum; i++)
	{
	  ki1 = m_newKnotIntervals[j];

	  if(ki1 >= m_oldVertsNum)
	    ki1 += m_newVertsNum;

	  ki2 = m_newKnotIntervals[j+1];

	  if(ki2 >= m_oldVertsNum)
	    ki2 += m_newVertsNum;

	  meshBuilder->addKnotInterval(ki1, ki2, m_newKnots[i]);

	  j += 2;
	}
    }

  meshBuilder->finishKnotIntervals();

  MeshBuilder::dispose(meshBuilder);

 return;
} /* FaceExtruder::finishKnotIntervals */



/**
 * FaceRemover: Helper class to remove faces from a mesh.
 *
 */
class FaceRemover : public FlatMeshHandler
{
public:
  FaceRemover(sdnpatch_object *sdnpatch, ay_point *pnts);

  void addVertex(VertexPrecision x,
		 VertexPrecision y,
		 VertexPrecision z,
		 VertexPrecision w,
		 unsigned int id);
  void addToFace(unsigned int vertNum);
  void closeFace(void);
  void addKnotInterval(unsigned int vertex1,
		       unsigned int vertex2,
		       KnotPrecision interval);
  void finishKnotIntervals(void);

  Mesh *m_newMesh;

private:

  sdnpatch_object *m_sdnpatch;
  ay_point *m_pnts;

  vector<unsigned int> m_faceVerts;

  // collect new vertices
  unsigned int m_newVertsNum;
  vector<VertexPrecision> m_newVerts;
  vector<unsigned int> m_newVertexIDs;

  // collect new faces
  unsigned int m_newFacesNum;
  vector<unsigned int> m_newFaces;

  // collect knot intervals
  unsigned int m_newKnotIntervalsNum;
  vector<unsigned int> m_newKnotIntervals;
  vector<KnotPrecision> m_newKnots;

};


FaceRemover::FaceRemover(sdnpatch_object *sdnpatch, ay_point *pnts)
{
  m_sdnpatch = sdnpatch;
  m_pnts = pnts;
  m_newVertsNum = 0;
  m_newFacesNum = 0;
  m_newKnotIntervalsNum = 0;

  /*
    XXXX ToDo: reserve memory for various vectors
  */

} /* FaceRemover::FaceRemover */


void
FaceRemover::addVertex(VertexPrecision x,
		       VertexPrecision y,
		       VertexPrecision z,
		       VertexPrecision w,
		       unsigned int id)
{
  m_newVerts.push_back(x);
  m_newVerts.push_back(y);
  m_newVerts.push_back(z);
  m_newVerts.push_back(w);
  m_newVertexIDs.push_back(id);
  m_newVertsNum++;
 return;
} /* FaceRemover::addVertex */


void
FaceRemover::addToFace(unsigned int vertNum)
{
  m_faceVerts.push_back(vertNum);
 return;
} /* FaceRemover::addToFace */


void
FaceRemover::closeFace(void)
{
 vector<unsigned int>::iterator fi;
 bool found = false, isSelected = true;
 unsigned int i = 0;
 ay_point *pnt = NULL;

  /* is this face selected? */
  fi = m_faceVerts.begin();
  for(i = 0; i < m_faceVerts.size(); i++)
    {
      pnt = m_pnts;
      found = false;
      while(pnt)
	{
	  if(pnt->index == m_newVertexIDs[*fi])
	    {
	      found = true;
	      break;
	    }
	  pnt = pnt->next;
	}

      if(!found)
	{
	  isSelected = false;
	  break;
	}

      fi++;
    }

  if(!isSelected)
    {
      /* face is not selected => just copy original face */
      m_newFaces.push_back(m_faceVerts.size());
      fi = m_faceVerts.begin();
      for(i = 0; i < m_faceVerts.size(); i++)
	{
	  m_newFaces.push_back(*fi);
	  fi++;
	}
      m_newFacesNum++;
    }

  m_faceVerts.clear();

 return;
} /* FaceRemover::closeFace */


void
FaceRemover::addKnotInterval(unsigned int vertex1,
			     unsigned int vertex2,
			     KnotPrecision interval)
{
  m_newKnotIntervals.push_back(vertex1);
  m_newKnotIntervals.push_back(vertex2);
  m_newKnots.push_back(interval);
  m_newKnotIntervalsNum++;
 return;
} /* FaceRemover::addKnotInterval */


void
FaceRemover::finishKnotIntervals(void)
{
 MeshBuilder *meshBuilder = NULL;
 vector<unsigned int>::iterator fi;
 unsigned int i, j, numVerts;

  m_newMesh = new Mesh(m_sdnpatch->subdivDegree);
  meshBuilder = MeshBuilder::create(*m_newMesh);

  j = 0;
  for(i = 0; i < m_newVertsNum; i++)
    {
      meshBuilder->addVertex(m_newVerts[j],
			     m_newVerts[j+1],
			     m_newVerts[j+2],
			     m_newVerts[j+3],
			     i);
      j += 4;
    }

  meshBuilder->finishVertices();

  fi = m_newFaces.begin();
  for(i = 0; i < m_newFacesNum; i++)
    {
      numVerts = *fi;
      meshBuilder->startFace(numVerts);

      fi++;
      for(j = 0; j < numVerts; j++)
	{
	  meshBuilder->addToFace(*fi);
	  fi++;
	}

      meshBuilder->closeFace();
    }

  meshBuilder->finishFaces();

  if(m_newKnotIntervalsNum > 0)
    {
      j = 0;
      for(i = 0; i < m_newKnotIntervalsNum; i++)
	{
	  meshBuilder->addKnotInterval(m_newKnotIntervals[j],
				       m_newKnotIntervals[j+1],
				       m_newKnots[i]);
	  j += 2;
	}
    }

  try
    {
      meshBuilder->finishKnotIntervals();
    }
  catch(...)
    {
      MeshBuilder::dispose(meshBuilder);
      delete m_newMesh;
      m_newMesh = NULL;
      return;
    }

  MeshBuilder::dispose(meshBuilder);

 return;
} /* FaceRemover::finishKnotIntervals */



/**
 * FaceMerger: Helper class to remove two faces from a mesh.
 *
 */
class FaceMerger : public FlatMeshHandler
{
public:
  FaceMerger(sdnpatch_object *sdnpatch, ay_point *pnts);

  void addVertex(VertexPrecision x,
		 VertexPrecision y,
		 VertexPrecision z,
		 VertexPrecision w,
		 unsigned int id);
  void addToFace(unsigned int vertNum);
  void closeFace(void);
  void addKnotInterval(unsigned int vertex1,
		       unsigned int vertex2,
		       KnotPrecision interval);
  void finishKnotIntervals(void);

  Mesh *m_newMesh;
  bool m_error;

private:

  sdnpatch_object *m_sdnpatch;
  ay_point *m_pnts;

  vector<unsigned int> m_faceVerts;

  // collect vertices to keep (first selected face)
  unsigned int m_keepVertsNum;
  vector<unsigned int> m_keepVerts;

  // collect vertices to remove (second selected face)
  unsigned int m_removeVertsNum;
  vector<unsigned int> m_removeVerts;

  // collect all vertices
  unsigned int m_newVertsNum;
  vector<VertexPrecision> m_newVerts;
  vector<unsigned int> m_newVertexIDs;

  // collect new faces (minus first and second selected face)
  unsigned int m_newFacesNum;
  vector<unsigned int> m_newFaces;

  // collect knot intervals
  unsigned int m_newKnotIntervalsNum;
  vector<unsigned int> m_newKnotIntervals;
  vector<KnotPrecision> m_newKnots;

  bool m_connected;
};


FaceMerger::FaceMerger(sdnpatch_object *sdnpatch, ay_point *pnts)
{
  m_sdnpatch = sdnpatch;
  m_pnts = pnts;
  m_newMesh = NULL;

  m_keepVertsNum = 0;
  m_removeVertsNum = 0;

  m_newVertsNum = 0;
  m_newFacesNum = 0;

  m_newKnotIntervalsNum = 0;
  m_connected = false;
  m_error = false;

  /*
    XXXX ToDo: reserve memory for various vectors
  */

} /* FaceMerger::FaceMerger */


void
FaceMerger::addVertex(VertexPrecision x,
		      VertexPrecision y,
		      VertexPrecision z,
		      VertexPrecision w,
		      unsigned int id)
{
  m_newVerts.push_back(x);
  m_newVerts.push_back(y);
  m_newVerts.push_back(z);
  m_newVerts.push_back(w);
  m_newVertexIDs.push_back(id);
  m_newVertsNum++;
 return;
} /* FaceMerger::addVertex */


void
FaceMerger::addToFace(unsigned int vertNum)
{
  m_faceVerts.push_back(vertNum);
 return;
} /* FaceMerger::addToFace */


void
FaceMerger::closeFace(void)
{
 vector<unsigned int>::iterator fi, vi;
 bool found = false, isSelected = true;
 bool copyFace = true;
 unsigned int i = 0, j = 0, k = 0, l = 0, nV1 = 0, nV2 = 0;
 double *p1, *p2, curdist, mindist;
 vector<unsigned int> nearestVerts;
 ay_point *pnt = NULL;

  /* is this face selected? */
  fi = m_faceVerts.begin();
  for(i = 0; i < m_faceVerts.size(); i++)
    {
      // rule out dummy faces
      if(*fi >= m_newVertexIDs.size())
	{
	  isSelected = false;
	  break;
	}

      pnt = m_pnts;
      found = false;
      while(pnt)
	{
	  if(pnt->index == m_newVertexIDs[*fi])
	    {
	      found = true;
	      break;
	    }
	  pnt = pnt->next;
	}

      if(!found)
	{
	  isSelected = false;
	  break;
	}

      fi++;
    } // for

  if(isSelected)
    {
      /*
	 this face is selected;
	 if it is the first selected face that we encounter,
	 we remove the face but keep its vertices;
	 if it is the second selected face that we encounter,
	 we remove the face and its vertices (and map the vertices
	 to the vertices of the first selected face later on, for
	 other faces of the mesh),
	 the third and following selected faces are ignored/copied!
      */
      if(m_keepVertsNum == 0)
	{
	  m_keepVerts.reserve(m_faceVerts.size());
	  for(i = 0; i < m_faceVerts.size(); i++)
	    {
	      m_keepVerts.push_back(m_faceVerts[i]);
	      m_keepVertsNum++;
	    }
	  copyFace = false;
	}
      else
	{
	  if(m_removeVertsNum == 0)
	    {
	      /*
		XXXX ToDo: add check for incompatible faces
		m_faceVerts.size() != m_keepVertsNum
		can only happen with dual meshes anyway?
	      */

	      m_removeVerts.reserve(m_faceVerts.size());
	      for(i = 0; i < m_faceVerts.size(); i++)
		{
		  m_removeVerts.push_back(m_faceVerts[m_faceVerts.size()-i-1]);
		}

	      /*
	      m_removeVerts.push_back(m_faceVerts[1]);
	      m_removeVerts.push_back(m_faceVerts[0]);
	      m_removeVerts.push_back(m_faceVerts[3]);
	      m_removeVerts.push_back(m_faceVerts[2]);
	      */
	      /*
	      m_removeVerts.push_back(m_faceVerts[2]);
	      m_removeVerts.push_back(m_faceVerts[1]);
	      m_removeVerts.push_back(m_faceVerts[0]);
	      m_removeVerts.push_back(m_faceVerts[3]);
	      */

	      // find the pair of nearest vertices between
	      // m_keepVerts and m_removeVerts,
	      // starting from that pair, walk around the two faces
	      nearestVerts.resize(m_removeVerts.size());
	      mindist = DBL_MAX;
	      for(i = 0; i < m_removeVerts.size(); i++)
		{
		  p1 = &(m_sdnpatch->controlCoords[
			  m_newVertexIDs[m_removeVerts[i]]*4]);
		  found = false;
		  for(j = 0; j < m_keepVerts.size(); j++)
		    {
		      p2 = &(m_sdnpatch->controlCoords[
			      m_newVertexIDs[m_keepVerts[j]]*4]);

		      if(fabs(p1[0]-p2[0])<AY_EPSILON &&
			 fabs(p1[1]-p2[1])<AY_EPSILON &&
			 fabs(p1[2]-p2[2])<AY_EPSILON)
			{
			  mindist = 0.0;
			  found = true;
			  nV1 = i;
			  nV2 = j;
			  break;
			}
		      else
			{
			  curdist = sqrt(((p1[0]-p2[0])*(p1[0]-p2[0]))+
					 ((p1[1]-p2[1])*(p1[1]-p2[1]))+
					 ((p1[2]-p2[2])*(p1[2]-p2[2])));
			  if(curdist < mindist)
			    {
			      mindist = curdist;
			      nV1 = i;
			      nV2 = j;
			    }
			} // if
		    } // for
		  if(found)
		    {
		      break;
		    }
		} // for

	      l = nV1;
	      k = nV2;

	      for(i = 0; i < m_removeVerts.size(); i++)
		{
		  nearestVerts[k] = m_removeVerts[l];

		  if(k == nearestVerts.size()-1)
		    k = 0;
		  else
		    k++;

		  if(l == m_removeVerts.size()-1)
		    l = 0;
		  else
		    l++;

		}

	      m_removeVerts.clear();
	      for(i = 0; i < m_faceVerts.size(); i++)
		{
		  /*
		     vertices that we already decided to keep must
		     not be removed (in this case the two selected
		     faces share an edge)
		     so we check, whether any of the vertices in m_faceVerts
		     are already in m_keepVerts
		  */
		  found = false;
		  vi = m_keepVerts.begin();
		  for(j = 0; j < m_keepVertsNum; j++)
		    {
		      if(nearestVerts[i] == m_keepVerts[j])
			{
			  found = true;
			  break;
			}
		      vi++;
		    }
		  if(!found)
		    {
		      m_removeVerts.push_back(nearestVerts[i]);
		      m_removeVertsNum++;
		    }
		  else
		    {
		      m_keepVerts.erase(vi);
		      m_keepVertsNum--;
		    }
		} // for

	      copyFace = false;
	    } // if
	} // if
    } /* if isSelected */

  if(copyFace)
    {
      m_newFaces.push_back(m_faceVerts.size());
      fi = m_faceVerts.begin();
      for(i = 0; i < m_faceVerts.size(); i++)
	{
	  m_newFaces.push_back(*fi);
	  fi++;
	}
      m_newFacesNum++;
    }

  m_faceVerts.clear();

 return;
} /* FaceMerger::closeFace */


void
FaceMerger::addKnotInterval(unsigned int vertex1,
			    unsigned int vertex2,
			    KnotPrecision interval)
{
  m_newKnotIntervals.push_back(vertex1);
  m_newKnotIntervals.push_back(vertex2);
  m_newKnots.push_back(interval);
  m_newKnotIntervalsNum++;
 return;
} /* FaceMerger::addKnotInterval */


void
FaceMerger::finishKnotIntervals(void)
{
 MeshBuilder *meshBuilder = NULL;
 vector<unsigned int>::iterator fi;
 unsigned int numVerts;
 unsigned int i, j, k, l;
 bool discard, rewrite;
 vector<unsigned int> newKeepIndices, newIndices;
 unsigned int sub;
 set<pair<unsigned int,unsigned int> > edges;
 set<pair<unsigned int,unsigned int> >::iterator ei;
 vector<unsigned int>::iterator fi1, fi2;

  if((m_keepVertsNum == 0) || (m_removeVertsNum == 0))
    {
      m_error = true;
      return;
    }

  m_newMesh = new Mesh(m_sdnpatch->subdivDegree);
  meshBuilder = MeshBuilder::create(*m_newMesh);

  // vertices
  j = 0;
  l = 0;
  for(i = 0; i < m_newVertsNum; i++)
    {
      discard = false;

      for(k = 0; k < m_removeVertsNum; k++)
	{
	  if(i == m_removeVerts[k])
	  {
	    discard = true;
	    break;
	  }
	}

      if(!discard)
	{
	  meshBuilder->addVertex(m_newVerts[j],
				 m_newVerts[j+1],
				 m_newVerts[j+2],
				 m_newVerts[j+3],
				 l);
	  l++;
	}
      j += 4;
    }

  meshBuilder->finishVertices();


  // calculate new indices

  // if any keep indices are higher than remove vertices
  // they need to be adapted (to account for the removed vertices in-between)
  newKeepIndices.resize(m_keepVertsNum);
  for(i = 0; i < m_keepVertsNum; i++)
    {
      newKeepIndices[i] = m_keepVerts[i];
      for(k = 0; k < m_removeVertsNum; k++)
	{
	  if(m_removeVerts[k] < m_keepVerts[i])
	    {
	      newKeepIndices[i]--;
	    }
	}
    }

  // the newIndices vector serves as translation table between
  // old indices (vector element adress) and
  // new indices (vector element content)

  // initialize vector with a 1:1 translation
  newIndices.reserve(m_newVertsNum);
  for(i = 0; i < m_newVertsNum; i++)
    {
      newIndices.push_back(i);
    }

  // now we rewrite the indices:
  // a) decrease all normal indices that are higher than
  //    any of the removed vertices (account for removed vertices in-between);
  // b) replace removed vertices with keep vertices
  sub = m_removeVertsNum;
  for(i = m_newVertsNum; i > 0; i--)
    {
      rewrite = false;
      for(k = 0; k < m_removeVertsNum; k++)
	{
	  if(m_removeVerts[k] == i-1)
	    {
	      rewrite = true;
	      newIndices[i-1] = newKeepIndices[k];
	      sub--;
	      break;
	    }
	}
      if(!rewrite)
	{
	  newIndices[i-1] -= sub;
	}
    }

  // faces
  fi = m_newFaces.begin();
  for(i = 0; i < m_newFacesNum; i++)
    {
      numVerts = *fi;
      fi++;

      /*
	the only way to reliably detect a failed merge (due to
	incompatible vertex orders in the two faces?) is via
	checking for doubly defined edges here (as libsnurbs
	sometimes happily accepts such data, even subdivides it
	correctly, but bombs later, on the second generation
	copy of the bad mesh)
      */

      fi1 = fi;
      fi2 = fi+1;
      for(j = 0; j < numVerts; j++)
	{
	  if(edges.size() > 0)
	    {
	      ei = edges.find(pair<unsigned int, unsigned int>
			      (newIndices[*fi1], newIndices[*fi2]));
	      if(ei != edges.end())
		{
		  // we already have this edge in the mesh => bail out
		  MeshBuilder::dispose(meshBuilder);
		  m_error = true;
		  return;
		}
	    }

	  edges.insert(pair<unsigned int, unsigned int>
			  (newIndices[*fi1], newIndices[*fi2]));

	  fi1++;
	  if(j == numVerts-2)
	    {
	      fi2 = fi;
	    }
	  else
	    {
	      fi2++;
	    }
	} // for

      meshBuilder->startFace(numVerts);

      for(j = 0; j < numVerts; j++)
	{
	  // XXXX ToDo add check for dummy face indices
	  meshBuilder->addToFace(newIndices[*fi]);
	  fi++;
	}

      meshBuilder->closeFace();
    } // for

  meshBuilder->finishFaces();

#if 0
  // knot intervals
  if(m_newKnotIntervalsNum > 0)
    {
      j = 0;
      for(i = 0; i < m_newKnotIntervalsNum; i++)
	{
	  try
	    {
	      meshBuilder->addKnotInterval(newIndices[m_newKnotIntervals[j]],
					   newIndices[m_newKnotIntervals[j+1]],
					   m_newKnots[i]);
	    }
	  catch(...)
	    {
	      // XXXX report error?
	    }
	  j += 2;
	}
    }
#endif
  try
    {
      meshBuilder->finishKnotIntervals();
    }
  catch(std::runtime_error)
    {
      m_error = true;
    }
  MeshBuilder::dispose(meshBuilder);

 return;
} /* FaceMerger::finishKnotIntervals */



/**
 * FaceConnector: Helper class to connect two faces of a mesh via
 * a set of new faces (removing the original two faces).
 *
 */
class FaceConnector : public FlatMeshHandler
{
public:
  FaceConnector(sdnpatch_object *sdnpatch, ay_point *pnts);
  ~FaceConnector();

  void addVertex(VertexPrecision x,
		 VertexPrecision y,
		 VertexPrecision z,
		 VertexPrecision w,
		 unsigned int id);
  void finishVertices();
  void addToFace(unsigned int vertNum);
  void closeFace(void);
  void finishFaces(void);
  void addKnotInterval(unsigned int vertex1,
		       unsigned int vertex2,
		       KnotPrecision interval);
  void finishKnotIntervals(void);

  Mesh *m_newMesh;
  bool m_error;

private:


  sdnpatch_object *m_sdnpatch;
  ay_point *m_pnts;

  MeshBuilder *m_meshBuilder;

  vector<unsigned int> m_faceVerts;

  vector<unsigned int> m_newVertexIDs;

  unsigned int m_face1VertsNum;
  vector<unsigned int> m_face1Verts;
  bool m_connected;
};


FaceConnector::FaceConnector(sdnpatch_object *sdnpatch, ay_point *pnts)
{
  m_sdnpatch = sdnpatch;
  m_pnts = pnts;

  m_newMesh = new Mesh(m_sdnpatch->subdivDegree);
  m_meshBuilder = MeshBuilder::create(*m_newMesh);
  m_face1VertsNum = 0;
  m_connected = false;
  m_error = false;
} /* FaceConnector::FaceConnector */

FaceConnector::~FaceConnector()
{
  MeshBuilder::dispose(m_meshBuilder);
} /* FaceConnector::~FaceConnector */

void
FaceConnector::addVertex(VertexPrecision x,
			 VertexPrecision y,
			 VertexPrecision z,
			 VertexPrecision w,
			 unsigned int id)
{
  m_meshBuilder->addVertex(x, y, z, w, id);
  m_newVertexIDs.push_back(id);
 return;
} /* FaceConnector::addVertex */

void
FaceConnector::finishVertices()
{
  m_meshBuilder->finishVertices();
 return;
} /* FaceConnector::finishVertices */

void
FaceConnector::addToFace(unsigned int vertNum)
{
  m_faceVerts.push_back(vertNum);
 return;
} /* FaceConnector::addToFace */

void
FaceConnector::closeFace(void)
{
 vector<unsigned int>::iterator fi;
 bool found = false, isSelected = true;
 bool copyFace = true;
 unsigned int i = 0, j = 0, k, l, /*nV = 0,*/ nV1 = 0, nV2 = 0;
 ay_point *pnt = NULL;
 double *p1, *p2, curdist, mindist;
 vector<unsigned int> nearestVerts;

  /* is this face selected? */
  fi = m_faceVerts.begin();
  for(i = 0; i < m_faceVerts.size(); i++)
    {
      // rule out dummy faces
      if(*fi >= m_newVertexIDs.size())
	{
	  isSelected = false;
	  break;
	}

      pnt = m_pnts;
      found = false;
      while(pnt)
	{
	  if(pnt->index == m_newVertexIDs[*fi])
	    {
	      found = true;
	      break;
	    }
	  pnt = pnt->next;
	}

      if(!found)
	{
	  isSelected = false;
	  break;
	}

      fi++;
    } // for

  if(isSelected)
    {
      /*
	 this face is selected;
	 if it is the first selected face that we encounter,
	 we keep its vertices but remove the face,
	 if it is the second selected face that we encounter,
	 we remove the face and connect the vertices to the
	 first faces vertices via new faces;
	 the third and following selected faces are ignored/copied!
      */
      if(m_face1VertsNum == 0)
	{
	  m_face1Verts.reserve(m_faceVerts.size());
	  for(i = 0; i < m_faceVerts.size(); i++)
	    {
	      m_face1Verts.push_back(m_faceVerts[i]);
	      m_face1VertsNum++;
	    }
	  copyFace = false;
	}
      else
	{
	  if(!m_connected)
	    {
	      /* m_faceVerts is m_face2Verts... */

	      /*
		XXXX ToDo: add check for incompatible faces
		m_faceVerts.size() != m_face1VertsNum
		can only happen with dual meshes anyway?
	      */

	      nearestVerts.resize(m_faceVerts.size());

#if 0
	      // old strategy, too fragile (crash for perpendicular faces)

	      // for each vertice in face1 find the nearest vertice in face2
	      for(i = 0; i < m_faceVerts.size(); i++)
		{
		  p1 = &(m_sdnpatch->controlCoords[
			       m_newVertexIDs[m_faceVerts[i]]*4]);
		  mindist = DBL_MAX;
		  for(j = 0; j < m_face1Verts.size(); j++)
		    {
		      p2 = &(
              m_sdnpatch->controlCoords[m_newVertexIDs[m_face1Verts[j]]*4]);
		      if(fabs(p1[0]-p2[0])<AY_EPSILON &&
			 fabs(p1[1]-p2[1])<AY_EPSILON &&
			 fabs(p1[2]-p2[2])<AY_EPSILON)
			{
			  nV = j;
			  break;
			}
		      else
			{
			  curdist = sqrt(((p1[0]-p2[0])*(p1[0]-p2[0]))+
					 ((p1[1]-p2[1])*(p1[1]-p2[1]))+
					 ((p1[2]-p2[2])*(p1[2]-p2[2])));
			  if(curdist < mindist)
			    {
			      mindist = curdist;
			      nV = j;
			    }
			} // if
		    } // for

		  //
		  for(j = 0; j < nearestVerts.size(); j++)
		    {
		      if(nearestVerts[j] == nV)
			{
			    m_error = true;
			    m_faceVerts.clear();
			    return;
			}
		    }

		  nearestVerts.push_back(m_face1Verts[nV]);
		} // for
#endif

	      // new strategy
	      // find the pair of nearest vertices between face1 and face2,
	      // starting from that pair, walk around the two faces in
	      // opposite directions...
	      mindist = DBL_MAX;
	      for(i = 0; i < m_faceVerts.size(); i++)
		{
		  p1 = &(m_sdnpatch->controlCoords[
			       m_newVertexIDs[m_faceVerts[i]]*4]);
		  for(j = 0; j < m_face1Verts.size(); j++)
		    {
		      p2 = &(
              m_sdnpatch->controlCoords[m_newVertexIDs[m_face1Verts[j]]*4]);

		      if(fabs(p1[0]-p2[0])<AY_EPSILON &&
			 fabs(p1[1]-p2[1])<AY_EPSILON &&
			 fabs(p1[2]-p2[2])<AY_EPSILON)
			{
			  nV1 = i;
			  nV2 = j;
			  break;
			}
		      else
			{
			  curdist = sqrt(((p1[0]-p2[0])*(p1[0]-p2[0]))+
					 ((p1[1]-p2[1])*(p1[1]-p2[1]))+
					 ((p1[2]-p2[2])*(p1[2]-p2[2])));
			  if(curdist < mindist)
			    {
			      mindist = curdist;
			      nV1 = i;
			      nV2 = j;
			    }
			} // if
		    } // for
		} // for
	      k = nV1;
	      l = nV2;
	      for(i = 0; i < m_faceVerts.size(); i++)
		{
		  for(j = 0; j < m_face1Verts.size(); j++)
		    {
		      nearestVerts[k] = m_face1Verts[l];

		      if(k == m_faceVerts.size()-1)
			k = 0;
		      else
			k++;

		      if(l == 0)
			l = m_face1Verts.size()-1;
		      else
			l--;
		    }
		}

	      // create new faces based on the connectivity
	      // information from nearestVerts
	      for(i = 0; i < m_faceVerts.size()-1; i++)
		{
		   m_meshBuilder->startFace(4);
		   m_meshBuilder->addToFace(m_faceVerts[i]);
		   m_meshBuilder->addToFace(m_faceVerts[i+1]);
		   m_meshBuilder->addToFace(nearestVerts[i+1]);
		   m_meshBuilder->addToFace(nearestVerts[i]);
		   m_meshBuilder->closeFace();
		}
	      m_meshBuilder->startFace(4);
	      m_meshBuilder->addToFace(m_faceVerts[i]);
	      m_meshBuilder->addToFace(m_faceVerts[0]);
	      m_meshBuilder->addToFace(nearestVerts[0]);
	      m_meshBuilder->addToFace(nearestVerts[i]);
	      m_meshBuilder->closeFace();

	      m_connected = true;
	      copyFace = false;
	    } // if
	} // if
    } // if

  if(copyFace)
    {
      m_meshBuilder->startFace(m_faceVerts.size());

      for(i = 0; i < m_faceVerts.size(); i++)
	{
	  m_meshBuilder->addToFace(m_faceVerts[i]);
	}

      m_meshBuilder->closeFace();
    }

  m_faceVerts.clear();

 return;
} /* FaceConnector::closeFace */

void
FaceConnector::finishFaces(void)
{
  if(!m_connected)
    {
      m_error = true;
    }
  m_meshBuilder->finishFaces();
 return;
} /* FaceConnector::finishFaces */

void
FaceConnector::addKnotInterval(unsigned int vertex1,
			       unsigned int vertex2,
			       KnotPrecision interval)
{
  if(!m_error)
    {
      try
	{
	  m_meshBuilder->addKnotInterval(vertex1, vertex2, interval);
	}
      catch(...)
	{
	  /* XXXX */
	}
    }
 return;
} /* FaceConnector::addKnotInterval */


void
FaceConnector::finishKnotIntervals(void)
{
  m_meshBuilder->finishKnotIntervals();
 return;
} /* FaceConnector::finishKnotIntervals */


/**
 * KnotSelector: Helper class to edit knots.
 *
 */
class KnotSelector : public FlatMeshHandler
{
public:
  KnotSelector(sdnpatch_object *sdnpatch, ay_point *pnts,
	       MeshFlattener *flattener, vector<KnotInterval *> *knots);
  ~KnotSelector();
  void addVertex(VertexPrecision x,
		 VertexPrecision y,
		 VertexPrecision z,
		 VertexPrecision w,
		 unsigned int id);
  void startFace(unsigned int numEdges);
  void addToFace(unsigned int vertNum);
  void closeFace(void);

private:

  sdnpatch_object *m_sdnpatch;
  ay_point *m_pnts;
  MeshFlattener *m_meshFlattener;

  unsigned int m_vertNum;
  unsigned int m_faceNum;
  unsigned int m_faceVertNum;

  KnotPrecision m_interval;

  vector<unsigned int> m_newVertexIDs;
  vector<KnotInterval *> *m_selectedKnots;

};


KnotSelector::KnotSelector(sdnpatch_object *sdnpatch, ay_point *pnts,
			   MeshFlattener *flattener,
			   vector<KnotInterval *> *knots)
{
  m_sdnpatch = sdnpatch;
  m_pnts = pnts;
  m_meshFlattener = flattener;
  m_selectedKnots = knots;
  m_vertNum = 0;
  m_faceNum = 0;
  m_faceVertNum = 0;
} /* KnotSelector::KnotSelector */

KnotSelector::~KnotSelector()
{

} /* KnotSelector::~KnotSelector */

void
KnotSelector::addVertex(VertexPrecision x,
			VertexPrecision y,
			VertexPrecision z,
			VertexPrecision w,
			unsigned int id)
{
  m_newVertexIDs.push_back(id);
  m_vertNum++;
 return;
} /* KnotSelector::addVertex */


void
KnotSelector::startFace(unsigned int numEdges)
{
 return;
} /* KnotSelector::startFace */

void
KnotSelector::addToFace(unsigned int vertNum)
{
 static bool found1 = false;
 static unsigned int vertex1;
 bool found2 = false;
 unsigned int vertex2;
 ay_point *pnt = NULL;
 KnotInterval *knot = NULL;

  // rule out dummy vertices
  if(vertNum >= m_vertNum)
    return;

  if(m_faceVertNum == 0)
    {
      /* first vertex in face */
      vertex1 = vertNum;
      found1 = false;
      pnt = m_pnts;
      while(pnt)
	{
	  if(pnt->index == m_newVertexIDs[vertex1])
	    {
    	      found1 = true;
	      break;
	    }
	  pnt = pnt->next;
	}
    }
  else
    {
      vertex2 = vertNum;
      pnt = m_pnts;
      while(pnt)
	{
	  if(pnt->index == m_newVertexIDs[vertex2])
	    {
	      found2 = true;
	      break;
	    }
	  pnt = pnt->next;
	} // while

      if(found1 && found2)
	{
	  /* edge is selected => select knot interval */
	  knot = m_meshFlattener->getKnotIntervalForEdge(m_faceNum,
							 m_faceVertNum-1);

	  if(knot)
	    m_selectedKnots->push_back(knot);
	}

      found1 = found2;
      vertex1 = vertex2;
    } // if

  m_faceVertNum++;

 return;
} /* KnotSelector::addToFace */

void
KnotSelector::closeFace(void)
{
  m_faceNum++;
  m_faceVertNum = 0;
 return;
} /* KnotSelector::closeFace */


/****************************************************************************/


/* sdnpatch_createcb:
 *  create callback function of sdnpatch object
 * crtOb SDNPatch -v {0 0 0 1  1 0 0 1  1 1 0 1   0 1 0 1} -f {4 0 1 2 3}
 */
int
sdnpatch_createcb(int argc, char *argv[], ay_object *o)
{
 int ay_status = AY_OK;
 int tcl_status = TCL_OK;
 char fname[] = "crtsdnpatch";
 char option_handled = AY_FALSE;
 char **av = NULL;
 int avlen = 0;
 int tmpi, optnum = 0, i = 2, j = 0;
 unsigned int k = 0;
 unsigned int *faces = NULL, *edges = NULL;
 int faceslen = 0, edgeslen = 0;
 double *verts = NULL, *knots = NULL;
 int vertslen = 0, knotslen = 0;
 sdnpatch_object *sdnpatch = NULL;
 MeshBuilder *meshBuilder = NULL;

  if(!o)
    return AY_ENULL;

  /* parse args */
  while(i < argc)
    {
      if(i+1 >= argc)
	{
	  ay_error(AY_EOPT, fname, argv[i]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

      tcl_status = TCL_OK;
      option_handled = AY_FALSE;
      optnum = i;
      if(argv[i] && argv[i][0] != '\0')
	{
	  switch(argv[i][1])
	    {
	    case 'f':
	      /* -faces */
	      if(Tcl_SplitList(ay_interp, argv[i+1], &avlen, &av) ==
		 TCL_OK)
		{
		  if(faces)
		    {
		      free(faces);
		    }
		  if(!(faces = (unsigned int*)calloc(avlen,
						     sizeof(unsigned int))))
		    {
		      Tcl_Free((char *) av);
		      ay_status = AY_EOMEM;
		      goto cleanup;
		    }
		  for(j = 0; j < avlen; j++)
		    {
		      tcl_status = Tcl_GetInt(ay_interp, av[j], &tmpi);
		      if(tcl_status != TCL_OK)
			{
			  break;
			}
		      faces[j] = tmpi;
		    } /* for */
		  faceslen = avlen;
		  Tcl_Free((char *) av);
		}
	      option_handled = AY_TRUE;
	      break;
	    case 'k':
	      /* -knots */
	      if(Tcl_SplitList(ay_interp, argv[i+1], &avlen, &av) ==
		 TCL_OK)
		{
		  if(edges)
		    {
		      free(edges);
		    }
		  if(!(edges = (unsigned int*)calloc((avlen/3)*2,
						    sizeof(unsigned int))))
		    {
		      Tcl_Free((char *) av);
		      ay_status = AY_EOMEM;
		      goto cleanup;
		    }
		  if(knots)
		    {
		      free(knots);
		    }
		  if(!(knots = (double*)calloc(avlen/3, sizeof(double))))
		    {
		      Tcl_Free((char *) av);
		      ay_status = AY_EOMEM;
		      goto cleanup;
		    }
		  k = 0;
		  for(j = 0; j < (avlen/3)*2; j+=2)
		    {
		      tcl_status = Tcl_GetInt(ay_interp, av[k], &tmpi);
		      if(tcl_status != TCL_OK)
			{
			  break;
			}
		      edges[j] = tmpi;
		      k++;
		      tcl_status = Tcl_GetInt(ay_interp, av[k], &tmpi);
		      if(tcl_status != TCL_OK)
			{
			  break;
			}
		      edges[j+1] = tmpi;
		      k += 2;
		    } /* for */
		  edgeslen = (avlen/3)*2;
		  k = 2;
		  for(j = 0; j < avlen/3; j++)
		    {
		      tcl_status = Tcl_GetDouble(ay_interp,
						 av[k], &knots[j]);
		      if(tcl_status != TCL_OK)
			{
			  break;
			}
		      k += 3;
		    } /* for */
		  knotslen = (avlen/3);
		  Tcl_Free((char *) av);
		}
	      option_handled = AY_TRUE;
	      break;
	    case 'v':
	      /* -vertices */
	      if(Tcl_SplitList(ay_interp, argv[i+1], &avlen, &av) ==
		 TCL_OK)
		{
		  if(verts)
		    {
		      free(verts);
		    }
		  if(!(verts = (double*)calloc(avlen, sizeof(double))))
		    {
		      Tcl_Free((char *) av);
		      ay_status = AY_EOMEM;
		      goto cleanup;
		    }
		  for(j = 0; j < avlen; j++)
		    {
		      tcl_status = Tcl_GetDouble(ay_interp,
						 av[j], &verts[j]);
		      if(tcl_status != TCL_OK)
			{
			  break;
			}
		    } /* for */
		  vertslen = avlen;
		  Tcl_Free((char *) av);
		}
	      option_handled = AY_TRUE;
	      break;
	    default:
	      break;
	    } /* switch */

	  if(option_handled && (tcl_status != TCL_OK))
	    {
	      ay_error(AY_EOPT, fname, argv[i]);
	      ay_status = AY_ERROR;
	      goto cleanup;
	    }

	  i += 2;
	}
      else
	{
	  i++;
	} /* if */

      if(!option_handled)
	{
	  ay_error(AY_EUOPT, fname, argv[optnum]);
	  ay_status = AY_ERROR;
	  goto cleanup;
	}

    } /* while */


  /* XXXX check args */

  /*sdnpatch->lowDegreeMesh = new snurbs::Mesh;*/

  if(!(sdnpatch = (sdnpatch_object*)calloc(1, sizeof(sdnpatch_object))))
    {
      ay_status = AY_EOMEM;
      goto cleanup;
    }

  sdnpatch->subdivDegree = 3;
  sdnpatch->subdivLevel = 0;

  sdnpatch->controlMesh = new Mesh(sdnpatch->subdivDegree);
  meshBuilder = MeshBuilder::create(*(sdnpatch->controlMesh));

  if(!verts)
    {
      meshBuilder->addVertex(-0.5,-0.5,-0.5,1,0);
      meshBuilder->addVertex( 0.5,-0.5,-0.5,1,1);
      meshBuilder->addVertex( 0.5, 0.5,-0.5,1,2);
      meshBuilder->addVertex(-0.5, 0.5,-0.5,1,3);

      meshBuilder->addVertex(-0.5,-0.5,0.5,1,4);
      meshBuilder->addVertex( 0.5,-0.5,0.5,1,5);
      meshBuilder->addVertex( 0.5, 0.5,0.5,1,6);
      meshBuilder->addVertex(-0.5, 0.5,0.5,1,7);

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
    }
  else
    {
      /* vertices */
      i = 0;
      for(j = 0; j < (vertslen/4); j++)
	{
	  meshBuilder->addVertex(verts[i], verts[i+1],
				 verts[i+2], verts[i+3], j);
	  i += 4;
	} /* for */
      meshBuilder->finishVertices();

      /* faces */
      for(j = 0; j < faceslen; j++)
	{
	  meshBuilder->startFace(faces[j]);
	  for(k = 1; k <= faces[j]; k++)
	    {
	      meshBuilder->addToFace(faces[j+k]);
	    } /* for */
	  meshBuilder->closeFace();
	  j += faces[j];
	} /* for */
      meshBuilder->finishFaces();

      /* knot intervals */
      if(knotslen > 0)
	{
	  for(j = 0; j < knotslen; j++)
	    {
	      meshBuilder->addKnotInterval(edges[k], edges[k+1], knots[j]);
	      k += 2;
	    }
	}
      meshBuilder->finishKnotIntervals();
    } /* if */

  MeshBuilder::dispose(meshBuilder);
  /*
  sdnpatch->controlWireList = glGenLists(1);
  sdnpatch->controlFaceList = glGenLists(1);
  */
  sdnpatch_getcontrolvertices(sdnpatch);

  o->refine = sdnpatch;

  ay_notify_object(o);

cleanup:

  if(verts)
    free(verts);

  if(edges)
    free(edges);

  if(faces)
    free(faces);

  if(knots)
    free(knots);

  if(ay_status == AY_EOMEM)
    {
      ay_error(AY_EOMEM, fname, NULL);
      ay_status = AY_ERROR;
    }

 return ay_status;
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

  sdnpatch_getcontrolvertices(sdnpatch);
  /*
  if(srcsdnpatch->subdivMesh)
    sdnpatch->subdivMesh = new Mesh(*(srcsdnpatch->subdivMesh));
  */
  sdnpatch->subdivMesh = NULL;

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

  if(sdnpatch->subdivMesh)
    meshFlattener = MeshFlattener::create(*(sdnpatch->subdivMesh));
  else
    meshFlattener = MeshFlattener::create(*(sdnpatch->controlMesh));
  meshFlattener->setCompatible(true);

  glBegin(GL_QUADS);
   meshFlattener->receiveVertices(std::tr1::bind(&sdnpatch_quadcb,
						 _1, _2, _3, _4));
   if(sdnpatch->subdivMesh && sdnpatch->subdivLevel > 0)
     {
       meshFlattener->receiveSmoothNormals(&glNormal3d);
     }
   else
     {
       meshFlattener->receiveFlatNormals(&glNormal3d);
     }
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
 char fname[] = "sdnpatch_setpropcb";
 char *n1 = "SDNPatchAttrData";
 Tcl_Obj *to = NULL, *toa = NULL, *ton = NULL;
 sdnpatch_object *sdnpatch = NULL;
 int itemp = 0;

  if(!interp || !o)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)o->refine;

  if(!sdnpatch)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Degree",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp,to, &itemp);

  if(itemp != 3 && itemp != 5 && itemp != 7)
    {
      ay_error(AY_EOMEM, fname, "Unsupported degree, must be 3, 5, or 7!");
    }
  else
    {
      sdnpatch->subdivDegree = itemp;
    }

  Tcl_SetStringObj(ton,"Level",-1);
  to = Tcl_ObjGetVar2(interp,toa,ton,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
  Tcl_GetIntFromObj(interp, to, &itemp);
  if(itemp >= 0)
    {
      sdnpatch->subdivLevel = itemp;
    }
  Tcl_IncrRefCount(toa);Tcl_DecrRefCount(toa);
  Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);


  (void)ay_notify_object(o);

  o->modified = AY_TRUE;
  (void)ay_notify_parent();

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

  if(!interp || !o)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)(o->refine);

  if(!sdnpatch)
    return AY_ENULL;

  toa = Tcl_NewStringObj(n1,-1);
  ton = Tcl_NewStringObj(n1,-1);

  Tcl_SetStringObj(ton,"Degree",-1);
  to = Tcl_NewIntObj(sdnpatch->subdivDegree);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);

  Tcl_SetStringObj(ton,"Level",-1);
  to = Tcl_NewIntObj(sdnpatch->subdivLevel);
  Tcl_ObjSetVar2(interp,toa,ton,to,TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);


  Tcl_SetStringObj(ton, "NPolys", -1);
  if(sdnpatch->subdivMesh)
    to = Tcl_NewIntObj(sdnpatch->subdivMesh->getNumFaces());
  else
    to = Tcl_NewIntObj(0);
  Tcl_ObjSetVar2(interp, toa, ton, to, TCL_LEAVE_ERR_MSG |
		 TCL_GLOBAL_ONLY);


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

  if(!fileptr || !o)
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
      meshBuilder->addVertex(x, y, z, w, i);
    }
  meshBuilder->finishVertices();

  fscanf(fileptr, "%u\n", &numFaces);

  for(i = 0; i < numFaces; i++)
    {
      fscanf(fileptr, "%u", &numEdges);
      meshBuilder->startFace(numEdges);

      for(j = 0; j < numEdges; j++)
	{
	  fscanf(fileptr, " %u", &e);
	  meshBuilder->addToFace(e);
	}
      meshBuilder->closeFace();
      fscanf(fileptr, "\n");
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

  sdnpatch_getcontrolvertices(sdnpatch);

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

  if(!fileptr || !o)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)(o->refine);

  if(!sdnpatch)
    return AY_ENULL;

  fprintf(fileptr, "%d\n", sdnpatch->subdivDegree);
  fprintf(fileptr, "%d\n", sdnpatch->subdivLevel);

  /* write the control mesh using the AyWriter MeshFlattener handler */
  mesh = sdnpatch->controlMesh;
  meshFlattener = MeshFlattener::create(*(sdnpatch->controlMesh));
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

  ay_object_deletemulti(c, AY_FALSE);

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
    } // for

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
 int currentLevel = 0, targetLevel = 0;
 char *in_action = NULL, arrname[] = "ay", varname[] = "action";

  if(!o)
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)(o->refine);

  if(!sdnpatch)
    return AY_ENULL;

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

  targetLevel = sdnpatch->subdivLevel;
  in_action = Tcl_GetVar2(ay_interp, arrname, varname, TCL_GLOBAL_ONLY);
  if(ay_prefs.lazynotify)
    {
      if(in_action && *in_action == '1')
	{
	  // XXXX ToDo: make this configurable
	  //targetLevel = 0;
	  return AY_OK;
	}
    }
  else
    {
      if(in_action && *in_action == '1')
	{
	  // XXXX ToDo: make this configurable
	  targetLevel = 1;
	}
    }

  /* update subdivision surface */
  if(sdnpatch->subdivMesh)
    {
      delete sdnpatch->subdivMesh;
    }

  sdnpatch->subdivMesh = new Mesh(*(sdnpatch->controlMesh));

  while(currentLevel < targetLevel)
    {
      sdnpatch->subdivMesh->subdivide();
      currentLevel++;
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

  if(!sdnpatch)
    return AY_ENULL;

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
	} // if

      *result = newo;
    } // if

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

  if(!sdnpatch)
    return AY_ENULL;

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
    } // if

  /* second, link new objects, or replace old objects with them */

  if(newo)
    {
      if(!in_place)
	{
	  ay_object_link(newo);
	}
      else
	{
	  ay_object_replace(newo, o);
	} // if
    } // if

 return ay_status;
} /* sdnpatch_convertcb */


/* sdnpatch_getpntcb:
 *  get point (editing and selection) callback function of sdnpatch object
 */
int
sdnpatch_getpntcb(int mode, ay_object *o, double *p, ay_pointedit *pe)
{
 sdnpatch_object *sdnpatch = NULL;
 ay_point *pnt = NULL, **lastpnt = NULL;
 unsigned int i = 0, a = 0;
 double minDist = ay_prefs.pick_epsilon, curDist = 0.0;
 double **tmpc, *c;
 unsigned int *tmpi;
 std::vector<Vertex*> *vertices = NULL;

  if(!o || ((mode != 3) && (!p || !pe)))
    return AY_ENULL;

  sdnpatch = (sdnpatch_object *)(o->refine);

  if(!sdnpatch || !sdnpatch->controlVertices)
    return AY_ENULL;

  vertices = sdnpatch->controlVertices;

  if(pe)
    pe->rational = AY_TRUE;

  switch(mode)
    {
    case 0:
      /* select all points */
      pe->coords = (double**)calloc(vertices->size(), sizeof(double*));
      pe->indices = (unsigned int*)calloc(vertices->size(),
					  sizeof(unsigned int));

      for(i = 0; i < vertices->size(); i++)
	{
	  pe->coords[i] = &(sdnpatch->controlCoords[a]);
	  pe->indices[i] = i;
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
	      if(!pe->indices)
		pe->indices = (unsigned int*)calloc(1, sizeof(unsigned int));

	      pe->coords[0] = &(sdnpatch->controlCoords[a]);
	      pe->indices[0] = i;

	      pe->num = 1;
	      minDist = curDist;
	    } // if
	  a += 4;
	} // for

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
	      tmpc = (double**)realloc(pe->coords,
			(pe->num+1) * sizeof(double*));
	      if(tmpc)
		pe->coords = tmpc;
	      else
		return AY_EOMEM;

	      pe->coords[pe->num] = c;

	      tmpi = (unsigned int*)realloc(pe->indices,
			(pe->num+1) * sizeof(unsigned int));
	      if(tmpi)
		pe->indices = tmpi;
	      else
		return AY_EOMEM;

	      pe->indices[pe->num] = i;

	      pe->num++;
	    } // if

	  a += 4;
	} // for

      break;
    case 3:
      /* rebuild from o->selp */
      pnt = o->selp;
      lastpnt = &o->selp;
      while(pnt)
	{
	  if(pnt->index < vertices->size())
	    {
	      pnt->point = &(sdnpatch->controlCoords[pnt->index*4]);
	      pnt->rational = AY_TRUE;
	      lastpnt = &(pnt->next);
	      pnt = pnt->next;
	    }
	  else
	    {
	      *lastpnt = pnt->next;
	      free(pnt);
	      pnt = *lastpnt;
	    }
	}
      break;
    default:
      break;
    } // switch

 return AY_OK;
} /* sdnpatch_getpntcb */


/* sdnpatch_addfaces:
 *  helper for sdnpatch_convnp() below
 *  create the faces for an open patch
 *
 *  for a standard 4x4 patch this results in
 *  the following configuration
 *  (vertices 16-35 being so called dummy vertices):
 *
 *   24 25 26 27 28 29
 *   16  0  4  8 12 20 <- ur
 *   17  1  5  9 13 21
 *   18  2  6 10 14 22
 *   19  3  7 11 15 23 <- lr
 *   30 31 32 33 34 35
 *
 * ToDo: add support for degrees other than 3
 */
void
sdnpatch_addfaces(MeshBuilder *meshBuilder,
		  unsigned int width, unsigned int height)
{
 unsigned int i, j, a, b, d;
 unsigned int ur, lr;

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
	} // for

      a++;
      b++;
    } // for

  /* dummy faces */

  /* left side */
  d = width*height;
  a = 0;
  for(j = 0; j < height-1; j++)
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

  /* right side */
  a = (width-1)*height;
  d++;
  ur = d; /* ll is ur-1 */
  for(j = 0; j < height-1; j++)
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

  lr = d;
  d++;
  /* upper side */
  meshBuilder->startFace(4);
  meshBuilder->addToFace(d);
  meshBuilder->addToFace(width*height);
  meshBuilder->addToFace(0);
  d++;
  meshBuilder->addToFace(d);
  meshBuilder->closeFace();

  a = 0;
  for(i = 0; i < width-1; i++)
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
  meshBuilder->addToFace((width-1)*height);
  meshBuilder->addToFace(ur);
  d++;
  meshBuilder->addToFace(d);
  meshBuilder->closeFace();


  /* lower side */
  d++;
  meshBuilder->startFace(4);
  meshBuilder->addToFace(ur-1);
  meshBuilder->addToFace(d);
  d++;
  meshBuilder->addToFace(d);
  meshBuilder->addToFace(height-1);
  meshBuilder->closeFace();

  a = height-1;
  for(i = 0; i < width-1; i++)
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
  meshBuilder->addToFace(width*height-1);
  meshBuilder->addToFace(d);
  d++;
  meshBuilder->addToFace(d);
  meshBuilder->addToFace(lr);
  meshBuilder->closeFace();

 return;
} /* sdnpatch_addfaces */


/* sdnpatch_addfacescu:
 *  helper for sdnpatch_convnp() below
 *  create the faces for a patch which is closed in u direction
 *
 *  for a standard 4x4 patch this results in
 *  the following configuration
 *  (vertices 16-23 being so called dummy vertices):
 *
 *  16 17 18 19
 *   0  4  8 12
 *   1  5  9 13
 *   2  6 10 14
 *   3  7 11 15
 *  20 21 22 23
 *
 * ToDo: add support for degrees other than 3
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
	} // for

      a++;
      b++;
    } // for

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
  /* upper side */
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

  /* lower side */
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
 *  helper for sdnpatch_convnp() below
 *  create the faces for a patch which is closed in v direction
 *
 *  for a standard 4x4 patch this results in
 *  the following configuration
 *  (vertices 16-23 being so called dummy vertices):
 *
 *   16  0  4  8 12 20
 *   17  1  5  9 13 21
 *   18  2  6 10 14 22
 *   19  3  7 11 15 23
 *
 * ToDo: add support for degrees other than 3
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
	} // for

      meshBuilder->startFace(4);
      meshBuilder->addToFace(a);
      meshBuilder->addToFace(a-(height-2));
      meshBuilder->addToFace(b-(height-2));
      meshBuilder->addToFace(b);
      meshBuilder->closeFace();

      a++;
      b++;
    } // for

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
 *  helper for sdnpatch_convnp() below
 *  create the faces for a patch which is closed in both directions
 */
void
sdnpatch_addfacescuv(MeshBuilder *meshBuilder,
		     unsigned int width, unsigned int height)
{
 unsigned int i, j, a, b;

  a = 0;
  b = height-1;

  for(i = 0; i < width-2; i++)
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
	} // for

      meshBuilder->startFace(4);
       meshBuilder->addToFace(a);
       meshBuilder->addToFace(a-(height-2));
       meshBuilder->addToFace(b-(height-2));
       meshBuilder->addToFace(b);
      meshBuilder->closeFace();

      a++;
      b++;
    } // for

  b = 0;
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
    }

  meshBuilder->startFace(4);
   meshBuilder->addToFace(a);
   meshBuilder->addToFace(a-(height-2));
   meshBuilder->addToFace(0);
   meshBuilder->addToFace(b);
  meshBuilder->closeFace();

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
 double *cv = NULL, ulen = 0.0, vlen = 0.0;
 unsigned int i = 0, endi = 0, j = 0, endj = 0, k = 0, a = 0;
 int is_closed_u = AY_FALSE, is_closed_v = AY_FALSE;

  if(!p || !result)
    return AY_ENULL;

  if(p->type != AY_IDNPATCH)
    return AY_ERROR;

  np = (ay_nurbpatch_object *)p->refine;

  if(np->utype == AY_CTCLOSED)
    is_closed_u = AY_TRUE;

  if(np->vtype == AY_CTCLOSED)
    is_closed_v = AY_TRUE;

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
	  meshBuilder->addVertex(cv[a], cv[a+1], cv[a+2], cv[a+3], k);
	  k++;
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

  /*
  ulen = np->uknotv[np->width+np->uorder]-np->uknotv[1];
  for(j = 0; j < np->width; j++)
    {
      meshBuilder->addKnotInterval(j, j+1,
				   (np->uknotv[i-1]-np->uknotv[i])/ulen);
    }
  k = 0;
  for(j = 0; j < np->height; j++)
    {
      meshBuilder->addKnotInterval(k, k+np->width,
				   (np->vknotv[i-1]-np->vknotv[i])/vlen);
    }
  */

  meshBuilder->finishKnotIntervals();

  MeshBuilder::dispose(meshBuilder);

  sdnpatch_getcontrolvertices(sdnpatch);

  newo->modified = AY_TRUE;
  ay_notify_object(newo);

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
	} // while
    } // if

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
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
	} // if

      if(p)
	{
	  ay_status = sdnpatch_convnp(0, p, &newo);
	  if(newo)
	    {
	      ay_object_link(newo);
	    }

	  ay_object_deletemulti(p, AY_FALSE);
	}

      sel = sel->next;
    } // while

  ay_notify_parent();

 return TCL_OK;
} /* sdnpatch_convnptcmd */


/* sdnpatch_convpo:
 *  convert PolyMesh to SDNPatch
 */
int
sdnpatch_convpo(int mode, ay_object *p, ay_object **result)
{
 ay_object *newo = NULL;
 ay_pomesh_object *po = NULL;
 sdnpatch_object *sdnpatch = NULL;
 double *cv = NULL;
 unsigned int i = 0, j = 0, k = 0, m = 0, n = 0, a = 0;
 int stride = 3;

  if(!p || !result)
    return AY_ENULL;

  if(p->type != AY_IDPOMESH)
    return AY_ERROR;

  po = (ay_pomesh_object *)p->refine;

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

  if(po->has_normals)
    {
      stride = 6;
    }

  cv = po->controlv;
  a = 0;
  for(i = 0; i < po->ncontrols; i++)
    {
      meshBuilder->addVertex(cv[a], cv[a+1], cv[a+2], 1.0, k);
      k++;
      a += stride;
    }
  meshBuilder->finishVertices();

  for(i = 0; i < po->npolys; i++)
    {
      if((po->nloops[i] == 1) && (po->nverts[n] == 4))
	{
	  meshBuilder->startFace(po->nverts[n]);
	  for(k = 0; k < po->nverts[n]; k++)
	    {
	      meshBuilder->addToFace(po->verts[m]);
	      m++;
	    }
	  meshBuilder->closeFace();
	  n++;
	}
      else
	{
	  /* just forward m and n */
	  for(j = 0; j < po->nloops[i]; j++)
	    {
	      for(k = 0; k < po->nverts[n]; k++)
		{
		  m++;
		}
	      n++;
	    }
	}
    }

  meshBuilder->finishFaces();

  meshBuilder->finishKnotIntervals();

  MeshBuilder::dispose(meshBuilder);

  sdnpatch_getcontrolvertices(sdnpatch);

  newo->modified = AY_TRUE;
  ay_notify_object(newo);

  /* return result */
  *result = newo;

 return AY_OK;
} /* sdnpatch_convpo */


/* sdnpatch_convpotcmd:
 *  Tcl command to convert PolyMesh objects to SDNPatch objects
 */
int
sdnpatch_convpotcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
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
	} // while
    } // if

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  while(sel)
    {
      o = sel->object;

      if(o->type != AY_IDPOMESH)
	{
	  ay_status = ay_provide_object(o, AY_IDPOMESH, &p);
	}
      else
	{
	  ay_status = ay_object_copy(o, &p);
	} // if

      if(p)
	{
	  ay_status = sdnpatch_convpo(0, p, &newo);
	  if(newo)
	    {
	      ay_object_link(newo);
	    }

	  ay_object_deletemulti(p, AY_FALSE);
	}

      sel = sel->next;
    } // while

  ay_notify_parent();

 return TCL_OK;
} /* sdnpatch_convpotcmd */


/* sdnpatch_impplytcmd:
 *  Tcl command to import PLY files
 */
int
sdnpatch_impplytcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
 int ay_status = AY_OK;
 ay_object *o = NULL;
 sdnpatch_object *sdnpatch = NULL;

  /* check args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "filename");
      return TCL_OK;
    }

  Mesh *controlMesh = new Mesh(3);
  MeshBuilder *meshBuilder = MeshBuilder::create(*controlMesh);
  PlyReader *plyReader = new PlyReader(*meshBuilder, true);

  try
    {
      plyReader->read(argv[1]);
    }
  catch(std::exception &e)
    {
      delete plyReader;
      MeshBuilder::dispose(meshBuilder);
      controlMesh->clear();
      delete controlMesh;
      ay_error(AY_ERROR, argv[0], "Error reading file!");
      ay_error(AY_ERROR, argv[0], argv[1]);
      return TCL_OK;
    }

  if(!(o = (ay_object*)calloc(1, sizeof(ay_object))))
    {
      ay_error(AY_EOMEM, argv[0], NULL);
      return TCL_OK;
    }

  ay_object_defaults(o);
  o->type = sdnpatch_id;

  if(!(sdnpatch = (sdnpatch_object*)calloc(1, sizeof(sdnpatch_object))))
    {
      ay_error(AY_EOMEM, argv[0], NULL);
      return TCL_OK;
    }

  sdnpatch->controlMesh = controlMesh;
  sdnpatch->subdivLevel = 2;
  sdnpatch->subdivDegree = controlMesh->getDegree();

  ay_status = sdnpatch_getcontrolvertices(sdnpatch);

  o->refine = sdnpatch;

  o->modified = AY_TRUE;
  ay_notify_object(o);

  ay_object_link(o);

  ay_notify_parent();

  delete plyReader;
  MeshBuilder::dispose(meshBuilder);

  ay_error(AY_EOUTPUT, argv[0], "Done importing PLY from:");
  ay_error(AY_EOUTPUT, argv[0], argv[1]);

 return TCL_OK;
} /* sdnpatch_impplytcmd */


/* sdnpatch_expplytcmd:
 *  Tcl command to export PLY files
 */
int
sdnpatch_expplytcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
  //int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 sdnpatch_object *sdnpatch = NULL;

  /* check args */
  if(argc < 2)
    {
      ay_error(AY_EARGS, argv[0], "filename");
      return TCL_OK;
    }

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  /* XXXX we currently only work with the first object from the selection */
  /* can PLY files hold more? */
  o = sel->object;

  if(o->type != sdnpatch_id)
    {
      return TCL_OK;
    }

  sdnpatch = (sdnpatch_object*)o->refine;

  std::ofstream file(argv[1]);

  if(file.is_open())
    {
      MeshFlattener *meshFlattener =
	MeshFlattener::create(*sdnpatch->controlMesh);
      meshFlattener->setCompatible(true);

      FlatMeshHandler *handler = new PlyWriter(&file);

      meshFlattener->flatten(*handler);
      delete handler;

      MeshFlattener::dispose(meshFlattener);
      file.close();
      ay_error(AY_EOUTPUT, argv[0], "Done exporting PLY to:");
      ay_error(AY_EOUTPUT, argv[0], argv[1]);
    }
  else
    {
      ay_error(AY_EOPENFILE, argv[0], argv[1]);
    }

 return TCL_OK;
} /* sdnpatch_expplytcmd */


/* sdnpatch_extrudefacetcmd:
 *  Tcl command to extrude faces
 */
int
sdnpatch_extrudefacetcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[])
{
  //int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 sdnpatch_object *sdnpatch = NULL;
 unsigned int i = 0;
 ay_point *newp = NULL;
 vector<unsigned int>::iterator si;
 double length = 1.0, scale = 1.0;

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  if(o->type != sdnpatch_id)
    {
      return TCL_OK;
    }

  if(!o->selp)
    {
      return TCL_OK;
    }

  if(argc > 1)
    {
      Tcl_GetDouble(interp, argv[1], &length);
      if(argc > 2)
	{
	  Tcl_GetDouble(interp, argv[2], &scale);
	}
    }

  sdnpatch = (sdnpatch_object*)o->refine;

  MeshFlattener *meshFlattener =
    MeshFlattener::create(*(sdnpatch->controlMesh));

  FlatMeshHandler *handler = new FaceExtruder(sdnpatch, o->selp,
					      length, scale);

  meshFlattener->flatten(*handler);

  delete sdnpatch->controlMesh;
  sdnpatch->controlMesh = ((FaceExtruder*)handler)->m_newMesh;

  sdnpatch_getcontrolvertices(sdnpatch);

  /* clear old selection, select extruded face(s) */
  ay_selp_clear(o);
  si = ((FaceExtruder*)handler)->m_newSelection.begin();
  for(i = 0; i < ((FaceExtruder*)handler)->m_newSelection.size(); i++)
    {
      if(!(newp = (ay_point*)calloc(1, sizeof(ay_point))))
	{
	  ay_error(AY_EOMEM, argv[0], NULL);
	  return TCL_OK;
	}

      newp->next = o->selp;
      o->selp = newp;
      newp->index = ((*sdnpatch->controlVertices)[*si])->getID();
      si++;
    }

  ay_pact_getpoint(3, o, NULL, NULL);

  o->modified = AY_TRUE;
  ay_notify_object(o);

  ay_notify_parent();

  /* clean up */
  delete handler;
  MeshFlattener::dispose(meshFlattener);

 return TCL_OK;
} /* sdnpatch_extrudefacetcmd */


/* sdnpatch_removefacetcmd:
 *  Tcl command to remove faces
 */
int
sdnpatch_removefacetcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[])
{
  //int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 sdnpatch_object *sdnpatch = NULL;

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  if(o->type != sdnpatch_id)
    {
      return TCL_OK;
    }

  if(!o->selp)
    {
      return TCL_OK;
    }

  sdnpatch = (sdnpatch_object*)o->refine;

  MeshFlattener *meshFlattener =
    MeshFlattener::create(*(sdnpatch->controlMesh));
  FaceRemover *fr_handler = new FaceRemover(sdnpatch, o->selp);

  meshFlattener->flatten(*fr_handler);

  if(fr_handler->m_newMesh)
    {
      delete sdnpatch->controlMesh;
      sdnpatch->controlMesh = fr_handler->m_newMesh;

      sdnpatch_getcontrolvertices(sdnpatch);

      ay_selp_clear(o);

      o->modified = AY_TRUE;
      ay_notify_object(o);

      ay_notify_parent();
    }
  else
    {
      ay_error(AY_ERROR, argv[0], "Could not remove face!");
    }

  delete fr_handler;
  MeshFlattener::dispose(meshFlattener);

 return TCL_OK;
} /* sdnpatch_removefacetcmd */


/* sdnpatch_mergefacetcmd:
 *  Tcl command to merge faces
 */
int
sdnpatch_mergefacetcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[])
{
  //int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 sdnpatch_object *sdnpatch = NULL;

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  if(o->type != sdnpatch_id)
    {
      return TCL_OK;
    }

  if(!o->selp)
    {
      return TCL_OK;
    }

  sdnpatch = (sdnpatch_object*)o->refine;

  MeshFlattener *meshFlattener =
    MeshFlattener::create(*(sdnpatch->controlMesh));
  FaceMerger *fm_handler = new FaceMerger(sdnpatch, o->selp);
  meshFlattener->flatten(*fm_handler);

  if(fm_handler->m_error)
    {
      if(fm_handler->m_newMesh)
	delete fm_handler->m_newMesh;
      ay_error(AY_ERROR, argv[0], "Face merge failed!");
      goto cleanup;
    }

  delete sdnpatch->controlMesh;
  sdnpatch->controlMesh = fm_handler->m_newMesh;

  delete fm_handler;
  MeshFlattener::dispose(meshFlattener);

  sdnpatch_getcontrolvertices(sdnpatch);

  ay_selp_clear(o);

  o->modified = AY_TRUE;
  ay_notify_object(o);

  ay_notify_parent();

cleanup:

 return TCL_OK;
} /* sdnpatch_mergefacetcmd */


/* sdnpatch_connectfacetcmd:
 *  Tcl command to connect faces
 */
int
sdnpatch_connectfacetcmd(ClientData clientData, Tcl_Interp *interp,
			 int argc, char *argv[])
{
  //int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 sdnpatch_object *sdnpatch = NULL;

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  if(o->type != sdnpatch_id)
    {
      return TCL_OK;
    }

  if(!o->selp)
    {
      return TCL_OK;
    }

  sdnpatch = (sdnpatch_object*)o->refine;

  MeshFlattener *meshFlattener =
    MeshFlattener::create(*(sdnpatch->controlMesh));

  FaceConnector *fc_handler = new FaceConnector(sdnpatch, o->selp);

  meshFlattener->flatten(*fc_handler);

  if(fc_handler->m_error)
    {
      if(fc_handler->m_newMesh)
	delete fc_handler->m_newMesh;
      ay_error(AY_ERROR, argv[0], "Face connect failed!");
      goto cleanup;
    }

  delete sdnpatch->controlMesh;
  //  sdnpatch->controlMesh = ((FaceConnector*)fc_handler)->m_newMesh;
  sdnpatch->controlMesh = fc_handler->m_newMesh;

  delete fc_handler;
  MeshFlattener::dispose(meshFlattener);

  sdnpatch_getcontrolvertices(sdnpatch);

  ay_selp_clear(o);

  o->modified = AY_TRUE;
  ay_notify_object(o);

  ay_notify_parent();

cleanup:

 return TCL_OK;
} /* sdnpatch_connectfacetcmd */


/* sdnpatch_editknottcmd:
 *  Tcl command to edit knots
 */
int
sdnpatch_editknottcmd(ClientData clientData, Tcl_Interp *interp,
		      int argc, char *argv[])
{
  //int ay_status = AY_OK;
 ay_point *oldselp = NULL;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 sdnpatch_object *sdnpatch = NULL;
 int i = 1;
 int mode = 0;
 char *varname = NULL;
 Tcl_Obj *to = NULL, *ton = NULL;
 KnotPrecision interval = 1.0;
 static vector<KnotInterval *> selectedKnots;
 vector<KnotInterval *>::iterator ki;

  /* parse args */
  if(argc > 1)
    {
      while(i < argc)
	{
	  if(!strcmp(argv[i], "-s"))
	    {
	      mode = 0; // start
	      if(i+1 < argc)
		{
		  varname = argv[i+1];
		}
	      selectedKnots.clear();
	      i--;
	    }
	  if(!strcmp(argv[i], "-r"))
	    {

	      mode = 2; // reset
	      i--;
	    }
	  if(!strcmp(argv[i], "-i"))
	    {
	      mode = 1; // set new interval
	      if(i+1 < argc)
		{
		  sscanf(argv[i+1], "%g", &interval);
		}
	    }
	  i += 2;
	} // while
    } // if

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  if(o->type != sdnpatch_id)
    {
      return TCL_OK;
    }

  // in reset mode, save original point selection
  // and select all points instead
  if(mode == 2)
    {
      oldselp = o->selp;
      o->selp = NULL;
      ay_selp_selall(o);
    }

  if(!o->selp)
    {
      return TCL_OK;
    }

  sdnpatch = (sdnpatch_object*)o->refine;

  MeshFlattener *meshFlattener =
    MeshFlattener::create(*(sdnpatch->controlMesh));
  KnotSelector *ks_handler = new KnotSelector(sdnpatch, o->selp,
					      meshFlattener, &selectedKnots);
  meshFlattener->flatten(*ks_handler);

  delete ks_handler;
  MeshFlattener::dispose(meshFlattener);

  switch(mode)
    {
    case 0:
      // start
      if(varname)
	{
	  if(selectedKnots.size() > 0)
	    {
	      interval = selectedKnots[0]->getInterval();
	      ton = Tcl_NewStringObj(varname, -1);
	      to = Tcl_NewDoubleObj(interval);
	      Tcl_ObjSetVar2(interp, ton, NULL, to,
			     TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY);
	      Tcl_IncrRefCount(ton);Tcl_DecrRefCount(ton);
	    }
	}
      else
	{
	  ay_error(AY_ERROR, argv[0],
		   "could not find interval; invalid edge selected?");
	}
      break;
    case 1:
      // set interval
      for(ki = selectedKnots.begin(); ki != selectedKnots.end(); ki++)
	{
	  (*ki)->setInterval(interval);
	}
      break;
    case 2:
      // reset all
      for(ki = selectedKnots.begin(); ki != selectedKnots.end(); ki++)
	{
	  (*ki)->setInterval(1.0);
	}
      // establish saved point selection
      ay_selp_clear(o);
      o->selp = oldselp;
      break;
    default:
      break;
    }

  o->modified = AY_TRUE;
  ay_notify_object(o);

  ay_notify_parent();

 return TCL_OK;
} /* sdnpatch_editknottcmd */


/* sdnpatch_reverttcmd:
 *  Tcl command to remove faces
 */
int
sdnpatch_reverttcmd(ClientData clientData, Tcl_Interp *interp,
		    int argc, char *argv[])
{
  //int ay_status = AY_OK;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL;
 sdnpatch_object *sdnpatch = NULL;

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  o = sel->object;

  if(o->type != sdnpatch_id)
    {
      return TCL_OK;
    }

  sdnpatch = (sdnpatch_object*)o->refine;

  MeshFlattener *meshFlattener =
    MeshFlattener::create(*(sdnpatch->controlMesh));
  Revertor *r_handler = new Revertor(sdnpatch);

  meshFlattener->flatten(*r_handler);

  if(r_handler->m_newMesh)
    {
      delete sdnpatch->controlMesh;
      sdnpatch->controlMesh = r_handler->m_newMesh;

      sdnpatch_getcontrolvertices(sdnpatch);

      ay_selp_clear(o);

      o->modified = AY_TRUE;
      ay_notify_object(o);

      ay_notify_parent();
    }
  else
    {
      ay_error(AY_ERROR, argv[0], "Could not revert!");
    }

  delete r_handler;
  MeshFlattener::dispose(meshFlattener);

 return TCL_OK;
} /* sdnpatch_reverttcmd */


/* sdnpatch_mergepatchtcmd:
 *  Tcl command to merge multiple sdnpatches into one
 */
int
sdnpatch_mergepatchtcmd(ClientData clientData, Tcl_Interp *interp,
			int argc, char *argv[])
{
 int ay_status = AY_OK;
 unsigned int degree = 0, level = 0;
 ay_list_object *sel = ay_selection;
 ay_object *o = NULL, *c = NULL;
 sdnpatch_object *sdnpatch = NULL;
 Mesh *newMesh = NULL;
 PatchMerger *pm_handler = NULL;
 MeshFlattener *meshFlattener = NULL;
 unsigned int i, a;
 double m[16] = {0};
 std::vector<Vertex*> *vertices = NULL;
 Vertex *v = NULL;

  /* check selection */
  if(!sel)
    {
      ay_error(AY_ENOSEL, argv[0], NULL);
      return TCL_OK;
    }

  pm_handler = new PatchMerger();

  while(sel)
    {
      o = sel->object;

      if(o->type != sdnpatch_id)
	{
	  sel = sel->next;
	  continue;
	}
      c = NULL;
      if(AY_ISTRAFO(o))
	{
	  ay_status = ay_object_copy(o, &c);
	  if(!ay_status && c)
	    {
	      ay_trafo_creatematrix(c, m);

	      sdnpatch = (sdnpatch_object*)c->refine;
	      sdnpatch_getcontrolvertices(sdnpatch);

	      vertices = sdnpatch->controlVertices;
	      a = 0;
	      for(i = 0; i < vertices->size(); i++)
		{
		  ay_trafo_apply4(&(sdnpatch->controlCoords[a]), m);
		  v = (*(sdnpatch->controlVertices))[i];
		  v->setX(sdnpatch->controlCoords[a]);
		  v->setY(sdnpatch->controlCoords[a+1]);
		  v->setZ(sdnpatch->controlCoords[a+2]);
		  v->setW(sdnpatch->controlCoords[a+3]);
		  a += 4;
		}
	    }
	}
      else
	{
	  sdnpatch = (sdnpatch_object*)o->refine;
	}

      if(degree == 0)
	{
	  degree = sdnpatch->subdivDegree;
	  level = sdnpatch->subdivLevel;
	}
      else
	{
	  if(degree != sdnpatch->subdivDegree)
	    {
	      if(c)
		ay_object_deletemulti(c, AY_FALSE);
	      sel = sel->next;
	      continue;
	    }
	}

      meshFlattener = MeshFlattener::create(*(sdnpatch->controlMesh));

      if(meshFlattener)
	{
	  pm_handler->addPatch();
	  meshFlattener->flatten(*pm_handler);

	  MeshFlattener::dispose(meshFlattener);
	}
      else
	{
	  ay_error(AY_EOMEM, argv[0], NULL);
	  if(c)
	    ay_object_deletemulti(c, AY_FALSE);
	  goto cleanup;
	}

      if(c)
	ay_object_deletemulti(c, AY_FALSE);

      sel = sel->next;
    } /* while */

  newMesh = pm_handler->buildMesh(degree);

  if(newMesh)
    {
      if(!(o = (ay_object*)calloc(1, sizeof(ay_object))))
	{
	  ay_error(AY_EOMEM, argv[0], NULL);
	  goto cleanup;
	}
      o->type = sdnpatch_id;
      ay_object_defaults(o);

      if(!(sdnpatch = (sdnpatch_object*)calloc(1, sizeof(sdnpatch_object))))
	{
	  free(o);
	  ay_error(AY_EOMEM, argv[0], NULL);
	  goto cleanup;
	}

      o->refine = sdnpatch;
      sdnpatch->subdivDegree = degree;
      sdnpatch->subdivLevel = 2;

      sdnpatch->controlMesh = newMesh;
      newMesh = NULL;

      sdnpatch_getcontrolvertices(sdnpatch);

      ay_object_link(o);

      o->modified = AY_TRUE;
      ay_notify_object(o);

      ay_notify_parent();
      newMesh = NULL;
    }
  else
    {
      ay_error(AY_ERROR, argv[0], "Could not merge patches!");
    }

cleanup:

  if(newMesh)
    delete newMesh;
  delete pm_handler;

 return TCL_OK;
} /* sdnpatch_mergepatchtcmd */


/* sdnpatch_getcontrolvertices:
 *  get adress and content of all control vertices
 *  (for selection and editing)
 */
int
sdnpatch_getcontrolvertices(sdnpatch_object *sdnpatch)
{
 std::vector<Vertex*>::iterator it;
 std::vector<Vertex*> *tmp = NULL;
 Vertex *v;
 unsigned int a = 0;

  if(!sdnpatch || !sdnpatch->controlMesh)
    return AY_ENULL;

  if(sdnpatch->controlVertices)
    {
      delete sdnpatch->controlVertices;
      sdnpatch->controlVertices = NULL;
    }

  if(sdnpatch->controlCoords)
    {
      free(sdnpatch->controlCoords);
      sdnpatch->controlCoords = NULL;
    }

  tmp = sdnpatch->controlMesh->getVertexPointers();

  if(tmp)
    {
      sdnpatch->controlVertices = new std::vector<Vertex *>;
      sdnpatch->controlVertices->resize(tmp->size());

      // correct order for our own ID scheme
      for(it = tmp->begin();
	  it != tmp->end();
	  it++)
	{
	  (*sdnpatch->controlVertices)[(*it)->getID()] = *it;
	}

      if(!(sdnpatch->controlCoords = (double*)calloc(
		sdnpatch->controlVertices->size(), 4*sizeof(double))))
	{
	  return AY_EOMEM;
	}

      a = 0;
      for(it = sdnpatch->controlVertices->begin();
	  it != sdnpatch->controlVertices->end();
	  it++)
	{
	  v = *it;
	  sdnpatch->controlCoords[a]   = v->getX();
	  sdnpatch->controlCoords[a+1] = v->getY();
	  sdnpatch->controlCoords[a+2] = v->getZ();
	  sdnpatch->controlCoords[a+3] = v->getW();

	  a += 4;
	}
      delete tmp;
    } // if

 return AY_OK;
} /* sdnpatch_getcontrolvertices */

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

  if(ay_checkversion(fname, sdnpatch_version_ma, sdnpatch_version_mi))
    {
      return TCL_ERROR;
    }

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

  ay_status += ay_notify_register(sdnpatch_notifycb, sdnpatch_id);

  ay_status += ay_convert_register(sdnpatch_convertcb, sdnpatch_id);

  ay_status += ay_provide_register(sdnpatch_providecb, sdnpatch_id);

  if(ay_status)
    {
      ay_error(AY_ERROR, fname, "Error registering custom object!");
      return TCL_OK;
    }

  // create new Tcl commands to interface with the plugin
  Tcl_CreateCommand(interp, "sdnconvertNP", (Tcl_CmdProc*) sdnpatch_convnptcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "sdnconvertPO", (Tcl_CmdProc*) sdnpatch_convpotcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "sdnimpPly", (Tcl_CmdProc*) sdnpatch_impplytcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "sdnexpPly", (Tcl_CmdProc*) sdnpatch_expplytcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "sdnextrudeFace",
		    (Tcl_CmdProc*) sdnpatch_extrudefacetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "sdnremoveFace",
		    (Tcl_CmdProc*) sdnpatch_removefacetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "sdnrevert",
		    (Tcl_CmdProc*) sdnpatch_reverttcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "sdnmergeFace",
		    (Tcl_CmdProc*) sdnpatch_mergefacetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "sdnmergePatch",
		    (Tcl_CmdProc*) sdnpatch_mergepatchtcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "sdnconnectFace",
		    (Tcl_CmdProc*) sdnpatch_connectfacetcmd,
		    (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "sdneditKnots",
		    (Tcl_CmdProc*) sdnpatch_editknottcmd,
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
