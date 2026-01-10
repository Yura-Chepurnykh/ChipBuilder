#ifndef CIRCUITS_HPP
#define CIRCUITS_HPP

#include "layers.hpp"
#include "dopant_material.hpp"

struct NMOS;
struct PMOS;
struct CustomMOS;

template<typename Type, typename DopantMaterial, size_t N>
struct MOSLayers
{
    Substrate<Type, DopantMaterial, N> substrate;
    Source<Type, DopantMaterial, N> source;
    Drain<Type, DopantMaterial, N> drain;
    Gate<N> gate;
};

template<typename DopantMaterial, size_t N>
struct NMOS 
{
    template<>
    MOSLayers<NType, >
};

#endif // CIRCUITS_HPP
