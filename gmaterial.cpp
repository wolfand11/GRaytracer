#include "gmaterial.h"
#include "gsampler.h"
#include "gmathutils.h"
#include "gutils.h"

using namespace std;
using namespace GMath;

GFColor GMaterial::Sample_f(GSurfaceInteraction& isect, float& pdf) const
{
    isect.wi = GSampler::CosineSampleHemisphere();
    GMathUtils::SameHemisphere(isect.shadingNormal, isect.wo, isect.wi);
    pdf = Pdf(isect);
    return f(isect);
}

float GMaterial::Pdf(const GSurfaceInteraction& isect) const
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

GFColor GLambertianMaterial::f(const GSurfaceInteraction& isect) const
{
    GFColor KdColor;
    if(GUtils::debugType==GDebugType::kNormalTex)
    {
        KdColor = normal->sample(isect.uv, isect.p);
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
        KdColor = Kd->sample(isect.uv, isect.p);
    }
    return KdColor * M_INVERSE_PI;
}

GFColor GSpecularReflectionMaterial::Sample_f(GSurfaceInteraction& isect, float &pdf) const
{
    GFColor KsColor = Ks->sample(isect.uv, isect.p);
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

GFColor GSpecularRefractionMaterial::Sample_f(GSurfaceInteraction &isect, float &pdf) const
{
    GFColor KtColor = Kt->sample(isect.uv, isect.p);
    if(GColor::IsBlack(KtColor))
    {
        pdf = 0;
        return GColor::blackF;
    }
    bool entering = isect.isFrontFace;
    float etaI = entering ? 1.0 : eta;
    float etaT = entering ? eta : 1.0;

    // tips: we trace ray from camera to scene, so wo is exit, wi is incidence
    if(!refract(isect.wo, isect.shadingNormal, (double)etaI/etaT, isect.wi))
    {
        pdf = 0;
        return GColor::blackF;
    }
    pdf = 1;
    auto cosTheta = absDot(isect.wi, isect.shadingNormal);
    GFColor KsColor = Ks->sample(isect.uv, isect.p);
    GFColor F = GMathUtils::SchlickFresnel(KsColor, cosTheta);
    GFColor ft = KtColor * (GColor::whiteF - F);
    // https://www.pbr-book.org/3ed-2018/Light_Transport_III_Bidirectional_Methods/The_Path-Space_Measurement_Equation#fragment-Accountfornon-symmetrywithtransmissiontodifferentmedium-0
    //if(false)
    {
        ft = ft * (etaI * etaI) / (etaT * etaT);
    }
    return ft / cosTheta;
}

GFColor GGlossyMaterial::Sample_f(GSurfaceInteraction &isect, float &pdf) const
{
    float cosThetaO = dot(isect.wo, isect.shadingNormal);
    if(cosThetaO <= 0) return GColor::blackF;
    vec3 wh = Sample_wh(isect);
    float HoO = dot(isect.wo, wh);
    if(HoO <= 0) return GColor::blackF;
    isect.wi = reflect(isect.wo, wh);
    float cosThetaI = dot(isect.wi, isect.shadingNormal);
    bool isInSameHemisphere = (dot(isect.wo, isect.shadingNormal) * cosThetaI) > 0;
    if(!isInSameHemisphere) return GColor::blackF;
    // wh space to wi space
    pdf = Pdf_wh(isect, wh) / (4 * HoO);
    return f(isect);
}

GFColor GGlossyMaterial::f(const GSurfaceInteraction &isect) const
{
    float cosThetaO = dot(isect.wo, isect.shadingNormal);
    float cosThetaI = dot(isect.wi, isect.shadingNormal);
    vec3 wh = isect.wi + isect.wo;
    if(cosThetaO == 0 || cosThetaI == 0) return GColor::blackF;
    wh.normalize();
    float NoH = dot(wh, isect.shadingNormal);
    if(NoH <=0)  return GColor::blackF;

    GFColor KsColor = Ks->sample(isect.uv, isect.p);

    GFColor F = GMathUtils::SchlickFresnel(KsColor, NoH);
    float rough = roughness->sample(isect.uv, isect.p).x();
    GFColor ret = KsColor * D_GGX(NoH, rough) * F * V_SmithGGXCorrelatedFast(cosThetaO, cosThetaI, rough);
    return ret;
}

float GGlossyMaterial::Pdf(const GSurfaceInteraction &isect) const
{
    float cosThetaO = dot(isect.wo, isect.shadingNormal);
    float cosThetaI = dot(isect.wi, isect.shadingNormal);
    if(cosThetaI * cosThetaO <= 0) return 0;
    vec3 wh = (isect.wi + isect.wo).normalize();
    return Pdf_wh(isect, wh) / (4*dot(isect.wo, wh));
}

float GGlossyMaterial::D_GGX(float NoH, float roughness) const
{
    float a = NoH * roughness;
    float k = roughness / (1.0 - NoH * NoH + a * a);
    return k * k * (1.0 / M_PI);
}

float GGlossyMaterial::V_SmithGGXCorrelatedFast(float NoV, float NoL, float roughness) const
{
    float a = roughness;
    float GGXV = NoL * (NoV * (1.0 - a) + a);
    float GGXL = NoV * (NoL * (1.0 - a) + a);
    return 0.5 / (GGXV + GGXL);
}

vec3 GGlossyMaterial::Sample_wh(const GSurfaceInteraction &isect) const
{
    vec3 wh;
    float cosTheta = 0;
    double ksi0 = GSampler::Random();
    double ksi1 = GSampler::Random();
    float phi = 2 * M_PI * ksi1;
    float rough = roughness->sample(isect.uv, isect.p).x();
    float tanTheta2 = rough * rough * ksi0 / (1-ksi0);
    cosTheta = 1/std::sqrt(1+tanTheta2);
    float sinTheta =std::sqrt(std::max(0., 1.-cosTheta*cosTheta));
    wh = vec3(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);
    GMathUtils::SameHemisphere(isect.shadingNormal, isect.wo, wh);
    return wh;
}

float GGlossyMaterial::Pdf_wh(const GSurfaceInteraction& isect, const GMath::vec3 &wh) const
{
    float NoH = dot(wh, isect.shadingNormal);
    float rough = roughness->sample(isect.uv, isect.p).x();
    return D_GGX(NoH, rough) * NoH;
}
