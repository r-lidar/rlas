/*
 ===============================================================================

 PROGRAMMERS:

 info@r-lidar.com  -  https://github.com/r-lidar/rlas

 COPYRIGHT:

 Copyright 2026 Jean-Romain Roussel

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
// LASlib reports errors by throwing (see byebye() in mydefs.cpp). A deleter runs in a destructor,
// possibly during stack unwinding, so it must never let an exception escape.
struct LASreaderCloser          { void operator()(LASreader* p) const          { try { p->close(); } catch (...) {} delete p; } };
struct LASwriterCloser          { void operator()(LASwriter* p) const          { try { p->close(); } catch (...) {} delete p; } };
struct LASwaveform13readerCloser{ void operator()(LASwaveform13reader* p) const{ try { p->close(); } catch (...) {} delete p; } };

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
