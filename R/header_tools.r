#' Public header block tools
#'
#' Create or update a header for a las file from a dataset. A las file consists of two parts. A
#' header that describes the data and the data itself. These functions make valid headers (public
#' header block only) that can be used in \link{write.las}.
#'
#' \code{header_create} makes a full header from data. \code{header_update} modifies the information that
#' needs to be updated. But most of the original information is not modified, for example point data
#' format is kept 'as is'.
#'
#' @param data data.frame or data.table
#' @param header list. A header
#'
#' @return A list containing the metadata required to write a las file.
#' @export
#' @examples
#' lasdata = data.frame(X = c(339002.889, 339002.983, 339002.918),
#'                      Y = c(5248000.515, 5248000.478, 5248000.318),
#'                      Z = c(975.589, 974.778, 974.471),
#'                      gpstime = c(269347.281418006, 269347.281428006, 269347.281438006),
#'                      Intensity = c(82L, 54L, 27L),
#'                      ReturnNumber = c(1L, 1L, 2L),
#'                      NumberOfReturns = c(1L, 1L, 2L),
#'                      ScanDirectionFlag = c(1L, 1L, 1L),
#'                      EdgeOfFlightline = c(1L, 0L, 0L),
#'                      Classification = c(1L, 1L, 1L),
#'                      ScanAngle = c(-21L, -21L, -21L),
#'                      UserData = c(32L, 32L, 32L),
#'                      PointSourceID = c(17L, 17L, 17L),
#'                      treeID = c(1L, 1L, 1L))
#'
#' lasheader = header_create(lasdata)
header_create = function(data)
{
  fields = names(data)

  header = list()
  header[["File Signature"]] = "LASF"
  header[["File Source ID"]] = 0L
  header[["Global Encoding"]] = list(`GPS Time Type` = TRUE,
                                     `Waveform Data Packets Internal` = FALSE,
                                     `Waveform Data Packets External` = FALSE,
                                     `Synthetic Return Numbers` = FALSE,
                                      WKT = FALSE,
                                     `Aggregate Model` = FALSE)
  header[["Project ID - GUID"]] = uuid::UUIDgenerate()
  header[["Version Major"]] = 1L
  header[["Version Minor"]] = 2L
  header[["System Identifier"]] = "rlas R package"
  header[["Generating Software"]] = "rlas R package"
  header[["File Creation Day of Year"]] = as.numeric(strftime(Sys.time(), format = "%j"))
  header[["File Creation Year"]] = as.numeric(strftime(Sys.time(), format = "%Y"))
  header[["Header Size"]] = 227
  header[["Offset to point data"]] = 227
  header[["Number of point records"]] = dim(data)[1]
  header[["Min X"]] = min(data$X)
  header[["Min Y"]] = min(data$Y)
  header[["Min Z"]] = min(data$Z)
  header[["Max X"]] = max(data$X)
  header[["Max Y"]] = max(data$Y)
  header[["Max Z"]] = max(data$Z)
  header[["X offset"]] = header[["Min X"]]
  header[["Y offset"]] = header[["Min Y"]]
  header[["Z offset"]] = header[["Min Z"]]
  header[["X scale factor"]] = 0.01
  header[["Y scale factor"]] = 0.01
  header[["Z scale factor"]] = 0.01

  if ("ReturnNumber" %in% fields)
    header[["Number of points by return"]] <- fast_table(data$ReturnNumber, 5L)

  if ("NIR" %in% fields) # format 8
  {
    header[["Point Data Format ID"]] = 8
    header[["Point Data Record Length"]] = 38
  }
  else if ("gpstime" %in% fields) # format 1, 3, 6, 7
  {
    if (all(c("R", "G", "B") %in% fields))  # format 3 (6 not supported)
    {
      header[["Point Data Format ID"]] = 3
      header[["Point Data Record Length"]] = 34
    }
    else # format 1 (7 not supported)
    {
      header[["Point Data Format ID"]] = 1
      header[["Point Data Record Length"]] = 28
    }
  }
  else # format 0 or 2
  {
    if (all(c("R", "G", "B") %in% fields))
    {
      header[["Point Data Format ID"]] = 2
      header[["Point Data Record Length"]] = 26
    }
    else {
      header[["Point Data Format ID"]] = 0
      header[["Point Data Record Length"]] = 20
    }
  }

  header[["Variable Length Records"]] = list()

  return(header)
}

#' @export
#' @rdname header_create
header_update = function(header, data)
{
  fields = names(data)

  if ("ReturnNumber" %in% fields)
  {
    n <- if (header[["Version Minor"]] < 4) 5L else 15L
    header[["Number of points by return"]] <- fast_table(data$ReturnNumber, n)
  }

  header[["Number of point records"]] <- nrow(data)
  header[["Min X"]] <- min(data$X)
  header[["Min Y"]] <- min(data$Y)
  header[["Min Z"]] <- min(data$Z)
  header[["Max X"]] <- max(data$X)
  header[["Max Y"]] <- max(data$Y)
  header[["Max Z"]] <- max(data$Z)

  return(header)
}

#' Variable length records tools
#'
#' Functions that update a header to describe variable length records according to the
#' \href{https://www.asprs.org/a/society/committees/standards/LAS_1_4_r13.pdf}{LAS specifications}
#'
#' @param header list
#' @param name character. The name of the extrabytes attributes to add in the file.
#' @param desc character. The description of the extrabytes attributes to add in the file.
#' @param type integer. The data type of the extrabytes attributes (page 25 of the spec).
#' @param min,max numeric or integer. The minimum and maximum value of the data. NULL if not relevant.
#' @param scale,offset numeric. The scale and offset of the data. NULL if not relevant.
#' @param NA_value numeric or integer. NA is not a valid value. At writing time it will be replaced by
#' this value that will be considered as NA. NULL if not relevant.
#' @param data vector. Data that must be added in the extrabytes attributes.
#'
#' @examples
#' data = data.frame(X = c(339002.889, 339002.983, 339002.918),
#'                   Y = c(5248000.515, 5248000.478, 5248000.318),
#'                   Z = c(975.589, 974.778, 974.471),
#'                   gpstime = c(269347.281418006, 269347.281428006, 269347.281438006),
#'                   Intensity = c(82L, 54L, 27L),
#'                   ReturnNumber = c(1L, 1L, 2L),
#'                   NumberOfReturns = c(1L, 1L, 2L),
#'                   ScanDirectionFlag = c(1L, 1L, 1L),
#'                   EdgeOfFlightline = c(1L, 0L, 0L),
#'                   Classification = c(1L, 1L, 1L),
#'                   ScanAngle = c(-21L, -21L, -21L),
#'                   UserData = c(32L, 32L, 32L),
#'                   PointSourceID = c(17L, 17L, 17L),
#'                   treeID = c(1L, 1L, 1L))
#'
#' lasheader = header_create(data)
#' lasheader[["Variable Length Records"]]
#'
#' lasheader = header_add_extrabytes(lasheader, data$treeID, "treeID", "An id for each tree")
#' lasheader[["Variable Length Records"]]
#' @export
header_add_extrabytes = function(header, data, name, desc)
{
  stopifnot(is.list(header), is.vector(data), is.character(name), is.character(desc))

  type = class(data)
  has_na = anyNA(data)
  offset = NULL
  dmin = NULL
  dmax = NULL
  NA_value = NULL
  scale = NULL

  if (has_na)
  {
    all_na = all(is.na(data))

    if (!all_na)
    {
      dmin = min(data, na.rm = TRUE)
      dmax = max(data, na.rm = TRUE)
    }
  }
  else
  {
    dmin = min(data)
    dmax = max(data)
  }

  if (type == "integer")
  {
    type = 6L

    if (has_na)
      NA_value = .Machine$integer.max
  }
  else if (type == "numeric")
  {
    type = 10L

    if (has_na)
      NA_value = .Machine$double.xmax
  }
  else if (type == "integer64")
  {
    type = 7L
    stop("integer64 not supported yet.")
  }
  else
    stop("Internal error. Process aborded")

  header = header_add_extrabytes_manual(header, name, desc, type, offset, scale, dmax, dmin, NA_value)

  return(header)
}

#' @export
#' @rdname header_add_extrabytes
header_add_extrabytes_manual = function(header, name, desc, type, offset = NULL, scale = NULL, max = NULL, min = NULL, NA_value = NULL)
{
  type = as.integer(type)

  if (type > 10L | type <= 0L)
    stop(paste0("Type ", type, " not supported."))

  if (nchar(name) > 32)
  {
    name = substr(name, 1, 31)
    message("Extrabytes name is longer than the 32 characters allowed and has been truncated.")
  }

  if (nchar(desc) > 32)
  {
    desc = substr(desc, 1, 31)
    message("Extrabytes description is longer than the 32 characters allowed and has been truncated.")
  }

  options = 0
  if (!is.null(NA_value))
    options = options + 2^0
  if (!is.null(min))
    options = options + 2^1
  if (!is.null(max))
    options = options + 2^2
  if (!is.null(scale))
    options = options + 2^3
  if (!is.null(offset))
    options = options + 2^4

  options = as.integer(options)

  description = list(reserved = 0,
                     data_type = type,
                     options = options,
                     name = name,
                     min = min,
                     max = max,
                     no_data = NA_value,
                     scale = scale,
                     offset = offset,
                     description = desc)

  description = description[!sapply(description, is.null)]

  header$`Variable Length Records`$Extra_Bytes$reserved = 43707L
  header$`Variable Length Records`$Extra_Bytes$`user ID` = "LASF_spec"
  header$`Variable Length Records`$Extra_Bytes$`record ID` = 4L
  header$`Variable Length Records`$Extra_Bytes$reserved = 0L
  header$`Variable Length Records`$Extra_Bytes$description = "rlas extra bytes"
  header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`[[name]] = description

  return(header)
}

#allowed_fields = c("X", "Y", "Z", "gpstime", "Intensity", "ReturnNumber", "NumberOfReturns", "ScanDirectionFlag", "EdgeOfFlightline", "Classification", "ScanAngle", "UserData", "PointSourceID", "R", "G", "B", "NIR")