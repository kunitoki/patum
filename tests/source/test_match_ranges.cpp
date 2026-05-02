/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#include "test_patum_common.h"

TEST_CASE("Simple matcher find in range", "[match][ranges]")
{
    std::vector<int> x = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };
    std::vector<int> y = { 1, 2 };

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(find(10) != end()) = [&] { matched_pattern = 1; },
            pattern(find(0) != end())  = [&] { matched_pattern = 2; },
            pattern(_)                 = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 2);
    }

    {
        int matched_pattern = 0;
        auto proj = [](const auto& x) { return x * x; };

        match(x)
        (
            pattern(find(10, proj) != end()) = [&] { matched_pattern = 1; },
            pattern(find(4, proj) != end())  = [&] { matched_pattern = 2; },
            pattern(_)                       = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 2);
    }

    {
        int matched_pattern = 0;

        match(y)
        (
            pattern(next(begin(), 2) == prev(end(), 2)) = [&] { matched_pattern = 1; },
            pattern(next(begin()) == prev(end()))       = [&] { matched_pattern = 2; },
            pattern(_)                                  = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 2);
    }
}

//=================================================================================================

TEST_CASE("Simple matcher sequence in range", "[match][ranges]")
{
    std::vector<int> x = { 1, 2, 3, 4 };
    std::array<int, 3> y = { 1, 2, 3 };
    std::span<const int> z = y;

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(seq(1, 2, 3))       = [&] { matched_pattern = 1; },
            pattern(seq(1, _, _x > 3))  = [&] { matched_pattern = 2; },
            pattern(seq(1, _, 3, _))    = [&] { matched_pattern = 3; },
            pattern(_)                  = [&] { matched_pattern = 4; }
        );

        CHECK(matched_pattern == 3);
    }

    {
        int matched_pattern = 0;

        match(z)
        (
            pattern(seq(1, 2, 4)) = [&] { matched_pattern = 1; },
            pattern(seq(1, _, 3)) = [&] { matched_pattern = 2; },
            pattern(_)            = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 2);
    }

    static_assert(match(std::array{ 1, 2, 3 })
    (
        pattern(seq(1, 2, 3)) = 1,
        pattern(_)            = 2
    ).value_or(0) == 1);
}

//=================================================================================================

TEST_CASE("Simple matcher contains and empty in range", "[match][ranges]")
{
    std::vector<int> x = { 1, 2, 3, 4 };
    std::vector<int> y;

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(contains(5)) = [&] { matched_pattern = 1; },
            pattern(contains(3)) = [&] { matched_pattern = 2; },
            pattern(_)           = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 2);
    }

    {
        int matched_pattern = 0;

        match(y)
        (
            pattern(non_empty()) = [&] { matched_pattern = 1; },
            pattern(empty())     = [&] { matched_pattern = 2; },
            pattern(_)           = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 2);
    }

    static_assert(match(std::array{ 1, 2, 3 })
    (
        pattern(contains(2)) = 1,
        pattern(_)           = 2
    ).value_or(0) == 1);
}

//=================================================================================================

TEST_CASE("Simple matcher starts and ends with range", "[match][ranges]")
{
    std::vector<int> x = { 1, 2, 3, 4 };

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(starts_with(2, 3)) = [&] { matched_pattern = 1; },
            pattern(starts_with(1, _)) = [&] { matched_pattern = 2; },
            pattern(_)                 = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 2);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(ends_with(2, 3))       = [&] { matched_pattern = 1; },
            pattern(ends_with(_x > 2, 4))  = [&] { matched_pattern = 2; },
            pattern(_)                     = [&] { matched_pattern = 3; }
        );

        CHECK(matched_pattern == 2);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(starts_with(1, 2, 3, 4, 5)) = [&] { matched_pattern = 1; },
            pattern(_)                          = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 2);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(ends_with(0, 1, 2, 3, 4)) = [&] { matched_pattern = 1; },
            pattern(_)                        = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 2);
    }

    static_assert(match(std::array{ 1, 2, 3 })
    (
        pattern(starts_with(1, 2) && ends_with(2, 3)) = 1,
        pattern(_)                                    = 2
    ).value_or(0) == 1);
}

//=================================================================================================

TEST_CASE("Simple matcher regex_match", "[match][regex]")
{
    {
        auto matched_pattern = match("12345689_abcdefgh")
        (
            pattern(regex("zzz"))       = 1,
            pattern(regex("[0-9]+_.*")) = 2,
            pattern(_)                  = 3
        );

        CHECK(matched_pattern.value_or(0) == 2);
    }

    {
        auto matched_pattern = match("12345689_abcdefgh")
        (
            pattern(regex("zzz"))       = 1,
            pattern(regex(".*_[a-z]+")) = 2,
            pattern(_)                  = 3
        );

        CHECK(matched_pattern.value_or(0) == 2);
    }

#if PATUM_HAS_FEATURE_RE2
    {
        auto matched_pattern = match("12345689_abcdefgh")
        (
            pattern(sregex("zzz"))       = 1,
            pattern(sregex("[0-9]+_.*")) = 2,
            pattern(_)                   = 3
        );

        CHECK(matched_pattern.value_or(0) == 2);
    }

    {
        auto matched_pattern = match("12345689_abcdefgh")
        (
            pattern(sregex("zzz"))       = 1,
            pattern(sregex(".*_[a-z]+")) = 2,
            pattern(_)                   = 3
        );

        CHECK(matched_pattern.value_or(0) == 2);
    }

    {
        auto matched_pattern = match("12345689_abcdefgh"s)
        (
            pattern(sregex("zzz"))       = 1,
            pattern(sregex(".*_[a-z]+")) = 2,
            pattern(_)                   = 3
        );

        CHECK(matched_pattern.value_or(0) == 2);
    }
#endif
}
