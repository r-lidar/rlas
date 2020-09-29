#' Public Header Block Tools
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
#' @family header_tools
#' @return A list containing the metadata required to write a las file.
#' @export
#'
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
#' @name public_header_block_tools
#' @rdname public_header_block_tools
header_create = function(data)
{
  fields = names(data)

  if (nrow(data) > 0L)
  {
    npts = nrow(data)
    minx = min(data$X)
    miny = min(data$Y)
    minz = min(data$Z)
    maxx = max(data$X)
    maxy = max(data$Y)
    maxz = max(data$Z)
  }
  else
  {
    npts = 0L
    minx = 0
    miny = 0
    minz = 0
    maxx = 0
    maxy = 0
    maxz = 0
  }

  header = list()
  header[["File Signature"]] = "LASF"
  header[["File Source ID"]] = 0L
  header[["Global Encoding"]] = list(`GPS Time Type` = TRUE,
                                     `Waveform Data Packets Internal` = FALSE,
                                     `Waveform Data Packets External` = FALSE,
                                     `Synthetic Return Numbers` = FALSE,
                                      WKT = FALSE,
                                     `Aggregate Model` = FALSE)
  header[["Project ID - GUID"]] = "00000000-0000-0000-0000-000000000000"
  header[["Version Major"]] = 1L
  header[["Version Minor"]] = 2L
  header[["System Identifier"]] = "rlas R package"
  header[["Generating Software"]] = "rlas R package"
  header[["File Creation Day of Year"]] = as.numeric(strftime(Sys.time(), format = "%j"))
  header[["File Creation Year"]] = as.numeric(strftime(Sys.time(), format = "%Y"))
  header[["Header Size"]] = 227
  header[["Offset to point data"]] = 227
  header[["Number of point records"]] = npts
  header[["Min X"]] = minx
  header[["Min Y"]] = miny
  header[["Min Z"]] = minz
  header[["Max X"]] = maxx
  header[["Max Y"]] = maxy
  header[["Max Z"]] = maxz
  header[["X offset"]] = header[["Min X"]]
  header[["Y offset"]] = header[["Min Y"]]
  header[["Z offset"]] = header[["Min Z"]]
  header[["X scale factor"]] = 0.01
  header[["Y scale factor"]] = 0.01
  header[["Z scale factor"]] = 0.01

  if ("ReturnNumber" %in% fields)
    header[["Number of points by return"]] <- tabulate(data$ReturnNumber, 5L)
  else
    header[["Number of points by return"]] <- rep(0L,5)

  header[["Point Data Format ID"]] <- guess_las_format(data)
  header[["Point Data Record Length"]] <- get_data_record_length(header[["Point Data Format ID"]])

  header[["Variable Length Records"]] = list()

  return(header)
}

#' @export
#' @rdname public_header_block_tools
header_update = function(header, data)
{
  fields = names(data)

  if (nrow(data) > 0L)
  {
    npts = nrow(data)
    minx = min(data$X)
    miny = min(data$Y)
    minz = min(data$Z)
    maxx = max(data$X)
    maxy = max(data$Y)
    maxz = max(data$Z)
  }
  else
  {
    npts = 0L
    minx = 0
    miny = 0
    minz = 0
    maxx = 0
    maxy = 0
    maxz = 0
  }

  if ("ReturnNumber" %in% fields)
  {
    n <- if (header[["Version Minor"]] < 4) 5L else 15L
    header[["Number of points by return"]] <- tabulate(data$ReturnNumber, n)
  }

  header[["Number of point records"]] = npts
  header[["Min X"]] = minx
  header[["Min Y"]] = miny
  header[["Min Z"]] = minz
  header[["Max X"]] = maxx
  header[["Max Y"]] = maxy
  header[["Max Z"]] = maxz

  return(header)
}

#' Extra Bytes Attributes Tools
#'
#' Functions that update a header to describe Extra Bytes Attributes according to the
#' \href{http://www.asprs.org/wp-content/uploads/2019/07/LAS_1_4_r15.pdf}{LAS specifications}
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
#' @family header_tools
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
#' @name extra_bytes_attribute_tools
#' @rdname extra_bytes_attribute_tools
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
#' @rdname extra_bytes_attribute_tools
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

#' Coordinate Reference System Tools
#'
#' Functions that update a header to describe coordinates reference system according to the
#' \href{http://www.asprs.org/wp-content/uploads/2019/07/LAS_1_4_r15.pdf}{LAS specifications}
#'
#' @param header list
#' @param epsg integer. An EPSG code
#' @param WKT string. A string of an WKT OGC CS
#' @family header_tools
#' @export
#' @name crs_tools
#' @rdname crs_tools
header_get_epsg = function(header)
{
  pos <- where_is_epsg(header)

  if (pos == 0)
    return(0)
  else
    return(header[["Variable Length Records"]][["GeoKeyDirectoryTag"]][["tags"]][[pos]][["value offset"]])
}

#' @export
#' @rdname crs_tools
header_set_epsg = function(header, epsg)
{
  pos <- where_is_epsg(header)

  if (pos == 0)
  {
    if (is.null(header[["Variable Length Records"]][["GeoKeyDirectoryTag"]]))
    {
      header[["Variable Length Records"]][["GeoKeyDirectoryTag"]][["reserved"]]            <- 0
      header[["Variable Length Records"]][["GeoKeyDirectoryTag"]][["user ID"]]             <- "LASF_Projection"
      header[["Variable Length Records"]][["GeoKeyDirectoryTag"]][["record ID"]]           <- 34735
      header[["Variable Length Records"]][["GeoKeyDirectoryTag"]][["length after header"]] <- 40
      header[["Variable Length Records"]][["GeoKeyDirectoryTag"]][["description"]]         <- "Geo Key Directory Tag"
      header[["Variable Length Records"]][["GeoKeyDirectoryTag"]][["tags"]]                <- vector("list", 1)
      pos <- 1
    }
    else
    {
      pos <- length(header[["Variable Length Records"]][["GeoKeyDirectoryTag"]][["tags"]]) + 1
    }
  }

  header[["Variable Length Records"]][["GeoKeyDirectoryTag"]][["tags"]][[pos]] <- list(key = 3072L, `tiff tag location` = 0L, count = 1L, `value offset` = as.integer(epsg))
  return(header)
}

#' @export
#' @rdname crs_tools
header_get_wktcs = function(header)
{
  wkt = header[["Variable Length Records"]][["WKT OGC CS"]][["WKT OGC COORDINATE SYSTEM"]]

  if (is.null(wkt))
    return("")
  else
    return(wkt)
}

#' @export
#' @rdname crs_tools
header_set_wktcs = function(header, WKT)
{
  if (is.null(header[["Variable Length Records"]]))
    header[["Variable Length Records"]] <- list()

  if (is.null(header[["Variable Length Records"]][["WKT OGC CS"]]))
    header[["Variable Length Records"]][["WKT OGC CS"]] <- list()


  header[["Variable Length Records"]][["WKT OGC CS"]][["reserved"]]                  <- 43707L
  header[["Variable Length Records"]][["WKT OGC CS"]][["user ID"]]                   <-  "LASF_Projection"
  header[["Variable Length Records"]][["WKT OGC CS"]][["record ID"]]                 <- 2112
  header[["Variable Length Records"]][["WKT OGC CS"]][["description"]]               <- "WKT Information"
  header[["Variable Length Records"]][["WKT OGC CS"]][["WKT OGC COORDINATE SYSTEM"]] <- WKT
  header[["Global Encoding"]][["WKT"]] <- TRUE
  return(header)
}

where_is_epsg = function(header)
{
  tags = header[["Variable Length Records"]][["GeoKeyDirectoryTag"]][["tags"]]

  for (i in seq_along(tags))
  {
    if (tags[[i]]$key == 3072)
      return(i)
  }

  return(0)
}

guess_las_format <- function(data)
{
  fields <- names(data)

  if ("NIR" %in% fields) # format 8 or 10
    return(8L) # Format 10 is not supported

  if ("gpstime" %in% fields) # format 1, 3:10
  {
    if (all(c("R", "G", "B") %in% fields))  # format 3, 5, 7, 8
    {
     if ("ScanAngle" %in% fields)
        return(7L) # It is not 8 because NIR has already been tested
      else
        return(3L)
    }
    else # format 1
      return(1L)
  }
  else # format 0 or 2
  {
    if (all(c("R", "G", "B") %in% fields))
      return(2L)
    else
      return(0L)
  }
}

get_data_record_length <- function(format)
{
  bytes <- c(20L, 28L, 26L, 34L, 57L, 63L, 30L, 36L, 38L, 59L, 67L)
  return(bytes[format + 1])
}


#allowed_fields = c("X", "Y", "Z", "gpstime", "Intensity", "ReturnNumber", "NumberOfReturns", "ScanDirectionFlag", "EdgeOfFlightline", "Classification", "ScanAngle", "UserData", "PointSourceID", "R", "G", "B", "NIR")