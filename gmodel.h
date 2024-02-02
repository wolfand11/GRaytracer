#ifndef GMODEL_H
#define GMODEL_H

#include "gmath.h"
#include "ggameobject.h"
#include "gray.h"
#include "gaabb.h"
#include "gmaterial.h"
#include "gshape.h"
#include "gobjmodel.h"

using GMath::vec3;
using GMath::aabb;

enum GModelType
{
    kMTInvalid,
    kMTTriange,
    kMTCube,
    kMTSphere,
    kMTObj,
};

class GModel : public GGameObject, public GHittable
{
public:
    GModel(std::shared_ptr<GShape>shape)
        :GGameObject(GGameObjectType::kModel), shape(shape)
    {
        shape->owner = this;
    }
    bool intersect(const GRay& ray, GMath::interval ray_t, GSurfaceInteraction& isect) = 0;
    aabb bBox();

    std::shared_ptr<GShape> shape;
};

class GSphereModel : public GModel
{
public:
    GSphereModel(double radius, std::shared_ptr<GMaterial> material)
        :GModel(std::make_shared<GSphere>(0, radius)), material(material)
    {}

    GSphereModel(vec3 desCenter, double radius, std::shared_ptr<GMaterial> material)
        :GModel(std::make_shared<GSphere>(0, desCenter, radius)), material(material)
    {}

    bool intersect(const GRay& ray, GMath::interval ray_t, GSurfaceInteraction& isect) override;
    aabb bBox() override
    {
        return shape->bBox() + (vec3)_position;
    }
private:
    std::shared_ptr<GMaterial> material;
};

class GTriangleModel : public GModel
{
public:
    static std::vector<std::shared_ptr<GTriangleModel>> CreateTriangleMesh(const std::string objModelPath, std::shared_ptr<GMaterial> material);
    static std::vector<std::shared_ptr<GTriangleModel>> CreateTriangleMesh(std::shared_ptr<GOBJModel> objModel, std::shared_ptr<GMaterial> material);
    GTriangleModel(std::shared_ptr<GShape> shape, std::shared_ptr<GMaterial> material)
        :GModel(shape), material(material)
    {}

    using GModel::bBox;
    bool intersect(const GRay& ray, GMath::interval ray_t, GSurfaceInteraction& isect) override;
private:
    std::shared_ptr<GMaterial> material;
};
#endif // GMODEL_H
