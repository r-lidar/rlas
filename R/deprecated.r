#' (Deprecated) Read data from a .las or .laz file
#'
#' This is a deprecated function to read las files. Use \link{read.las} or \link{read.lasheader} instead.
#'
#' @param files filepath character string to the .las or .laz files
#' @param file filepath character string to the .las or .laz file
#' @param i logical. do you want to load the Intensity field? default: TRUE
#' @param r logical. do you want to load the ReturnNumber field? default: TRUE
#' @param n logical. do you want to load the NumberOfReturns field? default: TRUE
#' @param d logical. do you want to load the ScanDirectionFlag field? default: TRUE
#' @param e logical. do you want to load the EdgeOfFlightline field? default: TRUE
#' @param c logical. do you want to load the Classification field? default: TRUE
#' @param a logical. do you want to load the ScanAngle field? default: TRUE
#' @param u logical. do you want to load the UserData field? default: TRUE
#' @param p logical. do you want to load the PointSourceID field? default: TRUE
#' @param rgb logical. do you want to load R,G and B fields? default: TRUE
#' @param t logical. do you want to load gpstime fields? default: TRUE
#' @param eb integer vector. which extra byte attributes to load (see
#' \href{http://www.asprs.org/a/society/committees/standards/LAS_1_4_r13.pdf}{LAS file format specs}).
#' default is 0 meaning that all extra fields will be loaded. \code{c(1,3,7)} to load the first, the
#' third and the seventh extra byte attributes. None is \code{numeric(0)} or \code{NULL}.
#' @param filter character. filter data while reading the file (streaming filter) without
#' allocating any useless memory. (see Details).
#' @importFrom Rcpp sourceCpp
#' @family rlas
#' @return A \code{data.table}
#' @export
#' @examples
#' lazfile <- system.file("extdata", "example.laz", package="rlas")
#'
#' lasdata <- readlasdata(lazfile)
#' lasdata <- readlasdata(lazfile, filter = "-keep_first")
#' lasdata <- readlasdata(lazfile, filter = "-drop_intensity_below 80")
#' @useDynLib rlas, .registration = TRUE
readlasdata = function(files, i = TRUE, r = TRUE, n = TRUE, d = TRUE, e = TRUE, c = TRUE, a = TRUE, u = TRUE, p = TRUE, rgb = TRUE, t = TRUE, filter = "", eb = 0)
{
  ofile = ""
  data  = streamlasdata(files, ofile, filter, i, r, n, d, e, c, a, u, p, rgb, TRUE, t, eb)
  return(data)
}

#' @rdname readlasdata
#' @export
readlasheader = function(file)
{
  valid = file.exists(file)
  islas = tools::file_ext(file) %in% c("las", "laz", "LAS", "LAZ")
  file = normalizePath(file)

  if(!valid)  stop("File not found", call. = F)
  if(!islas)  stop("File not supported", call. = F)

  data = lasheaderreader(file)

  return(data)
}