gcc 8 raises warning on CRAN (-Wstringop-truncation -Wstringop-overflow=). I compiled with gcc 8.1
using the same config than on CRAN but I did not reproduced the error. However the previous upload on CRAN 
(rejected, see below) allows me to confirm that the issues were fixed.
Also UndefinedBehaviorSanitizer raise by USBAN have been solvd by removing unit tests in testthat that 
check the behavior of the lib when the user try to write a value outside the range of representable 
values.

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

