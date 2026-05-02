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
#include <type_traits>

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

namespace detail {

template <class T>
concept matcher_like = requires
{
    { std::remove_cvref_t<T>::capture_count } -> std::convertible_to<std::size_t>;
};

template <class... M>
concept non_empty_matcher_pack = sizeof...(M) != 0;

template <class... M>
concept compatible_matcher_pack = (matcher_like<M> && ...);

template <std::size_t Count, class... M>
concept compatible_matcher_arity = (true && ... && (Count == std::remove_cvref_t<M>::capture_count));

template <std::size_t Count, class... M>
concept valid_match_expression =
    non_empty_matcher_pack<M...> &&
    compatible_matcher_pack<M...> &&
    compatible_matcher_arity<Count, M...>;

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
constexpr auto test_expressions(const M& matcher, E&& expressions)
{
    return std::apply([&]<class... E2>(E2&&... ex)
    {
        return matcher.get(std::forward<E2>(ex)...);
    }, std::forward<E>(expressions));
}

template <class R, class M, class E>
constexpr void invoke_result_expressions(std::optional<R>& result, M&& matcher, E&& expressions)
{
    std::apply([&, m = std::forward<M>(matcher)]<class... E2>(E2&&... ex) mutable
    {
        if constexpr (std::same_as<decltype(m.get(std::forward<E2>(ex)...)), void>)
            std::move(m).get(std::forward<E2>(ex)...);
        else
            result.emplace(std::move(m).get(std::forward<E2>(ex)...));
    }, std::forward<E>(expressions));
}

template <class M, class E>
constexpr void invoke_expressions(M&& matcher, E&& expressions)
{
    std::apply([m = std::forward<M>(matcher)]<class... E2>(E2&&... ex) mutable
    {
        std::move(m).get(std::forward<E2>(ex)...);
    }, std::forward<E>(expressions));
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
        requires detail::valid_match_expression<sizeof...(E), M...>
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
