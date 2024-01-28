#include "gintegrator.h"
#include "gbsdf.h"
#include "gsampler.h"
using namespace GMath;

void GIntegrator::Render(GScene &scene)
{
    auto camera = scene.camera;
    for(int j=camera->viewportY; j<camera->viewportH; j++)
    {
        GLog::LogInfo(j, "/", camera->viewportH);
        for(int i=camera->viewportX; i<camera->viewportW; i++)
        {
            GFColor L = GColor::blackF;
            for(int k=0; k<spp; k++)
            {
                auto pixelOffsetX = -0.5 + GSampler::Random();
                auto pixelOffsetY = -0.5 + GSampler::Random();
                auto ray = camera->GetRay(i+pixelOffsetX, j+pixelOffsetY);
                L = L + this->Li(ray, scene, 0);
            }
            L = L / spp;
            L.SetW(1);
            scene.film.SetColor(i,j, GColor::FromFloat01Color(L));
        }
    }
}

GFColor GIntegrator::SampleLight(const GScene &scene, const GMath::GSurfaceInteraction &isect, GFColor& beta)
{
    GFColor Ld = GColor::blackF;
    for(auto light : scene.lights)
    {
        vec3 wi;
        float pdf;
        GFColor Li = light->Sample_Li(scene, isect, wi, &pdf);
        if(GColor::IsBlack(Li) || pdf == 0) continue;
        GFColor f = isect.bsdf->f(isect.wo, wi);
        if(!GColor::IsBlack(f))
        {
            beta = f * std::abs(dot(wi, isect.normal)) / pdf;
            Ld = Ld + Li;
        }
    }
    return Ld;
}

GFColor GWhittedIntegrator::Li(GRay& ray, GScene& scene, int depth)
{
    auto camera = scene.camera;
    GFColor L = GColor::blackF;
    GFColor beta = GColor::whiteF;
    GSurfaceInteraction isect;

    interval ray_t = interval::init;
    if(!scene.intersect(ray, ray_t, isect))
    {
        for(auto light : scene.lights)
        {
            // infinite lights
            L = L + light->Le(ray);
            return L;
        }
    }

    const vec3& normal = isect.normal;
    vec3 wo = isect.wo;
    isect.ComputeScatteringFunctions();
    if(!isect.bsdf)  // skip over medium boundaries
    {
        ray.origin = isect.p;
        return Li(ray, scene, depth);
    }

    L = L + isect.Le(scene, wo);

    // add direct light
    L = L + SampleLight(scene, isect, beta) * beta;

    if(depth + 1 < maxDepth)
    {
        L = L + SpecularReflect(ray, isect, scene, depth);
        //L = L + SpecularTransmit(ray, isect, scene, depth);
    }
    return L;
}

GFColor GWhittedIntegrator::SpecularReflect(const GMath::GRay &ray, const GMath::GSurfaceInteraction &isect, GScene &scene, int depth)
{
    vec3 wo = isect.wo;
    vec3 wi;
    float pdf;
    BxDFType type = BxDFType(BSDF_REFLECTION | BSDF_SPECULAR);
    GFColor f = isect.bsdf->Sample_f(wo, wi, &pdf, type);

    const vec3& normal = isect.normal;
    if(pdf > 0.f && !GColor::IsBlack(f) && std::abs(dot(wi, normal))!=0.f)
    {
        GRay r;
        r.origin = isect.p;
        r.dir = wi;
        return f * Li(r, scene, depth+1) * std::abs(dot(wi, normal)) / pdf;
    }
    else
    {
        return GColor::blackF;
    }
}

GFColor GPathIntegrator::Li(GMath::GRay &ray, GScene &scene, int depth)
{
    GFColor L = GColor::blackF;
    GFColor beta = GColor::whiteF;
    bool specularBounce = false;
    int bounces;
    for(bounces=0; ;++bounces)
    {
        GSurfaceInteraction isect;
        interval ray_t = interval::init;
        bool hited = scene.intersect(ray, ray_t, isect);
        if(bounces == 0 || specularBounce)
        {
            if(hited)
            {
                L = L + beta * isect.Le(scene, -ray.dir);
            }
            else
            {
                for(auto light : scene.lights)
                {
                    // infinite lights
                    if(light->lightType==GLightType::kLTSky)
                    {
                        L = L + beta * light->Le(ray);
                    }
                }
            }
        }
        if(!hited || bounces>=maxDepth) break;

        isect.ComputeScatteringFunctions();
        if(isect.bsdf==nullptr) // skip over medium boundaries
        {
            ray.origin = isect.p;
            bounces--;
            continue;
        }

        if(isect.bsdf->NumComponents(BxDFType(BSDF_ALL & ~BSDF_SPECULAR)) > 0)
        {
            GFColor Ld = SampleLight(scene, isect, beta);
            // add direct light
            L = L + beta * Ld;
        }

        vec3 wo = -ray.dir;
        vec3 wi;
        float pdf;
        BxDFType flags;
        GFColor f = isect.bsdf->Sample_f(wo, wi, &pdf, BSDF_ALL, &flags);
        if(GColor::IsBlack(f) || pdf==0.f) break;

        beta = beta * f * std::abs(dot(wi, isect.normal)) / pdf;
        specularBounce = (flags & BSDF_SPECULAR) != 0;
        ray.origin = isect.p;
        ray.dir = wi;

        GFColor rrBeta = beta;
        if(rrBeta.max() < rrThreshold && bounces > 3)
        {
            auto q = std::max(0.05f, 1-rrBeta.max());
            if(GSampler::Random() < q) break;
            beta = beta / (1-q);
        }
    }
    return L;
}
