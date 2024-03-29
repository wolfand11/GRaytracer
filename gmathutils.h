#ifndef GMATHUTILS_H
#define GMATHUTILS_H
#include "gmath.h"
#include "gray.h"


class GMathUtils
{
public:
    static int FloatNegativOneToOne2Int32(float src);
    template<typename T>
    static T saturate(T v)
    {
        v = std::max(std::min(v, (T)1), (T)0);
        return v;
    }

    // vec helper
    static void CoordinateSystem(const GMath::vec3& v0, GMath::vec3& v1, GMath::vec3& v2);

    // matrix helper
    static GMath::mat4 LookAt(GMath::vec3 eyePos, GMath::vec3 lookAtPoint, GMath::vec3 up);
    static void DecomposeMatrix(const GMath::mat4f& mat, GMath::mat4f& translate, GMath::mat4f& rotation, GMath::mat4f& scale);
    static void DecomposeMatrix(const GMath::mat4f& mat, GMath::vec3f& translate, GMath::vec3f& rotation, GMath::vec3f& scale);
    static GMath::vec3f RotationMatrixToEulerAngle(GMath::mat4f matrix);
    static GMath::mat4f EulerAngleToRotationMatrix(GMath::vec3f eulerAngle);
    static GMath::mat4f TRS(GMath::vec3f& translate, GMath::vec3f& rotation, GMath::vec3f& scale);
    static GMath::mat3f RotationMatrix(GMath::vec3f fromV, GMath::vec3f toV);
    static void SameHemisphere(const GMath::vec3& normal, const GMath::vec3& wo, GMath::vec3& wi);

    // radian degree
    static float Rad2Deg(float rad);
    static float Deg2Rad(float degree);
    static GMath::vec3f Rad2Deg(GMath::vec3f rad);
    static GMath::vec3f Deg2Rad(GMath::vec3f degree);

    // color helper
    static GFColor SchlickFresnel(GFColor Ks, float cosTheta);
};

#endif // GMATHUTILS_H
