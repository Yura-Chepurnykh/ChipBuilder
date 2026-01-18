#ifndef SEMANTIC_ANALYZER_HPP
#define SEMANTIC_ANALYZER_HPP

#include <vector>
#include <algorithm>
#include <memory>
#include <map>
#include <string_view>

#include "token.hpp"
#include "parser.hpp"
#include "dopant_materials.hpp"
#include "layers.hpp"
#include "concepts.hpp"

template<typename D>
struct DopantsMap;

template<>
struct DopantsMap<NType>
{
    inline static std::map<std::string_view, std::unique_ptr<Dopant>> 
    {
        { "phosphorus", Phosphorus },
        { "arsenic", Arsenic },
        { "antimony", Antimony }
    };
};

template<>
struct DopantsMap<PType>
{
    inline static std::map<std::string_view, std::unique_ptr<Dopant>> 
    {
        { "boron", Boron }
    };
};

enum class CommandType
{

};

template<NumericType NT>
struct Rect 
{
    struct Pos
    {
        NT x, y;
    };

    Pos position;
    NT width, height;
};

struct TypifiedScheme 
{
    Command command;
    Dopant dopant;
    Rect rect;
};

class SemanticAnalyzer 
{
    SemanticAnalyzer(const CommandScheme& scheme);
    TypifiedScheme analyze();

private:
    CommandScheme m_scheme;
};

#endif // SEMANTIC_ANALYZER_HPP
