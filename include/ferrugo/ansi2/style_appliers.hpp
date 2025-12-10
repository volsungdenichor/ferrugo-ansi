#pragma once

#include <ferrugo/ansi2/output.hpp>
#include <functional>

namespace ansi
{

inline auto fg(color_t col) -> font_style_applier_t
{
    return [=](font_style_t& style) { style.foreground = col; };
}

inline auto bg(color_t col) -> font_style_applier_t
{
    return [=](font_style_t& style) { style.background = col; };
}

inline auto font(font_t font) -> font_style_applier_t
{
    return [=](font_style_t& style) { style.font = font; };
}

static const inline auto bold = font(font_t::bold);
static const inline auto italic = font(font_t::italic);
static const inline auto underline = font(font_t::underline);
static const inline auto dim = font(font_t::dim);
static const inline auto inverse = font(font_t::inverse);
static const inline auto crossed_out = font(font_t::crossed_out);
static const inline auto blink = font(font_t::blink);
static const inline auto hidden = font(font_t::hidden);

static const inline auto black = fg(basic_color_t::black);
static const inline auto red = fg(basic_color_t::red);
static const inline auto green = fg(basic_color_t::green);
static const inline auto yellow = fg(basic_color_t::yellow);
static const inline auto blue = fg(basic_color_t::blue);
static const inline auto magenta = fg(basic_color_t::magenta);
static const inline auto cyan = fg(basic_color_t::cyan);
static const inline auto white = fg(basic_color_t::white);

}  // namespace ansi
