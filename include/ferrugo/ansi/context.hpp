#pragma once

#include <ferrugo/ansi/mb_string.hpp>
#include <ferrugo/ansi/style.hpp>

namespace ferrugo
{

namespace ansi
{

struct styled_context_t
{
    virtual ~styled_context_t() = default;

    virtual style_t get_current_style() const = 0;
    virtual void push_style(const style_t& style) = 0;
    virtual void pop_style() = 0;
};

struct list_context_t
{
    virtual ~list_context_t() = default;

    virtual void on_list_start() = 0;
    virtual void on_list_end() = 0;
    virtual void on_list_item_start() = 0;
    virtual void on_list_item_end() = 0;
};

struct context_t : public styled_context_t, public list_context_t
{
    virtual ~context_t() = default;
    virtual void write_text(const mb_string& text) = 0;
    virtual void indent() = 0;
    virtual void unindent() = 0;
    virtual void new_line() = 0;
};

}  // namespace ansi
}  // namespace ferrugo
