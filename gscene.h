#ifndef GSCENE_H
#define GSCENE_H

#include "ggameobject.h"
#include "gcamera.h"
#include "gmodel.h"
#include "gbuffer.h"
#include "glight.h"
#include "gbvh.h"

class GScene
{
public:
    GScene(int filmW, int filmH)
        :film(filmW, filmH)
    {
    }

    bool intersect(const GRay& ray, GMath::interval& ray_t, GSurfaceInteraction& isect) const
    {
        if(bvhTree!=nullptr)
        {
            if(bvhTree->intersect(ray, ray_t, isect))
            {
                ray_t.max = isect.t;
                return true;
            }
            return false;
        }
        else
        {
            bool hitAny = false;
            for(auto light : lights)
            {
                if(light->intersect(ray, ray_t, isect))
                {
                    ray_t.max = isect.t;
                    hitAny = true;
                }
            }
            for(auto model : models)
            {
                if(model->intersect(ray, ray_t, isect))
                {
                    ray_t.max = isect.t;
                    hitAny = true;
                }
            }
            return hitAny;
        }
    }

    void add(std::shared_ptr<GGameObject> gObj)
    {
        if(gObj->mtype == GGameObject::kModel)
        {
            auto model = std::dynamic_pointer_cast<GModel>(gObj);
            models.push_back(model);
            bbox = aabb(bbox, model->bBox());
            hittables.push_back(model);
        }
        else if(gObj->mtype == GGameObject::kLight)
        {
            auto light = std::dynamic_pointer_cast<GLight>(gObj);
            lights.push_back(std::dynamic_pointer_cast<GLight>(gObj));

            if(light->lightType == GLightType::kLTDiffuseArea)
            {

                auto light = std::dynamic_pointer_cast<GDiffuseAreaLight>(gObj);
                bbox = aabb(bbox, light->bBox());
                hittables.push_back(light);
            }
        }
    }

    void add(std::vector<std::shared_ptr<GGameObject> >& gObjList)
    {
        for(auto& gobj : gObjList)
        {
            add(gobj);
        }
    }

    aabb bBox()
    {
        return bbox;
    }

    void BuildBVHTree()
    {
        bvhTree = make_shared<GBVHNode>(hittables, 0, hittables.size());
    }

    std::shared_ptr<GCamera> camera;
    GColorBuffer film;
    std::vector<std::shared_ptr<GModel>> models;
    std::vector<std::shared_ptr<GLight>> lights;
    std::vector<std::shared_ptr<GHittable>> hittables;

private:
    std::shared_ptr<GBVHNode> bvhTree;
    GMath::aabb bbox;
};

#endif // GSCENE_H
