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

// [[Rcpp::export]]
IntegerVector fast_decimal_count(NumericVector x)
{
  IntegerVector y(x.size());

  for (auto i = 0 ; i < x.size() ; i++)
  {
    // modified from https://stackoverflow.com/a/63215955/8442410
    unsigned int count = 0;
    double v = std::abs(x[i]);
    double c = v - std::floor(v);
    double factor = 10;
    double eps = std::numeric_limits<double>::epsilon() * c;

    while ((c > eps && c < (1 - eps)) && count < 8)
    {
      c = v * factor;
      c = c - std::floor(c);
      factor *= 10;
      eps = std::numeric_limits<double>::epsilon() * v * factor;
      count++;
    }

    y[i] = count;
  }

  return y;
}
