#ifndef GMATERIAL_H
#define GMATERIAL_H

#include "gcolor.h"
#include "gray.h"

class GMaterial
{
public:
    virtual ~GMaterial() = default;
    virtual GFColor Sample_f(const GMath::vec3& normal, const GMath::vec3& wo, GMath::vec3& wi, float& pdf) const;
    virtual GFColor f(const GMath::vec3& normal, const GMath::vec3& wo, GMath::vec3& wi) const = 0;
    virtual float Pdf(const GMath::vec3& normal, const GMath::vec3& wo, GMath::vec3& wi) const;

    virtual bool IsSpecular() { return false; }
};


class GLambertianMaterial : public GMaterial
{
public:
    GLambertianMaterial(const GFColor& Kd)
        :Kd(Kd)
    {}

    GFColor f(const GMath::vec3& normal, const GMath::vec3& wo, GMath::vec3& wi) const;

private:
    GFColor Kd;
};

class GMetalMaterial : public GMaterial
{
public:
    GMetalMaterial(const GFColor& Ks, double fuzz)
        :Ks(Ks),fuzz(fuzz)
    {}

    GFColor Sample_f(const GMath::vec3& normal, const GMath::vec3& wo, GMath::vec3& wi, float& pdf) const;
    bool IsSpecular() override;
private:
    GFColor Ks;
    double fuzz;
};

#endif // GMATERIAL_H
