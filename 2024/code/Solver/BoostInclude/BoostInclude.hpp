#pragma once
#pragma GCC system_header

#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/math/special_functions/binomial.hpp>
#include "../../Structure/Box/Box.hpp"

#define P_PER_NODE 16

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
namespace bgm = boost::geometry::model;
namespace bm = boost::math;

namespace boost { namespace geometry { namespace traits {
    template<> struct tag<Coord> { using type = point_tag; };
    template<> struct coordinate_type<Coord> { using type = double; };
    template<> struct coordinate_system<Coord> { using type = bg::cs::cartesian; };
    template<> struct dimension<Coord> : boost::mpl::int_<2> {};
    template<> struct access<Coord, 0> { static double get(Coord const& c) { return c.getX(); } };
    template<> struct access<Coord, 1> { static double get(Coord const& c) { return c.getY(); } };

    template<> struct tag<Box> { using type = box_tag; };
    template<> struct point_type<Box> { using type = bgm::point<double, 2, bg::cs::cartesian>; };
    template<> struct indexed_access<Box, min_corner, 0> { static double get(Box const& b) { return b.min_corner.getX(); } };
    template<> struct indexed_access<Box, min_corner, 1> { static double get(Box const& b) { return b.min_corner.getY(); } };
    template<> struct indexed_access<Box, max_corner, 0> { static double get(Box const& b) { return b.max_corner.getX(); } };
    template<> struct indexed_access<Box, max_corner, 1> { static double get(Box const& b) { return b.max_corner.getY(); } };
}}}

typedef bgi::rtree<Box, bgi::quadratic<P_PER_NODE>> BoxRTree;
