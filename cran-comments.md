## Test environments

* Linux Mint, R 4.2
* Debian Linux, R-devel, GCC ASAN/UBSAN (via R-hub)
* Debian Linux, R-release (via R-hub)
* Mac OS, R-release (via CRAN builder)
* Windows, R-release (via CRAN builder)
* Oracle Solaris 10, x86, 32 bit, R-release (via R-hub)

## R CMD check results
0 ERRORs | 0 WARNINGs | 1 NOTE

Found the following (possibly) invalid URLs:
  URL: https://www.asprs.org/divisions-committees/lidar-division/laser-las-file-format-exchange-activities
    From: DESCRIPTION
    Status: 500
    Message: Internal Server Error

This url is valid but for an unknown reason the server seems to reject wget's user agent. It works
with curl by the way. According to some stackoverflow anwsers it is a bug in the webpage.

## Downstream dependencies

There is currently 1 downstream dependency for this package.

* `lidR` no problem reported
