gcc 8 raises warning on CRAN (-Wstringop-truncation -Wstringop-overflow=). I compiled with gcc 8.1
using the same config than on CRAN but I did not reproduced the error. Thus, I can't swear that my 
fixes are correct but I'm confident.

## Test environments
* Linux Mint 18, R 3.4.4, g++ -std=gnu++11
* Linux Mint 18, R 3.4.4, g++-8 -std=gnu++11
* Linux Mint 18, R 3.4.4, clang++-4.0 -std=gnu++11
* windows via CRAN builder

## R CMD check results
There were no ERRORs or WARNINGs or NOTEs.

## Downstream dependencies
There is currently 1 downstream dependency for this package.

* `lidR` no problem reported with `devtools::revdep_check()`

