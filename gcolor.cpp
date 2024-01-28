#include "gcolor.h"
#include "glog.h"
using namespace std;
using namespace GMath;

GColor GColor::clear = {0,0,0,0};
GColor GColor::white = {255,255,255,255};
GColor GColor::black = {0,0,0,255};
GColor GColor::red = {255,0,0,255};
GColor GColor::green = {0,255,0,255};
GColor GColor::blue = {0,0,255,255};
GColor GColor::gray = {128,128,128,255};
GColor GColor::pink = {255,0,255,255};
GColor GColor::normal = {128,128,255,0};

GFColor GColor::clearF = {0,0,0,0};
GFColor GColor::whiteF = {1,1,1,1};
GFColor GColor::blackF = {0,0,0,1};
GFColor GColor::redF = {1,0,0,1};
GFColor GColor::greenF = {0,1,0,1};
GFColor GColor::blueF = {0,0,1,1};
GFColor GColor::grayF = {0.5,0.5,0.5,1};
GFColor GColor::pinkF = {1,0,1,1};
GFColor GColor::normalF = {0.5,0.5,1,0};

GColor GColor::Lerp(TGAColor color1, TGAColor color2, float f)
{
    GColor ret;
    ret.r = color1.bgra[2] * f + color2.bgra[2] * (1-f);
    ret.g = color1.bgra[1] * f + color2.bgra[1] * (1-f);
    ret.b = color1.bgra[0] * f + color2.bgra[0] * (1-f);
    ret.a = color1.bgra[3] * f + color2.bgra[3] * (1-f);
    return ret;
}

GColor GColor::Lerp(GColor color1, GColor color2, float f)
{
    GColor ret;
    ret.r = color1.r * f + color2.r * (1-f);
    ret.g = color1.g * f + color2.g * (1-f);
    ret.b = color1.b * f + color2.b * (1-f);
    ret.a = color1.a * f + color2.a * (1-f);
    return ret;

}

GFColor GColor::Lerp(GFColor color1, GFColor color2, float f)
{
    GFColor ret;
    ret.SetX(color1.x() * f + color2.x() * (1-f));
    ret.SetY(color1.y() * f + color2.y() * (1-f));
    ret.SetZ(color1.z() * f + color2.z() * (1-f));
    ret.SetW(color1.w() * f + color2.w() * (1-f));
    return ret;
}
