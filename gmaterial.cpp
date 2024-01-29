#include "gmaterial.h"
#include "gsampler.h"
#include "gmathutils.h"

using namespace std;
using namespace GMath;

GFColor GMaterial::Sample_f(const GMath::vec3& normal, const GMath::vec3& wo, GMath::vec3& wi, float& pdf) const
{
    wi = GSampler::CosineSampleHemisphere();
    auto rot = GMathUtils::RotationMatrix(vec3::up, normal);
    wi = rot * (vec3f)wi;
    if(dot(wi, normal) < 0)
    {
        wi = -wi;
    }
    pdf = Pdf(normal, wo, wi);
    return f(normal, wo, wi);
}

float GMaterial::Pdf(const GMath::vec3 &normal, const GMath::vec3 &wo, const GMath::vec3 &wi) const
{
    float cosThetaI = dot(wi, normal);
    bool isInSameHemisphere = (dot(wo, normal) * cosThetaI) > 0;
    return isInSameHemisphere ? std::abs(cosThetaI) * M_INVERSE_PI : 0;
}

GFColor GLambertianMaterial::f(const GMath::vec3 &normal, const GMath::vec3 &wo, GMath::vec3 &wi) const
{
    return Kd * M_INVERSE_PI;
}

GFColor GMetalMaterial::Sample_f(const GMath::vec3 &normal, const GMath::vec3 &wo, GMath::vec3 &wi, float &pdf) const
{
    wi = GSampler::CosineSampleHemisphere();
    auto rot = GMathUtils::RotationMatrix(vec3::up, normal);
    wi = rot * (vec3f)wi;
    vec3 reflectDir = reflect(wo, normal);
    wi = wi * fuzz + reflectDir;
    if(dot(wi, normal) < 0)
    {
        wi = -wi;
    }
    pdf = Pdf(normal, wo, wi);
    return f(normal, wo, wi);
}

GFColor GMetalMaterial::f(const GMath::vec3 &normal, const GMath::vec3 &wo, GMath::vec3 &wi) const
{
    return Ks;
}

float GMetalMaterial::Pdf(const GMath::vec3 &normal, const GMath::vec3 &wo, const GMath::vec3 &wi) const
{
    return 1.f;
}

bool GMetalMaterial::IsSpecular()
{
    return false;
}
