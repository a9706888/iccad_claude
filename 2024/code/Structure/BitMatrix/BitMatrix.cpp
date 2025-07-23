#include "BitMatrix.hpp"

#include "../Coord/Coord.hpp"
#include "../Cell/Cell.hpp"

BitMatrix::BitMatrix(size_t rows, size_t cols, size_t siteWidth, size_t siteHeight, double baseX, double baseY)
    : rows(rows), cols(cols), siteWidth(siteWidth), siteHeight(siteHeight), baseX(baseX), baseY(baseY), data((rows * cols + 7) / 8, 0) {}

void BitMatrix::set(size_t c, size_t r) {
    if (c >= cols || r >= rows) {
        std::cout << "rows: " << rows << " cols: " << cols << std::endl;
        std::cout << "c: " << c << " r: " << r << std::endl;
        throw std::out_of_range("Index out of range");
    }
    size_t index, bit;
    get_index_and_bit(c, r, index, bit);
    data[index] |= (char)(1 << bit);
}

void BitMatrix::clear(size_t c, size_t r) {
    if (c >= cols || r >= rows) {
        std::cout << "rows: " << rows << " cols: " << cols << std::endl;
        std::cout << "c: " << c << " r: " << r << std::endl;
        throw std::out_of_range("Index out of range");
    }
    size_t index, bit;
    get_index_and_bit(c, r, index, bit);
    data[index] &= (char)~(1 << bit);
}

bool BitMatrix::get(size_t c, size_t r) const {
    if (c >= cols || r >= rows) {
        std::cout << "rows: " << rows << " cols: " << cols << std::endl;
        std::cout << "c: " << c << " r: " << r << std::endl;
        throw std::out_of_range("Index out of range");
    }
    size_t index, bit;
    get_index_and_bit(c, r, index, bit);
    return (data[index] & (1 << bit));
}

bool BitMatrix::ableToPlace(Cell* cell, const Coord& loc) {
    if (!isInteger(loc.getX()) or !isInteger(loc.getY())){
        return false;
    }
    size_t x = (size_t)(loc.getX() - baseX) / siteWidth, y = (size_t)(loc.getY() - baseY) / siteHeight;
    size_t w = (size_t)cell->getWidth() / siteWidth, h = (size_t)cell->getHeight() / siteHeight;
    for (size_t c = x; c < x + w; c++) {
        for (size_t r = y; r < y + h; r++) {
            if (get(c, r)) {
                return false;
            }
        }
    }
    return true;
}

bool BitMatrix::place(Cell* cell, const Coord& loc) {
    size_t x = (size_t)(loc.getX() - baseX) / siteWidth, y = (size_t)(loc.getY() - baseY) / siteHeight;
    size_t w = (size_t)cell->getWidth() / siteWidth, h = (size_t)cell->getHeight() / siteHeight;
    for (size_t c = x; c < x + w; c++) {
        for (size_t r = y; r < y + h; r++) {
            set(c, r);
        }
    }
    return true;
}