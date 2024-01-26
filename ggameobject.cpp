#include "ggameobject.h"
#include "glog.h"
#include "gmathutils.h"
#include "gutils.h"
using namespace GMath;
using namespace std;


void GGameObject::SetT(vec3f pos)
{
    this->_position = pos;
    _trs_dirty = true;
}

void GGameObject::SetR(vec3f rotation)
{
    this->_rotation.SetX(GMathUtils::Deg2Rad(rotation.x()));
    this->_rotation.SetY(GMathUtils::Deg2Rad(rotation.y()));
    this->_rotation.SetZ(GMathUtils::Deg2Rad(rotation.z()));
    _trs_dirty = true;
}

void GGameObject::SetS(vec3f scale)
{
    this->_scale = scale;
    _trs_dirty = true;
}

void GGameObject::SetTRS(vec3f pos, vec3f rotation, vec3f scale)
{
    SetT(pos);
    SetR(rotation);
    SetS(scale);
}

void GGameObject::TRSInvertTRS(const mat4f* &trs, const mat4f* &invertTRS)
{
    if(_trs_dirty)
    {
        transform = GMathUtils::TRS(_position, _rotation, _scale);
        invertTransform = transform.invert();
        _trs_dirty = false;
    }
    trs = &transform;
    invertTRS = &invertTransform;
}



std::shared_ptr<GLight> GLight::CreateLightGObj(GLightType lightType, GColor lColor, float lIntensity)
{
    auto light = std::make_shared<GLight>(lightType);
    light->lightInfo.lightColor = lColor;
    light->lightInfo.lightIntensity = lIntensity;
    return light;
}

GLight::GLight(GLightType lightType)
    :GGameObject(GGameObject::GGameObjectType::kLight)
{
    lightInfo.lightType = lightType;
}

