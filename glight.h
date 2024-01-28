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
class GLight : public GGameObject
{
public:
    // light
    GLight(GLightType lightType, GFColor lColor=GColor::whiteF);

    virtual bool intersect(const GMath::GRay& ray, GMath::interval ray_t, GMath::GSurfaceInteraction& isect)
    {
        return false;
    }
    virtual GFColor Le(const GMath::GRay& ray)
    {
        return GColor::blackF;
    }
    virtual GFColor Le(const GScene& scene, const GMath::GSurfaceInteraction& isect, const GMath::vec3& w);
    virtual GFColor Sample_Li(const GScene& scene, const GMath::GSurfaceInteraction& isect, GMath::vec3& wi, float* pdf);

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

    GFColor Sample_Li(const GScene& scene, const GMath::GSurfaceInteraction& isect, GMath::vec3& wi, float* pdf) override;
};

class GPointLight: public GLight
{
public:
    GPointLight(GFColor lColor=GColor::whiteF)
        :GLight(GLightType::kLTPoint, lColor)
    {
    }

    GFColor Sample_Li(const GScene& scene, const GMath::GSurfaceInteraction& isect, GMath::vec3& wi, float* pdf) override;
};


class GSkyLight : public GLight
{
public:
    GSkyLight()
        :GLight(GLightType::kLTSky)
    {
    }
    GFColor Le(const GMath::GRay& ray) override;
};

class GDiffuseAreaLight : public GLight
{
public:
    GDiffuseAreaLight(std::shared_ptr<GShape> shape, bool twoSided=true, GFColor lColor=GColor::whiteF)
        :GLight(GLightType::kLTDiffuseArea, lColor),twoSided(twoSided),shape(shape)
    {
    }

    GFColor Le(const GScene& scene, const GMath::GSurfaceInteraction& isect, const GMath::vec3& w) override;

    bool twoSided;
    std::shared_ptr<GShape> shape;
};

class GSphereLight : public GDiffuseAreaLight
{
public:
    GSphereLight(double radius, bool twoSided=true, GFColor lColor=GColor::whiteF)
        :GDiffuseAreaLight(std::make_shared<GSphere>(0, radius),twoSided, lColor)
    {
    }

    bool intersect(const GMath::GRay& ray, GMath::interval ray_t, GMath::GSurfaceInteraction& isect) override;
};


#endif // GLIGHT_H
