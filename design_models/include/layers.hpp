#ifndef LAYERS_HPP
#define LAYERS_HPP

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include "concepts.hpp"
#include "geometry.hpp"
#include "../third_party/json.hpp"

using JSON = nlohmann::json;

// We are use composite pattern to handle 
// the different groups of layers and separate
// layers the same way, so in our case the layer is
// a leaf, and the Circuit layout is a composite
class AComponent
{
public:
    AComponent() = default;
    virtual ~AComponent() = default;
    
    // meta-information
    virtual std::string name() const = 0;
    virtual IShape* getShape() = 0;
    virtual void setShape(std::unique_ptr<IShape>) = 0;
    virtual void move(int dx, int dy) = 0;
    virtual void move(const Point&) = 0;
    virtual void accept(struct IVisitor& visitor) = 0;
    virtual double area() const = 0;

    unsigned int id;
    int zLevel = 0;

protected:
    AComponent(unsigned int id, int zLevel = 0) : id(id), zLevel(zLevel) { }
};

class CircuitLayout final : public AComponent 
{
public:
    CircuitLayout() = default;

    CircuitLayout(unsigned int id, int zLevel = 0, std::vector<std::shared_ptr<AComponent>> components = {}) :
        AComponent(id, zLevel), m_components(std::move(components)) { }

    std::string name() const override { return "circuit layout"; }

    void add(std::shared_ptr<AComponent> c) 
    {
        m_components.push_back(std::move(c));
    }

    void remove(unsigned int id)
    {
        if (auto it = std::ranges::find_if(m_components, [&](std::shared_ptr<AComponent>& c) {
            return c->id == id;
        }); it != m_components.end())
        {
            m_components.erase(it);
        }
    }

    void move(int dx, int dy) override
    {
        for (const auto& c : m_components)
            c->move(dx, dy);
    }

    void move(const Point& point) override
    {
        for (const auto& c : m_components)
            c->move(point);
    }

    void accept(IVisitor& visitor) override;
    
    IShape* getShape() override { return nullptr; }
    void setShape(std::unique_ptr<IShape>) override { }

    double area() const override
    {
        double total = 0.0;
        for (const auto& c : m_components)
            if (c) total += c->area();
        return total;
    }

// private:
    // we use unique_ptr to avoid slicing
    std::vector<std::shared_ptr<AComponent>> m_components;
};

struct Layer : public AComponent
{
public:
    virtual ~Layer() { }
    
    void move(int dx, int dy) override
    {
        if (m_shape) m_shape->move(dx, dy);
    }

    void move(const Point& point) override
    {
        if (m_shape) m_shape->move(point);
    }

    void setShape(std::unique_ptr<IShape> s) override { m_shape = std::move(s); }
    IShape* getShape() override { return m_shape.get(); }

    double area() const override
    {
        return m_shape ? m_shape->area() : 0.0;
    }

// protected:
    Layer(unsigned int id, int zLevel = 0, std::unique_ptr<IShape> shape = nullptr) noexcept : 
        AComponent(id, zLevel), m_shape(std::move(shape)) { }

private:
    std::unique_ptr<IShape> m_shape;
};

template<typename Derived, typename Base = Layer>
class CRTP : public Base 
{
public:
    using Base::Base;

    void accept(IVisitor& visitor) override;
};

class Active final : public CRTP<Active> { public: using CRTP<Active>::CRTP; std::string name() const override { return "active"; } };
class Poly final : public CRTP<Poly> { public: using CRTP<Poly>::CRTP; std::string name() const override { return "poly"; } };
class NWell final : public CRTP<NWell> { public: using CRTP<NWell>::CRTP; std::string name() const override { return "nwell"; } };
class PWell final : public CRTP<PWell> { public: using CRTP<PWell>::CRTP; std::string name() const override { return "pwell"; } };
class Metal1 final : public CRTP<Metal1> 
{ 
public: 
    Metal1(unsigned int id, int zLevel = 0, std::unique_ptr<IShape> shape = nullptr, int thickness = 2) : 
        CRTP<Metal1>(id, zLevel, std::move(shape)), thickness(thickness) { }
    
    std::string name() const override { return "metal1"; }
    
    double area() const override
    {
        if (auto s = const_cast<Metal1*>(this)->getShape())
        {
            double a = s->area();
            if (a > 0.001) return a;
            
            double len = s->length();
            if (len < 0.001) {
                // Handle degenerate case (1 point or all same points, or 0x0 rect)
                return (double)thickness * thickness;
            }
            return (double)thickness * len;
        }
        return 0.0;
    }

    int thickness = 2; // in lambda
};

class Via final : public CRTP<Via> { public: using CRTP<Via>::CRTP; std::string name() const override { return "via"; } };
class Contact final : public CRTP<Contact> { public: using CRTP<Contact>::CRTP; std::string name() const override { return "contact"; } };

// Pattern Visitor
struct IVisitor
{
    virtual void visit(const Active&)  = 0;
    virtual void visit(const Poly&)    = 0;
    virtual void visit(const NWell&)   = 0;
    virtual void visit(const PWell&)   = 0;
    virtual void visit(const Metal1&)  = 0;
    virtual void visit(const Via&)     = 0;
    virtual void visit(const Contact&) = 0;
    
    virtual void visit(const Rect& r)           = 0;
    virtual void visit(const PolygonShape& p)    = 0;

    virtual ~IVisitor() = default;
};

inline void CircuitLayout::accept(IVisitor& visitor)
{
    for (const auto& c : m_components)
        if (c != nullptr) c->accept(visitor);
}

class JSONSerializer final : public IVisitor
{
public:
    void serialize(const AComponent& component)
    {
        JSON json {
            { "id",       component.id   },
            { "zLevel",   component.zLevel },
            { "name",     component.name() },
        };

        if (auto shape = const_cast<AComponent&>(component).getShape())
        {
            JSONSerializer shapeSerializer;
            shape->accept(shapeSerializer);
            if (!shapeSerializer.getJson().empty())
            {
                json["shape"] = shapeSerializer.getJson().at(0);
            }
        }
        m_json.push_back(json);
    }

    void visit(const Active& active) override { serialize(active); }
    void visit(const Poly& poly) override { serialize(poly); }
    void visit(const NWell& nwell) override { serialize(nwell); }
    void visit(const PWell& pwell) override { serialize(pwell); }
    void visit(const Metal1& metal1) override 
    { 
        serialize(metal1); 
        m_json.back()["thickness"] = metal1.thickness;
    }
    void visit(const Via& via) override { serialize(via); }
    void visit(const Contact& contact) override { serialize(contact); }

    void visit(const Rect& r) override
    {
        JSON rectJson {
            { "id",       r.id       },
            { "geometry", "rectangle" },
            { "x",        r.point.x  },
            { "y",        r.point.y  },
            { "width",    r.width    },
            { "height",   r.height   }
        };
        m_json.push_back(rectJson);
    }

    void visit(const PolygonShape& polygon) override
    {
        JSON polygonJson {
            { "id",       polygon.id   },
            { "geometry", "polygon"    },
            { "points",   JSON::array() }
        };

        for (const auto& p : polygon.m_points)
            polygonJson["points"].push_back({ { "x", p.x }, { "y", p.y } });

        m_json.push_back(polygonJson);
    }

    JSON getJson() const { return m_json; }

private:
    JSON m_json = JSON::array();
};

class JSONDeserializer final
{
public:
    static std::vector<std::shared_ptr<AComponent>> deserialize(const JSON& json)
    {
        std::vector<std::shared_ptr<AComponent>> components;
        if (json.is_array())
            for (const auto& item : json)
                if (auto component = deserializeComponent(item))
                    components.push_back(component);
        return components;
    }

private:
    static std::shared_ptr<AComponent> deserializeComponent(const JSON& json)
    {
        if (!json.is_object()) return nullptr;

        unsigned int id = json.value("id", 0u);
        int zLevel = json.value("zLevel", 0);
        std::string name = json.value("name", "");
        
        std::unique_ptr<IShape> shape = nullptr;
        if (json.contains("shape"))
            shape = deserializeShape(json["shape"]);

        if (name == "active")  return std::make_shared<Active>(id, zLevel, std::move(shape));
        if (name == "poly")    return std::make_shared<Poly>(id, zLevel, std::move(shape));
        if (name == "nwell")   return std::make_shared<NWell>(id, zLevel, std::move(shape));
        if (name == "pwell")   return std::make_shared<PWell>(id, zLevel, std::move(shape));
        if (name == "metal1")
        {
            int thickness = json.value("thickness", 2);
            return std::make_shared<Metal1>(id, zLevel, std::move(shape), thickness);
        }
        if (name == "via")     return std::make_shared<Via>(id, zLevel, std::move(shape));
        if (name == "contact") return std::make_shared<Contact>(id, zLevel, std::move(shape));
        
        return nullptr;
    }

    static std::unique_ptr<IShape> deserializeShape(const JSON& json)
    {
        if (!json.is_object()) return nullptr;

        unsigned int id = json.value("id", 0u);
        std::string geometry = json.value("geometry", "");

        if (geometry == "rectangle")
        {
            int x = json.value("x", 0);
            int y = json.value("y", 0);
            int width = json.value("width", 0);
            int height = json.value("height", 0);
            auto rect = std::make_unique<Rect>(Point(-1, x, y), width, height);
            rect->id = id;
            return rect;
        }
        else if (geometry == "polygon")
        {
            std::vector<Point> points;
            if (json.contains("points") && json["points"].is_array())
                for (const auto& pJson : json["points"])
                    points.emplace_back(-1, pJson.value("x", 0), pJson.value("y", 0));
            auto polygon = std::make_unique<PolygonShape>(points);
            polygon->id = id;
            return polygon;
        }
        return nullptr;
    }
};

inline void Rect::accept(IVisitor& visitor) { visitor.visit(*this); }
inline void PolygonShape::accept(IVisitor& visitor) { visitor.visit(*this); }

template<typename Derived, typename Base>
inline void CRTP<Derived, Base>::accept(IVisitor& visitor)
{
    visitor.visit(static_cast<Derived&>(*this));
}

#endif // LAYERS_HPP
