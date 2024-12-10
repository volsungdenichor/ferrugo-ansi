#include <ferrugo/ansi/default_context.hpp>
#include <ferrugo/ansi/element.hpp>
#include <ferrugo/ansi/format.hpp>
#include <ferrugo/ansi/formatters.hpp>
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

    ctx << block(
        fg["00FFFF"]("Europe"),  //
        list(                    //
            "Germany",
            "France",
            block(  //
                fg["00FF00"]("Poland"),
                list(  //
                    "Warsaw",
                    "Kraków",
                    fg["FF0000"]("Wrocław")))))
        << text("\nAla ma {} kota oraz {} psa.", 1.5, "kosmatego");
    std::cout << "\n";
}
