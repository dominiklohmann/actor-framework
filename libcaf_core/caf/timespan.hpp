/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright 2011-2018 Dominik Charousset                                     *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#pragma once

#include <chrono>
#include <cstdint>
#include <limits>

namespace caf {

/// A portable timespan type with nanosecond resolution.
using timespan = std::chrono::duration<int64_t, std::nano>;

/// Constant representing an infinite amount of time.
static constexpr timespan infinite
  = timespan{std::numeric_limits<int64_t>::max()};

/// Checks whether `x` represents an infinite timespan. Timespan are considered
/// infinite if the underlying integer reached its maximum value.
template <class Rep, class Period>
constexpr bool is_infinite(std::chrono::duration<Rep, Period> x) {
  return x.count() == std::numeric_limits<int64_t>::max();
}

} // namespace caf
