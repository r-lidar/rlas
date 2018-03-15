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



#' Write a .las or .laz file
#'
#' Write a .las or .laz file. The user provides a table with the data in column. Column names must
#' respect specific allowed names (see details). A correct and complete header must also be provided.
#' This header can optionnally be generated with \link{header_create}.
#'
#' Allowed names are "X", "Y", "Z", "gpstime", "Intensity", "ReturnNumber", "NumberOfReturns",
#' "ScanDirectionFlag", "EdgeOfFlightline", "Classification", "ScanAngle", "UserData", "PointSourceID",
#' "R", "G", "B", "NIR". All other extra columns will be written in extra bytes attributes only if the
#' header specifically states to save these data into extra bytes attributes. To use the full potential
#' of the function \code{write.las} it is recommended to read the whole specifications of the
#' \href{http://www.asprs.org/a/society/committees/standards/LAS_1_4_r13.pdf}{LAS file format}.
#' Otherwise user can rely on automated procedures that are expected to be sufficient for most usages.
#'
#' @param file character. file path to .las or .laz file
#' @param header list. It can be partially recycled from another file (see \link{read.lasheader}) and
#' updated with \link{header_update} or generated with \link{header_create}.
#' @param data data.frame or data.table that contains the data to write in the file. Colunm names must
#' respect the imposed nomenclature (see details)
#' @export
#' @importFrom Rcpp sourceCpp
#' @family rlas
#' @return void
#' @examples
#' lasdata = data.frame(X = c(339002.889, 339002.983, 339002.918),
#'                      Y = c(5248000.515, 5248000.478, 5248000.318),
#'                      Z = c(975.589, 974.778, 974.471),
#'                      gpstime = c(269347.281418006, 269347.281428006, 269347.281438006),
#'                      Intensity = c(82L, 54L, 27L),
#'                      ReturnNumber = c(1L, 1L, 2L),
#'                      NumberOfReturns = c(1L, 1L, 2L),
#'                      ScanDirectionFlag = c(1L, 1L, 1L),
#'                      EdgeOfFlightline = c(1L, 0L, 0L),
#'                      Classification = c(1L, 1L, 1L),
#'                      ScanAngle = c(-21L, -21L, -21L),
#'                      UserData = c(32L, 32L, 32L),
#'                      PointSourceID = c(17L, 17L, 17L))
#'
#' lasheader = header_create(lasdata)
#' file = file.path(tempdir(), "temp.laz")
#'
#' write.las(file, lasheader, lasdata)
write.las = function(file, header, data)
{
  file = path.expand(file)
  check_output_file(file)
  check_header(header)
  check_data(data)
  check_data_vs_header(header, data)
  C_writer(file, header, data)
}