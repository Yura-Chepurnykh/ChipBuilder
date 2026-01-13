#include "token.hpp"

std::ostream& operator<<(std::ostream& stream, Type type)
{
    switch (type)
    {
        case Type::Word: return stream << "Word";
        case Type::DashWord: return stream << "DashWord";
        case Type::DoubleDashWord: return stream << "DoubleDashWord";
        case Type::String: return stream << "String";
        default: return stream << "Unknown token";
    }
}

std::ostream& operator<<(std::ostream& stream, const Token& token)
{
    return stream << "(" << token.pos << ", " << token.content << ", " << token.type << ")"; 
}
