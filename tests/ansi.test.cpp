#include <ferrugo/ansi/ansi.hpp>

int main()
{
    using namespace ferrugo;
    ansi::area_t area{ { 10, 5 } };
    const auto ch = ansi::multibyte_string{ "▞" }[0];
    area.mut_ref().fill(ansi::glyph_t{ ch | ansi::fg(ansi::basic_color_t::red) });
    std::cout << ansi::render(area) << std::flush;
}