#pragma once

#include <ferrugo/ansi2/color.hpp>
#include <ferrugo/ansi2/font.hpp>
#include <functional>

namespace ansi
{

struct font_style_t
{
    color_t foreground = default_color_t{};
    color_t background = default_color_t{};
    font_t font = font_t::none;

    friend std::ostream& operator<<(std::ostream& os, const font_style_t& item)
    {
        return os << "(font_style (foreground " << item.foreground << ") (background " << item.background << ") (font "
                  << item.font << "))";
    }
};

struct font_style_applier_t : public std::function<void(font_style_t&)>
{
    using base_t = std::function<void(font_style_t&)>;

    using base_t::base_t;
};

inline auto operator|(font_style_applier_t lhs, font_style_applier_t rhs) -> font_style_applier_t
{
    return [=](font_style_t& style)
    {
        lhs(style);
        rhs(style);
    };
}

}  // namespace ansi
