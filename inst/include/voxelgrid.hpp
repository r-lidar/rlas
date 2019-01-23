#ifndef VOXEL_GRID_H
#define VOXEL_GRID_H

#include <vector>
#include <math.h>
#include <set>

class voxelGrid{

  public:
    voxelGrid();
    voxelGrid(double vLen);
    void setVoxel(double vLen);
    void setX(double min, double max);
    void setY(double min, double max);
    void setZ(double min, double max);
    void startRegistry();
    bool checkRegistry(double x, double y, double z);
    int getLength(double first, double last);

    bool active;

    //read time objs
    void resetDynamicReg();
    bool dynamic;

  private:
    std::vector< std::vector< std::vector<bool> > > registry;

    double xMin;
    double yMin;
    double zMin;
    double xMax;
    double yMax;
    double zMax;
    double voxelSideLength;

    unsigned int xLength;
    unsigned int yLength;
    unsigned int zLength;

    //read time objs
    std::set< std::vector<int> > dynamic_registry;
};

#endif //VOXEL_GRID_H