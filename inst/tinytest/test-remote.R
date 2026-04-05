url <- "https://localhost:65535/nonexistent.copc.laz"

expect_error(read.las(url))

url <- "https://s3.amazonaws.com/hobu-lidar/autzen-classified.copc.laz"

h <- read.lasheader(url)

expect_equal(h$`Number of point records`, 10653336L)

p <- read.las(url, filter = "-max_depth 0")

expect_equal(nrow(p), 61201L)

p <- read.las(url, filter = "-inside_circle 637368.8 851944.8 15")

expect_equal(nrow(p), 831L)
