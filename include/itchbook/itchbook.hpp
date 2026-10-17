#pragma once

#include <cstddef>
#include <cstdint>
#include <istream>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace itchbook {

struct Quote {
    std::optional<std::uint32_t> price;
    std::uint64_t shares = 0;
};

struct Bbo {
    Quote bid;
    Quote ask;
};

class Book {
public:
    void apply(const std::vector<std::uint8_t>& message);
    Bbo bbo(const std::string& symbol) const;
    std::vector<std::string> symbols() const;
    std::size_t order_count() const noexcept { return orders_.size(); }

private:
    struct Order {
        std::uint16_t locate;
        std::string symbol;
        char side;
        std::uint32_t shares;
        std::uint32_t price;
    };
    struct Levels {
        std::map<std::uint32_t, std::uint64_t> bids;
        std::map<std::uint32_t, std::uint64_t> asks;
    };
    std::unordered_map<std::uint64_t, Order> orders_;
    std::unordered_set<std::uint64_t> seen_refs_;
    std::unordered_map<std::uint16_t, std::string> locates_;
    std::map<std::string, Levels> books_;
};

// Each record has a 2-byte big-endian length followed by one ITCH payload.
// Returns the number of successfully applied messages; throws on any error.
std::size_t replay(std::istream& input, Book& book);
std::string format_price(std::uint32_t price);

} // namespace itchbook
