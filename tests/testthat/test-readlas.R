context("readlas")

lazfile <- system.file("extdata", "example.laz", package="rlas")

test_that("read returns good values", {

  las = read.las(lazfile)

  expect_true(is.data.frame(las))
  expect_equal(dim(las), c(30, 16))
})

test_that("filter returns good values", {

  las = read.las(lazfile, filter = "-keep_first")

  expect_true(is.data.frame(las))
  expect_equal(dim(las), c(26, 16))
})

test_that("colunm selection works", {

  las = read.las(lazfile, select = "xyzrndecaupRGBN0")

  expect_true(is.data.frame(las))
  expect_equal(dim(las), c(30, 11))
  expect_true(!"gpstime" %in% names(las))
  expect_true(!"Intensity" %in% names(las))
  expect_true("Classification" %in% names(las))

  las = read.las(lazfile, select = "xyztirndeauRGBN0")

  expect_true(is.data.frame(las))
  expect_equal(dim(las), c(30, 11))
  expect_true("gpstime" %in% names(las))
  expect_true("Intensity" %in% names(las))
  expect_true(!"Classification" %in% names(las))
})

test_that("colunm unselection works", {

  las1 = read.las(lazfile, select = "xyzrndecskwaupRGBN0")
  las2 = read.las(lazfile, select = "* -i -t")

  expect_equal(las1, las2)

  las1 = read.las(lazfile, select = "xyztirndeauRGBN0")
  las2 = read.las(lazfile, select = "* -c -s -k -w -p")

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

test_that("filter wkt works with a POLYGON", {
  ofile = ""
  wkt = "POLYGON ((339008 5248000, 339007 5248001, 339011 5248001, 339010 5248000, 339008 5248000))"
  las = rlas:::stream.las(lazfile, ofile, select = "* -t -i -s -k -w", filter_wkt = wkt)
  expect_equal(dim(las), c(14, 11))
})

test_that("filter wkt works with a POLYGON with hole", {
  ofile = ""
  wkt = "POLYGON ((339008 5248000, 339007 5248001, 339011 5248001, 339010 5248000, 339008 5248000), (339008.3 5248000.5, 339008.2 5248000.1, 339008.8 5248000.1, 339009.7 5248000.2, 339009.1 5248000.5, 339008.3 5248000.5))"
  las = rlas:::stream.las(lazfile, ofile, filter_wkt = wkt)
  expect_equal(dim(las), c(10, 16))
})


test_that("filter wkt works with a MULTIPOLYGON", {
  ofile = ""
  wkt = "MULTIPOLYGON (((339008 5248000, 339008.1 5248001, 339009.2 5248001, 339010 5248000, 339008 5248000), (339008.3 5248000.5, 339008.2 5248000.1, 339008.8 5248000.1, 339009.7 5248000.2, 339009.1 5248000.6, 339008.3 5248000.5)), ((339003 5248001, 339003 5248000, 339007.3 5248001, 339003 5248001)))"
  las = rlas:::stream.las(lazfile, ofile, filter_wkt = wkt)
  expect_equal(dim(las), c(11, 16))
})

test_that("extra byte selection works", {
  lazfile <- system.file("extdata", "extra_byte.laz", package="rlas")
  las = read.las(lazfile)

  expect_true("Pulse width" %in% names(las))
  expect_true("Amplitude" %in% names(las))
  expect_equal(ncol(las), 18)

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
  expect_equal(ncol(las), 16)
})