#ifndef GBSDF_H
#define GBSDF_H

#include "gcolor.h"
#include "gmath.h"
#include <vector>
#include "gray.h"

enum BxDFType
{
    BSDF_REFLECTION = 1 << 0,
    BSDF_TRANSMISSION = 1 << 1,
    BSDF_DIFFUSE = 1 << 2,
    BSDF_GLOSSY = 1 << 3,
    BSDF_SPECULAR = 1 << 4,
    BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION | BSDF_TRANSMISSION,
};

class GBSDF;
class GBxDF
{
public:
    virtual ~GBxDF() {}
    GBxDF(std::shared_ptr<GBSDF> bsdf, BxDFType type);
    bool MatchesFlags(BxDFType t) const { return (type & t) == type; }

    virtual GFColor Sample_f(GSurfaceInteraction& isect, float& pdf) const;
    virtual GFColor f(const GSurfaceInteraction& isect) const = 0;
    virtual float Pdf(const GSurfaceInteraction& isect) const;

    const BxDFType type;
    std::shared_ptr<GBSDF> bsdf;
};

class GLambertianReflection : public GBxDF
{
public:
    GLambertianReflection(std::shared_ptr<GBSDF> bsdf)
       :GBxDF(bsdf, BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)){}

    GFColor f(const GSurfaceInteraction& isect) const;
};

class GSpecularReflection : public GBxDF
{
    GSpecularReflection(std::shared_ptr<GBSDF> bsdf)
       :GBxDF(bsdf, BxDFType(BSDF_REFLECTION | BSDF_SPECULAR)){}

    GFColor Sample_f(GSurfaceInteraction& isect, float& pdf) const override;
    GFColor f(const GSurfaceInteraction& isect) const override
    {
        return GColor::blackF;
    }
    float Pdf(const GSurfaceInteraction& isect) const override
    {
        return 0;
    }
};

class GSpecularRefraction : public GBxDF
{
    GSpecularRefraction(std::shared_ptr<GBSDF> bsdf)
       :GBxDF(bsdf, BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)){}

    GFColor Sample_f(GSurfaceInteraction& isect, float& pdf) const override;
    GFColor f(const GSurfaceInteraction& isect) const override
    {
        return GColor::blackF;
    }
    float Pdf(const GSurfaceInteraction& isect) const override
    {
        return 0;
    }
};

class GFresnelSpecular : public GBxDF
{
    GFresnelSpecular(std::shared_ptr<GBSDF> bsdf)
       :GBxDF(bsdf, BxDFType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_SPECULAR)){}

    GFColor Sample_f(GSurfaceInteraction& isect, float& pdf) const override;
    GFColor f(const GSurfaceInteraction& isect) const override
    {
        return GColor::blackF;
    }
    float Pdf(const GSurfaceInteraction& isect) const override
    {
        return 0;
    }
};

class GBSDF
{
public:
    GBSDF(){}

    GFColor f(const GSurfaceInteraction& isect) const;
    GFColor Sample_f(GSurfaceInteraction& isect, float& pdf) const;
    GFColor Sample_f(const GMath::vec3& wo, GMath::vec3& wi, float* pdf, BxDFType type = BSDF_ALL, BxDFType* sampledType=nullptr);
    inline int NumComponents(BxDFType flags) const;

    std::vector<std::shared_ptr<GBxDF>> bxdfs;

    void Init(GFColor Kd, GFColor Ks, GFColor Kt, float roughness)
    {
        this->Kd = Kd;
        this->Ks = Ks;
        this->Kt = Kt;
        this->roughness = roughness;
    }
    GFColor Kd;
    GFColor Ks;
    GFColor Kt;
    float roughness;

    void InitEta(float etaA, float etaB)
    {
        this->etaA = etaA;
        this->etaB = etaB;
    }
    float eta;
    float etaA;
    float etaB;
};

#endif // GBSDF_H
