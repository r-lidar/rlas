lazfile <- system.file("extdata", "example.las", package = "rlas")
header <- read.lasheader(lazfile)
data <- read.las(lazfile)

gz <- as.list(is_compressed(data))
expect_true(gz$Withheld_flag)
expect_true(gz$ScanDirectionFlag)
expect_true(gz$Synthetic_flag)
expect_true(gz$PointSourceID)
expect_true(gz$UserData)
expect_true(object.size(las) > true_size(las))

# Test that check function do not materialize ALTREP
check_las_validity(header, las)
check_las_compliance(header, las)
gz2 <- as.list(is_compressed(data))
expect_true(all.equal(gz, gz2))

# Test write.las does not materialize
f <- tempfile(fileext = ".las")
write.las(f, header, data)
gz3 <- as.list(is_compressed(data))
expect_true(all.equal(gz, gz3))