#include "gbsdf.h"
#include "gsampler.h"
using namespace std;

GFColor GBxDF::Sample_f(const GMath::vec3& normal, const GMath::vec3 &wo, GMath::vec3 &wi, float *pdf, BxDFType *sampledType) const
{
    wi = GSampler::CosineSampleHemisphere();
    if(dot(wi, normal) < 0)
    {
        wi = -wi;
    }
    *pdf = Pdf(normal, wo, wi);
    return f(normal, wo, wi);
}

double GBxDF::Pdf(const GMath::vec3& normal, const GMath::vec3 &wo, const GMath::vec3 &wi) const
{
    double cosThetaI = dot(wi, normal);
    bool isInSameHemisphere = (dot(wo, normal) * cosThetaI) > 0;
    return isInSameHemisphere ? std::abs(cosThetaI) * M_INVERSE_PI : 0;
}

GFColor GBSDF::f(const GMath::vec3 &wo, const GMath::vec3 &wi, BxDFType flags) const
{
    bool reflect = dot(wi, normalGeometry) * dot(wo, normalGeometry) > 0;
    GFColor f = GColor::blackF;
    for(auto bxdf : bxdfs)
    {
        if(bxdf->MatchesFlags(flags))
        {
            if((reflect && bxdf->type & BSDF_REFLECTION) || (!reflect && bxdf->type & BSDF_TRANSMISSION))
            {
                f = f + bxdf->f(normalShading, wo, wi);
            }
        }
    }
    return f;
}

GFColor GBSDF::Sample_f(const GMath::vec3 &wo, GMath::vec3 &wi, float *pdf, BxDFType type, BxDFType *sampledType)
{
    int matchingComps = NumComponents(type);
    if(matchingComps == 0)
    {
        *pdf = 0;
        if(sampledType) *sampledType=BxDFType(0);
        return GColor::blackF;
    }

    shared_ptr<GBxDF> bxdf = nullptr;
    int count = std::min((int)std::floor(GSampler::Random() * matchingComps), matchingComps-1);
    for(auto tBxdf : bxdfs)
    {
        if(tBxdf->MatchesFlags(type) && count-- == 0)
        {
            bxdf = tBxdf;
            break;
        }
    }
    *pdf = 0;
    if(sampledType) *sampledType=bxdf->type;
    GFColor f = bxdf->Sample_f(normalShading, wo, wi, pdf, sampledType);
    if(*pdf == 0)
    {
        if(sampledType) *sampledType = BxDFType(0);
        return GColor::blackF;
    }
    if(!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1)
    {
        for(auto tBxdf : bxdfs)
        {
            if(tBxdf!=bxdf && tBxdf->MatchesFlags(type))
            {
                *pdf += tBxdf->Pdf(normalShading, wo, wi);
            }
        }
    }
    if(matchingComps > 1) *pdf /= matchingComps;

    if(!(bxdf->type & BSDF_SPECULAR))
    {
        bool reflect = dot(wi, normalGeometry) * dot(wo, normalGeometry) > 0;
        f = GColor::blackF;
        for(auto tBxdf : bxdfs)
        {
            if(tBxdf->MatchesFlags(type))
            {
                if((reflect && tBxdf->type & BSDF_REFLECTION) || (!reflect && tBxdf->type & BSDF_TRANSMISSION))
                {
                    f = f + tBxdf->f(normalShading, wo, wi);
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







