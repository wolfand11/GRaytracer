#include "gmaterial.h"
#include "gbsdf.h"
using namespace std;
using namespace GMath;

void GLambertianMaterial::ComputeScatteringFunctions(GMath::GSurfaceInteraction *isect)
{
    // TODO geometry normal, shading normal
    isect->bsdf = make_shared<GBSDF>(isect->normal, isect->normal);
    if(!GColor::IsBlack(Kd))
    {
        vec3 normalColor = (isect->normal + vec3::one) * 0.5;
        GFColor normalFColor = GFColor(normalColor.x(), normalColor.y(), normalColor.z(), 1);
        //auto lambertianReflection = make_shared<GLambertianReflection>(normalFColor);
        auto lambertianReflection = make_shared<GLambertianReflection>(Kd);
        isect->bsdf->bxdfs.push_back(lambertianReflection);
    }
}
