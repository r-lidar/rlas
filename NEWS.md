### rlas v1.1.2 (Release date: 2017-06-09)

#### BUG FIXES:

* Fix  [[#4](https://github.com/Jean-Romain/rlas/issues/4)] bug computer precision when writing files 
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

* `readlasdata()` gains a parameter `filter` enabling to use memory optimized streaming filters.
* `readlasdata()` support .lax file for faster filter (thanks to Florian de Boissieu [#2](https://github.com/Jean-Romain/rlas/pull/2))

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
