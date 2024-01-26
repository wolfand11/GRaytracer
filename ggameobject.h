#ifndef GGAMEOBJECT_H
#define GGAMEOBJECT_H
#include "gmath.h"
#include "gcolor.h"

enum GLightType
{
    kLTDirection,
    kLTPoint,
};

struct GLightInfo
{
    GColor lightColor;
    float lightIntensity;
    GLightType lightType;
    GMath::vec3 lightPosOrDir;
};

class GGameObject
{
public:
    enum GGameObjectType
    {
        kCamera,
        kLight,
        kModel,
    };

    GGameObject()=default;
    virtual ~GGameObject(){};
    GGameObject(GGameObjectType t):mtype(t){}

    // common
    void SetT(GMath::vec3f pos);
    void SetR(GMath::vec3f rotation);
    void SetS(GMath::vec3f scale);
    void SetTRS(GMath::vec3f pos,GMath::vec3f rotation,GMath::vec3f scale);
    void TRSInvertTRS(const GMath::mat4f*& trs, const GMath::mat4f*& invertTRS);
    const GMath::vec3f& position() { return _position; }
    const GMath::vec3f& rotation() { return _rotation; }
    const GMath::vec3f& scale() { return _scale; }

    GGameObjectType mtype;
    GMath::vec3f right();
    GMath::vec3f up();
    GMath::vec3f forward();
protected:
    // common
    GMath::vec3f _position;
    GMath::vec3f _rotation;
    GMath::vec3f _scale = {1,1,1};
    bool _trs_dirty = true;
    GMath::mat4f transform;
    GMath::mat4f invertTransform;
};

class GLight : public GGameObject
{
public:
    // light
    static std::shared_ptr<GLight> CreateLightGObj(GLightType lightType, GColor lColor=GColor::white, float lIntensity=1);
    GLight(GLightType lightType);
    GLightInfo lightInfo;
};

#endif // GGAMEOBJECT_H
