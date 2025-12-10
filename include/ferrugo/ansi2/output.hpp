#pragma once

#include <cstddef>
#include <ferrugo/ansi2/font_style.hpp>
#include <functional>
#include <memory>
#include <string>
#include <tuple>

namespace ansi
{

struct output_t;

template <class T, class = void>
struct formatter_t;

struct output_t;

struct output_applier_t : public std::function<void(output_t&)>
{
    using base_t = std::function<void(output_t&)>;

    using base_t::base_t;
};

struct output_t
{
    struct interface
    {
        virtual ~interface() = default;
        virtual void indent(std::size_t n) = 0;
        virtual void unindent() = 0;
        virtual void new_line() = 0;
        virtual void put(char32_t ch) = 0;
        virtual void flush() = 0;
        virtual font_style_t font_style() const = 0;
        virtual void push_font_style(const font_style_t& style) = 0;
        virtual void pop_font_style() = 0;
    };

    std::unique_ptr<interface> m_impl;

    explicit output_t(std::unique_ptr<interface> impl) : m_impl(std::move(impl))
    {
    }

    ~output_t()
    {
        flush();
    }

    font_style_t font_style() const
    {
        return m_impl->font_style();
    }

    output_t& indent(std::size_t n)
    {
        m_impl->indent(n);
        return *this;
    }

    output_t& unindent()
    {
        m_impl->unindent();
        return *this;
    }

    output_t& flush()
    {
        m_impl->flush();
        return *this;
    }

    output_t& new_line()
    {
        m_impl->new_line();
        return *this;
    }

    output_t& put(char32_t ch)
    {
        m_impl->put(ch);
        return *this;
    }

    output_t& push_font_style(const font_style_t& style)
    {
        m_impl->push_font_style(style);
        return *this;
    }

    output_t& modify_font_style(const font_style_applier_t& style_applier)
    {
        font_style_t style = font_style();
        style_applier(style);
        push_font_style(style);
        return *this;
    }

    output_t& pop_font_style()
    {
        m_impl->pop_font_style();
        return *this;
    }

    output_t& write(const std::string& str)
    {
        for (char ch : str)
        {
            if (ch == '\n')
            {
                new_line();
            }
            else
            {
                put(static_cast<char32_t>(ch));
            }
        }
        return *this;
    }

    template <class Applier>
    void apply(Applier&& applier)
    {
        if constexpr (std::is_invocable_v<std::decay_t<Applier>, output_t&>)
        {
            std::invoke(std::forward<Applier>(applier), *this);
        }
        else if constexpr (std::is_invocable_v<Applier, font_style_t&>)
        {
        }
        else
        {
            formatter_t<std::decay_t<Applier>>{}.format(*this, std::forward<Applier>(applier));
        }
    }

    template <class... Appliers>
    output_t& operator()(Appliers&&... appliers)
    {
        (apply(std::forward<Appliers>(appliers)), ...);
        return *this;
    }
};

template <>
struct formatter_t<const char*>
{
    void format(output_t& out, const char* item) const
    {
        out.write(item);
    }
};

template <class T>
struct sstream_formatter_t
{
    void format(output_t& out, const T& item) const
    {
        std::stringstream ss;
        ss << item;
        out.write(ss.str());
    }
};

template <>
struct formatter_t<int> : sstream_formatter_t<int>
{
};

template <>
struct formatter_t<char>
{
    void format(output_t& out, char item) const
    {
        out.put(item);
    }
};

}  // namespace ansi
