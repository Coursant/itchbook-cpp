#include "itchbook/itchbook.hpp"

#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace itchbook {
namespace {

std::uint64_t big_endian(const std::vector<std::uint8_t>& bytes,
                         std::size_t offset, std::size_t length) {
    std::uint64_t value = 0;
    for (std::size_t i = 0; i < length; ++i) {
        value = (value << 8U) | bytes.at(offset + i);
    }
    return value;
}

std::string stock(const std::vector<std::uint8_t>& message) {
    std::string result;
    result.reserve(8);
    for (std::size_t i = 24; i < 32; ++i) {
        const auto c = message.at(i);
        if (c < 0x20 || c > 0x7e) {
            throw std::runtime_error("invalid stock ASCII");
        }
        result.push_back(static_cast<char>(c));
    }
    while (!result.empty() && result.back() == ' ') {
        result.pop_back();
    }
    if (result.empty()) {
        throw std::runtime_error("empty stock symbol");
    }
    return result;
}

} // namespace

void Book::apply(const std::vector<std::uint8_t>& message) {
    if (message.empty()) {
        throw std::runtime_error("empty ITCH message");
    }
    const char type = static_cast<char>(message[0]);
    if (type != 'A' && type != 'D') {
        throw std::runtime_error(std::string("unsupported ITCH type: ") + type);
    }
    const std::size_t expected = type == 'A' ? 36U : 19U;
    if (message.size() != expected) {
        throw std::runtime_error("invalid ITCH message length");
    }
    const auto locate = static_cast<std::uint16_t>(big_endian(message, 1, 2));
    const auto id = big_endian(message, 11, 8);
    if (locate == 0) {
        throw std::runtime_error("zero stock locate for order message");
    }

    if (type == 'A') {
        const char side = static_cast<char>(message[19]);
        const auto shares = static_cast<std::uint32_t>(big_endian(message, 20, 4));
        const auto symbol = stock(message);
        const auto price = static_cast<std::uint32_t>(big_endian(message, 32, 4));
        if (side != 'B' && side != 'S') {
            throw std::runtime_error("invalid order side");
        }
        if (shares == 0 || price == 0) {
            throw std::runtime_error("zero shares or price");
        }
        if (seen_refs_.count(id) != 0) {
            throw std::runtime_error("duplicate order reference");
        }
        const auto locate_it = locates_.find(locate);
        if (locate_it != locates_.end() && locate_it->second != symbol) {
            throw std::runtime_error("stock locate maps to another symbol");
        }
        auto& levels = books_[symbol];
        auto& side_levels = side == 'B' ? levels.bids : levels.asks;
        auto& total = side_levels[price];
        if (total > std::numeric_limits<std::uint64_t>::max() - shares) {
            throw std::runtime_error("price level share overflow");
        }
        total += shares;
        locates_.emplace(locate, symbol);
        orders_.emplace(id, Order{locate, symbol, side, shares, price});
        seen_refs_.insert(id);
        return;
    }

    const auto order_it = orders_.find(id);
    if (order_it == orders_.end()) {
        throw std::runtime_error("delete references unknown order");
    }
    const Order& order = order_it->second;
    if (order.locate != locate) {
        throw std::runtime_error("delete stock locate mismatch");
    }
    auto& levels = books_.at(order.symbol);
    auto& side_levels = order.side == 'B' ? levels.bids : levels.asks;
    auto level_it = side_levels.find(order.price);
    if (level_it == side_levels.end() || level_it->second < order.shares) {
        throw std::logic_error("inconsistent price level");
    }
    level_it->second -= order.shares;
    if (level_it->second == 0) {
        side_levels.erase(level_it);
    }
    orders_.erase(order_it);
}

Bbo Book::bbo(const std::string& symbol) const {
    const auto book_it = books_.find(symbol);
    if (book_it == books_.end()) {
        return {};
    }
    Bbo result;
    if (!book_it->second.bids.empty()) {
        const auto& level = *book_it->second.bids.rbegin();
        result.bid = {level.first, level.second};
    }
    if (!book_it->second.asks.empty()) {
        const auto& level = *book_it->second.asks.begin();
        result.ask = {level.first, level.second};
    }
    return result;
}

std::vector<std::string> Book::symbols() const {
    std::vector<std::string> result;
    result.reserve(books_.size());
    for (const auto& entry : books_) {
        result.push_back(entry.first);
    }
    return result;
}

std::size_t replay(std::istream& input, Book& book) {
    std::size_t count = 0;
    while (true) {
        const int high = input.get();
        if (high == std::char_traits<char>::eof()) {
            if (input.eof()) {
                return count;
            }
            throw std::runtime_error("failed to read message length");
        }
        const int low = input.get();
        if (low == std::char_traits<char>::eof()) {
            throw std::runtime_error("truncated message length");
        }
        const auto length = static_cast<std::size_t>((high << 8) | low);
        if (length == 0) {
            throw std::runtime_error("zero message length");
        }
        std::vector<std::uint8_t> message(length);
        input.read(reinterpret_cast<char*>(message.data()), static_cast<std::streamsize>(length));
        if (input.gcount() != static_cast<std::streamsize>(length)) {
            throw std::runtime_error("truncated ITCH message");
        }
        book.apply(message);
        ++count;
    }
}

std::string format_price(std::uint32_t price) {
    std::ostringstream output;
    output << price / 10000U << '.' << std::setw(4) << std::setfill('0')
           << price % 10000U;
    return output.str();
}

} // namespace itchbook
