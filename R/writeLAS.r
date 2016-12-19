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



#' Write a las or laz file
#'
#' Write a las or laz file. All the fields are optionnal except X Y and Z coordinates. If
#' the user does not provide a field such as Intensity for example but this field is requiered
#' according to the version of files specified in the header, 0 will be written in this fields.
#' @param file character. filename of .las or .laz file
#' @param header list of character. The data for the file header properly labelled
#' (see \link[rlas:readlasheader]{readlasheader})
#' @param X numeric array X data
#' @param Y numeric array Y data
#' @param Z numeric array Z data
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
writelas = function(file, header, X, Y, Z, gpstime, Intensity, ReturnNumber,
                     NumberOfReturns, ScanDirectionFlag, EdgeOfFlightline,
                     Classification, ScanAngle, UserData, PointSourceID,
                     R, G, B)
{
  islas = tools::file_ext(file) %in% c("las", "laz")

  if(length(file) > 1) stop("Please write only one file at a time", call. = F)
  if(!islas) stop("File not supported", call. = F)

  file = path.expand(file)

  I = RN = NoR = SDF = EoF = C = SA = UD = PSI = R = G = B = integer(0)
  time = numeric(0)

  if(!missing(Intensity))
    I = Intensity
  if(!missing(ReturnNumber))
    RN = ReturnNumber
  if(!missing(NumberOfReturns))
    NoR = NumberOfReturns
  if(!missing(ScanDirectionFlag))
    SDF = ScanDirectionFlag
  if(!missing(EdgeOfFlightline))
    EoF = EdgeOfFlightline
  if(!missing(Classification))
    C = Classification
  if(!missing(ScanAngle))
    SA = ScanAngle
  if(!missing(UserData))
    UD = UserData
  if(!missing(gpstime))
    time = gpstime
  if(!missing(PointSourceID))
    PSI = PointSourceID
  if(!missing(R) & !missing(G) & !missing(B))
  {
    red = R
    green = G
    blue = B
  }

  laswriter(file,header,X,Y,Z,I,RN,NoR,SDF,EoF,C,SA,UD, PSI,time,red,green,blue)
}