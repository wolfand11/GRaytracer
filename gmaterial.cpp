#include "gmaterial.h"
#include "gsampler.h"
#include "gmathutils.h"

using namespace std;
using namespace GMath;

GFColor GMaterial::Sample_f(GMath::GSurfaceInteraction& isect, float& pdf) const
{
    isect.wi = GSampler::CosineSampleHemisphere();
    SameHemisphere(isect.normal, isect.wo, isect.wi);
    pdf = Pdf(isect);
    return f(isect);
}

float GMaterial::Pdf(const GMath::GSurfaceInteraction& isect) const
{
    float cosThetaI = dot(isect.wi, isect.normal);
    bool isInSameHemisphere = (dot(isect.wo, isect.normal) * cosThetaI) > 0;
    if(isInSameHemisphere)
    {
        return std::abs(cosThetaI) * M_INVERSE_PI;
    }
    else
    {
        return 0;
    }
}

void GMaterial::SameHemisphere(const GMath::vec3& normal, const GMath::vec3& wo, GMath::vec3& wi) const
{
    auto rot = GMathUtils::RotationMatrix(vec3::up, normal);
    wi = rot * (vec3f)wi;
    if(dot(wi, normal) * dot(wo, normal) < 0)
    {
        wi = -wi;
    }
}

GFColor GLambertianMaterial::f(const GMath::GSurfaceInteraction& isect) const
{
    return Kd * M_INVERSE_PI;
}

GFColor GSpecularMaterial::Sample_f(GMath::GSurfaceInteraction& isect, float &pdf) const
{
    isect.wi = reflect(isect.wo, isect.normal);
    pdf = 1;
    auto cosTheta = absDot(isect.wi, isect.normal);
    GFColor F = SchlickFresnel(Ks->sample(isect.uv, isect.p), cosTheta);
    return F * Ks->sample(isect.uv, isect.p) / cosTheta;
}

GFColor GGlossyMaterial::Sample_f(GMath::GSurfaceInteraction &isect, float &pdf) const
{
    float cosThetaO = dot(isect.wo, isect.normal);
    if(cosThetaO <= 0) return GColor::blackF;
    vec3 wh = Sample_wh(isect);
    float HoO = dot(isect.wo, wh);
    if(HoO <= 0) return GColor::blackF;
    isect.wi = reflect(isect.wo, wh);
    float cosThetaI = dot(isect.wi, isect.normal);
    bool isInSameHemisphere = (dot(isect.wo, isect.normal) * cosThetaI) > 0;
    if(!isInSameHemisphere) return GColor::blackF;
    // wh space to wi space
    pdf = Pdf_wh(isect, wh) / (4 * HoO);
    return f(isect);
}

GFColor GGlossyMaterial::f(const GMath::GSurfaceInteraction &isect) const
{
    float cosThetaO = dot(isect.wo, isect.normal);
    float cosThetaI = dot(isect.wi, isect.normal);
    vec3 wh = isect.wi + isect.wo;
    if(cosThetaO == 0 || cosThetaI == 0) return GColor::blackF;
    wh.normalize();
    float NoH = dot(wh, isect.normal);
    if(NoH <=0)  return GColor::blackF;

    GFColor KsColor = Ks->sample(isect.uv, isect.p);

    GFColor F = SchlickFresnel(KsColor, NoH);
    float rough = roughness->sample(isect.uv, isect.p).x();
    GFColor ret = KsColor * D_GGX(NoH, rough) * F * V_SmithGGXCorrelatedFast(cosThetaO, cosThetaI, rough);
    return ret;
}

float GGlossyMaterial::Pdf(const GMath::GSurfaceInteraction &isect) const
{
    float cosThetaO = dot(isect.wo, isect.normal);
    float cosThetaI = dot(isect.wi, isect.normal);
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

vec3 GGlossyMaterial::Sample_wh(const GMath::GSurfaceInteraction &isect) const
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
    SameHemisphere(isect.normal, isect.wo, wh);
    return wh;
}

float GGlossyMaterial::Pdf_wh(const GMath::GSurfaceInteraction& isect, const GMath::vec3 &wh) const
{
    float NoH = dot(wh, isect.normal);
    float rough = roughness->sample(isect.uv, isect.p).x();
    return D_GGX(NoH, rough) * NoH;
}
