#ifndef PARAMETER_OBJECT_HPP
#define PARAMETER_OBJECT_HPP

#include <type_traits>
#include <string>
#include "geometry.hpp"
#include "concepts.hpp"
#include "layers.hpp"

struct IParameterObject 
{
    virtual ~IParameterObject() = default; 
};

template<NumericType NT>
struct Rect;

// pattern parameter object 
template<NumericType NT>
struct ParameterObject : public IParameterObject
{
    Rect<NT> rect;
};

template<typename S, NumericType NT>
struct CreateMosParams : public ParameterObject<NT>
{
    using SemiConductorType = S;
};

template<typename L, typename S, typename D, NumericType NT>
struct CreateParams : public ParameterObject<NT>
{
    static_assert(std::is_base_of<Layer, L>::value, "L must inherit from Layer");
    using LayerType = L;
    using SemiConductorType = S;
    using DopantType = D;
};

template<typename L, NumericType NT>
struct RemoveParams : public ParameterObject<NT>
{
    static_assert(std::is_base_of<Layer, L>::value, "L must inherit from Layer");
    using LayerType = L;
};

template<typename L, NumericType NT>
using GoToParams = RemoveParams<L, NT>;

template<typename L, NumericType NT>
using ReadParams = RemoveParams<L, NT>;

template<NumericType NT>
struct Rect 
{
    Pos<NT, 2> position;
    NT width, height;
};

// TEMP: may be needed later
// struct IParameterObject { virtual ~IParameterObject() = default; };

// struct IAction : public IParameterObject { };
// struct Create : public IAction { };
// struct Remove : public IAction { };
// struct GoTo : public IAction { };
// struct Read : public IAction { };

// // decorator pattern
// struct Decorator : public IParameterObject { };

// template<typename Dopant>
// struct Dopant : public Decorator 
// {
//     using DopantType = Dopant; 
// };

// template<typename SemiConductor>
// struct SemiConductor : public Decorator 
// {
//     using SemiConductorType = SemiConductor;
// };

// template<typename LayerLevel>
// struct LayerLevel : public Decorator
// {
//     using LayerLevelType = LayerLevel;
// };

// struct ISubject : public IParameterObject
// {

// };

// struct Substrate : public ISubject 
// {

// };

#endif // PARAMETER_OBJECT_HPP

