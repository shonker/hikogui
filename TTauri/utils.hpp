// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "exceptions.hpp"
#include <gsl/gsl>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <functional>
#include <algorithm>
#include <thread>
#include <atomic>
#include <type_traits>
#include <unordered_map>

namespace TTauri {

template<typename T>
inline T &get_singleton() noexcept
{
    static auto x = std::make_unique<T>();
    return *x;
}

template<typename T>
gsl_suppress(type.1)
inline T &at(gsl::span<std::byte> bytes, size_t offset) noexcept
{
    required_assert(to_int64(offset + sizeof(T)) <= to_int64(bytes.size()));
    return *reinterpret_cast<T *>(&bytes[offset]);
}

template<typename T>
gsl_suppress(type.1)
inline T const &at(gsl::span<std::byte const> bytes, size_t offset) noexcept
{
    required_assert(to_int64(offset + sizeof(T)) <= to_int64(bytes.size()));
    return *reinterpret_cast<T const *>(&bytes[offset]);
}

template<typename T>
gsl_suppress(type.1)
inline gsl::span<T> make_span(gsl::span<std::byte> bytes, size_t offset, size_t count) noexcept
{
    let size = count * sizeof(T);
    required_assert(to_int64(offset + size) <= to_int64(bytes.size()));
    return gsl::span<T>(reinterpret_cast<T *>(&bytes[offset]), count);
}

template<typename T>
gsl_suppress(type.1)
inline gsl::span<T const> make_span(gsl::span<std::byte const> bytes, size_t offset, size_t count) noexcept
{
    let size = count * sizeof(T);
    required_assert(to_int64(offset + size) <= to_int64(bytes.size()));
    return gsl::span<T const>(reinterpret_cast<T const *>(&bytes[offset]), count);
}

template<typename T>
gsl_suppress(type.1)
inline gsl::span<T> make_span(gsl::span<std::byte> bytes, size_t offset=0)
{
    let size = numeric_cast<size_t>(bytes.size());
    let count = size / sizeof(T);
    required_assert(size % sizeof(T) == 0);
    return gsl::span<T>(reinterpret_cast<T *>(&bytes[offset]), count);
}

template<typename T>
gsl_suppress(type.1)
inline gsl::span<T const> make_span(gsl::span<std::byte const> bytes, size_t offset=0)
{
    let size = numeric_cast<size_t>(bytes.size());
    let count = size / sizeof(T);
    required_assert(size % sizeof(T) == 0);
    return gsl::span<T const>(reinterpret_cast<T const *>(&bytes[offset]), count);
}

template<typename R, typename T>
gsl_suppress3(type.1,26487,lifetime.4)
inline R align(T ptr, size_t alignment) noexcept
{
    let byteOffset = reinterpret_cast<ptrdiff_t>(ptr);
    let alignedByteOffset = ((byteOffset + alignment - 1) / alignment) * alignment;

    return reinterpret_cast<R>(alignedByteOffset);
}

/*! Align an end iterator.
 * This lowers the end interator so that it the last read is can be done fully.
 */
template<typename R, typename T>
gsl_suppress5(f.23,bounds.3,type.1,26487,lifetime.4)
inline R align_end(T ptr, size_t alignment) noexcept
{
    let byteOffset = reinterpret_cast<ptrdiff_t>(ptr);
    let alignedByteOffset = (byteOffset / alignment) * alignment;

    return reinterpret_cast<R>(alignedByteOffset);
}

gsl_suppress3(f.23,bounds.1,bounds.3)
inline constexpr uint32_t fourcc(char const txt[5]) noexcept
{
    return (
        (static_cast<uint32_t>(txt[0]) << 24) |
        (static_cast<uint32_t>(txt[1]) << 16) |
        (static_cast<uint32_t>(txt[2]) <<  8) |
        static_cast<uint32_t>(txt[3])
   );
}

gsl_suppress(bounds.3)
inline std::string fourcc_to_string(uint32_t x) noexcept
{
    char c_str[5];
    c_str[0] = numeric_cast<char>((x >> 24) & 0xff);
    c_str[1] = numeric_cast<char>((x >> 16) & 0xff);
    c_str[2] = numeric_cast<char>((x >> 8) & 0xff);
    c_str[3] = numeric_cast<char>(x & 0xff);
    c_str[4] = 0;

    return {c_str};
}

template<typename T>
inline typename T::value_type pop_back(T &v) noexcept
{
    typename T::value_type x = std::move(v.back());
    v.pop_back();
    return x;
}

template<typename T>
inline std::vector<T> split(T haystack, typename T::value_type needle) noexcept
{
    std::vector<T> r;

    size_t offset = 0;
    size_t pos = haystack.find(needle, offset);
    while (pos != haystack.npos) {
        r.push_back(haystack.substr(offset, pos - offset));

        offset = pos + 1;
        pos = haystack.find(needle, offset);
    }

    r.push_back(haystack.substr(offset, haystack.size() - offset));
    return r;
}

template<typename CharT>
inline typename std::basic_string<CharT> join(std::vector<std::basic_string<CharT>> const &list, std::basic_string<CharT> const &joiner = {}) noexcept
{
    std::basic_string<CharT> r;
    
    if (list.size() > 1) {
        size_t final_size = (list.size() - 1) * joiner.size();
        for (let &item: list) {
            final_size += item.size();
        }
        r.capacity(final_size);
    }

    int64_t i = 0;
    for (let &item: list) {
        if (i++ > 0) {
            r.push_back(joiner);
        }
        r.push_back(item);
    }
    return r;
}

template<typename CharT>
inline typename std::basic_string<CharT> join(std::vector<std::basic_string_view<CharT>> const &list, std::basic_string<CharT> const &joiner = {}) noexcept
{
    std::basic_string<CharT> r;

    if (list.size() > 1) {
        size_t final_size = (list.size() - 1) * joiner.size();
        for (let &item: list) {
            final_size += item.size();
        }
        r.capacity(final_size);
    }

    int64_t i = 0;
    for (let &item: list) {
        if (i++ > 0) {
            r.push_back(joiner);
        }
        r.push_back(item);
    }
    return r;
}

template<typename It, typename T>
constexpr It rfind(It const begin, It const end, T const &value)
{
    auto i = end;
    do {
        i--;
        if (*i == value) {
            return i;
        }
    } while (i != begin);
    return end;
}

template<typename T>
inline std::enable_if_t<!std::is_pointer_v<T>, T> middle(T begin, T end) noexcept
{
    return begin + std::distance(begin, end) / 2;
}

template<typename T>
inline std::enable_if_t<std::is_pointer_v<T>, T> middle(T begin, T end) noexcept
{
    return reinterpret_cast<T>((reinterpret_cast<intptr_t>(begin) + reinterpret_cast<intptr_t>(end)) / 2);;
}

template<typename T, typename U>
inline T binary_nearest_find(T begin, T end, U value) noexcept
{
    while (begin < end) {
        let m = middle(begin, end);

        if (value > *m) {
            begin = m + 1;
        } else if (value < *m) {
            end = m;
        } else {
            return m;
        }
    }
    return begin;
}

template<typename T, typename U, typename F>
inline T transform(const U &input, F operation)
{
    T result = {};
    result.reserve(input.size());
    std::transform(input.begin(), input.end(), std::back_inserter(result), operation);
    return result;
}

template<typename T, size_t N, typename F>
constexpr std::array<T, N> generate_array(F operation)
{
    std::array<T, N> a{};

    for (size_t i = 0; i < N; i++) {
        a.at(i) = operation(i);
    }

    return a;
}

template<typename T, typename F>
inline void erase_if(T &v, F operation)
{
    while (true) {
        let i = std::find_if(v.begin(), v.end(), operation);
        if (i == v.end()) {
            return;
        }
        v.erase(i);
    }
}

template <class To, class From>
typename std::enable_if<(sizeof(To) == sizeof(From)) && std::is_trivially_copyable<From>::value && std::is_trivial<To>::value,
// this implementation requires that To is trivially default constructible
To>::type
// constexpr support needs compiler magic
bit_cast(const From &src) noexcept
{
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}

constexpr char nibble_to_char(uint8_t nibble) noexcept
{
    if (nibble <= 9) {
        return '0' + nibble;
    } else if (nibble <= 15) {
        return 'a' + nibble - 10;
    } else {
        no_default;
    }
}

constexpr uint8_t char_to_nibble(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return (c - 'a') + 10;
    } else if (c >= 'A' && c <= 'F') {
        return (c - 'A') + 10;
    } else {
         TTAURI_THROW(parse_error("Could not parse hexadecimal digit")
            << error_info<"parse_string"_tag>(std::string(1, c))
        );
    }
}

template<typename T>
inline void cleanupWeakPointers(std::vector<std::weak_ptr<T>> &v) noexcept
{
    auto i = v.begin();
    while (i != v.end()) {
        if (i->expired()) {
            i = v.erase(i);
        } else {
            i++;
        }
    }
}

template<typename K, typename T>
inline void cleanupWeakPointers(std::unordered_map<K,std::weak_ptr<T>> &v) noexcept
{
    auto i = v.begin();
    while (i != v.end()) {
        if (i->second.expired()) {
            i = v.erase(i);
        } else {
            i++;
        }
    }
}

template<typename K, typename T>
inline void cleanupWeakPointers(std::unordered_map<K,std::vector<std::weak_ptr<T>>> &v) noexcept
{
    auto i = v.begin();
    while (i != v.end()) {
        cleanupWeakPointers(i->second);
        if (i->second.size() == 0) {
            i = v.erase(i);
        } else {
            i++;
        }
    }
}

}
