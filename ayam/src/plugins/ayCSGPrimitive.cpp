/*
 * Ayam, a free 3D modeler for the RenderMan interface.
 *
 * Ayam is copyrighted 1998-2004 by Randolf Schultz
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

    ayCSGPrimitive::ayCSGPrimitive(ay_object *ayobject, struct Togl *togl,
				   Operation o, unsigned int c)
        : Primitive(o, c)
    {
        ayobject_ = ayobject;
	togl_ = togl;
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
      int has_tm = AY_FALSE, cw = AY_TRUE;
      GLint ff;

      if(ayobject_->tags && (ayobject_->tags->type == aycsg_tm_tagtype))
	{
	  has_tm = AY_TRUE;
	  glPushMatrix();
	  glMultMatrixd((GLdouble*)(ayobject_->tags->val));
	  if(ayobject_->tags->name)
	    {
	      glGetIntegerv(GL_FRONT_FACE, &ff);
	      if(ff == GL_CW)
		{
		  glFrontFace(GL_CCW);
		  cw = AY_FALSE;
		}
	      else
		{
		  glFrontFace(GL_CW);
		} // if

	    } // if

	} // if

      ay_shade_object(togl_, ayobject_, AY_FALSE);

      if(has_tm)
	{
	  if(ayobject_->tags->name)
	    {
	      if(cw)
		{
		  glFrontFace(GL_CCW);
		}
	      else
		{
		  glFrontFace(GL_CW);
		} // if

	    } // if

	  glPopMatrix();

	} // if

    } // render()

} // namespace OpenCSG
