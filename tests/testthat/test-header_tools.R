context("header tools")

lazfile <- system.file("extdata", "example.laz", package = "rlas")
las = read.las(lazfile)
header = read.lasheader(lazfile)
write_path = file.path(tempdir(), "temp.las")

test_that("add_extrabytes creates a correct VLR with no NA", {

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

test_that("add_extrabytes creates a correct VLR with NAs", {

  # int

  ex = sample(1:10, nrow(las), TRUE)
  ex[c(5,8)] = NA_integer_
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
  ex[c(5,8)] = NA_real_
  las$Exdata = ex
  new_header = header_add_extrabytes(header, las$Exdata, "Exdata", "Extra numeric data")

  write.las(write_path, new_header, las)

  wlas = read.las(write_path)
  wheader = read.lasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(wheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Exdata$options, 7)
  expect_true(is.numeric(wlas$Exdata))
})

test_that("add_extrabytes creates a correct VLR with NAs only", {

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

test_that("add_extrabytes_manual creates a correct VLR", {

  # from int to unsigned short

  las$Exdata = sample(0:10, nrow(las), TRUE)
  #las$Exdata[1] = 2^16+1
  new_header = header_add_extrabytes_manual(header, "Exdata", "Extra numeric data", 3L)

  write.las(write_path, new_header, las)

  wlas = read.las(write_path)
  wheader = read.lasheader(write_path)

  expected = las$Exdata
  #expected[expected < 0] = 0
  #expected[expected > 2^16] = expected[expected > 2^16] - 2^16
  get = wlas$Exdata

  expect_equal(get, expected)
  expect_equal(wheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Exdata$data_type, 3L)
  expect_equal(wheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Exdata$options, 0L)
  expect_true(is.integer(wlas$Exdata))

  # from double to int short

  las$Exdata = runif(nrow(las), 0, 100)
  new_header = header_add_extrabytes_manual(header, "Exdata", "Extra numeric data", 6L)

  write.las(write_path, new_header, las)

  wlas = read.las(write_path)
  wheader = read.lasheader(write_path)

  expected = round(las$Exdata)
  get = wlas$Exdata

  expect_equal(get, expected)
  expect_equal(wheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Exdata$data_type, 6L)
  expect_equal(wheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Exdata$options, 0L)
  expect_true(is.integer(wlas$Exdata))

  # Double stored in in + scale + offset

  las$Exdata = runif(nrow(las), 10, 100)
  new_header = header_add_extrabytes_manual(header, "Exdata", "Extra numeric data", 7L, scale = 0.01, offset = 10)

  write.las(write_path, new_header, las)

  wlas = read.las(write_path)
  wheader = read.lasheader(write_path)

  expected = round(las$Exdata, 2)
  get = wlas$Exdata

  expect_equal(get, expected)
  expect_equal(wheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Exdata$data_type, 7L)
  expect_equal(wheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Exdata$options, 24L)
  expect_true(is.double(wlas$Exdata))

})

test_that("add_extrabytes raise error for unproper request", {

  expect_error(header_add_extrabytes_manual(header, "Exdata", "Extra numeric data", 0L), "not supported")
  expect_message(header_add_extrabytes_manual(header, "Exdata", "A long decription exeding the 32 characters allowed by the spec", 5L), "description is longer than the 32 characters allowed")

  new_header = suppressMessages(header_add_extrabytes_manual(header, "Exdata", "A long decription exeding the 32 characters allowed by the spec", 5L))

  expect_error(write.las(write_path, new_header, las), "not in the data")

  las$Exdata = runif(nrow(las), 10, 100)

  write.las(write_path, new_header, las)
  wheader = read.lasheader(write_path)

  desc = wheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Exdata$description
})

test_that("set_epsg updates the header", {

  new_header = header_set_epsg(header, 123)

  expect_equal(header_get_epsg(new_header), 123)

  new_header = header_set_epsg(new_header, 1234)

  expect_equal(header_get_epsg(new_header), 1234)
})

test_that("set_wkt updates the header", {

  new_header = header_set_wktcs(header, "STRING")

  expect_equal(header_get_wktcs(new_header), "STRING")
  expect_true(new_header[["Global Encoding"]][["WKT"]])
})
