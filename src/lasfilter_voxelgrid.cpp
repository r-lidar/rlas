#include "lasfilter_voxelgrid.hpp"

void voxelGrid::setVoxel(double vLen){
    voxelSideLength = vLen;
}

int voxelGrid::getLength(double first, double last){
    double distance = last - first;
    int n = floor(distance / voxelSideLength);
    return n;
}

bool voxelGrid::checkRegistry(double x, double y, double z){

    int nx = getLength(0, x);
    int ny = getLength(0, y);
    int nz = getLength(0, z);

    std::vector<int> nVals = {nx, ny, nz};

    int nReg = dynamic_registry.size();
    dynamic_registry.insert(nVals);

    return nReg == dynamic_registry.size();
}

void voxelGrid::resetDynamicReg(){
    std::set< std::vector<int> > newDm;
    dynamic_registry = newDm;
}