#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string_view>
#include <vector>

namespace ansi
{

struct font_t
{
    using underlying_type = std::uint16_t;

    underlying_type m_value;

    constexpr font_t() : font_t{ 0 }
    {
    }

    constexpr explicit font_t(underlying_type value) : m_value(value)
    {
    }

    constexpr friend bool operator==(const font_t lhs, const font_t rhs)
    {
        return lhs.m_value == rhs.m_value;
    }

    constexpr friend bool operator<(const font_t lhs, const font_t rhs)
    {
        return lhs.m_value < rhs.m_value;
    }

    constexpr friend bool operator!=(const font_t lhs, const font_t rhs)
    {
        return !(lhs == rhs);
    }

    constexpr friend bool operator>(const font_t lhs, const font_t rhs)
    {
        return rhs < lhs;
    }

    constexpr friend bool operator<=(const font_t lhs, const font_t rhs)
    {
        return !(lhs > rhs);
    }

    constexpr friend bool operator>=(const font_t lhs, const font_t rhs)
    {
        return !(lhs < rhs);
    }

    constexpr friend font_t operator~(const font_t item)
    {
        return font_t(~item.m_value);
    }

    constexpr friend font_t operator&(const font_t lhs, const font_t rhs)
    {
        return font_t(lhs.m_value & rhs.m_value);
    }

    constexpr friend font_t& operator&=(font_t& lhs, const font_t rhs)
    {
        lhs.m_value &= rhs.m_value;
        return lhs;
    }

    constexpr friend font_t operator|(const font_t lhs, const font_t rhs)
    {
        return font_t(lhs.m_value | rhs.m_value);
    }

    constexpr friend font_t& operator|=(font_t& lhs, const font_t rhs)
    {
        lhs.m_value |= rhs.m_value;
        return lhs;
    }

    constexpr friend font_t operator^(const font_t lhs, const font_t rhs)
    {
        return font_t(lhs.m_value ^ rhs.m_value);
    }

    constexpr friend font_t& operator^=(font_t& lhs, const font_t rhs)
    {
        lhs.m_value ^= rhs.m_value;
        return lhs;
    }

    constexpr explicit operator bool() const
    {
        return static_cast<bool>(m_value);
    }

    static const font_t none;
    static const font_t standout;
    static const font_t bold;
    static const font_t dim;
    static const font_t italic;
    static const font_t underline;
    static const font_t blink;
    static const font_t inverse;
    static const font_t hidden;
    static const font_t crossed_out;
    static const font_t double_underline;

    bool contains(font_t v) const
    {
        return static_cast<bool>(*this & v);
    }

    font_t& unset(const font_t v)
    {
        return *this = *this & ~v;
    }

    font_t& set(const font_t v)
    {
        return *this = *this | v;
    }

    friend std::ostream& operator<<(std::ostream& os, font_t item)
    {
        static const auto map = std::vector<std::pair<font_t, std::string_view>>{
            { font_t::none, "none" },
            { font_t::standout, "standout" },
            { font_t::bold, "bold" },
            { font_t::dim, "dim" },
            { font_t::italic, "italic" },
            { font_t::underline, "underline" },
            { font_t::blink, "blink" },
            { font_t::inverse, "inverse" },
            { font_t::hidden, "hidden" },
            { font_t::crossed_out, "crossed_out" },
            { font_t::double_underline, "double_underline" },
        };
        os << "(font_t";
        for (const auto& [f, n] : map)
        {
            if (item.contains(f))
            {
                os << " " << n;
            }
        }
        os << ")";
        return os;
    }
};

inline const font_t font_t::none{ 0 };
inline const font_t font_t::standout{ 1 << 0 };
inline const font_t font_t::bold{ 1 << 1 };
inline const font_t font_t::dim{ 1 << 2 };
inline const font_t font_t::italic{ 1 << 3 };
inline const font_t font_t::underline{ 1 << 4 };
inline const font_t font_t::blink{ 1 << 5 };
inline const font_t font_t::inverse{ 1 << 6 };
inline const font_t font_t::hidden{ 1 << 7 };
inline const font_t font_t::crossed_out{ 1 << 8 };
inline const font_t font_t::double_underline{ 1 << 9 };

}  // namespace ansi
