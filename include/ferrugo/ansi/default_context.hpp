#pragma once

#include <ferrugo/ansi/context.hpp>
#include <functional>
#include <vector>

namespace ferrugo
{
namespace ansi
{

using list_state_t = std::vector<std::size_t>;

using list_item_formatter_t = std::function<void(context_t&, const list_state_t&)>;
using list_item_formatter_factory_t = std::function<list_item_formatter_t(std::size_t)>;

class default_context_t : public context_t
{
public:
    explicit default_context_t(std::ostream& os, list_item_formatter_factory_t list_item_formatter_factory)
        : m_os{ &os }
        , m_style_stack{ style_t{} }
        , m_list_state{}
        , m_list_item_formatter_factory(std::move(list_item_formatter_factory))
    {
    }

    style_t get_current_style() const override
    {
        assert(!m_style_stack.empty());
        return m_style_stack.back();
    }

    void push_style(const style_t& style) override
    {
        const style_t current_style = get_current_style();
        m_style_stack.push_back(style);
        change_style(current_style, style);
    }

    void pop_style() override
    {
        assert(m_style_stack.size() >= 2);
        const style_t current_style = get_current_style();
        m_style_stack.pop_back();
        const style_t prev_style = m_style_stack.back();
        change_style(current_style, prev_style);
    }

    void write_text(const mb_string& text) override
    {
        *m_os << text;
    }

    void indent() override
    {
    }

    void unindent() override
    {
    }

    void new_line() override
    {
        write_text(mb_string("\n"));
    }

    void on_list_start() override
    {
        m_list_state.push_back(0);
    }

    void on_list_end() override
    {
        m_list_state.pop_back();
    }

    void on_list_item_start() override
    {
        const list_item_formatter_t formatter = m_list_item_formatter_factory(m_list_state.size());
        formatter(*this, m_list_state);
    }

    void on_list_item_end() override
    {
        ++m_list_state.back();
    }

private:
    void change_style(const style_t& prev_style, const style_t& new_style)
    {
        if (prev_style.font != new_style.font)
        {
            *m_os << (new_style.font - prev_style.font);
        }
        if (prev_style.foreground != new_style.foreground)
        {
            *m_os << foreground(new_style.foreground);
        }
        if (prev_style.background != new_style.background)
        {
            *m_os << background(new_style.background);
        }
    }

private:
    std::ostream* m_os;
    std::vector<style_t> m_style_stack;
    list_state_t m_list_state;
    list_item_formatter_factory_t m_list_item_formatter_factory;
};

}  // namespace ansi
}  // namespace ferrugo
