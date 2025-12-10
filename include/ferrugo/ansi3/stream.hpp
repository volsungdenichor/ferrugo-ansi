#pragma once

#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "font_style.hpp"

struct new_line_t
{
};

struct indent_t
{
};

struct unindent_t
{
};

struct text_t
{
    std::string content;
};

struct text_ref_t
{
    std::string_view content;
};

struct push_style_t
{
    font_style_t style;
};

struct pop_style_t
{
};

using stream_op_t = std::variant<new_line_t, indent_t, unindent_t, text_t, text_ref_t, push_style_t, pop_style_t>;

constexpr inline auto new_line = new_line_t{};

constexpr inline auto indent = indent_t{};

constexpr inline auto unindent = unindent_t{};

constexpr inline auto text = [](std::string content) { return text_t{ std::move(content) }; };

constexpr inline auto text_ref = [](std::string_view content) { return text_ref_t{ content }; };

constexpr inline auto push_style = [](font_style_t style) { return push_style_t{ std::move(style) }; };

constexpr inline auto pop_style = pop_style_t{};

template <class T>
struct formatter_t;

struct stream_t
{
    std::vector<stream_op_t> m_ops;

    stream_t() = default;
    stream_t(const stream_t&) = default;
    stream_t(stream_t&&) noexcept = default;

    template <class... Tail>
    stream_t(stream_op_t head, Tail&&... tail) : m_ops{ std::move(head), std::forward<Tail>(tail)... }
    {
    }

    stream_t& operator<<(const stream_op_t& op)
    {
        m_ops.push_back(op);
        return *this;
    }

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

    template <class T, std::enable_if_t<!std::is_constructible_v<stream_op_t, T>, int> = 0>
    stream_t& operator<<(const T& item)
    {
        formatter_t<T>{}.format(*this, item);
        return *this;
    }
};

template <class T>
void do_format_to(stream_t& stream, const T& item)
{
    formatter_t<T>{}.format(stream, item);
}

template <class... Args>
void format_to(stream_t& stream, const Args&... args)
{
    (do_format_to(stream, args), ...);
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
            format_to(stream, *it);
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
struct formatter_t<int> : ostream_formatter_t
{
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
        format_to(stream, item.get());
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

template <class... Ops>
auto indented(Ops&&... ops) -> stream_t
{
    stream_t stream;
    stream << indent_t{};
    (stream << ... << std::forward<Ops>(ops));
    stream << unindent_t{};
    return stream;
}

template <class Head, class... Tail>
void make_list(stream_t& stream, Head&& head, Tail&&... tail)
{
    stream << std::forward<Head>(head) << new_line;
    if constexpr (sizeof...(tail) > 0)
    {
        make_list(stream, std::forward<Tail>(tail)...);
    }
}

template <class... Ops>
auto list(Ops&&... ops) -> stream_t
{
    stream_t stream;
    make_list(stream, std::forward<Ops>(ops)...);
    return stream;
}

template <class... Ops>
auto line(Ops&&... ops) -> stream_t
{
    stream_t stream;
    (stream << ... << std::forward<Ops>(ops));
    stream << new_line;
    return stream;
}
