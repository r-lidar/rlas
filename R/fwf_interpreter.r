# ===============================================================================
#
# PROGRAMMERS:
#
# info@r-lidar.com  -  https://github.com/r-lidar/rlas
#
# COPYRIGHT:
#
# Copyright 2019-2026 Jean-Romain Roussel
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

#' Full Waveform Interpreter
#'
#' \bold{This is an experimental function that may change}\cr
#' Raw full waveform data read from LAS files might be cryptic even with a good understanding
#' of the LAS specifications. This function interpret the full waveform data as a set of
#' XYZ coordinates and an amplitude which is the digitized voltage.
#'
#' @param data data.frame or data.table
#' @param header list. A header
#' @family header_tools
#' @return A list containing a \code{data.frame} per pulse with the XYZ coordinates of the
#' waveform and the voltage of the record (\code{Amplitude})
#' @export
#' @examples
#' \dontrun{
#' f <- system.file("extdata", "fwf.laz", package="rlas")
#' head <- read.lasheader(f)
#' data <- read.las(f)
#' fwf <- fwf_interpreter(head, data)
#' }
fwf_interpreter = function(header, data)
{
  ts    <- header[["Variable Length Records"]][["Full WaveForm Description"]][["Full WaveForm"]][["Temporal Spacing"]]
  nbits <- header[["Variable Length Records"]][["Full WaveForm Description"]][["Full WaveForm"]][["Bits per sample"]]
  nsamp <- header[["Variable Length Records"]][["Full WaveForm Description"]][["Full WaveForm"]][["Number of sample"]]
  gain  <- header[["Variable Length Records"]][["Full WaveForm Description"]][["Full WaveForm"]][["Digitizer Gain"]]
  offs  <- header[["Variable Length Records"]][["Full WaveForm Description"]][["Full WaveForm"]][["Digitizer Offset"]]

  if (is.null(ts) | is.null(gain) | is.null(offs))
    stop("The header is incomplete and does not contain the VLR data required to interpret the waveform.", call. = FALSE)

  if (is.null(data[["Xt"]]) | is.null(data[["Yt"]]) | is.null(data[["Zt"]]) | is.null(data[["WDPLocation"]]))
    stop("The wave data packet descriptors are incomplete and do not contain the data required to interpret the waveform.", call. = FALSE)

  if ( is.null(data[["FWF"]]))
    stop("The raw waveform is missing.", call. = FALSE)

  FWF <- apply(data, 1, function(P)
  {
    if (length(P$FWF) == 1 && P$FWF == 0)
      return(NULL)

    Xstart <- P$X + P$WDPLocation * P$Xt
    Ystart <- P$Y + P$WDPLocation * P$Yt
    Zstart <- P$Z + P$WDPLocation * P$Zt

    t <- 0:(length(P$FWF) - 1)*ts

    Px <- Xstart - t * P$Xt
    Py <- Ystart - t * P$Yt
    Pz <- Zstart - t * P$Zt

    return(data.frame(X = Px, Y = Py, Z = Pz, Amplitude = gain * P$FWF + offs))
  })

  return(FWF)
}
