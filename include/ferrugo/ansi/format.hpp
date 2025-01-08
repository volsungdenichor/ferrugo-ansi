#pragma once

#include <algorithm>
#include <ferrugo/ansi/context.hpp>
#include <ferrugo/ansi/element.hpp>
#include <functional>
#include <iostream>
#include <string_view>

namespace ferrugo
{
namespace ansi
{

struct format_error : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

template <class T, class = void>
struct formatter;

template <class... Args>
void write(context_t& format_ctx, const Args&... args)
{
    (formatter<Args>{}.format(format_ctx, args), ...);
}

class parse_context
{
public:
    explicit parse_context(std::string_view specifier) : m_specifier{ specifier }
    {
    }

    std::string_view specifier() const
    {
        return m_specifier;
    }

private:
    std::string_view m_specifier;
};

struct arg_ref
{
    using arg_printer_t = void (*)(context_t&, const void*, const parse_context&);
    arg_printer_t m_printer;
    const void* m_ptr;

    template <class T>
    explicit arg_ref(const T& item)
        : m_printer{ [](context_t& format_ctx, const void* ptr, const parse_context& parse_ctx)
                     {
                         formatter<T> f{};
                         f.parse(parse_ctx);
                         f.format(format_ctx, *static_cast<const T*>(ptr));
                     } }
        , m_ptr{ std::addressof(item) }
    {
    }

    arg_ref(const arg_ref&) = default;
    arg_ref(arg_ref&&) = default;

    void print(context_t& format_ctx, const parse_context& parse_ctx) const
    {
        m_printer(format_ctx, m_ptr, parse_ctx);
    }
};

template <class... Args>
auto wrap_args(const Args&... args) -> std::vector<arg_ref>
{
    std::vector<arg_ref> result;
    result.reserve(sizeof...(args));
    (result.push_back(arg_ref{ args }), ...);
    return result;
}

class format_string
{
private:
    struct print_text
    {
        std::string_view text;
    };

    struct print_argument
    {
        int index;
        parse_context context;
    };

    using print_action = std::variant<print_text, print_argument>;

    struct format_visitor
    {
        context_t& m_ctx;
        const std::vector<arg_ref>& m_arguments;

        void operator()(const print_text& arg) const
        {
            m_ctx.write_text(mb_string(arg.text));
        }

        void operator()(const print_argument& arg) const
        {
            m_arguments.at(arg.index).print(m_ctx, arg.context);
        }
    };

    struct print_visitor
    {
        std::ostream& m_os;

        void operator()(const print_text& arg) const
        {
            m_os << arg.text;
        }

        void operator()(const print_argument& arg) const
        {
            if (arg.context.specifier().empty())
            {
                m_os << "{" << arg.index << "}";
            }
            else
            {
                m_os << "{" << arg.index << ":" << arg.context.specifier() << "}";
            }
        }
    };

public:
    explicit format_string(std::string_view fmt) : m_actions{ parse(fmt) }
    {
    }

    void format(context_t& format_ctx, const std::vector<arg_ref>& arguments) const
    {
        for (const auto& action : m_actions)
        {
            std::visit(format_visitor{ format_ctx, arguments }, action);
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const format_string& item)
    {
        for (const auto& action : item.m_actions)
        {
            std::visit(print_visitor{ os }, action);
        }
        return os;
    }

private:
    std::vector<print_action> m_actions;

    static auto parse(std::string_view fmt) -> std::vector<print_action>
    {
        static const auto is_opening_bracket = [](char c) { return c == '{'; };
        static const auto is_closing_bracket = [](char c) { return c == '}'; };
        static const auto is_bracket = [](char c) { return is_opening_bracket(c) || is_closing_bracket(c); };
        static const auto is_colon = [](char c) { return c == ':'; };
        std::vector<print_action> result;
        int arg_index = 0;
        while (!fmt.empty())
        {
            const auto begin = std::begin(fmt);
            const auto end = std::end(fmt);
            const auto bracket = std::find_if(begin, end, is_bracket);
            if (bracket == end)
            {
                result.push_back(print_text{ fmt });
                fmt = make_string_view(bracket, end);
            }
            else if (bracket[0] == bracket[1])
            {
                result.push_back(print_text{ make_string_view(begin, bracket + 1) });
                fmt = make_string_view(bracket + 2, end);
            }
            else if (is_opening_bracket(bracket[0]))
            {
                const auto closing_bracket = std::find_if(bracket + 1, end, is_closing_bracket);
                if (closing_bracket == end)
                {
                    throw format_error{ "unclosed bracket" };
                }
                result.push_back(print_text{ make_string_view(begin, bracket) });

                const auto [actual_index, fmt_specifer] = std::invoke(
                    [](std::string_view arg, int current_index) -> std::tuple<int, std::string_view>
                    {
                        const auto colon = std::find_if(std::begin(arg), std::end(arg), is_colon);
                        const auto index_part = make_string_view(std::begin(arg), colon);
                        const auto fmt_part = make_string_view(colon != std::end(arg) ? colon + 1 : colon, std::end(arg));
                        const auto index = !index_part.empty() ? parse_int(index_part) : current_index;
                        return { index, fmt_part };
                    },
                    make_string_view(bracket + 1, closing_bracket),
                    arg_index);
                result.push_back(print_argument{ actual_index, parse_context{ fmt_specifer } });
                fmt = make_string_view(closing_bracket + 1, end);
                ++arg_index;
            }
        }
        return result;
    }

    static auto make_string_view(std::string_view::iterator b, std::string_view::iterator e) -> std::string_view
    {
        if (b < e)
            return { std::addressof(*b), std::string_view::size_type(e - b) };
        else
            return {};
    }

    static auto parse_int(std::string_view txt) -> int
    {
        int result = 0;
        for (char c : txt)
        {
            assert('0' <= c && c <= '9');
            result = result * 10 + (c - '0');
        }
        return result;
    }
};

struct text_fn
{
    struct impl_fn
    {
        format_string m_formatter;

        template <class... Args>
        auto operator()(Args&&... args) const -> element_t
        {
            const auto wrapped_args = wrap_args(std::forward<Args>(args)...);
            return [=](context_t& ctx) { m_formatter.format(ctx, wrapped_args); };
        }

        friend std::ostream& operator<<(std::ostream& os, const impl_fn& item)
        {
            return os << item.m_formatter;
        }
    };

    template <class... Args>
    auto operator()(std::string_view fmt, Args&&... args) const -> element_t
    {
        const auto formatter = format_string{ fmt };
        const auto wrapped_args = wrap_args(std::forward<Args>(args)...);
        return [=](context_t& ctx) { formatter.format(ctx, wrapped_args); };
    }
};

static constexpr inline auto text = text_fn{};

}  // namespace ansi
}  // namespace ferrugo
