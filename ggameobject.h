#ifndef GGAMEOBJECT_H
#define GGAMEOBJECT_H
#include "gmath.h"

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
    const GMath::vec3f& position()const { return _position; }
    const GMath::vec3f& rotation()const { return _rotation; }
    const GMath::vec3f& scale()const { return _scale; }

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

#endif // GGAMEOBJECT_H
