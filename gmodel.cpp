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
        isect.material = material.get();
        isect.material->UpdateShadingNormal(isect);
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
    auto gray = std::make_shared<GSolidColor>(GColor::grayF);
    std::vector<std::shared_ptr<GTriangleModel>> trianges;
    std::shared_ptr<GTexture> diffuse;
    if(objModel->diffusemap_.get_width() > 0)
    {
        diffuse = make_shared<GImageTexture>(objModel->diffusemap_);
    }
    else
    {
        diffuse = gray;
    }
    std::shared_ptr<GTexture> normal;
    if(objModel->normalmap_.get_width() > 0)
    {
        normal = make_shared<GImageTexture>(objModel->normalmap_);
    }
    else
    {
        normal = std::make_shared<GSolidColor>(GColor::normalF);
    }
    material->Init(diffuse, gray, gray, normal);
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
        isect.material->UpdateShadingNormal(isect);
        isect.light = nullptr;
        return true;
    }
    return false;
}
