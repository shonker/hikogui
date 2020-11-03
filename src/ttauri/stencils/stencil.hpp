// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../GUI/Window_forward.hpp"
#include "../aarect.hpp"

namespace tt {
class DrawContext;

class stencil {
public:
    stencil(Alignment alignment) : _alignment(alignment), _data_is_modified(true), _layout_is_modified(true) {}
    virtual ~stencil() = default;
    stencil(stencil const &) noexcept = delete;
    stencil(stencil &&) noexcept = delete;
    stencil &operator=(stencil const &) noexcept = delete;
    stencil &operator=(stencil &&) noexcept = delete;

    /** Return the extent that this cell wants to be drawn as.
     */
    [[nodiscard]] virtual vec preferred_extent() noexcept
    {
        return {};
    }

    /** Pass layout parameters in local coordinates.
     * @param rectangle The rectangle the stencil will be drawn into.
     * @param base_line_position The position of the base line within the rectangle.
     */
    void
    set_layout_parameters(aarect const &rectangle, float base_line_position = std::numeric_limits<float>::infinity()) noexcept
    {
        _rectangle = rectangle;
        _base_line_position =
            base_line_position != std::numeric_limits<float>::infinity() ? base_line_position : rectangle.middle();
        _layout_is_modified = true;
    }

    /** Draw the cell.
     *
     * @param context The current draw context.
     * @param use_context_color True to use the colors in the context, False to use the colors in the cell itself.
     */
    virtual void draw(DrawContext context, bool use_context_color = false) noexcept = 0;

protected:
    Alignment _alignment;
    aarect _rectangle;
    float _base_line_position;

    /** Set to true when the data of the cell has been modified.
     */
    bool _data_is_modified;
    bool _layout_is_modified;
};

} // namespace tt