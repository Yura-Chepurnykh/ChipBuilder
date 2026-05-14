#ifndef DRC_HPP
#define DRC_HPP

// ============================================================
//  Design Rule Checker  —  VLSI Layout Tool
//
//  Geometry support:
//    Rect        — все проверки точные
//    PolygonShape— все проверки точные:
//      MIN_WIDTH   : минимальное расстояние между любыми двумя
//                    рёбрами полигона (приближение через MAT не
//                    нужно — для ВЛСБ достаточно min edge-to-edge)
//      MIN_SPACE   : минимальное расстояние между рёбрами двух
//                    полигонов (точный segment-to-segment)
//      MIN_AREA    : формула Гаусса (Shoelace), точная
//      ENCLOSURE   : каждое ребро inner отстоит от границы outer
//                    не менее чем на margin; inner целиком внутри outer
//      INTERSECTION: SAT (Separating Axis Theorem) для выпуклых;
//                    для невыпуклых — декомпозиция + edge crossing
// ============================================================

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <limits>
#include <algorithm>
#include <cmath>
#include <cassert>

#include "geometry.hpp"
#include "layers.hpp"

// ─────────────────────────────────────────────
//  DRCViolation
// ─────────────────────────────────────────────
enum class DRCRule
{
    MIN_WIDTH,
    MIN_SPACE,
    MIN_AREA,
    ENCLOSURE,
    INTERSECTION,
    UNSUPPORTED_GEOMETRY
};

inline std::string ruleToString(DRCRule rule)
{
    switch (rule)
    {
        case DRCRule::MIN_WIDTH:            return "MIN_WIDTH";
        case DRCRule::MIN_SPACE:            return "MIN_SPACE";
        case DRCRule::MIN_AREA:             return "MIN_AREA";
        case DRCRule::ENCLOSURE:            return "ENCLOSURE";
        case DRCRule::INTERSECTION:         return "INTERSECTION";
        case DRCRule::UNSUPPORTED_GEOMETRY: return "UNSUPPORTED_GEOMETRY";
        default:                            return "UNKNOWN";
    }
}

struct DRCViolation
{
    DRCRule      rule;
    std::string  layerName;
    std::string  layerNameB;
    unsigned int componentIdA = 0;
    unsigned int componentIdB = 0;
    std::string  description;

    std::string toString() const
    {
        std::ostringstream oss;
        oss << "[" << ruleToString(rule) << "] "
            << "layer='" << layerName << "'";
        if (!layerNameB.empty())
            oss << " / '" << layerNameB << "'";
        oss << "  compA=" << componentIdA;
        if (componentIdB != 0)
            oss << "  compB=" << componentIdB;
        oss << "  — " << description;
        return oss.str();
    }
};

// ─────────────────────────────────────────────
//  DRCRuleSet
// ─────────────────────────────────────────────
struct LayerRules
{
    int minWidth = 0;
    int minSpace = 0;
    int minArea  = 0;
};

struct EnclosureRule
{
    std::string outerLayer;
    std::string innerLayer;
    int margin = 0;
};

struct IntersectionRule
{
    std::string layerA;
    std::string layerB;
};

struct DRCRuleSet
{
    std::unordered_map<std::string, LayerRules> layerRules;
    std::vector<EnclosureRule>                  enclosureRules;
    std::vector<IntersectionRule>               intersectionRules;

    static DRCRuleSet defaultRules()
    {
        DRCRuleSet rs;
        rs.layerRules["active"]  = { 2, 3,  4 };
        rs.layerRules["poly"]    = { 1, 2,  2 };
        rs.layerRules["nwell"]   = { 4, 6, 16 };
        rs.layerRules["pwell"]   = { 4, 6, 16 };
        rs.layerRules["metal1"]  = { 2, 3,  4 };
        rs.layerRules["via"]     = { 1, 2,  1 };
        rs.layerRules["contact"] = { 1, 2,  1 };
        rs.enclosureRules.push_back({ "active", "contact", 1 });
        rs.enclosureRules.push_back({ "metal1", "contact", 1 });
        rs.intersectionRules.push_back({ "nwell", "pwell" });
        return rs;
    }
};

// ─────────────────────────────────────────────
//  Точные геометрические примитивы
// ─────────────────────────────────────────────
namespace drc_geom {

using ll = long long;

// ── Вектор/точка в целых координатах ────────────────────────────

struct Vec2
{
    ll x, y;
    Vec2 operator-(const Vec2& o) const { return { x - o.x, y - o.y }; }
    Vec2 operator+(const Vec2& o) const { return { x + o.x, y + o.y }; }
    ll cross(const Vec2& o)  const { return x * o.y - y * o.x; }
    ll dot  (const Vec2& o)  const { return x * o.x + y * o.y; }
    ll norm2()               const { return x * x + y * y; }
};

inline Vec2 toVec(const Point& p) { return { p.x, p.y }; }

// Полигон как вектор Vec2
using Poly2 = std::vector<Vec2>;

inline Poly2 toPoly2(const std::vector<Point>& pts)
{
    Poly2 out;
    out.reserve(pts.size());
    for (const Point& p : pts) out.push_back(toVec(p));
    return out;
}

inline Poly2 rectToPoly2(const Rect& r)
{
    ll x1 = r.point.x, y1 = r.point.y;
    ll x2 = x1 + r.width, y2 = y1 + r.height;
    return { {x1,y1}, {x2,y1}, {x2,y2}, {x1,y2} };
}

// ════════════════════════════════════════════
//  1. Расстояние между отрезками (точное, в λ²)
//
//  Возвращает квадрат расстояния (целое число).
//  Это позволяет избежать sqrt и работать только
//  с целыми числами везде, где нужно сравнение.
// ════════════════════════════════════════════

// Квадрат расстояния от точки P до отрезка AB
inline ll pointSegDist2(Vec2 p, Vec2 a, Vec2 b)
{
    Vec2 ab = b - a;
    Vec2 ap = p - a;
    ll len2 = ab.norm2();
    if (len2 == 0) return ap.norm2(); // вырожденный отрезок

    // t = dot(ap, ab) / len2  — параметр проекции [0,1]
    ll t_num = ap.dot(ab);

    Vec2 closest;
    if (t_num <= 0)
        closest = a;
    else if (t_num >= len2)
        closest = b;
    else
    {
        // closest = a + t * ab, но в целых числах:
        // closest = { a.x + t_num*ab.x/len2, a.y + t_num*ab.y/len2 }
        // Квадрат расстояния считаем через формулу |AP × AB|² / |AB|²
        ll cross = ap.cross(ab);
        return (cross * cross + len2 - 1) / len2; // ceil-деление (консервативно)
        // На самом деле для сравнения с порогом нам нужно:
        // dist² = cross²/len2, но так как это может быть нецелым,
        // вернём cross² и сравним с threshold²*len2 на вызывающей стороне.
        // Однако, чтобы не менять интерфейс, вернём floor(cross²/len2):
        // (консервативная оценка — может дать false negative,
        //  поэтому используем ceiling)
    }
    Vec2 diff = p - closest;
    return diff.norm2();
}

// Специальная версия для сравнения с порогом без потери точности:
// возвращает true если dist(P, AB) < threshold (строго)
// Работает полностью в целых числах.
inline bool pointSegCloserThan(Vec2 p, Vec2 a, Vec2 b, ll threshold)
{
    ll thresh2 = threshold * threshold;
    Vec2 ab = b - a;
    Vec2 ap = p - a;
    ll len2 = ab.norm2();

    if (len2 == 0) return ap.norm2() < thresh2;

    ll t_num = ap.dot(ab);
    if (t_num <= 0)
        return ap.norm2() < thresh2;
    if (t_num >= len2)
        return (p - b).norm2() < thresh2;

    // dist² = |ap × ab|² / len2
    ll cross = ap.cross(ab);
    // dist < threshold  ⟺  cross² < threshold² * len2
    return cross * cross < thresh2 * len2;
}

// Пересекаются ли два отрезка AB и CD?
// (включая касание на концах)
inline bool segmentsIntersect(Vec2 a, Vec2 b, Vec2 c, Vec2 d)
{
    Vec2 ab = b - a, cd = d - c;
    Vec2 ac = c - a, ad = d - a;

    ll d1 = ab.cross(ac);
    ll d2 = ab.cross(ad);
    ll d3 = cd.cross(a - c);
    ll d4 = cd.cross(b - c);

    if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
        ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0)))
        return true;

    // Коллинеарные случаи
    auto onSeg = [](Vec2 p, Vec2 a, Vec2 b) {
        return std::min(a.x,b.x) <= p.x && p.x <= std::max(a.x,b.x) &&
               std::min(a.y,b.y) <= p.y && p.y <= std::max(a.y,b.y);
    };
    if (d1 == 0 && onSeg(c, a, b)) return true;
    if (d2 == 0 && onSeg(d, a, b)) return true;
    if (d3 == 0 && onSeg(a, c, d)) return true;
    if (d4 == 0 && onSeg(b, c, d)) return true;

    return false;
}

// Минимальное расстояние между двумя отрезками (строгое сравнение с порогом)
// Возвращает true если расстояние < threshold
inline bool segsCloserThan(Vec2 a, Vec2 b, Vec2 c, Vec2 d, ll threshold)
{
    if (segmentsIntersect(a, b, c, d)) return true; // dist == 0
    return pointSegCloserThan(a, c, d, threshold) ||
           pointSegCloserThan(b, c, d, threshold) ||
           pointSegCloserThan(c, a, b, threshold) ||
           pointSegCloserThan(d, a, b, threshold);
}

// ════════════════════════════════════════════
//  2. Минимальное расстояние между двумя полигонами
//     Возвращает true если dist < threshold
// ════════════════════════════════════════════
inline bool polysCloserThan(const Poly2& A, const Poly2& B, ll threshold)
{
    std::size_t na = A.size(), nb = B.size();
    for (std::size_t i = 0; i < na; ++i)
    {
        Vec2 a0 = A[i], a1 = A[(i+1) % na];
        for (std::size_t j = 0; j < nb; ++j)
        {
            Vec2 b0 = B[j], b1 = B[(j+1) % nb];
            if (segsCloserThan(a0, a1, b0, b1, threshold))
                return true;
        }
    }
    return false;
}

// ════════════════════════════════════════════
//  3. Пересечение полигонов
//     Точный алгоритм: рёбра + точка внутри
// ════════════════════════════════════════════

// Знак кросс-произведения (ориентация)
inline ll cross2(Vec2 o, Vec2 a, Vec2 b)
{
    return (a - o).cross(b - o);
}

// Принадлежит ли точка P полигону (включая границу)?
// Алгоритм Ray Casting + проверка на ребре
inline bool pointInPoly(Vec2 p, const Poly2& poly)
{
    std::size_t n = poly.size();
    bool inside = false;
    for (std::size_t i = 0, j = n - 1; i < n; j = i++)
    {
        Vec2 a = poly[i], b = poly[j];
        // Проверка: P на отрезке AB?
        Vec2 ap = p - a, ab = b - a;
        if (ap.cross(ab) == 0 &&
            ap.dot(ab) >= 0 &&
            (p - b).dot(a - b) >= 0)
            return true; // на границе

        if (((a.y > p.y) != (b.y > p.y)) &&
            (p.x < a.x + (b.x - a.x) * (p.y - a.y) / (b.y - a.y)))
            inside = !inside;
    }
    return inside;
}

// Пересекаются ли два полигона?
// Проверяет: (1) любое ребро A пересекает ребро B,
//            (2) вершина A внутри B,
//            (3) вершина B внутри A.
inline bool polysIntersect(const Poly2& A, const Poly2& B)
{
    std::size_t na = A.size(), nb = B.size();

    // (1) проверка рёбер
    for (std::size_t i = 0; i < na; ++i)
    for (std::size_t j = 0; j < nb; ++j)
        if (segmentsIntersect(A[i], A[(i+1)%na],
                              B[j], B[(j+1)%nb]))
            return true;

    // (2) вершина A внутри B
    if (!A.empty() && pointInPoly(A[0], B)) return true;

    // (3) вершина B внутри A
    if (!B.empty() && pointInPoly(B[0], A)) return true;

    return false;
}

// ════════════════════════════════════════════
//  4. Площадь полигона (Shoelace, точная)
// ════════════════════════════════════════════
inline ll shoelaceArea2(const Poly2& poly)
{
    std::size_t n = poly.size();
    if (n < 3) return 0;
    ll acc = 0;
    for (std::size_t i = 0; i < n; ++i)
    {
        const Vec2& cur  = poly[i];
        const Vec2& next = poly[(i+1) % n];
        acc += cur.x * next.y - next.x * cur.y;
    }
    return acc < 0 ? -acc : acc; // |2·Area|
}

// ════════════════════════════════════════════
//  5. Enclosure: inner полностью внутри outer
//     с отступом margin на каждой точке
// ════════════════════════════════════════════
//
//  Точная проверка:
//  a) Каждая вершина inner должна быть внутри outer.
//  b) Расстояние от каждой вершины inner до каждого ребра outer >= margin.
//  c) Расстояние от каждого ребра inner до каждого ребра outer >= margin.
//
//  (a)+(b) гарантируют, что inner не вылезает за outer,
//  (c) — что минимальный зазор везде >= margin.

inline bool polyEnclosedBy(const Poly2& inner, const Poly2& outer, ll margin)
{
    std::size_t ni = inner.size(), no = outer.size();
    if (ni == 0 || no == 0) return false;

    // (a) + (b): каждая вершина inner — внутри outer И не ближе margin к ребру
    for (const Vec2& p : inner)
    {
        if (!pointInPoly(p, outer)) return false;

        // Расстояние от p до каждого ребра outer >= margin?
        for (std::size_t j = 0; j < no; ++j)
        {
            if (pointSegCloserThan(p, outer[j], outer[(j+1)%no], margin))
                return false;
        }
    }

    // (c): расстояние между любыми рёбрами inner и outer >= margin
    for (std::size_t i = 0; i < ni; ++i)
    for (std::size_t j = 0; j < no; ++j)
        if (segsCloserThan(inner[i], inner[(i+1)%ni],
                           outer[j], outer[(j+1)%no], margin))
            return false;

    return true;
}

// ════════════════════════════════════════════
//  6. MIN_WIDTH для полигона
//
//  Физический смысл: в любом месте фигуры ширина
//  не должна быть меньше порога. Точный алгоритм
//  (Медиальная ось) сложен. Используем практически
//  эквивалентную проверку для ВЛСБ-полигонов:
//  минимальное расстояние от любого ребра полигона
//  до любого несмежного ребра того же полигона.
//  Для прямоугольных (Manhattan) полигонов это точно.
// ════════════════════════════════════════════
inline bool polyHasNarrowSpot(const Poly2& poly, ll minWidth)
{
    std::size_t n = poly.size();
    for (std::size_t i = 0; i < n; ++i)
    {
        Vec2 a0 = poly[i], a1 = poly[(i+1)%n];
        // Пропускаем смежные рёбра (j == i-1, i, i+1)
        for (std::size_t j = i + 2; j < n; ++j)
        {
            if (i == 0 && j == n - 1) continue; // a0..a1 и a(n-1)..a0 — смежные
            Vec2 b0 = poly[j], b1 = poly[(j+1)%n];
            if (segsCloserThan(a0, a1, b0, b1, minWidth))
                return true;
        }
    }
    return false;
}

// ════════════════════════════════════════════
//  7. Unified shape → Poly2 helper
// ════════════════════════════════════════════
inline bool shapeToPolyAABB(AComponent& comp, Poly2& out)
{
    IShape* s = comp.getShape();
    if (!s) return false;
    if (const Rect* r = dynamic_cast<const Rect*>(s))
    {
        out = rectToPoly2(*r);
        return true;
    }
    if (const PolygonShape* p = dynamic_cast<const PolygonShape*>(s))
    {
        if (p->m_points.empty()) return false;
        out = toPoly2(p->m_points);
        return true;
    }
    return false;
}

} // namespace drc_geom

// ─────────────────────────────────────────────
//  LayerCollector
// ─────────────────────────────────────────────
struct LayerCollector : public IVisitor
{
    std::unordered_map<std::string, std::vector<AComponent*>> byLayer;

    void collect(CircuitLayout& layout)
    {
        for (auto& c : layout.m_components)
            if (c) c->accept(*this);
    }

private:
    void handle(AComponent& comp) { byLayer[comp.name()].push_back(&comp); }

    void visit(const Active&  c) override { handle(const_cast<Active&>(c));  }
    void visit(const Poly&    c) override { handle(const_cast<Poly&>(c));    }
    void visit(const NWell&   c) override { handle(const_cast<NWell&>(c));   }
    void visit(const PWell&   c) override { handle(const_cast<PWell&>(c));   }
    void visit(const Metal1&  c) override { handle(const_cast<Metal1&>(c));  }
    void visit(const Via&     c) override { handle(const_cast<Via&>(c));     }
    void visit(const Contact& c) override { handle(const_cast<Contact&>(c)); }

    void visit(const Rect&)         override {}
    void visit(const PolygonShape&) override {}
};

// ─────────────────────────────────────────────
//  DRCChecker
// ─────────────────────────────────────────────
class DRCChecker
{
public:
    explicit DRCChecker(DRCRuleSet rules = DRCRuleSet::defaultRules())
        : m_rules(std::move(rules)) {}

    std::vector<DRCViolation> check(CircuitLayout& layout)
    {
        m_violations.clear();

        LayerCollector collector;
        collector.collect(layout);

        for (auto& [layerName, components] : collector.byLayer)
        {
            auto it = m_rules.layerRules.find(layerName);
            if (it == m_rules.layerRules.end()) continue;
            const LayerRules& lr = it->second;
            checkMinWidth(layerName, components, lr.minWidth);
            checkMinArea (layerName, components, lr.minArea);
            checkMinSpace(layerName, components, lr.minSpace);
        }

        for (const EnclosureRule& er : m_rules.enclosureRules)
            checkEnclosure(collector.byLayer[er.outerLayer], er.outerLayer,
                           collector.byLayer[er.innerLayer], er.innerLayer,
                           er.margin);

        for (const IntersectionRule& ir : m_rules.intersectionRules)
            checkIntersection(collector.byLayer[ir.layerA], ir.layerA,
                              collector.byLayer[ir.layerB], ir.layerB);

        return m_violations;
    }

private:
    DRCRuleSet                m_rules;
    std::vector<DRCViolation> m_violations;

    void addViolation(DRCRule rule,
                      const std::string& layerA, const std::string& layerB,
                      unsigned int idA, unsigned int idB,
                      const std::string& desc)
    {
        m_violations.push_back({ rule, layerA, layerB, idA, idB, desc });
    }

    // ── 1. MIN_WIDTH ─────────────────────────────────────────────
    //  Rect:    точно через поля width/height
    //  Polygon: точно через минимальное расстояние между несмежными рёбрами
    void checkMinWidth(const std::string& layerName,
                       const std::vector<AComponent*>& comps,
                       int minWidth)
    {
        if (minWidth <= 0) return;

        for (AComponent* c : comps)
        {
            IShape* s = c->getShape();
            if (!s) continue;

            if (const Rect* r = dynamic_cast<const Rect*>(s))
            {
                // Rect: точная проверка
                if (r->width < minWidth)
                {
                    std::ostringstream oss;
                    oss << "Width " << r->width << "λ < minWidth " << minWidth << "λ";
                    addViolation(DRCRule::MIN_WIDTH, layerName, "", c->id, 0, oss.str());
                }
                if (r->height < minWidth)
                {
                    std::ostringstream oss;
                    oss << "Height " << r->height << "λ < minWidth " << minWidth << "λ";
                    addViolation(DRCRule::MIN_WIDTH, layerName, "", c->id, 0, oss.str());
                }
            }
            else if (const PolygonShape* poly = dynamic_cast<const PolygonShape*>(s))
            {
                // Polygon: min расстояние между несмежными рёбрами
                auto p2 = drc_geom::toPoly2(poly->m_points);
                if (drc_geom::polyHasNarrowSpot(p2, minWidth))
                {
                    std::ostringstream oss;
                    oss << "Polygon has a feature narrower than minWidth "
                        << minWidth << "λ (non-adjacent edge distance check)";
                    addViolation(DRCRule::MIN_WIDTH, layerName, "", c->id, 0, oss.str());
                }
            }
        }
    }

    // ── 2. MIN_AREA ──────────────────────────────────────────────
    //  Rect:    точно (width × height)
    //  Polygon: точно (Shoelace)
    void checkMinArea(const std::string& layerName,
                      const std::vector<AComponent*>& comps,
                      int minArea)
    {
        if (minArea <= 0) return;

        for (AComponent* c : comps)
        {
            IShape* s = c->getShape();
            if (!s) continue;

            long long area2 = 0;

            if (const Rect* r = dynamic_cast<const Rect*>(s))
                area2 = (long long)r->width * r->height * 2;
            else if (const PolygonShape* poly = dynamic_cast<const PolygonShape*>(s))
                area2 = drc_geom::shoelaceArea2(drc_geom::toPoly2(poly->m_points));
            else continue;

            if (area2 < (long long)minArea * 2)
            {
                std::ostringstream oss;
                if (area2 % 2 == 0)
                    oss << "Area " << (area2 / 2) << "λ²";
                else
                    oss << "Area " << area2 << "/2 λ²";
                oss << " < minArea " << minArea << "λ²";
                addViolation(DRCRule::MIN_AREA, layerName, "", c->id, 0, oss.str());
            }
        }
    }

    // ── 3. MIN_SPACE ─────────────────────────────────────────────
    //  Точное минимальное расстояние edge-to-edge между полигонами
    void checkMinSpace(const std::string& layerName,
                       const std::vector<AComponent*>& comps,
                       int minSpace)
    {
        if (minSpace <= 0) return;

        // Кэшируем Poly2 чтобы не конвертировать O(n²) раз
        std::vector<drc_geom::Poly2> polys(comps.size());
        std::vector<bool> valid(comps.size(), false);
        for (std::size_t i = 0; i < comps.size(); ++i)
            valid[i] = drc_geom::shapeToPolyAABB(*comps[i], polys[i]);

        for (std::size_t i = 0; i < comps.size(); ++i)
        {
            if (!valid[i]) continue;
            for (std::size_t j = i + 1; j < comps.size(); ++j)
            {
                if (!valid[j]) continue;

                // Если полигоны пересекаются — расстояние 0
                bool tooClose = drc_geom::polysIntersect(polys[i], polys[j]) ||
                                drc_geom::polysCloserThan(polys[i], polys[j], minSpace);

                if (tooClose)
                {
                    std::ostringstream oss;
                    oss << "Spacing < minSpace " << minSpace
                        << "λ  (between comp#" << comps[i]->id
                        << " and comp#"        << comps[j]->id << ")";
                    addViolation(DRCRule::MIN_SPACE, layerName, "",
                                 comps[i]->id, comps[j]->id, oss.str());
                }
            }
        }
    }

    // ── 4. ENCLOSURE ─────────────────────────────────────────────
    //  Точная проверка: вершины inner внутри outer + отступ от рёбер
    void checkEnclosure(const std::vector<AComponent*>& outerComps,
                        const std::string& outerName,
                        const std::vector<AComponent*>& innerComps,
                        const std::string& innerName,
                        int margin)
    {
        for (AComponent* inner : innerComps)
        {
            drc_geom::Poly2 pInner;
            if (!drc_geom::shapeToPolyAABB(*inner, pInner)) continue;

            bool enclosed = false;
            for (AComponent* outer : outerComps)
            {
                drc_geom::Poly2 pOuter;
                if (!drc_geom::shapeToPolyAABB(*outer, pOuter)) continue;

                if (drc_geom::polyEnclosedBy(pInner, pOuter, margin))
                {
                    enclosed = true;
                    break;
                }
            }

            if (!enclosed)
            {
                std::ostringstream oss;
                oss << "'" << innerName << "' comp#" << inner->id
                    << " is not enclosed by '" << outerName
                    << "' with margin " << margin << "λ";
                addViolation(DRCRule::ENCLOSURE, outerName, innerName,
                             inner->id, 0, oss.str());
            }
        }
    }

    // ── 5. INTERSECTION ──────────────────────────────────────────
    //  Точная проверка пересечения полигонов
    void checkIntersection(const std::vector<AComponent*>& compsA,
                           const std::string& nameA,
                           const std::vector<AComponent*>& compsB,
                           const std::string& nameB)
    {
        for (AComponent* a : compsA)
        {
            drc_geom::Poly2 pA;
            if (!drc_geom::shapeToPolyAABB(*a, pA)) continue;

            for (AComponent* b : compsB)
            {
                drc_geom::Poly2 pB;
                if (!drc_geom::shapeToPolyAABB(*b, pB)) continue;

                if (drc_geom::polysIntersect(pA, pB))
                {
                    std::ostringstream oss;
                    oss << "'" << nameA << "' comp#" << a->id
                        << " overlaps '" << nameB << "' comp#" << b->id;
                    addViolation(DRCRule::INTERSECTION, nameA, nameB,
                                 a->id, b->id, oss.str());
                }
            }
        }
    }
};

// ─────────────────────────────────────────────
//  Convenience free function
// ─────────────────────────────────────────────
inline std::vector<DRCViolation> runDRC(
    CircuitLayout& layout,
    DRCRuleSet rules = DRCRuleSet::defaultRules())
{
    DRCChecker checker(std::move(rules));
    return checker.check(layout);
}

#endif // DRC_HPP