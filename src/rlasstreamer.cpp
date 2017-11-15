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

  lasreadopener.parse_str(filterchar);

  useFilter = true;

  return;
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

    bool has_rgb = format == 2 || format == 3;
    bool has_t  = format == 1 || format == 3;

    t = t && has_t;
    rgb = rgb && has_rgb;
    read_at(&at0, 0);
    read_at(&at1, 1);
    read_at(&at2, 2);
    read_at(&at3, 3);
    read_at(&at4, 4);
    read_at(&at5, 5);
    read_at(&at6, 6);
    read_at(&at7, 7);
    read_at(&at8, 8);
    read_at(&at9, 9);

    int nalloc;

    if (useFilter)
      nalloc = ceil((float)npoints/8);
    else
      nalloc = npoints;

    // Allocate the required amount of data only if necessary
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
    if(a) SA.reserve(nalloc);
    if(u) UD.reserve(nalloc);
    if(p) PSI.reserve(nalloc);
    if(rgb)
    {
      R.reserve(nalloc);
      G.reserve(nalloc);
      B.reserve(nalloc);
    }
    if(at0) At0.reserve(nalloc);
    if(at1) At1.reserve(nalloc);
    if(at2) At2.reserve(nalloc);
    if(at3) At3.reserve(nalloc);
    if(at4) At4.reserve(nalloc);
    if(at5) At5.reserve(nalloc);
    if(at6) At6.reserve(nalloc);
    if(at7) At7.reserve(nalloc);
    if(at8) At8.reserve(nalloc);
    if(at9) At9.reserve(nalloc);
  }

  initialized = true;

  return;
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
    if(a) SA.push_back(lasreader->point.get_scan_angle_rank());
    if(u) UD.push_back(lasreader->point.get_user_data());
    if(p) PSI.push_back(lasreader->point.get_point_source_ID());
    if(rgb)
    {
      R.push_back(lasreader->point.get_R());
      G.push_back(lasreader->point.get_G());
      B.push_back(lasreader->point.get_B());
    }
    if(at0) At0.push_back(lasreader->point.get_attribute_as_float(0));
    if(at1) At1.push_back(lasreader->point.get_attribute_as_float(1));
    if(at2) At2.push_back(lasreader->point.get_attribute_as_float(1));
    if(at3) At3.push_back(lasreader->point.get_attribute_as_float(1));
    if(at4) At4.push_back(lasreader->point.get_attribute_as_float(1));
    if(at5) At5.push_back(lasreader->point.get_attribute_as_float(1));
    if(at6) At6.push_back(lasreader->point.get_attribute_as_float(1));
    if(at7) At7.push_back(lasreader->point.get_attribute_as_float(1));
    if(at8) At8.push_back(lasreader->point.get_attribute_as_float(1));
    if(at9) At9.push_back(lasreader->point.get_attribute_as_float(1));
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
    CharacterVector attribute_names;
    for(int i = 0; i < header->number_attributes; i++)
    {
      attribute_names.push_back(header->attributes[i].name);
    }

    lasreader->close();
    delete lasreader;
    lasreader = 0;

    List lasdata = List::create(X,Y,Z);

    CharacterVector field(0);
    field.push_back("X");
    field.push_back("Y");
    field.push_back("Z");

    if(t)
    {
      lasdata.push_back(T);
      field.push_back("gpstime");
    }

    if(i)
    {
      lasdata.push_back(I);
      field.push_back("Intensity");
    }

    if(r)
    {
      lasdata.push_back(RN);
      field.push_back("ReturnNumber");
    }

    if(n)
    {
      lasdata.push_back(NoR);
      field.push_back("NumberOfReturns");
    }

    if(d)
    {
      lasdata.push_back(SDF);
      field.push_back("ScanDirectionFlag");
    }

    if(e)
    {
      lasdata.push_back(EoF);
      field.push_back("EdgeOfFlightline");
    }

    if(c)
    {
      lasdata.push_back(C);
      field.push_back("Classification");
    }

    if(a)
    {
      lasdata.push_back(SA);
      field.push_back("ScanAngle");
    }

    if(u)
    {
      lasdata.push_back(UD);
      field.push_back("UserData");
    }

    if(p)
    {
      lasdata.push_back(PSI);
      field.push_back("PointSourceID");
    }

    if(rgb)
    {
      lasdata.push_back(R);
      field.push_back("R");

      lasdata.push_back(G);
      field.push_back("G");

      lasdata.push_back(B);
      field.push_back("B");
    }

    if(at0)
    {
      lasdata.push_back(At0);
      field.push_back(attribute_names[0]);
    }

    if(at1)
    {
      lasdata.push_back(At1);
      field.push_back(attribute_names[1]);
    }

    if(at2)
    {
      lasdata.push_back(At2);
      field.push_back(attribute_names[2]);
    }

    if(at3)
    {
      lasdata.push_back(At3);
      field.push_back(attribute_names[3]);
    }

    if(at4)
    {
      lasdata.push_back(At4);
      field.push_back(attribute_names[4]);
    }

    if(at5)
    {
      lasdata.push_back(At5);
      field.push_back(attribute_names[5]);
    }

    if(at6)
    {
      lasdata.push_back(At6);
      field.push_back(attribute_names[6]);
    }

    if(at7)
    {
      lasdata.push_back(At7);
      field.push_back(attribute_names[7]);
    }

    if(at8)
    {
      lasdata.push_back(At8);
      field.push_back(attribute_names[8]);
    }

    if(at9)
    {
      lasdata.push_back(At9);
      field.push_back(attribute_names[9]);
    }

    lasdata.names() = field;

    ended = true;

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
  a = true;
  u = true;
  p = true;
  rgb = true;
  at1 = true; // attribute 1
  inR = true;
  useFilter = false;
  initialized = false;
  ended = false;
  lasreader = 0;
  laswriter = 0;
}

void RLASstreamer::read_t(bool b)
{
  t = b && (format == 1 || format == 3);
}

void RLASstreamer::read_i(bool b)
{
  i = b;
}

void RLASstreamer::read_r(bool b)
{
  r = b;
}

void RLASstreamer::read_n(bool b)
{
  n = b;
}

void RLASstreamer::read_d(bool b)
{
  d= b;
}

void RLASstreamer::read_e(bool b)
{
  e = b;
}

void RLASstreamer::read_c(bool b)
{
  c = b;
}

void RLASstreamer::read_a(bool b)
{
  a = b;
}

void RLASstreamer::read_u(bool b)
{
  u = b;
}

void RLASstreamer::read_p(bool b)
{
  p = b;
}

void RLASstreamer::read_rgb(bool b)
{
  rgb = b && (format == 2 || format == 3);
}

void RLASstreamer::read_at(bool* b, int index)
{
  *b = *b && (index < header->number_attributes);
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
    throw std::runtime_error("LAS format not yet supported");
    break;
  case 5:
    throw std::runtime_error("LAS format not yet supported");
    break;
  case 6:
    throw std::runtime_error("LAS format not yet supported");
    break;
  case 7:
    throw std::runtime_error("LAS format not yet supported");
    break;
  case 8:
    throw std::runtime_error("LAS format not yet supported");
    break;
  case 9:
    throw std::runtime_error("LAS format not yet supported");
    break;
  case 10:
    throw std::runtime_error("LAS format not yet supported");
    break;
  default:
    throw std::runtime_error("LAS format not valid");
  }

  return(format);
}