#ifndef DRC_HPP
#define DRC_HPP

// ============================================================
//  Design Rule Checker  —  VLSI Layout Tool
//
//  Supported rules:
//    1. MIN_WIDTH   — shape width/height must be >= threshold
//    2. MIN_SPACE   — spacing between same-layer shapes >= threshold
//    3. MIN_AREA    — shape area must be >= threshold
//    4. ENCLOSURE   — Contact must be fully enclosed by Active
//                     and by Metal1 (with per-layer enclosure margin)
//    5. INTERSECTION— shapes on "forbidden-pair" layers must not
//                     overlap (e.g. NWell ∩ PWell)
//
//  All thresholds are in lambda-units (integers).
//  Only Rect geometry is handled for now; PolygonShape shapes
//  are skipped with a dedicated UNSUPPORTED_GEOMETRY violation.
// ============================================================

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <sstream>
#include <limits>
#include <algorithm>

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
    DRCRule     rule;
    std::string layerName;      // primary layer name
    std::string layerNameB;     // secondary layer (for ENCLOSURE / INTERSECTION)
    unsigned int componentIdA   = 0;
    unsigned int componentIdB   = 0; // second component (if applicable)
    std::string description;

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
//  DRCRuleSet  (configurable thresholds)
// ─────────────────────────────────────────────

// Per-layer minimum rules
struct LayerRules
{
    int minWidth   = 0;   // lambda
    int minSpace   = 0;   // lambda
    int minArea    = 0;   // lambda²
};

// Enclosure rule: the "outer" layer must extend beyond the "inner" layer
// by at least `margin` on all sides.
struct EnclosureRule
{
    std::string outerLayer;
    std::string innerLayer;
    int margin = 0;
};

// Two layers that must never overlap
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

    // Factory: reasonable defaults for a generic Lambda-based process
    static DRCRuleSet defaultRules()
    {
        DRCRuleSet rs;

        // ── per-layer rules ──────────────────────────────────────
        //                  minWidth  minSpace  minArea
        rs.layerRules["active"]  = { 2,  3,  4  };
        rs.layerRules["poly"]    = { 1,  2,  2  };
        rs.layerRules["nwell"]   = { 4,  6,  16 };
        rs.layerRules["pwell"]   = { 4,  6,  16 };
        rs.layerRules["metal1"]  = { 2,  3,  4  };
        rs.layerRules["via"]     = { 1,  2,  1  };
        rs.layerRules["contact"] = { 1,  2,  1  };

        // ── enclosure rules ──────────────────────────────────────
        // Contact must be enclosed by Active  (margin = 1λ)
        rs.enclosureRules.push_back({ "active",  "contact", 1 });
        // Contact must be enclosed by Metal1  (margin = 1λ)
        rs.enclosureRules.push_back({ "metal1",  "contact", 1 });

        // ── forbidden intersections ──────────────────────────────
        rs.intersectionRules.push_back({ "nwell", "pwell" });

        return rs;
    }
};

// ─────────────────────────────────────────────
//  Internal geometry helpers
// ─────────────────────────────────────────────
namespace drc_detail {

// Axis-aligned bounding box derived from a Rect
struct AABB
{
    int x1, y1, x2, y2; // inclusive

    static AABB fromRect(const Rect& r)
    {
        return { r.point.x,
                 r.point.y,
                 r.point.x + r.width,
                 r.point.y + r.height };
    }

    bool intersects(const AABB& o) const
    {
        return x1 < o.x2 && x2 > o.x1 &&
               y1 < o.y2 && y2 > o.y1;
    }

    // Returns true if `inner` fits inside `outer` with at least `margin` gap
    static bool enclosedBy(const AABB& inner, const AABB& outer, int margin)
    {
        return outer.x1 + margin <= inner.x1 &&
               outer.y1 + margin <= inner.y1 &&
               inner.x2 + margin <= outer.x2 &&
               inner.y2 + margin <= outer.y2;
    }

    // Euclidean-inspired but integer: minimum axis separation (0 if overlapping)
    static int minSeparation(const AABB& a, const AABB& b)
    {
        int dx = std::max(0, std::max(a.x1, b.x1) - std::min(a.x2, b.x2));
        int dy = std::max(0, std::max(a.y1, b.y1) - std::min(a.y2, b.y2));
        // Rectilinear (Chebyshev) distance is the more conservative measure
        // but for DRC the standard is the smaller axis gap, so:
        if (a.intersects(b)) return 0;
        // non-overlapping: return the actual shortest rectilinear distance
        if (dx == 0) return dy; // vertically separated, touching in x
        if (dy == 0) return dx; // horizontally separated, touching in y
        // diagonal: minimum of the two axes is the "channel" that's too small
        return std::min(dx, dy);
    }
};

// Try to extract a Rect from an AComponent's shape.
// Returns nullptr if shape is absent or is not a Rect.
const Rect* getRect(AComponent& comp)
{
    IShape* s = comp.getShape();
    if (!s) return nullptr;
    return dynamic_cast<const Rect*>(s);
}

} // namespace drc_detail

// ─────────────────────────────────────────────
//  Layer collector visitor
// ─────────────────────────────────────────────
//  Walks a CircuitLayout and buckets every component by layer name.

struct LayerCollector : public IVisitor
{
    // layer-name → list of (component ptr, rect or nullptr)
    std::unordered_map<std::string, std::vector<AComponent*>> byLayer;

    // Components whose shape is a PolygonShape (not yet handled by DRC)
    std::vector<AComponent*> polygonComponents;

    void collect(CircuitLayout& layout)
    {
        for (auto& c : layout.m_components)
            if (c) c->accept(*this);
    }

private:
    void handle(AComponent& comp)
    {
        IShape* s = comp.getShape();
        if (s && dynamic_cast<PolygonShape*>(s))
            polygonComponents.push_back(&comp);
        else
            byLayer[comp.name()].push_back(&comp);
    }

    void visit(const Active&  c) override { handle(const_cast<Active&>(c));  }
    void visit(const Poly&    c) override { handle(const_cast<Poly&>(c));    }
    void visit(const NWell&   c) override { handle(const_cast<NWell&>(c));   }
    void visit(const PWell&   c) override { handle(const_cast<PWell&>(c));   }
    void visit(const Metal1&  c) override { handle(const_cast<Metal1&>(c));  }
    void visit(const Via&     c) override { handle(const_cast<Via&>(c));     }
    void visit(const Contact& c) override { handle(const_cast<Contact&>(c)); }

    // Shape visitors — not used by the collector itself
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

    // Run all checks on a CircuitLayout, return list of violations
    std::vector<DRCViolation> check(CircuitLayout& layout)
    {
        m_violations.clear();

        // Collect & bucket components by layer
        LayerCollector collector;
        collector.collect(layout);

        // Warn about polygon shapes — DRC not implemented for them yet
        for (AComponent* c : collector.polygonComponents)
        {
            DRCViolation v;
            v.rule         = DRCRule::UNSUPPORTED_GEOMETRY;
            v.layerName    = c->name();
            v.componentIdA = c->id;
            v.description  = "PolygonShape geometry is not yet supported by DRC; skipped.";
            m_violations.push_back(v);
        }

        // ── per-layer checks ─────────────────────────────────────
        for (auto& [layerName, components] : collector.byLayer)
        {
            auto it = m_rules.layerRules.find(layerName);
            if (it == m_rules.layerRules.end()) continue;

            const LayerRules& lr = it->second;

            checkMinWidth (layerName, components, lr.minWidth);
            checkMinArea  (layerName, components, lr.minArea);
            checkMinSpace (layerName, components, lr.minSpace);
        }

        // ── enclosure checks ─────────────────────────────────────
        for (const EnclosureRule& er : m_rules.enclosureRules)
        {
            auto& outerComps = collector.byLayer[er.outerLayer];
            auto& innerComps = collector.byLayer[er.innerLayer];
            checkEnclosure(outerComps, er.outerLayer,
                           innerComps, er.innerLayer,
                           er.margin);
        }

        // ── intersection checks ──────────────────────────────────
        for (const IntersectionRule& ir : m_rules.intersectionRules)
        {
            auto& compsA = collector.byLayer[ir.layerA];
            auto& compsB = collector.byLayer[ir.layerB];
            checkIntersection(compsA, ir.layerA,
                              compsB, ir.layerB);
        }

        return m_violations;
    }

private:
    DRCRuleSet              m_rules;
    std::vector<DRCViolation> m_violations;

    // ── helpers ─────────────────────────────────────────────────

    void addViolation(DRCRule rule,
                      const std::string& layerA,
                      const std::string& layerB,
                      unsigned int idA,
                      unsigned int idB,
                      const std::string& desc)
    {
        DRCViolation v;
        v.rule         = rule;
        v.layerName    = layerA;
        v.layerNameB   = layerB;
        v.componentIdA = idA;
        v.componentIdB = idB;
        v.description  = desc;
        m_violations.push_back(v);
    }

    // ── 1. MIN_WIDTH ─────────────────────────────────────────────
    void checkMinWidth(const std::string& layerName,
                       const std::vector<AComponent*>& comps,
                       int minWidth)
    {
        if (minWidth <= 0) return;

        for (AComponent* c : comps)
        {
            const Rect* r = drc_detail::getRect(*c);
            if (!r) continue;

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
    }

    // ── 2. MIN_AREA ──────────────────────────────────────────────
    void checkMinArea(const std::string& layerName,
                      const std::vector<AComponent*>& comps,
                      int minArea)
    {
        if (minArea <= 0) return;

        for (AComponent* c : comps)
        {
            const Rect* r = drc_detail::getRect(*c);
            if (!r) continue;

            int area = r->width * r->height;
            if (area < minArea)
            {
                std::ostringstream oss;
                oss << "Area " << area << "λ² < minArea " << minArea << "λ²";
                addViolation(DRCRule::MIN_AREA, layerName, "", c->id, 0, oss.str());
            }
        }
    }

    // ── 3. MIN_SPACE ─────────────────────────────────────────────
    // O(n²) within the same layer — acceptable for a prototype
    void checkMinSpace(const std::string& layerName,
                       const std::vector<AComponent*>& comps,
                       int minSpace)
    {
        if (minSpace <= 0) return;

        for (std::size_t i = 0; i < comps.size(); ++i)
        {
            const Rect* ri = drc_detail::getRect(*comps[i]);
            if (!ri) continue;
            auto aabbI = drc_detail::AABB::fromRect(*ri);

            for (std::size_t j = i + 1; j < comps.size(); ++j)
            {
                const Rect* rj = drc_detail::getRect(*comps[j]);
                if (!rj) continue;
                auto aabbJ = drc_detail::AABB::fromRect(*rj);

                int sep = drc_detail::AABB::minSeparation(aabbI, aabbJ);
                if (sep < minSpace)
                {
                    std::ostringstream oss;
                    oss << "Spacing " << sep << "λ < minSpace " << minSpace
                        << "λ  (between comp#" << comps[i]->id
                        << " and comp#" << comps[j]->id << ")";
                    addViolation(DRCRule::MIN_SPACE, layerName, "",
                                 comps[i]->id, comps[j]->id, oss.str());
                }
            }
        }
    }

    // ── 4. ENCLOSURE ─────────────────────────────────────────────
    // Every inner-layer shape must be fully enclosed by at least one
    // outer-layer shape with the required margin.
    void checkEnclosure(const std::vector<AComponent*>& outerComps,
                        const std::string& outerName,
                        const std::vector<AComponent*>& innerComps,
                        const std::string& innerName,
                        int margin)
    {
        for (AComponent* inner : innerComps)
        {
            const Rect* ri = drc_detail::getRect(*inner);
            if (!ri) continue;
            auto aabbInner = drc_detail::AABB::fromRect(*ri);

            bool enclosed = false;
            for (AComponent* outer : outerComps)
            {
                const Rect* ro = drc_detail::getRect(*outer);
                if (!ro) continue;
                auto aabbOuter = drc_detail::AABB::fromRect(*ro);

                if (drc_detail::AABB::enclosedBy(aabbInner, aabbOuter, margin))
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
    // Shapes on two different layers must not geometrically overlap.
    void checkIntersection(const std::vector<AComponent*>& compsA,
                           const std::string& nameA,
                           const std::vector<AComponent*>& compsB,
                           const std::string& nameB)
    {
        for (AComponent* a : compsA)
        {
            const Rect* ra = drc_detail::getRect(*a);
            if (!ra) continue;
            auto aabbA = drc_detail::AABB::fromRect(*ra);

            for (AComponent* b : compsB)
            {
                const Rect* rb = drc_detail::getRect(*b);
                if (!rb) continue;
                auto aabbB = drc_detail::AABB::fromRect(*rb);

                if (aabbA.intersects(aabbB))
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