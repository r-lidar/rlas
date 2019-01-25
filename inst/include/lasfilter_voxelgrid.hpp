#ifndef VOXEL_GRID_H
#define VOXEL_GRID_H

#include <math.h>
#include <set>
#include <stdio.h>
#include <vector>

#include "lasfilter.hpp"
#include "laszip_decompress_selective_v3.hpp"

class voxelGrid{

  public:
    void setVoxel(double vLen);
    int getLength(double first, double last);
    bool checkRegistry(double x, double y, double z);
    void resetDynamicReg();

  private:
    double voxelSpacing;
    double xAnker;
    double yAnker;
    double zAnker;
    std::set< std::vector<int> > dynamic_registry;
};

class LAScriterionThinWithVoxel : public LAScriterion{
  public:
    inline const CHAR* name() const { return "thin_with_voxel"; };
    inline I32 get_command(CHAR* string) const { return sprintf(string, "-%s ", name()); };
    inline U32 get_decompress_selective() const { return LASZIP_DECOMPRESS_SELECTIVE_CHANNEL_RETURNS_XY | LASZIP_DECOMPRESS_SELECTIVE_Z; };
    inline BOOL filter(const LASpoint* point){
      return box.checkRegistry(point->get_x(), point->get_y(), point->get_z());
    };
    void reset(){
      box.resetDynamicReg();
    };
    LAScriterionThinWithVoxel(F32 voxel_resolution){
      box.setVoxel(voxel_resolution);
    };
    ~LAScriterionThinWithVoxel(){ reset(); };

  private:
    voxelGrid box;
};

#endif //VOXEL_GRID_H