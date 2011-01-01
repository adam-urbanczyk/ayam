// -*- Mode: c++ -*-
// $Id: quadmesh.h,v 1.1 2011/01/01 16:48:38 randolf Exp $
// $Source: /home/randi/bak/ayam/src/plugins/subdivide/include/quadmesh.h,v $

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

#ifndef __QUADMESH_H__
#define __QUADMESH_H__

#include "compat.h"
#include "cvec3t.h"
#include "vertex.h"

// the face class for quad meshes -- internal
class Quad;

// template for the mesh representation -- internal
template<class Quad> class TagMeshTp;

// face manipulation interface (defined separately)
class PickableQuad;

// wrapper class for quad-based hierarchical meshes

class QuadMesh {
  friend class PickableQuad;
public:
  QuadMesh();
  virtual ~QuadMesh();

  // construct from flat data (from Ayam)
  QuadMesh(Vertex *cv, uint nfaces, uint *nverts, uint *verts);

  // construct from other mesh, share vertices and faces
  QuadMesh(const QuadMesh& m);

  // assignment, share vertices and faces
  QuadMesh& operator=(const QuadMesh& m);

  // create a new mesh, clone vertices and faces
  QuadMesh* clone() const;

  // set the current mesh to be the clone of mesh m
  void setClone(const QuadMesh& m);

  // return a mesh containing just the leaf faces
  QuadMesh* leafMesh() const;

  // depth of the mesh
  int meshDepth() const;

  // allow access to subdivided data
  void toAyam(double **cv, unsigned int *cvlen, unsigned int **nverts,
	      unsigned int **verts, unsigned int *nfaces);

  // midpoint subdivide until level maxl
  // maxl == -1: just recompute current vertex position
  void midsub(int maxl = -1);

  // extended Catmull-Clark subdivision until level maxl
  // maxl == -1: just recompute current vertex position
  void subdivide(int maxl = -1);

  // number of toplevel vertices
  int numberOfVertices() const ;

  // get position of vertex #index
  const cvec3f& getVertexPos(uint index) const;

  // set position of vertex #incex
  void setVertexPos(uint index, const cvec3f& p);

  TagMeshTp<Quad>*& quadTagMesh() { return _quadTagMesh; }
private:
  TagMeshTp<Quad>* _quadTagMesh;
};

#endif /* __QUADMESH_H__ */
