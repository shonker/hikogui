// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/geometry.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/rect.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include "TTauri/Foundation/R32G32B32A32SFloat.hpp"
#include "TTauri/Foundation/R32G32B32SFloat.hpp"
#include <vulkan/vulkan.hpp>

namespace TTauri::GUI::PipelineSDF {

/*! A vertex defining a rectangle on a window.
* The vertex shader will convert window pixel-coordinates to normalized projection-coordinates.
*/
struct Vertex {
    //! The pixel-coordinates where the origin is located relative to the bottom-left corner of the window.
    R32G32B32SFloat position;

    //! Clipping rectangle. (x,y)=bottom-left, (z,w)=top-right
    R32G32B32A32SFloat clippingRectangle;

    //! The x, y (relative to bottom-left) coordinate inside the texture-atlas, z is used as an index in the texture-atlas array
    R32G32B32SFloat textureCoord;

    //! The color of the glyph.
    R16G16B16A16SFloat color;

    //! The multiplier to use to convert a SDF distance from texture space to screen-space.
    float distanceMultiplier;

    //! The number of pixels that the shadow is, negative values make inset shadow. Should be less than or equal to the SDF8::max_distance.
    float shadowSize;

    Vertex(vec position, rect clippingRectangle, vec textureCoord, vec color, float distanceMultiplier, float shadowSize) noexcept :
        position(position),
        clippingRectangle(clippingRectangle),
        textureCoord(textureCoord),
        color(color),
        distanceMultiplier(distanceMultiplier),
        shadowSize(shadowSize) {}

    static vk::VertexInputBindingDescription inputBindingDescription()
    {
        return {
            0, sizeof(Vertex), vk::VertexInputRate::eVertex
        };
    }

    static std::vector<vk::VertexInputAttributeDescription> inputAttributeDescriptions()
    {
        return {
            { 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position) },
            { 1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, clippingRectangle) },
            { 2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, textureCoord) },                
            { 3, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(Vertex, color) },
            { 4, 0, vk::Format::eR32Sfloat, offsetof(Vertex, distanceMultiplier) },
            { 5, 0, vk::Format::eR32Sfloat, offsetof(Vertex, shadowSize) }
        };
    }
};
}