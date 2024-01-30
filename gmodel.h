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
    virtual GMath::GSurfaceInteraction Sample(const GGameObject& owner, float& pdf) const = 0;
    virtual GMath::GSurfaceInteraction Sample(const GGameObject& owner, const GMath::GSurfaceInteraction& ref, float& pdf) const = 0;
};

class GSphere : public GShape
{
public:
    GSphere(vec3 center, double radius)
        :center(center), radius(radius)
    {}

    bool intersect(const GMath::GRay& ray, GMath::interval ray_t, GMath::GSurfaceInteraction& isect) override;
    GMath::GSurfaceInteraction Sample(const GGameObject& owner, float& pdf) const override;
    GMath::GSurfaceInteraction Sample(const GGameObject& owner, const GMath::GSurfaceInteraction& ref, float& pdf) const override;

    vec3 center;
    double radius;
};

class GModel : public GGameObject
{
public:
    GModel();
    virtual bool intersect(const GMath::GRay& ray, GMath::interval intervalV, GMath::GSurfaceInteraction& isect) = 0;

protected:
};

class GSphereModel : public GModel
{
public:
    GSphereModel(double radius, std::shared_ptr<GMaterial> material)
        :sphere(0, radius), material(material)
    {}

    bool intersect(const GMath::GRay& ray, GMath::interval ray_t, GMath::GSurfaceInteraction& isect) override;
private:
    GSphere sphere;
    std::shared_ptr<GMaterial> material;
};

#endif // GMODEL_H
