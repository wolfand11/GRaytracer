#ifndef GCAMERA_H
#define GCAMERA_H

#include "ggameobject.h"
#include "gray.h"
#include "gbuffer.h"

class GCamera : public GGameObject
{
public:
    enum GCameraType
    {
        kOrthographic,
        kProjection,
    };

    static std::shared_ptr<GCamera> CreateProjCamera(float near, float far, float fov);
    static GCamera* activeCamera;
    GCamera(GCameraType cameraType);
    void SetViewport(int x, int y, int w, int h);
    void SetFov(float fov);
    GMath::vec2 NDCPosToScreenPos(GMath::vec3 ndc);
    GMath::vec4 ScreenPosToViewPos(GMath::vec2 screenPos);
    float ToWBufferValue(float wValue);
    int viewportX;
    int viewportY;
    int viewportW;
    int viewportH;
    GCameraType cameraType;
    float fov{60};
    float near{0.1};
    float far{200.0f};
    float aspectRatio{1.0f};
    float focalDistance{10.f};
    float apertureRadius{1.f};
    GMath::mat4f& LookAt(GMath::vec3f eyePos, GMath::vec3f lookAtPoint, GMath::vec3f up);
    void ProjInvertProj(const GMath::mat4f*& tproj,const GMath::mat4f*& tinvertProj);
    GRay GetRay(double i, double j);
    //GFColor RayColor(const GRay& ray, int depth);

private:
    // camera
    GMath::mat4f projMat;
    GMath::mat4f invertProjMat;
    bool _proj_dirty = true;
};



#endif // GCAMERA_H
