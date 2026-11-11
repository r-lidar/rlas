#!/bin/bash

# Updates the bundled LASlib and LASzip from LAStools (https://github.com/LAStools/LAStools).
#
# LASlib and LASzip are developed by Martin Isenburg and rapidlasso GmbH. R does not accept some of
# their code (printing to stdout/stderr, exit(), rand(), sprintf()...) and rlas carries a few fixes and
# features that are not upstream. The update is done in three steps:
#
#   1. copy the upstream sources that rlas compiles into src/LASlib and src/LASzip
#   2. apply the mechanical changes below (sed/perl)
#   3. apply laslib2R.patch, the hand-written rlas changes
#
# Usage, from the root of the package:
#
#   git clone https://github.com/LAStools/LAStools.git /tmp/LAStools
#   ./laslib2R.sh /tmp/LAStools
#
# If the patch does not apply any more, fix the rejected hunks by hand, then regenerate the patch with
#   ./laslib2R.sh /tmp/LAStools --make-patch
# which rebuilds steps 1 and 2 in a temporary directory and diffs them against src/.
# If upstream added or removed a .cpp, update SOURCES in src/Makevars.in and src/Makevars.ucrt.
#
# Content of laslib2R.patch (search "rlas" in the sources to find each change):
#
#   R policies
#   - lasmessage.cpp: messages are printed with REprintf(). The last error is kept for byebye().
#   - mydefs.cpp: byebye() throws std::runtime_error instead of calling exit(). halt_on_error stays TRUE
#     because LASlib relies on laserror() stopping the caller (many laserror() are not followed by return).
#   - lasprogress.hpp, lasreadpoint.cpp, lasfilter.cpp: no stderr/std::cerr.
#   - laswriter.cpp, lasreaderpipeon.cpp, bytestreamout_file.hpp: no stdout.
#   - lasfilter.cpp, lastransform.cpp: #include <R_ext/Random.h> for unif_rand() (step 2).
#   - mydefs.hpp: #include <R_ext/Print.h>. mydefs.cpp: getcwd() result is checked (-Wunused-result).
#   Portability
#   - mydefs.hpp: I8 is signed char (char is unsigned on arm64), CHAR for strings in lasreader_bin.cpp,
#     laswriter_bin.cpp and laswaveform13writer.cpp.
#   rlas features
#   - Remote files through GDAL's virtual file system: is_remote_path() (mydefs.hpp), LASreaderLAS::open()
#     and bytestreamin_gdal.hpp (rlas file, not upstream).
#   - LASreadOpener::add_file_name() adds the path verbatim (no glob()) like the former LASlib.
#   - '-thin_with_voxel' filter (lasfilter.cpp, Tiago de Conto).
#   - '-inside 0 0 0 0' is allowed (lidR uses it).
#   Bug fixes not upstream
#   - Several ExtraBytes VLRs are merged instead of the last one replacing the others (lasreader_las.cpp).
#   - remove_evlr() leaves no dangling pointer (lasdefinitions.hpp).
#   - ~LASinterval() deletes the start cell with its own type (ASAN new-delete-type-mismatch).
#   - LASreadOpener::open() no longer resets '-max_depth' and '-resolution' set by parse().

set -euo pipefail

UPSTREAM=${1:?usage: ./laslib2R.sh /path/to/LAStools [--make-patch]}
MODE=${2:-}
UPSTREAM=$(cd "$UPSTREAM" && pwd)
PKG=$(cd "$(dirname "$0")" && pwd)

# Files of LAStools that rlas does not compile: DLL wrappers, demzip, unused headers
EXCLUDE="demzip_dll.cpp demzip_api.h laszip_dll.cpp lasunzipper.cpp lasunzipper.hpp laszipper.cpp laszipper.hpp lasvlr.cpp endian.hpp"

# 1. copy the upstream sources into <dest>/LASlib and <dest>/LASzip
copy_upstream()
{
  local dest=$1
  mkdir -p "$dest/LASlib" "$dest/LASzip"
  find "$dest/LASlib" "$dest/LASzip" -type f ! -name bytestreamin_gdal.hpp -delete
  cp "$UPSTREAM"/LASlib/src/*.cpp "$UPSTREAM"/LASlib/inc/*.hpp "$UPSTREAM"/LASlib/inc/*.h "$dest/LASlib/"
  cp "$UPSTREAM"/LASzip/src/*.cpp "$UPSTREAM"/LASzip/src/*.hpp "$UPSTREAM"/LASzip/include/laszip/laszip_common.h "$dest/LASzip/"
  for f in $EXCLUDE; do rm -f "$dest/LASlib/$f" "$dest/LASzip/$f"; done
}

# 2. mechanical changes
mechanical()
{
  local dest=$1

  # sprintf() is deprecated on macOS: snprintf() with the size of the command buffers of get_command()
  # and unparse() (256, see LAStransform::check_for_overflow()). rlas never calls unparse().
  perl -pi -e 's/\bsprintf\((string|&string\[n\]|string \+ n),/snprintf($1, 256,/g' "$dest"/LASlib/*.cpp "$dest"/LASlib/*.hpp

  # R's RNG instead of srand()/rand(): -keep_random_fraction and -jitter_xy follow set.seed()
  perl -0pi -e 's/srand\(seed\);\s*\n(\s*)seed = rand\(\);/seed = unif_rand()*RAND_MAX; \/\/ rlas: R\x27s RNG instead of rand()/g' "$dest"/LASlib/lasfilter.cpp "$dest"/LASlib/lastransform.cpp

  # -Wunused-result on fgets()
  perl -pi -e 's/(^\s*|\)\s+)fgets\(([^()]*)\);/$1if (fgets($2) == NULL) {}/g' "$dest"/LASlib/*.cpp
}

if [ "$MODE" = "--make-patch" ]
then
  tmp=$(mktemp -d)
  copy_upstream "$tmp/src"
  mechanical "$tmp/src"
  cp "$PKG/src/LASzip/bytestreamin_gdal.hpp" "$tmp/src/LASzip/" 2>/dev/null || true
  (cd "$tmp" && mkdir a b && mv src a/ && cp -r "$PKG/src" b/src && rm -f b/src/*.[ch]* b/src/Makevars* && \
   find b/src -name '*.o' -delete && \
   diff -ruN a/src b/src > "$PKG/laslib2R.patch" || true)
  rm -rf "$tmp"
  echo "laslib2R.patch regenerated ($(grep -c '^@@' "$PKG/laslib2R.patch") hunks)"
  exit 0
fi

copy_upstream "$PKG/src"
mechanical "$PKG/src"
(cd "$PKG" && patch -p1 --no-backup-if-mismatch < laslib2R.patch)

echo "LASlib and LASzip updated from $(git -C "$UPSTREAM" rev-parse --short HEAD 2>/dev/null || echo "$UPSTREAM")"
