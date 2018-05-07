context("header tools")

lazfile <- system.file("extdata", "example.laz", package="rlas")
las = read.las(lazfile)
header = read.lasheader(lazfile)
write_path = file.path(tempdir(), "temp.las")

test_that("Add extra byte creates a correct VLR with no NA", {

  # int

  las$treeID = sample(1:10, nrow(las), TRUE)
  new_header = header_add_extrabytes(header, las$treeID, "treeID", "Unique ID")

  write.las(write_path, new_header, las)

  wlas = read.las(write_path)
  wheader = read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(wheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$treeID$description, "Unique ID")
  expect_true(is.integer(wlas$treeID))

  # double

  las$treeID = NULL
  las$Exdata = runif(nrow(las), 1, 10)
  new_header = header_add_extrabytes(header, las$Exdata, "Exdata", "Extra numeric data")

  write.las(write_path, new_header, las)

  wlas = read.las(write_path)
  wheader = read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(wheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Exdata$options, 6)
  expect_true(is.numeric(wlas$Exdata))
})

test_that("Add extra byte creates a correct VLR with NAs", {

  # int

  ex = sample(1:10, nrow(las), TRUE)
  ex[c(5,8)] = NA
  las$Exdata = ex
  new_header = header_add_extrabytes(header, las$Exdata, "Exdata", "Extra numeric data")

  write.las(write_path, new_header, las)

  wlas = read.las(write_path)
  wheader = read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(wheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Exdata$options, 7)
  expect_true(is.integer(wlas$Exdata))

  # double

  ex = runif(nrow(las), 1, 10)
  ex[c(5,8)] = NA
  las$Exdata = ex
  new_header = header_add_extrabytes(header, las$Exdata, "Exdata", "Extra numeric data")

  write.las(write_path, new_header, las)

  wlas = read.las(write_path)
  wheader = read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(wheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Exdata$options, 7)
  expect_true(is.numeric(wlas$Exdata))
})

test_that("Add extra byte creates a correct VLR with NAs only", {

  # int

  ex = rep(NA_integer_, nrow(las))
  las$Exdata = ex
  new_header = header_add_extrabytes(header, las$Exdata, "Exdata", "Extra numeric data")

  write.las(write_path, new_header, las)

  wlas = read.las(write_path)
  wheader = read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(wheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Exdata$options, 1)
  expect_true(is.integer(wlas$Exdata))

  # double

  ex = rep(NA_real_, nrow(las))
  las$Exdata = ex
  new_header = header_add_extrabytes(header, las$Exdata, "Exdata", "Extra numeric data")

  write.las(write_path, new_header, las)

  wlas = read.las(write_path)
  wheader = read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(wheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Exdata$options, 1)
  expect_true(is.numeric(wlas$Exdata))
})