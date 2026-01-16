#ifndef TRAITS_HPP
#define TRAITS_HPP

#include <type_traits>

#include "tags.hpp"
#include "enums.hpp"

template<typename LayerTag>
struct Traits;

template<typename S, typename D>
struct Traits<Substrate<Type, DopantMaterial, N>>
{
    using DopantType = typename CarrierMap<DopantMaterial>::type;
    using CarrierType = 
};

#endif // TRAITS_HPP
