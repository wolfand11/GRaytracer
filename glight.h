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
    GMath::GAABB<double> bBox() override
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
        material = make_shared<GLambertianMaterial>(GColor::blackF);
    }

    GFColor Le(const GScene& scene, const GSurfaceInteraction& isect, const GMath::vec3& w) override;
    GFColor Sample_Li(const GScene& scene, const GSurfaceInteraction& isect, GMath::vec3& wi, float& pdf) override;
    aabb bBox() override
    {
        return shape->bBox() + (vec3)_position;
    }

    bool twoSided;
    std::shared_ptr<GShape> shape;
    std::shared_ptr<GMaterial> material;
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

class GMeshLight : public GDiffuseAreaLight
{
public:
    template<typename ShapeT>
    static std::vector<std::shared_ptr<GGameObject>> CreateLights(const std::string objModelPath, GFColor lColor=GColor::whiteF, bool twoSided=true)
    {
        auto objModel = make_shared<GOBJModel>(objModelPath);
        return CreateMeshLights(objModel, lColor, twoSided);
    }
    template<typename ShapeT>
    static std::vector<std::shared_ptr<GGameObject>> CreateLights(std::shared_ptr<GOBJModel> objModel, GFColor lColor=GColor::whiteF, bool twoSided=true)
    {
        std::vector<std::shared_ptr<GGameObject>> lights;
        for(int i=0; i<objModel->nfaces(); i++)
        {
            auto tShape = std::make_shared<ShapeT>(objModel, i);
            auto light = std::make_shared<GMeshLight>(tShape, lColor, twoSided);
            tShape->owner = light.get();
            lights.push_back(light);
        }
        return lights;
    }
    GMeshLight(std::shared_ptr<GShape> shape, GFColor lColor=GColor::whiteF, bool twoSided=true)
        :GDiffuseAreaLight(shape, lColor, twoSided)
    {
    }
    bool intersect(const GRay& ray, GMath::interval ray_t, GSurfaceInteraction& isect) override;
};
#endif // GLIGHT_H
