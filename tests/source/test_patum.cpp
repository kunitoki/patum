/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#include <patum.h>

#include <snitch_all.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <iostream>

//=================================================================================================

using namespace ptm;
using namespace std::literals;

//=================================================================================================

namespace {
struct movable_copyable
{
    movable_copyable() = default;

    explicit movable_copyable(int n)
        : counter(n)
    {
    }

    movable_copyable(const movable_copyable& other)
        : counter(other.counter)
    {
        ++copy_count;
    }

    movable_copyable(movable_copyable&& other)
        : counter(std::exchange(other.counter, 0))
    {
        ++move_count;
    }

    movable_copyable& operator=(const movable_copyable& other)
    {
        counter = other.counter;

        ++copy_assign_count;

        return *this;
    }

    movable_copyable& operator=(movable_copyable&& other)
    {
        counter = std::exchange(other.counter, 0);

        ++move_assign_count;

        return *this;
    }

    static void reset_counters()
    {
        copy_count = 0;
        move_count = 0;
        copy_assign_count = 0;
        move_assign_count = 0;
    }

    int counter = 0;

    static int copy_count;
    static int move_count;
    static int copy_assign_count;
    static int move_assign_count;
};

int movable_copyable::copy_count = 0;
int movable_copyable::move_count = 0;
int movable_copyable::copy_assign_count = 0;
int movable_copyable::move_assign_count = 0;
} // namespace

//=================================================================================================

template <class T>
constexpr bool specifying_no_match_valid_expression = requires
{
    { ptm::match()(ptm::pattern(ptm::_) = std::declval<T>()) } -> std::same_as<std::optional<T>>;
};

template <class T>
constexpr bool specifying_no_patterns_valid_expression = requires
{
    { ptm::match(std::declval<T>())() } -> std::same_as<void>;
};

template <class T>
constexpr bool specifying_wrong_match_numbers_valid_expression = requires
{
    { ptm::match(std::declval<T>(), std::declval<T>())(ptm::pattern(ptm::_) = 1) } -> std::same_as<std::optional<int>>;
};

template <class T>
constexpr bool specifying_wrong_pattern_numbers_valid_expression = requires
{
    { ptm::match(std::declval<T>())(ptm::pattern(ptm::_, ptm::_) = 1) } -> std::same_as<std::optional<int>>;
};

template <class T>
constexpr bool specifying_incompatible_returned_types_should_return_void = requires
{
    { ptm::match(std::declval<T>())(ptm::pattern(ptm::_) = 1, ptm::pattern(ptm::_) = std::string("")) } -> std::same_as<void>;
};

TEST_CASE("Patum invarians", "[match][invariants]")
{
    static_assert(not specifying_no_match_valid_expression<int>);
    static_assert(not specifying_no_patterns_valid_expression<int>);
    static_assert(not specifying_wrong_match_numbers_valid_expression<int>);
    static_assert(not specifying_wrong_pattern_numbers_valid_expression<int>);
    static_assert(specifying_incompatible_returned_types_should_return_void<int>);
}

//=================================================================================================

TEST_CASE("Simple matcher wildcard_only", "[match][integral]")
{
    int x = 5;
    int matched_pattern = 0;

    match(x)
    (
        pattern(_)   = [&] { matched_pattern = 1; }
    );

    CHECK(matched_pattern == 1);

    static_assert(match(5)
    (
        pattern(_) = [] { return 1; }
    ).value_or(0) == 1);
}

//=================================================================================================

TEST_CASE("Simple matcher movable objects", "[match][move]")
{
    movable_copyable::reset_counters();

    auto matched_pattern = match("789")
    (
        pattern("123") = movable_copyable(1),
        pattern("456") = movable_copyable(2),
        pattern("789") = movable_copyable(3)
    );

    REQUIRE(matched_pattern.has_value());
    CHECK(matched_pattern->counter == 3);
#if _MSC_VER
    // The result of 5 move constructors (there are 3 movable_copyable objects):
    // + (1) the matched one is then moved into the invoker
    // + (1) then it's emplaced into the result optional
    // = (5)
    CHECK(movable_copyable::move_count == 5);
#else
    // The result of 6 move constructors (there are 3 movable_copyable objects):
    //   (3) they are moved into the pattern()
    // + (1) the matched one is then moved into the invoker
    // + (1) then it's emplaced into the result optional
    // + (1) then the optional is returned with NRVO
    // = (6)
    CHECK(movable_copyable::move_count == 6);
#endif
    CHECK(movable_copyable::copy_count == 0);
    CHECK(movable_copyable::move_assign_count == 0);
    CHECK(movable_copyable::copy_assign_count == 0);
}

//=================================================================================================

namespace {
consteval bool evaluate_match_if_ten(int x)
{
    return match(x)
    (
        pattern(10)   = true,
        pattern(_)    = false
    ).value_or(false);
}
} // namespace

TEST_CASE("Simple matcher constexpr", "[match][constexpr]")
{
    static_assert(evaluate_match_if_ten(10));
    static_assert(not evaluate_match_if_ten(11));
}

//=================================================================================================

TEST_CASE("Simple matcher r-value reference", "[match]")
{
    auto make_string = [] { return "456"s; };

    auto matched_pattern = match(make_string())
    (
        pattern("123") = 1,
        pattern("456") = 2
    );

    CHECK(matched_pattern.value_or(0) == 2);

    static_assert(match("456"sv)
    (
        pattern("123") = 1,
        pattern("456") = 2
    ).value_or(0) == 2);
}

//=================================================================================================

TEST_CASE("Simple matcher return type", "[match][return]")
{
    auto matched_pattern = match(8)
    (
        pattern(1)  = char(1),
        pattern(2)  = (unsigned char)(2),
        pattern(3)  = short(3),
        pattern(4)  = (unsigned short)(4),
        pattern(5)  = int(5),
        pattern(6)  = (unsigned int)(6),
        pattern(7)  = long(7),
        pattern(8)  = (unsigned long)(8),
        pattern(9)  = (long long)(9),
        pattern(10) = (unsigned long long)(10),
        pattern(_)  = 11
    );

    static_assert(std::is_same_v<decltype(matched_pattern)::value_type, unsigned long long>);
    CHECK(matched_pattern.value_or(0) == 8);

    static_assert(match(8)
    (
        pattern(1)  = char(1),
        pattern(2)  = (unsigned char)(2),
        pattern(3)  = short(3),
        pattern(4)  = (unsigned short)(4),
        pattern(5)  = int(5),
        pattern(6)  = (unsigned int)(6),
        pattern(7)  = long(7),
        pattern(8)  = (unsigned long)(8),
        pattern(9)  = (long long)(9),
        pattern(10) = (unsigned long long)(10),
        pattern(_)  = 11
    ).value_or(0) == 8);
}

//=================================================================================================

TEST_CASE("Simple matcher catch one and assign", "[match][integral]")
{
    {
        auto matched_pattern = match(5)
        (
            pattern(10) = 1,
            pattern(5)  = 2,
            pattern(3)  = 3,
            pattern(_)  = 4
        );

        CHECK(matched_pattern.value_or(0) == 2);

        static_assert(match(5)
        (
            pattern(10) = 1,
            pattern(5)  = 2,
            pattern(3)  = 3,
            pattern(_)  = 4
        ).value_or(0) == 2);
    }

    {
        auto matched_pattern = match(15)
        (
            pattern(10) = 1,
            pattern(5)  = 2,
            pattern(3)  = 3
        );

        CHECK(matched_pattern.value_or(0) == 0);

        static_assert(match(15)
        (
            pattern(10) = 1,
            pattern(5)  = 2,
            pattern(3)  = 3
        ).value_or(0) == 0);
    }

    {
        auto matched_pattern = match(5)
        (
            pattern(10) = [] { return 1; },
            pattern(5)  = [] { return 2; },
            pattern(3)  = [] { return 3; },
            pattern(_)  = [] { return 4; }
        );

        CHECK(matched_pattern.value_or(0) == 2);

        static_assert(match(5)
        (
            pattern(10) = [] { return 1; },
            pattern(5)  = [] { return 2; },
            pattern(3)  = [] { return 3; },
            pattern(_)  = [] { return 4; }
        ).value_or(0) == 2);
    }

    {
        auto matched_pattern = match(5)
        (
            pattern(10) = [] {},
            pattern(5)  = [] {},
            pattern(3)  = [] {},
            pattern(_)  = 4
        );

        CHECK(matched_pattern.value_or(0) == 0);

        static_assert(match(5)
        (
            pattern(10) = [] {},
            pattern(5)  = [] {},
            pattern(3)  = [] {},
            pattern(_)  = 4
        ).value_or(0) == 0);
    }

    {
        auto matched_pattern = match(5)
        (
            pattern(10) = 1,
            pattern(5)  = [] { return 2; },
            pattern(3)  = 3,
            pattern(_)  = 4
        );

        CHECK(matched_pattern.value_or(0) == 2);

        static_assert(match(5)
        (
            pattern(10) = 1,
            pattern(5)  = [] { return 2; },
            pattern(3)  = 3,
            pattern(_)  = 4
        ).value_or(0) == 2);
    }
}

//=================================================================================================

TEST_CASE("Simple matcher catch one from all same types", "[match][integral]")
{
    int x = 5;
    int matched_pattern = 0;

    match(x)
    (
        pattern(10) = [&](auto&& v) { matched_pattern = 1; },
        pattern(5)  = [&](auto&& v) { matched_pattern = 2; },
        pattern(3)  = [&] { matched_pattern = 3; },
        pattern(_)  = [&] { matched_pattern = 4; }
    );

    CHECK(matched_pattern == 2);
}

//=================================================================================================

TEST_CASE("Simple matcher catch none from all same types", "[match][integral]")
{
    int x = 111;
    int matched_pattern = 0;

    match(x)
    (
        pattern(10) = [&](auto&& v) { matched_pattern = 1; },
        pattern(5)  = [&](auto&& v) { matched_pattern = 2; },
        pattern(3)  = [&] { matched_pattern = 3; },
        pattern(_)  = [&] { matched_pattern = 4; }
    );

    CHECK(matched_pattern == 4);
}

//=================================================================================================

TEST_CASE("Simple matcher string literal catch one", "[match][string]")
{
    int matched_pattern = 0;

    match("12345")
    (
        pattern("1"sv)    = [&](auto&& v) { matched_pattern = 1; },
        pattern("2")      = [&](auto&& v) { matched_pattern = 2; },
        pattern("12345"s) = [&] { matched_pattern = 3; },
        pattern(_)        = [&] { matched_pattern = 4; }
    );

    CHECK(matched_pattern == 3);
}

//=================================================================================================

TEST_CASE("Simple matcher string catch one", "[match][string]")
{
    std::string x = "12345";
    int matched_pattern = 0;

    match(x)
    (
        pattern("1"sv)    = [&](auto&& v) { matched_pattern = 1; },
        pattern("2"s)     = [&](auto&& v) { matched_pattern = 2; },
        pattern("12345")  = [&] { matched_pattern = 3; },
        pattern(_)        = [&] { matched_pattern = 4; }
    );

    CHECK(matched_pattern == 3);
}

//=================================================================================================

TEST_CASE("Simple matcher const char pointer catch one", "[match][string]")
{
    std::string x = "12345";
    int matched_pattern = 0;

    match(x.c_str())
    (
        pattern("1"sv)    = [&](auto&& v) { matched_pattern = 1; },
        pattern("2"s)     = [&](auto&& v) { matched_pattern = 2; },
        pattern("12345")  = [&] { matched_pattern = 3; },
        pattern(_)        = [&] { matched_pattern = 4; }
    );

    CHECK(matched_pattern == 3);
}

//=================================================================================================

TEST_CASE("Double matcher catch one from all same types", "[match][multi][integral]")
{
    int x = 42, y = 1337;

    {
        int matched_pattern = 0;

        match(x, y)
        (
            pattern(42, 13370) = [&](auto&& x, auto&& y) { matched_pattern = 1; },
            pattern(42, _)     = [&](auto&& x, auto&& y) { matched_pattern = 2; },
            pattern(_, 1337)   = [&] { matched_pattern = 3; },
            pattern(_, _)      = [&] { matched_pattern = 4; }
        );

        CHECK(matched_pattern == 2);

        static_assert(match(42, 1337)
        (
            pattern(42, 13370) = [](auto&&, auto&&) { return 1; },
            pattern(42, _)     = [](auto&&, auto&&) { return 2; },
            pattern(_, 1337)   = [] { return 3; },
            pattern(_, _)      = [] { return 4; }
        ).value_or(0) == 2);
    }

    {
        int matched_pattern = 0;

        match(x, y)
        (
            pattern(42, 1337)  = [&](auto&& x, auto&& y) { matched_pattern = 1; },
            pattern(42, _)     = [&](auto&& v, auto&& y) { matched_pattern = 2; },
            pattern(_, 1337)   = [&] { matched_pattern = 3; },
            pattern(_, _)      = [&] { matched_pattern = 4; }
        );

        CHECK(matched_pattern == 1);

        static_assert(match(42, 1337)
        (
            pattern(42, 1337) = [](auto&&, auto&&) { return 1; },
            pattern(42, _)    = [](auto&&, auto&&) { return 2; },
            pattern(_, 1337)  = [] { return 3; },
            pattern(_, _)     = [] { return 4; }
        ).value_or(0) == 1);
    }

    {
        int matched_pattern = 0;

        match(x, y)
        (
            pattern(420, 1337) = [&](auto&& x, auto&& y) { matched_pattern = 1; },
            pattern(420, _)    = [&](auto&& x, auto&& y) { matched_pattern = 2; },
            pattern(_, 1337)   = [&] { matched_pattern = 3; },
            pattern(_, _)      = [&] { matched_pattern = 4; }
        );

        CHECK(matched_pattern == 3);

        static_assert(match(42, 1337)
        (
            pattern(420, 1337) = [](auto&&, auto&&) { return 1; },
            pattern(420, _)    = [](auto&&, auto&&) { return 2; },
            pattern(_, 1337)   = [] { return 3; },
            pattern(_, _)      = [] { return 4; }
        ).value_or(0) == 3);
    }

    {
        int matched_pattern = 0;

        match(x, y)
        (
            pattern(420, 13370) = [&](auto&& x, auto&& y) { matched_pattern = 1; },
            pattern(420, _)     = [&](auto&& x, auto&& y) { matched_pattern = 2; },
            pattern(_, 13370)   = [&] { matched_pattern = 3; },
            pattern(_, _)       = [&] { matched_pattern = 4; }
        );

        CHECK(matched_pattern == 4);

        static_assert(match(42, 1337)
        (
            pattern(420, 13370) = [](auto&&, auto&&) { return 1; },
            pattern(420, _)     = [](auto&&, auto&&) { return 2; },
            pattern(_, 13370)   = [] { return 3; },
            pattern(_, _)       = [] { return 4; }
        ).value_or(0) == 4);
    }
}

//=================================================================================================

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
            pattern(some(42)) = [&] { matched_pattern = 1; },
            pattern(_)        = [&] { matched_pattern = 2; }
        );

        CHECK(matched_pattern == 1);
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

/*
namespace {
struct Shape { virtual ~Shape() = default; };
struct Circle : Shape { Circle(int r) : radius(r) {} int radius; };
struct Rectangle : Shape { Rectangle(int w, int h) : width(w), height(h) {} int width, height; };
} // namespace

TEST_CASE("Simple matcher is type on polymorphic", "[match][is][polymorphic]")
{
    auto get_area = [](const Shape& shape)
    {
        return match(shape)
        (
            pattern(is<Circle>)    = [](auto&& c) { return 3.14 * c.radius * c.radius; },
            pattern(is<Rectangle>) = [](auto&& r) { return r.width * r.height; }
        ).value_or(0);
    };

    {
        auto shape = Rectangle{ 100, 200 };
        auto area = get_area(shape);
        CHECK(100 * 200 == area);
    }

    {
        auto shape = Circle{ 100 };
        auto area = get_area(shape);
        CHECK(3.14 * 100 * 100 == area);
    }
}
*/

//=================================================================================================

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
}

//=================================================================================================

namespace {
struct aggregate
{
    int x = 0;
    float y = 1000.0f;
    char z = 'a';
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
}

//=================================================================================================

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
#endif
}
