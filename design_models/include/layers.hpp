#ifndef LAYERS_HPP
#define LAYERS_HPP

#include <vector>
#include <memory>
#include <algorithm>
#include "concepts.hpp"
#include "geometry.hpp"

// We are use composite pattern to handle 
// the different groups of layers and separate
// layers the same way, so in our case the layer is
// a leaf, and the Circuit layout is a composite 

class AComponent
{
public:
    virtual ~AComponent() = default;
    virtual void move(int dx, int dy) = 0;

    unsigned int id;

protected:
    AComponent(unsigned int id) : id(id) { }
};

class CircuitLayout : public AComponent 
{
public:
    CircuitLayout(unsigned int id, const std::vector<std::unique_ptr<AComponent>>& components) :
        AComponent(id), m_components(components) { }

    void add(std::unique_ptr<AComponent> c) 
    {
        m_components.push_back(std::move(c));
    }

    void remove(unsigned int id)
    {
        if (auto it = std::ranges::find_if(m_components, [&](std::unique_ptr<AComponent>& c) {
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

private:
    // we use unique_ptr to avoid slicing
    std::vector<std::unique_ptr<AComponent>> m_components;
};

struct Layer : public AComponent
{
public:
    void move(int dx, int dy) override
    {
        m_shape->move(dx, dy);
    }

    void setShape(std::unique_ptr<IShape> s) noexcept { m_shape = std::move(s); }
    IShape* getShape() const noexcept { return m_shape.get(); }

protected:
    Layer(unsigned int id, std::unique_ptr<IShape> shape) noexcept : 
        AComponent(id), m_shape(std::move(shape)) { }

private:
    std::unique_ptr<IShape> m_shape;
};

template<SemiconductorType Type>
class Semiconductor : Layer 
{
protected:
    Semiconductor(unsigned int id, std::unique_ptr<IShape> shape) : Layer(id, shape) { }
};

template<SemiconductorType Type>
class Substrate final : public Semiconductor<Type> 
{
public:
    Substrate(unsigned int id, std::unique_ptr<IShape> shape) noexcept : Semiconductor<Type>(id, shape) { }
};

template<SemiconductorType Type>
class Diffusion final : public Semiconductor<Type> 
{
public:
    Diffusion(unsigned int id, std::unique_ptr<IShape> shape) noexcept : Semiconductor<Type>(id, shape) { }
};

template<size_t N>
class Metal final : public Layer
{
public:
    Metal(unsigned int id, int g, Polygon path) noexcept :
        m_gap(g), AComponent(id), m_path(path) { }

    void addToBack(Point point) 
    {
        m_path.m_points.push_back(point);
    }

    void removeFromRear()
    {
        m_path.m_points.pop_back();
    }

    void removeById(size_t id)
    {
        if (auto it = std::ranges::find_if(m_path, [&](Point node){
            return node.id == id;
        }); it != m_path.m_points.end())
        {
            m_path.m_points.erase(it);
        }
    }

    int getGap() const noexcept { return m_gap; }
    void setGap(int g) { m_gap = g; }

private:
    int m_gap;  
    Polygon m_path;
};

template<size_t N, size_t M>
class Via final : public Layer 
{
public:
    Via(unsigned int id, std::unique_ptr<IShape> shape) noexcept : 
        Layer(id, std::move(shape)) { }    
};

class PolySilicon final : public Layer 
{
public:
    PolySilicon(unsigned int id, std::unique_ptr<IShape> shape) noexcept : 
        Layer(id, std::move(shape)) { }    
};

class Oxide final : public Layer
{
public:
    Oxide(unsigned int id, std::unique_ptr<IShape> shape) noexcept : 
        Layer(id, std::move(shape)) { }    
};

#endif // LAYERS_HPP