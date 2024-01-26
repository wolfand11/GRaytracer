#include "gray.h"
using namespace GMath;

GMath::GRay::GRay(vec3 origin, vec3 dir)
    :origin(origin), dir(dir)
{

}

GMath::vec3 GMath::GRay::GetPoint(float t)
{
    return origin + dir * t;
}

GRayDifferential::GRayDifferential(vec3 originX, vec3 dirX, vec3 originY, vec3 dirY)
    : xRay(originX, dirX), yRay(originY, dirY)
{
}
