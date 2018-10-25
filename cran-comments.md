gcc 8 raises a warning on CRAN (-Wstringop-overflow=). I compiled rlas with gcc 8.1 using the same 
config than on CRAN but I did not reproduced the error. In addition it is a false positive. Anyway
I fixed this issue by replacing strncpy by an equivalent code that uses memcpy.

## Test environments
* Linux Mint 18, R 3.4.4, g++ -std=gnu++11
* Linux Mint 18, R 3.4.4, g++-8 -std=gnu++11
* Linux Mint 18, R 3.4.4, clang++-4.0 -std=gnu++11
* Windows via CRAN builder, R 3.5.1, R 3.6

## R CMD check results
There were no ERRORs or WARNINGs or NOTEs.

## Downstream dependencies
There is currently 1 downstream dependency for this package.

* `lidR` no problem reported with `devtools::revdep_check()`

