#include "builder.hpp"

IShapeBuilder& RectBuilder::onPress(const Point& p)
{
    m_rect.setTopLeft(p);
    return *this;
}

IShapeBuilder& RectBuilder::onMove(const Point& p)
{
    m_rect.setBottomRight(p);
    return *this;
}

IShapeBuilder& RectBuilder::onRelease(const Point& p)
{
    onMove(p);
    return *this;
}

IShapeBuilder& RectBuilder::onDouble(const Point&)
{

}

std::unique_ptr<IShape> RectBuilder::build()
{
    return std::make_unique<Rect>(m_rect);
}

IShapeBuilder& PolygonBuilder::onPress(const Point& p)
{
    if (m_polygon.m_points.empty())
    {
        m_polygon.m_points.push_back(p); // First point (fixed)
        m_polygon.m_points.push_back(p); // Second point (moving)
    }
    else
    {
        // Fix the current moving point and prepare a new one
        m_polygon.m_points.back() = p;
        m_polygon.m_points.push_back(p);
    }

    return *this;
}

IShapeBuilder& PolygonBuilder::onRelease(const Point& p)
{
    // For polygon, we don't necessarily add a point on release
    // if we already added it on press.
    // The user said "когда он делает mousePressEvent создается этот полигон, 
    // потом когда делает снова mousePressEvent полигон увеличивается"
    return *this;
}

IShapeBuilder& PolygonBuilder::onMove(const Point& p)
{
    if (!m_polygon.m_points.empty())
    {
        m_polygon.m_points.back() = p;
    }
    return *this;
}

IShapeBuilder& PolygonBuilder::onDouble(const Point& p)
{
    if (!m_polygon.m_points.empty())
    {
        m_polygon.m_points.back() = p;
    }
    return *this;
}

std::unique_ptr<IShape> PolygonBuilder::build()
{
    // Clean up identical consecutive points
    std::vector<Point> cleaned;
    for (const auto& p : m_polygon.m_points)
    {
        if (cleaned.empty() || !(cleaned.back() == p))
        {
            cleaned.push_back(p);
        }
    }
    return std::make_unique<PolygonShape>(cleaned);
}
