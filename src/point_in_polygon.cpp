/*
===============================================================================

 PROGRAMMERS:

jean-romain.roussel.1@ulaval.ca  -  https://github.com/Jean-Romain/lidR

COPYRIGHT:

Copyright 2016 Jean-Romain Roussel

This file is part of rlas R package.

lidR is free software: you can redistribute it and/or modify
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
using namespace Rcpp;

// [[Rcpp::export]]
bool point_in_polygon(NumericVector vertx, NumericVector verty, double pointx, double pointy)
{
  bool c = false;
  int nvert = vertx.length();

  for (int i = 0, j = nvert-1 ; i < nvert ; j = i++)
  {
    if( ((verty[i] > pointy) != (verty[j] > pointy)) && (pointx < (vertx[j]-vertx[i]) * (pointy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
      c = !c;
  }

  return c;
}

