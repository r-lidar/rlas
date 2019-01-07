#include "rlasstreamer.h"

RLASstreamer::RLASstreamer()
{
  initialize_bool();
}

RLASstreamer::RLASstreamer(CharacterVector ifiles)
{
  initialize_bool();
  setinputfiles(ifiles);
  initialize();
}


RLASstreamer::RLASstreamer(CharacterVector ifiles, CharacterVector filter)
{
  initialize_bool();
  setinputfiles(ifiles);
  setfilter(filter);
  initialize();
}

RLASstreamer::RLASstreamer(CharacterVector ifiles, CharacterVector ofile, CharacterVector filter)
{
  initialize_bool();
  setinputfiles(ifiles);
  setfilter(filter);
  setoutputfile(ofile);
  initialize();
}

RLASstreamer::~RLASstreamer()
{
  if (!ended)
    terminate();

  if(0 != lasreader && NULL != lasreader)
    delete lasreader;

  if(0 != laswriter && NULL != laswriter)
    delete laswriter;
}

void RLASstreamer::setinputfiles(CharacterVector ifiles)
{
  if (ifiles.length() == 0)
    throw std::runtime_error("Input files vector is null");

  lasreadopener.set_merged(true);
  lasreadopener.set_populate_header(true);

  for (int j = 0; j < ifiles.length(); j++)
  {
    std::string filestd  = as<std::string>(ifiles[j]);
    const char* filechar = filestd.c_str();
    lasreadopener.add_file_name(filechar);
  }

  return;
}

void RLASstreamer::setoutputfile(CharacterVector ofile)
{
  if (ofile.length() == 0)
    throw std::runtime_error("Output file vector is empty.");

  if (ofile.length() > 1)
    throw std::runtime_error("Cannot write several files at one time.");

  std::string ofilestd  = as<std::string>(ofile);

  if (ofilestd == "")
    return;

  if (!useFilter)
    throw std::runtime_error("Writing an output file without filter is useless.");

  const char* ofilechar = ofilestd.c_str();

  laswriteopener.set_file_name(ofilechar);

  inR = false;

  return;
}

void RLASstreamer::setfilter(CharacterVector filter)
{
  if (filter.length() == 0)
    throw std::runtime_error("Filter vector is empty.");

  if (filter.length() > 1)
    throw std::runtime_error("Filter must have a length 1.");

  std::string filterstd  = as<std::string>(filter);

  if (filterstd == "")
    return;

  char* filterchar = const_cast<char*>(filterstd.c_str());

  if (!lasreadopener.parse_str(filterchar))
  {
    lasreadopener.set_filter(0);
    throw std::runtime_error("Filter error see message above.");
  }

  useFilter = true;

  return;
}

void RLASstreamer::select(CharacterVector string)
{
  std::string select = as<std::string>(string);
  std::string unselect = as<std::string>(string);

  t = false;
  i = false;
  r = false;
  n = false;
  d = false;
  e = false;
  c = false;
  s = false;
  k = false;
  w = false;
  a = false;
  u = false;
  p = false;
  rgb = false;
  nir = false;

  if (select.find("*") != std::string::npos)
    select = "xyztirndecskwaupRGBN0";

  if (select.find("i") != std::string::npos) read_i(true);
  if (select.find("t") != std::string::npos) read_t(true);
  if (select.find("r") != std::string::npos) read_r(true);
  if (select.find("n") != std::string::npos) read_n(true);
  if (select.find("d") != std::string::npos) read_d(true);
  if (select.find("e") != std::string::npos) read_e(true);
  if (select.find("c") != std::string::npos) read_c(true);
  if (select.find("s") != std::string::npos) read_s(true);
  if (select.find("k") != std::string::npos) read_k(true);
  if (select.find("w") != std::string::npos) read_w(true);
  if (select.find("a") != std::string::npos) read_a(true);
  if (select.find("u") != std::string::npos) read_u(true);
  if (select.find("p") != std::string::npos) read_p(true);
  if (select.find("R") != std::string::npos) read_rgb(true);
  if (select.find("G") != std::string::npos) read_rgb(true);
  if (select.find("B") != std::string::npos) read_rgb(true);
  if (select.find("N") != std::string::npos) read_nir(true);

  if (unselect.find("-i") != std::string::npos) read_i(false);
  if (unselect.find("-t") != std::string::npos) read_t(false);
  if (unselect.find("-r") != std::string::npos) read_r(false);
  if (unselect.find("-n") != std::string::npos) read_n(false);
  if (unselect.find("-d") != std::string::npos) read_d(false);
  if (unselect.find("-e") != std::string::npos) read_e(false);
  if (unselect.find("-c") != std::string::npos) read_c(false);
  if (unselect.find("-s") != std::string::npos) read_s(false);
  if (unselect.find("-k") != std::string::npos) read_k(false);
  if (unselect.find("-w") != std::string::npos) read_w(false);
  if (unselect.find("-a") != std::string::npos) read_a(false);
  if (unselect.find("-u") != std::string::npos) read_u(false);
  if (unselect.find("-p") != std::string::npos) read_p(false);
  if (unselect.find("-R") != std::string::npos) read_rgb(false);
  if (unselect.find("-G") != std::string::npos) read_rgb(false);
  if (unselect.find("-B") != std::string::npos) read_rgb(false);
  if (unselect.find("-N") != std::string::npos) read_nir(false);

  std::vector<bool> select_eb(9);
  std::fill(select_eb.begin(), select_eb.end(), false);

  if (select.find("0") != std::string::npos)
    std::fill(select_eb.begin(), select_eb.end(), true);
  else
  {
    if (select.find("1") != std::string::npos) select_eb[0] = true;
    if (select.find("2") != std::string::npos) select_eb[1] = true;
    if (select.find("3") != std::string::npos) select_eb[2] = true;
    if (select.find("4") != std::string::npos) select_eb[3] = true;
    if (select.find("5") != std::string::npos) select_eb[4] = true;
    if (select.find("6") != std::string::npos) select_eb[5] = true;
    if (select.find("7") != std::string::npos) select_eb[6] = true;
    if (select.find("8") != std::string::npos) select_eb[7] = true;
    if (select.find("9") != std::string::npos) select_eb[8] = true;
  }

  if (unselect.find("-0") != std::string::npos)
    std::fill(select_eb.begin(), select_eb.end(), false);
  else
  {
    if (unselect.find("-1") != std::string::npos) select_eb[0] = false;
    if (unselect.find("-2") != std::string::npos) select_eb[1] = false;
    if (unselect.find("-3") != std::string::npos) select_eb[2] = false;
    if (unselect.find("-4") != std::string::npos) select_eb[3] = false;
    if (unselect.find("-5") != std::string::npos) select_eb[4] = false;
    if (unselect.find("-6") != std::string::npos) select_eb[5] = false;
    if (unselect.find("-7") != std::string::npos) select_eb[6] = false;
    if (unselect.find("-8") != std::string::npos) select_eb[7] = false;
    if (unselect.find("-9") != std::string::npos) select_eb[8] = false;
  }

  IntegerVector pos_eb;
  for (size_t i = 0 ; i < select_eb.size() ; i++)
  {
    if(select_eb[i])
      pos_eb.push_back(i);
  }

  read_eb(pos_eb);
}

void RLASstreamer::initialize()
{
  // Intialize the reader
  lasreader = lasreadopener.open();
  header = &lasreader->header;

  if(0 == lasreader || NULL == lasreader)
    throw std::runtime_error("LASlib internal error. See message above.");

  // Initilize the writer if write in file
  if (!inR)
  {
    laswriter = laswriteopener.open(&lasreader->header);

    if(0 == laswriter || NULL == laswriter)
      throw std::runtime_error("LASlib internal error. See message above.");
  }
  else
  {
    // Read the header and test some properties of the file
    U8 point_type = lasreader->header.point_data_format;
    format = get_format(point_type);
    int npoints = lasreader->header.number_of_point_records;

    bool has_rgb = (format == 2 || format == 3 || format == 7 || format == 8);
    bool has_t  = (format == 1 || format == 3 || format == 6 || format == 7 || format == 8);

    t = t && has_t;
    rgb = rgb && has_rgb;

    if (useFilter)
      nalloc = ceil((float)npoints/8);
    else
      nalloc = npoints;
  }

  nsynthetic = 0;
  nwithheld = 0;
  initialized = true;

  return;
}

void RLASstreamer::allocation()
{
  // Allocate the required amount of data for activated options
  if(inR)
  {
    // Allocate the required amount of data for mandatory variables
    X.reserve(nalloc);
    Y.reserve(nalloc);
    Z.reserve(nalloc);
    if(t) T.reserve(nalloc);
    if(i) I.reserve(nalloc);
    if(r) RN.reserve(nalloc);
    if(n) NoR.reserve(nalloc);
    if(d) SDF.reserve(nalloc);
    if(e) EoF.reserve(nalloc);
    if(c) C.reserve(nalloc);
    if(s) Synthetic.reserve(nalloc);
    if(k) Keypoint.reserve(nalloc);
    if(w) Withheld.reserve(nalloc);
    if(a) SA.reserve(nalloc);
    if(u) UD.reserve(nalloc);
    if(p) PSI.reserve(nalloc);
    if(rgb)
    {
      R.reserve(nalloc);
      G.reserve(nalloc);
      B.reserve(nalloc);
    }
    if(nir) NIR.reserve(nalloc);

    // Find if extra bytes are 32 of 64 bytes types
    for(size_t j = 0; j < eb.size(); j++)
    {
      int index = eb[j];

      RLASExtrabyteAttributes extrabyte;
      extrabyte.id = index;
      extrabyte.start = header->get_attribute_start(index);
      extrabyte.name = std::string(header->attributes[index].name);
      extrabyte.data_type = header->attributes[index].data_type;

      if(header->attributes[index].has_no_data())
      {
        extrabyte.has_no_data = true;

        if (extrabyte.is_32bits())
        {
          I64* temp = ((I64*)header->attributes[index].no_data);
          extrabyte.no_data = *temp;
        }
        else
        {
          F64* temp = ((F64*)header->attributes[index].no_data);
          extrabyte.no_data = *temp;
        }
      }

      if (header->attributes[index].has_offset())
      {
        extrabyte.has_offset = true;
        extrabyte.offset = header->attributes[index].offset[0];
      }

      if (header->attributes[index].has_scale())
      {
        extrabyte.has_scale = true;
        extrabyte.scale = header->attributes[index].scale[0];
      }

      if(extrabyte.is_supported())
      {
        if (extrabyte.is_32bits())
          extrabyte.eb32.reserve(nalloc);
        else
          extrabyte.eb64.reserve(nalloc);

        extra_bytes_attr.push_back(extrabyte);
      }
      else
        Rprintf("WARNING: data type %d of attribute %d not implemented.\n", extrabyte.data_type, extrabyte.id);
    }
  }
}

bool RLASstreamer::read_point()
{
  return lasreader->read_point();
}

void RLASstreamer::write_point()
{
  if (!inR)
  {
    laswriter->write_point(&lasreader->point);
    laswriter->update_inventory(&lasreader->point);
  }
  else
  {
    X.push_back(lasreader->point.get_x());
    Y.push_back(lasreader->point.get_y());
    Z.push_back(lasreader->point.get_z());

    if(t) T.push_back(lasreader->point.get_gps_time());
    if(i) I.push_back(lasreader->point.get_intensity());
    if(r) RN.push_back(lasreader->point.get_return_number());
    if(n) NoR.push_back(lasreader->point.get_number_of_returns());
    if(d) SDF.push_back(lasreader->point.get_scan_direction_flag());
    if(e) EoF.push_back(lasreader->point.get_edge_of_flight_line());
    if(c) C.push_back(lasreader->point.get_classification());
    if(s) Synthetic.push_back(lasreader->point.get_synthetic_flag());
    if(k) Keypoint.push_back(lasreader->point.get_keypoint_flag());
    if(w) Withheld.push_back(lasreader->point.get_withheld_flag());
    if(a) SA.push_back(lasreader->point.get_scan_angle_rank());
    if(u) UD.push_back(lasreader->point.get_user_data());
    if(p) PSI.push_back(lasreader->point.get_point_source_ID());
    if(rgb)
    {
      R.push_back(lasreader->point.get_R());
      G.push_back(lasreader->point.get_G());
      B.push_back(lasreader->point.get_B());
    }
    if(nir) NIR.push_back(lasreader->point.get_NIR());

    for(auto& extra_byte : extra_bytes_attr)
      extra_byte.push_back(&lasreader->point);

    if (lasreader->point.get_synthetic_flag())
      nsynthetic++;

    if(lasreader->point.get_withheld_flag())
      nwithheld++;
  }
}


LASpoint* RLASstreamer::point()
{
  return &lasreader->point;
}

List RLASstreamer::terminate()
{
  if(!inR)
  {
    laswriter->update_header(header, true);

    laswriter->close();
    delete laswriter;

    lasreader->close();
    delete lasreader;

    lasreader = 0;
    laswriter = 0;

    ended = true;
    return List(0);
  }
  else
  {
    lasreader->close();
    delete lasreader;
    lasreader = 0;
    ended = true;

    List lasdata = List::create(X,Y,Z);
    X.clear();
    X.shrink_to_fit();
    Y.clear();
    Y.shrink_to_fit();
    Z.clear();
    Z.shrink_to_fit();

    CharacterVector field(0);
    field.push_back("X");
    field.push_back("Y");
    field.push_back("Z");

    if(t)
    {
      lasdata.push_back(T);
      field.push_back("gpstime");
      T.clear();
      T.shrink_to_fit();
    }

    if(i)
    {
      lasdata.push_back(I);
      field.push_back("Intensity");
      I.clear();
      I.shrink_to_fit();
    }

    if(r)
    {
      lasdata.push_back(RN);
      field.push_back("ReturnNumber");
      RN.clear();
      RN.shrink_to_fit();
    }

    if(n)
    {
      lasdata.push_back(NoR);
      field.push_back("NumberOfReturns");
      NoR.clear();
      NoR.shrink_to_fit();
    }

    if(d)
    {
      lasdata.push_back(SDF);
      field.push_back("ScanDirectionFlag");
      SDF.clear();
      SDF.shrink_to_fit();
    }

    if(e)
    {
      lasdata.push_back(EoF);
      field.push_back("EdgeOfFlightline");
      EoF.clear();
      EoF.shrink_to_fit();
    }

    if(c)
    {
      lasdata.push_back(C);
      field.push_back("Classification");
      C.clear();
      C.shrink_to_fit();
    }

    if(s)
    {
      lasdata.push_back(Synthetic);
      field.push_back("Synthetic_flag");
      Synthetic.clear();
      Synthetic.shrink_to_fit();
    }

    if(k)
    {
      lasdata.push_back(Keypoint);
      field.push_back("Keypoint_flag");
      Keypoint.clear();
      Keypoint.shrink_to_fit();
    }

    if(w)
    {
      lasdata.push_back(Withheld);
      field.push_back("Withheld_flag");
      Withheld.clear();
      Withheld.shrink_to_fit();
    }

    if(a)
    {
      lasdata.push_back(SA);
      field.push_back("ScanAngle");
      SA.clear();
      SA.shrink_to_fit();
    }

    if(u)
    {
      lasdata.push_back(UD);
      field.push_back("UserData");
      UD.clear();
      UD.shrink_to_fit();
    }

    if(p)
    {
      lasdata.push_back(PSI);
      field.push_back("PointSourceID");
      PSI.clear();
      PSI.shrink_to_fit();
    }

    if(rgb)
    {
      lasdata.push_back(R);
      field.push_back("R");
      R.clear();
      R.shrink_to_fit();

      lasdata.push_back(G);
      field.push_back("G");
      G.clear();
      G.shrink_to_fit();

      lasdata.push_back(B);
      field.push_back("B");
      B.clear();
      B.shrink_to_fit();
    }

    if(nir)
    {
      lasdata.push_back(NIR);
      field.push_back("NIR");
      NIR.clear();
      NIR.shrink_to_fit();
    }

    for(size_t j = 0; j < extra_bytes_attr.size(); j++)
    {
      if (extra_bytes_attr[j].is_32bits())
      {
        IntegerVector X = wrap(extra_bytes_attr[j].eb32);
        lasdata.push_back(X);
        field.push_back(extra_bytes_attr[j].name);
      }
      else
      {
        NumericVector X = wrap(extra_bytes_attr[j].eb64);
        lasdata.push_back(X);
        field.push_back(extra_bytes_attr[j].name);
      }
    }

    lasdata.names() = field;

    if (nwithheld > 0)
      Rcerr << "Warning: there are " << nwithheld << " points flagged 'withheld'." << std::endl;

    if (nsynthetic > 0)
      Rcerr << "Warning: there are " << nsynthetic << " points flagged 'synthetic'."  << std::endl;

    return(lasdata);
  }
}

void RLASstreamer::initialize_bool()
{
  t = true;
  i = true;
  r = true;
  n = true;
  d = true;
  e = true;
  c = true;
  s = true;
  k = true;
  w = true;
  a = true;
  u = true;
  p = true;
  rgb = true;
  nir = true;

  inR = true;
  useFilter = false;
  initialized = false;
  ended = false;
  lasreader = 0;
  laswriter = 0;
}

void RLASstreamer::read_t(bool b){ t = b && (format == 1 || format == 3 || format == 6 || format == 7 || format == 8); }
void RLASstreamer::read_i(bool b){ i = b; }
void RLASstreamer::read_r(bool b){ r = b; }
void RLASstreamer::read_n(bool b){ n = b; }
void RLASstreamer::read_d(bool b){ d = b; }
void RLASstreamer::read_e(bool b){ e = b; }
void RLASstreamer::read_c(bool b){ c = b; }
void RLASstreamer::read_s(bool b){ s = b; }
void RLASstreamer::read_k(bool b){ k = b; }
void RLASstreamer::read_w(bool b){ w = b; }
void RLASstreamer::read_a(bool b){ a = b; }
void RLASstreamer::read_u(bool b){ u = b; }
void RLASstreamer::read_p(bool b){ p = b; }
void RLASstreamer::read_rgb(bool b){ rgb = b && (format == 2 || format == 3 || format == 7 || format == 8); }
void RLASstreamer::read_nir(bool b){ nir = b && (format == 8); }
void RLASstreamer::read_eb(IntegerVector x)
{
  std::sort(x.begin(), x.end());
  x.erase( std::unique( x.begin(), x.end() ), x.end() );

  if(x.size() && x[0]==-1 ) // 0 means get all available extra_bytes
  {
    for(int j = 0; j < header->number_attributes; j++)
    {
      eb.push_back(j);
    }
  }
  else // filters attribute numbers not existing
  {
    for(int j = 0; j < x.size(); j++)
    {
      if(x[j] < header->number_attributes)
      {
        eb.push_back(x[j]);
      }
    }
  }
}

int RLASstreamer::get_format(U8 point_type)
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
    throw std::runtime_error("Point data record type 4 not yet supported");
    break;
  case 5:
    throw std::runtime_error("Point data record type 5 not yet supported");
    break;
  case 6:
    format = 6;
    break;
  case 7:
    format = 7;
    break;
  case 8:
    format = 8;
    break;
  case 9:
    throw std::runtime_error("Point data record type 9 not yet supported");
    break;
  case 10:
    throw std::runtime_error("Point data record type 10 not yet supported");
    break;
  default:
    throw std::runtime_error("LAS format not valid");
  }

  return(format);
}
