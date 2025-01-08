#pragma once

#include <ferrugo/ansi/format.hpp>
#include <sstream>
#include <tuple>
#include <vector>

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

template <class T>
struct struct_formatter
{
    struct field_info
    {
        using field_formatter_fn = std::function<void(context_t&, const T&)>;

        std::string_view name;
        field_formatter_fn field_formater;

        template <class Type>
        field_info(std::string_view name, Type T::*field)
            : name{ name }
            , field_formater{ [=](context_t& ctx, const T& item) { write(ctx, item.*field); } }
        {
        }
    };

    struct_formatter(std::string_view name, std::vector<field_info> fields) : m_name{ name }, m_fields{ std::move(fields) }
    {
    }

    void parse(const parse_context& ctx)
    {
    }

    void format(context_t& ctx, const T& item) const
    {
        write(ctx, "(", m_name, " ");
        for (const field_info& field : m_fields)
        {
            write(ctx, "(", field.name, " ");
            field.field_formater(ctx, item);
            write(ctx, ")");
        }
        write(ctx, ")");
    }

    std::string_view m_name;
    std::vector<field_info> m_fields;
};

struct range_formatter
{
    void parse(const parse_context& ctx)
    {
    }

    template <class T>
    void format(context_t& ctx, const T& item) const
    {
        write(ctx, "[");
        auto it = std::begin(item);
        const auto end = std::end(item);
        if (it != end)
        {
            write(ctx, *it++);
        }
        for (; it != end; ++it)
        {
            write(ctx, " ");
            write(ctx, *it);
        }
        write(ctx, "]");
    }
};

struct tuple_formatter
{
    void parse(const parse_context& ctx)
    {
    }

    template <class T>
    void format(context_t& ctx, const T& item) const
    {
        write(ctx, "(");
        std::apply(
            [&ctx](const auto&... args)
            {
                auto n = 0u;
                (write(ctx, args, (++n != sizeof...(args) ? " " : "")), ...);
            },
            item);
        write(ctx, ")");
    }
};

template <class... Args>
struct formatter<std::vector<Args...>> : range_formatter
{
};

template <class... Args>
struct formatter<std::tuple<Args...>> : tuple_formatter
{
};

template <class... Args>
struct formatter<std::pair<Args...>> : tuple_formatter
{
};

}  // namespace ansi
}  // namespace ferrugo
