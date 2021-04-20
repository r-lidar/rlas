lazfile <- system.file("extdata", "example.las", package = "rlas")
data    <- read.las(lazfile)
header  <- read.lasheader(lazfile)

# "header_create creates a valid header", {
h = header_create(data)

expect_equal(h[["Point Data Format ID"]],  header[["Point Data Format ID"]])

# "header_create creates a valid header", {

data = data.frame(X = runif(10), Y = runif(10), Z = runif(10))
data$gpstime   <- runif(10, 0, 100)
data$Intensity <- as.integer(runif(10, 0, 100))
data$R         <- as.integer(runif(10, 0, 2^16))
data$G         <- as.integer(runif(10, 0, 2^16))
data$B         <- as.integer(runif(10, 0, 2^16))

header = header_create(data)

expect_equal(header[["Version Minor"]], 2L)
expect_equal(header[["Point Data Format ID"]], 3L)

