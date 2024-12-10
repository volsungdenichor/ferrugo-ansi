#pragma once

#include <ferrugo/ansi/format.hpp>
#include <sstream>

namespace ferrugo
{

namespace ansi
{

template <class T>
struct ostream_formatter
{
    void parse(const parse_context&)
    {
    }

    void format(context_t& ctx, const T& item) const
    {
        std::stringstream ss;
        ss << item;
        ctx.write_text(mb_string(ss.str()));
    }
};

template <char... Fmt>
struct sprintf_formatter
{
    void parse(const parse_context&)
    {
    }

    template <class T>
    void format(context_t& ctx, T item) const
    {
        static const char fmt[] = { '%', Fmt..., '\0' };
        char buffer[64];
        int chars_written = std::sprintf(buffer, fmt, item);
        ctx.write_text(mb_string(std::string(buffer, chars_written)));
    }
};

template <class T>
struct formatter<T, std::enable_if_t<std::is_integral_v<T>>>
{
    void parse(const parse_context&)
    {
    }

    void format(context_t& ctx, T item) const
    {
        ctx.write_text(mb_string(std::to_string(item)));
    }
};

template <>
struct formatter<int> : sprintf_formatter<'d'>
{
};

template <>
struct formatter<long> : sprintf_formatter<'l', 'd'>
{
};

template <>
struct formatter<long long> : sprintf_formatter<'l', 'l', 'd'>
{
};

template <>
struct formatter<unsigned> : sprintf_formatter<'u'>
{
};

template <>
struct formatter<unsigned long> : sprintf_formatter<'l', 'u'>
{
};

template <>
struct formatter<unsigned long long> : sprintf_formatter<'l', 'l', 'u'>
{
};

template <>
struct formatter<float> : sprintf_formatter<'f'>
{
};

template <>
struct formatter<double> : sprintf_formatter<'f'>
{
};

template <>
struct formatter<long double> : sprintf_formatter<'L', 'f'>
{
};

template <>
struct formatter<bool>
{
    void parse(const parse_context&)
    {
    }

    void format(context_t& ctx, bool item) const
    {
        ctx.write_text(mb_string(item ? "true" : "false"));
    }
};

template <>
struct formatter<std::string>
{
    void parse(const parse_context&)
    {
    }

    void format(context_t& ctx, const std::string& item) const
    {
        ctx.write_text(mb_string(item));
    }
};

template <>
struct formatter<std::string_view>
{
    void parse(const parse_context&)
    {
    }

    void format(context_t& ctx, std::string_view item) const
    {
        ctx.write_text(mb_string(item));
    }
};

template <>
struct formatter<const char*>
{
    void parse(const parse_context&)
    {
    }

    void format(context_t& ctx, const char* item) const
    {
        ctx.write_text(mb_string(item));
    }
};

template <>
struct formatter<char>
{
    void parse(const parse_context&)
    {
    }

    void format(context_t& ctx, const char item) const
    {
        ctx.write_text(mb_string(std::string(1, item)));
    }
};

template <std::size_t N>
struct formatter<char[N]>
{
    void parse(const parse_context&)
    {
    }

    void format(context_t& ctx, const char (&item)[N]) const
    {
        ctx.write_text(mb_string(item));
    }
};

}  // namespace ansi
}  // namespace ferrugo
