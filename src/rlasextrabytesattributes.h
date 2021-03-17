#ifndef LASEXTRABYTESATTRIBUTES_H
#define LASEXTRABYTESATTRIBUTES_H

#include <Rcpp.h>
#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lasfilter.hpp"

class RLASExtrabyteAttributes
{
public:
  int id;                            // Specific to LASlib
  int start;                         // Specific to LASlib
  int data_type;                     // All the following are defined in the LAS specifications
  int options;
  bool has_scale;
  bool has_offset;
  bool has_no_data;
  bool has_min;
  bool has_max;
  double scale;
  double offset;
  double no_data;
  double min;
  double max;
  std::string name;
  std::string desc;
  std::vector<int> eb32;              // Stores data read from file that fits in a R signed int
  std::vector<double> eb64;           // Stores data read from file that fits in a R signed double
  Rcpp::NumericVector Reb;            // Stores data read from R. Always casted to double before to be witten

public:
  RLASExtrabyteAttributes();
  bool is_supported();                // Test if the data_type is supporte by rlas
  bool is_32bits();                   // Test if the data_type fits in a R signed int r a R signed double
  void push_back(LASpoint*);          // Push and extrabytes value either into eb32 or eb64.
  void parse_options();               // Interpret the int as a set of bit according to the specification
  void set_attribute(int, LASpoint*); // Update a LASpoint by attibuting the ith value of the extrabytes attribute
  LASattribute make_LASattribute();   // Create a LASattribute from RLASExtrabytesAttribute

private:
  F64 get_attribute_double(LASpoint*);
  I32 get_attribute_int(LASpoint*);
};

#endif //LASEXTRABYTESATTRIBUTES_H
