/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#include "test_patum_common.h"

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

struct not_equality_comparable
{
};
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
    static_assert(not ptm::detail::match_evaluable_v<not_equality_comparable, not_equality_comparable>);
    static_assert(ptm::detail::valid_match_expression<1, decltype(ptm::pattern(ptm::_) = 1)>);
    static_assert(not ptm::detail::valid_match_expression<2, decltype(ptm::pattern(ptm::_) = 1)>);
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
