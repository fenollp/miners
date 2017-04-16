#pragma once

#include <Algo3DCube.hh>

class Algo3DCubeContiRainbow : public Algo3DCube {
public:
    Algo3DCubeContiRainbow() {}
    virtual ~Algo3DCubeContiRainbow() {}

    virtual bool apply(Floats& vertices, Floats& colors, VertIndices& selected,
                       size_t width, size_t height, size_t depth);
};
