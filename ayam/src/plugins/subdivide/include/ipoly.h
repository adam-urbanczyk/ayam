// -*- Mode: c++ -*-
// $Id: ipoly.h,v 1.1 2011/01/01 16:48:38 randolf Exp $
// $Source: /home/randi/bak/ayam/src/plugins/subdivide/include/ipoly.h,v $

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

#ifndef	__IPOLY_H__
#define	__IPOLY_H__

#include "compat.h"

//: Intermediate polygon representation
// A class supporting a number of utility functions for 
// operations with polygons represented as subsequences 
// of vertex indices in an array of indices representing a mesh
// (used for flat meshes).

//TODO: rename to IFace

class IPoly { 
public: 
  IPoly( uint si = -1, uint novtx = 0): _sindex(si), _novtx(novtx) { }
  
  // number of vertices
  uint novtx() const { return _novtx; }

  // the first entry of a polygon
  uint start() const { return _sindex;}

  // move to the next vertex (represented by number in the polygon) 
  uint NextV(uint vno) const { 
    assert(vno < _novtx); 
    return (vno + 1) % _novtx; 
  }

  // move to the previous vertex (represented by number in the polygon) 
  uint PrevV(uint vno) const { 
    assert(vno < _novtx); 
    return (vno + _novtx - 1) % _novtx; 
  }

  // move to the next vertex (represented by number in the whole mesh array )
  uint iNextVi( uint vi) const { 
    assert(vi >= _sindex && vi < _sindex + _novtx); 
    return NextV(vi - _sindex ) + _sindex;
  }

  // move to the previous vertex (represented by number in the whole mesh array )
  uint iPrevVi( uint vi) const { 
    assert(vi >= _sindex && vi < _sindex + _novtx); 
    return PrevV(vi - _sindex ) + _sindex;
  }

  // takes polygon vertex index and returns next vertex as index into vertex array
  uint iNextV( uint vno) const { 
    assert(vno < _novtx );
    return NextV(vno ) + _sindex;
  }

  // takes polygon vertex index and returns previous vertex as index into vertex array
  uint iPrevV( uint vno) const { 
    assert(vno < _novtx );
    return PrevV(vno) + _sindex;
  }

  // increment the number of vertices in this polygon
  void incnovtx() { _novtx++; }

private:
  uint _sindex;  // index of the first vertex in the array of indices
  uint _novtx;   // number of vertices 
};


#endif /* __IPOLY_H__ */




