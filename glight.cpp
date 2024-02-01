#include "glight.h"
#include "gscene.h"
#include "gutils.h"
using namespace GMath;

GLight::GLight(GLightType lightType, GFColor lColor)
    :GGameObject(GGameObject::GGameObjectType::kLight),lightType(lightType),lightColor(lColor)
{
}

GFColor GLight::Le(const GScene &scene, const GSurfaceInteraction &isect, const vec3 &w)
{
    return GColor::blackF;
}

GFColor GLight::Sample_Li(const GScene &scene, const GSurfaceInteraction &isect, vec3 &wi, float& pdf)
{
    return GColor::blackF;
}

GFColor GDirectionalLight::Sample_Li(const GScene& scene, const GSurfaceInteraction &isect, vec3 &wi, float& pdf)
{
    wi = -forward();
    pdf = 1;
    vec3 pOutside = isect.p + wi * GUtils::worldSize;
    vec3 rayDir = pOutside - isect.p;
    interval ray_t(0.001, rayDir.length()*0.999);
    GRay shadowRay(isect.p, rayDir.normalize());
    GSurfaceInteraction newIsect;
    newIsect.time = isect.time;
    if(!scene.intersect(shadowRay, ray_t, newIsect))
    {
        return lightColor;
    }
    return GColor::blackF;
}

GFColor GPointLight::Sample_Li(const GScene& scene, const GSurfaceInteraction &isect, vec3 &wi, float& pdf)
{
    vec3 rayDir = (vec3)_position - isect.p;
    interval ray_t(0.001, rayDir.length()*0.999);
    wi = rayDir;
    wi.normalize();
    pdf = 1;
    GRay shadowRay(isect.p, wi);
    GSurfaceInteraction newIsect;
    newIsect.time = isect.time;
    if(!scene.intersect(shadowRay, ray_t, newIsect))
    {
        return lightColor / rayDir.length2();
    }
    return GColor::blackF;
}

GFColor GSkyLight::Le(const GRay &ray)
{
    vec3 normalizedDir = ray.dir;
    normalizedDir.normalize();
    auto t = dot(normalizedDir, vec3::up);
    GFColor ret = GColor::blackF;
    GFColor sky = vec4f(0.5,0.7,1.0,1.0);
    GFColor ground = vec4f(0.68,0.5,0.15,1.0);
    GFColor dark = vec4f(0.3, 0.3, 0.2, 1.0);
    if(t>=0)
    {
        ret = GColor::Lerp(sky, ground, t);
    }
    else
    {
        ret = GColor::Lerp(dark, ground, -t);
    }
    return ret;
}

GFColor GDiffuseAreaLight::Le(const GScene& scene, const GSurfaceInteraction &isect, const vec3& w)
{
    return (twoSided || dot(isect.normal, w)>0) ? lightColor : GColor::blackF;
}

GFColor GDiffuseAreaLight::Sample_Li(const GScene &scene, const GSurfaceInteraction &isect, vec3 &wi, float &pdf)
{
    const GGameObject& self = *this;
    GSurfaceInteraction intr = shape->Sample(self, isect, pdf);
    vec3 rayDir = intr.p - isect.p;
    if(pdf==0 || rayDir.length2()==0)
    {
        pdf = 0;
        return GColor::blackF;
    }
    interval ray_t(0.001, rayDir.length()*0.999);
    wi = rayDir.normalize();
    GRay shadowRay(isect.p, wi);
    GSurfaceInteraction newIsect;
    newIsect.time = isect.time;
    if(!scene.intersect(shadowRay, ray_t, newIsect) || newIsect.light==this)
    {
        return Le(scene, newIsect, -wi);
    }
    pdf = 0;
    return GColor::blackF;
}

bool GSphereLight::intersect(const GRay &ray, GMath::interval ray_t, GSurfaceInteraction &isect)
{
    auto sphere = std::dynamic_pointer_cast<GSphere>(shape);
    sphere->center = _position;
    if(sphere->intersect(ray, ray_t, isect))
    {
        isect.model = nullptr;
        isect.light = this;
        return true;
    }
    return false;
}


