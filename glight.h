#ifndef GLIGHT_H
#define GLIGHT_H

#include "gcolor.h"
#include "ggameobject.h"
#include "gmodel.h"

enum GLightType
{
    kLTDirection,
    kLTPoint,
    kLTDiffuseArea,
    kLTSky,
};

class GScene;
class GLight : public GGameObject, public GHittable
{
public:
    // light
    GLight(GLightType lightType, GFColor lColor=GColor::whiteF);

    bool intersect(const GRay& ray, GMath::interval ray_t, GSurfaceInteraction& isect) override
    {
        return false;
    }
    GMath::GAABB<double> bBox() const override
    {
        return aabb();
    }
    virtual GFColor Le(const GRay& ray)
    {
        return GColor::blackF;
    }
    virtual GFColor Le(const GScene& scene, const GSurfaceInteraction& isect, const GMath::vec3& w);
    virtual GFColor Sample_Li(const GScene& scene, const GSurfaceInteraction& isect, GMath::vec3& wi, float& pdf);

    GFColor lightColor;
    GLightType lightType;
};

class GDirectionalLight: public GLight
{
public:
    GDirectionalLight(GFColor lColor=GColor::whiteF)
        :GLight(GLightType::kLTDirection, lColor)
    {
    }

    GFColor Sample_Li(const GScene& scene, const GSurfaceInteraction& isect, GMath::vec3& wi, float& pdf) override;
};

class GPointLight: public GLight
{
public:
    GPointLight(GFColor lColor=GColor::whiteF)
        :GLight(GLightType::kLTPoint, lColor)
    {
    }

    GFColor Sample_Li(const GScene& scene, const GSurfaceInteraction& isect, GMath::vec3& wi, float& pdf) override;
};


class GSkyLight : public GLight
{
public:
    GSkyLight()
        :GLight(GLightType::kLTSky)
    {
    }
    GFColor Le(const GRay& ray) override;
};

class GDiffuseAreaLight : public GLight
{
public:
    GDiffuseAreaLight(std::shared_ptr<GShape> shape, GFColor lColor=GColor::whiteF, bool twoSided=true)
        :GLight(GLightType::kLTDiffuseArea, lColor),twoSided(twoSided),shape(shape)
    {
        shape->owner = this;
    }

    GFColor Le(const GScene& scene, const GSurfaceInteraction& isect, const GMath::vec3& w) override;
    GFColor Sample_Li(const GScene& scene, const GSurfaceInteraction& isect, GMath::vec3& wi, float& pdf) override;
    aabb bBox() const override
    {
        return shape->bBox() + (vec3)_position;
    }

    bool twoSided;
    std::shared_ptr<GShape> shape;
};

class GSphereLight : public GDiffuseAreaLight
{
public:
    GSphereLight(double radius, GFColor lColor=GColor::whiteF, bool twoSided=true)
        :GDiffuseAreaLight(std::make_shared<GSphere>(0, radius), lColor, twoSided)
    {
    }

    bool intersect(const GRay& ray, GMath::interval ray_t, GSurfaceInteraction& isect) override;
};


#endif // GLIGHT_H
