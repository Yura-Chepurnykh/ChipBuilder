#ifndef TRAITS_HPP
#define TRAITS_HPP

#include <type_traits>
#include "layers.hpp"

template<typename S>
struct is_semi : std::false_type { };

template<>
struct is_semi<NType> : std::true_type { };

template<>
struct is_semi<PType> : std::true_type { };

#endif // TRAITS_HPP
