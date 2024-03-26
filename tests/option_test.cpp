#include <cmath>
#include <gtest/gtest.h>
#include <rustly/option.h>
#include <rustly/result.h>

using namespace rustly;

TEST(Option, Constructor)
{
    EXPECT_TRUE(None().is_none());
    EXPECT_TRUE(None<uint16_t>().is_none());
    EXPECT_FALSE(None().is_some());
    EXPECT_FALSE(None<uint16_t>().is_some());

    EXPECT_FALSE(Some("hiya").is_none());
    EXPECT_TRUE(Some(71).is_some());
}

TEST(Option, Equality)
{
    EXPECT_EQ(None(), None());
    EXPECT_EQ(None(), None<uint32_t>()); // Also check that typed None's are equal to None
    EXPECT_EQ(None<uint32_t>(), None());
    EXPECT_EQ(None<uint32_t>(), None<uint32_t>());

    EXPECT_EQ(Some("hiya"), Some("hiya"));
    EXPECT_NE(Some("hiya"), Some(17));
    EXPECT_NE(Some(17), Some("hiya"));
    EXPECT_NE(Some("hiya"), None());
    EXPECT_NE(None(), Some("hiya"));
    EXPECT_NE(Some("hiya"), None<const char *>());
    EXPECT_NE(None<const char *>(), Some("hiya"));
}

TEST(Option, Boolean)
{
    EXPECT_TRUE(None().is_none());
    EXPECT_FALSE(None().is_some());
    EXPECT_FALSE(None().is_some_and([](auto v)
                                    { return true; }));
    EXPECT_FALSE(None().is_some_and([](auto v)
                                    { return false; }));

    EXPECT_FALSE(Some(17).is_none());
    EXPECT_TRUE(Some(17).is_some());
    EXPECT_TRUE(Some(17).is_some_and([](auto v)
                                     { return v == 17; }));
    EXPECT_FALSE(Some(17).is_some_and([](auto v)
                                      { return v != 17; }));

    EXPECT_EQ(None<uint32_t>().and_b(None<const char *>()), None()); // Add explicit types to check compatibility
    EXPECT_EQ(None<uint32_t>().and_b(Some("foo")), None());
    EXPECT_EQ(Some(2).and_b(None<std::string>()), None());
    EXPECT_EQ(Some(2).and_b(Some("foo")), Some("foo"));

    std::function<Option<std::string>(uint32_t)>
        sq_then_to_string = [](uint32_t x)
    {
        return (x > sqrt(UINT32_MAX)) ? None() : Some(std::to_string(x * x));
    };

    EXPECT_EQ(Some((uint32_t)2).and_then(sq_then_to_string), Some("4"));
    EXPECT_EQ(Some((uint32_t)1'000'000).and_then(sq_then_to_string), None());
    EXPECT_EQ(None().and_then(sq_then_to_string), None());

    EXPECT_EQ(None<uint32_t>().or_b(None()), None()); // Add explicit types to check compatibility
    EXPECT_EQ(None().or_b(Some(100)), Some(100));
    EXPECT_EQ(Some(2).or_b(None()), Some(2));
    EXPECT_EQ(Some(2).or_b(Some(100)), Some(2));

    std::function<Option<const char *>()> nobody = []()
    { return None(); };
    std::function<Option<const char *>()> vikings = []()
    { return Some("vikings"); };

    EXPECT_EQ(Some("barbarians").or_else(vikings), Some("barbarians"));
    EXPECT_EQ(None().or_else(vikings), Some("vikings"));
    EXPECT_EQ(None().or_else(nobody), None());

    EXPECT_EQ(Some(2).xor_b(None()), Some(2));
    EXPECT_EQ(None().xor_b(Some(2)), Some(2));
    EXPECT_EQ(Some(2).xor_b(Some(2)), None());
    EXPECT_EQ(None<uint32_t>().xor_b(None<uint32_t>()), None());
}

TEST(Option, Expect)
{
    EXPECT_EXIT(None().expect("a message"), ::testing::KilledBySignal(SIGABRT), "panicked at .*\na message");
    EXPECT_EQ(Some("foo").expect("a message"), "foo");
}

TEST(Option, Unwrap)
{
    EXPECT_EXIT(None().unwrap(), ::testing::KilledBySignal(SIGABRT), "panicked at .*\ncalled `Option::unwrap\\(\\)` on a `None` value");
    EXPECT_EQ(Some("foo").unwrap(), "foo");

    EXPECT_EQ(None().unwrap_or("bar"), "bar");
    EXPECT_EQ(Some("foo").unwrap_or("bar"), "foo");

    EXPECT_EQ(None().unwrap_or_else<const char *>([]
                                                  { return "bar"; }),
              "bar");
    EXPECT_EQ(Some("foo").unwrap_or_else([]
                                         { return "bar"; }),
              "foo");

    EXPECT_EQ(None<std::string>().unwrap_or_default(), "");
    EXPECT_EQ(None<uint16_t>().unwrap_or_default(), 0);
    EXPECT_EQ(Some("foo").unwrap_or_default(), "foo");
}

TEST(Option, Map)
{
    std::function<size_t(std::string)> f = [](auto s)
    { return s.size(); };

    // Test explicit and lambda notation
    EXPECT_EQ(Some(std::string("Hello, World!")).map(f), Some(13));
    EXPECT_EQ(Some(std::string("Hello, World!")).map<size_t>([](auto s)
                                                             { return s.size(); }),
              Some(13));
    EXPECT_EQ(None().map(f), None());
    EXPECT_EQ(None<std::string>().map<size_t>([](auto s)
                                              { return s.size(); }),
              None());

    // Same for map_or
    EXPECT_EQ(Some(std::string("foo")).map_or((size_t)42, f), 3);
    EXPECT_EQ(Some(std::string("foo")).map_or<size_t>(42, [](auto s)
                                                      { return s.size(); }),
              3);
    EXPECT_EQ(None().map_or((size_t)42, f), 42);
    EXPECT_EQ(None<std::string>().map_or<size_t>(42, [](auto s)
                                                 { return s.size(); }),
              42);

    std::function<size_t()> d = []()
    { return 2 * 21; };

    // And same for map_or_else
    EXPECT_EQ(Some(std::string("foo")).map_or_else(d, f), 3);
    EXPECT_EQ(Some(std::string("foo")).map_or_else<size_t>([]() -> size_t
                                                           { return 42; },
                                                           [](auto s) -> size_t
                                                           { return s.size(); }),
              3);
    EXPECT_EQ(None().map_or_else(d, f), 42);
    EXPECT_EQ(None<std::string>().map_or_else<size_t>([]() -> size_t
                                                      { return 42; },
                                                      [](auto s) -> size_t
                                                      { return s.size(); }),
              42);
}

TEST(Option, Filter)
{
    std::function<bool(uint32_t)>
        is_even = [](uint32_t x)
    {
        return x % 2 == 0;
    };

    EXPECT_EQ(None().filter(is_even), None());
    EXPECT_EQ(Some(3).filter(is_even), None());
    EXPECT_EQ(Some(4).filter(is_even), Some(4));
}

TEST(Option, Result)
{
    EXPECT_TRUE(Some("foo").ok_or(0).is_ok());
    EXPECT_TRUE(None().ok_or(0).is_err());

    EXPECT_TRUE(Some("foo").ok_or_else<int>([]()
                                            { return 0; })
                    .is_ok());
    EXPECT_TRUE(None().ok_or_else<int>([]()
                                       { return 0; })
                    .is_err());
}

// TEST(Option, Iterator)
// {
//     std::vector<std::string> x = {};
//     Some(x).begin();
// }