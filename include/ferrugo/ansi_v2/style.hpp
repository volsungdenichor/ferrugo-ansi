#pragma once

#include <ferrugo/ansi_v2/color.hpp>
#include <ferrugo/ansi_v2/font.hpp>

namespace ferrugo
{
namespace ansi_v2
{

struct style_t
{
    color_t foreground = default_color_t{};
    color_t background = default_color_t{};
    font_t font = font_t::none;

    style_t() = default;
    style_t(const style_t&) = default;
    style_t(style_t&&) = default;

    friend std::ostream& operator<<(std::ostream& os, const style_t& item)
    {
        return os << "((foreground " << item.foreground << ") (background " << item.background << ") (font " << item.font
                  << "))";
    }
};

}  // namespace ansi_v2
}  // namespace ferrugo
