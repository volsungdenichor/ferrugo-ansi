#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <variant>

namespace ansi
{

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

}  // namespace ansi