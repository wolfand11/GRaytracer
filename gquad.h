#ifndef GQUAD_H
#define GQUAD_H

#include "gshape.h"
#include "gmodel.h"
#include "gobjmodel.h"

class GQuad : public GShape
{
public:
    GQuad(std::shared_ptr<GOBJModel> objModel, int faceIndex)
        :objModel(objModel), faceIndex(faceIndex)
    {
        Q = objModel->vert(faceIndex, 0);
        auto p1 = objModel->vert(faceIndex, 1);
        auto p2 = objModel->vert(faceIndex, 3);
        vec3 normal0 = objModel->normal(faceIndex, 0);
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

        bbox = aabb(Q, Q+u+v).pad();
        area =tmpN.length();
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
    double area;
};

#endif // GQUAD_H
