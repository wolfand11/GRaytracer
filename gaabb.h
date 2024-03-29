#ifndef GAABB_H
#define GAABB_H
#include "gmath.h"
#include "gray.h"

namespace GMath
{
template<typename T>
struct GAABB
{
public:
    GInterval<T> x,y,z;

    GAABB()=default;

    GAABB(const GInterval<T>& ix, const GInterval<T>& iy, const GInterval<T>& iz)
        :x(ix),y(iy),z(iz)
    {}

    GAABB(const GVect<T,3>& a, const GVect<T,3>& b)
    {
        x = GInterval<T>(std::fmin(a[0], b[0]), std::fmax(a[0], b[0]));
        y = GInterval<T>(std::fmin(a[1], b[1]), std::fmax(a[1], b[1]));
        z = GInterval<T>(std::fmin(a[2], b[2]), std::fmax(a[2], b[2]));
    }

    GAABB(const GVect<T,3>& a, const GVect<T,3>& b, const GVect<T,3>& c)
    {
        x = GInterval<T>(std::min(std::min(a[0], b[0]), c[0]), std::max(std::max(a[0], b[0]), c[0]));
        y = GInterval<T>(std::min(std::min(a[1], b[1]), c[1]), std::max(std::max(a[1], b[1]), c[1]));
        z = GInterval<T>(std::min(std::min(a[2], b[2]), c[2]), std::max(std::max(a[2], b[2]), c[2]));
    }

    GAABB(const GAABB<T>& box0, const GAABB<T>& box1)
    {
        x = GInterval<T>(box0.x, box1.x);
        y = GInterval<T>(box0.y, box1.y);
        z = GInterval<T>(box0.z, box1.z);
    }

    GAABB<T> pad(T delta=0.0001)
    {
        GInterval<T> new_x = (x.size()>=delta) ? x : x.expand(delta);
        GInterval<T> new_y = (y.size()>=delta) ? y : y.expand(delta);
        GInterval<T> new_z = (z.size()>=delta) ? z : z.expand(delta);
        return GAABB<T>(new_x, new_y, new_z);
    }

    const GInterval<T>& axis(int n) const
    {
        if(n==1)return y;
        if(n==2)return z;
        return x;
    }

    bool intersect(const GRay &r, GMath::GInterval<T> ray_t) const
    {
        for(int i=0; i<3; i++)
        {
            auto invD = 1/r.dir[i];
            auto origin = r.origin[i];

            auto t0 = (axis(i).min - origin) * invD;
            auto t1 = (axis(i).max - origin) * invD;

            if(invD < 0) std::swap(t0, t1);

            if(t0 > ray_t.min) ray_t.min = t0;
            if(t1 < ray_t.max) ray_t.max = t1;

            if(ray_t.max <= ray_t.min) return false;
        }
        return true;
    }
};

template <typename T>
GAABB<T> operator+(const GAABB<T>& bbox, const GVect<T,3> offset)
{
    return GAABB<T>(bbox.x + offset[0], bbox.y+offset[1], bbox.z+offset[2]);
}

template <typename T>
GAABB<T> operator+(const GVect<T,3> offset,const GAABB<T>& bbox)
{
    return bbox + offset;
}

template <typename T>
GAABB<T> transformBBox(const GAABB<T> &box, const mat4f &mat)
{
    vec3 xa = mat.col(0).xyz() * box.x.min;
    vec3 xb = mat.col(0).xyz() * box.x.max;
    vec3 ya = mat.col(1).xyz() * box.y.min;
    vec3 yb = mat.col(1).xyz() * box.y.max;
    vec3 za = mat.col(2).xyz() * box.z.min;
    vec3 zb = mat.col(2).xyz() * box.z.max;

    float w = mat[3][3];
    vec3 pmin_ = min(xa,xb) + min(ya,yb) + min(za,zb) + (vec3)(mat.col(3).xyz());
    vec3 pmax_ = max(xa,xb) + max(ya,yb) + max(za,zb) + (vec3)(mat.col(3).xyz());
    GAABB<T> b;
    b.x = GInterval<T>(pmin_[0]/w, pmax_[0]/w);
    b.y = GInterval<T>(pmin_[1]/w, pmax_[1]/w);
    b.z = GInterval<T>(pmin_[2]/w, pmax_[2]/w);
    return b;
}

typedef GAABB<float> aabbf;
typedef GAABB<double> aabb;
}
#endif // GAABB_H
