/*
 ===============================================================================

 PROGRAMMERS:

 jean-romain.roussel.1@ulaval.ca  -  https://github.com/Jean-Romain/rlas

 COPYRIGHT:

 Copyright 2016-2019 Jean-Romain Roussel

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
#include "rlasstreamer.h"

using namespace Rcpp;

int  get_point_data_record_length(int x);
void set_guid(LASheader&, const char*);
void set_global_enconding(LASheader&, List);

// [[Rcpp::export]]
void C_writer(CharacterVector file, List LASheader, DataFrame data)
{
  try
  {
    class LASheader header;

    // ===========================
    // Public Header Block
    // ===========================

    // 1.1. These ones are easy

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

    // 1.2. These one need special interpretation

    header.point_data_record_length = get_point_data_record_length(header.point_data_format);
    strcpy(header.generating_software, "rlas R package");

    CharacterVector guid = LASheader["Project ID - GUID"];
    std::string stdguid  = as<std::string>(guid);
    set_guid(header, stdguid.c_str());

    set_global_enconding(header, LASheader["Global Encoding"]);

    // ===========================
    // 2. Variable Lenght Records
    // ===========================

    // 2.1 ESPG code

    if(LASheader.containsElementNamed("Variable Length Records"))
    {
      List vlr = LASheader["Variable Length Records"];

      if(vlr.containsElementNamed("GeoKeyDirectoryTag"))
      {
        List GKDT = vlr["GeoKeyDirectoryTag"];
        List tags = GKDT["tags"];

        for (int i = 0 ; i < tags.size() ; i ++)
        {
          List tag = tags[i];
          int key  = tag["key"];

          if (key == 3072)
          {
            LASvlr_key_entry* vlr_epsg = new LASvlr_key_entry();
            vlr_epsg->key_id = 3072;
            vlr_epsg->tiff_tag_location = 0;
            vlr_epsg->count = 1;
            vlr_epsg->value_offset = (U16)tag["value offset"];

            header.set_geo_keys(1, vlr_epsg);
            delete vlr_epsg;

            break;
          }
        }
      }
    }

    // 2.2 WKT OGC MATH TRANSFORM

    // 2.3 WKT OGC COORDINATE

    if (LASheader.containsElementNamed("Variable Length Records"))
    {
      List vlr = LASheader["Variable Length Records"];

      if (vlr.containsElementNamed("WKT OGC CS"))
      {
        List WKTOGCCS = vlr["WKT OGC CS"];

        if (WKTOGCCS.containsElementNamed("WKT OGC COORDINATE SYSTEM"))
        {
          CharacterVector WKT = WKTOGCCS["WKT OGC COORDINATE SYSTEM"];
          std::string sWKT    = as<std::string>(WKT);
          const char* cWKT    = sWKT.c_str();

          header.set_geo_ogc_wkt(sWKT.size(), cWKT);
        }
      }
    }

    // 2.4 Extrabytes attributes

    std::vector<RLASExtrabyteAttributes> ExtraBytesAttr;
    if(LASheader.containsElementNamed("Variable Length Records"))
    {
      List vlr = LASheader["Variable Length Records"];

      if(vlr.containsElementNamed("Extra_Bytes"))
      {
        List extra_bytes = vlr["Extra_Bytes"];

        if(extra_bytes.containsElementNamed("Extra Bytes Description"))
        {
          List description_eb = extra_bytes["Extra Bytes Description"];

          for(int i = 0 ; i < description_eb.size() ; i++)
          {
            List description = description_eb[i];
            RLASExtrabyteAttributes ExtraByte;

            ExtraByte.name = as< std::string >(description["name"]);

            if (!data.containsElementNamed(ExtraByte.name.c_str()))
              throw std::runtime_error("Extra Bytes described but not present in data.");

            ExtraByte.options = (int)description["options"];
            ExtraByte.parse_options();

            ExtraByte.data_type = ((int)(description["data_type"])-1) % 10;
            ExtraByte.desc = as< std::string >(description["description"]);

            if(ExtraByte.has_scale)
              ExtraByte.scale = (double)(description["scale"]);

            if(ExtraByte.has_offset)
              ExtraByte.offset = (double)(description["offset"]);

            if(ExtraByte.has_no_data)
              ExtraByte.no_data = ((double)(description["no_data"]) - ExtraByte.offset)/ExtraByte.scale;

            if(ExtraByte.has_min)
              ExtraByte.min = ((double)(description["min"]) - ExtraByte.offset)/ExtraByte.scale;

            if(ExtraByte.has_max)
              ExtraByte.max =  ((double)(description["max"]) - ExtraByte.offset)/ExtraByte.scale;

            LASattribute attribute = ExtraByte.make_LASattribute();

            ExtraByte.id  = header.add_attribute(attribute);
            ExtraByte.Reb = as<NumericVector>(data[ExtraByte.name.c_str()]);
            ExtraBytesAttr.push_back(ExtraByte);
          }

          header.update_extra_bytes_vlr();
          header.point_data_record_length += header.get_attributes_size();

          // Starting byte
          for(auto& ExtraByte : ExtraBytesAttr)
            ExtraByte.start = header.get_attribute_start(ExtraByte.id);
        }
      }
    }

    // ===============================
    // 3. Write the data into the file
    // ===============================

    LASwriteOpener laswriteopener;
    laswriteopener.set_file_name(as<std::string>(file).c_str());

    LASpoint p;
    p.init(&header, header.point_data_format, header.point_data_record_length, 0);

    LASwriter* laswriter = laswriteopener.open(&header);

    if(0 == laswriter || NULL == laswriter)
      throw std::runtime_error("LASlib internal error. See message above.");

    double scaled_value;

    NumericVector X = data["X"];
    NumericVector Y = data["Y"];
    NumericVector Z = data["Z"];
    IntegerVector I = IntegerVector(0);
    IntegerVector RN = IntegerVector(0);
    IntegerVector NoR = IntegerVector(0);
    IntegerVector SDF = IntegerVector(0);
    IntegerVector EoF = IntegerVector(0);
    IntegerVector C = IntegerVector(0);
    LogicalVector S = LogicalVector(0);
    LogicalVector K = LogicalVector(0);
    LogicalVector W = LogicalVector(0);
    IntegerVector SA = IntegerVector(0);
    IntegerVector UD = IntegerVector(0);
    IntegerVector PSI = IntegerVector(0);
    NumericVector T = NumericVector(0);
    IntegerVector R  = IntegerVector(0);
    IntegerVector G = IntegerVector(0);
    IntegerVector B = IntegerVector(0);
    IntegerVector NIR = IntegerVector(0);

    if (data.containsElementNamed("Intensity"))
      I   = data["Intensity"];
    if (data.containsElementNamed("ReturnNumber"))
      RN  = data["ReturnNumber"];
    if (data.containsElementNamed("NumberOfReturns"))
      NoR = data["NumberOfReturns"];
    if (data.containsElementNamed("ScanDirectionFlag"))
      SDF = data["ScanDirectionFlag"];
    if (data.containsElementNamed("EdgeOfFlightline"))
      EoF = data["EdgeOfFlightline"];
    if (data.containsElementNamed("Classification"))
      C   = data["Classification"];
    if (data.containsElementNamed("Synthetic_flag"))
      S   = data["Synthetic_flag"];
    if (data.containsElementNamed("Keypoint_flag"))
      K   = data["Keypoint_flag"];
    if (data.containsElementNamed("Withheld_flag"))
      W   = data["Withheld_flag"];
    if (data.containsElementNamed("ScanAngle"))
      SA  = data["ScanAngle"];
    if (data.containsElementNamed("UserData"))
      UD  = data["UserData"];
    if (data.containsElementNamed("PointSourceID"))
      PSI = data["PointSourceID"];
    if (data.containsElementNamed("gpstime"))
      T   = data["gpstime"];
    if (data.containsElementNamed("R"))
      R   = data["R"];
    if (data.containsElementNamed("G"))
      G   = data["G"];
    if (data.containsElementNamed("B"))
      B   = data["B"];
    if (data.containsElementNamed("NIR"))
      NIR = data["NIR"];

    for(int i = 0 ; i < X.length() ; i++)
    {
      // Add regular data
      p.set_x(X[i]);
      p.set_y(Y[i]);
      p.set_z(Z[i]);

      if(I.length() > 0)   { p.set_intensity((U16)I[i]); }
      if(RN.length() > 0)  { p.set_return_number((U8)RN[i]); }
      if(NoR.length() > 0) { p.set_number_of_returns((U8)NoR[i]); }
      if(SDF.length() > 0) { p.set_scan_direction_flag((U8)SDF[i]); }
      if(EoF.length() > 0) { p.set_edge_of_flight_line((U8)EoF[i]); }
      if(C.length() > 0)   { p.set_classification((U8)C[i]); }
      if(S.length() > 0)   { p.set_synthetic_flag((U8)S[i]); }
      if(K.length() > 0)   { p.set_keypoint_flag((U8)K[i]); }
      if(W.length() > 0)   { p.set_withheld_flag((U8)W[i]); }
      if(SA.length() > 0)  { p.set_scan_angle_rank((I8)SA[i]); }
      if(UD.length() > 0)  { p.set_user_data((U8)UD[i]); }
      if(PSI.length() > 0) { p.set_point_source_ID((U16)PSI[i]); }
      if(T.length() > 0)   { p.set_gps_time((F64)T[i]); }
      if(R.length() > 0)   { p.set_R((U16)R[i]); }
      if(G.length() > 0)   { p.set_G((U16)G[i]); }
      if(B.length() > 0)   { p.set_B((U16)B[i]); }
      if(NIR.length() > 0) { p.set_NIR((U16)NIR[i]); }

      // Add extra bytes
      for(auto& ExtraByte : ExtraBytesAttr)
        ExtraByte.set_attribute(i, &p);

      laswriter->write_point(&p);
      laswriter->update_inventory(&p);
    }

    laswriter->update_header(&header, true);
    laswriter->close();
    delete laswriter;
  }
  catch (std::exception const& e)
  {
    Rcerr << e.what() << std::endl;
  }
}

void set_global_enconding(LASheader &header, List encoding)
{
  if (encoding["GPS Time Type"])
    header.set_global_encoding_bit(0);

  if (encoding["Waveform Data Packets Internal"])
    header.set_global_encoding_bit(1);

  if (encoding["Waveform Data Packets External"])
    header.set_global_encoding_bit(2);

  if (encoding["Synthetic Return Numbers"])
    header.set_global_encoding_bit(3);

  if (encoding["WKT"])
    header.set_global_encoding_bit(4);

  if (encoding["Aggregate Model"])
    header.set_global_encoding_bit(5);
}

void set_guid(LASheader &header, const char* guid)
{
  // uuid
  long long unsigned int set_GUID_data_1 = 0;
  unsigned int set_GUID_data_2 = 0;
  unsigned int set_GUID_data_3 = 0;
  unsigned int set_GUID_data_4a = 0;
  long long unsigned int set_GUID_data_4b = 0;

  sscanf(guid, "%llx-%x-%x-%x-%llx", &set_GUID_data_1, &set_GUID_data_2, &set_GUID_data_3, &set_GUID_data_4a, &set_GUID_data_4b);

  U32 GUID_data_1 = U32_CLAMP(set_GUID_data_1);
  U16 GUID_data_2 = U16_CLAMP(set_GUID_data_2);
  U16 GUID_data_3 = U16_CLAMP(set_GUID_data_3);
  U16 GUID_data_4a = U16_CLAMP(set_GUID_data_4a);
  U16 GUID_data_4b_a = U16_CLAMP(set_GUID_data_4b >> 32);
  U32 GUID_data_4b_b = U32_CLAMP(set_GUID_data_4b & 0xFFFFFFFF);

  U8 byte1 = * ((U8 *)&GUID_data_4a);
  U8 byte2 = * ((U8 *)&GUID_data_4a+1);
  U8 byte3 = * ((U8 *)&GUID_data_4b_a);
  U8 byte4 = * ((U8 *)&GUID_data_4b_a+1);
  U8 byte5 = * ((U8 *)&GUID_data_4b_b);
  U8 byte6 = * ((U8 *)&GUID_data_4b_b+1);
  U8 byte7 = * ((U8 *)&GUID_data_4b_b+2);
  U8 byte8 = * ((U8 *)&GUID_data_4b_b+3);
  U8 GUID_data_4[8] = {byte1, byte2, byte3, byte4, byte5, byte6, byte7, byte8};

  header.project_ID_GUID_data_1 = GUID_data_1;
  header.project_ID_GUID_data_2 = GUID_data_2;
  header.project_ID_GUID_data_3 = GUID_data_3;
  memcpy(header.project_ID_GUID_data_4, GUID_data_4, sizeof(U8)*8);
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

/* attributes data_type:
 * data_type = 0 : unsigned char
 * data_type = 1 : char
 * data_type = 2 : unsigned short
 * data_type = 3 : short
 * data_type = 4 : unsigned int
 * data_type = 5 : int
 * data_type = 6 : unsigned int64
 * data_type = 7 : int64
 * data_type = 8 : float  (try not to use)
 * data_type = 9 : double (try not to use)
 */
