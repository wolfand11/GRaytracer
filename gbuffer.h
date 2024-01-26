#ifndef GBUFFER_H
#define GBUFFER_H

#include "glog.h"
#include <vector>
#include <tuple>
#include "ggraphiclibdefine.h"
#include "gmath.h"
#include "gcolor.h"
class GRenderBuffer;
class GColorBuffer;
class GDepthStencilBuffer;

class GRenderBuffer
{
public:
    virtual ~GRenderBuffer(){}
    bool CheckRangeValid(int x, int y)
    {
        if(data == nullptr)
        {
            GLog::LogError("buffer is nullptr! buffer type = ",typeid(*this).name());
            return false;
        }
        if(x<0 || y<0 || x>=width || y>=height)
        {
            GLog::LogError("width = ", width, " height = ", height, "x = ", x, " y = ", y);
            return false;
        }
        return true;
    }

    int width;
    int height;

protected:
    void* data;
};

class GColorBuffer : public GRenderBuffer
{
public:
    GColorBuffer(int w, int h)
    {
        GLog::LogInfo("Create Color Buffer w = ",w," h = ", h);
        data = nullptr;
        if(w>0 && h>0)
        {
            width = w;
            height = h;
            data = new GColor[w*h];
        }
        else
        {
            GLog::LogError("w = ", w, " h = ", h);
        }
    }
    virtual ~GColorBuffer()
    {
        if(data!=nullptr)
        {
            delete colorData();
        }
        data = nullptr;
    }



    void Clear(GColor color)
    {
        for(int i=0; i<width; i++)
        {
            for(int j=0; j<height; j++)
            SetColor(i, j, color);
        }
    }

    void SetColor(int x, int y, GColor color)
    {
        if(!CheckRangeValid(x,y)) return;
        colorData()[y*width+x] = color;
    }

    GColor GetColor(int x, int y)
    {
        if(!CheckRangeValid(x,y)) return GColor::black;
        return colorData()[y*width+x];
    }

    unsigned char* GetData()
    {
        return (unsigned char*)data;
    }
private:
    GColor* colorData()
    {
        return (GColor*)data;
    }
};

class GDepthStencilBuffer : public GRenderBuffer
{
public:
    GDepthStencilBuffer(int w, int h)
    {
        data = nullptr;
        if(w>0 && h>0)
        {
            width = w;
            height = h;
            data = new int[w*h];
        }
        else
        {
            GLog::LogError("w = ", w, " h = ", h);
        }
    }
    virtual ~GDepthStencilBuffer()
    {
        if(data!=nullptr)
        {
            delete depthStencilData();
        }
        data = nullptr;
    }
    void Clear(unsigned int depth=~0)
    {
        for(int i=0; i<width; i++)
        {
            for(int j=0; j<height; j++)
            SetValue(i, j, depth);
        }
    }
    void ClearF(float depth=1.0f)
    {
        Clear(DepthFToUInt(depth));
    }
    void SetValue(int x, int y, unsigned int depth)
    {
        if(!CheckRangeValid(x,y)) return;

        depthStencilData()[y*width+x] = depth;
    }
    void SetFValue(int x, int y, float depth)
    {
        SetValue(x, y, DepthFToUInt(depth));
    }
    unsigned int GetValue(int x, int y)
    {
        if(!CheckRangeValid(x,y)) return 1;
        return depthStencilData()[y*width+x];
    }
    float GetFValue(int x, int y)
    {
        unsigned int ivalue = GetValue(x, y);
        return DepthUIntToF(ivalue);
    }
private:
    unsigned int DepthFToUInt(float fdepth)
    {
        fdepth = std::max(std::min(fdepth, 1.0f), -1.0f);
        unsigned int idepth = (fdepth + 1.0) * 0.5 * UINT_MAX;
        return idepth;
    }
    float DepthUIntToF(unsigned int idepth)
    {
        float fvalue = (double)idepth / UINT_MAX * 2.0 - 1.0;
        return fvalue;
    }
    unsigned int* depthStencilData()
    {
        return (unsigned int*)data;
    }
};

class GDataBuffer
{
public:
    GDataBuffer(GDataBufferType t)
    {
        bufferType = t;
    }

    ~GDataBuffer()
    {
        InvalidateData();
    }

    void SetData(void* data, int size, int offset=0)
    {
        if(data==nullptr || size<1)
        {
            GLog::LogError("data = ", data, " size = ", size);
            return;
        }
        unsigned char* start = (unsigned char*)data;
        _buffer.insert(_buffer.begin()+offset, start, start+size);
    }

    void InvalidateData()
    {
        _buffer.clear();
    }

    template<typename T>T* GetData(int offset)
    {
        return (T*)(_buffer.data()+offset);
    }

    void* buffer()
    {
        return _buffer.data();
    }

    GDataBufferType bufferType;
    std::vector<unsigned char> _buffer;
};

#endif // GBUFFER_H
