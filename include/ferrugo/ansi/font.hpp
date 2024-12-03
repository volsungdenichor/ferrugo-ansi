#pragma once

#include <cstdint>
#include <ferrugo/ansi/escape.hpp>
#include <iostream>
#include <map>
#include <set>

namespace ferrugo
{
namespace ansi
{

struct font_t
{
    using underlying_type = std::uint16_t;

    underlying_type m_value;

    font_t() : font_t{ 0 }
    {
    }

    explicit font_t(underlying_type value) : m_value(value)
    {
    }

    friend bool operator==(const font_t lhs, const font_t rhs)
    {
        return lhs.m_value == rhs.m_value;
    }

    friend bool operator<(const font_t lhs, const font_t rhs)
    {
        return lhs.m_value < rhs.m_value;
    }

    friend bool operator!=(const font_t lhs, const font_t rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator>(const font_t lhs, const font_t rhs)
    {
        return rhs < lhs;
    }

    friend bool operator<=(const font_t lhs, const font_t rhs)
    {
        return !(lhs > rhs);
    }

    friend bool operator>=(const font_t lhs, const font_t rhs)
    {
        return !(lhs < rhs);
    }

    friend font_t operator~(const font_t item)
    {
        return font_t(~item.m_value);
    }

    friend font_t operator&(const font_t lhs, const font_t rhs)
    {
        return font_t(lhs.m_value & rhs.m_value);
    }

    friend font_t& operator&=(font_t& lhs, const font_t rhs)
    {
        lhs.m_value &= rhs.m_value;
        return lhs;
    }

    friend font_t operator|(const font_t lhs, const font_t rhs)
    {
        return font_t(lhs.m_value | rhs.m_value);
    }

    friend font_t& operator|=(font_t& lhs, const font_t rhs)
    {
        lhs.m_value |= rhs.m_value;
        return lhs;
    }

    friend font_t operator^(const font_t lhs, const font_t rhs)
    {
        return font_t(lhs.m_value ^ rhs.m_value);
    }

    explicit operator bool() const
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
        os << "(font_t";
        if (item.contains(font_t::none))
        {
            os << " none";
        }
        if (item.contains(font_t::standout))
        {
            os << " standout";
        }
        if (item.contains(font_t::bold))
        {
            os << " bold";
        }
        if (item.contains(font_t::dim))
        {
            os << " dim";
        }
        if (item.contains(font_t::italic))
        {
            os << " italic";
        }
        if (item.contains(font_t::underline))
        {
            os << " underline";
        }
        if (item.contains(font_t::blink))
        {
            os << " blink";
        }
        if (item.contains(font_t::inverse))
        {
            os << " inverse";
        }
        if (item.contains(font_t::hidden))
        {
            os << " hidden";
        }
        if (item.contains(font_t::crossed_out))
        {
            os << " crossed_out";
        }
        if (item.contains(font_t::double_underline))
        {
            os << " double_underline";
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

struct font_diff_t
{
    std::set<font_t> enabled;
    std::set<font_t> disabled;

    friend std::ostream& operator<<(std::ostream& os, const font_diff_t& item)
    {
        args_t result;

        const auto handle = [&](font_t f, int on_set, int on_reset)
        {
            if (item.enabled.count(f))
            {
                result.push_back(on_set);
            }
            if (item.disabled.count(f))
            {
                result.push_back(on_reset);
            }
        };

        handle(font_t::bold, 1, 21);
        handle(font_t::dim, 2, 22);
        handle(font_t::italic, 3, 23);
        handle(font_t::underline, 4, 24);
        handle(font_t::blink, 5, 25);
        handle(font_t::crossed_out, 9, 29);
        handle(font_t::hidden, 8, 28);

        if (!result.empty())
        {
            os << result;
        }
        return os;
    }
};

inline auto operator-(const font_t lhs, const font_t rhs) -> font_diff_t
{
    font_diff_t result = {};
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
        const auto lhs_contains = lhs.contains(f);
        const auto rhs_contains = rhs.contains(f);
        if (lhs_contains && !rhs_contains)
        {
            result.enabled.insert(f);
        }
        if (!lhs_contains && rhs_contains)
        {
            result.disabled.insert(f);
        }
    }
    return result;
}

}  // namespace ansi
}  // namespace ferrugo
