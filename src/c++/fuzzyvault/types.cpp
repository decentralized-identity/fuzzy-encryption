#include "types.h"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <memory.h>
#include "json.h"
#include "exceptions.h"


std::ostream& operator<<(std::ostream& os, const std::vector<int>& xs)
{
    bool is_first = true;
    os << "[";
    for (int x: xs)
    {
        if (!is_first)
            os << ",";
        else 
            is_first = false;
        os << " " << x;
    }
    os << " ]";
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<uint8_t>& xs)
{
        for (uint8_t x : xs)
            os 
                << std::uppercase 
                << std::hex
                << std::setw(2)
                << std::setfill('0') 
                << (int)x;
        return os;
}

std::string loads(const std::string& path)
{
    std::ifstream input(path);
    std::string output;
    std::getline(input, output, (char) input.eof());
    return output;
}

void pushback_int(int t, std::vector<uint8_t>& out)
{
    const uint8_t* p = (const uint8_t*)&t;
    for (size_t i = 0; i < sizeof(t); i++)
        out.push_back(p[i]);
}
