#ifndef GMODEL_H
#define GMODEL_H

#include "gmath.h"
#include "ggameobject.h"
#include "gray.h"
#include "gmaterial.h"

using GMath::vec3;

enum GModelType
{
    kMTInvalid,
    kMTTriange,
    kMTCube,
    kMTSphere,
    kMTObj,
};

class GShape
{
public:
    GShape()=default;
    ~GShape(){}
    virtual bool intersect(const GMath::GRay& ray, GMath::interval intervalV, GMath::GSurfaceInteraction& isect) = 0;
};

class GSphere : public GShape
{
public:
    GSphere(vec3 center, double radius)
        :center(center), radius(radius)
    {}

    bool intersect(const GMath::GRay& ray, GMath::interval ray_t, GMath::GSurfaceInteraction& isect) override;

    vec3 center;
    double radius;
};

class GModel : public GGameObject
{
public:
    GModel();
    virtual bool intersect(const GMath::GRay& ray, GMath::interval intervalV, GMath::GSurfaceInteraction& isect) = 0;
};

class GSphereModel : public GModel
{
public:
    GSphereModel(double radius, std::shared_ptr<GMaterial> mateiral)
        :sphere(0, radius), mateiral(mateiral)
    {}

    bool intersect(const GMath::GRay& ray, GMath::interval ray_t, GMath::GSurfaceInteraction& isect) override;
private:
    GSphere sphere;
    std::shared_ptr<GMaterial> mateiral;
};

#endif // GMODEL_H
