I finally figured out how to fix the warnings with gcc 7+ on fedora an debian flavors. 
Hopefully the CRAN logs where more informative than mine thus I found the true
incriminated lines of code. This release is expected to resolve the problem. My compilation 
logs do not show any -Wformat-overflow type warnings with gcc > 7.

## Test environments
* Linux Mint 18 (Ubuntu 16.04), R 3.4.3, g++ -std=gnu++11
* Linux Mint 18 (Ubuntu 16.04), R 3.4.3, g++-7 -std=gnu++11
* win-builder (release and devel), g++

## R CMD check results
There were no ERRORs or WARNINGs.

There was between 0 or 1 NOTE depending on platforms:

* checking CRAN incoming feasibility ... NOTE

Identical with the previous release

* checking installed package size ... NOTE

Identical with the previous release

## Downstream dependencies
There is currently 1 downstream dependency for this package.

* `lidR` no problem reported with `devtools::revdep_check()`

