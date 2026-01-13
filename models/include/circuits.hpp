#ifndef CIRCUITS_HPP
#define CIRCUITS_HPP

#include <type_traits>
#include "layers.hpp"
#include "dopant_material.hpp"

struct NMOS
{
    using BodyType = PType;
    using ChannelType = NType; 
};

struct PMOS
{
    using BodyType = NType;
    using ChannelType = PType;
};

struct CMOS;
struct CustomMos;

template<typename MOSType, typename BodyDopant = Boron, typename SourceDrainDopant = Phosphorus>
struct MOSLayersTraits
{
    using SubstrateType = Substrate<typename MOSType::BodyType, BodyDopant>;
    using SourceType = Source<typename MOSType::ChannelType, SourceDrainDopant>;
    using DrainType = Drain<typename MOSType::ChannelType, SourceDrainDopant>;
    using GateType = Gate;
};

#endif // CIRCUITS_HPP
