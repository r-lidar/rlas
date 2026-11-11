#include "rlasstreamer.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <type_traits>

using namespace Rcpp;

RLASstreamer::RLASstreamer(CharacterVector ifiles, CharacterVector ofile, CharacterVector filter)
{
  set_input_files(ifiles);
  set_filter(filter);
  set_output_file(ofile);
  open();
}

// The destructor only runs without terminate() when an error was raised while streaming.
// It must not throw nor call R: it only finalizes and releases the LASlib objects.
RLASstreamer::~RLASstreamer()
{
  if (!ended && laswriter)
    laswriter->update_header(header, true); // # nocov
}

void RLASstreamer::set_input_files(CharacterVector ifiles)
{
  if (ifiles.length() == 0)
    stop("Input files vector is null");

  lasreadopener.set_merged(true);
  lasreadopener.set_populate_header(true);
  lasreadopener.set_copc_stream_ordered_by_chunk();

  for (const auto& file : ifiles)
    lasreadopener.add_file_name(as<std::string>(file).c_str());
}

void RLASstreamer::set_output_file(CharacterVector ofile)
{
  if (ofile.length() == 0)
    stop("Output file vector is empty.");

  if (ofile.length() > 1)
    stop("Cannot write several files at one time.");

  std::string path = as<std::string>(ofile);

  if (path.empty())
    return;

  if (!use_filter)
    stop("Writing an output file without filter is useless.");

  laswriteopener.set_file_name(path.c_str());
  in_R = false;
}

void RLASstreamer::set_filter(CharacterVector filter)
{
  if (filter.length() == 0)
    stop("Filter vector is empty.");

  if (filter.length() > 1)
    stop("Filter must have a length 1.");

  std::string str = as<std::string>(filter);

  if (str.empty())
    return;

  if (!lasreadopener.parse_str(str.data()))
  {
    lasreadopener.set_filter(0); // # nocov
    stop("Filter error see message above."); // # nocov
  }

  use_filter = true;
}

void RLASstreamer::open()
{
  lasreader.reset(lasreadopener.open());

  if (!lasreader)
    stop("LASlib internal error. See message above."); // # nocov

  header = &lasreader->header;

  if (rlas::point_format::has_waveform(header->point_data_format))
    laswaveform13reader.reset(lasreadopener.open_waveform13(header));

  if (!in_R)
  {
    format = header->point_data_format;
    laswriter.reset(laswriteopener.open(header));

    if (!laswriter)
      stop("LASlib internal error. See message above."); // # nocov

    return;
  }

  format = header->point_data_format;
  if (!rlas::point_format::is_valid(format))
    throw std::runtime_error("LAS format not valid.");

  extended = (header->version_minor >= 4) && (format >= 6);

  int npoints = header->number_of_point_records;
  nalloc = use_filter ? std::ceil((float)npoints/8) : npoints;
}

// The select string contains one letter per attribute (e.g. "xyzirn"), '*' for everything,
// digits for extra bytes ('0' for all) and '-' to unselect (e.g. "* -i").
void RLASstreamer::select(CharacterVector string)
{
  const std::string unselect = as<std::string>(string);
  std::string selected = unselect;

  if (selected.find('*') != std::string::npos)
    selected = "xyztirndecCskwoaupRGBNW0";

  auto keep = [&](char c)
  {
    return selected.find(c) != std::string::npos && unselect.find(std::string("-") + c) == std::string::npos;
  };

  using namespace rlas::point_format;

  sel.gpstime            = keep('t') && has_gpstime(format);
  sel.intensity          = keep('i');
  sel.return_number      = keep('r');
  sel.number_of_returns  = keep('n');
  sel.scan_direction     = keep('d');
  sel.edge_of_flightline = keep('e');
  sel.classification     = keep('c');
  sel.synthetic          = keep('s');
  sel.keypoint           = keep('k');
  sel.withheld           = keep('w');
  sel.overlap            = keep('o') && extended;
  sel.scan_angle         = keep('a');
  sel.user_data          = keep('u');
  sel.point_source_id    = keep('p');
  sel.nir                = keep('N') && has_nir(format);
  sel.scanner_channel    = keep('C') && extended;
  sel.waveform           = keep('W') && has_waveform(format);

  // R, G and B are a single attribute: any of them selects the three, any of them unselected removes the three
  bool rgb_selected   = selected.find_first_of("RGB") != std::string::npos;
  bool rgb_unselected = unselect.find("-R") != std::string::npos ||
                        unselect.find("-G") != std::string::npos ||
                        unselect.find("-B") != std::string::npos;
  sel.rgb = rgb_selected && !rgb_unselected && has_rgb(format);

  // Extra bytes: '1' to '9' are the 9 first attributes, '0' means all of them
  const bool all_eb   = selected.find('0') != std::string::npos;
  const bool none_eb  = unselect.find("-0") != std::string::npos;

  sel.extra_bytes.clear();
  for (int j = 0 ; j < 9 ; j++)
  {
    const char digit = '1' + j;
    bool keep_eb = all_eb || selected.find(digit) != std::string::npos;

    if (none_eb || unselect.find(std::string("-") + digit) != std::string::npos)
      keep_eb = false;

    if (keep_eb && j < header->number_attributes)
      sel.extra_bytes.push_back(j);
  }
}

void RLASstreamer::allocation()
{
  if (!in_R)
    return;

  X.reserve(nalloc);
  Y.reserve(nalloc);
  Z.reserve(nalloc);
  if (sel.gpstime)         T.reserve(nalloc);
  if (sel.intensity)       I.reserve(nalloc);
  if (sel.nir)             NIR.reserve(nalloc);
  if (sel.scanner_channel) Channel.reserve(nalloc);

  if (sel.rgb)
  {
    R.reserve(nalloc);
    G.reserve(nalloc);
    B.reserve(nalloc);
  }

  if (sel.waveform)
  {
    wavePacketIndex.reserve(nalloc);
    wavePacketOffset.reserve(nalloc);
    wavePacketSize.reserve(nalloc);
    wavePacketLocation.reserve(nalloc);
    Xt.reserve(nalloc);
    Yt.reserve(nalloc);
    Zt.reserve(nalloc);
  }

  for (int index : sel.extra_bytes)
  {
    const LASattribute& attribute = header->attributes[index];

    RLASExtrabyteAttributes extrabyte;
    extrabyte.id        = index;
    extrabyte.start     = header->get_attribute_start(index);
    extrabyte.name      = std::string(attribute.name);
    extrabyte.data_type = attribute.data_type;

    if (attribute.has_no_data())
    {
      // Scale and offset are not yet known at this stage so the no data value is
      // interpreted as an integer for any integer data type.
      extrabyte.has_no_data = true;
      if (extrabyte.data_type <= 6)
        extrabyte.no_data = attribute.no_data[0].i64;
      else
        extrabyte.no_data = attribute.no_data[0].f64;
    }

    if (attribute.has_offset())
    {
      extrabyte.has_offset = true;
      extrabyte.offset = attribute.offset[0];
    }

    if (attribute.has_scale())
    {
      extrabyte.has_scale = true;
      extrabyte.scale = attribute.scale[0];
    }

    if (!extrabyte.is_supported())
    {
      std::string msg = "data type " + std::to_string(extrabyte.data_type) + " of extra bytes attribute " + std::to_string(extrabyte.id) + " is deprecated and not supported by rlas.";
      Rf_warningcall(R_NilValue, "%s", msg.c_str());
      continue;
    }

    extrabyte.reserve(nalloc);
    extra_bytes_attr.push_back(std::move(extrabyte));
  }
}

bool RLASstreamer::read_point()
{
  progress = (double)lasreader->p_count/(double)header->number_of_point_records*100;
  return lasreader->read_point();
}

void RLASstreamer::init_lazy_columns(const LASpoint& point)
{
  SDF.init(point.get_scan_direction_flag());
  EoF.init(point.get_edge_of_flight_line());

  if (!extended)
  {
    RN.init(point.get_return_number());
    NoR.init(point.get_number_of_returns());
    C.init(point.get_classification());
    SAR.init(point.get_scan_angle());
  }
  else
  {
    RN.init(point.get_extended_return_number());
    NoR.init(point.get_extended_number_of_returns());
    C.init(point.get_extended_classification());
    SA.init(point.get_scan_angle());
  }

  Synthetic.init(point.get_synthetic_flag());
  Keypoint.init(point.get_keypoint_flag());
  Withheld.init(point.get_withheld_flag());
  Overlap.init(point.get_extended_overlap_flag());
  UD.init(point.get_user_data());
  PSI.init(point.get_point_source_ID());
}

void RLASstreamer::read_waveform()
{
  LASpoint& point = lasreader->point;

  if (!laswaveform13reader || !laswaveform13reader->read_waveform(&point))
  {
    wavePacketIndex.push_back(0);
    wavePacketOffset.push_back(0);
    wavePacketSize.push_back(0);
    wavePacketLocation.push_back(0);
    Xt.push_back(0);
    Yt.push_back(0);
    Zt.push_back(0);
    fullwaveform.push_back({0});
    return;
  }

  const LASwavepacket& wavepacket = point.wavepacket;
  wavePacketIndex.push_back(wavepacket.getIndex());
  wavePacketOffset.push_back((U32)wavepacket.getOffset());
  wavePacketSize.push_back(wavepacket.getSize());
  wavePacketLocation.push_back(wavepacket.getLocation());
  Xt.push_back(wavepacket.getXt());
  Yt.push_back(wavepacket.getYt());
  Zt.push_back(wavepacket.getZt());

  // If the wave packet offset was already seen, several points share the same waveform
  // (e.g. multiple returns of a pulse). There is no need to copy the same samples twice.
  if (!wavePacketRegistry.insert(wavepacket.getOffset()).second)
  {
    fullwaveform.push_back({0});
    return;
  }

  const unsigned int nsamples = laswaveform13reader->nsamples;
  const U8* samples = laswaveform13reader->samples;

  switch (laswaveform13reader->nbits)
  {
    case 8:
      fullwaveform.emplace_back(samples, samples + nsamples);
      break;
    case 16:
    {
      const U16* samples16 = reinterpret_cast<const U16*>(samples);
      fullwaveform.emplace_back(samples16, samples16 + nsamples);
      break;
    }
    case 32:
      throw std::runtime_error("32 bits full waveform not supported yet.");
  }
}

void RLASstreamer::write_point()
{
  const LASpoint& point = lasreader->point;

  if (!in_R)
  {
    laswriter->write_point(&point);
    laswriter->update_inventory(&point);
    return;
  }

  // The attributes that are often constant are initialized with the first point.
  // They are expanded to one value per point only if a different value is found.
  if (X.empty())
    init_lazy_columns(point);

  X.push_back(point.get_x());
  Y.push_back(point.get_y());
  Z.push_back(point.get_z());

  const size_t n = X.size();
  const size_t capacity = X.capacity();

  if (sel.gpstime)   T.push_back(point.get_gps_time());
  if (sel.intensity) I.push_back(point.get_intensity());

  if (sel.return_number)
    RN.push(extended ? point.get_extended_return_number() : point.get_return_number(), n, capacity);

  if (sel.number_of_returns)
    NoR.push(extended ? point.get_extended_number_of_returns() : point.get_number_of_returns(), n, capacity);

  if (sel.scan_direction)     SDF.push(point.get_scan_direction_flag(), n, capacity);
  if (sel.edge_of_flightline) EoF.push(point.get_edge_of_flight_line(), n, capacity);

  if (sel.classification)
    C.push(extended ? point.get_extended_classification() : point.get_classification(), n, capacity);

  if (sel.scanner_channel && extended)
    Channel.push_back(point.get_extended_scanner_channel());

  if (sel.synthetic) Synthetic.push(point.get_synthetic_flag(), n, capacity);
  if (sel.keypoint)  Keypoint.push(point.get_keypoint_flag(), n, capacity);
  if (sel.withheld)  Withheld.push(point.get_withheld_flag(), n, capacity);
  if (sel.overlap && extended) Overlap.push(point.get_extended_overlap_flag(), n, capacity);

  if (sel.scan_angle)
  {
    if (extended)
      SA.push(point.get_scan_angle(), n, capacity);
    else
      SAR.push(point.get_scan_angle_rank(), n, capacity);
  }

  if (sel.user_data)       UD.push(point.get_user_data(), n, capacity);
  if (sel.point_source_id) PSI.push(point.get_point_source_ID(), n, capacity);

  if (sel.rgb)
  {
    R.push_back(point.get_R());
    G.push_back(point.get_G());
    B.push_back(point.get_B());
  }

  if (sel.nir) NIR.push_back(point.get_NIR());

  if (sel.waveform) read_waveform();

  for (auto& extra_byte : extra_bytes_attr)
    extra_byte.push_back(&point);

  if (point.get_synthetic_flag()) nsynthetic++;
  if (point.get_withheld_flag())  nwithheld++;
}

List RLASstreamer::terminate()
{
  ended = true;

  if (!in_R)
  {
    laswriter->update_header(header, true);
    laswriter.reset();
    laswaveform13reader.reset();
    lasreader.reset();
    return List(0);
  }

  laswaveform13reader.reset();
  lasreader.reset();

  List lasdata = build_R_list();

  if (nwithheld > 0)
  {
    std::string msg = "There are " + std::to_string(nwithheld) + " points flagged 'withheld'.";
    Rf_warningcall(R_NilValue, "%s", msg.c_str());
  }

  if (nsynthetic > 0)
  {
    std::string msg = "There are " + std::to_string(nsynthetic) + " points flagged 'synthetic'.";
    Rf_warningcall(R_NilValue, "%s", msg.c_str());
  }

  return lasdata;
}

// Moves the C++ vectors into an R named list. Each C++ vector is released as soon
// as it has been copied to limit the peak memory usage.
List RLASstreamer::build_R_list()
{
  List lasdata;
  CharacterVector names;

  auto add = [&](const char* name, auto& vec)
  {
    lasdata.push_back(wrap(vec));
    names.push_back(name);
    std::remove_reference_t<decltype(vec)>().swap(vec);
  };

  add("X", X);
  add("Y", Y);
  add("Z", Z);
  if (sel.gpstime)            add("gpstime", T);
  if (sel.intensity)          add("Intensity", I);
  if (sel.return_number)      add("ReturnNumber", RN.values);
  if (sel.number_of_returns)  add("NumberOfReturns", NoR.values);
  if (sel.scan_direction)     add("ScanDirectionFlag", SDF.values);
  if (sel.edge_of_flightline) add("EdgeOfFlightline", EoF.values);
  if (sel.classification)     add("Classification", C.values);
  if (sel.scanner_channel)    add("ScannerChannel", Channel);
  if (sel.synthetic)          add("Synthetic_flag", Synthetic.values);
  if (sel.keypoint)           add("Keypoint_flag", Keypoint.values);
  if (sel.withheld)           add("Withheld_flag", Withheld.values);
  if (sel.overlap)            add("Overlap_flag", Overlap.values);

  if (sel.scan_angle)
  {
    if (extended)
      add("ScanAngle", SA.values);
    else
      add("ScanAngleRank", SAR.values);
  }

  if (sel.user_data)       add("UserData", UD.values);
  if (sel.point_source_id) add("PointSourceID", PSI.values);

  if (sel.rgb)
  {
    add("R", R);
    add("G", G);
    add("B", B);
  }

  if (sel.nir) add("NIR", NIR);

  if (sel.waveform)
  {
    add("WDPIndex", wavePacketIndex);
    add("WDPOffset", wavePacketOffset);
    add("WDPSize", wavePacketSize);
    add("WDPLocation", wavePacketLocation);
    add("Xt", Xt);
    add("Yt", Yt);
    add("Zt", Zt);
    add("FWF", fullwaveform);
  }

  for (auto& extra_byte : extra_bytes_attr)
  {
    lasdata.push_back(extra_byte.to_R());
    names.push_back(extra_byte.name);
  }

  lasdata.names() = names;
  return lasdata;
}
