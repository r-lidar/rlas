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

int get_point_data_record_length(int x);

// [[Rcpp::export]]
void C_writer(CharacterVector file, List LASheader, DataFrame data)
{
  try
  {
    // 1. Make a standard header

    class LASheader header;
    header.file_source_ID = (int)LASheader["File Source ID"];
    header.version_major = (int)LASheader["Version Major"];
    header.version_minor = (int)LASheader["Version Minor"];
    header.header_size = (int)LASheader["Header Size"];
    header.offset_to_point_data = header.header_size;
    header.file_creation_year = (int)LASheader["File Creation Year"];
    header.point_data_format = (int)LASheader["Point Data Format ID"];
    header.x_scale_factor = (double)LASheader["X scale factor"];
    header.y_scale_factor = (double)LASheader["Y scale factor"];
    header.z_scale_factor = (double)LASheader["Z scale factor"];
    header.x_offset =  (double)LASheader["X offset"];
    header.y_offset =  (double)LASheader["Y offset"];
    header.z_offset =  (double)LASheader["Z offset"];
    header.point_data_record_length = get_point_data_record_length(header.point_data_format);
    strcpy(header.generating_software, "rlas R package");

    // 2. Deal with extra bytes attributes

    // Get the extra bytes description
    List description_eb(0);                         // Create an empty description for extra bytes
    if(LASheader.containsElementNamed("Variable Length Records"))
    {
      List vlr = LASheader["Variable Length Records"];

      if(vlr.containsElementNamed("Extra_Bytes"))
      {
        List extra_bytes = vlr["Extra_Bytes"];

        if(extra_bytes.containsElementNamed("Extra Bytes Description"))
        {
          description_eb = extra_bytes["Extra Bytes Description"];
        }
      }
    }

    int num_eb = description_eb.size();              // Get the number of extra byte
    std::vector<std::string> ebnames(num_eb);        // Get the name of the extra bytes based on column name of the data.frame
    std::vector<int> attribute_index(num_eb);        // Index of attribute in the header
    std::vector<int> attribute_starts(num_eb);       // Attribute starting byte number
    std::vector<double> scale(num_eb, 1.0);          // Default scale factor
    std::vector<double> offset(num_eb, 0.0);         // Default offset factor
    std::vector<int> type(num_eb);                   // Attribute type (see comment at the very end of this file)
    double scaled_value;                             // Temporary variable


    // Update the header
    for(int i = 0; i < num_eb; i++)
    {
      List description = description_eb[i];

      type[i] = ((int)(description["data_type"])-1) % 10;
      int options = description["options"];
      ebnames[i] = as< std::string >(description["name"]);
      std::string desc = description["description"];

      //  checks if name exist in data
      if(!data.containsElementNamed(ebnames[i].c_str()))
        throw std::runtime_error("Extra Byte described but not present in data.");

      LASattribute attribute(type[i], ebnames[i].c_str(), desc.c_str(), 1);

      // see extra byte option definition in LAS 1.4
      bool has_no_data = options & 0x01;
      bool has_min = options & 0x02;
      bool has_max = options & 0x04;
      bool has_scale = options & 0x08;
      bool has_offset = options & 0x10;


      // set scale value if option set
      if(has_scale)
      {
        scale[i] = (double)(as<List>(description["scale"])[0]);
        attribute.set_scale(scale[i], 0);
      }

      // set offset value if option set
      if(has_offset)
      {
        offset[i] = (double)(as<List>(description["offset"])[0]);
        attribute.set_offset(offset[i], 0);
      }

      // set no data value if option set
      if(has_no_data)
      {
        scaled_value=((double)(as<List>(description["no_data"])[0]) - offset[i])/scale[i];

        switch(type[i])
        {
        case 0:
          attribute.set_no_data(U8_CLAMP(U8_QUANTIZE(scaled_value)));
          break;
        case 1:
          attribute.set_no_data(I8_CLAMP(I8_QUANTIZE(scaled_value)));
          break;
        case 2:
          attribute.set_no_data(U16_CLAMP(U16_QUANTIZE(scaled_value)));
          break;
        case 3:
          attribute.set_no_data(I16_CLAMP(I16_QUANTIZE(scaled_value)));
          break;
        case 4:
          attribute.set_no_data(U32_CLAMP(U32_QUANTIZE(scaled_value)));
          break;
        case 5:
          attribute.set_no_data(I32_CLAMP(I32_QUANTIZE(scaled_value)));
          break;
        case 6:
          attribute.set_no_data(U64_QUANTIZE(scaled_value));
          break;
        case 7:
          attribute.set_no_data(I64_QUANTIZE(scaled_value));
          break;
        case 8:
          attribute.set_no_data((float)(scaled_value));
          break;
        case 9:
          attribute.set_no_data(scaled_value);
          break;
        }
      }

      // set min value if option set
      if(has_min)
      {
        scaled_value=((double)(as<List>(description["min"])[0]) - offset[i])/scale[i];

        switch(type[i])
        {
        case 0:
          attribute.set_min(U8_CLAMP(U8_QUANTIZE(scaled_value)));
          break;
        case 1:
          attribute.set_min(I8_CLAMP(I8_QUANTIZE(scaled_value)));
          break;
        case 2:
          attribute.set_min(U16_CLAMP(U16_QUANTIZE(scaled_value)));
          break;
        case 3:
          attribute.set_min(I16_CLAMP(I16_QUANTIZE(scaled_value)));
          break;
        case 4:
          attribute.set_min(U32_CLAMP(U32_QUANTIZE(scaled_value)));
          break;
        case 5:
          attribute.set_min(I32_CLAMP(I32_QUANTIZE(scaled_value)));
          break;
        case 6:
          attribute.set_min(U64_QUANTIZE(scaled_value));
          break;
        case 7:
          attribute.set_min(I64_QUANTIZE(scaled_value));
          break;
        case 8:
          attribute.set_min((float)(scaled_value));
          break;
        case 9:
          attribute.set_min(scaled_value);
          break;
        }
      }

      // set max value if option set
      if(has_max)
      {
        scaled_value=((double)(as<List>(description["max"])[0]) - offset[i])/scale[i];
        switch(type[i])
        {
        case 0:
          attribute.set_max(U8_CLAMP(U8_QUANTIZE(scaled_value)));
          break;
        case 1:
          attribute.set_max(I8_CLAMP(I8_QUANTIZE(scaled_value)));
          break;
        case 2:
          attribute.set_max(U16_CLAMP(U16_QUANTIZE(scaled_value)));
          break;
        case 3:
          attribute.set_max(I16_CLAMP(I16_QUANTIZE(scaled_value)));
          break;
        case 4:
          attribute.set_max(U32_CLAMP(U32_QUANTIZE(scaled_value)));
          break;
        case 5:
          attribute.set_max(I32_CLAMP(I32_QUANTIZE(scaled_value)));
          break;
        case 6:
          attribute.set_max(U64_QUANTIZE(scaled_value));
          break;
        case 7:
          attribute.set_max(I64_QUANTIZE(scaled_value));
          break;
        case 8:
          attribute.set_max((float)(scaled_value));
          break;
        case 9:
          attribute.set_max(scaled_value);
          break;
        }
      }

      // Finally add the attribute to the header
      attribute_index[i] = header.add_attribute(attribute);
    }

    header.update_extra_bytes_vlr();
    header.point_data_record_length += header.get_attributes_size();

    // starting byte in point format of extra byte j
    for(int i = 0; i < num_eb; i++)
      attribute_starts[i]=header.get_attribute_start(attribute_index[i]);

    // 3. write the data to the file

    LASwriteOpener laswriteopener;
    laswriteopener.set_file_name(as<std::string>(file).c_str());

    LASpoint p;
    p.init(&header, header.point_data_format, header.point_data_record_length, 0);

    LASwriter* laswriter = laswriteopener.open(&header);

    if(0 == laswriter || NULL == laswriter)
      throw std::runtime_error("LASlib internal error. See message above.");

    NumericVector X = data["X"];
    NumericVector Y = data["Y"];
    NumericVector Z = data["Z"];
    IntegerVector I = IntegerVector(0);
    IntegerVector RN = IntegerVector(0);
    IntegerVector NoR = IntegerVector(0);
    IntegerVector SDF = IntegerVector(0);
    IntegerVector EoF = IntegerVector(0);
    IntegerVector C = IntegerVector(0);
    IntegerVector SA = IntegerVector(0);
    IntegerVector UD = IntegerVector(0);
    IntegerVector PSI = IntegerVector(0);
    NumericVector T = NumericVector(0);
    IntegerVector R  = IntegerVector(0);
    IntegerVector G = IntegerVector(0);
    IntegerVector B = IntegerVector(0);
    IntegerVector NIR = IntegerVector(0);

    if (data.containsElementNamed("Intensity"))
      I = data["Intensity"];
    if (data.containsElementNamed("ReturnNumber"))
      RN = data["ReturnNumber"];
    if (data.containsElementNamed("NumberOfReturns"))
      NoR = data["NumberOfReturns"];
    if (data.containsElementNamed("ScanDirectionFlag"))
      SDF = data["ScanDirectionFlag"];
    if (data.containsElementNamed("EdgeOfFlightline"))
      EoF = data["EdgeOfFlightline"];
    if (data.containsElementNamed("Classification"))
      C = data["Classification"];
    if (data.containsElementNamed("ScanAngle"))
      SA = data["ScanAngle"];
    if (data.containsElementNamed("UserData"))
      UD = data["UserData"];
    if (data.containsElementNamed("PointSourceID"))
      PSI = data["PointSourceID"];
    if (data.containsElementNamed("gpstime"))
      T = data["gpstime"];
    if (data.containsElementNamed("R"))
      R = data["R"];
    if (data.containsElementNamed("G"))
      G = data["G"];
    if (data.containsElementNamed("B"))
      B = data["B"];
    if (data.containsElementNamed("NIR"))
      NIR = data["NIR"];

    // convert data.frame to Numeric vector to reduce access time
    std::vector< NumericVector > EB(num_eb);                        // For fast access to data.frame elements

    for(int i = 0; i < num_eb; i++)
      EB[i]=data[ebnames[i]];

    for(int i = 0 ; i < X.length() ; i++)
    {
      // Add regular data
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
      if(R.length() > 0) { p.set_R((U16)R[i]); }
      if(G.length() > 0) { p.set_G((U16)G[i]); }
      if(B.length() > 0) { p.set_B((U16)B[i]); }
      if(NIR.length() > 0) { p.set_NIR((U16)NIR[i]); }

      // Add extra bytes
      for(int j = 0 ; j < num_eb ; j++)
      {
        // value is scaled, quantized and clamped to fit chosen datatype
        scaled_value = (EB[j][i] - offset[j])/scale[j];

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


int get_point_data_record_length(int point_data_format)
{
  switch (point_data_format)
  {
  case 0:
    return 20;
    break;
  case 1:
    return 28;
    break;
  case 2:
    return 26;
    break;
  case 3:
    return 34;
    break;
  case 4:
    return 57;
    break;
  case 5:
    return 63;
    break;
  case 6:
    return 30;
    break;
  case 7:
    return 36;
    break;
  case 8:
    return 38;
    break;
  case 9:
    return 59;
    break;
  case 10:
    return 67;
    break;
  default:
    throw std::runtime_error("point_data_format out of range.");
    break;
  }
}

/* attributes type:
 * type = 0 : unsigned char
 * type = 1 : char
 * type = 2 : unsigned short
 * type = 3 : short
 * type = 4 : unsigned int
 * type = 5 : int
 * type = 6 : unsigned int64
 * type = 7 : int64
 * type = 8 : float  (try not to use)
 * type = 9 : double (try not to use)
 */
