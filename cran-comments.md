Second submission. I added some reference for the format specifications as requiered.
As discussed I'm not able to reproduce the warnings reported on CRAN for debian and fedora. 
I enforced several compilers and R CMD check never reported any warning.

## Test environments
* Linux Mint 18 (Ubuntu 16.04), R 3.4.3, g++ -std=gnu++11
* Linux Mint 18 (Ubuntu 16.04), R 3.4.3, g++-5 -std=gnu++11
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

