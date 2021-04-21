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

  X     <- data[["X"]]
  Y     <- data[["Y"]]
  Z     <- data[["Z"]]
  W     <- data[["FWF"]]
  dx    <- data[["Xt"]]
  dy    <- data[["Yt"]]
  dz    <- data[["Zt"]]
  loc   <- data[["WDPLocation"]]

  if (is.null(ts) | is.null(gain) | is.null(offs))
    stop("The header is incomplete and does not contain the VLR data required to interpret the waveform.", call. = FALSE)

  if (is.null(loc) | is.null(dx) | is.null(dy) | is.null(dz))
    stop("The wave data packet descriptors are incomplete and do not contain the data required to interpret the waveform.", call. = FALSE)

  if (is.null(W))
    stop("The raw waveform is missing.", call. = FALSE)

  FWF <- mapply(function(X, Y, Z, W, dx, dy, dz, loc)
  {
    if (length(W) == 1 && W == 0)
      return(NULL)

    Xstart <- X + loc * dx
    Ystart <- Y + loc * dy
    Zstart <- Z + loc * dz

    t <- 0:(length(W) - 1)*ts

    Px <- Xstart - t * dx
    Py <- Ystart - t * dy
    Pz <- Zstart - t * dz

    return(data.frame(X = Px, Y = Py, Z = Pz, Amplitude = gain*W + offs))
  },
  X, Y, Z, W, dx, dy, dz, loc,
  SIMPLIFY = FALSE)

  return(FWF)
}
