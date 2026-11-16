# ===============================================================================
#
# PROGRAMMERS:
#
# info@r-lidar.com  -  https://github.com/r-lidar/rlas
#
# COPYRIGHT:
#
# Copyright 2022-2026 Jean-Romain Roussel
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

#' ALTREP utilities
#'
#' Test if an a vector is compressed using the ALTREP framework
#'
#' @param x an R object
#'
#' @examples
#' lazfile <- system.file("extdata", "example.las", package = "rlas")
#' las <- read.las(lazfile)
#' is_compressed(las)
#'
#' # The difference is more substantial on bigger point clouds (~30%)
#' object.size(las)
#' true_size(las)
#' @return `is_compressed` return a boolean. `true_size` return the real size of the object in bytes
#' @md
#' @name compression
#' @rdname compression
NULL

#' @rdname compression
#' @export
is_compressed <- function(x)
{
  if (is.data.frame(x) | is.list(x))
    return(sapply(x, is_compact))

  return(!R_is_materialized(x))
}

#' @rdname compression
#' @export
true_size <- function(x)
{
  size <- 0
  gz <- is_compressed(x)

  if (length(gz) == 1)
  {
    size <- if (isTRUE(gz)) 644 else utils::object.size(x)
  }
  else
  {
    for (i in seq_along(gz))
    {
      if (isTRUE(gz[i]))
        size <- size + 644L
      else
        size <- size + utils::object.size(x[[i]])
    }
  }

  class(size) <- 'object_size'
  return(size)
}

is_compact <- function(x)
{
  altrep <- R_altrep_full_class(x)
  if (is.null(altrep)) return(FALSE)

  if (getRversion() < "4.6.0")
    altrep = altrep[[2]]

  if (altrep == "rlas")
  {
    if (R_is_materialized(x)) return(FALSE)
    return(TRUE)
  }

  return(FALSE)
}

