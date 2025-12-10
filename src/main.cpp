#include <ferrugo/ansi3/font_style.hpp>
#include <ferrugo/ansi3/stream.hpp>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <string>

template <int Base>
struct color_visitor_t
{
    auto operator()(default_color_t) const -> std::vector<int>
    {
        return { Base + 39 };
    }

    auto operator()(standard_color_t col) const -> std::vector<int>
    {
        return { Base + 30 + static_cast<int>(col.m_color) };
    }

    auto operator()(bright_color_t col) const -> std::vector<int>
    {
        return { Base + 90 + static_cast<int>(col.m_color) };
    }

    auto operator()(palette_color_t col) const -> std::vector<int>
    {
        return { Base + 38, 5, col.m_index };
    }

    auto operator()(rgb_color_t col) const -> std::vector<int>
    {
        return { Base + 38, 2, col[0], col[1], col[2] };
    }
};

inline void write_args(std::ostream& os, const std::vector<int>& args)
{
    os << "\033[";
    for (std::size_t i = 0; i < args.size(); ++i)
    {
        if (i != 0)
        {
            os << ";";
        }
        os << args[i];
    }
    os << "m";
}

inline std::string write_args(const std::vector<int>& args)
{
    std::stringstream ss;
    write_args(ss, args);
    return ss.str();
}

inline std::string change_style(const font_style_t& old_style, const font_style_t& new_style)
{
    std::stringstream ss;
    if (old_style.foreground != new_style.foreground)
    {
        write_args(ss, std::visit(color_visitor_t<0>{}, new_style.foreground.m_data));
    }
    if (old_style.background != new_style.background)
    {
        write_args(ss, std::visit(color_visitor_t<10>{}, new_style.background.m_data));
    }
    if (old_style.font != new_style.font)
    {
        static const std::vector<std::pair<font_t, int>> set_map = {
            { font_t::bold, 1 },      { font_t::dim, 2 },         { font_t::italic, 3 },
            { font_t::underline, 4 }, { font_t::blink, 5 },       { font_t::inverse, 7 },
            { font_t::hidden, 8 },    { font_t::crossed_out, 9 }, { font_t::double_underline, 21 },
        };
        std::vector<int> args = { 22, 23, 24, 25, 27, 28, 29 };
        for (const auto& [f, v] : set_map)
        {
            if (new_style.font.contains(f))
            {
                args.push_back(v);
            }
        }
        write_args(ss, args);
    }
    return ss.str();
}

inline void render(std::ostream& out, const stream_t& stream)
{
    struct context_t
    {
        std::ostream& os;
        int indent_level = 0;
        bool new_line = false;
        std::vector<font_style_t> style_stack = { font_style_t{} };
    };

    struct visitor_t
    {
        context_t& m_ctx;

        void operator()(new_line_t) const
        {
            m_ctx.new_line = true;
        }

        void operator()(indent_t) const
        {
            m_ctx.indent_level += 1;
        }

        void operator()(unindent_t) const
        {
            m_ctx.indent_level = std::max(0, m_ctx.indent_level - 1);
        }

        void operator()(const text_t& v) const
        {
            handle_indent();
            m_ctx.os << v.content;
        }

        void operator()(const text_ref_t& v) const
        {
            handle_indent();
            m_ctx.os << v.content;
        }

        void operator()(const push_style_t& v) const
        {
            const font_style_t previous_style = m_ctx.style_stack.back();
            m_ctx.style_stack.push_back(v.style);
            m_ctx.os << change_style(previous_style, m_ctx.style_stack.back());
        }

        void operator()(const modify_style_t& v) const
        {
            const font_style_t previous_style = m_ctx.style_stack.back();
            font_style_t new_style = previous_style;
            v.applier(new_style);
            m_ctx.style_stack.push_back(new_style);
            m_ctx.os << change_style(previous_style, m_ctx.style_stack.back());
        }

        void operator()(pop_style_t) const
        {
            const font_style_t previous_style = m_ctx.style_stack.back();
            m_ctx.style_stack.pop_back();
            m_ctx.os << change_style(previous_style, m_ctx.style_stack.back());
        }

        void handle_indent() const
        {
            if (m_ctx.new_line)
            {
                m_ctx.os << write_args({ 0 });
                m_ctx.os << "\n" << std::string(m_ctx.indent_level * 2, ' ');
                m_ctx.new_line = false;
                m_ctx.os << change_style({}, m_ctx.style_stack.back());
            }
        }
    };

    context_t context{ out, 0, false };
    for (const auto& op : stream.m_ops)
    {
        std::visit(visitor_t{ context }, op);
    }
    if (context.new_line)
    {
        out << "\n";
    }
}

constexpr inline auto set_style = [](font_style_t style)
{
    return [style](auto&&... ops) -> stream_t
    {
        stream_t stream;
        stream << push_style(style);
        (stream << ... << std::forward<decltype(ops)>(ops));
        stream << pop_style;
        return stream;
    };
};

constexpr inline auto add_style = [](font_style_applier_t applier)
{
    return [applier](auto&&... ops) -> stream_t
    {
        stream_t stream;
        stream << modify_style(applier);
        (stream << ... << std::forward<decltype(ops)>(ops));
        stream << pop_style;
        return stream;
    };
};

template <class Func, class Range>
constexpr inline auto map(Func&& func, Range&& range) -> stream_t
{
    stream_t stream;
    for (auto&& item : range)
    {
        stream << func(std::forward<decltype(item)>(item));
    }
    return stream;
}

constexpr inline auto quoted = [](std::string_view text) -> stream_t {
    return set_style({ bright_color_t{ basic_color_t::yellow }, {}, font_t::italic })(line("\"", text, "\""));
};

int main()
{
    stream_t stream;
    const std::string name = "example.txt";
    const auto braces = set_style({ basic_color_t::cyan, basic_color_t::yellow, font_t::bold });
    const auto underlined_text = set_style({ palette_color_t::from_grayscale(12), {}, font_t::underline });
    const auto red_style = set_style({ bright_color_t{ basic_color_t::white }, basic_color_t::red, font_t::underline });
    const auto green_style = set_style({ basic_color_t::green, {}, font_t::bold });

    const auto get_style = [](int v) -> font_style_t
    {
        if (v < 40)
        {
            return font_style_t{ bright_color_t{ basic_color_t::green }, {}, {} };
        }
        if (v > 80)
        {
            return font_style_t{ basic_color_t::red, {}, font_t::bold };
        }
        return font_style_t{};
    };

    const auto render_item = [&](int v) -> stream_t { return set_style(get_style(v))(line("⦿ Item: ", v)); };

    stream << braces(line("{"))
           << indented(
                  line(":file ", name),
                  red_style(line(":size ", 12345)),
                  green_style(line(":lines ", 100)),
                  indented(
                      line(quoted("This is an indented block.")),
                      line("It has multiple lines."),
                      underlined_text(
                          indented(line("This is a nested ", "indented", " block."), line("It is further indented."))),
                      line("Back to the first indented block.")),
                  line("End of file metadata."))
           << braces(line("}")) << map(render_item, std::vector<int>{ 10, 90, 20, 30, 100, 0, 30, 80 });
    // std::cout << stream << std::endl;
    render(std::cout, stream);
}
