![licence](https://img.shields.io/badge/Licence-GPL--3-blue.svg)

R package to read and write `.las` and `.laz` binary files used to store LiDAR data.

`rlas` relies on a modified version of the open source parts of [LAStools](https://github.com/LAStools/LAStools). `LASlib` and `LASzip` were modified to be compatible with `R`. The library can therefore be compiled into `R` without any complaints from `R CMD check`.
It enables users to read and write into R binary files commonly used to store LiDAR data in R both at the R level and at the C++ level. LAS version 1.0 to 1.4 are supported. Point data record format 0,1,2,3,6,7,8 are supported.

```r
library(rlas)
lasdata   = read.las("<myfile.las>")
lasheader = read.lasheader("<myfile.las>")
```

## Install `rlas`

* The latest released version from CRAN with

```r
install.packages("rlas")
```

* The latest development version from github with

```r
devtools::install_github("Jean-Romain/rlas")
```

To install the package from github make sure you have a working development environment.

* **Windows**: Install [Rtools.exe](https://cran.r-project.org/bin/windows/Rtools/).  
* **Mac**: Install `Xcode` from the Mac App Store.
* **Linux**: Install the R development package, usually called `r-devel` or `r-base-dev`

## Changelog

[See changelog here](https://github.com/Jean-Romain/rlas/blob/master/NEWS.md)

## Copyright Information

`rlas` contains code written  by both Jean-Romain Roussel and Martin Isenburg. The latter is included
for technical reasons. Details below.

* For `LASlib` and `LASzip`:
  - (c) 2007-2015 martin.isenburg@rapidlasso.com - http://rapidlasso.com
  - Provided under LGPL license and modified to be R-compliant by Jean-Romain Roussel. Each modification is dated and registered into the file headers.
* For `rlas` code enabling Martin Isenburg's code to be wrapped into R:
  - (c) 2016 Jean-Romain Roussel
  - Provided under GPL-3 license.
