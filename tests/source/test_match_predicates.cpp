/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#include "test_patum_common.h"

TEST_CASE("Simple matcher with predicates", "[match][predicates]")
{
    const int x = 1337;

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(_x == 1337) = [&] { matched_pattern = 1; },
            pattern(_)          = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(1337 == _x) = [&] { matched_pattern = 1; },
            pattern(_)          = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(_x != 1) = [&] { matched_pattern = 1; },
            pattern(_)       = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(1 != _x) = [&] { matched_pattern = 1; },
            pattern(_)       = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(_x > 10) = [&] { matched_pattern = 1; },
            pattern(_)       = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(2000 > _x) = [&] { matched_pattern = 1; },
            pattern(_)         = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(_x >= 1337) = [&] { matched_pattern = 1; },
            pattern(_)          = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(2000 >= _x) = [&] { matched_pattern = 1; },
            pattern(_)          = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(_x <= 2337) = [&] { matched_pattern = 1; },
            pattern(_)          = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(1337 <= _x) = [&] { matched_pattern = 1; },
            pattern(_)          = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(_x < 10 || range(2000, 2100)) = [&] { matched_pattern = 1; },
            pattern(_)                            = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 2);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(_x < 2000 && range(1000, 2100)) = [&] { matched_pattern = 1; },
            pattern(_)                              = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern((_x << 1) == 2674) = [&] { matched_pattern = 1; },
            pattern(_)                 = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern((_x >> 1) == 668) = [&] { matched_pattern = 1; },
            pattern(_)                = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }
}

//=================================================================================================

namespace {
template <class F>
constexpr auto is_even(const predicate<F>& m)
{
    return predicate([m](const auto& x) { return m(x) % 2 == 0; });
}
} // namespace

TEST_CASE("Simple matcher custom even & odd", "[match][predicates]")
{
    {
        int x = 1337;

        bool should_be_even = false;
        bool should_be_odd = false;

        match(x)
        (
            pattern(is_even(_x))  = [&] { should_be_even = true; },
            pattern(!is_even(_x)) = [&] { should_be_odd = true; }
        );

        CHECK(should_be_odd);
        CHECK(not should_be_even);
    }

    {
        int x = 1336;

        auto should_be_even = match(x)
        (
            pattern(is_even(_x)) = true
        );

        CHECK(should_be_even.value_or(false));
    }

    {
        int x = 1336;

        auto should_be_even = match(x)
        (
            pattern(_x % 2 == 0) = true
        );

        CHECK(should_be_even.value_or(false));
    }
}

//=================================================================================================

TEST_CASE("Simple matcher predicate arithmetic operators", "[match][predicates]")
{
    const int x = 6;

    CHECK((+_x == 6)(x));
    CHECK((-_x == -6)(x));
    CHECK((_x + 4 == 10)(x));
    CHECK((4 + _x == 10)(x));
    CHECK((_x + _x == 12)(x));
    CHECK((_x - 2 == 4)(x));
    CHECK((10 - _x == 4)(x));
    CHECK((_x - (_x / 2) == 3)(x));
    CHECK((_x * 7 == 42)(x));
    CHECK((7 * _x == 42)(x));
    CHECK((_x * _x == 36)(x));
    CHECK((_x / 3 == 2)(x));
    CHECK((36 / _x == 6)(x));
    CHECK((_x / (_x / 3) == 3)(x));
    CHECK((_x % 4 == 2)(x));
    CHECK((20 % _x == 2)(x));
    CHECK((_x % (_x - 4) == 0)(x));
}

//=================================================================================================

TEST_CASE("Simple matcher predicate bitwise operators", "[match][predicates]")
{
    const int x = 6;

    CHECK((~_x == ~6)(x));
    CHECK(((_x & 3) == 2)(x));
    CHECK(((3 & _x) == 2)(x));
    CHECK(((_x & _x) == 6)(x));
    CHECK(((_x | 1) == 7)(x));
    CHECK(((1 | _x) == 7)(x));
    CHECK(((_x | _x) == 6)(x));
    CHECK(((_x ^ 3) == 5)(x));
    CHECK(((3 ^ _x) == 5)(x));
    CHECK(((_x ^ _x) == 0)(x));
    CHECK((_x << 1 == 12)(x));
    CHECK((1 << _x == 64)(x));
    CHECK((_x << (_x - 5) == 12)(x));
    CHECK((_x >> 1 == 3)(x));
    CHECK((64 >> _x == 1)(x));
    CHECK((_x >> (_x - 5) == 3)(x));
}

//=================================================================================================
