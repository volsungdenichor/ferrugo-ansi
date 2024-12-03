#pragma once

#include <ferrugo/ansi/mb_string.hpp>
#include <ferrugo/ansi/style.hpp>

namespace ferrugo
{

namespace ansi
{

struct context_t
{
    virtual ~context_t() = default;
    virtual style_t get_current_style() const = 0;
    virtual void push_style(const style_t& style) = 0;
    virtual void pop_style() = 0;
    virtual void write_text(const mb_string& text) = 0;
    virtual void push_list() = 0;
    virtual void pop_list() = 0;
    virtual void start_list_item() = 0;
    virtual void end_list_item() = 0;
};

}  // namespace ansi
}  // namespace ferrugo
