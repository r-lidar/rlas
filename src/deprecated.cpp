#include <Rcpp.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "laswriter.hpp"
#include "rlasstreamer.h"
#include "lasreader.hpp"
#include "laswriter.hpp"
#include "lasfilter.hpp"

bool point_in_polygon(NumericVector, NumericVector, double, double);

using namespace Rcpp;

int get_point_data_record_length(int x);

// [[Rcpp::export]]
void laswriter(CharacterVector file,
               List LASheader,
               NumericVector X,
               NumericVector Y,
               NumericVector Z,
               IntegerVector I = IntegerVector(0),
               IntegerVector RN = IntegerVector(0),
               IntegerVector NoR = IntegerVector(0),
               IntegerVector SDF = IntegerVector(0),
               IntegerVector EoF = IntegerVector(0),
               IntegerVector C = IntegerVector(0),
               IntegerVector SA = IntegerVector(0),
               IntegerVector UD = IntegerVector(0),
               IntegerVector PSI = IntegerVector(0),
               NumericVector T = NumericVector(0),
               IntegerVector R  = IntegerVector(0),
               IntegerVector G = IntegerVector(0),
               IntegerVector B = IntegerVector(0))
{
  try
  {
    class LASheader header;

    header.file_source_ID = (int)LASheader["File Source ID"];
    header.version_major = (int)LASheader["Version Major"];
    header.version_minor = (int)LASheader["Version Minor"];
    header.header_size = (int)LASheader["Header Size"];
    header.offset_to_point_data = header.header_size; // No extra offset since I don't know how to deal with the data in the offset.
    header.file_creation_year = (int)LASheader["File Creation Year"];
    header.point_data_format = (int)LASheader["Point Data Format ID"];
    header.x_scale_factor = (double)LASheader["X scale factor"];
    header.y_scale_factor = (double)LASheader["Y scale factor"];
    header.z_scale_factor = (double)LASheader["Z scale factor"];
    header.x_offset =  (double)LASheader["X offset"];
    header.y_offset =  (double)LASheader["Y offset"];
    header.z_offset =  (double)LASheader["Z offset"];
    header.point_data_record_length = (int)LASheader["Point Data Record Length"];
    strcpy(header.generating_software, "rlas R package");

    std::string filestd = as<std::string>(file);

    LASwriteOpener laswriteopener;
    laswriteopener.set_file_name(filestd.c_str());

    LASpoint p;
    p.init(&header, header.point_data_format, header.point_data_record_length, 0);

    LASwriter* laswriter = laswriteopener.open(&header);

    if(0 == laswriter || NULL == laswriter)
      throw std::runtime_error("LASlib internal error. See message above.");

    for(int i = 0 ; i < X.length() ; i++)
    {
      p.set_x(X[i]);
      p.set_y(Y[i]);
      p.set_z(Z[i]);

      if(I.length() > 0){ p.set_intensity((U16)I[i]); }
      if(RN.length() > 0){ p.set_return_number((U8)RN[i]); }
      if(NoR.length() > 0){ p.set_number_of_returns((U8)NoR[i]); }
      if(SDF.length() > 0){ p.set_scan_direction_flag((U8)SDF[i]); }
      if(EoF.length() > 0){ p.set_edge_of_flight_line((U8)EoF[i]); }
      if(C.length() > 0){ p.set_classification((U8)C[i]); }
      if(SA.length() > 0){ p.set_scan_angle_rank((I8)SA[i]); }
      if(UD.length() > 0){ p.set_user_data((U8)UD[i]); }
      if(PSI.length() > 0){ p.set_point_source_ID((U16)PSI[i]); }
      if(T.length() > 0){ p.set_gps_time((F64)T[i]); }
      if(R.length() > 0)
      {
        p.set_R((U16)R[i]);
        p.set_G((U16)G[i]);
        p.set_B((U16)B[i]);
      }

      laswriter->write_point(&p);
      laswriter->update_inventory(&p);
    }

    laswriter->update_header(&header, true);
    I64 total_bytes = laswriter->close();
    delete laswriter;

    //Rcout << total_bytes << " bytes written" << std::endl;
  }
  catch (std::exception const& e)
  {
    Rcerr << e.what() << std::endl;
  }
}

// [[Rcpp::export]]
List lasdatareader(CharacterVector ifiles, CharacterVector ofile, CharacterVector filter, bool i, bool r, bool n, bool d, bool e, bool c, bool a, bool u, bool p, bool rgb, bool t, IntegerVector eb)
{
  try
  {
    RLASstreamer streamer(ifiles, ofile, filter);

    streamer.read_t(t);
    streamer.read_i(i);
    streamer.read_r(r);
    streamer.read_n(n);
    streamer.read_d(d);
    streamer.read_e(e);
    streamer.read_c(c);
    streamer.read_a(a);
    streamer.read_u(u);
    streamer.read_p(p);
    streamer.read_rgb(rgb);
    streamer.read_nir(false);
    streamer.read_eb(eb);

    streamer.allocation();

    while(streamer.read_point())
    {
      streamer.write_point();
    }

    return streamer.terminate();
  }
  catch (std::exception const& ex)
  {
    Rcerr << "Error: " << ex.what() << std::endl;
    return(List(0));
  }
}


// [[Rcpp::export]]
List lasdatareader_inpoly(CharacterVector ifiles, NumericVector x, NumericVector y, CharacterVector ofile, CharacterVector filter, bool i, bool r, bool n, bool d, bool e, bool c, bool a, bool u, bool p, bool rgb, bool t, IntegerVector eb)
{
  try
  {
    RLASstreamer streamer(ifiles, ofile, filter);

    streamer.read_t(t);
    streamer.read_i(i);
    streamer.read_r(r);
    streamer.read_n(n);
    streamer.read_d(d);
    streamer.read_e(e);
    streamer.read_c(c);
    streamer.read_a(a);
    streamer.read_u(u);
    streamer.read_p(p);
    streamer.read_rgb(rgb);
    streamer.read_eb(eb);

    streamer.allocation();

    while(streamer.read_point())
    {
      double xi = streamer.point()->get_x();
      double yi = streamer.point()->get_y();

      if (point_in_polygon(x, y, xi, yi))
      {
        streamer.write_point();
      }
    }

    return streamer.terminate();
  }
  catch (std::exception const& ex)
  {
    Rcerr << "Error: " << ex.what() << std::endl;
    return(List(0));
  }
}