#include <Rcpp.h>
#include <string.h>

#include "laswriter.hpp"
#include "rlasextrabytesattributes.h"

using namespace Rcpp;

int  get_point_data_record_length(int x);
void set_guid(LASheader&, const char*);
void set_global_enconding(LASheader&, List);

// [[Rcpp::export]]
void C_writer(CharacterVector file, List LASheader, List data)
{

  int format = (int)LASheader["Point Data Format ID"];
  if (format == 4 || format == 5 || format == 9 || format == 10)
    Rcpp::stop("Point format with full waveform are not supported yet");

  class LASheader header;

  // ===========================
  // Public Header Block
  // ===========================

  // 1.1. These ones are easy

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

  bool extended = (header.version_minor >= 4) && (header.point_data_format >= 6);

  // 1.2. These one need special interpretation

  header.point_data_record_length = get_point_data_record_length(header.point_data_format);
  strcpy(header.generating_software, "rlas R package");

  CharacterVector guid = LASheader["Project ID - GUID"];
  std::string stdguid  = as<std::string>(guid);
  set_guid(header, stdguid.c_str());

  set_global_enconding(header, LASheader["Global Encoding"]);

  // ===============================
  // 2. VLRS and EVLRS
  // ===============================

  std::vector<RLASExtrabyteAttributes> ExtraBytesAttr;

  for(auto extended : {false, true})
  {
    auto record = (!extended) ? "Variable Length Records" : "Extended Variable Length Records";
    bool is_set_record = LASheader.containsElementNamed(record);

    List vlrs = (is_set_record) ? LASheader[record] : List(0);

    int n = vlrs.size();
    if (n == 0) continue;

    CharacterVector names = vlrs.names();

    for (int i = 0; i < n; ++i)
    {
      List vlr = vlrs[i];

      if (names[i] == "GeoKeyDirectoryTag")
      {
        List tags = vlr["tags"];
        int ntags = tags.size();
        LASvlr_key_entry* vlr_epsg = new LASvlr_key_entry[ntags];
        for (int j = 0 ; j < ntags ; j ++)
        {
          List tag = tags[j];
          vlr_epsg[j].key_id = (U16)tag["key"];
          vlr_epsg[j].tiff_tag_location = (U16)tag["tiff tag location"];
          vlr_epsg[j].count = (U16)tag["count"];
          vlr_epsg[j].value_offset = (U16)tag["value offset"];
        }

        header.set_geo_keys(ntags, vlr_epsg);
        delete[] vlr_epsg;
      }
      else if (names[i] == "GeoAsciiParamsTag")
      {
        if (vlr.containsElementNamed("tags"))
        {
          CharacterVector ASCII = vlr["tags"];
          std::string sascii    = as<std::string>(ASCII);
          header.set_geo_ascii_params(sascii.size(), sascii.c_str());
        }
      }
      else if (names[i] == "GeoDoubleParamsTag")
      {
        if (vlr.containsElementNamed("tags"))
        {
          NumericVector DOUBLE = vlr["tags"];
          std::vector<double> sdouble = as<std::vector<double> >(DOUBLE);
          header.set_geo_double_params(sdouble.size(), &sdouble[0]);
        }
      }
      else if (names[i] == "WKT OGC CS")
      {
        if (vlr.containsElementNamed("WKT OGC COORDINATE SYSTEM"))
        {
          CharacterVector WKT = vlr["WKT OGC COORDINATE SYSTEM"];
          std::string sWKT    = as<std::string>(WKT);
          header.set_geo_ogc_wkt(sWKT.size(), sWKT.c_str(), extended);
        }
      }
      else if (names[i] == "Extra_Bytes")
      {
        if (vlr.containsElementNamed("Extra Bytes Description"))
        {
          List description_eb = vlr["Extra Bytes Description"];

          for(int i = 0 ; i < description_eb.size() ; i++)
          {
            List description = description_eb[i];
            RLASExtrabyteAttributes ExtraByte;

            ExtraByte.name = as< std::string >(description["name"]);

            if (!data.containsElementNamed(ExtraByte.name.c_str()))
              stop("Extra Bytes described but not present in data.");

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
      else if (names[i] == "TextArea")
      {
        if (vlr.containsElementNamed("Text Area Description"))
        {
          CharacterVector TEXT = vlr["Text Area Description"];
          std::string stext    = as<std::string>(TEXT);
          CharacterVector desc  = vlr["description"];
          std::string sdesc  = as<std::string>(desc);
          I32 stext_size = stext.size();

          // truncate strings larger than U16_MAX, and ensure it is null terminated
          if ((sizeof(CHAR)*stext_size) >= U16_MAX)
          {
            stext_size = U16_MAX;
            stext[stext_size-1]='\0';
          }
          
          // ensure that any text area description is null terminated.
          I32 null_terminator = 0;
          CHAR* vlr_text_area_desc;
          if (stext[stext_size-1] == '\0')
          {
             vlr_text_area_desc = new CHAR[stext_size];
          }
          else
          {
            null_terminator = 1;
            vlr_text_area_desc = new CHAR[stext_size+1];
            vlr_text_area_desc[stext_size] = '\0';
          }
          memcpy(vlr_text_area_desc, stext.c_str(), sizeof(CHAR)*stext_size);

          header.add_vlr("LASF_Spec", 3, (U16)(sizeof(CHAR)*(stext_size+null_terminator)), (U8*)vlr_text_area_desc, FALSE, sdesc.c_str());
        }
      }
      else
      {
      }
    }
  }

  // ===============================
  // 3. Write the data into the file
  // ===============================

  LASwriteOpener laswriteopener;
  laswriteopener.set_file_name(as<std::string>(file).c_str());

  LASpoint point;
  point.init(&header, header.point_data_format, header.point_data_record_length, 0);

  LASwriter* laswriter = laswriteopener.open(&header);

  if(0 == laswriter || NULL == laswriter)
    stop("LASlib internal error. See message above.");

  #define ISSET(NAME) data.containsElementNamed(NAME)

  bool i = ISSET("Intensity");
  bool r = ISSET("ReturnNumber");
  bool n = ISSET("NumberOfReturns");
  bool d = ISSET("ScanDirectionFlag");
  bool e = ISSET("EdgeOfFlightline");
  bool c = ISSET("Classification");
  bool s = ISSET("Synthetic_flag");
  bool k = ISSET("Keypoint_flag");
  bool w = ISSET("Withheld_flag");
  bool o = ISSET("Overlap_flag") && extended;
  bool u = ISSET("UserData");
  bool p = ISSET("PointSourceID");
  bool t = ISSET("gpstime");
  bool R = ISSET("R");
  bool G = ISSET("G");
  bool B = ISSET("B");
  bool N = ISSET("NIR");
  bool sar = ISSET("ScanAngleRank");
  bool esa = ISSET("ScanAngle");
  bool cha = ISSET("ScannerChannel") && extended;

  NumericVector X   = data["X"];
  NumericVector Y   = data["Y"];
  NumericVector Z   = data["Z"];
  IntegerVector I   = (i) ? data["Intensity"] : IntegerVector(0);
  IntegerVector RN  = (r) ? data["ReturnNumber"] : IntegerVector(0);
  IntegerVector NR  = (n) ? data["NumberOfReturns"] : IntegerVector(0);
  IntegerVector D   = (d) ? data["ScanDirectionFlag"] : IntegerVector(0);
  IntegerVector E   = (e) ? data["EdgeOfFlightline"] : IntegerVector(0);
  IntegerVector C   = (c) ? data["Classification"] : IntegerVector(0);
  LogicalVector S   = (s) ? data["Synthetic_flag"] : LogicalVector(0);
  LogicalVector K   = (k) ? data["Keypoint_flag"] : LogicalVector(0);
  LogicalVector W   = (w) ? data["Withheld_flag"] : LogicalVector(0);
  LogicalVector O   = (o) ? data["Overlap_flag"] : LogicalVector(0);
  IntegerVector U   = (u) ? data["UserData"] : IntegerVector(0);
  IntegerVector P   = (p) ? data["PointSourceID"] : IntegerVector(0);
  NumericVector T   = (t) ? data["gpstime"] : NumericVector(0);
  IntegerVector Red = (R) ? data["R"] : IntegerVector(0);
  IntegerVector Gre = (G) ? data["G"] : IntegerVector(0);
  IntegerVector Blu = (B) ? data["B"] : IntegerVector(0);
  IntegerVector NIR = (N) ? data["NIR"] : IntegerVector(0);
  IntegerVector SAR = (sar) ? data["ScanAngleRank"] : IntegerVector(0);
  NumericVector ESA = (esa) ? data["ScanAngle"] : NumericVector(0);
  IntegerVector CHA = (cha) ? data["ScannerChannel"] : IntegerVector(0);

  // The following attributes can be ALTREPed at R level to be represented as compact
  // repetition. If they were ALTREPed then we received a single value instead of a vector
  // of n times the same value. In this case we initialize the LASpoint with the value and we
  // declare the attribute as missing to avoid looping through undefined indices.
  if (!extended)
  {
    if (RN.size() == 1) { r = false; point.set_return_number((U8)RN[0]); }
    if (NR.size() == 1) { n = false; point.set_number_of_returns((U8)NR[0]); }
    if (C.size() == 1)  { c = false; point.set_classification((U8)C[0]); }
    if (SAR.size() == 1){ sar = false; point.set_scan_angle_rank((U8)SAR[0]); }
  }
  else
  {
    if (RN.size() == 1) { r = false; point.set_extended_return_number((U8)RN[0]); }
    if (NR.size() == 1) { n = false; point.set_extended_number_of_returns((U8)NR[0]); }
    if (C.size() == 1)  { c = false; point.set_extended_classification((U8)C[0]); }
    if (ESA.size() == 1){ esa = false; point.set_extended_scan_angle((I16)((ESA[0]/0.006f))); }
  }
  if (D.size() == 1) { d = false; point.set_scan_direction_flag((U8)D[0]); }
  if (E.size() == 1) { e = false; point.set_edge_of_flight_line((U8)E[0]); }
  if (S.size() == 1) { s = false; point.set_synthetic_flag((U8)S[0]); }
  if (K.size() == 1) { k = false; point.set_keypoint_flag((U8)K[0]); }
  if (W.size() == 1) { w = false; point.set_withheld_flag((U8)W[0]); }
  if (O.size() == 1) { o = false; point.set_extended_overlap_flag((U8)O[0]); }
  if (U.size() == 1) { u = false; point.set_user_data((U8)U[0]); }
  if (P.size() == 1) { p = false; point.set_point_source_ID((U16)P[0]); }

  for(int j = 0 ; j < X.length() ; j++)
  {
    // Add regular data
    point.set_x(X[j]);
    point.set_y(Y[j]);
    point.set_z(Z[j]);

    if(i) { point.set_intensity((U16)I[j]); }

    if(r && !extended) { point.set_return_number((U8)RN[j]); }
    if(r &&  extended) { point.set_extended_return_number((U8)RN[j]); }

    if(n && !extended) { point.set_number_of_returns((U8)NR[j]); }
    if(n &&  extended) { point.set_extended_number_of_returns((U8)NR[j]); }

    if(d) { point.set_scan_direction_flag((U8)D[j]); }
    if(e) { point.set_edge_of_flight_line((U8)E[j]); }

    if(c && !extended) { point.set_classification((U8)C[j]); }
    if(c &&  extended) { point.set_extended_classification((U8)C[j]); }

    if(cha) { point.set_extended_scanner_channel((U8)CHA[j]); }

    if(s) { point.set_synthetic_flag((U8)S[j]); }
    if(k) { point.set_keypoint_flag((U8)K[j]); }
    if(w) { point.set_withheld_flag((U8)W[j]); }
    if(o) { point.set_extended_overlap_flag((U8)O[j]); }

    if(sar) { point.set_scan_angle_rank((I8)SAR[j]); }
    if(esa) { point.set_extended_scan_angle((I16)((ESA[j]/0.006f))); }

    if(u) { point.set_user_data((U8)U[j]); }
    if(p) { point.set_point_source_ID((U16)P[j]); }
    if(t) { point.set_gps_time((F64)T[j]); }
    if(R) { point.set_R((U16)Red[j]); }
    if(G) { point.set_G((U16)Gre[j]); }
    if(B) { point.set_B((U16)Blu[j]); }
    if(N) { point.set_NIR((U16)NIR[j]); }

    // Add extra bytes
    for(auto& ExtraByte : ExtraBytesAttr)
      ExtraByte.set_attribute(j, &point);

    laswriter->write_point(&point);
    laswriter->update_inventory(&point);
  }

  laswriter->update_header(&header, true);
  laswriter->close();
  delete laswriter;
}

void set_global_enconding(LASheader &header, List encoding)
{
  if (encoding["GPS Time Type"]) header.set_global_encoding_bit(0);
  if (encoding["Waveform Data Packets Internal"]) header.set_global_encoding_bit(1);
  if (encoding["Waveform Data Packets External"]) header.set_global_encoding_bit(2);
  if (encoding["Synthetic Return Numbers"]) header.set_global_encoding_bit(3);
  if (encoding["WKT"]) header.set_global_encoding_bit(4);
  if (encoding["Aggregate Model"]) header.set_global_encoding_bit(5);
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
  case 0: return 20; break;
  case 1: return 28; break;
  case 2: return 26; break;
  case 3: return 34; break;
  case 4: return 57; break;
  case 5: return 63; break;
  case 6: return 30; break;
  case 7: return 36; break;
  case 8: return 38; break;
  case 9: return 59; break;
  case 10: return 67; break;
  default: stop("point_data_format out of range."); break;
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
