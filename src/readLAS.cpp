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
#include "rlasstreamer.h"
#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lasfilter.hpp"

using namespace Rcpp;

bool point_in_polygon(NumericVector, NumericVector, double, double);

// Read data from a las and laz file with LASlib
//
// Read data from a las or laz file in format 1 to 4 according to LAS specification and return a list.
//
// This function musn't be used as is. It is an internal function. Please use \link[lidR:readLAS]{readLAS} abstraction.
//
// @param ifiles character. The name of the files to be read
// @param ofile character. The name of the output file. If ofile = "" then it is loaded into R.
// @param filter character. LASlib filters
// @param i bool. do you want to load Intensity field?
// @param r bool. do you want to load ReturnNumber field?
// @param n bool. do you want to load NumberOfReturns field?
// @param s bool. do you want to load ScanDirectionFlag field?
// @param e bool. do you want to load EdgeofFlightline field?
// @param c bool. do you want to load Classification field?
// @param a bool. do you want to load intensity field?
// @param u bool. do you want to load UserData field?
// @param p bool. do you want to load PointSourceID field?
// @param rgb bool. do you want to load intensity R,G and B?
// @param eb IntegerVector. which extra byte attributes to load (see LAS v1.4)? zero-based numering.
//
// @return Rcpp::List

// [[Rcpp::export]]
List lasdatareader(CharacterVector ifiles, CharacterVector ofile, CharacterVector filter, bool i, bool r, bool n, bool d, bool e, bool c, bool a, bool u, bool p, bool rgb, bool t, IntegerVector eb)
{
  try
  {
    RLASstreamer streamer(ifiles, ofile, filter);

    streamer.read_t(t);
    streamer.read_i(i);
    streamer.read_r(r);
    streamer.read_n(n);
    streamer.read_d(d);
    streamer.read_e(e);
    streamer.read_c(c);
    streamer.read_a(a);
    streamer.read_u(u);
    streamer.read_p(p);
    streamer.read_rgb(rgb);
    streamer.read_eb(eb);

    streamer.allocation();

    while(streamer.read_point())
    {
      streamer.write_point();
    }

    return streamer.terminate();
  }
  catch (std::exception const& ex)
  {
    Rcerr << "Error: " << ex.what() << std::endl;
    return(List(0));
  }
}

// [[Rcpp::export]]
List lasdatareader_inpoly(CharacterVector ifiles, NumericVector x, NumericVector y, CharacterVector ofile, CharacterVector filter, bool i, bool r, bool n, bool d, bool e, bool c, bool a, bool u, bool p, bool rgb, bool t, IntegerVector eb)
{
  try
  {
    RLASstreamer streamer(ifiles, ofile, filter);

    streamer.read_t(t);
    streamer.read_i(i);
    streamer.read_r(r);
    streamer.read_n(n);
    streamer.read_d(d);
    streamer.read_e(e);
    streamer.read_c(c);
    streamer.read_a(a);
    streamer.read_u(u);
    streamer.read_p(p);
    streamer.read_rgb(rgb);
    streamer.read_eb(eb);

    streamer.allocation();

    while(streamer.read_point())
    {
      double xi = streamer.point()->get_x();
      double yi = streamer.point()->get_y();

      if (point_in_polygon(x, y, xi, yi))
      {
        streamer.write_point();
      }
    }

    return streamer.terminate();
  }
  catch (std::exception const& ex)
  {
    Rcerr << "Error: " << ex.what() << std::endl;
    return(List(0));
  }
}