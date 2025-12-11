#include <ferrugo/ansi3/stream.hpp>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <string>

constexpr inline auto quoted = [](std::string_view text) -> ansi::stream_t
{
    return ansi::set_style({ ansi::bright_color_t{ ansi::basic_color_t::yellow }, {}, ansi::font_t::italic })(
        ansi::line("\"", text, "\""));
};

template <class T>
auto fmt(const T& value) -> ansi::stream_t
{
    if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view> || std::is_same_v<T, const char*>)
    {
        return ansi::stream_t{} << "\"" << value << "\"";
    }
    else
    {
        return ansi::stream_t{} << value;
    }
}

constexpr inline struct eq_fn
{
    template <class T>
    struct impl_t
    {
        T m_expected;

        void operator()(ansi::stream_t& stream) const
        {
            stream << ":eq " << fmt(m_expected);
        }

        template <class U>
        auto operator()(U actual) const -> ansi::stream_t
        {
            if (actual == m_expected)
            {
                return ansi::set_style({ ansi::basic_color_t::green, {}, ansi::font_t::bold })(*this);
            }
            return ansi::set_style({ ansi::basic_color_t::white, ansi::basic_color_t::red, ansi::font_t::bold })(
                ansi::line(":expected ", *this, ", :actual ", fmt(actual)));
        }
    };

    template <class T>
    auto operator()(T expected) const
    {
        return impl_t<T>{ std::move(expected) };
    }
} eq{};

int main()
{
    ansi::stream_t stream;
    const std::string name = "example.txt";
    const auto braces = ansi::set_style({ ansi::basic_color_t::cyan, ansi::basic_color_t::yellow, ansi::font_t::bold });
    const auto underlined_text = ansi::set_style({ ansi::palette_color_t::from_grayscale(12), {}, ansi::font_t::underline });
    const auto red_style = ansi::set_style(
        { ansi::bright_color_t{ ansi::basic_color_t::white }, ansi::basic_color_t::red, ansi::font_t::underline });
    const auto green_style = ansi::set_style({ ansi::basic_color_t::green, {}, ansi::font_t::bold });

    const auto get_style = [](int v) -> ansi::font_style_t
    {
        if (v < 40)
        {
            return ansi::font_style_t{ ansi::bright_color_t{ ansi::basic_color_t::green }, {}, {} };
        }
        if (v > 80)
        {
            return ansi::font_style_t{ ansi::basic_color_t::red, {}, ansi::font_t::bold };
        }
        return ansi::font_style_t{};
    };

    const auto render_item
        = [&](int v) -> ansi::stream_t { return ansi::set_style(get_style(v))(ansi::line("* Item: ", v)); };

    stream << braces(ansi::line("{"))
           << ansi::indented(
                  ansi::line(":file ", name),
                  red_style(ansi::line(":size ", 12345)),
                  green_style(ansi::line(":lines ", 100)),
                  ansi::indented(
                      ansi::line(quoted("This is an indented block.")),
                      ansi::line("It has multiple lines."),
                      underlined_text(ansi::indented(
                          ansi::line("This is a nested ", "indented", " block."), ansi::line("It is further indented."))),
                      ansi::line("Back to the first indented block."),
                      ansi::line("Περιοίϰεον δέ σϕεας τὰ πολλὰ τῶν χώρων τοῦτον τὸν χρόνον")),
                  ansi::line("End of file metadata."))
           << braces(ansi::line("}")) << ansi::map(render_item, std::vector<int>{ 10, 90, 20, 30, 100, 0, 30, 80 });

    for (int r = 0; r < 6; ++r)
    {
        for (int g = 0; g < 6; ++g)
        {
            for (int b = 0; b < 6; ++b)
            {
                stream << ansi::set_style({ {}, ansi::palette_color_t::from_rgb(r, g, b), {} })(ansi::text("  "));
            }
            stream << ansi::text(" ");
        }
        stream << ansi::new_line;
    }

    const auto matcher = eq(42);

    for (int i : { 10, 42, 100 })
    {
        stream << matcher(i) << ansi::new_line;
    }
    // std::cout << stream << std::endl;
    ansi::render(std::cout, stream);
}
