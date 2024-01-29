#ifndef GINTEGRATOR_H
#define GINTEGRATOR_H

#include "gscene.h"
#include "gray.h"

class GIntegrator
{
public:
    GIntegrator(float rrThreshold=1.0)
         :rrThreshold(rrThreshold)
    {}
    virtual ~GIntegrator(){}
    virtual void Render(GScene &scene);
    virtual GFColor Li(GMath::GRay& ray, GScene& scene, int depth);
    GFColor SampleLight(const GScene& scene, const GMath::GSurfaceInteraction& isect);

    int spp = 1;
    int maxDepth = 1;
    float rrThreshold;
};

#endif // GINTEGRATOR_H
