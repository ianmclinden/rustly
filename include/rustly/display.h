#pragma once

#include <concepts>
#include <iostream>
#include <string>

namespace rustly
{
    /// Requires a `std::to_string()` implementation or an explicit `to_string()`
    /// function that returns a `std::string`
    template <typename T>
    concept ToString = std::convertible_to<T, std::string> ||
                       requires(T t) { std::to_string(t); } ||
                       requires(T t) {
                           {
                               t.to_string()
                           } noexcept -> std::convertible_to<const std::string>;
                       };
};

// Default ToString implementations
namespace std
{
    template <typename T>
    // Only for non-implemented classes
        requires(!std::convertible_to<T, std::string> && !requires(T t) { std::to_string(t); })
    std::string to_string(T t) noexcept
    {
        return t.to_string();
    }

    template <typename T>
    // Only for non-implemented classes
        requires(std::convertible_to<T, std::string> && !requires(T t) { std::to_string(t); })
    std::string to_string(T t) noexcept
    {
        return std::string(t);
    }
}

namespace rustly
{
    template <typename T>
    concept ToStream = std::convertible_to<T, std::ostream &> ||
                       requires(std::ostream &os, const T &t) {
                           {
                               os << t
                           };
                       };

    /// Requires that a class be convertible to a `std::ostream` or `std::string`
    template <typename T>
    concept Display = ToString<T> || ToStream<T>;

    // TODO: Default Display implementation?
    // template <typename T>
    //     requires(ToString<T> && !ToStream<T>)
    // std::ostream &operator<<(std::ostream &os, const T &t)
    // {
    //     os << std::to_string(t);
    //     return os;
    // }
}