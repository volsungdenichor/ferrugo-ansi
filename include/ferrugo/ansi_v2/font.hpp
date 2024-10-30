#pragma once

#include <cstdint>
#include <iostream>

namespace ferrugo
{
namespace ansi_v2
{

struct font
{
    using underlying_type = std::uint16_t;

    underlying_type m_value;

    font() : font{ 0 }
    {
    }

    explicit font(underlying_type value) : m_value(value)
    {
    }

    friend bool operator==(font lhs, const font rhs)
    {
        return lhs.m_value == rhs.m_value;
    }

    friend bool operator<(font lhs, const font rhs)
    {
        return lhs.m_value < rhs.m_value;
    }

    friend bool operator!=(font lhs, const font rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator>(font lhs, const font rhs)
    {
        return rhs < lhs;
    }

    friend bool operator<=(font lhs, const font rhs)
    {
        return !(lhs > rhs);
    }

    friend bool operator>=(font lhs, const font rhs)
    {
        return !(lhs < rhs);
    }

    friend font operator&(font lhs, const font rhs)
    {
        return font(lhs.m_value & rhs.m_value);
    }

    friend font operator|(font lhs, const font rhs)
    {
        return font(lhs.m_value | rhs.m_value);
    }

    explicit operator bool() const
    {
        return static_cast<bool>(m_value);
    }

    static const font none;
    static const font standout;
    static const font bold;
    static const font dim;
    static const font italic;
    static const font underline;
    static const font blink;
    static const font inverse;
    static const font hidden;
    static const font crossed_out;
    static const font double_underline;

    bool contains(font v) const
    {
        return static_cast<bool>(*this & v);
    }

    friend std::ostream& operator<<(std::ostream& os, font item)
    {
        os << "(font";
        if (item.contains(font::none))
        {
            os << " none";
        }
        if (item.contains(font::standout))
        {
            os << " standout";
        }
        if (item.contains(font::bold))
        {
            os << " bold";
        }
        if (item.contains(font::dim))
        {
            os << " dim";
        }
        if (item.contains(font::italic))
        {
            os << " italic";
        }
        if (item.contains(font::underline))
        {
            os << " underline";
        }
        if (item.contains(font::blink))
        {
            os << " blink";
        }
        if (item.contains(font::inverse))
        {
            os << " inverse";
        }
        if (item.contains(font::hidden))
        {
            os << " hidden";
        }
        if (item.contains(font::crossed_out))
        {
            os << " crossed_out";
        }
        if (item.contains(font::double_underline))
        {
            os << " double_underline";
        }
        os << ")";
        return os;
    }
};

inline const font font::none{ 0 };
inline const font font::standout{ 1 << 0 };
inline const font font::bold{ 1 << 1 };
inline const font font::dim{ 1 << 2 };
inline const font font::italic{ 1 << 3 };
inline const font font::underline{ 1 << 4 };
inline const font font::blink{ 1 << 5 };
inline const font font::inverse{ 1 << 6 };
inline const font font::hidden{ 1 << 7 };
inline const font font::crossed_out{ 1 << 8 };
inline const font font::double_underline{ 1 << 9 };

}  // namespace ansi_v2
}  // namespace ferrugo