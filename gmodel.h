#ifndef GMODEL_H
#define GMODEL_H

#include "gmath.h"
#include "ggameobject.h"
#include "gray.h"
#include "gaabb.h"
#include "gmaterial.h"

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

class GShape : public GHittable
{
public:
    virtual GSurfaceInteraction Sample(const GGameObject& owner, float& pdf, double time) const = 0;
    virtual GSurfaceInteraction Sample(const GGameObject& owner, const GSurfaceInteraction& ref, float& pdf) const = 0;

    aabb bBox() const override
    {
        return bbox;
    }

    aabb bbox;
};

class GShapeList : public GShape
{
public:
    GSurfaceInteraction Sample(const GGameObject& owner, float& pdf, double time) const override;
    GSurfaceInteraction Sample(const GGameObject& owner, const GSurfaceInteraction& ref, float& pdf) const override;
    bool intersect(const GRay& ray, GMath::interval ray_t, GSurfaceInteraction& isect) override;

    void clear()
    {
        shapes.clear();
        bbox = aabb();
    }

    void add(std::shared_ptr<GShape> shape){
        shapes.push_back(shape);
        bbox = aabb(bbox, shape->bBox());
    }
    std::vector<std::shared_ptr<GShape>> shapes;
};

class GSphere : public GShape
{
public:
    GSphere(vec3 center, double radius)
        :center(center), radius(radius), isMoving(false)
    {
        auto halfSize = vec3(radius, radius, radius);
        bbox = aabb(center - halfSize, center + halfSize);
    }
    GSphere(vec3 center, vec3 desCenter, double radius)
        :center(center), radius(radius), isMoving(true)
    {
        auto halfSize = vec3(radius, radius, radius);
        aabb bbox0(center-halfSize, center+halfSize);
        aabb bbox1(desCenter-halfSize, desCenter+halfSize);
        bbox = aabb(bbox0, bbox1);
        moveDir = desCenter - center;
    }

    bool intersect(const GRay& ray, GMath::interval ray_t, GSurfaceInteraction& isect) override;
    GSurfaceInteraction Sample(const GGameObject& owner, float& pdf, double time) const override;
    GSurfaceInteraction Sample(const GGameObject& owner, const GSurfaceInteraction& ref, float& pdf) const override;

    vec3 sphereCenter(vec3 center, double time) const
    {
        if(isMoving)
        {
            return center + moveDir*time;
        }
        return center;
    }

    static GMath::vec2 getUV(const vec3&p);

    vec3 center;
    double radius;
    vec3 moveDir;
    bool isMoving;
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
    aabb bBox() const override
    {
        return shape->bBox() + (vec3)_position;
    }
private:
    std::shared_ptr<GMaterial> material;
};

#endif // GMODEL_H
