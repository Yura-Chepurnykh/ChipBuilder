#ifndef CONCEPTS_HPP
#define CONCEPTS_HPP

#include <contexts>

template<typename T>
concept NumericType = std::integral<T> || std::floating_point<T>; 

#endif // CONCEPTS_HPP
