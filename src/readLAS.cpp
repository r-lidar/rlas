/*
===============================================================================

PROGRAMMERS:

jean-romain.roussel.1@ulaval.ca  -  https://github.com/Jean-Romain/rlas

COPYRIGHT:

Copyright 2016 Jean-Romain Roussel

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

#include <Rcpp.h>
#include <chrono>
#include <string>
#include "rlasstreamer.h"
#include "laspoint.hpp"
#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lasfilter.hpp"

bool pnpoly(NumericMatrix polygon, double x, double y)
{
  int nvert = polygon.nrow();
  bool c = false;
  int i, j;
  for (i = 0, j = nvert - 1; i < nvert; j = i++)
  {
    if (((polygon(i, 1) > y) != (polygon(j, 1) > y)) &&
        (x < (polygon(j, 0) - polygon(i, 0)) * (y - polygon(i, 1)) / (polygon(j, 1) - polygon(i, 1)) + polygon(i, 0))) {
      c = !c;
    }
  }
  return c;
}

// Progress printing function
inline void print_progress(float progress, const std::chrono::steady_clock::time_point& start)
{
  int width = 50;
  int pos = static_cast<int>(progress / 100.0 * width);

  auto now = std::chrono::steady_clock::now();
  double elapsed = std::chrono::duration<double>(now - start).count();
  if (elapsed < 2.0) return;

  double eta = (progress > 0.0) ? elapsed * (100.0 - progress) / progress : 0.0;

  std::string bar = "[";
  for (int i = 0; i < width; ++i)
    bar += (i < pos) ? "=" : (i == pos ? ">" : " ");
  bar += "]";

  Rcpp::Rcout << "\r" << bar << " "
              << static_cast<int>(progress) << "% "
              << "ETA: " << static_cast<int>(eta) << "s     " << std::flush;
}

// [[Rcpp::export]]
List C_reader(CharacterVector ifiles, CharacterVector ofile, CharacterVector select, CharacterVector filter, Rcpp::List polygons)
{
  RLASstreamer streamer(ifiles, ofile, filter);
  streamer.select(select);
  streamer.allocation();

  auto start = std::chrono::steady_clock::now();
  int counter = 0;

  if (polygons.size() == 0)
  {
    while(streamer.read_point())
    {
      streamer.write_point();

      if (++counter % 10000 == 0)
      {
        Rcpp::checkUserInterrupt();
        print_progress(streamer.progress, start);
      }
    }
  }
  else
  {
    while(streamer.read_point())
    {
      double x = streamer.point()->get_x();
      double y = streamer.point()->get_y();

      bool inpoly = false;
      for (unsigned int i = 0 ; i < polygons.size() ; i++)
      {
        // This list can be made of several rings (MULTIPOLYGON) and interior rings
        Rcpp::List rings = polygons[i];

        // Loop through sub polygons (ring)
        for (int j = 0 ; j < rings.size() ; j++)
        {
          Rcpp::NumericMatrix ring = rings[j];

          // We need to know if the ring is an exterior/interior ring (hole)
          bool exterior_ring = ring(0,2) == 1;

          bool b = pnpoly(ring, x, y);

          if (b)
          {
            if (exterior_ring)
            {
              inpoly = true;
            }
            else
            {
              inpoly = false;
              break;
            }
          }
        }

        if (inpoly)
        {
          streamer.write_point();
        }
      }

      if (++counter % 10000 == 0)
      {
        Rcpp::checkUserInterrupt();
        print_progress(streamer.progress, start);
      }
    }
  }

  Rcpp::Rcout << "\r" << std::string(80, ' ') << "\r" << std::flush;

  return streamer.terminate();
}