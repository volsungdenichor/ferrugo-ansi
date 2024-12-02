#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <cuchar>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace ferrugo
{
namespace ansi
{

struct mb_char
{
    std::array<char, 4> m_data;
    std::uint8_t m_size;

    mb_char() : m_data{}, m_size{ 0 }
    {
    }

    mb_char(char32_t ch) : m_data{}, m_size{}
    {
        auto state = std::mbstate_t{};
        m_size = std::c32rtomb(m_data.data(), ch, &state);
        if (m_size == std::size_t(-1))
        {
            throw std::runtime_error{ "u32_to_mb: error in conversion" };
        }
    }

    mb_char(const char* b, const char* e) : m_data{}, m_size{ std::uint8_t(e - b) }
    {
        assert(m_size <= 4);
        std::copy(b, e, std::begin(m_data));
    }

    operator char32_t() const
    {
        auto c32 = char32_t{};
        auto mb_state = std::mbstate_t{};
        auto const error = std::mbrtoc32(&c32, m_data.data(), 4, &mb_state);
        if (error == std::size_t(-1))
        {
            throw std::runtime_error{ "mb_to_u32: bad byte sequence" };
        }
        if (error == std::size_t(-2))
        {
            throw std::runtime_error{ "mb_to_u32: incomplete byte sequence" };
        }
        return c32;
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

    friend std::ostream& operator<<(std::ostream& os, const mb_char& item)
    {
        std::copy(std::begin(item), std::end(item), std::ostream_iterator<char>{ os });
        return os;
    }
};

struct mb_string : private std::vector<mb_char>
{
    using base_t = std::vector<mb_char>;

    using base_t::empty;
    using base_t::size;
    using base_t::operator[];
    using base_t::at;
    using base_t::begin;
    using base_t::end;

    mb_string(std::string_view str)
    {
        std::setlocale(LC_ALL, "en_US.utf8");
        char32_t c32 = {};
        const char* ptr = str.data();
        const char* end = str.data() + str.size();
        std::mbstate_t state{};
        while (std::size_t rc = std::mbrtoc32(&c32, ptr, end - ptr, &state))
        {
            assert(rc != (std::size_t)-3);
            if (rc == std::size_t(-1))
            {
                break;
            }
            if (rc == std::size_t(-2))
            {
                break;
            }
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

    friend std::ostream& operator<<(std::ostream& os, const mb_string& item)
    {
        std::copy(std::begin(item), std::end(item), std::ostream_iterator<mb_char>{ os });
        return os;
    }
};

}  // namespace ansi
}  // namespace ferrugo
