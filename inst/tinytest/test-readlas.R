lazfile <- system.file("extdata", "example.laz", package = "rlas")
stream.las <- rlas:::stream.las

# "read returns good values", {

las <- read.las(lazfile)

expect_true(is.data.frame(las))
expect_equal(dim(las), c(30, 16))


# "filter returns good values", {

las <- read.las(lazfile, filter = "-keep_first")

expect_true(is.data.frame(las))
expect_equal(dim(las), c(26, 16))


# "tranform returns good values", {

las <- read.las(lazfile, transform = "-scale_z 0.5")

expect_equal(mean(las$Z), 487.9, tol = 0.1)

las <- read.las(lazfile, filter = "-keep_first", transform = "-scale_z 0.5")

expect_equal(dim(las), c(26, 16))
expect_equal(mean(las$Z), 488, tol = 0.1)


# "colunm selection works", {

las <- read.las(lazfile, select = "xyzrndecaupRGBN0")

expect_true(is.data.frame(las))
expect_equal(dim(las), c(30, 11))
expect_true(!"gpstime" %in% names(las))
expect_true(!"Intensity" %in% names(las))
expect_true("Classification" %in% names(las))

las <- read.las(lazfile, select = "xyztirndeauRGBN0")

expect_true(is.data.frame(las))
expect_equal(dim(las), c(30, 11))
expect_true("gpstime" %in% names(las))
expect_true("Intensity" %in% names(las))
expect_true(!"Classification" %in% names(las))


# "colunm unselection works", {

las1 <- read.las(lazfile, select = "xyzrndecskwaupRGBN0")
las2 <- read.las(lazfile, select = "* -i -t")

expect_equal(las1, las2)

las1 <- read.las(lazfile, select = "xyztirndeauRGBN0")
las2 <- read.las(lazfile, select = "* -c -s -k -w -p")

expect_equal(las1, las2)



# "streaming works", {

ifile <- system.file("extdata", "example.laz", package = "rlas")
ofile <- tempfile(fileext = ".las")

las1 <- read.las(ifile, filter = "-keep_first")
stream.las(ifile, ofile, filter = "-keep_first")
las2 <- read.las(ofile)

expect_equal(las1, las2)


# "streaming works with extra bytes", {

ifile <- system.file("extdata", "extra_byte.laz", package = "rlas")
ofile <- tempfile(fileext = ".las")

las1 <- read.las(ifile, filter = "-keep_first")
stream.las(ifile, ofile, filter = "-keep_first")
las2 <- read.las(ofile)

expect_equal(las1, las2)


# "streaming reads if no ofile", {

ifile <- system.file("extdata", "example.laz", package = "rlas")
ofile <- ""
las1  <- read.las(ifile, filter = "-keep_first")
las2  <- stream.las(ifile, ofile, filter = "-keep_first")

expect_equal(las1, las2)


# "filter wkt works with a POLYGON", {

ifile <- system.file("extdata", "example.laz", package = "rlas")
ofile <- ""
wkt   <- "POLYGON ((339008 5248000, 339007 5248001, 339011 5248001, 339010 5248000, 339008 5248000))"
las   <- stream.las(ifile, ofile, select = "* -t -i -s -k -w", filter_wkt = wkt)
expect_equal(dim(las), c(14, 11))


# "filter wkt works with a POLYGON with hole", {

ifile <- system.file("extdata", "example.laz", package = "rlas")
ofile <- ""
wkt   <- "POLYGON ((339008 5248000, 339007 5248001, 339011 5248001, 339010 5248000, 339008 5248000), (339008.3 5248000.5, 339008.2 5248000.1, 339008.8 5248000.1, 339009.7 5248000.2, 339009.1 5248000.5, 339008.3 5248000.5))"
las   <- stream.las(ifile, ofile, filter_wkt = wkt)
expect_equal(dim(las), c(10, 16))



# "filter wkt works with a MULTIPOLYGON", {

ifile <- system.file("extdata", "example.laz", package = "rlas")
ofile <- ""
wkt   <- "MULTIPOLYGON (((339008 5248000, 339008.1 5248001, 339009.2 5248001, 339010 5248000, 339008 5248000), (339008.3 5248000.5, 339008.2 5248000.1, 339008.8 5248000.1, 339009.7 5248000.2, 339009.1 5248000.6, 339008.3 5248000.5)), ((339003 5248001, 339003 5248000, 339007.3 5248001, 339003 5248001)))"
las   <- rlas:::stream.las(ifile, ofile, filter_wkt = wkt)
expect_equal(dim(las), c(11, 16))


# "extra byte selection works", {

ifile <- system.file("extdata", "extra_byte.laz", package = "rlas")
las   <- read.las(ifile)

expect_true("Pulse width" %in% names(las))
expect_true("Amplitude" %in% names(las))
expect_equal(ncol(las), 18)

pw1 <- las$`Pulse width`
las <- read.las(ifile, select = "xyztirndecaupRGBN25")

expect_true("Pulse width" %in% names(las))
expect_false("Amplitude" %in% names(las))
expect_equal(ncol(las), 14)

pw2 <- las$`Pulse width`

expect_equal(pw1, pw2)

las <- read.las(lazfile, select = "xyztirndecaupRGBN")

expect_false("Pulse width" %in% names(las))
expect_false("Amplitude" %in% names(las))
expect_equal(ncol(las), 13)

las <- read.las(lazfile, select = "* -0")

expect_false("Pulse width" %in% names(las))
expect_false("Amplitude" %in% names(las))
expect_equal(ncol(las), 16)