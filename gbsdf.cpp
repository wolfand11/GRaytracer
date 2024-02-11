#include "gbsdf.h"
#include "gmathutils.h"
#include "gsampler.h"
#include "gutils.h"
using namespace std;

GBxDF::GBxDF(std::shared_ptr<GBSDF> bsdf, BxDFType type) : bsdf(bsdf), type(type) {}

GFColor GBxDF::Sample_f(GSurfaceInteraction &isect, float &pdf) const
{
    isect.wi = GSampler::CosineSampleHemisphere();
    GMathUtils::SameHemisphere(isect.shadingNormal, isect.wo, isect.wi);
    pdf = Pdf(isect);
    return f(isect);
}

float GBxDF::Pdf(const GSurfaceInteraction& isect) const
{
    float cosThetaI = dot(isect.wi, isect.shadingNormal);
    bool isInSameHemisphere = (dot(isect.wo, isect.shadingNormal) * cosThetaI) > 0;
    if(isInSameHemisphere)
    {
        return std::abs(cosThetaI) * M_INVERSE_PI;
    }
    else
    {
        return 0;
    }
}

GFColor GLambertianReflection::f(const GSurfaceInteraction &isect) const
{
    GFColor KdColor(GColor::blackF);
    if(GUtils::debugType==GDebugType::kNormalTex)
    {
        //KdColor = bsdf->normal->sample(isect.uv, isect.p);
    }
    else if(GUtils::debugType==GDebugType::kWorldNormal)
    {
        KdColor = GColor::whiteF;
        KdColor.SetXYZ((isect.shadingNormal + vec3::one) * 0.5);
    }
    else if(GUtils::debugType==GDebugType::kWorldTangent)
    {
        KdColor = GColor::whiteF;
        KdColor.SetXYZ((isect.tangent + vec3::one) * 0.5);
    }
    else
    {
        KdColor = bsdf->Kd;
    }
    return KdColor * M_INVERSE_PI;
}

GFColor GSpecularReflection::Sample_f(GSurfaceInteraction &isect, float &pdf) const
{
    GFColor KsColor = bsdf->Ks;
    if(GColor::IsBlack(KsColor))
    {
        pdf = 0;
        return GColor::blackF;
    }
    isect.wi = reflect(isect.wo, isect.shadingNormal);
    pdf = 1;
    auto cosTheta = absDot(isect.wi, isect.shadingNormal);
    GFColor F = GMathUtils::SchlickFresnel(KsColor, cosTheta);
    return F * KsColor / cosTheta;
}

GFColor GSpecularRefraction::Sample_f(GSurfaceInteraction &isect, float &pdf) const
{
    bool entering = dot(isect.wo, isect.normal) > 0;
    float etaI = entering ? bsdf->etaA : bsdf->etaB;
    float etaT = entering ? bsdf->etaB : bsdf->etaA;

    // tips: we trace ray from camera to scene, so wo is exit, wi is incidence
    if(!refract(isect.wo, isect.shadingNormal, (double)etaI/etaT, isect.wi))
    {
        return GColor::blackF;
    }
    pdf = 1;
    auto cosTheta = absDot(isect.wi, isect.shadingNormal);
    GFColor F = GMathUtils::SchlickFresnel(bsdf->Ks, cosTheta);
    GFColor KtColor = bsdf->Ks;
    GFColor ft = KtColor * (GColor::whiteF - F);
    // https://www.pbr-book.org/3ed-2018/Light_Transport_III_Bidirectional_Methods/The_Path-Space_Measurement_Equation#fragment-Accountfornon-symmetrywithtransmissiontodifferentmedium-0
    //if(false)
    {
        ft = ft * (etaI * etaI) / (etaT * etaT);
    }
    return ft / cosTheta;
}

GFColor GBSDF::f(const GSurfaceInteraction& isect) const
{
    bool reflect = dot(isect.wi, isect.normal) * dot(isect.wo, isect.normal) > 0;
    GFColor f = GColor::blackF;
    for(auto& bxdf : bxdfs)
    {
        if(bxdf->MatchesFlags(isect.bxdfTypes))
        {
            if((reflect && (bxdf->type & BSDF_REFLECTION)) || (!reflect && (bxdf->type & BSDF_TRANSMISSION)))
            {
                f = f + bxdf->f(isect);
            }
        }
    }
    return f;
}

GFColor GBSDF::Sample_f(GSurfaceInteraction &isect, float &pdf) const
{
    int matchingComps = NumComponents(isect.bxdfTypes);
    if(matchingComps == 0)
    {
        pdf = 0;
        isect.sampledBxDFType=BxDFType(0);
        return GColor::blackF;
    }

    shared_ptr<GBxDF> bxdf = nullptr;
    int count = std::min((int)std::floor(GSampler::Random() * matchingComps), matchingComps-1);
    for(auto& tBxdf : bxdfs)
    {
        if(tBxdf->MatchesFlags(isect.bxdfTypes) && count-- == 0)
        {
            bxdf = tBxdf;
            break;
        }
    }
    pdf = 0;
    isect.sampledBxDFType=bxdf->type;
    GFColor f = bxdf->Sample_f(isect, pdf);
    if(pdf == 0)
    {
        isect.sampledBxDFType=BxDFType(0);
        return GColor::blackF;
    }

    // Compute overall PDF with all matching _BxDF_s
    if(!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1)
    {
        for(auto& tBxdf : bxdfs)
        {
            if(tBxdf!=bxdf && tBxdf->MatchesFlags(isect.bxdfTypes))
            {
                pdf += tBxdf->Pdf(isect);
            }
        }
    }
    if(matchingComps > 1) pdf /= matchingComps;

    if(!(bxdf->type & BSDF_SPECULAR))
    {
        bool reflect = dot(isect.wi, isect.normal) * dot(isect.wo, isect.normal) > 0;
        f = GColor::blackF;
        for(auto& tBxdf : bxdfs)
        {
            if(tBxdf->MatchesFlags(isect.bxdfTypes))
            {
                if((reflect && tBxdf->type & BSDF_REFLECTION) || (!reflect && tBxdf->type & BSDF_TRANSMISSION))
                {
                    f = f + tBxdf->f(isect);
                }
            }
        }
    }
    return f;
}

inline int GBSDF::NumComponents(BxDFType flags) const
{
    int num = 0;
    for (int i = 0; i < bxdfs.size(); ++i)
        if (bxdfs[i]->MatchesFlags(flags)) ++num;
    return num;
}
