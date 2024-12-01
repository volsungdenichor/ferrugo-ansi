#pragma once

#include <ferrugo/ansi_v2/color.hpp>
#include <ferrugo/ansi_v2/font.hpp>

namespace ferrugo
{
namespace ansi_v2
{

struct style_t
{
    color_t foreground;
    color_t background;
    font_t font;

    friend std::ostream& operator<<(std::ostream& os, const style_t& item)
    {
        return os << "((foreground " << item.foreground << ") (background " << item.background << ") (font " << item.font
                  << "))";
    }
};

}  // namespace ansi_v2
}  // namespace ferrugo
