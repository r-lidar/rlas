I removed strnlen that is not C++ but POSIX. Compilation on Solaris will be OK.

## Test environments
* Linux Mint 18, R 3.4.4, 
* windows via CRAN builder

## R CMD check results
There were no ERRORs or WARNINGs or NOTEs.

## Downstream dependencies
There is currently 1 downstream dependency for this package.

* `lidR` no problem reported with `devtools::revdep_check()`

