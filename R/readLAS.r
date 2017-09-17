# ===============================================================================
#
# PROGRAMMERS:
#
# jean-romain.roussel.1@ulaval.ca  -  https://github.com/Jean-Romain/rlas
#
# COPYRIGHT:
#
# Copyright 2016 Jean-Romain Roussel
#
# This file is part of rlas R package.
#
# rlas is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>
#
# ===============================================================================



#' Read data from a .las or .laz file
#'
#' Reads data from .las or .laz files in format 1 to 4 according to LAS specifications and returns
#' a \code{data.table} labeled according to LAS specifications. See the ASPRS documentation for the
#' \href{http://www.asprs.org/a/society/committees/standards/LAS_1_4_r13.pdf}{LAS file format}.
#' The optional logical parameters enables the user to save memory by choosing to load only the
#' fields they need. Indeed, data is loaded into the computer's memory (RAM) suboptimally because
#' R does not accommodate many different data types. Moreover, the function provides a streaming filter
#' to load only the points of interest into the memory without allocating any superfluous memory.
#'
#' Because \code{rlas} relies on the well-known \code{LASlib} library written by Martin Isenburg
#' to read the binary files, the package also inherits the filter commands available in
#' \href{https://rapidlasso.com/lastools/}{LAStools}. To use these filters the user can pass the
#' common commands from \code{LAStools} into the parameter \code{'filter'}. Type \code{rlas:::lasfilterusage()} to
#' display the \code{LASlib} documentation and the available filters.\cr\cr
#' The filter works in two passes. First it streams the file without loading anything and counts
#' the number of points of interest. Then it allocates the necessary amount of memory and reads the file
#' a second time, and stores the points of interest in the computer's memory (RAM).
#'
#' @param files filepath character string to the .las or .laz files
#' @param i logical. do you want to load the Intensity field? default: TRUE
#' @param r logical. do you want to load the ReturnNumber field? default: TRUE
#' @param n logical. do you want to load the NumberOfReturns field? default: TRUE
#' @param d logical. do you want to load the ScanDirectionFlag field? default: TRUE
#' @param e logical. do you want to load the EdgeOfFlightline field? default: TRUE
#' @param c logical. do you want to load the Classification field? default: TRUE
#' @param a logical. do you want to load the ScanAngle field? default: TRUE
#' @param u logical. do you want to load the UserData field? default: TRUE
#' @param p logical. do you want to load the PointSourceID field? default: TRUE
#' @param rgb logical. do you want to load R,G and B fields? default: TRUE
#' @param t logical. do you want to load gpstime fields? default: TRUE
#' @param filter character. filter data while reading the file (streaming filter) without
#' allocating any useless memory. (see Details).
#' @importFrom Rcpp sourceCpp
#' @family rlas
#' @return A \code{data.table}
#' @export
#' @examples
#' lazfile <- system.file("extdata", "example.laz", package="rlas")
#'
#' lasdata <- readlasdata(lazfile)
#' lasdata <- readlasdata(lazfile, filter = "-keep_first")
#' lasdata <- readlasdata(lazfile, filter = "-drop_intensity_below 80")
#' @useDynLib rlas, .registration = TRUE
readlasdata = function(files, i = TRUE, r = TRUE, n = TRUE, d = TRUE, e = TRUE, c = TRUE, a = TRUE, u = TRUE, p = TRUE, rgb = TRUE, t = TRUE, filter = "")
{
  check_file(files)
  check_filter(filter)

  files = normalizePath(files)

  if (filter == "")
    data = lasdatareader(files, i, r, n, d, e, c, a, u, p, rgb, t)
  else
    data = lasdatastreamer(files, "", filter, i, r, n, d, e, c, a, u, p, rgb, t, numeric(0), numeric(0))

  data.table::setDT(data)

  return(data)
}

streamlasdata = function(ifiles, i = TRUE, r = TRUE, n = TRUE, d = TRUE, e = TRUE, c = TRUE, a = TRUE, u = TRUE, p = TRUE, rgb = TRUE, t = TRUE, filter = "", ofile = "", xpoly = NULL, ypoly = NULL)
{
  check_file(ifiles)
  check_filter(filter)

  ifiles = normalizePath(ifiles)

  if (ofile != "")
    ofile  = normalizePath(ofile)

  if (!is.null(xpoly) & !is.null(ypoly))
  {
    if (length(xpoly) != length(ypoly))
      stop("Invalide polygon", call. = FALSE)

    xmin <- min(xpoly)-0.5
    xmax <- max(xpoly)+0.5
    ymin <- min(ypoly)-0.5
    ymax <- max(ypoly)+0.5

    filter <- paste("-inside", xmin, ymin, xmax, ymax)
  }


  data = lasdatastreamer(ifiles, ofile, filter, i, r, n, d, e, c, a, u, p, rgb, t, xpoly, ypoly)

  if (ofile != "")
    return(invisible())

  data.table::setDT(data)

  return(data)
}

#' Read header from a .las or .laz file
#'
#' Reads header from .las or .laz files in format 1 to 4 according to LAS specifications and returns
#' a \code{list} labeled according to LAS specifications. See the ASPRS documentation for the
#' \href{http://www.asprs.org/a/society/committees/standards/LAS_1_4_r13.pdf}{LAS file format}.
#'
#' @param file filepath character string to the .las or .laz file
#' @family rlas
#' @return A \code{list}
#' @importFrom Rcpp sourceCpp
#' @export
#' @examples
#' lazfile   <- system.file("extdata", "example.laz", package="rlas")
#' lasheader <- readlasheader(lazfile)
readlasheader = function(file)
{
  valid = file.exists(file)
  islas = tools::file_ext(file) %in% c("las", "laz", "LAS", "LAZ")
  file = normalizePath(file)

  if(!valid)  stop("File not found", call. = F)
  if(!islas)  stop("File not supported", call. = F)

  data = lasheaderreader(file)

  return(data)
}

check_file = function(file)
{
  valid = file.exists(file)
  islas = tools::file_ext(file) %in% c("las", "laz", "LAS", "LAZ")
  file = normalizePath(file)

  if (!all(valid))
    stop("File not found", call. = F)

  if (!all(islas))
    stop("File not supported", call. = F)
}

check_filter = function(filter)
{
  if (!is.character(filter))
    stop("Incorrect argument 'filter'", call. = F)
}
