# ===============================================================================
#
# PROGRAMMERS:
#
# jean-romain.roussel.1@ulaval.ca  -  https://github.com/Jean-Romain/rlas
#
# COPYRIGHT:
#
# Copyright 2016-2018 Jean-Romain Roussel
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
#' Reads data from .las or .laz files according to LAS specifications and returns
#' a \code{data.table} labelled according to LAS specifications. See the ASPRS documentation for the
#' \href{https://www.asprs.org/wp-content/uploads/2019/07/LAS_1_4_r15.pdf}{LAS file format}.
#' The optional parameters enable the user to save memory by choosing to load only the
#' attributes they need. Moreover, the function provides a streaming filter to load only the points of
#' interest into the memory and hence avoids allocating any superfluous memory.
#'
#' \strong{Select:} the 'select' argument specifies the data that will actually be loaded. For example,
#' 'xyzia' means that the x, y, and z coordinates, the intensity and the scan angle will be loaded.
#' The supported entries are t - gpstime, a - scan angle, i - intensity, n - number of returns,
#' r - return number, c - classification, s - synthetic flag, k - keypoint flag, w - withheld flag,
#' o - overlap flag (format 6+), u - user data, p - point source ID, e - edge of flight line flag,
#' d - direction of scan flag, R - red channel of RGB color, G - green channel of RGB color,
#' B - blue channel of RGB color, N - near-infrared channel, C - scanner channel (format 6+),
#' W - Full waveform.
#' Also numbers from 1 to 9 for the extra bytes data numbers 1 to 9. 0 enables all extra bytes to be
#' loaded and '*' is the wildcard that enables everything to be loaded from the LAS file. \cr
#' Note that x, y, z are implicit and always loaded. 'xyzia' is equivalent to 'ia'.\cr\cr
#' \strong{Filter:} the 'filter' argument allows filtering of the point cloud while reading files.
#' \code{rlas} relies on the well-known \code{LASlib} library written by Martin Isenburg
#' to read the binary files. Thus the package inherits the filter commands available in
#' \href{https://rapidlasso.de/product-overview/}{LAStools}. To use these filters the user can pass the
#' common commands from \code{LAStools} into the parameter \code{'filter'}. Type \code{read.las(filter = "-help")}
#' to display the \code{LASlib} documentation and the available filters.\cr\cr
#' \strong{Transform:} the 'transform' argument allows transformation of the point cloud while reading files.
#' \code{rlas} relies on the well-known \code{LASlib} library written by Martin Isenburg
#' to read the binary files. Thus the package inherits the transform commands available in
#' \href{https://rapidlasso.de/product-overview/}{LAStools}. To use these transformations the user can pass the
#' common commands from \code{LAStools} into the parameter \code{'transform'}. Type \code{read.las(transform = "-help")}
#' to display the \code{LASlib} documentation and the available transformations.
#'
#' @section Full Waveform:
#' The support of full waveform is still in development. The version 1.4.1 introduced the
#' support of point formats 4, 5, 9 and 10. The current state consists in reading the raw data.
#' We also introduced the function \link{fwf_interpreter} to interpret the raw data into something
#' easier to manipulate (but that uses more memory). The current behaviour is not set
#' in stone and is prone to design modification until version 1.5.0 where we aims to
#' get enough insight to lock our engineering choices to something that suit best the needs.
#'
#'
#' @param files array of characters
#' @param select character. select only columns of interest to save memory (see details)
#' @param filter character. streaming filters - filter data while reading the file (see details)
#' @param transform character. streaming transformation - transform data while reading the file (see details)
#' @return A \code{data.table}
#' @export
#' @examples
#' lasfile <- system.file("extdata", "example.las", package="rlas")
#'
#' lasdata <- read.las(lasfile)
#' lasdata <- read.las(lasfile, filter = "-keep_first")
#' lasdata <- read.las(lasfile, filter = "-drop_intensity_below 80")
#' lasdata <- read.las(lasfile, select = "xyzia")
#' @useDynLib rlas, .registration = TRUE
read.las = function(files, select = "*", filter = "", transform = "")
{
    if (filter == "-h" | filter == "-help")
      lasfilterusage()

    if (transform == "-h" | transform == "-help")
      lastransformusage()

    if (filter == "-h" | transform == "-h" | filter == "-help" | transform == "-help")
      return(invisible())

  filter = paste(filter, transform)
  stream.las(files, select = select, filter = filter)
}

#' Read header from a .las or .laz file
#'
#' Reads header from .las or .laz files according to LAS specifications and returns
#' a \code{list} labeled according to LAS specifications. See the ASPRS documentation for the
#' \href{https://www.asprs.org/wp-content/uploads/2019/07/LAS_1_4_r15.pdf}{LAS file format}.
#'
#' @param file filepath character string to the .las or .laz file
#' @family rlas
#' @return A \code{list}
#' @importFrom Rcpp sourceCpp
#' @export
#' @examples
#' lazfile   <- system.file("extdata", "example.las", package="rlas")
#' lasheader <- read.lasheader(lazfile)
read.lasheader = function(file)
{
  valid     <- file.exists(file)
  supported <- tools::file_ext(file) %in% c("las", "laz", "LAS", "LAZ", "ply", "PLY")
  file      <- enc2native(normalizePath(file))

  if (!valid)      stop("File not found", call. = F)
  if (!supported)  stop("File not supported", call. = F)

  data <- lasheaderreader(file)

  return(data)
}

stream.las = function(ifiles, ofile = "", select = "*", filter = "", polygons = list())
{
  stream    <- ofile != ""
  ifiles    <- enc2native(normalizePath(ifiles))
  ofile     <- enc2native(normalizePath(ofile, mustWork = FALSE))
  valid     <- file.exists(ifiles)
  supported <- tools::file_ext(ifiles) %in% c("las", "laz", "LAS", "LAZ", "ply", "PLY")

  if (!all(valid))      stop("File not found", call. = F)
  if (!all(supported))  stop("File not supported", call. = F)

  check_filter(filter)

  raw_list <- C_reader(ifiles, ofile, select, filter, polygons)

  data <- raw_list[1:3]
  data.table::setDT(data)
  n <- nrow(data)
  for (name in names(raw_list)[-(1:3)])
  {
    attr <- raw_list[[name]]
    if (n > 1L && length(attr) == 1L) attr <- R_compact_rep(n, attr)
    data[[name]] <- attr
  }

  if (stream) return(invisible())

  return(data)
}


