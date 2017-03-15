## Test environments
* Linux Mint 18 (Ubuntu 16.04), R 3.3.2, g++
* Linux Mint 18 (Ubuntu 16.04), R 3.3.2, clang++ -stdlib=libc++
* win-builder (release and devel), g++

## R CMD check results
There were no ERRORs or WARNINGs.

There were between 1 and 2 NOTEs depending on platforms:

* checking CRAN incoming feasibility ... NOTE

Identical with the previous release

* checking installed package size ... NOTE

Identical with the previous release

* checking compiled code ... NOTE
  Found no calls to: 'R_registerRoutines', 'R_useDynamicSymbols'
  
This is a new requierement of the new R-devel version. This note is definitively beyond my knowlegdes. The documention is too technical to me and all my tests, following threads from the R mailling list, failed. CRAN seems to still accept packages with this note. I will resolve this note in further version when nice knowlegeable people will have written more accessible documentation on internet.

## Downstream dependencies
There is currently 1 downstream dependency for this package.

* `lidR` is my package so I can guarantee that there is no problem of compatibility. Moreover reverse dependency test return 0 error, 0 warning, 0 note

