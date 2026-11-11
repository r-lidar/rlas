#ifndef RLAS_LASLIB_H
#define RLAS_LASLIB_H

// Small helpers shared by the rlas sources to work with LASlib objects:
// RAII ownership of readers/writers and the capabilities of each point data format.

#include <memory>

#include "lasreader.hpp"
#include "laswriter.hpp"
#include "laswaveform13reader.hpp"

namespace rlas
{

// LASlib objects are allocated by their openers and must be closed before being deleted.
struct LASreaderCloser          { void operator()(LASreader* p) const          { p->close(); delete p; } };
struct LASwriterCloser          { void operator()(LASwriter* p) const          { p->close(); delete p; } };
struct LASwaveform13readerCloser{ void operator()(LASwaveform13reader* p) const{ p->close(); delete p; } };

using LASreaderPtr           = std::unique_ptr<LASreader, LASreaderCloser>;
using LASwriterPtr           = std::unique_ptr<LASwriter, LASwriterCloser>;
using LASwaveform13readerPtr = std::unique_ptr<LASwaveform13reader, LASwaveform13readerCloser>;

// Attributes available in each point data format (LAS specification 1.4 R15, table 7 to 17)
namespace point_format
{
  constexpr bool has_gpstime(int f)  { return f == 1 || f >= 3; }
  constexpr bool has_rgb(int f)      { return f == 2 || f == 3 || f == 5 || f == 7 || f == 8 || f == 10; }
  constexpr bool has_nir(int f)      { return f == 8 || f == 10; }
  constexpr bool has_waveform(int f) { return f == 4 || f == 5 || f == 9 || f == 10; }
  constexpr bool is_valid(int f)     { return f >= 0 && f <= 10; }

  // Size in bytes of the core point record (without extra bytes)
  constexpr int record_length(int f)
  {
    constexpr int length[] = {20, 28, 26, 34, 57, 63, 30, 36, 38, 59, 67};
    return is_valid(f) ? length[f] : -1;
  }
}

}

#endif // RLAS_LASLIB_H
