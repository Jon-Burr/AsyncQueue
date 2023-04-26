/**
 * @file concepts.hxx
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Commonly used concepts used in the library
 * @version 0.1
 * @date 2023-04-26
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef ASYNCQUEUE_CONCEPTS_HXX
#define ASYNCQUEUE_CONCEPTS_HXX

#include "AsyncQueue/Fwd.hxx"
#include "AsyncQueue/TaskStatus.hxx"
#include "AsyncQueue/packManipulation.hxx"

#include <chrono>
#include <concepts>
#include <type_traits>

namespace AsyncQueue {
    namespace detail {
        /// @brief Trait type that checks if a type T is a specialisation of the template class C
        template <typename T, template <typename...> class C>
        struct is_specialisation_of : public std::false_type {};
        template <template <typename...> class C, typename... Args>
        struct is_specialisation_of<C<Args...>, C> : public std::true_type {};
        /// @brief Value that checks if a type T is a specialisation of the template class C
        template <typename T, template <typename...> class C>
        static constexpr inline bool is_specialisation_of_v = is_specialisation_of<T, C>::value;

        template <typename F, typename Tuple> struct is_applicable;

        template <typename F, typename... Args>
        struct is_applicable<F, std::tuple<Args...>> : std::is_invocable<F, Args...> {};

        template <typename F, typename Tuple>
        static constexpr inline bool is_applicable_v = is_applicable<F, Tuple>::value;

        template <typename F, typename Tuple> struct apply_result;

        template <typename F, typename... Args>
        struct apply_result<F, std::tuple<Args...>> : public std::invoke_result<F, Args...> {};

        template <typename F, typename Tuple>
        using apply_result_t = typename apply_result<F, Tuple>::type;

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

    } // namespace detail

    namespace concepts {
        template <typename T, template <typename...> class C>
        concept SpecialisationOf = detail::is_specialisation_of_v<T, C>;
        /// @brief Concept only satisfied by std::chrono::durations
        template <typename T> concept Duration = SpecialisationOf<T, std::chrono::duration>;
        /// @brief Concept only satisfied by std::chrono::time_points
        template <typename T> concept TimePoint = SpecialisationOf<T, std::chrono::time_point>;
        template <typename F, typename Tuple>
        concept Applicable = detail::is_applicable_v<F, Tuple>;
        template <typename F, typename Tuple>
        concept AsyncApplicable = detail::is_async_applicable_v<F, Tuple>;
        /// @brief Concept satisfied by function + argument combinations that can be looped
        /// returning void
        template <typename F, typename Tuple>
        concept LoopableVoidTuple =
                AsyncApplicable<F, Tuple> &&std::same_as<detail::async_result_t<F, Tuple>, void>;
        /// @brief Concept satisfied by function + argument combinations that can be looped
        /// returning TaskStatus
        template <typename F, typename Tuple>
        concept LoopableTaskTuple = AsyncApplicable<F, Tuple>
                &&std::convertible_to<detail::async_result_t<F, Tuple>, TaskStatus>;

        /// @brief Concept satisfied by function + argument combinations that can be looped
        template <typename F, typename Tuple>
        concept LoopableTuple = LoopableVoidTuple<F, Tuple> || LoopableTaskTuple<F, Tuple>;

        /// @brief Concept satisfied by function + argument combinations that can be looped
        /// returning void
        template <typename F, typename... Args>
        concept LoopableVoid = LoopableVoidTuple<F, std::tuple<Args...>>;
        /// @brief Concept satisfied by function + argument combinations that can be looped
        /// returning TaskStatus
        template <typename F, typename... Args>
        concept LoopableTask = LoopableTaskTuple<F, std::tuple<Args...>>;
        /// @brief Concept satisfied by function + argument combinations that can be looped
        template <typename F, typename... Args>
        concept Loopable = LoopableTuple<F, std::tuple<Args...>>;

        template <std::size_t Idx, typename F, typename T, typename... Args>
        concept ConsumerVoid =
                LoopableVoidTuple<F, detail::insert_tuple_element_t<Idx, T, std::tuple<Args...>>>;

        template <std::size_t Idx, typename F, typename T, typename... Args>
        concept ConsumerTask =
                LoopableTaskTuple<F, detail::insert_tuple_element_t<Idx, T, std::tuple<Args...>>>;

        template <std::size_t Idx, typename F, typename T, typename... Args>
        concept Consumer = (ConsumerVoid<Idx, F, T, Args...> || ConsumerTask<Idx, F, T, Args...>);

        template <std::size_t Idx, typename F, typename T, typename... Args>
        concept ProducerVoid = LoopableVoidTuple<
                F, detail::insert_tuple_element_t<
                           Idx, std::reference_wrapper<AsyncQueue<T>>, std::tuple<Args...>>>;

        template <std::size_t Idx, typename F, typename T, typename... Args>
        concept ProducerTask = LoopableTaskTuple<
                F, detail::insert_tuple_element_t<
                           Idx, std::reference_wrapper<AsyncQueue<T>>, std::tuple<Args...>>>;

        template <std::size_t Idx, typename F, typename T, typename... Args>
        concept Producer = (ProducerVoid<Idx, F, T, Args...> || ProducerTask<Idx, F, T, Args...>);

    } // namespace concepts
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_CONCEPTS_HXX