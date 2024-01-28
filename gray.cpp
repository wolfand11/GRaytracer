#include "gray.h"
#include "gmaterial.h"
#include "glight.h"
using namespace GMath;
using namespace std;

GMath::GRay::GRay(vec3 origin, vec3 dir)
    :origin(origin), dir(dir)
{

}

GMath::vec3 GMath::GRay::GetPos(float t) const
{
    return origin + dir * t;
}

GRayDifferential::GRayDifferential(vec3 originX, vec3 dirX, vec3 originY, vec3 dirY)
    : xRay(originX, dirX), yRay(originY, dirY)
{
}

void GSurfaceInteraction::ComputeScatteringFunctions()
{
    mateiral->ComputeScatteringFunctions(this);
}

GFColor GSurfaceInteraction::Le(const GScene& scene, const vec3 &w) const
{
    if(light!=nullptr)
    {
        return light->Le(scene, *this, w);
    }
    return GColor::blackF;
}
