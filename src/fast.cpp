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
