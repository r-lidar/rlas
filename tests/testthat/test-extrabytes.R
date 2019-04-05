context("extra bytes attributes")

lazfile    <- system.file("extdata", "example.laz", package = "rlas")
las        <- read.las(lazfile)
header     <- read.lasheader(lazfile)
write_path <- file.path(tempdir(), "temp.las")

test_that("add_extrabytes creates a correct VLR for int (no NA)", {

  las$treeID <- sample(1:10, nrow(las), TRUE)
  new_header <- header_add_extrabytes(header, las$treeID, "treeID", "Unique ID")

  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["treeID"]][["description"]], "Unique ID")
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["treeID"]][["data_type"]], 6L)
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["treeID"]][["options"]], 6L)

  write.las(write_path, new_header, las)

  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["treeID"]][["description"]], "Unique ID")
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["treeID"]][["data_type"]], 6L)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["treeID"]][["options"]], 6L)
  expect_true(is.integer(wlas$treeID))
})

test_that("add_extrabytes creates a correct VLR for double (no NA)", {

  las$Exdata <- runif(nrow(las), 1, 10)
  new_header <- header_add_extrabytes(header, las$Exdata, "Exdata", "Extra numeric data")

  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 10L)
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 6L)

  write.las(write_path, new_header, las)

  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 10L)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 6L)
  expect_true(is.numeric(wlas$Exdata))
})

test_that("add_extrabytes creates a correct VLR for int (NAs)", {

  ex <- sample(1:10, nrow(las), TRUE)
  ex[c(5,8)] <- NA_integer_
  las$Exdata <- ex
  new_header <- header_add_extrabytes(header, las$Exdata, "Exdata", "Extra numeric data")

  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 6L)
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["no_data"]], .Machine$integer.max)
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 7L)

  write.las(write_path, new_header, las)

  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 6L)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["no_data"]], .Machine$integer.max)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 7L)
  expect_true(is.integer(wlas$Exdata))
})

test_that("add_extrabytes creates a correct VLR for double (NAs)", {

  ex <- runif(nrow(las), 1, 10)
  ex[c(5,8)] <- NA_real_
  las$Exdata <- ex
  new_header <- header_add_extrabytes(header, las$Exdata, "Exdata", "Extra numeric data")

  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 10L)
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["no_data"]], .Machine$double.xmax)
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 7L)

  write.las(write_path, new_header, las)

  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 10L)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["no_data"]], .Machine$double.xmax)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 7L)
  expect_true(is.numeric(wlas$Exdata))
})

test_that("add_extrabytes creates a correct VLR with int (NAs only)", {

  ex <- rep(NA_integer_, nrow(las))
  las$Exdata <- ex
  new_header <- header_add_extrabytes(header, las$Exdata, "Exdata", "Extra numeric data")

  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 6L)
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["no_data"]], .Machine$integer.max)
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 1L)

  write.las(write_path, new_header, las)

  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 6L)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["no_data"]], .Machine$integer.max)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 1L)
  expect_true(is.integer(wlas$Exdata))
})

test_that("add_extrabytes creates a correct VLR with double (NAs only)", {

  ex <- rep(NA_real_, nrow(las))
  las$Exdata <- ex
  new_header <- header_add_extrabytes(header, las$Exdata, "Exdata", "Extra numeric data")

  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 10L)
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["no_data"]], .Machine$double.xmax)
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 1L)

  write.las(write_path, new_header, las)

  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 10L)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["no_data"]], .Machine$double.xmax)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 1L)
  expect_true(is.numeric(wlas$Exdata))
})

test_that("add_extrabytes_manual creates a correct VLR from int to ushort (no NA)", {

  las$Exdata <- sample(0:10, nrow(las), TRUE)
  new_header <- header_add_extrabytes_manual(header, "Exdata", "Extra numeric data", 3L)

  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 3L)
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 0L)

  write.las(write_path, new_header, las)

  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)
  expect  <- las$Exdata
  get     <- wlas$Exdata

  expect_equal(get, expect)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 3L)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 0L)
  expect_true(is.integer(wlas$Exdata))
})

test_that("add_extrabytes_manual creates a correct VLR from double to int (no NA)", {

  las$Exdata <- runif(nrow(las), 0, 100)
  new_header <- header_add_extrabytes_manual(header, "Exdata", "Extra numeric data", 6L)

  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 6L)
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 0L)

  write.las(write_path, new_header, las)

  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)
  expect  <- round(las$Exdata)
  get     <- wlas$Exdata

  expect_equal(get, expect)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 6L)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 0L)
  expect_true(is.integer(wlas$Exdata))
})

test_that("add_extrabytes_manual creates a correct VLR double stored with scale + offset (no NA)", {

  las$Exdata <- runif(nrow(las), 10, 100)
  new_header <- header_add_extrabytes_manual(header, "Exdata", "Extra numeric data", 7L, scale = 0.01, offset = 10)

  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 7L)
  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 24L)

  write.las(write_path, new_header, las)

  wlas    <- read.las(write_path)
  wheader <- read.lasheader(write_path)
  expect  <- round(las$Exdata, 2)
  get     <- wlas$Exdata

  expect_equal(get, expect)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "Extra numeric data")
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["data_type"]], 7L)
  expect_equal(wheader[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["options"]], 24L)
  expect_true(is.double(wlas$Exdata))
})

test_that("add_extrabytes truncate long descriptions", {

  expect_message(header_add_extrabytes_manual(header, "Exdata", "A long decription exeding the 32 characters allowed by the spec", 5L), "description is longer than the 32 characters allowed")

  new_header <- suppressMessages(header_add_extrabytes_manual(header, "Exdata", "A long decription exeding the 32 characters allowed by the spec", 5L))

  expect_equal(new_header[["Variable Length Records"]][["Extra_Bytes"]][["Extra Bytes Description"]][["Exdata"]][["description"]], "A long decription exeding the 3")
})

test_that("add_extrabytes raise error for unproper request", {

  expect_error(header_add_extrabytes_manual(header, "Exdata", "Extra numeric data", 0L), "not supported")

  new_header <- suppressMessages(header_add_extrabytes_manual(header, "Exdata", "A long decription exeding the 32 characters allowed by the spec", 5L))

  expect_error(write.las(write_path, new_header, las), "not in the data")
})
