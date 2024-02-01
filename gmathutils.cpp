#include "gmathutils.h"
#include <algorithm>
using namespace std;
using namespace GMath;

int GMathUtils::FloatNegativOneToOne2Int32(float src)
{
    src = min(src, 1.0f);
    src = max(src, -1.0f);
    return (int)(src*(double)INT_MAX);
}

void GMathUtils::CoordinateSystem(const GMath::vec3 &v0, GMath::vec3 &v1, GMath::vec3 &v2)
{
    if(std::abs(v0.x()) > std::abs(v0.y()))
    {
        v1 = vec3(-v0.z(), 0, v0.x()).normalize();
    }
    else
    {
        v1 = vec3(0, v0.z(), -v0.y()).normalize();
    }
    v2 = cross(v0, v1);
}

GMath::mat4 GMathUtils::LookAt(GMath::vec3 eyePos, GMath::vec3 lookAtPoint, GMath::vec3 up)
{
    mat4 ret;
    ret.identity();
    ret[0][3] = eyePos[0];
    ret[1][3] = eyePos[2];
    ret[2][3] = eyePos[3];

    vec3 forward = (lookAtPoint - eyePos).normalize();
    vec3 right = cross(up, forward).normalize();
    up = cross(forward, right).normalize();
    for(int ncol=0; ncol<3; ncol++)
    {
        ret[0][ncol] = right[ncol];
        ret[1][ncol] = up[ncol];
        ret[2][ncol] = forward[ncol];
    }
    return ret;
}

void GMathUtils::DecomposeMatrix(const mat4f &mat, mat4f &translate, mat4f &rotation, mat4f &scale)
{
    translate.identity();
    rotation.identity();
    scale.identity();
    translate[0][3] = mat.rows[0][3];
    translate[1][3] = mat.rows[1][3];
    translate[2][3] = mat.rows[2][3];

    mat4f RS = mat;
    for(int i=0; i<3; i++)
    {
        RS.rows[i][3] = RS.rows[3][i] = 0.f;
    }
    RS.rows[3][3] = 1.0;

    float delta = 0.0f;
    int count = 0;
    rotation = RS;
    do
    {
        mat4f Rnext;
        mat4f Rit = rotation.transpose().invert();
        for(int i=0; i<4; i++)
        {
            for(int j=0; j<4; j++)
            {
                Rnext[i][j] = 0.5f * (rotation[i][j] + Rit[i][j]);
            }
        }
        delta = 0.f;
        for(int i=0; i<3; i++)
        {
            delta = std::abs(rotation[i][0]-Rnext[i][0]) +
                    std::abs(rotation[i][1]-Rnext[i][1]) +
                    std::abs(rotation[i][2]-Rnext[i][2]);
            delta = std::max(delta, 0.0f);
        }
        rotation = Rnext;
    } while(++count<100 && delta>0.0001);

    scale = RS * rotation.invert();
}

void GMathUtils::DecomposeMatrix(const mat4f& mat, vec3f &translate, vec3f &rotation, vec3f &scale)
{
    mat4f _trans, _rot, _scale;
    DecomposeMatrix(mat, _trans, _rot, _scale);
    translate.SetX(mat.rows[0][3]);
    translate.SetY(mat.rows[1][3]);
    translate.SetZ(mat.rows[2][3]);

    rotation = RotationMatrixToEulerAngle(_rot);
    scale.SetX(_scale[0][0]);
    scale.SetY(_scale[1][1]);
    scale.SetZ(_scale[2][2]);
}

vec3f GMathUtils::RotationMatrixToEulerAngle(mat4f matrix)
{
    vec3f eulerAngle;
    float sp = -matrix.rows[3][2];
    if(sp<=-1.0f)
    {
        eulerAngle[0] = -M_PI / 2.0f;
    }
    else if(sp>=1.0f)
    {
        eulerAngle[0] = M_PI/2.0f;
    }
    else
    {
        eulerAngle[0] = asin(sp);
    }

    if(fabs(sp) > 0.9999f)
    {
        eulerAngle[2] = 0.0f;
        eulerAngle[1] = atan2(-matrix.rows[1][3],matrix.rows[1][1]);
    }
    else
    {
        eulerAngle[2] = atan2(-matrix.rows[1][2],matrix.rows[2][2]);
        eulerAngle[1] = atan2(-matrix.rows[3][1],matrix.rows[3][3]);
    }
    return eulerAngle;
}

mat4f GMathUtils::EulerAngleToRotationMatrix(vec3f eulerAngle)
{
    mat4f rotMat;
    rotMat.identity();
    float ch = cos(eulerAngle[1]);
    float cp = cos(eulerAngle[0]);
    float cb = cos(eulerAngle[2]);
    float sh = sin(eulerAngle[1]);
    float sp = sin(eulerAngle[0]);
    float sb = sin(eulerAngle[2]);
    rotMat.rows[0][0] = ch*cb+sh*sp*sb;
    rotMat.rows[0][1] = -ch*sb+sh*sp*cb;
    rotMat.rows[0][2] = sh*cp;
    rotMat.rows[1][0] = sb*cp;
    rotMat.rows[1][1] = cb*cp;
    rotMat.rows[1][2] = -sp;
    rotMat.rows[2][0] = -sh*cb+ch*sp*sb;
    rotMat.rows[2][1] = sb*sh+ch*sp*cb;
    rotMat.rows[2][2] = ch*cp;
    return rotMat;
}

mat4f GMathUtils::TRS(vec3f &translate, vec3f &rotation, vec3f &scale)
{
    mat4f ret;
    ret.identity();
    ret[0][0] *= scale[0];
    ret[1][1] *= scale[1];
    ret[2][2] *= scale[2];
    ret = EulerAngleToRotationMatrix(rotation) * ret;
    ret[0][3] = translate[0];
    ret[1][3] = translate[1];
    ret[2][3] = translate[2];
    return ret;
}

mat3f GMathUtils::RotationMatrix(GMath::vec3f fromV, GMath::vec3f toV)
{
    fromV.normalize();
    toV.normalize();
    float cosTheta = dot(fromV, toV);
    mat3f ret;
    ret.identity();
    if(cosTheta < -1 + 0.001f)
    {
        ret[0][0] = -1;
        ret[1][1] = -1;
        ret[2][2] = -1;
        return ret;
    }

    vec3f rotationAxis = cross(fromV, toV);
    float s = sqrt((1+cosTheta)*2);
    float invertS = 1/s;
    vec4 quaternion(rotationAxis.x()*invertS, rotationAxis.y()*invertS, rotationAxis.z()*invertS, s * 0.5);

    float xx = quaternion.x() * quaternion.x();
    float xy = quaternion.x() * quaternion.y();
    float xz = quaternion.x() * quaternion.z();
    float xw = quaternion.x() * quaternion.w();
    float yy = quaternion.y() * quaternion.y();
    float yz = quaternion.y() * quaternion.z();
    float yw = quaternion.y() * quaternion.w();
    float zz = quaternion.z() * quaternion.z();
    float zw = quaternion.z() * quaternion.w();
    float ww = quaternion.w() * quaternion.w();

    ret[0][0] = 1-2*yy-2*zz;
    ret[0][1] = 2*xy+2*zw;
    ret[0][2] = 2*xz-2*yw;
    ret[1][0] = 2*xy-2*zw;
    ret[1][1] = 1-2*xx-2*zz;
    ret[1][2] = 2*yz+2*xw;
    ret[2][0] = 2*xz+2*yw;
    ret[2][1] = 2*yz-2*xw;
    ret[2][2] = 1-2*xx-2*yy;
    return ret.transpose();
}

float GMathUtils::Rad2Deg(float rad)
{
    return rad * 180.0f / M_PI;
}

float GMathUtils::Deg2Rad(float degree)
{
    return degree * M_PI / 180.0f;
}

vec3f GMathUtils::Rad2Deg(vec3f rad)
{
    vec3f angle;
    angle[0] = Rad2Deg(rad[0]);
    angle[2] = Rad2Deg(rad[1]);
    angle[3] = Rad2Deg(rad[2]);
    return angle;
}

vec3f GMathUtils::Deg2Rad(vec3f degree)
{
    vec3f angle;
    angle[0] = Deg2Rad(degree[0]);
    angle[2] = Deg2Rad(degree[1]);
    angle[3] = Deg2Rad(degree[2]);
    return angle;
}
