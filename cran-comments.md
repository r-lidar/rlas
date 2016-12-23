## Test environments
* Linux Mint 18 (Ubuntu 16.04), R 3.3.2
* Ubuntu 14.04 (on travis-ci)
* Windows 7 (on VM), R 3.3.2
* win-builder (release and devel)

## R CMD check results
There were no ERRORs or WARNINGs.

There were 3 NOTEs (only 2 on Windows platforms):

* checking CRAN incoming feasibility ... NOTE
Maintainer: 'Jean-Romain Roussel <jean-romain.roussel.1@ulaval.ca>'

New submission

Possibly mis-spelled words in DESCRIPTION:
  LiDAR (11:101)
  las (3:23, 11:55)
  laz (3:31, 11:63)

`LiDAR`: is an acronym for 'light detection and ranging'. No confusion can happen with this well-known term.
`las` and `laz`: are two formats of standardized binary files used to store LiDAR data.

* checking installed package size ... NOTE
  installed size is  6.7Mb
  sub-directories of 1Mb or more:
    libs   6.6Mb

The source code of the `LASlib` library is included and compiled into the package.
`LASlib` is not packaged for Linux nor MacOS and contains deprecated code.
I modified the code to be compatible both with R and ISO C++11.
See section `LASlib and `LASzip` at the end of this file for more informations


* checking for GNU extensions in Makefiles ... NOTE
GNU make is a SystemRequirements.

GNU make serves only to use the wildcard both in makevar and makevar.win
Nobody has reported an issue relative to GNU make on Windows.

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
