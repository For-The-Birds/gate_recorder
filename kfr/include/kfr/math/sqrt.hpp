/** @addtogroup basic_math
 *  @{
 */
/*
  Copyright (C) 2016 D Levin (https://www.kfrlib.com)
  This file is part of KFR

  KFR is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  KFR is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with KFR.

  If GPL is not suitable for your project, you must purchase a commercial license to use KFR.
  Buying a commercial license is mandatory as soon as you develop commercial activities without
  disclosing the source code of your own applications.
  See https://www.kfrlib.com for details.
 */
#pragma once

#include "impl/sqrt.hpp"

namespace kfr
{
inline namespace CMT_ARCH_NAME
{

/**
 * @brief Returns the positive square root of the x. \f$\sqrt{x}\f$
 */
template <typename T1, KFR_ENABLE_IF(is_numeric<T1>)>
KFR_INTRINSIC flt_type<T1> sqrt(const T1& x)
{
    return intrinsics::sqrt(x);
}

/**
 * @brief Returns template expression that returns the positive square root of the x. \f$\sqrt{x}\f$
 */
template <typename E1, KFR_ENABLE_IF(is_input_expression<E1>)>
KFR_INTRINSIC internal::expression_function<fn::sqrt, E1> sqrt(E1&& x)
{
    return { fn::sqrt(), std::forward<E1>(x) };
}
} // namespace CMT_ARCH_NAME
} // namespace kfr
