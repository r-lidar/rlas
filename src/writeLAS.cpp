#include <Rcpp.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "laswriter.hpp"
#include "rlas_laslib.h"
#include "rlasextrabytesattributes.h"

using namespace Rcpp;

namespace
{

// An optional point attribute of the data.frame to write. A column that is absent or not
// allowed by the point format is empty and inactive. A column of length 1 (a compact
// ALTREP repetition) is constant: it is written once into the LASpoint and then deactivated.
template <typename V>
struct Column
{
  V v;
  bool active;

  Column(const List& data, const char* name, bool allowed = true)
  {
    active = allowed && data.containsElementNamed(name);
    v = active ? V(data[name]) : V(0);
  }

  bool is_constant() const { return v.size() == 1; }

  // If the column is constant, set its value once with 'setter' and stop writing it point by point
  template <typename F>
  void set_if_constant(F setter)
  {
    if (!is_constant()) return;
    active = false;
    setter(v[0]);
  }
};

void set_global_encoding(LASheader& header, List encoding)
{
  const char* bits[] = {"GPS Time Type", "Waveform Data Packets Internal", "Waveform Data Packets External",
                        "Synthetic Return Numbers", "WKT", "Aggregate Model"};

  for (int bit = 0 ; bit < 6 ; bit++)
  {
    if (encoding[bits[bit]])
      header.set_global_encoding_bit(bit);
  }
}

// Parses a GUID formatted like "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
void set_guid(LASheader& header, const std::string& guid)
{
  long long unsigned int data_1 = 0;
  unsigned int data_2 = 0;
  unsigned int data_3 = 0;
  unsigned int data_4a = 0;
  long long unsigned int data_4b = 0;

  std::sscanf(guid.c_str(), "%llx-%x-%x-%x-%llx", &data_1, &data_2, &data_3, &data_4a, &data_4b);

  U16 data_4a_16  = U16_CLAMP(data_4a);
  U16 data_4b_hi  = U16_CLAMP(data_4b >> 32);
  U32 data_4b_low = U32_CLAMP(data_4b & 0xFFFFFFFF);

  header.project_ID_GUID_data_1 = U32_CLAMP(data_1);
  header.project_ID_GUID_data_2 = U16_CLAMP(data_2);
  header.project_ID_GUID_data_3 = U16_CLAMP(data_3);
  std::memcpy(header.project_ID_GUID_data_4,     &data_4a_16,  2);
  std::memcpy(header.project_ID_GUID_data_4 + 2, &data_4b_hi,  2);
  std::memcpy(header.project_ID_GUID_data_4 + 4, &data_4b_low, 4);
}

void add_geokeys(LASheader& header, const List& vlr)
{
  List tags = vlr["tags"];
  std::vector<LASvlr_key_entry> keys(tags.size());

  for (int j = 0 ; j < tags.size() ; j++)
  {
    List tag = tags[j];
    keys[j].key_id            = (U16)tag["key"];
    keys[j].tiff_tag_location = (U16)tag["tiff tag location"];
    keys[j].count             = (U16)tag["count"];
    keys[j].value_offset      = (U16)tag["value offset"];
  }

  header.set_geo_keys(keys.size(), keys.data());
}

void add_text_area(LASheader& header, const List& vlr)
{
  std::string text = as<std::string>(vlr["Text Area Description"]);
  std::string desc = as<std::string>(vlr["description"]);

  // Truncate strings larger than U16_MAX and ensure that the text is null terminated
  if (text.size() >= U16_MAX)
  {
    text.resize(U16_MAX);
    text.back() = '\0';
  }

  if (text.empty() || text.back() != '\0')
    text.push_back('\0');

  // The header takes the ownership of the data
  CHAR* data = new CHAR[text.size()];
  std::memcpy(data, text.data(), text.size());

  header.add_vlr("LASF_Spec", 3, (U16)text.size(), (U8*)data, FALSE, desc.c_str());
}

// Adds the extra bytes attributes described in the VLR and binds them to their data
void add_extra_bytes(LASheader& header, const List& vlr, const List& data, std::vector<RLASExtrabyteAttributes>& attributes)
{
  List descriptions = vlr["Extra Bytes Description"];

  for (List description : descriptions)
  {
    RLASExtrabyteAttributes eb;

    eb.name = as<std::string>(description["name"]);

    if (!data.containsElementNamed(eb.name.c_str()))
      stop("Extra Bytes described but not present in data.");

    eb.options = (int)description["options"];
    eb.parse_options();

    // Deprecated tuples and triples (data_type > 10) are written as scalars
    eb.data_type = ((int)(description["data_type"]) - 1) % 10 + 1;
    eb.desc = as<std::string>(description["description"]);

    if (eb.has_scale)   eb.scale   = (double)(description["scale"]);
    if (eb.has_offset)  eb.offset  = (double)(description["offset"]);

    // no_data, min and max are stored unscaled
    auto unscale = [&](const char* field) { return ((double)(description[field]) - eb.offset)/eb.scale; };
    if (eb.has_no_data) eb.no_data = unscale("no_data");
    if (eb.has_min)     eb.min     = unscale("min");
    if (eb.has_max)     eb.max     = unscale("max");

    eb.id  = header.add_attribute(eb.make_LASattribute());
    eb.Reb = as<NumericVector>(data[eb.name.c_str()]);
    attributes.push_back(eb);
  }

  header.update_extra_bytes_vlr();
  header.point_data_record_length += header.get_attributes_size();

  for (auto& eb : attributes)
    eb.start = header.get_attribute_start(eb.id);
}

void add_vlrs(LASheader& header, const List& lasheader, const List& data, std::vector<RLASExtrabyteAttributes>& extrabytes)
{
  for (bool in_evlr : {false, true})
  {
    const char* record = in_evlr ? "Extended Variable Length Records" : "Variable Length Records";

    if (!lasheader.containsElementNamed(record))
      continue;

    List vlrs = lasheader[record];

    if (vlrs.size() == 0)
      continue;

    CharacterVector names = vlrs.names();

    for (int i = 0 ; i < vlrs.size() ; ++i)
    {
      List vlr = vlrs[i];
      std::string name = as<std::string>(names[i]);

      if (name == "GeoKeyDirectoryTag")
      {
        add_geokeys(header, vlr);
      }
      else if (name == "GeoAsciiParamsTag" && vlr.containsElementNamed("tags"))
      {
        std::string ascii = as<std::string>(vlr["tags"]);
        header.set_geo_ascii_params(ascii.size(), ascii.c_str());
      }
      else if (name == "GeoDoubleParamsTag" && vlr.containsElementNamed("tags"))
      {
        std::vector<double> params = as<std::vector<double>>(vlr["tags"]);
        header.set_geo_double_params(params.size(), params.data());
      }
      else if (name == "WKT OGC CS" && vlr.containsElementNamed("WKT OGC COORDINATE SYSTEM"))
      {
        std::string wkt = as<std::string>(vlr["WKT OGC COORDINATE SYSTEM"]);
        header.set_geo_ogc_wkt(wkt.size(), wkt.c_str(), in_evlr);
      }
      else if (name == "Extra_Bytes" && vlr.containsElementNamed("Extra Bytes Description"))
      {
        add_extra_bytes(header, vlr, data, extrabytes);
      }
      else if (name == "TextArea" && vlr.containsElementNamed("Text Area Description"))
      {
        add_text_area(header, vlr);
      }
    }
  }
}

} // namespace

// [[Rcpp::export]]
void C_writer(CharacterVector file, List LASheader, List data)
{
  int format = (int)LASheader["Point Data Format ID"];
  if (rlas::point_format::has_waveform(format))
    Rcpp::stop("Point format with full waveform are not supported yet");

  // ===========================
  // 1. Public Header Block
  // ===========================

  class LASheader header;

  header.file_source_ID       = (int)LASheader["File Source ID"];
  header.version_major        = (int)LASheader["Version Major"];
  header.version_minor        = (int)LASheader["Version Minor"];
  header.header_size          = (int)LASheader["Header Size"];
  header.offset_to_point_data = header.header_size;
  header.file_creation_year   = (int)LASheader["File Creation Year"];
  header.file_creation_day    = (int)LASheader["File Creation Day of Year"];
  header.point_data_format    = (int)LASheader["Point Data Format ID"];
  header.x_scale_factor       = (double)LASheader["X scale factor"];
  header.y_scale_factor       = (double)LASheader["Y scale factor"];
  header.z_scale_factor       = (double)LASheader["Z scale factor"];
  header.x_offset             = (double)LASheader["X offset"];
  header.y_offset             = (double)LASheader["Y offset"];
  header.z_offset             = (double)LASheader["Z offset"];

  const bool extended = (header.version_minor >= 4) && (header.point_data_format >= 6);

  header.point_data_record_length = rlas::point_format::record_length(header.point_data_format);
  if (header.point_data_record_length < 0)
    stop("point_data_format out of range.");

  std::snprintf(header.generating_software, sizeof(header.generating_software), "%s", "rlas R package");
  set_guid(header, as<std::string>(LASheader["Project ID - GUID"]));
  set_global_encoding(header, LASheader["Global Encoding"]);

  // ===============================
  // 2. VLRs and EVLRs
  // ===============================

  std::vector<RLASExtrabyteAttributes> extrabytes;
  add_vlrs(header, LASheader, data, extrabytes);

  // ===============================
  // 3. Write the points
  // ===============================

  LASwriteOpener laswriteopener;
  laswriteopener.set_file_name(as<std::string>(file).c_str());

  LASpoint point;
  point.init(&header, header.point_data_format, header.point_data_record_length, 0);

  rlas::LASwriterPtr laswriter(laswriteopener.open(&header));

  if (!laswriter)
    stop("LASlib internal error. See message above.");

  NumericVector X = data["X"];
  NumericVector Y = data["Y"];
  NumericVector Z = data["Z"];
  Column<IntegerVector> I  (data, "Intensity");
  Column<IntegerVector> RN (data, "ReturnNumber");
  Column<IntegerVector> NR (data, "NumberOfReturns");
  Column<IntegerVector> D  (data, "ScanDirectionFlag");
  Column<IntegerVector> E  (data, "EdgeOfFlightline");
  Column<IntegerVector> C  (data, "Classification");
  Column<LogicalVector> S  (data, "Synthetic_flag");
  Column<LogicalVector> K  (data, "Keypoint_flag");
  Column<LogicalVector> W  (data, "Withheld_flag");
  Column<LogicalVector> O  (data, "Overlap_flag", extended);
  Column<IntegerVector> U  (data, "UserData");
  Column<IntegerVector> P  (data, "PointSourceID");
  Column<NumericVector> T  (data, "gpstime");
  Column<IntegerVector> Red(data, "R");
  Column<IntegerVector> Gre(data, "G");
  Column<IntegerVector> Blu(data, "B");
  Column<IntegerVector> NIR(data, "NIR");
  Column<IntegerVector> SAR(data, "ScanAngleRank");
  Column<NumericVector> ESA(data, "ScanAngle");
  Column<IntegerVector> CHA(data, "ScannerChannel", extended);

  // The following attributes can be ALTREPed at R level to be represented as compact
  // repetitions. In this case we received a single value instead of a vector of n times
  // the same value. The LASpoint is initialized once with this value.
  if (!extended)
  {
    RN.set_if_constant ([&](int v) { point.set_return_number((U8)v); });
    NR.set_if_constant ([&](int v) { point.set_number_of_returns((U8)v); });
    C.set_if_constant  ([&](int v) { point.set_classification((U8)v); });
    SAR.set_if_constant([&](int v) { point.set_scan_angle_rank((U8)v); });
  }
  else
  {
    RN.set_if_constant ([&](int v) { point.set_extended_return_number((U8)v); });
    NR.set_if_constant ([&](int v) { point.set_extended_number_of_returns((U8)v); });
    C.set_if_constant  ([&](int v) { point.set_extended_classification((U8)v); });
    ESA.set_if_constant([&](double v) { point.set_extended_scan_angle((I16)((v/0.006f))); });
  }
  D.set_if_constant([&](int v) { point.set_scan_direction_flag((U8)v); });
  E.set_if_constant([&](int v) { point.set_edge_of_flight_line((U8)v); });
  S.set_if_constant([&](int v) { point.set_synthetic_flag((U8)v); });
  K.set_if_constant([&](int v) { point.set_keypoint_flag((U8)v); });
  W.set_if_constant([&](int v) { point.set_withheld_flag((U8)v); });
  O.set_if_constant([&](int v) { point.set_extended_overlap_flag((U8)v); });
  U.set_if_constant([&](int v) { point.set_user_data((U8)v); });
  P.set_if_constant([&](int v) { point.set_point_source_ID((U16)v); });

  for (R_xlen_t j = 0 ; j < X.length() ; j++)
  {
    point.set_x(X[j]);
    point.set_y(Y[j]);
    point.set_z(Z[j]);

    if (I.active) point.set_intensity((U16)I.v[j]);

    if (RN.active)
    {
      if (extended) point.set_extended_return_number((U8)RN.v[j]);
      else          point.set_return_number((U8)RN.v[j]);
    }

    if (NR.active)
    {
      if (extended) point.set_extended_number_of_returns((U8)NR.v[j]);
      else          point.set_number_of_returns((U8)NR.v[j]);
    }

    if (D.active) point.set_scan_direction_flag((U8)D.v[j]);
    if (E.active) point.set_edge_of_flight_line((U8)E.v[j]);

    if (C.active)
    {
      if (extended) point.set_extended_classification((U8)C.v[j]);
      else          point.set_classification((U8)C.v[j]);
    }

    if (CHA.active) point.set_extended_scanner_channel((U8)CHA.v[j]);
    if (S.active)   point.set_synthetic_flag((U8)S.v[j]);
    if (K.active)   point.set_keypoint_flag((U8)K.v[j]);
    if (W.active)   point.set_withheld_flag((U8)W.v[j]);
    if (O.active)   point.set_extended_overlap_flag((U8)O.v[j]);
    if (SAR.active) point.set_scan_angle_rank((I8)SAR.v[j]);
    if (ESA.active) point.set_extended_scan_angle((I16)((ESA.v[j]/0.006f)));
    if (U.active)   point.set_user_data((U8)U.v[j]);
    if (P.active)   point.set_point_source_ID((U16)P.v[j]);
    if (T.active)   point.set_gps_time((F64)T.v[j]);
    if (Red.active) point.set_R((U16)Red.v[j]);
    if (Gre.active) point.set_G((U16)Gre.v[j]);
    if (Blu.active) point.set_B((U16)Blu.v[j]);
    if (NIR.active) point.set_NIR((U16)NIR.v[j]);

    for (auto& eb : extrabytes)
      eb.set_attribute(j, &point);

    laswriter->write_point(&point);
    laswriter->update_inventory(&point);
  }

  laswriter->update_header(&header, true);
}
