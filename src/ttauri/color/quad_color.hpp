

#pragma once

#include "color.hpp"

namespace tt {

class quad_color {
public:
    color p0; ///< left-bottom
    color p1; ///< right-bottom
    color p2; ///< left-top
    color p3; ///< right-top

    constexpr quad_color(quad_color const &) noexcept = default;
    constexpr quad_color(quad_color &&) noexcept = default;
    constexpr quad_color &operator=(quad_color const &) noexcept = default;
    constexpr quad_color &operator=(quad_color &&) noexcept = default;
    constexpr quad_color(color const &p0, color const &p1, color const &p2, color const &p3) noexcept : p0(p0), p1(p1), p2(p2), p3(p3) {}
    constexpr quad_color(color const &c) noexcept : p0(c), p1(c), p2(c), p3(c) {}

};


}
