// Copyright 2019 Pokitec
// All rights reserved.

#include "Widget.hpp"
#include "../GUI/utils.hpp"

namespace tt {

widget::widget(Window &_window, std::shared_ptr<widget> _parent) noexcept :
    enabled(true),
    window(_window),
    parent(_parent),
    _draw_layer(0.0f),
    _logical_layer(0),
    _semantic_layer(0)
{
    if (_parent) {
        ttlet lock = std::scoped_lock(GUISystem_mutex);
        _draw_layer = _parent->draw_layer() + 1.0f;
        _logical_layer = _parent->logical_layer() + 1;
        _semantic_layer = _parent->semantic_layer() + 1;
    }

    _enabled_callback = enabled.subscribe([this](auto...) {
        window.requestRedraw = true;
    });

    _preferred_size = {
        vec{0.0f, 0.0f},
        vec{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}
    };
}

widget::~widget()
{
}

GUIDevice *widget::device() const noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    auto device = window.device();
    tt_assert(device);
    return device;
}

bool widget::update_constraints() noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());
    return std::exchange(_request_reconstrain, false);
}

bool widget::update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    need_layout |= std::exchange(_request_relayout, false);
    if (need_layout) {
        // Used by draw().
        _to_window_transform = mat::T(_window_rectangle.x(), _window_rectangle.y(), _draw_layer);

        // Used by handle_mouse_event()
        _from_window_transform = ~_to_window_transform;
    }

    return need_layout;
}

DrawContext widget::make_draw_context(DrawContext context) const noexcept
{
    tt_assume(GUISystem_mutex.recurse_lock_count());

    context.clippingRectangle = _window_clipping_rectangle;
    context.transform = _to_window_transform;

    // The default fill and border colors.
    context.color = theme->borderColor(_semantic_layer);
    context.fillColor = theme->fillColor(_semantic_layer);

    if (*enabled) {
        if (_focus && window.active) {
            context.color = theme->accentColor;
        } else if (_hover) {
            context.color = theme->borderColor(_semantic_layer + 1);
        }

        if (_hover) {
            context.fillColor = theme->fillColor(_semantic_layer + 1);
        }

    } else {
        // Disabled, only the outline is shown.
        context.color = theme->borderColor(_semantic_layer - 1);
        context.fillColor = theme->fillColor(_semantic_layer - 1);
    }

    return context;
}

bool widget::handle_command(command command) noexcept
{
    return false;
}

bool widget::handle_mouse_event(MouseEvent const &event) noexcept {
    ttlet lock = std::scoped_lock(GUISystem_mutex);
    auto handled = false;

    if (event.type == MouseEvent::Type::Entered) {
        handled = true;
        _hover = true;
        window.requestRedraw = true;

    } else if (event.type == MouseEvent::Type::Exited) {
        handled = true;
        _hover = false;
        window.requestRedraw = true;
    }
    return handled;
}

bool widget::handle_keyboard_event(KeyboardEvent const &event) noexcept {
    ttlet lock = std::scoped_lock(GUISystem_mutex);
    auto handled = false;

    switch (event.type) {
    case KeyboardEvent::Type::Entered:
        handled = true;
        _focus = true;
        window.requestRedraw = true;
        break;

    case KeyboardEvent::Type::Exited:
        handled = true;
        _focus = false;
        window.requestRedraw = true;
        break;

    default:;
    }

    return handled;
}

std::shared_ptr<widget>
widget::next_keyboard_widget(std::shared_ptr<widget> const &current_keyboard_widget, bool reverse) const noexcept
{
    ttlet lock = std::scoped_lock(GUISystem_mutex);

    if (!current_keyboard_widget && accepts_focus()) {
        // If the current_keyboard_widget is empty or expired, then return the first widget
        // that accepts focus.
        return std::const_pointer_cast<widget>(shared_from_this());

    } else {
        return {};
    }
}

}