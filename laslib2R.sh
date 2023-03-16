#!/bin/bash

# The LASlib and LASzip are open source librairies developped by Martin Isenburg to read and
# write las and laz file. I wrapped these libraries into R. However R does not like the way
# LASLib and LASzip are coded. R does not like rand and srand and exit(1) and some other stuff.
# The following regex enable to automatically correct some problems:

files=`grep -lr "fprintf" src/`
perl -pi -w -e 's/fprintf\(stderr,/REprintf(/g;' $files

files=`grep -lr "exit(1)" src/`
perl -pi -w -e 's/exit\(1\)/throw std::runtime_error\("Internal error"\)/g;' $files

# 2022 dec 20 new sprintf issues on CRAN
# =======================================
grep -lr "return sprintf(string" src/*/*[hc]pp | xargs sed -i 's/return sprintf(string/return snprintf(string, 256/g'
grep -lr "sprintf(last_warning" src/*/*[hc]pp | xargs sed -i 's/sprintf(last_warning/snprintf(last_warning, 128/g'
grep -lr "sprintf(last_error" src/*/*[hc]pp | xargs sed -i 's/sprintf(last_error/snprintf(last_error, 128/g'
grep -lr "sprintf(header.system_identifier" src/*/*[hc]pp | xargs sed -i 's/sprintf(header.system_identifier/snprintf(header.system_identifier, 32/g'
grep -lr "sprintf(header.system_identifier" src/*/*[hc]pp | xargs sed -i 's/sprintf(header.system_identifier/snprintf(header.system_identifier, 32/g'
grep -lr "sprintf(header.generating_software" src/*/*[hc]pp | xargs sed -i 's/sprintf(header.generating_software/snprintf(header.generating_software, 32/g'
grep -lr "sprintf(description" src/*/*[hc]pp | xargs sed -i 's/sprintf(description/snprintf(description, 32/g'
grep -lr "sprintf(vlrs\[i\].description" src/*/*[hc]pp | xargs sed -i 's/sprintf(vlrs\[i\].description/snprintf(vlrs\[i\].description, 32/g'
grep -lr "sprintf(evlrs\[i\].description" src/*/*[hc]pp | xargs sed -i 's/sprintf(evlrs\[i\].description/snprintf(evlrs\[i\].description, 32/g'
grep -lr "sprintf(lax_evlr.user_id" src/*/*[hc]pp | xargs sed -i 's/sprintf(lax_evlr.user_id/snprintf(lax_evlr.user_id, 16/g'
grep -lr "sprintf(lax_evlr.description" src/*/*[hc]pp | xargs sed -i 's/sprintf(lax_evlr.description/snprintf(lax_evlr.description, 32/g'
sed -i 's/sprintf(string/snprintf(string, 512/g' src/LASlib/laswriter_wrl.cpp
sed -i 's/sprintf(string/snprintf(string, 512/g' src/LASlib/laswriter_txt.cpp
sed -i 's/sprintf(string/snprintf(string, 512/g' src/LASlib/lasutility.cpp

# put 0 in unparse() functions because I don't know what to put. Unused anyway.
sed -i 's/sprintf(\&string\[n\]/snprintf(\&string\[n\], 0/g' src/LASlib/lasutility.cpp
sed -i 's/sprintf(\&string\[n\]/snprintf(\&string\[n\], 0/g' src/LASlib/lasignore.cpp
sed -i 's/n += sprintf(string + n/n += snprintf(string + n, 0/g' src/LASlib/lasreader.cpp
sed -i 's/n = sprintf(string/n = snprintf(string, 0/g' src/LASlib/lasreader.cpp
sed -i 's/sprintf(temp/snprintf(temp, 32/g' src/LASlib/lasreader_ply.cpp
sed -i 's/sprintf(temp/snprintf(temp, 32/g' src/LASlib/lasreader_txt.cpp

# occurences in laszip.dll must be done by hand (easy)
# some remaining occurencesdone by hand in lasfilter.cpp
# two occurences done by hand in lasreader.cpp
# one occurence done by hand in lastransform.cpp
# one occurence done by hand in laswaveform13writer.cpp
# five occurences done by hand in laswriter.cpp (no trivial)
# fe occurences done by hand in laswrite_qfit.cpp and laswriter_txt.cpp
# =================================================

# Some extra changes were done manually:

# Compile with g++7 and -flto -Wlto-type-mismatch -Wodr -Wall -pedantic -mtune=native -Wno-ignored-attributes -Wno-deprecated-declarations -Wno-parentheses

# fopen_compressed.cpp                #include <R.h> and #define STRICT_R_HEADERS
# mydefs.hpp                          #include <R.h> and #define STRICT_R_HEADERS
# lasreaderpipeon.cpp      l96-101    comment lines because of stdout
# laswriter.cpp            l139-204   comment lines because of stdout (I guess)
# laswriter.cpp            l1130,1135 for gcc 8+ replace strncpy by memcpy
# lasattributer.hpp        l79-80     for gcc 8+ replace strncpy by memcpy - requires to get the char* length:
#     int len = 0 ; while(*(description+len) != '\0' && len < 32) len++;
#     memcpy(this->description, description, len);
# lasdefinition.hpp        l570       cast to void* : memset((void*)&(vlrs[i]), 0, sizeof(LASvlr));
# lasdefinition.hpp        l572,697   for gcc 8+ replace strncpy by memcpy in the same way than lasattributer.hpp
# laswriter_bin.cpp        l173       for gcc 8+ replace strncpy by memcpy
# bytestreamount_file.hpp  l134       return (true) to get rid of stdout
# lastransform.cpp         l584       R::runif(0, RAND_MAX);
# lasfilter.cpp            l1310      R::runif(0, RAND_MAX);
# lasutility.cpp                      #include <stdexcept>
# lasinterval.cpp          l552       delete ((LASintervalStartCell*)previous_cell); for gcc-asan fix in lax files
# lasquadtree.cpp          l1662      for integer overflow
#    if (l < 16)
#      level_offset[l+1] = level_offset[l] + ((1<<l)*(1<<l));
#    else
#      level_offset[l+1] = level_offset[l];
# bytestreamin_file.hpp    l153,166   off_t -> off64_t (see #50)
# bytestreamout_file.hpp   l152,163   off_t -> off64_t (see #50)
# lasreader.cpp            l1875,1880 add (min_y != 0 || max_y != 0) to allow -inside 0 0 0 0 in lidR
# lasreader_txt.cpp        l1833 1851 I32 -> U32
# lasreader_ply.cpp        l1485 1503 I32 -> U32
# Fix various -Wempty-body caused by if(fget(...)); with clang++
# CRAN is happy now!

# In addition:
# - Tiago added the support of a new filter: see issue #32

