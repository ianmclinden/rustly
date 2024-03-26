#pragma once

#include <format>
#include <source_location>

namespace
{
    template <class... Args>
    [[noreturn]] static inline void __panic_impl(
        const std::source_location loc = std::source_location::current(),
        const std::format_string<Args...> &fmt = "explicit panic",
        Args &&...args) noexcept
    {
        auto m = std::format("panicked at {}:{}\n{}\n", loc.file_name(), loc.line(), std::format(fmt, std::forward<Args>(args)...));
        std::fputs(m.c_str(), stderr);
        std::abort();
    }
}

/// @brief Allows a program to terminate immediately and provide feedback to the caller of the program.
#define panic(...) __panic_impl(std::source_location::current(), ##__VA_ARGS__)