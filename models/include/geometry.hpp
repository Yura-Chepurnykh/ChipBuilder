#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <array>
#include "concepts.hpp"

template<NumericType NT, size_t Dimension>
struct Pos 
{
    std::array<NT, Dimension> coordinates;
};

template<NumericType NT>
struct Pos<NT, 3>
{
    NT x, y, z;
};

#endif // GEOMETRY_HPP
