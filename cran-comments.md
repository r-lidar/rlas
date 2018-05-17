I fixed additionnal errors from CRAN

 * valgrind issues were all fixed (at least on my computer I got 0 errors from 0 context)
 * gcc-8 warning were all false positive. I changed the code anyway but I don't have access to
   gcc-8 yet so I cannot swear I fixed each issue. I'm pretty confident.

## Test environments
* Linux Mint 18, R 3.4.4, 
    * g++-7 -std=gnu++11
    * g++-8 -std=gnu++11
    * clang++-3.8 -stdlib=libc++
    * clang++-4.0 -stdlib=libc++
* osx release via r-hub.io builder
* windows via CRAN builder

## R CMD check results
There were no ERRORs or WARNINGs or NOTEs.

## Downstream dependencies
There is currently 1 downstream dependency for this package.

* `lidR` no problem reported with `devtools::revdep_check()`

