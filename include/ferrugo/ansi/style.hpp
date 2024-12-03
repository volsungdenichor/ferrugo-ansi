#pragma once

#include <ferrugo/ansi/color.hpp>
#include <ferrugo/ansi/font.hpp>

namespace ferrugo
{
namespace ansi
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

}  // namespace ansi
}  // namespace ferrugo
