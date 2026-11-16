/*
 ===============================================================================

 PROGRAMMERS:

 info@r-lidar.com  -  https://github.com/r-lidar/rlas

 COPYRIGHT:

 Copyright 2026 Jean-Romain Roussel

 This file is part of rlas R package.

 rlas is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>

 ===============================================================================
 */

#ifndef RLAS_LAZBLOCKS_H
#define RLAS_LAZBLOCKS_H

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

class LASreader;
class LASreaderLAS;
class LASreadOpener;

namespace rlas
{

// Decodes the LAS/LAZ files of a LASreader with several threads and hands the points back one by
// one, in the order the reader would have read them.
//
// A LAZ file is a sequence of chunks compressed independently. The files are cut into numbered
// blocks of whole chunks. With a spatial index (LAX or COPC) a block only holds the intervals of
// points that the sequential reader would visit. The workers decode the blocks in any order and
// next() consumes them in the order of their numbers. Only a small window of blocks is decoded ahead
// of the consumer, so the memory is bounded.
//
// A worker only decompresses and applies the spatial query, which is independent of the order of the
// points. It never calls R: its LASlib messages are muted and its errors are thrown by next(). The
// filter and the transform of the reader may depend on the order: they are applied by next().
class LazBlocks
{
public:
  // 'main' is the reader opened by 'opener'. Throws std::runtime_error, which says why, when its
  // files cannot be decoded this way.
  LazBlocks(LASreadOpener& opener, LASreader& main, int threads);
  ~LazBlocks();

  LazBlocks(const LazBlocks&) = delete;
  LazBlocks& operator=(const LazBlocks&) = delete;

  // Reads the next point into the point of 'main', as main.read_point() does. false at the end.
  // Throws the error of a worker after the points that were read before it.
  bool next();

private:
  struct File
  {
    std::string path;
    bool rescale, reoffset; // the coordinates must be converted to the scales and offsets of 'main'
    bool indexed;           // read with a spatial index
  };

  struct Block
  {
    std::size_t file;
    std::vector<std::pair<std::int64_t, std::int64_t>> pieces; // (first point, number of points)
    std::int64_t count = 0;

    // Filled by a worker
    std::vector<unsigned char> data; // the records of the points in the query
    std::size_t kept = 0;
    std::string error;
    bool done = false;
  };

  void work();
  void decode(Block& block, LASreaderLAS& reader) const;
  void stop();

  LASreader& main;
  std::vector<File> files;
  std::vector<Block> blocks;
  std::size_t fields = 0; // bytes of the fields of a LASpoint that a record holds, before the extra bytes
  std::size_t stride = 0; // bytes of a record
  std::size_t window = 0;

  std::vector<std::thread> workers;
  std::mutex mutex;
  std::condition_variable wakeup;
  std::size_t next_job = 0; // next block to decode
  std::size_t current = 0;  // block read by next()
  std::size_t cursor = 0;   // next record of this block
  bool stopping = false;
};

}

#endif // RLAS_LAZBLOCKS_H
