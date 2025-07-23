#pragma once
#include <memory>
#include <string>

#include "../../global/global.hpp"
#include "../Coord/Coord.hpp"

class Net;
class Instance;

class Pin {
   public:
    using ptr = std::unique_ptr<Pin>;
    enum Type {
        Q,      // for FF only
        OUT,    // for GATE only
        INPUT,  // for IO only
        CLK,    // for FF only
        D,      // for FF only
        IN,     // for GATE only
        OUTPUT  // for IO only
    };

    /* For pin in cell library */
    Pin(const std::string& name, size_t pinID, Coord loc, Type type);
    /* For Gate instances pin */
    Pin(const std::string& name, Instance* inst, Coord loc, Type type);
    /* For FF instances pin */
    Pin(const std::string& name, size_t tag, Instance* inst, Coord loc, Type type);
    /* For duplicate clk pin */
    Pin(const std::string& name, size_t tag, Instance* inst, Coord loc, Type type, Net* net);

    void setID(size_t id) { this->pinID = id; }
    void setNet(Net* n) { this->net = n; }
    void setLoc(Coord lo) { this->loc = lo; }
    void setDelay(double dly) { this->delay = dly; }
    void setInst(Instance* inst) { this->inst = inst; }
    void setRqrTime(double rqrT) { this->rqrTime = rqrT; }
    void setSlack(double slk) {
        double prevSlack = this->slack;
        this->slack = slk;
        if (prevSlack >= 0) {
            if (slk < 0)
                eval.TNS += slk;
        } else {
            if (slk >= 0) {
                eval.TNS -= prevSlack;
            } else {
                eval.TNS += slk - prevSlack;
            }
        }
    }

    Net* getNet() const { return net; }
    Type getType() const { return type; }
    size_t getID() const { return pinID; }
    size_t getTag() const { return old_tag; }
    double getSlack() const { return slack; }
    double getDelay() const { return delay; }
    Instance* getInst() const { return inst; }
    double getRqrTime() const { return rqrTime; }
    const Coord& getLoc() const { return loc; }
    const std::string& getName() const { return name; }

    bool isType(Type type) const { return this->type == type; }
    double Mdist(const Pin* pin) const { return loc.Mdist(pin->getLoc()); }

    Pin::ptr duplicate() {
        return std::make_unique<Pin>(name, old_tag, inst, loc, type, net);
    }

   private:
    std::string name;
    size_t old_tag;
    Instance* inst;
    size_t pinID;
    Coord loc;
    Type type;
    double slack;
    double delay;
    double rqrTime;
    Net* net;
};
