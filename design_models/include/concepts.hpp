#ifndef CONCEPTS_HPP
#define CONCEPTS_HPP

#include <concepts>
#include <type_traits>

struct NType { }; 
struct PType { };

template<typename T>
concept SemiconductorType = std::is_same_v<NType, T> || std::is_same_v<PType, T>;

#endif // CONCEPTS_HPP