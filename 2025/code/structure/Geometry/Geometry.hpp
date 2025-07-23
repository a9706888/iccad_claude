#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <vector>
#include <string>

// 矩形類別
class Rect {
private:
    double x1, y1, x2, y2;

public:
    // Constructor
    Rect() : x1(0), y1(0), x2(0), y2(0) {}
    Rect(double x1, double y1, double x2, double y2) 
        : x1(x1), y1(y1), x2(x2), y2(y2) {}

    // Getters
    double getX1() const { return x1; }
    double getY1() const { return y1; }
    double getX2() const { return x2; }
    double getY2() const { return y2; }

    // Setters
    void setX1(double x1) { this->x1 = x1; }
    void setY1(double y1) { this->y1 = y1; }
    void setX2(double x2) { this->x2 = x2; }
    void setY2(double y2) { this->y2 = y2; }
    void setBounds(double x1, double y1, double x2, double y2) {
        this->x1 = x1; this->y1 = y1; this->x2 = x2; this->y2 = y2;
    }

    // 實用方法
    double getWidth() const { return x2 - x1; }
    double getHeight() const { return y2 - y1; }
    double getArea() const { return getWidth() * getHeight(); }
    double getCenterX() const { return (x1 + x2) / 2.0; }
    double getCenterY() const { return (y1 + y2) / 2.0; }
    
    bool isValid() const { return x2 > x1 && y2 > y1; }
    bool contains(double x, double y) const {
        return x >= x1 && x <= x2 && y >= y1 && y <= y2;
    }
};

// 多邊形類別
class Polygon {
private:
    std::vector<double> pts; // x1,y1,x2,y2,...

public:
    // Constructor
    Polygon() {}
    Polygon(const std::vector<double>& points) : pts(points) {}

    // Getters
    const std::vector<double>& getPoints() const { return pts; }
    size_t getPointCount() const { return pts.size() / 2; }

    // Setters
    void setPoints(const std::vector<double>& points) { pts = points; }
    void clearPoints() { pts.clear(); }

    // 點管理方法
    void addPoint(double x, double y) {
        pts.push_back(x);
        pts.push_back(y);
    }
    
    void removeLastPoint() {
        if (pts.size() >= 2) {
            pts.pop_back();
            pts.pop_back();
        }
    }
    
    std::pair<double, double> getPoint(size_t index) const {
        if (index * 2 + 1 < pts.size()) {
            return {pts[index * 2], pts[index * 2 + 1]};
        }
        return {0.0, 0.0};
    }
    
    // 實用方法
    bool isEmpty() const { return pts.empty(); }
    bool isValid() const { return pts.size() >= 6 && pts.size() % 2 == 0; } // 至少3個點
    
    // 計算邊界框
    Rect getBoundingBox() const {
        if (pts.empty()) return Rect();
        
        double minX = pts[0], maxX = pts[0];
        double minY = pts[1], maxY = pts[1];
        
        for (size_t i = 2; i < pts.size(); i += 2) {
            minX = std::min(minX, pts[i]);
            maxX = std::max(maxX, pts[i]);
            minY = std::min(minY, pts[i + 1]);
            maxY = std::max(maxY, pts[i + 1]);
        }
        
        return Rect(minX, minY, maxX, maxY);
    }
};

#endif // GEOMETRY_HPP