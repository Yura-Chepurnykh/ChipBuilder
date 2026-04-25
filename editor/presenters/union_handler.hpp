// #ifndef UNION_HANDLER_HPP
// #define UNION_HANDLER_HPP

// #include "layers.hpp"
// #include <vector>
// #include <memory>
// #include <map>

// class UnionHandler {
// public:
//     static bool performUnions(CircuitLayout& layout) {
//         bool changed = false;
//         std::map<std::string, std::vector<std::shared_ptr<AComponent>>> layersMap;
        
//         for (auto& comp : layout.m_components) {
//             layersMap[comp->name()].push_back(comp);
//         }

//         for (auto& [name, components] : layersMap) {
//             for (size_t i = 0; i < components.size(); ++i) {
//                 for (size_t j = i + 1; j < components.size(); ++j) {
//                     auto r1 = dynamic_cast<Rect*>(components[i]->getShape());
//                     auto r2 = dynamic_cast<Rect*>(components[j]->getShape());
                    
//                     if (r1 && r2 && r1->intersects(*r2)) {
//                         auto polyPoints = getUnionPoints(*r1, *r2);
//                         auto newPoly = std::make_unique<PolygonShape>(polyPoints);
                        
//                         components[i]->setShape(std::move(newPoly));
//                         layout.remove(components[j]->id);
                        
//                         changed = true;
//                         return true;
//                     }
//                 }
//             }
//         }
//         return changed;
//     }

//     static std::unique_ptr<IShape> tryMerge(const std::vector<std::shared_ptr<AComponent>>& components) {
//         if (components.size() < 2) return nullptr;
        
//         // Verify same type
//         std::string type = components[0]->name();
//         for (const auto& c : components) {
//             if (c->name() != type) return nullptr;
//         }

//         // Simple check: are they all connected? (at least r1 intersects r2, r2 intersects r3...)
//         // For now, let's just merge all into a bounding box or simple union of the first two for demonstration
//         auto r1 = dynamic_cast<Rect*>(components[0]->getShape());
//         auto r2 = dynamic_cast<Rect*>(components[1]->getShape());
        
//         if (r1 && r2 && r1->intersects(*r2)) {
//             return std::make_unique<PolygonShape>(getUnionPoints(*r1, *r2));
//         }
        
//         return nullptr;
//     }

//     static std::vector<Point> getUnionPoints(const Rect& r1, const Rect& r2) {
//         std::vector<Point> p;
//         int x1 = r1.point.x, y1 = r1.point.y, x2 = x1 + r1.width, y2 = y1 + r1.height;
//         int x3 = r2.point.x, y3 = r2.point.y, x4 = x3 + r2.width, y4 = y3 + r2.height;

//         int minX = std::min(x1, x3);
//         int minY = std::min(y1, y3);
//         int maxX = std::max(x2, x4);
//         int maxY = std::max(y2, y4);

//         p.emplace_back(-1, minX, minY);
//         p.emplace_back(-1, maxX, minY);
//         p.emplace_back(-1, maxX, maxY);
//         p.emplace_back(-1, minX, maxY);
//         p.emplace_back(-1, minX, minY);
        
//         return p;
//     }
// };

// #endif // UNION_HANDLER_HPP
