### rlas v1.3.0 (Release date: )

* New: improved support of LAS 1.4.
* New: introduction of set of functions to test compliance with LAS specifications
* Fix: segfault in `read.las` when using an inappropriated filter such as -keep_class 123 (see [#26](https://github.com/Jean-Romain/rlas/issues/26). Error is properly handled now.

### rlas v1.2.9 (Release date: 2018-10-26)

* `write.las` writes the epsg code in the header.

### rlas v1.2.6 (Release date: 2018-08-20)

#### ENHANCEMENTS

* `check_data` now check the adequation between the `ReturnNumber`and the `NumberOfReturns`

#### BUG FIXES:

* `las` format 7 was supported at the C++ level when reading a file but an old test still throw an error at the R level stating that this format was not supported.
* fix several compilation warnings on CRAN

#### OTHERS

`readlasdata`and `readlasheader` were definitively removed.

### rlas v1.2.4 (Release date: 2018-06-10)

* Reupload on CRAN after being removed.

### rlas v1.2.3 (Release date: 2018-05-17)

#### NEW FEATURES

* Read the classification flags in fields `Synthetic_flag`, `Keypoint_flag` and `Withheld_flag`.

#### BUG FIXES:

* Fix: point in polygon includes the points that define the polygon.
* Fix: [[#21](https://github.com/Jean-Romain/rlas/issues/21)] writing a vector that contains only NAs in extra bytes
* Fix: additional warnings on CRAN with gcc 8+
* Fix: [[#19](https://github.com/Jean-Romain/rlas/issues/19)] additional errors on CRAN with valgrind

### rlas v1.2.1 (Release date: 2018-04-13)

#### BUG FIXES:

* Fix: [[#18](https://github.com/Jean-Romain/rlas/issues/18)] additional errors on CRAN with ASAN and USBAN

### rlas v1.2.0 (Release date: 2018-04-10)

#### NEW FEATURES

* Enables the support of files with point data formats 6, 7 and 8 in read mode.
* Enables the support of extra bytes attributes in write mode.
* New function `read.las` that replaces `readlasdata`, which is now deprecated. The selection of the field to be loaded is made easier by use of string syntax instead of multiple logical parameters.
* New function `read.lasheader` that replaces `readlasheader`, which is now deprecated. This change was made only for naming consistency.
* New functions `header_*` that allows the user to make valid headers and thus properly write the las files.
* New functions `check_*` that check data integrity.

#### BUG FIXES:

* Fix [[#9](https://github.com/Jean-Romain/rlas/issues/9)] with valgrind: uninitialized value(s)

### rlas v1.1.9 (Release date: 2018-01-17)

#### BUG FIXES:

* Fix a segfault when reading extrabytes

### rlas v1.1.8 (Release date: 2018-01-15)

#### NEW FEATURES

* new straighforward C++ class `RLASstreamer` to use `rlas`at the C++ level.
* Support of extra bytes attributes in `readlasdata`

#### BUG FIXES:

* Fix some internal features that cannot be used by regular users. Does not affect users.

### rlas v1.1.5 (Release date: 2017-10-23)

#### NEW FEATURES

* `writelax` enabled for writing LAX files.
* `readlasdata` enabled for reading several files.
* Moved the headers in `inst/include` to make the lib callable from other packages

#### BUG FIXES:

* The las files were read twice every time. `lasdatareader` is twice as fast since it reads files only once
* `readlasdata` enables loading (or not) of the gpstime field.

### rlas v1.1.4 (Release date: 2017-07-26)

#### BUG FIXES:

* Solved the compilation issue on CRAN with clang++ and gnu++11

### rlas v1.1.3 (Release date: 2017-06-09)

#### BUG FIXES:

* Fix  [[#4](https://github.com/Jean-Romain/rlas/issues/4)] bug of computer precision when writing files 
* Fix  [[#61](https://github.com/Jean-Romain/lidR/issues/61)] wrong header size for files version 1.3

#### OTHER CHANGES

* Update LASlib and LASzip
* Better integration of LASlib and LASzip in R

### rlas v1.1.2 (Release date: 2017-05-04)

#### BUG FIXES:

* Write the proper RGB colors instead of 0.

### rlas v1.1.1 (Release date: 2017-03-15)

#### BUG FIXES:

* [[#3](https://github.com/Jean-Romain/rlas/issues/3)] `readlasdata()` is able to read file when LAS specifications are not respected.

#### OTHER CHANGES

* `writelas` performs tests on the header before entering C++ code (enable to fail with informative errors).

### rlas v1.1.0 (Release date: 2017-02-04)

#### NEW FEATURES

* `readlasdata()` gains a parameter `filter` enabling use of memory-optimized streaming filters.
* `readlasdata()` supports .lax files for faster filter (thanks to Florian de Boissieu [#2](https://github.com/Jean-Romain/rlas/pull/2))

#### OTHER CHANGES

* All the default options for `readlasdata()` are now `TRUE`

#### BUG FIXES:

* `readlasheader()` is now able to read the `Variable length record`

### rlas v1.0.3 (Release date: 2016-12-24)

CRAN does not build binary packages. After exchanges with prof Bryan Ripley

* Change: removed CXX_STD flag in Makevars. g++ compile whith flag -std=c++98
* Change: removed std::to_string, replaced by a macro ISO C++98
* Change: replaced formats specifiers which were ISO C++11 but not ISO C++98
* Change: added prepocessor statements to get proper path to libraries when clang++ is used with flag -stdlib=libc++
* Change: title in DESCRIPTION according to prof Bryan Ripley
* Change: description in DESCRIPTION according to prof Bryan Ripley

#### rlas v1.0.2 (Release date: 2016-12-23)

Third submission

* Change: Manually list sources in makevars
* Change: Remove SystemRequirements field in DESCRIPTION

#### rlas v1.0.1 (Release date: 2016-12-22)

Second submission

* Add: runnable examples in documentaion
* Add: tiny 755 bytes 'laz' file
* Change: single quoted 'las' and 'laz' in Description and Title fields in DESCRIPTION


#### rlas v1.0.0 (Release date: 2016-12-22)

First submission
