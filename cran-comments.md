This release fixes 2 issues detected by valgrind

-  Invalid read of size 1

This was due to the .las files provided in the package that do not strictly respect format specification and some string were not null-terminated. null-terminated string is now tested and if a string is not null-terminated it is reallocated and extended to be null-terminated.

- Conditional jump or move depends on uninitialised value(s)

A boolean was not initialized in some specific cases. This was all my bad but consequentless for the output. This was fixed.

Other issues with USBAN and USAN are not reproducible to me but they concern the same lines of code so I'm sure they have been fixed.

- ERROR: AddressSanitizer: heap-buffer-overflow on address 

Is due to not null-terminated arrays read from file.

## Test environments
* Linux Mint 18, R 3.4.4, g++ -std=gnu++11
* Windows via CRAN builder, R 3.5.1, R 3.6

## R CMD check results
There were no ERRORs or WARNINGs or NOTEs.

## Downstream dependencies
There is currently 2 downstream dependency for this package.

* `lidR` no problem reported
* `uavRst` no problem reported
