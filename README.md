![Version](http://img.shields.io/Version/1.0.0.png)  ![licence](https://img.shields.io/badge/Licence-GPL--3-blue.svg)

R package to read and write `las` and `laz` binary files used to store LiDAR data.

`rlas` relies on a modified version of the open source parts of [LAStools](https://github.com/LAStools/LAStools) v160730 (LASlib and LASzip). LASlib and LASzip were modified to be compatible with `R`. The library can therefore be compiled into `R` without any complain from `R CMD check`.
It enables to read and write binary files commonly used to store LiDAR data into R.

## Modifications made into LASlib and LASzip

* `exit(1)` is not correct in a library. Such code was removed and replaced by an exception.
* the use of `stderr` output is not permitted in `R`. Error messages now raise exceptions.
* the use of `stdout` is not permitted in `R`. Replaced by `Rcpp::Rcout` or commented when compilation of such changes fail on Windows.
* `%I64d` and `%lld` format specifiers did not not respect C++98 standard. Replaced by `%ld` and remove prepocessor statements specific to Windows.
* `long long` type does not respect C++98 standard. Replaced by `int64_t`.
* replace `__int64` type by `int64_t` and removed preprocessor statements specific to Windows. 
* `rand` and `srand` are deprecated. Replaced by `runif` from `Rcpp`.

## Copyrights for LASlib and LASzip

(c) 2007-2015 martin.isenburg@rapidlasso.com - http://rapidlasso.com
Provided under LGPL licence.
