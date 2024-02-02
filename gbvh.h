#ifndef GBVH_H
#define GBVH_H

#include "gray.h"
#include "gmath.h"
#include "gaabb.h"

using GMath::aabb;
using namespace std;

class GBVHNode : public GHittable
{
public:
    GBVHNode(const vector<shared_ptr<GHittable>>& hittables, size_t start, size_t end);

    bool intersect(const GRay& ray, GMath::interval ray_t, GSurfaceInteraction& isect) override;

    aabb bBox() override
    {
        return bbox;
    }
private:
    shared_ptr<GHittable> left;
    shared_ptr<GHittable> right;
    aabb bbox;

    static bool boxCompare(const shared_ptr<GHittable> a, const shared_ptr<GHittable> b, int axis)
    {
        return a->bBox().axis(axis).min < b->bBox().axis(axis).min;
    }
    static bool boxCompareX(const shared_ptr<GHittable> a, const shared_ptr<GHittable> b)
    {
        return boxCompare(a, b, 0);
    }
    static bool boxCompareY(const shared_ptr<GHittable> a, const shared_ptr<GHittable> b)
    {
        return boxCompare(a, b, 1);
    }
    static bool boxCompareZ(const shared_ptr<GHittable> a, const shared_ptr<GHittable> b)
    {
        return boxCompare(a, b, 2);
    }
};

#endif // GBVH_H
