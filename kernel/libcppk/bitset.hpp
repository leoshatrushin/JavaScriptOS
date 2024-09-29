#pragma once
#include <cstddef>
#include "array.hpp"

namespace mystd {

template<std::size_t N>
class bitset {
private:
    static constexpr std::size_t BITS_PER_BLOCK = sizeof(unsigned long) * 8;
    static constexpr std::size_t NUM_BLOCKS = (N + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK;

    mystd::array<unsigned long, NUM_BLOCKS> blocks;

    constexpr std::size_t block_index(std::size_t pos) const { return pos / BITS_PER_BLOCK; }
    constexpr std::size_t bit_position(std::size_t pos) const { return pos % BITS_PER_BLOCK; }
public:
    bitset() { blocks.fill(0); }

    // Access operator (read/write)
    bool& operator[](std::size_t pos) {
        if (pos >= N) {
            //throw mystd::out_of_range("Bit position out of range");
        }
        return Proxy(*this, pos);
    }

    // Access operator (read-only)
    bool operator[](std::size_t pos) const {
        if (pos >= N) {
            //throw mystd::out_of_range("Bit position out of range");
        }
        std::size_t idx = block_index(pos);
        std::size_t bit = bit_position(pos);
        return (blocks[idx] >> bit) & 1ul;
    }

    class Proxy {
    public:
        Proxy(class bitset& bs, std::size_t p) : parent_bitset{bs}, pos{p} {}
        Proxy& operator=(bool value) {
            if (value) {
                parent_bitset.set(pos);
            } else {
                parent_bitset.reset(pos);
            }
            return *this;
        }
        operator bool() const { return parent_bitset[pos]; }
    private:
        bitset& parent_bitset;
        std::size_t pos;
    };

    bitset& set(std::size_t pos, bool value = true) {
        if (pos >= N) {
            //throw mystd::out_of_range("Bit position out of range");
        }
        std::size_t idx = block_index(pos);
        std::size_t bit = bit_position(pos);
        if (value) {
            blocks[idx] |= (1ul << bit);
        } else {
            blocks[idx] &= ~(1ul << bit);
        }
        return *this;
    }

    constexpr std::size_t size() const { return N; }
};

}
