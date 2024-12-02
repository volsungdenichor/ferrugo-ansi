#include <ferrugo/ansi/default_context.hpp>
#include <ferrugo/ansi/element.hpp>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <string>

int main()
{
    using namespace ferrugo::ansi;

    auto ctx = default_context_t{ std::cout };

    const auto e = (fg[basic_color_t::red] | bg[basic_color_t::white])("Ala", "ma", "kota");

    e.apply(ctx);
    std::cout << "\n";
}
