//
// ayCSGPrimitive.h - OpenCSG-Ayam-Wrapper
//

#include <GL/glew.h>
#include "ayam.h"
#include <opencsg.h>

namespace OpenCSG {

    class  ayCSGPrimitive : public Primitive {
    public:
        ayCSGPrimitive(ay_object *ayobject, Operation, unsigned int convexity);

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

