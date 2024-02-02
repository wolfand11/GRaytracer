#ifndef GSHAPE_H
#define GSHAPE_H

#include "gray.h"
#include "gaabb.h"

class GShape : public GHittable
{
public:
    virtual GSurfaceInteraction Sample(float& pdf, double time) const = 0;
    virtual GSurfaceInteraction Sample(const GSurfaceInteraction& ref, float& pdf) const = 0;

    aabb bBox()override
    {
        return bbox;
    }

    aabb bbox;
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
    GSurfaceInteraction Sample(float& pdf, double time) const override;
    GSurfaceInteraction Sample(const GSurfaceInteraction& ref, float& pdf) const override;

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

#endif // GSHAPE_H
