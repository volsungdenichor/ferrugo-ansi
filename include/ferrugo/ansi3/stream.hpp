#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

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
        return os << "{:standard_color " << item.m_color << "}";
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
        return os << "{:bright_color " << item.m_color << "}";
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
        return os << "{:rgb_color ["  //
                  << static_cast<int>(item[0]) << " " << static_cast<int>(item[1]) << " " << static_cast<int>(item[2])
                  << "]}";
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
        return os << "{:palette_color " << static_cast<int>(item.m_index) << "}";
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

        bool first = true;
        for (const auto& [f, n] : map)
        {
            if (!first)
            {
                os << " ";
            }
            if (item.contains(f))
            {
                os << n;
                first = true;
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

struct font_style_applier_t : public std::function<void(font_style_t&)>
{
    using base_t = std::function<void(font_style_t&)>;

    using base_t::base_t;
};

inline auto operator|(font_style_applier_t lhs, font_style_applier_t rhs) -> font_style_applier_t
{
    return [=](font_style_t& style)
    {
        lhs(style);
        rhs(style);
    };
}

inline auto fg(color_t col) -> font_style_applier_t
{
    return [=](font_style_t& style) { style.foreground = col; };
}

inline auto bg(color_t col) -> font_style_applier_t
{
    return [=](font_style_t& style) { style.background = col; };
}

inline auto font(font_t font) -> font_style_applier_t
{
    return [=](font_style_t& style) { style.font |= font; };
}

struct op_new_line_t
{
};

struct op_indent_t
{
};

struct op_unindent_t
{
};

struct op_text_t
{
    std::string content;
};

struct op_text_ref_t
{
    std::string_view content;
};

struct op_push_style_t
{
    font_style_t style;
};

struct op_modify_style_t
{
    font_style_applier_t applier;
};

struct op_pop_style_t
{
};

using stream_op_t = std::variant<  //
    op_new_line_t,
    op_indent_t,
    op_unindent_t,
    op_text_t,
    op_text_ref_t,
    op_push_style_t,
    op_modify_style_t,
    op_pop_style_t>;

constexpr inline auto new_line = op_new_line_t{};

constexpr inline auto indent = op_indent_t{};

constexpr inline auto unindent = op_unindent_t{};

constexpr inline auto text = [](std::string content) { return op_text_t{ std::move(content) }; };

constexpr inline auto text_ref = [](std::string_view content) { return op_text_ref_t{ content }; };

constexpr inline auto push_style = [](font_style_t style) { return op_push_style_t{ std::move(style) }; };
constexpr inline auto modify_style = [](font_style_applier_t applier) { return op_modify_style_t{ std::move(applier) }; };

constexpr inline auto pop_style = op_pop_style_t{};

template <class T>
struct formatter_t;

struct stream_t
{
    std::vector<stream_op_t> m_ops;

    stream_t() = default;
    stream_t(const stream_t&) = default;
    stream_t(stream_t&&) noexcept = default;

    stream_t& operator<<(const stream_t& other)
    {
        m_ops.insert(m_ops.end(), other.m_ops.begin(), other.m_ops.end());
        return *this;
    }

    stream_t& operator<<(stream_t&& other)
    {
        m_ops.insert(m_ops.end(), std::make_move_iterator(other.m_ops.begin()), std::make_move_iterator(other.m_ops.end()));
        return *this;
    }

    template <class T>
    stream_t& operator<<(const T& item)
    {
        if constexpr (std::is_constructible_v<stream_op_t, T>)
        {
            m_ops.push_back(item);
        }
        else if constexpr (std::is_invocable_v<T, stream_t&>)
        {
            std::invoke(item, *this);
        }
        else
        {
            formatter_t<T>{}.format(*this, item);
        }
        return *this;
    }

    template <class... Args>
    stream_t& operator()(Args&&... args)
    {
        (*this << ... << std::forward<Args>(args));
        return *this;
    }
};

template <class... Args>
stream_t& format_to(stream_t& stream, Args&&... args)
{
    return stream(std::forward<Args>(args)...);
}

struct render_fn
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
            return { Base + 30 + static_cast<int>(col.m_color) };
        }

        auto operator()(bright_color_t col) const -> std::vector<int>
        {
            return { Base + 90 + static_cast<int>(col.m_color) };
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

    static void write_args(std::ostream& os, const std::vector<int>& args)
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

    static std::string write_args(const std::vector<int>& args)
    {
        std::stringstream ss;
        write_args(ss, args);
        return ss.str();
    }

    static std::string change_style(const font_style_t& old_style, const font_style_t& new_style)
    {
        std::stringstream ss;
        if (old_style.foreground != new_style.foreground)
        {
            write_args(ss, std::visit(color_visitor_t<0>{}, new_style.foreground.m_data));
        }
        if (old_style.background != new_style.background)
        {
            write_args(ss, std::visit(color_visitor_t<10>{}, new_style.background.m_data));
        }
        if (old_style.font != new_style.font)
        {
            static const std::vector<std::pair<font_t, int>> set_map = {
                { font_t::bold, 1 },      { font_t::dim, 2 },         { font_t::italic, 3 },
                { font_t::underline, 4 }, { font_t::blink, 5 },       { font_t::inverse, 7 },
                { font_t::hidden, 8 },    { font_t::crossed_out, 9 }, { font_t::double_underline, 21 },
            };
            std::vector<int> args = { 22, 23, 24, 25, 27, 28, 29 };
            for (const auto& [f, v] : set_map)
            {
                if (new_style.font.contains(f))
                {
                    args.push_back(v);
                }
            }
            write_args(ss, args);
        }
        return ss.str();
    }

    struct context_t
    {
        std::ostream& os;
        int indent_level = 0;
        bool new_line = false;
        std::vector<font_style_t> style_stack = { font_style_t{} };
    };

    struct visitor_t
    {
        context_t& m_ctx;

        void operator()(op_new_line_t) const
        {
            m_ctx.new_line = true;
        }

        void operator()(op_indent_t) const
        {
            m_ctx.indent_level += 1;
        }

        void operator()(op_unindent_t) const
        {
            m_ctx.indent_level = std::max(0, m_ctx.indent_level - 1);
        }

        void operator()(const op_text_t& v) const
        {
            handle_indent();
            m_ctx.os << v.content;
        }

        void operator()(const op_text_ref_t& v) const
        {
            handle_indent();
            m_ctx.os << v.content;
        }

        void operator()(const op_push_style_t& v) const
        {
            const font_style_t previous_style = m_ctx.style_stack.back();
            m_ctx.style_stack.push_back(v.style);
            m_ctx.os << change_style(previous_style, m_ctx.style_stack.back());
        }

        void operator()(const op_modify_style_t& v) const
        {
            const font_style_t previous_style = m_ctx.style_stack.back();
            font_style_t new_style = previous_style;
            v.applier(new_style);
            m_ctx.style_stack.push_back(new_style);
            m_ctx.os << change_style(previous_style, m_ctx.style_stack.back());
        }

        void operator()(op_pop_style_t) const
        {
            const font_style_t previous_style = m_ctx.style_stack.back();
            m_ctx.style_stack.pop_back();
            m_ctx.os << change_style(previous_style, m_ctx.style_stack.back());
        }

        void handle_indent() const
        {
            if (m_ctx.new_line)
            {
                m_ctx.os << write_args({ 0 });
                m_ctx.os << "\n" << std::string(m_ctx.indent_level * 2, ' ');
                m_ctx.new_line = false;
                m_ctx.os << change_style({}, m_ctx.style_stack.back());
            }
        }
    };

    struct impl_t
    {
        mutable context_t m_ctx;

        impl_t(std::ostream& out) : m_ctx{ out, 0, false }
        {
        }

        void operator()(const stream_t& stream) const
        {
            for (const auto& op : stream.m_ops)
            {
                std::visit(visitor_t{ m_ctx }, op);
            }
            if (m_ctx.new_line)
            {
                m_ctx.os << "\n";
            }
        }
    };

    auto operator()(std::ostream& out) const -> impl_t
    {
        return impl_t{ out };
    }
};

constexpr inline auto render = render_fn{};

constexpr inline struct indented_fn
{
    template <class... Ops>
    auto operator()(Ops&&... ops) const -> stream_t
    {
        return stream_t{}(indent, std::forward<Ops>(ops)..., unindent);
    }
} indented{};

constexpr inline struct line_fn
{
    template <class... Ops>
    auto operator()(Ops&&... ops) const -> stream_t
    {
        return stream_t{}(std::forward<Ops>(ops)..., new_line);
    }
} line{};

constexpr inline struct set_style_fn
{
    struct impl_t
    {
        font_style_t m_style;

        template <class... Ops>
        auto operator()(Ops&&... ops) const -> stream_t
        {
            return stream_t{} << push_style(m_style) << (... << std::forward<Ops>(ops)) << pop_style;
        }
    };

    auto operator()(font_style_t style) const -> impl_t
    {
        return { std::move(style) };
    }
} set_style{};

constexpr inline struct change_style_fn
{
    struct impl_t
    {
        font_style_applier_t m_applier;

        template <class... Ops>
        auto operator()(Ops&&... ops) const -> stream_t
        {
            return stream_t{} << modify_style(m_applier) << (... << std::forward<Ops>(ops)) << pop_style;
        }
    };

    auto operator()(font_style_applier_t applier) const -> impl_t
    {
        return { std::move(applier) };
    }
} change_style{};

template <class Func, class Range>
constexpr inline auto map(Func&& func, Range&& range) -> stream_t
{
    stream_t stream;
    for (auto&& item : range)
    {
        stream << func(std::forward<decltype(item)>(item));
    }
    return stream;
}

struct range_formatter_t
{
    template <class Range>
    void format(stream_t& stream, Range&& item) const
    {
        stream << "[";
        auto begin = std::begin(item);
        auto end = std::end(item);
        for (auto it = begin; it != end; ++it)
        {
            if (it != begin)
            {
                stream << ", ";
            }
            stream << *it;
        }
        stream << "]";
    }
};

struct ostream_formatter_t
{
    template <class T>
    void format(stream_t& stream, const T& item) const
    {
        std::stringstream ss;
        ss << item;
        stream << text(ss.str());
    }
};

struct tuple_formatter_t
{
    template <class T>
    void format(stream_t& stream, const T& item) const
    {
        std::apply(
            [&](const auto&... elems)
            {
                stream << "(";
                int n = 0;
                ((stream << (n++ ? ", " : "") << elems), ...);
                stream << ")";
            },
            item);
    }
};

template <class... Ts>
struct formatter_t<std::tuple<Ts...>> : tuple_formatter_t
{
};

template <class F, class S>
struct formatter_t<std::pair<F, S>> : tuple_formatter_t
{
};

template <class T>
struct formatter_t : ostream_formatter_t
{
};

template <>
struct formatter_t<bool>
{
    void format(stream_t& stream, bool item) const
    {
        stream << (item ? "true" : "false");
    }
};

template <std::size_t N>
struct formatter_t<char[N]>
{
    void format(stream_t& stream, const char (&item)[N]) const
    {
        stream << text_ref(item);
    }
};

template <>
struct formatter_t<const char*>
{
    void format(stream_t& stream, const char* item) const
    {
        stream << text_ref(item);
    }
};

template <>
struct formatter_t<std::string_view>
{
    void format(stream_t& stream, std::string_view item) const
    {
        stream << text_ref(item);
    }
};

template <>
struct formatter_t<std::string>
{
    void format(stream_t& stream, std::string item) const
    {
        stream << text(std::move(item));
    }
};

template <>
struct formatter_t<char>
{
    void format(stream_t& stream, char item) const
    {
        stream << text(std::string(1, item));
    }
};

template <class T>
struct formatter_t<std::vector<T>> : range_formatter_t
{
};

template <class T>
struct formatter_t<std::reference_wrapper<T>>
{
    void format(stream_t& stream, std::reference_wrapper<T> item) const
    {
        stream << item.get();
    }
};

template <>
struct formatter_t<std::reference_wrapper<std::string>>
{
    void format(stream_t& stream, std::reference_wrapper<std::string> item) const
    {
        stream << text_ref(item.get());
    }
};

template <>
struct formatter_t<std::reference_wrapper<const std::string>>
{
    void format(stream_t& stream, std::reference_wrapper<const std::string> item) const
    {
        stream << text_ref(item.get());
    }
};

}  // namespace ansi
