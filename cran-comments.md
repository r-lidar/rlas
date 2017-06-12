This is an early re-submission because the new version (1.1.3) I just released did not compile
on Mac OS flavors. This is due to a macro I previously defined to get a C++98 ISO code in former 
versions. The new version compiles in C++11 but the macro introduced compilation errors with some
flavors. The current submission (1.1.4) removes the former macro and uses only C++11 code.

I cannot ensure this will solve the problem since I was not able to reproduce the compilation
error (compilation ended without error even with the macro) but I am pretty confident it will work.

## Test environments
* Linux Mint 18 (Ubuntu 16.04), R 3.3.3, g++
* Linux Mint 18 (Ubuntu 16.04), R 3.3.3, clang++ -stdlib=libc++
* Linux Mint 18 (Ubuntu 16.04), R 3.3.3, clang++ -stdlib=gnu++11
* win-builder (release and devel), g++

## R CMD check results
There were no ERRORs or WARNINGs.

There were between 1 or 2 NOTEs depending on platforms:

* checking CRAN incoming feasibility ... NOTE

Identical with the previous release

* checking installed package size ... NOTE

Identical with the previous release

## Downstream dependencies
There is currently 1 downstream dependency for this package.

* `lidR` is my package so I can guarantee that there is no problem of compatibility.

