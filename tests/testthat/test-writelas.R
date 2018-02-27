context("writelas")

lazfile <- system.file("extdata", "example.laz", package="rlas")
las = readlasdata(lazfile)
header = readlasheader(lazfile)
write_path = file.path(tempdir(), "temp.laz")

test_that("write.las writes a correct file",{
  write.las(write_path, header, las)
  wlas = readlasdata(write_path)
  wheader = readlasheader(write_path)

  expect_equal(las, wlas)
  expect_equal(header$`Point Data Format ID`, wheader$`Point Data Format ID`)
})

test_that("write.las does not write time if point format enforced to 0",{
  new_header = header
  new_header$`Point Data Format ID` = 0

  expect_warning(write.las(write_path, new_header, las), "gpstime")

  wlas = readlasdata(write_path)
  wheader = readlasheader(write_path)

  expect_true(!"gpstime" %in% names(wlas))
  expect_equal(new_header$`Point Data Format ID`, 0)
})


lazfile <- system.file("extdata", "extra_byte.laz", package="rlas")
las = readlasdata(lazfile)
header = readlasheader(lazfile)

check_EB_header <- function(new, origin){
  if(is.list(new))
    mapply(check_EB_header, new, origin)
  else
    new[1] == origin[1]
}

test_that("write.las writes Extra Bytes correctly",{
  write.las(write_path, header, las)
  wlas = readlasdata(write_path)
  wheader = readlasheader(write_path)

  eb1 <- wheader$`Variable Length Records`$Extra_Bytes
  eb2 <- header$`Variable Length Records`$Extra_Bytes
  EB_header_check <- check_EB_header(eb1, eb2)
  EB_header_check$description <- NULL
  EB_header_check$reserved <- NULL
  EB_header_check <- unlist(EB_header_check)

  expect_true(all(EB_header_check))

  expect_equal(las, wlas)
})


test_that("write.las skip extra bytes if empty VLR", {
  new_header = header
  new_header$`Variable Length Records` = list()
  write.las(write_path, new_header, las)
  wlas = readlasdata(write_path)
  wheader = readlasheader(write_path)

  expect_true(!any(c("Amplitude", "Pulse width") %in% names(wlas)))
  expect_equal(wlas, las[, -c(14:15)])
  expect_equal(length(wheader$`Variable Length Records`), 0)
})

test_that("write.las skiped selectively extra byte if missing VLR",{
  new_header = header
  new_header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude = NULL

  write.las(write_path, new_header, las)

  wlas <- readlasdata(write_path)
  wheader <- readlasheader(write_path)

  expect_true(!"Amplitude" %in% names(wlas))
  expect_equal(wlas, las[, -c(14)])
})

test_that("Modifying Extra Byte format works",{
  new_header = header
  new_header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude$data_type = 1
  new_header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude$scale = NULL
  new_header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude$options = 0
  new_header$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude$scale = NULL

  write.las(write_path, new_header, las)
  wlas <- readlasdata(write_path)
  wheader <- readlasheader(write_path)

  expect_true(all(wlas$Amplitude == floor(las$Amplitude+.5)))
  expect_true(wheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude$data_type==1)
})
