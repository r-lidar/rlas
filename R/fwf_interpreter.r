#' Full Waveform Interpreter
#'
#' !! This is an experimental function that may change !!
#' Raw full waveform data read from LAS files might be cryptic even with a good understanding
#' of the LAS specifications. This function interpret the full waveform data as a set of
#' XYZ coordinates and an amplitude which is the digitized voltage.
#'
#' @param data data.frame or data.table
#' @param header list. A header
#' @family header_tools
#' @return A list containing a \code{data.frame} per pulse with the XYZ coordinates of the
#' waveform and the Voltage \code{Volts} of the record.
#' @export
fwf_interpreter = function(header, data)
{
  ts    <- header[["Variable Length Records"]][["Full WaveForm Description"]][["Full WaveForm"]][["Temporal Spacing"]]
  nbits <- header[["Variable Length Records"]][["Full WaveForm Description"]][["Full WaveForm"]][["Bits per sample"]]
  nsamp <- header[["Variable Length Records"]][["Full WaveForm Description"]][["Full WaveForm"]][["Number of sample"]]
  gain  <- header[["Variable Length Records"]][["Full WaveForm Description"]][["Full WaveForm"]][["Digitizer Gain"]]
  offs  <- header[["Variable Length Records"]][["Full WaveForm Description"]][["Full WaveForm"]][["Digitizer Offset"]]

  keep  <- !duplicated(data, by = "WDPLocation")
  X     <- data[["X"]]
  Y     <- data[["Y"]]
  Z     <- data[["Z"]]
  W     <- data[["FWF"]]
  dx    <- data[["Xt"]]
  dy    <- data[["Yt"]]
  dz    <- data[["Zt"]]
  loc   <- data[["WDPLocation"]]

  FWF <- mapply(function(X, Y, Z, W, dx, dy, dz, loc, keep)
  {
    if (keep == FALSE)
      return(NULL)

    Xstart <- X + loc * dx
    Ystart <- Y + loc * dy
    Zstart <- Z + loc * dz

    t <- 0:(length(W) - 1)*ts

    Px <- Xstart - t * dx
    Py <- Ystart - t * dy
    Pz <- Zstart - t * dz

    return(data.frame(X = Px, Y = Py, Z = Pz, Volts = gain*W +offs))
  },
  X, Y, Z, W, dz, dy, dz, loc, keep,
  SIMPLIFY = FALSE)

  return(FWF)
}