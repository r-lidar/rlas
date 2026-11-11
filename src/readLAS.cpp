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
#include <vector>

#include "rlasstreamer.h"

using namespace Rcpp;

namespace
{

// A ring of a polygon. The matrix has 3 columns: x, y and 1 for an exterior ring or 0 for a hole
struct Ring
{
  NumericMatrix coords;
  bool exterior;
};

using Polygon = std::vector<Ring>;

// Point in polygon test (W. Randolph Franklin's algorithm)
bool pnpoly(const NumericMatrix& polygon, double x, double y)
{
  int nvert = polygon.nrow();
  bool c = false;
  for (int i = 0, j = nvert - 1 ; i < nvert ; j = i++)
  {
    if (((polygon(i, 1) > y) != (polygon(j, 1) > y)) &&
        (x < (polygon(j, 0) - polygon(i, 0)) * (y - polygon(i, 1)) / (polygon(j, 1) - polygon(i, 1)) + polygon(i, 0)))
    {
      c = !c;
    }
  }
  return c;
}

std::vector<Polygon> parse_polygons(const List& polygons)
{
  std::vector<Polygon> out;
  out.reserve(polygons.size());

  // Each polygon is a list of rings (MULTIPOLYGON and interior rings)
  for (List rings : polygons)
  {
    Polygon polygon;
    for (NumericMatrix ring : rings)
      polygon.push_back({ring, ring(0, 2) == 1});
    out.push_back(std::move(polygon));
  }

  return out;
}

// true if (x, y) is inside the polygon: in one of its exterior rings and not in one of its holes
bool in_polygon(const Polygon& polygon, double x, double y)
{
  bool inside = false;

  for (const Ring& ring : polygon)
  {
    if (!pnpoly(ring.coords, x, y))
      continue;

    if (!ring.exterior)
      return false; // in a hole

    inside = true;
  }

  return inside;
}

// true if (x, y) is inside at least one of the polygons
bool in_polygons(const std::vector<Polygon>& polygons, double x, double y)
{
  for (const Polygon& polygon : polygons)
  {
    if (in_polygon(polygon, x, y))
      return true;
  }

  return false;
}

void print_progress(float progress, const std::chrono::steady_clock::time_point& start)
{
  constexpr int width = 50;
  int pos = static_cast<int>(progress / 100.0 * width);

  double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
  if (elapsed < 2.0) return;

  double eta = (progress > 0.0) ? elapsed * (100.0 - progress) / progress : 0.0;

  std::string bar = "[";
  for (int i = 0 ; i < width ; ++i)
    bar += (i < pos) ? "=" : (i == pos ? ">" : " ");
  bar += "]";

  Rcout << "\r" << bar << " "
        << static_cast<int>(progress) << "% "
        << "ETA: " << static_cast<int>(eta) << "s     " << std::flush;
}

} // namespace

// [[Rcpp::export]]
List C_reader(CharacterVector ifiles, CharacterVector ofile, CharacterVector select, CharacterVector filter, Rcpp::List polygons, bool parallel = false)
{
  RLASstreamer streamer(ifiles, ofile, filter);
  streamer.select(select);
  streamer.allocation();
  streamer.parallel(parallel);

  const bool clip = polygons.size() > 0;
  const std::vector<Polygon> polys = parse_polygons(polygons);

  auto start = std::chrono::steady_clock::now();
  int counter = 0;

  while (streamer.read_point())
  {
    const LASpoint& point = streamer.point();

    if (!clip || in_polygons(polys, point.get_x(), point.get_y()))
      streamer.write_point();

    if (++counter % 10000 == 0)
    {
      Rcpp::checkUserInterrupt();
      print_progress(streamer.progress, start);
    }
  }

  Rcout << "\r" << std::string(80, ' ') << "\r" << std::flush;

  return streamer.terminate();
}
