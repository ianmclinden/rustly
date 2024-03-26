#include <gtest/gtest.h>
#include <rustly/display.h>

using namespace rustly;

class Foo
{
public:
    Foo(const std::string &data) : mData(data) {}
    const std::string to_string() noexcept { return mData; } // ToString<Foo>

protected:
    std::string mData;
};

class Bar : public Foo
{
public:
    Bar(const std::string &data) : Foo(data) {}
    friend std::ostream &operator<<(std::ostream &out, const Bar &rhs)
    {
        out << "Bar: " << rhs.mData;
        return out;
    }
};

class Baz
{
public:
    Baz() {}
    const std::string to_string() noexcept { return std::string("baz"); } // ToString<Baz>
};

class Bam
{
public:
    Bam(const std::string &data) : mData(data) {}

private:
    std::string mData;
};

TEST(Display, ToString)
{
    EXPECT_TRUE(ToString<int>);          // Native types should be derived
    EXPECT_TRUE(ToString<uint32_t>);     // Native types should be derived
    EXPECT_TRUE(ToString<const char *>); // Convertible types should be derived
    EXPECT_TRUE(ToString<Foo>);          // Explicitly derived
    EXPECT_TRUE(ToString<Bar>);          // Inherited derive
    EXPECT_TRUE(ToString<Baz>);          // Static impl
    EXPECT_FALSE(ToString<Bam>);         // Not derived

    Foo foo("hello");

    std::string test;
    EXPECT_NO_THROW(test = foo.to_string());
    EXPECT_EQ(test, "hello");
    EXPECT_NO_THROW(test = std::to_string(foo)); // This is derived by ToString
    EXPECT_EQ(test, "hello");

    Bar bar("goodbye");

    EXPECT_NO_THROW(test = bar.to_string());
    EXPECT_EQ(test, "goodbye");
    EXPECT_NO_THROW(test = std::to_string(bar)); // This is derived by ToString
    EXPECT_EQ(test, "goodbye");

    Baz baz;

    EXPECT_NO_THROW(test = baz.to_string());
    EXPECT_EQ(test, "baz");
    EXPECT_NO_THROW(test = std::to_string(baz)); // This is derived by ToString
    EXPECT_EQ(test, "baz");
}

TEST(Display, Display)
{
    EXPECT_TRUE(Display<int>);      // Native types should be derived
    EXPECT_TRUE(Display<uint32_t>); // Native types should be derived
    EXPECT_TRUE(Display<Foo>);      // Explicitly derived
    EXPECT_TRUE(Display<Bar>);      // Inherited derive
    EXPECT_TRUE(Display<Baz>);      // Statically derived
    EXPECT_FALSE(Display<Bam>);     // Not derived

    std::ostringstream oss;
    EXPECT_NO_THROW(oss << (int)17);
    EXPECT_EQ(oss.str(), "17");

    oss.str("");
    oss.clear();
    EXPECT_NO_THROW(oss << (uint32_t)17);
    EXPECT_EQ(oss.str(), "17");

    // // FIXME: Auto derived implementation for ToString
    // oss.str("");
    // oss.clear();
    // Foo foo("hello");
    // EXPECT_NO_THROW(oss << foo);
    // EXPECT_EQ(oss.str(), "hello");

    oss.str("");
    oss.clear();
    Bar bar("hello");
    EXPECT_NO_THROW(oss << bar);
    EXPECT_EQ(oss.str(), "Bar: hello");
}