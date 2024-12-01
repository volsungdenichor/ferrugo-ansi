#pragma once

#include <ferrugo/ansi_v2/context.hpp>
#include <ferrugo/ansi_v2/style.hpp>
#include <functional>
#include <vector>

namespace ferrugo
{
namespace ansi_v2
{

struct element_t
{
    using function_type = std::function<void(context_t&)>;

    function_type m_fn;

    explicit element_t(function_type fn) : m_fn{ std::move(fn) }
    {
        assert(m_fn);
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

    void operator()(context_t& ctx) const
    {
        m_fn(ctx);
    }
};

template <class... Args>
auto to_elements(Args&&... args) -> std::vector<element_t>
{
    return std::vector<element_t>{ element_t{ std::forward<Args>(args) }... };
}

struct block_fn
{
    auto operator()(std::vector<element_t> children) const -> element_t
    {
        return element_t{ [=](context_t& ctx)
                          {
                              for (const element_t& ch : children)
                              {
                                  ch(ctx);
                              }
                          } };
    }

    template <class... Args>
    auto operator()(Args&&... args) const -> element_t
    {
        return (*this)(to_elements(std::forward<Args>(args)...));
    }
};

using style_modifier_t = std::function<void(style_t&)>;

style_modifier_t operator|(style_modifier_t lhs, style_modifier_t rhs)
{
    return [=](style_t& style)
    {
        lhs(style);
        rhs(style);
    };
}

struct style_applier_fn
{
    style_modifier_t m_modifier;

    explicit style_applier_fn(style_modifier_t modifier) : m_modifier{ std::move(modifier) }
    {
        assert(m_modifier);
    }

    auto operator()(std::vector<element_t> children) const -> element_t
    {
        return element_t{ [=](context_t& ctx)
                          {
                              const auto new_style = std::invoke(
                                  [&]() -> style_t
                                  {
                                      auto s = ctx.get_current_style();
                                      m_modifier(s);
                                      return s;
                                  });
                              ctx.push_style(new_style);
                              for (const element_t& ch : children)
                              {
                                  ch(ctx);
                              }
                              ctx.pop_style();
                          } };
    }

    template <class... Args>
    auto operator()(Args&&... args) const -> element_t
    {
        return (*this)(to_elements(std::forward<Args>(args)...));
    }

    friend style_applier_fn operator|(const style_applier_fn& lhs, const style_applier_fn& rhs)
    {
        return style_applier_fn{ lhs.m_modifier | rhs.m_modifier };
    }
};

struct fg_fn
{
    auto operator[](const color_t& value) const -> style_applier_fn
    {
        return style_applier_fn{ [=](style_t& style) { style.foreground = value; } };
    }
};

struct bg_fn
{
    auto operator[](const color_t& value) const -> style_applier_fn
    {
        return style_applier_fn{ [=](style_t& style) { style.background = value; } };
    }
};

struct font_fn
{
    auto operator()(font_t value) const -> style_applier_fn
    {
        return style_applier_fn{ [=](style_t& style) { style.font = value; } };
    }
};

static constexpr inline auto block = block_fn{};
static constexpr inline auto font = font_fn{};

static constexpr auto fg = fg_fn{};
static constexpr auto bg = bg_fn{};

static const inline auto bold = font(font_t::bold);
static const inline auto italic = font(font_t::italic);
static const inline auto underline = font(font_t::underline);
static const inline auto dim = font(font_t::dim);
static const inline auto inverse = font(font_t::inverse);
static const inline auto crossed_out = font(font_t::crossed_out);

}  // namespace ansi_v2
}  // namespace ferrugo
