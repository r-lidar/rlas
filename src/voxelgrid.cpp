#include "voxelgrid.hpp"

voxelGrid::voxelGrid(){    
    active = false;
    dynamic = false;
}

voxelGrid::voxelGrid(double vLen){
    active = vLen > 0;
    if(active) setVoxel(vLen);
}

void voxelGrid::setVoxel(double vLen){
    voxelSideLength = vLen;
}

void voxelGrid::setX(double min, double max){
    xMin = min;
    xMax = max;
}

void voxelGrid::setY(double min, double max){
    yMin = min;
    yMax = max;
}

void voxelGrid::setZ(double min, double max){
    zMin = min;
    zMax = max;   
}

int voxelGrid::getLength(double first, double last){
    double distance = last - first;
    int n = floor(distance / voxelSideLength);
    return n;
}

void voxelGrid::startRegistry(){
    xLength = getLength(xMin, xMax) + 1;
    yLength = getLength(yMin, yMax) + 1;
    zLength = getLength(zMin, zMax) + 1;

    std::vector<bool> z(zLength,false);
    std::vector< std::vector<bool> > y(yLength,z);
    std::vector< std::vector< std::vector<bool> > > x(xLength,y);

    registry = x;
}

bool voxelGrid::checkRegistry(double x, double y, double z){
    bool check;

    if(!dynamic){
        unsigned int nx = getLength(xMin, x);
        unsigned int ny = getLength(yMin, y);
        unsigned int nz = getLength(zMin, z);

        check = registry[nx][ny][nz];

        if(!check)
            registry[nx][ny][nz] = true;

    }else{        
        int nx = getLength(0, x);
        int ny = getLength(0, y);
        int nz = getLength(0, z);

        std::vector<int> nVals = {nx, ny, nz};

        int nReg = dynamic_registry.size();
        dynamic_registry.insert(nVals);

        check = (nReg != dynamic_registry.size()) ? false : true;
    }

    return check;
}

void voxelGrid::resetDynamicReg(){
    std::set< std::vector<int> > newDm;
    dynamic_registry = newDm;
}
