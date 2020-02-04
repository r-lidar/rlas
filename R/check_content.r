#' Check if data and headers respect the LAS specification
#'
#' las files are normalized files. These functions perform tests of compliance with LAS specification.
#' \itemize{
#' \item \code{check_las_validity} tests if the data and the header contain information that cannot be
#' written into a las file. For example it tests is the intensities do not exeed 65535. It throws an
#' error for each deviance to the specification. It is useful for testing if modified R objects are
#' still valid.
#' \item \code{check_las_compliance} test if the data and the header contain information
#' that can be written into a las file but are invalid with repect of the specification. For example it
#' test if the RGB colors are recoded on 16 bits. It is possible to store colors recorded on 8 bits
#' (0 to 255) but it is not correct to do that. It throws a warning for each deviance to the specification.
#' It is useful for testing if the data read from a file are correct.
#' }
#
#'
#'
#' @param data a data.frame or a data.table containing a point cloud
#' @param header a list containing the header of a las file
#'
#' @export
#' @rdname check
check_las_validity = function(header, data)
{
  is_defined_offsets(header, "stop")
  is_defined_scalefactors(header, "stop")
  is_defined_filesourceid(header, "stop")
  is_defined_version(header, "stop")
  is_defined_globalencoding(header, "stop")
  is_defined_date(header, "stop")
  is_defined_pointformat(header, "stop")

  is_defined_coordinates(data, "stop")

  is_valid_offsets(header, "stop")
  is_valid_scalefactors(header, "stop")
  is_valid_globalencoding(header, "stop")
  is_valid_date(header, "stop")
  is_valid_pointformat(header, "stop")
  is_valid_extrabytes(header, "stop")
  is_valid_filesourceid(header, "stop")

  is_valid_XYZ(data, "stop")
  is_valid_Intensity(data, "stop")
  is_valid_ReturnNumber(header, data, "stop")
  is_valid_NumberOfReturns(header, data, "stop")
  is_valid_ScanDirectionFlag(data, "stop")
  is_valid_EdgeOfFlightline(data, "stop")
  is_valid_Classification(data, header, "stop")
  is_valid_ScannerChannel(data, "stop")
  is_valid_SyntheticFlag(data, "stop")
  is_valid_KeypointFlag(data, "stop")
  is_valid_WithheldFlag(data, "stop")
  is_valid_OverlapFlag(data, "stop")
  is_valid_ScanAngleRank(data, "stop")
  is_valid_ScanAngle(data, "stop")
  is_valid_UserData(data, "stop")
  is_valid_gpstime(data, "stop")
  is_valid_PointSourceID(data, "stop")
  is_valid_RGB(data, "stop")
  is_valid_NIR(data, "stop")

  is_NIR_in_valid_format(header, data, "warning")
  is_gpstime_in_valid_format(header, data, "warning")
  is_RGB_in_valid_format(header, data, "warning")
  is_ScanAngle_in_valid_format(header, data, "warning")
  is_ScannerChannel_in_valid_format(header, data, "warning")
  is_extrabytes_in_accordance_with_data(header, data, "stop")

  return(invisible())
}

#' @export
#' @rdname check
check_las_compliance = function(header, data)
{
  is_defined_offsets(header, "stop")
  is_defined_scalefactors(header, "stop")
  is_defined_filesourceid(header, "stop")
  is_defined_version(header, "stop")
  is_defined_globalencoding(header, "stop")
  is_defined_date(header, "stop")

  is_defined_coordinates(data, "stop")

  is_compliant_ReturnNumber(data, "warning")
  is_compliant_NumberOfReturns(data, "warning")
  is_compliant_RGB(data, "warning")
  is_compliant_ScanAngle(data, "warning")
  is_compliant_ScanAngleRank(data, "warning")
  is_compliant_ReturnNumber_vs_NumberOfReturns(data, "warning")
  is_XY_larger_than_bbox(header, data, "warning")
  is_valid_scalefactors(header, behavior = "warning")
  is_number_of_points_in_accordance_with_header(header, data, "warning")
  is_number_of_points_by_return_in_accordance_with_header(header, data, "warning")
  is_XY_smaller_than_bbox(header, data, "warning")
  is_Z_in_bbox(header, data, "warning")
  is_RGB_in_valid_format(header, data, "warning")
  is_NIR_in_valid_format(header, data, "warning")
  is_gpstime_in_valid_format(header, data, "warning")
  is_ScanAngle_in_valid_format(header, data, "warning")

  return(invisible())
}

check_output_file = function(file)
{
  islas = tools::file_ext(file) %in% c("las", "laz")

  if (length(file) > 1)
    stop("Write only one file at a time.")

  if (!islas)
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