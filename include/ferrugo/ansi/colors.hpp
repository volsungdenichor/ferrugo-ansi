#pragma once

#include <cstdint>
#include <ferrugo/core/overloaded.hpp>
#include <iostream>
#include <stdexcept>
#include <variant>

namespace ferrugo
{
namespace ansi
{

struct true_color_t
{
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;

    friend bool operator==(const true_color_t& lhs, const true_color_t& rhs)
    {
        return std::tie(lhs.red, lhs.green, lhs.blue) == std::tie(rhs.red, rhs.green, rhs.blue);
    }

    friend bool operator!=(const true_color_t& lhs, const true_color_t& rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const true_color_t& item)
    {
        return os << "(rgb " << static_cast<int>(item.red) << " " << static_cast<int>(item.green) << " "
                  << static_cast<int>(item.blue) << ")";
    }
};

struct default_color_t
{
    friend bool operator==(const default_color_t&, const default_color_t&)
    {
        return true;
    }

    friend bool operator!=(const default_color_t&, const default_color_t&)
    {
        return false;
    }

    friend std::ostream& operator<<(std::ostream& os, const default_color_t&)
    {
        return os << "default";
    }
};

struct palette_color_t
{
    std::uint8_t index;

    friend bool operator==(const palette_color_t& lhs, const palette_color_t& rhs)
    {
        return lhs.index == rhs.index;
    }

    friend bool operator!=(const palette_color_t& lhs, const palette_color_t& rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const palette_color_t& item)
    {
        return os << "(palette " << static_cast<int>(item.index) << ")";
    }
};

enum class basic_color_t
{
    black = 0,
    dark_red = 1,
    dark_green = 2,
    dark_yellow = 3,
    dark_blue = 4,
    dark_magenta = 5,
    dark_cyan = 6,
    gray = 7,
    dark_gray = 60,
    red = 61,
    green = 62,
    yellow = 63,
    blue = 64,
    magenta = 65,
    cyan = 66,
    white = 67,
};

inline std::ostream& operator<<(std::ostream& os, basic_color_t item)
{
#define CASE(v) \
    case basic_color_t::v: return os << #v
    switch (item)
    {
        CASE(black);
        CASE(dark_red);
        CASE(dark_green);
        CASE(dark_yellow);
        CASE(dark_blue);
        CASE(dark_magenta);
        CASE(dark_cyan);
        CASE(gray);
        CASE(dark_gray);
        CASE(red);
        CASE(green);
        CASE(yellow);
        CASE(blue);
        CASE(magenta);
        CASE(cyan);
        CASE(white);
        default: throw std::runtime_error{ "unknown basic_color_t" };
    }
#undef CASE
    return os;
}

using color_t = std::variant<palette_color_t, true_color_t, default_color_t, basic_color_t>;

inline bool operator==(const color_t& lhs, const color_t& rhs)
{
    return std::visit(
        ferrugo::core::overloaded{ [](const true_color_t& lhs, const true_color_t& rhs) { return lhs == rhs; },
                                   [](const palette_color_t& lhs, const palette_color_t& rhs) { return lhs == rhs; },
                                   [](const default_color_t& lhs, const default_color_t& rhs) { return lhs == rhs; },
                                   [](const basic_color_t& lhs, const basic_color_t& rhs) { return lhs == rhs; },
                                   [](const auto& lhs, const auto& rhs) { return false; } },
        lhs,
        rhs);
}

inline bool operator!=(const color_t& lhs, const color_t& rhs)
{
    return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& os, const color_t& item)
{
    std::visit([&](const auto& c) { os << c; }, item);
    return os;
}

}  // namespace ansi
}  // namespace ferrugo