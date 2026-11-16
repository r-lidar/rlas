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

#include "rlaslazblocks.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <utility>

#include "arithmeticdecoder.hpp"
#include "bytestreamin_file.hpp"
#include "integercompressor.hpp"
#include "lasmessage.hpp"
#include "lasreader_las.hpp"
#include "rlas_laslib.h"

// Defined in LASzip/lasmessage.cpp but not declared in its header: it prints with REprintf.
void las_default_message_handler(LAS_MESSAGE_TYPE type, const char* msg, void* user_data);

namespace rlas
{

// ── The small types of the class ──────────────────────────────────────────────────────────────────

// A file and how its points are decoded to match the main reader.
struct LazBlocks::Source
{
  std::string path;
  std::int64_t npoints = 0;
  bool rescale  = false;
  bool reoffset = false;
  double scale[3]  = {0, 0, 0};
  double offset[3] = {0, 0, 0};
};

// The points of a block, one record after the other (see LazBlocks::record_size).
struct LazBlocks::Block
{
  std::size_t expected = 0; // points the block should hold
  std::size_t points   = 0; // points really read: fewer = the file is shorter than its header says
  std::vector<std::uint8_t> data;
  std::vector<std::pair<LAS_MESSAGE_TYPE, std::string>> messages; // printed when the consumer reaches the block
  bool failed = false;
  std::string error;
};

// A reader of one of the files, with its own opener: the workers share nothing.
// Its readers are opened by the main thread before the threads start (see prepare()).
struct LazBlocks::Worker
{
  LASreadOpener opener;
  std::vector<LASreaderPtr> readers;   // one per file
  std::vector<std::int64_t> positions; // index of the next point each reader returns, -1 = unknown
};

namespace
{
  // A block is made of whole chunks and holds about this number of points.
  constexpr std::int64_t target_points = 50000;

  // ── LASlib messages ───────────────────────────────────────────────────────────────────────────────

  // LASlib prints its messages with REprintf, which must not be called from a worker thread. While the
  // workers run, the messages of the main thread are printed as usual. Those of a worker are attached
  // to the block it decodes and printed by the main thread when it reaches the block, i.e. when the
  // sequential reader would have printed them. The workers never open a file (see prepare()).
  struct MessageCapture
  {
    std::thread::id main_thread;
    bool active = false; // only one LazBlocks at a time owns the handler
  };

  MessageCapture capture;
  thread_local LazBlocks::Block* worker_block = nullptr;

  // The last error message of this worker. LASlib throws its errors with las_last_error(), a global
  // that another worker may have overwritten in the meantime: the error of a block is taken from here.
  thread_local std::string worker_error;

  constexpr std::size_t max_messages = 100;

  // At most this number of readers (threads x files) are open at a time.
  constexpr std::size_t max_readers = 256;

  void capture_handler(LAS_MESSAGE_TYPE type, const char* msg, void* user_data)
  {
    if (std::this_thread::get_id() == capture.main_thread)
    {
      las_default_message_handler(type, msg, user_data);
      return;
    }

    if (type >= LAS_ERROR)
      worker_error = msg;

    if (worker_block && worker_block->messages.size() < max_messages)
      worker_block->messages.emplace_back(type, msg);
  }

  // ── Chunks ────────────────────────────────────────────────────────────────────────────────────────

  // Reads and checks the chunk table of a LAZ file, as LASreadPoint::read_chunk_table() does (LASlib keeps
  // its copy private). 'counts' receives the number of points of each chunk when the chunk sizes are
  // variable (COPC included): a table with fixed chunk sizes stores only the byte sizes of the chunks.
  // false when the table is missing (e.g. a truncated file) or inconsistent with the file. LASzip must then
  // read the file sequentially: seeking in it would decode the chunks after the end of the data.
  bool read_chunk_table(const std::string& path, bool variable, U32& number_chunks, std::vector<std::int64_t>& counts)
  {
    FILE* file = LASfopen(path.c_str(), "rb");
    if (file == nullptr)
      return false;

    bool ok = false;

    try
    {
      ByteStreamInFileLE stream(file);

      if (!stream.seekEnd(0)) throw 1;
      const I64 file_size = stream.tell();

      // The offset to the point data as written in the file (LASreaderLAS adjusts the one of its header
      // when it removes VLRs). The start of the chunk table is the first thing there.
      U32 offset_to_point_data = 0;
      if (!stream.seek(96)) throw 1;
      stream.get32bitsLE((U8*)&offset_to_point_data);

      I64 table_start = 0;
      if (!stream.seek(offset_to_point_data)) throw 1;
      stream.get64bitsLE((U8*)&table_start);
      const I64 chunks_start = (I64)offset_to_point_data + 8;

      // The compressor wrote to a non-seekable stream: the start of the table is at the end of the file.
      if (table_start == -1)
      {
        if (!stream.seekEnd(8)) throw 1;
        stream.get64bitsLE((U8*)&table_start);
      }

      // The compressor was interrupted before writing the table, or the file is truncated.
      if (table_start <= chunks_start || table_start + 8 > file_size)
        throw 1;

      if (!stream.seek(table_start)) throw 1;

      U32 version = 0;
      stream.get32bitsLE((U8*)&version);
      stream.get32bitsLE((U8*)&number_chunks);
      if (version != 0 || number_chunks == 0 || number_chunks > 100000000)
        throw 1;

      counts.assign(variable ? number_chunks : 0, 0);

      ArithmeticDecoder dec;
      if (!dec.init(&stream)) throw 1;
      IntegerCompressor ic(&dec, 32, 2);
      ic.initDecompressor();

      U32 previous_count = 0;
      U32 previous_size  = 0;
      I64 end_of_chunks  = chunks_start;
      for (U32 i = 0 ; i < number_chunks ; i++)
      {
        if (variable)
        {
          previous_count = (U32)ic.decompress(i > 0 ? previous_count : 0, 0);
          counts[i] = previous_count;
        }
        previous_size = (U32)ic.decompress(i > 0 ? previous_size : 0, 1);
        if (previous_size == 0) throw 1;
        end_of_chunks += previous_size;
      }
      dec.done();

      // The chunks must end where the table starts.
      ok = (end_of_chunks <= table_start);
    }
    catch (...)
    {
      ok = false;
    }

    fclose(file);
    return ok;
  }

  // Cuts the chunks of a file into ranges of about 'target' points. A chunk cannot be decoded from its
  // middle, so it is never split: a big chunk makes a big range.
  void add_ranges(const std::vector<std::int64_t>& chunks, std::int64_t target, std::vector<std::pair<std::int64_t, std::int64_t>>& out)
  {
    std::int64_t first = 0;
    std::int64_t count = 0;

    for (std::int64_t chunk : chunks)
    {
      count += chunk;
      if (count >= target)
      {
        out.emplace_back(first, count);
        first += count;
        count = 0;
      }
    }

    if (count > 0)
      out.emplace_back(first, count);
  }

  // The chunks of a file: fixed size or variable size (read from the chunk table) for a LAZ file, small
  // blocks of 1000 points for a LAS file. false when they cannot be known or the file is incomplete.
  bool file_chunks(const std::string& path, const LASheader& header, std::int64_t npoints, std::vector<std::int64_t>& chunks)
  {
    chunks.clear();

    if (npoints == 0)
      return true;

    const LASzip* laszip = header.laszip;

    // LAS: the points must all be in the file (a seek beyond its end would not fail like a sequential read).
    if (laszip == nullptr)
    {
      FILE* file = LASfopen(path.c_str(), "rb");
      if (file == nullptr) return false;
      U32 offset_to_point_data = 0; // as written in the file
      const bool read = fseek_las(file, 96, SEEK_SET) == 0 && fread(&offset_to_point_data, 4, 1, file) == 1;
      fseek_las(file, 0, SEEK_END);
      const I64 file_size = ftell_las(file);
      fclose(file);

      if (!read || (I64)offset_to_point_data + npoints * (I64)header.point_data_record_length > file_size)
        return false;

      for (std::int64_t first = 0 ; first < npoints ; first += 1000)
        chunks.push_back(std::min<std::int64_t>(1000, npoints - first));
      return true;
    }

    // A chunk size of 0 makes the whole file one chunk: LASzip cannot seek in it (nor split it).
    if (laszip->chunk_size == 0)
    {
      chunks.push_back(npoints);
      return true;
    }

    const bool variable = laszip->chunk_size == U32_MAX;

    U32 number_chunks = 0;
    if (!read_chunk_table(path, variable, number_chunks, chunks))
      return false;

    if (variable)
    {
      std::int64_t total = 0;
      for (std::int64_t c : chunks) total += c;
      return total == npoints;
    }

    const std::int64_t size = laszip->chunk_size;
    if ((std::int64_t)number_chunks != (npoints + size - 1) / size)
      return false;

    for (std::int64_t first = 0 ; first < npoints ; first += size)
      chunks.push_back(std::min(size, npoints - first));
    return true;
  }

  // The reader of a file, built like LASreaderMerged and LASreadOpener build theirs: it converts the
  // coordinates to the scale factors and offsets of the main reader when they differ.
  LASreaderLAS* new_reader(LASreadOpener* opener, const LazBlocks::Source& s)
  {
    if (s.rescale && s.reoffset)
      return new LASreaderLASrescalereoffset(opener, s.scale[0], s.scale[1], s.scale[2], s.offset[0], s.offset[1], s.offset[2]);
    if (s.rescale)
      return new LASreaderLASrescale(opener, s.scale[0], s.scale[1], s.scale[2]);
    if (s.reoffset)
      return new LASreaderLASreoffset(opener, s.offset[0], s.offset[1], s.offset[2]);
    return new LASreaderLAS(opener);
  }

  bool same_layout(const LASpoint& a, const LASpoint& b)
  {
    return a.total_point_size    == b.total_point_size &&
           a.extended_point_type == b.extended_point_type &&
           a.have_gps_time       == b.have_gps_time &&
           a.have_rgb            == b.have_rgb &&
           a.have_nir            == b.have_nir &&
           a.have_wavepacket     == b.have_wavepacket &&
           a.extra_bytes_number  == b.extra_bytes_number;
  }

  constexpr U16 gps_encoding_bits = (1 << LAS_TOOLS_GLOBAL_ENCODING_BIT_GPS_TIME_TYPE) | (1 << LAS_TOOLS_GLOBAL_ENCODING_BIT_TIME_OFFSET_FLAG);

  // ── A point record ────────────────────────────────────────────────────────────────────────────────

  // The record holds every field a decoder writes: X to gps_time (the legacy and the extended fields,
  // as the item readers write them), then RGB/NIR, the wave packet and the extra bytes. Copying the
  // fields, not the LAS record (LASpoint::copy_to()), keeps the legacy fields that the decoders derive
  // from LAS 1.4 points (e.g. return number, scan angle rank).
  std::size_t core_bytes(const LASpoint& p)
  {
    return static_cast<std::size_t>(reinterpret_cast<const U8*>(&p.gps_time) - reinterpret_cast<const U8*>(&p.X)) + sizeof(p.gps_time);
  }
}

// ── Life cycle ────────────────────────────────────────────────────────────────────────────────────────

LazBlocks::~LazBlocks()
{
  stop();

  if (capturing)
  {
    unset_las_message_handler();
    capture.active = false;
  }
}

std::unique_ptr<LazBlocks> LazBlocks::create(const std::vector<std::string>& files, const LASreader& main, unsigned int threads, std::string& reason)
{
  reason.clear();

#ifdef __EMSCRIPTEN__
  reason = "threads are not available on this platform";
  return nullptr;
#endif

  if (threads < 2)
    return nullptr;

  if (capture.active)
  {
    reason = "another file is already being decoded in parallel";
    return nullptr;
  }

  std::unique_ptr<LazBlocks> blocks(new LazBlocks());

  if (!blocks->prepare(files, main, threads, reason))
    return nullptr;

  if (!blocks->start(reason))
    return nullptr;

  return blocks;
}

// prepare() runs on the main thread, in three steps:
//   1. Files    each file is opened to check that it decodes like the main reader and to find its chunks.
//   2. Blocks   the chunks are grouped in blocks, unless the files are not worth it.
//   3. Workers  one per thread.
bool LazBlocks::prepare(const std::vector<std::string>& files, const LASreader& main, unsigned int nthreads, std::string& reason)
{
  const LASheader& mh = main.header;
  const LASpoint& mp  = main.point;

  // ── 1. Files ──────────────────────────────────────────────────────────────────────────────────────

  // The files were already opened by the main reader: their messages were printed then.
  const LAS_MESSAGE_TYPE level = get_message_log_level();
  set_message_log_level(LAS_QUIET);

  std::vector<std::vector<std::int64_t>> chunks(files.size());
  std::int64_t total = 0;

  try
  {
    for (std::size_t i = 0 ; i < files.size() ; i++)
    {
      LASreadOpener opener;
      LASreaderLAS* las = new LASreaderLAS(&opener);
      LASreaderPtr probe(las);

      if (!las->open(files[i].c_str()))
      {
        reason = "a file cannot be opened";
        break;
      }

      const LASheader& h = probe->header;

      if (h.point_data_format != mh.point_data_format || h.point_data_record_length != mh.point_data_record_length || !same_layout(probe->point, mp))
      {
        reason = "the files have different point formats";
        break;
      }

      if (h.number_attributes != mh.number_attributes)
      {
        reason = "the files have different extra bytes attributes";
        break;
      }

      bool same_attributes = true;
      for (int j = 0 ; j < h.number_attributes ; j++)
        same_attributes = same_attributes && std::memcmp(&h.attributes[j], &mh.attributes[j], 192) == 0;

      if (!same_attributes)
      {
        reason = "the files have different extra bytes attributes";
        break;
      }

      if ((h.global_encoding & gps_encoding_bits) != (mh.global_encoding & gps_encoding_bits) ||
          ((h.global_encoding & (1 << LAS_TOOLS_GLOBAL_ENCODING_BIT_TIME_OFFSET_FLAG)) && h.time_offset != mh.time_offset))
      {
        reason = "the files have different GPS time encodings";
        break;
      }

      Source s;
      s.path     = files[i];
      s.npoints  = probe->npoints;
      s.rescale  = h.x_scale_factor != mh.x_scale_factor || h.y_scale_factor != mh.y_scale_factor || h.z_scale_factor != mh.z_scale_factor;
      s.reoffset = h.x_offset != mh.x_offset || h.y_offset != mh.y_offset || h.z_offset != mh.z_offset;
      s.scale[0]  = mh.x_scale_factor; s.scale[1]  = mh.y_scale_factor; s.scale[2]  = mh.z_scale_factor;
      s.offset[0] = mh.x_offset;       s.offset[1] = mh.y_offset;       s.offset[2] = mh.z_offset;

      if (!file_chunks(files[i], h, s.npoints, chunks[i]))
      {
        reason = "the chunk table of a file is missing or the file is incomplete";
        break;
      }

      total += s.npoints;
      sources.push_back(std::move(s));
    }
  }
  catch (std::exception const& e)
  {
    reason = std::string("a file cannot be opened (") + e.what() + ")";
  }
  catch (...)
  {
    reason = "a file cannot be opened";
  }

  set_message_log_level(level);

  if (!reason.empty())
    return false;

  if (total != main.npoints)
  {
    reason = "the number of points of the files does not match the header";
    return false;
  }

  // ── 2. Blocks ─────────────────────────────────────────────────────────────────────────────────────

  // About target_points points per block, or less for a small cloud so that each thread gets a few
  // blocks (possible only when the chunks are small too).
  const std::int64_t target = std::max<std::int64_t>(1, std::min<std::int64_t>(target_points, total / (4 * (std::int64_t)nthreads)));

  std::vector<std::pair<std::int64_t, std::int64_t>> file_ranges;
  for (std::size_t i = 0 ; i < sources.size() ; i++)
  {
    file_ranges.clear();
    add_ranges(chunks[i], target, file_ranges);
    for (const auto& r : file_ranges)
      ranges.push_back({i, r.first, r.second});
  }

  if (ranges.size() < 2)
    return false; // not worth the threads

  core_size          = core_bytes(mp);
  have_rgb           = mp.have_rgb;
  have_wavepacket    = mp.have_wavepacket;
  extra_bytes_number = mp.extra_bytes_number;
  record_size        = core_size;
  if (have_rgb)               record_size += sizeof(mp.rgb);
  if (have_wavepacket)        record_size += sizeof(LASwavepacket);
  if (extra_bytes_number > 0) record_size += extra_bytes_number;

  ready.resize(ranges.size());

  // ── 3. Workers ────────────────────────────────────────────────────────────────────────────────────

  // Never more threads than blocks, nor more readers than max_readers. The window leaves each thread
  // one block to decode plus one waiting.
  std::size_t n = std::min<std::size_t>(nthreads, ranges.size());
  n = std::min<std::size_t>(n, std::max<std::size_t>(1, max_readers / sources.size()));
  if (n < 2)
  {
    reason = "too many files";
    return false;
  }
  window = 2 * n;

  // The readers are opened here, on the main thread and silently: LASlib counts its messages in global
  // variables (it summarises a message repeated more than 15 times), and the messages of opening a file
  // were already printed when the main reader opened it.
  set_message_log_level(LAS_QUIET);

  try
  {
    for (std::size_t i = 0 ; i < n ; i++)
    {
      std::unique_ptr<Worker> worker(new Worker());
      for (const Source& source : sources)
      {
        LASreaderLAS* reader = new_reader(&worker->opener, source);
        worker->readers.emplace_back(reader);
        if (!reader->open(source.path.c_str()))
          throw std::runtime_error("cannot open file");
      }
      worker->positions.assign(sources.size(), 0);
      workers.push_back(std::move(worker));
    }
  }
  catch (...)
  {
    reason = "a file cannot be opened";
  }

  set_message_log_level(level);

  return reason.empty();
}

bool LazBlocks::start(std::string& reason)
{
  capture.main_thread = std::this_thread::get_id();
  capture.active = true;
  set_las_message_handler(&capture_handler);
  capturing = true;

  try
  {
    for (auto& worker : workers)
      threads.emplace_back(&LazBlocks::worker_loop, this, worker.get());
  }
  catch (...)
  {
    // Threads cannot be created: the files are read sequentially. The destructor joins the ones started.
    reason = "threads cannot be created";
    return false;
  }

  return true;
}

// Tells the workers to stop and waits for them: they use this object.
void LazBlocks::stop()
{
  {
    std::lock_guard<std::mutex> lock(mutex);
    stopping = true;
  }
  block_ready.notify_all();
  space_freed.notify_all();

  for (auto& thread : threads)
  {
    if (thread.joinable())
      thread.join();
  }
  threads.clear();

  // The readers are closed here, on the main thread.
  workers.clear();
}

// ── Worker side ───────────────────────────────────────────────────────────────────────────────────────

// Each worker takes a block number, decodes the block, puts it in the table and starts again until
// there is no block left. An error is stored in its block with the points read before it: the consumer
// throws it after these points, and only if it reaches the block (the sequential reader may never reach
// it, e.g. after a file that ended early).
void LazBlocks::worker_loop(Worker* worker)
{
  std::size_t index = 0;
  while (take_next_block(index))
  {
    std::unique_ptr<Block> block(new Block());
    worker_block = block.get();
    worker_error.clear();

    try
    {
      decode_block(worker, index, block.get());
    }
    catch (std::exception const& e)
    {
      block->failed = true;
      block->error = worker_error.empty() ? e.what() : worker_error;
    }
    catch (...)
    {
      block->failed = true;
      block->error = "unknown error while decoding the file";
    }

    worker_block = nullptr;

    if (block->failed)
    {
      // The reader may be in any state: its next block seeks.
      worker->positions[ranges[index].source] = -1;
    }

    publish_block(index, block.release());
  }
}

// Gives the next block number, or false when there is none left (or the object is being destroyed).
// Waits while the workers are too far ahead of the consumer, which bounds the memory.
bool LazBlocks::take_next_block(std::size_t& index)
{
  std::unique_lock<std::mutex> lock(mutex);

  space_freed.wait(lock, [this] {
    return stopping || next_to_decode >= ranges.size() || next_to_decode < consumer_block + window;
  });

  if (stopping || next_to_decode >= ranges.size())
    return false;

  index = next_to_decode++;
  return true;
}

// Reads the points of a block with the reader of this worker. The block starts on a chunk boundary:
// the seek jumps to the chunk, nothing is decoded and thrown away. No lock: the worker only touches
// its own reader and its own block.
void LazBlocks::decode_block(Worker* worker, std::size_t index, Block* block) const
{
  const Range& range = ranges[index];
  const Source& source = sources[range.source];

  block->expected = static_cast<std::size_t>(range.count);
  block->data.resize(block->expected * record_size);

  LASreader* reader = worker->readers[range.source].get();
  std::int64_t& position = worker->positions[range.source];

  // 1. Go to the first point of the block (not needed when the previous block ended there).
  if (position != range.first)
  {
    position = -1;
    if (!reader->seek(range.first))
      return; // no point: the file is shorter than its header says
    position = range.first;
  }

  // The messages of LASlib count the points read: as if the file had been read up to here.
  reader->p_cnt = range.first;

  // 2. Read the points.
  const LASpoint& p = reader->point;
  const U8* core = reinterpret_cast<const U8*>(&p.X);

  for (std::size_t i = 0 ; i < block->expected ; i++)
  {
    if (!reader->read_point())
      break;

    U8* out = &block->data[i * record_size];
    std::memcpy(out, core, core_size);
    out += core_size;

    if (have_rgb)
    {
      std::memcpy(out, p.rgb, sizeof(p.rgb));
      out += sizeof(p.rgb);
    }

    if (have_wavepacket)
    {
      std::memcpy(out, &p.wavepacket, sizeof(LASwavepacket));
      out += sizeof(LASwavepacket);
    }

    if (extra_bytes_number > 0)
      std::memcpy(out, p.extra_bytes, extra_bytes_number);

    block->points++;
  }

  position += static_cast<std::int64_t>(block->points);

  // 3. Read one more point, as the sequential reader does after the last point of the block. LASzip
  //    checks that a chunk is intact only when it starts the next one (its position must match the chunk
  //    table), and checks the end of the encoding after the last point of the file: without this read, a
  //    corrupt chunk would be decoded silently. An error thrown here comes after the points of the block,
  //    the consumer throws it once it has read them. The point itself is not kept.
  if (block->points == block->expected)
  {
    position = -1; // the next block of this reader seeks
    reader->read_point();
  }
}

// Puts a decoded block in the table, at its number, and wakes the consumer.
void LazBlocks::publish_block(std::size_t index, Block* block)
{
  {
    std::lock_guard<std::mutex> lock(mutex);
    ready[index].reset(block);
  }
  block_ready.notify_all();
}

// ── Consumer side ─────────────────────────────────────────────────────────────────────────────────────

// Prints the messages of the worker that decoded the current block.
void LazBlocks::print_messages()
{
  for (const auto& m : current->messages)
    las_default_message_handler(m.first, m.second.c_str(), nullptr);
  current->messages.clear();
}

// Makes 'current' the next block, in the order of the files. false at the end of the files.
bool LazBlocks::next_block()
{
  std::unique_lock<std::mutex> lock(mutex);

  // 1. Done with the block: it is freed, the workers may go one block further.
  if (current)
  {
    if (current->points < current->expected)
      truncated_source = ranges[consumer_block].source;

    current.reset();
    consumer_block++;
    space_freed.notify_all();
  }

  // 2. The blocks of a file that ended early are skipped, as the sequential reader moves to the next file.
  while (consumer_block < ranges.size() && ranges[consumer_block].source == truncated_source)
  {
    block_ready.wait(lock, [this] { return ready[consumer_block] != nullptr; });
    ready[consumer_block].reset();
    consumer_block++;
    space_freed.notify_all();
  }

  // 3. End of the files.
  if (consumer_block >= ranges.size())
  {
    finished = true;
    return false;
  }

  // 4. Next block: wait for the workers to decode it, take it out of the table.
  block_ready.wait(lock, [this] { return ready[consumer_block] != nullptr; });
  current = std::move(ready[consumer_block]);
  cursor = 0;
  lock.unlock();

  print_messages();

  return true;
}

bool LazBlocks::next(LASpoint& point)
{
  if (finished)
    return false;

  // The current block is read to the end (or there is none yet): take the next one. A block can hold
  // no point (a short file), hence the loop.
  while (!current || cursor >= current->points)
  {
    // A block that failed holds the points read before the error: the error comes after them.
    if (current && current->failed)
    {
      finished = true;
      throw std::runtime_error(current->error);
    }

    if (!next_block())
      return false;
  }

  const U8* in = &current->data[cursor * record_size];
  std::memcpy(reinterpret_cast<U8*>(&point.X), in, core_size);
  in += core_size;

  if (have_rgb)
  {
    std::memcpy(point.rgb, in, sizeof(point.rgb));
    in += sizeof(point.rgb);
  }

  if (have_wavepacket)
  {
    std::memcpy(&point.wavepacket, in, sizeof(LASwavepacket));
    in += sizeof(LASwavepacket);
  }

  if (extra_bytes_number > 0)
    std::memcpy(point.extra_bytes, in, extra_bytes_number);

  cursor++;
  consumed_points++;
  return true;
}

}
