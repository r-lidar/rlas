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

    if (useFilter)
      nalloc = ceil((float)npoints/8);
    else
      nalloc = npoints;

    // Allocate the required amount of data for mandatory variables
    X.reserve(nalloc);
    Y.reserve(nalloc);
    Z.reserve(nalloc);
  }

  initialized = true;

  return;
}

void RLASstreamer::allocation()
{
  // Allocate the required amount of data for activated options
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

  // Find if extra bytes are 32 of 64 bytes types
  for(int j = 0; j < eb.size(); j++)
  {
    int type = header->attributes[eb[j]].data_type;
    bool has_scale = header->attributes[eb[j]].has_scale();
    bool has_offset = header->attributes[eb[j]].has_offset();

    if (type <= 6 && !(has_scale || has_offset))    // unsigned char | char | unsigned shor | short | unsigned long | long
      eb32.push_back(eb[j]);
    else if (type <= 10)      // unsigned long long | long long | unsigned double | double
      eb64.push_back(eb[j]);
    else
      Rprintf("WARNING: data type %d of attribute %d not implemented.\n", type, j);
  }

  ExtraBytes32.resize(eb32.size());
  ExtraBytes64.resize(eb64.size());

  for(int j = 0; j < eb32.size(); j++)
    ExtraBytes32[j].reserve(nalloc);

  for(int j = 0; j < eb64.size(); j++)
    ExtraBytes64[j].reserve(nalloc);
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

    for(int j = 0; j < eb32.size(); j++)
    {
      ExtraBytes32[j].push_back(get_attribute_int(&lasreader->point, eb32[j]));
    }

    for(int j = 0; j < eb64.size(); j++)
    {
      ExtraBytes64[j].push_back(get_attribute_double(&lasreader->point, eb64[j]));
    }
  }
}

F64 RLASstreamer::get_attribute_double(LASpoint* point, I32 index)
{
  F64 casted_value;
  U8* value = point->extra_bytes + header->get_attribute_start(index);

  if (index < header->number_attributes)
  {
    switch (header->attributes[index].data_type)
    {
    case 1:
      casted_value = (F64)*((U8*)value);
      break;
    case 2:
      casted_value = (F64)*((I8*)value);
      break;
    case 3:
      casted_value = (F64)*((U16*)value);
      break;
    case 4:
      casted_value = (F64)*((I16*)value);
      break;
    case 5:
      casted_value = (F64)*((U32*)value);
      break;
    case 6:
      casted_value = (F64)*((I32*)value);
      break;
    case 7:
      casted_value = (F64)(I64)*((U64*)value);
      break;
    case 8:
      casted_value = (F64)*((I64*)value);
      break;
    case 9:
      casted_value = (F64)*((F32*)value);
      break;
    case 10:
      casted_value = *((F64*)value);
      break;
    default:
      throw std::runtime_error("LAS Extra Byte data type not supported.");
    }
    if(header->attributes[index].has_scale() || header->attributes[index].has_offset())
      return header->attributes[index].scale[0]*casted_value + header->attributes[index].offset[0];
    else
      return casted_value;
  }
  return 0.0;
}

I32 RLASstreamer::get_attribute_int(LASpoint* point, I32 index)
{
  I32 casted_value;
  U8* value = point->extra_bytes + + header->get_attribute_start(index);

  if (index < header->number_attributes)
  {
    switch (header->attributes[index].data_type)
    {
    case 1:
      casted_value = (I32)*((U8*)value);
      break;
    case 2:
      casted_value = (I32)*((I8*)value);
      break;
    case 3:
      casted_value = (I32)*((U16*)value);
      break;
    case 4:
      casted_value = (I32)*((I16*)value);
      break;
    case 5:
      casted_value = (I32)*((U32*)value);
      break;
    case 6:
      casted_value = (I32)*((I32*)value);
      break;
    default:
      throw std::runtime_error("LAS Extra Byte data type not supported in I32.");
    }
  }
  return 0;
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
    for(int j = 0; j < header->number_attributes; j++)
    {
      attribute_names.push_back(header->attributes[j].name);
    }

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

    for(int j = 0; j < eb32.size(); j++)
    {
      lasdata.push_back(ExtraBytes32[j]);
      field.push_back(attribute_names[eb32[j]]);
    }

    for(int j = 0; j < eb64.size(); j++)
    {
      lasdata.push_back(ExtraBytes64[j]);
      field.push_back(attribute_names[eb64[j]]);
    }

    lasdata.names() = field;

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