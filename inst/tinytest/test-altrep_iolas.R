lazfile <- system.file("extdata", "example.las", package = "rlas")
header <- read.lasheader(lazfile)
data <- read.las(lazfile)

gz <- as.list(is_compressed(data))
expect_true(gz$Withheld_flag)
expect_true(gz$Synthetic_flag)
expect_true(gz$PointSourceID)
expect_true(gz$UserData)
expect_true(utils::object.size(data) > true_size(data))

# Test that check function do not materialize ALTREP
check_las_validity(header, data)
check_las_compliance(header, data)
gz2 <- as.list(is_compressed(data))
expect_true(all.equal(gz, gz2))

# Test that write function does not materialize ALTREP
f <- tempfile(fileext = ".las")
write.las(f, header, data)
gz3 <- as.list(is_compressed(data))
expect_true(all.equal(gz, gz2))