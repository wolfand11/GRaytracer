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
    virtual void InitTransmit(shared_ptr<GTexture> Kt, double eta)
    {
        this->Kt = Kt;
        this->eta = eta;
    }
    virtual GFColor Sample_f(GSurfaceInteraction& isect, float& pdf) const;
    virtual GFColor f(const GSurfaceInteraction& isect) const = 0;
    virtual float Pdf(const GSurfaceInteraction& isect) const;

    virtual bool IsSpecular() { return false; }
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

    shared_ptr<GTexture> Kt;
    double eta;
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

class GSpecularReflectionMaterial : public GMaterial
{
public:
    GSpecularReflectionMaterial()=default;
    GSpecularReflectionMaterial(const GFColor& Ks)
    {
        this->Ks = std::make_shared<GSolidColor>(Ks);
    }
    GSpecularReflectionMaterial(shared_ptr<GTexture> Ks)
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
    bool IsSpecular() override { return true; }
};

class GSpecularRefractionMaterial : public GMaterial
{
public:
    GSpecularRefractionMaterial()=default;
    GSpecularRefractionMaterial(const GFColor& Ks, const GFColor& Kt, double eta)
    {
        this->Ks = std::make_shared<GSolidColor>(Ks);
        this->Kt = std::make_shared<GSolidColor>(Kt);
        this->eta = eta;
    }
    GSpecularRefractionMaterial(shared_ptr<GTexture> Ks, shared_ptr<GTexture> Kt, double eta)
    {
        this->Ks = Ks;
        this->Kt = Kt;
        this->eta = eta;
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
    bool IsSpecular() override { return true; }
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
