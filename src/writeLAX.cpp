/*
 ===============================================================================

 PROGRAMMERS:

 jean-romain.roussel.1@ulaval.ca  -  https://github.com/Jean-Romain/rlas

 COPYRIGHT:

 Copyright 2017 Jean-Romain Roussel
 Copyright 2011-17, martin isenburg, rapidlasso - fast tools to catch reality

 This file is part of rlas R package. It is mainly a subset of the code
 from lasindex a tool writen by martin isenburg in lastools integrated in
 R.

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

#include <stdexcept>
#include <string>

#include "lasreader.hpp"
#include "laszip_decompress_selective_v3.hpp"
#include "lasindex.hpp"
#include "lasquadtree.hpp"
#include "rlas_laslib.h"

using namespace Rcpp;

// [[Rcpp::export]]
void laxwriter(CharacterVector file, bool verbose)
{
  try
  {
    LASreadOpener lasreadopener;
    lasreadopener.set_file_name(as<std::string>(file).c_str());

    rlas::LASreaderPtr lasreader(lasreadopener.open());

    if (!lasreader)
      throw std::runtime_error("LASlib internal error. See message above.");

    lasreadopener.set_decompress_selective(LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY);

    // Setup the quadtree: the cell size grows with the extent of the file
    // (10 for extents < 1000, 100 for extents < 10000, ..., up to 100000)
    const LASheader& header = lasreader->header;
    float w = header.max_x - header.min_x;
    float h = header.max_y - header.min_y;

    F32 t = 10.0;
    while (t < 100000.0 && !(w < 100*t && h < 100*t))
      t *= 10;

    LASquadtree* lasquadtree = new LASquadtree; // owned by lasindex
    lasquadtree->setup(header.min_x, header.max_x, header.min_y, header.max_y, t);

    LASindex lasindex;
    lasindex.prepare(lasquadtree, 1000);

    while (lasreader->read_point())
      lasindex.add(lasreader->point.get_x(), lasreader->point.get_y(), (U32)(lasreader->p_cnt-1));

    lasreader.reset();

    // LASlib has no verbose argument any more: verbosity is the message log level, restored on exit.
    struct LogLevelGuard
    {
      LAS_MESSAGE_TYPE level = get_message_log_level();
      ~LogLevelGuard() { set_message_log_level(level); }
    } log_level_guard;
    if (verbose) set_message_log_level(LAS_VERBOSE);
    lasindex.complete(100000, -20);
    lasindex.write(lasreadopener.get_file_name());
  }
  catch (std::exception const& e)
  {
    Rcerr << "Error: " << e.what() << std::endl;
  }
}
