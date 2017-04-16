#pragma once

#include <Algo3DSphere.hh>

class Algo3DSphereContiRainbow : public Algo3DSphere {
public:
    Algo3DSphereContiRainbow() {}
    virtual ~Algo3DSphereContiRainbow() {}

    virtual bool apply(Floats& vertices, Floats& colors, VertIndices& selected,
                       size_t width, size_t height, size_t depth);
};
