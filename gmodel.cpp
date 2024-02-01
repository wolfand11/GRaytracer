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

GSurfaceInteraction GShapeList::Sample(const GGameObject &owner, float &pdf, double time) const
{
    GSurfaceInteraction ret;
    return ret;
}

GSurfaceInteraction GShapeList::Sample(const GGameObject &owner, const GSurfaceInteraction &ref, float &pdf) const
{
    GSurfaceInteraction ret;
    return ret;
}

bool GShapeList::intersect(const GRay &ray, interval ray_t, GSurfaceInteraction &isect)
{
    bool hitAny = false;

    for(const auto& s : shapes)
    {
        if(s->intersect(ray, ray_t, isect))
        {
            hitAny = true;
            ray_t.max = isect.t;
        }
    }
    return hitAny;
}

bool GSphere::intersect(const GRay &ray, GMath::interval ray_t, GSurfaceInteraction &isect)
{
    vec3 curCenter = isMoving ? sphereCenter(center, ray.time) : center;
    vec3 oc = ray.origin - curCenter;
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
    isect.time = ray.time;
    vec3 outNormal = (isect.p - curCenter)/radius;
    isect.SetFaceNormal(ray, outNormal);
    isect.wo = -ray.dir;
    return true;
}

GSurfaceInteraction GSphere::Sample(const GGameObject& owner, float &pdf, double time) const
{
    vec3 pObj = GSampler::UniformSampleSphere() * radius;
    GSurfaceInteraction intr;
    intr.time = time;
    vec3 pCenter = sphereCenter(owner.position(), time);
    intr.p = pCenter + pObj;
    intr.normal = pObj.normalize();
    pdf = 1/(4*M_PI*radius*radius);
    return intr;
}

GSurfaceInteraction GSphere::Sample(const GGameObject& owner, const GSurfaceInteraction &ref, float &pdf) const
{
    vec3 pCenter = sphereCenter(owner.position(), ref.time);
    auto dir = pCenter - ref.p;
    if(dir.length2() <= radius*radius)
    {
        GSurfaceInteraction intr = Sample(owner, pdf, ref.time);
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

vec2 GSphere::getUV(const vec3 &p)
{
    auto theta = std::acos(-p.y());
    auto phi = std::atan2(-p.z(), p.x()) + M_PI;
    return vec2(phi/(2*M_PI), theta/M_PI);
}



bool GSphereModel::intersect(const GRay &ray, GMath::interval ray_t, GSurfaceInteraction &isect)
{
    auto sphere = std::dynamic_pointer_cast<GSphere>(shape);
    sphere->center = _position;
    if(sphere->intersect(ray, ray_t, isect))
    {
        isect.model = this;
        isect.uv = sphere->getUV(isect.p);
        isect.material = material.get();
        isect.light = nullptr;
        return true;
    }
    return false;
}
