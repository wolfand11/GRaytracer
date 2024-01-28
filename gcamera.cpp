#include "gcamera.h"
#include "gutils.h"
#include "gmathutils.h"

using namespace GMath;
using namespace std;

GCamera* GCamera::activeCamera = nullptr;

std::shared_ptr<GCamera> GCamera::CreateProjCamera(float near, float far, float fov)
{
    float aspectRatio = GUtils::screenAspectRatio();
    auto cameraGObj = std::make_shared<GCamera>(GCameraType::kProjection);
    cameraGObj->near = near;
    cameraGObj->far = far;
    cameraGObj->fov = fov;
    assert(far>near);
    cameraGObj->aspectRatio = aspectRatio;
    cameraGObj->_proj_dirty = true;
    return cameraGObj;
}

GCamera::GCamera(GCameraType cameraType)
    :GGameObject(GGameObjectType::kCamera),cameraType(cameraType)
{

}

void GCamera::SetViewport(int x, int y, int w, int h)
{
    viewportX = x;
    viewportY = y;
    viewportW = w;
    viewportH = h;
}

void GCamera::SetFov(float fov)
{
    this->fov = fov;
    _proj_dirty = true;
}

vec2 GCamera::NDCPosToScreenPos(vec3 ndc)
{
    vec2 ret;
    ret.SetX((ndc.x()+1.0)*viewportW*0.5+viewportX);
    ret.SetY((ndc.y()+1.0)*viewportH*0.5+viewportY);
    return ret;
}

vec4 GCamera::ScreenPosToViewPos(GMath::vec2 screenPos)
{
    vec2 normalizedScreenPos;
    normalizedScreenPos.SetX((screenPos.x()-viewportX)/viewportW * 2.0 - 1.0);
    normalizedScreenPos.SetY((screenPos.y()-viewportY)/viewportH * 2.0 - 1.0);
    vec4 homogeneousPos = vec4::one;
    homogeneousPos.SetX(normalizedScreenPos.x());
    homogeneousPos.SetY(normalizedScreenPos.y());
    homogeneousPos.SetZ(-1); // near plane
    homogeneousPos = homogeneousPos * near;
    const mat4f* projMat;
    const mat4f* invertProjMat;
    ProjInvertProj(projMat, invertProjMat);
    vec4 viewPos = (*invertProjMat) * (vec4f)homogeneousPos;
    viewPos.SetW(1);
    return viewPos;
}

float GCamera::ToWBufferValue(float wValue)
{
    wValue = min(max(wValue, near), far);
    wValue -= near;
    wValue /= (far-near);
    return (wValue*2.0-1.0);
}

GMath::mat4f &GCamera::LookAt(GMath::vec3f eyePos, GMath::vec3f lookAtPoint, GMath::vec3f up)
{
    _position = eyePos;
    _scale = vec3f::one;

    transform.identity();
    vec3f forward = (lookAtPoint - eyePos).normalize();
    vec3f right = cross(up, forward).normalize();
    up = cross(forward, right).normalize();
    transform.set_col(0, embed<float,4>(right,0));
    transform.set_col(1, embed<float,4>(up,0));
    transform.set_col(2, embed<float,4>(forward,0));
    _rotation = GMathUtils::RotationMatrixToEulerAngle(transform);

    transform[0][3] = eyePos[0];
    transform[1][3] = eyePos[1];
    transform[2][3] = eyePos[2];
    // TODO calc invertTransform to set _trs_dirty false
    _trs_dirty = true;
    return transform;
}

void GCamera::ProjInvertProj(const mat4f*& tproj,const mat4f*& tinvertProj)
{
    if(_proj_dirty)
    {
        // fov axis is yAxis
        float zoomY = 1.0/std::tan(GMathUtils::Deg2Rad(fov/2.0));
        float zoomX = zoomY / aspectRatio;
        projMat.zero();
        projMat[0][0] = zoomX;
        projMat[1][1] = zoomY;
        projMat[2][2] = (far+near)/(far-near);
        projMat[2][3] = (-2*near*far)/(far-near);
        projMat[3][2] = 1;
        projMat[3][3] = 0;

        invertProjMat.zero();
        invertProjMat[0][0] = 1.0f/zoomX;
        invertProjMat[1][1] = 1.0f/zoomY;
        invertProjMat[2][2] = 0;
        invertProjMat[2][3] = 1;
        invertProjMat[3][2] = (near-far)/(2*near*far);
        invertProjMat[3][3] = (near+far)/(2*near*far);
        _proj_dirty = false;
    }
    tproj = &projMat;
    tinvertProj = &invertProjMat;
}

GRay GCamera::GetRay(double i, double j)
{
    GRay r;
    vec4 viewPos = ScreenPosToViewPos(vec2(i,j));
    const mat4f* trsMat; // view2World
    const mat4f* invertTRSMat; // world2View
    TRSInvertTRS(trsMat, invertTRSMat);
    vec4 wPos =  (*trsMat) * (vec4f)viewPos;
    r.origin = _position;
    r.dir = vec3(wPos.x()-_position.x(), wPos.y()-_position.y(), wPos.z()-_position.z());
    r.dir.normalize();
    return r;
}

/*
GFColor GCamera::RayColor(const GMath::GRay &ray, int depth)
{
    vec3 normalizedDir = ray.dir;
    normalizedDir.normalize();
    auto a = 0.5 * (normalizedDir.y() + 1.0);
    GFColor ret = GColor::Lerp(vec4f(0.5,0.7,1.0,1.0), GColor::whiteF, a);
    return ret;
}
*/
