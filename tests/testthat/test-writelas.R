context("writelas")

lazfile <- system.file("extdata", "extra_byte.laz", package="rlas")
las = readlasdata(lazfile)
lasheader = readlasheader(lazfile)

# file written temporarly
tmpdir = tempdir()
laswFpath = file.path(tmpdir, "temp.laz")

check_EB_header <- function(new, origin){
  if(is.list(new))
    mapply(check_EB_header, new, origin)
  else
    new[1] == origin[1]
}

test_that("Writes Extra Bytes correctly",{
  writelas(laswFpath, lasheader, X = las$X, Y = las$Y, Z = las$Z, ExtraBytes = las[,.(Amplitude, `Pulse width`)])
  lasw = readlasdata(laswFpath)
  laswheader = readlasheader(laswFpath)

  eb1 <- laswheader$`Variable Length Records`$Extra_Bytes
  eb2 <- lasheader$`Variable Length Records`$Extra_Bytes
  EB_header_check <- check_EB_header(eb1, eb2)
  EB_header_check$description <- NULL
  EB_header_check$reserved <- NULL
  EB_header_check <- unlist(EB_header_check)

  expect_true(all(EB_header_check))

  data1 = lasw[,.(X, Y, Z, Amplitude, `Pulse width`)]
  data2 = las[,.(X, Y, Z, Amplitude, `Pulse width`)]

  expect_equal(data1, data2)
})

test_that("Writes also without Extra Bytes", {
  writelas(laswFpath, lasheader, X = las$X, Y = las$Y, Z = las$Z)
  lasw = readlasdata(laswFpath)
  laswheader = readlasheader(laswFpath)

  data1 = lasw[,.(X, Y, Z)]
  data2 = las[,.(X, Y, Z)]

  expect_equal(data1, data2)
  expect_equal(length(laswheader$`Variable Length Records`), 0)
})

test_that("Default Extra Byte description values are well set when not avaliable",{
  laswheader <- readlasheader(laswFpath)
  laswheader$`Variable Length Records` <- list()

  writelas(laswFpath, laswheader, X = las$X, Y = las$Y, Z = las$Z, ExtraBytes = las[,.(Amplitude)])

  lasw <- readlasdata(laswFpath)
  laswheader <- readlasheader(laswFpath)

  data1 = lasw[,.(Amplitude)]
  data2 = las[,.(Amplitude)]

  expect_equal(data1, data2)
})

test_that("Modifying Extra Byte format works",{
  laswheader <- readlasheader(laswFpath)
  laswheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude$data_type = 1
  laswheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude$scale = NULL
  laswheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude$options = 0
  laswheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude$scale = NULL

  writelas(laswFpath, laswheader, X = las$X, Y = las$Y, Z = las$Z, ExtraBytes = las[,.(Amplitude)])
  lasw <- readlasdata(laswFpath)
  laswheader <- readlasheader(laswFpath)

  expect_true(all(lasw$Amplitude == floor(las$Amplitude+.5)))
  expect_true(laswheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude$data_type==1)
})









