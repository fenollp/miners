#pragma once

#include <Algo3DSphere.hh>

class Algo3DSphereFull : public Algo3DSphere {
public:
    Algo3DSphereFull() {}
    virtual ~Algo3DSphereFull() {}

    virtual bool apply(GLfloat* vertices, GLfloat* colors, VertIndices& selected,
                       size_t width, size_t height, size_t depth);
};
