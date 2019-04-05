#' Full Waveform interpreter
#'
#' Full waveform data read from las files might be cryptic even with a good understanding of the LAS
#' specifications. This function interpret the Full waveform data in a set of XYZ coordinates and and
#' amplitude normalized by the maximum value possible (i.e 2 power the number of bits used to store
#' the full waveform)
#'
#' @param data data.frame or data.table
#' @param header list. A header
#' @family header_tools
#' @return A list containing a \code{data.frame} per points
#' @export
fwf_interpreter = function(header, data)
{
  ts    <- header[["Variable Length Records"]][["Full WaveForm Description"]][["Full WaveForm"]][["Temporal Spacing"]]
  nbits <- header[["Variable Length Records"]][["Full WaveForm Description"]][["Full WaveForm"]][["Bits per sample"]]
  nsamp <- header[["Variable Length Records"]][["Full WaveForm Description"]][["Full WaveForm"]][["Number of sample"]]

  X     <- data[["X"]]
  Y     <- data[["Y"]]
  Z     <- data[["Z"]]
  W     <- data[["FWF"]]
  dx    <- data[["Xt"]]
  dy    <- data[["Yt"]]
  dz    <- data[["Zt"]]
  loc   <- data[["WDPLocation"]]

  FWF <- mapply(function(X, Y, Z, W, dx, dy, dz, loc)
  {
    Xstart <- X + loc * dx
    Ystart <- Y + loc * dy
    Zstart <- Z + loc * dz

    t <- 0:(length(W) - 1)*ts

    Px <- Xstart - t * dx
    Py <- Ystart - t * dy
    Pz <- Zstart - t * dz

    return(data.frame(X = Px, Y = Py, Z = Pz, t = t, Amplitude = W/2^nbits))
  },
  X, Y, Z, W, dz, dy, dz, loc,
  SIMPLIFY = FALSE)

  return(FWF)
}