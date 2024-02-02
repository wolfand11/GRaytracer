#ifndef GRAY_H
#define GRAY_H

#include "gmath.h"
#include "gcolor.h"

using namespace GMath;

class GModel;
class GMaterial;
class GBSDF;
class GLight;
class GScene;
class GGameObject;

namespace GMath
{
template <typename T> struct GAABB;
}

struct GRay
{
public:
    GRay(): origin(0,0,0),dir(0,0,0),time(0) {};
    GRay(vec3 origin, vec3 dir, double time=0);

    vec3 GetPos(float t) const;

    vec3 origin;
    vec3 dir;
    double time;
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
    vec3 wi;
    vec2 uv;
    double t;
    double time = 0;
    bool isFrontFace;
    GMaterial* material = nullptr;
    GModel* model = nullptr;
    GLight* light = nullptr;

    void SetFaceNormal(const GRay& r, const vec3& outNormal)
    {
        isFrontFace = dot(r.dir, outNormal) < 0;
        normal = isFrontFace ? outNormal : -outNormal;
    }

    GFColor Le(const GScene& scene, const vec3& w) const;
};

class GHittable
{
public:
    virtual ~GHittable() = default;

    virtual bool intersect(const GRay& ray, GMath::interval ray_t, GSurfaceInteraction& isect) = 0;

    virtual GMath::GAABB<double> bBox() = 0;

    GGameObject* owner;
};

#endif // GRAY_H
