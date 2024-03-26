<h1 align="center"><code>rustly</code></h1>

<p align="center">
    <a href="https://github.com/ianmclinden/rustly/actions/workflows/rust.yml?query=branch%3Amain+" title="Build status">
        <img src="https://github.com/ianmclinden/rustly/actions/workflows/ci.yaml/badge.svg?branch=main">
    </a>
    <a href="https://github.com/ianmclinden/rustly/releases/latest" title="GitHub release">
        <img src="https://img.shields.io/github/release/ianmclinden/rustly.svg">
    </a>
    <a href="https://opensource.org/licenses/MIT" title="License: MIT">
        <img src="https://img.shields.io/badge/License-MIT-blue.svg">
    </a>
</p>

## About
*A header-only C++20 library that provides some rust-like functionality*

## Usage

Include rustly as part of an existing CMake project with
```cmake
# Add rustly as a subdirectory
add_subdirectory(rustly)

# Or, using an installed rustly (e.g. /usr/lib/cmake/rustly)
find_package(rustly REQUIRED)

# Or, with CMake FetchContent
include(FetchContent)
FetchContent_Declare(
    rustly
    URL https://github.com/ianmclinden/rustly/archive/refs/tags/v0.1.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(rustly)
```

## Samples

Some basic examles are provided, below. See each header or the included [tests](/tests/) for more detail and sample usage.

### [`Option<T>`](include/rustly/option.h)
```cpp
using namespace rustly;

assert(Some(17).is_some());
assert(Some(17).expect("has no value") == 17);
assert(Some("foo").unwrap() == "foo");
assert(Some(17).unwrap_or(42) == 17);
assert(Some("foo").unwrap_or_default() == "foo");
assert(Some(17).ok_or("no value") == Ok<int, const char *>(17)); 

assert(None().is_none());
assert(None().expect("has no value") == 17); // Panics with "has no value"
None().unwrap(); // Panics
assert(None().unwrap_or(42) == 42);
assert(None<int>().unwrap_or_default() == 0); // deafault for int
assert(None<int>().ok_or("no value") == Err<int, const char *>("no value")); 

```

### [`Result<T, E>`](include/rustly/result.h)
```cpp
using namespace rustly;

auto x = Ok<int, char *>(17);
assert(x.is_ok());
assert(x.is_ok_and([](auto x){ return x > 2; }));
assert(x.unwrap() == 17);
assert(x.unwrap_or(42) == 17);
x.unwrap_err(); // Panics
assert(x.expect("no value") == 17);
x.unwrap_err("wasn't an error"); // Panics with "wasn't an error"
assert(x.ok() == Some(42));
assert(x.err() == None());

auto x = Err<int, char *>("something happened");
assert(x.is_err());
assert(x.is_err_and([](auto e){ return e == "something happened"; }));
x.unwrap(); // Panics
assert(x.unwrap_or(42) == 42);
assert(x.unwrap_err() == "something happened");
x.expect("no value"); // Panics with "no value"
assert(x.unwrap_err("wasn't a value") == "something happened");
assert(x.ok() == None());
assert(x.err() == Some("something happened"))
```

## Concepts

[`ToString`](/include/rustly/display.h) provides a concept for classes that can be converted to string, either natively or via an implementation
```cpp
using namespace rustly;

// Native types should be auto-derived
static_assert(ToString<int>);

class Foo
{
public:
    Foo(const std::string &data) : mData(data) {}
    const std::string to_string() noexcept { return mData; } // ToString implementation

private:
    std::string mData;
};

// Foo implements ToString manually
static_assert(ToString<Foo>);
Foo foo("Hello, world!");
 // std::to_string(T) is derived for each ToString<T>
assert(foo.to_string() == std::to_string(foo) );
```

[`Display`](/include/rustly/display.h) provides a concept for classes that can be streamed to a `std::ostream`.
Requires that a class also implement `ToString`
```cpp
using namespace rustly;

// Native types should be auto-derived
static_assert(Display<int>);

class Bar
{
public:
    Bar(const std::string &data) : mData(data) {}
    const std::string to_string() noexcept { return mData; } // ToString implementation
    friend std::ostream &operator<<(std::ostream &out, const Bar &rhs) // Display implementation
    {
        out << "Bar: " << rhs.mData;
        return out;
    }
private:
    std::string mData;
};

// Bar implements Display manually
static_assert(Display<Bar>);
std::cout << Bar("baz") << std::endl; // Prints "Bar: baz"
```


### Macros
```cpp
// Panics with
//  | panicked at path/to/example.cpp:8
/// | unexpected error
panic("unexpected error");

// Panics with
//  | panicked at path/to/example.cpp:8
/// | error 16: a description
panic("error {}: {}", 16, "a description");
```