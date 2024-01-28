#ifndef GRAY_H
#define GRAY_H

#include "gmath.h"
#include "gcolor.h"

class GModel;
class GMaterial;
class GBSDF;
class GLight;
class GScene;

namespace GMath
{
struct GRay
{
public:
    GRay(): origin(0,0,0),dir(0,0,0) {};
    GRay(vec3 origin, vec3 dir);

    vec3 GetPos(float t) const;

    vec3 origin;
    vec3 dir;
};

struct GRayDifferential
{
public:
    GRayDifferential(){};
    GRayDifferential(vec3 originX, vec3 dirX, vec3 originY, vec3 dirY);

    GRay xRay;
    GRay yRay;
};

class GSurfaceInteraction
{
public:
    vec3 p;
    vec3 normal;
    vec3 wo;
    double t;
    bool isFrontFace;
    std::shared_ptr<GMaterial> mateiral;
    std::shared_ptr<GModel> model = nullptr;
    std::shared_ptr<GBSDF> bsdf = nullptr;
    std::shared_ptr<GLight> light = nullptr;

    void SetFaceNormal(const GRay& r, const vec3& outNormal)
    {
        isFrontFace = dot(r.dir, outNormal) < 0;
        normal = isFrontFace ? outNormal : -outNormal;
    }

    void ComputeScatteringFunctions();
    GFColor Le(const GScene& scene, const vec3& w) const;
};
}

#endif // GRAY_H
