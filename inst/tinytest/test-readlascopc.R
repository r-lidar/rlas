lazfile <- system.file("extdata", "example.copc.laz", package = "rlas")

# "read returns good values", {

header <- read.lasheader(lazfile)

expect_equal(header$`File Signature`, "LASF")