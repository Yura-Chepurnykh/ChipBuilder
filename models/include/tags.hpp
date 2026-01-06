#ifndef TAGS_HPP
#define TAGS_HPP

// tags (N/P)
struct N;
struct P;

template<size_t N>
struct Layer { }

template<typename Type, typename DopantMaterial, size_t N>
struct SemiConductor : Layer<N> { }

template<typename Type, typename DopantMaterial, size_t N>
struct Substrate : public SemiConductor<Type, DopantMaterial, N> { };

template<typename Type, typename DopantMaterial, size_t N>
struct Well : public SemiConductor<Type, DopantMaterial, N> { };

template<typename Type, typename DopantMaterial, size_t N>
struct Diffusion : public SemiConductor<Type, DopantMaterial, N> { };

template<typename Type, typename DopantMaterial, size_t N>
struct Implant : public SemiConductor<Type, DopantMaterial, N> { };

template<size_t N>
struct Metal : public Layer<N> { };

template<size_t N, size_t M>
struct Via { };

template<typename Type, typename DopantMaterial, size_t N>
struct Source : public Diffusion<Type, DopantMaterial, N> { };

template<typename Type, typename DopantMaterial, size_t N>
struct Drain : public Diffusion<Type, DopantMaterial, N> { };

template<size_t N>
struct Gate : public Layer<N> { };

#endif // TAGS_HPP
