#ifndef MEASUREMENT_UNITS_OVERLOADS_HPP
#define MEASUREMENT_UNITS_OVERLOADS_HPP

template<typename T>
constexpr T operator"" _coulomb(T coulomb) { return coulomb; }

template<typename T>
constexpr T operator"" _m(T meter) { return meter; }

template<typename T>
constexpr T operator"" _kg(T kilogram) { return kilogram; }

template<typename>
constexpr T operator"" _s(T second) { return second; }

template<typename>
constexpr T operator"" _kelvin(T kelvin) { return kelvin; } 

template<typename>
constexpr T operator"" _A(T ampere) { return ampere; }

#endif // MEASUREMENT_UNITS_OVERLOADS_HPP

