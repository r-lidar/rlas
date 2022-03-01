if ( !(identical(Sys.getenv("NOT_CRAN"), "true") || (isTRUE(unname(Sys.info()["user"]) == "jr"))) ) exit_file("Skip on CRAN")

lazfile <- system.file("extdata", "las14_prf6.laz", package = "rlas")
stream.las <- rlas:::stream.las

# "read returns good values", {

las <- read.las(lazfile)
header <- read.lasheader(lazfile)

expect_true(is.data.frame(las))
expect_equal(dim(las), c(135, 18))
expect_equal(mean(las$X), 487823.6, tolerance = 0.001)
expect_equal(mean(las$Y), 5313802, tolerance = 0.001)
expect_equal(mean(las$Z), 689.388, tolerance = 0.001)
expect_equal(mean(las$Intensity), 27601.32, tolerance = 0.001)
expect_equal(tabulate(las$ReturnNumber, 15), header$`Number of points by return`)
expect_equal(tabulate(las$NumberOfReturns, 15), c(50,45,33,6,1,0,0,0,0,0,0,0,0,0,0))
expect_equal(unique(las$ScanDirectionFlag), c(0,1))
expect_equal(as.numeric(table(las$ScanDirectionFlag)), c(65, 70))
expect_true(is.null(las$ScanAngleRank))
expect_true(is.double(las$ScanAngle))
expect_equal(mean(las$ScanAngle), -13.50187, tolerance = 0.000001)
expect_equal(unique(las$PointSourceID), 108)



