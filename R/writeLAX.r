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



#' Write a .lax file from a .las or .laz file
#'
#' Write a lax file from a las or laz file. A lax file is a tiny file which can come with a
#' las or laz and which spatially index the data to make faster spatial queries. It has
#' been invented by Martin Isenburg in LASlib. rlas support lax file and enable to write
#' a lax file with default settings. For more options, use lasindex from binaries provided
#' by LASlib (for more informations see references)
#'
#' @references
#' https://rapidlasso.com/\cr
#' https://rapidlasso.com/2012/12/03/lasindex-spatial-indexing-of-lidar-data/\cr
#' https://github.com/LAStools/LAStools
#' @param file character. filename of .las or .laz file
#' @return NULL
#' @export
writelax = function(file)
{
  islas = tools::file_ext(file) %in% c("las", "laz")

  if(length(file) > 1)
    stop("Please write only one file at a time.", call. = F)

  if(!islas)
    stop("File not supported. Extension should be 'las' or 'laz'", call. = F)

  file = path.expand(file)

  laxwriter(file)

  return(invisible())
}