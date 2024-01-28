#include "gmodel.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "glog.h"
#include "gutils.h"
using namespace GMath;
using namespace std;

bool GSphere::intersect(const GMath::GRay &ray, GMath::interval ray_t, GMath::GSurfaceInteraction &isect)
{
    vec3 oc = ray.origin - center;
    auto a = ray.dir.length2();
    auto half_b = dot(oc, ray.dir);
    auto c = oc.length2() - radius * radius;

    auto discriminant = half_b * half_b - a * c;
    if(discriminant < 0)
    {
        return false;
    }

    auto sqrtd = sqrt(discriminant);
    auto root = (-half_b - sqrtd) / a;

    if(!ray_t.surrounds(root))
    {
        root = (-half_b + sqrtd) / a;
        if(!ray_t.surrounds(root))
        {
            return false;
        }
    }
    isect.t = root;
    isect.p = ray.GetPos(root);
    vec3 outNormal = (isect.p - center)/radius;
    isect.SetFaceNormal(ray, outNormal);
    isect.wo = -ray.dir;
    return true;
}

GModel::GModel() :GGameObject(GGameObjectType::kModel)
{
}

bool GSphereModel::intersect(const GMath::GRay &ray, GMath::interval ray_t, GMath::GSurfaceInteraction &isect)
{
    sphere.center = _position;
    if(sphere.intersect(ray, ray_t, isect))
    {
        isect.mateiral = mateiral;
        return true;
    }
    return false;
}
