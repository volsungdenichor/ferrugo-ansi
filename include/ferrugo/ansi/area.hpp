#pragma once

#include <array>
#include <ferrugo/ansi/glyph.hpp>
#include <ferrugo/core/ostream_utils.hpp>
#include <ferrugo/core/overloaded.hpp>
#include <map>

namespace ferrugo
{
namespace ansi
{

using location_t = std::array<int, 2>;
using extent_t = std::array<int, 2>;

struct bounds_t
{
    location_t location = {};
    extent_t extent = {};
};

struct area_t
{
    struct ref_type
    {
        explicit ref_type(const glyph_t* ptr, extent_t e) : m_ptr{ ptr }, m_extent{ e }
        {
        }

        const glyph_t& operator[](location_t loc) const
        {
            return *(m_ptr + to_index(loc));
        }

        std::size_t to_index(location_t loc) const
        {
            if (((0 <= loc[0] < m_extent[0]) && (0 <= loc[1] < m_extent[1])))
            {
                return m_extent[0] * loc[1] + loc[0];
            }
            throw std::runtime_error{ "location out of bounds" };
        }

        const glyph_t* m_ptr;
        extent_t m_extent;
    };

    struct mut_ref_type
    {
        explicit mut_ref_type(glyph_t* ptr, extent_t e) : m_ptr{ ptr }, m_extent{ e }
        {
        }

        glyph_t& operator[](location_t loc) const
        {
            return *(m_ptr + to_index(loc));
        }

        std::size_t to_index(location_t loc) const
        {
            if (((0 <= loc[0] < m_extent[0]) && (0 <= loc[1] < m_extent[1])))
            {
                return m_extent[0] * loc[1] + loc[0];
            }
            throw std::runtime_error{ "location out of bounds" };
        }

        void fill(glyph_t value)
        {
            for (int y = 0; y < m_extent[1]; ++y)
            {
                for (int x = 0; x < m_extent[0]; ++x)
                {
                    (*this)[location_t{ x, y }] = value;
                }
            }
        }

        glyph_t* m_ptr;
        extent_t m_extent;
    };

    explicit area_t(extent_t e) : m_extent{ e }, m_data(e[0] * e[1], glyph_t{})
    {
    }

    ref_type ref() const
    {
        return ref_type{ m_data.data(), m_extent };
    }

    mut_ref_type mut_ref()
    {
        return mut_ref_type{ m_data.data(), m_extent };
    }

    extent_t m_extent;
    std::vector<glyph_t> m_data;
};

enum class ground_type_t
{
    foreground,
    background
};

struct buffer_t
{
    std::ostream& m_os;
    glyph_style_t m_prev_style = {};
    using args_t = std::vector<int>;

    explicit buffer_t(std::ostream& os) : m_os{ os }
    {
        // reset();
    }

    buffer_t& set_style(const glyph_style_t& style)
    {
        static const std::map<mode, std::tuple<args_t, args_t>> map = {
            { mode::bold, { args_t{ 1 }, args_t{ 21 } } },   { mode::dim, { args_t{ 2 }, args_t{ 22 } } },
            { mode::italic, { args_t{ 3 }, args_t{ 23 } } }, { mode::underline, { args_t{ 4 }, args_t{ 24 } } },
            { mode::blink, { args_t{ 5 }, args_t{ 25 } } },  { mode::inverse, { args_t{ 7 }, args_t{ 27 } } },
            { mode::hidden, { args_t{ 8 }, args_t{ 28 } } }, { mode::crossed_out, { args_t{ 9 }, args_t{ 29 } } },
        };
        for (const mode m : { mode::bold,
                              mode::dim,
                              mode::italic,
                              mode::underline,
                              mode::blink,
                              mode::inverse,
                              mode::crossed_out,
                              mode::standout })
        {
            if (style.mode_value.contains(m) != m_prev_style.mode_value.contains(m))
            {
                const auto iter = map.find(m);
                if (iter != map.end())
                {
                    const auto& [on_set, on_reset] = iter->second;
                    if (style.mode_value.contains(m))
                    {
                        escape(on_set);
                    }
                    else
                    {
                        escape(on_reset);
                    }
                }
            }
        }

        if (style.foreground != m_prev_style.foreground || style.background != m_prev_style.background)
        {
            write_color(ground_type_t::foreground, style.foreground);
            write_color(ground_type_t::background, style.background);
        }
        m_prev_style = style;
        return *this;
    }

    buffer_t& write(const multibyte& character)
    {
        m_os << character;
        return *this;
    }

    buffer_t& new_line()
    {
        m_prev_style = {};
        escape({ 0 });
        m_os << '\n';
        return *this;
    }

    void escape(const args_t& args)
    {
        m_os << "\033[" << ferrugo::core::delimit(args, ";") << "m";
    }

    void write_color(ground_type_t type, const color_t& col)
    {
        const args_t args = std::visit(
            ferrugo::core::overloaded{ [&](const true_color_t& c) -> args_t {
                                          return { type == ground_type_t::foreground ? 38 : 48, 2, c.red, c.green, c.blue };
                                      },
                                       [&](const palette_color_t& c) -> args_t {
                                           return { type == ground_type_t::foreground ? 38 : 48, 5, c.index };
                                       },
                                       [&](const default_color_t& c) -> args_t
                                       { return { type == ground_type_t::foreground ? 39 : 49 }; },
                                       [&](const basic_color_t& c) -> args_t
                                       { return { static_cast<int>(c) + (type == ground_type_t::foreground ? 30 : 40) }; } },
            col);
        escape(args);
    }

    void reset()
    {
        escape({});
    }
};

inline void output(const area_t::ref_type& area_t, buffer_t& buf)
{
    for (int y = 0; y < area_t.m_extent[1]; ++y)
    {
        for (int x = 0; x < area_t.m_extent[0]; ++x)
        {
            const glyph_t& g = area_t[location_t{ x, y }];
            buf.set_style(g.style).write(g.character);
        }
        buf.new_line();
    }
    buf.reset();
}

std::string render(const area_t& area)
{
    std::stringstream ss;
    ss << "\033[2J\033[;H";
    buffer_t buffer{ ss };
    output(area.ref(), buffer);
    return ss.str();
}

}  // namespace ansi
}  // namespace ferrugo
