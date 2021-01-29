/*
 ===============================================================================

 PROGRAMMERS:

 jean-romain.roussel.1@ulaval.ca  -  https://github.com/Jean-Romain/rlas

 COPYRIGHT:

 Copyright 2017-2019 Jean-Romain Roussel

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lasreader.hpp"
#include "lasfilter.hpp"
#include "lastransform.hpp"


using namespace Rcpp;

List vlrsreader(LASheader*);
List evlrsreader(LASheader*);
List globalencodingreader(LASheader*);
inline std::string nullterminate(CHAR*, int);

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

    if((0 == lasreader) || (NULL == lasreader))
      throw std::runtime_error("LASlib internal error. See message above.");

    char cguid[38];
    sprintf(cguid, "%08x-%04x-%04x-%04x-%04x%08x",
            lasheader->project_ID_GUID_data_1,
            lasheader->project_ID_GUID_data_2,
            lasheader->project_ID_GUID_data_3,
            *((U16*)(lasheader->project_ID_GUID_data_4)),
            *((U16*)(lasheader->project_ID_GUID_data_4+2)),
            *((U32*)(lasheader->project_ID_GUID_data_4+4)));

    CharacterVector guid(cguid);

    List head(0);
    head.push_back(lasheader->file_signature);
    head.push_back(lasheader->file_source_ID);
    head.push_back(globalencodingreader(lasheader));
    head.push_back(guid);
    head.push_back((int)lasheader->version_major);
    head.push_back((int)lasheader->version_minor);
    head.push_back(lasheader->system_identifier);
    head.push_back(lasheader->generating_software);
    head.push_back(lasheader->file_creation_day);
    head.push_back(lasheader->file_creation_year);
    head.push_back(lasheader->header_size);
    head.push_back(lasheader->offset_to_point_data);
    head.push_back(lasheader->number_of_variable_length_records);
    head.push_back((int)lasheader->point_data_format);
    head.push_back(lasheader->point_data_record_length);

    // Added support of LAS 1.4
    if (lasheader->version_minor == 4)
    {
      if (lasheader->extended_number_of_point_records > 2147483647)
        throw std::runtime_error("R cannot handle more than 2147483647 points.");

      head.push_back((int)lasheader->extended_number_of_point_records);
      IntegerVector extended_number_of_points_by_return(15);
      for(int i = 0 ; i < 15 ; i++) extended_number_of_points_by_return[i] = (int)lasheader->extended_number_of_points_by_return[i];
      head.push_back(extended_number_of_points_by_return);
    }
    else
    {
      if (lasheader->number_of_point_records > 2147483647)
        throw std::runtime_error("R cannot handle more than 2147483647 points.");

      head.push_back((int)lasheader->number_of_point_records);

      IntegerVector number_of_points_by_return(5);
      for(int i = 0 ; i < 5 ; i++) number_of_points_by_return[i] = (int)lasheader->number_of_points_by_return[i];
      head.push_back(number_of_points_by_return);
    }

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
    head.push_back(evlrsreader(lasheader));

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
    names.push_back("Number of points by return");
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
    names.push_back("Extended Variable Length Records");

    head.names() = names;

    lasreader->close();
    delete lasreader;

    return(head);
  }
  catch (std::exception const& e)
  {
    Rcerr << "Error: " << e.what() << std::endl;
    return(List(0));
  }
}

List globalencodingreader(LASheader* lasheader)
{
  bool GPSTimeType = lasheader->get_global_encoding_bit(0);
  bool WaveformDataPacketsInternal = lasheader->get_global_encoding_bit(1);
  bool WaveformDataPacketsExternal = lasheader->get_global_encoding_bit(2);
  bool SyntheticReturnNumbers = lasheader->get_global_encoding_bit(3);
  bool WKT = lasheader->get_global_encoding_bit(4);
  bool AggregateModel = lasheader->get_global_encoding_bit(5);

  return List::create(Named("GPS Time Type") = GPSTimeType,
                      Named("Waveform Data Packets Internal") = WaveformDataPacketsInternal,
                      Named("Waveform Data Packets External") = WaveformDataPacketsExternal,
                      Named("Synthetic Return Numbers") = SyntheticReturnNumbers,
                      Named("WKT") = WKT,
                      Named("Aggregate Model") = AggregateModel);
}

List vlrsreader(LASheader* lasheader)
{
  int nvlrs = (int)lasheader->number_of_variable_length_records;

  List lvlrs;
  List lvlrsnames;

  for (int i = 0; i < nvlrs; i++)
  {
    LASvlr vlr     = lasheader->vlrs[i];
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
        lvlr.push_back(nullterminate(lasheader->vlr_geo_ascii_params, vlr.record_length_after_header));
        lvlrnames.push_back("tags");
        lvlrsnames.push_back("GeoAsciiParamsTag");
      }
      else if (vlr.record_id == 2111) // WKT OGC MATH TRANSFORM
      {
        lvlr.push_back(nullterminate(lasheader->vlr_geo_ogc_wkt_math, vlr.record_length_after_header));
        lvlrnames.push_back("WKT OGC MATH TRANSFORM");
        lvlrsnames.push_back("WKT OGC MT");
      }
      else if (vlr.record_id == 2112) // WKT OGC COORDINATE SYSTEM
      {
        lvlr.push_back(nullterminate(lasheader->vlr_geo_ogc_wkt, vlr.record_length_after_header));
        lvlrnames.push_back("WKT OGC COORDINATE SYSTEM");
        lvlrsnames.push_back("WKT OGC CS");
      }
    }
    else if ((strcmp(vlr.user_id, "LASF_Spec") == 0) && (vlr.data != 0))
    {
        if (vlr.record_id == 4) // ExtraBytes
        {
          lvlrsnames.push_back("Extra_Bytes");
          lvlrnames.push_back("Extra Bytes Description");

          List ExtraBytes(0);
          List ExtraBytesnames(0);

          for (int j = 0; j < lasheader->number_attributes; j++)
          {
            LASattribute attemp(lasheader->attributes[j]);

            if (attemp.data_type)
            {
              int data_type = ((I32)(attemp.data_type)-1)%10;
              //int dim = ((I32)(attemp.data_type)-1)/10+1;

              List ExtraByte(0);
              List ExtraBytenames(0);
              ExtraByte.push_back(((I16*)(attemp.reserved))[0]);
              ExtraBytenames.push_back("reserved");
              ExtraByte.push_back((I32)(attemp.data_type));
              ExtraBytenames.push_back("data_type");
              ExtraByte.push_back((I32)(attemp.options));
              ExtraBytenames.push_back("options");
              ExtraByte.push_back(attemp.name);
              ExtraBytenames.push_back("name");

              // 2 and 3 dimensional arrays are deprecated in LASlib
              // (see https://github.com/LAStools/LAStools/blob/master/LASlib/example/lasexample_write_only_with_extra_bytes.cpp)
              double scale = 1.0;
              if(attemp.has_scale())
              {
                scale = attemp.scale[0];
                ExtraByte.push_back(scale);
                ExtraBytenames.push_back("scale");
              }

              double offset = 0.0;
              if(attemp.has_offset())
              {
                offset = attemp.offset[0];
                ExtraByte.push_back(offset);
                ExtraBytenames.push_back("offset");
              }

              if (data_type < 8)
              {
                I64* temp; // as R does not support long long int it is converted to double

                if (attemp.has_no_data())
                {
                  temp = ((I64*)(attemp.no_data));
                  ExtraByte.push_back(*temp);
                  ExtraBytenames.push_back("no_data");
                }

                if (attemp.has_min())
                {
                  temp = ((I64*)(attemp.min));
                  ExtraByte.push_back(*temp*scale+offset);
                  ExtraBytenames.push_back("min");
                }

                if (attemp.has_max())
                {
                  temp = ((I64*)(attemp.max));
                  ExtraByte.push_back(*temp*scale+offset);
                  ExtraBytenames.push_back("max");
                }
              }
              else
              {
                F64* temp;

                if (attemp.has_no_data())
                {
                  temp = ((F64*)(attemp.no_data));
                  ExtraByte.push_back(*temp*scale+offset);
                  ExtraBytenames.push_back("no_data");
                }

                if (attemp.has_min())
                {
                  temp = ((F64*)(attemp.min));
                  ExtraByte.push_back(*temp*scale+offset);
                  ExtraBytenames.push_back("min");
                }

                if (attemp.has_max())
                {
                  temp = ((F64*)(attemp.max));
                  ExtraByte.push_back(*temp*scale+offset);
                  ExtraBytenames.push_back("max");
                }
              }

              ExtraByte.push_back(attemp.description);
              ExtraBytenames.push_back("description");

              ExtraByte.names() =  ExtraBytenames;
              ExtraBytes.push_back(ExtraByte);
              ExtraBytesnames.push_back(attemp.name);
            }
            else
            {
              Rcout << "extra byte " << j << " undocumented: dropped" << std::endl;
            }
          }

          ExtraBytes.names() = ExtraBytesnames;
          lvlr.push_back(ExtraBytes);
        }
        else
        {
          // not supported yet
          lvlrsnames.push_back(vlr.user_id);
        }
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

List evlrsreader(LASheader* lasheader)
{
  int nvlrs = (int)lasheader->number_of_extended_variable_length_records;

  List lvlrs;
  List lvlrsnames;

  for (int i = 0; i < nvlrs; i++)
  {
    LASevlr vlr    = lasheader->evlrs[i];
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
        lvlr.push_back(nullterminate(lasheader->vlr_geo_ascii_params, vlr.record_length_after_header));
        lvlrnames.push_back("tags");
        lvlrsnames.push_back("GeoAsciiParamsTag");
      }
      else if (vlr.record_id == 2111) // WKT OGC MATH TRANSFORM
      {
        lvlr.push_back(nullterminate(lasheader->vlr_geo_ogc_wkt_math, vlr.record_length_after_header));
        lvlrnames.push_back("WKT OGC MATH TRANSFORM");
        lvlrsnames.push_back("WKT OGC MT");
      }
      else if (vlr.record_id == 2112) // WKT OGC COORDINATE SYSTEM
      {
        lvlr.push_back(nullterminate(lasheader->vlr_geo_ogc_wkt, vlr.record_length_after_header));
        lvlrnames.push_back("WKT OGC COORDINATE SYSTEM");
        lvlrsnames.push_back("WKT OGC CS");
      }
    }
    else if ((strcmp(vlr.user_id, "LASF_Spec") == 0) && (vlr.data != 0))
    {
      if (vlr.record_id == 4) // ExtraBytes
      {
        lvlrsnames.push_back("Extra_Bytes");
        lvlrnames.push_back("Extra Bytes Description");

        List ExtraBytes(0);
        List ExtraBytesnames(0);

        for (int j = 0; j < lasheader->number_attributes; j++)
        {
          LASattribute attemp(lasheader->attributes[j]);

          if (attemp.data_type)
          {
            int data_type = ((I32)(attemp.data_type)-1)%10;
            //int dim = ((I32)(attemp.data_type)-1)/10+1;

            List ExtraByte(0);
            List ExtraBytenames(0);
            ExtraByte.push_back(((I16*)(attemp.reserved))[0]);
            ExtraBytenames.push_back("reserved");
            ExtraByte.push_back((I32)(attemp.data_type));
            ExtraBytenames.push_back("data_type");
            ExtraByte.push_back((I32)(attemp.options));
            ExtraBytenames.push_back("options");
            ExtraByte.push_back(attemp.name);
            ExtraBytenames.push_back("name");

            // 2 and 3 dimensional arrays are deprecated in LASlib
            // (see https://github.com/LAStools/LAStools/blob/master/LASlib/example/lasexample_write_only_with_extra_bytes.cpp)
            double scale = 1.0;
            if(attemp.has_scale())
            {
              scale = attemp.scale[0];
              ExtraByte.push_back(scale);
              ExtraBytenames.push_back("scale");
            }

            double offset = 0.0;
            if(attemp.has_offset())
            {
              offset = attemp.offset[0];
              ExtraByte.push_back(offset);
              ExtraBytenames.push_back("offset");
            }

            if (data_type < 8)
            {
              I64* temp; // as R does not support long long int it is converted to double

              if (attemp.has_no_data())
              {
                temp = ((I64*)(attemp.no_data));
                ExtraByte.push_back(*temp);
                ExtraBytenames.push_back("no_data");
              }

              if (attemp.has_min())
              {
                temp = ((I64*)(attemp.min));
                ExtraByte.push_back(*temp*scale+offset);
                ExtraBytenames.push_back("min");
              }

              if (attemp.has_max())
              {
                temp = ((I64*)(attemp.max));
                ExtraByte.push_back(*temp*scale+offset);
                ExtraBytenames.push_back("max");
              }
            }
            else
            {
              F64* temp;

              if (attemp.has_no_data())
              {
                temp = ((F64*)(attemp.no_data));
                ExtraByte.push_back(*temp*scale+offset);
                ExtraBytenames.push_back("no_data");
              }

              if (attemp.has_min())
              {
                temp = ((F64*)(attemp.min));
                ExtraByte.push_back(*temp*scale+offset);
                ExtraBytenames.push_back("min");
              }

              if (attemp.has_max())
              {
                temp = ((F64*)(attemp.max));
                ExtraByte.push_back(*temp*scale+offset);
                ExtraBytenames.push_back("max");
              }
            }

            ExtraByte.push_back(attemp.description);
            ExtraBytenames.push_back("description");

            ExtraByte.names() =  ExtraBytenames;
            ExtraBytes.push_back(ExtraByte);
            ExtraBytesnames.push_back(attemp.name);
          }
          else
          {
            Rcout << "extra byte " << j << " undocumented: dropped" << std::endl;
          }
        }

        ExtraBytes.names() = ExtraBytesnames;
        lvlr.push_back(ExtraBytes);
      }
      else
      {
        // not supported yet
        lvlrsnames.push_back(vlr.user_id);
      }
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

// [[Rcpp::export]]
void lastransformusage()
{
  try
  {
    LAStransform transform;
    transform.usage();
  }
  catch (std::exception const& e)
  {
    Rcerr << "Error: " << e.what() << std::endl;
  }
}

inline std::string nullterminate(CHAR* str, int len)
{
  if (str[len-1] != '\0')
    return(std::string(str, str+len));
  else
    return(std::string(str));
}