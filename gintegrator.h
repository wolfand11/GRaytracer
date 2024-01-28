#ifndef GINTEGRATOR_H
#define GINTEGRATOR_H

#include "gscene.h"
#include "gray.h"

class GIntegrator
{
public:
    virtual ~GIntegrator(){}
    virtual void Render(GScene &scene);
    virtual GFColor Li(GMath::GRay& ray, GScene& scene, int depth)=0;
    GFColor SampleLight(const GScene& scene, const GMath::GSurfaceInteraction& isect, GFColor& beta);

    int spp = 1;
    int maxDepth = 1;
};

class GWhittedIntegrator: public GIntegrator
{
public:
    GWhittedIntegrator() {}

    GFColor Li(GMath::GRay& ray, GScene& scene, int depth) override;
    GFColor SpecularReflect(const GMath::GRay& ray, const GMath::GSurfaceInteraction& isect, GScene& scene, int depth);
};

class GPathIntegrator: public GIntegrator
{
 public:
    GPathIntegrator(float rrThreshold=1.0)
         :rrThreshold(rrThreshold)
    {}

    GFColor Li(GMath::GRay& ray, GScene& scene, int depth) override;

    float rrThreshold;
};

#endif // GINTEGRATOR_H
