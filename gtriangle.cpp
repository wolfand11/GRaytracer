#include "gtriangle.h"
#include "ggameobject.h"

GSurfaceInteraction GTriangle::Sample(const GGameObject &owner, float &pdf, double time) const
{
    GSurfaceInteraction intr;
    return intr;
}

GSurfaceInteraction GTriangle::Sample(const GGameObject &owner, const GSurfaceInteraction &ref, float &pdf) const
{
    GSurfaceInteraction intr;
    return intr;
}

bool GTriangle::intersect(const GRay &ray, interval ray_t, GSurfaceInteraction &isect)
{
    const mat4f* trsMat; // obj2World
    const mat4f* invertTRSMat; // world2Obj
    owner->TRSInvertTRS(trsMat, invertTRSMat);

    vec4 tmpRayO = vec4::one;
    tmpRayO.SetXYZ(ray.origin);
    auto localRayO = (*invertTRSMat) * (vec4f)tmpRayO;
    vec4 tmpRayDir = vec4::zero;
    auto localRayDir = (*invertTRSMat) * (vec4f)tmpRayDir;
    double rayScale = localRayDir.length() / ray.dir.length();

    //auto DoN = dot(geoNormal)
    return false;
}
