// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/wsRGBA.hpp"
#include "TTauri/Foundation/memory.hpp"
#include "TTauri/Foundation/type_traits.hpp"
#include "TTauri/Foundation/throw_exception.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstring>
#include <cstdint>
#include <variant>
#include <limits>
#include <type_traits>
#include <typeinfo>
#include <ostream>
#include <numeric>
#include <string_view>

namespace TTauri {
template<bool HasLargeObjects>
class datum_impl;

}

namespace std {

template<bool HasLargeObjects>
class hash<TTauri::datum_impl<HasLargeObjects>> {
public:
    size_t operator()(TTauri::datum_impl<HasLargeObjects> const &value) const;
};

}

namespace TTauri {

enum class datum_type_t {
    Null,
    Undefined,
    Boolean,
    Integer,
    Float,
    String,
    URL,
    Map,
    Vector,
    wsRGBA
};

/** A fixed size (64 bits) class for a generic value type.
 * A datum can hold and do calculations with the following types:
 *  - Floating point number (double, without NaN)
 *  - Signed integer number (52 bits)
 *  - Boolean
 *  - Null
 *  - Undefined
 *  - String
 *  - Vector of datum
 *  - Unordered map of datum:datum.
 *  - wsRGBA color.
 *
 * Due to the recursive nature of the datum type (through vector and map)
 * you can serialize your own types by adding conversion constructor and
 * operator to and from the datum on your type.
 *
 * @param HasLargeObjects true when the datum will manage memory for large objects 
 */
template<bool HasLargeObjects>
class datum_impl {
private:
    /** Encode 0 to 6 UTF-8 code units in to a uint64_t.
     *
     * @param str String to encode into an uint64_t
     * @return Encoded string, or zero if `str` did not fit.
     */
    static constexpr uint64_t make_string(std::string_view str) {
        let len = str.size();

        if (len > 6) {
            return 0;
        }

        uint64_t x = 0;
        for (uint64_t i = 0; i < len; i++) {
            x <<= 8;
            x |= str[i];
        }
        return (string_mask + (len << 48)) | x;
    }

    /** Encode a pointer into a uint64_t.
     *
     * @param mask A mask signifying the type of the pointer.
     * @param ptr Pointer to encode.
     * @return Encoded pointer.
     */
    static uint64_t make_pointer(uint64_t mask, void *ptr) {
        return mask | (reinterpret_cast<uint64_t>(ptr) & pointer_mask);
    }

    /** Convert an type_id to a mask.
     *
     * @param id Type id
     * @return Encoded type id.
     */
    static constexpr uint64_t id_to_mask(uint64_t id) {
        return id << 48;
    }

    /** Make an id from a 5 bit integer.
     * Encodes the 5 bit integer into the top 16 bits of a floating
     * point NaN. The msb is encoded into the sign-bit the other 4 bits
     * in bits 52:49.
     *
     * @param id 5 bit integer.
     * @return 16-bit type id.
     */
    static constexpr uint16_t make_id(uint16_t id) {
        return ((id & 0x10) << 11) | (id & 0xf) | 0x7ff0;
    }

    /// Lowest integer that can be encoded into datum's storage.
    static constexpr int64_t minimum_int = 0xfffc'0000'0000'0000LL;
    /// Highest integer that can be encoded into datum's storage.
    static constexpr int64_t maximum_int = 0x0003'ffff'ffff'ffffLL;

    static constexpr uint16_t exponent_mask = 0b0111'1111'1111'0000;
    static constexpr uint64_t pointer_mask = 0x0000'ffff'ffff'ffff;

    static constexpr uint16_t phy_boolean_id       = make_id(0b00001);
    static constexpr uint16_t phy_null_id          = make_id(0b00010);
    static constexpr uint16_t phy_undefined_id     = make_id(0b00011);
    static constexpr uint16_t phy_reserved_id0     = make_id(0b00100);
    static constexpr uint16_t phy_reserved_id1     = make_id(0b00101);
    static constexpr uint16_t phy_reserved_id2     = make_id(0b00110);
    static constexpr uint16_t phy_reserved_id3     = make_id(0b00111);
    static constexpr uint16_t phy_integer_id0      = make_id(0b01000);
    static constexpr uint16_t phy_integer_id1      = make_id(0b01001);
    static constexpr uint16_t phy_integer_id2      = make_id(0b01010);
    static constexpr uint16_t phy_integer_id3      = make_id(0b01011);
    static constexpr uint16_t phy_integer_id4      = make_id(0b01100);
    static constexpr uint16_t phy_integer_id5      = make_id(0b01101);
    static constexpr uint16_t phy_integer_id6      = make_id(0b01110);
    static constexpr uint16_t phy_integer_id7      = make_id(0b01111);

    static constexpr uint16_t phy_string_id0       = make_id(0b10001);
    static constexpr uint16_t phy_string_id1       = make_id(0b10010);
    static constexpr uint16_t phy_string_id2       = make_id(0b10011);
    static constexpr uint16_t phy_string_id3       = make_id(0b10100);
    static constexpr uint16_t phy_string_id4       = make_id(0b10101);
    static constexpr uint16_t phy_string_id5       = make_id(0b10110);
    static constexpr uint16_t phy_string_id6       = make_id(0b10111);
    static constexpr uint16_t phy_string_ptr_id    = make_id(0b11000);
    static constexpr uint16_t phy_url_ptr_id       = make_id(0b11001);
    static constexpr uint16_t phy_integer_ptr_id   = make_id(0b11010);
    static constexpr uint16_t phy_vector_ptr_id    = make_id(0b11011);
    static constexpr uint16_t phy_map_ptr_id       = make_id(0b11100);
    static constexpr uint16_t phy_wsrgba_ptr_id    = make_id(0b11101);
    static constexpr uint16_t phy_reserved_ptr_id1 = make_id(0b11110);
    static constexpr uint16_t phy_reserved_ptr_id2 = make_id(0b11111);

    static constexpr uint64_t boolean_mask = id_to_mask(phy_boolean_id);
    static constexpr uint64_t null_mask = id_to_mask(phy_null_id);
    static constexpr uint64_t undefined_mask = id_to_mask(phy_undefined_id);
    static constexpr uint64_t string_mask = id_to_mask(phy_string_id0);
    static constexpr uint64_t character_mask = id_to_mask(phy_string_id1);
    static constexpr uint64_t integer_mask = id_to_mask(phy_integer_id0);
    static constexpr uint64_t string_ptr_mask = id_to_mask(phy_string_ptr_id);
    static constexpr uint64_t url_ptr_mask = id_to_mask(phy_url_ptr_id);
    static constexpr uint64_t integer_ptr_mask = id_to_mask(phy_integer_ptr_id);
    static constexpr uint64_t vector_ptr_mask = id_to_mask(phy_vector_ptr_id);
    static constexpr uint64_t map_ptr_mask = id_to_mask(phy_map_ptr_id);
    static constexpr uint64_t wsrgba_ptr_mask = id_to_mask(phy_wsrgba_ptr_id);

    union {
        double f64;
        uint64_t u64;
    };

    /** Extract the type_id from datum's storage.
     * This function will get the most siginicant 16 bits from
     * the datum's storage. It uses std::memcpy() to make sure
     * there is no undefined behaviour, due to not knowing
     * ahead of time if a double or uint64_t was stored.
     *
     * @return The type_id of the stored object.
     */
    force_inline uint16_t type_id() const noexcept {
        uint64_t data;
        std::memcpy(&data, this, sizeof(data));
        return static_cast<uint16_t>(data >> 48);
    }

    force_inline bool is_phy_float() const noexcept {
        let id = type_id();
        return (id & 0x7ff0) != 0x7ff0 || (id & 0x000f) == 0;
    }

    force_inline bool is_phy_integer() const noexcept {
        return (type_id() & 0xfff8) == 0x7ff8;
    }

    force_inline bool is_phy_string() const noexcept {
        let id = type_id();
        return (id & 0xfff8) == 0xfff0 && (id & 0x0007) > 0;
    }

    force_inline bool is_phy_boolean() const noexcept {
        return type_id() == phy_boolean_id;
    }

    force_inline bool is_phy_null() const noexcept {
        return type_id() == phy_null_id;
    }

    force_inline bool is_phy_undefined() const noexcept {
        return type_id() == phy_undefined_id;
    }

    force_inline bool is_phy_pointer() const noexcept {
        return HasLargeObjects && (type_id() & 0xfff8) == 0xfff8;
    }

    force_inline bool is_phy_string_ptr() const noexcept {
        return HasLargeObjects && type_id() == phy_string_ptr_id;
    }

    force_inline bool is_phy_url_ptr() const noexcept {
        return HasLargeObjects && type_id() == phy_url_ptr_id;
    }

    force_inline bool is_phy_integer_ptr() const noexcept {
        return HasLargeObjects && type_id() == phy_integer_ptr_id;
    }

    force_inline bool is_phy_vector_ptr() const noexcept {
        return HasLargeObjects && type_id() == phy_vector_ptr_id;
    }

    force_inline bool is_phy_map_ptr() const noexcept {
        return HasLargeObjects && type_id() == phy_map_ptr_id;
    }

    force_inline bool is_phy_wsrgba_ptr() const noexcept {
        return HasLargeObjects && type_id() == phy_wsrgba_ptr_id;
    }

    /** Extract the 48 bit unsigned integer from datum's storage.
     */
    force_inline uint64_t get_unsigned_integer() const noexcept {
        return (u64 << 16) >> 16;
    }

    /** Extract the 48 bit signed integer from datum's storage and sign extent to 64 bit.
     */
    force_inline int64_t get_signed_integer() const noexcept {
        return static_cast<int64_t>(u64 << 16) >> 16;
    }

    /** Extract a pointer for an existing object from datum's storage.
     * Canonical pointers on x86 and ARM are at most 48 bit and are sign extended to 64 bit.
     * Since the pointer is stored as a 48 bit integer, this function will launder it.
     */
    template<typename O>
    force_inline O *get_pointer() const {
        return std::launder(reinterpret_cast<O *>(get_signed_integer()));
    }

    /** Delete the object that the datum is pointing to.
     * This function should only be called on a datum that holds a pointer.
     */
    void delete_pointer() noexcept {
        if constexpr (HasLargeObjects) {
            switch (type_id()) {
            case phy_integer_ptr_id: delete get_pointer<int64_t>(); break;
            case phy_string_ptr_id: delete get_pointer<std::string>(); break;
            case phy_url_ptr_id: delete get_pointer<URL>(); break;
            case phy_vector_ptr_id: delete get_pointer<datum_impl::vector>(); break;
            case phy_map_ptr_id: delete get_pointer<datum_impl::map>(); break;
            case phy_wsrgba_ptr_id: delete get_pointer<wsRGBA>(); break;
            default: no_default;
            }
        }
    }

    /** Copy the object pointed to by the other datum into this datum.
     * Other datum must point to an object. This datum must not point to an object.
     *
     * @param other The other datum which holds a pointer to an object.
     */
    void copy_pointer(datum_impl const &other) noexcept {
        if constexpr (HasLargeObjects) {
            switch (other.type_id()) {
            case phy_integer_ptr_id: {
                auto * const p = new int64_t(*other.get_pointer<int64_t>());
                u64 = make_pointer(integer_ptr_mask, p);
            } break;

            case phy_string_ptr_id: {
                auto * const p = new std::string(*other.get_pointer<std::string>());
                u64 = make_pointer(string_ptr_mask, p);
            } break;

            case phy_url_ptr_id: {
                auto * const p = new URL(*other.get_pointer<URL>());
                u64 = make_pointer(url_ptr_mask, p);
            } break;

            case phy_vector_ptr_id: {
                auto * const p = new datum_impl::vector(*other.get_pointer<datum_impl::vector>());
                u64 = make_pointer(vector_ptr_mask, p);
            } break;

            case phy_map_ptr_id: {
                auto * const p = new datum_impl::map(*other.get_pointer<datum_impl::map>());
                u64 = make_pointer(map_ptr_mask, p);
            } break;

            case phy_wsrgba_ptr_id: {
                auto * const p = new wsRGBA(*other.get_pointer<wsRGBA>());
                u64 = make_pointer(wsrgba_ptr_mask, p);
            } break;

            default:
                no_default;
            }
        }
    }

    
public:
    using vector = std::vector<datum_impl>;
    using map = std::unordered_map<datum_impl,datum_impl>;
    struct undefined {};
    struct null {};

    datum_impl() noexcept : u64(undefined_mask) {}

    ~datum_impl() noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
    }

    datum_impl(datum_impl const &other) noexcept {
        if (ttauri_unlikely(other.is_phy_pointer())) {
            copy_pointer(other);
        } else {
            // We do a memcpy, because we don't know the type in the union.
            std::memcpy(this, &other, sizeof(*this));
        }
    }

    datum_impl &operator=(datum_impl const &other) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        if (ttauri_unlikely(other.is_phy_pointer())) {
            copy_pointer(other);
        } else {
            // We do a memcpy, because we don't know the type in the union.
            std::memcpy(this, &other, sizeof(*this));
        }
        return *this;
    }

    datum_impl(datum_impl &&other) noexcept : u64(undefined_mask) {
        // We do a memcpy, because we don't know the type in the union.
        std::memcpy(this, &other, sizeof(*this));
        other.u64 = undefined_mask;
    }

    datum_impl &operator=(datum_impl &&other) noexcept {
        swap(*this, other);
        return *this;
    }

    datum_impl(datum_impl::null) noexcept : u64(null_mask) {}

    template <class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    datum_impl(T value) noexcept :
        f64(value)
    {
        if (value != value) {
            u64 = undefined_mask;
        }
    }

    template <class T, std::enable_if_t<is_numeric_integer_v<T> && std::is_unsigned_v<T>, int> = 0>
    datum_impl(T value) noexcept :
        u64(integer_mask | value)
    {
        if (ttauri_unlikely(value > maximum_int)) {
            if constexpr (HasLargeObjects) {
                auto * const p = new uint64_t(value);
                u64 = make_pointer(integer_ptr_mask, p);
            } else {
                TTAURI_THROW_OVERFLOW_ERROR("Constructing datum from integer {}, larger than {}", value, maximum_int);
            }
        }
    }

    template <class T, std::enable_if_t<is_numeric_integer_v<T> && std::is_signed_v<T>, int> = 0>
    datum_impl(T value) noexcept :
        u64(integer_mask | (static_cast<uint64_t>(value) & 0x0007ffff'ffffffff))
    {
        if (ttauri_unlikely(value < minimum_int || value > maximum_int)) {
            if constexpr (HasLargeObjects) {
                auto * const p = new int64_t(value);
                u64 = make_pointer(integer_ptr_mask, p);
            } else {
                TTAURI_THROW_OVERFLOW_ERROR("Constructing integer {} to datum, outside {} and {}", value, minimum_int, maximum_int);
            }
        }
    }

    datum_impl(bool value) noexcept : u64(boolean_mask | static_cast<uint64_t>(value)) {}

    datum_impl(char value) noexcept : u64(character_mask | value) {}

    datum_impl(std::string_view value) noexcept : u64(make_string(value)) {
        if (value.size() > 6) {
            if constexpr (HasLargeObjects) {
                auto * const p = new std::string(value);
                u64 = make_pointer(string_ptr_mask, p);
            } else {
                TTAURI_THROW_OVERFLOW_ERROR("Constructing string {} to datum, larger than 6 characters", value);
            }
        }
    }

    datum_impl(std::string const &value) noexcept : datum_impl(std::string_view(value)) {}

    datum_impl(char const *value) noexcept : datum_impl(std::string_view(value)) {}

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl(URL const &value) noexcept {
        auto * const p = new URL(value);
        u64 = make_pointer(url_ptr_mask, p);
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl(datum_impl::vector const &value) noexcept {
        auto * const p = new datum_impl::vector(value);
        u64 = make_pointer(vector_ptr_mask, p);
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl(datum_impl::map const &value) noexcept {
        auto * const p = new datum_impl::map(value);
        u64 = make_pointer(map_ptr_mask, p);
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl(wsRGBA const &value) noexcept {
        auto * const p = new wsRGBA(value);
        u64 = make_pointer(wsrgba_ptr_mask, p);
    }

    datum_impl &operator=(datum_impl::null rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = null_mask;
        return *this;
    }

    template <class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    datum_impl &operator=(T rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        if (rhs == rhs) {
            f64 = rhs;
        } else {
            u64 = undefined_mask;
        }
        return *this;
    }
    
    template <class T, std::enable_if_t<is_numeric_integer_v<T> && std::is_unsigned_v<T>, int> = 0>
    datum_impl &operator=(T rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        u64 = integer_mask | static_cast<uint64_t>(rhs);
        if (ttauri_unlikely(rhs > maximum_int)) {
            if constexpr (HasLargeObjects) {
                auto * const p = new uint64_t(rhs);
                u64 = make_pointer(integer_ptr_mask, p);
            } else {
                TTAURI_THROW_OVERFLOW_ERROR("Assigning integer {} to datum, larger than {}", rhs, maximum_int);
            }
        }
        return *this;
    }

    template <class T, std::enable_if_t<is_numeric_integer_v<T> && std::is_signed_v<T>, int> = 0>
    datum_impl &operator=(T rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        u64 = integer_mask | (static_cast<uint64_t>(rhs) & 0x0007ffff'ffffffff);
        if (ttauri_unlikely(rhs < minimum_int || rhs > maximum_int)) {
            if constexpr (HasLargeObjects) {
                auto * const p = new int64_t(rhs);
                u64 = make_pointer(integer_ptr_mask, p);
            } else {
                TTAURI_THROW_OVERFLOW_ERROR("Assigning integer {} to datum, outside {} and {}", rhs, minimum_int, maximum_int);
            }
        }

        return *this;
    }

    datum_impl &operator=(bool rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = boolean_mask | static_cast<uint64_t>(rhs);
        return *this;
    }
    
    datum_impl &operator=(char rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }
        u64 = character_mask | static_cast<uint64_t>(rhs);
        return *this;
    }

    datum_impl &operator=(std::string_view rhs) {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        u64 = make_string(rhs);
        if (rhs.size() > 6) {
            if constexpr (HasLargeObjects) {
                auto * const p = new std::string(rhs);
                u64 = make_pointer(string_ptr_mask, p);
            } else {
                TTAURI_THROW_OVERFLOW_ERROR("Assigning string {} to datum, larger than 6 characters", rhs);
            }
        }
        return *this;
    }

    datum_impl &operator=(std::string const &rhs) {
        *this = std::string_view{rhs};
        return *this;
    }

    datum_impl &operator=(char const *rhs) {
        *this = std::string_view{rhs};
        return *this;
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl &operator=(URL const &rhs) noexcept {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        auto * const p = new URL(rhs);
        u64 = make_pointer(url_ptr_mask, p);
        return *this;
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl &operator=(datum_impl::vector const &rhs) {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        auto * const p = new datum_impl::vector(rhs);
        u64 = make_pointer(vector_ptr_mask, p);

        return *this;
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl &operator=(datum_impl::map const &rhs) {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        auto * const p = new datum_impl::map(rhs);
        u64 = make_pointer(map_ptr_mask, p);

        return *this;
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    datum_impl &operator=(wsRGBA const &rhs) {
        if (ttauri_unlikely(is_phy_pointer())) {
            delete_pointer();
        }

        auto * const p = new wsRGBA(rhs);
        u64 = make_pointer(wsrgba_ptr_mask, p);

        return *this;
    }

    explicit operator double() const {
        if (is_phy_float()) {
            return f64;
        } else if (is_phy_integer()) {
            return static_cast<double>(get_signed_integer());
        } else if (is_phy_integer_ptr()) {
            return static_cast<double>(*get_pointer<int64_t>());
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a double", this->repr(), this->type_name());
        }
    }

    explicit operator float() const {
        return static_cast<float>(static_cast<double>(*this));
    }

    explicit operator signed long long () const {
        if (is_phy_integer()) {
            return get_signed_integer();
        } else if (is_phy_integer_ptr()) {
            return *get_pointer<signed long long>();
        } else if (is_phy_float()) {
            return static_cast<signed long long>(f64);
        } else if (is_phy_boolean()) {
            return get_unsigned_integer() > 0 ? 1 : 0;
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a signed long long", this->repr(), this->type_name());
        }
    }

    explicit operator signed long () const {
        let v = static_cast<signed long long>(*this);
        if (v < std::numeric_limits<signed long>::min() || v > std::numeric_limits<signed long>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a signed long", this->repr(), this->type_name());
        }
        return static_cast<signed long>(v);
    }

    explicit operator signed int () const {
        let v = static_cast<signed long long>(*this);
        if (v < std::numeric_limits<signed int>::min() || v > std::numeric_limits<signed int>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a signed int", this->repr(), this->type_name());
        }
        return static_cast<signed int>(v);
    }

    explicit operator signed short () const {
        let v = static_cast<signed long long>(*this);
        if (v < std::numeric_limits<signed short>::min() || v > std::numeric_limits<signed short>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a signed short", this->repr(), this->type_name());
        }
        return static_cast<signed short>(v);
    }

    explicit operator signed char () const {
        let v = static_cast<int64_t>(*this);
        if (v < std::numeric_limits<signed char>::min() || v > std::numeric_limits<signed char>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a signed char", this->repr(), this->type_name());
        }
        return static_cast<signed char>(v);
    }

    explicit operator unsigned long long () const {
        let v = static_cast<signed long long>(*this);
        return static_cast<unsigned long long>(v);
    }

    explicit operator unsigned long () const {
        let v = static_cast<unsigned long long>(*this);
        if ( v > std::numeric_limits<unsigned long>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a unsigned long", this->repr(), this->type_name());
        }
        return static_cast<unsigned long>(v);
    }

    explicit operator unsigned int () const {
        let v = static_cast<unsigned long long>(*this);
        if (v > std::numeric_limits<unsigned int>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a unsigned int", this->repr(), this->type_name());
        }
        return static_cast<unsigned int>(v);
    }

    explicit operator unsigned short () const {
        let v = static_cast<unsigned long long>(*this);
        if (v > std::numeric_limits<unsigned short>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a unsigned short", this->repr(), this->type_name());
        }
        return static_cast<unsigned short>(v);
    }

    explicit operator unsigned char () const {
        let v = static_cast<unsigned long long>(*this);
        if (v > std::numeric_limits<unsigned char>::max()) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a unsigned char", this->repr(), this->type_name());
        }
        return static_cast<unsigned char>(v);
    }

    explicit operator bool() const noexcept {
        switch (type_id()) {
        case phy_boolean_id: return get_unsigned_integer() > 0;
        case phy_null_id: return false;
        case phy_undefined_id: return false;
        case phy_integer_id0:
        case phy_integer_id1:
        case phy_integer_id2:
        case phy_integer_id3:
        case phy_integer_id4:
        case phy_integer_id5:
        case phy_integer_id6:
        case phy_integer_id7: return static_cast<int64_t>(*this) != 0;
        case phy_integer_ptr_id: return *get_pointer<int64_t>() != 0;
        case phy_string_id0:
        case phy_string_id1:
        case phy_string_id2:
        case phy_string_id3:
        case phy_string_id4:
        case phy_string_id5:
        case phy_string_id6:
        case phy_string_ptr_id: return this->size() > 0;
        case phy_url_ptr_id: return true;
        case phy_vector_ptr_id: return this->size() > 0;
        case phy_map_ptr_id: return this->size() > 0;
        case phy_wsrgba_ptr_id: return !(get_pointer<wsRGBA>()->isTransparent());
        default:
            if (ttauri_likely(is_phy_float())) {
                return static_cast<double>(*this) != 0.0;
            } else {
                no_default;
            };
        }
    }

    explicit operator char() const {
        if (is_phy_string() && size() == 1) {
            return u64 & 0xff;
        } else if (is_phy_string_ptr() && size() == 1) {
            return get_pointer<std::string>()->at(0);
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a char", this->repr(), this->type_name());
        }
    }

    explicit operator std::string() const noexcept {
        switch (type_id()) {
        case phy_boolean_id:
            return static_cast<bool>(*this) ? "true" : "false";

        case phy_null_id:
            return "null";

        case phy_undefined_id:
            return "undefined";

        case phy_integer_id0:
        case phy_integer_id1:
        case phy_integer_id2:
        case phy_integer_id3:
        case phy_integer_id4:
        case phy_integer_id5:
        case phy_integer_id6:
        case phy_integer_id7: return fmt::format("{}", static_cast<int64_t>(*this));
        case phy_integer_ptr_id:
            if constexpr (HasLargeObjects) {
                return fmt::format("{}", static_cast<int64_t>(*this));
            } else {
                no_default;
            }

        case phy_string_id0:
        case phy_string_id1:
        case phy_string_id2:
        case phy_string_id3:
        case phy_string_id4:
        case phy_string_id5:
        case phy_string_id6: {
                let length = size();
                char buffer[6];
                for (int i = 0; i < length; i++) {
                    buffer[i] = (u64 >> ((length - i - 1) * 8)) & 0xff;
                }
                return std::string(buffer, length);
            }

        case phy_string_ptr_id:
            if constexpr (HasLargeObjects) {
                return *get_pointer<std::string>();
            } else {
                no_default;
            }

        case phy_url_ptr_id:
            if constexpr (HasLargeObjects) {
                return get_pointer<URL>()->string();
            } else {
                no_default;
            }

        case phy_wsrgba_ptr_id:
            if constexpr (HasLargeObjects) {
                return to_string(*get_pointer<wsRGBA>());
            } else {
                no_default;
            }

        case phy_vector_ptr_id:
            if constexpr (HasLargeObjects) {
                std::string r = "[";
                auto count = 0;
                for (auto i = vector_begin(); i != vector_end(); i++) {
                    if (count++ > 0) {
                        r += ", ";
                    }
                    r += i->repr();
                }
                r += "]";
                return r;
            } else {
                no_default;
            }

        case phy_map_ptr_id:
            if constexpr (HasLargeObjects) {
                std::vector<std::pair<datum,datum>> items;
                items.reserve(size());
                std::copy(map_begin(), map_end(), std::back_inserter(items));
                std::sort(items.begin(), items.end(), [](auto &a, auto &b) {
                    return a.first < b.first;
                });

                std::string r = "{";
                auto count = 0;
                for (auto &item: items) {
                    if (count++ > 0) {
                        r += ", ";
                    }
                    r += item.first.repr();
                    r += ": ";
                    r += item.second.repr();
                }
                r += "}";
                return r;
            } else {
                no_default;
            }

        default:
            if (is_phy_float()) {
                auto str = fmt::format("{:g}", static_cast<double>(*this));
                if (str.find('.') == str.npos) {
                    str += ".0";
                }
                return str;
            } else {
                no_default;
            }
        }
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    explicit operator URL() const {
        if (is_string()) {
            return URL{static_cast<std::string>(*this)};
        } else if (is_url()) {
            return *get_pointer<URL>();
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a URL", this->repr(), this->type_name());
        }
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    explicit operator datum_impl::vector() const {
        if (is_vector()) {
            return *get_pointer<datum_impl::vector>();
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a Vector", this->repr(), this->type_name());
        }
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    explicit operator datum_impl::map() const {
        if (is_map()) {
            return *get_pointer<datum_impl::map>();
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a Map", this->repr(), this->type_name());
        }
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    explicit operator wsRGBA() const {
        if (is_wsrgba()) {
            return *get_pointer<wsRGBA>();
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Value {} of type {} can not be converted to a wsRGBA", this->repr(), this->type_name());
        }
    }

    /** Index into a datum::map or datum::vector.
     * This datum must hold a vector, map or undefined.
     * When this datum holds undefined it is treated as if datum holds an empty map.
     * When this datum holds a vector, the index must be datum holding an integer.
     *
     * @param rhs An index into the map or vector.
     */
    datum_impl &operator[](datum_impl const &rhs) {
        if (is_undefined()) {
            // When accessing a name on an undefined it means we need replace it with an empty map.
            auto *p = new datum_impl::map();
            u64 = map_ptr_mask | (reinterpret_cast<uint64_t>(p) & pointer_mask);
        }

        if (is_map()) {
            auto *m = get_pointer<datum_impl::map>();
            auto [i, did_insert] = m->try_emplace(rhs);
            return i->second;

        } else if (is_vector() && rhs.is_integer()) {
            let index = static_cast<int64_t>(rhs);
            auto *v = get_pointer<datum_impl::vector>();

            if (index < 0 || index >= to_signed(v->size())) {
                TTAURI_THROW_INVALID_OPERATION_ERROR("Index {} out of range to access value in vector of size {}", index, v->size());
            } else {
                return (*v)[index];
            }
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot index value of type {} with {} of type {}", type_name(), rhs.repr(), rhs.type_name());
        }
    }

    /** Index into a datum::map or datum::vector.
     * This datum must hold a vector, map or undefined.
     * When this datum holds undefined it is treated as if datum holds an empty map.
     * When this datum holds a vector, the index must be datum holding an integer.
     *
     * @param rhs An index into the map or vector.
     */
    datum_impl operator[](datum_impl const &rhs) const {
        if (is_map()) {
            auto *m = get_pointer<datum_impl::map>();
            let i = m->find(rhs);
            if (i == m->end()) {
                TTAURI_THROW_INVALID_OPERATION_ERROR("Could not find key {} in map of size {}", rhs.repr(), m->size());
            }
            return i->second;

        } else if (is_vector() && rhs.is_integer()) {
            let index = static_cast<int64_t>(rhs);
            auto *v = get_pointer<datum_impl::vector>();

            if (index < 0 || index >= to_signed(v->size())) {
                TTAURI_THROW_INVALID_OPERATION_ERROR("Index {} out of range to access value in vector of size {}", index, v->size());
            } else {
                return (*v)[index];
            }
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot index value of type {} with {} of type {}", type_name(), rhs.repr(), rhs.type_name());
        }
    }

    /** Append and return a reference to a datum holding undefined to this datum.
     * This datum holding a undefined will be treated as if it is holding an empty vector.
     * This datum must hold a vector.
     * @return a reference to datum holding a vector.
     */ 
    datum_impl &append() {
        if (is_undefined()) {
            // When appending on undefined it means we need replace it with an empty vector.
            auto *p = new datum_impl::vector();
            u64 = vector_ptr_mask | (reinterpret_cast<uint64_t>(p) & pointer_mask);
        }

        if (is_vector()) {
            auto *v = get_pointer<datum_impl::vector>();
            v->emplace_back();
            return v->back();

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot append new item onto type {}", type_name());
        }
    }

    template<typename... Args>
    void emplace_back(Args... &&args) {
        if (is_undefined()) {
            // When appending on undefined it means we need replace it with an empty vector.
            auto *p = new datum_impl::vector();
            u64 = vector_ptr_mask | (reinterpret_cast<uint64_t>(p) & pointer_mask);
        }

        if (is_vector()) {
            auto *v = get_pointer<datum_impl::vector>();
            v->emplace_back(std::forward<Args>(args)...);

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot append new item onto type {}", type_name());
        }
    }

    template<typename Arg>
    void push_back(Arg &&arg) {
        if (is_undefined()) {
            // When appending on undefined it means we need replace it with an empty vector.
            auto *p = new datum_impl::vector();
            u64 = vector_ptr_mask | (reinterpret_cast<uint64_t>(p) & pointer_mask);
        }

        if (is_vector()) {
            auto *v = get_pointer<datum_impl::vector>();
            v->push_back(std::forward<Arg>(arg));

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Cannot append new item onto type {}", type_name());
        }
    }

    std::string repr() const noexcept{
        switch (type_id()) {
        case phy_boolean_id: return static_cast<std::string>(*this);
        case phy_null_id: return static_cast<std::string>(*this);
        case phy_undefined_id: return static_cast<std::string>(*this);
        case phy_integer_id0:
        case phy_integer_id1:
        case phy_integer_id2:
        case phy_integer_id3:
        case phy_integer_id4:
        case phy_integer_id5:
        case phy_integer_id6:
        case phy_integer_id7:
        case phy_integer_ptr_id: return static_cast<std::string>(*this);
        case phy_string_id0:
        case phy_string_id1:
        case phy_string_id2:
        case phy_string_id3:
        case phy_string_id4:
        case phy_string_id5:
        case phy_string_id6:
        case phy_string_ptr_id: return fmt::format("\"{}\"", static_cast<std::string>(*this));
        case phy_url_ptr_id: return fmt::format("<URL {}>", static_cast<std::string>(*this));
        case phy_vector_ptr_id: return static_cast<std::string>(*this);
        case phy_map_ptr_id: return static_cast<std::string>(*this);
        case phy_wsrgba_ptr_id: return fmt::format("<wsRGBA {}>", static_cast<std::string>(*this));
        default:
            if (ttauri_likely(is_phy_float())) {
                return static_cast<std::string>(*this);
            } else {
                no_default;
            }
        }
    }

    /*! Return ordering of types.
     * Used in less-than comparison between different types.
     */
    int type_order() const noexcept {
        if (is_float() || is_phy_integer_ptr()) {
            // Fold all numeric values into the same group (literal integers).
            return phy_integer_id0;
        } else {
            return type_id();
        }
    }

    datum_impl &get_by_path(std::vector<std::string> const &key) {
        if (key.size() > 0 && is_map()) {
            let index = key.at(0);
            auto &next = (*this)[index];
            let next_key = std::vector<std::string>{key.begin() + 1, key.end()};
            return next.get_by_path(next_key);

        } else if (key.size() > 0 && is_vector()) {
            size_t const index = std::stoll(key.at(0));
            auto &next = (*this)[index];
            let next_key = std::vector<std::string>{key.begin() + 1, key.end()};
            return next.get_by_path(next_key);

        } else if (key.size() > 0) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("type {} does not support get() with '{}'", type_name(), key.at(0));
        } else {
            return *this;
        }
    }

    datum_impl get_by_path(std::vector<std::string> const &key) const {
        if (key.size() > 0 && is_map()) {
            let index = key.at(0);
            let next = (*this)[index];
            return next.get_by_path({key.begin() + 1, key.end()});

        } else if (key.size() > 0 && is_vector()) {
            size_t const index = std::stoll(key.at(0));
            let next = (*this)[index];
            return next.get_by_path({key.begin() + 1, key.end()});

        } else if (key.size() > 0) {
            TTAURI_THROW_INVALID_OPERATION_ERROR("type {} does not support get() with '{}'", type_name(), key.at(0));
        } else {
            return *this;
        }
    }

    bool is_integer() const noexcept { return is_phy_integer() || is_phy_integer_ptr(); }
    bool is_float() const noexcept { return is_phy_float(); }
    bool is_string() const noexcept { return is_phy_string() || is_phy_string_ptr(); }
    bool is_boolean() const noexcept { return is_phy_boolean(); }
    bool is_null() const noexcept { return is_phy_null(); }
    bool is_undefined() const noexcept { return is_phy_undefined(); }
    bool is_url() const noexcept { return is_phy_url_ptr(); }
    bool is_vector() const noexcept { return is_phy_vector_ptr(); }
    bool is_map() const noexcept { return is_phy_map_ptr(); }
    bool is_wsrgba() const noexcept { return is_phy_wsrgba_ptr(); }
    bool is_numeric() const noexcept { return is_integer() || is_float(); }
    bool is_color() const noexcept { return is_wsrgba(); }

    datum_type_t type() const noexcept {
        switch (type_id()) {
        case phy_boolean_id: return datum_type_t::Boolean;
        case phy_null_id: return datum_type_t::Null;
        case phy_undefined_id: return datum_type_t::Undefined;
        case phy_integer_id0:
        case phy_integer_id1:
        case phy_integer_id2:
        case phy_integer_id3:
        case phy_integer_id4:
        case phy_integer_id5:
        case phy_integer_id6:
        case phy_integer_id7:
        case phy_integer_ptr_id: return datum_type_t::Integer;
        case phy_string_id0:
        case phy_string_id1:
        case phy_string_id2:
        case phy_string_id3:
        case phy_string_id4:
        case phy_string_id5:
        case phy_string_id6:
        case phy_string_ptr_id: return datum_type_t::String;
        case phy_url_ptr_id: return datum_type_t::URL;
        case phy_vector_ptr_id: return datum_type_t::Vector;
        case phy_map_ptr_id: return datum_type_t::Map;
        case phy_wsrgba_ptr_id: return datum_type_t::wsRGBA;
        default:
            if (ttauri_likely(is_phy_float())) {
                return datum_type_t::Float;
            } else {
                no_default;
            }
        }
    }

    char const *type_name() const noexcept{
        switch (type_id()) {
        case phy_boolean_id: return "Boolean";
        case phy_null_id: return "Null";
        case phy_undefined_id: return "Undefined";
        case phy_integer_id0:
        case phy_integer_id1:
        case phy_integer_id2:
        case phy_integer_id3:
        case phy_integer_id4:
        case phy_integer_id5:
        case phy_integer_id6:
        case phy_integer_id7:
        case phy_integer_ptr_id: return "Integer";
        case phy_string_id0:
        case phy_string_id1:
        case phy_string_id2:
        case phy_string_id3:
        case phy_string_id4:
        case phy_string_id5:
        case phy_string_id6:
        case phy_string_ptr_id: return "String";
        case phy_url_ptr_id: return "URL";
        case phy_vector_ptr_id: return "Vector";
        case phy_map_ptr_id: return "Map";
        case phy_wsrgba_ptr_id: return "wsRGBA";
        default:
            if (ttauri_likely(is_phy_float())) {
                return "Float";
            } else {
                no_default;
            }
        }
    }

    size_t size() const{
        switch (type_id()) {
        case phy_string_id0:
        case phy_string_id1:
        case phy_string_id2:
        case phy_string_id3:
        case phy_string_id4:
        case phy_string_id5:
        case phy_string_id6: return to_signed(((u64 & 0xffff'0000'0000'0000ULL) - string_mask) >> 48);
        case phy_string_ptr_id: return get_pointer<std::string>()->size();
        case phy_vector_ptr_id: return get_pointer<datum_impl::vector>()->size();
        case phy_map_ptr_id: return get_pointer<datum_impl::map>()->size();
        case phy_wsrgba_ptr_id: return 4;
        default: TTAURI_THROW_INVALID_OPERATION_ERROR("Can't get size of value {} of type {}.", this->repr(), this->type_name());
        }
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    typename map::const_iterator map_begin() const noexcept {
        if (is_phy_map_ptr()) {
            return get_pointer<datum_impl::map>()->begin();
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("map_begin() expect datum to be a map, but it is a {}.", this->type_name());
        }
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    typename map::const_iterator map_end() const noexcept {
        if (is_phy_map_ptr()) {
            return get_pointer<datum_impl::map>()->end();
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("map_end() expect datum to be a map, but it is a {}.", this->type_name());
        }
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    typename vector::const_iterator vector_begin() const noexcept {
        if (is_phy_vector_ptr()) {
            return get_pointer<datum_impl::vector>()->begin();
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("vector_begin() expect datum to be a vector, but it is a {}.", this->type_name());
        }
    }

    template<bool P=HasLargeObjects, std::enable_if_t<P,int> = 0>
    typename vector::const_iterator vector_end() const noexcept{
        if (is_phy_vector_ptr()) {
            return get_pointer<datum_impl::vector>()->end();
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("vector_end() expect datum to be a vector, but it is a {}.", this->type_name());
        }
    }

    size_t hash() const noexcept{
        if (is_phy_float()) {
            return std::hash<double>{}(f64);
        } else if (ttauri_unlikely(is_phy_pointer())) {
            switch (type_id()) {
            case phy_string_ptr_id:
                return std::hash<std::string>{}(*get_pointer<std::string>());
            case phy_url_ptr_id:
                return std::hash<URL>{}(*get_pointer<URL>());
            case phy_vector_ptr_id:
                return std::accumulate(vector_begin(), vector_end(), size_t{0}, [](size_t a, auto x) {
                    return a ^ x.hash();
                    });
            case phy_map_ptr_id:
                return std::accumulate(map_begin(), map_end(), size_t{0}, [](size_t a, auto x) {
                    return a ^ (x.first.hash() ^ x.second.hash());
                    });
            case phy_wsrgba_ptr_id:
                return std::hash<wsRGBA>{}(*get_pointer<wsRGBA>());
            default: no_default;
            }
        } else {
            return std::hash<uint64_t>{}(u64);
        }
    }

    friend bool operator==(datum_impl const &lhs, datum_impl const &rhs) noexcept {
        switch (lhs.type_id()) {
        case datum_impl::phy_boolean_id:
            return rhs.is_boolean() && static_cast<bool>(lhs) == static_cast<bool>(rhs);
        case datum_impl::phy_null_id:
            return rhs.is_null();
        case datum_impl::phy_undefined_id:
            return rhs.is_undefined();
        case datum_impl::phy_integer_id0:
        case datum_impl::phy_integer_id1:
        case datum_impl::phy_integer_id2:
        case datum_impl::phy_integer_id3:
        case datum_impl::phy_integer_id4:
        case datum_impl::phy_integer_id5:
        case datum_impl::phy_integer_id6:
        case datum_impl::phy_integer_id7:
        case datum_impl::phy_integer_ptr_id:
            return (
                (rhs.is_float() && static_cast<double>(lhs) == static_cast<double>(rhs)) ||
                (rhs.is_integer() && static_cast<int64_t>(lhs) == static_cast<int64_t>(rhs))
                );
        case datum_impl::phy_string_id0:
        case datum_impl::phy_string_id1:
        case datum_impl::phy_string_id2:
        case datum_impl::phy_string_id3:
        case datum_impl::phy_string_id4:
        case datum_impl::phy_string_id5:
        case datum_impl::phy_string_id6:
        case datum_impl::phy_string_ptr_id:
            return (
                (rhs.is_string() && static_cast<std::string>(lhs) == static_cast<std::string>(rhs)) ||
                (rhs.is_url() && static_cast<URL>(lhs) == static_cast<URL>(rhs))
                );
        case datum_impl::phy_url_ptr_id:
            return (rhs.is_url() || rhs.is_string()) && static_cast<URL>(lhs) == static_cast<URL>(rhs);
        case datum_impl::phy_vector_ptr_id:
            return rhs.is_vector() && *lhs.get_pointer<datum_impl::vector>() == *rhs.get_pointer<datum_impl::vector>();
        case datum_impl::phy_map_ptr_id:
            return rhs.is_map() && *lhs.get_pointer<datum_impl::map>() == *rhs.get_pointer<datum_impl::map>();
        case datum_impl::phy_wsrgba_ptr_id:
            return rhs.is_wsrgba() && *lhs.get_pointer<wsRGBA>() == *rhs.get_pointer<wsRGBA>();
        default:
            if (lhs.is_phy_float()) {
                return rhs.is_numeric() && static_cast<double>(lhs) == static_cast<double>(rhs);
            } else {
                no_default;
            }
        }
    }

    friend bool operator<(datum_impl const &lhs, datum_impl const &rhs) noexcept {
        switch (lhs.type_id()) {
        case datum_impl::phy_boolean_id:
            if (rhs.is_boolean()) {
                return static_cast<bool>(lhs) < static_cast<bool>(rhs);
            } else {
                return lhs.type_order() < rhs.type_order();
            }
        case datum_impl::phy_null_id:
            return lhs.type_order() < rhs.type_order();
        case datum_impl::phy_undefined_id:
            return lhs.type_order() < rhs.type_order();
        case datum_impl::phy_integer_id0:
        case datum_impl::phy_integer_id1:
        case datum_impl::phy_integer_id2:
        case datum_impl::phy_integer_id3:
        case datum_impl::phy_integer_id4:
        case datum_impl::phy_integer_id5:
        case datum_impl::phy_integer_id6:
        case datum_impl::phy_integer_id7:
        case datum_impl::phy_integer_ptr_id:
            if (rhs.is_float()) {
                return static_cast<double>(lhs) < static_cast<double>(rhs);
            } else if (rhs.is_integer()) {
                return static_cast<int64_t>(lhs) < static_cast<int64_t>(rhs);
            } else {
                return lhs.type_order() < rhs.type_order();
            }
        case datum_impl::phy_string_id0:
        case datum_impl::phy_string_id1:
        case datum_impl::phy_string_id2:
        case datum_impl::phy_string_id3:
        case datum_impl::phy_string_id4:
        case datum_impl::phy_string_id5:
        case datum_impl::phy_string_id6:
        case datum_impl::phy_string_ptr_id:
            if (rhs.is_string()) {
                return static_cast<std::string>(lhs) < static_cast<std::string>(rhs);
            } else if (rhs.is_url()) {
                return static_cast<URL>(lhs) < static_cast<URL>(rhs);
            } else {
                return lhs.type_order() < rhs.type_order();
            }
        case datum_impl::phy_url_ptr_id:
            if (rhs.is_url() || rhs.is_string()) {
                return static_cast<URL>(lhs) < static_cast<URL>(rhs);
            } else {
                return lhs.type_order() < rhs.type_order();
            }
        case datum_impl::phy_vector_ptr_id:
            if (rhs.is_vector()) {
                return *lhs.get_pointer<datum_impl::vector>() < *rhs.get_pointer<datum_impl::vector>();
            } else {
                return lhs.type_order() < rhs.type_order();
            }
        case datum_impl::phy_map_ptr_id:
            if (rhs.is_map()) {
                return *lhs.get_pointer<datum_impl::map>() < *rhs.get_pointer<datum_impl::map>();
            } else {
                return lhs.type_order() < rhs.type_order();
            }
        case datum_impl::phy_wsrgba_ptr_id:
            if (rhs.is_wsrgba()) {
                return *lhs.get_pointer<wsRGBA>() < *rhs.get_pointer<wsRGBA>();
            } else {
                return lhs.type_order() < rhs.type_order();
            }
        default:
            if (lhs.is_phy_float()) {
                if (rhs.is_numeric()) {
                    return static_cast<double>(lhs) < static_cast<double>(rhs);
                } else {
                    return lhs.type_order() < rhs.type_order();
                }
            } else {
                no_default;
            }
        }
    }

    friend bool operator!=(datum_impl const &lhs, datum_impl const &rhs) noexcept {
        return !(lhs == rhs);
    }

    friend bool operator>(datum_impl const &lhs, datum_impl const &rhs) noexcept {
        return rhs < lhs;
    }

    friend bool operator<=(datum_impl const &lhs, datum_impl const &rhs) noexcept {
        return !(rhs < lhs);
    }

    friend bool operator>=(datum_impl const &lhs, datum_impl const &rhs) noexcept {
        return !(lhs < rhs);
    }

    //friend bool operator!(datum_impl const &rhs) const noexcept {
    //    return !static_cast<bool>(rhs);
    //}

    friend datum_impl operator~(datum_impl const &rhs) {
        if (rhs.is_integer()) {
            return datum{~static_cast<int64_t>(rhs)};
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't bit-wise negate '~' value {} of type {}",
                rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator-(datum_impl const &rhs) {
        if (rhs.is_integer()) {
            return datum{-static_cast<int64_t>(rhs)};
        } else if (rhs.is_float()) {
            return datum{-static_cast<double>(rhs)};
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't arithmetic negate '-' value {} of type {}",
                rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator+(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_integer() && rhs.is_integer()) {
            let lhs_ = static_cast<int64_t>(lhs);
            let rhs_ = static_cast<int64_t>(rhs);
            return datum{lhs_ + rhs_};

        } else if (lhs.is_numeric() && rhs.is_numeric()) {
            let lhs_ = static_cast<double>(lhs);
            let rhs_ = static_cast<double>(rhs);
            return datum{lhs_ + rhs_};

        } else if (lhs.is_string() && rhs.is_string()) {
            let lhs_ = static_cast<std::string>(lhs);
            let rhs_ = static_cast<std::string>(rhs);
            return datum{std::move(lhs_ + rhs_)};

        } else if (lhs.is_vector() && rhs.is_vector()) {
            auto lhs_ = static_cast<datum_impl::vector>(lhs);
            let &rhs_ = *(rhs.get_pointer<datum_impl::vector>());
            std::copy(rhs_.begin(), rhs_.end(), std::back_inserter(lhs_));
            return datum{std::move(lhs_)};

        } else if (lhs.is_map() && rhs.is_map()) {
            let &lhs_ = *(lhs.get_pointer<datum_impl::map>());
            auto rhs_ = static_cast<datum_impl::map>(rhs);
            for (let &item: lhs_) {
                rhs_.try_emplace(item.first, item.second);
            }
            return datum{std::move(rhs_)};

        } else if (lhs.is_wsrgba() && rhs.is_wsrgba()) {
            auto lhs_ = static_cast<wsRGBA>(lhs);
            lhs_.composit(*(rhs.get_pointer<wsRGBA>()));
            return datum{lhs_};

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't add '+' value {} of type {} to value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator-(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_integer() && rhs.is_integer()) {
            let lhs_ = static_cast<int64_t>(lhs);
            let rhs_ = static_cast<int64_t>(rhs);
            return datum_impl{lhs_ - rhs_};

        } else if (lhs.is_numeric() && rhs.is_numeric()) {
            let lhs_ = static_cast<double>(lhs);
            let rhs_ = static_cast<double>(rhs);
            return datum_impl{lhs_ - rhs_};

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't subtract '-' value {} of type {} from value {} of type {}",
                rhs.repr(), rhs.type_name(), lhs.repr(), lhs.type_name()
            );
        }
    }

    friend datum_impl operator*(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_integer() && rhs.is_integer()) {
            let lhs_ = static_cast<int64_t>(lhs);
            let rhs_ = static_cast<int64_t>(rhs);
            return datum_impl{lhs_ * rhs_};

        } else if (lhs.is_numeric() && rhs.is_numeric()) {
            let lhs_ = static_cast<double>(lhs);
            let rhs_ = static_cast<double>(rhs);
            return datum_impl{lhs_ * rhs_};

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't multiply '+' value {} of type {} with value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator/(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_integer() && rhs.is_integer()) {
            let lhs_ = static_cast<int64_t>(lhs);
            let rhs_ = static_cast<int64_t>(rhs);
            return datum_impl{lhs_ / rhs_};

        } else if (lhs.is_numeric() && rhs.is_numeric()) {
            let lhs_ = static_cast<double>(lhs);
            let rhs_ = static_cast<double>(rhs);
            return datum_impl{lhs_ / rhs_};

            // XXX implement path concatenation.
        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't divide '/' value {} of type {} by value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator%(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_integer() && rhs.is_integer()) {
            let lhs_ = static_cast<int64_t>(lhs);
            let rhs_ = static_cast<int64_t>(rhs);
            return datum_impl{lhs_ % rhs_};

        } else if (lhs.is_numeric() && rhs.is_numeric()) {
            let lhs_ = static_cast<double>(lhs);
            let rhs_ = static_cast<double>(rhs);
            return datum_impl{std::fmod(lhs_,rhs_)};

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't take modulo '%' value {} of type {} by value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator<<(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_integer() && rhs.is_integer()) {
            let lhs_ = static_cast<uint64_t>(lhs);
            let rhs_ = static_cast<int64_t>(rhs);
            if (rhs_ < -63) {
                return datum_impl{0};
            } else if (rhs_ < 0) {
                // Pretend this is a unsigned shift right.
                return datum_impl{lhs_ >> -rhs_};
            } else if (rhs_ == 0) {
                return lhs;
            } else if (rhs_ > 63) {
                return datum_impl{0};
            } else {
                return datum_impl{lhs_ << rhs_};
            }

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't logical shift-left '<<' value {} of type {} with value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator>>(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_integer() && rhs.is_integer()) {
            let lhs_ = static_cast<uint64_t>(lhs);
            let rhs_ = static_cast<int64_t>(rhs);
            if (rhs_ < -63) {
                return datum_impl{0};
            } else if (rhs_ < 0) {
                return datum_impl{lhs_ << -rhs_};
            } else if (rhs_ == 0) {
                return lhs;
            } else if (rhs_ > 63) {
                return (lhs_ >= 0) ? datum_impl{0} : datum_impl{-1};
            } else {
                return datum_impl{static_cast<int64_t>(lhs_) >> rhs_};
            }

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't arithmetic shift-right '>>' value {} of type {} with value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator&(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_integer() && rhs.is_integer()) {
            let lhs_ = static_cast<uint64_t>(lhs);
            let rhs_ = static_cast<uint64_t>(rhs);
            return datum_impl{lhs_ & rhs_};

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't AND '&' value {} of type {} with value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator|(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_integer() && rhs.is_integer()) {
            let lhs_ = static_cast<uint64_t>(lhs);
            let rhs_ = static_cast<uint64_t>(rhs);
            return datum_impl{lhs_ | rhs_};

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't OR '|' value {} of type {} with value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend datum_impl operator^(datum_impl const &lhs, datum_impl const &rhs) {
        if (lhs.is_integer() && rhs.is_integer()) {
            let lhs_ = static_cast<uint64_t>(lhs);
            let rhs_ = static_cast<uint64_t>(rhs);
            return datum{lhs_ ^ rhs_};

        } else {
            TTAURI_THROW_INVALID_OPERATION_ERROR("Can't XOR '^' value {} of type {} with value {} of type {}",
                lhs.repr(), lhs.type_name(), rhs.repr(), rhs.type_name()
            );
        }
    }

    friend std::string to_string(datum_impl const &d) {
        return static_cast<std::string>(d);
    }

    friend std::ostream &operator<<(std::ostream &os, datum_impl const &d) {
        return os << static_cast<std::string>(d);
    }

    friend void swap(datum_impl &lhs, datum_impl &rhs) noexcept {
        memswap(lhs, rhs);
    }

    template<typename T>
    friend bool will_cast_to(datum_impl<HasLargeObjects> const &rhs) {
        if constexpr (std::is_same_v<T, bool>) {
            return true;
        } else if constexpr (std::is_same_v<T, char>) {
            return rhs.is_string();
        } else if constexpr (std::is_arithmetic_v<T>) {
            return rhs.is_numeric();
        } else if constexpr (std::is_same_v<T, datum_impl::undefined>) {
            return rhs.is_undefined();
        } else if constexpr (std::is_same_v<T, datum_impl::vector>) {
            return rhs.is_vector();
        } else if constexpr (std::is_same_v<T, datum_impl::map>) {
            return rhs.is_map();
        } else if constexpr (std::is_same_v<T, wsRGBA>) {
            return rhs.is_wsrgba();
        } else if constexpr (std::is_same_v<T, URL>) {
            return rhs.is_url() || rhs.is_string();
        } else if constexpr (std::is_same_v<T, std::string>) {
            return true;
        } else {
            return false;
        }
    }
};

template<bool HasLargeObjects>
bool operator<(typename datum_impl<HasLargeObjects>::map const &lhs, typename datum_impl<HasLargeObjects>::map const &rhs) noexcept {
    auto lhs_keys = transform<datum_impl::vector>(lhs, [](auto x) { return x.first; });
    auto rhs_keys = transform<datum_impl::vector>(lhs, [](auto x) { return x.first; });

    if (lhs_keys == rhs_keys) {
        for (let &k: lhs_keys) {
            if (lhs.at(k) == rhs.at(k)) {
                continue;
            } else if (lhs.at(k) < rhs.at(k)) {
                return true;
            } else {
                return false;
            }
        }
    } else {
        return lhs_keys < rhs_keys;
    }
    return false;
}

using datum = datum_impl<true>;
using sdatum = datum_impl<false>;

}

namespace std {

template<bool HasLargeObjects>
inline size_t hash<TTauri::datum_impl<HasLargeObjects>>::operator()(TTauri::datum_impl<HasLargeObjects> const &value) const {
    return value.hash();
}



}
