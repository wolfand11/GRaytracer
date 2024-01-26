#ifndef GRAY_H
#define GRAY_H

#include "gmath.h"

namespace GMath
{
struct GRay
{
public:
    GRay(): origin(0,0,0),dir(0,0,0) {};
    GRay(vec3 origin, vec3 dir);

    vec3 GetPoint(float t);

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

struct GHitInfo
{
public:
    vec3 p;
    vec3 normal;
    double t;
    bool isFrontFace;

    void SetFaceNormal(const GRay& r, const vec3& outNormal)
    {
        isFrontFace = dot(r.dir, outNormal) < 0;
        normal = isFrontFace ? outNormal : -outNormal;
    }
};
}

#endif // GRAY_H
