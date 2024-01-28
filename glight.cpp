#include "glight.h"
#include "gscene.h"
using namespace GMath;

GLight::GLight(GLightType lightType, GFColor lColor)
    :GGameObject(GGameObject::GGameObjectType::kLight),lightType(lightType),lightColor(lColor)
{
}

GFColor GLight::Le(const GScene &scene, const GMath::GSurfaceInteraction &isect, const vec3 &w)
{
    return GColor::blackF;
}

GFColor GLight::Sample_Li(const GScene &scene, const GMath::GSurfaceInteraction &isect, vec3 &wi, float *pdf)
{
    return GColor::blackF;
}

GFColor GDirectionalLight::Sample_Li(const GScene& scene, const GMath::GSurfaceInteraction &isect, vec3 &wi, float *pdf)
{
    // TODO
    wi = forward();
    *pdf = 1;
    return GColor::blackF;
}

GFColor GPointLight::Sample_Li(const GScene& scene, const GMath::GSurfaceInteraction &isect, vec3 &wi, float *pdf)
{
    // TODO
    *pdf = 1;
    return GColor::blackF;
}

GFColor GSkyLight::Le(const GMath::GRay &ray)
{
    vec3 normalizedDir = ray.dir;
    normalizedDir.normalize();
    auto t = dot(normalizedDir, vec3::up);
    GFColor ret = GColor::blackF;
    if(t>=0)
    {
        ret = GColor::Lerp(vec4f(0.5,0.7,1.0,1.0), GColor::whiteF, t);
    }
    return ret;
}

GFColor GDiffuseAreaLight::Le(const GScene& scene, const GMath::GSurfaceInteraction &isect, const vec3& w)
{
    return (twoSided || dot(isect.normal, w)>0) ? lightColor : GColor::blackF;
}

bool GSphereLight::intersect(const GMath::GRay &ray, GMath::interval ray_t, GMath::GSurfaceInteraction &isect)
{
    auto sphere = std::dynamic_pointer_cast<GSphere>(shape);
    sphere->center = _position;
    if(sphere->intersect(ray, ray_t, isect))
    {
        //isect.mateiral = mateiral;
        return true;
    }
    return false;
}


