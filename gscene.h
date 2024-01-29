#ifndef GSCENE_H
#define GSCENE_H

#include "ggameobject.h"
#include "gcamera.h"
#include "gmodel.h"
#include "gbuffer.h"
#include "glight.h"

class GScene
{
public:
    GScene(int filmW, int filmH)
        :film(filmW, filmH)
    {
    }

    bool intersect(const GMath::GRay& ray, GMath::interval& ray_t, GMath::GSurfaceInteraction& isect) const
    {
        for(auto model : models)
        {
            if(model->intersect(ray, ray_t, isect))
            {
                return true;
            }
        }
        return false;
    }

    std::shared_ptr<GCamera> camera;
    std::vector<std::shared_ptr<GModel>> models;
    std::vector<std::shared_ptr<GLight>> lights;

    GColorBuffer film;
};

#endif // GSCENE_H
