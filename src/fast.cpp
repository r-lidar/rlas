/*
 ===============================================================================

 PROGRAMMERS:

 info@r-lidar.com  -  https://github.com/r-lidar/rlas

 COPYRIGHT:

 Copyright 2018-2026 Jean-Romain Roussel

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

#include <Rcpp.h>

#include <algorithm>
#include <cmath>
#include <limits>

using namespace Rcpp;

// [[Rcpp::export]]
int fast_countequal(IntegerVector x, int t)
{
  return std::count(x.begin(), x.end(), t);
}

// [[Rcpp::export]]
int fast_countbelow(NumericVector x, double t)
{
  return std::count_if(x.begin(), x.end(), [t](double v) { return v < t; });
}

// [[Rcpp::export]]
int fast_countover(NumericVector x, double t)
{
  return std::count_if(x.begin(), x.end(), [t](double v) { return v > t; });
}

// Number of decimal digits of each value (at most 8)
// modified from https://stackoverflow.com/a/63215955/8442410
// [[Rcpp::export]]
IntegerVector fast_decimal_count(NumericVector x)
{
  IntegerVector y(x.size());

  for (R_xlen_t i = 0 ; i < x.size() ; i++)
  {
    constexpr double epsilon = std::numeric_limits<double>::epsilon();

    int count = 0;
    double v = std::abs(x[i]);
    double c = v - std::floor(v);
    double factor = 10;
    double eps = epsilon * c;

    while ((c > eps && c < (1 - eps)) && count < 8)
    {
      c = v * factor;
      c = c - std::floor(c);
      factor *= 10;
      eps = epsilon * v * factor;
      count++;
    }

    y[i] = count;
  }

  return y;
}
