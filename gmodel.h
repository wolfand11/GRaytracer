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

class GMeshModel : public GModel
{
public:
    template<typename ShapeT>
    static std::vector<std::shared_ptr<GGameObject>> CreateMesh(const std::string objModelPath, std::shared_ptr<GMaterial> material)
    {
        auto objModel = make_shared<GOBJModel>(objModelPath);
        return CreateMesh<ShapeT>(objModel, material);
    }
    template<typename ShapeT>
    static std::vector<std::shared_ptr<GGameObject>> CreateMesh(std::shared_ptr<GOBJModel> objModel, std::shared_ptr<GMaterial> material)
    {
        auto gray = std::make_shared<GSolidColor>(GColor::grayF);
        std::vector<std::shared_ptr<GGameObject>> models;
        std::shared_ptr<GTexture> diffuse = material->Kd;
        if(diffuse==nullptr)
        {
            if(objModel->diffusemap_.get_width() > 0)
            {
                diffuse = make_shared<GImageTexture>(objModel->diffusemap_);
            }
            else
            {
                diffuse = gray;
            }
        }
        std::shared_ptr<GTexture> normal = material->normal;
        if(normal == nullptr)
        {
            if(objModel->normalmap_.get_width() > 0)
            {
                normal = make_shared<GImageTexture>(objModel->normalmap_);
            }
        }
        material->Init(diffuse, gray, gray, normal);
        for(int i=0; i<objModel->nfaces(); i++)
        {
            auto tShape = std::make_shared<ShapeT>(objModel, i);
            auto model = std::make_shared<GMeshModel>(tShape, material);
            tShape->owner = model.get();
            models.push_back(model);
        }
        return models;
    }

    GMeshModel(std::shared_ptr<GShape> shape, std::shared_ptr<GMaterial> material)
        :GModel(shape), material(material)
    {}

    bool intersect(const GRay& ray, GMath::interval ray_t, GSurfaceInteraction& isect) override;
protected:
    std::shared_ptr<GMaterial> material;
};
#endif // GMODEL_H
