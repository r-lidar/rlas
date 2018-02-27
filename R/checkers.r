check_data = function(data)
{
  if(is.null(data$X) | is.null(data$Y) | is.null(data$Z))
    stop("Missing coordinates in the data", call. = FALSE)
  if(!is.null(data$Intensity))
    stopifnot(is.integer(data$Intensity))
  if(!is.null(data$ReturnNumber))
    stopifnot(is.integer(data$ReturnNumber))
  if(!is.null(data$NumberOfReturns))
    stopifnot(is.integer(data$NumberOfReturns))
  if(!is.null(data$ScanDirectionFlag))
    stopifnot(is.integer(data$ScanDirectionFlag))
  if(!is.null(data$EdgeOfFlightline))
    stopifnot(is.integer(data$EdgeOfFlightline))
  if(!is.null(data$Classification))
    stopifnot(is.integer(data$Classification))
  if(!is.null(data$ScanAngle))
    stopifnot(is.integer(data$ScanAngle))
  if(!is.null(data$UserData))
    stopifnot(is.integer(data$UserData))
  if(!is.null(data$gpstime))
    stopifnot(is.numeric(data$gpstime))
  if(!is.null(data$PointSourceID))
    stopifnot(is.integer(data$PointSourceID))
  if(!is.null(data$R))
    stopifnot(is.integer(data$R))
  if(!is.null(data$G))
    stopifnot(is.integer(data$G))
  if(!is.null(data$B))
    stopifnot(is.integer(data$B))
  if(!is.null(data$NIR))
    stopifnot(is.integer(data$NIR))

  return(invisible())
}

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

  if (header[["Point Data Format ID"]] %in% c(4,5,6,7,9))
    stop(paste0("The point data format ", header[["Point Data Format ID"]], " is not supported yet."), call. = FALSE)

  if (!is.null(header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`))
  {
    for (extra_byte in header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`)
    {

      if (is.null(extra_byte[["options"]]))
        stop("'options' is missing in extra bytes description", call. = FALSE)

      if (is.null(extra_byte[["data_type"]]))
        stop("'data_type' is missing in extra bytes description", call. = FALSE)

      if (is.null(extra_byte[["name"]]))
        stop("'name' is missing in extra bytes description", call. = FALSE)

      if (is.null(extra_byte[["description"]]))
        stop("'description' is missing in extra bytes description", call. = FALSE)

      options = extra_byte[["options"]]
      options = as.integer(intToBits(options))[1:5]

      has_no_data = options[1] == 1L;
      has_min = options[2] == 1L;
      has_max = options[3] == 1L;
      has_scale = options[4] == 1L;
      has_offset = options[5] == 1L;

      if (is.null(extra_byte[["min"]]) & has_min)
        stop("'min' is missing in extra bytes description", call. = FALSE)

      if (is.null(extra_byte[["max"]]) & has_max)
        stop("max' is missing in extra bytes description", call. = FALSE)

      if (is.null(extra_byte[["scale"]]) & has_scale)
        stop("'scale' is missing in extra bytes description", call. = FALSE)

      if (is.null(extra_byte[["offset"]]) & has_offset)
        stop("'offset' is missing in extra bytes description", call. = FALSE)
    }
  }

  return(invisible())
}

check_data_header = function(header, data)
{
  format = header$`Point Data Format ID`
  fields = names(data)

  if("NIR" %in% fields & format != 8)
    warning("The data contains a 'NIR' field but point data format is not set to 8. 'NIR' will not be writen.")

  if("gpstime" %in% fields & !format %in% c(1,3,6,7,8))
    warning("The data contains a 'gpstime' field but point data format is not set to 1 or 3. 'gpstime' will not be writen.")

  if(any(c("R", "G", "B") %in% fields) & !format %in% c(2,3,8))
    warning("The data contains a 'RGB' field but point data format is not set to 2 or 3. 'RGB' will not be writen.")

  if (!is.null(header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`))
  {
    extrabytes = names(header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`)

    if (!all(extrabytes %in% names(data)))
      stop("The header describes extra bytes attributes that are not in the data.", call. = FALSE)
  }

  return(invisible())
}

check_output_file = function(file)
{
  islas = tools::file_ext(file) %in% c("las", "laz")

  if(length(file) > 1)
    stop("Write only one file at a time.", call. = F)

  if(!islas)
    stop("File not supported. Extension should be 'las' or 'laz'", call. = F)
}

check_file = function(file)
{
  valid = file.exists(file)
  islas = tools::file_ext(file) %in% c("las", "laz", "LAS", "LAZ")
  file = normalizePath(file)

  if (!all(valid))
    stop("File not found", call. = F)

  if (!all(islas))
    stop("File not supported", call. = F)
}

check_filter = function(filter)
{
  if (!is.character(filter))
    stop("Incorrect argument 'filter'", call. = F)
}