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
    } // namespace detail

    namespace concepts {
        template <typename T, template <typename...> class C>
        concept SpecialisationOf = detail::is_specialisation_of_v<T, C>;
        /// @brief Concept only satisfied by std::chrono::durations
        template <typename T>
        concept Duration = SpecialisationOf<T, std::chrono::duration>;
        /// @brief Concept only satisfied by std::chrono::time_points
        template <typename T>
        concept TimePoint = SpecialisationOf<T, std::chrono::time_point>;
    } // namespace concepts
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_CONCEPTS_HXX