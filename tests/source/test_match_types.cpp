/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#include "test_patum_common.h"

TEST_CASE("Simple matcher typed on variant", "[match][typed]")
{
    std::variant<int, std::string> x = 11223344;

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(valued(11223344))    = [&] { matched_pattern = 1; },
            pattern(valued("11223344"s)) = [&] { matched_pattern = 2; },
            pattern(_)                   = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(valued("11223344"s)) = [&] { matched_pattern = 1; },
            pattern(valued(11223344))    = [&] { matched_pattern = 2; },
            pattern(_)                   = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 2);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(valued("0"s)) = [&] { matched_pattern = 1; },
            pattern(valued(0))    = [&] { matched_pattern = 2; },
            pattern(_)            = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 3);
    }
}

//=================================================================================================

TEST_CASE("Simple matcher is type on variant", "[match][is]")
{
    std::variant<int, std::string> x = 11223344;
    std::variant<int, std::string> y = "11223344"s;

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(typed<int>)         = [&] { matched_pattern = 1; },
            pattern(typed<std::string>) = [&] { matched_pattern = 2; },
            pattern(_)                  = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(y)
        (
            pattern(typed<char>)        = [&] { matched_pattern = 1; },
            pattern(typed<std::string>) = [&] { matched_pattern = 2; },
            pattern(_)                  = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 2);
    }

    {
        int matched_pattern = 0;

        match(y)
        (
            pattern(typed<float>)       = [&] { matched_pattern = 1; },
            pattern(typed<const char*>) = [&] { matched_pattern = 2; },
            pattern(_)                  = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 3);
    }
}

//=================================================================================================

namespace {
template <class T>
auto stringize_type(T* x = nullptr)
{
    using namespace ptm;

    return match(x)
    (
        pattern(is<char*>)             = "char",
        pattern(is<short*>)            = "short",
        pattern(is<int*>)              = "int",
        pattern(is<long*>)             = "long",
        pattern(is<long long*>)        = "long long",
        pattern(is<std::string_view*>) = "string_view",
        pattern(_)                     = "other"
    ).value_or("invalid");
}
} // namespace

TEST_CASE("Simple matcher is type on generic", "[match][is]")
{
    int x = 11223344;
    std::string_view y = "11223344"sv;
    std::string z = "11223344"s;

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(is<int>)              = [&] { matched_pattern = 1; },
            pattern(is<std::string_view>) = [&] { matched_pattern = 2; },
            pattern(_)                    = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(y)
        (
            pattern(is<char>)             = [&] { matched_pattern = 1; },
            pattern(is<std::string_view>) = [&] { matched_pattern = 2; },
            pattern(_)                    = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 2);
    }

    {
        int matched_pattern = 0;

        match(y)
        (
            pattern(is<float>)       = [&] { matched_pattern = 1; },
            pattern(is<const char*>) = [&] { matched_pattern = 2; },
            pattern(is<std::string>) = [&] { matched_pattern = 3; },
            pattern(_)               = [&] { matched_pattern = 4; }
        );

        CHECK(matched_pattern == 4);
    }

    {
        int matched_pattern = 0;

        match(z)
        (
            pattern(is<const char*>)      = [&] { matched_pattern = 1; },
            pattern(is<std::string_view>) = [&] { matched_pattern = 2; },
            pattern(is<std::string>)      = [&] { matched_pattern = 3; },
            pattern(_)                    = [&] { matched_pattern = 4; }
        );

        CHECK(matched_pattern == 3);
    }

    {
        int x = 42;

        CHECK("int"sv == stringize_type(&x));
        CHECK("int"sv == stringize_type<decltype(x)>());
    }
}

//=================================================================================================

namespace {
struct Shape { virtual ~Shape() = default; };
struct Circle : Shape { Circle(int r) : radius(r) {} int radius; };
struct Rectangle : Shape { Rectangle(int w, int h) : width(w), height(h) {} int width, height; };
} // namespace

TEST_CASE("Simple matcher is type on polymorphic", "[match][is][polymorphic]")
{
    auto get_area = [](Shape* shape)
    {
        return match(shape)
        (
            pattern(some() and is<Circle>)    = [](Circle* c) {
                return 3.14 * c->radius * c->radius;
            },
            pattern(some() and is<Rectangle>) = [](Rectangle* r) {
                return r->width * r->height;
            }
        ).value_or(0);
    };

    {
        auto shape = Rectangle{ 100, 200 };
        auto area = get_area(&shape);
        CHECK(100 * 200 == area);
    }

    {
        auto shape = Circle{ 100 };
        auto area = get_area(&shape);
        CHECK(3.14 * 100 * 100 == area);
    }
}

//=================================================================================================
