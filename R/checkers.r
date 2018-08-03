#' Check if data and headers are correct
#'
#' las files are normalized files. The \code{check_*} functions test if the data is valid,
#' if the header is valid or if the header is in accordance with the data.
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
    stop("Invalid data: missing coordinates X Y Z", call. = FALSE)

  if(!is.double(data$X))
    stop("Invalid data: X is not a double.", call. = FALSE)

  if(!is.double(data$Y))
    stop("Invalid data: Y is not a double.", call. = FALSE)

  if(!is.double(data$Z))
    stop("Invalid data: Z is not a double.", call. = FALSE)

  if (!is.null(data$Intensity))
  {
    if(!is.integer(data$Intensity))
      stop("Invalid data: Intensity is not an integer", call. = FALSE)
  }

  if (!is.null(data$ReturnNumber))
  {
    if(!is.integer(data$ReturnNumber))
      stop("Invalid data: ReturnNumber is not an integer", call. = FALSE)
  }

  if(!is.null(data$NumberOfReturns))
  {
    if(!is.integer(data$NumberOfReturns))
      stop("Invalid data: NumberOfReturns is not an integer", call. = FALSE)
  }

  if (!is.null(data$ScanDirectionFlag))
  {
    if(!is.integer(data$ScanDirectionFlag))
      stop("Invalid data: ScanDirectionFlag is not an integer", call. = FALSE)
  }

  if (!is.null(data$EdgeOfFlightline))
  {
    if(!is.integer(data$EdgeOfFlightline))
      stop("Invalid data: EdgeOfFlightline is not an integer", call. = FALSE)
  }

  if (!is.null(data$Classification))
  {
    if(!is.integer(data$Classification))
      stop("Invalid data: Classification is not an integer", call. = FALSE)
  }

  if(!is.null(data$Synthetic_flag))
  {
    if(!is.logical(data$Synthetic_flag))
      stop("Invalid data: Synthetic_flag is not a logical", call. = FALSE)
  }

  if (!is.null(data$Keypoint_flag))
  {
    if(!is.logical(data$Keypoint_flag))
      stop("Invalid data: Keypoint_flag is not a logical", call. = FALSE)
  }

  if (!is.null(data$Withheld_flag))
  {
    if(!is.logical(data$Withheld_flag))
      stop("Invalid data: Withheld_flag is not a logical", call. = FALSE)
  }

  if (!is.null(data$ScanAngle))
  {
    if(!is.integer(data$ScanAngle))
      stop("Invalid data: ScanAngle is not an integer", call. = FALSE)
  }

  if (!is.null(data$UserData))
  {
    if(!is.integer(data$UserData))
      stop("Invalid data: UserData is not an integer", call. = FALSE)
  }

  if (!is.null(data$gpstime))
  {
    if(!is.numeric(data$gpstime))
      stop("Invalid data: gpstime is not a double", call. = FALSE)
  }

  if (!is.null(data$PointSourceID))
  {
    if(!is.integer(data$PointSourceID))
      stop("Invalid data: PointSourceID is not an integer", call. = FALSE)
  }

  if ("R" %in% names(data))
  {
    if(!is.integer(data$R))
      stop("Invalid data: R is not an integer", call. = FALSE)
  }

  if ("G" %in% names(data))
  {
    if(!is.integer(data$G))
      stop("Invalid data: G is not an integer", call. = FALSE)
  }

  if ("B" %in% names(data))
  {
    if(!is.integer(data$B))
      stop("Invalid data: B is not an integer", call. = FALSE)
  }

  if (!is.null(data$NIR))
  {
    if(!is.integer(data$NIR))
      stop("Invalid data: NIR is not an integer", call. = FALSE)
  }

  if (!is.null(data$ReturnNumber) & !is.null(data$NumberOfReturns))
  {
    s = sum(data$ReturnNumber > data$NumberOfReturns)

    if (s > 0)
      warning(paste0("Invalid data: ", s, " points with a 'return number' greater than the 'number of returns'."), call. = FALSE)
  }

  if(!is.null(data$ReturnNumber))
  {
    class0 = fast_countequal(data$ReturnNumber, 0L)

    if(class0 > 0)
      warning(paste0("Invalid data: ", class0, " points with a return number equal to 0 found."), call. = FALSE)
  }

  if(!is.null(data$NumberOfReturns))
  {
    class0 = fast_countequal(data$NumberOfReturns, 0L)

    if(class0 > 0)
      warning(paste0("Invalid data: ", class0, " points with a number of returns equal to 0 found."), call. = FALSE)
  }

  # Should maybe belong in an check_extra function
  negvalues = fast_countbelow(data$Z, 0)

  if(negvalues > 0)
    message(paste0(negvalues, " points below 0 found."))

  if(!is.null(data$Classification))
  {
    class0 = fast_countequal(data$Classification, 0L)

    if(class0 > 0 & class0 < nrow(data))
      message(paste0(class0, " unclassified points found."))
  }

  maxR <- maxG <- maxB <- 2^16
  if ("R" %in% names(data)) maxR = max(data$R)
  if ("G" %in% names(data)) maxG = max(data$G)
  if ("B" %in% names(data)) maxB = max(data$B)

  if ((maxR > 0 & maxR <= 255) | (maxG > 0 & maxG <= 255) | (maxB > 0 & maxB <= 255))
    warning("Invalid data: RGB colors are recorded on 8 bits instead of 16 bits.", call. = FALSE)

  return(invisible())
}

#' @export
#' @rdname check
check_header = function(header)
{
  if (is.null(header[["X offset"]]) | is.null(header[["Y offset"]]) | is.null(header[["Z offset"]]))
    stop("Invalid header: Offsetting not defined in the header", call. = FALSE)

  if (is.null(header[["X scale factor"]]) | is.null(header[["Y scale factor"]]) | is.null(header[["Z scale factor"]]))
    stop("Invalid header: Scaling not defined in the header", call. = FALSE)

  if (is.null(header[["File Source ID"]]))
    stop("Invalid header: File source ID not defined in the header", call. = FALSE)

  if (is.null(header[["Version Major"]]))
    stop("Invalid header: Version major not defined in the header", call. = FALSE)

  if (is.null(header[["Version Minor"]]))
    stop("Invalid header: Version minor not defined in the header", call. = FALSE)

  if (is.null(header[["File Creation Day of Year"]]))
    stop("Invalid header: File Creation Day of Year not defined in the header", call. = FALSE)

  if (is.null(header[["File Creation Year"]]))
    stop("Invalid header: File Creation Year not defined in the header", call. = FALSE)

  if (is.null(header[["Point Data Format ID"]]))
    stop("Invalid header: Point Data Format ID not defined in the header", call. = FALSE)

  if (!is.null(header[["X scale factor"]]) & !is.null(header[["Y scale factor"]]) & !is.null(header[["Z scale factor"]]))
  {
    s = c(1,10,100,1000,10000)
    valid = c(1/s, 0.5/s, 0.25/s)

    if (!header[["X scale factor"]] %in% valid)
      warning(paste0("Invalid header: X scale factor should be factor ten of 0.1 or 0.5 or 0.25 and not ", header[["X scale factor"]]), call. = FALSE)

     if (!header[["Y scale factor"]] %in% valid)
      warning(paste0("Invalid header: Y scale factor should be factor ten of 0.1 or 0.5 or 0.25 and not ", header[["Y scale factor"]]), call. = FALSE)

    if (!header[["Z scale factor"]] %in% valid)
      warning(paste0("Invalid header: Z scale factor should be factor ten of 0.1 or 0.5 or 0.25 and not ", header[["Z scale factor"]]), call. = FALSE)
  }
  else
    stop("Invalid header: scale factors not defined in the header", call. = FALSE)

  if (header[["Point Data Format ID"]] %in% c(4,5,9,10))
    stop(paste0("Invalid header: The point data format ", header[["Point Data Format ID"]], " is not supported yet."))

  if (!is.null(header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`))
  {
    for (extra_byte in header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`)
    {
      if (is.null(extra_byte[["options"]]))
        stop("Invalid header: 'options' is missing in extra bytes description")

      if (is.null(extra_byte[["data_type"]]))
        stop("Invalid header: 'data_type' is missing in extra bytes description")

      if (is.null(extra_byte[["name"]]))
        stop("Invalid header: 'name' is missing in extra bytes description")

      if (is.null(extra_byte[["description"]]))
        stop("Invalid header: 'description' is missing in extra bytes description")

      options = extra_byte[["options"]]
      options = as.integer(intToBits(options))[1:5]

      has_no_data = options[1] == 1L;
      has_min = options[2] == 1L;
      has_max = options[3] == 1L;
      has_scale = options[4] == 1L;
      has_offset = options[5] == 1L;

      if (is.null(extra_byte[["min"]]) & has_min)
        stop("Invalid header: 'min' is missing in extra bytes description")

      if (is.null(extra_byte[["max"]]) & has_max)
        stop("Invalid header: max' is missing in extra bytes description")

      if (is.null(extra_byte[["scale"]]) & has_scale)
        stop("Invalid header: 'scale' is missing in extra bytes description")

      if (is.null(extra_byte[["offset"]]) & has_offset)
        stop("Invalid header: 'offset' is missing in extra bytes description")
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
    warning("Invalid file: the data contains a 'NIR' field but point data format is not set to 8.", call. = FALSE)

  if("gpstime" %in% fields & !format %in% c(1,3,6,7,8))
    warning("Invalid file: the data contains a 'gpstime' field but point data format is not set to 1, 3, 6, 7 or 8.", call. = FALSE)

  if(any(c("R", "G", "B") %in% fields) & !format %in% c(2,3,8))
    warning("Invalid file: the data contains a 'RGB' field but point data format is not set to 2, 3 or 8.", call. = FALSE)

  if (max(data$X) > header$`Max X` | max(data$Y) > header$`Max Y` | min(data$X) < header$`Min X` | min(data$Y) < header$`Min Y`)
    warning("Invalid file: some points are outside the bounding box defined by the header", call. = FALSE)

  if (max(data$Z) > header$`Max Z` |  min(data$Z) < header$`Min Z`)
    warning("Invalid file: some points are outside the elevation range defined by the header", call. = FALSE)

  if(hard)
  {
    if(dim(data)[1] != h["Number of point records"])
      warning(paste0("Invalid file: header states the file contains ", h["Number of point records"], " points but ", nrow(data), " were found."), call. = FALSE)

    if("ReturnNumber" %in% fields)
    {
      number_of <- fast_table(data$ReturnNumber, 5L)

      if(h["Number of 1st return"] != number_of[1])
        warning(paste0("Invalid file: the header states the file contains ", h["Number of 1st return"], " 1st returns but ", number_of[1], " were found."), call. = FALSE)

      if(h["Number of 2nd return"] != number_of[2])
        warning(paste0("Invalid file: the header states the file contains ", h["Number of 2nd return"], " 2nd returns but ", number_of[2], " were found."), call. = FALSE)

      if(h["Number of 3rd return"] != number_of[3])
        warning(paste0("Invalid file: the header states the file contains ", h["Number of 3rd return"], " 3rd returns but ", number_of[3], " were found."), call. = FALSE)

      if(h["Number of 4th return"] != number_of[4])
        warning(paste0("Invalid file: the header states the file contains ", h["Number of 4th return"], " 4th returns but ", number_of[4], " were found."), call. = FALSE)

      if(h["Number of 5th return"] != number_of[5])
        warning(paste0("Invalid file: the header states the file contains ", h["Number of 5th return"], " 5th returns but ", number_of[5], " were found."), call. = FALSE)
    }
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
    stop("File not found.")

  if (!all(islas))
    stop("File not supported.")
}

check_filter = function(filter)
{
  if (!is.character(filter) & length(filter) > 1)
    stop("Incorrect argument 'filter'. A string is expected.")
}