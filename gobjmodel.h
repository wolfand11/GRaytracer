#ifndef GOBJMODEL_H
#define GOBJMODEL_H

#include "gmath.h"
#include "tgaimage.h"

class GOBJModel {
public:
    GOBJModel() = default;
    GOBJModel(const std::string filename);
    void Setup(const std::string filename);
    int nfaces() const;
    GMath::vec3 normal(const int iface, const int nthvert) const;
    GMath::vec4 tangent(const int iface, const int nthvert) const;
    GMath::vec3 normal(const GMath::vec2 &uv) const;
    GMath::vec3 vert(const int i) const;
    GMath::vec3 vert(const int iface, const int nthvert) const;
    GMath::vec2 uv(const int iface, const int nthvert) const;
    TGAColor diffuse(const GMath::vec2 &uv) const;
    double specular(const GMath::vec2 &uv) const;

    void GenTangent();
    std::vector<GMath::vec3> verts_;
    std::vector<GMath::vec2> uv_;
    std::vector<GMath::vec3> norms_;
    std::vector<GMath::vec4> tans_;
    std::vector<int> facet_vrt_;
    std::vector<int> facet_tex_;
    std::vector<int> facet_nrm_;
    TGAImage diffusemap_;
    TGAImage normalmap_;
    TGAImage specularmap_;
    std::string modelFilePath;
};

#endif // GOBJMODEL_H
