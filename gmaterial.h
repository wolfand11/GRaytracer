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
};


class GLambertianMaterial : public GMaterial
{
public:
    GLambertianMaterial(const GFColor& Kd)
        :Kd(std::make_shared<GSolidColor>(Kd))
    {
    }
    GLambertianMaterial(shared_ptr<GTexture> Kd)
        :Kd(Kd)
    {
    }

    GFColor f(const GSurfaceInteraction& isect) const;

private:
    shared_ptr<GTexture> Kd;
};

class GSpecularMaterial : public GMaterial
{
public:
    GSpecularMaterial(const GFColor& Ks)
        :Ks(std::make_shared<GSolidColor>(Ks))
    {
    }
    GSpecularMaterial(shared_ptr<GTexture> Ks)
        :Ks(Ks)
    {}

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
private:
    shared_ptr<GTexture> Ks;
};

class GGlossyMaterial : public GMaterial
{
public:
    GGlossyMaterial(const GFColor& Ks, float roughness)
        :Ks(std::make_shared<GSolidColor>(Ks)), roughness(std::make_shared<GSolidColor>(roughness))
    {
    }
    GGlossyMaterial(shared_ptr<GTexture> Ks, shared_ptr<GTexture> roughness)
        :Ks(Ks), roughness(roughness)
    {}
    GFColor Sample_f(GSurfaceInteraction& isect, float& pdf) const;
    GFColor f(const GSurfaceInteraction& isect) const;
    float Pdf(const GSurfaceInteraction& isect) const;

    float D_GGX(float NoH, float roughness) const;
    float V_SmithGGXCorrelatedFast(float NoV, float NoL, float roughness) const;
    GMath::vec3 Sample_wh(const GSurfaceInteraction& isect) const;
    float Pdf_wh(const GSurfaceInteraction& isect, const GMath::vec3& wh) const;
private:
    shared_ptr<GTexture> Ks;
    shared_ptr<GTexture> roughness;
};
#endif // GMATERIAL_H
