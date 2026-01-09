#include "enums.hpp"

std::ostream& operator<<(std::ostream& stream, Dopant dopant)
{
    switch (dopant)
    {
        case Dopant::Arsenic: return stream << "Arsenic";
        case Dopant::Phosphorus: return stream << "Phosphorus";
        case Dopant::Boron: return stream << "Boron";
        case Dopant::Antimony: return stream << "Antimony";
        default: return stream << "Unknown Dopant";
    }
}

std::ostream& operator<<(std::ostream& stream, Carrier carrier)
{
    switch (carrier)
    {
        case Carrier::Hole: return stream << "Hole";
        case Carrier::Electron: return stream << "Electron";
        default: return stream << "Unknown Carrier";
    }
}