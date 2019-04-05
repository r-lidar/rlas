context("read header")

lazfile <- system.file("extdata", "example.laz", package = "rlas")
header <- read.lasheader(lazfile)

test_that("read.lasheader read the header", {

  expect_equal(header[["Point Data Format ID"]], 1L)
  expect_equal(header[["Number of point records"]], 30L)
  expect_equal(header[["Number of points by return"]], c(26, 4, 0, 0, 0))
  expect_equal(header[["System Identifier"]], "LAStools (c) by rapidlasso GmbH")
})
