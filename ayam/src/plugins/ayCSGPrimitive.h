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
// ayCSGPrimitive.h - OpenCSG-Ayam-Wrapper
//

#include <GL/glew.h>
#include "ayam.h"
#include <opencsg.h>

namespace OpenCSG {

    class  ayCSGPrimitive : public Primitive {
    public:
        ayCSGPrimitive(ay_object *ayobject, struct Togl *togl,
		       Operation, unsigned int convexity);

        void setAyObject(ay_object *);
        ay_object *getAyObject() const;

	void setTogl(struct Togl *);
        struct Togl *getTogl() const;

        virtual void render();

    private:
        ay_object *ayobject_;
        struct Togl *togl_;
    };

} // namespace OpenCSG

extern char *aycsg_tm_tagtype;
