#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <variant>
#include <vector>

enum class basic_color_t
{
    black,
    red,
    green,
    yellow,
    blue,
    magenta,
    cyan,
    white,
};

inline std::ostream& operator<<(std::ostream& os, basic_color_t item)
{
#define CASE(v) \
    case basic_color_t::v: return os << #v
    switch (item)
    {
        CASE(black);
        CASE(red);
        CASE(green);
        CASE(yellow);
        CASE(blue);
        CASE(magenta);
        CASE(cyan);
        CASE(white);
        default: throw std::runtime_error{ "unknown basic_color_t" };
    }
#undef CASE
    return os;
}

struct default_color_t
{
    constexpr friend bool operator==(default_color_t, default_color_t)
    {
        return true;
    }

    constexpr friend bool operator!=(default_color_t, default_color_t)
    {
        return false;
    }

    friend std::ostream& operator<<(std::ostream& os, default_color_t)
    {
        return os << "default_color";
    }
};

struct standard_color_t
{
    basic_color_t m_color;

    constexpr explicit standard_color_t(basic_color_t color) : m_color(color)
    {
    }

    constexpr friend bool operator==(const standard_color_t& lhs, const standard_color_t& rhs)
    {
        return lhs.m_color == rhs.m_color;
    }

    constexpr friend bool operator!=(const standard_color_t& lhs, const standard_color_t& rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const standard_color_t item)
    {
        return os << "(standard_color " << item.m_color << ")";
    }
};

struct bright_color_t
{
    basic_color_t m_color;

    constexpr explicit bright_color_t(basic_color_t color) : m_color(color)
    {
    }

    constexpr friend bool operator==(const bright_color_t& lhs, const bright_color_t& rhs)
    {
        return lhs.m_color == rhs.m_color;
    }

    constexpr friend bool operator!=(const bright_color_t& lhs, const bright_color_t& rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const bright_color_t item)
    {
        return os << "(bright_color " << item.m_color << ")";
    }
};

struct rgb_color_t : public std::array<std::uint8_t, 3>
{
    using base_t = std::array<std::uint8_t, 3>;
    using base_t::base_t;

    constexpr rgb_color_t(std::uint8_t r, std::uint8_t g, std::uint8_t b) : base_t{ { r, g, b } }
    {
    }

    constexpr explicit rgb_color_t(std::string_view txt) : rgb_color_t{ 0, 0, 0 }
    {
        if (txt.front() == '#')
        {
            txt.remove_prefix(1);
        }
        assert(txt.size() == 6);
        for (std::size_t i = 0; i < 3; ++i)
        {
            (*this)[i] = parse_sub(txt.substr(2 * i, 2));
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const rgb_color_t& item)
    {
        return os << "(rgb_color "  //
                  << static_cast<int>(item[0]) << " " << static_cast<int>(item[1]) << " " << static_cast<int>(item[2])
                  << ")";
    }

private:
    static std::uint8_t parse_sub(std::string_view str)
    {
        std::stringstream ss;
        ss << std::hex << str;
        std::int16_t result;
        ss >> result;
        return static_cast<std::uint8_t>(result);
    }
};

struct palette_color_t
{
    std::uint8_t m_index;

    constexpr explicit palette_color_t(std::uint8_t index) : m_index{ index }
    {
    }

    constexpr explicit palette_color_t(standard_color_t col)
        : palette_color_t{ static_cast<std::uint8_t>(static_cast<int>(col.m_color) + 0) }
    {
    }

    constexpr explicit palette_color_t(bright_color_t col)
        : palette_color_t{ static_cast<std::uint8_t>(static_cast<int>(col.m_color) + 8) }
    {
    }

    constexpr explicit palette_color_t(basic_color_t col) : palette_color_t{ standard_color_t{ col } }
    {
    }

    static palette_color_t from_rgb(const std::array<std::uint8_t, 3>& values)
    {
        std::uint8_t index = 0;
        int e = 36;
        for (std::size_t i = 0; i < 3; ++i)
        {
            assert(values[i] < 6);
            index += e * values[i];
            e /= 6;
        }
        return palette_color_t{ static_cast<std::uint8_t>(index + 16) };
    }

    static palette_color_t from_rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b)
    {
        return from_rgb({ r, g, b });
    }

    static palette_color_t from_grayscale(std::uint8_t brightness)
    {
        assert(brightness < 24);
        return palette_color_t{ static_cast<std::uint8_t>(brightness + 232) };
    }

    bool is_standard_color() const
    {
        return 0 <= m_index && m_index <= 7;
    }

    bool is_bright_color() const
    {
        return 8 <= m_index && m_index <= 15;
    }

    bool is_grayscale() const
    {
        return 232 <= m_index && m_index <= 255;
    }

    bool is_rgb() const
    {
        return 16 <= m_index && m_index <= 231;
    }

    friend bool operator==(const palette_color_t lhs, const palette_color_t rhs)
    {
        return lhs.m_index == rhs.m_index;
    }

    friend bool operator!=(const palette_color_t lhs, const palette_color_t rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const palette_color_t item)
    {
        return os << "(palette_color " << static_cast<int>(item.m_index) << ")";
    }
};

struct color_t
{
    using data_type = std::variant<default_color_t, standard_color_t, bright_color_t, palette_color_t, rgb_color_t>;

    data_type m_data;

    constexpr color_t() : m_data{ std::in_place_type<default_color_t>, default_color_t{} }
    {
    }

    constexpr color_t(default_color_t col) : m_data{ std::in_place_type<default_color_t>, col }
    {
    }

    constexpr color_t(standard_color_t col) : m_data{ std::in_place_type<standard_color_t>, col }
    {
    }

    constexpr color_t(basic_color_t col) : color_t{ standard_color_t{ col } }
    {
    }

    constexpr color_t(bright_color_t col) : m_data{ std::in_place_type<bright_color_t>, col }
    {
    }

    constexpr color_t(palette_color_t col) : m_data{ std::in_place_type<palette_color_t>, col }
    {
    }

    constexpr color_t(rgb_color_t col) : m_data{ std::in_place_type<rgb_color_t>, col }
    {
    }

    constexpr color_t(std::string_view txt) : color_t{ rgb_color_t{ txt } }
    {
    }

    constexpr color_t(const char* txt) : color_t{ std::string_view(txt) }
    {
    }

    friend std::ostream& operator<<(std::ostream& os, const color_t& item)
    {
        std::visit([&](const auto& c) { os << c; }, item.m_data);
        return os;
    }

    constexpr friend bool operator==(const color_t& lhs, const color_t& rhs)
    {
        return lhs.m_data == rhs.m_data;
    }

    constexpr friend bool operator!=(const color_t& lhs, const color_t& rhs)
    {
        return !(lhs == rhs);
    }
};

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
        os << "[";
        for (const auto& [f, n] : map)
        {
            if (item.contains(f))
            {
                os << " " << n;
            }
        }
        os << "]";
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

struct font_style_t
{
    color_t foreground = default_color_t{};
    color_t background = default_color_t{};
    font_t font = font_t::none;

    friend std::ostream& operator<<(std::ostream& os, const font_style_t& item)
    {
        return os << "{"
                  << ":foreground " << item.foreground << " :background " << item.background << " :font " << item.font
                  << "}";
    }
};
