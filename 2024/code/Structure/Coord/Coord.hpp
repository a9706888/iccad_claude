#pragma once
#include <cmath>
#include <iostream>
#include <unordered_set>

//座標容器
class Coord {
   public:
    // Custom hash function for Coord
    struct CoordHash {
        std::size_t operator()(const Coord& coord) const {
            return std::hash<int>()(coord.x) ^ (std::hash<int>()(coord.y) << 1);
        }
    };

    // Custom equality function for Coord
    struct CoordEqual {
        bool operator()(const Coord& lhs, const Coord& rhs) const {
            return lhs.x == rhs.x && lhs.y == rhs.y;
        }
    };
    using CoordSet = std::unordered_set<Coord, Coord::CoordHash, Coord::CoordEqual>;

    inline Coord() : x(0), y(0) {}
    inline Coord(double x, double y) : x(x), y(y) {}
    Coord(const Coord& other) = default;
    inline double getX() const { return x; }
    inline double getY() const { return y; }
    inline void setX(double _x) { this->x = _x; }
    inline void setY(double _y) { this->y = _y; }
    inline void setMaxCoord(Coord other) {
        x = std::max(x, other.getX());
        y = std::max(y, other.getY());
    }
    inline void setMinCoord(Coord other) {
        x = std::min(x, other.getX());
        y = std::min(y, other.getY());
    }

    // Distance functions
    inline double Edist(const Coord& other) const {
        return std::sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
    }

    // Manhattan distance(只看水平和垂直距離)
    inline double Mdist(const Coord& other) const {
        return std::abs(x - other.x) + std::abs(y - other.y);
    }

    // midpoint
    inline Coord medianTo(const Coord& other) const {
        return Coord((x + other.x) / 2, (y + other.y) / 2);
    }

    //檢查是否被包含
    inline bool isDominatedBy(const Coord& other) const {
        return (x <= other.x) and (y <= other.y);
    }

    //+
    inline Coord operator+(const Coord& other) const {
        return Coord(x + other.x, y + other.y);
    }

    //-
    inline Coord operator-(const Coord& other) const {
        return Coord(x - other.x, y - other.y);
    }

    // ==
    inline bool operator==(const Coord& other) const {
        return (x == other.x) and (y == other.y);
    }

    // !=
    inline bool operator!=(const Coord& other) const {
        return (x != other.x) or (y != other.y);
    }

    // <
    inline bool operator<(const Coord& other) const {
        if (x < other.x) return true;
        if (x == other.x) return y < other.y;
        return false;
    }

    // >
    inline bool operator>(const Coord& other) const {
        if (x > other.x) return true;
        if (x == other.x) return y > other.y;
        return false;
    }

    //=
    inline Coord& operator=(const Coord& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
        }
        return *this;
    }
    friend std::ostream& operator<<(std::ostream& os, const Coord& p) {
        os << (long long)p.x << " " << (long long)p.y;
        return os;
    }

   private:
    double x, y;
};
