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

#ifdef __GNUC__
  #if __GNUC__ < 5

#include "gcc_4_9_patch/std_is_trivially.hpp"
#include "gcc_4_9_patch/boost_compare_patched.hpp"

  #endif
#endif

#include <Rcpp.h>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include "rlasstreamer.h"
#include "laspoint.hpp"
#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lasfilter.hpp"

typedef boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian> Point;
typedef boost::geometry::model::polygon<Point> Polygon;
typedef boost::geometry::model::multi_polygon<Polygon> MultiPolygon;

// [[Rcpp::export]]
List C_reader(CharacterVector ifiles, CharacterVector ofile, CharacterVector select, CharacterVector filter, std::string filter_wkt)
{
  RLASstreamer streamer(ifiles, ofile, filter);
  streamer.select(select);
  streamer.allocation();

  if (filter_wkt == "")
  {
    while(streamer.read_point())
      streamer.write_point();
  }
  else if (filter_wkt.find("MULTIPOLYGON") != std::string::npos)
  {
    Point p;
    MultiPolygon polygons;
    boost::geometry::read_wkt(filter_wkt, polygons);

    while(streamer.read_point())
    {
      p.set<0>(streamer.point()->get_x());
      p.set<1>(streamer.point()->get_y());

      if (boost::geometry::covered_by(p, polygons))
        streamer.write_point();
    }
  }
  else if (filter_wkt.find("POLYGON") != std::string::npos)
  {
    Point p;
    Polygon polygon;
    boost::geometry::read_wkt(filter_wkt, polygon);

    while(streamer.read_point())
    {
      p.set<0>(streamer.point()->get_x());
      p.set<1>(streamer.point()->get_y());

      if (boost::geometry::covered_by(p, polygon))
        streamer.write_point();
    }
  }
  else
    stop("WKT not supported. Should be a POLYGON or MULTIPOLYGON");

  return streamer.terminate();
}