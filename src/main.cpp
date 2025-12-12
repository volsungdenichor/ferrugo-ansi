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

template <class K, class V>
auto fmt(const std::map<K, V>& m) -> ansi::stream_t
{
    ansi::stream_t stream;

    stream << ansi::line("{");

    stream << ansi::indent;

    for (const auto& [key, value] : m)
    {
        stream << ansi::line(fmt(key), ": ", fmt(value));
    }

    stream << ansi::unindent;
    stream << ansi::line("}");
    return stream;
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

template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

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

    std::stringstream ss;

    ss << "\033[2J\033[H";

    stream << braces(ansi::line("{")) << ansi::line(std::vector<bool>{ true, false })
           << ansi::line(std::tuple{ 1, 2.5, "three" }) << ansi::line(std::pair{ 2.5, 'x' })
           << fmt(std::map<std::string, std::map<std::string, int>>{ { "first", { { "a", 1 }, { "b", 2 } } },
                                                                     { "second", { { "c", 3 }, { "d", 4 } } } })
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

#if 1
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

    const auto n = 6;
    for (int r = 0; r < n; ++r)
    {
        for (int g = 0; g < n; ++g)
        {
            for (int b = 0; b < n; ++b)
            {
                stream << ansi::set_style({ {}, ansi::rgb_color_t(r * 256 / n, g * 256 / n, b * 256 / n), {} })(
                    ansi::text("  "));
            }
            stream << ansi::text(" ");
        }
        stream << ansi::new_line;
    }
#endif

    const auto matcher = eq(42);

    for (int i : { 10, 42, 100 })
    {
        stream << matcher(i) << ansi::new_line;
    }

    ansi::render(ss)(stream);

    for (const auto& op : stream.m_ops)
    {
        std::visit(
            overloaded{
                [](ansi::op_new_line_t) { std::cout << "NewLine"; },
                [](ansi::op_indent_t) { std::cout << "Indent"; },
                [](ansi::op_unindent_t) { std::cout << "Unindent"; },
                [](const ansi::op_text_t& v) { std::cout << "Text: " << v.content; },
                [](const ansi::op_text_ref_t& v) { std::cout << "TextRef: " << v.content; },
                [](const ansi::op_push_style_t& v) { std::cout << "PushStyle: " << v.style; },
                [](const ansi::op_modify_style_t& v) { std::cout << "ModifyStyle"; },
                [](ansi::op_pop_style_t) { std::cout << "PopStyle"; },
            },
            op);
        std::cout << "\n";
    }

    std::cout << ss.str() << std::flush;
    return 0;
}
