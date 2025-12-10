#pragma once

#include <ferrugo/ansi2/output.hpp>
#include <ferrugo/ansi2/style_appliers.hpp>
#include <functional>

namespace ansi
{

inline auto indent_by(std::size_t n) -> output_applier_t
{
    return [=](output_t& out) { out.indent(n); };
}

static const inline auto indent = indent_by(2);
static const inline auto unindent = output_applier_t{ [](output_t& out) { out.unindent(); } };

static const inline auto new_line = output_applier_t{ [](output_t& out) { out.new_line(); } };
static const inline auto pop_font_style = output_applier_t{ [](output_t& out) { out.pop_font_style(); } };

inline auto push_font_style(font_style_t style) -> output_applier_t
{
    return [=](output_t& out) { out.push_font_style(style); };
}

inline auto modify_font_style(font_style_applier_t applier) -> output_applier_t
{
    return [=](output_t& out) { out.modify_font_style(applier); };
}

}  // namespace ansi
