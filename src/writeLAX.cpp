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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lasreader.hpp"
#include "laszip_decompress_selective_v3.hpp"
#include "lasindex.hpp"
#include "lasquadtree.hpp"

using namespace Rcpp;

// [[Rcpp::export]]
void laxwriter(CharacterVector file)
{
  try
  {
    // Cast CharacterVector into proper types
    std::string filestd   = as<std::string>(file);
    const char* filechar = filestd.c_str();;

    // Initialize las objects
    LASreadOpener lasreadopener;
    lasreadopener.set_file_name(filechar);
    LASreader* lasreader = lasreadopener.open();

    if(0 == lasreader || NULL == lasreader)
      throw std::runtime_error("LASlib internal error. See message above.");

    lasreadopener.set_decompress_selective(LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY);

    // setup the quadtree

    LASquadtree* lasquadtree = new LASquadtree;

    float w = lasreader->header.max_x - lasreader->header.min_x;
    float h = lasreader->header.max_y - lasreader->header.min_y;
    F32 t;

    if ((w < 1000) && (h < 1000))
      t = 10.0;
    else if ((w < 10000) && (h < 10000))
      t = 100.0;
    else if ((w < 100000) && (h < 100000))
      t = 1000.0;
    else if ((w < 1000000) && (h < 1000000))
      t = 10000.0;
    else
      t = 100000.0;

    lasquadtree->setup(lasreader->header.min_x, lasreader->header.max_x, lasreader->header.min_y, lasreader->header.max_y, t);

    LASindex lasindex;
    lasindex.prepare(lasquadtree, 1000);

    while (lasreader->read_point())
      lasindex.add(lasreader->point.get_x(), lasreader->point.get_y(), (U32)(lasreader->p_count-1));

    lasreader->close();
    delete lasreader;

    lasindex.complete(100000, -20);
    lasindex.write(lasreadopener.get_file_name());

    return;
  }
  catch (std::exception const& e)
  {
    Rcerr << "Error: " << e.what() << std::endl;
    return;
  }
}