#ifndef GSCENE_H
#define GSCENE_H

#include "ggameobject.h"
#include "gcamera.h"
#include "gmodel.h"
#include "gbuffer.h"

class GScene
{
public:
    GScene(int filmW, int filmH)
        :film(filmW, filmH)
    {
    }

    std::shared_ptr<GCamera> camera;
    std::vector<std::shared_ptr<GModel>> models;
    std::vector<std::shared_ptr<GLight>> lights;

    GColorBuffer film;
};

#endif // GSCENE_H
