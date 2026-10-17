#include "itchbook/itchbook.hpp"

#include <exception>
#include <fstream>
#include <iostream>
#include <string>

namespace {
std::string show(const itchbook::Quote& quote) {
    return quote.price ? itchbook::format_price(*quote.price) + "@" +
                             std::to_string(quote.shares)
                       : "-";
}
} // namespace

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: itchbook <length-prefixed-itch-file>\n";
        return 2;
    }
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        std::cerr << "cannot open input file\n";
        return 2;
    }
    try {
        itchbook::Book book;
        const auto count = itchbook::replay(input, book);
        std::cout << "messages=" << count << " active_orders=" << book.order_count() << '\n';
        for (const auto& symbol : book.symbols()) {
            const auto bbo = book.bbo(symbol);
            std::cout << symbol << " bid=" << show(bbo.bid) << " ask=" << show(bbo.ask)
                      << '\n';
        }
    } catch (const std::exception& error) {
        std::cerr << "replay failed: " << error.what() << '\n';
        return 1;
    }
}
