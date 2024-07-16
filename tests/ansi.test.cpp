#include <ferrugo/ansi/ansi.hpp>

template <class... Args>
std::string format(ferrugo::ansi::glyph_style_t style, Args&&... args)
{
    std::stringstream ss;
    (ss << ... << args);
    const std::string content = ss.str();
    ferrugo::ansi::area_t area{ { static_cast<int>(content.size()), 1 } };
    for (std::size_t i = 0; i < content.size(); ++i)
    {
        auto& cell = area.mut_ref()[{ static_cast<int>(i), 0 }];
        cell.character = content[i];
        cell.style = style;
    }
    return ferrugo::ansi::render(area);
}

int main()
{
    using namespace ferrugo;
    ansi::area_t area{ { 10, 5 } };
    const auto ch = ansi::multibyte_string{ "▞" }[0];
    area.mut_ref().fill(ansi::glyph_t{ ch | ansi::fg(ansi::basic_color_t::red) });
    std::cout << ansi::render(area) << std::flush;
    std::cout << format(ansi::fg(ansi::basic_color_t::red) | ansi::bg(ansi::basic_color_t::yellow), "Ala ma kota")
              << std::endl;
}
