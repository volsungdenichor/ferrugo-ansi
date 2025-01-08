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

struct Person
{
    std::string name;
    std::vector<int> dates;
};

template <>
struct ferrugo::ansi::formatter<Person> : ferrugo::ansi::struct_formatter<Person>
{
    formatter() : struct_formatter<Person>{ "Person", { { "name", &Person::name }, { "dates", &Person::dates } } }
    {
    }
};

int main()
{
    using namespace ferrugo::ansi;

    static const auto ordered_list = [](context_t& ctx, const list_state_t& list_state)
    {
        ctx.write_text(mb_string("\n"));
        ctx.write_text(mb_string(std::string(2 * list_state.size(), ' ')));

        for (std::size_t level : list_state)
        {
            ctx.write_text(mb_string(std::to_string(level + 1)));
            ctx.write_text(mb_string("."));
        }
        ctx.write_text(mb_string(" "));
    };

    static const auto unordered_list = [](mb_string bullet)
    {
        return [=](context_t& ctx, const list_state_t& list_state)
        {
            ctx.write_text(mb_string("\n"));
            ctx.write_text(mb_string(std::string(2 * list_state.size(), ' ')));
            ctx.write_text(bullet);
            ctx.write_text(mb_string(" "));
        };
    };

    static const list_item_formatter_factory_t get_list_item_formatter = [](std::size_t level) -> list_item_formatter_t
    {
        switch (level)
        {
            case 0:
            case 1: return ordered_list;
            case 2: return unordered_list(mb_string("🔶"));
            default: return unordered_list(mb_string("◻️"));
        }
    };

    auto ctx = default_context_t{ std::cout, get_list_item_formatter };

    const auto persons = std::vector{ Person{ "Fryderyk", { 1810, 1849 } }, Person{ "Juliusz", { 1809, 1849 } } };
    ctx << text("{}\n", persons);
    ctx << text("{}\n", std::tuple{ 2.3, 123, "ABC" });

    ctx << block(
        fg["00FFFF"]("Europe"),  //
        list(                    //
            "Germany",
            "France",
            block(  //
                fg["00FF00"]("Poland"),
                list(  //
                    "Warsaw",
                    block(  //
                        "Kraków",
                        list(  //
                            "Stare Miasto",
                            "Zwierzyniec",
                            "Kleparz",
                            "Grzegórzki",
                            "Podgórze",
                            "Nowa Huta")),
                    block(  //
                        fg["FF0000"]("Wrocław"),
                        list(  //
                            "Stare Miasto",
                            "Śródmieście",
                            "Fabryczna",
                            "Krzyki",
                            "Psie Pole"))))))
        << text("\nAla ma {} kota oraz {} psa.", 1.5, "kosmatego");
    std::cout << "\n";
}
