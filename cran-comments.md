This release aims to fix an issue with g++-12 that will prevent the package BH to be uploaded. New sprintf error has not been fixed yet. This change implies 784 occurrences spread in 20.000 lines of code from a third party library that is not mine. Occurrences if sprintf are not all trivial to fix and it represents a significant job. I'll fixed that but I need more time.

## Test environments

* Linux Mint, R 4.2
* Debian Linux, R-devel, GCC ASAN/UBSAN (via R-hub)
* Debian Linux, R-release (via R-hub)
* Mac OS, R-release (via CRAN builder)
* Windows, R-release (via CRAN builder)
* Oracle Solaris 10, x86, 32 bit, R-release (via R-hub)

## R CMD check results
0 ERRORs | 0 WARNINGs | 0 NOTE

## Downstream dependencies

There is currently 1 downstream dependency for this package.

* `lidR` no problem reported
