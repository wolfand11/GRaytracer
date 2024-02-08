#include "gmodel.h"
#include "gtriangle.h"
#include "gquad.h"
using namespace GMath;
using namespace std;

aabb GModel::bBox()
{
    aabb localBBox = shape->bBox();

    const mat4f* obj2World; // obj2World
    const mat4f* world2Obj; // world2Obj
    TRSInvertTRS(obj2World, world2Obj);

    aabb wBBox = transformBBox(localBBox, *obj2World);
    return wBBox;
}

bool GSphereModel::intersect(const GRay &ray, GMath::interval ray_t, GSurfaceInteraction &isect)
{
    auto sphere = std::dynamic_pointer_cast<GSphere>(shape);
    sphere->center = _position;
    if(sphere->intersect(ray, ray_t, isect))
    {
        isect.model = this;
        isect.material = material.get();
        isect.material->UpdateShadingNormal(isect);
        isect.light = nullptr;
        return true;
    }
    return false;
}

bool GMeshModel::intersect(const GRay &ray, interval ray_t, GSurfaceInteraction &isect)
{
    if(shape->intersect(ray, ray_t, isect))
    {
        isect.model = this;
        isect.material = material.get();
        isect.material->UpdateShadingNormal(isect);
        isect.light = nullptr;
        return true;
    }
    return false;
}
