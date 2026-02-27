#include "sphere.hpp"

Sphere::Sphere(int latitude, int longitude) : 
    m_latitudeSegments(latitude), 
    m_longitudeSegments(longitude)
{
    auto calculateVertices = [&](float phi, float theta)
    {
        auto x = cx + radius * sin(phi) * cos(theta);
        auto y = cy + radius * sin(phi) * sin(theta);
        auto z = cz + radius * cos(phi);

        return std::tuple{ x, y, z };
    };

    for (int i = 0; i < m_latitudeSegments; ++i)
    {
        auto phi1 = (std::numbers::pi * i) / m_latitudeSegments;
        auto phi2 = (std::numbers::pi * (i + 1)) / m_latitudeSegments; 

        for (int j = 0; j < m_longitudeSegments; ++j)
        {
            auto theta1 = (2 * std::numbers::pi * j) / m_longitudeSegments;
            auto theta2 = (2 * std::numbers::pi * (j + 1)) / m_longitudeSegments;
            
            auto [x1, y1, z1] = calculateVertices(phi1, theta1);
            auto [x2, y2, z2] = calculateVertices(phi2, theta1);
            auto [x3, y3, z3] = calculateVertices(phi1, theta2);
            auto [x4, y4, z4] = calculateVertices(phi2, theta2);

            // first triangle
            m_vertices.insert(m_vertices.end(),{ x1, y1, z1, x2, y2, z2, x4, y4, z4 });

            // second triangle
            m_vertices.insert(m_vertices.end(),{ x1, y1, z1, x4, y4, z4, x3, y3, z3 });
        }
    }
}