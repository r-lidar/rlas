#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
int fast_countequal(IntegerVector x, int t)
{
  return std::count(x.begin(), x.end(), t);
}

// [[Rcpp::export]]
int fast_countbelow(NumericVector x, double t)
{
  return std::count_if(x.begin(), x.end(), std::bind(std::less<double>(), std::placeholders::_1, t));
}

// [[Rcpp::export]]
int fast_countover(NumericVector x, double t)
{
  return std::count_if(x.begin(), x.end(), std::bind(std::greater<double>(), std::placeholders::_1, t));
}
