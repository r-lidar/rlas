#include "lasfilter_voxelgrid.hpp"

void voxelGrid::setVoxel(double vLen)
{
    voxelSpacing = vLen < 0 ? vLen : -vLen;
}

int voxelGrid::getLength(double first, double last)
{
    double distance = last - first;
    int n = floor(distance / voxelSpacing);
    return n;
}

bool voxelGrid::checkRegistry(double x, double y, double z){

    if(voxelSpacing < 0){
        xAnker = x;
        yAnker = y;
        zAnker = z;
        voxelSpacing = -voxelSpacing;
    }

    int nx = getLength(xAnker, x);
    int ny = getLength(yAnker, y);
    int nz = getLength(zAnker, z);

    std::vector<int> nVals = {nx, ny, nz};

    int nReg = dynamic_registry.size();
    dynamic_registry.insert(nVals);

    return nReg == dynamic_registry.size();
}

void voxelGrid::resetDynamicReg()
{
    std::set< std::vector<int> > newDm;
    dynamic_registry = newDm;
    voxelSpacing = -voxelSpacing;
}