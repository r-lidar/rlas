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

#include "laswriter.hpp"

using namespace Rcpp;

// Write a las file with LASlib
//
// Methods to write las files using LASlib
//
// This function musn't be used as is. It is an internal function. Please use \link[lidR:writeLAS]{writeLAS} abstraction.
//
// @param file character. filename of .las file
// @param LASheader list a header from a LAS object
// @param X numeric array X data
// @param Y numeric array Y data
// @param Z numeric array Z data
// @param ExtraBytes numeric data frame Extra Bytes data
// @param I integer array intensity data
// @param RN integer array return number data
// @param NoR integer array number of returns data
// @param SDF integer array scan direction flag data
// @param EoF integer array edge of flightline data
// @param C integer array classification data
// @param SA integer array scan angle data
// @param UD integer array user data data
// @param PSI integer array point source id data
// @param T numeric array gpstime data
// @param R integer array red data
// @param G integer array green data
// @param B integer array blue data
// @return void
// [[Rcpp::export]]
void laswriter(CharacterVector file,
             List LASheader,
             NumericVector X,
             NumericVector Y,
             NumericVector Z,
             DataFrame ExtraBytes, // Did not find how set empty DataFrame as default...
             IntegerVector I = IntegerVector(0),
             IntegerVector RN = IntegerVector(0),
             IntegerVector NoR = IntegerVector(0),
             IntegerVector SDF = IntegerVector(0),
             IntegerVector EoF = IntegerVector(0),
             IntegerVector C = IntegerVector(0),
             IntegerVector SA = IntegerVector(0),
             IntegerVector UD = IntegerVector(0),
             IntegerVector PSI = IntegerVector(0),
             NumericVector T = NumericVector(0),
             IntegerVector R  = IntegerVector(0),
             IntegerVector G = IntegerVector(0),
             IntegerVector B = IntegerVector(0))


{
  try
  {
    class LASheader header;

    header.file_source_ID = (int)LASheader["File Source ID"];
    header.version_major = (int)LASheader["Version Major"];
    header.version_minor = (int)LASheader["Version Minor"];
    header.header_size = (int)LASheader["Header Size"];
    header.offset_to_point_data = header.header_size; // (int)LASheader["Offset to point data"]; // No extra offset since I don't know how to deal with the data in the offset.
    header.file_creation_year = (int)LASheader["File Creation Year"];
    header.point_data_format = (int)LASheader["Point Data Format ID"];
    header.x_scale_factor = (double)LASheader["X scale factor"];
    header.y_scale_factor = (double)LASheader["Y scale factor"];
    header.z_scale_factor = (double)LASheader["Z scale factor"];
    header.x_offset =  (double)LASheader["X offset"];
    header.y_offset =  (double)LASheader["Y offset"];
    header.z_offset =  (double)LASheader["Z offset"];

    switch (header.point_data_format)
    {
    case 0:
      header.point_data_record_length = 20;
      break;
    case 1:
      header.point_data_record_length = 28;
      break;
    case 2:
      header.point_data_record_length = 26;
      break;
    case 3:
      header.point_data_record_length = 34;
      break;
    case 4:
      header.point_data_record_length = 57;
      break;
    case 5:
      header.point_data_record_length = 63;
      break;
    case 6:
      header.point_data_record_length = 30;
      break;
    case 7:
      header.point_data_record_length = 36;
      break;
    case 8:
      header.point_data_record_length = 38;
      break;
    case 9:
      header.point_data_record_length = 59;
      break;
    case 10:
      header.point_data_record_length = 67;
      break;
    }

    strcpy(header.generating_software, "rlas R package");

    //Check if Extra Bytes description exists
    List headereb_description(0);
    if(LASheader.containsElementNamed("Variable Length Records"))
    {
      List headervlr = LASheader["Variable Length Records"];
      if(headervlr.containsElementNamed("Extra_Bytes"))
      {
        List headereb = headervlr["Extra_Bytes"];
        if(headereb.containsElementNamed("Extra Bytes Description"))
          headereb_description = headereb["Extra Bytes Description"];
      }
    }

    StringVector ebnames = ExtraBytes.names();
    int number_attributes = ExtraBytes.length();

    // add attributes

    // type = 0 : unsigned char
    // type = 1 : char
    // type = 2 : unsigned short
    // type = 3 : short
    // type = 4 : unsigned int
    // type = 5 : int
    // type = 6 : unsigned int64
    // type = 7 : int64
    // type = 8 : float  (try not to use)
    // type = 9 : double (try not to use)
    std::vector<int> attribute_index, attribute_starts, type;
    std::vector<double> scale(number_attributes, 1.0), offset(number_attributes, 0.0);

    if(number_attributes > 0)
    {
      attribute_index.reserve(number_attributes);
      type.reserve(number_attributes);
      for(int i = 0; i < number_attributes; i++)
      {
        // Default values for description
        List ebparam(0);
        type[i] = 9;
        int dim = 1; // 2 and 3 dimensional arrays are deprecated in LASlib (see https://github.com/LAStools/LAStools/blob/master/LASlib/example/lasexample_write_only_with_extra_bytes.cpp)
        int options = 0;
        std::string name = as<std::string>(ebnames[i]);
        std::string description = as<std::string>(ebnames[i]);

        // if(i < headereb_description.length())
        if(headereb_description.containsElementNamed(name.c_str()))
        {
          ebparam = headereb_description[name];
          type[i] = ((int)(ebparam["data_type"])-1)%10; // see data_type definition in LAS1.4
          dim = 1; // 2 and 3 dimensional arrays are deprecated in LASlib (see https://github.com/LAStools/LAStools/blob/master/LASlib/example/lasexample_write_only_with_extra_bytes.cpp)
          options = (int)ebparam["options"];
          // std::string name = as<std::string>(ebparam["name"]);
          description = as<std::string>(ebparam["description"]);
        }
        else
        {
          Rcout << "Default header set to Extra Byte field '" << name << "'" << std::endl;
        }
        LASattribute attribute(type[i], name.c_str(), description.c_str(), dim);

        BOOL has_no_data = options & 0x01;
        BOOL has_min = options & 0x02;
        BOOL has_max = options & 0x04;
        BOOL has_scale = options & 0x08;
        BOOL has_offset = options & 0x10;

        if(has_no_data)
          attribute.set_no_data(as<double>(Rcpp::as<Rcpp::List>(ebparam["no_data"])[0]), 0);


        switch (type[i])
        {
        case 0:
        case 2:
        case 4:
        case 6:
          if(has_min)
          {
            I64 min=(I64)((double)(as<List>(ebparam["min"])[0]));
            attribute.set_min((U8*)(&min), 0);
          }
          if(has_max)
          {
            I64 max=(I64)((double)(Rcpp::as<Rcpp::List>(ebparam["max"])[0]));
            attribute.set_max((U8*)(&max), 0);
          }
          break;
        case 1:
        case 3:
        case 5:
        case 7:
          if(has_min)
          {
            U64 min=(U64)((double)(as<List>(ebparam["min"])[0]));
            attribute.set_min((U8*)(&min), 0);
          }
          if(has_max)
          {
            U64 max=(U64)((double)(Rcpp::as<Rcpp::List>(ebparam["max"])[0]));
            attribute.set_max((U8*)(&max), 0);
          }
          break;
        default:
          if(has_min)
          {
            double min=(double)(as<List>(ebparam["min"])[0]);
            attribute.set_min((U8*)(&min), 0);
          }
          if(has_max)
          {
            double max=(double)(Rcpp::as<Rcpp::List>(ebparam["max"])[0]);
            attribute.set_max((U8*)(&max), 0);
          }
        }

        if(has_scale)
        {
          scale[i] = (double)(Rcpp::as<Rcpp::List>(ebparam["scale"])[0]);
          attribute.set_scale(scale[i], 0);
        }
        if(has_offset)
        {
          offset[i] = (double)(Rcpp::as<Rcpp::List>(ebparam["offset"])[0]);
          attribute.set_offset(offset[i], 0);
        }

        attribute_index.push_back(header.add_attribute(attribute));
      }
    }
    header.update_extra_bytes_vlr();

    LASattribute attribute = header.attributes[0];

    // add number of extra bytes to the point size
    header.point_data_record_length += header.get_attributes_size();

    attribute_starts.reserve(attribute_index.size());
    for(int i = 0; i < attribute_index.size(); i++)
    {
      attribute_starts.push_back(header.get_attribute_start(attribute_index[i]));
    }

    std::string filestd = as<std::string>(file);

    LASwriteOpener laswriteopener;
    laswriteopener.set_file_name(filestd.c_str());

    LASpoint p;
    p.init(&header, header.point_data_format, header.point_data_record_length, 0);

    LASwriter* laswriter = laswriteopener.open(&header);

    if(0 == laswriter || NULL == laswriter)
      throw std::runtime_error("LASlib internal error. See message above.");

    double scaled_value;
    NumericVector EB[ExtraBytes.length()];
    for(int i = 0; i < ExtraBytes.length(); i++)
      EB[i]=ExtraBytes[i];


    for(int i = 0 ; i < X.length() ; i++)
    {
      p.set_x(X[i]);
      p.set_y(Y[i]);
      p.set_z(Z[i]);

      if(I.length() > 0){ p.set_intensity((U16)I[i]); }
      if(RN.length() > 0){ p.set_return_number((U8)RN[i]); }
      if(NoR.length() > 0){ p.set_number_of_returns((U8)NoR[i]); }
      if(SDF.length() > 0){ p.set_scan_direction_flag((U8)SDF[i]); }
      if(EoF.length() > 0){ p.set_edge_of_flight_line((U8)EoF[i]); }
      if(C.length() > 0){ p.set_classification((U8)C[i]); }
      if(SA.length() > 0){ p.set_scan_angle_rank((I8)SA[i]); }
      if(UD.length() > 0){ p.set_user_data((U8)UD[i]); }
      if(PSI.length() > 0){ p.set_point_source_ID((U16)PSI[i]); }
      if(T.length() > 0){ p.set_gps_time((F64)T[i]); }
      if(R.length() > 0)
      {
        p.set_R((U16)R[i]);
        p.set_G((U16)G[i]);
        p.set_B((U16)B[i]);
      }

      // add extra bytes
      for(int j = 0; j < number_attributes; j++)
      {

        scaled_value=(EB[j][i] - offset[j])/scale[j];

        switch(type[j])
        {
        case 0:
          p.set_attribute(attribute_starts[j], U8_CLAMP(U8_QUANTIZE(scaled_value)));
          break;
        case 1:
          p.set_attribute(attribute_starts[j], I8_CLAMP(I8_QUANTIZE(scaled_value)));
          break;
        case 2:
          p.set_attribute(attribute_starts[j], U16_CLAMP(U16_QUANTIZE(scaled_value)));
          break;
        case 3:
          p.set_attribute(attribute_starts[j], I16_CLAMP(I16_QUANTIZE(scaled_value)));
          break;
        case 4:
          p.set_attribute(attribute_starts[j], U32_CLAMP(U32_QUANTIZE(scaled_value)));
          break;
        case 5:
          p.set_attribute(attribute_starts[j], I32_CLAMP(I32_QUANTIZE(scaled_value)));
          break;
        case 6:
          p.set_attribute(attribute_starts[j], U64_QUANTIZE(scaled_value));
          break;
        case 7:
          p.set_attribute(attribute_starts[j], I64_QUANTIZE(scaled_value));
          break;
        case 8:
          p.set_attribute(attribute_starts[j], (float)(scaled_value));
          break;
        case 9:
          p.set_attribute(attribute_starts[j], scaled_value);
          break;
        }
      }

      laswriter->write_point(&p);
      laswriter->update_inventory(&p);
    }

    laswriter->update_header(&header, true);
    I64 total_bytes = laswriter->close();
    delete laswriter;
  }
  catch (std::exception const& e)
  {
    Rcerr << e.what() << std::endl;
  }
}
