#pragma once

#include <ferrugo/ansi_v2/context.hpp>
#include <vector>

namespace ferrugo
{
namespace ansi_v2
{

class default_context_t : public context_t
{
public:
    explicit default_context_t(std::ostream& os) : m_os{ &os }, m_style_stack{ style_t{} }
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

private:
    void change_style(const style_t& prev_style, const style_t& new_style)
    {
        if (prev_style.font != new_style.font)
        {
            *m_os << esc(to_args(new_style.font - prev_style.font));
        }
        if (prev_style.foreground != new_style.foreground)
        {
            *m_os << esc(to_args(foreground(new_style.foreground)));
        }
        if (prev_style.background != new_style.background)
        {
            *m_os << esc(to_args(background(new_style.background)));
        }
    }

private:
    std::ostream* m_os;
    std::vector<style_t> m_style_stack;
};

}  // namespace ansi_v2
}  // namespace ferrugo
