#ifndef GMATERIAL_H
#define GMATERIAL_H

#include "gcolor.h"
#include "gray.h"

class GMaterial
{
public:
    virtual ~GMaterial() = default;
    virtual void ComputeScatteringFunctions(GMath::GSurfaceInteraction* isect) = 0;
};


class GLambertianMaterial : public GMaterial
{
public:
    GLambertianMaterial(const GFColor& Kd)
        :Kd(Kd)
    {}

    void ComputeScatteringFunctions(GMath::GSurfaceInteraction* isect);

private:
    GFColor Kd;
};

#endif // GMATERIAL_H
