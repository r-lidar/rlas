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

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "lasreader.hpp"
#include "lasfilter.hpp"
#include "lastransform.hpp"
#include "rlas_laslib.h"

using namespace Rcpp;

namespace
{

template <typename T>
T load(const void* ptr)
{
  T value;
  std::memcpy(&value, ptr, sizeof(T));
  return value;
}

std::string nullterminate(const CHAR* str, int len)
{
  if (str[len-1] != '\0')
    return std::string(str, str+len);
  else
    return std::string(str);
}

List globalencodingreader(LASheader* lasheader)
{
  return List::create(Named("GPS Time Type")                  = (bool)lasheader->get_global_encoding_bit(0),
                      Named("Waveform Data Packets Internal") = (bool)lasheader->get_global_encoding_bit(1),
                      Named("Waveform Data Packets External") = (bool)lasheader->get_global_encoding_bit(2),
                      Named("Synthetic Return Numbers")       = (bool)lasheader->get_global_encoding_bit(3),
                      Named("WKT")                            = (bool)lasheader->get_global_encoding_bit(4),
                      Named("Aggregate Model")                = (bool)lasheader->get_global_encoding_bit(5));
}

List geokeysreader(LASheader* lasheader)
{
  List GeoKeys(0);

  for (int j = 0 ; j < lasheader->vlr_geo_keys->number_of_keys ; j++)
  {
    const LASvlr_key_entry& entry = lasheader->vlr_geo_key_entries[j];
    GeoKeys.push_back(List::create(Named("key")               = entry.key_id,
                                   Named("tiff tag location") = entry.tiff_tag_location,
                                   Named("count")             = entry.count,
                                   Named("value offset")      = entry.value_offset));
  }

  return GeoKeys;
}

// Description of one extra bytes attribute. 'fix_description' reads the description as a
// string of at most 32 characters, possibly not null terminated (#53).
List extrabytereader(const LASattribute& attribute, bool fix_description)
{
  int data_type = ((I32)(attribute.data_type)-1)%10;

  List ExtraByte(0);
  CharacterVector names(0);
  auto add = [&](const char* name, const auto& value) { ExtraByte.push_back(value); names.push_back(name); };

  add("reserved", load<I16>(attribute.reserved));
  add("data_type", (I32)(attribute.data_type));
  add("options", (I32)(attribute.options));
  add("name", attribute.name);

  // 2 and 3 dimensional arrays are deprecated in LASlib
  // (see https://github.com/LAStools/LAStools/blob/master/LASlib/example/lasexample_write_only_with_extra_bytes.cpp)
  double scale = 1.0;
  if (attribute.has_scale())
  {
    scale = attribute.scale[0];
    add("scale", scale);
  }

  double offset = 0.0;
  if (attribute.has_offset())
  {
    offset = attribute.offset[0];
    add("offset", offset);
  }

  if (data_type < 8)
  {
    // As R does not support 64 bits integers they are converted to double
    if (attribute.has_no_data()) add("no_data", attribute.no_data[0].i64);
    if (attribute.has_min())     add("min", attribute.min[0].i64*scale+offset);
    if (attribute.has_max())     add("max", attribute.max[0].i64*scale+offset);
  }
  else
  {
    if (attribute.has_no_data()) add("no_data", attribute.no_data[0].f64*scale+offset);
    if (attribute.has_min())     add("min", attribute.min[0].f64*scale+offset);
    if (attribute.has_max())     add("max", attribute.max[0].f64*scale+offset);
  }

  if (fix_description)
  {
    int len = 0;
    while (len < 32 && attribute.description[len] != '\0') len++;
    add("description", std::string(attribute.description, len));
  }
  else
  {
    add("description", attribute.description);
  }

  ExtraByte.names() = names;
  return ExtraByte;
}

List extrabytesreader(LASheader* lasheader, bool fix_description)
{
  List ExtraBytes(0);
  CharacterVector names(0);

  for (int j = 0 ; j < lasheader->number_attributes ; j++)
  {
    const LASattribute& attribute = lasheader->attributes[j];

    if (!attribute.data_type)
    {
      Rcout << "extra byte " << j << " undocumented: dropped" << std::endl;
      continue;
    }

    ExtraBytes.push_back(extrabytereader(attribute, fix_description));
    names.push_back(attribute.name);
  }

  ExtraBytes.names() = names;
  return ExtraBytes;
}

// Reads the VLRs (V = LASvlr) or the EVLRs (V = LASevlr) of a header
template <typename V>
List recordsreader(LASheader* lasheader, const V* records, int nrecords)
{
  constexpr bool is_vlr = std::is_same_v<V, LASvlr>;

  List lvlrs;
  List lvlrsnames;

  for (int i = 0 ; i < nrecords ; i++)
  {
    const V& vlr   = records[i];
    List lvlr      = List::create(vlr.reserved, vlr.user_id, vlr.record_id, vlr.record_length_after_header, vlr.description);
    List lvlrnames = List::create("reserved", "user ID", "record ID", "length after header", "description");

    auto add = [&](const char* name, const char* record_name, const auto& value)
    {
      lvlr.push_back(value);
      lvlrnames.push_back(name);
      lvlrsnames.push_back(record_name);
    };

    const bool has_data = vlr.data != 0;

    if (has_data && std::strcmp(vlr.user_id, "LASF_Projection") == 0)
    {
      switch (vlr.record_id)
      {
        case 34735:
          add("tags", "GeoKeyDirectoryTag", geokeysreader(lasheader));
          break;
        case 34736:
        {
          int n = vlr.record_length_after_header/8;
          NumericVector GeoDouble(lasheader->vlr_geo_double_params, lasheader->vlr_geo_double_params + n);
          add("tags", "GeoDoubleParamsTag", GeoDouble);
          break;
        }
        case 34737:
          add("tags", "GeoAsciiParamsTag", nullterminate(lasheader->vlr_geo_ascii_params, vlr.record_length_after_header));
          break;
        case 2111:
          add("WKT OGC MATH TRANSFORM", "WKT OGC MT", nullterminate(lasheader->vlr_geo_ogc_wkt_math, vlr.record_length_after_header));
          break;
        case 2112:
          add("WKT OGC COORDINATE SYSTEM", "WKT OGC CS", nullterminate(lasheader->vlr_geo_ogc_wkt, vlr.record_length_after_header));
          break;
        default:
          break; // not supported yet
      }
    }
    else if (has_data && std::strcmp(vlr.user_id, "LASF_Spec") == 0)
    {
      if (vlr.record_id == 3) // TextArea
      {
        add("Text Area Description", "TextArea", std::string((const CHAR*)vlr.data, vlr.record_length_after_header));
      }
      else if (vlr.record_id == 4) // ExtraBytes
      {
        add("Extra Bytes Description", "Extra_Bytes", extrabytesreader(lasheader, is_vlr));
      }
      else if (is_vlr && vlr.record_id >= 100 && vlr.record_id < 355) // FWF
      {
        const LASvlr_wave_packet_descr* descr = (const LASvlr_wave_packet_descr*) vlr.data;

        List FWF = List::create(
          Named("Bits per sample")           = (U32)(descr->getBitsPerSample()),
          Named("Waveform compression type") = (U32)(descr->getCompressionType()),
          Named("Number of sample")          = descr->getNumberOfSamples(),
          Named("Temporal Spacing")          = descr->getTemporalSpacing(),
          Named("Digitizer Gain")            = descr->getDigitizerGain(),
          Named("Digitizer Offset")          = descr->getDigitizerOffset());

        add("Full WaveForm", "Full WaveForm Description", FWF);
      }
      else
      {
        lvlrsnames.push_back(vlr.user_id); // not supported yet
      }
    }
    else
    {
      lvlrsnames.push_back(vlr.user_id); // not supported yet
    }

    lvlr.names() = lvlrnames;
    lvlrs.push_back(lvlr);
  }

  lvlrs.names() = lvlrsnames;
  return lvlrs;
}

} // namespace

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
    LASreadOpener lasreadopener;
    lasreadopener.set_file_name(as<std::string>(file).c_str());

    rlas::LASreaderPtr lasreader(lasreadopener.open());

    if (!lasreader)
      throw std::runtime_error("LASlib internal error. See message above.");

    LASheader* lasheader = &lasreader->header;

    char cguid[38];
    std::snprintf(cguid, 38, "%08x-%04x-%04x-%04x-%04x%08x",
                  lasheader->project_ID_GUID_data_1,
                  lasheader->project_ID_GUID_data_2,
                  lasheader->project_ID_GUID_data_3,
                  load<U16>(lasheader->project_ID_GUID_data_4),
                  load<U16>(lasheader->project_ID_GUID_data_4+2),
                  load<U32>(lasheader->project_ID_GUID_data_4+4));

    List head(0);
    CharacterVector names(0);
    auto add = [&](const char* name, const auto& value) { head.push_back(value); names.push_back(name); };

    add("File Signature", lasheader->file_signature);
    add("File Source ID", lasheader->file_source_ID);
    add("Global Encoding", globalencodingreader(lasheader));
    add("Project ID - GUID", CharacterVector(cguid));
    add("Version Major", (int)lasheader->version_major);
    add("Version Minor", (int)lasheader->version_minor);
    add("System Identifier", lasheader->system_identifier);
    add("Generating Software", lasheader->generating_software);
    add("File Creation Day of Year", lasheader->file_creation_day);
    add("File Creation Year", lasheader->file_creation_year);
    add("Header Size", lasheader->header_size);
    add("Offset to point data", lasheader->offset_to_point_data);
    add("Number of variable length records", lasheader->number_of_variable_length_records);
    add("Point Data Format ID", (int)lasheader->point_data_format);
    add("Point Data Record Length", lasheader->point_data_record_length);

    // Added support of LAS 1.4
    const bool las14 = lasheader->version_minor == 4;
    const U64 npoints = las14 ? lasheader->extended_number_of_point_records : lasheader->number_of_point_records;
    const int nreturns = las14 ? 15 : 5;

    if (npoints > 2147483647)
      throw std::runtime_error("R cannot handle more than 2147483647 points.");

    IntegerVector number_of_points_by_return(nreturns);
    for (int i = 0 ; i < nreturns ; i++)
      number_of_points_by_return[i] = las14 ? (int)lasheader->extended_number_of_points_by_return[i] : (int)lasheader->number_of_points_by_return[i];

    add("Number of point records", (int)npoints);
    add("Number of points by return", number_of_points_by_return);
    add("X scale factor", lasheader->x_scale_factor);
    add("Y scale factor", lasheader->y_scale_factor);
    add("Z scale factor", lasheader->z_scale_factor);
    add("X offset", lasheader->x_offset);
    add("Y offset", lasheader->y_offset);
    add("Z offset", lasheader->z_offset);
    add("Max X", lasheader->max_x);
    add("Min X", lasheader->min_x);
    add("Max Y", lasheader->max_y);
    add("Min Y", lasheader->min_y);
    add("Max Z", lasheader->max_z);
    add("Min Z", lasheader->min_z);
    add("Variable Length Records", recordsreader(lasheader, lasheader->vlrs, (int)lasheader->number_of_variable_length_records));
    add("Extended Variable Length Records", recordsreader(lasheader, lasheader->evlrs, (int)lasheader->number_of_extended_variable_length_records));

    head.names() = names;
    return head;
  }
  catch (std::exception const& e)
  {
    Rcerr << "Error: " << e.what() << std::endl;
    return List(0);
  }
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
