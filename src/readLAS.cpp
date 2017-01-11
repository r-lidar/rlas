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

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

using namespace Rcpp;

int get_format(U8);
List vlrsreader(LASheader*);

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
List lasdatareader(CharacterVector file, bool Intensity, bool ReturnNumber, bool NumberOfReturns,
                   bool ScanDirectionFlag, bool EdgeOfFlightline, bool Classification, bool ScanAngle,
                   bool UserData, bool PointSourceID, bool RGB,
                   CharacterVector filter)
{
  try
  {
    // Initialize 0 length data
    NumericVector X,Y,Z,T;
    IntegerVector I,RN,NoR,SDF,EoF,C,SA,UD,PSI,R,G,B;

    // Cast CharacterVector into proper types
    std::string filestd   = as<std::string>(file);
    std::string filterstd = as<std::string>(filter);
    const char* filechar = filestd.c_str();
    char* filterchar = const_cast<char*>(filterstd.c_str());

    // Initialize las objects
    LASreadOpener lasreadopener;
    lasreadopener.set_file_name(filechar);
    LASreader* lasreader = lasreadopener.open();
    LASfilter lasfilter;

    if(0 == lasreader || NULL == lasreader)
      throw std::runtime_error("LASlib internal error. See message above.");

    // Read the header and test some properties of the file
    U8 point_type = lasreader->header.point_data_format;
    int format    = get_format(point_type);
    int npoints   = lasreader->header.number_of_point_records;
    bool hasrgb   = format == 2 || format == 3;
    bool hasgpst  = format == 1 || format == 3;
    bool stream   = filter.length() > 0;

    // If the user want stream the data (clip data during the reading)
    // First pass to read the whole file. Aims to know how much memory we must allocate
    if(stream)
    {
      lasfilter.parse_string(filterchar);

      npoints = 0;
      while (lasreader->read_point())
      {
        if(!lasfilter.filter(&lasreader->point))
          npoints++;
      }

      lasreader->seek(0);
    }

    // Allocate the required amount of data only if necessary
    X = NumericVector(npoints);
    Y = NumericVector(npoints);
    Z = NumericVector(npoints);

    if(hasgpst)           T   = NumericVector(npoints);
    if(Intensity)         I   = IntegerVector(npoints);
    if(ReturnNumber)      RN  = IntegerVector(npoints);
    if(NumberOfReturns)   NoR = IntegerVector(npoints);
    if(ScanDirectionFlag) SDF = IntegerVector(npoints);
    if(EdgeOfFlightline)  EoF = IntegerVector(npoints);
    if(Classification)    C   = IntegerVector(npoints);
    if(ScanAngle)         SA  = IntegerVector(npoints);
    if(UserData)          UD  = IntegerVector(npoints);
    if(PointSourceID)     PSI = IntegerVector(npoints);
    if(RGB && hasrgb)
    {
      R   = IntegerVector(npoints);
      G   = IntegerVector(npoints);
      B   = IntegerVector(npoints);
    }

    // Set data
    unsigned long int i = 0;
    while (lasreader->read_point())
    {
      if(stream && lasfilter.filter(&lasreader->point))
        continue;

      X[i]   = lasreader->point.get_x();
      Y[i]   = lasreader->point.get_y();
      Z[i]   = lasreader->point.get_z();

      if(hasgpst)           T[i]   = lasreader->point.get_gps_time();
      if(Intensity)         I[i]   = lasreader->point.get_intensity();
      if(ReturnNumber)      RN[i]  = lasreader->point.get_return_number();
      if(NumberOfReturns)   NoR[i] = lasreader->point.get_number_of_returns();
      if(ScanDirectionFlag) SDF[i] = lasreader->point.get_scan_direction_flag();
      if(EdgeOfFlightline)  EoF[i] = lasreader->point.get_edge_of_flight_line();
      if(Classification)    C[i]   = lasreader->point.get_classification();
      if(ScanAngle)         SA[i]  = lasreader->point.get_scan_angle_rank();
      if(UserData)          UD[i]  = lasreader->point.get_user_data();
      if(PointSourceID)     PSI[i] = lasreader->point.get_point_source_ID();
      if(RGB && hasrgb)
      {
        R[i]   = lasreader->point.get_R();
        G[i]   = lasreader->point.get_G();
        B[i]   = lasreader->point.get_B();
      }

      i++;
    }

    lasreader->close();
    delete lasreader;

    List lasdata = List::create(X,Y,Z);
    CharacterVector field(0);
    field.push_back("X");
    field.push_back("Y");
    field.push_back("Z");

    if(hasgpst)           lasdata.push_back(T),   field.push_back("gpstime");
    if(Intensity)         lasdata.push_back(I),   field.push_back("Intensity");
    if(ReturnNumber)      lasdata.push_back(RN),  field.push_back("ReturnNumber");
    if(NumberOfReturns)   lasdata.push_back(NoR), field.push_back("NumberOfReturns");
    if(ScanDirectionFlag) lasdata.push_back(SDF), field.push_back("ScanDirectionFlag");
    if(EdgeOfFlightline)  lasdata.push_back(EoF), field.push_back("EdgeOfFlightline");
    if(Classification)    lasdata.push_back(C),   field.push_back("Classification");
    if(ScanAngle)         lasdata.push_back(SA),  field.push_back("ScanAngle");
    if(UserData)          lasdata.push_back(UD),  field.push_back("UserData");
    if(PointSourceID)     lasdata.push_back(PSI), field.push_back("PointSourceID");
    if(RGB && hasrgb)
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

// Read header in a las or laz file
//
// Read data from a las  or laz file in format 1 to 4 according to LAS specification and return a list.
//
// This function musn't be used as is. It is an internal function. Please use \link[lidR:readLAS]{readLAS} abstraction.
//
// @param file character. the name of the file which the data are to be read from
// @return A list
// [[Rcpp::export]]
List lasheaderreader(CharacterVector file)
{
  try
  {
    std::string filestd = as<std::string>(file);

    LASreadOpener lasreadopener;
    lasreadopener.set_file_name(filestd.c_str());

    LASreader* lasreader = lasreadopener.open();
    LASheader* lasheader = &lasreader->header;

    if(0 == lasreader | NULL == lasreader)
      throw std::runtime_error("LASlib internal error. See message above.");

    List head(0);
    head.push_back(lasheader->file_signature);
    head.push_back(lasheader->file_source_ID);
    head.push_back(lasheader->global_encoding);
    head.push_back(0);
    head.push_back(lasheader->version_major);
    head.push_back(lasheader->version_minor);
    head.push_back(lasheader->system_identifier);
    head.push_back(lasheader->generating_software);
    head.push_back(lasheader->file_creation_day);
    head.push_back(lasheader->file_creation_year);
    head.push_back(lasheader->header_size);
    head.push_back(lasheader->offset_to_point_data);
    head.push_back(lasheader->number_of_variable_length_records);
    head.push_back(lasheader->point_data_format);
    head.push_back(lasheader->point_data_record_length);
    head.push_back(lasheader->number_of_point_records);
    head.push_back(lasheader->number_of_points_by_return[0]);
    head.push_back(lasheader->number_of_points_by_return[1]);
    head.push_back(lasheader->number_of_points_by_return[2]);
    head.push_back(lasheader->number_of_points_by_return[3]);
    head.push_back(lasheader->number_of_points_by_return[4]);
    head.push_back(lasheader->x_scale_factor);
    head.push_back(lasheader->y_scale_factor);
    head.push_back(lasheader->z_scale_factor);
    head.push_back(lasheader->x_offset);
    head.push_back(lasheader->y_offset);
    head.push_back(lasheader->z_offset);
    head.push_back(lasheader->max_x);
    head.push_back(lasheader->min_x);
    head.push_back(lasheader->max_y);
    head.push_back(lasheader->min_y);
    head.push_back(lasheader->max_z);
    head.push_back(lasheader->min_z);
    head.push_back(vlrsreader(lasheader));

    lasreader->close();
    delete lasreader;

    CharacterVector names(0);
    names.push_back("File Signature");
    names.push_back("File Source ID");
    names.push_back("Global Encoding");
    names.push_back("Project ID - GUID");
    names.push_back("Version Major");
    names.push_back("Version Minor");
    names.push_back("System Identifier");
    names.push_back("Generating Software");
    names.push_back("File Creation Day of Year");
    names.push_back("File Creation Year");
    names.push_back("Header Size");
    names.push_back("Offset to point data");
    names.push_back("Number of variable length records");
    names.push_back("Point Data Format ID");
    names.push_back("Point Data Record Length");
    names.push_back("Number of point records");
    names.push_back("Number of 1st return");
    names.push_back("Number of 2nd return");
    names.push_back("Number of 3rd return");
    names.push_back("Number of 4th return");
    names.push_back("Number of 5th return");
    names.push_back("X scale factor");
    names.push_back("Y scale factor");
    names.push_back("Z scale factor");
    names.push_back("X offset");
    names.push_back("Y offset");
    names.push_back("Z offset");
    names.push_back("Max X");
    names.push_back("Min X");
    names.push_back("Max Y");
    names.push_back("Min Y");
    names.push_back("Max Z");
    names.push_back("Min Z");
    names.push_back("Variable Length Records");

    head.names() = names;

    return(head);
  }
  catch (std::exception const& e)
  {
    Rcerr << "Error: " << e.what() << std::endl;
    return(List(0));
  }
}

List vlrsreader(LASheader* lasheader)
{
  int nvlrs = (int)lasheader->number_of_variable_length_records;

  List lvlrs;
  List lvlrsnames;

  for (int i = 0; i < nvlrs; i++)
  {
    LASvlr vlr = lasheader->vlrs[i];

    List lvlr      = List::create(vlr.reserved, vlr.user_id, vlr.record_id, vlr.record_length_after_header, vlr.description);
    List lvlrnames = List::create("reserved", "user ID", "record ID", "length after header", "description");

    if (strcmp(vlr.user_id, "LASF_Projection") == 0 && vlr.data != 0)
    {
      if (vlr.record_id == 34735) // GeoKeyDirectoryTag
      {
        List GeoKeys(0);

        for (int j = 0; j < lasheader->vlr_geo_keys->number_of_keys; j++)
        {
          List GeoKey = List::create(Named("key") = lasheader->vlr_geo_key_entries[j].key_id,
                                     Named("tiff tag location") =  lasheader->vlr_geo_key_entries[j].tiff_tag_location,
                                     Named("count") = lasheader->vlr_geo_key_entries[j].count,
                                     Named("value offset") = lasheader->vlr_geo_key_entries[j].value_offset);

          GeoKeys.push_back(GeoKey);
        }

        lvlr.push_back(GeoKeys);
        lvlrnames.push_back("tags");
        lvlrsnames.push_back("GeoKeyDirectoryTag");
      }
      else if (vlr.record_id == 34736) // GeoDoubleParamsTag
      {
        int n = vlr.record_length_after_header/8;
        NumericVector GeoDouble(n);

        for (int j = 0; j < n; j++)
        {
          GeoDouble(j) = lasheader->vlr_geo_double_params[j];
        }

        lvlr.push_back(GeoDouble);
        lvlrnames.push_back("tags");
        lvlrsnames.push_back("GeoDoubleParamsTag");
      }
      else if (vlr.record_id == 34737) // GeoAsciiParamsTag
      {
        lvlr.push_back(SSTR(vlr.data));
        lvlrnames.push_back("tags");
        lvlrsnames.push_back("GeoAsciiParamsTag");
      }
      else if (vlr.record_id == 2111) // WKT OGC MATH TRANSFORM
      {
        lvlr.push_back(SSTR(vlr.data));
        lvlrnames.push_back("WKT OGC MATH TRANSFORM");
        lvlrsnames.push_back("WKT OGC MT");
      }
      else if (vlr.record_id == 2112) // WKT OGC COORDINATE SYSTEM
      {
        lvlr.push_back(SSTR(vlr.data));
        lvlrnames.push_back("WKT OGC COORDINATE SYSTEM");
        lvlrsnames.push_back("WKT OGC CS");
      }
    }
    else if ((strcmp(vlr.user_id, "LASF_Spec") == 0) && (vlr.data != 0))
    {
      // not supported yet
      lvlrsnames.push_back(vlr.user_id);
    }
    else
    {
      // not supported yet
      lvlrsnames.push_back(vlr.user_id);
    }

    lvlr.names() = lvlrnames;
    lvlrs.push_back(lvlr);
  }

  lvlrs.names() = lvlrsnames;
  return lvlrs;
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

// [[Rcpp::export]]
void lasfilterusage()
{
  try
  {
    LASfilter filter;
    filter.usage();
  }
  catch (std::exception const& e)
  {
    Rcerr << "Error: " << e.what() << std::endl;
  }
}