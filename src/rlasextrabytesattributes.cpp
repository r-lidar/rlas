#include "rlasextrabytesattributes.h"

RLASExtrabyteAttributes::RLASExtrabyteAttributes()
{
  id = 0;
  start = 0;
  data_type = 0;
  options = 0;
  has_scale = false;
  has_min = false;
  has_max = false;
  has_offset = false;
  has_no_data = false;
  scale = 1;
  offset = 0;
  no_data = 0;
  min = 0;
  max = 0;
}

bool RLASExtrabyteAttributes::is_supported(){ return(data_type <= 10); }
bool RLASExtrabyteAttributes::is_32bits() { return(data_type <= 6 && !(has_scale || has_offset)); }

void RLASExtrabyteAttributes::push_back(LASpoint* point)
{
  if (is_32bits())
    eb32.push_back(get_attribute_int(point));
  else
    eb64.push_back(get_attribute_double(point));
}

void RLASExtrabyteAttributes::parse_options()
{
  has_no_data = options & 0x01;
  has_min = options & 0x02;
  has_max = options & 0x04;
  has_scale = options & 0x08;
  has_offset = options & 0x10;
}

F64 RLASExtrabyteAttributes::get_attribute_double(LASpoint* point)
{
  F64 casted_value;
  U8* value = point->extra_bytes + start;

  switch (data_type)
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
    throw std::runtime_error("LAS Extra Byte data data_type not supported.");
  }

  if(has_scale || has_offset)
    casted_value = scale*casted_value + offset;

  if (has_no_data)
  {
    if (casted_value == no_data)
      casted_value = NA_REAL;
  }

  return casted_value;
}

I32 RLASExtrabyteAttributes::get_attribute_int(LASpoint* point)
{
  I32 casted_value;
  U8* value = point->extra_bytes + start;

  switch (data_type)
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
    throw std::runtime_error("LAS Extra Byte data data_type not supported in I32.");
  }

  if (has_no_data)
  {
    if (casted_value == no_data)
      casted_value = NA_INTEGER;
  }

  return casted_value;
}

void RLASExtrabyteAttributes::set_attribute(int i, LASpoint* p)
{
  double value;

  if (has_no_data && Rcpp::NumericVector::is_na(Reb[i]))
    value = no_data;
  else
    value = (Reb[i] - offset)/scale;

  switch(data_type)
  {
    case 0: p->set_attribute(start, U8_CLAMP(U8_QUANTIZE(value))); break;
    case 1: p->set_attribute(start, I8_CLAMP(I8_QUANTIZE(value))); break;
    case 2: p->set_attribute(start, U16_CLAMP(U16_QUANTIZE(value))); break;
    case 3: p->set_attribute(start, I16_CLAMP(I16_QUANTIZE(value))); break;
    case 4: p->set_attribute(start, U32_CLAMP(U32_QUANTIZE(value))); break;
    case 5: p->set_attribute(start, I32_CLAMP(I32_QUANTIZE(value))); break;
    case 6: p->set_attribute(start, U64_QUANTIZE(value)); break;
    case 7: p->set_attribute(start, I64_QUANTIZE(value)); break;
    case 8: p->set_attribute(start, (float)(value)); break;
    case 9: p->set_attribute(start, value); break;
  }
}


LASattribute RLASExtrabyteAttributes::make_LASattribute()
{
  LASattribute attribute(data_type, name.c_str(), desc.c_str(), 1);

  if(has_scale)
    attribute.set_scale(scale, 0);

  if(has_offset)
    attribute.set_offset(offset, 0);

  if(has_no_data)
  {
    switch(data_type)
    {
    case 0: attribute.set_no_data(U8_CLAMP(U8_QUANTIZE(no_data))); break;
    case 1: attribute.set_no_data(I8_CLAMP(I8_QUANTIZE(no_data))); break;
    case 2: attribute.set_no_data(U16_CLAMP(U16_QUANTIZE(no_data))); break;
    case 3: attribute.set_no_data(I16_CLAMP(I16_QUANTIZE(no_data))); break;
    case 4: attribute.set_no_data(U32_CLAMP(U32_QUANTIZE(no_data))); break;
    case 5: attribute.set_no_data(I32_CLAMP(I32_QUANTIZE(no_data))); break;
    case 6: attribute.set_no_data(U64_QUANTIZE(no_data)); break;
    case 7: attribute.set_no_data(I64_QUANTIZE(no_data)); break;
    case 8: attribute.set_no_data((float)(no_data)); break;
    case 9: attribute.set_no_data(no_data); break;
    }
  }

  if(has_min)
  {
    switch(data_type)
    {
    case 0: attribute.set_min(U8_CLAMP(U8_QUANTIZE(min))); break;
    case 1: attribute.set_min(I8_CLAMP(I8_QUANTIZE(min))); break;
    case 2: attribute.set_min(U16_CLAMP(U16_QUANTIZE(min))); break;
    case 3: attribute.set_min(I16_CLAMP(I16_QUANTIZE(min))); break;
    case 4: attribute.set_min(U32_CLAMP(U32_QUANTIZE(min))); break;
    case 5: attribute.set_min(I32_CLAMP(I32_QUANTIZE(min))); break;
    case 6: attribute.set_min(U64_QUANTIZE(min)); break;
    case 7: attribute.set_min(I64_QUANTIZE(min)); break;
    case 8: attribute.set_min((float)(min)); break;
    case 9: attribute.set_min(min); break;
    }
  }

  // set max value if option set
  if(has_max)
  {
    switch(data_type)
    {
    case 0: attribute.set_max(U8_CLAMP(U8_QUANTIZE(max))); break;
    case 1: attribute.set_max(I8_CLAMP(I8_QUANTIZE(max))); break;
    case 2: attribute.set_max(U16_CLAMP(U16_QUANTIZE(max))); break;
    case 3: attribute.set_max(I16_CLAMP(I16_QUANTIZE(max))); break;
    case 4: attribute.set_max(U32_CLAMP(U32_QUANTIZE(max))); break;
    case 5: attribute.set_max(I32_CLAMP(I32_QUANTIZE(max))); break;
    case 6: attribute.set_max(U64_QUANTIZE(max)); break;
    case 7: attribute.set_max(I64_QUANTIZE(max)); break;
    case 8: attribute.set_max((float)(max)); break;
    case 9: attribute.set_max(max); break;
    }
  }

  return attribute;
}