#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <iostream>

enum class Dopant 
{
    Boron, 
    Phosphorus,
    Arsenic, 
    Antimony
};

enum class Carrier 
{
    Hole, 
    Electron
};

std::ostream& operator<<(std::ostream&, Dopant);
std::ostream& operator<<(std::ostream&, Carrier);

#endif // ENUMS_HPP