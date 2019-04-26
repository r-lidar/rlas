context("write las")

lazfile    <- system.file("extdata", "example.laz", package = "rlas")
las        <- read.las(lazfile)
header     <- read.lasheader(lazfile)
write_path <- tempfile(fileext = ".las")

test_that("write.las writes a correct file",{

  write.las(write_path, header, las)

  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(header[-c(7:10, 12)], wheader[-c(7:10, 12)])
})

test_that("UUID is properly written",{

  uuid <- "13d5cf87-ee3b-43d5-bbf1-c33a3cda5948"
  new_header <- header
  new_header[["Project ID - GUID"]] <- uuid

  write.las(write_path, new_header, las)
  wheader <- read.lasheader(write_path)

  expect_equal(wheader[["Project ID - GUID"]], uuid)
})

test_that("write.las does not write time if point format enforced to 0",{

  new_header <- header
  new_header[["Point Data Format ID"]] <- 0L

  expect_warning(write.las(write_path, new_header, las), "gpstime")

  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)

  expect_true(!"gpstime" %in% names(wlas))
  expect_equal(new_header[["Point Data Format ID"]], 0L)
})

lazfile    <- system.file("extdata", "extra_byte.laz", package = "rlas")
las        <- read.las(lazfile)
header     <- read.lasheader(lazfile)
write_path <- tempfile(fileext = ".las")


test_that("write.las writes Extra Bytes correctly",{

  write.las(write_path, header, las)
  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)

  eb1 <- wheader[["Variable Length Records"]][["Extra_Bytes"]]
  eb2 <- header[["Variable Length Records"]][["Extra_Bytes"]]
  eb1[["reserved"]] <- NULL
  eb2[["reserved"]] <- NULL
  eb1[["description"]] <- NULL
  eb2[["description"]] <- NULL

  expect_equal(eb1, eb2)
  expect_equal(las, wlas)
})

test_that("write.las skips extra bytes if empty VLR", {

  new_header <- header
  new_header[["Variable Length Records"]] <- list()

  write.las(write_path, new_header, las)
  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)

  expect_true(!any(c("Amplitude", "Pulse width") %in% names(wlas)))
  expect_equal(wlas, las[, -c(17:18)])
  expect_equal(length(wheader[["Variable Length Records"]]), 0L)
})

test_that("write.las skips selectively extra bytes if missing VLR",{

  new_header <- header
  new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Amplitude"]] <- NULL

  write.las(write_path, new_header, las)

  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)

  expect_true(!"Amplitude" %in% names(wlas))
  expect_equal(wlas, las[, -c(17)])
})

# test_that("write.las writes LAS 1.4",{
#
#   new_header <- header
#   new_header[["Version Minor"]] <- 4L
#   new_header[["Header Size"]]   <- 375L
#
#   write.las(write_path, new_header, las)
#
#   wlas    <- read.las(write_path)
#   wheader <- read.lasheader(write_path)
#
#   expect_equal(wheader[["Version Minor"]], 4L)
#   expect_equal(length(wheader[["Number of points by return"]]), 15)
#   expect_equal(las, wlas)
# })
#
# test_that("write.las writes LAS 1.4 point format 6",{
#
#   new_header <- header
#   new_header[["Version Minor"]] <- 4L
#   new_header[["Point Data Format ID"]] <- 6L
#   new_header[["Header Size"]] <- 375L
#
#   las$Overlap_flag <- TRUE
#   las$ScannerChannel <- 1L
#
#   colnames(las)[colnames(las) == "ScanAngleRank"] <- "ScanAngle"
#   las <- data.table::setcolorder(las, c("X", "Y", "Z", "gpstime", "Intensity", "ReturnNumber", "NumberOfReturns",
#                                  "ScanDirectionFlag", "EdgeOfFlightline", "Classification", "ScannerChannel",
#                                  "Synthetic_flag", "Keypoint_flag", "Withheld_flag", "Overlap_flag",
#                                  "ScanAngle", "UserData", "PointSourceID", "Amplitude", "Pulse width"))
#
#   write.las(write_path, new_header, las)
#
#   wlas    <- read.las(write_path)
#   wheader <- read.lasheader(write_path)
#
#   expect_equal(wheader[["Version Minor"]], 4L)
#   expect_equal(length(wheader[["Number of points by return"]]), 15)
#   expect_equal(las, wlas, tolerance = 0.00015)
# })



