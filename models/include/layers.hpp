#ifndef LAYERS_HPP
#define LAYERS_HPP

// tags (N/P)
struct NType;
struct PType;

struct Layer { }

template<typename Type, typename DopantMaterial>
struct SemiConductor : Layer { }

template<typename Type, typename DopantMaterial>
struct Substrate : public SemiConductor<Type, DopantMaterial> { };

template<typename Type, typename DopantMaterial>
struct Well : public SemiConductor<Type, DopantMaterial> { };

template<typename Type, typename DopantMaterial>
struct Diffusion : public SemiConductor<Type, DopantMaterial> { };

template<typename Type, typename DopantMaterial>
struct Implant : public SemiConductor<Type, DopantMaterial> { };

template<size_t N>
struct Metal : public Layer { };

template<size_t N, size_t M>
struct Via { };

template<typename Type, typename DopantMaterial>
struct Source : public Diffusion<Type, DopantMaterial> { };

template<typename Type, typename DopantMaterial>
struct Drain : public Diffusion<Type, DopantMaterial> { };

struct Gate : public Layer { };

struct PolySilicon : public Layer { };

#endif // LAYERS_HPP
