#ifndef RTREE_HPP
#define RTREE_HPP

#include "concepts.hpp"

template<NumericType NT>
struct Rect 
{
    NT minX, maxX, minY, maxY;

    Rect() noexcept;
    Rect(NT minX, NT maxY, NT maxX, NT minY) noexcept;

    NT area() const noexcept;
    void enlarged() noexcept;
    NT enlargementRect() noexcept;
    NT enlargementNeeded(const Rect&) const noexcept;
    bool intersects(const Rect&) const noexcept;
    bool contains(const Rect&) const noexcept;
};

#include "rtree.tpp"

#endif // RTREE_HPP
