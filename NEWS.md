### rlas v1.1.1 (in development)


### rlas v1.1.0 (Release date: 2017-02-04)

#### NEW

* `readlasdata()` gains a parameter `filter` enabling to use memory optimized streaming filters.
* `readlasdata()` support .lax file for faster filter (thanks to Florian de Boissieu [#2](https://github.com/Jean-Romain/rlas/pull/2))

#### CHANGE

* All the default options for `readlasdata()` are now `TRUE`

#### FIXE:

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
