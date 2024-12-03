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

    const auto e = block(
        fg["00FFFF"]("Europe"),  //
        list(                    //
            "Germany",
            "France",
            block(  //
                fg["00FF00"]("Poland"),
                list(  //
                    "Warsaw",
                    "Krakow",
                    fg["FF0000"]("Wroclaw")))));

    ctx << e;
    std::cout << "\n";
}
