#pragma once

#include <array>
#include <cassert>
#include <cuchar>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace ferrugo
{
namespace ansi
{

struct multibyte
{
    std::array<char, 4> m_data;
    std::size_t m_size;

    multibyte() : m_data{}, m_size{ 0 }
    {
    }

    multibyte(char32_t ch) : m_data{}, m_size{}
    {
        auto state = std::mbstate_t{};
        m_size = std::c32rtomb(m_data.data(), ch, &state);
        if (m_size == std::size_t(-1))
            throw std::runtime_error{ "u32_to_mb: error in conversion" };
    }

    multibyte(const char* b, const char* e) : m_data{}, m_size{ std::size_t(e - b) }
    {
        assert(m_size <= 4);
        std::copy(b, e, m_data.begin());
    }

    operator char32_t() const
    {
        auto result = char32_t{};
        auto mb_state = std::mbstate_t{};
        auto const error = std::mbrtoc32(&result, m_data.data(), 4, &mb_state);
        if (error == std::size_t(-1))
            throw std::runtime_error{ "mb_to_u32: bad byte sequence" };
        if (error == std::size_t(-2))
            throw std::runtime_error{ "mb_to_u32: incomplete byte sequence" };
        return result;
    }

    std::size_t size() const
    {
        return m_size;
    }

    const char* begin() const
    {
        return m_data.data();
    }

    const char* end() const
    {
        return begin() + size();
    }

    friend std::ostream& operator<<(std::ostream& os, const multibyte& item)
    {
        for (std::size_t i = 0; i < item.m_size; ++i)
        {
            os << item.m_data[i];
        }
        return os;
    }
};

struct multibyte_string : private std::vector<multibyte>
{
    using base_type = std::vector<multibyte>;

    using base_type::empty;
    using base_type::size;
    using base_type::operator[];
    using base_type::at;
    using base_type::begin;
    using base_type::end;

    multibyte_string(const std::string& str)
    {
        std::setlocale(LC_ALL, "en_US.utf8");
        char32_t c32;
        const char* ptr = str.data();
        const char* end = str.data() + str.size();
        std::mbstate_t state{};
        while (std::size_t rc = std::mbrtoc32(&c32, ptr, end - ptr, &state))
        {
            assert(rc != (std::size_t)-3);
            if (rc == (std::size_t)-1)
                break;
            if (rc == (std::size_t)-2)
                break;
            this->emplace_back(ptr, ptr + rc);
            ptr += rc;
        }
    }

    operator std::string() const
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const multibyte_string& item)
    {
        for (const auto& mb : item)
        {
            os << mb;
        }
        return os;
    }
};

}  // namespace ansi
}  // namespace ferrugo