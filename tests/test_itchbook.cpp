#include "itchbook/itchbook.hpp"

#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void put(std::vector<std::uint8_t>& bytes, std::size_t at, std::size_t width,
         std::uint64_t value) {
    for (std::size_t i = 0; i < width; ++i) {
        bytes.at(at + width - 1 - i) = static_cast<std::uint8_t>(value & 0xffU);
        value >>= 8U;
    }
}

std::vector<std::uint8_t> add(std::uint16_t locate, std::uint64_t id,
                              const std::string& symbol, char side,
                              std::uint32_t shares, std::uint32_t price) {
    std::vector<std::uint8_t> message(36, 0);
    message[0] = 'A';
    put(message, 1, 2, locate);
    put(message, 11, 8, id);
    message[19] = static_cast<std::uint8_t>(side);
    put(message, 20, 4, shares);
    for (std::size_t i = 0; i < 8; ++i) {
        message[24 + i] = static_cast<std::uint8_t>(i < symbol.size() ? symbol[i] : ' ');
    }
    put(message, 32, 4, price);
    return message;
}

std::vector<std::uint8_t> remove(std::uint16_t locate, std::uint64_t id) {
    std::vector<std::uint8_t> message(19, 0);
    message[0] = 'D';
    put(message, 1, 2, locate);
    put(message, 11, 8, id);
    return message;
}

void expect(bool value, const std::string& description) {
    if (!value) {
        throw std::runtime_error(description);
    }
}

template <typename Function>
void expect_failure(Function action, const std::string& description) {
    try {
        action();
    } catch (const std::exception&) {
        return;
    }
    throw std::runtime_error(description);
}

void write_record(std::stringstream& stream, const std::vector<std::uint8_t>& bytes) {
    stream.put(static_cast<char>(bytes.size() >> 8U));
    stream.put(static_cast<char>(bytes.size() & 0xffU));
    stream.write(reinterpret_cast<const char*>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));
}

void lifecycle() {
    itchbook::Book book;
    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
    write_record(stream, add(1, 10, "AAA", 'B', 100, 1000000));
    write_record(stream, add(1, 11, "AAA", 'B', 50, 1000000));
    write_record(stream, add(1, 12, "AAA", 'S', 40, 1010000));
    write_record(stream, add(2, 13, "BBB", 'S', 30, 2000000));
    write_record(stream, remove(1, 10));
    stream.seekg(0);
    expect(itchbook::replay(stream, book) == 5, "record count");
    expect(book.order_count() == 3, "active order count");
    const auto a = book.bbo("AAA");
    const auto b = book.bbo("BBB");
    expect(a.bid.price == 1000000 && a.bid.shares == 50, "aggregated bid after delete");
    expect(a.ask.price == 1010000 && a.ask.shares == 40, "AAA ask");
    expect(!b.bid.price && b.ask.price == 2000000 && b.ask.shares == 30,
           "symbol isolation");
    expect(itchbook::format_price(*a.bid.price) == "100.0000", "fixed point price");
    book.apply(remove(1, 11));
    expect(!book.bbo("AAA").bid.price, "empty side after final delete");
}

void rejection() {
    itchbook::Book book;
    book.apply(add(1, 1, "AAA", 'B', 10, 10000));
    expect_failure([&] { book.apply(add(1, 1, "AAA", 'B', 10, 10000)); },
                   "duplicate order accepted");
    expect_failure([&] { book.apply(add(1, 2, "BBB", 'B', 10, 10000)); },
                   "locate reused for another symbol");
    expect_failure([&] { book.apply(remove(2, 1)); }, "wrong locate deleted order");
    expect_failure([&] { book.apply(remove(1, 9)); }, "unknown reference accepted");
    expect_failure([&] { book.apply(add(2, 2, "BBB", '?', 10, 10000)); },
                   "invalid side accepted");
    expect_failure([&] { book.apply(std::vector<std::uint8_t>{'E'}); },
                   "unsupported event accepted");
    expect(book.order_count() == 1 && book.bbo("AAA").bid.shares == 10,
           "failed message changed state");

    std::stringstream short_prefix(std::ios::in | std::ios::out | std::ios::binary);
    short_prefix.put(0);
    short_prefix.seekg(0);
    expect_failure([&] { itchbook::replay(short_prefix, book); }, "short prefix accepted");

    std::stringstream short_body(std::ios::in | std::ios::out | std::ios::binary);
    short_body.put(0);
    short_body.put(36);
    short_body.put('A');
    short_body.seekg(0);
    expect_failure([&] { itchbook::replay(short_body, book); }, "short body accepted");
}

} // namespace

int main() {
    try {
        lifecycle();
        rejection();
    } catch (const std::exception& error) {
        std::cerr << "test failure: " << error.what() << '\n';
        return 1;
    }
}
