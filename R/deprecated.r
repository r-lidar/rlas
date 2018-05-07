#' Read data from a .las or .laz file
#'
#' (deprecated) Use \link{read.las}
#'
#' @param files filepath character string to the .las or .laz files
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
#' @return A \code{data.table}
#' @export
#' @useDynLib rlas, .registration = TRUE
readlasdata = function(files, i = TRUE, r = TRUE, n = TRUE, d = TRUE, e = TRUE, c = TRUE, a = TRUE, u = TRUE, p = TRUE, rgb = TRUE, t = TRUE, filter = "", eb = 0)
{
  #.Deprecated("read.las")
  ofile = ""
  data  = streamlasdata(files, ofile, filter, i, r, n, d, e, c, a, u, p, rgb, t, eb)
  return(data)
}

streamlasdata = function(ifiles, ofile = "", filter = "", i = TRUE, r = TRUE, n = TRUE, d = TRUE, e = TRUE, c = TRUE, a = TRUE, u = TRUE, p = TRUE, rgb = TRUE, t = TRUE, eb = 0)
{
  check_file(ifiles)
  check_filter(filter)

  ifiles = normalizePath(ifiles)

  if (ofile != "")
    ofile = suppressWarnings(normalizePath(ofile))

  if (is.null(eb))
    eb = numeric(0)

  # converts eb to zero-based numbering
  data = lasdatareader(ifiles, ofile, filter, i, r, n, d, e, c, a, u, p, rgb, t, eb-1)

  if (ofile != "")
    return(invisible())

  data.table::setDT(data)

  return(data)
}

streamlasdata_inpoly = function(ifiles, xpoly, ypoly, ofile = "", filter = "", i = TRUE, r = TRUE, n = TRUE, d = TRUE, e = TRUE, c = TRUE, a = TRUE, u = TRUE, p = TRUE, rgb = TRUE, t = TRUE, eb = c(1:9))
{
  check_file(ifiles)
  check_filter(filter)

  ifiles = normalizePath(ifiles)

  if (ofile != "")
    ofile = suppressWarnings(normalizePath(ofile))

  if (is.null(eb))
    eb = numeric(0)

  if (length(xpoly) != length(ypoly))
    stop("Invalide polygon", call. = FALSE)

  if (xpoly[1] != xpoly[length(xpoly)] | xpoly[1] != xpoly[length(xpoly)])
    stop("The polygon is not closed", call. = FALSE)

  xmin <- min(xpoly)-0.1
  xmax <- max(xpoly)+0.1
  ymin <- min(ypoly)-0.1
  ymax <- max(ypoly)+0.1

  filter <- paste(paste("-inside", xmin, ymin, xmax, ymax), filter)

  # converts eb to zero-based numbering
  data = lasdatareader_inpoly(ifiles, xpoly, ypoly, ofile, filter, i, r, n, d, e, c, a, u, p, rgb, t, eb-1)

  if (ofile != "")
    return(invisible())

  data.table::setDT(data)

  return(data)
}

#' (deprecated) Read header from a .las or .laz file
#'
#' (deprecated) Use \link{read.lasheader}
#'
#' @param file filepath character string to the .las or .laz file
#' @return A \code{list}
#' @importFrom Rcpp sourceCpp
#' @export
readlasheader = function(file)
{
  #.Deprecated("read.lasheader")
  valid = file.exists(file)
  islas = tools::file_ext(file) %in% c("las", "laz", "LAS", "LAZ")
  file = normalizePath(file)

  if(!valid)  stop("File not found", call. = F)
  if(!islas)  stop("File not supported", call. = F)

  data = lasheaderreader(file)

  return(data)
}

#' (deprecated) Write a .las or .laz file
#'
#' (deprecated) Use \link{write.las}
#'
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
#' @return void
writelas = function(file, header, X, Y, Z, gpstime, Intensity, ReturnNumber,
                    NumberOfReturns, ScanDirectionFlag, EdgeOfFlightline,
                    Classification, ScanAngle, UserData, PointSourceID,
                    R, G, B)
{

  #.Deprecated("write.las")
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

  laswriter(file,header,X,Y,Z,I,RN,NoR,SDF,EoF,C,SA,UD, PSI,time,red,green,blue)
}