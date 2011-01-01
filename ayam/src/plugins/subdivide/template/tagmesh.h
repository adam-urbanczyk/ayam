// -*- Mode: c++ -*-
// $Id: tagmesh.h,v 1.1 2011/01/01 16:48:38 randolf Exp $
// $Source: /home/randi/bak/ayam/src/plugins/subdivide/template/tagmesh.h,v $
/* Subdivide V2.0
   Copyright (C) 2000 Henning Biermann, Denis Zorin, NYU

This file is part of Subdivide.

Subdivide is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Subdivide is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Subdivide; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#ifndef __TAGMESH_H__
#define __TAGMESH_H__

#include "mesh.h"
#include "sectorinfo.h"

template<class Face>
class TagMeshTp : public MeshTp<Face> {
public:

  typedef MeshTp<Face> MeshFace;
  typedef typename MeshFace::FaceIterType FaceIterType;
  typedef typename MeshFace::EdgeType EdgeType;
  typedef typename MeshFace::VertexType VertexType;
  typedef typename MeshFace::FaceType FaceType;
  typedef typename MeshFace::TLFaceType TLFaceType;
  typedef typename MeshFace::FaceRingType FaceRingType;
 


  TagMeshTp() { ; }
  TagMeshTp(Vertex *cv, uint nfaces, uint *nverts,
	    uint *verts) : MeshTp<Face>(cv, nfaces, nverts, verts) { ; }

  TagMeshTp* clone() const;
  TagMeshTp* clone(map<Vertex*, Vertex*>& vvMap, map<FaceType*, FaceType*>& ttMap) const;
  void setClone(const TagMeshTp& m, map<Vertex*, Vertex*>& vvMap, map<FaceType*, FaceType*>& ttMap);
  void setClone(const TagMeshTp& m);
  
  void midsub(int maxl = -1) {
    FaceIterType it;
    
    if(maxl == -1) {
      FaceType* f = (*this->faceBegin(0));
      maxl = 0;
      while(f->childCount() != 0) {
	f = f->child(0);
	++maxl;
      }
    }
      
    for(it = this->faceBegin(0); it != this->faceEnd(0); ++it)
      (*it)->clearFace(0);
    
    for(int l = 0; l < maxl; ++l) {
      for(it = this->faceBegin(l); (it != this->faceEnd(l)); ++it) 
      if(it.depth() == l) { 
	(*it)->midSub(l);
      }
    }
  }

  void subdivide(int maxl = -1) {
    FaceIterType it;
    
    for(it = this->faceBegin(maxl); it != this->faceEnd(maxl); ++it) {
      (*it)->clearFace(0);
      (*it)->clearNormal();
    }
    
    if(maxl == -1) {
      FaceType* f = (*this->faceBegin(0));
      maxl = 0;
      while(f->childCount() != 0) {
	f = f->child(0);
	++maxl;
      }
    }

    for(int l = 0; l < maxl; ++l) {
      for(it = this->faceBegin(l); (it != this->faceEnd(l)); ++it) 
	if(it.depth() == l)
	  (*it)->subdivide(l);
    }
    
    for(it = this->faceBegin(maxl); it != this->faceEnd(maxl); ++it) 
      for(EnoType e = 1; e < (*it)->noVtx()+1; ++e) 
	//	if((*it)->isLeaf())
	if(it.depth() == maxl)
	  (*it)->computeNormalAndLimit(it.depth());
  }

protected:
  void cleanup();

private:

  typedef map<Vertex*, int> VertexMapType;
  typedef map<Vertex*, EdgeType> EMapType;
  typedef pair<Vertex*, Vertex*> VertPairType;
  typedef map<VertPairType, EdgeType> EdgeMapType;
  typedef typename Face::VertexTagType VertexTagType;

  void fixBoundaryTag();
  void fixSector();


  void cloneTag(map<FaceType*, FaceType*>& ttMap);

  void fixVertexToggle(FaceType* f, EnoType e) {
    typename Face::FaceRingType fr;
    fr.collectRing(f, e);
    int creaseCount = 0;
    int i;
    for(i = 0; i < fr.noVtx(); ++i) {
      EnoType ringEno;
      FaceType* ringFace = fr.face(i, ringEno); 
      if(ringFace->edgeTag(ringEno) == Face::CREASE_EDGE) 
	++creaseCount;
    }
    assert(f->headVert(e) == fr.centerVert());
    if(creaseCount < 2) 
      ((TLFaceType*)f)->setVertexTag(f->headVno(e), FaceType::NOTAG_VERTEX);
    else if(creaseCount == 2) 
      ((TLFaceType*)f)->setVertexTag(f->headVno(e), FaceType::CREASE_VERTEX);      
    else
      ((TLFaceType*)f)->setVertexTag(f->headVno(e), FaceType::CORNER_VERTEX);      

    for(i = 0; i < fr.noFace(); ++i) {
      EnoType ringEno;
      FaceType* ringFace = fr.face(i, ringEno); 
      ((TLFaceType*)ringFace)->setSectorInfo(ringFace->tailVno(ringEno), 0);
    }
    
    VertexTagType vertexTag = f->vertexTag(f->headVno(e));
    if((vertexTag == FaceType::CREASE_VERTEX) ||
       (vertexTag == FaceType::CORNER_VERTEX))
      for(i = 0; i < fr.noFace(); ++i) {
	EnoType ringEno;
	FaceType* ringFace = fr.face(i, ringEno); 
	SectorInfo* si = 0;
	assert(ringFace->tailVert(ringEno) == fr.centerVert());
	if((si = ringFace->sectorInfo(ringFace->tailVno(ringEno))) == 0) {
	  si = new SectorInfo();
	  if(vertexTag == FaceType::CORNER_VERTEX)
	    si->setSectorTag(SectorInfo::CONVEX_SECTOR);
	  ((TLFaceType*)ringFace)->setSectorInfo(ringFace->tailVno(ringEno), si);
	}
      }
  }

  void specializeEdge(FaceType* f, EnoType e) {
    EnoType he, te;
    if(!f->isLeaf()) {
      assert(f->headVert(e)->isSpecial());
      assert(f->tailVert(e)->isSpecial());
      f->midVert(e)->makeSpecial();
      FaceType* hf = f->headSubEdge(e, he);
      FaceType* tf = f->tailSubEdge(e, te);
      specializeEdge(tf, te);
      specializeEdge(hf, he);
    }
  }

  EdgeMapType _edgeMap;
  map<SectorInfo*, EdgeType> _sectorMap;
};



template<class Face>
void TagMeshTp<Face>::fixSector() {
  FaceIterType fi;
  for(fi = this->faceBegin(); fi != this->faceEnd(); ++fi) {
    for(EnoType e = 1; e < (*fi)->noVtx()+1; ++e) {
      TLFaceType* f = (TLFaceType*)(*fi);
      if((f->vertexTag(f->headVno(e)) == Face::CREASE_VERTEX) &&
	 (f->sectorInfo(f->headVno(e)) == 0)) {

	SectorInfo* si = new SectorInfo();
	si->setSectorTag(SectorInfo::NOTAG_SECTOR);
	f->setSectorInfo(f->headVno(e), si);
      }
      if((f->vertexTag(f->headVno(e)) == Face::CORNER_VERTEX) &&
	 (f->sectorInfo(f->headVno(e)) == 0)) {

	SectorInfo* si = new SectorInfo();
	si->setSectorTag(SectorInfo::CONVEX_SECTOR);
	f->setSectorInfo(f->headVno(e), si);
      }
      if((f->sectorTag(f->headVno(e)) != SectorInfo::NOTAG_SECTOR) &&
	 (f->vertexTag(f->headVno(e)) == Face::NOTAG_VERTEX)) {

      }
    }
  }
}

template<class Face>
void TagMeshTp<Face>::fixBoundaryTag() 
{    
    // fix edges
    FaceIterType fi;
    for(fi = this->faceBegin(); fi != this->faceEnd(); ++fi) 
    {
        for(EnoType e = 1; e < (*fi)->noVtx()+1; ++e) 
        {
            TLFaceType* f = (TLFaceType*)(*fi);
            EnoType ne;
            FaceType* nf = f->neighbor(e, ne);
            if(nf == 0) 
            {
                if(f->edgeTag(e) == Face::NOTAG_EDGE) 
                {                    
                    f->setEdgeTag(e, Face::CREASE_EDGE);
                }
            }
        }
    }
    
    for(fi = this->faceBegin(); fi != this->faceEnd(); ++fi) 
    {
        for(EnoType e = 1; e < (*fi)->noVtx()+1; ++e) 
        {
            FaceType* f = (*fi);
            EnoType ne;
            FaceType* nf = f->neighbor(e, ne);
            assert(nf || (f->edgeTag(e) == Face::CREASE_EDGE));
        }
    }
    
    // fix boundary vertices
    for(fi = this->faceBegin(); fi != this->faceEnd(); ++fi) 
    {
        for(EnoType e = 1; e < (*fi)->noVtx()+1; ++e) 
        {
            TLFaceType* f = (TLFaceType*)(*fi);
            if((f->edgeTag(e) == Face::CREASE_EDGE) && 
                (f->vertexTag(f->headVno(e)) == Face::NOTAG_VERTEX)) 
            {
                typename Face::FaceRingType fr;
                fr.collectRing(f, e);
                int incedentCreaseCount = 0;
                for(uint i = 0; i < fr.noVtx(); ++i) 
                {
                    EnoType ringE;
                    FaceType* ringF = fr.face(i, ringE);
                    if(ringF->edgeTag(ringE) == Face::CREASE_EDGE)
                        ++incedentCreaseCount;
                }
                if(incedentCreaseCount == 1) 
                {
                    ;
                } else if(incedentCreaseCount == 2) 
                {
                    f->setVertexTag(f->headVno(e), Face::CREASE_VERTEX);
                } 
                else if(incedentCreaseCount > 2) 
                {
                    f->setVertexTag(f->headVno(e), Face::CORNER_VERTEX);
                }
            }
        }
    }
}


template<class Face>
TagMeshTp<Face>* TagMeshTp<Face>::clone() const {
  map<Vertex*, Vertex*> vvMap;
  map<FaceType*, FaceType*> ttMap;
  return clone(vvMap, ttMap);
}

template<class Face>
TagMeshTp<Face>* TagMeshTp<Face>::clone(map<Vertex*, Vertex*>& vvMap, map<FaceType*, FaceType*>& ttMap) const {
  TagMeshTp* m = new TagMeshTp();
  m->setClone(*this, vvMap, ttMap);
  return m;
}

template<class Face>
void TagMeshTp<Face>::setClone(const TagMeshTp& m, map<Vertex*, Vertex*>& vvMap, map<FaceType*, FaceType*>& ttMap) {
  if(this != &m) {
    MeshTp<Face>::setClone(m, vvMap, ttMap);
    cloneTag(ttMap);
  }
}

template<class Face>
void TagMeshTp<Face>::setClone(const TagMeshTp& m) {
  if(this != &m) {
    map<Vertex*, Vertex*> vvMap;
    map<FaceType*, FaceType*> ttMap;
    MeshTp<Face>::setClone(m, vvMap, ttMap);
    cloneTag(ttMap);
  }
}

template<class Face>
void TagMeshTp<Face>::cloneTag(map<FaceType*, FaceType*>& ttMap) {
  typename map<FaceType*, FaceType*>::iterator it;
  for(it = ttMap.begin(); it != ttMap.end(); ++it) {
    FaceType* fOrig = (*it).first;
    TLFaceType* fClone = (TLFaceType*)((*it).second);
    for(EnoType e = 1; e < fOrig->noVtx()+1; ++e) {
      VnoType headVno = fOrig->headVno(e);
      if(fOrig->edgeTag(e) != Face::NOTAG_EDGE)
	fClone->setEdgeTag(e, fOrig->edgeTag(e));
      typename Face::VertexTagType tag = fOrig->vertexTag(headVno);
      if(tag != Face::NOTAG_VERTEX)
	fClone->setVertexTag(headVno, tag);
    }
  }

  // apply sectors...
  for(it = ttMap.begin(); it != ttMap.end(); ++it) {
    FaceType* fOrig = (*it).first;
    TLFaceType* fClone = (TLFaceType*)((*it).second);
    for(EnoType e = 1; e < fOrig->noVtx()+1; ++e) {
      VnoType headVno = fOrig->headVno(e);
      if((fOrig->sectorInfo(headVno) != 0) &&
	 (fClone->sectorInfo(headVno) == 0)) {
	fClone->setSectorInfo(headVno, fOrig->sectorInfo(headVno)->clone());
	if(fClone->sectorInfo(headVno)->sectorTag() != SectorInfo::NOTAG_SECTOR) {
	  assert(fOrig->vertexTag(headVno) != FaceType::NOTAG_VERTEX);
	  assert(fClone->vertexTag(headVno) != FaceType::NOTAG_VERTEX);
	}
	  
      }
    }
  }

  fixBoundaryTag();
  fixSector();
}


#endif /* __TAGMESH_H__ */
