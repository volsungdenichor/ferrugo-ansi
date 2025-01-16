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

using element_t = std::function<void(context_t&)>;

inline context_t& operator<<(context_t& ctx, const element_t& item)
{
    item(ctx);
    return ctx;
}

namespace detail
{

struct to_element_fn
{
    auto operator()(const mb_string& text) const -> element_t
    {
        return [=](context_t& ctx) { ctx.write_text(text); };
    }

    auto operator()(const std::string& text) const -> element_t
    {
        return (*this)(mb_string(text));
    }

    auto operator()(element_t element) const -> element_t
    {
        return element;
    }
};

static constexpr inline auto to_element = to_element_fn{};

template <class... Args>
auto to_elements(Args&&... args) -> std::vector<element_t>
{
    return std::vector<element_t>{ to_element(std::forward<Args>(args))... };
}

struct block_fn
{
    auto operator()(std::vector<element_t> children) const -> element_t
    {
        return element_t{ [=](context_t& ctx)
                          {
                              for (const element_t& child : children)
                              {
                                  child(ctx);
                              }
                          } };
    }

    template <class... Args>
    auto operator()(Args&&... args) const -> element_t
    {
        return (*this)(to_elements(std::forward<Args>(args)...));
    }
};

struct list_fn
{
    auto operator()(std::vector<element_t> children) const -> element_t
    {
        return element_t{ [=](context_t& ctx)
                          {
                              ctx.on_list_start();
                              for (const element_t& child : children)
                              {
                                  ctx.on_list_item_start();
                                  child(ctx);
                                  ctx.on_list_item_end();
                              }
                              ctx.on_list_end();
                          } };
    }

    template <class... Args>
    auto operator()(Args&&... args) const -> element_t
    {
        return (*this)(to_elements(std::forward<Args>(args)...));
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
        return element_t{ [=, *this](context_t& ctx)
                          {
                              const style_t new_style = std::invoke(
                                  [&]() -> style_t
                                  {
                                      style_t res = ctx.get_current_style();
                                      m_modifier(res);
                                      return res;
                                  });

                              ctx.push_style(new_style);
                              for (const element_t& child : children)
                              {
                                  child(ctx);
                              }
                              ctx.pop_style();
                          } };
    }

    template <class... Args>
    auto operator()(Args&&... args) const -> element_t
    {
        return (*this)(to_elements(std::forward<Args>(args)...));
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
static constexpr inline auto list = detail::list_fn{};
static constexpr inline auto font = detail::font_fn{};

static constexpr auto fg = detail::set_color_fn<ground_type_t::foreground>{};
static constexpr auto bg = detail::set_color_fn<ground_type_t::background>{};

static const inline auto bold = font[font_t::bold];
static const inline auto italic = font[font_t::italic];
static const inline auto underline = font[font_t::underline];
static const inline auto dim = font[font_t::dim];
static const inline auto inverse = font[font_t::inverse];
static const inline auto crossed_out = font[font_t::crossed_out];
static const inline auto blink = font[font_t::blink];
static const inline auto hidden = font[font_t::hidden];

}  // namespace ansi
}  // namespace ferrugo
