#include "gquad.h"
#include "ggameobject.h"
#include "gsampler.h"

GSurfaceInteraction GQuad::Sample(float &pdf, double time) const
{
    const mat4f* obj2World; // obj2World
    const mat4f* world2Obj; // world2Obj
    this->owner->TRSInvertTRS(obj2World, world2Obj);

    auto ksi0 = GSampler::Random();
    auto ksi1 = GSampler::Random();
    vec3 localPos = Q + u * ksi0 + v * ksi1;
    vec4 tmpWPos = ((*obj2World) * (vec4f)embed<double,4>(localPos, 1.));
    GSurfaceInteraction intr;
    intr.p = tmpWPos.xyz();
    intr.normal = geoNormal;
    pdf = 1.0/area;
    return intr;
}

GSurfaceInteraction GQuad::Sample(const GSurfaceInteraction &ref, float &pdf) const
{
    GSurfaceInteraction intr = Sample(pdf, ref.time);
    vec3 wi = intr.p - ref.p;
    auto dis2 = wi.length2();
    if(dis2 == 0)
    {
        pdf = 0;
    }
    else
    {
        wi.normalize();
        pdf = pdf * dis2 / dot(intr.normal, -wi);
    }
    if(std::isinf(pdf)) pdf = 0;
    return intr;
}

bool GQuad::intersect(const GRay &ray, interval ray_t, GSurfaceInteraction &isect)
{
    const mat4f* obj2World; // obj2World
    const mat4f* world2Obj; // world2Obj
    owner->TRSInvertTRS(obj2World, world2Obj);

    vec4 tmpRayO = vec4::one;
    tmpRayO.SetXYZ(ray.origin);
    vec3 localRayO = ((*world2Obj) * (vec4f)tmpRayO).xyz();
    vec4 tmpRayDir = vec4::zero;
    tmpRayDir.SetXYZ(ray.dir);
    vec3 localRayDir = ((*world2Obj) * (vec4f)tmpRayDir).xyz();
    double rayScale = localRayDir.length() / ray.dir.length();

    auto DoN = dot(geoNormal, localRayDir);
    if(std::fabs(DoN) < 1e-8)
    {
        return false;
    }

    interval localRayT = interval(ray_t.min * rayScale, ray_t.max * rayScale);
    auto t = (D - dot(geoNormal, localRayO)) / DoN;
    if(!localRayT.contains(t)) return false;

    vec3 localHitPos = localRayO + localRayDir * t;
    vec3 Q2HitPosVec = localHitPos - Q;
    auto alpha = dot(w, cross(u, Q2HitPosVec));
    auto beta = dot(w, cross(Q2HitPosVec, v));
    if(alpha < 0 || 1<alpha || beta<0 || 1<beta)
    {
        // not in quad
        return false;
    }

    vec4 hitPos = (*obj2World) * (vec4f)embed<double,4>(localHitPos, 1.0);
    vec3 wNormal = (mat3)(world2Obj->get_minor(3,3).transpose()) * geoNormal;
    isect.p = hitPos.xyz();
    isect.t = t * rayScale;
    isect.time = ray.time;
    isect.SetFaceNormal(ray, wNormal.normalize());
    isect.wo = -ray.dir;
    return true;
}

