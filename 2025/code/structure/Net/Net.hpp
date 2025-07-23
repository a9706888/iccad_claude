#ifndef NET_HPP
#define NET_HPP

#include <string>
#include <vector>
#include <algorithm>

class Net 
{
private:
    std::string name;
    std::vector<std::string> connections;

public:
    // Constructor
    Net() {}
    Net(const std::string& name) : name(name) {}
    Net(const std::string& name, const std::vector<std::string>& connections)
        : name(name), connections(connections) {}

    // Getters
    const std::string& getName() const { return name; }
    const std::vector<std::string>& getConnections() const { return connections; }
    size_t getConnectionCount() const { return connections.size(); }

    // Setters
    void setName(const std::string& name) { this->name = name; }
    void setConnections(const std::vector<std::string>& connections) { this->connections = connections; }
    
    // Connection management methods
    void addConnection(const std::string& connection) { connections.push_back(connection); }
    void removeConnection(const std::string& connection) {
        connections.erase(std::remove(connections.begin(), connections.end(), connection), connections.end());
    }
    void clearConnections() { connections.clear(); }
    bool hasConnection(const std::string& connection) const {
        return std::find(connections.begin(), connections.end(), connection) != connections.end();
    }
};

#endif // NET_HPP