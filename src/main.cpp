#include <ferrugo/ansi/colors.hpp>
#include <ferrugo/ansi/glyph.hpp>
#include <ferrugo/ansi_v2/color.hpp>
#include <ferrugo/ansi_v2/default_context.hpp>
#include <ferrugo/ansi_v2/font.hpp>
#include <ferrugo/core/demangle.hpp>
#include <ferrugo/core/ostream_utils.hpp>
#include <ferrugo/core/pipe.hpp>
#include <ferrugo/core/std_ostream.hpp>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <string>

int main()
{
    using namespace ferrugo::ansi_v2;
    auto ctx = default_context_t{ std::cout };

    ctx.write_text(mb_string{ "❎Ala ma " });
    ctx.push_style(style_t{ "FF0000", {}, font_t::underline });
    ctx.write_text(mb_string{ "kota" });
    ctx.pop_style();
    ctx.write_text(mb_string{ "..." });
}
