lazfile <- system.file("extdata", "example.laz", package = "rlas")
data    <- read.las(lazfile)
header  <- read.lasheader(lazfile)

test_that("header_create creates a valid header", {
  h = header_create(data)

  expect_equal(h[["Point Data Format ID"]],  header[["Point Data Format ID"]])
})
