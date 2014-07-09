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

#ifndef CPPA_MIXED_HPP
#define CPPA_MIXED_HPP

namespace cppa {

namespace detail {

template<class D, class B, template<class, class> class... Ms>
struct extend_helper;

template<class D, class B>
struct extend_helper<D, B> {
    using type = B;

};

template<class D, class B, template<class, class> class M,
          template<class, class> class... Ms>
struct extend_helper<D, B, M, Ms...> : extend_helper<D, M<B, D>, Ms...> {};

} // namespace detail

/**
 * @brief Allows convenient definition of types using mixins.
 *        For example, "extend<ar, T>::with<ob, fo>" is an alias for
 *        "fo<ob<ar, T>, T>".
 *
 * Mixins always have two template parameters: base type and
 * derived type. This allows mixins to make use of the curiously recurring
 * template pattern (CRTP). However, if none of the used mixins use CRTP,
 * the second template argument can be ignored (it is then set to Base).
 */
template<class Base, class Derived = Base>
struct extend {
    /**
     * @brief Identifies the combined type.
     */
    template<template<class, class> class... Mixins>
    using with = typename detail::extend_helper<Derived, Base, Mixins...>::type;

};

} // namespace cppa

#endif // CPPA_MIXED_HPP
