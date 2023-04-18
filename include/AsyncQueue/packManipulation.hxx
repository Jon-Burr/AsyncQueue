/// @file packManipulation.hxx
/// @brief Helpers for manipulating parameter packs

#ifndef ASYNCQUEUE_PACKMANIPULATION_HXX
#define ASYNCQUEUE_PACKMANIPULATION_HXX

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace AsyncQueue { namespace detail {

    template <std::size_t I, typename... Args>
    struct pack_element : public std::tuple_element<I, std::tuple<Args...>> {};

    template <std::size_t I, typename... Args>
    using pack_element_t = typename pack_element<I, Args...>::type;

    template <std::size_t I, typename... Args>
    pack_element_t<I, Args...> extract_pack_element(Args &&...args) {
        return std::get<I>(std::forward_as_tuple(args...));
    }

    template <std::size_t Idx, std::size_t... Is, typename Tuple>
    auto tuple_piece(Tuple &&tuple, std::index_sequence<Is...>) {
        return std::forward_as_tuple(std::get<Idx + Is>(tuple)...);
    }
    template <std::size_t N, typename Tuple> auto tuple_piece(Tuple &&tuple) {
        return tuple_piece<0>(std::forward<Tuple>(tuple), std::make_index_sequence<N>());
    }
    template <std::size_t Idx, std::size_t N, typename Tuple> auto tuple_piece(Tuple &&tuple) {
        return tuple_piece<Idx>(std::forward<Tuple>(tuple), std::make_index_sequence<N>());
    }

    template <std::size_t Idx, typename T, typename Tuple>
    auto insert_tuple_element(T &&element, Tuple &&tuple) {
        static constexpr std::size_t N = std::tuple_size_v<std::decay_t<Tuple>>;
        if constexpr (Idx == 0)
            // Insert at beginning
            return std::tuple_cat(std::forward_as_tuple(element), std::forward<Tuple>(tuple));
        else if constexpr (Idx < N)
            // Insert part way through
            return std::tuple_cat(
                    tuple_piece<0, Idx>(std::forward<Tuple>(tuple)), std::forward_as_tuple(element),
                    tuple_piece<Idx, N - Idx>(std::forward<Tuple>(tuple)));
        else
            // Append to the end
            return std::tuple_cat(std::forward<Tuple>(tuple), std::forward_as_tuple(element));
    }

    template <std::size_t Idx, typename T, typename Tuple>
    using insert_tuple_element_t =
            decltype(insert_tuple_element<Idx>(std::declval<T>(), std::declval<Tuple>()));

    template <std::size_t Idx, typename T, typename Tuple>
    auto replace_tuple_element(T &&element, Tuple &&tuple) {
        static constexpr std::size_t N = std::tuple_size_v<std::decay_t<Tuple>>;
        if constexpr (Idx == 0)
            return std::tuple_cat(std::forward_as_tuple(element), tuple_piece<1, N - 1>(tuple));
        else if constexpr (Idx == N - 1)
            return std::tuple_cat(tuple_piece<Idx>(tuple), std::forward_as_tuple(element));
        else
            return std::tuple_cat(
                    tuple_piece<Idx>(tuple), std::forward_as_tuple(element),
                    tuple_piece<Idx + 1, N - Idx - 1>(tuple));
    }

    template <std::size_t Idx, typename T, typename Tuple>
    using replace_tuple_element_t =
            decltype(replace_tuple_element<Idx>(std::declval<T>(), std::declval<Tuple>()));

    template <typename F, typename Tuple> struct apply_result;

    template <typename F, typename... Args>
    struct apply_result<F, std::tuple<Args...>> : public std::invoke_result<F, Args...> {};

    template <typename F, typename Tuple>
    using apply_result_t = typename apply_result<F, Tuple>::type;

    template <typename F, typename Tuple> struct is_applicable;

    template <typename F, typename... Args>
    struct is_applicable<F, std::tuple<Args...>> : public std::is_invocable<F, Args...> {};

    template <typename F, typename Tuple>
    static constexpr inline bool is_applicable_v = is_applicable<F, Tuple>::value;

    template <typename F, typename Tuple> struct async_result;

    template <typename F, typename... Args>
    struct async_result<F, std::tuple<Args...>>
            : public std::invoke_result<std::decay_t<F>, std::decay_t<Args>...> {};

    template <typename F, typename Tuple>
    using async_result_t = typename async_result<F, Tuple>::type;

    template <typename F, typename Tuple> struct is_async_applicable;

    template <typename F, typename... Args>
    struct is_async_applicable<F, std::tuple<Args...>>
            : public std::is_invocable<std::decay_t<F>, std::decay_t<Args>...> {};

    template <typename F, typename Tuple>
    static constexpr inline bool is_async_applicable_v = is_async_applicable<F, Tuple>::value;

}} // namespace AsyncQueue::detail

#endif //> !ASYNCQUEUE_PACKMANIPULATION_HXX