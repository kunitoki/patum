/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#include "test_patum_common.h"

TEST_CASE("Simple matcher range", "[match][range]")
{
    int x = 12;

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(range(1, 124)) = [&] { matched_pattern = 1; },
            pattern(_)             = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);

        static_assert(match(12)
        (
            pattern(range(1, 124)) = 1,
            pattern(_)             = 2
        ).value_or(0) == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(range(1, 11)) = [&] { matched_pattern = 1; },
            pattern(_)            = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 2);

        static_assert(match(12)
        (
            pattern(range(1, 11)) = 1,
            pattern(_)            = 2
        ).value_or(0) == 2);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(range(13, 124)) = [&] { matched_pattern = 1; },
            pattern(_)              = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 2);

        static_assert(match(12)
        (
            pattern(range(13, 1124)) = 1,
            pattern(_)               = 2
        ).value_or(0) == 2);
    }
}

//=================================================================================================

TEST_CASE("Simple matcher in", "[match][in]")
{
    int x = 4;

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(in(1, 2, 3, 4, 5, 6, 7, 10, 12)) = [&] { matched_pattern = 1; },
            pattern(_)                               = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);

        static_assert(match(4)
        (
            pattern(in(1, 2, 3, 4, 5, 6, 7, 10, 12)) = 1,
            pattern(_)                               = 2
        ).value_or(0) == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(in(1, 2, 3, 5, 6, 7, 10, 12)) = [&] { matched_pattern = 1; },
            pattern(_)                            = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 2);

        static_assert(match(4)
        (
            pattern(in(1, 2, 3, 5, 6, 7, 10, 12)) = 1,
            pattern(_)                            = 2
        ).value_or(0) == 2);
    }
}

//=================================================================================================

TEST_CASE("Simple matcher not_in", "[match][range]")
{
    int x = 4;

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(!in(1, 2, 3, 5, 6, 7, 10, 12)) = [&] { matched_pattern = 1; },
            pattern(_)                             = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);

        static_assert(match(4)
        (
            pattern(!in(1, 2, 3, 5, 6, 7, 10, 12)) = 1,
            pattern(_)                             = 2
        ).value_or(0) == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(!in(1, 2, 3, 4, 5, 6, 7, 10, 12)) = [&] { matched_pattern = 1; },
            pattern(_)                                = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 2);

        static_assert(match(4)
        (
            pattern(!in(1, 2, 3, 4, 5, 6, 7, 10, 12)) = 1,
            pattern(_)                                = 2
        ).value_or(0) == 2);
    }
}

//=================================================================================================

TEST_CASE("Simple matcher optional some", "[match][some]")
{
    std::optional<int> x = 42;
    std::optional<int> y;

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(some()) = [&](int v) { matched_pattern = v; },
            pattern(_)      = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 42);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(some()) = [&] { matched_pattern = 1; },
            pattern(_)      = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);

        static_assert(match(std::optional<int>(42))
        (
            pattern(some()) = 1,
            pattern(_)      = 2
        ).value_or(0) == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(some(42)) = [&] { matched_pattern = 1; },
            pattern(_)        = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);

        static_assert(match(std::optional<int>(42))
        (
            pattern(some(42)) = 1,
            pattern(_)        = 2
        ).value_or(0) == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(some(11)) = [&] { matched_pattern = 1; },
            pattern(_)        = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 2);

        static_assert(match(std::optional<int>(42))
        (
            pattern(some(11)) = 1,
            pattern(_)        = 2
        ).value_or(0) == 2);
    }

    {
        int matched_pattern = 0;

        match(y)
        (
            pattern(some()) = [&] { matched_pattern = 1; },
            pattern(_)      = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 2);

        static_assert(match(std::optional<int>())
        (
            pattern(some()) = 1,
            pattern(_)      = 2
        ).value_or(0) == 2);
    }

    {
        int matched_pattern = 0;

        match(y)
        (
            pattern(some(42)) = [&] { matched_pattern = 1; },
            pattern(_)        = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 2);

        static_assert(match(std::optional<int>())
        (
            pattern(some(42)) = 1,
            pattern(_)        = 2
        ).value_or(0) == 2);
    }
}

//=================================================================================================

TEST_CASE("Simple matcher optional none", "[match][some]")
{
    std::optional<int> x;
    std::optional<int> y = 42;

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(none) = [&] { matched_pattern = 1; },
            pattern(_)    = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);

        static_assert(match(std::optional<int>())
        (
            pattern(none) = 1,
            pattern(_)    = 2
        ).value_or(0) == 1);
    }

    {
        int matched_pattern = 0;

        match(y)
        (
            pattern(none) = [&] { matched_pattern = 1; },
            pattern(_)    = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 2);

        static_assert(match(std::optional<int>(42))
        (
            pattern(none) = 1,
            pattern(_)    = 2
        ).value_or(0) == 2);
    }
}

//=================================================================================================

TEST_CASE("Simple matcher pointer some", "[match][some]")
{
    std::unique_ptr<int> x = std::make_unique<int>(42);

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(some()) = [&] { matched_pattern = 1; },
            pattern(_)      = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(some()) = [&](int v) { matched_pattern = v; },
            pattern(_)      = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 42);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(some(42)) = [&] { matched_pattern = 1; },
            pattern(_)        = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(some(42)) = [&](int v) { matched_pattern = v; },
            pattern(_)        = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 42);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(some(11)) = [&] { matched_pattern = 1; },
            pattern(_)        = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 2);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(some(42)) = [&](std::unique_ptr<int>&& v) { matched_pattern = *v; },
            pattern(_)        = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 42);
    }
}

//=================================================================================================

TEST_CASE("Simple matcher pointer none", "[match][some]")
{
    std::unique_ptr<int> x;
    std::unique_ptr<int> y = std::make_unique<int>(42);

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(none) = [&] { matched_pattern = 1; },
            pattern(_)    = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(y)
        (
            pattern(none) = [&] { matched_pattern = 1; },
            pattern(_)    = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 2);
    }
}

//=================================================================================================

TEST_CASE("Simple matcher raw pointer some", "[match][some]")
{
    int* x = new int(42);

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(some()) = [&] { matched_pattern = 1; },
            pattern(_)      = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(some()) = [&](int v) { matched_pattern = v; },
            pattern(_)      = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 42);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(some(42)) = [&] { matched_pattern = 1; },
            pattern(_)        = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(some(42)) = [&](int v) { matched_pattern = v; },
            pattern(_)        = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 42);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(some(11)) = [&] { matched_pattern = 1; },
            pattern(_)        = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 2);
    }
    
    delete x;
}

//=================================================================================================

TEST_CASE("Simple matcher sized on vector", "[match][sized]")
{
    std::vector<int> x = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(sized(1))    = [&] { matched_pattern = 1; },
            pattern(sized(2))    = [&] { matched_pattern = 2; },
            pattern(sized(3))    = [&] { matched_pattern = 3; },
            pattern(sized(10))   = [&] { matched_pattern = 4; },
            pattern(_)           = [&] { matched_pattern = 5; }
        );

        CHECK(matched_pattern == 4);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(size(_x) == 1)    = [&] { matched_pattern = 1; },
            pattern(size(_x) == 2)    = [&] { matched_pattern = 2; },
            pattern(size(_x) == 3)    = [&] { matched_pattern = 3; },
            pattern(size(_x) == 10)   = [&] { matched_pattern = 4; },
            pattern(_)                = [&] { matched_pattern = 5; }
        );

        CHECK(matched_pattern == 4);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(ssized(1))    = [&] { matched_pattern = 1; },
            pattern(ssized(2))    = [&] { matched_pattern = 2; },
            pattern(ssized(3))    = [&] { matched_pattern = 3; },
            pattern(ssized(10))   = [&] { matched_pattern = 4; },
            pattern(_)            = [&] { matched_pattern = 5; }
        );

        CHECK(matched_pattern == 4);
    }

    {
        int matched_pattern = 0;

        match(x)
        (
            pattern(ssize(_x) == -1) = [&] { matched_pattern = 1; },
            pattern(ssize(_x) == -2) = [&] { matched_pattern = 2; },
            pattern(ssize(_x) == -3) = [&] { matched_pattern = 3; },
            pattern(ssize(_x) == 10) = [&] { matched_pattern = 4; },
            pattern(_)               = [&] { matched_pattern = 5; }
        );

        CHECK(matched_pattern == 4);
    }
}

//=================================================================================================
