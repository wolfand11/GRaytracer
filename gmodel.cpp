#include "gmodel.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "glog.h"
#include "gutils.h"
#include "gsampler.h"
#include "gmathutils.h"
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

GSurfaceInteraction GSphere::Sample(const GGameObject& owner, float &pdf) const
{
    vec3 pObj = GSampler::UniformSampleSphere() * radius;
    GSurfaceInteraction intr;
    vec3 pCenter = owner.position();
    intr.p = pCenter + pObj;
    intr.normal = pObj.normalize();
    pdf = 1/(4*M_PI*radius*radius);
    return intr;
}

GSurfaceInteraction GSphere::Sample(const GGameObject& owner, const GMath::GSurfaceInteraction &ref, float &pdf) const
{
    vec3 pCenter = owner.position();
    auto dir = pCenter - ref.p;
    if(dir.length2() <= radius*radius)
    {
        GSurfaceInteraction intr = Sample(owner, pdf);
        vec3 wi = intr.p - ref.p;
        if(wi.length2() == 0)
        {
            pdf = 0;
        }
        else
        {
            float dis2 = wi.length2();
            wi.normalize();
            pdf = pdf * dis2 / dot(intr.normal, -wi);
        }
        if(std::isinf(pdf)) pdf=0;
        return intr;
    }
    float dc = dir.length();
    float invDc = 1/dc;
    vec3 wc = dir * invDc;
    vec3 wcX, wcY;
    GMathUtils::CoordinateSystem(wc, wcX, wcY);

    float sinThetaMax = radius * invDc;
    float sinThetaMax2 = sinThetaMax * sinThetaMax;
    float invSinThetaMax = 1/sinThetaMax;
    float cosThetaMax = std::sqrt(std::max(0.f, 1-sinThetaMax2));

    double ksi0 = GSampler::Random();
    double ksi1 = GSampler::Random();
    float cosTheta = (cosThetaMax - 1) * ksi0 + 1;
    float sinTheta2 = 1 - cosTheta * cosTheta;

    if(sinThetaMax2 < 0.00068523f)
    {
        sinTheta2 = sinThetaMax2 * ksi0;
        cosTheta = std::sqrt(1-sinTheta2);
    }

    float cosAlpha = sinTheta2 * invSinThetaMax + cosTheta * std::sqrt(std::max(0.f, 1.f-sinTheta2*invSinThetaMax*invSinThetaMax));
    float sinAlpha = std::sqrt(std::max(0.f, 1.f-cosAlpha*cosAlpha));
    float phi = ksi1 * 2 * M_PI;

    vec3 nWorld = wcX * sinAlpha * std::cos(phi) + wcY * sinAlpha * std::sin(phi) + wc * cosAlpha;
    nWorld.inverse();
    vec3 pWorld = pCenter + nWorld * nWorld;

    GSurfaceInteraction intr;
    intr.p = pWorld;
    intr.normal = nWorld;

    pdf = 1 / (2 * M_PI * (1-cosThetaMax));
    return intr;
}

GModel::GModel() :GGameObject(GGameObjectType::kModel)
{
}

bool GSphereModel::intersect(const GMath::GRay &ray, GMath::interval ray_t, GMath::GSurfaceInteraction &isect)
{
    sphere.center = _position;
    if(sphere.intersect(ray, ray_t, isect))
    {
        isect.material = material;
        return true;
    }
    return false;
}
