/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#include "test_patum_common.h"

//=================================================================================================

namespace {
struct invocation_base
{
    virtual ~invocation_base() = default;
};

struct invocation_derived : invocation_base
{
    int value = 42;
};
} // namespace

TEST_CASE("Simple matcher direct invocation helpers", "[match][invoke]")
{
    {
        auto matcher = pattern(_) = [] { return 7; };
        CHECK(ptm::test_expressions(matcher, std::forward_as_tuple(1)) == 7);
    }

    {
        auto matcher = pattern(_) = [](int value) { return value + 1; };
        CHECK(ptm::test_expressions(matcher, std::forward_as_tuple(41)) == 42);
    }

    {
        auto matcher = pattern(_) = [](int value) { return value + 1; };
        CHECK(ptm::test_expressions(matcher, std::forward_as_tuple(std::optional<int>{ 41 })) == 42);
    }

    {
        auto matcher = pattern(ds(_, _)) = [](int value, const std::string& text) {
            return value + static_cast<int>(text.size());
        };
        auto value = std::make_tuple(39, "abc"s);
        CHECK(ptm::test_expressions(matcher, std::forward_as_tuple(value)) == 42);
    }

    {
        auto matcher = pattern(_) = [](invocation_derived* value) {
            return value->value;
        };
        invocation_derived derived;
        invocation_base* base = &derived;
        CHECK(ptm::test_expressions(matcher, std::forward_as_tuple(base)) == 42);
    }

    {
        auto matcher = pattern(_) = 42;
        CHECK(ptm::test_expressions(matcher, std::forward_as_tuple(1)) == 42);
    }
}

//=================================================================================================

TEST_CASE("Simple matcher direct lvalue get helpers", "[match][invoke]")
{
    {
        auto matcher = pattern(_) = [] { return 7; };
        CHECK(matcher.get(1) == 7);
    }

    {
        auto matcher = pattern(_) = [](int value) { return value + 1; };
        CHECK(matcher.get(41) == 42);
    }

    {
        auto matcher = pattern(_) = [](int value) { return value + 1; };
        CHECK(matcher.get(std::optional<int>{ 41 }) == 42);
    }

    {
        auto matcher = pattern(ds(_, _)) = [](int value, const std::string& text) {
            return value + static_cast<int>(text.size());
        };
        auto value = std::make_tuple(39, "abc"s);
        CHECK(matcher.get(value) == 42);
    }

    {
        auto matcher = pattern(_) = [](invocation_derived* value) {
            return value->value;
        };
        invocation_derived derived;
        invocation_base* base = &derived;
        CHECK(matcher.get(base) == 42);
    }

    {
        auto matcher = pattern(_) = 42;
        CHECK(matcher.get(1) == 42);
    }
}

//=================================================================================================

TEST_CASE("Simple matcher direct const invocation helpers", "[match][invoke]")
{
    {
        const auto matcher = pattern(_) = [] { return 7; };
        CHECK(ptm::test_expressions(matcher, std::forward_as_tuple(1)) == 7);
    }

    {
        const auto matcher = pattern(_) = [](int value) { return value + 1; };
        CHECK(ptm::test_expressions(matcher, std::forward_as_tuple(41)) == 42);
    }

    {
        const auto matcher = pattern(_) = [](int value) { return value + 1; };
        CHECK(ptm::test_expressions(matcher, std::forward_as_tuple(std::optional<int>{ 41 })) == 42);
    }

    {
        const auto matcher = pattern(ds(_, _)) = [](int value, const std::string& text) {
            return value + static_cast<int>(text.size());
        };
        auto value = std::make_tuple(39, "abc"s);
        CHECK(ptm::test_expressions(matcher, std::forward_as_tuple(value)) == 42);
    }

    {
        const auto matcher = pattern(_) = [](invocation_derived* value) {
            return value->value;
        };
        invocation_derived derived;
        invocation_base* base = &derived;
        CHECK(ptm::test_expressions(matcher, std::forward_as_tuple(base)) == 42);
    }

    {
        const auto matcher = pattern(_) = 42;
        CHECK(ptm::test_expressions(matcher, std::forward_as_tuple(1)) == 42);
    }
}

//=================================================================================================

TEST_CASE("Simple matcher direct capture and dereference helpers", "[match][invoke]")
{
    {
        int value = 42;
        auto captures = ptm::match_captures(1, value);
        CHECK(&std::get<0>(captures) == &value);
    }

    {
        int value = 42;
        CHECK(&ptm::dereference(value) == &value);
    }

    {
        const std::optional<int> value = 42;
        CHECK(ptm::dereference(value) == 42);
        CHECK(ptm::dereference(std::move(value)) == 42);
    }

    {
        const std::unique_ptr<int> value = std::make_unique<int>(42);
        CHECK(ptm::dereference(value) == 42);
        CHECK(ptm::dereference(std::move(value)) == 42);
    }
}
