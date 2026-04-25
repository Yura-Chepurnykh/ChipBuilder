// =============================================================================
//  incremental_drc.hpp  —  Minimal Incremental DRC (Magic VLSI style)
//
//  Features:
//    • Plane of Tiles  (one plane per layer)
//    • Corner Stitching (NW / NE / SW / SE neighbours)
//    • Merge / Split tiles
//    • Incremental DRC:  min-width, min-space, min-area,
//                        intersection, enclosure, overlap
//    • 7 layers: Active, Poly, NWell, PWell, Metal1, Via, Contact
//
//  Everything in one header; C++17 required.
//  Usage:
//      Layout layout;
//      layout.insertRect(LayerType::Metal1, {0,0,100,50});
//      layout.insertRect(LayerType::Metal1, {110,0,200,50});
//      auto violations = layout.runDRC();
//      for (auto& v : violations) std::cout << v.message << "\n";
// =============================================================================
#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace DRC {

// =============================================================================
//  Layer types
// =============================================================================
enum class LayerType : uint8_t {
    Active  = 0,
    Poly    = 1,
    NWell   = 2,
    PWell   = 3,
    Metal1  = 4,
    Via     = 5,
    Contact = 6,
    _Count  = 7
};

static const char* layerName(LayerType lt) {
    switch (lt) {
        case LayerType::Active:  return "Active";
        case LayerType::Poly:    return "Poly";
        case LayerType::NWell:   return "NWell";
        case LayerType::PWell:   return "PWell";
        case LayerType::Metal1:  return "Metal1";
        case LayerType::Via:     return "Via";
        case LayerType::Contact: return "Contact";
        default:                 return "Unknown";
    }
}

// =============================================================================
//  Coordinate type (integer lambda units)
// =============================================================================
using Coord = int32_t;
static constexpr Coord COORD_MIN = std::numeric_limits<Coord>::min() / 2;
static constexpr Coord COORD_MAX = std::numeric_limits<Coord>::max() / 2;

// =============================================================================
//  Rect  – axis-aligned bounding rectangle [x1,x2) × [y1,y2)
// =============================================================================
struct Rect {
    Coord x1, y1, x2, y2;   // x1<x2, y1<y2

    Rect() : x1(0), y1(0), x2(0), y2(0) {}
    Rect(Coord x1, Coord y1, Coord x2, Coord y2)
        : x1(x1), y1(y1), x2(x2), y2(y2) {
        assert(x1 <= x2 && y1 <= y2);
    }

    bool valid()  const { return x1 < x2 && y1 < y2; }
    Coord width() const { return x2 - x1; }
    Coord height()const { return y2 - y1; }
    int64_t area()const { return (int64_t)(x2-x1)*(y2-y1); }

    bool contains(Coord x, Coord y) const {
        return x >= x1 && x < x2 && y >= y1 && y < y2;
    }
    bool overlaps(const Rect& o) const {
        return x1 < o.x2 && x2 > o.x1 && y1 < o.y2 && y2 > o.y1;
    }
    std::optional<Rect> intersect(const Rect& o) const {
        Coord nx1 = std::max(x1, o.x1), ny1 = std::max(y1, o.y1);
        Coord nx2 = std::min(x2, o.x2), ny2 = std::min(y2, o.y2);
        if (nx1 >= nx2 || ny1 >= ny2) return std::nullopt;
        return Rect{nx1, ny1, nx2, ny2};
    }
    Rect unite(const Rect& o) const {
        return {std::min(x1,o.x1), std::min(y1,o.y1),
                std::max(x2,o.x2), std::max(y2,o.y2)};
    }
    bool operator==(const Rect& o) const {
        return x1==o.x1 && y1==o.y1 && x2==o.x2 && y2==o.y2;
    }
    std::string str() const {
        std::ostringstream s;
        s << "(" << x1 << "," << y1 << ")-(" << x2 << "," << y2 << ")";
        return s.str();
    }
};

// =============================================================================
//  Tile  – basic unit of the Corner-Stitch plane
//
//  A tile covers a horizontal strip of the plane.
//  Solid tiles carry geometry; space tiles are empty.
//  Corner stitches: rt (right-top), rb (right-bottom),
//                   lt (left-top),  lb (left-bottom)
//  means: rt = first solid tile whose bottom edge is above this tile's bottom
//              on the right side, etc.  (Magic convention simplified)
// =============================================================================
struct Tile {
    Rect     bbox;          // tile bounding box
    bool     solid = false; // true → occupied geometry
    LayerType layer;

    // Corner stitches (raw pointers – owned by TilePlane)
    Tile* rt = nullptr;  // right-top
    Tile* rb = nullptr;  // right-bottom
    Tile* lt = nullptr;  // left-top
    Tile* lb = nullptr;  // left-bottom
    Tile* tr = nullptr;  // top-right  (above, right side)
    Tile* tl = nullptr;  // top-left
    Tile* br = nullptr;  // bottom-right
    Tile* bl = nullptr;  // bottom-left

    explicit Tile(LayerType l = LayerType::Active) : layer(l) {}
};

// =============================================================================
//  TilePlane  – Corner-Stitched Plane of Tiles for one layer
//
//  Implementation strategy (simplified Magic):
//    • The entire plane is initially one big space tile.
//    • insertRect():  split existing tiles, mark region solid, merge neighbours.
//    • We keep a flat tile list for simplicity (O(n) search; good enough for
//      a "minimal" implementation – a real BVH/quadtree speeds this up).
// =============================================================================
class TilePlane {
public:
    LayerType layer;

    explicit TilePlane(LayerType l) : layer(l) {
        // seed: one giant space tile covering everything
        auto* t  = newTile();
        t->bbox  = {COORD_MIN, COORD_MIN, COORD_MAX, COORD_MAX};
        t->solid = false;
        tiles_.push_back(t);
    }

    ~TilePlane() {
        for (auto* t : tiles_) delete t;
    }

    // -------------------------------------------------------------------------
    //  Public API
    // -------------------------------------------------------------------------

    /// Insert a filled rectangle into the plane.
    void insertRect(const Rect& r) {
        splitForRect(r);
        for (auto* t : tiles_)
            if (t->bbox == r || rectsIntersect(t->bbox, r))
                if (fullyContained(t->bbox, r))
                    t->solid = true;
        mergeAll();
    }

    /// Remove a filled rectangle from the plane.
    void removeRect(const Rect& r) {
        splitForRect(r);
        for (auto* t : tiles_)
            if (fullyContained(t->bbox, r))
                t->solid = false;
        mergeAll();
    }

    /// Enumerate all solid tiles, calling cb(tile).
    void forEachSolid(std::function<void(const Tile*)> cb) const {
        for (auto* t : tiles_)
            if (t->solid) cb(t);
    }

    /// Enumerate all tiles (solid+space) in the given rect.
    void forEachInRect(const Rect& query,
                       std::function<void(const Tile*)> cb) const {
        for (auto* t : tiles_)
            if (rectsIntersect(t->bbox, query)) cb(t);
    }

    const std::vector<Tile*>& allTiles() const { return tiles_; }

private:
    std::vector<Tile*> tiles_;

    Tile* newTile() {
        auto* t = new Tile(layer);
        return t;
    }

    static bool rectsIntersect(const Rect& a, const Rect& b) {
        return a.x1 < b.x2 && a.x2 > b.x1 && a.y1 < b.y2 && a.y2 > b.y1;
    }
    static bool fullyContained(const Rect& inner, const Rect& outer) {
        return inner.x1 >= outer.x1 && inner.x2 <= outer.x2 &&
               inner.y1 >= outer.y1 && inner.y2 <= outer.y2;
    }

    // -------------------------------------------------------------------------
    //  Split  –  subdivide tiles so that 'r' aligns with tile boundaries.
    //  We split along x1, x2, y1, y2 of r.
    // -------------------------------------------------------------------------
    void splitForRect(const Rect& r) {
        splitAtX(r.x1);
        splitAtX(r.x2);
        splitAtY(r.y1);
        splitAtY(r.y2);
    }

    void splitAtX(Coord x) {
        std::vector<Tile*> toAdd;
        std::vector<Tile*> toDelete;
        for (auto* t : tiles_) {
            if (x > t->bbox.x1 && x < t->bbox.x2) {
                auto* left  = newTile();
                auto* right = newTile();
                left->bbox  = {t->bbox.x1, t->bbox.y1, x,          t->bbox.y2};
                right->bbox = {x,          t->bbox.y1, t->bbox.x2, t->bbox.y2};
                left->solid  = t->solid;
                right->solid = t->solid;
                toAdd.push_back(left);
                toAdd.push_back(right);
                toDelete.push_back(t);
            }
        }
        for (auto* t : toDelete) {
            tiles_.erase(std::find(tiles_.begin(), tiles_.end(), t));
            delete t;
        }
        for (auto* t : toAdd) tiles_.push_back(t);
    }

    void splitAtY(Coord y) {
        std::vector<Tile*> toAdd;
        std::vector<Tile*> toDelete;
        for (auto* t : tiles_) {
            if (y > t->bbox.y1 && y < t->bbox.y2) {
                auto* bot = newTile();
                auto* top = newTile();
                bot->bbox  = {t->bbox.x1, t->bbox.y1, t->bbox.x2, y};
                top->bbox  = {t->bbox.x1, y,           t->bbox.x2, t->bbox.y2};
                bot->solid  = t->solid;
                top->solid  = t->solid;
                toAdd.push_back(bot);
                toAdd.push_back(top);
                toDelete.push_back(t);
            }
        }
        for (auto* t : toDelete) {
            tiles_.erase(std::find(tiles_.begin(), tiles_.end(), t));
            delete t;
        }
        for (auto* t : toAdd) tiles_.push_back(t);
    }

    // -------------------------------------------------------------------------
    //  Merge  –  combine adjacent tiles with the same solid flag.
    //  Greedy pass: repeatedly scan until no merges happen.
    // -------------------------------------------------------------------------
    void mergeAll() {
        bool changed = true;
        while (changed) {
            changed = false;
            for (size_t i = 0; i < tiles_.size() && !changed; ++i) {
                for (size_t j = i+1; j < tiles_.size() && !changed; ++j) {
                    if (tryMerge(i, j)) changed = true;
                }
            }
        }
        rebuildStitches();
    }

    /// Try merging tiles_[i] and tiles_[j]. Returns true if merged.
    bool tryMerge(size_t i, size_t j) {
        auto* a = tiles_[i];
        auto* b = tiles_[j];
        if (a->solid != b->solid) return false;

        // horizontally adjacent: same y-span, a.x2 == b.x1
        if (a->bbox.y1 == b->bbox.y1 && a->bbox.y2 == b->bbox.y2) {
            Tile *left = nullptr, *right = nullptr;
            if (a->bbox.x2 == b->bbox.x1) { left = a; right = b; }
            else if (b->bbox.x2 == a->bbox.x1) { left = b; right = a; }
            if (left && right) {
                left->bbox.x2 = right->bbox.x2;
                tiles_.erase(tiles_.begin() + (int)j);
                return true;
            }
        }
        // vertically adjacent: same x-span, a.y2 == b.y1
        if (a->bbox.x1 == b->bbox.x1 && a->bbox.x2 == b->bbox.x2) {
            Tile *bot = nullptr, *top = nullptr;
            if (a->bbox.y2 == b->bbox.y1) { bot = a; top = b; }
            else if (b->bbox.y2 == a->bbox.y1) { bot = b; top = a; }
            if (bot && top) {
                bot->bbox.y2 = top->bbox.y2;
                tiles_.erase(tiles_.begin() + (int)j);
                return true;
            }
        }
        return false;
    }

    // -------------------------------------------------------------------------
    //  Rebuild corner stitches (simplified: linear scan).
    //  In production Magic uses O(1) amortized via stitch pointers; here we
    //  just recompute neighbours after each merge pass.
    // -------------------------------------------------------------------------
    void rebuildStitches() {
        for (auto* t : tiles_) {
            t->rt = t->rb = t->lt = t->lb = nullptr;
            t->tr = t->tl = t->br = t->bl = nullptr;
        }
        for (auto* t : tiles_) {
            for (auto* u : tiles_) {
                if (u == t) continue;
                const Rect& tb = t->bbox;
                const Rect& ub = u->bbox;

                // right neighbour: u starts where t ends in x, y-ranges overlap
                if (ub.x1 == tb.x2 && ub.y1 < tb.y2 && ub.y2 > tb.y1) {
                    // rt: topmost right neighbour, rb: bottommost
                    if (!t->rt || ub.y1 > t->rt->bbox.y1) t->rt = u;
                    if (!t->rb || ub.y1 < t->rb->bbox.y1) t->rb = u;
                }
                // left neighbour
                if (ub.x2 == tb.x1 && ub.y1 < tb.y2 && ub.y2 > tb.y1) {
                    if (!t->lt || ub.y1 > t->lt->bbox.y1) t->lt = u;
                    if (!t->lb || ub.y1 < t->lb->bbox.y1) t->lb = u;
                }
                // top neighbour
                if (ub.y1 == tb.y2 && ub.x1 < tb.x2 && ub.x2 > tb.x1) {
                    if (!t->tr || ub.x1 > t->tr->bbox.x1) t->tr = u;
                    if (!t->tl || ub.x1 < t->tl->bbox.x1) t->tl = u;
                }
                // bottom neighbour
                if (ub.y2 == tb.y1 && ub.x1 < tb.x2 && ub.x2 > tb.x1) {
                    if (!t->br || ub.x1 > t->br->bbox.x1) t->br = u;
                    if (!t->bl || ub.x1 < t->bl->bbox.x1) t->bl = u;
                }
            }
        }
    }
};

// =============================================================================
//  DRC Rules  – per-layer and cross-layer
// =============================================================================
struct LayerRules {
    LayerType layer;
    Coord minWidth  = 0;   // min dimension of any solid shape
    Coord minSpace  = 0;   // min distance between solid shapes (same layer)
    int64_t minArea = 0;   // min area of any solid shape
};

struct CrossLayerRule {
    enum class Kind { Enclosure, Overlap, Intersection } kind;
    LayerType inner;   // e.g. Via must be enclosed by Metal1
    LayerType outer;
    Coord     value;   // enclosure margin or min overlap size
};

// Default design rules (lambda-based, adjust as needed)
static std::vector<LayerRules> defaultLayerRules() {
    // {layer, minWidth, minSpace, minArea}
    return {
        { LayerType::Active,  3,  3,  9  },
        { LayerType::Poly,    2,  2,  4  },
        { LayerType::NWell,   4,  4,  16 },
        { LayerType::PWell,   4,  4,  16 },
        { LayerType::Metal1,  3,  3,  9  },
        { LayerType::Via,     2,  2,  4  },
        { LayerType::Contact, 2,  2,  4  },
    };
}

static std::vector<CrossLayerRule> defaultCrossLayerRules() {
    using K = CrossLayerRule::Kind;
    return {
        // Via must be enclosed by Metal1 by at least 1 lambda
        { K::Enclosure,    LayerType::Via,     LayerType::Metal1,  1 },
        // Contact must be enclosed by Active by at least 1 lambda
        { K::Enclosure,    LayerType::Contact, LayerType::Active,  1 },
        // Contact must be enclosed by Poly by at least 1 lambda
        { K::Enclosure,    LayerType::Contact, LayerType::Poly,    1 },
        // NWell / PWell must overlap Active (for transistors) by at least 2
        { K::Overlap,      LayerType::Active,  LayerType::NWell,   2 },
        { K::Overlap,      LayerType::Active,  LayerType::PWell,   2 },
        // Poly must intersect Active for gate formation
        { K::Intersection, LayerType::Poly,    LayerType::Active,  0 },
    };
}

// =============================================================================
//  DRC Violation
// =============================================================================
struct DRCViolation {
    std::string  ruleType;   // "min-width", "min-space", etc.
    LayerType    layer;
    Rect         location;
    std::string  message;
};

// =============================================================================
//  Layout  – top-level object: holds planes, rules, runs DRC
// =============================================================================
class Layout {
public:
    Layout() {
        // We will create planes on demand or just initialize zLevel 0
        ensureZLevel(0);
        layerRules_      = defaultLayerRules();
        crossLayerRules_ = defaultCrossLayerRules();
    }

    void clearAll() {
        zPlanes_.clear();
        ensureZLevel(0);
    }

    // -------------------------------------------------------------------------
    //  Geometry operations
    // -------------------------------------------------------------------------
    void insertRect(LayerType l, const Rect& r, int zLevel = 0) {
        ensureZLevel(zLevel);
        plane(l, zLevel).insertRect(r);
    }
    void removeRect(LayerType l, const Rect& r, int zLevel = 0) {
        if (zPlanes_.count(zLevel)) {
            plane(l, zLevel).removeRect(r);
        }
    }

    TilePlane& plane(LayerType l, int zLevel = 0) {
        ensureZLevel(zLevel);
        return *zPlanes_[zLevel].planes[(int)l];
    }
    const TilePlane& plane(LayerType l, int zLevel = 0) const {
        auto it = zPlanes_.find(zLevel);
        assert(it != zPlanes_.end());
        return *it->second.planes[(int)l];
    }

    std::vector<int> activeZLevels() const {
        std::vector<int> levels;
        for (auto const& [level, _] : zPlanes_) levels.push_back(level);
        return levels;
    }

    // -------------------------------------------------------------------------
    //  Rule accessors
    // -------------------------------------------------------------------------
    void setLayerRules(const std::vector<LayerRules>& rules) {
        layerRules_ = rules;
    }
    void setCrossLayerRules(const std::vector<CrossLayerRule>& rules) {
        crossLayerRules_ = rules;
    }

    // -------------------------------------------------------------------------
    //  Run full DRC – returns all violations
    // -------------------------------------------------------------------------
    std::vector<DRCViolation> runDRC() const {
        std::vector<DRCViolation> v;

        for (auto const& [zLevel, _] : zPlanes_) {
            for (auto& lr : layerRules_) {
                checkMinWidth (lr, zLevel, v);
                checkMinSpace (lr, zLevel, v);
                checkMinArea  (lr, zLevel, v);
            }
            for (auto& cr : crossLayerRules_) {
                switch (cr.kind) {
                    case CrossLayerRule::Kind::Enclosure:
                        checkEnclosure(cr, zLevel, v); break;
                    case CrossLayerRule::Kind::Overlap:
                        checkOverlap(cr, zLevel, v);   break;
                    case CrossLayerRule::Kind::Intersection:
                        checkIntersection(cr, zLevel, v); break;
                }
            }
        }
        return v;
    }

    // -------------------------------------------------------------------------
    //  Incremental DRC – check only the dirty region (after insert/remove)
    // -------------------------------------------------------------------------
    std::vector<DRCViolation> runIncrementalDRC(LayerType changedLayer,
                                                const Rect& dirtyRegion,
                                                int zLevel = 0) const {
        std::vector<DRCViolation> v;

        // Expand dirty region by max design rule to catch neighbouring shapes
        Coord margin = 0;
        for (auto& lr : layerRules_)
            margin = std::max(margin, std::max(lr.minWidth, lr.minSpace));
        Rect expanded = {
            dirtyRegion.x1 - margin, dirtyRegion.y1 - margin,
            dirtyRegion.x2 + margin, dirtyRegion.y2 + margin
        };

        for (auto& lr : layerRules_) {
            if (lr.layer != changedLayer) continue;
            checkMinWidthInRegion (lr, expanded, zLevel, v);
            checkMinSpaceInRegion (lr, expanded, zLevel, v);
            checkMinAreaInRegion  (lr, expanded, zLevel, v);
        }
        for (auto& cr : crossLayerRules_) {
            if (cr.inner != changedLayer && cr.outer != changedLayer) continue;
            switch (cr.kind) {
                case CrossLayerRule::Kind::Enclosure:
                    checkEnclosureInRegion(cr, expanded, zLevel, v); break;
                case CrossLayerRule::Kind::Overlap:
                    checkOverlapInRegion(cr, expanded, zLevel, v);   break;
                case CrossLayerRule::Kind::Intersection:
                    checkIntersectionInRegion(cr, expanded, zLevel, v); break;
            }
        }
        return v;
    }

private:
    struct ZPlane {
        std::unique_ptr<TilePlane> planes[(int)LayerType::_Count];
    };
    mutable std::unordered_map<int, ZPlane> zPlanes_;

    void ensureZLevel(int z) {
        if (zPlanes_.find(z) == zPlanes_.end()) {
            ZPlane zp;
            for (int i = 0; i < (int)LayerType::_Count; ++i)
                zp.planes[i] = std::make_unique<TilePlane>((LayerType)i);
            zPlanes_.emplace(z, std::move(zp));
        }
    }

    std::vector<LayerRules>    layerRules_;
    std::vector<CrossLayerRule> crossLayerRules_;

    // =========================================================================
    //  DRC checks – full-plane variants
    // =========================================================================

    // 1. Min-Width
    void checkMinWidth(const LayerRules& lr, int zLevel,
                       std::vector<DRCViolation>& out) const {
        plane(lr.layer, zLevel).forEachSolid([&](const Tile* t) {
            if (t->bbox.width()  < lr.minWidth ||
                t->bbox.height() < lr.minWidth) {
                DRCViolation v;
                v.ruleType = "min-width";
                v.layer    = lr.layer;
                v.location = t->bbox;
                std::ostringstream s;
                s << "[min-width] Layer=" << layerName(lr.layer)
                  << " tile=" << t->bbox.str()
                  << " width=" << std::min(t->bbox.width(), t->bbox.height())
                  << " < " << lr.minWidth;
                v.message = s.str();
                out.push_back(v);
            }
        });
    }

    // 2. Min-Space  (check distance between every pair of solid tiles)
    void checkMinSpace(const LayerRules& lr, int zLevel,
                       std::vector<DRCViolation>& out) const {
        std::vector<const Tile*> solids;
        plane(lr.layer, zLevel).forEachSolid([&](const Tile* t) {
            solids.push_back(t);
        });
        for (size_t i = 0; i < solids.size(); ++i) {
            for (size_t j = i+1; j < solids.size(); ++j) {
                Coord dist = rectDistance(solids[i]->bbox, solids[j]->bbox);
                if (dist > 0 && dist < lr.minSpace) {
                    DRCViolation v;
                    v.ruleType = "min-space";
                    v.layer    = lr.layer;
                    v.location = solids[i]->bbox.unite(solids[j]->bbox);
                    std::ostringstream s;
                    s << "[min-space] Layer=" << layerName(lr.layer)
                      << " between " << solids[i]->bbox.str()
                      << " and " << solids[j]->bbox.str()
                      << " dist=" << dist << " < " << lr.minSpace;
                    v.message = s.str();
                    out.push_back(v);
                }
            }
        }
    }

    // 3. Min-Area
    void checkMinArea(const LayerRules& lr, int zLevel,
                      std::vector<DRCViolation>& out) const {
        plane(lr.layer, zLevel).forEachSolid([&](const Tile* t) {
            if (t->bbox.area() < lr.minArea) {
                DRCViolation v;
                v.ruleType = "min-area";
                v.layer    = lr.layer;
                v.location = t->bbox;
                std::ostringstream s;
                s << "[min-area] Layer=" << layerName(lr.layer)
                  << " tile=" << t->bbox.str()
                  << " area=" << t->bbox.area() << " < " << lr.minArea;
                v.message = s.str();
                out.push_back(v);
            }
        });
    }

    // 4. Intersection  (check that inner shapes intersect outer shapes)
    void checkIntersection(const CrossLayerRule& cr, int zLevel,
                           std::vector<DRCViolation>& out) const {
        plane(cr.inner, zLevel).forEachSolid([&](const Tile* ti) {
            bool found = false;
            plane(cr.outer, zLevel).forEachSolid([&](const Tile* to) {
                if (ti->bbox.overlaps(to->bbox)) found = true;
            });
            if (!found) {
                DRCViolation v;
                v.ruleType = "intersection";
                v.layer    = cr.inner;
                v.location = ti->bbox;
                std::ostringstream s;
                s << "[intersection] " << layerName(cr.inner)
                  << " tile " << ti->bbox.str()
                  << " does not intersect any " << layerName(cr.outer);
                v.message = s.str();
                out.push_back(v);
            }
        });
    }

    // 5. Enclosure  (inner shape must be enclosed by outer by 'value' margin)
    void checkEnclosure(const CrossLayerRule& cr, int zLevel,
                        std::vector<DRCViolation>& out) const {
        plane(cr.inner, zLevel).forEachSolid([&](const Tile* ti) {
            bool enclosed = false;
            plane(cr.outer, zLevel).forEachSolid([&](const Tile* to) {
                if (to->bbox.x1 <= ti->bbox.x1 - cr.value &&
                    to->bbox.y1 <= ti->bbox.y1 - cr.value &&
                    to->bbox.x2 >= ti->bbox.x2 + cr.value &&
                    to->bbox.y2 >= ti->bbox.y2 + cr.value)
                    enclosed = true;
            });
            if (!enclosed) {
                DRCViolation v;
                v.ruleType = "enclosure";
                v.layer    = cr.inner;
                v.location = ti->bbox;
                std::ostringstream s;
                s << "[enclosure] " << layerName(cr.inner)
                  << " tile " << ti->bbox.str()
                  << " not enclosed by " << layerName(cr.outer)
                  << " with margin=" << cr.value;
                v.message = s.str();
                out.push_back(v);
            }
        });
    }

    // 6. Overlap  (inner and outer must overlap by at least 'value' in each dim)
    void checkOverlap(const CrossLayerRule& cr, int zLevel,
                      std::vector<DRCViolation>& out) const {
        plane(cr.inner, zLevel).forEachSolid([&](const Tile* ti) {
            bool ok = false;
            plane(cr.outer, zLevel).forEachSolid([&](const Tile* to) {
                auto isect = ti->bbox.intersect(to->bbox);
                if (isect && isect->width()  >= cr.value &&
                             isect->height() >= cr.value)
                    ok = true;
            });
            if (!ok) {
                DRCViolation v;
                v.ruleType = "overlap";
                v.layer    = cr.inner;
                v.location = ti->bbox;
                std::ostringstream s;
                s << "[overlap] " << layerName(cr.inner)
                  << " tile " << ti->bbox.str()
                  << " insufficient overlap with " << layerName(cr.outer)
                  << " (need " << cr.value << ")";
                v.message = s.str();
                out.push_back(v);
            }
        });
    }

    // =========================================================================
    //  DRC checks – region-limited variants for incremental DRC
    // =========================================================================
    void checkMinWidthInRegion(const LayerRules& lr, const Rect& region, int zLevel,
                               std::vector<DRCViolation>& out) const {
        plane(lr.layer, zLevel).forEachInRect(region, [&](const Tile* t) {
            if (!t->solid) return;
            if (t->bbox.width()  < lr.minWidth ||
                t->bbox.height() < lr.minWidth) {
                DRCViolation v;
                v.ruleType = "min-width";
                v.layer    = lr.layer;
                v.location = t->bbox;
                std::ostringstream s;
                s << "[min-width/incr] Layer=" << layerName(lr.layer)
                  << " tile=" << t->bbox.str()
                  << " width=" << std::min(t->bbox.width(), t->bbox.height())
                  << " < " << lr.minWidth;
                v.message = s.str();
                out.push_back(v);
            }
        });
    }

    void checkMinSpaceInRegion(const LayerRules& lr, const Rect& region, int zLevel,
                               std::vector<DRCViolation>& out) const {
        std::vector<const Tile*> solids;
        plane(lr.layer, zLevel).forEachInRect(region, [&](const Tile* t) {
            if (t->solid) solids.push_back(t);
        });
        for (size_t i = 0; i < solids.size(); ++i)
            for (size_t j = i+1; j < solids.size(); ++j) {
                Coord dist = rectDistance(solids[i]->bbox, solids[j]->bbox);
                if (dist > 0 && dist < lr.minSpace) {
                    DRCViolation v;
                    v.ruleType = "min-space";
                    v.layer    = lr.layer;
                    v.location = solids[i]->bbox.unite(solids[j]->bbox);
                    std::ostringstream s;
                    s << "[min-space/incr] Layer=" << layerName(lr.layer)
                      << " dist=" << dist << " < " << lr.minSpace;
                    v.message = s.str();
                    out.push_back(v);
                }
            }
    }

    void checkMinAreaInRegion(const LayerRules& lr, const Rect& region, int zLevel,
                              std::vector<DRCViolation>& out) const {
        plane(lr.layer, zLevel).forEachInRect(region, [&](const Tile* t) {
            if (!t->solid) return;
            if (t->bbox.area() < lr.minArea) {
                DRCViolation v;
                v.ruleType = "min-area";
                v.layer    = lr.layer;
                v.location = t->bbox;
                std::ostringstream s;
                s << "[min-area/incr] Layer=" << layerName(lr.layer)
                  << " tile=" << t->bbox.str()
                  << " area=" << t->bbox.area() << " < " << lr.minArea;
                v.message = s.str();
                out.push_back(v);
            }
        });
    }

    void checkEnclosureInRegion(const CrossLayerRule& cr, const Rect& region, int zLevel,
                                std::vector<DRCViolation>& out) const {
        plane(cr.inner, zLevel).forEachInRect(region, [&](const Tile* ti) {
            if (!ti->solid) return;
            bool enclosed = false;
            plane(cr.outer, zLevel).forEachSolid([&](const Tile* to) {
                if (to->bbox.x1 <= ti->bbox.x1 - cr.value &&
                    to->bbox.y1 <= ti->bbox.y1 - cr.value &&
                    to->bbox.x2 >= ti->bbox.x2 + cr.value &&
                    to->bbox.y2 >= ti->bbox.y2 + cr.value)
                    enclosed = true;
            });
            if (!enclosed) {
                DRCViolation v;
                v.ruleType = "enclosure";
                v.layer    = cr.inner;
                v.location = ti->bbox;
                std::ostringstream s;
                s << "[enclosure/incr] " << layerName(cr.inner)
                  << " " << ti->bbox.str()
                  << " not enclosed by " << layerName(cr.outer)
                  << " margin=" << cr.value;
                v.message = s.str();
                out.push_back(v);
            }
        });
    }

    void checkOverlapInRegion(const CrossLayerRule& cr, const Rect& region, int zLevel,
                              std::vector<DRCViolation>& out) const {
        plane(cr.inner, zLevel).forEachInRect(region, [&](const Tile* ti) {
            if (!ti->solid) return;
            bool ok = false;
            plane(cr.outer, zLevel).forEachSolid([&](const Tile* to) {
                auto isect = ti->bbox.intersect(to->bbox);
                if (isect && isect->width()  >= cr.value &&
                             isect->height() >= cr.value)
                    ok = true;
            });
            if (!ok) {
                DRCViolation v;
                v.ruleType = "overlap";
                v.layer    = cr.inner;
                v.location = ti->bbox;
                std::ostringstream s;
                s << "[overlap/incr] " << layerName(cr.inner)
                  << " " << ti->bbox.str()
                  << " insufficient overlap with " << layerName(cr.outer);
                v.message = s.str();
                out.push_back(v);
            }
        });
    }

    void checkIntersectionInRegion(const CrossLayerRule& cr, const Rect& region, int zLevel,
                                   std::vector<DRCViolation>& out) const {
        plane(cr.inner, zLevel).forEachInRect(region, [&](const Tile* ti) {
            if (!ti->solid) return;
            bool found = false;
            plane(cr.outer, zLevel).forEachSolid([&](const Tile* to) {
                if (ti->bbox.overlaps(to->bbox)) found = true;
            });
            if (!found) {
                DRCViolation v;
                v.ruleType = "intersection";
                v.layer    = cr.inner;
                v.location = ti->bbox;
                std::ostringstream s;
                s << "[intersection/incr] " << layerName(cr.inner)
                  << " " << ti->bbox.str()
                  << " does not intersect any " << layerName(cr.outer);
                v.message = s.str();
                out.push_back(v);
            }
        });
    }

    // =========================================================================
    //  Utilities
    // =========================================================================

    /// Minimum separation between two non-overlapping rectangles (0 if touching).
    static Coord rectDistance(const Rect& a, const Rect& b) {
        Coord dx = 0, dy = 0;
        if      (a.x2 <= b.x1) dx = b.x1 - a.x2;
        else if (b.x2 <= a.x1) dx = a.x1 - b.x2;
        if      (a.y2 <= b.y1) dy = b.y1 - a.y2;
        else if (b.y2 <= a.y1) dy = a.y1 - b.y2;
        if (dx > 0 && dy > 0) return std::min(dx, dy); // corner distance
        return std::max(dx, dy);                        // edge distance
    }
};

// =============================================================================
//  Convenience: print violations to a stream
// =============================================================================
inline void printViolations(const std::vector<DRCViolation>& vs,
                             std::ostream& os = std::cout) {
    if (vs.empty()) {
        os << "DRC CLEAN – no violations.\n";
        return;
    }
    os << "DRC found " << vs.size() << " violation(s):\n";
    for (auto& v : vs)
        os << "  " << v.message << "\n";
}

// =============================================================================
//  Quick self-test (compile with -DDRC_SELFTEST and run)
// =============================================================================
#ifdef DRC_SELFTEST
#include <iostream>
int main() {
    Layout layout;

    std::cout << "=== Test 1: min-width violation on Metal1 ===\n";
    // Metal1 minWidth = 3; insert a 2-wide rectangle
    layout.insertRect(LayerType::Metal1, {0, 0, 2, 10});
    printViolations(layout.runDRC());

    std::cout << "\n=== Test 2: min-space violation on Metal1 ===\n";
    Layout l2;
    l2.insertRect(LayerType::Metal1, {0,  0, 10, 10});
    l2.insertRect(LayerType::Metal1, {11, 0, 21, 10}); // gap = 1, need 3
    printViolations(l2.runDRC());

    std::cout << "\n=== Test 3: min-area violation on Metal1 ===\n";
    Layout l3;
    l3.insertRect(LayerType::Metal1, {0, 0, 3, 2}); // area=6 < 9
    printViolations(l3.runDRC());

    std::cout << "\n=== Test 4: enclosure – Via not enclosed by Metal1 ===\n";
    Layout l4;
    l4.insertRect(LayerType::Via,    {10, 10, 14, 14});
    l4.insertRect(LayerType::Metal1, {10, 10, 14, 14}); // no margin
    printViolations(l4.runDRC());

    std::cout << "\n=== Test 5: enclosure – Via properly enclosed by Metal1 ===\n";
    Layout l5;
    l5.insertRect(LayerType::Via,    {10, 10, 14, 14});
    l5.insertRect(LayerType::Metal1, { 9,  9, 15, 15}); // 1-lambda margin
    // Also add valid Poly/Active/NWell/PWell to avoid other cross-layer noise
    l5.insertRect(LayerType::Active, {0, 0, 50, 50});
    l5.insertRect(LayerType::NWell,  {0, 0, 50, 50});
    l5.insertRect(LayerType::PWell,  {0, 0, 50, 50});
    // Contact enclosed by Active
    l5.insertRect(LayerType::Contact,{5, 5, 9, 9});
    l5.insertRect(LayerType::Active, {4, 4, 10, 10});
    l5.insertRect(LayerType::Poly,   {4, 4, 10, 10});
    printViolations(l5.runDRC());

    std::cout << "\n=== Test 6: incremental DRC ===\n";
    Layout l6;
    l6.insertRect(LayerType::Poly, {0, 0, 20, 10});
    Rect newRect = {21, 0, 41, 10}; // gap = 1, need 2
    l6.insertRect(LayerType::Poly, newRect);
    auto incr = l6.runIncrementalDRC(LayerType::Poly, newRect);
    std::cout << "[incremental] ";
    printViolations(incr);

    std::cout << "\n=== Test 7: tile merge / stitch dump ===\n";
    Layout l7;
    l7.insertRect(LayerType::Active, {0,  0, 10, 10});
    l7.insertRect(LayerType::Active, {10, 0, 20, 10}); // should merge
    auto& pl = l7.plane(LayerType::Active);
    int solidCount = 0;
    pl.forEachSolid([&](const Tile* t){
        std::cout << "  solid tile: " << t->bbox.str()
                  << "  rt=" << (t->rt ? "yes" : "null")
                  << "  lt=" << (t->lt ? "yes" : "null") << "\n";
        ++solidCount;
    });
    std::cout << "  total solid tiles after merge: " << solidCount
              << "  (expected 1)\n";

    return 0;
}
#endif // DRC_SELFTEST

} // namespace DRC


// =============================================================================
//  incremental_drc_naive.hpp  —  Minimal Naive Incremental DRC

//  Design:
//    • No plane of tiles
//    • No corner stitching
//    • Just a list of rectangles per layer
//    • Naive O(N^2) checks for min-space / cross-layer rules
//    • Full DRC + dirty-region incremental DRC

//  C++17
// =============================================================================
// #pragma once

// #include <algorithm>
// #include <cassert>
// #include <cstdint>
// #include <functional>
// #include <iostream>
// #include <limits>
// #include <memory>
// #include <optional>
// #include <sstream>
// #include <string>
// #include <vector>

// namespace DRC {

// // =============================================================================
// //  Layer types
// // =============================================================================
// enum class LayerType : uint8_t {
//     Active  = 0,
//     Poly    = 1,
//     NWell   = 2,
//     PWell   = 3,
//     Metal1  = 4,
//     Via     = 5,
//     Contact = 6,
//     _Count  = 7
// };

// inline const char* layerName(LayerType lt) {
//     switch (lt) {
//         case LayerType::Active:  return "Active";
//         case LayerType::Poly:    return "Poly";
//         case LayerType::NWell:   return "NWell";
//         case LayerType::PWell:   return "PWell";
//         case LayerType::Metal1:  return "Metal1";
//         case LayerType::Via:     return "Via";
//         case LayerType::Contact: return "Contact";
//         default:                 return "Unknown";
//     }
// }

// // =============================================================================
// //  Coordinate type
// // =============================================================================
// using Coord = int32_t;

// // =============================================================================
// //  Rect — axis-aligned rectangle [x1,x2) × [y1,y2)
// // =============================================================================
// struct Rect {
//     Coord x1, y1, x2, y2;

//     Rect() : x1(0), y1(0), x2(0), y2(0) {}
//     Rect(Coord ax1, Coord ay1, Coord ax2, Coord ay2)
//         : x1(ax1), y1(ay1), x2(ax2), y2(ay2) {
//         assert(x1 <= x2 && y1 <= y2);
//     }

//     bool valid() const { return x1 < x2 && y1 < y2; }
//     Coord width() const { return x2 - x1; }
//     Coord height() const { return y2 - y1; }
//     int64_t area() const { return int64_t(width()) * int64_t(height()); }

//     bool contains(Coord x, Coord y) const {
//         return x >= x1 && x < x2 && y >= y1 && y < y2;
//     }

//     bool contains(const Rect& o) const {
//         return x1 <= o.x1 && y1 <= o.y1 && x2 >= o.x2 && y2 >= o.y2;
//     }

//     bool overlaps(const Rect& o) const {
//         return x1 < o.x2 && x2 > o.x1 && y1 < o.y2 && y2 > o.y1;
//     }

//     std::optional<Rect> intersect(const Rect& o) const {
//         Coord nx1 = std::max(x1, o.x1);
//         Coord ny1 = std::max(y1, o.y1);
//         Coord nx2 = std::min(x2, o.x2);
//         Coord ny2 = std::min(y2, o.y2);
//         if (nx1 >= nx2 || ny1 >= ny2) return std::nullopt;
//         return Rect{nx1, ny1, nx2, ny2};
//     }

//     Rect unite(const Rect& o) const {
//         return Rect{
//             std::min(x1, o.x1),
//             std::min(y1, o.y1),
//             std::max(x2, o.x2),
//             std::max(y2, o.y2)
//         };
//     }

//     std::string str() const {
//         std::ostringstream s;
//         s << "(" << x1 << "," << y1 << ")-(" << x2 << "," << y2 << ")";
//         return s.str();
//     }

//     bool operator==(const Rect& o) const {
//         return x1 == o.x1 && y1 == o.y1 && x2 == o.x2 && y2 == o.y2;
//     }
// };

// // =============================================================================
// //  Shape — one rectangle on one layer
// // =============================================================================
// struct Shape {
//     LayerType layer;
//     Rect bbox;
//     bool alive = true;
// };

// // =============================================================================
// //  DRC rules
// // =============================================================================
// struct LayerRules {
//     LayerType layer;
//     Coord minWidth = 0;
//     Coord minSpace = 0;
//     int64_t minArea = 0;
// };

// struct CrossLayerRule {
//     enum class Kind { Enclosure, Overlap, Intersection } kind;
//     LayerType inner;
//     LayerType outer;
//     Coord value; // enclosure margin or minimum overlap size
// };

// // Default rules
// inline std::vector<LayerRules> defaultLayerRules() {
//     return {
//         { LayerType::Active,  3,  3,   9 },
//         { LayerType::Poly,    2,  2,   4 },
//         { LayerType::NWell,   4,  4,  16 },
//         { LayerType::PWell,   4,  4,  16 },
//         { LayerType::Metal1,  3,  3,   9 },
//         { LayerType::Via,     2,  2,   4 },
//         { LayerType::Contact, 2,  2,   4 },
//     };
// }

// inline std::vector<CrossLayerRule> defaultCrossLayerRules() {
//     using K = CrossLayerRule::Kind;
//     return {
//         { K::Enclosure,    LayerType::Via,     LayerType::Metal1,  1 },
//         { K::Enclosure,    LayerType::Contact, LayerType::Active,  1 },
//         { K::Enclosure,    LayerType::Contact, LayerType::Poly,    1 },
//         { K::Overlap,      LayerType::Active,  LayerType::NWell,   2 },
//         { K::Overlap,      LayerType::Active,  LayerType::PWell,   2 },
//         { K::Intersection, LayerType::Poly,    LayerType::Active,  0 },
//     };
// }

// // =============================================================================
// //  DRC violation
// // =============================================================================
// struct DRCViolation {
//     std::string ruleType;
//     LayerType layer;
//     Rect location;
//     std::string message;
// };

// // =============================================================================
// //  Layout — list-based geometry storage + naive DRC
// // =============================================================================
// class Layout {
// public:
//     Layout()
//         : layerRules_(defaultLayerRules()),
//           crossLayerRules_(defaultCrossLayerRules()) {}

//     // -------------------------------------------------------------------------
//     // Geometry operations
//     // -------------------------------------------------------------------------
//     void insertRect(LayerType l, const Rect& r) {
//         assert(r.valid());
//         shapes_[(int)l].push_back(Shape{l, r, true});
//     }

//     void removeRect(LayerType l, const Rect& r) {
//         auto& vec = shapes_[(int)l];
//         for (auto it = vec.begin(); it != vec.end(); ++it) {
//             if (it->alive && it->bbox == r) {
//                 it->alive = false;
//                 vec.erase(it);
//                 return;
//             }
//         }
//     }

//     const std::vector<Shape>& shapes(LayerType l) const {
//         return shapes_[(int)l];
//     }

//     // -------------------------------------------------------------------------
//     // Rule accessors
//     // -------------------------------------------------------------------------
//     void setLayerRules(const std::vector<LayerRules>& rules) {
//         layerRules_ = rules;
//     }

//     void setCrossLayerRules(const std::vector<CrossLayerRule>& rules) {
//         crossLayerRules_ = rules;
//     }

//     // -------------------------------------------------------------------------
//     // Full DRC
//     // -------------------------------------------------------------------------
//     std::vector<DRCViolation> runDRC() const {
//         std::vector<DRCViolation> out;

//         for (const auto& lr : layerRules_) {
//             checkMinWidth(lr, out);
//             checkMinSpace(lr, out);
//             checkMinArea(lr, out);
//         }

//         for (const auto& cr : crossLayerRules_) {
//             switch (cr.kind) {
//                 case CrossLayerRule::Kind::Enclosure:
//                     checkEnclosure(cr, out);
//                     break;
//                 case CrossLayerRule::Kind::Overlap:
//                     checkOverlap(cr, out);
//                     break;
//                 case CrossLayerRule::Kind::Intersection:
//                     checkIntersection(cr, out);
//                     break;
//             }
//         }

//         return out;
//     }

//     // -------------------------------------------------------------------------
//     // Naive incremental DRC:
//     // check only shapes that intersect expanded dirty region
//     // -------------------------------------------------------------------------
//     std::vector<DRCViolation> runIncrementalDRC(LayerType changedLayer,
//                                                 const Rect& dirtyRegion) const {
//         std::vector<DRCViolation> out;

//         Coord margin = 0;
//         for (const auto& lr : layerRules_) {
//             margin = std::max(margin, std::max(lr.minWidth, lr.minSpace));
//         }
//         for (const auto& cr : crossLayerRules_) {
//             margin = std::max(margin, cr.value);
//         }

//         Rect expanded{
//             dirtyRegion.x1 - margin,
//             dirtyRegion.y1 - margin,
//             dirtyRegion.x2 + margin,
//             dirtyRegion.y2 + margin
//         };

//         for (const auto& lr : layerRules_) {
//             if (lr.layer != changedLayer) continue;
//             checkMinWidthInRegion(lr, expanded, out);
//             checkMinSpaceInRegion(lr, expanded, out);
//             checkMinAreaInRegion(lr, expanded, out);
//         }

//         for (const auto& cr : crossLayerRules_) {
//             if (cr.inner != changedLayer && cr.outer != changedLayer) continue;
//             switch (cr.kind) {
//                 case CrossLayerRule::Kind::Enclosure:
//                     checkEnclosureInRegion(cr, expanded, out);
//                     break;
//                 case CrossLayerRule::Kind::Overlap:
//                     checkOverlapInRegion(cr, expanded, out);
//                     break;
//                 case CrossLayerRule::Kind::Intersection:
//                     checkIntersectionInRegion(cr, expanded, out);
//                     break;
//             }
//         }

//         return out;
//     }

// private:
//     std::vector<Shape> shapes_[(int)LayerType::_Count];
//     std::vector<LayerRules> layerRules_;
//     std::vector<CrossLayerRule> crossLayerRules_;

//     // -------------------------------------------------------------------------
//     // Utilities
//     // -------------------------------------------------------------------------
//     static Coord rectDistance(const Rect& a, const Rect& b) {
//         Coord dx = 0, dy = 0;

//         if (a.x2 <= b.x1) dx = b.x1 - a.x2;
//         else if (b.x2 <= a.x1) dx = a.x1 - b.x2;

//         if (a.y2 <= b.y1) dy = b.y1 - a.y2;
//         else if (b.y2 <= a.y1) dy = a.y1 - b.y2;

//         if (dx > 0 && dy > 0) return std::min(dx, dy);
//         return std::max(dx, dy);
//     }

//     template <typename Fn>
//     void forEachShape(LayerType l, Fn&& fn) const {
//         for (const auto& s : shapes_[(int)l]) {
//             if (s.alive) fn(s);
//         }
//     }

//     template <typename Fn>
//     void forEachShapeInRect(LayerType l, const Rect& q, Fn&& fn) const {
//         for (const auto& s : shapes_[(int)l]) {
//             if (s.alive && s.bbox.overlaps(q)) fn(s);
//         }
//     }

//     // -------------------------------------------------------------------------
//     // Full DRC checks
//     // -------------------------------------------------------------------------
//     void checkMinWidth(const LayerRules& lr,
//                        std::vector<DRCViolation>& out) const {
//         forEachShape(lr.layer, [&](const Shape& s) {
//             if (s.bbox.width() < lr.minWidth || s.bbox.height() < lr.minWidth) {
//                 DRCViolation v;
//                 v.ruleType = "min-width";
//                 v.layer = lr.layer;
//                 v.location = s.bbox;
//                 std::ostringstream ss;
//                 ss << "[min-width] Layer=" << layerName(lr.layer)
//                    << " tile=" << s.bbox.str()
//                    << " width=" << std::min(s.bbox.width(), s.bbox.height())
//                    << " < " << lr.minWidth;
//                 v.message = ss.str();
//                 out.push_back(v);
//             }
//         });
//     }

//     void checkMinSpace(const LayerRules& lr,
//                        std::vector<DRCViolation>& out) const {
//         std::vector<const Shape*> solids;
//         forEachShape(lr.layer, [&](const Shape& s) {
//             solids.push_back(&s);
//         });

//         for (size_t i = 0; i < solids.size(); ++i) {
//             for (size_t j = i + 1; j < solids.size(); ++j) {
//                 Coord dist = rectDistance(solids[i]->bbox, solids[j]->bbox);
//                 if (dist > 0 && dist < lr.minSpace) {
//                     DRCViolation v;
//                     v.ruleType = "min-space";
//                     v.layer = lr.layer;
//                     v.location = solids[i]->bbox.unite(solids[j]->bbox);
//                     std::ostringstream ss;
//                     ss << "[min-space] Layer=" << layerName(lr.layer)
//                        << " between " << solids[i]->bbox.str()
//                        << " and " << solids[j]->bbox.str()
//                        << " dist=" << dist << " < " << lr.minSpace;
//                     v.message = ss.str();
//                     out.push_back(v);
//                 }
//             }
//         }
//     }

//     void checkMinArea(const LayerRules& lr,
//                       std::vector<DRCViolation>& out) const {
//         forEachShape(lr.layer, [&](const Shape& s) {
//             if (s.bbox.area() < lr.minArea) {
//                 DRCViolation v;
//                 v.ruleType = "min-area";
//                 v.layer = lr.layer;
//                 v.location = s.bbox;
//                 std::ostringstream ss;
//                 ss << "[min-area] Layer=" << layerName(lr.layer)
//                    << " tile=" << s.bbox.str()
//                    << " area=" << s.bbox.area()
//                    << " < " << lr.minArea;
//                 v.message = ss.str();
//                 out.push_back(v);
//             }
//         });
//     }

//     void checkIntersection(const CrossLayerRule& cr,
//                            std::vector<DRCViolation>& out) const {
//         forEachShape(cr.inner, [&](const Shape& innerShape) {
//             bool found = false;
//             forEachShape(cr.outer, [&](const Shape& outerShape) {
//                 if (innerShape.bbox.overlaps(outerShape.bbox)) {
//                     found = true;
//                 }
//             });

//             if (!found) {
//                 DRCViolation v;
//                 v.ruleType = "intersection";
//                 v.layer = cr.inner;
//                 v.location = innerShape.bbox;
//                 std::ostringstream ss;
//                 ss << "[intersection] " << layerName(cr.inner)
//                    << " tile " << innerShape.bbox.str()
//                    << " does not intersect any " << layerName(cr.outer);
//                 v.message = ss.str();
//                 out.push_back(v);
//             }
//         });
//     }

//     void checkEnclosure(const CrossLayerRule& cr,
//                         std::vector<DRCViolation>& out) const {
//         forEachShape(cr.inner, [&](const Shape& innerShape) {
//             bool enclosed = false;
//             forEachShape(cr.outer, [&](const Shape& outerShape) {
//                 if (outerShape.bbox.x1 <= innerShape.bbox.x1 - cr.value &&
//                     outerShape.bbox.y1 <= innerShape.bbox.y1 - cr.value &&
//                     outerShape.bbox.x2 >= innerShape.bbox.x2 + cr.value &&
//                     outerShape.bbox.y2 >= innerShape.bbox.y2 + cr.value) {
//                     enclosed = true;
//                 }
//             });

//             if (!enclosed) {
//                 DRCViolation v;
//                 v.ruleType = "enclosure";
//                 v.layer = cr.inner;
//                 v.location = innerShape.bbox;
//                 std::ostringstream ss;
//                 ss << "[enclosure] " << layerName(cr.inner)
//                    << " tile " << innerShape.bbox.str()
//                    << " not enclosed by " << layerName(cr.outer)
//                    << " with margin=" << cr.value;
//                 v.message = ss.str();
//                 out.push_back(v);
//             }
//         });
//     }

//     void checkOverlap(const CrossLayerRule& cr,
//                       std::vector<DRCViolation>& out) const {
//         forEachShape(cr.inner, [&](const Shape& innerShape) {
//             bool ok = false;
//             forEachShape(cr.outer, [&](const Shape& outerShape) {
//                 auto isect = innerShape.bbox.intersect(outerShape.bbox);
//                 if (isect && isect->width() >= cr.value &&
//                             isect->height() >= cr.value) {
//                     ok = true;
//                 }
//             });

//             if (!ok) {
//                 DRCViolation v;
//                 v.ruleType = "overlap";
//                 v.layer = cr.inner;
//                 v.location = innerShape.bbox;
//                 std::ostringstream ss;
//                 ss << "[overlap] " << layerName(cr.inner)
//                    << " tile " << innerShape.bbox.str()
//                    << " insufficient overlap with " << layerName(cr.outer)
//                    << " (need " << cr.value << ")";
//                 v.message = ss.str();
//                 out.push_back(v);
//             }
//         });
//     }

//     // -------------------------------------------------------------------------
//     // Incremental region checks
//     // -------------------------------------------------------------------------
//     void checkMinWidthInRegion(const LayerRules& lr, const Rect& region,
//                                std::vector<DRCViolation>& out) const {
//         forEachShapeInRect(lr.layer, region, [&](const Shape& s) {
//             if (s.bbox.width() < lr.minWidth || s.bbox.height() < lr.minWidth) {
//                 DRCViolation v;
//                 v.ruleType = "min-width";
//                 v.layer = lr.layer;
//                 v.location = s.bbox;
//                 std::ostringstream ss;
//                 ss << "[min-width/incr] Layer=" << layerName(lr.layer)
//                    << " tile=" << s.bbox.str()
//                    << " width=" << std::min(s.bbox.width(), s.bbox.height())
//                    << " < " << lr.minWidth;
//                 v.message = ss.str();
//                 out.push_back(v);
//             }
//         });
//     }

//     void checkMinSpaceInRegion(const LayerRules& lr, const Rect& region,
//                                std::vector<DRCViolation>& out) const {
//         std::vector<const Shape*> solids;
//         forEachShapeInRect(lr.layer, region, [&](const Shape& s) {
//             solids.push_back(&s);
//         });

//         for (size_t i = 0; i < solids.size(); ++i) {
//             for (size_t j = i + 1; j < solids.size(); ++j) {
//                 Coord dist = rectDistance(solids[i]->bbox, solids[j]->bbox);
//                 if (dist > 0 && dist < lr.minSpace) {
//                     DRCViolation v;
//                     v.ruleType = "min-space";
//                     v.layer = lr.layer;
//                     v.location = solids[i]->bbox.unite(solids[j]->bbox);
//                     std::ostringstream ss;
//                     ss << "[min-space/incr] Layer=" << layerName(lr.layer)
//                        << " dist=" << dist << " < " << lr.minSpace;
//                     v.message = ss.str();
//                     out.push_back(v);
//                 }
//             }
//         }
//     }

//     void checkMinAreaInRegion(const LayerRules& lr, const Rect& region,
//                               std::vector<DRCViolation>& out) const {
//         forEachShapeInRect(lr.layer, region, [&](const Shape& s) {
//             if (s.bbox.area() < lr.minArea) {
//                 DRCViolation v;
//                 v.ruleType = "min-area";
//                 v.layer = lr.layer;
//                 v.location = s.bbox;
//                 std::ostringstream ss;
//                 ss << "[min-area/incr] Layer=" << layerName(lr.layer)
//                    << " tile=" << s.bbox.str()
//                    << " area=" << s.bbox.area()
//                    << " < " << lr.minArea;
//                 v.message = ss.str();
//                 out.push_back(v);
//             }
//         });
//     }

//     void checkIntersectionInRegion(const CrossLayerRule& cr, const Rect& region,
//                                    std::vector<DRCViolation>& out) const {
//         forEachShapeInRect(cr.inner, region, [&](const Shape& innerShape) {
//             bool found = false;
//             forEachShape(cr.outer, [&](const Shape& outerShape) {
//                 if (innerShape.bbox.overlaps(outerShape.bbox)) {
//                     found = true;
//                 }
//             });

//             if (!found) {
//                 DRCViolation v;
//                 v.ruleType = "intersection";
//                 v.layer = cr.inner;
//                 v.location = innerShape.bbox;
//                 std::ostringstream ss;
//                 ss << "[intersection/incr] " << layerName(cr.inner)
//                    << " " << innerShape.bbox.str()
//                    << " does not intersect any " << layerName(cr.outer);
//                 v.message = ss.str();
//                 out.push_back(v);
//             }
//         });
//     }

//     void checkEnclosureInRegion(const CrossLayerRule& cr, const Rect& region,
//                                 std::vector<DRCViolation>& out) const {
//         forEachShapeInRect(cr.inner, region, [&](const Shape& innerShape) {
//             bool enclosed = false;
//             forEachShape(cr.outer, [&](const Shape& outerShape) {
//                 if (outerShape.bbox.x1 <= innerShape.bbox.x1 - cr.value &&
//                     outerShape.bbox.y1 <= innerShape.bbox.y1 - cr.value &&
//                     outerShape.bbox.x2 >= innerShape.bbox.x2 + cr.value &&
//                     outerShape.bbox.y2 >= innerShape.bbox.y2 + cr.value) {
//                     enclosed = true;
//                 }
//             });

//             if (!enclosed) {
//                 DRCViolation v;
//                 v.ruleType = "enclosure";
//                 v.layer = cr.inner;
//                 v.location = innerShape.bbox;
//                 std::ostringstream ss;
//                 ss << "[enclosure/incr] " << layerName(cr.inner)
//                    << " " << innerShape.bbox.str()
//                    << " not enclosed by " << layerName(cr.outer)
//                    << " margin=" << cr.value;
//                 v.message = ss.str();
//                 out.push_back(v);
//             }
//         });
//     }

//     void checkOverlapInRegion(const CrossLayerRule& cr, const Rect& region,
//                               std::vector<DRCViolation>& out) const {
//         forEachShapeInRect(cr.inner, region, [&](const Shape& innerShape) {
//             bool ok = false;
//             forEachShape(cr.outer, [&](const Shape& outerShape) {
//                 auto isect = innerShape.bbox.intersect(outerShape.bbox);
//                 if (isect && isect->width() >= cr.value &&
//                             isect->height() >= cr.value) {
//                     ok = true;
//                 }
//             });

//             if (!ok) {
//                 DRCViolation v;
//                 v.ruleType = "overlap";
//                 v.layer = cr.inner;
//                 v.location = innerShape.bbox;
//                 std::ostringstream ss;
//                 ss << "[overlap/incr] " << layerName(cr.inner)
//                    << " " << innerShape.bbox.str()
//                    << " insufficient overlap with " << layerName(cr.outer);
//                 v.message = ss.str();
//                 out.push_back(v);
//             }
//         });
//     }
// };

// // =============================================================================
// //  Convenience: print violations
// // =============================================================================
// inline void printViolations(const std::vector<DRCViolation>& vs,
//                             std::ostream& os = std::cout) {
//     if (vs.empty()) {
//         os << "DRC CLEAN – no violations.\n";
//         return;
//     }

//     os << "DRC found " << vs.size() << " violation(s):\n";
//     for (const auto& v : vs) {
//         os << "  " << v.message << "\n";
//     }
// }

// } // namespace DRC

// // =============================================================================
// //  Self-test
// // =============================================================================
// #ifdef DRC_SELFTEST
// #include <iostream>

// int main() {
//     using namespace DRC;

//     std::cout << "=== Test 1: min-width violation on Metal1 ===\n";
//     Layout layout1;
//     layout1.insertRect(LayerType::Metal1, {0, 0, 2, 10});
//     printViolations(layout1.runDRC());

//     std::cout << "\n=== Test 2: min-space violation on Metal1 ===\n";
//     Layout layout2;
//     layout2.insertRect(LayerType::Metal1, {0,  0, 10, 10});
//     layout2.insertRect(LayerType::Metal1, {11, 0, 21, 10});
//     printViolations(layout2.runDRC());

//     std::cout << "\n=== Test 3: min-area violation on Metal1 ===\n";
//     Layout layout3;
//     layout3.insertRect(LayerType::Metal1, {0, 0, 3, 2});
//     printViolations(layout3.runDRC());

//     std::cout << "\n=== Test 4: enclosure – Via not enclosed by Metal1 ===\n";
//     Layout layout4;
//     layout4.insertRect(LayerType::Via,    {10, 10, 14, 14});
//     layout4.insertRect(LayerType::Metal1, {10, 10, 14, 14});
//     printViolations(layout4.runDRC());

//     std::cout << "\n=== Test 5: incremental DRC ===\n";
//     Layout layout5;
//     layout5.insertRect(LayerType::Poly, {0, 0, 20, 10});
//     Rect newRect = {21, 0, 41, 10};
//     layout5.insertRect(LayerType::Poly, newRect);
//     auto incr = layout5.runIncrementalDRC(LayerType::Poly, newRect);
//     printViolations(incr);

//     return 0;
// }
// #endif
