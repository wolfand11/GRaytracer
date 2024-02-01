#include "gbvh.h"
#include "gmath.h"
#include "gsampler.h"

GBVHNode::GBVHNode(const vector<shared_ptr<GHittable>> &hittables, size_t start, size_t end)
{
    owner = nullptr;
    auto objects = hittables;
    int axis = GSampler::RandomInt(0, 2);
    auto comparator = boxCompareZ;
    if(axis == 0)
    {
        comparator = boxCompareX;
    }
    else if(axis == 1)
    {
        comparator = boxCompareY;
    }

    size_t objSpan = end - start;
    if(objSpan == 1)
    {
        left = right = objects[start];
    }
    else if(objSpan == 2)
    {
        if(comparator(objects[start], objects[start+1]))
        {
            left = objects[start];
            right = objects[start+1];
        }
        else
        {
            left = objects[start+1];
            right = objects[start];
        }
    }
    else
    {
        std::sort(objects.begin()+start, objects.begin()+end, comparator);
        auto mid = start + objSpan/2;
        left = make_shared<GBVHNode>(objects, start, mid);
        right = make_shared<GBVHNode>(objects, mid, end);
    }
    bbox = aabb(left->bBox(), right->bBox());
}

bool GBVHNode::intersect(const GRay &ray, GMath::interval ray_t, GSurfaceInteraction &isect)
{
    if(!bbox.intersect(ray, ray_t))
    {
        return false;
    }

    bool hitLeft = left->intersect(ray, ray_t, isect);
    if(hitLeft)
    {
        ray_t.max = isect.t;
    }
    bool hitRight = right->intersect(ray, ray_t, isect);
    return hitLeft || hitRight;
}
