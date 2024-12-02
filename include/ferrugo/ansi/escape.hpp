#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

namespace ferrugo
{
namespace ansi
{

struct args_t : public std::vector<int>
{
    using base_t = std::vector<int>;
    using base_t::base_t;

    friend std::ostream& operator<<(std::ostream& os, const args_t& item)
    {
        os << "\033[";
        for (std::size_t i = 0; i < item.size(); ++i)
        {
            if (i != 0)
            {
                os << ";";
            }
            os << item[i];
        }
        os << "m";
        return os;
    }
};

}  // namespace ansi
}  // namespace ferrugo