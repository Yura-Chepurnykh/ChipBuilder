#ifndef CARRIERS_HPP
#define CARRIERS_HPP

#include "concepts.hpp"
#include "measurement_unit_overloads.hpp"
#include "geometry.hpp"

struct Electron { }
struct Hole { }

template<typename CarrierType, NumericType NT, Dimension D>
struct Carrier
{
    Pos<NT, D> pos;
};

template<typename CarrierType, NumericType NT>
struct Carrier<CarrierType, NT, 3>
{
    Pos<NT, 3> pos;
};

template<typename CarrierType>
struct CarrierTraits;

template<>
struct CarrierTraits<Electron>
{
    static constexpr float mass = 9.109'383'7015e-31_kg;
    static constexpr float charge = -1.602'176'634e-19_coulomb;
};

template<> 
struct CarrierTraits<Hole>
{
    static constexpr float charge = +1.602'176'634e-19_coulomb;
};

#endif // CARRIERS_HPP
