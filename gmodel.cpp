#include "gmodel.h"
#include "gtriangle.h"
using namespace GMath;
using namespace std;

aabb GModel::bBox()
{
    aabb localBBox = shape->bBox();

    const mat4f* obj2World; // obj2World
    const mat4f* world2Obj; // world2Obj
    TRSInvertTRS(obj2World, world2Obj);

    aabb wBBox = transformBBox(localBBox, *obj2World);
    return wBBox;
}

bool GSphereModel::intersect(const GRay &ray, GMath::interval ray_t, GSurfaceInteraction &isect)
{
    auto sphere = std::dynamic_pointer_cast<GSphere>(shape);
    sphere->center = _position;
    if(sphere->intersect(ray, ray_t, isect))
    {
        isect.model = this;
        isect.uv = sphere->getUV(isect.p);
        isect.material = material.get();
        isect.light = nullptr;
        return true;
    }
    return false;
}

std::vector<std::shared_ptr<GTriangleModel> > GTriangleModel::CreateTriangleMesh(const std::string objModelPath, std::shared_ptr<GMaterial> material)
{
    auto objModel = make_shared<GOBJModel>(objModelPath);
    return CreateTriangleMesh(objModel, material);
}

std::vector<std::shared_ptr<GTriangleModel>>
GTriangleModel::CreateTriangleMesh(std::shared_ptr<GOBJModel> objModel, std::shared_ptr<GMaterial> material)
{
    std::vector<std::shared_ptr<GTriangleModel>> trianges;
    auto gray = std::make_shared<GCheckerTexture>(0.2, GColor::whiteF, GColor::grayF);
    material->Init(gray, gray, gray);
    for(int i=0; i<objModel->nfaces(); i++)
    {
        auto triShape = std::make_shared<GTriangle>(objModel, i);
        auto tri = std::make_shared<GTriangleModel>(triShape, material);
        triShape->owner = tri.get();
        trianges.push_back(tri);
    }
    return trianges;
}

bool GTriangleModel::intersect(const GRay &ray, interval ray_t, GSurfaceInteraction &isect)
{
    auto tri = std::dynamic_pointer_cast<GTriangle>(shape);
    if(tri->intersect(ray, ray_t, isect))
    {
        isect.model = this;
        isect.material = material.get();
        isect.light = nullptr;
        return true;
    }
    return false;
}
