/*
 ===============================================================================

 PROGRAMMERS:

 jean-romain.roussel.1@ulaval.ca  -  https://github.com/Jean-Romain/rlas

 COPYRIGHT:

 Copyright 2017 Jean-Romain Roussel

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


using namespace Rcpp;

List vlrsreader(LASheader*);

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
        lvlr.push_back(std::string(reinterpret_cast< char const* >(vlr.data)));
        lvlrnames.push_back("tags");
        lvlrsnames.push_back("GeoAsciiParamsTag");
      }
      else if (vlr.record_id == 2111) // WKT OGC MATH TRANSFORM
      {
        lvlr.push_back(std::string(reinterpret_cast< char const* >(vlr.data)));
        lvlrnames.push_back("WKT OGC MATH TRANSFORM");
        lvlrsnames.push_back("WKT OGC MT");
      }
      else if (vlr.record_id == 2112) // WKT OGC COORDINATE SYSTEM
      {
        lvlr.push_back(std::string(reinterpret_cast< char const* >(vlr.data)));
        lvlrnames.push_back("WKT OGC COORDINATE SYSTEM");
        lvlrsnames.push_back("WKT OGC CS");
      }
    }
    else if ((strcmp(vlr.user_id, "LASF_Spec") == 0) && (vlr.data != 0))
    {
        if (vlr.record_id == 4) // ExtraBytes
        {

          lvlrsnames.push_back("Extra_Bytes");
          const char* name_table[10] = { "unsigned char", "char", "unsigned short", "short", "unsigned long", "long", "unsigned long long", "long long", "float", "double" };

          lvlrnames.push_back("Extra Bytes Description");

          List ExtraBytes(0);
          List ExtraBytesnames(0);

          for (int j = 0; j < lasheader->number_attributes; j++)
          {
            LASattribute attemp(lasheader->attributes[j]);

            if (attemp.data_type)
            {
              int type = ((I32)(attemp.data_type)-1)%10;
              int dim = ((I32)(attemp.data_type)-1)/10+1;



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
              if (type < 8)
              { I64* temp; // as R does not support ong long int it is converted to double
                if (attemp.has_no_data())
                {
                  temp = ((I64*)(attemp.no_data));
                  std::vector<double> no_data(temp, temp + sizeof temp / sizeof temp[0]);
                  ExtraByte.push_back(no_data);
                  ExtraBytenames.push_back("no_data");
                }
                if (attemp.has_min())
                {
                  temp = ((I64*)(attemp.min));
                  std::vector<double> min(temp, temp + sizeof temp / sizeof temp[0]);
                  ExtraByte.push_back(min);
                  ExtraBytenames.push_back("min");
                }
                if (attemp.has_max())
                {
                  temp = ((I64*)(attemp.max));
                  std::vector<double> max(temp, temp + sizeof temp / sizeof temp[0]);
                  ExtraByte.push_back(max);
                  ExtraBytenames.push_back("max");
                }
              }
              else
              {
                F64* temp;
                if (attemp.has_no_data())
                {
                  temp = ((F64*)(attemp.no_data));
                  std::vector<double> no_data(temp, temp + sizeof temp / sizeof temp[0]);
                  ExtraByte.push_back(no_data);
                  ExtraBytenames.push_back("no_data");
                }
                if (attemp.has_min())
                {
                  temp = ((F64*)(attemp.min));
                  std::vector<double> min(temp, temp + sizeof temp / sizeof temp[0]);
                  ExtraByte.push_back(min);
                  ExtraBytenames.push_back("min");
                }
                if (attemp.has_max())
                {
                  temp = ((F64*)(attemp.max));
                  std::vector<double> max(temp, temp + sizeof temp / sizeof temp[0]);
                  ExtraByte.push_back(max);
                  ExtraBytenames.push_back("max");
                }
              }

              if(attemp.has_scale()){
                std::vector<double> scale(attemp.scale, attemp.scale + sizeof attemp.scale / sizeof attemp.scale[0]);
                ExtraByte.push_back(scale);
                ExtraBytenames.push_back("scale");
              }
              if(attemp.has_offset()){
                std::vector<double> offset(attemp.offset, attemp.offset + sizeof attemp.offset / sizeof attemp.offset[0]);
                ExtraByte.push_back(offset);
                ExtraBytenames.push_back("offset");
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
        else{
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