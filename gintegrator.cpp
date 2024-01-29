#include "gintegrator.h"
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
            //GColor::FastTonemap(L);
            scene.film.SetColor(i,j, GColor::FromFloat01Color(L));
        }
    }
}

GFColor GIntegrator::Li(GMath::GRay &ray, GScene &scene, int depth)
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

        if(isect.material==nullptr) // skip over medium boundaries
        {
            ray.origin = isect.p;
            bounces--;
            continue;
        }

        // direct light
        if(!isect.material->IsSpecular())
        {
            GFColor Ld = SampleLight(scene, isect);
            // add direct light
            L = L + beta * Ld;
        }

        vec3 wo = -ray.dir;
        vec3 wi;
        float pdf;
        GFColor f = isect.material->Sample_f(isect.normal, wo, wi, pdf);
        if(GColor::IsBlack(f) || pdf==0.f) break;

        beta = beta * f * absDot(wi, isect.normal) / pdf;
        specularBounce = isect.material->IsSpecular();
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

//
// GFColor GIntegrator::SampleLight(const GScene &scene, const GMath::GSurfaceInteraction &isect, GFColor& beta)
GFColor GIntegrator::SampleLight(const GScene &scene, const GMath::GSurfaceInteraction &isect)
{
    GFColor Ld = GColor::blackF;
    for(auto light : scene.lights)
    {
        vec3 wi;
        float lightPdf;
        GFColor Li = light->Sample_Li(scene, isect, wi, &lightPdf);
        if(GColor::IsBlack(Li) || lightPdf == 0) continue;
        GFColor f = isect.material->f(isect.normal, isect.wo, wi) * absDot(wi, isect.normal);
        float scatteringPdf = isect.material->Pdf(isect.normal, isect.wo, wi);
        if(!GColor::IsBlack(f))
        {
            Ld = Ld + Li * f / lightPdf;
        }
    }
    return Ld;
}
