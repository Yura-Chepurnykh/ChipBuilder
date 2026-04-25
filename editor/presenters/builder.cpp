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
        m_polygon.m_points.push_back(p);
        m_polygon.m_points.push_back(p);
    }
    else
    {
        m_polygon.m_points.back() = p;
        m_polygon.m_points.push_back(p);
    }

    return *this;
}

IShapeBuilder& PolygonBuilder::onRelease(const Point& p)
{
    onPress(p);
    return *this;
}

IShapeBuilder& PolygonBuilder::onMove(const Point& p)
{
    fprintf(stderr, "PolygonBuilder::onMove");
    m_polygon.m_points.back() = p;
    return *this;
}

IShapeBuilder& PolygonBuilder::onDouble(const Point& p)
{
    onPress(p);
    return *this;
}

std::unique_ptr<IShape> PolygonBuilder::build()
{
    return std::make_unique<PolygonShape>(m_polygon);
}
