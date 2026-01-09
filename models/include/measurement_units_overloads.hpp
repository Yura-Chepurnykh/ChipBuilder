#ifndef MEASUREMENT_UNITS_OVERLOADS_HPP
#define MEASUREMENT_UNITS_OVERLOADS_HPP

#include "concepts.hpp"

template<NumericType NT>
constexpr NT operator"" _coulomb(NT coulomb) { return coulomb; }

template<NumericType NT>
constexpr NT operator"" _m(NT meter) { return meter; }

template<NumericType NT>
constexpr NT operator"" _kg(NT kilogram) { return kilogram; }

template<NumericType NT>
constexpr NT operator"" _s(NT second) { return second; }

template<NumericType NT>
constexpr NT operator"" _kelvin(NT kelvin) { return kelvin; } 

template<NumericType NT>
constexpr NT operator"" _A(NT ampere) { return ampere; }

#endif // MEASUREMENT_UNITS_OVERLOADS_HPP

