#include <iostream>
#include <stdexcept>
#include <vector>
#include <math.h>


class Cell;
class Coord;

class BitMatrix {
   private:
    size_t rows, cols, siteWidth, siteHeight;
    double baseX, baseY;
    std::vector<char> data;

    inline void get_index_and_bit(size_t c, size_t r, size_t& index, size_t& bit) const {
        size_t pos = r * cols + c;
        index = pos / 8;
        bit = pos % 8;
    }

   public:
    BitMatrix() = default;
    BitMatrix(size_t rows, size_t cols, size_t siteWidth, size_t siteHeight, double baseX, double baseY);

    void set(size_t c, size_t r);
    void clear(size_t c, size_t r);
    bool get(size_t c, size_t r) const;

    inline bool isInteger(double d) { return floor(d) == d; }
    bool ableToPlace(Cell* cell, const Coord& loc);
    bool place(Cell* cell, const Coord& loc);
};