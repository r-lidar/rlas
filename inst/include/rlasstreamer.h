#ifndef LASSTREAMER_H
#define LASSTREAMER_H

#include <Rcpp.h>
#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lasfilter.hpp"
#include "rlasextrabytesattributes.h"

using namespace Rcpp;

class RLASstreamer
{
  public:
    RLASstreamer();
    RLASstreamer(CharacterVector);
    RLASstreamer(CharacterVector, CharacterVector);
    RLASstreamer(CharacterVector, CharacterVector, CharacterVector);
    ~RLASstreamer();
    void setinputfiles(CharacterVector);
    void setoutputfile(CharacterVector);
    void setfilter(CharacterVector);
    void select(CharacterVector);
    void allocation();
    bool read_point();
    void write_point();
    LASpoint* point();
    List terminate();
    void read_t(bool);
    void read_i(bool);
    void read_r(bool);
    void read_n(bool);
    void read_d(bool);
    void read_e(bool);
    void read_c(bool);
    void read_s(bool);
    void read_k(bool);
    void read_w(bool);
    void read_o(bool);
    void read_a(bool);
    void read_u(bool);
    void read_p(bool);
    void read_rgb(bool);
    void read_nir(bool);
    void read_cha(bool);
    void read_eb(IntegerVector); // extra byte numbers

  private:
    void initialize_bool();
    void initialize();
    int get_format(U8);

  private:
    std::vector<double> X;
    std::vector<double> Y;
    std::vector<double> Z;
    std::vector<double> T;
    std::vector<unsigned short> I;
    std::vector<unsigned short> RN;
    std::vector<unsigned short> NoR;
    std::vector<unsigned short> SDF;
    std::vector<unsigned short> EoF;
    std::vector<unsigned short> C;
    std::vector<unsigned short> Channel;
    std::vector<bool> Synthetic;
    std::vector<bool> Keypoint;
    std::vector<bool> Withheld;
    std::vector<bool> Overlap;
    std::vector<float> SA;
    std::vector<unsigned short> UD;
    std::vector<unsigned short> PSI;
    std::vector<unsigned short> R;
    std::vector<unsigned short> G;
    std::vector<unsigned short> B;
    std::vector<unsigned short> NIR;

    LASreadOpener lasreadopener;
    LASwriteOpener laswriteopener;
    LASreader* lasreader;
    LASwriter* laswriter;
    LASheader* header;

    int format;
    int nalloc;

    int nsynthetic;
    int nwithheld;

    bool inR;
    bool useFilter;
    bool initialized;
    bool ended;
    bool extended;

    bool t;
    bool i;
    bool r;
    bool n;
    bool d;
    bool e;
    bool c;
    bool s;
    bool k;
    bool w;
    bool o;
    bool a;
    bool u;
    bool p;
    bool rgb;
    bool nir;
    bool cha;
    std::vector<RLASExtrabyteAttributes> extra_bytes_attr;
    std::vector<int> eb;
};

#endif //LASSTREAMER_H
