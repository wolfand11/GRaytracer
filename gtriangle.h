#ifndef GTRIANGLE_H
#define GTRIANGLE_H

#include "gmodel.h"
#include "gobjmodel.h"

class GTriangle : public GShape
{
public:
    GTriangle(std::shared_ptr<GOBJModel> objModel, int faceIndex)
        :objModel(objModel), faceIndex(faceIndex)
    {
        Q = objModel->vert(faceIndex, 0);
        auto p1 = objModel->vert(faceIndex, 1);
        auto p2 = objModel->vert(faceIndex, 2);
        u = p1 - Q;
        v = p2 - Q;
        auto tmpN = cross(u, v);
        geoNormal = tmpN;
        D = dot(geoNormal.normalize(), Q);
        w = tmpN / dot(tmpN,tmpN);

        bbox = aabb(Q, Q+u+v).pad();
    }

    GSurfaceInteraction Sample(const GGameObject& owner, float& pdf, double time) const override;
    GSurfaceInteraction Sample(const GGameObject& owner, const GSurfaceInteraction& ref, float& pdf) const override;
    bool intersect(const GRay& ray, GMath::interval ray_t, GSurfaceInteraction& isect) override;

    std::shared_ptr<GOBJModel> objModel;
    int faceIndex;
    vec3 geoNormal;
    vec3 Q;
    vec3 u;
    vec3 v;
    double D;
    vec3 w;
    double area;
};

class GTriangleList : public GShapeList
{
public:
    std::shared_ptr<GOBJModel> objModel;
};

#endif // GTRIANGLE_H
