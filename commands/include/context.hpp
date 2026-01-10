#ifndef CONTEXT_HPP 
#define CONTEXT_HPP

#include <memory>
#include <vector>
#include "layers.hpp"
#include "circuits.hpp"

// TEMP: may be needed later
// struct IContext
// {
//     virtual ~IContext() = default;
// };

// template<typename ContextType>
// struct Context : public IContext;

// template<typename Type, typename DopantMaterial, size_t N>
// struct Context<PMOS>
// {
//     Substrate<Type, DopantMaterial, N> substrate;
//     Drain<Type, DopantMaterial, N> drain;
//     Source<Type, DopantMaterial, N> source;
//     Gate<N> gate;
// };

// template<>
// struct Context<Circuit>
// {
//     std::vector<std::unique_ptr<IContext>> contexts;
// };

template <
    typename MOSType,
    typename BodyDopant = Boron,
    typename SourceDrainDopant = Phosphorus
>
struct MOSContext 
{
    using Traits = MosLayersTraits<MOSType, BodyDopant, SourceDrainDopant>;

    typename Traits::SubstrateType substrate;
    typename Traits::SourceType source;
    typename Traits::DrainType drain;
    typename Traits::GateType gate;
};

// pattern builder
template <
    typename MOSType,
    typename BodyDopant = Boron,
    typename SourceDrainDopant = Phosphorus
>
class MOSBuilder 
{
    using Context = MOSContext<MOSType, BodyDopant, SourceDrainDopant>;

    MOSBuilder& setSubstrate()
    {
        return *this;
    }

    MOSBuilder& setDrain()
    {
        return *this;
    }

    MOSBuilder& setSource()
    {
        return *this;
    }

    MOSBuilder& setGate()
    {
        return *this;
    }

    Context build() 
    {
        return m_context;
    }

private:
    Context m_context;
};

#endif // CONTEXT_HPP
