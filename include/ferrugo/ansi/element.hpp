#pragma once

#include <ferrugo/ansi/context.hpp>
#include <ferrugo/ansi/style.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace ferrugo
{
namespace ansi
{

struct element_t
{
    using function_type = std::function<void(context_t&)>;

    function_type m_fn;

    explicit element_t(const function_type& fn) : m_fn{ std::move(fn) }
    {
    }

    element_t(const mb_string& text) : element_t{ [=](context_t& ctx) { ctx.write_text(text); } }
    {
    }

    element_t(const std::string& text) : element_t{ mb_string{ text } }
    {
    }

    element_t(const char* text) : element_t{ std::string{ text } }
    {
    }

    void apply(context_t& ctx) const
    {
        m_fn(ctx);
    }
};

namespace detail
{

template <class... Args>
auto to_elements(const Args&... args) -> std::vector<element_t>
{
    return std::vector<element_t>{ element_t{ args }... };
}

struct block_fn
{
    auto operator()(std::vector<element_t> children) const -> element_t
    {
        return element_t{ [=](context_t& ctx)
                          {
                              for (const element_t& child : children)
                              {
                                  child.apply(ctx);
                              }
                          } };
    }

    template <class... Args>
    auto operator()(const Args&... args) const -> element_t
    {
        return (*this)(to_elements(args...));
    }
};

struct style_applier_fn
{
    using style_modifier_t = std::function<void(style_t&)>;
    style_modifier_t m_modifier;

    explicit style_applier_fn(style_modifier_t modifier) : m_modifier{ std::move(modifier) }
    {
    }

    auto operator()(std::vector<element_t> children) const -> element_t
    {
        return element_t{ [=](context_t& ctx)
                          {
                              style_t new_style = ctx.get_current_style();
                              m_modifier(new_style);
                              ctx.push_style(new_style);
                              for (const element_t& child : children)
                              {
                                  child.apply(ctx);
                              }
                              ctx.pop_style();
                          } };
    }

    template <class... Args>
    auto operator()(const Args&... args) const -> element_t
    {
        return (*this)(to_elements(args...));
    }

    friend style_applier_fn operator|(style_applier_fn lhs, style_applier_fn rhs)
    {
        return style_applier_fn{ [=](style_t& s)
                                 {
                                     lhs.m_modifier(s);
                                     rhs.m_modifier(s);
                                 } };
    }
};

template <ground_type_t G>
struct set_color_fn;

template <>
struct set_color_fn<ground_type_t::foreground>
{
    auto operator[](const color_t& value) const -> style_applier_fn
    {
        return style_applier_fn{ [=](style_t& s) { s.foreground = value; } };
    }
};

template <>
struct set_color_fn<ground_type_t::background>
{
    auto operator[](const color_t& value) const -> style_applier_fn
    {
        return style_applier_fn{ [=](style_t& s) { s.background = value; } };
    }
};

struct font_fn
{
    auto operator[](font_t value) const -> style_applier_fn
    {
        return style_applier_fn{ [=](style_t& s) { s.font = value; } };
    }
};

}  // namespace detail

static constexpr inline auto block = detail::block_fn{};
static constexpr inline auto font = detail::font_fn{};

static constexpr auto fg = detail::set_color_fn<ground_type_t::foreground>{};
static constexpr auto bg = detail::set_color_fn<ground_type_t::background>{};

static const inline auto bold = font[font_t::bold];
static const inline auto italic = font[font_t::italic];
static const inline auto underline = font[font_t::underline];
static const inline auto dim = font[font_t::dim];
static const inline auto inverse = font[font_t::inverse];
static const inline auto crossed_out = font[font_t::crossed_out];

}  // namespace ansi
}  // namespace ferrugo
