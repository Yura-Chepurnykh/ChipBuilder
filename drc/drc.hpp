#ifndef DRC_HPP
#define DRC_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------
// Magic-style DRC skeleton in C++17
// Plane of Tiles + neighbor stitching + basic DRC checks.
//
// This is a practical, self-contained implementation for orthogonal geometry.
// It is intentionally conservative and easy to extend.
// -----------------------------------------------------------------------------

namespace drc {

using i64 = long long;

struct Rect {
    i64 x1 = 0, y1 = 0, x2 = 0, y2 = 0; // half-open [x1,y1) .. [x2,y2)

    bool valid() const { return x2 > x1 && y2 > y1; }
    i64 width() const { return x2 - x1; }
    i64 height() const { return y2 - y1; }
    i64 area() const {
        if (!valid()) return 0;
        return width() * height();
    }

    bool intersects(const Rect& o) const {
        return !(x2 <= o.x1 || o.x2 <= x1 || y2 <= o.y1 || o.y2 <= y1);
    }

    Rect intersection(const Rect& o) const {
        if (!intersects(o)) return {};
        return {std::max(x1, o.x1), std::max(y1, o.y1), std::min(x2, o.x2), std::min(y2, o.y2)};
    }

    bool contains(const Rect& o) const {
        return x1 <= o.x1 && y1 <= o.y1 && x2 >= o.x2 && y2 >= o.y2;
    }

    Rect expand(i64 d) const {
        return {x1 - d, y1 - d, x2 + d, y2 + d};
    }

    static i64 gap1D(i64 a1, i64 a2, i64 b1, i64 b2) {
        if (a2 < b1) return b1 - a2;
        if (b2 < a1) return a1 - b2;
        return 0;
    }

    i64 manhattanDistanceTo(const Rect& o) const {
        const i64 dx = gap1D(x1, x2, o.x1, o.x2);
        const i64 dy = gap1D(y1, y2, o.y1, o.y2);
        if (dx == 0) return dy;
        if (dy == 0) return dx;
        // For orthogonal layout DRC, diagonal separation is not a spacing violation
        // unless you explicitly want Euclidean distance. We return max(dx,dy) here.
        return std::max(dx, dy);
    }

    std::string str() const {
        std::ostringstream oss;
        oss << "(" << x1 << "," << y1 << ")-(" << x2 << "," << y2 << ")";
        return oss.str();
    }
};

enum class LayerKind : int {
    Space = 0,
    Nwell,
    Pwell,
    Active,
    Contact,
    Via1,
    Poly,
    Metal1,
};

static const char* layerName(LayerKind k) {
    switch (k) {
        case LayerKind::Space:   return "SPACE";
        case LayerKind::Nwell:   return "Nwell";
        case LayerKind::Pwell:   return "Pwell";
        case LayerKind::Active:  return "Active";
        case LayerKind::Contact: return "Contact";
        case LayerKind::Via1:    return "Via1";
        case LayerKind::Poly:    return "Poly";
        case LayerKind::Metal1:  return "Metal1";
    }
    return "Unknown";
}

struct Tile {
    Rect box;
    LayerKind kind = LayerKind::Space;
    int featureId = -1; // different logical features within the same plane
    int id = -1;        // stable only after rebuild

    // Corner-stitch-style neighbor links (recomputed after every normalize())
    int left = -1;
    int right = -1;
    int top = -1;
    int bottom = -1;
};

struct Violation {
    std::string rule;
    std::string layerA;
    std::string layerB;
    int featureA = -1;
    int featureB = -1;
    Rect a{};
    Rect b{};
    i64 measured = 0;
    std::string message;

    std::string str() const {
        std::ostringstream oss;
        oss << "[" << rule << "] ";
        if (!layerA.empty()) oss << layerA;
        if (!layerB.empty()) oss << " vs " << layerB;
        if (featureA >= 0) oss << " featA=" << featureA;
        if (featureB >= 0) oss << " featB=" << featureB;
        if (a.valid()) oss << " A=" << a.str();
        if (b.valid()) oss << " B=" << b.str();
        if (measured) oss << " measured=" << measured;
        if (!message.empty()) oss << " : " << message;
        return oss.str();
    }
};

struct RuleSet {
    std::map<LayerKind, i64> minWidth;
    std::map<LayerKind, i64> minSpace;
    std::map<LayerKind, i64> minArea;

    std::map<std::pair<LayerKind, LayerKind>, i64> enclosure; // inner, outer
    std::set<std::pair<LayerKind, LayerKind>> forbiddenIntersection;

    i64 getMinWidth(LayerKind k) const {
        auto it = minWidth.find(k);
        return it == minWidth.end() ? 0 : it->second;
    }
    i64 getMinSpace(LayerKind k) const {
        auto it = minSpace.find(k);
        return it == minSpace.end() ? 0 : it->second;
    }
    i64 getMinArea(LayerKind k) const {
        auto it = minArea.find(k);
        return it == minArea.end() ? 0 : it->second;
    }
    i64 getEnclosure(LayerKind inner, LayerKind outer) const {
        auto it = enclosure.find({inner, outer});
        return it == enclosure.end() ? 0 : it->second;
    }
    bool isForbidden(LayerKind a, LayerKind b) const {
        return forbiddenIntersection.count({a, b}) || forbiddenIntersection.count({b, a});
    }
};

class Plane {
public:
    Plane() = default;

    Plane(Rect bbox, LayerKind kind)
        : bbox_(bbox), defaultKind_(kind) {
        tiles_.push_back(Tile{bbox_, LayerKind::Space, -1, 0, -1, -1, -1, -1});
        normalize();
    }

    const std::vector<Tile>& tiles() const { return tiles_; }
    const Rect& bbox() const { return bbox_; }

    // Paint a rectangle into this plane. The plane is partitioned into tiles.
    // Any existing tiles intersecting the paint rectangle are split and replaced.
    void paint(const Rect& r, LayerKind kind, int featureId) {
        if (!r.valid()) return;
        if (!bbox_.contains(r)) {
            throw std::runtime_error("paint() rect must be inside plane bbox");
        }

        std::vector<Tile> next;
        next.reserve(tiles_.size() + 4);

        for (const auto& t : tiles_) {
            if (!t.box.intersects(r)) {
                next.push_back(t);
                continue;
            }

            // Split existing tile t by the intersection region i = t ∩ r.
            const Rect i = t.box.intersection(r);
            if (!i.valid()) {
                next.push_back(t);
                continue;
            }

            // Left strip
            if (t.box.x1 < i.x1) {
                next.push_back(makeTile({t.box.x1, t.box.y1, i.x1, t.box.y2}, t.kind, t.featureId));
            }
            // Right strip
            if (i.x2 < t.box.x2) {
                next.push_back(makeTile({i.x2, t.box.y1, t.box.x2, t.box.y2}, t.kind, t.featureId));
            }
            // Bottom strip (between i.x1..i.x2)
            if (t.box.y1 < i.y1) {
                next.push_back(makeTile({i.x1, t.box.y1, i.x2, i.y1}, t.kind, t.featureId));
            }
            // Top strip (between i.x1..i.x2)
            if (i.y2 < t.box.y2) {
                next.push_back(makeTile({i.x1, i.y2, i.x2, t.box.y2}, t.kind, t.featureId));
            }
        }

        // Add painted rectangle as a new tile.
        next.push_back(makeTile(r, kind, featureId));

        tiles_.swap(next);
        normalize();
    }

    // Find tile containing a point (linear scan; you can replace with spatial hinting)
    int locate(i64 x, i64 y) const {
        for (int i = 0; i < (int)tiles_.size(); ++i) {
            const auto& t = tiles_[i];
            if (x >= t.box.x1 && x < t.box.x2 && y >= t.box.y1 && y < t.box.y2) return i;
        }
        return -1;
    }

    // Simple touch-based neighbor list. Corner-stitching in Magic keeps links stable;
    // here we rebuild them after every normalize().
    void rebuildStitching() {
        for (auto& t : tiles_) {
            t.left = t.right = t.top = t.bottom = -1;
            t.id = -1;
        }

        for (int i = 0; i < (int)tiles_.size(); ++i) {
            tiles_[i].id = i;
        }

        auto overlap1D = [](i64 a1, i64 a2, i64 b1, i64 b2) {
            return std::max(a1, b1) < std::min(a2, b2);
        };

        for (int i = 0; i < (int)tiles_.size(); ++i) {
            for (int j = i + 1; j < (int)tiles_.size(); ++j) {
                const auto& A = tiles_[i].box;
                const auto& B = tiles_[j].box;

                if (A.x2 == B.x1 && overlap1D(A.y1, A.y2, B.y1, B.y2)) {
                    tiles_[i].right = j;
                    tiles_[j].left = i;
                } else if (B.x2 == A.x1 && overlap1D(A.y1, A.y2, B.y1, B.y2)) {
                    tiles_[j].right = i;
                    tiles_[i].left = j;
                }

                if (A.y2 == B.y1 && overlap1D(A.x1, A.x2, B.x1, B.x2)) {
                    tiles_[i].top = j;
                    tiles_[j].bottom = i;
                } else if (B.y2 == A.y1 && overlap1D(A.x1, A.x2, B.x1, B.x2)) {
                    tiles_[j].top = i;
                    tiles_[i].bottom = j;
                }
            }
        }
    }

    // Merge adjacent tiles of same kind and feature if they form a rectangle.
    // Repeats until stable.
    void normalize() {
        bool changed;
        do {
            changed = false;
            std::vector<bool> dead(tiles_.size(), false);

            for (int i = 0; i < (int)tiles_.size() && !changed; ++i) {
                if (dead[i]) continue;
                for (int j = i + 1; j < (int)tiles_.size(); ++j) {
                    if (dead[j]) continue;
                    if (canMerge(tiles_[i], tiles_[j])) {
                        tiles_[i].box = mergeBox(tiles_[i].box, tiles_[j].box);
                        dead[j] = true;
                        changed = true;
                    }
                }
            }

            if (changed) {
                std::vector<Tile> compact;
                compact.reserve(tiles_.size());
                for (int i = 0; i < (int)tiles_.size(); ++i) {
                    if (!dead[i]) compact.push_back(tiles_[i]);
                }
                tiles_.swap(compact);
            }
        } while (changed);

        rebuildStitching();
    }

private:
    Rect bbox_{};
    LayerKind defaultKind_ = LayerKind::Space;
    std::vector<Tile> tiles_;

    static Tile makeTile(const Rect& r, LayerKind kind, int featureId) {
        return Tile{r, kind, featureId, -1, -1, -1, -1, -1};
    }

    static Rect mergeBox(const Rect& a, const Rect& b) {
        return {std::min(a.x1, b.x1), std::min(a.y1, b.y1), std::max(a.x2, b.x2), std::max(a.y2, b.y2)};
    }

    static bool canMerge(const Tile& a, const Tile& b) {
        if (a.kind != b.kind || a.featureId != b.featureId) return false;

        // Vertical merge: same x-span, touching in y
        if (a.box.x1 == b.box.x1 && a.box.x2 == b.box.x2) {
            if (a.box.y2 == b.box.y1 || b.box.y2 == a.box.y1) return true;
        }

        // Horizontal merge: same y-span, touching in x
        if (a.box.y1 == b.box.y1 && a.box.y2 == b.box.y2) {
            if (a.box.x2 == b.box.x1 || b.box.x2 == a.box.x1) return true;
        }

        return false;
    }
};

struct LayerPlane {
    LayerKind kind;
    Plane plane;
    std::string name;
};

class DRCChecker {
public:
    DRCChecker(Rect bbox)
        : bbox_(bbox) {}

    Plane& addLayer(LayerKind kind) {
        const std::string name = layerName(kind);
        auto [it, inserted] = layers_.emplace(kind, LayerPlane{kind, Plane(bbox_, kind), name});
        if (!inserted) {
            throw std::runtime_error("Layer already exists: " + name);
        }
        return it->second.plane;
    }

    Plane& layer(LayerKind kind) {
        auto it = layers_.find(kind);
        if (it == layers_.end()) throw std::runtime_error(std::string("Missing layer: ") + layerName(kind));
        return it->second.plane;
    }

    const Plane& layer(LayerKind kind) const {
        auto it = layers_.find(kind);
        if (it == layers_.end()) throw std::runtime_error(std::string("Missing layer: ") + layerName(kind));
        return it->second.plane;
    }

    void paint(LayerKind kind, const Rect& r, int featureId) {
        layer(kind).paint(r, kind, featureId);
    }

    std::vector<Violation> run(const RuleSet& rules) const {
        std::vector<Violation> out;

        // Per-layer checks
        for (const auto& kv : layers_) {
            const LayerKind k = kv.first;
            const Plane& p = kv.second.plane;
            out.insert(out.end(), checkMinWidth(p, k, rules).begin(), checkMinWidth(p, k, rules).end());
            out.insert(out.end(), checkMinArea(p, k, rules).begin(), checkMinArea(p, k, rules).end());
            out.insert(out.end(), checkMinSpace(p, k, rules).begin(), checkMinSpace(p, k, rules).end());
        }

        // Pairwise cross-layer checks
        for (auto itA = layers_.begin(); itA != layers_.end(); ++itA) {
            for (auto itB = std::next(itA); itB != layers_.end(); ++itB) {
                const LayerKind a = itA->first;
                const LayerKind b = itB->first;

                if (rules.isForbidden(a, b)) {
                    auto v = checkForbiddenIntersection(itA->second.plane, itA->second.name,
                                                        itB->second.plane, itB->second.name);
                    out.insert(out.end(), v.begin(), v.end());
                }
            }
        }

        // Required intersections and enclosure rules
        for (const auto& kv : rules.enclosure) {
            const LayerKind inner = kv.first.first;
            const LayerKind outer = kv.first.second;
            const i64 enc = kv.second;
            if (!hasLayer(inner) || !hasLayer(outer)) continue;
            auto v = checkEnclosure(layer(inner), layerName(inner), layer(outer), layerName(outer), enc);
            out.insert(out.end(), v.begin(), v.end());
        }

        return out;
    }

    void dumpLayerSummary() const {
        for (const auto& kv : layers_) {
            const auto& pl = kv.second.plane;
            std::cout << pl.tiles().size() << " tiles on " << plName(kv.first) << "\n";
        }
    }

private:
    Rect bbox_{};
    std::map<LayerKind, LayerPlane> layers_;

    bool hasLayer(LayerKind k) const {
        return layers_.count(k) != 0;
    }

    static std::string plName(LayerKind k) {
        return layerName(k);
    }

    static std::vector<Violation> checkMinWidth(const Plane& p, LayerKind k, const RuleSet& rules) {
        std::vector<Violation> out;
        const i64 minW = rules.getMinWidth(k);
        if (minW <= 0) return out;

        // Conservative: every non-space tile must satisfy local min-width.
        // In a full Magic-style implementation, width is edge/neck based.
        for (const auto& t : p.tiles()) {
            if (t.kind != k) continue;
            const i64 local = std::min(t.box.width(), t.box.height());
            if (local < minW) {
                out.push_back(Violation{
                    "MIN_WIDTH",
                    layerName(k),
                    "",
                    t.featureId,
                    -1,
                    t.box,
                    {},
                    local,
                    "tile local width below minimum"
                });
            }
        }
        return out;
    }

    static std::vector<Violation> checkMinArea(const Plane& p, LayerKind k, const RuleSet& rules) {
        std::vector<Violation> out;
        const i64 minA = rules.getMinArea(k);
        if (minA <= 0) return out;

        std::map<int, i64> areaByFeature;
        std::map<int, Rect> bboxByFeature;
        std::set<int> seen;

        for (const auto& t : p.tiles()) {
            if (t.kind != k) continue;
            areaByFeature[t.featureId] += t.box.area();
            if (!seen.count(t.featureId)) {
                bboxByFeature[t.featureId] = t.box;
                seen.insert(t.featureId);
            } else {
                auto& b = bboxByFeature[t.featureId];
                b.x1 = std::min(b.x1, t.box.x1);
                b.y1 = std::min(b.y1, t.box.y1);
                b.x2 = std::max(b.x2, t.box.x2);
                b.y2 = std::max(b.y2, t.box.y2);
            }
        }

        for (const auto& kv : areaByFeature) {
            if (kv.second < minA) {
                out.push_back(Violation{
                    "MIN_AREA",
                    layerName(k),
                    "",
                    kv.first,
                    -1,
                    bboxByFeature[kv.first],
                    {},
                    kv.second,
                    "feature area below minimum"
                });
            }
        }
        return out;
    }

    static std::vector<Violation> checkMinSpace(const Plane& p, LayerKind k, const RuleSet& rules) {
        std::vector<Violation> out;
        const i64 minS = rules.getMinSpace(k);
        if (minS <= 0) return out;

        const auto& tiles = p.tiles();
        for (int i = 0; i < (int)tiles.size(); ++i) {
            const auto& a = tiles[i];
            if (a.kind != k) continue;
            for (int j = i + 1; j < (int)tiles.size(); ++j) {
                const auto& b = tiles[j];
                if (b.kind != k) continue;
                if (a.featureId == b.featureId) continue; // same feature, internal boundaries ignored

                const i64 d = a.box.manhattanDistanceTo(b.box);
                if (d < minS && d > 0) {
                    out.push_back(Violation{
                        "MIN_SPACE",
                        layerName(k),
                        layerName(k),
                        a.featureId,
                        b.featureId,
                        a.box,
                        b.box,
                        d,
                        "same-layer spacing below minimum"
                    });
                }
            }
        }
        return out;
    }

    static std::vector<Violation> checkForbiddenIntersection(const Plane& aPlane, const std::string& aName,
                                                             const Plane& bPlane, const std::string& bName) {
        std::vector<Violation> out;
        for (const auto& a : aPlane.tiles()) {
            if (a.kind == LayerKind::Space) continue;
            for (const auto& b : bPlane.tiles()) {
                if (b.kind == LayerKind::Space) continue;
                if (a.box.intersects(b.box)) {
                    out.push_back(Violation{
                        "INTERSECTION_FORBIDDEN",
                        aName,
                        bName,
                        a.featureId,
                        b.featureId,
                        a.box,
                        b.box,
                        0,
                        "layers must not intersect"
                    });
                }
            }
        }
        return out;
    }

    static std::vector<Violation> checkEnclosure(const Plane& innerPlane, const std::string& innerName,
                                                 const Plane& outerPlane, const std::string& outerName,
                                                 i64 enc) {
        std::vector<Violation> out;
        if (enc < 0) return out;

        for (const auto& in : innerPlane.tiles()) {
            if (in.kind == LayerKind::Space) continue;
            const Rect need = in.box.expand(enc);
            bool ok = false;
            for (const auto& outT : outerPlane.tiles()) {
                if (outT.kind == LayerKind::Space) continue;
                if (outT.box.contains(need)) {
                    ok = true;
                    break;
                }
            }
            if (!ok) {
                out.push_back(Violation{
                    "ENCLOSURE",
                    innerName,
                    outerName,
                    in.featureId,
                    -1,
                    in.box,
                    {},
                    enc,
                    "outer layer does not enclose inner layer by required amount"
                });
            }
        }
        return out;
    }
};

} // namespace drc

#endif // DRC_HPP
