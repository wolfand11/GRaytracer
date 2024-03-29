#ifndef GUTILS_H
#define GUTILS_H
#include<string>
#include "gmath.h"
#include "gbuffer.h"
#include "tgaimage.h"


enum GTextureWrapMode
{
    kTWMRepeat,
    kTWMClamp
};

enum GDebugType
{
    kNone,
    kDiffuseTex,
    kNormalTex,
    kWorldNormal,
    kWorldTangent,
};

class GUtils
{
public:
    static int screenWidth;
    static int screenHeight;
    static float screenAspectRatio();
    static float worldSize;

    static GDebugType debugType;

    static std::string GetProjRootPath();
    static std::string GetAbsPath(const std::string& relativePath);
    static bool IsFileExist(const std::string filepath);

    // sample image
    static GColor SampleImage(const TGAImage* img, GMath::vec2 uv, GTextureWrapMode wrapMode=GTextureWrapMode::kTWMClamp, GColor defaultColor=GColor::black);
    static GColor SampleImage(const std::vector<TGAImage>* mipmaps, GMath::vec2 uv, int mipmapLevel, GTextureWrapMode wrapMode=GTextureWrapMode::kTWMClamp, GColor defaultColor=GColor::black);

    // vector
    template<typename T>
    static void ReleaseVector(std::vector<T*>& v)
    {
        for(size_t i=0; i<v.size(); i++)
        {
            if(v[i]!=nullptr)
            {
                delete v[i];
                v[i] = nullptr;
            }
        }
    }
};

#endif // GUTILS_H
