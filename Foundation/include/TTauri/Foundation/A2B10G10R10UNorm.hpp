// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/geometry.hpp"
#include <algorithm>

namespace TTauri {

[[nodiscard]] constexpr uint32_t make_A2B10G10R10UNorm_value(glm::vec4 const &rhs) noexcept
{
    let r = static_cast<uint32_t>(std::clamp(rhs.r, 0.0f, 1.0f) * 1023.0f);
    let g = static_cast<uint32_t>(std::clamp(rhs.g, 0.0f, 1.0f) * 1023.0f);
    let b = static_cast<uint32_t>(std::clamp(rhs.b, 0.0f, 1.0f) * 1023.0f);
    let a = static_cast<uint32_t>(std::clamp(rhs.a, 0.0f, 1.0f) * 3.0f);
    return (a << 30) | (b << 20) | (g << 10) | r;
}

[[nodiscard]] constexpr uint32_t make_A2B10G10R10UNorm_value(glm::vec3 const &rhs) noexcept
{
    let r = static_cast<uint32_t>(std::clamp(rhs.r, 0.0f, 1.0f) * 1023.0f);
    let g = static_cast<uint32_t>(std::clamp(rhs.g, 0.0f, 1.0f) * 1023.0f);
    let b = static_cast<uint32_t>(std::clamp(rhs.b, 0.0f, 1.0f) * 1023.0f);
    return (3 << 30) | (b << 20) | (g << 10) | r;
}


struct A2B10G10R10UNorm {
    uint32_t value;

    A2B10G10R10UNorm() = default;
    constexpr A2B10G10R10UNorm(A2B10G10R10UNorm const &rhs) noexcept = default;
    constexpr A2B10G10R10UNorm(A2B10G10R10UNorm &&rhs) noexcept = default;
    constexpr A2B10G10R10UNorm &operator=(A2B10G10R10UNorm const &rhs) noexcept = default;
    constexpr A2B10G10R10UNorm &operator=(A2B10G10R10UNorm &&rhs) noexcept = default;
    ~A2B10G10R10UNorm() = default;

    constexpr explicit A2B10G10R10UNorm(glm::vec4 const &rhs) noexcept :
        value(make_A2B10G10R10UNorm_value(rhs)) {}

    constexpr explicit A2B10G10R10UNorm(glm::vec3 const &rhs) noexcept :
        value(make_A2B10G10R10UNorm_value(rhs)) {}

    constexpr explicit A2B10G10R10UNorm(float r, float g, float b, float a) noexcept :
        A2B10G10R10UNorm(glm::vec4{r, g, b, a}) {}

    constexpr explicit A2B10G10R10UNorm(float r, float g, float b) noexcept :
        A2B10G10R10UNorm(glm::vec3{r, g, b}) {}

    constexpr A2B10G10R10UNorm &operator=(glm::vec4 const &rhs) noexcept {
        value = make_A2B10G10R10UNorm_value(rhs);
        return *this;
    }

    constexpr A2B10G10R10UNorm &operator=(glm::vec3 const &rhs) noexcept {
        value = make_A2B10G10R10UNorm_value(rhs);
        return *this;
    }

    constexpr explicit operator glm::vec4 () const noexcept {
        return glm::vec4{
            static_cast<float>((value >> 20) & 0x3ff) / 1023.0f,
            static_cast<float>((value >> 10) & 0x3ff) / 1023.0f,
            static_cast<float>(value & 0x3ff) / 1023.0f,
            static_cast<float>(value >> 30) / 3.0f
        };
    }

    constexpr explicit operator glm::vec3 () const noexcept {
        return glm::vec3{
            static_cast<float>((value >> 20) & 0x3ff) / 1023.0f,
            static_cast<float>((value >> 10) & 0x3ff) / 1023.0f,
            static_cast<float>(value & 0x3ff) / 1023.0f
        };
    }
};

}