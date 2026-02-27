#ifndef SPHERE_HPP
#define SPHERE_HPP

#include <vector>
#include <numbers>
#include <cmath>

class Sphere 
{
public:
    Sphere(int, int);

private:
    int m_latitudeSegments, m_longitudeSegments;
    float cx, cy, cz;
    float radius;
    std::vector<float> m_vertices;
};

#endif // SPHERE_HPP