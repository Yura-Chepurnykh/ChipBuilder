#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace drc {

// Типы слоев
enum class LayerType {
    Metal,
    Poly,
    Diffusion,
    Via,
    Contact,
    NWell,
    PWell
};

// Примитивы для проверки
struct Shape {
    LayerType layer;
    std::string name;
    // Прямоугольник: min_x, min_y, max_x, max_y
    double min_x, min_y, max_x, max_y;
};

using Box = boost::geometry::model::box<boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>>;
using RTree = boost::geometry::index::rtree<std::pair<Box, size_t>, boost::geometry::index::quadratic<16>>;

// Базовый интерфейс для DRC правил
class DRCRule {
public:
    virtual ~DRCRule() = default;
    virtual std::vector<std::string> check(const std::vector<Shape>& shapes, const RTree& rtree) const = 0;
    virtual std::string name() const = 0;
};

// Правило минимальной ширины
class MinimumWidthRule : public DRCRule {
public:
    MinimumWidthRule(const std::unordered_map<LayerType, double>& minWidths)
        : minWidths_(minWidths) {}

    std::vector<std::string> check(const std::vector<Shape>& shapes, const RTree&) const override {
        std::vector<std::string> violations;
        for (const auto& shape : shapes) {
            auto it = minWidths_.find(shape.layer);
            if (it != minWidths_.end()) {
                double width = std::min(shape.max_x - shape.min_x, shape.max_y - shape.min_y);
                if (width < it->second) {
                    violations.push_back(shape.name + ": width " + std::to_string(width) + " < min " + std::to_string(it->second));
                }
            }
        }
        return violations;
    }

    std::string name() const override { return "Minimum Width"; }
private:
    std::unordered_map<LayerType, double> minWidths_;
};

// Правило минимального расстояния
class MinimumSpacingRule : public DRCRule {
public:
    MinimumSpacingRule(const std::unordered_map<LayerType, double>& minSpacings)
        : minSpacings_(minSpacings) {}

    std::vector<std::string> check(const std::vector<Shape>& shapes, const RTree& rtree) const override {
        std::vector<std::string> violations;
        for (size_t i = 0; i < shapes.size(); ++i) {
            const auto& shape = shapes[i];
            auto it = minSpacings_.find(shape.layer);
            if (it != minSpacings_.end()) {
                Box box1(boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(shape.min_x, shape.min_y),
                         boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(shape.max_x, shape.max_y));
                // Найти объекты того же слоя
                std::vector<std::pair<Box, size_t>> result;
                rtree.query(boost::geometry::index::intersects(box1), std::back_inserter(result));
                for (const auto& [box2, idx] : result) {
                    if (idx == i) continue; // не сравнивать с самим собой
                    const auto& other = shapes[idx];
                    if (other.layer != shape.layer) continue;
                    double dist = boost::geometry::distance(box1, box2);
                    if (dist < it->second) {
                        violations.push_back(shape.name + " vs " + other.name + ": spacing " + std::to_string(dist) + " < min " + std::to_string(it->second));
                    }
                }
            }
        }
        return violations;
    }

    std::string name() const override { return "Minimum Spacing"; }
private:
    std::unordered_map<LayerType, double> minSpacings_;
};

// Правило минимальной площади
class MinimumAreaRule : public DRCRule {
public:
    MinimumAreaRule(const std::unordered_map<LayerType, double>& minAreas)
        : minAreas_(minAreas) {}

    std::vector<std::string> check(const std::vector<Shape>& shapes, const RTree&) const override {
        std::vector<std::string> violations;
        for (const auto& shape : shapes) {
            auto it = minAreas_.find(shape.layer);
            if (it != minAreas_.end()) {
                double area = (shape.max_x - shape.min_x) * (shape.max_y - shape.min_y);
                if (area < it->second) {
                    violations.push_back(shape.name + ": area " + std::to_string(area) + " < min " + std::to_string(it->second));
                }
            }
        }
        return violations;
    }

    std::string name() const override { return "Minimum Area"; }
private:
    std::unordered_map<LayerType, double> minAreas_;
};

// Правило минимального охвата
class MinimumEnclosureRule : public DRCRule {
public:
    MinimumEnclosureRule(LayerType enclosingLayer, LayerType innerLayer, double minEnclosure)
        : enclosingLayer_(enclosingLayer), innerLayer_(innerLayer), minEnclosure_(minEnclosure) {}

    std::vector<std::string> check(const std::vector<Shape>& shapes, const RTree& rtree) const override {
        std::vector<std::string> violations;
        for (size_t i = 0; i < shapes.size(); ++i) {
            const auto& inner = shapes[i];
            if (inner.layer != innerLayer_) continue;
            Box innerBox(boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(inner.min_x, inner.min_y),
                         boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(inner.max_x, inner.max_y));
            // Найти перекрывающие объекты
            std::vector<std::pair<Box, size_t>> result;
            rtree.query(boost::geometry::index::intersects(innerBox), std::back_inserter(result));
            bool found = false;
            for (const auto& [box, idx] : result) {
                const auto& enclosing = shapes[idx];
                if (enclosing.layer != enclosingLayer_) continue;
                // Проверить enclosure
                double left = inner.min_x - enclosing.min_x;
                double right = enclosing.max_x - inner.max_x;
                double top = enclosing.max_y - inner.max_y;
                double bottom = inner.min_y - enclosing.min_y;
                double minEnclosure = std::min({left, right, top, bottom});
                if (minEnclosure < minEnclosure_) {
                    violations.push_back(enclosing.name + " does not enclose " + inner.name + " by min " + std::to_string(minEnclosure_));
                }
                found = true;
            }
            if (!found) {
                violations.push_back(inner.name + " is not enclosed by " + std::to_string(minEnclosure_));
            }
        }
        return violations;
    }

    std::string name() const override { return "Minimum Enclosure"; }
private:
    LayerType enclosingLayer_;
    LayerType innerLayer_;
    double minEnclosure_;
};

// Правило минимального перекрытия
class MinimumOverlapRule : public DRCRule {
public:
    MinimumOverlapRule(LayerType layerA, LayerType layerB, double minOverlap)
        : layerA_(layerA), layerB_(layerB), minOverlap_(minOverlap) {}

    std::vector<std::string> check(const std::vector<Shape>& shapes, const RTree& rtree) const override {
        std::vector<std::string> violations;
        for (size_t i = 0; i < shapes.size(); ++i) {
            const auto& shapeA = shapes[i];
            if (shapeA.layer != layerA_) continue;
            Box boxA(boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(shapeA.min_x, shapeA.min_y),
                     boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(shapeA.max_x, shapeA.max_y));
            std::vector<std::pair<Box, size_t>> result;
            rtree.query(boost::geometry::index::intersects(boxA), std::back_inserter(result));
            bool found = false;
            for (const auto& [boxB, idx] : result) {
                const auto& shapeB = shapes[idx];
                if (shapeB.layer != layerB_) continue;
                double overlap_x = std::min(shapeA.max_x, shapeB.max_x) - std::max(shapeA.min_x, shapeB.min_x);
                double overlap_y = std::min(shapeA.max_y, shapeB.max_y) - std::max(shapeA.min_y, shapeB.min_y);
                double overlap_area = (overlap_x > 0 && overlap_y > 0) ? overlap_x * overlap_y : 0;
                if (overlap_area < minOverlap_) {
                    violations.push_back(shapeA.name + " and " + shapeB.name + ": overlap " + std::to_string(overlap_area) + " < min " + std::to_string(minOverlap_));
                }
                found = true;
            }
            if (!found) {
                violations.push_back(shapeA.name + " does not overlap " + std::to_string(minOverlap_));
            }
        }
        return violations;
    }

    std::string name() const override { return "Minimum Overlap"; }
private:
    LayerType layerA_;
    LayerType layerB_;
    double minOverlap_;
};

// Правило минимального расширения
class MinimumExtensionRule : public DRCRule {
public:
    MinimumExtensionRule(LayerType extendingLayer, LayerType baseLayer, double minExtension)
        : extendingLayer_(extendingLayer), baseLayer_(baseLayer), minExtension_(minExtension) {}

    std::vector<std::string> check(const std::vector<Shape>& shapes, const RTree& rtree) const override {
        std::vector<std::string> violations;
        for (size_t i = 0; i < shapes.size(); ++i) {
            const auto& base = shapes[i];
            if (base.layer != baseLayer_) continue;
            Box baseBox(boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(base.min_x, base.min_y),
                        boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(base.max_x, base.max_y));
            std::vector<std::pair<Box, size_t>> result;
            rtree.query(boost::geometry::index::intersects(baseBox), std::back_inserter(result));
            for (const auto& [extBox, idx] : result) {
                const auto& ext = shapes[idx];
                if (ext.layer != extendingLayer_) continue;
                double ext_left = ext.min_x - base.min_x;
                double ext_right = ext.max_x - base.max_x;
                double ext_top = ext.max_y - base.max_y;
                double ext_bottom = ext.min_y - base.min_y;
                double maxExtension = std::max({ext_left, ext_right, ext_top, ext_bottom});
                if (maxExtension < minExtension_) {
                    violations.push_back(ext.name + " does not extend " + base.name + " by min " + std::to_string(minExtension_));
                }
            }
        }
        return violations;
    }

    std::string name() const override { return "Minimum Extension"; }
private:
    LayerType extendingLayer_;
    LayerType baseLayer_;
    double minExtension_;
};

// Contact / Via Spacing Rule
class ContactViaSpacingRule : public DRCRule {
public:
    ContactViaSpacingRule(LayerType contactLayer, double minSpacing)
        : contactLayer_(contactLayer), minSpacing_(minSpacing) {}

    std::vector<std::string> check(const std::vector<Shape>& shapes, const RTree& rtree) const override {
        std::vector<std::string> violations;
        for (size_t i = 0; i < shapes.size(); ++i) {
            const auto& shape = shapes[i];
            if (shape.layer != contactLayer_) continue;
            Box box1(boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(shape.min_x, shape.min_y),
                     boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(shape.max_x, shape.max_y));
            std::vector<std::pair<Box, size_t>> result;
            rtree.query(boost::geometry::index::intersects(box1), std::back_inserter(result));
            for (const auto& [box2, idx] : result) {
                if (idx == i) continue;
                const auto& other = shapes[idx];
                if (other.layer != contactLayer_) continue;
                double dist = boost::geometry::distance(box1, box2);
                if (dist < minSpacing_) {
                    violations.push_back(shape.name + " vs " + other.name + ": contact/via spacing " + std::to_string(dist) + " < min " + std::to_string(minSpacing_));
                }
            }
        }
        return violations;
    }

    std::string name() const override { return "Contact / Via Spacing"; }
private:
    LayerType contactLayer_;
    double minSpacing_;
};

// Contact / Via Enclosure Rule
class ContactViaEnclosureRule : public DRCRule {
public:
    ContactViaEnclosureRule(LayerType enclosingLayer, LayerType contactLayer, double minEnclosure)
        : enclosingLayer_(enclosingLayer), contactLayer_(contactLayer), minEnclosure_(minEnclosure) {}

    std::vector<std::string> check(const std::vector<Shape>& shapes, const RTree& rtree) const override {
        std::vector<std::string> violations;
        for (size_t i = 0; i < shapes.size(); ++i) {
            const auto& contact = shapes[i];
            if (contact.layer != contactLayer_) continue;
            Box contactBox(boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(contact.min_x, contact.min_y),
                           boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(contact.max_x, contact.max_y));
            std::vector<std::pair<Box, size_t>> result;
            rtree.query(boost::geometry::index::intersects(contactBox), std::back_inserter(result));
            bool found = false;
            for (const auto& [box, idx] : result) {
                const auto& enclosing = shapes[idx];
                if (enclosing.layer != enclosingLayer_) continue;
                double left = contact.min_x - enclosing.min_x;
                double right = enclosing.max_x - contact.max_x;
                double top = contact.max_y - contact.min_y;
                double bottom = contact.min_y - enclosing.min_y;
                double minEnclosure = std::min({left, right, top, bottom});
                if (minEnclosure < minEnclosure_) {
                    violations.push_back(enclosing.name + " does not enclose contact/via " + contact.name + " by min " + std::to_string(minEnclosure_));
                }
                found = true;
            }
            if (!found) {
                violations.push_back(contact.name + " is not enclosed by " + std::to_string(minEnclosure_));
            }
        }
        return violations;
    }

    std::string name() const override { return "Contact / Via Enclosure"; }
private:
    LayerType enclosingLayer_;
    LayerType contactLayer_;
    double minEnclosure_;
};

// Well Spacing Rule
class WellSpacingRule : public DRCRule {
public:
    WellSpacingRule(LayerType wellA, LayerType wellB, double minSpacing)
        : wellA_(wellA), wellB_(wellB), minSpacing_(minSpacing) {}

    std::vector<std::string> check(const std::vector<Shape>& shapes, const RTree& rtree) const override {
        std::vector<std::string> violations;
        for (size_t i = 0; i < shapes.size(); ++i) {
            const auto& shapeA = shapes[i];
            if (shapeA.layer != wellA_ && shapeA.layer != wellB_) continue;
            Box boxA(boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(shapeA.min_x, shapeA.min_y),
                     boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(shapeA.max_x, shapeA.max_y));
            std::vector<std::pair<Box, size_t>> result;
            rtree.query(boost::geometry::index::intersects(boxA), std::back_inserter(result));
            for (const auto& [boxB, idx] : result) {
                if (idx == i) continue;
                const auto& shapeB = shapes[idx];
                if ((shapeA.layer == wellA_ && shapeB.layer == wellB_) || (shapeA.layer == wellB_ && shapeB.layer == wellA_)) {
                    double dist = boost::geometry::distance(boxA, boxB);
                    if (dist < minSpacing_) {
                        violations.push_back(shapeA.name + " vs " + shapeB.name + ": well spacing " + std::to_string(dist) + " < min " + std::to_string(minSpacing_));
                    }
                }
            }
        }
        return violations;
    }

    std::string name() const override { return "Well Spacing"; }
private:
    LayerType wellA_;
    LayerType wellB_;
    double minSpacing_;
};

// Gate Length / Poly Spacing Rule
class GatePolyRule : public DRCRule {
public:
    GatePolyRule(double minGateLength, double minPolySpacing)
        : minGateLength_(minGateLength), minPolySpacing_(minPolySpacing) {}

    std::vector<std::string> check(const std::vector<Shape>& shapes, const RTree& rtree) const override {
        std::vector<std::string> violations;
        // Проверка минимальной длины канала (poly над diffusion)
        for (const auto& shape : shapes) {
            if (shape.layer == LayerType::Poly) {
                double gateLength = shape.max_x - shape.min_x;
                if (gateLength < minGateLength_) {
                    violations.push_back(shape.name + ": gate length " + std::to_string(gateLength) + " < min " + std::to_string(minGateLength_));
                }
            }
        }
        // Проверка spacing между poly
        for (size_t i = 0; i < shapes.size(); ++i) {
            const auto& shapeA = shapes[i];
            if (shapeA.layer != LayerType::Poly) continue;
            Box boxA(boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(shapeA.min_x, shapeA.min_y),
                     boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>(shapeA.max_x, shapeA.max_y));
            std::vector<std::pair<Box, size_t>> result;
            rtree.query(boost::geometry::index::intersects(boxA), std::back_inserter(result));
            for (const auto& [boxB, idx] : result) {
                if (idx == i) continue;
                const auto& shapeB = shapes[idx];
                if (shapeB.layer != LayerType::Poly) continue;
                double dist = boost::geometry::distance(boxA, boxB);
                if (dist < minPolySpacing_) {
                    violations.push_back(shapeA.name + " vs " + shapeB.name + ": poly spacing " + std::to_string(dist) + " < min " + std::to_string(minPolySpacing_));
                }
            }
        }
        return violations;
    }

    std::string name() const override { return "Gate Length / Poly Spacing"; }
private:
    double minGateLength_;
    double minPolySpacing_;
};

// ...дальнейшая реализация правил...

} // namespace drc
