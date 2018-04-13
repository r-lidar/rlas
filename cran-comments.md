This is an early release because ASAN and USBAN raised additionnal errors.

* clang ASAN/USBAN: the code now ensures that NAs will no be casted to int
* gcc ASAN: I disable minor features because I was not able to fix this error

To be honnest I can't swear that these additional errors are fixed because I was not able 
to reproduce them. I tried to reproduce with `rhub::check_with_sanitizer()` but tests were ok.
I have not been able to compile with sanitizer myself. Thus I only guess the errors are fixed.
I'm almost sure for the first one and I hope that the incriminated line was correctly given in the 
CRAN logs.

## Test environments
* Linux Mint 18 (Ubuntu 16.04), R 3.4.4, g++ -std=gnu++11
* Linux Mint 18 (Ubuntu 16.04), R 3.4.4, g++-7 -std=gnu++11
* osx release via r-hub.io builder
* windows via CRAN builder

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

