#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <vector>
#include <cmath>

// We are not use templates for Pos and Rect, since
// in VLSI does not use floating point numbers, instead
// for present number used the power, for example
// 4.5 * 10e-9 => 45 * 10e-10

// From gemotries point of view line does not have thickness,
// line is an abstract object

struct IVisitor;

struct Point
{
    Point(unsigned int id = -1, int x = int(), int y = int()) noexcept : id(id), x(x), y(y) { }
    unsigned int id;
    int x, y;
};

// dummy class for polymorphism
struct IShape
{
    unsigned int id; 
    virtual ~IShape() = default;
    virtual void move(int dx, int dy) = 0;
    virtual void move(const Point&) = 0;
    virtual void accept(IVisitor&) = 0;
};

struct Polygon final : public IShape
{
    Polygon(const std::vector<Point>& points = std::vector<Point>()) noexcept : m_points(points) { }

    void move(int dx, int dy) override
    {
        for (auto& p : m_points)
        {
            p.x += dx;
            p.y += dy;
        }
    }

    void move(const Point& point) override
    {
        for (auto& p : m_points)
        {
            p.x = point.x;
            p.y = point.y;
        }
    }

    void accept(IVisitor&) override;

    std::vector<Point> m_points;
};

struct Rect final : public IShape
{
    Rect(Point p = Point(), int w = int(), int h = int()) noexcept : point(p), width(w), height(h) { }

    Rect(Point leftTop, Point rightBottom) noexcept : point(leftTop)
    {
        width = std::abs(rightBottom.x - leftTop.x);
        height = std::abs(rightBottom.y - leftTop.y);
    }

    void setTopLeft(const Point& p)
    {
        point = p;
    }

    void setBottomRight(const Point& p)
    {
        width = std::abs(p.x - point.x);
        height = std::abs(p.y - point.y);
    }

    void move(int dx, int dy) override
    {
        point.x += dx;
        point.y += dy;
    }

    void move(const Point& p) override
    {
        point.x = p.x;
        point.y = p.y;
    }

    void accept(IVisitor&) override;

    Point point;
    int width, height;
};

#endif // GEOMETRY_HPP
