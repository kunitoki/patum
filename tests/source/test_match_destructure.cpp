/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#include "test_patum_common.h"

TEST_CASE("Simple matcher destructure tuple", "[match][destructure]")
{
    auto x = std::make_tuple(1337, "123"s);

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(ds(1338, _))            = [&] { matched_pattern = 1; },
            pattern(ds(_x <= 1338, "1234")) = [&] { matched_pattern = 2; },
            pattern(_)                      = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 3);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(ds(_, "1234"))         = [&] { matched_pattern = 1; },
            pattern(ds(_x <= 1338, "123")) = [&] { matched_pattern = 2; },
            pattern(_)                     = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 2);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(ds(_x < 1337))  = [&] { matched_pattern = 1; },
            pattern(ds(_x > 1337))  = [&] { matched_pattern = 2; },
            pattern(ds(_x != 1337)) = [&] { matched_pattern = 3; },
            pattern(ds(_x == 1337)) = [&] { matched_pattern = 4; },
            pattern(_)              = [&] { matched_pattern = 5; }
        );

        CHECK(matched_pattern == 4);
    }

    {
        auto matched_pattern = match(x)
        (
            pattern(ds(_, _)) = [](int value, const std::string& text) {
                return value + static_cast<int>(text.size());
            }
        );

        CHECK(matched_pattern.value_or(0) == 1340);
    }

    {
        match(x)
        (
            pattern(ds(_, _)) = [](int& value, std::string& text) {
                value = 42;
                text = "changed";
            }
        );

        CHECK(std::get<0>(x) == 42);
        CHECK(std::get<1>(x) == "changed");
    }
}

//=================================================================================================

namespace {
struct aggregate
{
    int x = 0;
    float y = 1000.0f;
    char z = 'a';
};

struct aggregate_0
{
};

struct aggregate_4
{
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
};

struct aggregate_5
{
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;
};

struct aggregate_6
{
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;
    int f = 6;
};

struct aggregate_7
{
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;
    int f = 6;
    int g = 7;
};

struct aggregate_8
{
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;
    int f = 6;
    int g = 7;
    int h = 8;
};
} // namespace

TEST_CASE("Simple matcher destructure struct", "[match][destructure]")
{
    auto x = aggregate{ 1337, 42.0f, 'b' };

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(ds(1338, _, 'a'))                         = [&] { matched_pattern = 1; },
            pattern(ds(_x <= 1338, _y >= 0.0f, in('a', 'b'))) = [&] { matched_pattern = 2; },
            pattern(_)                                        = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 2);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(ds(_x < 1337))  = [&] { matched_pattern = 1; },
            pattern(ds(_x > 1337))  = [&] { matched_pattern = 2; },
            pattern(ds(_x != 1337)) = [&] { matched_pattern = 3; },
            pattern(ds(_x == 1337)) = [&] { matched_pattern = 4; },
            pattern(_)              = [&] { matched_pattern = 5; }
        );

        CHECK(matched_pattern == 4);
    }

    {
        auto matched_pattern = match(x)
        (
            pattern(ds(_, _, _)) = [](int value, float amount, char tag) {
                return value + static_cast<int>(amount) + tag;
            }
        );

        CHECK(matched_pattern.value_or(0) == 1477);
    }

    {
        match(x)
        (
            pattern(ds(_, _, _)) = [](int& value, float& amount, char& tag) {
                value = 1;
                amount = 2.0f;
                tag = 'c';
            }
        );

        CHECK(x.x == 1);
        CHECK(x.y == 2.0f);
        CHECK(x.z == 'c');
    }
}

//=================================================================================================

TEST_CASE("Simple matcher destructure wider structs", "[match][destructure]")
{
    CHECK(match(aggregate_0{})
    (
        pattern(ds()) = 1,
        pattern(_)    = 2
    ).value_or(0) == 1);

    CHECK(match(aggregate_4{})
    (
        pattern(ds(1, 2, 3, 4)) = 1,
        pattern(_)              = 2
    ).value_or(0) == 1);

    CHECK(match(aggregate_5{})
    (
        pattern(ds(1, 2, 3, 4, 5)) = 1,
        pattern(_)                 = 2
    ).value_or(0) == 1);

    CHECK(match(aggregate_6{})
    (
        pattern(ds(1, 2, 3, 4, 5, 6)) = 1,
        pattern(_)                    = 2
    ).value_or(0) == 1);

    CHECK(match(aggregate_7{})
    (
        pattern(ds(1, 2, 3, 4, 5, 6, 7)) = 1,
        pattern(_)                       = 2
    ).value_or(0) == 1);

    CHECK(match(aggregate_8{})
    (
        pattern(ds(1, 2, 3, 4, 5, 6, 7, 8)) = 1,
        pattern(_)                          = 2
    ).value_or(0) == 1);
}

//=================================================================================================

TEST_CASE("Simple matcher destructure wider struct captures", "[match][destructure]")
{
    {
        auto captures = ds().captures(aggregate_0{});
        static_assert(std::tuple_size_v<decltype(captures)> == 0);
    }

    {
        auto x = aggregate_4{};
        auto captures = ds(_, _, _, _).captures(x);
        CHECK(&std::get<0>(captures) == &x.a);
        CHECK(&std::get<3>(captures) == &x.d);
    }

    {
        auto x = aggregate_5{};
        auto captures = ds(_, _, _, _, _).captures(x);
        CHECK(&std::get<0>(captures) == &x.a);
        CHECK(&std::get<4>(captures) == &x.e);
    }

    {
        auto x = aggregate_6{};
        auto captures = ds(_, _, _, _, _, _).captures(x);
        CHECK(&std::get<0>(captures) == &x.a);
        CHECK(&std::get<5>(captures) == &x.f);
    }

    {
        auto x = aggregate_7{};
        auto captures = ds(_, _, _, _, _, _, _).captures(x);
        CHECK(&std::get<0>(captures) == &x.a);
        CHECK(&std::get<6>(captures) == &x.g);
    }

    {
        auto x = aggregate_8{};
        auto captures = ds(_, _, _, _, _, _, _, _).captures(x);
        CHECK(&std::get<0>(captures) == &x.a);
        CHECK(&std::get<7>(captures) == &x.h);
    }
}

//=================================================================================================
