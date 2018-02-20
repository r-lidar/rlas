context("readlas")

lazfile <- system.file("extdata", "example.laz", package="rlas")

test_that("read returns good values", {

  las = read.las(lazfile)

  expect_true(data.table::is.data.table(las))
  expect_equal(dim(las), c(30, 13))
})

test_that("filter returns good values", {

  las = read.las(lazfile, filter = "-keep_first")

  expect_true(data.table::is.data.table(las))
  expect_equal(dim(las), c(26, 13))
})

test_that("colunm selection works", {

  las = read.las(lazfile, select = "xyzrndecaupRGBN0")

  expect_true(data.table::is.data.table(las))
  expect_equal(dim(las), c(30, 11))
  expect_true(!"gpstime" %in% names(las))
  expect_true(!"Intensity" %in% names(las))
  expect_true("Classification" %in% names(las))

  las = read.las(lazfile, select = "xyztirndeauRGBN0")

  expect_true(data.table::is.data.table(las))
  expect_equal(dim(las), c(30, 11))
  expect_true("gpstime" %in% names(las))
  expect_true("Intensity" %in% names(las))
  expect_true(!"Classification" %in% names(las))
})

test_that("colunm unselection works", {

  las1 = read.las(lazfile, select = "xyzrndecaupRGBN0")
  las2 = read.las(lazfile, select = "* -i -t")

  expect_equal(las1, las2)

  las1 = read.las(lazfile, select = "xyztirndeauRGBN0")
  las2 = read.las(lazfile, select = "* -c -p")

  expect_equal(las1, las2)
})


test_that("streaming works", {
  ofile = paste0(tempfile(), ".las")

  las1 = read.las(lazfile, filter = "-keep_first")
  rlas:::stream.las(lazfile, ofile, filter = "-keep_first")
  las2 = read.las(ofile)

  expect_equal(las1, las2)
})

test_that("streaming works with extra bytes", {
  lazfile <- system.file("extdata", "extra_byte.laz", package="rlas")
  ofile = paste0(tempfile(), ".las")

  las1 = read.las(lazfile, filter = "-keep_first")
  rlas:::stream.las(lazfile, ofile, filter = "-keep_first")
  las2 = read.las(ofile)

  expect_equal(las1, las2)
})

test_that("streaming reads if no ofile", {
  ofile = ""
  las1 = read.las(lazfile, filter = "-keep_first")
  las2 = rlas:::stream.las(lazfile, ofile, filter = "-keep_first")

  expect_equal(las1, las2)
})

test_that("read in poly works", {
  ofile = ""

  xpoly = c(8, 10, 11, 7, 8) + 339000
  ypoly = c(0, 0, 1, 1, 0) + 5248000

  las = rlas:::stream.las_inpoly(lazfile, xpoly, ypoly, ofile, filter = "")

  expect_equal(dim(las), c(14, 13))
})


test_that("read in poly works with filter and select", {
  ofile = ""

  xpoly = c(8, 10, 11, 7, 8) + 339000
  ypoly = c(0, 0, 1, 1, 0) + 5248000

  las = rlas:::stream.las_inpoly(lazfile, xpoly, ypoly, ofile, select = "* -t -i", filter = "-drop_z_above 977.5")

  expect_equal(dim(las), c(8, 11))
})

test_that("extra byte selection works", {
  lazfile <- system.file("extdata", "extra_byte.laz", package="rlas")

  las = read.las(lazfile)

  expect_true("Pulse width" %in% names(las))
  expect_true("Amplitude" %in% names(las))
  expect_equal(ncol(las), 15)

  pw1 = las$`Pulse width`

  las = read.las(lazfile, select = "xyztirndecaupRGBN25")

  expect_true("Pulse width" %in% names(las))
  expect_false("Amplitude" %in% names(las))
  expect_equal(ncol(las), 14)

  pw2 = las$`Pulse width`

  expect_equal(pw1, pw2)

  las = read.las(lazfile, select = "xyztirndecaupRGBN")

  expect_false("Pulse width" %in% names(las))
  expect_false("Amplitude" %in% names(las))
  expect_equal(ncol(las), 13)

  las = read.las(lazfile, select = "* -0")

  expect_false("Pulse width" %in% names(las))
  expect_false("Amplitude" %in% names(las))
  expect_equal(ncol(las), 13)
})