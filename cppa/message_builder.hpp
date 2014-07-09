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

#ifndef CPPA_OBJECT_ARRAY_HPP
#define CPPA_OBJECT_ARRAY_HPP

#include <vector>

#include "cppa/message.hpp"
#include "cppa/message_handler.hpp"
#include "cppa/uniform_type_info.hpp"

#include "cppa/detail/message_data.hpp"

namespace cppa {

/**
 * @brief Provides a convenient interface to create a {@link message}
 *        from a series of values using the member function @p append.
 */
class message_builder {

    message_builder(const message_builder&) = delete;

    message_builder& operator=(const message_builder&) = delete;

 public:

    message_builder();

    /**
     * @brief Creates a new instance and immediately calls
     *        <tt>append(first, last)</tt>.
     */
    template<typename Iter>
    message_builder(Iter first, Iter last) {
        init();
        append(first, last);
    }

    ~message_builder();

    /**
     * @brief Adds @p what to the elements of the internal buffer.
     */
    message_builder& append(uniform_value what);

    /**
     * @brief Appends all values in range [first, last).
     */
    template<typename Iter>
    message_builder& append(Iter first, Iter last) {
        using vtype = typename detail::rm_const_and_ref<decltype(*first)>::type;
        using converted = typename detail::implicit_conversions<vtype>::type;
        auto uti = uniform_typeid<converted>();
        for (; first != last; ++first) {
            auto uval = uti->create();
            *reinterpret_cast<converted*>(uval->val) = *first;
            append(std::move(uval));
        }
        return *this;
    }

    /**
     * @brief Adds @p what to the elements of the internal buffer.
     */
    template<typename T>
    message_builder& append(T what) {
        return append_impl<T>(std::move(what));
    }

    /**
     * @brief Converts the internal buffer to an actual message object.
     *
     * It is worth mentioning that a call to @p to_message does neither
     * invalidate the @p message_builder instance nor clears the internal
     * buffer. However, calling any non-const member function afterwards
     * can cause the @p message_builder to detach its data, i.e.,
     * copy it if there is more than one reference to it.
     */
    message to_message();

    /**
     * @brief Convenience function for <tt>to_message().apply(handler)</tt>.
     */
    inline optional<message> apply(message_handler handler) {
        return to_message().apply(std::move(handler));
    }

    /**
     * @brief Removes all elements from the internal buffer.
     */
    void clear();

    /**
     * @brief Returns whether the internal buffer is empty.
     */
    bool empty() const;

    /**
     * @brief Returns the number of elements in the internal buffer.
     */
    size_t size() const;

 private:

    void init();

    template<typename T>
    message_builder&
    append_impl(typename detail::implicit_conversions<T>::type what) {
        using type = decltype(what);
        auto uti = uniform_typeid<type>();
        auto uval = uti->create();
        *reinterpret_cast<type*>(uval->val) = std::move(what);
        return append(std::move(uval));
    }

    class dynamic_msg_data;

    dynamic_msg_data* data();

    const dynamic_msg_data* data() const;

    intrusive_ptr<ref_counted> m_data; // hide dynamic_msg_data implementation

};

} // namespace cppa

#endif // CPPA_OBJECT_ARRAY_HPP
