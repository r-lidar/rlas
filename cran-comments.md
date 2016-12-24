## Test environments
* Linux Mint 18 (Ubuntu 16.04), R 3.3.2, g++
* Linux Mint 18 (Ubuntu 16.04), R 3.3.2, clang++ -stdlib=libc++
* Fedora 25, R 3.3.2, g++
* Fedora 25, R 3.3.2, clang++ -stdlib=libc++
* Ubuntu 14.04 (on travis-ci), g++
* Windows 7 (on VM), R 3.3.2, g++
* win-builder (release and devel), g++

## Comments
Update after prof Brian Ripley's request. Package was accepted by CRAN but did not compile
with clang++ on Fedora and MacOS. Furthermore win-builder accepted the package compiling 
it with C++11 as stated in Makevars but CRAN compiled it on Windows with C++98. Correction
were made to make the code ISO C++98 and compilable with clang++ using libc++.
Title chanded on Brian Ripley's demand
Description chanded on Brian Ripley's demand

## R CMD check results
There were no ERRORs or WARNINGs.

There were 2 NOTEs (1 on Windows platforms):

Possibly mis-spelled words in DESCRIPTION:
  LAS (12:70, 12:188, 12:357)
  LAZ (12:287)
  Photogrammetry (12:248)
  lossless (12:318)

`LAZ` and `LAS`: are two files formats
`Photogrammetry`: this word exists
`lossless`: this word exists

## Downstream dependencies
There are currently no downstream dependencies for this package
