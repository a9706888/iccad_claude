#include "Pin.hpp"

#include "../Instance/Instance.hpp"
/* For pin in cell library */
Pin::Pin(const std::string& name, size_t pinID, Coord loc, Type type)
    : name(name),
      inst(nullptr),
      pinID(pinID),
      loc(loc),
      type(type),
      slack(0),
      net(nullptr) {}

/* For Gate instances pin */
Pin::Pin(const std::string& name, Instance* inst, Coord loc, Type type)
    : name(name),
      inst(inst),
      loc(loc),
      type(type),
      slack(0),
      net(nullptr) {}

/* For FF instances pin */
Pin::Pin(const std::string& name, size_t tag, Instance* inst, Coord loc, Type type)
    : name(name),
      old_tag(tag),
      inst(inst),
      loc(loc),
      type(type),
      slack(0),
      net(nullptr) {}

/* For duplicate clk pin */
Pin::Pin(const std::string& name, size_t tag, Instance* inst, Coord loc, Type type, Net* net)
    : name(name),
      old_tag(tag),
      inst(inst),
      loc(loc),
      type(type),
      slack(0),
      net(net) {}
