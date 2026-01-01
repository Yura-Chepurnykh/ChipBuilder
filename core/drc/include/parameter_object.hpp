#ifndef PARAMETER_OBJECT_HPP
#define PARAMETER_OBJECT_HPP

#include <memory>
#include "shape.hpp"

struct ParameterObject 
{ 
    virtual ~ParameterObject() = default;
};

struct MinimumWidthParameterObject : public ParameterObject 
{
    int width;
};

struct MinimumSpacingParameterObject : public ParameterObject 
{
    Shape& shape1;
    Shape& shape2;
};

struct MinimumAreaParameterObject : public ParameterObject 
{
    int area;
};

#endif // PARAMETER_OBJECT_HPP
