#pragma once

#include <ferrugo/ansi_v2/mb_string.hpp>
#include <ferrugo/ansi_v2/style.hpp>

namespace ferrugo
{

namespace ansi_v2
{

struct context_t
{
    virtual ~context_t() = default;
    virtual style_t get_current_style() const = 0;
    virtual void push_style(const style_t& style) = 0;
    virtual void pop_style() = 0;
    virtual void write_text(const mb_string& text) = 0;
};

}  // namespace ansi_v2
}  // namespace ferrugo
