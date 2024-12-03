#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <ferrugo/ansi/escape.hpp>
#include <variant>

namespace ferrugo
{
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

enum class bright_color_t
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
    case basic_color_t::v: return os << "basic_color_t::" << #v
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

inline std::ostream& operator<<(std::ostream& os, bright_color_t item)
{
#define CASE(v) \
    case bright_color_t::v: return os << "bright_color_t::" << #v
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
        default: throw std::runtime_error{ "unknown bright_color_t" };
    }
#undef CASE
    return os;
}
struct default_color_t
{
    friend bool operator==(default_color_t, default_color_t)
    {
        return true;
    }

    friend bool operator!=(default_color_t, default_color_t)
    {
        return false;
    }

    friend std::ostream& operator<<(std::ostream& os, default_color_t)
    {
        return os << "default";
    }
};

struct true_color_t : public std::array<std::uint8_t, 3>
{
    using base_t = std::array<std::uint8_t, 3>;
    using base_t::base_t;

    true_color_t(std::uint8_t r, std::uint8_t g, std::uint8_t b) : base_t{ { r, g, b } }
    {
    }

    explicit true_color_t(std::string_view txt) : true_color_t{}
    {
        assert(txt.size() == 6);
        for (std::size_t i = 0; i < 3; ++i)
        {
            (*this)[i] = parse_sub(txt.substr(2 * i, 2));
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const true_color_t& item)
    {
        return os << "(rgb "  //
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
    std::uint8_t index;

    explicit palette_color_t(std::uint8_t index) : index{ index }
    {
    }

    explicit palette_color_t(basic_color_t col) : palette_color_t{ static_cast<std::uint8_t>(static_cast<int>(col) + 0) }
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

    friend bool operator==(const palette_color_t lhs, const palette_color_t rhs)
    {
        return lhs.index == rhs.index;
    }

    friend bool operator!=(const palette_color_t lhs, const palette_color_t rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const palette_color_t item)
    {
        return os << "(palette " << static_cast<int>(item.index) << ")";
    }
};

struct color_t
{
    using data_type = std::variant<default_color_t, basic_color_t, bright_color_t, palette_color_t, true_color_t>;

    data_type m_data;

    color_t() : m_data{ std::in_place_type<default_color_t>, default_color_t{} }
    {
    }

    color_t(default_color_t col) : m_data{ std::in_place_type<default_color_t>, col }
    {
    }

    color_t(basic_color_t col) : m_data{ std::in_place_type<basic_color_t>, col }
    {
    }

    color_t(bright_color_t col) : m_data{ std::in_place_type<bright_color_t>, col }
    {
    }

    color_t(palette_color_t col) : m_data{ std::in_place_type<palette_color_t>, col }
    {
    }

    color_t(true_color_t col) : m_data{ std::in_place_type<true_color_t>, col }
    {
    }

    color_t(std::string_view txt) : color_t{ true_color_t{ txt } }
    {
    }

    color_t(const char* txt) : color_t{ std::string_view(txt) }
    {
    }

    friend std::ostream& operator<<(std::ostream& os, const color_t& item)
    {
        os << "(color ";
        std::visit([&](const auto& c) { os << c; }, item.m_data);
        os << ")";
        return os;
    }

    friend bool operator==(const color_t& lhs, const color_t& rhs)
    {
        return lhs.m_data == rhs.m_data;
    }

    friend bool operator!=(const color_t& lhs, const color_t& rhs)
    {
        return !(lhs == rhs);
    }
};

enum class ground_type_t
{
    foreground,
    background
};

template <ground_type_t GroundType>
struct color_specifier_t
{
    color_t color = {};

    struct visitor
    {
        static const int Base = GroundType == ground_type_t::foreground ? 0 : 10;

        auto operator()(default_color_t) const -> args_t
        {
            return args_t{ Base + 39 };
        }

        auto operator()(basic_color_t col) const -> args_t
        {
            return args_t{ static_cast<int>(col) + (Base + 30) };
        }

        auto operator()(bright_color_t col) const -> args_t
        {
            return args_t{ static_cast<int>(col) + (Base + 90) };
        }

        auto operator()(palette_color_t col) const -> args_t
        {
            return args_t{ Base + 38, 5, col.index };
        }

        auto operator()(true_color_t col) const -> args_t
        {
            return args_t{ Base + 38, 2, col[0], col[1], col[2] };
        }
    };

    friend std::ostream& operator<<(std::ostream& os, const color_specifier_t& item)
    {
        os << std::visit(visitor{}, item.color.m_data);
        return os;
    }
};

template <ground_type_t GroundType>
struct color_specifier_builder_fn
{
    constexpr auto operator()(color_t col) const -> color_specifier_t<GroundType>
    {
        return { std::move(col) };
    }
};

static constexpr inline auto foreground = color_specifier_builder_fn<ground_type_t::foreground>{};
static constexpr inline auto background = color_specifier_builder_fn<ground_type_t::background>{};

}  // namespace ansi
}  // namespace ferrugo
