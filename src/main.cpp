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

    const auto e = block((fg[bright_color_t::white] | bg[basic_color_t::yellow])("Ala", underline("ma"), "kota"));

    ctx << e;
    std::cout << "\n";
}
