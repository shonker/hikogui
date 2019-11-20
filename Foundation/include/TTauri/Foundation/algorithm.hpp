// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <algorithm>
#include <tuple>

namespace TTauri {

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


template<typename It, typename UnaryPredicate>
constexpr It rfind_if(It const first, It const last, UnaryPredicate predicate)
{
    auto i = last;
    do {
        i--;
        if (predicate(*i)) {
            return i;
        }
    } while (i != first);
    return last;
}

template<typename It, typename UnaryPredicate>
constexpr It rfind_if_not(It const first, It const last, UnaryPredicate predicate)
{
    return rfind_if(first, last, [&](auto const &x) { return !predicate(x); });
}

template<typename It, typename T>
constexpr It rfind(It const first, It const last, T const &value)
{
    return rfind_if(first, last, [&](auto const &x) { return x == value; });
}

/*! For each cluster.
 * func() is executed for each cluster that is found between first-last.
 * A cluster is found between two seperators, a seperator is detected with IsClusterSeperator().
 * A cluster does not include the seperator itself.
 */
template<typename It, typename S, typename F>
inline void for_each_cluster(It first, It last, S IsClusterSeperator, F Function)
{
    if (first == last) {
        return;
    }

    // If the first item is a cluster seperator skip over it.
    if (IsClusterSeperator(*first)) {
        first++;
    }

    for (auto i = first; i != last;) {
        auto j = std::find_if(i, last, IsClusterSeperator);
        Function(i, j);

        auto skipOverSeperator = (j == last) ? 0 : 1;
        i = j + skipOverSeperator;
    }
}

template<typename InputIt1, typename InputIt2>
inline bool starts_with(InputIt1 haystack_first, InputIt1 haystack_last, InputIt2 needle_first, InputIt2 needle_last) noexcept
{
    let [haystack_result, needle_result] = std::mismatch(haystack_first, haystack_last, needle_first, needle_last);
    return needle_result == needle_last;
}

template<typename Container1, typename Container2>
inline bool starts_with(Container1 haystack, Container2 needle) noexcept
{
    return starts_with(haystack.begin(), haystack.end(), needle.begin(), needle.end());
}

template<typename InputIt1, typename InputIt2, typename BinaryPredicate>
inline std::pair<InputIt1,InputIt2> rmismatch(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, BinaryPredicate predicate) noexcept
{
    auto i1 = last1;
    auto i2 = last2;

    while (true) {
        if (i1 == first1 && i2 == first2) {
            return {last1, last2};
        } else if (i1 == first1) {
            return {last1, --i2};
        } else if (i2 == first2) {
            return {--i1, last2};
        }

        if (!predicate(*(--i1), *(--i2))) {
            return {i1, i2};
        }
    }
}

template<typename InputIt1, typename InputIt2>
inline std::pair<InputIt1,InputIt2> rmismatch(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2) noexcept
{
    return rmismatch(first1, last1, first2, last2, [&](auto a, auto b) { return a == b; });  
}

template<typename InputIt1, typename InputIt2>
inline bool ends_with(InputIt1 haystack_first, InputIt1 haystack_last, InputIt2 needle_first, InputIt2 needle_last) noexcept
{
    let [haystack_result, needle_result] = rmismatch(haystack_first, haystack_last, needle_first, needle_last);
    return needle_result == needle_last;
}

template<typename Container1, typename Container2>
inline bool ends_with(Container1 haystack, Container2 needle) noexcept
{
    return ends_with(haystack.begin(), haystack.end(), needle.begin(), needle.end());
}

}
