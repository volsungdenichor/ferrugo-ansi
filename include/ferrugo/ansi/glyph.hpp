#pragma once

#include <ferrugo/ansi/colors.hpp>
#include <ferrugo/ansi/multibyte_string.hpp>
#include <functional>

namespace ferrugo
{
namespace ansi
{

enum class mode
{
    none = 0,
    standout = 1,
    bold = 2,
    dim = 4,
    italic = 8,
    underline = 16,
    blink = 32,
    inverse = 64,
    hidden = 128,
    crossed_out = 256,
    double_underline = 512,
};

template <class E, class = std::enable_if_t<std::is_enum_v<E>>>
class bitmask
{
public:
    using enum_type = E;
    using underlying_type = std::underlying_type_t<E>;

    bitmask() : m_value{ 0 }
    {
    }

    bitmask(enum_type v) : m_value{ to_underlying(v) }
    {
    }

    bool contains(bitmask b) const
    {
        return (m_value & b.m_value) != 0;
    }

    bitmask& set(bitmask b)
    {
        m_value |= b.m_value;
        return *this;
    }

    bitmask& reset(bitmask b)
    {
        m_value &= ~b.m_value;
        return *this;
    }

    friend bitmask operator|(bitmask lhs, bitmask rhs)
    {
        return lhs.set(rhs);
    }

    friend bitmask operator|(bitmask lhs, enum_type rhs)
    {
        return lhs.set(rhs);
    }

    friend bitmask operator|(enum_type lhs, bitmask rhs)
    {
        return bitmask(lhs) | rhs;
    }

    friend bitmask& operator|=(bitmask& lhs, bitmask rhs)
    {
        return lhs.set(rhs);
    }

    friend bool operator&(bitmask lhs, bitmask rhs)
    {
        return lhs.contains(rhs);
    }

    friend bool operator==(bitmask lhs, bitmask rhs)
    {
        return lhs.m_value == rhs.m_value;
    }

    friend bool operator!=(bitmask lhs, bitmask rhs)
    {
        return !(lhs == rhs);
    }

private:
    static underlying_type to_underlying(E v)
    {
        return static_cast<underlying_type>(v);
    }

    underlying_type m_value;
};

using modes_t = bitmask<mode>;

inline modes_t operator|(mode lhs, mode rhs)
{
    return modes_t(lhs) | rhs;
}

inline std::ostream& operator<<(std::ostream& os, modes_t item)
{
    bool init = true;
    const auto separator = [&]()
    {
        if (!init)
        {
            os << " ";
        }
        init = false;
    };
    os << "[";
#define CASE(v)                 \
    if (item.contains(mode::v)) \
    {                           \
        separator();            \
        os << #v;               \
    }

    CASE(standout)
    CASE(bold)
    CASE(dim)
    CASE(italic)
    CASE(underline)
    CASE(blink)
    CASE(inverse)
    CASE(hidden)
    CASE(crossed_out)
    CASE(double_underline)
#undef CASE
    os << "]";
    return os;
}

struct glyph_style_t
{
    color_t foreground = default_color_t{};
    color_t background = default_color_t{};
    modes_t mode_value = modes_t{ mode::none };

    friend bool operator==(const glyph_style_t& lhs, const glyph_style_t& rhs)
    {
        static const auto tie
            = [](const glyph_style_t& item) { return std::tie(item.foreground, item.background, item.mode_value); };
        return tie(lhs) == tie(rhs);
    }

    friend bool operator!=(const glyph_style_t& lhs, const glyph_style_t& rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const glyph_style_t& item)
    {
        return os << "[" << item.foreground << " " << item.background << " " << item.mode_value << "]";
    }
};

using character_t = multibyte;

struct glyph_t
{
    character_t character = character_t{ ' ' };
    glyph_style_t style = {};

    glyph_t(character_t character, glyph_style_t style = {}) : character{ std::move(character) }, style{ std::move(style) }
    {
    }

    glyph_t() : glyph_t(' ')
    {
    }

    glyph_t& operator=(glyph_t other)
    {
        std::swap(character, other.character);
        std::swap(style, other.style);
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const glyph_t& item)
    {
        return os << "[\"" << item.character << "\" " << item.style << "]";
    }
};

struct glyph_style_applier_t
{
    using applier_type = std::function<void(glyph_style_t&)>;

    applier_type applier;

    explicit glyph_style_applier_t(applier_type applier) : applier{ std::move(applier) }
    {
    }

    glyph_style_applier_t(modes_t m) : glyph_style_applier_t{ [=](glyph_style_t& s) { s.mode_value |= m; } }
    {
    }

    glyph_style_applier_t(mode m) : glyph_style_applier_t{ [=](glyph_style_t& s) { s.mode_value |= m; } }
    {
    }

    glyph_style_t& operator()(glyph_style_t& s) const
    {
        applier(s);
        return s;
    }

    glyph_t& operator()(glyph_t& g) const
    {
        (*this)(g.style);
        return g;
    }

    operator glyph_style_t() const
    {
        glyph_style_t result{};
        applier(result);
        return result;
    }
};

inline glyph_style_applier_t operator|(glyph_style_applier_t lhs, glyph_style_applier_t rhs)
{
    return glyph_style_applier_t{ [=](glyph_style_t& s)
                                  {
                                      lhs(s);
                                      rhs(s);
                                  } };
}

inline glyph_style_t& operator|=(glyph_style_t& g, const glyph_style_applier_t& applier)
{
    applier(g);
    return g;
}

inline glyph_style_t operator|(glyph_style_t g, const glyph_style_applier_t& applier)
{
    applier(g);
    return g;
}

inline glyph_t operator|(glyph_t g, const glyph_style_applier_t& applier)
{
    applier(g);
    return g;
}

inline glyph_t operator|(character_t ch, const glyph_style_applier_t& applier)
{
    glyph_t g{ ch };
    applier(g);
    return g;
}

inline glyph_t operator|(const char* ch, const glyph_style_applier_t& applier)
{
    glyph_t g{ multibyte_string(ch)[0] };
    applier(g);
    return g;
}

inline glyph_style_applier_t fg(const color_t& col)
{
    return glyph_style_applier_t{ [=](glyph_style_t& s) { s.foreground = col; } };
}

inline glyph_style_applier_t bg(const color_t& col)
{
    return glyph_style_applier_t{ [=](glyph_style_t& s) { s.background = col; } };
}

inline glyph_style_applier_t color(const color_t& fg, const color_t& bg = default_color_t{})
{
    return glyph_style_applier_t{ [=](glyph_style_t& s)
                                  {
                                      s.foreground = fg;
                                      s.background = bg;
                                  } };
}

}  // namespace ansi
}  // namespace ferrugo
