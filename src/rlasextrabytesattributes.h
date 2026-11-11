#ifndef LASEXTRABYTESATTRIBUTES_H
#define LASEXTRABYTESATTRIBUTES_H

#include <Rcpp.h>

#include <string>
#include <vector>

#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lasfilter.hpp"

// An extra bytes attribute, used both to read it from a file into R and to write it from R into a file.
//
// 'data_type' follows the LAS specification (1 = unsigned char, 2 = char, ..., 9 = float, 10 = double).
// Integer types up to 32 bits without scale nor offset fit in an R integer, others are stored as R double.
class RLASExtrabyteAttributes
{
public:
  int id         = 0;      // Specific to LASlib
  int start      = 0;      // Specific to LASlib: position of the attribute in the extra bytes of a point
  int data_type  = 0;      // All the following are defined in the LAS specifications
  int options    = 0;
  bool has_scale   = false;
  bool has_offset  = false;
  bool has_no_data = false;
  bool has_min     = false;
  bool has_max     = false;
  double scale   = 1;
  double offset  = 0;
  double no_data = 0;
  double min     = 0;
  double max     = 0;
  std::string name;
  std::string desc;
  std::vector<int> eb32;    // Values read from a file that fit in an R signed int
  std::vector<double> eb64; // Values read from a file that fit in an R double
  Rcpp::NumericVector Reb;  // Values read from R. Always casted to double before being written

public:
  bool is_supported() const { return data_type <= 10; }
  bool is_32bits() const    { return data_type <= 6 && !(has_scale || has_offset); }
  void parse_options();               // Interprets 'options' as a set of bits according to the specification

  // Reading
  void reserve(size_t n);
  void push_back(const LASpoint*);    // Reads the value of a point into eb32 or eb64
  SEXP to_R() const;                  // eb32 or eb64 as an R vector

  // Writing
  void set_attribute(int i, LASpoint*); // Writes the ith value of Reb into a point
  LASattribute make_LASattribute() const;

private:
  F64 get_attribute_double(const LASpoint*) const;
  I32 get_attribute_int(const LASpoint*) const;
};

#endif //LASEXTRABYTESATTRIBUTES_H
