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

//
// ayCSGPrimitive.cpp - OpenCSG-Ayam-Wrapper
// this file implements a wrapper class derived from OpenCSG::Primitive
// to shade Ayam objects

#include "ayCSGPrimitive.h"

namespace OpenCSG {

    ayCSGPrimitive::ayCSGPrimitive(ay_object *ayobject, Operation o,
				   unsigned int c)
        : Primitive(o, c)
    {
        ayobject_ = ayobject;
    }

    void ayCSGPrimitive::setAyObject(ay_object *ayobject) {
        ayobject_ = ayobject;
    }

    ay_object *ayCSGPrimitive::getAyObject() const {
        return ayobject_;
    }

    void ayCSGPrimitive::setTogl(struct Togl *togl) {
        togl_ = togl;
    }

    struct Togl *ayCSGPrimitive::getTogl() const {
        return togl_;
    }

    void ayCSGPrimitive::render() {
      ay_shade_object(togl_, ayobject_, AY_FALSE);
    }

} // namespace OpenCSG
