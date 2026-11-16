/*
 ===============================================================================

 PROGRAMMERS:

 info@r-lidar.com  -  https://github.com/r-lidar/rlas

 COPYRIGHT:

 Copyright 2019-2026 Jean-Romain Roussel

 This file is part of rlas R package.

 rlas is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>

 ===============================================================================
 */

#include "rlasextrabytesattributes.h"

#include <cstring>
#include <stdexcept>
#include <type_traits>

namespace
{
  // Calls f(T{}) where T is the C++ type of the LAS extra bytes data type 'data_type'
  // (1 = U8 ... 10 = F64). Returns false if the data type is not a known scalar type.
  template <typename F>
  bool dispatch(int data_type, F&& f)
  {
    switch (data_type)
    {
      case 1:  f(U8{});  return true;
      case 2:  f(I8{});  return true;
      case 3:  f(U16{}); return true;
      case 4:  f(I16{}); return true;
      case 5:  f(U32{}); return true;
      case 6:  f(I32{}); return true;
      case 7:  f(U64{}); return true;
      case 8:  f(I64{}); return true;
      case 9:  f(F32{}); return true;
      case 10: f(F64{}); return true;
      default: return false;
    }
  }

  // Reads a value of type T stored at 'ptr' (not necessarily aligned)
  template <typename T>
  T load(const U8* ptr)
  {
    T value;
    std::memcpy(&value, ptr, sizeof(T));
    return value;
  }

  // Converts a double into the type T of an attribute, rounding and clamping like LASlib does
  template <typename T>
  T encode(double v)
  {
    if constexpr (std::is_same_v<T, U8>)  return U8_CLAMP(U8_QUANTIZE(v));
    if constexpr (std::is_same_v<T, I8>)  return I8_CLAMP(I8_QUANTIZE(v));
    if constexpr (std::is_same_v<T, U16>) return U16_CLAMP(U16_QUANTIZE(v));
    if constexpr (std::is_same_v<T, I16>) return I16_CLAMP(I16_QUANTIZE(v));
    if constexpr (std::is_same_v<T, U32>) return U32_CLAMP(U32_QUANTIZE(v));
    if constexpr (std::is_same_v<T, I32>) return I32_CLAMP(I32_QUANTIZE(v));
    if constexpr (std::is_same_v<T, U64>) return U64_QUANTIZE(v);
    if constexpr (std::is_same_v<T, I64>) return I64_QUANTIZE(v);
    if constexpr (std::is_same_v<T, F32>) return (F32)v;
    if constexpr (std::is_same_v<T, F64>) return v;
  }
}

void RLASExtrabyteAttributes::parse_options()
{
  has_no_data = options & 0x01;
  has_min     = options & 0x02;
  has_max     = options & 0x04;
  has_scale   = options & 0x08;
  has_offset  = options & 0x10;
}

void RLASExtrabyteAttributes::reserve(size_t n)
{
  if (is_32bits())
    eb32.reserve(n);
  else
    eb64.reserve(n);
}

void RLASExtrabyteAttributes::push_back(const LASpoint* point)
{
  if (is_32bits())
    eb32.push_back(get_attribute_int(point));
  else
    eb64.push_back(get_attribute_double(point));
}

SEXP RLASExtrabyteAttributes::to_R() const
{
  if (is_32bits())
    return Rcpp::wrap(eb32);
  else
    return Rcpp::wrap(eb64);
}

F64 RLASExtrabyteAttributes::get_attribute_double(const LASpoint* point) const
{
  const U8* ptr = point->extra_bytes + start;
  F64 value = 0;

  bool known = dispatch(data_type, [&](auto tag)
  {
    using T = decltype(tag);
    if constexpr (std::is_same_v<T, U64>)
      value = (F64)(I64)load<U64>(ptr); // R has no 64 bits integers
    else
      value = (F64)load<T>(ptr);
  });

  if (!known)
    throw std::runtime_error("LAS Extra Byte data data_type not supported.");

  if (applies_scale() || applies_offset())
    value = scale*value + offset;

  if (has_no_data && value == no_data)
    value = NA_REAL;

  return value;
}

I32 RLASExtrabyteAttributes::get_attribute_int(const LASpoint* point) const
{
  if (data_type < 1 || data_type > 6)
    throw std::runtime_error("LAS Extra Byte data data_type not supported in I32.");

  const U8* ptr = point->extra_bytes + start;
  I32 value = 0;
  dispatch(data_type, [&](auto tag) { value = (I32)load<decltype(tag)>(ptr); });

  if (has_no_data && value == no_data)
    value = NA_INTEGER;

  return value;
}

void RLASExtrabyteAttributes::set_attribute(int i, LASpoint* p)
{
  double value;

  if (has_no_data && Rcpp::NumericVector::is_na(Reb[i]))
    value = no_data;
  else
    value = (Reb[i] - offset)/scale;

  dispatch(data_type, [&](auto tag)
  {
    using T = decltype(tag);
    p->set_attribute(start, encode<T>(value));
  });
}

LASattribute RLASExtrabyteAttributes::make_LASattribute() const
{
  // LASattribute takes the data type 0-based
  LASattribute attribute(data_type - 1, name.c_str(), desc.c_str());

  if (has_scale)
    attribute.set_scale(scale);

  if (has_offset)
    attribute.set_offset(offset);

  dispatch(data_type, [&](auto tag)
  {
    using T = decltype(tag);
    if (has_no_data) attribute.set_no_data(encode<T>(no_data));
    if (has_min)     attribute.set_min(encode<T>(min));
    if (has_max)     attribute.set_max(encode<T>(max));
  });

  return attribute;
}
