#ifndef LASSTREAMER_H
#define LASSTREAMER_H

#include <Rcpp.h>

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lasfilter.hpp"
#include "laswaveform13reader.hpp"
#include "rlas_laslib.h"
#include "rlasextrabytesattributes.h"
#include "rlaslazblocks.h"

// A column of point attributes that is very often constant (e.g. ScanDirectionFlag,
// UserData, PointSourceID). It stores a single value until a point with a different
// value is seen. Only then the column is expanded to one value per point. A column that
// is never expanded is returned to R as a length-1 vector that will be ALTREPed.
template <typename T>
class LazyColumn
{
public:
  // Records the value of the first point. Must be called once, before the first push().
  void init(T first) { values.assign(1, first); expanded = false; }

  // 'npoints' is the number of points already read including the current one and
  // 'capacity' is the expected final number of points.
  template <typename U>
  void push(U value, size_t npoints, size_t capacity)
  {
    if (!expanded && value != values[0])
    {
      expanded = true;
      T first = values[0];
      values.reserve(capacity);
      values.insert(values.end(), npoints - 2, first);
    }

    if (expanded)
      values.push_back(value);
  }

  std::vector<T> values;

private:
  bool expanded = false;
};

// Which point attributes are loaded in R. Parsed from the 'select' string of read.las()
struct Selection
{
  bool gpstime            = true;
  bool intensity          = true;
  bool return_number      = true;
  bool number_of_returns  = true;
  bool scan_direction     = true;
  bool edge_of_flightline = true;
  bool classification     = true;
  bool synthetic          = true;
  bool keypoint           = true;
  bool withheld           = true;
  bool overlap            = true;
  bool scan_angle         = true;
  bool user_data          = true;
  bool point_source_id    = true;
  bool rgb                = true;
  bool nir                = true;
  bool scanner_channel    = true;
  bool waveform           = true;
  std::vector<int> extra_bytes; // 0-based indices of the requested extra bytes attributes
};

// Streams the points of one or several LAS/LAZ files (merged on the fly) either into
// R vectors (read.las) or into another file (read_and_write.las with an output file).
class RLASstreamer
{
public:
  RLASstreamer(Rcpp::CharacterVector ifiles, Rcpp::CharacterVector ofile, Rcpp::CharacterVector filter);
  ~RLASstreamer();

  RLASstreamer(const RLASstreamer&) = delete;
  RLASstreamer& operator=(const RLASstreamer&) = delete;

  void select(Rcpp::CharacterVector select);
  void allocation();
  void parallel(bool requested);
  bool read_point();
  void write_point();
  const LASpoint& point() const { return lasreader->point; }
  Rcpp::List terminate();

  float progress = 0;

private:
  void set_input_files(Rcpp::CharacterVector ifiles);
  void set_output_file(Rcpp::CharacterVector ofile);
  void set_filter(Rcpp::CharacterVector filter);
  void open();
  void init_lazy_columns(const LASpoint& point);
  void read_waveform();
  bool read_point_parallel();
  std::string parallel_impossible() const;
  Rcpp::List build_R_list();

private:
  LASreadOpener lasreadopener;
  LASwriteOpener laswriteopener;

  // Declaration order matters: members are destroyed in reverse order and the
  // writer (that references the reader's header) must be destroyed first.
  rlas::LASreaderPtr lasreader;
  rlas::LASwaveform13readerPtr laswaveform13reader;
  rlas::LASwriterPtr laswriter;
  LASheader* header = nullptr;

  // Parallel decoding (read.las(parallel = TRUE)). Declared after the reader: the workers are
  // stopped before the reader is destroyed.
  std::unique_ptr<rlas::LazBlocks> blocks;
  std::vector<std::string> input_files;
  std::string filter_string;

  bool in_R       = true;  // false when the points are written into a file
  bool use_filter = false;
  bool ended      = false;
  bool extended   = false; // LAS 1.4 with point format >= 6
  int format      = 0;
  int nalloc      = 0;
  int nsynthetic  = 0;
  int nwithheld   = 0;

  Selection sel;

  // Point attributes
  std::vector<double> X, Y, Z, T;
  std::vector<unsigned short> I, R, G, B, NIR, Channel;
  LazyColumn<unsigned short> RN, NoR, SDF, EoF, C, UD, PSI;
  LazyColumn<bool> Synthetic, Keypoint, Withheld, Overlap;
  LazyColumn<double> SA;  // extended scan angle (point formats >= 6)
  LazyColumn<short> SAR;  // scan angle rank (point formats < 6)

  // Full waveform
  std::vector<int> wavePacketIndex;
  std::vector<U64> wavePacketOffset;
  std::vector<U32> wavePacketSize;
  std::vector<F32> wavePacketLocation;
  std::vector<F32> Xt, Yt, Zt;
  std::vector<std::vector<int>> fullwaveform;
  std::unordered_set<U64> wavePacketRegistry;

  std::vector<RLASExtrabyteAttributes> extra_bytes_attr;
};

#endif //LASSTREAMER_H
