#pragma once
#ifndef COMMON_MATH_H_
#define COMMON_MATH_H_

#include <limits>

namespace common { namespace math {

  // e
  template<typename T> constexpr T e();
  template<> constexpr double e() { return 2.71828182845904523536028747135266250; }
  template<> constexpr float e() { return static_cast<float>(e<double>()); }

  // pi
  template<typename T> constexpr T pi();
  template<> constexpr double pi() { return 3.14159265358979323846264338327950288; }
  template<> constexpr float pi() { return static_cast<float>(pi<double>()); }

  // tau = 2 * pi
  template<typename T> constexpr T tau();
  template<> constexpr double tau() { return pi<double>() * 2.0; }
  template<> constexpr float tau() { return static_cast<float>(tau<double>()); }

  // Degrees to radians conversion multiplier.
  template<typename T> constexpr T degrees_to_radians();
  template<> constexpr double degrees_to_radians() { return static_cast<float>(pi<double>() / 180.0); }
  template<> constexpr float degrees_to_radians() { return static_cast<float>(degrees_to_radians<double>()); }

} } // namespace common::math

#endif // COMMON_MATH_H_
