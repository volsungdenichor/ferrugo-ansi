#pragma once

#include <ferrugo/ansi/context.hpp>
#include <functional>
#include <vector>

namespace ferrugo
{
namespace ansi
{

using list_item_formatter = std::function<void(context_t&, std::size_t, std::size_t)>;

class default_context_t : public context_t
{
public:
    explicit default_context_t(std::ostream& os) : m_os{ &os }, m_style_stack{ style_t{} }, m_indent{}, m_list_state{}
    {
    }

    style_t get_current_style() const override
    {
        assert(!m_style_stack.empty());
        return m_style_stack.back();
    }

    void push_style(const style_t& style) override
    {
        const auto current_style = get_current_style();
        m_style_stack.push_back(style);
        change_style(current_style, style);
    }

    void pop_style() override
    {
        assert(m_style_stack.size() >= 2);
        const auto current_style = get_current_style();
        m_style_stack.pop_back();
        const auto prev_style = m_style_stack.back();
        change_style(current_style, prev_style);
    }

    void write_text(const mb_string& text) override
    {
        *m_os << text;
    }

    void push_list() override
    {
        ++m_indent;
        m_list_state.push_back(0);
    }

    void pop_list() override
    {
        --m_indent;
        m_list_state.pop_back();
    }

    void start_list_item() override
    {
        const list_item_formatter formatter = get_list_item_formatter(m_list_state.size());
        formatter(*this, m_list_state.back(), m_list_state.size());
        ++m_list_state.back();
    }

    void end_list_item() override
    {
    }

private:
    list_item_formatter get_list_item_formatter(std::size_t level) const
    {
        return [](context_t& ctx, std::size_t n, std::size_t indent)
        {
            ctx.write_text(mb_string("\n"));
            ctx.write_text(mb_string(std::string(2 * indent, ' ')));
            ctx.write_text(mb_string("▪️ "));
            ctx.write_text(mb_string(std::to_string(n + 1)));
            ctx.write_text(mb_string(". "));
        };
    }

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
    std::size_t m_indent;
    std::vector<std::size_t> m_list_state;
};

}  // namespace ansi
}  // namespace ferrugo
