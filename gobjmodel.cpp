#include "gobjmodel.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
using namespace GMath;
using namespace std;

GOBJModel::GOBJModel(const std::string filename)
    :verts_(), uv_(), norms_(), facet_vrt_(), facet_tex_(), facet_nrm_(), diffusemap_(), normalmap_(), specularmap_()
{
    Setup(filename);
}

void GOBJModel::Setup(const std::string filename)
{
    modelFilePath = filename;
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail())
    {
        std::cerr << "open model failed. " << filename << std::endl;
        return;
    }
    std::string line;
    while (!in.eof())
    {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            vec3 v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            vec3 n;
            for (int i=0;i<3;i++) iss >> n[i];
            norms_.push_back(n.normalize());
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            vec2 uv;
            for (int i=0;i<2;i++) iss >> uv[i];
            uv_.push_back(uv);
        }  else if (!line.compare(0, 2, "f ")) {
            int f,t,n;
            iss >> trash;
            int cnt = 0;
            while (iss >> f >> trash >> t >> trash >> n) {
                facet_vrt_.push_back(--f);
                facet_tex_.push_back(--t);
                facet_nrm_.push_back(--n);
                cnt++;
            }
            if (3!=cnt) {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                in.close();
                return;
            }
        }
    }
    in.close();

    // gen tangent
    GenTangent();

    std::cerr << "# v# " << verts_.size() << " f# "  << nfaces() << " vt# " << uv_.size() << " vn# " << norms_.size() << std::endl;
    string diffusePath = std::regex_replace(filename, std::regex("\.obj"), "_diffuse.tga");
    if(std::filesystem::exists(diffusePath))
    {
        if(!diffusemap_.read_tga_file(diffusePath))
        {
            std::cout << "load file failed! " << diffusePath << std::endl;
        }
        else
        {
            // we use OpenGL style uv
            diffusemap_.flip_vertically();
        }
    }
    string normalPath = std::regex_replace(filename, std::regex("\.obj"), "_normal.tga");
    if(std::filesystem::exists(normalPath))
    {
        if(!normalmap_.read_tga_file(normalPath))
        {
            std::cout << "load file failed! " << normalPath << std::endl;
        }
        else
        {
            // we use OpenGL style uv
            normalmap_.flip_vertically();
        }
    }
}

int GOBJModel::nfaces() const {
    return facet_vrt_.size()/3;
}

vec3 GOBJModel::vert(const int i) const {
    return verts_[i];
}

vec3 GOBJModel::vert(const int iface, const int nthvert) const {
    return verts_[facet_vrt_[iface*3+nthvert]];
}

TGAColor GOBJModel::diffuse(const vec2 &uvf) const {
    return diffusemap_.get(uvf[0]*diffusemap_.get_width(), uvf[1]*diffusemap_.get_height());
}

vec3 GOBJModel::normal(const vec2 &uvf) const {
    TGAColor c = normalmap_.get(uvf[0]*normalmap_.get_width(), uvf[1]*normalmap_.get_height());
    vec3 res;
    for (int i=0; i<3; i++)
        res[2-i] = c[i]/255.*2 - 1;
    return res;
}

double GOBJModel::specular(const vec2 &uvf) const {
    return specularmap_.get(uvf[0]*specularmap_.get_width(), uvf[1]*specularmap_.get_height())[0];
}

void GOBJModel::GenTangent()
{
    int normsCount = norms_.size();
    auto tmpTangent = std::vector<vec3>(normsCount,vec3::zero);
    auto tmpBitangent = std::vector<vec3>(normsCount,vec3::zero);

    for(int i=0; i<nfaces(); i++)
    {
        vec3 p0 = vert(i, 0);
        vec3 p1 = vert(i, 1);
        vec3 p2 = vert(i, 2);
        vec2 uv0 = uv(i, 0);
        vec2 uv1 = uv(i, 1);
        vec2 uv2 = uv(i, 2);

        vec3 q1 = p1 - p0;
        vec3 q2 = p2 - p0;
        auto s1 = uv1.x() - uv0.x();
        auto s2 = uv2.x() - uv0.x();
        auto t1 = uv1.y() - uv0.y();
        auto t2 = uv2.y() - uv0.y();

        vec3 tangent = (q1*t2 - q2*t1).normalize();
        vec3 bitangent = (q2*s1 - q1*s2).normalize();
        for(int j=0; j<3; j++)
        {
            tmpTangent[facet_nrm_[i*3+j]] = tmpTangent[facet_nrm_[i*3+j]] + tangent;
            tmpBitangent[facet_nrm_[i*3+j]] = tmpBitangent[facet_nrm_[i*3+j]] + bitangent;
        }
    }

    tans_.assign(normsCount, vec4::zero);
    for(int i=0; i<normsCount; i++)
    {
        vec3 normal = norms_[i];
        vec3 tangent = tmpTangent[i];
        tangent = tangent - normal * dot(tangent, normal);
        tans_[i].SetXYZ(tangent.normalize());

        if(dot(cross(normal, tangent), tmpBitangent[i]) < 0.0f)
        {
            tans_[i].SetW(-1.0f);
        }
        else
        {
            tans_[i].SetW(+1.0f);
        }
    }
}

vec2 GOBJModel::uv(const int iface, const int nthvert) const {
    return uv_[facet_tex_[iface*3+nthvert]];
}

vec3 GOBJModel::normal(const int iface, const int nthvert) const {
    return norms_[facet_nrm_[iface*3+nthvert]];
}

vec4 GOBJModel::tangent(const int iface, const int nthvert) const
{
    return tans_[facet_nrm_[iface*3+nthvert]];
}
