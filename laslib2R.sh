#!/bin/bash

# The LASlib and LASzip are open source librairies developped by Martin Isenburg to read and
# write las and laz file. I wrapped these libraries into R. However R does not like the way
# LASLib and LASzip are coded. R does not like rand and srand and exit(1). Moreover and the
# use of standard output (sdtout, stderr) are not correct in a library.

# The following regex enable to automatically correct some problems:

files=`grep -lr "fprintf" src/`

perl -pi -w -e 's/fprintf\(stderr,/REprintf(/g;' $files

files=`grep -lr "exit(1)" src/`

perl -pi -w -e 's/exit\(1\)/throw std::runtime_error\("Internal error"\)/g;' $files

# Some extra changes were done manually:

# fopen_compressed.cpp                #include <R.h> and #define STRICT_R_HEADERS
# mydefs.hpp                          #include <R.h> and #define STRICT_R_HEADERS
# lasreaderpipeon.cpp      l96-101    comment lines
# laswriter.cpp            l135-199   comment lines
# laswriter.cpp            l1037      #pragma GCC diagnostic push
#                                     #pragma GCC diagnostic ignored "-Wstringop-truncation"
#                                     strncpy(...)
#                                     #pragma GCC diagnostic pop
# lasattributer.cpp        l79-80     #pragma GCC diagnostic push
#                                     #pragma GCC diagnostic ignored "-Wstringop-truncation"
#                                     strncpy(...)
# laswriter_bin.cpp        l176       #pragma GCC diagnostic push
#                                     #pragma GCC diagnostic ignored "-Wstringop-overflow"
#                                     strncpy(...)
#                                     #pragma GCC diagnostic pop
# bytestreamount_file.hpp  l134       return (true)
# lastransform.cpp         l357       R::runif(0, RAND_MAX);
# lasfilter.cpp            l1298      R::runif(0, RAND_MAX);
# lasutility.cpp                      #include <stdexcept>

# R is happy now!

