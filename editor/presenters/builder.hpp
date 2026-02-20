#ifndef BUILDER_HPP
#define BUILDER_HPP

#include <type_traits>
#include <QPointF>
#include <QRectF>
#include <QPolygonF>
#include <QPainterPath>
#include "geometry.hpp"

struct IShapeBuilder
{
    virtual ~IShapeBuilder() = default;
    virtual IShapeBuilder& onPress(const Point& p) = 0;
    virtual IShapeBuilder& onMove(const Point& p) = 0;
    virtual IShapeBuilder& onRelease(const Point& p) = 0;
    virtual IShapeBuilder& onDouble(const Point& p) = 0;
    virtual std::unique_ptr<IShape> build() = 0;
};

class RectBuilder final : public IShapeBuilder
{
public:
    IShapeBuilder& onPress(const Point& p) override;
    IShapeBuilder& onMove(const Point& p) override;
    IShapeBuilder& onRelease(const Point& p) override;
    IShapeBuilder& onDouble(const Point&) override;
    std::unique_ptr<IShape> build() override;

private:
    Rect m_rect;
};

class PolygonBuilder final : public IShapeBuilder
{
public:
    IShapeBuilder& onPress(const Point& p) override;
    IShapeBuilder& onMove(const Point& p) override;
    IShapeBuilder& onRelease(const Point& p) override;
    IShapeBuilder& onDouble(const Point& p) override;
    std::unique_ptr<IShape> build() override;

private:
    Polygon m_polygon;
};

#endif // BUILDER_HPP
