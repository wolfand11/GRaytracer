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
            L = GColor::FastTonemap(L);
            scene.film.SetColor(i,j, GColor::FromFloat01Color(L));
        }
    }
}

GFColor GIntegrator::Li(GRay &ray, GScene &scene, int depth)
{
    GFColor L = GColor::blackF;
    GFColor beta = GColor::whiteF;
    bool specularBounce = false;
    int bounces;
    for(bounces=0; ;++bounces)
    {
        GSurfaceInteraction isect;
        isect.time = ray.time;
        interval ray_t = interval::init;
        bool hited = scene.intersect(ray, ray_t, isect);
        if(bounces == 0 || specularBounce)
        {
            if(hited)
            {
                // emitted lights(e.g:area light) at path
                L = L + beta * isect.Le(scene, -ray.dir);
            }
            else
            {
                for(auto light : scene.lights)
                {
                    // infinite lights at path
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

        if(!isect.material->IsSpecular())
        {
            // direct light shading
            GFColor Ld = SampleLight(scene, isect);
            L = L + beta * Ld;
        }

        // process ray recursive
        isect.wo = -ray.dir;
        float pdf;
        GFColor f = isect.material->Sample_f(isect, pdf);
        if(GColor::IsBlack(f) || pdf==0.f) break;

        beta = beta * f * absDot(isect.wi, isect.shadingNormal) / pdf;
        specularBounce = isect.material->IsSpecular();
        ray.origin = isect.p;
        ray.dir = isect.wi;

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

// Tips: don't return beta.
// GFColor GIntegrator::SampleLight(const GScene &scene, const GSurfaceInteraction &isect, GFColor& beta)
GFColor GIntegrator::SampleLight(const GScene &scene, GSurfaceInteraction &isect)
{
    GFColor Ld = GColor::blackF;
    for(auto light : scene.lights)
    {
        vec3 wi;
        float lightPdf;
        GFColor Li = light->Sample_Li(scene, isect, wi, lightPdf);
        if(GColor::IsBlack(Li) || lightPdf == 0) continue;
        isect.wi = wi;
        GFColor f = isect.material->f(isect) * absDot(wi, isect.shadingNormal);
        float scatteringPdf = isect.material->Pdf(isect);
        if(!GColor::IsBlack(f))
        {
            Ld = Ld + Li * f / lightPdf;
        }
    }
    return Ld;
}
