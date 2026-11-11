#ifndef RLAS_LAZBLOCKS_H
#define RLAS_LAZBLOCKS_H

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "lasreader.hpp"

namespace rlas
{

// Decodes one or several LAS/LAZ files with several threads and hands the points back, one by one
// and in the order of the files, to the main thread.
//
// A LAZ file is a sequence of chunks (50 000 points by default) that are compressed independently.
// The files are cut into numbered blocks of whole chunks (or of 50 000 points for a LAS file).
// Worker threads decode the blocks in any order and the consumer (next()) reads them in the order
// of their numbers:
//
//      worker 1 ─┐                       ┌─ block 0 ─┐
//      worker 2 ─┼─ decode ─> table of ──┼─ block 1 ─┼─> next() reads block 0, then 1, then 2...
//      worker 3 ─┘            blocks      └─ block 2 ─┘
//
// The workers only decompress: each one owns its LASreaderLAS (no filter, no transform) and never
// calls R. Filters, transforms and everything else stay on the main thread, where the points are
// consumed in the same order as the sequential reader. Only a small window of blocks is decoded
// ahead of the consumer, so the memory is bounded.
//
// The LASlib messages emitted by the workers are kept and printed by the main thread.
class LazBlocks
{
public:
  // Returns nullptr when the files cannot be read this way. 'reason' then says why, or is empty when
  // the files are just not worth the threads (too few blocks). 'main' is the reader opened by
  // RLASstreamer: its header and its point tell how the workers must decode the files.
  static std::unique_ptr<LazBlocks> create(const std::vector<std::string>& files, const LASreader& main, unsigned int threads, std::string& reason);
  ~LazBlocks();

  LazBlocks(const LazBlocks&) = delete;
  LazBlocks& operator=(const LazBlocks&) = delete;

  // Copies the next point into 'point' (the point of the main reader). false at the end of the files.
  // Throws the error of a worker where the sequential reader would have thrown it: after the points of
  // the failed block that were read before the error.
  bool next(LASpoint& point);

  std::uint64_t consumed() const { return consumed_points; }

  struct Source; // a file and how to decode it
  struct Block;  // the decoded points of a block
  struct Worker; // a reader and its position

private:
  LazBlocks() = default;

  bool prepare(const std::vector<std::string>& files, const LASreader& main, unsigned int threads, std::string& reason);
  bool start(std::string& reason);
  void stop();

  // Worker side
  void worker_loop(Worker* worker);
  bool take_next_block(std::size_t& index);
  void decode_block(Worker* worker, std::size_t index, Block* block) const;
  void publish_block(std::size_t index, Block* block);

  // Consumer side
  bool next_block();
  void print_messages();

  // The blocks
  struct Range { std::size_t source; std::int64_t first; std::int64_t count; };
  std::vector<Source> sources;
  std::vector<Range> ranges;
  std::size_t window = 0;

  // The layout of a point record in the blocks
  std::size_t core_size   = 0; // X to gps_time (included): every field the decoders write
  std::size_t record_size = 0;
  bool have_rgb           = false;
  bool have_wavepacket    = false;
  int extra_bytes_number  = 0;

  // Threads
  std::vector<std::unique_ptr<Worker>> workers;
  std::vector<std::thread> threads;

  // Shared by the workers and the consumer (protected by mutex)
  std::mutex mutex;
  std::condition_variable block_ready;  // a block was added to 'ready'
  std::condition_variable space_freed;  // the consumer moved on: a worker may take another block
  std::vector<std::unique_ptr<Block>> ready; // by block number: decoded and waiting, else empty
  std::size_t next_to_decode = 0;
  std::size_t consumer_block = 0;
  bool stopping = false;

  // Consumer only
  std::unique_ptr<Block> current;
  std::size_t cursor = 0;
  std::uint64_t consumed_points = 0;
  std::size_t truncated_source = SIZE_MAX; // a file that ended before its header said: its next blocks are skipped
  bool finished = false;
  bool capturing = false; // the LASlib message handler is ours
};

}

#endif // RLAS_LAZBLOCKS_H
