/**
 * patum - A pattern matching library for modern C++
 *
 * Copyright (c) 2025 - kunitoki <kunitoki@gmail.com>
 *
 * Licensed under the MIT License. Visit https://opensource.org/licenses/MIT for more information.
 */

#pragma once

#include <concepts>
#include <optional>
#include <tuple>

#include "match_expression.h"

namespace ptm {

//=================================================================================================

template <class... M>
constexpr bool compatible_patterns() noexcept
{
    return (true && ... && (requires { M::capture_count; }));
}

template <class... M>
constexpr bool compatible_patterns_args(std::size_t count) noexcept
{
    return (true && ... && (count == M::capture_count));
}

//=================================================================================================

template <class M, class E>
constexpr auto match_expressions(const M& matcher, const E& expressions)
{
    return std::apply([&](const auto&... ex)
    {
        return matcher.check(ex...);
    }, expressions);
}

template <class M, class E>
constexpr auto test_expressions(const M& matcher, const E& expressions)
{
    return std::apply([&](const auto&... ex)
    {
        return matcher.get(ex...);
    }, expressions);
}

template <class R, class M, class E>
constexpr void invoke_result_expressions(std::optional<R>& result, M&& matcher, const E& expressions)
{
    std::apply([&, m = std::forward<M>(matcher)](const auto&... ex) mutable
    {
        if constexpr (std::same_as<decltype(m.get(ex...)), void>)
            std::move(m).get(ex...);
        else
            result.emplace(std::move(m).get(ex...));
    }, expressions);
}

template <class M, class E>
constexpr void invoke_expressions(M&& matcher, const E& expressions)
{
    std::apply([m = std::forward<M>(matcher)](const auto&... ex) mutable
    {
        std::move(m).get(ex...);
    }, expressions);
}

//=================================================================================================

template <class... E>
struct match_helper
{
    constexpr explicit match_helper(E&&... expressions)
        : expressions_(std::forward_as_tuple(expressions...))
    {
    }

    template <class... M>
        requires(compatible_patterns<M...>()
            && compatible_patterns_args<M...>(sizeof...(E))
            && sizeof...(M) != 0)
    constexpr auto operator()(M&&... matchers) const
    {
        using ReturnType = non_void_common_type_t<decltype(test_expressions(matchers, expressions_))...>;

        if constexpr (std::same_as<ReturnType, not_found_t>)
        {
            [[maybe_unused]] auto ignore = ((match_expressions(matchers, expressions_)
                && (void(invoke_expressions(std::forward<M>(matchers), expressions_)), 1)) || ...);
        }
        else
        {
            std::optional<ReturnType> result;

            [[maybe_unused]] auto ignore = ((match_expressions(matchers, expressions_)
                && (void(invoke_result_expressions(result, std::forward<M>(matchers), expressions_)), 1)) || ...);

            return result;
        }
    }

private:
    std::tuple<E...> expressions_;
};

template <class... Args>
[[nodiscard]] constexpr match_helper<Args...> match(Args&&... args)
{
    return match_helper<Args...>{ std::forward<Args>(args)... };
}

} // namespace ptm
