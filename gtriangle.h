#ifndef GTRIANGLE_H
#define GTRIANGLE_H

#include "gmodel.h"
#include "gobjmodel.h"

class GTriangle : public GShape
{
public:
    GTriangle(std::shared_ptr<GOBJModel> objModel, int faceIndex)
        :objModel(objModel), faceIndex(faceIndex)
    {}

    std::shared_ptr<GOBJModel> objModel;
    int faceIndex;
};

class GTriangleList : public GShapeList
{
};

#endif // GTRIANGLE_H
