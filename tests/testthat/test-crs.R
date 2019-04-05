context("coordinate reference system")

lazfile    <- system.file("extdata", "example.laz", package = "rlas")
las        <- read.las(lazfile)
header     <- read.lasheader(lazfile)
write_path <- tempfile(fileext = ".las")

test_that("set_epsg updates the header", {

  new_header <- header_set_epsg(header, 123)

  expect_equal(header_get_epsg(new_header), 123)

  new_header <- header_set_epsg(new_header, 1234)

  expect_equal(header_get_epsg(new_header), 1234)
})

test_that("set_wkt updates the header", {

  new_header <- header_set_wktcs(header, "STRING")

  expect_equal(header_get_wktcs(new_header), "STRING")
  expect_true(new_header[["Global Encoding"]][["WKT"]])
})


test_that("write.las writes epsg code",{

  new_header <- header_set_epsg(header, 26917)

  write.las(write_path, new_header, las)

  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(wheader[["Variable Length Records"]][["GeoKeyDirectoryTag"]][["tags"]][[1]][["value offset"]], 26917)
})

test_that("write.las writes WKT string",{

  new_header <- header_set_wktcs(header, "STRING \"STRING")

  write.las(write_path, new_header, las)

  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(wheader[["Global Encoding"]][["WKT"]], TRUE)
  expect_equal(wheader[["Variable Length Records"]][["WKT OGC CS"]][["WKT OGC COORDINATE SYSTEM"]], "STRING \"STRING")
})