#pragma once

#include <functional>
#include <optional>
#include <rustly/panic.h>
#include <rustly/result.h>

namespace rustly
{
    template <class T, class E>
    class Result; // forward declare

    template <class T>
    class Option : private std::optional<T>
    {
    public:
        Option() : std::optional<T>() {}
        Option(T t) : std::optional<T>(t) {}

        Option([[maybe_unused]] const Option<std::monostate>
                   &other) // None copy constructor
            : std::optional<T>()
        {
        }

        Option([[maybe_unused]] Option<std::monostate> &&other) noexcept // None move constructor
            : std::optional<T>()
        {
        }

        Option &
        operator=([[maybe_unused]] Option<std::monostate> other) noexcept // None assignment constructor
        {
            this->reset();
            return *this;
        }

        bool
        operator==([[maybe_unused]] const Option<std::monostate> &rhs) const
        {
            return is_none(); // Monostate is always None, only equal if we are too
        }

        template <class U>
            requires std::equality_comparable_with<T, U>
        bool
        operator==(const Option<U> &rhs) const
        {
            if (is_none() != rhs.is_none())
            {
                return false;
            }
            // None types are equal even if types aren't
            return (is_none() && rhs.is_none()) || (this->value() == rhs.unwrap());
        }

        template <class U>
        bool
        operator==(const Option<U> &rhs) const
        {
            return (is_none() && rhs.is_none()) || false; // Not value-comparable so not equal
        }

        /// @brief Returns `true` if the option is a `Some` value.
        [[nodiscard("if you intended to assert that this has a value, consider "
                    "`.unwrap()` instead")]] inline bool
        is_some() const
        {
            // Can't be some if this contains the monostate
            return !std::is_same<T, std::monostate>::value && this->has_value();
        }

        bool
        is_some_and(const std::function<bool(T)> &f) const
        {
            return is_some() && f(this->value());
        }

        /// @brief Returns `true` if the option is a `None` value.
        [[nodiscard(
            "if you intended to assert that this doesn't have a value, consider \
                  `.and_then(|_| panic(\"`Option` had a value when expected `None`\"))` instead")]] inline bool
        is_none() const
        {
            return !is_some();
        }

        /// Returns the contained `Some` value.
        ///
        /// ## Panics
        /// Panics if the self value equals `None` with a custom panic message
        /// provided by `msg`.
        ///
        /// ## Examples
        /// ```cpp
        /// Option<uint32_t> x(71);
        /// assert(x.expect("Not a number") == 71);
        ///
        /// Option<uint32_t> y;
        /// y.expect("Not a number"); // panics with `Not a number`
        /// ```
        inline T
        expect(const std::string &msg, const std::source_location _loc = std::source_location::current()) const
        {
            if (is_some())
            {
                return this->value();
            }
            __panic_impl(_loc, "{}", msg);
        }

        /// Returns the contained `Some` value.
        ///
        /// Because this function may panic, its use is generally discouraged.
        /// Instead, call `unwrap_or`, `unwrap_or_else`, or `unwrap_or_default`.
        ///
        /// ## Panics
        /// Panics if the self value equals `None`.
        ///
        /// ## Examples
        /// ```cpp
        /// Option<uint32_t> x(71);
        /// assert(x.unwrap() == 71);
        ///
        /// Option<uint32_t> y;
        /// assert(y.unwrap() == 71); // fails
        /// ```
        inline T
        unwrap(const std::source_location _loc = std::source_location::current()) const
        {
            if (is_some())
            {
                return this->value();
            }
            __panic_impl(_loc, "called `Option::unwrap()` on a `None` value");
        }

        /// Returns the contained `Some` value or the provided default `def`.
        ///
        /// ## Examples
        /// ```cpp
        /// assert(Some("car").unwrap_or("bike") == "car");
        /// assert(None().unwrap_or("bike") == "bike");
        /// ```
        inline T
        unwrap_or(T def) const
        {
            return (is_some() ? this->value() : def);
        }

        template <class U>
        U unwrap_or(U def) const;

        /// Returns the contained `Some` value or computes it from a closure.
        ///
        /// ## Examples
        /// ```cpp
        /// uint16_t k = 10;
        /// assert(Some(4).unwrap_or_else(|| 2 * k) == 4);
        /// assert(None().unwrap_or_else<uint16_t>(|| 2 * k) == 20);
        /// ```
        inline T
        unwrap_or_else(const std::function<T()> &f) const
        {
            return (is_some() ? this->value() : f());
        }

        template <class U>
        inline U unwrap_or_else(const std::function<U()> &f) const;

        /// Returns the contained `Some` value or a default.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = None<int>();
        /// auto y = Some(12);
        ///
        /// assert(x.unwrap_or_default() == 0);
        /// assert(y.unwrap_or_default() == 12);
        /// ```
        inline T
        unwrap_or_default() const
            requires std::default_initializable<T>
        {
            return (is_some() ? this->value() : T{});
        }

        /// Maps an `Option<T>` to `Option<U>` by applying a function to a contained
        /// value (if `Some`) or returns `None` (if `None`).
        ///
        /// ## Examples
        /// ```cpp
        /// auto maybe_some_string = Some(std::string("Hello, World!"));
        /// auto maybe_some_len = maybe_some_string.map<size_t>([](auto s) { return
        /// s.size(); }); assert(maybe_some_len == Some(13));
        ///
        /// auto x = None().map<size_t>([](auto s){ return s.size(); })
        /// assert(x.map(|s| s.len()) == None());
        /// ```
        template <class U>
        inline Option<U>
        map(const std::function<U(T)> &f) const
        {
            return (is_none() ? Option<U>() : Option<U>(f(this->value())));
        }

        template <class R, class U>
        inline Option<U> map(const std::function<U(R)> &f) const;

        /// Returns the provided default result (if none),
        /// or applies a function to the contained value (if any).
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Some(std::string("foo"));
        /// assert(x.map_or<size_t>(42, [](auto v) { return v.size(); }) == 3);
        ///
        /// auto y = None();
        /// assert(y.map_or<size_t>(42, [](auto v) { return v.size(); }) == 42);
        /// ```
        template <class U>
        inline U
        map_or(U def, const std::function<U(T)> &f) const
        {
            return (is_none() ? def : f(this->value()));
        }

        template <class R, class U>
        inline U map_or(U def, const std::function<U(R)> &f) const;

        /// Computes a default function result (if none), or
        /// applies a different function to the contained value (if any).
        ///
        /// ## Examples
        /// ```cpp
        /// size_t k = 21;
        ///
        /// auto x = Some(std::string("foo"));
        /// assert(x.map_or_else<size_t>([&]() { return 2 * k; }, [](auto v) { return
        /// v.size(); }) == 3);
        ///
        /// auto y = None();
        /// assert(y.map_or_else<size_t>([&]() { return 2 * k; }, [](auto v) { return
        /// v.size(); }) == 42);
        /// ```
        template <class U>
        inline U
        map_or_else(const std::function<U()> &def,
                    const std::function<U(T)> &f) const
        {
            return (is_none() ? def() : f(this->value()));
        }

        template <class R, class U>
        inline U map_or_else(const std::function<U()> &def,
                             const std::function<U(R)> &f) const;

        /// Transforms the `Option<T>` into a `Result<T, E>`, mapping `Some(v)` to
        /// `Ok(v)` and `None` to `Err(err)`.
        ///
        /// Arguments passed to `ok_or` are eagerly evaluated; if you are passing the
        /// result of a function call, it is recommended to use `ok_or_else`, which
        /// is lazily evaluated.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Some("foo");
        /// assert(x.ok_or(0) == Ok("foo"));
        ///
        /// auto y = None();
        /// assert(x.ok_or(0) == Err(0));
        /// ```
        template <class E>
        inline Result<T, E>
        ok_or(E err)
        {
            return (is_none() ? Result<T, E>(err) : Result<T, E>(this->value()));
        }

        /// Transforms the `Option<T>` into a `Result<T, E>`, mapping `Some(v)` to
        /// `Ok(v)` and `None` to `Err(err())`.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Some("foo");
        /// assert(x.ok_or_else<int>([](){ return 0; }) == Ok("foo"));
        ///
        /// auto y = None();
        /// assert(x.ok_or_else<int>(x.ok_or_else([](){ return 0; }) == Err(0));
        /// ```
        template <class E>
        inline Result<T, E>
        ok_or_else(const std::function<E()> &err)
        {
            return (is_none() ? Result<T, E>(err())
                              : Result<T, E>(this->value()));
        }

        /// Returns `None` if the option is `None`, otherwise returns `optb`.
        ///
        /// Arguments passed to `and` are eagerly evaluated; if you are passing the
        /// result of a function call, it is recommended to use `and_then`, which is
        /// lazily evaluated.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Some(2);
        /// auto y = None();
        /// assert(x.and_b(y) == None());
        ///
        /// auto x = None();
        /// auto y = Some("foo");
        /// assert(x.and_b(y) == None());
        ///
        /// auto x = Some(2);
        /// auto y = Some("foo");
        /// assert(x.and_b(y) == Some("foo"));
        ///
        /// auto x = None();
        /// auto y = None();
        /// assert(x.and_b(y) == None());
        /// ```
        template <class U>
        inline Option<U>
        and_b(Option<U> optb) const
        {
            return (is_none() ? Option<U>() : optb);
        }

        /// Returns `None` if the option is `None`, otherwise calls `f` with the
        /// wrapped value and returns the result.
        ///
        /// Some languages call this operation flatmap.
        ///
        /// ## Examples
        /// ```cpp
        /// #include <cmath>
        ///
        /// std::function<Option<std::string>(uint32_t)>
        ///     sq_then_to_string = [](uint32_t x)
        /// {
        ///     return (x > sqrt(UINT32_MAX)) ? None() : Some(std::to_string(x * x));
        /// };
        ///
        /// assert(Some((uint32_t)2).and_then(sq_then_to_string) == Some("4"));
        /// assert(Some((uint32_t)1'000'000).and_then(sq_then_to_string) == None());
        /// // overflowed! assert(None().and_then(sq_then_to_string) == None());
        /// ```
        template <class U>
        inline Option<U>
        and_then(const std::function<Option<U>(T)> &f) const
        {
            return (is_none() ? Option<U>() : f(this->value()));
        }

        template <class R, class U>
        inline Option<U> and_then(const std::function<Option<U>(R)> &f) const;

        /// Returns `None` if the option is `None`, otherwise calls `predicate`
        /// with the wrapped value and returns:
        ///
        /// - `Some(t)` if `predicate` returns `true` (where `t` is the wrapped
        /// value), and
        /// - `None` if `predicate` returns `false`.
        ///
        /// ## Examples
        /// ```cpp
        /// std::function<bool(uint32_t)>
        ///     is_even = [](uint32_t x)
        /// {
        ///     return x % 2 == 0;
        /// };
        ///
        // EXPECT_EQ(None().filter(is_even), None());
        // EXPECT_EQ(Some(3).filter(is_even), None());
        // EXPECT_EQ(Some(4).filter(is_even), Some(4));
        /// ```
        template <class R>
            requires std::convertible_to<T, R>
        inline Option<R>
        filter(const std::function<bool(R)> &predicate) const
        {
            if (is_some() && predicate((R)this->value()))
            {
                return Option<R>((R)this->value());
            }
            else
            {
                return Option<R>();
            }
        }

        template <class R>
        inline Option<R> filter(const std::function<bool(R)> &predicate) const;

        /// Returns the option if it contains a value, otherwise returns `optb`.
        ///
        /// Arguments passed to `or` are eagerly evaluated; if you are passing the
        /// result of a function call, it is recommended to use `or_else`, which is
        /// lazily evaluated.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Some(2);
        /// auto y = None();
        /// assert(x.or_b(y) == Some(2));
        ///
        /// auto x = None();
        /// auto y = Some(100);
        /// assert(x.or_b(y) == Some(100));
        ///
        /// auto x = Some(2);
        /// auto y = Some(100);
        /// assert(x.or_b(y) == Some(2));
        ///
        /// auto x = None<uint32_t>();
        /// auto y = None();
        /// assert(x.or_b(y) == None);
        /// ```
        inline Option<T>
        or_b(Option<T> optb) const
        {
            return (is_some() ? *this : optb);
        }

        /// Returns the option if it contains a value, otherwise returns `optb`.
        ///
        /// Arguments passed to `or` are eagerly evaluated; if you are passing the
        /// result of a function call, it is recommended to use `or_else`, which is
        /// lazily evaluated.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Some(2);
        /// auto y = None();
        /// assert(x.or_b(y) == Some(2));
        ///
        /// auto x = None();
        /// auto y = Some(100);
        /// assert(x.or_b(y) == Some(100));
        ///
        /// auto x = Some(2);
        /// auto y = Some(100);
        /// assert(x.or_b(y) == Some(2));
        ///
        /// auto x = None<uint32_t>();
        /// auto y = None();
        /// assert(x.or_b(y) == None);
        /// ```
        inline Option<T>
        or_b(Option<std::monostate> optb) const
            requires(!std::same_as<T, std::monostate>)
        {
            return (is_some() ? *this : Option<T>()); // None type conversion
        }

        template <class U>
        inline Option<U> or_b(Option<U> optb) const;

        /// Returns the option if it contains a value, otherwise calls `f` and
        /// returns the result.
        ///
        /// ## Examples
        /// ```cpp
        /// std::function<Option<const char*>()> nobody = []() { return None(); };
        /// std::function<Option<const char*>()> vikings = []() { return
        /// Some("Vikings"); };
        ///
        /// assert(Some("barbarians").or_else(vikings) == Some("barbarians"));
        /// assert(None().or_else(vikings) == Some("vikings"));
        /// assert(None().or_else(nobody) == None());
        /// ```
        inline Option<T>
        or_else(const std::function<Option<T>()> &f) const
        {
            return (is_some() ? *this : f());
        }

        template <class U>
        inline Option<U> or_else(const std::function<Option<U>()> &f) const;

        /// Returns `Some` if exactly one of `this`, `optb` is `Some`, otherwise
        /// returns `None`.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Some(2);
        /// auto y = None<uint32_t>();
        /// assert(x.xor_b(y) == Some(2));
        ///
        /// auto x = None<uint32_t>();
        /// auto y = Some(2);
        /// assert(x.xor_b(y) == Some(2));
        ///
        /// auto x = Some(2);
        /// auto y = Some(2);
        /// assert(x.xor_b(y) == None());
        ///
        /// auto x = None<uint32_t>();
        /// auto y = None<uint32_t>();
        /// assert(x.xor_b(y) == None());
        /// ```
        inline Option<T>
        xor_b(Option<T> optb) const
        {
            if (is_some() && optb.is_none())
            {
                return *this;
            }
            else if (is_none() && optb.is_some())
            {
                return optb;
            }
            else
            {
                return Option<T>();
            }
        }

        /// Returns `Some` if exactly one of `this`, `optb` is `Some`, otherwise
        /// returns `None`.
        ///
        /// ## Examples
        /// ```cpp
        /// auto x = Some(2);
        /// auto y = None<uint32_t>();
        /// assert(x.xor_b(y) == Some(2));
        ///
        /// auto x = None<uint32_t>();
        /// auto y = Some(2);
        /// assert(x.xor_b(y) == Some(2));
        ///
        /// auto x = Some(2);
        /// auto y = Some(2);
        /// assert(x.xor_b(y) == None());
        ///
        /// auto x = None<uint32_t>();
        /// auto y = None<uint32_t>();
        /// assert(x.xor_b(y) == None());
        /// ```
        inline Option<T>
        xor_b(Option<std::monostate> optb) const
            requires(!std::same_as<T, std::monostate>)
        {
            return (is_some() ? *this : Option<T>());
        }

        template <class U>
        inline Option<U> xor_b(Option<U> optb) const;
    };

    /// Returns the contained `Some` value or the provided default `def`.
    ///
    /// ## Examples
    /// ```cpp
    /// assert(Some("car").unwrap_or("bike") == "car");
    /// assert(None().unwrap_or("bike") == "bike");
    /// ```
    template <>
    template <class U>
    inline U
    Option<std::monostate>::unwrap_or(U def) const // Explicit None
    {
        return def;
    }

    /// Returns the contained `Some` value or computes it from a closure.
    ///
    /// ## Examples
    /// ```cpp
    /// uint16_t k = 10;
    /// assert(Some(4).unwrap_or_else(|| 2 * k) == 4);
    /// assert(None().unwrap_or_else<uint16_t>(|| 2 * k) == 20);
    /// ```
    template <>
    template <class U>
    inline U
    Option<std::monostate>::unwrap_or_else(
        const std::function<U()> &f) const // Explicit None
    {
        return f();
    }

    /// Maps an `Option<T>` to `Option<U>` by applying a function to a contained
    /// value (if `Some`) or returns `None` (if `None`).
    ///
    /// ## Examples
    /// ```cpp
    /// auto maybe_some_string = Some(std::string("Hello, World!"));
    /// auto maybe_some_len = maybe_some_string.map<size_t>([](auto s) { return
    /// s.size(); }); assert(maybe_some_len == Some(13));
    ///
    /// auto x = None().map<size_t>([](auto s){ return s.size(); })
    /// assert(x.map(|s| s.len()) == None());
    /// ```
    template <>
    template <class R, class U>
    inline Option<U>
    Option<std::monostate>::map(
        [[maybe_unused]] const std::function<U(R)> &f) const // Explicit None
    {
        return Option<U>();
    }

    /// Returns the provided default result (if none),
    /// or applies a function to the contained value (if any).
    ///
    /// ## Examples
    /// ```cpp
    /// auto x = Some(std::string("foo"));
    /// assert(x.map_or<size_t>(42, [](auto v) { return v.size(); }) == 3);
    ///
    /// auto y = None();
    /// assert(y.map_or<size_t>(42, [](auto v) { return v.size(); }) == 42);
    /// ```
    template <>
    template <class R, class U>
    inline U
    Option<std::monostate>::map_or(
        U def,
        [[maybe_unused]] const std::function<U(R)> &f) const // Explicit None
    {
        return def;
    }

    /// Computes a default function result (if none), or
    /// applies a different function to the contained value (if any).
    ///
    /// ## Examples
    /// ```cpp
    /// size_t k = 21;
    ///
    /// auto x = Some(std::string("foo"));
    /// assert(x.map_or_else<size_t>([&]() { return 2 * k; }, [](auto v) { return
    /// v.size(); }) == 3);
    ///
    /// auto y = None();
    /// assert(y.map_or_else<size_t>([&]() { return 2 * k; }, [](auto v) { return
    /// v.size(); }) == 42);
    /// ```
    template <>
    template <class R, class U>
    inline U
    Option<std::monostate>::map_or_else(
        const std::function<U()> &def,
        [[maybe_unused]] const std::function<U(R)> &f) const // Explicit None
    {
        return def();
    }

    /// Returns `None` if the option is `None`, otherwise calls `f` with the
    /// wrapped value and returns the result.
    ///
    /// Some languages call this operation flatmap.
    ///
    /// ## Examples
    /// ```cpp
    /// #include <cmath>
    ///
    /// std::function<Option<std::string>(uint32_t)>
    ///     sq_then_to_string = [](uint32_t x)
    /// {
    ///     return (x > sqrt(UINT32_MAX)) ? None() : Some(std::to_string(x * x));
    /// };
    ///
    /// assert(Some((uint32_t)2).and_then(sq_then_to_string) == Some("4"));
    /// assert(Some((uint32_t)1'000'000).and_then(sq_then_to_string) == None()); //
    /// overflowed! assert(None().and_then(sq_then_to_string) == None());
    /// ```
    template <>
    template <class R, class U>
    inline Option<U>
    Option<std::monostate>::and_then(const std::function<Option<U>(R)> &f) const
    {
        return Option<U>();
    }

    /// Returns `None` if the option is `None`, otherwise calls `predicate`
    /// with the wrapped value and returns:
    ///
    /// - `Some(t)` if `predicate` returns `true` (where `t` is the wrapped value),
    /// and
    /// - `None` if `predicate` returns `false`.
    ///
    /// ## Examples
    /// ```cpp
    /// std::function<bool(uint32_t)>
    ///     is_even = [](uint32_t x)
    /// {
    ///     return x % 2 == 0;
    /// };
    ///
    // EXPECT_EQ(None().filter(is_even), None());
    // EXPECT_EQ(Some(3).filter(is_even), None());
    // EXPECT_EQ(Some(4).filter(is_even), Some(4));
    /// ```
    template <>
    template <class R>
    inline Option<R>
    Option<std::monostate>::filter(const std::function<bool(R)> &predicate) const
    {
        return Option<std::monostate>();
    }

    /// Returns the option if it contains a value, otherwise returns `optb`.
    ///
    /// Arguments passed to `or` are eagerly evaluated; if you are passing the
    /// result of a function call, it is recommended to use `or_else`, which is
    /// lazily evaluated.
    ///
    /// ## Examples
    /// ```cpp
    /// auto x = Some(2);
    /// auto y = None();
    /// assert(x.or_b(y) == Some(2));
    ///
    /// auto x = None();
    /// auto y = Some(100);
    /// assert(x.or_b(y) == Some(100));
    ///
    /// auto x = Some(2);
    /// auto y = Some(100);
    /// assert(x.or_b(y) == Some(2));
    ///
    /// auto x = None<uint32_t>();
    /// auto y = None();
    /// assert(x.or_b(y) == None);
    /// ```
    template <>
    template <class U>
    inline Option<U>
    Option<std::monostate>::or_b(Option<U> optb) const
    {
        return optb;
    }

    /// Returns the option if it contains a value, otherwise calls `f` and
    /// returns the result.
    ///
    /// ## Examples
    /// ```cpp
    /// std::function<Option<const char*>()> nobody = []() { return None(); };
    /// std::function<Option<const char*>()> vikings = []() { return
    /// Some("Vikings"); };
    ///
    /// assert(Some("barbarians").or_else(vikings) == Some("barbarians"));
    /// assert(None().or_else(vikings) == Some("vikings"));
    /// assert(None().or_else(nobody) == None());
    /// ```
    template <>
    template <class U>
    inline Option<U>
    Option<std::monostate>::or_else(const std::function<Option<U>()> &f) const
    {
        return f();
    }

    /// Returns `Some` if exactly one of `this`, `optb` is `Some`, otherwise
    /// returns `None`.
    ///
    /// ## Examples
    /// ```cpp
    /// auto x = Some(2);
    /// auto y = None<uint32_t>();
    /// assert(x.xor_b(y) == Some(2));
    ///
    /// auto x = None<uint32_t>();
    /// auto y = Some(2);
    /// assert(x.xor_b(y) == Some(2));
    ///
    /// auto x = Some(2);
    /// auto y = Some(2);
    /// assert(x.xor_b(y) == None());
    ///
    /// auto x = None<uint32_t>();
    /// auto y = None<uint32_t>();
    /// assert(x.xor_b(y) == None());
    /// ```
    template <>
    template <class U>
    inline Option<U>
    Option<std::monostate>::xor_b(Option<U> optb) const
    {
        return (optb.is_some() ? optb : Option<U>());
    }

    /// Construct an `Option` with a contained `Some` value
    ///
    /// ## Examples
    /// ```cpp
    /// auto x = Some("hello");
    /// ```
    template <class T>
    inline static Option<T>
    Some(T t)
    {
        return Option(t);
    }

    /// Construct an `Option` with a contained `None` value
    ///
    /// ## Examples
    /// ```cpp
    /// auto x = None();
    /// Option<std::string> y = None();
    /// ```
    template <class T = std::monostate>
    inline static Option<T>
    None()
    {
        return Option<T>();
    }
}