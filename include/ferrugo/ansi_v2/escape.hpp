#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

namespace ferrugo
{
namespace ansi_v2
{

using args_t = std::vector<int>;

inline auto esc(const args_t& args) -> std::string
{
    std::stringstream ss;
#if 1
    ss << "\033[";
    for (std::size_t i = 0; i < args.size(); ++i)
    {
        if (i != 0)
        {
            ss << ";";
        }
        ss << args[i];
    }
    ss << "m";
#else
    ss << "\033[96m";
    ss << "{";
    for (const int v : args)
    {
        ss << " " << v;
    }
    ss << " }";
#endif
    return ss.str();
}

}  // namespace ansi_v2
}  // namespace ferrugo