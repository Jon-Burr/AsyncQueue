/// @file packManipulation.hxx
/// @brief Helpers for manipulating parameter packs

#ifndef ASYNCQUEUE_PACKMANIPULATION_HXX
#define ASYNCQUEUE_PACKMANIPULATION_HXX

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace AsyncQueue {
    namespace detail {

        template <std::size_t Idx, typename T, typename Tuple>
        auto insert_tuple_element(T &&element, Tuple &&tuple) {
            static constexpr std::size_t N = std::tuple_size_v<std::decay_t<Tuple>>;
            if constexpr (Idx == 0)
                // Insert at beginning
                return std::tuple_cat(std::forward_as_tuple(element), std::forward<Tuple>(tuple));
            else if constexpr (Idx < N)
                // Insert part way through
                return std::tuple_cat(
                        tuple_piece<0, Idx>(std::forward<Tuple>(tuple)),
                        std::forward_as_tuple(element),
                        tuple_piece<Idx, N - Idx>(std::forward<Tuple>(tuple)));
            else
                // Append to the end
                return std::tuple_cat(std::forward<Tuple>(tuple), std::forward_as_tuple(element));
        }

        template <std::size_t Idx, typename T, typename Tuple>
        using insert_tuple_element_t =
                decltype(insert_tuple_element<Idx>(std::declval<T>(), std::declval<Tuple>()));

    } // namespace detail
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_PACKMANIPULATION_HXX