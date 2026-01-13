// rtree.tpp
template<NumericType NT>
Rect::Rect() noexcept : minX(0), maxX(0), minY(0), maxY(0) { }

template<NumericType NT>
Rect::Rect(NT minX, NT maxY, NT maxX, NT minY) noexcept : 
    minX(minX), maxY(maxY), maxX(maxX), minY(minY) { }

template<NumericType NT>
NT Rect::area() const noexcept 
{
    return (maxX - minX) * (maxY - minY);
}

// template<NumericType NT>
// void Rect::enlarged(const Rect& rect) noexcept 
// {
//     minX = std::min(minX, rect.minX);
//     maxX = std::max(maxX, rect.maxX);
//     minY = std::min(minY, rect.minY);
//     maxY = std::max(maxX, rect.maxX);
// }

// template<typename NT>
// NT Rect::enlargementNeeded(const Rect& rect) const noexcept 
// {
// }
