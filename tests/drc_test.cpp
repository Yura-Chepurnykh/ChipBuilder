#include "drc_engine.hpp"
#include <iostream>
#include <cassert>

void test_min_spacing() {
    std::cout << "Testing Min Spacing Rule..." << std::endl;
    DRCEngine engine;
    
    // Two rectangles on 'metal1' layer
    // S1: (0,0) to (20,20)
    engine.addShape(Geometry::Shape::Type::Rectangle, "metal1", 1, {{0,0}, {20,0}, {20,20}, {0,20}});
    // S2: (25,0) to (45,20) -> Spacing is 5, threshold is 30
    engine.addShape(Geometry::Shape::Type::Rectangle, "metal1", 2, {{25,0}, {45,0}, {45,20}, {25,20}});
    
    engine.addRule(std::make_unique<DRC::MinSpacingRule>("metal1", 30.0));
    
    auto violations = engine.run();
    std::cout << "Violations found: " << violations.size() << std::endl;
    for (auto& v : violations) {
        std::cout << "  - " << v.description << " at " << v.location.minX << "," << v.location.minY << std::endl;
    }
    assert(violations.size() > 0);
    std::cout << "Min Spacing Rule Test Passed!" << std::endl << std::endl;
}

void test_overlap() {
    std::cout << "Testing Overlap (Spacing 0)..." << std::endl;
    DRCEngine engine;
    
    // Two overlapping rectangles
    engine.addShape(Geometry::Shape::Type::Rectangle, "metal1", 1, {{0,0}, {20,0}, {20,20}, {0,20}});
    engine.addShape(Geometry::Shape::Type::Rectangle, "metal1", 2, {{10,10}, {30,10}, {30,30}, {10,30}});
    
    engine.addRule(std::make_unique<DRC::MinSpacingRule>("metal1", 30.0));
    
    auto violations = engine.run();
    std::cout << "Violations found: " << violations.size() << std::endl;
    assert(violations.size() > 0);
    std::cout << "Overlap Test Passed!" << std::endl << std::endl;
}

void test_min_area() {
    std::cout << "Testing Min Area Rule..." << std::endl;
    DRCEngine engine;
    
    // Small rectangle: 10x10 = 100, threshold 900
    engine.addShape(Geometry::Shape::Type::Rectangle, "poly", 1, {{0,0}, {10,0}, {10,10}, {0,10}});
    
    engine.addRule(std::make_unique<DRC::MinAreaRule>("poly", 900.0));
    
    auto violations = engine.run();
    std::cout << "Violations found: " << violations.size() << std::endl;
    assert(violations.size() == 1);
    std::cout << "Min Area Rule Test Passed!" << std::endl << std::endl;
}

void test_enclosure() {
    std::cout << "Testing Enclosure Rule..." << std::endl;
    DRCEngine engine;
    
    // Active inside NWell, but margin too small
    // NWell: (0,0) to (100,100)
    engine.addShape(Geometry::Shape::Type::Rectangle, "nwell", 1, {{0,0}, {100,0}, {100,100}, {0,100}});
    // Active: (10,10) to (90,90) -> margin 10, threshold 30
    engine.addShape(Geometry::Shape::Type::Rectangle, "active", 2, {{10,10}, {90,10}, {90,90}, {10,90}});
    
    engine.addRule(std::make_unique<DRC::EnclosureRule>("nwell", "active", 30.0));
    
    auto violations = engine.run();
    std::cout << "Violations found: " << violations.size() << std::endl;
    assert(violations.size() > 0);
    std::cout << "Enclosure Rule Test Passed!" << std::endl << std::endl;
}

int main() {
    try {
        test_min_spacing();
        test_overlap();
        test_min_area();
        test_enclosure();
        std::cout << "ALL TESTS PASSED!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
