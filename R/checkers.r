#' Check if data and headers are correct
#'
#' las files are normalized files. The \code{check_*} functions test whether either the dataset is valid, the header is valid, or if the header is in accordance with the data.
#'
#' @param data a data.frame or a data.table containing a point cloud
#' @param header a list containing the header of a las file
#' @param ... internal use only
#'
#' @export
#' @rdname check
check_data = function(data)
{
  if(is.null(data$X) | is.null(data$Y) | is.null(data$Z))
    stop("Missing coordinates in the data")

  stopifnot(is.double(data$X))
  stopifnot(is.double(data$Y))
  stopifnot(is.double(data$X))

  if (!is.null(data$Intensity))
    stopifnot(is.integer(data$Intensity))

  if (!is.null(data$ReturnNumber))
    stopifnot(is.integer(data$ReturnNumber))

  if(!is.null(data$NumberOfReturns))
    stopifnot(is.integer(data$NumberOfReturns))

  if (!is.null(data$ScanDirectionFlag))
    stopifnot(is.integer(data$ScanDirectionFlag))

  if (!is.null(data$EdgeOfFlightline))
    stopifnot(is.integer(data$EdgeOfFlightline))

  if (!is.null(data$Classification))
    stopifnot(is.integer(data$Classification))

  if(!is.null(data$Synthetic_flag))
    stopifnot(is.logical(data$Synthetic_flag))

  if (!is.null(data$Keypoint_flag))
    stopifnot(is.logical(data$Keypoint_flag))

  if (!is.null(data$Withheld_flag))
    stopifnot(is.logical(data$Withheld_flag))

  if (!is.null(data$ScanAngle))
    stopifnot(is.integer(data$ScanAngle))

  if (!is.null(data$UserData))
    stopifnot(is.integer(data$UserData))

  if (!is.null(data$gpstime))
    stopifnot(is.numeric(data$gpstime))

  if (!is.null(data$PointSourceID))
    stopifnot(is.integer(data$PointSourceID))

  if (!is.null(data$R))
    stopifnot(is.integer(data$R))

  if (!is.null(data$G))
    stopifnot(is.integer(data$G))

  if (!is.null(data$B))
    stopifnot(is.integer(data$B))

  if (!is.null(data$NIR))
    stopifnot(is.integer(data$NIR))

  if (!is.null(data$ReturnNumber) & !is.null(data$NumberOfReturns))
  {
    s = sum(data$ReturnNumber > data$NumberOfReturns)

    if (s > 0)
      warning(paste0("Invalid data: ", s, " points with a 'return number' greater than the 'number of returns'."), call. = FALSE)
  }

  return(invisible())
}

#' @export
#' @rdname check
check_header = function(header)
{
  if (is.null(header[["X offset"]]) | is.null(header[["Y offset"]]) | is.null(header[["Z offset"]]))
    stop("Offsetting not defined in the header", call. = FALSE)

  if (is.null(header[["X scale factor"]]) | is.null(header[["Y scale factor"]]) | is.null(header[["Z scale factor"]]))
    stop("Scaling not defined in the header", call. = FALSE)

  if (is.null(header[["File Source ID"]]))
    stop("File source ID not defined in the header", call. = FALSE)

  if (is.null(header[["Version Major"]]))
    stop("Version major not defined in the header", call. = FALSE)

  if (is.null(header[["Version Minor"]]))
    stop("Version minor not defined in the header", call. = FALSE)

  if (is.null(header[["File Creation Day of Year"]]))
    stop("File Creation Day of Year not defined in the header", call. = FALSE)

  if (is.null(header[["File Creation Year"]]))
    stop("File Creation Year not defined in the header", call. = FALSE)

  if (is.null(header[["Point Data Format ID"]]))
    stop("Point Data Format ID not defined in the header", call. = FALSE)

  if (header[["Point Data Format ID"]] %in% c(4,5,9,10))
    stop(paste0("The point data format ", header[["Point Data Format ID"]], " is not supported yet."))

  if (!is.null(header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`))
  {
    for (extra_byte in header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`)
    {
      if (is.null(extra_byte[["options"]]))
        stop("'options' is missing in extra bytes description")

      if (is.null(extra_byte[["data_type"]]))
        stop("'data_type' is missing in extra bytes description")

      if (is.null(extra_byte[["name"]]))
        stop("'name' is missing in extra bytes description")

      if (is.null(extra_byte[["description"]]))
        stop("'description' is missing in extra bytes description")

      options = extra_byte[["options"]]
      options = as.integer(intToBits(options))[1:5]

      has_no_data = options[1] == 1L;
      has_min = options[2] == 1L;
      has_max = options[3] == 1L;
      has_scale = options[4] == 1L;
      has_offset = options[5] == 1L;

      if (is.null(extra_byte[["min"]]) & has_min)
        stop("'min' is missing in extra bytes description")

      if (is.null(extra_byte[["max"]]) & has_max)
        stop("max' is missing in extra bytes description")

      if (is.null(extra_byte[["scale"]]) & has_scale)
        stop("'scale' is missing in extra bytes description")

      if (is.null(extra_byte[["offset"]]) & has_offset)
        stop("'offset' is missing in extra bytes description")
    }
  }

  return(invisible())
}

#' @export
#' @rdname check
check_data_vs_header = function(header, data, ...)
{
  p = list(...)
  hard = TRUE

  if (!is.null(p$hard))
    hard = p$hard

  h = header
  format = header$`Point Data Format ID`
  fields = names(data)

  if("NIR" %in% fields & format != 8)
    warning("The data contains a 'NIR' field but point data format is not set to 8.", call. = FALSE)

  if("gpstime" %in% fields & !format %in% c(1,3,6,7,8))
    warning("The data contains a 'gpstime' field but point data format is not set to 1, 3, 6, 7 or 8.", call. = FALSE)

  if(any(c("R", "G", "B") %in% fields) & !format %in% c(2,3,8))
    warning("The data contains a 'RGB' field but point data format is not set to 2, 3 or 8.", call. = FALSE)

  if(hard)
  {
    if(dim(data)[1] != h["Number of point records"])
      warning(paste0("Invalid file: header states the file contains ", h["Number of point records"], " points but ", nrow(data), " were found."), call. = FALSE)

    if("ReturnNumber" %in% fields)
    {
      number_of <- fast_table(data$ReturnNumber, 5L)

      if(h["Number of 1st return"] != number_of[1])
        warning(paste0("Invalid file: header states the file contains ", h["Number of 1st return"], " 1st returns but ", number_of[1], " were found."), call. = FALSE)

      if(h["Number of 2nd return"] != number_of[2])
        warning(paste0("Invalid file: header states the file contains ", h["Number of 2nd return"], " 2nd returns but ", number_of[2], " were found."), call. = FALSE)

      if(h["Number of 3rd return"] != number_of[3])
        warning(paste0("Invalid file: header states the file contains ", h["Number of 3rd return"], " 3rd returns but ", number_of[3], " were found."), call. = FALSE)

      if(h["Number of 4th return"] != number_of[4])
        warning(paste0("Invalid file: header states the file contains ", h["Number of 4th return"], " 4th returns but ", number_of[4], " were found."), call. = FALSE)

      if(h["Number of 5th return"] != number_of[5])
        warning(paste0("Invalid file: header states the file contains ", h["Number of 5th return"], " 5th returns but ", number_of[5], " were found."), call. = FALSE)
    }
  }

  negvalues = fast_countbelow(data$Z, 0)

  if(negvalues > 0)
    message(paste0(negvalues, " points below 0 found."))

  if("Classification" %in% fields)
  {
    class0 = fast_countequal(data$Classification, 0L)

    if(class0 > 0 & class0 < nrow(data))
      message(paste0(class0, " unclassified points found."))
  }

  if("ReturnNumber" %in% fields)
  {
    class0 = fast_countequal(data$ReturnNumber, 0L)

    if(class0 > 0)
      warning(paste0("Dataset is invalid: ", class0, " points with a return number of 0 found."), call. = FALSE)
  }

  if (!is.null(header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`))
  {
    extrabytes = names(header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`)

    if (!all(extrabytes %in% names(data)))
      stop("The header describes extra bytes attributes that are not in the data.")
  }

  return(invisible())
}

check_output_file = function(file)
{
  islas = tools::file_ext(file) %in% c("las", "laz")

  if(length(file) > 1)
    stop("Write only one file at a time.")

  if(!islas)
    stop("File not supported. Extension should be 'las' or 'laz'")
}

check_file = function(file)
{
  valid = file.exists(file)
  islas = tools::file_ext(file) %in% c("las", "laz", "LAS", "LAZ")
  file = normalizePath(file)

  if (!all(valid))
    stop("File not found")

  if (!all(islas))
    stop("File not supported")
}

check_filter = function(filter)
{
  if (!is.character(filter))
    stop("Incorrect argument 'filter'")
}