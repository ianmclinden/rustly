#pragma once

#include <functional>
#include <variant>
#include <rustly/display.h>
#include <rustly/panic.h>
#include <rustly/option.h>

namespace rustly
{
    template <class T>
    class Option; // forward declare

    template <class T, class E>
    class Result : private std::variant<T, E> // C++23 std::expected
    {
    public:
        [[nodiscard("this `Result` may be an `Err` variant, which should be handled")]] Result(std::variant<T, E> v)
            : std::variant<T, E>{v} {}

        bool operator==(const Result<T, E> &rhs) const
        {
            return (is_ok() && rhs.is_ok() && (std::get<0>(*this) == std::get<0>(rhs))) ||
                   (is_err() && rhs.is_err() && (std::get<1>(*this) == std::get<1>(rhs)));
        }

        /// Returns `true` if the result is `Ok`.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Ok<uint32_t, const char *>(-3);
        /// assert(x.is_ok() == true);
        ///
        /// auto x = Err<uint32_t, const char *>("Some error message");
        /// assert(x.is_ok() == false);
        /// ```
        [[nodiscard("if you intended to assert that this is ok, consider `.unwrap()` instead")]] inline bool
        is_ok() const
        {
            return this->index() == 0;
        }

        /// Returns `true` if the result is `Ok` and the value inside of it matches a predicate.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Ok<uint32_t, const char *>(2);
        /// assert(x.is_ok_and([](auto x){ return x > 1; }) == true);
        ///
        /// auto x = Ok<uint32_t, const char *>(0);
        /// assert(x.is_ok_and([](auto x){ return x > 1; }) == false);
        ///
        /// auto x = Err<uint32_t, const char *>("hey");
        /// assert(x.is_ok_and([](auto x){ return x > 1; }) == false);
        /// ```
        [[nodiscard]] inline bool
        is_ok_and(const std::function<bool(T)> &f) const
        {
            return is_ok() && f(std::get<0>(*this));
        }

        /// Returns `true` if the result is `Err`.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Ok<uint32_t, const char *>(-3);
        /// assert(x.is_err() == false);
        ///
        /// auto x = Err<uint32_t, const char *>("Some error message");
        /// assert(x.is_err() == true);
        /// ```
        [[nodiscard("if you intended to assert that this is err, consider `.unwrap_err()` instead")]] inline bool
        is_err() const
        {
            return this->index() == 1;
        }

        /// Returns `true` if the result is `Err` and the value inside of it matches a predicate.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Err<uint32_t, const char *>("Some error message");
        /// assert(x.is_err_and([](auto x){ return x == "Some error message"; }) == true);
        ///
        /// auto x = Err<uint32_t, const char *>("Some other message");
        /// assert(x.is_err_and([](auto x){ return x == "Some error message"; }) == false);
        ///
        /// auto x = Ok<uint32_t, const char *>(17);
        /// assert(x.is_err_and([](auto x){ return x == "Some error message"; }) == false);
        /// ```
        [[nodiscard]] inline bool
        is_err_and(const std::function<bool(E)> &f) const
        {
            return is_err() && f(std::get<1>(*this));
        }

        /// Converts from `Result<T, E>` to `Option<T>`.
        ///
        /// Converts `this` into an `Option<T>`, discarding the error, if any.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Ok<uint32_t, const char *>(2);
        /// assert(x.ok() == Some(2));
        ///
        /// auto y = Err<uint32_t, const char *>("Nothing here");
        /// assert(y.ok() == None());
        /// ```
        inline Option<T> ok() const
        {
            return (is_ok() ? Option<T>(std::get<0>(*this)) : Option<T>());
        }

        /// Converts from `Result<T, E>` to `Option<E>`.
        ///
        /// Converts `this` into an `Option<E>`, discarding the success value, if any.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Ok<uint32_t, const char *>(2);
        /// assert(x.err() == None(2));
        ///
        /// auto y = Err<uint32_t, const char *>("Nothing here");
        /// assert(y.err() == Some("Nothing here"));
        /// ```
        inline Option<E> err() const
        {
            return (is_err() ? Option<E>(std::get<1>(*this)) : Option<E>());
        }

        /// Maps a `Result<T, E>` to `Result<U, E>` by applying a function to a
        /// contained `Ok` value, leaving an `Err` value untouched.
        ///
        /// This function can be used to compose the results of two functions.
        ///
        /// ## Examples
        /// ```cpp
        /// std::function<size_t(std::string)> f = [](auto s){ return s.size(); };
        ///
        /// auto x = Ok<std::string, int>(std::string("Hello, World!"));
        /// assert(x.map(f) == Ok<size_t, int>(13));
        ///
        /// auto y = Err<std::string, int>(-1);
        /// assert(y.map(f) == Err<size_t, int>(-1));
        /// ```
        template <class U>
        inline Result<U, E> map(const std::function<U(T)> &f) const
        {
            return (is_ok() ? Result<U, E>(f(std::get<0>(*this))) : Result<U, E>(std::get<1>(*this)));
        }

        /// Returns the provided default (if `Err`), or
        /// applies a function to the contained value (if `Ok`).
        ///
        /// Arguments passed to `map_or` are eagerly evaluated; if you are passing
        /// the result of a function call, it is recommended to use `map_or_else`,
        /// which is lazily evaluated.
        ///
        /// ## Examples
        /// ```cpp
        /// std::function<size_t(std::string)> f = [](auto s){ return s.size(); };
        ///
        /// auto x = Ok<std::string, int>(std::string("Hello, World!"));
        /// assert(x.map_or((size_t)42, f) == 13);
        ///
        /// auto y = Err<std::string, int>(-1);
        /// assert(y.map_or((size_t)42, f) == 42);
        /// ```
        template <class U>
        inline U map_or(U def, const std::function<U(T)> &f) const
        {
            return (is_ok() ? f(std::get<0>(*this)) : def);
        }

        /// Maps a `Result<T, E>` to `U` by applying fallback function `default` to
        /// a contained `Err` value, or function `f` to a contained `Ok` value.
        ///
        /// This function can be used to unpack a successful result
        /// while handling an error.
        ///
        /// ## Examples
        /// ```cpp
        /// std::function<size_t(std::string)> len = [](auto s){ return s.size(); };
        /// std::function<int(int)> dbl = [](auto x){ return 2 * x; };
        ///
        /// int k = 21;
        ///
        /// auto x = Ok<std::string, std::string>(std::string("foo"));
        /// assert(x.map_or_else(dbl, len) == 3);
        ///
        /// auto y = Err<std::string, std::string> (std::string("bar"));
        /// assert(x.map_or_else(dbl, len) == 42);
        /// ```
        template <class U>
        inline U map_or_else(const std::function<U(E)> &d, const std::function<U(T)> &f) const
        {
            return (is_ok() ? f(std::get<0>(*this)) : d(std::get<1>(*this)));
        }

        /// Maps a `Result<T, E>` to `Result<T, F>` by applying a function to a
        /// contained `Err` value, leaving an `Ok` value untouched.
        ///
        /// This function can be used to pass through a successful result while handling
        /// an error.
        ///
        /// ## Examples
        /// ```cpp
        /// std::function<std::string(int)> stringify = [](auto x){ return std::string("error code: " + std::to_string(x)); };
        ///
        /// auto x = Ok<int, int>(2);
        /// assert(x.map_err(stringify) == Ok<int, std::string>(2));
        ///
        /// auto y = Err<int, int>(13);
        /// assert(y.map_err(stringify) == Err<int, std::string>(std::string("error code: 13")));
        /// ```
        template <class F>
        inline Result<T, F> map_err(const std::function<F(E)> &op) const
        {
            if (is_ok())
            {
                return Result<T, F>(std::variant<T, F>{std::in_place_index<0>, std::get<0>(*this)});
            }
            else
            {
                return Result<T, F>(std::variant<T, F>{std::in_place_index<1>, op(std::get<1>(*this))});
            }
        }

        /// Returns the contained `Ok` value.
        ///
        /// Because this function may panic, its use is generally discouraged.
        /// Instead, call `unwrap_or`, `unwrap_or_else`, or `unwrap_or_default`.
        ///
        /// ## Panics
        /// Panics if the value is an `Err`, with a panic message including the
        /// passed message, and the content of the `Err`.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Err<int, const char *>("emergency failure");
        /// x.expect("Testing expect"); // panics with `Testing expect: emergency failure`
        /// ```
        inline T expect(const std::string &msg, const std::source_location _loc = std::source_location::current()) const
            requires ToString<E>
        {
            if (is_ok())
            {
                return std::get<0>(*this);
            }
            __panic_impl(_loc, "{}: {}", msg, std::to_string(std::get<1>(*this)));
        }

        /// Returns the contained `Ok` value.
        ///
        /// Because this function may panic, its use is generally discouraged.
        /// Instead, call `unwrap_or`, `unwrap_or_else`, or `unwrap_or_default`.
        ///
        /// ## Panics
        /// Panics if the value is an `Err`, with a panic message provided by the
        /// `Err`'s value.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Ok<int, const char *>(2);
        /// assert(x.unwrap() == 2);
        ///
        /// auto y = Err<int, const char *>("emergency failure");
        /// y.unwrap(); // panics with `emergency failure`
        /// ```
        inline T unwrap(const std::source_location _loc = std::source_location::current()) const
            requires ToString<E>
        {
            if (is_ok())
            {
                return std::get<0>(*this);
            }
            __panic_impl(_loc, "called `Result::unwrap()` on an `Err` value: {}", std::to_string(std::get<1>(*this)));
        }

        /// Returns the contained `Ok` value or a provided default.
        ///
        /// Arguments passed to `unwrap_or` are eagerly evaluated; if you are passing
        /// the result of a function call, it is recommended to use `unwrap_or_else`,
        /// which is lazily evaluated.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Ok<int, const char*>(9);
        /// assert(x.unwrap_or(default) == 9);
        ///
        /// auto y = Err<int, const char*>("error");
        /// assert(y.unwrap_or(2) == 2);
        /// ```
        inline T unwrap_or(T def) const
        {
            return (is_ok() ? std::get<0>(*this) : def);
        }

        /// Returns the contained `Ok` value or computes it from a closure.
        ///
        /// ## Examples
        /// ```cpp
        /// std::function<size_t(std::string)> count = [](auto s) { return s.size(); };
        ///
        /// auto x = Ok<size_t, std::string>(2);
        /// assert(x.unwrap_or_else(count) == 2);
        ///
        /// auto y = Err<size_t, std::string>(std::string("foo"));
        /// assert(x.unwrap_or_else(count) == 3);
        /// ```
        inline T unwrap_or_else(const std::function<T(E)> &op) const
        {
            return (is_ok() ? std::get<0>(*this) : op(std::get<1>(*this)));
        }

        /// Returns the contained `Ok` value or a default
        ///
        /// If `Ok`, returns the contained value, otherwise if
        //  `Err`, returns the default value for that type.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Ok<int, const char *>(71);
        /// assert(x.unwrap_or_default() == 71);
        ///
        /// auto y = Err<int, const char *>("an error");
        /// assert(y.unwrap_or_default() == 0);
        /// ```
        inline T unwrap_or_default() const
            requires std::default_initializable<T>
        {
            return (is_ok() ? std::get<0>(*this) : T{});
        }

        /// Returns the contained `Err` value.
        ///
        /// ## Panics
        /// Panics if the value is an `Ok`, with a panic message including the
        /// passed message, and the content of the `Ok`.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Ok<int, const char *>(10);
        /// x.expect_err("Testing expect_err"); // panics with `Testing expect_err: 10`
        /// ```
        inline E expect_err(const std::string &msg, const std::source_location loc = std::source_location::current()) const
            requires ToString<T>
        {
            if (is_err())
            {
                return std::get<1>(*this);
            }
            __panic_impl(loc, "{}: {}", msg, std::to_string(std::get<0>(*this)));
        }

        /// Returns the contained `Err` value.
        ///
        /// ## Panics
        /// Panics if the value is an `Ok`, with a custom panic message provided
        /// by the `Ok`'s value.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Ok<int, const char*>(2);
        /// x.unwrap_err(); // panics with `2`
        ///
        /// auto y = Err<int, const char *>("emergency failure");
        /// assert(x.unwrap_err() == "emergency failure");
        /// ```
        inline E unwrap_err(const std::source_location loc = std::source_location::current()) const
            requires ToString<T>
        {
            if (is_err())
            {
                return std::get<1>(*this);
            }
            __panic_impl(loc, "called `Result::unwrap_err()` on an `Ok` value: {}", std::to_string(std::get<0>(*this)));
        }

        /// Returns `res` if the result is `Ok`, otherwise returns the `Err` value of `this`.
        ///
        /// Arguments passed to `and` are eagerly evaluated; if you are passing the
        /// result of a function call, it is recommended to use `and_then`, which is
        /// lazily evaluated.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Ok<uint32_t, const char *>(2);
        /// auto y = Err<const char *, const char *>("late error");
        /// assert(x.and_b(y) == Err<const char *, const char *>("late error"));
        ///
        /// auto x = Err<uint32_t, const char *>("early error");
        /// auto y = Ok<const char *, const char *>("foo");
        /// assert(x.and_b(y) == Err<const char *, const char *>("early error"));
        ///
        /// auto x = Err<uint32_t, const char *>("not a 2");
        /// auto y = Err<const char *, const char *>("late error");
        /// assert(x.and_b(y) ==  Err<const char *, const char *>("not a 2"));
        ///
        /// auto x = Ok<uint32_t, const char *>(2);
        /// auto y = Ok<const char *, const char *>("different result type");
        /// assert(x.and_b(y) == Ok<const char *, const char *>("different result type"));
        /// ```
        template <class U>
        inline Result<U, E> and_b(Result<U, E> res) const
        {
            return (is_ok() ? res : Result<U, E>(std::variant<U, E>{std::in_place_index<1>, std::get<1>(*this)}));
        }

        /// Calls `op` if the result is `Ok`, otherwise returns the `Err` value of `this`.
        ///
        /// This function can be used for control flow based on `Result` values.
        ///
        /// ## Examples
        /// ```cpp
        /// #include <cmath>
        ///
        /// std::function<Result<std::string, std::string>(uint32_t)>
        ///     sq_then_to_string = [](uint32_t x)
        /// {
        ///     return (x > sqrt(UINT32_MAX)) ? Err<std::string, std::string>("overflowed")
        ///                                   : Ok<std::string, std::string>(std::to_string(x * x));
        /// };
        ///
        /// auto x = Ok<uint32_t, std::string>(2);
        /// auto expected = Ok<std::string, std::string>(std::string("4"));
        /// assert(x.and_then(sq_then_to_string) == expected);
        ///
        /// auto x = Ok<std::uint32_t, std::string>(1000000);
        /// auto expected = Err<std::string, std::string>(std::string("overflowed"));
        /// assert(x.and_then(sq_then_to_string) ==  expected);
        ///
        /// auto x = Err<uint32_t, std::string>(std::string("not a number"));
        /// auto expected = Err<std::string, std::string>(std::string("not a number"));
        /// assert(x.and_then(sq_then_to_string) == expected);
        /// ```
        template <class U>
        inline Result<U, E> and_then(std::function<Result<U, E>(T)> &op) const
        {
            if (is_ok())
            {
                return op(std::get<0>(*this));
            }
            else
            {
                return Result<U, E>(std::variant<U, E>{std::in_place_index<1>, std::get<1>(*this)});
            }
        }

        /// Returns `res` if the result is `Err`, otherwise returns the `Ok` value of `this`.
        ///
        /// Arguments passed to `or` are eagerly evaluated; if you are passing the
        /// result of a function call, it is recommended to use `or_else`, which is
        /// lazily evaluated.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Ok<uint32_t, const char *>(2);
        /// auto y = Err<uint32_t, const char *>("late error");
        /// assert(x.or_b(y) == Ok<uint32_t, const char *>(2));
        ///
        /// auto x = Err<uint32_t, const char *>("early error");
        /// auto y = Ok<uint32_t, const char *>(2);
        /// assert(x.or_b(y) == Ok<uint32_t, const char *>(2));
        ///
        /// auto x = Err<uint32_t, const char *>("not a 2");
        /// auto y = Err<uint32_t, const char *>("late error");
        /// assert(x.or_b(y) == Err<uint32_t, const char *>("late error"));
        ///
        /// auto x = Ok<uint32_t, const char *>(2);
        /// auto y = Ok<uint32_t, const char *>(100);
        /// assert(x.or_b(y) == Ok<uint32_t, const char *>(2));
        /// ```
        template <class F>
        inline Result<T, F> or_b(Result<T, F> res) const
        {
            return (is_ok() ? Result<T, F>(std::variant<T, F>{std::in_place_index<0>, std::get<0>(*this)}) : res);
        }

        /// Calls `op` if the result is `Err`, otherwise returns the `Ok` value of `this`.
        ///
        /// This function can be used for control flow based on result values.
        ///
        /// ## Examples
        /// ```
        /// std::function<Result<int, int>(int)> sq = [](auto x){ return Ok<int, int>(x * x); };
        /// std::function<Result<int, int>(int)> err = [](auto x){ return Err<int, int>(x); };
        ///
        /// assert(Ok<int, int>(2).or_else(sq).or_else(sq) == Ok<int, int>(2));
        /// assert(Ok<int, int>(2).or_else(err).or_else(sq) == Ok<int, int>(2));
        /// assert(Err<int, int>(3).or_else(sq).or_else(err) == Ok<int, int>(9));
        /// assert(Err<int, int>(3).or_else(err).or_else(err) == Err<int, int>(3));
        /// ```
        template <class F>
        inline Result<T, F> or_else(std::function<Result<T, F>(E)> &op)
        {
            if (is_ok())
            {
                return Result<T, F>(std::variant<T, F>{std::in_place_index<0>, std::get<0>(*this)});
            }
            else
            {
                return op(std::get<1>(*this));
            }
        }
    };

    /// Construct a `Result` with an `Ok` value
    ///
    /// ## Examples
    /// ```cpp
    /// auto x = Ok<const char *, int>("hello");
    /// ```
    template <class T, class E>
    inline static Result<T, E> Ok(T t) { return Result<T, E>(std::variant<T, E>{std::in_place_index<0>, t}); }

    /// Construct a `Result` with an `Err` value
    ///
    /// ## Examples
    /// ```cpp
    /// auto x = Err<uint32_t, const char *>("unexpected");
    /// ```
    template <class T, class E>
    [[nodiscard("this `Result` may be an `Err` variant, which should be handled")]] inline static Result<T, E>
    Err(E e) { return Result<T, E>(std::variant<T, E>{std::in_place_index<1>, e}); }
}