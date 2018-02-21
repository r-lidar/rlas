#' Create a header from data
#'
#' Create a header for a las file from data. The header is not necessarily entierely correct regarding
#' specification but is expected to be enought to be passed in \link{write.las} since many redoundant
#' information are computed at writing time anyway.
#'
#' @param data data.frame or data.table
#' @param extra_bytes Labelled list. Specify which column must be saved as extra bytes attribute and
#' provide a description to this data. The
#'
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
#' extra = list(treeID = "Unique ID for segmented trees")
#'
#' lasheader = make_header(lasdata, extra)
make_header = function(data, extra_bytes = NULL)
{
  fields = names(data)

  if(!is.null(extra_bytes))
  {
    if(!all(names(extra_bytes) %in% fields))
      stop("extra bytes names do not match with the data", call. = FALSE)
  }

  header = list()
  header$`File Signature` = "LASF"
  header$`File Source ID` = 0L
  header$`Global Encoding` = 0L
  header$`Project ID - GUID` = 0
  header$`Version Major` = 1L
  header$`Version Minor` = 2L
  header$`System Identifier` = "rlas R package"
  header$`Generating Software` = "rlas R package"
  header$`File Creation Day of Year` = as.numeric(strftime(Sys.time(), format = "%j"))
  header$`File Creation Year` = as.numeric(strftime(Sys.time(), format = "%Y"))
  header$`Header Size` = 227
  header$`Offset to point data` = 227
  header$`Number of point records` = dim(data)[1]
  header$`Min X` = min(data$X)
  header$`Min Y` = min(data$Y)
  header$`Min Z` = min(data$Z)
  header$`Max X` = max(data$X)
  header$`Max Y` = max(data$Y)
  header$`Max Z` = max(data$Z)
  header$`X offset` = header$`Min X`
  header$`Y offset` = header$`Min Y`
  header$`Z offset` = header$`Min Z`
  header$`X scale factor` = 0.01
  header$`Y scale factor` = 0.01
  header$`Z scale factor` = 0.01

  if("ReturnNumber" %in% fields) {
    number_of <- as.list(table(data$ReturnNumber))
    header$`Number of 1st return` <- number_of$`1`
    header$`Number of 2nd return` <- number_of$`2`
    header$`Number of 3rd return` <- number_of$`3`
    header$`Number of 4th return` <- number_of$`4`
    header$`Number of 5th return` <- number_of$`5`
  }

  if("NIR" %in% fields) { # format 8
    header$`Point Data Format ID` = 8
    header$`Point Data Record Length` = 38
  }
  else if("gpstime" %in% fields) { # format 1, 3, 6, 7
    if(all(c("R", "G", "B") %in% fields)) { # format 3 (6 not supported)
      header$`Point Data Format ID` = 3
      header$`Point Data Record Length` = 34
    }
    else { # format 1 (7 not supported)
      header$`Point Data Format ID` = 1
      header$`Point Data Record Length` = 28
    }
  }
  else { # format 0 or 2
    if(all(c("R", "G", "B") %in% fields)) {
      header$`Point Data Format ID` = 2
      header$`Point Data Record Length` = 26
    }
    else {
      header$`Point Data Format ID` = 0
      header$`Point Data Record Length` = 20
    }
  }

  header$`Variable Length Records` = list()

  if (length(extra_bytes) > 0)
  {
    for(i in seq_along(extra_bytes))
    {
      name = names(extra_bytes[i])
      desc = extra_bytes[[i]]
      extra_byte = data[[name]]

      header = add_extra_bytes(header, extra_byte, name, desc)
    }
  }

  return(header)
}

add_extra_bytes = function(header, data, name, description)
{
 type = class(data)
 dmin = min(data)
 dmax = max(data)
 offset = dmin

 if (type == "integer")
 {
   type = 7
   scale = NULL
   options = strtoi("10110", base = 2L)
 }
 else if (type == "numeric")
 {
   type = 7
   scale = 0.01
   options = strtoi("11110", base = 2L)
 }
 else
   stop("Internal error. Process aborded")

 description = list(reserved = 0,
                    data_type = type,
                    options = options,
                    name = name,
                    min = dmin,
                    max = dmax,
                    scale = scale,
                    offset = offset,
                    description = description)

 header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`[[name]] = description

 return(header)
}

#allowed_fields = c("X", "Y", "Z", "gpstime", "Intensity", "ReturnNumber", "NumberOfReturns", "ScanDirectionFlag", "EdgeOfFlightline", "Classification", "ScanAngle", "UserData", "PointSourceID", "R", "G", "B", "NIR")