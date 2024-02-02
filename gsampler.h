#ifndef GSAMPLER_H
#define GSAMPLER_H
#include <cstdlib>
#include "gmath.h"

using GMath::vec3;

class GSampler
{
public:
    GSampler();

    static inline double Random()
    {
        return std::rand() /(RAND_MAX + 1.0);
    }
    static inline double Random(double min, double max)
    {
        return min + (max-min)*Random();
    }
    static inline int RandomInt(int min, int max)
    {
        return static_cast<int>(Random(min, max+1));
    }

    static inline vec3 RandomVec3()
    {
        return vec3(Random(),Random(),Random());
    }

    static inline vec3 RandomVec3(double min, double max)
    {
        return vec3(Random(min,max),Random(min,max),Random(min,max));
    }

    static inline vec3 RandomVec3InUnitDisk(int zeroChannel=2)
    {
        while(true)
        {
            auto p = vec3(zeroChannel==0 ? 0 : Random(-1,1),
                          zeroChannel==1 ? 0 : Random(-1,1),
                          zeroChannel==2 ? 0 : Random(-1,1));

            if(p.length2()<1) return p;
        }
    }

    static inline vec3 RandomVec3InUnitSphere()
    {
        while(true)
        {
            auto p = RandomVec3(-1,1);
            if(p.length2()<1)
                return p;
        }
    }

    // return barycentric coordinates
    static inline vec3 UniformSampleTriangle()
    {
        double ksi0 = Random();
        double sqrtKsi0 = std::sqrt(ksi0);
        double ksi1 = Random();
        vec3 bCoord = (1.0-sqrtKsi0, ksi1*sqrtKsi0, 0);
        bCoord.SetZ(1-bCoord.x()-bCoord.y());
        return bCoord;
    }

    static inline vec3 UniformSampleSphere()
    {
        double ksi0 = Random();
        double ksi1 = Random();
        double z = 1 - 2 * ksi0;
        double r = std::sqrt(std::max(0.0, 1.0 - z * z));
        double phi = 2 * M_PI * ksi1;
        return vec3(r*std::cos(phi), r*std::sin(phi), z);
    }

    static inline vec3 UniformSampleHemisphere()
    {
        double ksi0 = Random();
        double ksi1 = Random();
        double z = ksi0;
        double r = std::sqrt(std::max(0.0, 1.0 - z * z));
        double phi = 2 * M_PI * ksi1;
        return vec3(r*std::cos(phi), r*std::sin(phi), z);
    }

    static inline vec3 CosineSampleHemisphere()
    {
        double ksi0 = Random();
        double ksi1 = Random();
        double z = std::sqrt(1-ksi0);
        double r = std::sqrt(ksi0);
        double phi = 2 * M_PI * ksi1;
        return vec3(r*std::cos(phi), r*std::sin(phi), z);
    }
};

#endif // GSAMPLER_H
