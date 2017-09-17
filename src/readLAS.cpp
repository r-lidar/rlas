/*
===============================================================================

PROGRAMMERS:

jean-romain.roussel.1@ulaval.ca  -  https://github.com/Jean-Romain/rlas

COPYRIGHT:

Copyright 2016 Jean-Romain Roussel

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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lasfilter.hpp"

using namespace Rcpp;

int get_format(U8);

// Read data from a las and laz file with LASlib
//
// Read data from a las or laz file in format 1 to 4 according to LAS specification and return a list.
//
// This function musn't be used as is. It is an internal function. Please use \link[lidR:readLAS]{readLAS} abstraction.
//
// @param file character. The name of the file which the data are to be read from
// @param Intensity logical. do you want to load Intensity field? default: TRUE
// @param ReturnNumber logical. do you want to load ReturnNumber field? default: TRUE
// @param NumberOfReturns logical. do you want to load NumberOfReturns field? default: TRUE
// @param ScanDirectionFlag logical. do you want to load ScanDirectionFlag field? default: FALSE
// @param EdgeOfFlightline logical. do you want to load EdgeofFlightline field? default: FALSE
// @param Classification logical. do you want to load Classification field? default: TRUE
// @param ScanAngle logical. do you want to load intensity field? default: TRUE
// @param UserData logical. do you want to load UserData field? default: FALSE
// @param PointSourceID logical. do you want to load PointSourceID field? default: FALSE
// @param RGB logical. do you want to load intensity R,G and B? default: TRUE
//
// @return list
// [[Rcpp::export]]
List lasdatareader(CharacterVector files, bool i, bool r, bool n, bool d, bool e, bool c, bool a, bool u, bool p, bool RGB, bool t)
{
  try
  {
    // Initialize 0 length data
    NumericVector X,Y,Z,T;
    IntegerVector I,RN,NoR,SDF,EoF,C,SA,UD,PSI,R,G,B;

    // Initialize reader
    LASreadOpener lasreadopener;
    lasreadopener.set_merged(true);
    lasreadopener.set_populate_header(true);

    for (int j = 0; j < files.length(); j++)
    {
      std::string filestd   = as<std::string>(files[j]);
      const char* filechar = filestd.c_str();
      lasreadopener.add_file_name(filechar);
    }

    LASreader* lasreader = lasreadopener.open();

    if(0 == lasreader || NULL == lasreader)
      throw std::runtime_error("LASlib internal error. See message above.");

    // Read the header and test some properties of the file
    U8 point_type = lasreader->header.point_data_format;
    int format    = get_format(point_type);
    int npoints   = lasreader->header.number_of_point_records;
    bool hasrgb   = format == 2 || format == 3;
    bool hast    = format == 1 || format == 3;

    t = t && hast;
    RGB = RGB && hasrgb;

    // Allocate the required amount of data only if necessary
    X = NumericVector(npoints);
    Y = NumericVector(npoints);
    Z = NumericVector(npoints);

    if(t) T   = NumericVector(npoints);
    if(i) I   = IntegerVector(npoints);
    if(r) RN  = IntegerVector(npoints);
    if(n) NoR = IntegerVector(npoints);
    if(d) SDF = IntegerVector(npoints);
    if(e) EoF = IntegerVector(npoints);
    if(c) C   = IntegerVector(npoints);
    if(a) SA  = IntegerVector(npoints);
    if(u) UD  = IntegerVector(npoints);
    if(p) PSI = IntegerVector(npoints);
    if(RGB)
    {
      R   = IntegerVector(npoints);
      G   = IntegerVector(npoints);
      B   = IntegerVector(npoints);
    }

    // Set data
    unsigned long int k = 0;
    while (lasreader->read_point())
    {
      X[k] = lasreader->point.get_x();
      Y[k] = lasreader->point.get_y();
      Z[k] = lasreader->point.get_z();

      if(t) T[k]   = lasreader->point.get_gps_time();
      if(i) I[k]   = lasreader->point.get_intensity();
      if(r) RN[k]  = lasreader->point.get_return_number();
      if(n) NoR[k] = lasreader->point.get_number_of_returns();
      if(d) SDF[k] = lasreader->point.get_scan_direction_flag();
      if(e) EoF[k] = lasreader->point.get_edge_of_flight_line();
      if(c) C[k]   = lasreader->point.get_classification();
      if(a) SA[k]  = lasreader->point.get_scan_angle_rank();
      if(u) UD[k]  = lasreader->point.get_user_data();
      if(p) PSI[k] = lasreader->point.get_point_source_ID();
      if(RGB)
      {
        R[k]   = lasreader->point.get_R();
        G[k]   = lasreader->point.get_G();
        B[k]   = lasreader->point.get_B();
      }

      k++;
    }

    lasreader->close();
    delete lasreader;

    List lasdata = List::create(X,Y,Z);
    CharacterVector field(0);
    field.push_back("X");
    field.push_back("Y");
    field.push_back("Z");

    if(t) lasdata.push_back(T),   field.push_back("gpstime");
    if(i) lasdata.push_back(I),   field.push_back("Intensity");
    if(r) lasdata.push_back(RN),  field.push_back("ReturnNumber");
    if(n) lasdata.push_back(NoR), field.push_back("NumberOfReturns");
    if(d) lasdata.push_back(SDF), field.push_back("ScanDirectionFlag");
    if(e) lasdata.push_back(EoF), field.push_back("EdgeOfFlightline");
    if(c) lasdata.push_back(C),   field.push_back("Classification");
    if(a) lasdata.push_back(SA),  field.push_back("ScanAngle");
    if(u) lasdata.push_back(UD),  field.push_back("UserData");
    if(p) lasdata.push_back(PSI), field.push_back("PointSourceID");
    if(RGB)
    {
      lasdata.push_back(R), field.push_back("R");
      lasdata.push_back(G), field.push_back("G");
      lasdata.push_back(B), field.push_back("B");
    }

    lasdata.names() = field;

    return(lasdata);
  }
  catch (std::exception const& e)
  {
    Rcerr << "Error: " << e.what() << std::endl;
    return(List(0));
  }
}

// [[Rcpp::export]]
List lasdatastreamer(CharacterVector ifiles, std::string ofile, std::string filter,
                     bool i, bool r, bool n, bool d, bool e, bool c, bool a, bool u, bool p, bool RGB, bool t)
{
  try
  {
    // Initialize 0 length data
    std::vector<double> X,Y,Z,T;
    std::vector<long> I,RN,NoR,SDF,EoF,C,SA,UD,PSI,R,G,B;

    bool inmemory = ofile == "";

    // Cast string into char*
    const char* ofilechar  = ofile.c_str();
    char* filterchar = const_cast<char*>(filter.c_str());

    // Create reader and writer
    LASreadOpener  lasreadopener;
    LASwriteOpener laswriteopener;

    // Initialize reader
    lasreadopener.set_merged(true);
    lasreadopener.set_populate_header(true);

    for (int j = 0; j < ifiles.length(); j++)
    {
      std::string filestd  = as<std::string>(ifiles[j]);
      const char* filechar = filestd.c_str();
      lasreadopener.add_file_name(filechar);
    }

    lasreadopener.parse_str(filterchar);
    LASreader* lasreader = lasreadopener.open();
    LASheader* header = &lasreader->header;

    if(0 == lasreader || NULL == lasreader)
      throw std::runtime_error("LASlib internal error. See message above.");

    // Initialize writer
    LASwriter* laswriter;

    if (!inmemory)
    {
      laswriteopener.set_file_name(ofilechar);

      laswriter = laswriteopener.open(header);

      if(0 == lasreader || NULL == lasreader)
        throw std::runtime_error("LASlib internal error. See message above.");
    }

    // Read the header and test some properties of the file
    U8 point_type = lasreader->header.point_data_format;
    int format    = get_format(point_type);
    int npoints   = lasreader->header.number_of_point_records;
    bool hasrgb   = format == 2 || format == 3;
    bool hast  = format == 1 || format == 3;

    t = t && hast;
    RGB = RGB && hasrgb;

    // If the user want to load data in R, then reserve memory
    if(inmemory)
    {
      int nalloc = ceil((float)npoints/8);

      // Allocate the required amount of data only if necessary
      X.reserve(nalloc);
      Y.reserve(nalloc);
      Z.reserve(nalloc);

      if(t) T.reserve(nalloc);
      if(i) I.reserve(nalloc);
      if(r) RN.reserve(nalloc);
      if(n) NoR.reserve(nalloc);
      if(d) SDF.reserve(nalloc);
      if(e) EoF.reserve(nalloc);
      if(c) C.reserve(nalloc);
      if(a) SA.reserve(nalloc);
      if(u) UD.reserve(nalloc);
      if(p) PSI.reserve(nalloc);
      if(RGB)
      {
        R.reserve(nalloc);
        G.reserve(nalloc);
        B.reserve(nalloc);
      }
    }

    // Set data
    while (lasreader->read_point())
    {
      if (inmemory)
      {
        X.push_back(lasreader->point.get_x());
        Y.push_back(lasreader->point.get_y());
        Z.push_back(lasreader->point.get_z());

        if(t) T.push_back(lasreader->point.get_gps_time());
        if(i) I.push_back(lasreader->point.get_intensity());
        if(r) RN.push_back(lasreader->point.get_return_number());
        if(n) NoR.push_back(lasreader->point.get_number_of_returns());
        if(d) SDF.push_back(lasreader->point.get_scan_direction_flag());
        if(e) EoF.push_back(lasreader->point.get_edge_of_flight_line());
        if(c) C.push_back(lasreader->point.get_classification());
        if(a) SA.push_back(lasreader->point.get_scan_angle_rank());
        if(u) UD.push_back(lasreader->point.get_user_data());
        if(p) PSI.push_back(lasreader->point.get_point_source_ID());
        if(RGB)
        {
          R.push_back(lasreader->point.get_R());
          G.push_back(lasreader->point.get_G());
          B.push_back(lasreader->point.get_B());
        }
      }
      else
      {
        laswriter->write_point(&lasreader->point);
        laswriter->update_inventory(&lasreader->point);
      }
    }

    lasreader->close();
    delete lasreader;

    if(!inmemory)
    {
      laswriter->update_header(header, true);
      laswriter->close();
      delete laswriter;

      return List(0);
    }
    else
    {
      List lasdata = List::create(X,Y,Z);
      CharacterVector field(0);
      field.push_back("X");
      field.push_back("Y");
      field.push_back("Z");

      if(t) lasdata.push_back(T),   field.push_back("gpstime");
      if(i) lasdata.push_back(I),   field.push_back("Intensity");
      if(r) lasdata.push_back(RN),  field.push_back("ReturnNumber");
      if(n) lasdata.push_back(NoR), field.push_back("NumberOfReturns");
      if(d) lasdata.push_back(SDF), field.push_back("ScanDirectionFlag");
      if(e) lasdata.push_back(EoF), field.push_back("EdgeOfFlightline");
      if(c) lasdata.push_back(C),   field.push_back("Classification");
      if(a) lasdata.push_back(SA),  field.push_back("ScanAngle");
      if(u) lasdata.push_back(UD),  field.push_back("UserData");
      if(p) lasdata.push_back(PSI), field.push_back("PointSourceID");
      if(RGB)
      {
        lasdata.push_back(R), field.push_back("R");
        lasdata.push_back(G), field.push_back("G");
        lasdata.push_back(B), field.push_back("B");
      }

      lasdata.names() = field;

      return(lasdata);
    }
  }
  catch (std::exception const& e)
  {
    Rcerr << "Error: " << e.what() << std::endl;
    return(List(0));
  }
}

int get_format(U8 point_type)
{
  int format;

  switch (point_type)
  {
  case 0:
    format = 0;
    break;
  case 1:
    format = 1;
    break;
  case 2:
    format = 2;
    break;
  case 3:
    format = 3;
    break;
  case 4:
    throw std::runtime_error("LAS format not yet supported");
    break;
  case 5:
    throw std::runtime_error("LAS format not yet supported");
    break;
  case 6:
    throw std::runtime_error("LAS format not yet supported");
    break;
  case 7:
    throw std::runtime_error("LAS format not yet supported");
    break;
  case 8:
    throw std::runtime_error("LAS format not yet supported");
    break;
  case 9:
    throw std::runtime_error("LAS format not yet supported");
    break;
  case 10:
    throw std::runtime_error("LAS format not yet supported");
    break;
  default:
    throw std::runtime_error("LAS format not valid");
  }

  return(format);
}