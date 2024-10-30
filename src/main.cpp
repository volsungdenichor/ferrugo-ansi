#include <ferrugo/ansi/colors.hpp>
#include <ferrugo/ansi/glyph.hpp>
#include <ferrugo/ansi_v2/color.hpp>
#include <ferrugo/ansi_v2/font.hpp>
#include <ferrugo/ansi_v2/mb_string.hpp>
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
    std::cout << fg("20FF30") << "Ala" << fg(default_color_t{});
    std::cout << fg(bright_color_t::red) << "Kot" << fg(default_color_t{});
}
