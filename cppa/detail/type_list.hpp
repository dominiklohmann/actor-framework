/******************************************************************************\
 *           ___        __                                                    *
 *          /\_ \    __/\ \                                                   *
 *          \//\ \  /\_\ \ \____    ___   _____   _____      __               *
 *            \ \ \ \/\ \ \ '__`\  /'___\/\ '__`\/\ '__`\  /'__`\             *
 *             \_\ \_\ \ \ \ \L\ \/\ \__/\ \ \L\ \ \ \L\ \/\ \L\.\_           *
 *             /\____\\ \_\ \_,__/\ \____\\ \ ,__/\ \ ,__/\ \__/.\_\          *
 *             \/____/ \/_/\/___/  \/____/ \ \ \/  \ \ \/  \/__/\/_/          *
 *                                          \ \_\   \ \_\                     *
 *                                           \/_/    \/_/                     *
 *                                                                            *
 * Copyright (C) 2011 - 2014                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the Boost Software License, Version 1.0. See             *
 * accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt  *
\******************************************************************************/

#ifndef CPPA_DETAIL_TYPE_LIST_HPP
#define CPPA_DETAIL_TYPE_LIST_HPP

#include <typeinfo>
#include <type_traits>

#include "cppa/unit.hpp"

#include "cppa/detail/tbind.hpp"
#include "cppa/detail/type_pair.hpp"

namespace cppa {
namespace detail {

/**
 * @addtogroup MetaProgramming
 * @{
 */

/**
 * @brief A list of types.
 */
template<typename... Ts>
struct type_list { };

/**
 * @brief Denotes the empty list.
 */
using empty_type_list = type_list<>;

template<typename T>
struct is_type_list {
    static constexpr bool value = false;
};

template<typename... Ts>
struct is_type_list<type_list<Ts...>> {
    static constexpr bool value = true;
};

// T head(type_list)

/**
 * @brief Gets the first element of @p List.
 */
template<class List>
struct tl_head;

template<template<typename...> class List>
struct tl_head<List<>> {
    using type = void;
};

template<template<typename...> class List, typename T0, typename... Ts>
struct tl_head<List<T0, Ts...>> {
    using type = T0;
};

// type_list tail(type_list)

/**
 * @brief Gets the tail of @p List.
 */
template<class List>
struct tl_tail;

template<template<typename...> class List>
struct tl_tail<List<>> {
    using type = List<>;
};

template<template<typename...> class List, typename T0, typename... Ts>
struct tl_tail<List<T0, Ts...>> {
    using type = List<Ts...>;
};

// size_t size(type_list)

/**
 * @brief Gets the number of template parameters of @p List.
 */
template<class List>
struct tl_size;

template<template<typename...> class List, typename... Ts>
struct tl_size<List<Ts...>> {
    static constexpr size_t value = sizeof...(Ts);
};

// T back(type_list)

/**
 * @brief Gets the last element in @p List.
 */
template<class List>
struct tl_back;

template<template<typename...> class List>
struct tl_back<List<>> {
    using type = unit_t;
};

template<template<typename...> class List, typename T0>
struct tl_back<List<T0>> {
    using type = T0;
};

template<template<typename...> class List,
         typename T0,
         typename T1,
         typename... Ts>
struct tl_back<List<T0, T1, Ts...>> {
    // remaining arguments are forwarded as type_list to prevent
    // recursive instantiation of List class
    using type = typename tl_back<type_list<T1, Ts...>>::type;
};

// bool empty(type_list)

/**
 * @brief Tests whether a list is empty.
 */
template<class List>
struct tl_empty {
    static constexpr bool value = std::is_same<empty_type_list, List>::value;
};

// list slice(size_t, size_t)

template<size_t LeftOffset,
         size_t Remaining,
         typename PadType,
         class List,
         typename... T>
struct tl_slice_impl {
    using type = typename tl_slice_impl<
                     LeftOffset - 1,
                     Remaining,
                     PadType,
                     typename tl_tail<List>::type,
                     T...
                 >::type;
};

template<size_t Remaining, typename PadType, class List, typename... T>
struct tl_slice_impl<0, Remaining, PadType, List, T...> {
    using type = typename tl_slice_impl<
                     0,
                     Remaining - 1,
                     PadType,
                     typename tl_tail<List>::type,
                     T...,
                     typename tl_head<List>::type
                 >::type;
};

template<size_t Remaining, typename PadType, typename... T>
struct tl_slice_impl<0, Remaining, PadType, empty_type_list, T...> {
    using type = typename tl_slice_impl<
                     0,
                     Remaining - 1,
                     PadType,
                     empty_type_list,
                     T...,
                     PadType
                 >::type;
};

template<typename PadType, class List, typename... T>
struct tl_slice_impl<0, 0, PadType, List, T...> {
    using type = type_list<T...>;
};

template<typename PadType, typename... T>
struct tl_slice_impl<0, 0, PadType, empty_type_list, T...> {
    using type = type_list<T...>;
};

template<class List, size_t ListSize, size_t First, size_t Last,
          typename PadType = unit_t>
struct tl_slice_ {
    using type = typename tl_slice_impl<
                     First,
                     (Last - First),
                     PadType,
                     List
                 >::type;
};

template<class List, size_t ListSize, typename PadType>
struct tl_slice_<List, ListSize, 0, ListSize, PadType> {
    using type = List;
};

/**
 * @brief Creates a new list from range (First, Last].
 */
template<class List, size_t First, size_t Last>
struct tl_slice {
    using type = typename tl_slice_<
                     List,
                     tl_size<List>::value,
                     (First > Last ? Last : First),
                     Last
                 >::type;
};

/**
 * @brief Zips two lists of equal size.
 *
 * Creates a list formed from the two lists @p ListA and @p ListB,
 * e.g., tl_zip<type_list<int, double>, type_list<float, string>>::type
 * is type_list<type_pair<int, float>, type_pair<double, string>>.
 */
template<class ListA,
         class ListB,
         template<typename, typename> class Fun = to_type_pair>
struct tl_zip_impl;

template<typename... LhsElements,
         typename... RhsElements,
         template<typename, typename> class Fun>
struct tl_zip_impl<type_list<LhsElements...>, type_list<RhsElements...>, Fun> {
    static_assert(sizeof...(LhsElements) == sizeof...(RhsElements),
                  "Lists have different size");
    using type = type_list<typename Fun<LhsElements, RhsElements>::type...>;

};

template<class ListA, class ListB, template<typename, typename> class Fun>
struct tl_zip {

    static constexpr size_t sizea = tl_size<ListA>::value;
    static constexpr size_t sizeb = tl_size<ListB>::value;

    static constexpr size_t result_size = (sizea < sizeb) ? sizea : sizeb;

    using type = typename tl_zip_impl<
                     typename tl_slice<ListA, 0, result_size>::type,
                     typename tl_slice<ListB, 0, result_size>::type,
                     Fun
                 >::type;

};

template<class ListA,
         class ListB,
         typename PadA = unit_t,
         typename PadB = unit_t,
         template<typename, typename> class Fun = to_type_pair>
struct tl_zip_all {

    static constexpr size_t result_size =
        (tl_size<ListA>::value > tl_size<ListB>::value) ? tl_size<ListA>::value
                                                        : tl_size<ListB>::value;

    using type = typename tl_zip_impl<
                     typename tl_slice_<
                         ListA,
                         tl_size<ListA>::value,
                         0,
                         result_size
                     >::type,
                     typename tl_slice_<
                         ListB,
                         tl_size<ListB>::value,
                         0,
                         result_size
                     >::type,
                     Fun
                 >::type;

};

template<class ListA>
struct tl_unzip;

template<typename... Elements>
struct tl_unzip<type_list<Elements...>> {
    using first = type_list<typename Elements::first...>;
    using second = type_list<typename Elements::second...>;
};

// int index_of(list, type)

template<class List, typename T>
struct tl_index_of {
    static constexpr size_t value =
        tl_index_of<typename tl_tail<List>::type, T>::value;
};

template<size_t N, typename T, typename... Ts>
struct tl_index_of<
    type_list<type_pair<std::integral_constant<size_t, N>, T>, Ts...>, T> {
    static constexpr size_t value = N;
};

// list reverse()

template<class From, typename... Elements>
struct tl_reverse_impl;

template<typename T0, typename... T, typename... E>
struct tl_reverse_impl<type_list<T0, T...>, E...> {
    using type = typename tl_reverse_impl<type_list<T...>, T0, E...>::type;
};

template<typename... E>
struct tl_reverse_impl<empty_type_list, E...> {
    using type = type_list<E...>;
};

/**
 * @brief Creates a new list wih elements in reversed order.
 */
template<class List>
struct tl_reverse {
    using type = typename tl_reverse_impl<List>::type;
};

// bool find(list, type)

/**
 * @brief Finds the first element of type @p What beginning at
 *        index @p Pos.
 */
template<class List,
         template<typename> class Predicate,
         int Pos = 0>
struct tl_find_impl;

template<template<typename...> class List,
         template<typename> class Predicate,
         int Pos>
struct tl_find_impl<List<>, Predicate, Pos> {
    static constexpr int value = -1;

};

template<template<typename...> class List,
         template<typename> class Predicate,
         int Pos,
         typename T0,
         typename... Ts>
struct tl_find_impl<List<T0, Ts...>, Predicate, Pos> {
    static constexpr int value =
        Predicate<T0>::value ?
            Pos :
            // use type_list from now on, because we don't
            // know the instantiation cost of List
            tl_find_impl<type_list<Ts...>, Predicate, Pos + 1>::value;

};

/**
 * @brief Finds the first element satisfying @p Predicate beginning at
 *        index @p Pos.
 */
template<class List, template<typename> class Predicate, int Pos = 0>
struct tl_find_if {
    static constexpr int value = tl_find_impl<List, Predicate, Pos>::value;

};

/**
 * @brief Finds the first element of type @p What beginning at
 *        index @p Pos.
 */
template<class List, typename What, int Pos = 0>
struct tl_find {
    static constexpr int value = tl_find_impl<
        List, tbind<std::is_same, What>::template type, Pos>::value;
};

// bool forall(predicate)

/**
 * @brief Tests whether a predicate holds for all elements of a list.
 */
template<class List, template<typename> class Predicate>
struct tl_forall {
    static constexpr bool value =
        Predicate<typename tl_head<List>::type>::value &&
        tl_forall<typename tl_tail<List>::type, Predicate>::value;

};

template<template<typename> class Predicate>
struct tl_forall<empty_type_list, Predicate> {
    static constexpr bool value = true;

};

template<class ListA, class ListB,
          template<typename, typename> class Predicate>
struct tl_forall2_impl {
    static constexpr bool value =
        Predicate<typename tl_head<ListA>::type,
                  typename tl_head<ListB>::type>::value &&
        tl_forall2_impl<typename tl_tail<ListA>::type,
                        typename tl_tail<ListB>::type, Predicate>::value;

};

template<template<typename, typename> class Predicate>
struct tl_forall2_impl<empty_type_list, empty_type_list, Predicate> {
    static constexpr bool value = true;

};

/**
 * @brief Tests whether a binary predicate holds for all
 *        corresponding elements of @p ListA and @p ListB.
 */
template<class ListA, class ListB,
          template<typename, typename> class Predicate>
struct tl_binary_forall {
    static constexpr bool value =
        tl_size<ListA>::value == tl_size<ListB>::value &&
        tl_forall2_impl<ListA, ListB, Predicate>::value;

};

/**
 * @brief Tests whether a predicate holds for some of the elements of a list.
 */
template<class List, template<typename> class Predicate>
struct tl_exists {
    static constexpr bool value =
        Predicate<typename tl_head<List>::type>::value ||
        tl_exists<typename tl_tail<List>::type, Predicate>::value;

};

template<template<typename> class Predicate>
struct tl_exists<empty_type_list, Predicate> {
    static constexpr bool value = false;

};

// size_t count(predicate)

/**
 * @brief Counts the number of elements in the list which satisfy a predicate.
 */
template<class List, template<typename> class Predicate>
struct tl_count {
    static constexpr size_t value =
        (Predicate<typename tl_head<List>::type>::value ? 1 : 0) +
        tl_count<typename tl_tail<List>::type, Predicate>::value;

};

template<template<typename> class Predicate>
struct tl_count<empty_type_list, Predicate> {
    static constexpr size_t value = 0;

};

// size_t count_not(predicate)

/**
 * @brief Counts the number of elements in the list which satisfy a predicate.
 */
template<class List, template<typename> class Predicate>
struct tl_count_not {
    static constexpr size_t value =
        (Predicate<typename tl_head<List>::type>::value ? 0 : 1) +
        tl_count_not<typename tl_tail<List>::type, Predicate>::value;

};

template<template<typename> class Predicate>
struct tl_count_not<empty_type_list, Predicate> {
    static constexpr size_t value = 0;

};

template<class ListA, class ListB>
struct tl_concat_impl;

/**
 * @brief Concatenates two lists.
 */
template<typename... LhsTs, typename... RhsTs>
struct tl_concat_impl<type_list<LhsTs...>, type_list<RhsTs...>> {
    using type = type_list<LhsTs..., RhsTs...>;

};

// static list concat(list, list)

/**
 * @brief Concatenates lists.
 */
template<class... Lists>
struct tl_concat;

template<class List0>
struct tl_concat<List0> {
    using type = List0;

};

template<class List0, class List1, class... Lists>
struct tl_concat<List0, List1, Lists...> {
    using type = typename tl_concat<
                     typename tl_concat_impl<List0, List1>::type,
                     Lists...
                 >::type;

};

// list push_back(list, type)

template<class List, typename What>
struct tl_push_back;

/**
 * @brief Appends @p What to given list.
 */
template<typename... ListTs, typename What>
struct tl_push_back<type_list<ListTs...>, What> {
    using type = type_list<ListTs..., What>;

};

template<class List, typename What>
struct tl_push_front;

/**
 * @brief Appends @p What to given list.
 */
template<typename... ListTs, typename What>
struct tl_push_front<type_list<ListTs...>, What> {
    using type = type_list<What, ListTs...>;

};

// list map(list, trait)

template<typename T, template<typename> class... Funs>
struct tl_apply_all;

template<typename T>
struct tl_apply_all<T> {
    using type = T;

};

template<typename T, template<typename> class Fun0,
          template<typename> class... Funs>
struct tl_apply_all<T, Fun0, Funs...> {
    using type = typename tl_apply_all<typename Fun0<T>::type, Funs...>::type;

};

/**
 * @brief Creates a new list by applying a "template function" to each element.
 */
template<class List, template<typename> class... Funs>
struct tl_map;

template<typename... Ts, template<typename> class... Funs>
struct tl_map<type_list<Ts...>, Funs...> {
    using type = type_list<typename tl_apply_all<Ts, Funs...>::type...>;

};

/**
 * @brief Creates a new list by applying a @p Fun to each element which
 *        returns @p TraitResult for @p Trait.
 */
template<class List, template<typename> class Trait, bool TRes,
          template<typename> class... Funs>
struct tl_map_conditional {
    using type = typename tl_concat<
                     type_list<
                         typename std::conditional<
                             Trait<typename tl_head<List>::type>::value == TRes,
                             typename tl_apply_all<
                                 typename tl_head<List>::type,
                                 Funs...
                             >::type,
                             typename tl_head<List>::type
                         >::type
                     >,
                     typename tl_map_conditional<
                         typename tl_tail<List>::type,
                         Trait,
                         TRes,
                         Funs...
                     >::type
                 >::type;
};

template<template<typename> class Trait, bool TraitResult,
          template<typename> class... Funs>
struct tl_map_conditional<empty_type_list, Trait, TraitResult, Funs...> {
    using type = empty_type_list;

};

// list pop_back()

/**
 * @brief Creates a new list wih all but the last element of @p List.
 */
template<class List>
struct tl_pop_back {
    using type = typename tl_slice<List, 0, tl_size<List>::value - 1>::type;

};

template<>
struct tl_pop_back<empty_type_list> {
    using type = empty_type_list;

};

// type at(size_t)

template<size_t N, typename... E>
struct tl_at_impl;

template<size_t N, typename E0, typename... E>
struct tl_at_impl<N, E0, E...> {
    using type = typename tl_at_impl<N - 1, E...>::type;

};

template<typename E0, typename... E>
struct tl_at_impl<0, E0, E...> {
    using type = E0;

};

template<size_t N>
struct tl_at_impl<N> {
    using type = unit_t; // no such element

};

template<class List, size_t N>
struct tl_at;

/**
 * @brief Gets element at index @p N of @p List.
 */
template<size_t N, typename... E>
struct tl_at<type_list<E...>, N> {
    using type = typename tl_at_impl<N, E...>::type;

};

// list prepend(type)

template<class List, typename What>
struct tl_prepend;

/**
 * @brief Creates a new list with @p What prepended to @p List.
 */
template<typename What, typename... T>
struct tl_prepend<type_list<T...>, What> {
    using type = type_list<What, T...>;

};

// list filter(predicate)
// list filter_not(predicate)

template<class List, bool... Selected>
struct tl_filter_impl;

template<>
struct tl_filter_impl<empty_type_list> {
    using type = empty_type_list;

};

template<typename T0, typename... T, bool... S>
struct tl_filter_impl<type_list<T0, T...>, false, S...> {
    using type = typename tl_filter_impl<type_list<T...>, S...>::type;

};

template<typename T0, typename... T, bool... S>
struct tl_filter_impl<type_list<T0, T...>, true, S...> {
    using type = typename tl_prepend<
                     typename tl_filter_impl<
                         type_list<T...>,
                         S...
                     >::type,
                     T0
                 >::type;
};

template<class List, template<typename> class Predicate>
struct tl_filter;

/**
 * @brief Create a new list containing all elements which satisfy @p Predicate.
 */
template<template<typename> class Predicate, typename... T>
struct tl_filter<type_list<T...>, Predicate> {
    using type = typename tl_filter_impl<
                     type_list<T...>,
                     Predicate<T>::value...
                 >::type;
};

/**
 * @brief Creates a new list containing all elements which
 *        do not satisfy @p Predicate.
 */
template<class List, template<typename> class Predicate>
struct tl_filter_not;

template<template<typename> class Predicate>
struct tl_filter_not<empty_type_list, Predicate> {
    using type = empty_type_list;
};

template<template<typename> class Predicate, typename... T>
struct tl_filter_not<type_list<T...>, Predicate> {
    using type = typename tl_filter_impl<
                     type_list<T...>,
                     !Predicate<T>::value...
                 >::type;
};

/**
 * @brief Creates a new list containing all elements which
 *        are equal to @p Type.
 */
template<class List, class Type>
struct tl_filter_type;

template<class Type, typename... T>
struct tl_filter_type<type_list<T...>, Type> {
    using type = typename tl_filter_impl<
                     type_list<T...>,
                     !std::is_same<T, Type>::value...
                 >::type;
};

/**
 * @brief Creates a new list containing all elements which
 *        are not equal to @p Type.
 */
template<class List, class Type>
struct tl_filter_not_type;

template<class Type, typename... T>
struct tl_filter_not_type<type_list<T...>, Type> {
    using type = typename tl_filter_impl<
                     type_list<T...>,
                     (!std::is_same<T, Type>::value)...
                 >::type;
};

// list distinct(list)

/**
 * @brief Creates a new list from @p List without any duplicate elements.
 */
template<class List>
struct tl_distinct;

template<>
struct tl_distinct<empty_type_list> {
    using type = empty_type_list;
};

template<typename T0, typename... Ts>
struct tl_distinct<type_list<T0, Ts...>> {
    using type = typename tl_concat<
                     type_list<T0>,
                     typename tl_distinct<
                         typename tl_filter_type<
                             type_list<Ts...>,
                             T0
                         >::type
                     >::type
                 >::type;
};

// bool is_distinct

/**
 * @brief Tests whether a list is distinct.
 */
template<class L>
struct tl_is_distinct {
    static constexpr bool value =
        tl_size<L>::value == tl_size<typename tl_distinct<L>::type>::value;
};

/**
 * @brief Creates a new list containing the last @p N elements.
 */
template<class List, size_t N>
struct tl_right {
    static constexpr size_t list_size = tl_size<List>::value;
    static constexpr size_t first_idx = (list_size > N) ? (list_size - N) : 0;
    using type = typename tl_slice<List, first_idx, list_size>::type;
};

template<size_t N>
struct tl_right<empty_type_list, N> {
    using type = empty_type_list;
};

// list resize(list, size, fill_type)

template<class List, bool OldSizeLessNewSize, size_t OldSize, size_t NewSize,
          typename FillType>
struct tl_pad_right_impl;

template<class List, size_t OldSize, size_t NewSize, typename FillType>
struct tl_pad_right_impl<List, false, OldSize, NewSize, FillType> {
    using type = typename tl_slice<List, 0, NewSize>::type;
};

template<class List, size_t Size, typename FillType>
struct tl_pad_right_impl<List, false, Size, Size, FillType> {
    using type = List;
};

template<class List, size_t OldSize, size_t NewSize, typename FillType>
struct tl_pad_right_impl<List, true, OldSize, NewSize, FillType> {
    using type = typename tl_pad_right_impl<
                     typename tl_push_back<List, FillType>::type,
                     OldSize + 1 < NewSize,
                     OldSize + 1,
                     NewSize,
                     FillType
                 >::type;
};

/**
 * @brief Resizes the list to contain @p NewSize elements and uses
 *        @p FillType to initialize the new elements with.
 */
template<class List, size_t NewSize, typename FillType = unit_t>
struct tl_pad_right {
    using type = typename tl_pad_right_impl<
                     List,
                     (tl_size<List>::value < NewSize),
                     tl_size<List>::value,
                     NewSize,
                     FillType
                 >::type;
};

// bool pad_left(list, N)

template<class List, size_t OldSize, size_t NewSize, typename FillType>
struct tl_pad_left_impl {
    using type = typename tl_pad_left_impl<
                     typename tl_push_front<
                         List,
                         FillType
                     >::type,
                     OldSize + 1,
                     NewSize,
                     FillType
                 >::type;
};

template<class List, size_t Size, typename FillType>
struct tl_pad_left_impl<List, Size, Size, FillType> {
    using type = List;

};

/**
 * @brief Resizes the list to contain @p NewSize elements and uses
 *        @p FillType to initialize prepended elements with.
 */
template<class List, size_t NewSize, typename FillType = unit_t>
struct tl_pad_left {
    static constexpr size_t list_size = tl_size<List>::value;
    using type = typename tl_pad_left_impl<
                     List,
                     list_size,
                     (list_size > NewSize) ? list_size
                                           : NewSize,
                     FillType
                 >::type;
};

// bool is_zipped(list)

template<class List>
struct tl_is_zipped {
    static constexpr bool value = tl_forall<List, is_type_pair>::value;
};

/**
 * @brief Removes trailing @p What elements from the end.
 */
template<class List, typename What = unit_t>
struct tl_trim {
    using type = typename std::conditional<
                     std::is_same<typename tl_back<List>::type, What>::value,
                     typename tl_trim<
                         typename tl_pop_back<List>::type,
                         What
                     >::type,
                     List
                 >::type;
};

template<typename What>
struct tl_trim<empty_type_list, What> {
    using type = empty_type_list;
};

// list group_by(list, predicate)

template<bool Append, typename What, class Where>
struct tl_group_by_impl_step;

template<typename What, typename... Ts>
struct tl_group_by_impl_step<true, What, type_list<Ts...>> {
    using type = type_list<type_list<Ts..., What>>;
};

template<typename What, class List>
struct tl_group_by_impl_step<false, What, List> {
    using type = type_list<List, type_list<What>>;
};

template<class In, class Out, template<typename, typename> class Predicate>
struct tl_group_by_impl {

    using last_group = typename tl_back<Out>::type;

    using suffix = typename tl_group_by_impl_step<
                       Predicate<
                           typename tl_head<In>::type,
                           typename tl_back<last_group>::type
                       >::value,
                       typename tl_head<In>::type,
                       last_group
                   >::type;

    using prefix = typename tl_pop_back<Out>::type;

    using new_out = typename tl_concat<prefix, suffix>::type;

    using type = typename tl_group_by_impl<
                     typename tl_tail<In>::type,
                     new_out,
                     Predicate
                 >::type;

};

template<template<typename, typename> class Predicate, typename T0,
          typename... Ts>
struct tl_group_by_impl<type_list<T0, Ts...>, empty_type_list, Predicate> {
    using type = typename tl_group_by_impl<
                     type_list<Ts...>,
                     type_list<type_list<T0>>,
                     Predicate
                 >::type;
};

template<class Out, template<typename, typename> class Predicate>
struct tl_group_by_impl<empty_type_list, Out, Predicate> {
    using type = Out;
};

template<class List, template<typename, typename> class Predicate>
struct tl_group_by {
    using type = typename tl_group_by_impl<
                     List,
                     empty_type_list,
                     Predicate
                 >::type;
};

/**
 * @brief Applies the types of the list to @p VarArgTemplate.
 */
template<class List, template<typename...> class VarArgTemplate>
struct tl_apply;

template<typename... Ts, template<typename...> class VarArgTemplate>
struct tl_apply<type_list<Ts...>, VarArgTemplate> {
    using type = VarArgTemplate<Ts...>;

};

// bool is_strict_subset(list,list)

template<class ListB>
struct tl_is_strict_subset_step {
    template<typename T>
    struct inner {
        using type = std::integral_constant<
                         bool,
                         tl_find<ListB, T>::value != -1
                     >;
    };
};

/**
 * @brief Tests whether ListA ist a strict subset of ListB (or equal).
 */
template<class ListA, class ListB>
struct tl_is_strict_subset {
    static constexpr bool value =
           std::is_same<ListA, ListB>::value
        || std::is_same<
               type_list<std::integral_constant<bool, true>>,
               typename tl_distinct<
                   typename tl_map<
                       ListA,
                       tl_is_strict_subset_step<ListB>::template inner
                   >::type
               >::type
           >::value;

};

/**
 * @brief Tests whether ListA contains the same elements as ListB
 *        and vice versa. This comparison ignores element positions.
 */
template<class ListA, class ListB>
struct tl_equal {
    static constexpr bool value = tl_is_strict_subset<ListA, ListB>::value &&
                                  tl_is_strict_subset<ListB, ListA>::value;

};

/**
 * @}
 */

} // namespace detail
} // namespace cppa

namespace cppa {
template<size_t N, typename... Ts>
typename detail::tl_at<detail::type_list<Ts...>, N>::type
get(const detail::type_list<Ts...>&) {
    return {};
}
} // namespace cppa

#endif // CPPA_DETAIL_TYPE_LIST_HPP
