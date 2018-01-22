
context("writelas")

lazfile <- system.file("extdata", "extra_byte.laz", package="rlas")
las=readlasdata(lazfile)
lasheader=readlasheader(lazfile)

# file written temporarly
tmpdir=tempdir()
laswFpath = file.path(tmpdir, "temp.laz")

check_EB_header <- function(new, origin){
  if(is.list(new))
    mapply(check_EB_header, new, origin)
  else
    new[1] == origin[1]
}

test_that("Writes Extra Bytes correctly",{
  writelas(laswFpath, lasheader, X = las$X, Y = las$Y, Z = las$Z, ExtraBytes = las[,.(Amplitude, `Pulse width`)])
  lasw=readlasdata(laswFpath)
  laswheader=readlasheader(laswFpath)
  EB_header_check=check_EB_header(laswheader$`Variable Length Records`$Extra_Bytes,
                                  lasheader$`Variable Length Records`$Extra_Bytes)

  expect_true(all(lasw[,.(X, Y, Z, Amplitude, `Pulse width`)] == las[,.(X, Y, Z, Amplitude, `Pulse width`)]))
  expect_true(all(unlist(EB_header_check[c("user ID", "record ID", "length after header")])))
  expect_true(all(EB_header_check$`Extra Bytes Description`))
})

# cat("Original header: \n\n", yaml::as.yaml(lasheader$`Variable Length Records`$Extra_Bytes),"\n\n",
#     "New header: \n\n", yaml::as.yaml(laswheader$`Variable Length Records`$Extra_Bytes),"\n\n")

# Write without Extra Bytes
test_that("Writes also without Extra Bytes", {
  writelas(laswFpath, lasheader, X = las$X, Y = las$Y, Z = las$Z)
  lasw=readlasdata(laswFpath)
  laswheader=readlasheader(laswFpath)
  expect_true(all(lasw[,.(X, Y, Z)] == las[,.(X, Y, Z)]))
  expect_equal(length(laswheader$`Variable Length Records`), 0)
})

test_that("Default Extra Byte description values are well set when not avaliable",{
  laswheader=readlasheader(laswFpath)
  laswheader$`Variable Length Records`=list()
  writelas(laswFpath, laswheader, X = las$X, Y = las$Y, Z = las$Z, ExtraBytes = las[,.(Amplitude)])
  lasw=readlasdata(laswFpath)
  laswheader=readlasheader(laswFpath)

  expect_true(all(lasw[,.(Amplitude)] == las[,.(Amplitude)]))
  expect_true(all(check_EB_header(laswheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude,
                                  list(reserved=0,data_type=10,options=0, name="Amplitude", description="Amplitude"))))
})

test_that("Modifying Extra Byte format works",{
  laswheader=readlasheader(laswFpath)
  laswheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude$data_type=1
  laswheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude$scale=NULL
  laswheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude$options=0

  laswheader$`Variable Length Records`$Extra_Bytes$`Extra Bytes Description`$Amplitude$scale=NULL

  writelas(laswFpath, laswheader, X = las$X, Y = las$Y, Z = las$Z, ExtraBytes = las[,.(Amplitude)])
  lasw=readlasdata(laswFpath)
  laswheader=readlasheader(laswFpath)

  expect_true(all(lasw$Amplitude == floor(las$Amplitude+.5)))
  expect_true(laswheader$`Variable Length Records`$Extra_Bytes$
                 `Extra Bytes Description`$Amplitude$data_type==1)
})









