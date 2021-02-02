### rlas v1.4.0 (Release date: )

- Fix: build failure with GCC 4.x
- Fix: gcc-asan signed integer overflow.
- Fix: when reading an non supported extra bytes attribute rlas used to print a warning message. It now throws a real warning.
- Change: use `tinytest` instead of `testthat`
- New: `read.lasheader()` now reads the extended variable length record (EVLR)
- New: `write.las()` now writes all GeoKeyDirectoryTag + GeoAsciiParamsTag + GeoDoubleParamsTag
- New: `write.las()` can now write WKT OGC CS in EVLR

### rlas v1.3.9 (Release date: 2021-01-11)

- Fix gcc-asan new-delete-type-mismatch error when reading las/laz with lax files that appeared with C++14 and later standard.

### rlas v1.3.8 (Release date: 2020-12-14)

- Switch to C++14 to use boost 1.75.0 (see [#49](https://github.com/Jean-Romain/rlas/issues/49))

### rlas v1.3.7 (Release date: 2020-11-11)

- Fix [#48](https://github.com/Jean-Romain/rlas/issues/48). `header_set_epsg()` incorrectly set the epsg code when the key 3072 is missing but the VLR is not empty

### rlas v1.3.6 (Release date: 2020-06-02)

- Fix: The function `is_valid_ReturnNumber()` wrongly triggered errors when checking if the ReturnNumber attribute is correct. It missed errors for LAS < 1.4 and triggered wrong errors for LAS 1.4 prf > 6. [#45](https://github.com/Jean-Romain/rlas/pull/45)
- Fix: The function `is_empty_point_cloud()` did not actually test what it was expected to test.
- Fix: `is_valid_scalefactors()` does not trigger any report for very fine scale factors such as 0.000001 that is a valid scale of long/lat coordinates.
- Fix: `is_valid_returnnumber()` does not complain for ReturnNumber = 0. This is valid but not compliant to the specs. 
- Fix: formats 5, 7 and 10 were missing in the list of formats valid for storing RGB
- Fix: format 10 was missing in the list of formats valid for storing NIR
- Fix: more tests and fixes relative to asprs specification
- New: new parameter transform to apply streaming transformations
- New: list of filters and transformations are available via `read.las(filter = "-h")` and  `read.las(transform = "-h")`
- Enhancement: las specification tests now check for NAs

### rlas v1.3.5 (Release date: 2020-02-04)

- Change: defunc deprecated functions `check_data`, `check_header` and `check_data_vs_header`.
- Fix: `is_valid_ScanAngle()` triggers an error if |a| > 196.6 instead of 180. 196.6 is the maximum writable angle but is not compliant with the specs ([#44](https://github.com/Jean-Romain/rlas/issues/44)).
- New: function `is_compliant_ScanAngle()` to check the compliance of the Scan Angle.

### rlas v1.3.4 (Release date: 2019-09-19)

- Fix: One Definition Rule (ODR) violation in LASzip (LTO issue)
- Note: use the function `tabulate()` instead of home made C++ code to count the return numbers. 

### rlas v1.3.3 (Release date: 2019-08-30)

* New: unlock reading `.ply` files in `read.las` and `read.lasheader`. This is an undocumented feature not actually intended to be used. ([#39](https://github.com/Jean-Romain/rlas/issues/40)).
* New: verbose option in `writelax()`.
* Fix: ([#40](https://github.com/Jean-Romain/rlas/issues/40)) in `read.las` and `read.lasheader` modifications in string encoding in R 3.5.0 generated issues with path containing special characters. Native encoding is now enforced.

### rlas v1.3.2 (Release date: 2019-04-26)

* Fix: [#33](https://github.com/Jean-Romain/rlas/issues/33) it is now allowed to write `ScanAngleRank` above 90 degrees but not above 127 degrees.
* Fix: [#34](https://github.com/Jean-Romain/rlas/issues/34) fix a misinterpretation of the LAS specification. Offset can be negative.
* Fix: [#35](https://github.com/Jean-Romain/rlas/issues/35) Erreur reading a specific laz file. The actual problem is unknown. This has been fixed by updating `LASlib`
* Fix: [#36](https://github.com/Jean-Romain/rlas/issues/35) Classification was zeroed when writing LAS1.4 prf 6. The actual problem is not known. This has been fixed by updating `LASlib`
* Fix: `ScanAngle` was rounded to integer for LAS 1.4 prf >= 6 before to be written. With the 0.006 factor conversion this lead to unrelated values when reading back a written file.
* Change: `header_create()` does not generates random UUID. The UUID is set to `"00000000-0000-0000-0000-000000000000"`. Dependence to `uuid` has been removed.
* Change: Dependancies to `sp`, `sf` and `rgeos` were no longer useful. `rlas` only depends on `data.table + Rcpp`.

### rlas v1.3.1 (Release date: 2019-02-08)

* Fix: valgrind - Conditionnal jump on uninitialized value (consequentless for users)
* Fix: valgrind - Invalid read of size 2
* Fix: USBAN-ASAN - heap-buffer-overflow on address 

### rlas v1.3.0 (Release date: 2019-02-03)

* New: `write.las` is 25% faster
* New: [#27](https://github.com/Jean-Romain/rlas/issues/27) improved support of LAS specifications including LAS 1.4.
* New: introduction of a set of 50+ functions to test compliance with LAS specifications
* New: introduction of a set of 5+ functions to modify a header easily repecting LAS specifications
* Fix: [#26](https://github.com/Jean-Romain/rlas/issues/26) segfault in `read.las` when using an inappropriated filter such as -keep_class 123.

### rlas v1.2.9 (Release date: 2018-10-26)

* New: `write.las` writes the epsg code in the header.

### rlas v1.2.6 (Release date: 2018-08-20)

* Change: `check_data` now check the adequation between the `ReturnNumber`and the `NumberOfReturns`
* Fix: `las` format 7 was supported at the C++ level when reading a file but an old test still throw an error at the R level stating that this format was not supported.
* Fix: several compilation warnings on CRAN
* Change: `readlasdata` and `readlasheader` were definitively removed.

### rlas v1.2.4 (Release date: 2018-06-10)

* Reupload on CRAN after being removed.

### rlas v1.2.3 (Release date: 2018-05-17)

* New: Read the classification flags in fields `Synthetic_flag`, `Keypoint_flag` and `Withheld_flag`.
* Fix: point in polygon includes the points that define the polygon.
* Fix: [[#21](https://github.com/Jean-Romain/rlas/issues/21)] writing a vector that contains only NAs in extra bytes
* Fix: additional warnings on CRAN with gcc 8+
* Fix: [[#19](https://github.com/Jean-Romain/rlas/issues/19)] additional errors on CRAN with valgrind

### rlas v1.2.1 (Release date: 2018-04-13)

* Fix: [[#18](https://github.com/Jean-Romain/rlas/issues/18)] additional errors on CRAN with ASAN and USBAN

### rlas v1.2.0 (Release date: 2018-04-10)

* New: enables the support of files with point data formats 6, 7 and 8 in read mode.
* New: enables the support of extra bytes attributes in write mode.
* New: function `read.las` that replaces `readlasdata`, which is now deprecated. The selection of the field to be loaded is made easier by use of string syntax instead of multiple logical parameters.
* New: function `read.lasheader` that replaces `readlasheader`, which is now deprecated. This change was made only for naming consistency.
* New: functions `header_*` that allows the user to make valid headers and thus properly write the las files.
* New: functions `check_*` that check data integrity.
* Fix [[#9](https://github.com/Jean-Romain/rlas/issues/9)] with valgrind: uninitialized value(s)

### rlas v1.1.9 (Release date: 2018-01-17)

* Fix: segfault when reading extrabytes

### rlas v1.1.8 (Release date: 2018-01-15)

* New: support of extra bytes attributes in `readlasdata`
* Fix: some internal features that cannot be used by regular users. Does not affect users.

### rlas v1.1.5 (Release date: 2017-10-23)

* New: function `writelax` for writing LAX files.
* New: function `readlasdata` enable for reading several files.
* Fix: the las files were read twice every time. `lasdatareader` is twice as fast since it reads files only once
* Fix: `readlasdata` enables loading (or not) of the gpstime field.

### rlas v1.1.4 (Release date: 2017-07-26)

* Fix: solved the compilation issue on CRAN with clang++ and gnu++11

### rlas v1.1.3 (Release date: 2017-06-09)

* Fix: [[#4](https://github.com/Jean-Romain/rlas/issues/4)] bug of computer precision when writing files 
* Fix: [[#61](https://github.com/Jean-Romain/lidR/issues/61)] wrong header size for files version 1.3
* Change: update LASlib and LASzip
* Change: better integration of LASlib and LASzip in R

### rlas v1.1.2 (Release date: 2017-05-04)

* Fix: write the proper RGB colors instead of 0.

### rlas v1.1.1 (Release date: 2017-03-15)

* Fix: [[#3](https://github.com/Jean-Romain/rlas/issues/3)] `readlasdata()` is able to read file when LAS specifications are not respected.
* Change: `writelas` performs tests on the header before entering C++ code (enable to fail with informative errors).

### rlas v1.1.0 (Release date: 2017-02-04)

* New: `readlasdata()` gains a parameter `filter` enabling use of memory-optimized streaming filters.
* New: `readlasdata()` supports .lax files for faster filter (thanks to Florian de Boissieu [#2](https://github.com/Jean-Romain/rlas/pull/2))
* Change: all the default options for `readlasdata()` are now `TRUE`
* Fix: `readlasheader()` is now able to read the `Variable length record`

### rlas v1.0.3 (Release date: 2016-12-24)

CRAN does not build binary packages. After exchanges with prof Bryan Ripley

* Change: removed CXX_STD flag in Makevars. g++ compile whith flag -std=c++98
* Change: removed std::to_string, replaced by a macro ISO C++98
* Change: replaced formats specifiers which were ISO C++11 but not ISO C++98
* Change: added prepocessor statements to get proper path to libraries when clang++ is used with flag -stdlib=libc++
* Change: title in DESCRIPTION according to prof Bryan Ripley
* Change: description in DESCRIPTION according to prof Bryan Ripley

### rlas v1.0.2 (Release date: 2016-12-23)

Third submission

* Change: Manually list sources in makevars
* Change: Remove SystemRequirements field in DESCRIPTION

### rlas v1.0.1 (Release date: 2016-12-22)

Second submission

* Add: runnable examples in documentaion
* Add: tiny 755 bytes 'laz' file
* Change: single quoted 'las' and 'laz' in Description and Title fields in DESCRIPTION


### rlas v1.0.0 (Release date: 2016-12-22)

First submission
