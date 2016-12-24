## Test environments
* Linux Mint 18 (Ubuntu 16.04), R 3.3.2, g++
* Linux Mint 18 (Ubuntu 16.04), R 3.3.2, clang++
* Fedora 25, R 3.3.2, g++
* Fedora 25, R 3.3.2, clang++
* Ubuntu 14.04 (on travis-ci), g++
* Windows 7 (on VM), R 3.3.2, g++
* Windows 7 (on VM), R 3.3.2, g++
* win-builder (release and devel), g++

## R CMD check results
There were no ERRORs or WARNINGs.

There were 2 NOTEs (only 1 on Windows platforms):

* checking CRAN incoming feasibility ... NOTE
Possibly mis-spelled words in DESCRIPTION:
  LiDAR (11:101)

`LiDAR`: is an acronym for 'light detection and ranging'. No confusion can happen with this well-known term.

* checking installed package size ... NOTE

The source code of the `LASlib` library is included and compiled into the package.
`LASlib` is not packaged for Linux nor MacOS and contains deprecated code.
I modified the code to be compatible both with R and ISO C++11.
See section `LASlib and `LASzip` at the end of this file for more informations

## Downstream dependencies
There are currently no downstream dependencies for this package

## LASlib and LASzip
LASlib and LASzip are two libraries developed by Martin Isenburg under LGPL 2.1. I included
the code into the package because the libraries are not packaged for any platform and must therefore
be compiled by hand. To be usable on all platforms by everybody I included the code to enable
automatic compilation during the installation.
Moreover, the original source code contained non-admissible statement such as 'srand', 'rand', 'stdout',
'stderr', exit(1) or non ISO code. I modified the code to be R compliant and ISO C++11 .

According to such considerations:
* I added Martin Isenburg as a copyright holder
* I documented my modifications according to the LGPL license recommendations and I registered my modifications by adding the date, my name, the line number and the reason of the modifications in the header of the modified files
* I used a GPL-3 license for the package which is compatible with LGPL.
