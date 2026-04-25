#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <vector>
#include <cmath>

struct IVisitor;

struct Point
{
    Point(unsigned int id = -1, int x = int(), int y = int()) noexcept : id(id), x(x), y(y) { }
    unsigned int id;
    int x, y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct IShape
{
    unsigned int id; 
    virtual ~IShape() = default;
    virtual void move(int dx, int dy) = 0;
    virtual void move(const Point&) = 0;
    virtual void accept(IVisitor&) = 0;
};

struct PolygonShape final : public IShape
{
    PolygonShape(const std::vector<Point>& points = std::vector<Point>()) noexcept : m_points(points) { }

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
        if (m_points.empty()) return;
        int dx = point.x - m_points[0].x;
        int dy = point.y - m_points[0].y;
        move(dx, dy);
    }

    void accept(IVisitor&) override;

    std::vector<Point> m_points;
};

struct Rect final : public IShape
{
    Rect(Point p = Point(), int w = int(), int h = int()) noexcept : point(p), width(w), height(h) { }

    Rect(Point leftTop, Point rightBottom) noexcept 
    {
        if (leftTop.x > rightBottom.x && leftTop.y > rightBottom.y) {
            point = rightBottom;
        }   

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

    // bool intersects(const Rect& other) const {
    //     return !(point.x + width < other.point.x ||
    //              other.point.x + other.width < point.x ||
    //              point.y + height < other.point.y ||
    //              other.point.y + other.height < point.y);
    // }
    
    // bool touches(const Rect& other) const {
    //     // Simple touch/intersect check: if they overlap or boundaries touch
    //     return !(point.x + width < other.point.x ||
    //              other.point.x + other.width < point.x ||
    //              point.y + height < other.point.y ||
    //              other.point.y + other.height < point.y);
    // }
};

#endif // GEOMETRY_HPP
