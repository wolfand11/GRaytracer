#ifndef GMATERIAL_H
#define GMATERIAL_H

#include "gcolor.h"
#include "gray.h"
#include "gtexture.h"

using std::shared_ptr;

class GMaterial
{
public:
    virtual ~GMaterial() = default;
    virtual void Init(shared_ptr<GTexture> Kd, shared_ptr<GTexture> Ks, shared_ptr<GTexture> roughness, shared_ptr<GTexture> normal)
    {
        this->Kd = Kd;
        this->Ks = Ks;
        this->roughness = roughness;
        this->normal = normal;
    }
    virtual GFColor Sample_f(GSurfaceInteraction& isect, float& pdf) const;
    virtual GFColor f(const GSurfaceInteraction& isect) const = 0;
    virtual float Pdf(const GSurfaceInteraction& isect) const;

    void SameHemisphere(const GMath::vec3& normal, const GMath::vec3& wo, GMath::vec3& wi) const;
    virtual bool IsSpecular() { return false; }
    GFColor SchlickFresnel(GFColor Ks, float cosTheta) const
    {
        auto pow5 =[](float v) {return v * v * v * v * v;};
        return Ks + (GColor::whiteF - Ks) * pow5(1-cosTheta);
    }
    void UpdateShadingNormal(GSurfaceInteraction& isect) const
    {
        if(normal!=nullptr)
        {
            vec3f tNormal = (vec3f)normal->sample(isect.uv, isect.p).xyz().normalize();
            tNormal = tNormal * 2 - vec3f::one;
            vec3 wBitangent = cross(isect.tangent, isect.normal);
            wBitangent.normalize();
            isect.shadingNormal = isect.tangent * tNormal.x() + wBitangent * tNormal.y() + isect.normal * tNormal.z();
            isect.shadingNormal.normalize();
        }
        else
        {
            isect.shadingNormal = isect.normal;
        }
    }

    shared_ptr<GTexture> Kd;
    shared_ptr<GTexture> Ks;
    shared_ptr<GTexture> roughness;
    shared_ptr<GTexture> normal;
};


class GLambertianMaterial : public GMaterial
{
public:
    GLambertianMaterial()=default;
    GLambertianMaterial(const GFColor& Kd)
    {
        this->Kd = std::make_shared<GSolidColor>(Kd);
    }
    GLambertianMaterial(shared_ptr<GTexture> Kd)
    {
        this->Kd = Kd;
    }

    GFColor f(const GSurfaceInteraction& isect) const;
};

class GSpecularMaterial : public GMaterial
{
public:
    GSpecularMaterial()=default;
    GSpecularMaterial(const GFColor& Ks)
    {
        this->Ks = std::make_shared<GSolidColor>(Ks);
    }
    GSpecularMaterial(shared_ptr<GTexture> Ks)
    {
        this->Ks = Ks;
    }

    GFColor Sample_f(GSurfaceInteraction& isect, float& pdf) const;
    GFColor f(const GSurfaceInteraction& isect) const
    {
        return GColor::blackF;
    }
    float Pdf(const GSurfaceInteraction& isect) const
    {
        return 0;
    }
    virtual bool IsSpecular() { return true; }
};

class GGlossyMaterial : public GMaterial
{
public:
    GGlossyMaterial()=default;
    GGlossyMaterial(const GFColor& Ks, float roughness)
    {
        this->Ks = std::make_shared<GSolidColor>(Ks);
        this->roughness = std::make_shared<GSolidColor>(roughness);
    }
    GGlossyMaterial(shared_ptr<GTexture> Ks, shared_ptr<GTexture> roughness)
    {
        this->Ks = Ks;
        this->roughness = roughness;
    }
    GFColor Sample_f(GSurfaceInteraction& isect, float& pdf) const;
    GFColor f(const GSurfaceInteraction& isect) const;
    float Pdf(const GSurfaceInteraction& isect) const;

    float D_GGX(float NoH, float roughness) const;
    float V_SmithGGXCorrelatedFast(float NoV, float NoL, float roughness) const;
    GMath::vec3 Sample_wh(const GSurfaceInteraction& isect) const;
    float Pdf_wh(const GSurfaceInteraction& isect, const GMath::vec3& wh) const;
};
#endif // GMATERIAL_H
