// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/FontWeight.hpp"

namespace tt {

/** A font variant is one of 16 different fonts that can be part of a family.
* It only contains the font-weight and if it is italic/oblique.
*
* monospace, serif, condensed, expanded & optical-size are all part of the font family.
*/
class FontVariant {
    uint8_t value;

public:
    constexpr static int max() { return 20; }
    constexpr static int half() { return max() / 2; }

    constexpr FontVariant(FontWeight weight, bool italic) noexcept : value(static_cast<uint8_t>(static_cast<int>(weight) + (italic ? half() : 0))) {}
    constexpr FontVariant() noexcept : FontVariant(FontWeight::Regular, false) {}
    constexpr FontVariant(FontWeight weight) noexcept : FontVariant(weight, false) {}
    constexpr FontVariant(bool italic) noexcept : FontVariant(FontWeight::Regular, italic) {}

    constexpr FontWeight weight() const noexcept {
        tt_assume(value < max());
        return static_cast<FontWeight>(value % half());
    }

    [[nodiscard]] constexpr bool italic() const noexcept {
        tt_assume(value < max());
        return value >= half();
    }

    constexpr FontVariant &set_weight(FontWeight rhs) noexcept {
        value = static_cast<uint8_t>(static_cast<int>(rhs) + (italic() ? half() : 0));
        tt_assume(value < max());
        return *this;
    }

    constexpr FontVariant &set_italic(bool rhs) noexcept {
        value = static_cast<uint8_t>(static_cast<int>(weight()) + (rhs ? half() : 0));
        tt_assume(value < max());
        return *this;
    }

    constexpr operator int () const noexcept {
        tt_assume(value < max());
        return value;
    }

    /** Get an alternative font variant.
    * @param i 0 is current value, 1 is best alternative, 15 is worst alternative.
    */
    constexpr FontVariant alternative(int i) const noexcept {
        tt_assume(i >= 0 && i < max());
        ttlet w = FontWeight_alterative(weight(), i % half());
        ttlet it = italic() == (i < half());
        return {w, it};
    }

    [[nodiscard]] friend std::string to_string(FontVariant const &rhs) noexcept {
        return fmt::format("{}", rhs.weight(), rhs.italic() ? "/italic" : "");
    }

    friend std::ostream &operator<<(std::ostream &lhs, FontVariant const &rhs) {
        return lhs << to_string(rhs);
    }
};

}