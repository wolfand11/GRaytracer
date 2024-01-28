#ifndef GBSDF_H
#define GBSDF_H

#include "gcolor.h"
#include "gmath.h"
#include <vector>

enum BxDFType
{
    BSDF_REFLECTION = 1 << 0,
    BSDF_TRANSMISSION = 1 << 1,
    BSDF_DIFFUSE = 1 << 2,
    BSDF_GLOSSY = 1 << 3,
    BSDF_SPECULAR = 1 << 4,
    BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION | BSDF_TRANSMISSION,
};

class GBxDF
{
public:

    virtual ~GBxDF() {}
    GBxDF(BxDFType type) : type(type) {}
    bool MatchesFlags(BxDFType t) const { return (type & t) == type; }
    virtual GFColor f(const GMath::vec3& normal, const GMath::vec3& wo, const GMath::vec3& wi) const = 0;
    GFColor Sample_f(const GMath::vec3& normal, const GMath::vec3& wo, GMath::vec3& wi, float* pdf, BxDFType* sampledType=nullptr) const;
    double Pdf(const GMath::vec3& normal, const GMath::vec3& wo, const GMath::vec3& wi) const;

    const BxDFType type;
};

class GLambertianReflection : public GBxDF
{
public:
    GLambertianReflection(const GFColor& R)
        :GBxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(R){}

    GFColor f(const GMath::vec3& normal, const GMath::vec3& wo, const GMath::vec3& wi) const
    {
        //return R * M_INVERSE_PI;
        return R;
    }
private:
    const GFColor R;
};

class GBSDF
{
public:
    GBSDF(GMath::vec3 normalShading, GMath::vec3 normalGeometry, float eta=1)
        :normalGeometry(normalGeometry),normalShading(normalShading),eta(eta)
    {}
    GFColor f(const GMath::vec3& wo, const GMath::vec3& wi, BxDFType flags = BSDF_ALL) const;
    GFColor Sample_f(const GMath::vec3& wo, GMath::vec3& wi, float* pdf, BxDFType type = BSDF_ALL, BxDFType* sampledType=nullptr);
    inline int NumComponents(BxDFType flags) const;

    GMath::vec3 normalGeometry;
    GMath::vec3 normalShading;
    const float eta;
    std::vector<std::shared_ptr<GBxDF>> bxdfs;
};

#endif // GBSDF_H
