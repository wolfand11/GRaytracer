#include "gtexture.h"
#include "glog.h"
#include "gutils.h"

using namespace GMath;

GFColor GCheckerTexture::sample(double u, double v, const GMath::vec3 &p) const
{
    auto x = (int)std::floor(invScale * p.x());
    auto y = (int)std::floor(invScale * p.y());
    auto z = (int)std::floor(invScale * p.z());

    bool isEven = (x + y + z)%2 == 0;
    return isEven ? even->sample(u, v, p) : odd->sample(u, v, p);
}

GImageTexture::GImageTexture(const char *filepath)
{
    if(!image.read_tga_file(filepath))
    {
        GLog::LogError("load image failed:", filepath);
        error = true;
    }
}

GImageTexture::GImageTexture(TGAImage image)
    :image(image)
{
}

GFColor GImageTexture::sample(double u, double v, const GMath::vec3 &p) const
{
    GColor errorColor(255,0,255,255);

    GColor c = GUtils::SampleImage(&image, vec2(u,v), GTextureWrapMode::kTWMClamp, errorColor);
    return GColor::ToFloat01Color(c);
}
