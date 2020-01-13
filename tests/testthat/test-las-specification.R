lazfile <- system.file("extdata", "example.laz", package = "rlas")
data    <- read.las(lazfile)
header  <- read.lasheader(lazfile)

test_that("check_las_validity works", {
  expect_error(check_las_validity(header, data), NA)
})

test_that("check_las_compliance works", {
  expect_error(check_las_compliance(header, data), NA)
})