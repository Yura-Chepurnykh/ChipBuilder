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
    virtual void accept(IVisitor& visitor) = 0;

    unsigned int id;

protected:
    AComponent(unsigned int id) : id(id) { }
};

class CircuitLayout final : public AComponent 
{
public:
    CircuitLayout() = default;

    CircuitLayout(unsigned int id, std::vector<std::shared_ptr<AComponent>> components) :
        AComponent(id), m_components(std::move(components)) { }

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

    void accept(IVisitor& visitor) override
    {
        for (const auto& c : m_components)
        {
            if (c != nullptr)
            {
                std::cout << c->name();
                std::cout.flush();
                c->accept(visitor);
            }
        }
    } 
    
    IShape* getShape() override { }
    void setShape(std::unique_ptr<IShape>) override { }

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
        m_shape->move(dx, dy);
    }

    void move(const Point& point) override
    {
        m_shape->move(point);
    }

    void setShape(std::unique_ptr<IShape> s) override { m_shape = std::move(s); }
    IShape* getShape() { return m_shape.get(); }

// protected:
    Layer(unsigned int id, std::unique_ptr<IShape> shape) noexcept : 
        AComponent(id), m_shape(std::move(shape)) { }

private:
    std::unique_ptr<IShape> m_shape;
};

template<typename Derived, typename Base = Layer>
class CRTP : public Base 
{
public:
    using Base::Base;

    void accept(IVisitor& visitor)
    {
        visitor.visit(static_cast<Derived&>(*this));
    }
};

template<SemiconductorType Type>
class Substrate final : public CRTP<Substrate<Type>> 
{
public:
    using CRTP<Substrate<Type>>::CRTP;

    std::string name() const override 
    {
        if (std::is_same_v<Type, NType>)
            return "Ntype Substrate";
        else if (std::is_same_v<Type, PType>)
            return "Ptype Substrate";
        
        throw std::runtime_error("Invalid Substrate");
    }
};

template<SemiconductorType Type>
class Diffusion final : public CRTP<Diffusion<Type>>
{
public:
    using CRTP<Diffusion<Type>>::CRTP;

    std::string name() const override 
    {
        if (std::is_same_v<Type, NType>)
            return "Ntype Diffusion";
        return "Ptype Diffusion";
    }
};

template<size_t N>
class Metal final : public CRTP<Metal<N>>
{
public:
    using CRTP<Metal<N>>::CRTP;

    void addToBack(Point point) 
    {
        m_path->m_points.push_back(point);
    }

    void removeFromRear()
    {
        m_path->m_points.pop_back();
    }

    void removeById(size_t id)
    {
        if (auto it = std::ranges::find_if(m_path, [&](Point node){
            return node.id == id;
        }); it != m_path->m_points.end())
        {
            m_path->m_points.erase(it);
        }
    }

    int getGap() const noexcept { return m_gap; }
    void setGap(int g) { m_gap = g; }

    std::string name() const override { return "metal"; }

private:
    int m_gap;  
    std::unique_ptr<Polygon> m_path;
};

template<size_t N, size_t M>
class Via final : public CRTP<Layer>
{
public:
    using CRTP<Via<N, M>>::CRTP;

    std::string name() const override { return "via"; }
};

class PolySilicon final : public CRTP<PolySilicon>
{
public:
    using CRTP<PolySilicon>::CRTP;

    std::string name() const override { return "polysilicon"; }
};

class Oxide final : public CRTP<Oxide>
{
public:
    using CRTP<Oxide>::CRTP;

    std::string name() const override { return "oxide"; }
};

// pattern Visitor
struct IVisitor
{
    virtual void visit(const Substrate<NType>&) = 0;
    virtual void visit(const Substrate<PType>&) = 0;
    virtual void visit(const Diffusion<NType>&) = 0;
    virtual void visit(const Diffusion<PType>&) = 0;
    virtual void visit(const Oxide&)            = 0;
    virtual void visit(const PolySilicon&)      = 0;
    virtual void visit(const Metal<1>&)         = 0;
    virtual void visit(const Rect& r)           = 0;
    virtual void visit(const Polygon& r)        = 0;

    virtual ~IVisitor() = default;
};

class JSONSerializer final : public IVisitor
{
public:
    void serialize(unsigned int id, const std::string& name)
    {
        JSON json {
            { "id",       id   },
            { "name",     name },
        };

        m_json.push_back(json);
    }

    void visit(const Substrate<NType>& nSubstrate) override
    {
        serialize(nSubstrate.id, nSubstrate.name());
    }

    void visit(const Substrate<PType>& pSubstrate) override
    {
        serialize(pSubstrate.id, pSubstrate.name());
    }

    void visit(const Diffusion<NType>& nDiffusion) override
    {
        serialize(nDiffusion.id, nDiffusion.name());
    }

    void visit(const Diffusion<PType>& pDiffusion) override 
    {
        serialize(pDiffusion.id, pDiffusion.name());
    }

    void visit(const Oxide& oxide) override 
    {
        serialize(oxide.id, oxide.name());
    }

    void visit(const PolySilicon& polysilicon) override
    {
        serialize(polysilicon.id, polysilicon.name());
    }

    void visit(const Metal<1>& metal) override
    {
        serialize(metal.id, metal.name());
    }

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

    void visit(const Polygon& polygon) override
    {
        JSON polygonJson {
            { "id",       polygon.id   },
            { "geometry", "polygon"    },
            { "points",   polygonJson.array() }
        };

        for (const auto& p : polygon.m_points)
        {
            polygonJson["points"].push_back({
                { "x", p.x },
                { "y", p.y }
            });
        }

        m_json.push_back(polygonJson);
    }

    JSON getJson() const { return m_json; }

private:
    JSON m_json = JSON::array();
};

inline void Rect::accept(IVisitor& visitor)
{
    visitor.visit(*this);
}

inline void Polygon::accept(IVisitor& visitor)
{
    visitor.visit(*this);
}

#endif // LAYERS_HPP
