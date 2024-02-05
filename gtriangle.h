#ifndef GTRIANGLE_H
#define GTRIANGLE_H

#include "gshape.h"
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
        uv0 = objModel->uv(faceIndex, 0);
        uv1 = objModel->uv(faceIndex, 1);
        uv2 = objModel->uv(faceIndex, 2);
        normal0 = objModel->normal(faceIndex, 0);
        normal1 = objModel->normal(faceIndex, 1);
        normal2 = objModel->normal(faceIndex, 2);

        u = p1 - Q;
        v = p2 - Q;
        auto tmpN = cross(u, v);
        geoNormal = tmpN;
        if(dot(normal0, geoNormal.normalize()) < 0)
        {
            geoNormal = -geoNormal;
        }
        D = dot(geoNormal, Q);
        w = tmpN / dot(tmpN,tmpN); // I / 2*area

        // calc tangent space tangent
        auto bitangent = vec3(uv2.x()-uv0.x(), uv2.y() - uv0.y(), 0).normalize();
        //tangent0 = cross(normal0, bitangent);
        //tangent1 = cross(normal1, bitangent);
        //tangent2 = cross(normal2, bitangent);
        tangent0 = objModel->tangent(faceIndex, 0).xyz();
        tangent1 = objModel->tangent(faceIndex, 1).xyz();
        tangent2 = objModel->tangent(faceIndex, 2).xyz();

        bbox = aabb(Q, p1, p2).pad();
        area =tmpN.length() * 0.5;
    }

    GSurfaceInteraction Sample(float& pdf, double time) const override;
    GSurfaceInteraction Sample(const GSurfaceInteraction& ref, float& pdf) const override;
    bool intersect(const GRay& ray, GMath::interval ray_t, GSurfaceInteraction& isect) override;

    std::shared_ptr<GOBJModel> objModel;
    int faceIndex;
    vec3 geoNormal;
    vec3 Q;
    vec3 u;
    vec3 v;
    double D;
    vec3 w;
    vec2 uv0;
    vec2 uv1;
    vec2 uv2;
    vec3 normal0;
    vec3 normal1;
    vec3 normal2;
    vec3 tangent0;
    vec3 tangent1;
    vec3 tangent2;
    double area;
};
#endif // GTRIANGLE_H
