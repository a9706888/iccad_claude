#ifndef PORT_HPP
#define PORT_HPP

#include <string>
#include <vector>
#include "../Geometry/Geometry.hpp"

class Port {
private:
    std::string layer;
    std::vector<Rect> rects;
    std::vector<Polygon> polygons;

public:
    // Constructor
    Port() {}
    Port(const std::string& layer) : layer(layer) {}

    // Getters
    const std::string& getLayer() const { return layer; }
    const std::vector<Rect>& getRects() const { return rects; }
    const std::vector<Polygon>& getPolygons() const { return polygons; }
    size_t getRectCount() const { return rects.size(); }
    size_t getPolygonCount() const { return polygons.size(); }

    // Setters
    void setLayer(const std::string& layer) { this->layer = layer; }
    void setRects(const std::vector<Rect>& rects) { this->rects = rects; }
    void setPolygons(const std::vector<Polygon>& polygons) { this->polygons = polygons; }

    // 幾何形狀管理方法
    void addRect(const Rect& rect) { rects.push_back(rect); }
    void addRect(double x1, double y1, double x2, double y2) {
        rects.emplace_back(x1, y1, x2, y2);
    }
    
    void addPolygon(const Polygon& polygon) { polygons.push_back(polygon); }
    void addPolygon(const std::vector<double>& points) {
        polygons.emplace_back(points);
    }
    
    void clearRects() { rects.clear(); }
    void clearPolygons() { polygons.clear(); }
    void clearAll() { clearRects(); clearPolygons(); }
    
    // 實用方法
    bool isEmpty() const { return rects.empty() && polygons.empty(); }
    size_t getTotalShapeCount() const { return rects.size() + polygons.size(); }
    
    // 計算總邊界框
    Rect getBoundingBox() const {
        if (isEmpty()) return Rect();
        
        bool first = true;
        double minX, minY, maxX, maxY;
        
        // 處理矩形
        for (const auto& rect : rects) {
            if (first) {
                minX = rect.getX1(); maxX = rect.getX2();
                minY = rect.getY1(); maxY = rect.getY2();
                first = false;
            } else {
                minX = std::min(minX, rect.getX1());
                maxX = std::max(maxX, rect.getX2());
                minY = std::min(minY, rect.getY1());
                maxY = std::max(maxY, rect.getY2());
            }
        }
        
        // 處理多邊形
        for (const auto& polygon : polygons) {
            Rect polyBounds = polygon.getBoundingBox();
            if (first) {
                minX = polyBounds.getX1(); maxX = polyBounds.getX2();
                minY = polyBounds.getY1(); maxY = polyBounds.getY2();
                first = false;
            } else {
                minX = std::min(minX, polyBounds.getX1());
                maxX = std::max(maxX, polyBounds.getX2());
                minY = std::min(minY, polyBounds.getY1());
                maxY = std::max(maxY, polyBounds.getY2());
            }
        }
        
        return first ? Rect() : Rect(minX, minY, maxX, maxY);
    }
};

#endif // PORT_HPP