#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <variant>

namespace ferrugo
{
namespace ansi_v2
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
    white
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
    white
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

    explicit palette_color_t(bright_color_t col) : palette_color_t{ static_cast<std::uint8_t>(static_cast<int>(col) + 8) }
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

    friend bool operator==(const palette_color_t& lhs, const palette_color_t& rhs)
    {
        return lhs.index == rhs.index;
    }

    friend bool operator!=(const palette_color_t& lhs, const palette_color_t& rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const palette_color_t& item)
    {
        return os << "(palette " << static_cast<int>(item.index) << ")";
    }
};

struct color_t
{
    using data_type = std::variant<default_color_t, basic_color_t, bright_color_t, palette_color_t, true_color_t>;
    data_type m_data;

    color_t() : m_data{ default_color_t{} }
    {
    }

    color_t(default_color_t col) : m_data{ col }
    {
    }

    color_t(basic_color_t col) : m_data{ col }
    {
    }

    color_t(bright_color_t col) : m_data{ col }
    {
    }

    color_t(palette_color_t col) : m_data{ col }
    {
    }

    color_t(true_color_t col) : m_data{ col }
    {
    }

    color_t(std::string_view txt) : m_data{ true_color_t{ txt } }
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
        return std::visit(
            [](const auto& lt, const auto& rt)
            {
                if constexpr (std::is_same_v<std::decay_t<decltype(lt)>, std::decay_t<decltype(rt)>>)
                {
                    return lt == rt;
                }
                return false;
            },
            lhs.m_data,
            rhs.m_data);
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
    color_t color;
};

inline auto fg(color_t col) -> color_specifier_t<ground_type_t::foreground>
{
    return { std::move(col) };
}

inline auto bg(color_t col) -> color_specifier_t<ground_type_t::background>
{
    return { std::move(col) };
}

using args_t = std::vector<int>;

template <ground_type_t GroundType>
auto to_args(const color_specifier_t<GroundType>& color_specifier) -> args_t
{
    struct visitor
    {
        ground_type_t m_ground;

        auto operator()(default_color_t) const -> args_t
        {
            return { m_ground == ground_type_t::foreground ? 39 : 49 };
        }

        auto operator()(basic_color_t col) const -> args_t
        {
            return { static_cast<int>(col) + (m_ground == ground_type_t::foreground ? 30 : 40) };
        }

        auto operator()(bright_color_t col) const -> args_t
        {
            return { static_cast<int>(col) + (m_ground == ground_type_t::foreground ? 90 : 100) };
        }

        auto operator()(palette_color_t col) const -> args_t
        {
            return { m_ground == ground_type_t::foreground ? 38 : 48, 5, col.index };
        }

        auto operator()(const true_color_t& col) const -> args_t
        {
            return { m_ground == ground_type_t::foreground ? 38 : 48, 2, col[0], col[1], col[2] };
        }
    };
    return std::visit(visitor{ GroundType }, color_specifier.color.m_data);
}

inline auto esc(const args_t& args) -> std::string
{
    std::stringstream ss;
    ss << "\033[";
    for (std::size_t i = 0; i < args.size(); ++i)
    {
        if (i != 0)
        {
            ss << ";";
        }
        ss << args[i];
    }
    ss << "m";
    return ss.str();
}

template <ground_type_t GroundType>
inline std::ostream& operator<<(std::ostream& os, const color_specifier_t<GroundType>& item)
{
    return os << esc(to_args(item));
}

}  // namespace ansi_v2
}  // namespace ferrugo
