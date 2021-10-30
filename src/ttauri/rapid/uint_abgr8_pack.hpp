// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/corner_shapes.hpp"
#include <algorithm>

namespace tt {

class uint_abgr8_pack {
    uint32_t v;

public:
    constexpr uint_abgr8_pack() = default;
    constexpr uint_abgr8_pack(uint_abgr8_pack const &rhs) noexcept = default;
    constexpr uint_abgr8_pack(uint_abgr8_pack &&rhs) noexcept = default;
    constexpr uint_abgr8_pack &operator=(uint_abgr8_pack const &rhs) noexcept = default;
    constexpr uint_abgr8_pack &operator=(uint_abgr8_pack &&rhs) noexcept = default;

    constexpr uint_abgr8_pack(uint32_t const &rhs) noexcept : v(rhs) {}
    constexpr uint_abgr8_pack &operator=(uint32_t const &rhs) noexcept { v = rhs; return *this; }
    constexpr operator uint32_t () noexcept { return v; }

    constexpr uint_abgr8_pack(f32x4 const &rhs) noexcept :
        v((static_cast<uint32_t>(rhs.w()) << 24) |
            (static_cast<uint32_t>(rhs.z()) << 16) |
            (static_cast<uint32_t>(rhs.y()) << 8) |
            static_cast<uint32_t>(rhs.x())) {}

    constexpr uint_abgr8_pack &operator=(f32x4 const &rhs) noexcept {
        v = (static_cast<uint32_t>(rhs.w()) << 24) |
            (static_cast<uint32_t>(rhs.z()) << 16) |
            (static_cast<uint32_t>(rhs.y()) << 8) |
            static_cast<uint32_t>(rhs.x());
        return *this;
    }

    constexpr uint_abgr8_pack(corner_shapes const &rhs) noexcept : uint_abgr8_pack(static_cast<f32x4>(rhs)) {}

    [[nodiscard]] constexpr friend bool operator==(uint_abgr8_pack const &lhs, uint_abgr8_pack const &rhs) noexcept = default;
};



}