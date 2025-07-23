#ifndef LEF_PIN_HPP
#define LEF_PIN_HPP

#include <string>
#include <vector>
#include "../Port/Port.hpp"

class LEFPin {
private:
    std::string name;
    std::string direction;
    std::string use;
    std::vector<Port> ports;

public:
    // Constructor
    LEFPin() {}
    LEFPin(const std::string& name, const std::string& direction, const std::string& use)
        : name(name), direction(direction), use(use) {}

    // Getters
    const std::string& getName() const { return name; }
    const std::string& getDirection() const { return direction; }
    const std::string& getUse() const { return use; }
    const std::vector<Port>& getPorts() const { return ports; }
    std::vector<Port>& getPorts() { return ports; }
    size_t getPortCount() const { return ports.size(); }

    // Setters
    void setName(const std::string& name) { this->name = name; }
    void setDirection(const std::string& direction) { this->direction = direction; }
    void setUse(const std::string& use) { this->use = use; }
    void setPorts(const std::vector<Port>& ports) { this->ports = ports; }

    // Port 管理方法
    void addPort(const Port& port) { ports.push_back(port); }
    void addPort(const std::string& layer) {
        ports.emplace_back(layer);
    }
    
    void clearPorts() { ports.clear(); }
    
    Port* getPort(size_t index) {
        return (index < ports.size()) ? &ports[index] : nullptr;
    }
    
    const Port* getPort(size_t index) const {
        return (index < ports.size()) ? &ports[index] : nullptr;
    }
    
    // 查找特定層的 Port
    Port* findPortByLayer(const std::string& layer) {
        for (auto& port : ports) {
            if (port.getLayer() == layer) {
                return &port;
            }
        }
        return nullptr;
    }
    
    const Port* findPortByLayer(const std::string& layer) const {
        for (const auto& port : ports) {
            if (port.getLayer() == layer) {
                return &port;
            }
        }
        return nullptr;
    }
    
    // 實用方法
    bool isInput() const { return direction == "INPUT"; }
    bool isOutput() const { return direction == "OUTPUT"; }
    bool isInOut() const { return direction == "INOUT"; }
    bool isEmpty() const { return ports.empty(); }
    
    // 計算所有 Port 的總邊界框
    Rect getBoundingBox() const {
        if (ports.empty()) return Rect();
        
        bool first = true;
        double minX, minY, maxX, maxY;
        
        for (const auto& port : ports) {
            Rect portBounds = port.getBoundingBox();
            if (first) {
                minX = portBounds.getX1(); maxX = portBounds.getX2();
                minY = portBounds.getY1(); maxY = portBounds.getY2();
                first = false;
            } else {
                minX = std::min(minX, portBounds.getX1());
                maxX = std::max(maxX, portBounds.getX2());
                minY = std::min(minY, portBounds.getY1());
                maxY = std::max(maxY, portBounds.getY2());
            }
        }
        
        return first ? Rect() : Rect(minX, minY, maxX, maxY);
    }
    
    // 統計總幾何形狀數量
    size_t getTotalShapeCount() const {
        size_t count = 0;
        for (const auto& port : ports) {
            count += port.getTotalShapeCount();
        }
        return count;
    }
};

#endif // LEF_PIN_HPP