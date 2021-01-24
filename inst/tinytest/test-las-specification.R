lazfile <- system.file("extdata", "example.laz", package = "rlas")
data    <- read.las(lazfile)
header  <- read.lasheader(lazfile)

# "check_las_validity works", {
#expect_error(check_las_validity(header, data), NA)


# "check_las_compliance works", {
#expect_error(check_las_compliance(header, data), NA)


# "las specification checks work", {

data$ReturnNumber[1] <- -1L
data$ReturnNumber[2] <- 2L^16L
data$ReturnNumber[3] <- 1.5
data$ReturnNumber[4] <- NA

expect_true(!is_valid_ReturnNumber(header, data))
expect_equal(length(is_valid_ReturnNumber(header, data, behavior = "vector")), 4)
expect_warning(is_valid_ReturnNumber(header, data, behavior = "warning"))
expect_error(is_valid_ReturnNumber(header, data, behavior = "stop"))

data$Intensity[1] <- as.integer(2L^23L)
data$Intensity[4] <- NA_integer_

expect_true(!is_valid_Intensity(data))
expect_equal(length(is_valid_Intensity(data, behavior = "vector")), 2)

expect_true(is_valid_NumberOfReturns(header, data))

data$R <- 1L
data$G <- -1
data$B <- 1L

expect_true(!is_valid_RGB(data))
expect_equal(length(is_valid_RGB(data, behavior = "vector")), 2)

data$B <- as.integer(2L^23L)

expect_equal(length(is_valid_RGB(data, behavior = "vector")), 3)

