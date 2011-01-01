// -*- Mode: c++ -*-
// $Id: trimesh.cpp,v 1.1 2011/01/01 16:48:38 randolf Exp $
// $Source: /home/randi/bak/ayam/src/plugins/subdivide/src/trimesh.cpp,v $

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

#include "trimesh.h"
#include "subtri.h"
#include "tagmesh.h"

TriMesh::TriMesh() 
{ _triTagMesh = new TagMeshTp<Tri>(); }

TriMesh::~TriMesh() { 
  if(_triTagMesh)
    delete _triTagMesh; 
}

TriMesh::TriMesh(Vertex *cv, uint nfaces,
		 uint *nverts, uint *verts) 
{
  _triTagMesh = new TagMeshTp<Tri>(cv, nfaces, nverts, verts);
}

TriMesh::TriMesh(const TriMesh& m) { 
  _triTagMesh = new TagMeshTp<Tri>(); 
  _triTagMesh->operator=(*(m._triTagMesh));
}

TriMesh& TriMesh::operator=(const TriMesh& m) { 
  _triTagMesh->operator=(*(m._triTagMesh));
  return (*this);
}

TriMesh* TriMesh::clone() const{
  TriMesh* nm = new TriMesh();
  nm->setClone(*this);
  return nm;
}

void TriMesh::setClone(const TriMesh& m) 
{ _triTagMesh->setClone(*(m._triTagMesh)); }

int TriMesh::meshDepth() const
{ return _triTagMesh->meshDepth(); }

void TriMesh::toAyam(double **cv, unsigned int *cvlen, unsigned int **nverts,
		      unsigned int **verts, unsigned int *nfaces) {
  _triTagMesh->toAyam(cv, cvlen, nverts, verts, nfaces);
}

void TriMesh::subdivide(int maxl)
{ _triTagMesh->subdivide(maxl); }

void TriMesh::midsub(int maxl)
{ _triTagMesh->midsub(maxl); }


TriMesh* TriMesh::leafMesh() const {
  
  TagMeshTp<Tri>* tm = new TagMeshTp<Tri>();

  TagMeshTp<Tri>::FaceIterType fi;
  for(fi = _triTagMesh->faceBegin(); fi != _triTagMesh->faceEnd(); ++fi) {
    if((*fi)->isLeaf())
      tm->insertFace(*fi);
  }
  
  TriMesh* m = new TriMesh();
  m->_triTagMesh = tm;
  return m;
}

int TriMesh::numberOfVertices() const 
{ return _triTagMesh->vertVector().size(); }

const cvec3f& TriMesh::getVertexPos(uint index) const { 
  assert(index < _triTagMesh->vertVector().size());
  return _triTagMesh->vertVector()[index]->getPos(0);
}


void TriMesh::setVertexPos(uint index, const cvec3f& p) { 
  assert(index < _triTagMesh->vertVector().size());
  _triTagMesh->vertVector()[index]->setPos(0, p);
  _triTagMesh->vertVector()[index]->set(0);

}
