#include "gray.h"
#include "glight.h"
using namespace GMath;
using namespace std;

GRay::GRay(vec3 origin, vec3 dir, double time)
    :origin(origin), dir(dir), time(time)
{

}

GMath::vec3 GRay::GetPos(float t) const
{
    return origin + dir * t;
}

GRayDifferential::GRayDifferential(vec3 originX, vec3 dirX, vec3 originY, vec3 dirY)
    : xRay(originX, dirX), yRay(originY, dirY)
{
}

GFColor GSurfaceInteraction::Le(const GScene& scene, const vec3 &w) const
{
    if(light!=nullptr)
    {
        return light->Le(scene, *this, w);
    }
    return GColor::blackF;
}
