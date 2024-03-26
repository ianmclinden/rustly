#include <cmath>
#include <gtest/gtest.h>
#include <rustly/result.h>

using namespace rustly;

TEST(Result, Constructor)
{
    auto x = Err<uint32_t, const char *>("Some error message");

    EXPECT_TRUE(x.is_err());
    EXPECT_FALSE(x.is_ok());

    auto y = Ok<uint32_t, const char *>(-3);
    EXPECT_TRUE(y.is_ok());
    EXPECT_FALSE(y.is_err());

    // Same type is OK too
    auto z = Ok<int, int>(7);
    EXPECT_TRUE(z.is_ok());
    EXPECT_FALSE(z.is_err());
}

TEST(Result, Equality)
{
    auto x = Err<int, const char *>("foo");
    auto y = Err<int, const char *>("bar");
    EXPECT_NE(x, y);

    x = Err<int, const char *>("foo");
    y = Err<int, const char *>("foo");
    EXPECT_EQ(x, y);

    x = Err<int, const char *>("foo");
    y = Ok<int, const char *>(13);
    EXPECT_NE(x, y);

    x = Ok<int, const char *>(27);
    y = Ok<int, const char *>(13);
    EXPECT_NE(x, y);

    x = Ok<int, const char *>(13);
    y = Ok<int, const char *>(13);
    EXPECT_EQ(x, y);
}

TEST(Result, Boolean)
{
    auto x = Err<uint32_t, const char *>("Some error message");
    auto y = Ok<uint32_t, const char *>(2);

    EXPECT_FALSE(x.is_ok());
    EXPECT_TRUE(y.is_ok());

    EXPECT_FALSE(x.is_ok_and([](auto x)
                             { return x > 2; }));
    EXPECT_FALSE(y.is_ok_and([](auto x)
                             { return x > 2; }));
    EXPECT_TRUE(y.is_ok_and([](auto x)
                            { return x == 2; }));

    EXPECT_TRUE(x.is_err());
    EXPECT_FALSE(y.is_err());

    EXPECT_FALSE(x.is_err_and([](auto x)
                              { return x == "Something else"; }));
    EXPECT_TRUE(x.is_err_and([](auto x)
                             { return x == "Some error message"; }));
    EXPECT_FALSE(y.is_err_and([](auto x)
                              { return x == "Some error message"; }));

    auto a = Ok<uint32_t, const char *>(2);
    auto b = Err<const char *, const char *>("late error");
    auto expected = Err<const char *, const char *>("late error");
    EXPECT_EQ(a.and_b(b), expected);

    a = Err<uint32_t, const char *>("early error");
    b = Ok<const char *, const char *>("foo");
    expected = Err<const char *, const char *>("early error");
    EXPECT_EQ(a.and_b(b), expected);

    a = Err<uint32_t, const char *>("not a 2");
    b = Err<const char *, const char *>("late error");
    expected = Err<const char *, const char *>("not a 2");
    EXPECT_EQ(a.and_b(b), expected);

    a = Ok<uint32_t, const char *>(2);
    b = Ok<const char *, const char *>("different result type");
    expected = Ok<const char *, const char *>("different result type");
    EXPECT_EQ(a.and_b(b), expected);

    std::function<Result<std::string, std::string>(uint32_t)>
        sq_then_to_string = [](uint32_t x)
    {
        return (x > sqrt(UINT32_MAX))
                   ? Err<std::string, std::string>("overflowed")
                   : Ok<std::string, std::string>(std::to_string(x * x));
    };

    auto c = Ok<uint32_t, std::string>(2);
    auto expected2 = Ok<std::string, std::string>(std::string("4"));
    EXPECT_EQ(c.and_then(sq_then_to_string), expected2);
    c = Ok<std::uint32_t, std::string>(1000000);
    expected2 = Err<std::string, std::string>(std::string("overflowed"));
    EXPECT_EQ(c.and_then(sq_then_to_string), expected2);
    c = Err<uint32_t, std::string>(std::string("not a number"));
    expected2 = Err<std::string, std::string>(std::string("not a number"));
    EXPECT_EQ(c.and_then(sq_then_to_string), expected2);

    x = Ok<uint32_t, const char *>(2);
    y = Err<uint32_t, const char *>("late error");
    auto expected3 = Ok<uint32_t, const char *>(2);
    EXPECT_EQ(x.or_b(y), expected3);
    x = Err<uint32_t, const char *>("early error");
    y = Ok<uint32_t, const char *>(2);
    expected3 = Ok<uint32_t, const char *>(2);
    EXPECT_EQ(x.or_b(y), expected3);
    x = Err<uint32_t, const char *>("not a 2");
    y = Err<uint32_t, const char *>("late error");
    expected3 = Err<uint32_t, const char *>("late error");
    EXPECT_EQ(x.or_b(y), expected3);
    x = Ok<uint32_t, const char *>(2);
    y = Ok<uint32_t, const char *>(100);
    expected3 = Ok<uint32_t, const char *>(2);
    EXPECT_EQ(x.or_b(y), expected3);

    std::function<Result<int, int>(int)> sq = [](auto x)
    { return Ok<int, int>(x * x); };
    std::function<Result<int, int>(int)> err = [](auto x)
    { return Err<int, int>(x); };

    auto d = Ok<int, int>(2);
    auto expected4 = Ok<int, int>(2);
    EXPECT_EQ(d.or_else(sq).or_else(sq), expected4);
    d = Ok<int, int>(2);
    expected4 = Ok<int, int>(2);
    EXPECT_EQ(d.or_else(err).or_else(sq), expected4);
    d = Err<int, int>(3);
    expected4 = Ok<int, int>(9);
    EXPECT_EQ(d.or_else(sq).or_else(err), expected4);
    d = Err<int, int>(3);
    expected4 = Err<int, int>(3);
    EXPECT_EQ(d.or_else(err).or_else(err), expected4);
}

TEST(Result, Option)
{
    auto x = Ok<uint32_t, const char *>(2);
    auto y = Err<uint32_t, const char *>("Nothing here");

    EXPECT_EQ(x.ok(), Some(2));
    EXPECT_EQ(y.ok(), None());

    EXPECT_EQ(x.err(), None());
    EXPECT_EQ(y.err(), Some("Nothing here"));
}

TEST(Result, Map)
{
    std::function<size_t(std::string)> len = [](auto s)
    { return s.size(); };
    std::function<size_t(int)> dbl = [](auto x)
    { return 2 * 21; };

    auto x = Ok<std::string, int>(std::string("Hello, World!"));
    auto y = Err<std::string, int>(-1);

    auto expected = Ok<size_t, int>(13);
    EXPECT_EQ(x.map(len), expected);
    expected = Err<size_t, int>(-1);
    EXPECT_EQ(y.map(len), expected);

    EXPECT_EQ(x.map_or((size_t)42, len), 13);
    EXPECT_EQ(y.map_or((size_t)42, len), 42);

    EXPECT_EQ(x.map_or_else(dbl, len), 13);
    EXPECT_EQ(y.map_or_else(dbl, len), 42);

    std::function<std::string(int)> stringify = [](auto x)
    { return std::string("error code: " + std::to_string(x)); };

    auto a = Ok<int, int>(2);
    auto b = Err<int, int>(13);

    auto expected2 = Ok<int, std::string>(2);
    EXPECT_EQ(a.map_err(stringify), expected2);
    expected2 = Err<int, std::string>(std::string("error code: 13"));
    EXPECT_EQ(b.map_err(stringify), expected2);
}

class FooBar
{
public:
    FooBar() {}
    bool operator==(const FooBar &) const = default;
    const std::string to_string() noexcept { return std::string("foobar"); } // ToString<FooBar>
};

TEST(Result, Expect)
{
    auto x = Err<int, int>(-1);
    auto y = Err<int, FooBar>(FooBar{});
    auto z = Ok<int, int>(1);

    EXPECT_EXIT(x.expect("a message"), ::testing::KilledBySignal(SIGABRT), "panicked at .*\na message: -1");
    EXPECT_EXIT(y.expect("another message"), ::testing::KilledBySignal(SIGABRT), "panicked at .*\nanother message: foo");
    EXPECT_EQ(z.expect("a message"), 1);

    EXPECT_EQ(x.expect_err("a message"), -1);
    EXPECT_EQ(y.expect_err("another message"), FooBar{});
    EXPECT_EXIT(z.expect_err("message"), ::testing::KilledBySignal(SIGABRT), "panicked at .*\nmessage: 1");
}

TEST(Result, Unwrap)
{
    auto x = Err<size_t, int>(17);
    auto y = Err<size_t, FooBar>(FooBar{});
    auto z = Ok<size_t, std::string>(1);

    EXPECT_EXIT(x.unwrap(), ::testing::KilledBySignal(SIGABRT), "panicked at .*\ncalled `Result::unwrap\\(\\)` on an `Err` value: 17");
    EXPECT_EXIT(y.unwrap(), ::testing::KilledBySignal(SIGABRT), "panicked at .*\ncalled `Result::unwrap\\(\\)` on an `Err` value: foo");
    EXPECT_EQ(z.unwrap(), 1);

    EXPECT_EQ(x.unwrap_or(42), 42);
    EXPECT_EQ(y.unwrap_or(42), 42);
    EXPECT_EQ(z.unwrap_or(42), 1);

    std::function<size_t(int)> f = [](auto s)
    { return 42; };
    std::function<size_t(FooBar)> g = [](auto s)
    { return 42; };
    std::function<size_t(std::string)> h = [](auto s)
    { return 42; };
    EXPECT_EQ(x.unwrap_or_else(f), 42);
    EXPECT_EQ(y.unwrap_or_else(g), 42);
    EXPECT_EQ(z.unwrap_or_else(h), 1);

    EXPECT_EQ(x.unwrap_or_default(), 0);
    EXPECT_EQ(y.unwrap_or_default(), 0);
    EXPECT_EQ(z.unwrap_or_default(), 1);

    EXPECT_EQ(x.unwrap_err(), 17);
    EXPECT_EQ(y.unwrap_err(), FooBar{});
    EXPECT_EXIT(z.unwrap_err(), ::testing::KilledBySignal(SIGABRT), "panicked at .*\ncalled `Result::unwrap_err\\(\\)` on an `Ok` value: 1");
}