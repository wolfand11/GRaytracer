#ifndef GTEXTURE_H
#define GTEXTURE_H

#include "gmath.h"
#include "gcolor.h"
#include "tgaimage.h"

using std::shared_ptr;
using std::make_shared;

class GTexture
{
public:
    ~GTexture() = default;
    virtual GFColor sample(double u, double v, const GMath::vec3& p) const =0;
    GFColor sample(const GMath::vec2& uv, const GMath::vec3& p)
    {
        return this->sample(uv.x(), uv.y(), p);
    }
};

class GSolidColor : public GTexture
{
public:
    GSolidColor(GFColor c):color(c){}
    GSolidColor(float r, float g, float b, float a=1):color(r,g,b,a){}

    GFColor sample(double u, double v, const GMath::vec3& p) const override
    {
        return color;
    }

private:
    GFColor color;
};

class GCheckerTexture:public GTexture
{
public:
    GCheckerTexture(double scale, shared_ptr<GTexture> even, shared_ptr<GTexture> odd)
        :invScale(1/scale), even(even), odd(odd)
    {}
    GCheckerTexture(double scale, GFColor even, GFColor odd)
        :invScale(1/scale), even(make_shared<GSolidColor>(even)), odd(make_shared<GSolidColor>(odd))
    {}

    GFColor sample(double u, double v, const GMath::vec3& p)const override;

private:
    double invScale;
    shared_ptr<GTexture> even;
    shared_ptr<GTexture> odd;
};

class GImageTexture : public GTexture
{
public:
    GImageTexture(const char* filepath);

    GFColor sample(double u, double v, const GMath::vec3& p)const override;
private:
    bool error = false;
    TGAImage image;
};

#endif // GTEXTURE_H
