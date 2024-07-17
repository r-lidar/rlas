![licence](https://img.shields.io/badge/Licence-GPL--3-blue.svg)
[![R build status](https://github.com/r-lidar/rlas/workflows/R-CMD-check/badge.svg)](https://github.com/r-lidar/rlas/actions)
[![Codecov test coverage](https://codecov.io/gh/r-lidar/rlas/branch/master/graph/badge.svg)](https://app.codecov.io/gh/r-lidar/rlas?branch=master)

R package to read and write `.las` and `.laz` binary files used to store LiDAR data.

This package is used by the [`lidR`](https://github.com/r-lidar/lidR) package and is not intended to be used by regular users. Everything you need is in the `lidR` package with a high-level front end. Only package developers who would like to build something new from a low-level API might be interested in `rlas`.

`rlas` relies on a modified version of `LASlib` and `LASzip` that were modified to be compatible with `R`. The library can therefore be compiled into `R` without any complaints from `R CMD check`. It enables R users to read and write binary files commonly used to store LiDAR data. LAS/LAZ version 1.0 to 1.4 are supported.

```r
library(rlas)
lasdata   <- read.las("<myfile.las>")
lasheader <- read.lasheader("<myfile.las>")
```

## Copyright Information

`rlas` contains code written  by both Jean-Romain Roussel and Martin Isenburg. The latter is included
for technical reasons. Details below.

* For `LASlib` and `LASzip`:
  - (c) 2007-2021 martin.isenburg@rapidlasso.com - http://rapidlasso.com
  - Provided under LGPL license and modified to be R-compliant by Jean-Romain Roussel.
* For `rlas` code enabling Martin Isenburg's code to be wrapped into R:
  - (c) 2016-2021 Jean-Romain Roussel
  - Provided under GPL-3 license.
