#ifndef GCOLOR_H
#define GCOLOR_H

#include "gmath.h"
#include "tgaimage.h"

#define ColorChannelType unsigned int

struct GColor
{
    GColor() = default;
    GColor(ColorChannelType r, ColorChannelType g, ColorChannelType b, ColorChannelType a)
    {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    ColorChannelType r;
    ColorChannelType g;
    ColorChannelType b;
    ColorChannelType a;

    bool IsBlack()
    {
        return (r==0 && g==0 && b==0);
    }

    static GColor clear;
    static GColor white;
    static GColor black;
    static GColor red;
    static GColor green;
    static GColor blue;
    static GColor gray;
    static GColor pink;
    static GColor normal;

    static GMath::vec4 FastTonemap(GMath::vec4 color)
    {
        GMath::vec4 ret;
        ret.SetX((color[0] * 1.0/(color[0]+1.0)));
        ret.SetY((color[1] * 1.0/(color[1]+1.0)));
        ret.SetZ((color[2] * 1.0/(color[2]+1.0)));
        ret.SetW(std::max(0.0, std::min(color[3], 1.0)));
        return ret;
    }

    static GMath::vec4f ToFloat01Color(const GColor& color)
    {
        GMath::vec4f ret;
        ret.SetX(((float)color.r)/255.0);
        ret.SetY(((float)color.g)/255.0);
        ret.SetZ(((float)color.b)/255.0);
        ret.SetW(((float)color.a)/255.0);
        return ret;
    }

    static GMath::vec4f ToFloatColor(const GColor& color)
    {
        GMath::vec4f ret;
        ret.SetX((float)color.r);
        ret.SetY((float)color.g);
        ret.SetZ((float)color.b);
        ret.SetW((float)color.a);
        return ret;
    }

    static GColor FromFloat01Color(GMath::vec4 color)
    {
        GColor ret;
        ret.r = (ColorChannelType)(color[0] * 255);
        ret.g = (ColorChannelType)(color[1] * 255);
        ret.b = (ColorChannelType)(color[2] * 255);
        ret.a = (ColorChannelType)(color[3] * 255);
        return ret;
    }

    static GColor FromFloatColor(GMath::vec4 color)
    {
        GColor ret;
        ret.r = (ColorChannelType)(color[0]);
        ret.g = (ColorChannelType)(color[1]);
        ret.b = (ColorChannelType)(color[2]);
        ret.a = (ColorChannelType)(color[3]);
        return ret;
    }

    static GColor TgaColor(TGAColor tgaColor)
    {
        GColor color;
        color.r = tgaColor.bgra[2];
        color.g = tgaColor.bgra[1];
        color.b = tgaColor.bgra[0];
        color.a = tgaColor.bgra[3];
        return color;
    }

    static GColor Lerp(TGAColor color1, TGAColor color2, float f);
    static GColor Lerp(GColor color1, GColor color2, float f);

    GMath::vec4f ToFloat01Color()
    {
        return ToFloat01Color(*this);
    }

    GMath::vec4f ToFloatColor()
    {
        return ToFloatColor(*this);
    }
};

struct GHDRColor
{
    ColorChannelType r;
    ColorChannelType g;
    ColorChannelType b;
    ColorChannelType a;

    float intensity;

    static GMath::vec4f ToFloatColor(const GHDRColor& hdrColor)
    {
        GMath::vec4f ret;
        float factor = std::pow(2, hdrColor.intensity);
        ret.SetX(factor * float(hdrColor.r)/255);
        ret.SetY(factor * float(hdrColor.g)/255);
        ret.SetY(factor * float(hdrColor.b)/255);
        ret.SetW(float(hdrColor.a)/255);
        return ret;
    }

    GMath::vec4f ToFloatColor()
    {
        return ToFloatColor(*this);
    }
};
#endif // GCOLOR_H
