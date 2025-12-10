#pragma once

#include <cuchar>
#include <ferrugo/ansi2/output.hpp>
#include <ostream>
#include <set>
#include <vector>

namespace ansi
{

template <int Base>
struct color_visitor_t
{
    auto operator()(default_color_t) const -> std::vector<int>
    {
        return { Base + 39 };
    }

    auto operator()(standard_color_t col) const -> std::vector<int>
    {
        return { static_cast<int>(col.m_color) + (Base + 30) };
    }

    auto operator()(bright_color_t col) const -> std::vector<int>
    {
        return { static_cast<int>(col.m_color) + (Base + 90) };
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

struct font_diff_t
{
    std::vector<int> m_args;

    font_diff_t(font_t old_font, font_t new_font)
    {
        std::set<font_t> enabled;
        std::set<font_t> disabled;
        for (const font_t f : { font_t::standout,
                                font_t::bold,
                                font_t::dim,
                                font_t::italic,
                                font_t::underline,
                                font_t::blink,
                                font_t::inverse,
                                font_t::hidden,
                                font_t::crossed_out,
                                font_t::double_underline })
        {
            const auto old_contains = old_font.contains(f);
            const auto new_contains = new_font.contains(f);
            if (old_contains && !new_contains)
            {
                disabled.insert(f);
            }
            if (!old_contains && new_contains)
            {
                enabled.insert(f);
            }
        }

        const auto handle = [&](font_t f, int on_set, int on_reset)
        {
            if (enabled.count(f))
            {
                m_args.push_back(on_set);
            }
            if (disabled.count(f))
            {
                m_args.push_back(on_reset);
            }
        };

        handle(font_t::bold, 1, 21);
        handle(font_t::dim, 2, 22);
        handle(font_t::italic, 3, 23);
        handle(font_t::underline, 4, 24);
        handle(font_t::blink, 5, 25);
        handle(font_t::crossed_out, 9, 29);
        handle(font_t::hidden, 8, 28);
    }

    friend std::ostream& operator<<(std::ostream& os, const font_diff_t& item)
    {
        if (!item.m_args.empty())
        {
            write_args(os, item.m_args);
        }
        return os;
    }
};

struct ostream_output_t : public output_t::interface
{
    std::ostream& m_os;
    std::vector<std::size_t> m_indents;
    std::vector<font_style_t> m_styles;
    bool m_new_line_needed;

    explicit ostream_output_t(std::ostream& os)
        : m_os{ os }
        , m_indents{ 0 }
        , m_styles{ { font_style_t{} } }
        , m_new_line_needed{ false }
    {
    }

    void indent(std::size_t n) override
    {
        m_indents.push_back(m_indents.back() + n);
    }

    void unindent() override
    {
        if (!m_indents.empty())
        {
            m_indents.pop_back();
        }
    }

    void new_line() override
    {
        m_new_line_needed = true;
        m_os << std::endl;
    }

    void put(char32_t ch) override
    {
        if (m_new_line_needed)
        {
            m_os << std::string(m_indents.back(), ' ');
            m_new_line_needed = false;
        }
        std::array<char, 4> data;
        auto state = std::mbstate_t{};
        const std::size_t size = std::c32rtomb(data.data(), ch, &state);
        if (size == std::size_t(-1))
        {
            throw std::runtime_error{ "u32_to_mb: error in conversion " + std::to_string(ch) };
        }
        for (std::size_t i = 0; i < size; ++i)
        {
            m_os << data[i];
        }
    }

    void flush() override
    {
        if (m_new_line_needed)
        {
            m_os << std::endl;
            m_new_line_needed = false;
        }
    }

    font_style_t font_style() const override
    {
        return m_styles.back();
    }

    void push_font_style(const font_style_t& style) override
    {
        const auto old_style = font_style();
        m_styles.push_back(style);
        change_style(old_style, font_style());
    }

    void pop_font_style() override
    {
        const auto old_style = font_style();
        m_styles.pop_back();
        change_style(old_style, font_style());
    }

    void change_style(const font_style_t& old_style, const font_style_t& new_style)
    {
        if (old_style.font != new_style.font)
        {
            m_os << font_diff_t{ old_style.font, new_style.font };
        }
        if (old_style.foreground != new_style.foreground)
        {
            write_args(m_os, std::visit(color_visitor_t<0>{}, new_style.foreground.m_data));
        }
        if (old_style.background != new_style.background)
        {
            write_args(m_os, std::visit(color_visitor_t<10>{}, new_style.background.m_data));
        }
    }
};

}  // namespace ansi
