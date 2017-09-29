context("readlas")

lazfile <- system.file("extdata", "example.laz", package="rlas")

test_that("read returns good values", {

  las = readlasdata(lazfile)

  expect_true(data.table::is.data.table(las))
  expect_equal(dim(las), c(30, 13))
})

test_that("filter returns good values", {

  las = readlasdata(lazfile, filter = "-keep_first")

  expect_true(data.table::is.data.table(las))
  expect_equal(dim(las), c(26, 13))
})

test_that("colunm selection works", {

  las = readlasdata(lazfile, t = F, i = F)

  expect_true(data.table::is.data.table(las))
  expect_equal(dim(las), c(30, 11))
  expect_true(!"gpstime" %in% names(las))
  expect_true(!"Intensity" %in% names(las))
  expect_true("Classification" %in% names(las))

  las = readlasdata(lazfile, p = F, c = F)

  expect_true(data.table::is.data.table(las))
  expect_equal(dim(las), c(30, 11))
  expect_true("gpstime" %in% names(las))
  expect_true("Intensity" %in% names(las))
  expect_true(!"Classification" %in% names(las))
})

test_that("streaming works", {
  ofile = paste0(tempfile(), ".las")

  las1 = readlasdata(lazfile, filter = "-keep_first")
  rlas:::streamlasdata(lazfile, ofile, "-keep_first")
  las2 = readlasdata(ofile)

  expect_equal(las1, las2)
})

test_that("streaming reads if no ofile", {
  ofile = ""

  las1 = readlasdata(lazfile, filter = "-keep_first")
  las2 = rlas:::streamlasdata(lazfile, ofile, "-keep_first")

  expect_equal(las1, las2)
})

test_that("read in poly works", {
  ofile = ""

  xpoly = c(8, 10, 11, 7, 8) + 339000
  ypoly = c(0, 0, 1, 1, 0) + 5248000

  las = rlas:::streamlasdata_inpoly(lazfile, xpoly, ypoly, ofile, filter = "")

  expect_equal(dim(las), c(14, 13))
})


test_that("read in poly works with filter and select", {
  ofile = ""

  xpoly = c(8, 10, 11, 7, 8) + 339000
  ypoly = c(0, 0, 1, 1, 0) + 5248000

  las = rlas:::streamlasdata_inpoly(lazfile, xpoly, ypoly, ofile, filter = "-drop_z_above 977.5", i = FALSE, t = FALSE)

  expect_equal(dim(las), c(8, 11))
})
