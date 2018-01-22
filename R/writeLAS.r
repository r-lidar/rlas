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



#' Write a .las or .laz file
#'
#' Write a .las or .laz file. All the fields are optional except X, Y and Z coordinates. If
#' the user does not provide a field such as \code{Intensity}, for example, but this field is required
#' according to the version of the file specified in the \code{header}, 0 will be written in this field.
#' For more informations, see the ASPRS documentation for the
#' \href{http://www.asprs.org/a/society/committees/standards/LAS_1_4_r13.pdf}{LAS file format}.
#' @param file character. filename of .las or .laz file
#' @param header list of character. The data for the file header properly labelled
#' (see \link[rlas:readlasheader]{readlasheader})
#' @param X numeric array X data
#' @param Y numeric array Y data
#' @param Z numeric array Z data
#' @param ExtratBytes data.table or data.frame of Extra Bytes numeric vectors. Must correspond to header Extra Bytes Description content.
#' @param Intensity integer array intensity data
#' @param ReturnNumber integer array return number data
#' @param NumberOfReturns integer array number of returns data
#' @param ScanDirectionFlag integer array scan direction flag data
#' @param EdgeOfFlightline integer array edge of flightline data
#' @param Classification integer array classification data
#' @param ScanAngle integer array scan angle data
#' @param UserData integer array user data data
#' @param PointSourceID integer array point source id data
#' @param gpstime numeric array gpstime data
#' @param R integer array red data
#' @param G integer array green data
#' @param B integer array blue data
#' @export
#' @importFrom Rcpp sourceCpp
#' @family rlas
#' @return void
writelas = function(file, header, X, Y, Z, ExtraBytes, gpstime, Intensity, ReturnNumber,
                    NumberOfReturns, ScanDirectionFlag, EdgeOfFlightline,
                    Classification, ScanAngle, UserData, PointSourceID,
                    R, G, B)
{
  islas = tools::file_ext(file) %in% c("las", "laz")

  if(length(file) > 1)
    stop("Please write only one file at a time.", call. = F)

  if(!islas)
    stop("File not supported. Extension should be 'las' or 'laz'", call. = F)

  if(is.null(header[["X offset"]]) | is.null(header[["Y offset"]]) | is.null(header[["Z offset"]]))
    stop("Offsetting not defined in the header", call. = FALSE)

  if(is.null(header[["X scale factor"]]) | is.null(header[["Y scale factor"]]) | is.null(header[["Z scale factor"]]))
    stop("Scaling not defined in the header", call. = FALSE)

  if(is.null(header[["File Source ID"]]))
	   stop("File source ID not defined in the header", call. = FALSE)

  if(is.null(header[["Version Major"]]))
    stop("Version major not defined in the header", call. = FALSE)

  if(is.null(header[["Version Minor"]]))
    stop("Version minor not defined in the header", call. = FALSE)

  if(is.null(header[["File Creation Day of Year"]]))
    stop("File Creation Day of Year not defined in the header", call. = FALSE)

  if(is.null(header[["File Creation Year"]]))
    stop("File Creation Year not defined in the header", call. = FALSE)

  if(is.null(header[["Point Data Format ID"]]))
    stop("Point Data Format ID not defined in the header", call. = FALSE)

  if(is.null(header[["Point Data Record Length"]]))
    stop("Point Data Record Length not defined in the header", call. = FALSE)

  file = path.expand(file)

  I <- RN <- NoR <- SDF <- EoF <- C <- SA <- UD <- PSI <- red <- green <- blue <- integer(0)
  time <- numeric(0)
  EB <- data.frame()

  if(!missing(ExtraBytes))
    EB <- ExtraBytes
  if(!missing(Intensity))
    I <- Intensity
  if(!missing(ReturnNumber))
    RN <- ReturnNumber
  if(!missing(NumberOfReturns))
    NoR <- NumberOfReturns
  if(!missing(ScanDirectionFlag))
    SDF <- ScanDirectionFlag
  if(!missing(EdgeOfFlightline))
    EoF <- EdgeOfFlightline
  if(!missing(Classification))
    C <- Classification
  if(!missing(ScanAngle))
    SA <- ScanAngle
  if(!missing(UserData))
    UD <- UserData
  if(!missing(gpstime))
    time <- gpstime
  if(!missing(PointSourceID))
    PSI <- PointSourceID
  if(!missing(R) & !missing(G) & !missing(B))
  {
    red <- R
    green <- G
    blue <- B
  }

  laswriter(file,header,X,Y,Z,EB,I,RN,NoR,SDF,EoF,C,SA,UD, PSI,time,red,green,blue)
}